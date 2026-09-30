from setuptools import setup

package_name = 'my_rqt_plugin'

setup(
    name=package_name,
    version='0.1.0',
    packages=[package_name],
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
    ],
    install_requires=['setuptools', 'rclpy', 'PyQt5', 'std_msgs'],
    zip_safe=True,
    author='Đạt',
    description='Rehabilitation robot rqt plugin',
    entry_points={
        'console_scripts': [],
        'rqt_gui.plugins': [
            'rehab_plugin = my_rqt_plugin.plugin:RehabPlugin'
        ],
        'rqt_gui.perspectives': [
        ],
    },
)
# from setuptools import setup
# import os
# from glob import glob

# package_name = 'my_rqt_plugin'

# setup(
#     name=package_name,
#     version='0.0.1',
#     packages=[package_name],
#     data_files=[
#         ('share/ament_index/resource_index/packages',
#             ['resource/' + package_name]),
#         ('share/' + package_name, ['package.xml']),
#         (os.path.join('share', package_name), glob('resource/*'))
#     ],
#     install_requires=['setuptools', 'pyqt5', 'rclpy'],
#     zip_safe=True,
#     maintainer='Your Name',
#     maintainer_email='your@email.com',
#     description='GUI for rehabilitation robot exercises',
#     license='Apache License 2.0',
#     tests_require=['pytest'],
#     entry_points={
#         'console_scripts': [
#             'app = my_rqt_plugin.app:main',   # <-- đây cho phép chạy bằng ros2 run
#         ],
#     },
# )
