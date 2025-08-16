from setuptools import setup

package_name = 'rqt_bag_plugins'
setup(
    name=package_name,
    version='1.1.5',
    package_dir={'': 'src'},
    packages=['rqt_bag_plugins'],
    data_files=[
        ('share/' + package_name + '/resource', ['resource/plot.ui']),
        ('share/' + package_name, ['plugin.xml']),
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml'])
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
        'rqt_bag_plugins provides GUI plugins for rqt_bag to display various message types.'
    ),
    license='BSD',
)
