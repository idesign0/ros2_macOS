from setuptools import setup

package_name = 'rqt_service_caller'

setup(
    name=package_name,
    version='1.0.5',
    packages=[package_name],
    package_dir={'': 'src'},
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name + '/resource',
            ['resource/ServiceCaller.ui']),
        ('share/' + package_name, ['package.xml']),
        ('share/' + package_name, ['plugin.xml']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    author='Dorian Scholz',
    maintainer='Dirk Thomas, Dorian Scholz, Mike Lautman',
    maintainer_email='dthomas@osrfoundation.org',
    keywords=['ROS'],
    classifiers=[
        'Intended Audience :: Developers',
        'License :: OSI Approved :: BSD License',
        'Programming Language :: Python',
        'Topic :: Software Development',
    ],
    description=(
        'rqt_service_caller provides a GUI plugin for calling arbitrary services.'
    ),
    license='BSD',
    entry_points={
        'console_scripts': [
            'rqt_service_caller = ' + package_name + '.main:main',
        ],
    },
)
