from setuptools import setup

package_name = 'classic_bags'

setup(
    name=package_name,
    version='0.0.0',
    description='A ROS 2 interface in the style of ROS 1 for reading and writing bag files',
    license='BSD 3-clause',
    maintainer='David V. Lu!!',
    maintainer_email='david@metrorobots.com',
    packages=[package_name],
    data_files=[
        ('share/ament_index/resource_index/packages', ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
    ],
)
