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
	$(VIVADO_CMD) -p project_nxaccess_hls

clean:
	echo "This will clean the vivado project files (not the source code)" 
	rm $(PROJECT)/{solution_xcvu9p_3e_nxAccess,.project,vivado_hls.app,.cproject,.apc,.settings} -rf


view_diff_trigger:
	vimdiff project_nxaccess_hls/sim/top_tb_0/tr*{gen,ref}*

