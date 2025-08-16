from setuptools import setup

package_name = 'rqt_console'

setup(
    name=package_name,
    version='2.0.3',
    packages=[package_name, package_name + '.filters'],
    package_dir={'': 'src'},
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name + '/resource',
            ['resource/console_settings_dialog.ui',
             'resource/console_widget.ui',
             'resource/text_browse_dialog.ui']),
        ('share/' + package_name + '/resource/filters',
            ['resource/filters/custom_filter_widget.ui',
             'resource/filters/filter_wrapper_widget.ui',
             'resource/filters/list_filter_widget.ui',
             'resource/filters/text_filter_widget.ui',
             'resource/filters/time_filter_widget.ui']),
        ('share/' + package_name, ['package.xml']),
        ('share/' + package_name, ['plugin.xml']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    author='Aaron Blasdel',
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
        'rqt_console provides a GUI plugin for displaying and filtering ROS messages.'
    ),
    license='BSD',
    entry_points={
        'console_scripts': [
            'rqt_console = ' + package_name + '.main:main',
        ],
    },
)
