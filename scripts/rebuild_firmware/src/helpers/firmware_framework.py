import io
import json
import logging
import os
import select
import subprocess
from pathlib import Path
from typing import List

from ruamel.yaml import YAML


logger = logging.getLogger(__name__)


def which(name: str, flags=os.X_OK) -> List:
    """Locates an executable in $PATH

    :param name: executable to locate
    :param flags: expected flag
    :return: list of executables with their paths
    """

    result = []
    paths = os.environ.get('PATH').split(os.pathsep)
    for outer_path in paths:
        for (inner_path, _, _) in os.walk(outer_path):
            path = os.path.join(inner_path, name)
            if os.access(path, flags):
                result.append(os.path.normpath(path))
    return result


def get_vivado_bin_file() -> str:
    """Get the full path to the Vivado executable

    :return: Vivado binary path
    """

    vivado_list = which('vivado')
    if not vivado_list:
        raise Exception('No Vivado executable found in $PATH')
    if len(vivado_list) > 2:
        logging.warning('Multiple versions of vivado: {}'.format(vivado_list))
        logging.warning('Using vivado in {}'.format(vivado_list[0]))
    else:
        logging.debug('Using vivado in {}'.format(vivado_list[0]))
    vivado_bin = vivado_list[0]

    return vivado_bin


