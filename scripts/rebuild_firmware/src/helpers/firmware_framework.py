import io
import json
import logging
import os
import re
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

def get_vivado_version_from_path(path):
    (root_dir,file) = os.path.split(path)
    path_list = []
    while root_dir != os.path.sep:
        path_list.append(file)
        (root_dir,file) = os.path.split(root_dir)
    if path_list[0] != 'vivado':
            raise Exception(
                'Error vivado path : Last word of vivado path is {} while it should be vivado'.format(path_list[0]))
    if path_list[1] != 'bin':
        raise Exception(
            'Error vivado path : Prevous last word of vivado path is {} while it should be bin'.format(path_list[1]))
    return path_list[2]


class FirmwareFramework:
    """Enyx Firmware Framework class

    Provides methods for managing projects relying on exported framework.
    """

    MAIN_SCRIPT = 'generate_firmware'
    PRINT_VERSION = '--version'
    ARG_DEVFRAMEWORK = 'devframework.yaml'
    CREATE_PROJECT = '-pc'
    TOP_NAME_LOCATION = 'rtl/tops'
    DEVFRAMEWORK_CONFIG_FILE = 'scripts/devframework.yaml'
    SUPPORTED_LICENSE_VERSIONS = [1,2,3]
    CORE_LICENSES = [
        'ENYX_IP_LICENSE_HFP',
        'ENYX_IP_LICENSE_TCP_DE',
        'ENYX_IP_LICENSE_UDP_DE',
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
        self.sandbox_path = os.path.join(self.path,"rtl/sandbox/algorithm_entrypoint/src")
        self.topName = self.get_topName()
        logging.info(f'New FirmwareFramework with path {self.path} and '
                     f'sandbox in {self.sandbox_path} '
                     f'for top name {self.topName}')
        self.config_file = os.path.join(self.path, FirmwareFramework.TOP_NAME_LOCATION, self.topName, 'src', 'firmware_config.yaml')
        self.firmware_config = \
            FirmwareFramework.load_config(path=self.config_file)
        self.devframeworkYAML_file = os.path.join(self.path,FirmwareFramework.DEVFRAMEWORK_CONFIG_FILE)
        self.devframeworkYAML_config = \
            FirmwareFramework.load_config(path=self.devframeworkYAML_file)
        self.top_recipe_file = os.path.join(self.path, FirmwareFramework.TOP_NAME_LOCATION, self.topName, 'recipe.txt')

    @staticmethod
    def get_framework_root(path: Path) -> Path:
        """
        Locate an Enyx Firmware Framework in the given path.

        :param path: path to an existing firmware framework project
        :return: framework root path
        """

        # convert to absolute path
        path = os.path.abspath(path)

        # Quick check to see if the folder looks like legit devFramework.
        # More exhaustive checks may be done here
        framework_tree = list(os.walk(path))

        if not framework_tree:
            raise Exception('No folder found at {}'.format(path))

        sandbox_expected_location = os.path.join(framework_tree[0][0],"rtl/sandbox/")
        sandbox_location = [i for i in framework_tree if os.path.samefile(sandbox_expected_location,i[0])]

        if len(sandbox_location) != 1:
            raise Exception(
                'No legit devFramework folder found at {}.There is {} sanbox location while 1 location is expected '.format(path,len(sandbox_location)))

        # if (sandbox_location[0][1]).sort() != (['enyx_oe_hwstrat_hls_demo', 'nxaccess_sandbox']).sort():
        #     raise Exception(
        #         'No legit devFramework folder found at {}. Folders at sandbox location are {} instead of [\'enyx_oe_hwstrat_hls_demo\', \'nxaccess_sandbox\']'.format(path,sandbox_location[0][1]))

        if sandbox_location[0][2] != []:
            raise Exception(
                'No legit devFramework folder found at {}. Files at sandbox location are {} instead of []'.format(path,sandbox_location[0][2]))


        logging.debug(f'Located framework root directory: {path}')

        return path

    def get_topName(self):
        _location = os.path.join(self.path,FirmwareFramework.TOP_NAME_LOCATION)
        _top_list = os.listdir(_location)
        if len(_top_list) != 1:
            raise Exception('Only 1 top is expected at location {}. Top found are {}'.format(_location, _top_list))
        return _top_list[0]

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
            logging.debug('Running command: {} at location {}'.format(' '.join(command), work_directory))
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

    def dump_config(self, filename: Path, config):
        """Dump current firmware configuration in the given file path/name

        :param filename: file name in which settings are to be stored
        :return: file path if created
        """

        stream = io.StringIO()
        YAML().dump(config, stream)

        if self.dry_run:
            logger.info(f'dry run: skipping writing {str(filename)}')
            return None

        YAML().dump(config, open(file=filename, mode='w'))
        return filename

    def update_license(self, filename: Path):
        """Update the firmware framework license data using the given client file

        :param filename: private client file
        :return: nothing
        """

        # Check if file exists
        if not filename:
            logger.info('no license file specified, skipping license update')
        elif not os.path.isfile(filename):
            logger.warning('License file not found, generated firmware will not contain it')
        else:
            logger.info('updating license information')

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

        # Check if file name exist
        if not filename:
            logger.info('no user config specified, skipping config update')
            return 0
        logger.info('updating user configuration')

        # Load user provided firmware configuration
        contents = YAML().load(open(filename).read())
        if not contents:
            logger.warning("YAML file {} is empty".format(filename))
            return 0
        else:
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

    def update_commit(self, value):
        """Update the firmware framework configuration  using the given commit sha

        :param filename: user firmware configuration file
        :return: number of settings updated
        """

        logger.info('updating user configuration whith sha {}'.format(value))

        if 'SANDBOX_REVISION' not in self.firmware_config['Application configuration'].keys():
            raise ValueError(f'section SANDBOX_REVISION is missing from firmware configuration')

        self.firmware_config['Application configuration']['SANDBOX_REVISION'] = value

    def update(self, filename, field, value):
        """Update the firmware framework configuration  using the given commit sha

        :param filename: user firmware configuration file
        :return: number of settings updated
        """
        if field in filename:
            logger.info('Overwritte {} field with value {}'.format(field,value))
            filename[field] = value
        else:
            logger.info('Create new {} field with value {}'.format(field,value))
            filename[field] = value

    def update_synthesis_tool(self):
        eda_toolchain = get_vivado_bin_file()  # Xilinx only at the moment
        vivado_version = str(get_vivado_version_from_path(eda_toolchain))
        top_name_vivado_version = str(self.get_vivado_version_from_top())
        if vivado_version != top_name_vivado_version:
            raise Exception(
                'Error vivado version for top is {} while system tool vivado version is {}'.format(top_name_vivado_version,vivado_version))
        logger.info('Update vivado {} path with {}'.format(vivado_version,eda_toolchain))
        self.devframeworkYAML_config['SYNTHESIS_TOOL_BIN']['xilinx']['xilinx'+vivado_version] = eda_toolchain

    def get_vivado_version_from_top(self):
        with open(self.top_recipe_file, 'r') as f:
            recipe = json.load(f)
        if 'vivado' not in recipe.keys():
            raise Exception("Error : field vivado is not present in top recipe {}".format(self.top_recipe_file))
        return recipe['vivado']

    def build(self, dry_run) -> int:
        """
        Launch the build process

        :return: return code
        """

        scripts_directory = os.path.join(self.path,'scripts')

        # Display the Firmware Framework version of support purposes

        print_version_command = [
            os.path.join(scripts_directory,FirmwareFramework.MAIN_SCRIPT), FirmwareFramework.PRINT_VERSION]
        version = self.run(
            command=print_version_command,
            work_directory=None,
            dry_run=False)
        logging.debug(f'Firmware Framework self reported version: {version}')

        # Newly copied files in the sandbox must be incorporated in the project
        # source file list. This is accomplished by recreating the project
        # files.

        create_project_command = [
            os.path.join(scripts_directory,FirmwareFramework.MAIN_SCRIPT),
            os.path.join(scripts_directory,FirmwareFramework.ARG_DEVFRAMEWORK),
            self.topName,
            FirmwareFramework.CREATE_PROJECT]
        transcript = self.run(
            command=create_project_command,
            work_directory=None,
            dry_run=dry_run)


        if any([re.search(r'Failed ".+?" after', line) for line in transcript]):
            logging.error(f'Build project failure reported by firmware framework')
            return 1

        return os.EX_OK
