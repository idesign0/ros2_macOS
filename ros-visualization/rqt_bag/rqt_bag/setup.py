from setuptools import setup

package_name = 'rqt_bag'
setup(
    name=package_name,
    version='1.1.5',
    package_dir={'': 'src'},
    packages=['rqt_bag', 'rqt_bag.plugins'],
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name + '/resource', ['resource/bag_widget.ui']),
        ('share/' + package_name, ['plugin.xml']),
        ('share/' + package_name, ['package.xml']),
        ('lib/' + package_name, ['scripts/rqt_bag'])
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    author='Dirk Thomas, Aaron Blasdel, Austin Hendrix, Tim Field',
    maintainer='Michael Jeronimo, Mabel Zhang',
    maintainer_email='michael.jeronimo@openrobotics.org',
    keywords=['ROS'],
    classifiers=[
        'Intended Audience :: Developers',
        'License :: OSI Approved :: BSD License',
        'Programming Language :: Python',
        'Topic :: Software Development',
    ],
    description=(
        'rqt_bag provides a GUI plugin for displaying and replaying ROS bag files.'
    ),
    license='BSD',
    scripts=['scripts/rqt_bag'],
)
