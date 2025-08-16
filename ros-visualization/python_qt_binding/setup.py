#!/usr/bin/env python

try:
    from setuptools import setup
except ImportError:
    from distutils.core import setup

try:
    from catkin_pkg.python_setup import generate_distutils_setup
    d = generate_distutils_setup()
except ImportError:
    # extract information from package.xml manually when catkin_pkg is unavailable
    from xml.etree import ElementTree
    tree = ElementTree.parse('package.xml')
    root = tree.getroot()
    d = {
        'name': root.find('./name').text,
        'version': root.find('./version').text,
        'maintainer': root.findall('./maintainer')[0].text,
        'maintainer_email': root.findall('./maintainer')[0].attrib['email'],
        'license': ', '.join([x.text for x in root.findall('./license')]),
        'url': root.findall('./url')[0].text,
        'author': ', '.join([x.text for x in root.findall('./author')]),
    }
    description = root.find('./description').text.strip()
    if len(description) <= 200:
        d['description'] = description
    else:
        d['description'] = description[:197] + '...'
        d['long_description'] = description

d.update({
    'packages': [d['name']],
    'package_dir': {'': 'src'},
    'classifiers': [
        'Development Status :: 5 - Production/Stable',
        'Intended Audience :: Developers',
        'Operating System :: OS Independent',
        'Programming Language :: Python',
        'Topic :: Software Development :: Libraries :: Python Modules',
        'License :: OSI Approved :: BSD License',
        'License :: OSI Approved :: GNU Library or Lesser General Public License (LGPL)',
        'License :: OSI Approved :: GNU General Public License (GPL)',
    ],
})

setup(**d)
