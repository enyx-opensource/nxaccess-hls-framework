"""
Rebuild Firmware module configuration
"""

from setuptools import setup, find_packages


with open('requirements.txt') as f:
    requirements = f.read().splitlines()

setup(
    author='Enyx',
    entry_points={
        'console_scripts': [
            'load_configuration = flows.firmware:load_configuration',
            'build_firmware = flows.firmware:build_firmware',
        ]
    },
    install_requires=requirements,
    packages=find_packages('src'),
    package_dir={'': 'src'},
    python_requires='>=3.6',
    name="rebuild_firmware",
    version="2019-12-04",
)
