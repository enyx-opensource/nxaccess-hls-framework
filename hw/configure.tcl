set here [file dirname [file normalize [info script]]]
source $here/open_solution.tcl

# Reset project
open_project -reset project_nxaccess_hls

set_top algorithm_entrypoint

add_files $here/project_nxaccess_hls/src/top.cpp -cflags {-fno-builtin -Wno-tautological-compare -I./ -I../include/enyx/hfp/hls -I../include/enyx/hls -I../include/enyx/md/hw -I../include/enyx/oe/hwstrat}
add_files $here/project_nxaccess_hls/src/tick2cancel.cpp -cflags {-fno-builtin -Wno-tautological-compare -I./ }
add_files $here/project_nxaccess_hls/src/tick2trade.cpp -cflags {-fno-builtin -Wno-tautological-compare -I./ }
add_files $here/project_nxaccess_hls/src/configuration.cpp -cflags {-fno-builtin -Wno-tautological-compare -I./ }
add_files $here/project_nxaccess_hls/src/notifications.cpp -cflags {-fno-builtin -Wno-tautological-compare -I./ }

add_files -tb $here/project_nxaccess_hls/sim/top_tb.cpp -cflags {-fno-builtin -Wno-unknown-pragmas -I../include }
add_files -tb $here/project_nxaccess_hls/sim/top_tb_0
#add_files -tb $here/project_nxaccess_hls/src/top.cpp -cflags {-fno-builtin -Wno-tautological-compare -I../include }

set_part {xcvu9p-flgb2104-3-e}
create_clock -period 4ns -name default
# set_clock_uncertainty 1.5ns
# Disable start propagation to ensure II=1 on board.
config_rtl -disable_start_propagation -header "$here/project_nxaccess_hls/version.txt"
config_dataflow -default_channel fifo
# Keep original files names.
config_compile -name_max_length 256

source $here/close_solution.tcl
