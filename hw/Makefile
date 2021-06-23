#--------------------------------------------------------------------------------
#--! Enyx Confidential
#--!
#--! Organization:          Enyx
#--! Project Identifier:    010 - Enyx nxAccess HLS Framework
#--! Author:                Raphael Charolois (raphael.charolois@enyx.com)
#--!
#--! © Copyright            Enyx 2019
#--! © Copyright Notice:    The source code for this program is not published or otherwise divested of its trade secrets,
#--!                        irrespective of what has been deposited with the U.S. Copyright Office.
#--------------------------------------------------------------------------------

PROJECT=project_nxaccess_hls
VIVADO_CMD=vivado_hls
PY_DIR=./../scripts/rebuild_firmware

create_project:
	echo "Will prepare Vivado HLS project ! "
	$(VIVADO_CMD) configure.tcl
	echo "Project prepared."
	$(VIVADO_CMD) open_solution.tcl
	echo "Solution created."

build: csim csynth cosim

csim:
	$(VIVADO_CMD) csim.tcl
csynth:
	$(VIVADO_CMD) csynth.tcl
cosim:
	$(VIVADO_CMD) cosim.tcl

open_gui:
	$(VIVADO_CMD) -p $(PROJECT)

$(PY_DIR)/venv:
	$(PY_DIR)/install

firmware: load_configuration build_firmware

load_configuration: $(PY_DIR)/venv
	$(PY_DIR)/load_configuration -v \
		./../../devFramework \
		--license-file ./../firmware_license.json \
		--user-config-file ./../user_firmware_config.yaml

build_firmware: $(PY_DIR)/venv
	$(PY_DIR)/build_firmware -v \
		./../../devFramework

.PHONY: clean
clean:
	${RM} -r $(PROJECT)/solution_xcvu9p_3e_nxAccess/
	${RM} $(PROJECT)/vivado_hls.app
	${RM} vivado_hls.log vivado.jou

view_diff_trigger:
	vimdiff project_nxaccess_hls/sim/top_tb_0/tr*{gen,ref}*

show_timing_info:
	@grep -B1 -A10  "Design Timing Summary" ../../devFramework/project/synth_00.runs/impl_1/shell_top_bittware_xupp3r_vu9p_mem_eth_dyn_timing_summary_routed.rpt

