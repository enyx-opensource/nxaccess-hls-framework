set here [file dirname [file normalize [info script]]]
source $here/open_solution.tcl

#csim_design -clean -compiler clang
csim_design -clean -compiler clang

source $here/close_solution.tcl
