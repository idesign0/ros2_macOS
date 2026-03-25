# Usage
#     python3 gz-update-assets.py -o <collection_owner> -t <private_token>
#
# Description
#     This script change usage of ignition to gz.
#
import sys,json,requests
import getopt
import fnmatch
import os
import re
import subprocess
import zipfile

# Replace all occurrences of a string with another string inside all 
# files with the provided extension. The extension is treated as
# case-insensitive.
def find_and_replace(directory, find, replace, extension):
  for root, dirs, files in os.walk(directory):
    for file in files:
      base, ext = os.path.splitext(file)
      if ext.lower() == extension.lower():
        filepath = os.path.join(root, file)
        with open(filepath) as f:
          contents = f.read()
        contents = re.sub(find, replace, contents)
        with open(filepath, "w") as f:
          f.write(contents)

if sys.version_info[0] < 3:
    raise Exception("Python 3 or greater is required. Try running `python3 download_collection.py`")

owner_name = ''
private_token = ''

# Read options
optlist, args = getopt.getopt(sys.argv[1:], 'o:t:')

for o, v in optlist:
    if o == "-o":
        owner_name = v.replace(" ", "%20")
    if o == "-t":
        private_token = v

if not owner_name:
    print('Error: missing `-o <owner_name>` option')
    quit()

if not private_token:
    print('Error: missing `-t <private_token>` option')
    quit()

print("Updating models from {}.".format(owner_name))

page = 1
count = 0

# The Fuel server URL.
base_url ='https://fuel.gazebosim.org/'

# Fuel server version.
fuel_version = '1.0'

# Path to get the models in the collection
next_url = '/{}/models?page={}&per_page=100'.format(owner_name, page)

# Path to download a single model in the collection
download_url = base_url + fuel_version + '/{}/models/'.format(owner_name)

download_dir = "dl"
if not os.path.exists(download_dir):
    os.makedirs(download_dir)

# Iterate over the pages
while True:
    url = base_url + fuel_version + next_url

    # Get the contents of the current page.
    r = requests.get(url)

    if not r or not r.text:
        break

    # Convert to JSON
    models = json.loads(r.text)

    # Compute the next page's URL
    page = page + 1
    next_url = '/{}/models?page={}&per_page=100'.format(owner_name, page)
  
    # Download each model 
    for model in models:
        count+=1
        model_name = model['name']
        print ('Downloading (%d) %s' % (count, model_name))
        download = requests.get(download_url+model_name+'.zip', stream=True)
        file = model_name + '.zip'
        dl_file = os.path.join(download_dir, file)
        model_path = os.path.abspath(os.path.join(download_dir, model_name))

        # Download the zip file
        with open(dl_file, 'wb') as fd:
            for chunk in download.iter_content(chunk_size=1024*1024):
                fd.write(chunk)
        # Extract the zip file
        with zipfile.ZipFile(dl_file, 'r') as zip_ref:
            zip_ref.extractall(model_path)
        # Update plugin names
        find_and_replace(model_path, "ignition::gazebo::systems", "gz::sim::systems", "sdf")
        find_and_replace(model_path, r"(?:lib)?ignition-gazebo-([^.\s]*)(?:\.so)?", r"gz-sim-\1", "sdf")
        find_and_replace(model_path, "ignition::gazebo::systems", "gz::sim::systems", "erb")
        find_and_replace(model_path, r"(?:lib)?ignition-gazebo-([^.\s]*)(?:\.so)?", r"gz-sim-\1", "erb")

        # Update fuel server
        find_and_replace(model_path, "fuel.ignitionrobotics", "fuel.gazebosim", "sdf")
        find_and_replace(model_path, "fuel.ignitionrobotics", "fuel.gazebosim", "config")
        find_and_replace(model_path, "fuel.ignitionrobotics", "fuel.gazebosim", "pbtxt")
        find_and_replace(model_path, "fuel.ignitionrobotics", "fuel.gazebosim", "dae")
        find_and_replace(model_path, "fuel.ignitionrobotics", "fuel.gazebosim", "obj")
        find_and_replace(model_path, "fuel.ignitionrobotics", "fuel.gazebosim", "mtl")
        model_url = '{}/{}/{}/models/{}'.format(base_url, fuel_version, owner_name, model_name)
        token_header = 'Private-Token: {}'.format(private_token)
        print("Uploading model {}".format(model_name))
        subprocess.call(['gz', 'fuel', 'edit', '-u', model_url, '-o', owner_name, '--header', token_header, '-m', model_path])


print('Done.')
