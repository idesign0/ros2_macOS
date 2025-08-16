from setuptools import setup

package_name = 'rqt_gui'
setup(
    name=package_name,
    version='1.1.7',
    package_dir={'': 'src'},
    packages=['rqt_gui'],
    data_files=[
        ('share/' + package_name + '/resource', [
            'resource/rqt.graffle',
            'resource/rqt.png',
            'resource/rqt.svg',
        ]),
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
        ('lib/' + package_name, ['bin/rqt_gui'])
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    author='Dirk Thomas',
    author_email='dthomas@osrfoundation.org',
    maintainer='Dirk Thomas',
    maintainer_email='dthomas@osrfoundation.org',
    keywords=['ROS'],
    classifiers=[
        'Intended Audience :: Developers',
        'License :: OSI Approved :: Apache Software License',
        'Programming Language :: Python',
        'Topic :: Software Development',
    ],
    description=(
        'rqt_gui provides the main to start an instance of the ROS integrated graphical user ' +
        'interface provided by qt_gui.'
    ),
    license='BSD',
    tests_require=['pytest'],
    entry_points={
        'console_scripts': [
            'rqt = rqt_gui.main:main',
        ],
    },
)
