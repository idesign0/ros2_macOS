from setuptools import setup

package_name = 'rqt_shell'

setup(
    name=package_name,
    version='1.0.2',
    packages=[package_name],
    package_dir={'': 'src'},
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name + '/resource',
            ['resource/shell_widget.ui']),
        ('share/' + package_name, ['package.xml']),
        ('share/' + package_name, ['plugin.xml']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    author='Dorian Scholz',
    maintainer='Dirk Thomas, Dorian Scholz, Kunal Tyagi',
    maintainer_email='dthomas@osrfoundation.org',
    keywords=['ROS'],
    classifiers=[
        'Intended Audience :: Developers',
        'License :: OSI Approved :: BSD License',
        'Programming Language :: Python',
        'Topic :: Software Development',
    ],
    description=(
        'rqt_shell is a Python GUI plugin providing an interactive shell.'
    ),
    license='BSD',
    entry_points={
        'console_scripts': [
            'rqt_shell = ' + package_name + '.main:main',
        ],
    },
)
