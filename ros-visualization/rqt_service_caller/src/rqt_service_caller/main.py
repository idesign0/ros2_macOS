import sys

from rqt_gui.main import Main


def main():
    main = Main()
    sys.exit(main.main(sys.argv, standalone='rqt_service_caller.service_caller.ServiceCaller'))


if __name__ == '__main__':
    main()
