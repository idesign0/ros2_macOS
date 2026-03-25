from setuptools import setup

package_name = 'angles'

setup(
    name=package_name,
    version='1.16.0',
    package_dir={'': 'src'},
    packages=[package_name],
    data_files=[
        ('share/ament_index/resource_index/packages', ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
    ],
    install_requires=['setuptools'],
    url='http://wiki.ros.org/angles',
    license='BSD',
    author='John Hsu',
    author_email='hsu@osrfoundation.org',
    description='Simple math utilities to work with angles',
)
