import os
from glob import glob
from setuptools import find_packages, setup

package_name = 'ackermann_nlmpc'

setup(
    name=package_name,
    version='1.0.3',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
        (os.path.join('share', package_name, 'launch'), glob('launch/*.py')),
        (os.path.join('share', package_name, 'codegen'), glob('codegen/*.c')),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='Jasper Pflughaupt',
    maintainer_email='j.pflughaupt@uni-luebeck.de',
    description='Lightweight non-linear MPC controller for autonomous driving in 2D environments',
    license='GPL-3.0-only',
    tests_require=['pytest'],
    entry_points={
        'console_scripts': [
            'ackermann_nlmpc_node = ackermann_nlmpc.ackermann_nlmpc_node:main',
            'ackermann_sim_node = ackermann_nlmpc.ackermann_sim_node:main',
            'trajectory_converter_node = ackermann_nlmpc.trajectory_converter_node:main',
        ],
    },
)
