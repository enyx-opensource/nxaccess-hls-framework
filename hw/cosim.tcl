set here [file dirname [file normalize [info script]]]
source $here/open_solution.tcl

cosim_design -trace_level port -rtl vhdl

source $here/close_solution.tcl
