from setuptools import setup

package_name = 'rqt_graph'
setup(
    name=package_name,
    version='1.3.1',
    package_dir={'': 'src'},
    packages=[package_name],
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name + '/resource', ['resource/RosGraph.ui']),
        ('share/' + package_name, ['package.xml']),
        ('share/' + package_name, ['plugin.xml']),
        ('lib/' + package_name, ['scripts/rqt_graph']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    author='Dirk Thomas',
    maintainer='Dirk Thomas, Aaron Blasdel',
    maintainer_email='dthomas@osrfoundation.org',
    keywords=['ROS'],
    classifiers=[
        'Intended Audience :: Developers',
        'License :: OSI Approved :: BSD License',
        'Programming Language :: Python',
        'Topic :: Software Development',
    ],
    description=(
        'rqt_graph provides a GUI plugin for visualizing the ROS computation graph.'
    ),
    license='BSD',
    tests_require=['pytest'],
    entry_points={
        'console_scripts': [
            'rqt_graph = rqt_graph.main:main',
        ],
    },
)