class FirmwareFramework:
    """Enyx Firmware Framework class

    Provides methods for managing projects relying on exported framework.
    """

    EXPECTED_BASE_DIRECTORIES = ['project',  'rtl',  'scripts']
    SANDBOX_PARENT_HIERARCHY = 'rtl/ip_cores/others'
    MAIN_SCRIPT = './generate_firmware'
    PRINT_VERSION = '--version'
    CREATE_PROJECT = '--project-creation'
    BUILD_PROJECT = '--project-compilation'
    FIRMWARE_CONFIG_FILE = 'scripts/firmware_config.yaml'
    SUPPORTED_LICENSE_VERSIONS = [1]
    CORE_LICENSES = [
        'ENYX_IP_LICENSE_HFP',
        'ENYX_IP_LICENSE_TCP',
        'ENYX_IP_LICENSE_UDP',
        'ENYX_IP_LICENSE_PCS',]

    def __init__(self, path, dry_run=False) -> None:
        """Construct a new FirmwareFramework class instance.

        :param path: path to an existing firmware framework project
        :param dry_run: if True do not alter framework contents
        """

        self.dry_run = dry_run
        if self.dry_run:
            logging.warning('Dry run mode')

        self.path = FirmwareFramework.get_framework_root(path=path)
        self.sandbox_path = \
            FirmwareFramework.get_sandbox_directory(path=self.path)
        logging.info(f'New FirmwareFramework with path {self.path} and '
                     f'sandbox in {self.sandbox_path}')
        self.config_file = self.path / FirmwareFramework.FIRMWARE_CONFIG_FILE
        self.firmware_config = \
            FirmwareFramework.load_config(path=self.config_file)

    @staticmethod
    def get_framework_root(path: Path) -> Path:
        """
        Locate an Enyx Firmware Framework in the given path.

        :param path: path to an existing firmware framework project
        :return: framework root path
        """

        # convert to absolute path
        path = os.path.abspath(path)

        framework_path_candidates = list()
        for (root, dirs, _) in os.walk(path):
            if all(directory in dirs for directory in
                   FirmwareFramework.EXPECTED_BASE_DIRECTORIES):
                framework_path_candidates.append(Path(root))

        if not framework_path_candidates:
            raise FileNotFoundError(
                f'no firmware frameworks found inside directory {str(path)}')
        if len(framework_path_candidates) > 1:
            raise FileNotFoundError(
                f'multiple firmware frameworks found inside {str(path)}: '
                f'{framework_path_candidates}')
        framework_root = Path(framework_path_candidates[0])
        logging.debug(f'Located framework root directory: {framework_root}')

        return framework_root

    @staticmethod
    def get_sandbox_directory(path: Path) -> Path:
        """
        Get the directory inside the framework containing sandbox HDL files.

        :param path: framework root path
        :return: path to the directory containing unencrypted HDL files
        """

        sandbox_directory = None
        for (root, _, _) in os.walk(path):
            if FirmwareFramework.SANDBOX_PARENT_HIERARCHY in root:
                sandbox_directory = Path(root)
                break
        if not sandbox_directory:
            raise FileNotFoundError(
                f'No sandbox directory located inside {str(path)}')
        logging.debug(
            f'Located sandbox directory: {str(sandbox_directory)}')

        return sandbox_directory

    @staticmethod
    def load_config(path: Path):
        """
        Load firmware configuration from YAML file.

        :param path: framework root path
        :return:
        """

        logger.debug(f'loading config file {path}')

        contents = YAML().load(open(path).read())
        logging.debug(
            f'Loaded firmware config with {len(contents)} sections '
            f'from file {str(path)}')

        return contents

    def run(self, command, work_directory, dry_run=None) -> List:
        """Run command and capture output.

        :param command: sequence of program arguments
        :param work_directory: current directory
        :param dry_run: if set to False or True, disregard self.dry_run

        :return: list of lines from stdout
        """

        if dry_run is None:
            dry_run = self.dry_run

        if not dry_run:
            transcript = []
            logging.debug('Running command: {}'.format(' '.join(command)))
            process = subprocess.Popen(
                    args=command, bufsize=0, stdout=subprocess.PIPE,
                    stderr=subprocess.PIPE,
                    cwd=work_directory, universal_newlines=True)
            while True:
                reads = [process.stdout.fileno(), process.stderr.fileno()]
                ret = select.select(reads, [], [])
                for fd in ret[0]:
                    if fd == process.stdout.fileno():
                        read = process.stdout.readline()
                        if len(read):
                            transcript.append(read)
                            print(read, end='')
                    if fd == process.stderr.fileno():
                        read = process.stderr.readline()
                        if len(read):
                            transcript.append(read)
                            print(read, end='')
                if process.poll() != None:
                    break

            return transcript
        else:
            logging.info(f'Dry run, skipping command: {" ".join(command)}')
            return list()

    def dump_config(self, filename: Path):
        """Dump current firmware configuration in the given file path/name

        :param filename: file name in which settings are to be stored
        :return: file path if created
        """

        stream = io.StringIO()
        YAML().dump(self.firmware_config, stream)

        if self.dry_run:
            logger.info(f'dry run: skipping writing {str(filename)}')
            return None

        YAML().dump(self.firmware_config, open(file=filename, mode='w'))
        return filename

    def update_license(self, filename: Path):
        """Update the firmware framework license data using the given client file

        :param filename: private client file
        :return: nothing
        """

        # Load and check license data

        license_data = json.load(open(filename))['enyx_firmware_license']
        if license_data['version'] not in \
                FirmwareFramework.SUPPORTED_LICENSE_VERSIONS:
            raise ValueError('Unsupported license file version')

        # Compare against already existing licensing data

        app_cfg = self.firmware_config['Application configuration']
        if any(core_license in app_cfg
               for core_license in FirmwareFramework.CORE_LICENSES):
            logger.warning('License data already present, updating with new values')

        # update licenses

        for core_name, core_license in license_data['cores'].items():
            core_license_name = core_name.upper()
            app_cfg[core_license_name] = core_license
            logger.debug(f'updated license {core_license_name}')

        return None

    def update_config(self, filename: Path) -> int:
        """Update the firmware framework configuration  using the given file

        :param filename: user firmware configuration file
        :return: number of settings updated
        """

        # Load user provided firmware configuration

        contents = YAML().load(open(filename).read())
        logger.debug(f'loaded {len(contents.keys())} section from {filename}')

        # update firmware configuration

        updates = 0
        for (k, v) in contents.items():
            if k not in self.firmware_config.keys():
                raise ValueError(f'section {k} is missing from firmware configuration')
            for (setting, value) in v.items():
                logger.debug(f'updating setting {setting} with value {value}')
                self.firmware_config[k][setting] = value
                updates += 1

        return updates

    def build(self) -> int:
        """
        Launch the build process

        :return: zero
        """

        # The Enyx client firmware framework requires a path to the FPGA vendor
        # toolchain.

        scripts_directory = self.path / 'scripts'
        eda_toolchain = get_vivado_bin_file()  # Xilinx only at the moment

        # Display the Firmware Framework version of support purposes

        print_version_command = [
            FirmwareFramework.MAIN_SCRIPT, FirmwareFramework.PRINT_VERSION]
        version = self.run(
            command=print_version_command,
            work_directory=scripts_directory,
            dry_run=False)
        logging.debug(f'Firmware Framework self reported version: {version}')

        # Newly copied files in the sandbox must be incorporated in the project
        # source file list. This is accomplished by recreating the project
        # files.

        create_project_command = [
            FirmwareFramework.MAIN_SCRIPT,
            FirmwareFramework.CREATE_PROJECT,
            eda_toolchain]
        self.run(
            command=create_project_command,
            work_directory=scripts_directory)

        # Rebuild the FPGA firmware

        build_project_command = [
            FirmwareFramework.MAIN_SCRIPT,
            FirmwareFramework.BUILD_PROJECT,
            eda_toolchain]
        self.run(
            command=build_project_command,
            work_directory=scripts_directory)

        return os.EX_OK
