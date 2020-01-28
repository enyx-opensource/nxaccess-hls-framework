#!/usr/bin/env python

"""Enyx FPGA Firmware Framework

Client helper script
"""


import argparse
import datetime
import logging
import sys
from os import PathLike
from pathlib import Path
from shutil import copy2
from typing import List

from helpers.firmware_framework import FirmwareFramework
from helpers.hls_project_repository import HlsProjectRepository

LOG_FORMAT = ('%(asctime)s - %(levelname)s - %(module)s - '
              '%(funcName)s - %(message)s')
logger = logging.getLogger(__name__)


def copy_files(files: List, target_dir: PathLike) -> List[Path]:
    """Copy a list of files into a target directory.

    :param files: list of files with paths to be copied
    :param target_dir: existing target directory
    :return: number of files copied
    """

    copied_files = list()
    for file in files:
        logger.debug(f'Copying {file} to {target_dir}')
        copy2(file, target_dir)
        copied_files.append(file)

    return copied_files


def configure_logger(verbose: bool) -> None:
    """Configure logging handler

    :param verbose: display debug and info messages
    :return: nothing
    """

    log = logging.getLogger()
    log.handlers = []
    stdout = logging.StreamHandler(sys.stdout)
    stdout.setLevel(level=logging.WARNING)
    stdout.setFormatter(logging.Formatter(LOG_FORMAT))
    log.addHandler(stdout)
    if verbose:
        stdout.setLevel(level=logging.DEBUG)
        log.setLevel(level=logging.DEBUG)


def rebuild(argv: List = None) -> int:
    """Import HDL files from HLS repository and rebuild firmware

    :param argv: optional argv override, used for testing purposes
    :return: shell exit code
    """

    # Parse command line arguments and configure logging

    if not argv:
        argv = sys.argv[1:]
    parser = argparse.ArgumentParser()
    arg = parser.add_argument
    arg('firmware-framework-path',
        help='path to the firmware framework directory')
    arg('--license-file', help='private client license file')
    arg('--user-config-file', help='user firmware configuration file')
    arg('--hls-path', help='HLS project repository alternate path')
    arg('-n', '--dry-run', action='store_true', help='force dry run mode')
    arg('-v', '--verbose', action='store_true', help='display debug messages')
    arguments = vars(parser.parse_args(argv))
    configure_logger(verbose=arguments['verbose'])
    logging.debug(f'Arguments: {arguments}')
    dry_run = arguments['dry_run']
    if dry_run:
        logger.warning('dry run mode requested')

    # Collect information from the HLS project repository

    hls_project = HlsProjectRepository(path=arguments['hls_path'])
    hdl_files = hls_project.list_hdl_files(language='vhdl')
    commit = hls_project.get_last_commit()

    # Open an Enyx FPGA firmware framework

    fw_fwk_path = Path(arguments['firmware-framework-path'])
    fw_fwk = FirmwareFramework(path=fw_fwk_path, dry_run=dry_run)

    # Backup framework project settings for support purposes

    fw_config_backup_name = (
        'firmware_config_backup_'
        f'{datetime.datetime.now().isoformat()}.yaml')
    fw_config_backup_path = fw_fwk_path / 'scripts' / fw_config_backup_name
    fw_fwk.dump_config(filename=fw_config_backup_path)

    # Update framework project license

    if arguments['license_file']:
        logger.info('updating license information')
        fw_fwk.update_license(filename=arguments['license_file'])
        fw_fwk.dump_config(filename=fw_fwk.config_file)
    else:
        logger.info('no license file specified, skipping license update')

    # Update framework configuration

    if arguments['user_config_file']:
        logger.info('updating user configuration')
        fw_fwk.update_config(filename=arguments['user_config_file'])
        fw_fwk.dump_config(filename=fw_fwk.config_file)
    else:
        logger.info('no user config specified, skipping config update')

    # Update framework project HLS commit value

    fw_fwk.sandbox_commit = commit
    fw_fwk.dump_config(filename=fw_fwk.config_file)

    # Copy HDL files from the HLS repository over to the firmware framework

    if not dry_run:
        copy_files(files=hdl_files, target_dir=fw_fwk.sandbox_path)
    else:
        logging.info(f'dry run: skipping copying of {len(hdl_files)} files')

    # Rebuild the firmware

    return_code = fw_fwk.build()

    return return_code


def main() -> int:

    return rebuild()


if __name__ == '__main__':
    sys.exit(main())
