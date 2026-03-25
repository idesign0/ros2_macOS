\page cmdline Command Line Tools

Next Tutorial: \ref cppapi

## Overview

The `gz fuel` command line tool provides a convenient way to handle [Gazebo
Fuel](https://app.gazebosim.org) resources from a terminal.

## List resources

Let's use the command line to list resources available in a web server.
We can do that by running the `gz fuel list` command and pass it the resource
type, such as `model` or `world` as follows:

`gz fuel list -t world`

You should see a list such as:

```
Fetching world list from https://fuel.gazebosim.org...
Received world list (took 350ms).
https://fuel.gazebosim.org
├── OpenRobotics
│   ├── Empty
│   └── Shapes
└── chapulina
    └── Shapes copy
```

By default, Fuel will list resources from all the servers listed in your
`config.yaml` file. See the \ref configuration for more details.

> **Tip**: If you want to see resources from a different Fuel server, add it to `config.yaml` to and re-run the list command to see resources from both servers!

There are a few other options on the command, run the help to see all of them:

`gz fuel list --help`

### Raw output

One of them is the `--raw` option, which tells the command to print the output in
a way that's easier for scripts to parse. For example, try:

`gz fuel list -t world -r`

And you'll get a list of the world URLs similar to the one below:

```
https://fuel.gazebosim.org/1.0/OpenRobotics/worlds/Empty
https://fuel.gazebosim.org/1.0/OpenRobotics/worlds/Shapes
https://fuel.gazebosim.org/1.0/chapulina/worlds/Shapes%20copy
```

### By owner

It's also possible to only list resources belonging to a given user, using the
`--owner` flag. Try for example:

`gz fuel list -t model -o OpenRobotics`

## Download resources

The command line tool also allows downloading resources from a web server to your
computer. We use the `gz fuel download` tool for this.

We learned above how to get resource URLs. Now we can use these URLs to download
them. For example, try:

`gz fuel download -v 3 -u https://fuel.gazebosim.org/1.0/OpenRobotics/worlds/Fortress%20Demo`

Note that we passed the `-v 3` option so we get a verbose output. You should see something like:

```
Downloading world:
  Name: fortress%20demo
  Owner: openrobotics
  Server:
    URL: https://fuel.gazebosim.org
    Version: 1.0

[Msg] Downloading world [fuel.gazebosim.org/openrobotics/worlds/fortress%20demo]
[Msg] Saved world at:
  /home/janedoe/.gz/fuel/fuel.gazebosim.org/openrobotics/worlds/fortress%20demo/1
Download succeeded.
```

If the model is private you can create a config file with your token. For example, create a file
`/tmp/my_config.yaml` with the following content and edit your token:

```yaml
---
# The list of servers.
servers:
  -
    url: https://fuel.gazebosim.org
    private-token: <your private token>
```

Then try to download the model:

```bash
gz fuel download -v 4 -u https://fuel.gazebosim.org/1.0/OpenRobotics/worlds/fortress%20demo -c /tmp/my_config.yaml
```

Worlds downloaded with the tool get conveniently organized into the same
directory, which we call the "local cache". The path is broken down as follows:

`<local cache path>/<server URL>/<owner name>/<resource type>/<resource name>/<version number>`

> **Tip**: You can change the local cache path in `config.yaml`.

> **Tip**: You can also use other tools such as `wget` to download a zipped file of a world, just add `.zip` to the end of the URL, for example: `wget https://fuel.gazebosim.org/1.0/OpenRobotics/worlds/Fortress%20Demo.zip`.

## Edit resources

It's possible to edit a resource from the command line after it has been
uploaded to Fuel. Edit functionality is available through the `edit`
sub-command. You can access the list of edit options using:

```
gz fuel edit -h
```

You must be the resource owner, or an authorized member of the organization that owns the resource, in order to edit the resource. This in turn means you must use the next option with the `edit` subcommand:
```
--header 'Private-token: YOUR_TOKEN'
```

### Change resource privacy

The edit sub-command supports toggling a resource's status between pubic and
private. A public resource is accessible to everyone, while a private
resource is accessible only to the owner. The owner of a resource could be
an organization, in which case all members of the organization would have
access.

Use the `-p` option to make a resources private. For example:

```
gz fuel edit -p -u https://fuel.gazebosim.org/1.0/OpenRobotics/worlds/Fortress%20Demo --header 'Private-token: YOUR_TOKEN'
```

Use the `-b` option to make a resource public. For example:

```
gz fuel edit -b -u https://fuel.gazebosim.org/1.0/OpenRobotics/worlds/Fortress%20Demo --header 'Private-token: YOUR_TOKEN'
```
