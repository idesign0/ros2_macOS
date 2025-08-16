from setuptools import setup

package_name = 'rqt_srv'

setup(
    name=package_name,
    version='1.0.3',
    packages=[package_name],
    package_dir={'': 'src'},
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
        ('share/' + package_name, ['plugin.xml']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    author='Aaron Blasdel',
    maintainer='Dirk Thomas, Aaron Blasdel, Michael Lautman',
    maintainer_email='dthomas@osrfoundation.org',
    keywords=['ROS'],
    classifiers=[
        'Intended Audience :: Developers',
        'License :: OSI Approved :: BSD License',
        'Programming Language :: Python',
        'Topic :: Software Development',
    ],
    description=(
        'A Python GUI plugin for introspecting available ROS message types. ' +
        'Note that the srvs available through this plugin is the ones that are stored ' +
        'on your machine, not on the ROS core your rqt instance connects to.'
    ),
    license='BSD',
    entry_points={
        'console_scripts': [
            'rqt_srv = ' + package_name + '.main:main',
        ],
    },
)
