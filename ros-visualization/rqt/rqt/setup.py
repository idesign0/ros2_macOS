from setuptools import setup

package_name = 'rqt'
setup(
    name=package_name,
    version='1.1.7',
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
    ],
    zip_safe=True,
    author='Dirk Thomas, Dorian Scholz, Aaron Blasdel',
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
        'rqt is a Qt-based framework for GUI development for ROS. It consists of three ' +
        'parts/metapackages. ' +
        'rqt_common_plugins- ROS backend tools suite that can be used on/off of robot runtime. ' +
        'rqt_robot_plugins - Tools for interacting with robots during their runtime. ' +
        'rqt_gui - that enables multiple `rqt` widgets to be docked in a single window.'
    ),
    packages=[],
    license='BSD',
)
