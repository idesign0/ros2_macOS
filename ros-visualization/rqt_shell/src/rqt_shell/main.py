import sys

from rqt_gui.main import Main
from rqt_shell.shell import Shell


def main():
    plugin = 'rqt_shell.shell.Shell'
    main = Main(filename=plugin)
    sys.exit(main.main(standalone=plugin, plugin_argument_provider=Shell.add_arguments))


if __name__ == '__main__':
    main()
