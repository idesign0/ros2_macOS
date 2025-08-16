import sys

from rqt_gui.main import Main
from rqt_msg.messages import Messages


def main():
    plugin = 'rqt_msg.messages.Messages'
    main = Main(filename=plugin)
    sys.exit(main.main(standalone=plugin))


if __name__ == '__main__':
    main()
