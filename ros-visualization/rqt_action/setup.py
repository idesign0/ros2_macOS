from setuptools import setup

package_name = 'rqt_action'
setup(
    name=package_name,
    version='2.0.1',
    package_dir={'': 'src'},
    packages=[package_name],
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
        ('share/' + package_name, ['plugin.xml'])
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    author='Isaac Isao Saito',
    maintainer='Mikael Arguedas, Aaron Blasdel, Geoffrey Biggs, Mabel Zhang',
    maintainer_email='mikael@osrfoundation.org',
    keywords=['ROS'],
    classifiers=[
        'Intended Audience :: Developers',
        'License :: OSI Approved :: BSD License',
        'Programming Language :: Python',
        'Topic :: Software Development',
    ],
    description=(
        'rqt_action provides a feature to introspect all available ROS action '
        'types. By utilizing rqt_msg, the output format is unified with it and rqt_srv. Note that '
        'the actions shown on this plugin is the ones that are stored on your machine, not on the '
        'ROS core your rqt instance connects to.'
    ),
    license='BSD',
    entry_points={
        'console_scripts': [
            'rqt_action = ' + package_name + '.main:main',
        ],
    },
)
