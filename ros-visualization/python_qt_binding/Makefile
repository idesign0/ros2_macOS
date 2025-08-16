.PHONY: all setup clean_dist distro clean install upload push

NAME=python_qt_binding
VERSION=`./setup.py --version`

all:
	echo "noop for debbuild"

setup:
	echo "building version ${VERSION}"

clean_dist:
	-rm -f MANIFEST
	-rm -rf dist

distro: setup clean_dist
	python setup.py sdist

push: distro
	python setup.py sdist register upload
	scp dist/${NAME}-${VERSION}.tar.gz ros@ftp-osl.osuosl.org:/home/ros/data/download.ros.org/downloads/${NAME}

clean: clean_dist
	echo "clean"

install: distro
	sudo checkinstall python setup.py install

testsetup:
	echo "running ${NAME} tests"

test: testsetup
	cd test && nosetests
