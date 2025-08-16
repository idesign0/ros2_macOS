from setuptools import setup

package_name = 'rqt_gui_py'
setup(
    name=package_name,
    version='1.1.7',
    package_dir={'': 'src'},
    packages=['rqt_gui_py'],
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
        ('share/' + package_name, ['plugin.xml'])
    ],
    install_requires=['setuptools'],
    zip_safe=False,
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
        'rqt_gui_py enables GUI plugins to use the Python client library for ROS.'
    ),
    license='BSD',
    tests_require=['pytest'],
)
