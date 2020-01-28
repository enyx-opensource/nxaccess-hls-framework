import io
import os
import unittest
from pathlib import Path
from unittest import mock

from ruamel.yaml import YAML
from helpers import firmware_framework


class FirmwareFrameworkHelpersTestCase(unittest.TestCase):

    def test_get_vivado_bin_file(self):
        """Test the get_vivado_bin_file() method

        Override the 'which' method in order to return a path containing the
        Vivado executable."""

        which_vivado_paths = [
            '/opt/xilinx2018.3/Vivado/2018.3/bin/', ]

        firmware_framework.which = mock.MagicMock(
            return_value=which_vivado_paths)
        vivado_bin = firmware_framework.get_vivado_bin_file()
        self.assertEqual(which_vivado_paths[0], vivado_bin)


class FirmwareFrameworkTestCase(unittest.TestCase):

    FRAMEWORK_ROOT_PATH = Path('/mock_fs/user/dummy_fwk_root')
    FRAMEWORK_FILE_HIERARCHY = [
        (str(FRAMEWORK_ROOT_PATH), ('project',  'rtl',  'scripts',),
         ('dummy_file',)),
        (str(FRAMEWORK_ROOT_PATH / 'project'), (), ('project_dummy_file',)),
        (str(FRAMEWORK_ROOT_PATH / 'rtl'), ('ip_cores',), ('rtl_dummy_file',)),
        (str(FRAMEWORK_ROOT_PATH / 'rtl/ip_cores'), ('others',), ('rtl_ip_cores_rtl_dummy_file',)),
        (str(FRAMEWORK_ROOT_PATH / 'rtl/ip_cores/others'), (), ('rtl_ip_cores_others_rtl_dummy_file',)),
        (str(FRAMEWORK_ROOT_PATH / 'scripts'), (), ('firmware_config.yaml', 'scripts_dummy_file',)),
    ]
    FIRMWARE_CONFIG = r"""
Application configuration: !!omap

#---------------------------------------------
#   /_\  _ __ _ __| (_)__ __ _| |_(_)___ _ _
#  / _ \| '_ \ '_ \ | / _/ _` |  _| / _ \ ' \
# /_/ \_\ .__/ .__/_|_\__\__,_|\__|_\___/_||_|
#       |_|  |_|
#---------------------------------------------

# The following paramters can be edited ----------------------------------------

# Max number of supported market data multicast stream
- MARKET_STREAM_COUNT_MAX: 8

# Max number of instruments (in thousands)
- MARKET_INSTR_HT_K_INSTR_COUNT_MAX: 2

# Upper limit for the HLS module frequency, the effective frequency depends on internal clock configuration
- MAX_SANDBOX_CLOCK_MHZ: 200

# Max quantity of messages triggered by the HLS
- CSRE_LARGE_COLLECTION_COUNT: 4096

# Size in kBytes of order entry message memory
- CSRE_MESSAGE_MEMORY_KBYTES: 2048

# Number of supported TCP sessions
- TCP_TOTAL_SESSION_COUNT: 4

# Depth of the TCP retry buffer in MTUs for each individual TCP session
- TCP_RETRY_BUFFER_MTU_QTY: 4

# Paramters below must not be modified -----------------------------------------

- EN_MD_AQUISITION: 'true'
- ARCHITECTURE_CHOICE: 0

- MARKET: CME_MDP3
- FEED_FORMAT: NXFEED

- ENABLE_OUTPUT_FILTER: 1
- ENABLE_TIMESTAMPS: 'true'
- ENABLE_DEBUG_STATS_REPORT: 'false'
- DISABLE_BR_DELETE_MODE: 1

- ENABLE_ARBITER: 1
- ARBITER_IS_V2: 1
- ENABLE_LANE_B: 1
- ENABLE_RECOVERY: 1


- LIMIT_MNG_EN: 1
- ENABLE_BOTTOM: 0
- LM_BOTTOM_PROCESSING_CORES: 2
- SIZE_LIMIT_MNG_LOG2: 20

- MASK_ORDER_MNG: 0
- ETHERNET_INPUT_MTU: 2048


- ENABLE_RECOVERY_PFP: 0

- TOP_MAX_FREQ_CLK_MHZ_ENGINE: 250
- DECODER_OUT_CLK_FREQ_MHZ: 250
- ARBITER_CLK_FREQ_MHZ: 160
- MARKET_NORM_CLK_FREQ_MHZ: 160
- OM_CLK_FREQ_MHZ: 160
- OM_OUT_SPE_CLK_FREQ_MHZ: 160
- LM_CLK_FREQ_MHZ: 160
- LM_OUT_SPE_CLK_FREQ_MHZ: 160
- MIXER_CLK_FREQ_MHZ: 200


### MSU GENERIC CONFIGURATION ###
- OUE_CONTENT_FIFO_256_COUNT: 23
- OUE_CONTENT_FIFO_256_FILLING_HW_COUNT: 3
- OUE_CONTENT_FIFO_1024_COUNT: 0
- OUE_CONTENT_PUF_COUNT: 16
- OUE_FIX_CHECKSUM_UNITS: 1
- TCP_MEM_CLK_MHZ: 160
- PAYLOAD_MERGE_CAPABILITY: 'true'

# HLS application name, must match with the name of the HLS platform
- NXACCESS_HLS: 'true'
- SANDBOX_REVISION: c784a36f
- HLS_APPLICATION_NAME: enyx_oe_hwstrat_hls_demo
- ENYX_OE_HWSTRAT_HLS_DEMO: 'true'

# HLS Trigger Arguments Conversion (one per OUE hw filling FIFO)
# HLS_ARGx_CONV_TYPE: one of the following: "bypass", "bin_to_ascii"
# HLS_ARGx_CONV_INPUT_WIDTH: (relevant only for bin_to_ascii) binary input width in bits
# HLS_ARGx_CONV_OUTPUT_WIDTH: (relevant only for bin_to_ascii) ASCII output width in bits

- HLS_ARG0_CONV_TYPE: bin_to_ascii
- HLS_ARG0_CONV_INPUT_WIDTH: 32
- HLS_ARG0_CONV_OUTPUT_WIDTH: 72

- HLS_ARG1_CONV_TYPE: bypass
- HLS_ARG1_CONV_INPUT_WIDTH: -1
- HLS_ARG1_CONV_OUTPUT_WIDTH: -1

- HLS_ARG2_CONV_TYPE: bypass
- HLS_ARG2_CONV_INPUT_WIDTH: -1
- HLS_ARG2_CONV_OUTPUT_WIDTH: -1

- HLS_ARG3_CONV_TYPE: bypass
- HLS_ARG3_CONV_INPUT_WIDTH: -1
- HLS_ARG3_CONV_OUTPUT_WIDTH: -1

- HLS_ARG4_CONV_TYPE: bypass
- HLS_ARG4_CONV_INPUT_WIDTH: -1
- HLS_ARG4_CONV_OUTPUT_WIDTH: -1


- TOP_NAME: top_v18-3_enyx-fpb2-vu9p-3e_mem-eth-hfp_nxaccess_cme-globex-mdp3_basic
- BUILD_DATE: '1573399360'
- HOSTNAME: mmichon
- SEED: 1598018599
- TRUNK_REV_ID: '31326'
- TOP_REV_ID: '31301'
- IS_FMAX: 0
- ENYX_IP_LICENSE_HFP: deadc0decafedeca
- ENYX_IP_LICENSE_TCP: babefacedeadbeef
- ENYX_IP_LICENSE_UDP: defac80013373476
- ENYX_IP_LICENSE_PCS: aa5533cccafedeca
Board configuration: !!omap
- VIVADO_MAJOR: 2018  ## Tool info do not modify
- VIVADO_MINOR: 3 ## Tool info do not modify

#--------------------------
# | _ ) ___  __ _ _ _ __| |
# | _ \/ _ \/ _` | '_/ _` |
# |___/\___/\__,_|_| \__,_|
#
#--------------------------

- PRODUCT_ID: nxAccess CME_MDP3 2.8.1 Basic

- PCIE_EN: 1
- PCIE_GEN: 3
- PCIE_LANE_COUNT: 8
- PCIE_DATA_WIDTH: 256

- PCIE_DMA_MTU: 8192
- PCIE_DMA_BUFFER_POW2: 15

- PCIE_DMA_0_EN: 1

- PCIE_DMA_1_EN: 1
- PCIE_DMA_1_FPGA2CPU_USAGE: audit_trail
- PCIE_DMA_1_CPU2FPGA_USAGE: csre_write
- PCIE_DMA_1_CPU2FPGA_STORE_AND_FORWARD: 'true'

- PCIE_DMA_2_EN: 1
- PCIE_DMA_2_FPGA2CPU_USAGE: feedback
- PCIE_DMA_2_CPU2FPGA_USAGE: csre_trigger
- PCIE_DMA_2_CPU2FPGA_STORE_AND_FORWARD: 'true'

- PCIE_DMA_3_EN: 1
- PCIE_DMA_3_FPGA2CPU_USAGE: user0
- PCIE_DMA_3_CPU2FPGA_USAGE: direct_send
- PCIE_DMA_3_CPU2FPGA_STORE_AND_FORWARD: 'true'

- PCIE_DMA_4_EN: 1
- PCIE_DMA_4_CPU2FPGA_USAGE: user0
- PCIE_DMA_4_CPU2FPGA_STORE_AND_FORWARD: 'true'

- PCIE_NIC_0_EN: 1
- PCIE_NIC_1_EN: 1


- QSFP0_0_EN: 1
- QSFP0_0_ETH_FLOW_CONTROL: 1
- QSFP0_0_MAC_DATA_WIDTH: 64
- QSFP0_0_HOST_NIC_EN: 1
- QSFP0_0_USAGE: MARKET_DATA_A
- QSFP0_PCS_ENYX: 1
- QSFP0_0_MAC_CLK_FREQ_KHZ: 250000

- QSFP1_0_EN: 1
- QSFP1_0_ETH_FLOW_CONTROL: 1
- QSFP1_0_MAC_DATA_WIDTH: 64
- QSFP1_0_HOST_NIC_EN: 1
- QSFP1_0_USAGE: MARKET_DATA_B

- QSFP1_PCS_ENYX: 1
- QSFP1_0_MAC_CLK_FREQ_KHZ: 250000
- QSFP2_0_EN: 1
- QSFP2_PCS_ENYX: 1
- QSFP2_0_MAC_DATA_WIDTH: 64
- QSFP2_0_USAGE: TCP_STACK_0
- QSFP2_0_MAC_CLK_FREQ_KHZ: 250000

- QSFP3_0_EN: 1
- QSFP3_PCS_ENYX: 1
- QSFP3_0_MAC_DATA_WIDTH: 64
- QSFP3_0_USAGE: TCP_STACK_1
- QSFP3_0_MAC_CLK_FREQ_KHZ: 250000


- QDR0_EN: 1
- QDR1_EN: 1
- QDR2_EN: 1
- QDR3_EN: 1
- QDR4_EN: 1
- QDR5_EN: 1
- QDR6_EN: 1
- QDR7_EN: 1


- CUSTOM_CLOCK_0_EN: 'true'
- CUSTOM_CLOCK_0_FREQ_KHZ: 160000

- CUSTOM_CLOCK_1_EN: 'true'
- CUSTOM_CLOCK_1_FREQ_KHZ: 200000

- CUSTOM_CLOCK_2_EN: 'true'
- CUSTOM_CLOCK_2_FREQ_KHZ: 300000

"""
    LICENSE = r"""
{
    "enyx_firmware_license":
    {
        "version": 1,
        "client": "Client_Name",
        "issue_date": "2020-01-03",
        "expiration_date": "2020-04-01",
        "cores":
        {
            "ENYX_IP_LICENSE_HFP": "000000000000000",
            "ENYX_IP_LICENSE_TCP": "111111111111111",
            "ENYX_IP_LICENSE_UDP": "222222222222222",
            "ENYX_IP_LICENSE_PCS": "333333333333333"
        }
    }
}
"""

    USER_CONFIG = r"""
Application configuration: !!omap

#---------------------------------------------
#   /_\  _ __ _ __| (_)__ __ _| |_(_)___ _ _
#  / _ \| '_ \ '_ \ | / _/ _` |  _| / _ \ ' \
# /_/ \_\ .__/ .__/_|_\__\__,_|\__|_\___/_||_|
#       |_|  |_|
#---------------------------------------------

# Max number of supported market data multicast stream
- MARKET_STREAM_COUNT_MAX: 11

# Max number of instruments (in thousands)
- MARKET_INSTR_HT_K_INSTR_COUNT_MAX: 22

# Upper limit for the HLS module frequency, the effective frequency depends on internal clock configuration
- MAX_SANDBOX_CLOCK_MHZ: 33

# Max quantity of messages triggered by the HLS
- CSRE_LARGE_COLLECTION_COUNT: 44

# Size in kBytes of order entry message memory
- CSRE_MESSAGE_MEMORY_KBYTES: 55

# Number of supported TCP sessions
- TCP_TOTAL_SESSION_COUNT: 66

# Depth of the TCP retry buffer in MTUs for each individual TCP session
- TCP_RETRY_BUFFER_MTU_QTY: 77

"""


    @staticmethod
    def mock_builtins_open(file, mode=''):
        if 'yaml' in str(file):
            output = io.StringIO(FirmwareFrameworkTestCase.FIRMWARE_CONFIG)
            return output
        else:
            raise ValueError('unexpected file name')

    def setUp(self) -> None:
        """Prepare test fixture

        """

        os.walk = mock.MagicMock(
            return_value=FirmwareFrameworkTestCase.FRAMEWORK_FILE_HIERARCHY)
        with mock.patch('builtins.open', FirmwareFrameworkTestCase.mock_builtins_open):
            self.fwk = firmware_framework.FirmwareFramework(
                path=FirmwareFrameworkTestCase.FRAMEWORK_ROOT_PATH)
            self.fwk_dry_run = firmware_framework.FirmwareFramework(
                path=FirmwareFrameworkTestCase.FRAMEWORK_ROOT_PATH,
                dry_run=True)

    def test_root_path(self):
        self.assertEqual(self.fwk.path, FirmwareFrameworkTestCase.FRAMEWORK_ROOT_PATH)

    def test_sandbox_path(self):
        sandbox_dir = \
            firmware_framework.FirmwareFramework.SANDBOX_PARENT_HIERARCHY
        expected_sandbox_path = \
            FirmwareFrameworkTestCase.FRAMEWORK_ROOT_PATH / sandbox_dir
        self.assertEqual(self.fwk.sandbox_path, expected_sandbox_path)

    def test_firmware_config(self):
        yaml_config = YAML().load(FirmwareFrameworkTestCase.FIRMWARE_CONFIG)
        self.assertEqual(self.fwk.firmware_config, yaml_config)

    def test_dry_run(self):
        self.assertFalse(self.fwk.dry_run)
        self.assertTrue(self.fwk_dry_run.dry_run)

    def test_dump_config(self):
        fwk_scripts_dir = \
            FirmwareFrameworkTestCase.FRAMEWORK_ROOT_PATH / 'scripts'
        backup_file = fwk_scripts_dir / 'conf_backup.yaml'

        io_mock = mock.MagicMock(wraps=io.StringIO())
        io_mock.__enter__.return_value = io_mock
        io_mock.close = mock.Mock()
        with mock.patch('builtins.open', return_value=io_mock):
            self.fwk.dump_config(filename=backup_file)
        self.assertGreater(len(io_mock.method_calls), 1)

    def test_update_license(self):
        io_mock = mock.MagicMock(
            wraps=io.StringIO(FirmwareFrameworkTestCase.LICENSE))
        io_mock.__enter__.return_value = io_mock
        io_mock.close = mock.Mock()
        with mock.patch('builtins.open', return_value=io_mock):
            self.fwk.update_license(filename="test_license.json")
        self.assertGreater(len(io_mock.method_calls), 0)

    def test_build(self):
        wrapper = mock.MagicMock(
            wraps=firmware_framework.FirmwareFramework.run)
        with mock.patch('helpers.firmware_framework.FirmwareFramework.run', return_value=wrapper) as run_mock:
            self.fwk.build()
        self.assertGreater(run_mock.call_count, 0)

    def test_update_config(self):
        io_mock = mock.MagicMock(
            wraps=io.StringIO(FirmwareFrameworkTestCase.USER_CONFIG))
        io_mock.__enter__.return_value = io_mock
        io_mock.close = mock.Mock()
        with mock.patch('builtins.open', return_value=io_mock):
            self.fwk.update_config(filename=Path("test_user_config.yaml"))

        yaml_user = YAML().load(FirmwareFrameworkTestCase.USER_CONFIG)
        for cat in yaml_user.keys():
            for (setting, value) in yaml_user[cat].items():
                self.assertEqual(self.fwk.firmware_config[cat][setting], value)


if __name__ == '__main__':
    unittest.main()
