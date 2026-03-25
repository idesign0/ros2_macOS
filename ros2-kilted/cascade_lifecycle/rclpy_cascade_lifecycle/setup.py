import os

from setuptools import setup

package_name = 'rclpy_cascade_lifecycle'

setup(
    name=package_name,
    version='2.0.4',
    packages=[package_name],
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
        (os.path.join('share', package_name, 'resource'),
            ['resource/rclpy_cascade_lifecycle']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='Juan Carlos Manzanares Serrano',
    maintainer_email='jc.manzanares.serrano@gmail.com',
    description='rclpy cascade lifecycle',
    license='Apache License, Version 2.0',
    tests_require=['pytest'],
    entry_points={
        'ros2cli.command_plugins': [
            'cascade_lifecycle = \
                rclpy_cascade_lifecycle.cascade_lifecycle_node:\
                CascadeLifecycleNode',
        ],
    },
)
