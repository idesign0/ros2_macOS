from setuptools import setup

package_name = 'rqt_reconfigure'
setup(
    name=package_name,
    version='1.1.2',
    package_dir={'': 'src'},
    packages=[package_name],
    data_files=[
        ('share/' + package_name + '/resource', [
            'resource/editor_bool.ui',
            'resource/editor_enum.ui',
            'resource/editor_number.ui',
            'resource/editor_string.ui',
            'resource/node_selector.ui',
            'resource/paramedit_pane.ui',
            'resource/param_main.ui',
            'resource/param_timeline.ui',
            'resource/singlenode_parameditor.ui',
            'resource/text_filter_widget.ui',
        ]),
        ('share/' + package_name, ['package.xml']),
        ('share/' + package_name, ['plugin.xml']),
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    author="Isaac Saito, Ze'ev Klapow",
    maintainer='Scott K Logan',
    maintainer_email='logans@cottsay.net',
    keywords=['ROS'],
    classifiers=[
        'Intended Audience :: Developers',
        'License :: OSI Approved :: BSD License',
        'Programming Language :: Python',
        'Topic :: Software Development',
    ],
    description=(
        "This rqt plugin succeeds former dynamic_reconfigure's GUI " +
        '(reconfigure_gui), and provides the way to view and edit the parameters ' +
        'that are accessible via dynamic_reconfigure.'
    ),
    license='BSD',
    tests_require=['pytest'],
    entry_points={
        'console_scripts': [
            package_name + ' = ' + package_name + '.__main__:main',
        ],
    },
)
