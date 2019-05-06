set here [file dirname [file normalize [info script]]]
source $here/open_solution.tcl

export_design -flow impl -rtl vhdl -format ip_catalog -description "nxAccess Trading Engine" -vendor "Enyx" -library "HLS" -display_name "nxAccess"

source $here/close_solution.tcl
