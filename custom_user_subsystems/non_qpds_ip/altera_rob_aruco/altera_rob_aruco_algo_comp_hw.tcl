###################################################################################
# Copyright (C) Altera Corporation
#
# This software and the related documents are Altera copyrighted materials, and
# your use of them is governed by the express license under which they were
# provided to you ("License"). Unless the License provides otherwise, you may
# not use, modify, copy, publish, distribute, disclose or transmit this software
# or the related documents without Altera's prior written permission.
#
# This software and the related documents are provided as is, with no express
# or implied warranties, other than those that are expressly stated in the License.
###################################################################################

source ./common_tcl/altera_vvp_helper.tcl
source ./common_tcl/altera_vvp_files.tcl
source ./common_tcl/altera_vvp_parameters.tcl
source ./common_tcl/altera_vvp_interfaces.tcl

# Common module properties for VIP components
declare_vvp_component_properties

set_module_property NAME altera_rob_aruco_algo_comp
set_module_property DISPLAY_NAME "Icon algorithmic component"

# --------------------------------------------------------------------------------------------------
# --                                                                                              --
# -- Callbacks                                                                                    --
# --                                                                                              --
# --------------------------------------------------------------------------------------------------

# Validation callback to check legality of parameter set
set_module_property   VALIDATION_CALLBACK         validation_cb

# Callback for the elaboration of this component
set_module_property   ELABORATION_CALLBACK        elaboration_cb


# --------------------------------------------------------------------------------------------------
# --                                                                                              --
# -- Files                                                                                        --
# --                                                                                              --
# --------------------------------------------------------------------------------------------------
add_static_sv_file src_hdl/altera_rob_aruco_pkg.sv
add_static_sv_file src_hdl/altera_rob_aruco_axi_pipeline_stage.sv
add_static_sv_file src_hdl/altera_rob_aruco_axi_master.sv
add_static_sv_file src_hdl/altera_rob_aruco_axi_zero_pad.sv
add_static_sv_file src_hdl/altera_rob_aruco_algo_comp.sv

for {set aruco_id 0} {$aruco_id <= 10} {incr aruco_id} {
  set marker_id [format "%02d" $aruco_id]
  foreach pip {1 2 4 8} {
    add_static_mif_file "aruco_4x4_50_id${marker_id}_altera_${pip}_pip.mif" src_hdl/aruco_markers
  }
}

setup_filesets altera_rob_aruco_algo_comp

# --------------------------------------------------------------------------------------------------
# --                                                                                              --
# -- Parameters                                                                                   --
# --                                                                                              --
# --------------------------------------------------------------------------------------------------
add_bps_parameters
add_pixels_in_parallel_parameters
set_parameter_property  PIXELS_IN_PARALLEL   ALLOWED_RANGES          {1 2 4 8}
add_parameter           ARUCO_ID             INTEGER                 0
set_parameter_property  ARUCO_ID             DISPLAY_NAME            "ArUco marker ID"
set_parameter_property  ARUCO_ID             ALLOWED_RANGES          0:10
set_parameter_property  ARUCO_ID             HDL_PARAMETER           true
set_parameter_property  ARUCO_ID             AFFECTS_ELABORATION     false

add_parameter           ICON_WIDTH           INTEGER                 128
set_parameter_property  ICON_WIDTH           DISPLAY_NAME            "Icon width"
set_parameter_property  ICON_WIDTH           ALLOWED_RANGES          16:1024
set_parameter_property  ICON_WIDTH           HDL_PARAMETER           true
set_parameter_property  ICON_WIDTH           AFFECTS_ELABORATION     false

add_parameter           ICON_HEIGHT          INTEGER                 128
set_parameter_property  ICON_HEIGHT          DISPLAY_NAME            "Icon height"
set_parameter_property  ICON_HEIGHT          ALLOWED_RANGES          16:1024
set_parameter_property  ICON_HEIGHT          HDL_PARAMETER           true
set_parameter_property  ICON_HEIGHT          AFFECTS_ELABORATION     false

add_device_family_parameters
add_display_item        "General"            ARUCO_ID                parameter


# --------------------------------------------------------------------------------------------------
# --                                                                                              --
# -- Validation callback                                                                          --
# -- Checking the legality of the parameter set chosen by the user                                --
# --                                                                                              --
# --------------------------------------------------------------------------------------------------

proc validation_cb {} {

}


# --------------------------------------------------------------------------------------------------
# --                                                                                              --
# -- Dynamic ports (elaboration callback)                                                         --
# --                                                                                              --
# --------------------------------------------------------------------------------------------------
proc elaboration_cb {} {

   set   v_bps               [get_parameter_value BPS]
   set   v_num_colors        3
   set   v_pip               [get_parameter_value PIXELS_IN_PARALLEL]

   add_clock_reset_intf   main

   add_axi_st_vvp_data_output_port  axi_st_data_out   main  ${v_bps}  ${v_num_colors}   ${v_pip}

}
