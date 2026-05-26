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

set_shell_parameter AVMM_HOST                 {{AUTO X}}
set_shell_parameter AVMM_HOST_2               {{AUTO X}}
set_shell_parameter EN_HPS_HOST               {0}

# General Video Controls
set_shell_parameter PIP                       {1}
set_shell_parameter VID_OUT_RATE              "p60"
set_shell_parameter VID_OUT_BPS               {10}
set_shell_parameter EN_DEBUG                  {1}
set_shell_parameter EN_FRAME_RD               {0}
set_shell_parameter EN_GS_OUTPUT              {0}

# Interrupts
set_shell_parameter LRES_VFW_IRQ_PRIORITY     "X"
set_shell_parameter LRES_VFW_IRQ_HOST         ""
set_shell_parameter LRES_VFW_GS_IRQ_PRIORITY  "X"
set_shell_parameter LRES_VFW_GS_IRQ_HOST      ""
set_shell_parameter LRES_VFR_IRQ_PRIORITY     "X"
set_shell_parameter LRES_VFR_IRQ_HOST         ""

# ArUco marker generation

set_shell_parameter ARUCO_MK_EN               {0}
set_shell_parameter ARUCO_MK_QTY              {5}

# EMIF settings
set_shell_parameter FPGA_EMIF_ASYNC           {0}
set_shell_parameter FPGA_EMIF_ASYNC_CLK_HZ    {1000000}

proc pre_creation_step {} {

    transfer_files

}

proc creation_step {} {

    create_isp_rob_subsystem

}

proc post_creation_step {} {

    edit_top_level_qsys
    add_auto_connections
    edit_top_v_file

}

proc transfer_files {} {

    set v_project_path      [get_shell_parameter PROJECT_PATH]
    set v_script_path       [get_shell_parameter SUBSYSTEM_SOURCE_PATH]
    set v_aruco_mk_en       [get_shell_parameter ARUCO_MK_EN]
    set v_aruco_mk_qty      [get_shell_parameter ARUCO_MK_QTY]

    # Copy Aruco marker generation
    if {${v_aruco_mk_en}} {
        exec cp -rf ${v_script_path}/../../non_qpds_ip/altera_rob_aruco \
                                                      ${v_project_path}/non_qpds_ip/user/altera_rob_aruco
        file_copy   ${v_script_path}/../../non_qpds_ip/altera_rob_aruco.ipx     ${v_project_path}/non_qpds_ip/user
    }
}

proc create_isp_rob_subsystem {} {

    set v_project_path              [get_shell_parameter PROJECT_PATH]
    set v_instance_name             [get_shell_parameter INSTANCE_NAME]
    set v_en_hps_host               [get_shell_parameter EN_HPS_HOST]
    set v_lres_vfw_irq_host         [get_shell_parameter LRES_VFW_IRQ_HOST]
    set v_en_frame_rd               [get_shell_parameter EN_FRAME_RD]
    set v_lres_vfr_irq_priority     [get_shell_parameter LRES_VFR_IRQ_PRIORITY]
    set v_enable_debug              [get_shell_parameter EN_DEBUG]
    set v_en_gs_output              [get_shell_parameter EN_GS_OUTPUT]
    set v_aruco_mk_en               [get_shell_parameter ARUCO_MK_EN]
    set v_aruco_mk_qty              [get_shell_parameter ARUCO_MK_QTY]

    # Video Pipeline
    set v_vid_out_rate              [get_shell_parameter VID_OUT_RATE]
    set v_vid_out_bps               [get_shell_parameter VID_OUT_BPS]
    set v_pip                       [get_shell_parameter PIP]

    # rob Video In Pipeline (fixed config)
    set v_rob_cppp                  {3}
    set v_rob_bps                   {8}

    # General
    set v_pipeline_ready            {1}

    # rob max input size
    # downscale is 2:1 ratio from 4k to 1k
    set v_rob_width_max             {1920}
    set v_rob_height_max            {1080}

    # half height and half width, with 1 extra pixel word and 1 extra line pair
    set v_lt_rob_width_max          [expr ((${v_rob_width_max} /2)  + 1)]
    set v_lt_rob_height_max         [expr ((${v_rob_height_max} /2) + 1)]

    # rob default input size
    # SW to program changes for 960x540 mode
    set v_rob_width_def             [expr (${v_rob_width_max} /2)]
    set v_rob_height_def            [expr (${v_rob_height_max} /2)]
    set v_rob_scale_ratio           8

    create_system ${v_instance_name}
    save_system   ${v_project_path}/rtl/user/${v_instance_name}.qsys
    load_system   ${v_project_path}/rtl/user/${v_instance_name}.qsys

    ## Add Instances      ##

    add_instance  isp_rob_cpu_clk_bridge             altera_clock_bridge
    add_instance  isp_rob_cpu_rst_bridge             altera_reset_bridge
    if {${v_en_hps_host}} {
        add_instance  isp_rob_hps_clk_bridge             altera_clock_bridge
        add_instance  isp_rob_hps_rst_bridge             altera_reset_bridge
    }
    add_instance  isp_rob_vid_clk_bridge             altera_clock_bridge
    add_instance  isp_rob_vid_rst_bridge             altera_reset_bridge
    add_instance  isp_rob_emif_clk_bridge            altera_clock_bridge
    add_instance  isp_rob_emif_rst_bridge            altera_reset_bridge
    add_instance  isp_rob_mm_bridge                  altera_avalon_mm_bridge
    add_instance  isp_rob_axi4s_bcast_full           intel_vvp_axi4s_broadcaster
    if {${v_aruco_mk_en}} {
        add_instance  isp_rob_mixer                   intel_vvp_mixer
        add_instance  isp_rob_aruco_ctrl              altera_avalon_pio
        for {set i 0} {$i < ${v_aruco_mk_qty}} {incr i} {
            add_instance  isp_rob_aruco_mk_gen_${i}     altera_rob_aruco
        }
    }

    add_instance  isp_rob_clipper                    intel_vvp_clipper
    add_instance  isp_rob_scaler_down                intel_vvp_scaler
    add_instance  isp_rob_axi4s_bcast                intel_vvp_axi4s_broadcaster
    if {${v_en_gs_output}} {
        add_instance  isp_rob_csc_gs_out                 intel_vvp_csc
        add_instance  isp_rob_cpm_gs_out                 intel_vvp_cpm
    }
    add_instance  isp_rob_vfb                        intel_vvp_vfb
    add_instance  isp_rob_se_vfb                     altera_address_span_extender
    add_instance  isp_rob_pix_adapt                  intel_vvp_pixel_adapter

    add_instance  isp_rob_vfw_lres                   intel_vvp_vfw
    add_instance  isp_rob_se_vfw_lres                altera_address_span_extender
    add_instance  isp_rob_irq_cdc_2                  altera_irq_clock_crosser
    if {${v_en_hps_host}} {
        add_instance  hps_fpga_rob_cc_bridge             mm_ccb
    }
    if {${v_en_frame_rd}} {
        add_instance  isp_rob_vfr_lres                   intel_vvp_vfr
        add_instance  isp_rob_pix_adapt2                 intel_vvp_pixel_adapter
        add_instance  isp_rob_se_vfr_lres                altera_address_span_extender
        add_instance  isp_rob_irq_cdc_3                  altera_irq_clock_crosser
    }
    add_instance  isp_rob_pix_adapt_gs               intel_vvp_pixel_adapter
    add_instance  isp_rob_csc_gs                     intel_vvp_csc
    add_instance  isp_rob_cpm_gs                     intel_vvp_cpm
    add_instance  isp_rob_vfw_lres_gs                intel_vvp_vfw
    add_instance  isp_rob_se_vfw_lres_gs             altera_address_span_extender
    add_instance  isp_rob_irq_cdc_gs                 altera_irq_clock_crosser

    ## Set Parameters     ##

    # isp_rob_cpu_clk_bridge
    set_instance_parameter_value      isp_rob_cpu_clk_bridge       EXPLICIT_CLOCK_RATE          {100000000.0}
    set_instance_parameter_value      isp_rob_cpu_clk_bridge       NUM_CLOCK_OUTPUTS            {1}

    # isp_rob_cpu_rst_bridge
    set_instance_parameter_value      isp_rob_cpu_rst_bridge       ACTIVE_LOW_RESET             {0}
    set_instance_parameter_value      isp_rob_cpu_rst_bridge       NUM_RESET_OUTPUTS            {1}
    set_instance_parameter_value      isp_rob_cpu_rst_bridge       SYNCHRONOUS_EDGES            {deassert}
    set_instance_parameter_value      isp_rob_cpu_rst_bridge       SYNC_RESET                   {0}
    set_instance_parameter_value      isp_rob_cpu_rst_bridge       USE_RESET_REQUEST            {0}

    if {${v_en_hps_host}} {
        # isp_rob_hps_clk_bridge
        set_instance_parameter_value      isp_rob_hps_clk_bridge       EXPLICIT_CLOCK_RATE          {200000000.0}
        set_instance_parameter_value      isp_rob_hps_clk_bridge       NUM_CLOCK_OUTPUTS            {1}

        # isp_rob_hps_rst_bridge
        set_instance_parameter_value      isp_rob_hps_rst_bridge       ACTIVE_LOW_RESET             {0}
        set_instance_parameter_value      isp_rob_hps_rst_bridge       NUM_RESET_OUTPUTS            {1}
        set_instance_parameter_value      isp_rob_hps_rst_bridge       SYNCHRONOUS_EDGES            {deassert}
        set_instance_parameter_value      isp_rob_hps_rst_bridge       SYNC_RESET                   {0}
        set_instance_parameter_value      isp_rob_hps_rst_bridge       USE_RESET_REQUEST            {0}
    }

    # isp_rob_mm_bridge
    set_instance_parameter_value      isp_rob_mm_bridge            ADDRESS_UNITS                {SYMBOLS}
    set_instance_parameter_value      isp_rob_mm_bridge            ADDRESS_WIDTH                {0}
    set_instance_parameter_value      isp_rob_mm_bridge            DATA_WIDTH                   {32}
    set_instance_parameter_value      isp_rob_mm_bridge            LINEWRAPBURSTS               {0}
    set_instance_parameter_value      isp_rob_mm_bridge            M0_WAITREQUEST_ALLOWANCE     {0}
    set_instance_parameter_value      isp_rob_mm_bridge            MAX_BURST_SIZE               {1}
    set_instance_parameter_value      isp_rob_mm_bridge            MAX_PENDING_RESPONSES        {4}
    set_instance_parameter_value      isp_rob_mm_bridge            MAX_PENDING_WRITES           {0}
    set_instance_parameter_value      isp_rob_mm_bridge            PIPELINE_COMMAND             {1}
    set_instance_parameter_value      isp_rob_mm_bridge            PIPELINE_RESPONSE            {1}
    set_instance_parameter_value      isp_rob_mm_bridge            S0_WAITREQUEST_ALLOWANCE     {0}
    set_instance_parameter_value      isp_rob_mm_bridge            SYMBOL_WIDTH                 {8}
    set_instance_parameter_value      isp_rob_mm_bridge            SYNC_RESET                   {0}
    set_instance_parameter_value      isp_rob_mm_bridge            USE_AUTO_ADDRESS_WIDTH       {1}
    set_instance_parameter_value      isp_rob_mm_bridge            USE_RESPONSE                 {0}
    set_instance_parameter_value      isp_rob_mm_bridge            USE_WRITERESPONSE            {0}

    if {${v_en_hps_host}} {
        # hps_fpga_rob_cc_bridge
        set_instance_parameter_value      hps_fpga_rob_cc_bridge       ADDRESS_UNITS                {SYMBOLS}
        set_instance_parameter_value      hps_fpga_rob_cc_bridge       ADDRESS_WIDTH                {0}
        set_instance_parameter_value      hps_fpga_rob_cc_bridge       DATA_WIDTH                   {32}
        set_instance_parameter_value      hps_fpga_rob_cc_bridge       MAX_BURST_SIZE               {1}
        set_instance_parameter_value      hps_fpga_rob_cc_bridge       SYMBOL_WIDTH                 {8}
        set_instance_parameter_value      hps_fpga_rob_cc_bridge       SYNC_RESET                   {0}
        set_instance_parameter_value      hps_fpga_rob_cc_bridge       USE_AUTO_ADDRESS_WIDTH       {1}
        set_instance_parameter_value      hps_fpga_rob_cc_bridge       COMMAND_FIFO_DEPTH           {4}
        set_instance_parameter_value      hps_fpga_rob_cc_bridge       MASTER_SYNC_DEPTH            {2}
        set_instance_parameter_value      hps_fpga_rob_cc_bridge       RESPONSE_FIFO_DEPTH          {4}
        set_instance_parameter_value      hps_fpga_rob_cc_bridge       SLAVE_SYNC_DEPTH             {2}
    }

    # isp_rob_vid_clk_bridge
    if {(${v_vid_out_rate} == "p60") || (${v_pip} == 1)} {
        set_instance_parameter_value      isp_rob_vid_clk_bridge       EXPLICIT_CLOCK_RATE            {297000000.0}
    } else {
        set_instance_parameter_value      isp_rob_vid_clk_bridge       EXPLICIT_CLOCK_RATE            {148500000.0}
    }
    set_instance_parameter_value      isp_rob_vid_clk_bridge       NUM_CLOCK_OUTPUTS            {1}

    # isp_rob_vid_rst_bridge
    set_instance_parameter_value      isp_rob_vid_rst_bridge       ACTIVE_LOW_RESET             {0}
    set_instance_parameter_value      isp_rob_vid_rst_bridge       NUM_RESET_OUTPUTS            {1}
    set_instance_parameter_value      isp_rob_vid_rst_bridge       SYNCHRONOUS_EDGES            {deassert}
    set_instance_parameter_value      isp_rob_vid_rst_bridge       SYNC_RESET                   {0}
    set_instance_parameter_value      isp_rob_vid_rst_bridge       USE_RESET_REQUEST            {0}

    # isp_rob_emif_clk_bridge
    set_instance_parameter_value      isp_rob_emif_clk_bridge      EXPLICIT_CLOCK_RATE          {200000000.0}
    set_instance_parameter_value      isp_rob_emif_clk_bridge      NUM_CLOCK_OUTPUTS            {1}

    # isp_rob_emif_rst_bridge
    set_instance_parameter_value      isp_rob_emif_rst_bridge      ACTIVE_LOW_RESET             {1}
    set_instance_parameter_value      isp_rob_emif_rst_bridge      NUM_RESET_OUTPUTS            {1}
    set_instance_parameter_value      isp_rob_emif_rst_bridge      SYNCHRONOUS_EDGES            {deassert}
    set_instance_parameter_value      isp_rob_emif_rst_bridge      SYNC_RESET                   {0}
    set_instance_parameter_value      isp_rob_emif_rst_bridge      USE_RESET_REQUEST            {0}

    # isp_rob_axi4s_bcast_full
    set_instance_parameter_value      isp_rob_axi4s_bcast_full       BPS                          ${v_vid_out_bps}
    set_instance_parameter_value      isp_rob_axi4s_bcast_full       GLOBAL_STALL                 {1}
    set_instance_parameter_value      isp_rob_axi4s_bcast_full       IN_TREADY                    {1}
    set_instance_parameter_value      isp_rob_axi4s_bcast_full       NUMBER_OF_COLOR_PLANES       ${v_rob_cppp}
    set_instance_parameter_value      isp_rob_axi4s_bcast_full       OUTPUTS                      {2}
    set_instance_parameter_value      isp_rob_axi4s_bcast_full       OUT_0_FIFO                   {1}
    set_instance_parameter_value      isp_rob_axi4s_bcast_full       OUT_0_FIFO_DEPTH             {16384}
    set_instance_parameter_value      isp_rob_axi4s_bcast_full       OUT_0_TREADY                 {1}
    set_instance_parameter_value      isp_rob_axi4s_bcast_full       OUT_1_FIFO                   {1}
    set_instance_parameter_value      isp_rob_axi4s_bcast_full       OUT_1_FIFO_DEPTH             {16384}
    set_instance_parameter_value      isp_rob_axi4s_bcast_full       OUT_1_TREADY                 {1}
    set_instance_parameter_value      isp_rob_axi4s_bcast_full       PIXELS_IN_PARALLEL           ${v_pip}
    set_instance_parameter_value      isp_rob_axi4s_bcast_full       VVP_INTF_TYPE                {Lite}

    # If we need ArUuco marker generation, do it in the beginning
    if {${v_aruco_mk_en}} {
        # isp_rob_mixer
        # Assuming 1 is the minimal mixer num of inputs, user defines
        # any other extra inputs to the mixer, exported to the top of the subsystem
        set_instance_parameter_value      isp_rob_mixer         NUM_LAYERS                [expr {${v_aruco_mk_qty} +1}]
        set_instance_parameter_value      isp_rob_mixer         BLENDING_MODE_1           {1}
        set_instance_parameter_value      isp_rob_mixer         BLENDING_MODE_2           {1}
        set_instance_parameter_value      isp_rob_mixer         BLENDING_MODE_3           {1}
        set_instance_parameter_value      isp_rob_mixer         BLENDING_MODE_4           {1}
        set_instance_parameter_value      isp_rob_mixer         BLENDING_MODE_5           {1}
        set_instance_parameter_value      isp_rob_mixer         BLENDING_MODE_6           {1}
        set_instance_parameter_value      isp_rob_mixer         BLENDING_MODE_7           {1}
        set_instance_parameter_value      isp_rob_mixer         BPS                        ${v_vid_out_bps}
        set_instance_parameter_value      isp_rob_mixer         C_OMNI_CAP_ID_ASSOCIATED  {0}
        set_instance_parameter_value      isp_rob_mixer         C_OMNI_CAP_ID_COMPONENT   {0}
        set_instance_parameter_value      isp_rob_mixer         C_OMNI_CAP_IRQ            {255}
        set_instance_parameter_value      isp_rob_mixer         C_OMNI_CAP_IRQ_ENABLE     {0}
        set_instance_parameter_value      isp_rob_mixer         C_OMNI_CAP_IRQ_ENABLE_EN  {0}
        set_instance_parameter_value      isp_rob_mixer         C_OMNI_CAP_IRQ_STATUS     {0}
        set_instance_parameter_value      isp_rob_mixer         C_OMNI_CAP_IRQ_STATUS_EN  {0}
        set_instance_parameter_value      isp_rob_mixer         C_OMNI_CAP_TAG            {0}
        set_instance_parameter_value      isp_rob_mixer         C_OMNI_CAP_TYPE           {563}
        set_instance_parameter_value      isp_rob_mixer         C_OMNI_CAP_VERSION        {1}
        set_instance_parameter_value      isp_rob_mixer         DO_ROUNDING               {0}
        set_instance_parameter_value      isp_rob_mixer         ENABLE_DEBUG              ${v_enable_debug}
        set_instance_parameter_value      isp_rob_mixer         EXPORT_PROBES             {0}
        set_instance_parameter_value      isp_rob_mixer         EXTERNAL_MODE             {1}
        set_instance_parameter_value      isp_rob_mixer         NUMBER_OF_COLOR_PLANES    ${v_rob_cppp}
        set_instance_parameter_value      isp_rob_mixer         PIPELINE_LEVEL            {2}
        set_instance_parameter_value      isp_rob_mixer         PIXELS_IN_PARALLEL        ${v_pip}
        set_instance_parameter_value      isp_rob_mixer         P_CORE_CTRL_ID            {0}
        set_instance_parameter_value      isp_rob_mixer         P_UPDATE_CMD_SUPPORTED    {0}
        set_instance_parameter_value      isp_rob_mixer         RESTRICTED_OFFSETS_1      {0}
        set_instance_parameter_value      isp_rob_mixer         RESTRICTED_OFFSETS_2      {0}
        set_instance_parameter_value      isp_rob_mixer         RESTRICTED_OFFSETS_3      {0}
        set_instance_parameter_value      isp_rob_mixer         RESTRICTED_OFFSETS_4      {0}
        set_instance_parameter_value      isp_rob_mixer         RESTRICTED_OFFSETS_5      {0}
        set_instance_parameter_value      isp_rob_mixer         RESTRICTED_OFFSETS_6      {0}
        set_instance_parameter_value      isp_rob_mixer         RESTRICTED_OFFSETS_7      {0}
        set_instance_parameter_value      isp_rob_mixer         RUNTIME_CONTROL           {1}
        set_instance_parameter_value      isp_rob_mixer         SEPARATE_SLAVE_CLOCK      {1}
        set_instance_parameter_value      isp_rob_mixer         SLAVE_PROTOCOL            {Avalon}


        set_instance_parameter_value      isp_rob_aruco_ctrl    width                     {32}
        set_instance_parameter_value      isp_rob_aruco_ctrl    direction                 {Output}
        set_instance_parameter_value      isp_rob_aruco_ctrl    bitModifyingOutReg        {0}
        set_instance_parameter_value      isp_rob_aruco_ctrl    captureEdge               {0}
        set_instance_parameter_value      isp_rob_aruco_ctrl    generateIRQ               {0}
        set_instance_parameter_value      isp_rob_aruco_ctrl    simDoTestBenchWiring      {0}

        for {set i 0} {$i < ${v_aruco_mk_qty}} {incr i} {
            # isp_icon
            set_instance_parameter_value      isp_rob_aruco_mk_gen_${i}     BPS                 ${v_vid_out_bps}
            set_instance_parameter_value      isp_rob_aruco_mk_gen_${i}     EXTERNAL_MODE       {1}
            set_instance_parameter_value      isp_rob_aruco_mk_gen_${i}     PIPELINE_READY      ${v_pipeline_ready}
            set_instance_parameter_value      isp_rob_aruco_mk_gen_${i}     PIXELS_IN_PARALLEL  ${v_pip}
            set_instance_parameter_value      isp_rob_aruco_mk_gen_${i}     ARUCO_ID            ${i}
        }
    }

    # isp_rob_clipper
    set_instance_parameter_value      isp_rob_clipper               BOTTOM_OFFSET               {0}
    set_instance_parameter_value      isp_rob_clipper               BPS                         ${v_vid_out_bps}
    set_instance_parameter_value      isp_rob_clipper               CLIPPING_METHOD             {RECTANGLE}
    set_instance_parameter_value      isp_rob_clipper               C_OMNI_CAP_ID_ASSOCIATED    {0}
    set_instance_parameter_value      isp_rob_clipper               C_OMNI_CAP_ID_COMPONENT     {0}
    set_instance_parameter_value      isp_rob_clipper               C_OMNI_CAP_IRQ              {255}
    set_instance_parameter_value      isp_rob_clipper               C_OMNI_CAP_IRQ_ENABLE       {0}
    set_instance_parameter_value      isp_rob_clipper               C_OMNI_CAP_IRQ_ENABLE_EN    {0}
    set_instance_parameter_value      isp_rob_clipper               C_OMNI_CAP_IRQ_STATUS       {0}
    set_instance_parameter_value      isp_rob_clipper               C_OMNI_CAP_IRQ_STATUS_EN    {0}
    set_instance_parameter_value      isp_rob_clipper               C_OMNI_CAP_TAG              {0}
    set_instance_parameter_value      isp_rob_clipper               C_OMNI_CAP_TYPE             {557}
    set_instance_parameter_value      isp_rob_clipper               C_OMNI_CAP_VERSION          {1}
    set_instance_parameter_value      isp_rob_clipper               ENABLE_DEBUG                ${v_enable_debug}
    set_instance_parameter_value      isp_rob_clipper               EXTERNAL_MODE               {1}
    set_instance_parameter_value      isp_rob_clipper               LEFT_OFFSET                 {0}
    set_instance_parameter_value      isp_rob_clipper               NUMBER_OF_COLOR_PLANES      ${v_rob_cppp}
    set_instance_parameter_value      isp_rob_clipper               PIXELS_IN_PARALLEL          ${v_pip}
    set_instance_parameter_value      isp_rob_clipper               RECTANGLE_HEIGHT            {1080}
    set_instance_parameter_value      isp_rob_clipper               RECTANGLE_WIDTH             {1920}
    set_instance_parameter_value      isp_rob_clipper               RIGHT_OFFSET                {0}
    set_instance_parameter_value      isp_rob_clipper               RUNTIME_CONTROL             {1}
    set_instance_parameter_value      isp_rob_clipper               SEPARATE_SLAVE_CLOCK        {1}
    set_instance_parameter_value      isp_rob_clipper               SLAVE_PROTOCOL              {Avalon}
    set_instance_parameter_value      isp_rob_clipper               TOP_OFFSET                  {0}

    # isp_rob_scaler_down
    set_instance_parameter_value      isp_rob_scaler_down           ALGORITHM                   {NEAREST_NEIGHBOUR}
    set_instance_parameter_value      isp_rob_scaler_down           BPS                         ${v_vid_out_bps}
    set_instance_parameter_value      isp_rob_scaler_down           C_OMNI_CAP_ID_ASSOCIATED    {0}
    set_instance_parameter_value      isp_rob_scaler_down           C_OMNI_CAP_ID_COMPONENT     {0}
    set_instance_parameter_value      isp_rob_scaler_down           C_OMNI_CAP_IRQ              {255}
    set_instance_parameter_value      isp_rob_scaler_down           C_OMNI_CAP_IRQ_ENABLE       {0}
    set_instance_parameter_value      isp_rob_scaler_down           C_OMNI_CAP_IRQ_ENABLE_EN    {0}
    set_instance_parameter_value      isp_rob_scaler_down           C_OMNI_CAP_IRQ_STATUS       {0}
    set_instance_parameter_value      isp_rob_scaler_down           C_OMNI_CAP_IRQ_STATUS_EN    {0}
    set_instance_parameter_value      isp_rob_scaler_down           C_OMNI_CAP_TAG              {0}
    set_instance_parameter_value      isp_rob_scaler_down           C_OMNI_CAP_TYPE             {564}
    set_instance_parameter_value      isp_rob_scaler_down           C_OMNI_CAP_VERSION          {1}
    set_instance_parameter_value      isp_rob_scaler_down           EDGE_MIRROR                 {REPLICATE}
    set_instance_parameter_value      isp_rob_scaler_down           ENABLE_420                  {0}
    set_instance_parameter_value      isp_rob_scaler_down           ENABLE_420_MIRROR           {0}
    set_instance_parameter_value      isp_rob_scaler_down           ENABLE_422                  {0}
    set_instance_parameter_value      isp_rob_scaler_down           ENABLE_444                  {1}
    set_instance_parameter_value      isp_rob_scaler_down           ENABLE_DEBUG                ${v_enable_debug}
    set_instance_parameter_value      isp_rob_scaler_down           ENABLE_H                    {1}
    set_instance_parameter_value      isp_rob_scaler_down           ENABLE_V                    {1}
    set_instance_parameter_value      isp_rob_scaler_down           EXTERNAL_MODE               {1}
    set_instance_parameter_value      isp_rob_scaler_down           HALF_RATE_420               {0}
    set_instance_parameter_value      isp_rob_scaler_down           H_BANKS                     {1}
    set_instance_parameter_value      isp_rob_scaler_down           H_COEFF_FRAC_BITS           {8}
    set_instance_parameter_value      isp_rob_scaler_down           H_COEFF_FUNCTION            {LANCZOS_1}
    set_instance_parameter_value      isp_rob_scaler_down           H_COEFF_INT_BITS            {1}
    set_instance_parameter_value      isp_rob_scaler_down           H_COEFF_SIGNED              {1}
    set_instance_parameter_value      isp_rob_scaler_down           H_INIT_FILE \
                                                                      {<enter file name (including full path)>}
    set_instance_parameter_value      isp_rob_scaler_down           H_PARTIAL_SCALING           {0}
    set_instance_parameter_value      isp_rob_scaler_down           H_PHASES                    {16}
    set_instance_parameter_value      isp_rob_scaler_down           H_TAPS                      ${v_rob_scale_ratio}
    set_instance_parameter_value      isp_rob_scaler_down           MAX_IN_WIDTH                ${v_rob_width_max}
    set_instance_parameter_value      isp_rob_scaler_down           MAX_OUT_WIDTH               ${v_rob_width_def}
    set_instance_parameter_value      isp_rob_scaler_down           MEM_INIT                    {1}
    set_instance_parameter_value      isp_rob_scaler_down           NO_BLANKING                 {0}
    set_instance_parameter_value      isp_rob_scaler_down           NUMBER_OF_COLOR_PLANES      ${v_rob_cppp}
    set_instance_parameter_value      isp_rob_scaler_down           OUTPUT_HEIGHT               ${v_rob_height_def}
    set_instance_parameter_value      isp_rob_scaler_down           PIPELINE_READY              ${v_pipeline_ready}
    set_instance_parameter_value      isp_rob_scaler_down           PIXELS_IN_PARALLEL          ${v_pip}
    set_instance_parameter_value      isp_rob_scaler_down           P_CORE_CTRL_ID              {0}
    set_instance_parameter_value      isp_rob_scaler_down           P_UPDATE_CMD_SUPPORTED      {0}
    set_instance_parameter_value      isp_rob_scaler_down           RUNTIME_CONTROL             {1}
    set_instance_parameter_value      isp_rob_scaler_down           RUNTIME_LOAD                {0}
    set_instance_parameter_value      isp_rob_scaler_down           SEPARATE_SLAVE_CLOCK        {1}
    set_instance_parameter_value      isp_rob_scaler_down           SLAVE_PROTOCOL              {Avalon}
    set_instance_parameter_value      isp_rob_scaler_down           V_BANKS                     {1}
    set_instance_parameter_value      isp_rob_scaler_down           V_COEFF_FRAC_BITS           {8}
    set_instance_parameter_value      isp_rob_scaler_down           V_COEFF_FUNCTION            {LANCZOS_1}
    set_instance_parameter_value      isp_rob_scaler_down           V_COEFF_INT_BITS            {1}
    set_instance_parameter_value      isp_rob_scaler_down           V_COEFF_SIGNED              {1}
    set_instance_parameter_value      isp_rob_scaler_down           V_INIT_FILE \
                                                                      {<enter file name (including full path)>}
    set_instance_parameter_value      isp_rob_scaler_down           V_PARTIAL_SCALING           {0}
    set_instance_parameter_value      isp_rob_scaler_down           V_PHASES                    {16}
    set_instance_parameter_value      isp_rob_scaler_down           V_PRES_FRAC_BITS            {0}
    set_instance_parameter_value      isp_rob_scaler_down           V_TAPS                      ${v_rob_scale_ratio}

    # isp_rob_axi4s_bcast
    set_instance_parameter_value      isp_rob_axi4s_bcast           BPS                         ${v_vid_out_bps}
    set_instance_parameter_value      isp_rob_axi4s_bcast           GLOBAL_STALL                {1}
    set_instance_parameter_value      isp_rob_axi4s_bcast           IN_TREADY                   {1}
    set_instance_parameter_value      isp_rob_axi4s_bcast           NUMBER_OF_COLOR_PLANES      ${v_rob_cppp}
    set_instance_parameter_value      isp_rob_axi4s_bcast           OUTPUTS                     {3}
    set_instance_parameter_value      isp_rob_axi4s_bcast           OUT_0_FIFO                  {1}
    set_instance_parameter_value      isp_rob_axi4s_bcast           OUT_0_FIFO_DEPTH            {16384}
    set_instance_parameter_value      isp_rob_axi4s_bcast           OUT_0_TREADY                {1}
    set_instance_parameter_value      isp_rob_axi4s_bcast           OUT_1_FIFO                  {1}
    set_instance_parameter_value      isp_rob_axi4s_bcast           OUT_1_FIFO_DEPTH            {16384}
    set_instance_parameter_value      isp_rob_axi4s_bcast           OUT_1_TREADY                {1}
    set_instance_parameter_value      isp_rob_axi4s_bcast           OUT_2_FIFO                  {1}
    set_instance_parameter_value      isp_rob_axi4s_bcast           OUT_2_FIFO_DEPTH            {16384}
    set_instance_parameter_value      isp_rob_axi4s_bcast           OUT_2_TREADY                {1}
    set_instance_parameter_value      isp_rob_axi4s_bcast           PIXELS_IN_PARALLEL          ${v_pip}
    set_instance_parameter_value      isp_rob_axi4s_bcast           VVP_INTF_TYPE               {Lite}

    if {${v_en_gs_output}} {
        # isp_rob_csc_gs
        set_instance_parameter_value      isp_rob_csc_gs_out        BPS_IN                      ${v_vid_out_bps}
        set_instance_parameter_value      isp_rob_csc_gs_out        BPS_OUT                     ${v_vid_out_bps}
        set_instance_parameter_value      isp_rob_csc_gs_out        COEFFICIENT_INT_BITS        {0}
        set_instance_parameter_value      isp_rob_csc_gs_out        COEFFICIENT_SIGNED          {1}
        set_instance_parameter_value      isp_rob_csc_gs_out        COEF_SUM_FRACTION_BITS      {8}
        set_instance_parameter_value      isp_rob_csc_gs_out        C_OMNI_CAP_ID_ASSOCIATED    {0}
        set_instance_parameter_value      isp_rob_csc_gs_out        C_OMNI_CAP_ID_COMPONENT     {1}
        set_instance_parameter_value      isp_rob_csc_gs_out        C_OMNI_CAP_IRQ              {255}
        set_instance_parameter_value      isp_rob_csc_gs_out        C_OMNI_CAP_IRQ_ENABLE       {0}
        set_instance_parameter_value      isp_rob_csc_gs_out        C_OMNI_CAP_IRQ_ENABLE_EN    {0}
        set_instance_parameter_value      isp_rob_csc_gs_out        C_OMNI_CAP_IRQ_STATUS       {0}
        set_instance_parameter_value      isp_rob_csc_gs_out        C_OMNI_CAP_IRQ_STATUS_EN    {0}
        set_instance_parameter_value      isp_rob_csc_gs_out        C_OMNI_CAP_TAG              {0}
        set_instance_parameter_value      isp_rob_csc_gs_out        C_OMNI_CAP_TYPE             {559}
        set_instance_parameter_value      isp_rob_csc_gs_out        C_OMNI_CAP_VERSION          {1}
        set_instance_parameter_value      isp_rob_csc_gs_out        ENABLE_DEBUG                ${v_enable_debug}
        set_instance_parameter_value      isp_rob_csc_gs_out        EXTERNAL_MODE               {1}
        set_instance_parameter_value      isp_rob_csc_gs_out        MOVE_BINARY_POINT_RIGHT     {0}
        set_instance_parameter_value      isp_rob_csc_gs_out        OUTPUT_COLORSPACE           {1}
        set_instance_parameter_value      isp_rob_csc_gs_out        PIPELINE_READY              ${v_pipeline_ready}
        set_instance_parameter_value      isp_rob_csc_gs_out        PIXELS_IN_PARALLEL          ${v_pip}
        set_instance_parameter_value      isp_rob_csc_gs_out        P_CORE_CTRL_ID              {0}
        set_instance_parameter_value      isp_rob_csc_gs_out        P_UPDATE_CMD_SUPPORTED      {0}
        set_instance_parameter_value      isp_rob_csc_gs_out        REMOVE_FRACTION_METHOD      {1}
        set_instance_parameter_value      isp_rob_csc_gs_out        REQ_FCOEFF_A0               {0.439}
        set_instance_parameter_value      isp_rob_csc_gs_out        REQ_FCOEFF_A1               {0.062}
        set_instance_parameter_value      isp_rob_csc_gs_out        REQ_FCOEFF_A2               {-0.04}
        set_instance_parameter_value      isp_rob_csc_gs_out        REQ_FCOEFF_B0               {-0.338}
        set_instance_parameter_value      isp_rob_csc_gs_out        REQ_FCOEFF_B1               {0.614}
        set_instance_parameter_value      isp_rob_csc_gs_out        REQ_FCOEFF_B2               {-0.399}
        set_instance_parameter_value      isp_rob_csc_gs_out        REQ_FCOEFF_C0               {-0.101}
        set_instance_parameter_value      isp_rob_csc_gs_out        REQ_FCOEFF_C1               {0.183}
        set_instance_parameter_value      isp_rob_csc_gs_out        REQ_FCOEFF_C2               {0.439}
        set_instance_parameter_value      isp_rob_csc_gs_out        REQ_FCOEFF_S0               {512.0}
        set_instance_parameter_value      isp_rob_csc_gs_out        REQ_FCOEFF_S1               {64.0}
        set_instance_parameter_value      isp_rob_csc_gs_out        REQ_FCOEFF_S2               {512.0}
        set_instance_parameter_value      isp_rob_csc_gs_out        RUNTIME_CONTROL             {0}
        set_instance_parameter_value      isp_rob_csc_gs_out        SEPARATE_SLAVE_CLOCK        {1}
        set_instance_parameter_value      isp_rob_csc_gs_out        SLAVE_PROTOCOL              {Avalon}
        set_instance_parameter_value      isp_rob_csc_gs_out        SUMMAND_INT_BITS            {8}
        set_instance_parameter_value      isp_rob_csc_gs_out        SUMMAND_SIGNED              {0}

        # isp_rob_cpm_gs
        set_instance_parameter_value      isp_rob_cpm_gs_out        BPS                         ${v_vid_out_bps}
        set_instance_parameter_value      isp_rob_cpm_gs_out        C_OMNI_CAP_ID_ASSOCIATED    {0}
        set_instance_parameter_value      isp_rob_cpm_gs_out        C_OMNI_CAP_ID_COMPONENT     {0}
        set_instance_parameter_value      isp_rob_cpm_gs_out        C_OMNI_CAP_IRQ              {255}
        set_instance_parameter_value      isp_rob_cpm_gs_out        C_OMNI_CAP_IRQ_ENABLE       {0}
        set_instance_parameter_value      isp_rob_cpm_gs_out        C_OMNI_CAP_IRQ_ENABLE_EN    {0}
        set_instance_parameter_value      isp_rob_cpm_gs_out        C_OMNI_CAP_IRQ_STATUS       {0}
        set_instance_parameter_value      isp_rob_cpm_gs_out        C_OMNI_CAP_IRQ_STATUS_EN    {0}
        set_instance_parameter_value      isp_rob_cpm_gs_out        C_OMNI_CAP_TAG              {0}
        set_instance_parameter_value      isp_rob_cpm_gs_out        C_OMNI_CAP_TYPE             {571}
        set_instance_parameter_value      isp_rob_cpm_gs_out        C_OMNI_CAP_VERSION          {1}
        set_instance_parameter_value      isp_rob_cpm_gs_out        ENABLE_DEBUG                ${v_enable_debug}
        set_instance_parameter_value      isp_rob_cpm_gs_out        EXTERNAL_MODE               {1}
        set_instance_parameter_value      isp_rob_cpm_gs_out        IP0_KEEP_COLOR_PLANE_0      {1}
        set_instance_parameter_value      isp_rob_cpm_gs_out        IP0_KEEP_COLOR_PLANE_1      {1}
        set_instance_parameter_value      isp_rob_cpm_gs_out        IP0_KEEP_COLOR_PLANE_2      {1}
        set_instance_parameter_value      isp_rob_cpm_gs_out        IP0_KEEP_COLOR_PLANE_3      {1}
        set_instance_parameter_value      isp_rob_cpm_gs_out        IP1_KEEP_COLOR_PLANE_0      {1}
        set_instance_parameter_value      isp_rob_cpm_gs_out        IP1_KEEP_COLOR_PLANE_1      {1}
        set_instance_parameter_value      isp_rob_cpm_gs_out        IP1_KEEP_COLOR_PLANE_2      {1}
        set_instance_parameter_value      isp_rob_cpm_gs_out        IP1_KEEP_COLOR_PLANE_3      {1}
        set_instance_parameter_value      isp_rob_cpm_gs_out        KEEP_IP1_AUX_PACKETS        {1}
        set_instance_parameter_value      isp_rob_cpm_gs_out        KEEP_OP1_AUX_PACKETS        {1}
        set_instance_parameter_value      isp_rob_cpm_gs_out        MAPPING_COLOR_PLANE_0       {1}
        set_instance_parameter_value      isp_rob_cpm_gs_out        MAPPING_COLOR_PLANE_1       {1}
        set_instance_parameter_value      isp_rob_cpm_gs_out        MAPPING_COLOR_PLANE_2       {1}
        set_instance_parameter_value      isp_rob_cpm_gs_out        MAPPING_COLOR_PLANE_3       {0}
        set_instance_parameter_value      isp_rob_cpm_gs_out        MODE                        {REARRANGE}
        set_instance_parameter_value      isp_rob_cpm_gs_out        NUMBER_OF_COLOR_PLANES_IN0  ${v_rob_cppp}
        set_instance_parameter_value      isp_rob_cpm_gs_out        NUMBER_OF_COLOR_PLANES_IN1  {2}
        set_instance_parameter_value      isp_rob_cpm_gs_out        NUMBER_OF_COLOR_PLANES_OUT0 ${v_rob_cppp}
        set_instance_parameter_value      isp_rob_cpm_gs_out        NUMBER_OF_COLOR_PLANES_OUT1 {2}
        set_instance_parameter_value      isp_rob_cpm_gs_out        OP0_KEEP_COLOR_PLANE_0      {0}
        set_instance_parameter_value      isp_rob_cpm_gs_out        OP0_KEEP_COLOR_PLANE_1      {1}
        set_instance_parameter_value      isp_rob_cpm_gs_out        OP0_KEEP_COLOR_PLANE_2      {0}
        set_instance_parameter_value      isp_rob_cpm_gs_out        OP0_KEEP_COLOR_PLANE_3      {0}
        set_instance_parameter_value      isp_rob_cpm_gs_out        OP1_KEEP_COLOR_PLANE_0      {1}
        set_instance_parameter_value      isp_rob_cpm_gs_out        OP1_KEEP_COLOR_PLANE_1      {0}
        set_instance_parameter_value      isp_rob_cpm_gs_out        OP1_KEEP_COLOR_PLANE_2      {1}
        set_instance_parameter_value      isp_rob_cpm_gs_out        OP1_KEEP_COLOR_PLANE_3      {0}
        set_instance_parameter_value      isp_rob_cpm_gs_out        PADDING_COLOR_PLANE_0       {0}
        set_instance_parameter_value      isp_rob_cpm_gs_out        PADDING_COLOR_PLANE_1       {0}
        set_instance_parameter_value      isp_rob_cpm_gs_out        PADDING_COLOR_PLANE_2       {0}
        set_instance_parameter_value      isp_rob_cpm_gs_out        PADDING_COLOR_PLANE_3       {0}
        set_instance_parameter_value      isp_rob_cpm_gs_out        PIXELS_IN_PARALLEL          ${v_pip}
        set_instance_parameter_value      isp_rob_cpm_gs_out        P_CORE_CTRL_ID              {0}
        set_instance_parameter_value      isp_rob_cpm_gs_out        P_UPDATE_CMD_SUPPORTED      {0}
        set_instance_parameter_value      isp_rob_cpm_gs_out        RUNTIME_CONTROL             {0}
        set_instance_parameter_value      isp_rob_cpm_gs_out        SEPARATE_SLAVE_CLOCK        {1}
        set_instance_parameter_value      isp_rob_cpm_gs_out        SLAVE_PROTOCOL              {Avalon}
    }

    # isp_rob_vfb
    set_instance_parameter_value      isp_rob_vfb                   BPS                             ${v_vid_out_bps}
    set_instance_parameter_value      isp_rob_vfb                   CLOCKS_ARE_SEPARATE             {1}
    set_instance_parameter_value      isp_rob_vfb                   C_OMNI_CAP_ID_ASSOCIATED        {0}
    set_instance_parameter_value      isp_rob_vfb                   C_OMNI_CAP_ID_COMPONENT         {0}
    set_instance_parameter_value      isp_rob_vfb                   C_OMNI_CAP_IRQ                  {255}
    set_instance_parameter_value      isp_rob_vfb                   C_OMNI_CAP_IRQ_ENABLE           {0}
    set_instance_parameter_value      isp_rob_vfb                   C_OMNI_CAP_IRQ_ENABLE_EN        {0}
    set_instance_parameter_value      isp_rob_vfb                   C_OMNI_CAP_IRQ_STATUS           {0}
    set_instance_parameter_value      isp_rob_vfb                   C_OMNI_CAP_IRQ_STATUS_EN        {0}
    set_instance_parameter_value      isp_rob_vfb                   C_OMNI_CAP_TAG                  {0}
    set_instance_parameter_value      isp_rob_vfb                   C_OMNI_CAP_TYPE                 {567}
    set_instance_parameter_value      isp_rob_vfb                   C_OMNI_CAP_VERSION              {1}
    set_instance_parameter_value      isp_rob_vfb                   DROP_BROKEN_FRAMES              {1}
    set_instance_parameter_value      isp_rob_vfb                   DROP_RPT_AUX_PKTS_WITH_FRAMES   {0}
    set_instance_parameter_value      isp_rob_vfb                   ENABLE_DEBUG                    ${v_enable_debug}
    set_instance_parameter_value      isp_rob_vfb                   EXTERNAL_MODE                   {1}
    set_instance_parameter_value      isp_rob_vfb                   FRAME_DROP_ENABLE               {1}
    set_instance_parameter_value      isp_rob_vfb                   FRAME_REPEAT_ENABLE             {1}
    set_instance_parameter_value      isp_rob_vfb                   MAX_CONTROL_PACKETS             {0}
    set_instance_parameter_value      isp_rob_vfb                   MAX_HEIGHT                      {4096}
    set_instance_parameter_value      isp_rob_vfb                   MAX_WIDTH                       {4096}
    set_instance_parameter_value      isp_rob_vfb                   MEM_BUFF_BASE_ADDR              {0x40000000}
    set_instance_parameter_value      isp_rob_vfb                   MEM_BUFF_LINE_STRIDE            {49152}
    set_instance_parameter_value      isp_rob_vfb                   MEM_BUFF_STRIDE                 {268435456}
    set_instance_parameter_value      isp_rob_vfb                   NUMBER_OF_COLOR_PLANES          ${v_rob_cppp}
    set_instance_parameter_value      isp_rob_vfb                   PACKING                         {PERFECT}
    set_instance_parameter_value      isp_rob_vfb                   PIXELS_IN_PARALLEL              ${v_pip}
    set_instance_parameter_value      isp_rob_vfb                   P_AV_MM_ADDR_WIDTH              {32}
    set_instance_parameter_value      isp_rob_vfb                   P_AV_MM_DATA_WIDTH              {256}
    set_instance_parameter_value      isp_rob_vfb                   READ_BURST_TARGET               {64}
    set_instance_parameter_value      isp_rob_vfb                   READ_FIFO_DEPTH                 {512}
    set_instance_parameter_value      isp_rob_vfb                   WRITE_BURST_TARGET              {64}
    set_instance_parameter_value      isp_rob_vfb                   WRITE_FIFO_DEPTH                {512}
    set_instance_parameter_value      isp_rob_vfb                   RUNTIME_CONTROL                 {1}
    set_instance_parameter_value      isp_rob_vfb                   SEPARATE_SLAVE_CLOCK            {1}

    # isp_rob_se_vfb
    set_instance_parameter_value      isp_rob_se_vfb                BURSTCOUNT_WIDTH            {7}
    set_instance_parameter_value      isp_rob_se_vfb                DATA_WIDTH                  {256}
    set_instance_parameter_value      isp_rob_se_vfb                ENABLE_SLAVE_PORT           {0}
    set_instance_parameter_value      isp_rob_se_vfb                MASTER_ADDRESS_DEF          {0}
    set_instance_parameter_value      isp_rob_se_vfb                MASTER_ADDRESS_WIDTH        {33}
    set_instance_parameter_value      isp_rob_se_vfb                MAX_PENDING_READS           {8}
    set_instance_parameter_value      isp_rob_se_vfb                SLAVE_ADDRESS_WIDTH         {27}
    set_instance_parameter_value      isp_rob_se_vfb                SUB_WINDOW_COUNT            {1}
    set_instance_parameter_value      isp_rob_se_vfb                SYNC_RESET                  {0}

    # isp_rob_pix_adapt
    set_instance_parameter_value      isp_rob_pix_adapt             BPS_IN                      ${v_vid_out_bps}
    set_instance_parameter_value      isp_rob_pix_adapt             BPS_OUT                     ${v_rob_bps}
    set_instance_parameter_value      isp_rob_pix_adapt             ENABLE_DEBUG                ${v_enable_debug}
    set_instance_parameter_value      isp_rob_pix_adapt             EXTERNAL_MODE               {1}
    set_instance_parameter_value      isp_rob_pix_adapt             NUMBER_OF_COLOR_PLANES      ${v_rob_cppp}
    set_instance_parameter_value      isp_rob_pix_adapt             PIPELINE_READY              ${v_pipeline_ready}
    set_instance_parameter_value      isp_rob_pix_adapt             PIXELS_IN_PARALLEL          ${v_pip}
    set_instance_parameter_value      isp_rob_pix_adapt             P_CORE_CTRL_ID              {0}
    set_instance_parameter_value      isp_rob_pix_adapt             P_UPDATE_CMD_SUPPORTED      {0}
    set_instance_parameter_value      isp_rob_pix_adapt             RUNTIME_CONTROL             {0}
    set_instance_parameter_value      isp_rob_pix_adapt             SEPARATE_SLAVE_CLOCK        {0}
    set_instance_parameter_value      isp_rob_pix_adapt             SLAVE_PROTOCOL              {Avalon}

    # isp_rob_vfw_lres
    set_instance_parameter_value      isp_rob_vfw_lres              BPS                         ${v_rob_bps}
    set_instance_parameter_value      isp_rob_vfw_lres              CLOCKS_ARE_SEPARATE         {1}
    set_instance_parameter_value      isp_rob_vfw_lres              C_OMNI_CAP_ID_ASSOCIATED    {0}
    set_instance_parameter_value      isp_rob_vfw_lres              C_OMNI_CAP_ID_COMPONENT     {1}
    set_instance_parameter_value      isp_rob_vfw_lres              C_OMNI_CAP_IRQ              {255}
    set_instance_parameter_value      isp_rob_vfw_lres              C_OMNI_CAP_IRQ_ENABLE       {0}
    set_instance_parameter_value      isp_rob_vfw_lres              C_OMNI_CAP_IRQ_ENABLE_EN    {0}
    set_instance_parameter_value      isp_rob_vfw_lres              C_OMNI_CAP_IRQ_STATUS       {0}
    set_instance_parameter_value      isp_rob_vfw_lres              C_OMNI_CAP_IRQ_STATUS_EN    {0}
    set_instance_parameter_value      isp_rob_vfw_lres              C_OMNI_CAP_TAG              {0}
    set_instance_parameter_value      isp_rob_vfw_lres              C_OMNI_CAP_TYPE             {585}
    set_instance_parameter_value      isp_rob_vfw_lres              C_OMNI_CAP_VERSION          {1}
    set_instance_parameter_value      isp_rob_vfw_lres              ENABLE_DEBUG                ${v_enable_debug}
    set_instance_parameter_value      isp_rob_vfw_lres              EXTERNAL_MODE               {1}
    set_instance_parameter_value      isp_rob_vfw_lres              MAX_HEIGHT                  ${v_lt_rob_height_max}
    set_instance_parameter_value      isp_rob_vfw_lres              MAX_WIDTH                   ${v_lt_rob_width_max}
    set_instance_parameter_value      isp_rob_vfw_lres              NUMBER_OF_COLOR_PLANES      ${v_rob_cppp}
    set_instance_parameter_value      isp_rob_vfw_lres              PACKING                     {PERFECT}
    set_instance_parameter_value      isp_rob_vfw_lres              PIXELS_IN_PARALLEL          ${v_pip}
    set_instance_parameter_value      isp_rob_vfw_lres              P_AV_MM_ADDR_WIDTH          {32}
    set_instance_parameter_value      isp_rob_vfw_lres              P_AV_MM_DATA_WIDTH          {256}
    set_instance_parameter_value      isp_rob_vfw_lres              SEPARATE_SLAVE_CLOCK        {1}
    set_instance_parameter_value      isp_rob_vfw_lres              SLAVE_PROTOCOL              {Avalon}
    set_instance_parameter_value      isp_rob_vfw_lres              WRITE_BURST_TARGET          {8}
    set_instance_parameter_value      isp_rob_vfw_lres              WRITE_FIFO_DEPTH            {512}

    # isp_rob_se_vfw_lres
    set_instance_parameter_value      isp_rob_se_vfw_lres           BURSTCOUNT_WIDTH            {6}
    set_instance_parameter_value      isp_rob_se_vfw_lres           DATA_WIDTH                  {256}
    set_instance_parameter_value      isp_rob_se_vfw_lres           ENABLE_SLAVE_PORT           {0}
    set_instance_parameter_value      isp_rob_se_vfw_lres           MASTER_ADDRESS_DEF          {0}
    set_instance_parameter_value      isp_rob_se_vfw_lres           MASTER_ADDRESS_WIDTH        {33}
    set_instance_parameter_value      isp_rob_se_vfw_lres           MAX_PENDING_READS           {8}
    set_instance_parameter_value      isp_rob_se_vfw_lres           SLAVE_ADDRESS_WIDTH         {26}
    set_instance_parameter_value      isp_rob_se_vfw_lres           SUB_WINDOW_COUNT            {1}
    set_instance_parameter_value      isp_rob_se_vfw_lres           SYNC_RESET                  {0}

    # isp_rob_irq_cdc_2
    set_instance_parameter_value      isp_rob_irq_cdc_2             IRQ_WIDTH                   {1}
    set_instance_parameter_value      isp_rob_irq_cdc_2             SYNC_RESET                  {0}

    # isp_rob_vfr_lres
    if {${v_en_frame_rd}} {
        set_instance_parameter_value      isp_rob_vfr_lres          BPS                         ${v_rob_bps}
        set_instance_parameter_value      isp_rob_vfr_lres          CLOCKS_ARE_SEPARATE         {1}
        set_instance_parameter_value      isp_rob_vfr_lres          C_OMNI_CAP_ID_ASSOCIATED    {0}
        set_instance_parameter_value      isp_rob_vfr_lres          C_OMNI_CAP_ID_COMPONENT     {0}
        set_instance_parameter_value      isp_rob_vfr_lres          C_OMNI_CAP_IRQ              {255}
        set_instance_parameter_value      isp_rob_vfr_lres          C_OMNI_CAP_IRQ_ENABLE       {0}
        set_instance_parameter_value      isp_rob_vfr_lres          C_OMNI_CAP_IRQ_ENABLE_EN    {0}
        set_instance_parameter_value      isp_rob_vfr_lres          C_OMNI_CAP_IRQ_STATUS       {0}
        set_instance_parameter_value      isp_rob_vfr_lres          C_OMNI_CAP_IRQ_STATUS_EN    {0}
        set_instance_parameter_value      isp_rob_vfr_lres          C_OMNI_CAP_TAG              {0}
        set_instance_parameter_value      isp_rob_vfr_lres          C_OMNI_CAP_TYPE             {586}
        set_instance_parameter_value      isp_rob_vfr_lres          C_OMNI_CAP_VERSION          {1}
        set_instance_parameter_value      isp_rob_vfr_lres          ENABLE_DEBUG                ${v_enable_debug}
        set_instance_parameter_value      isp_rob_vfr_lres          EXTERNAL_MODE               {1}
        set_instance_parameter_value      isp_rob_vfr_lres          MAX_BUFFER_SETS             {2}
        set_instance_parameter_value      isp_rob_vfr_lres          MAX_HEIGHT                  ${v_lt_rob_height_max}
        set_instance_parameter_value      isp_rob_vfr_lres          MAX_WIDTH                   ${v_lt_rob_width_max}
        set_instance_parameter_value      isp_rob_vfr_lres          NUMBER_OF_COLOR_PLANES      ${v_rob_cppp}
        set_instance_parameter_value      isp_rob_vfr_lres          PACKING                     {PERFECT}
        set_instance_parameter_value      isp_rob_vfr_lres          PIXELS_IN_PARALLEL          ${v_pip}
        set_instance_parameter_value      isp_rob_vfr_lres          P_AV_MM_ADDR_WIDTH          {32}
        set_instance_parameter_value      isp_rob_vfr_lres          P_AV_MM_DATA_WIDTH          {256}
        set_instance_parameter_value      isp_rob_vfr_lres          SEPARATE_SLAVE_CLOCK        {1}
        set_instance_parameter_value      isp_rob_vfr_lres          SLAVE_PROTOCOL              {Avalon}

        # isp_rob_pix_adapt2
        set_instance_parameter_value      isp_rob_pix_adapt2        BPS_IN                      ${v_rob_bps}
        set_instance_parameter_value      isp_rob_pix_adapt2        BPS_OUT                     ${v_vid_out_bps}
        set_instance_parameter_value      isp_rob_pix_adapt2        ENABLE_DEBUG                ${v_enable_debug}
        set_instance_parameter_value      isp_rob_pix_adapt2        EXTERNAL_MODE               {1}
        set_instance_parameter_value      isp_rob_pix_adapt2        NUMBER_OF_COLOR_PLANES      ${v_rob_cppp}
        set_instance_parameter_value      isp_rob_pix_adapt2        PIPELINE_READY              ${v_pipeline_ready}
        set_instance_parameter_value      isp_rob_pix_adapt2        PIXELS_IN_PARALLEL          ${v_pip}
        set_instance_parameter_value      isp_rob_pix_adapt2        P_CORE_CTRL_ID              {0}
        set_instance_parameter_value      isp_rob_pix_adapt2        P_UPDATE_CMD_SUPPORTED      {0}
        set_instance_parameter_value      isp_rob_pix_adapt2        RUNTIME_CONTROL             {0}
        set_instance_parameter_value      isp_rob_pix_adapt2        SEPARATE_SLAVE_CLOCK        {0}
        set_instance_parameter_value      isp_rob_pix_adapt2        SLAVE_PROTOCOL              {Avalon}

        # isp_rob_se_vfr_lres
        set_instance_parameter_value      isp_rob_se_vfr_lres       BURSTCOUNT_WIDTH            {6}
        set_instance_parameter_value      isp_rob_se_vfr_lres       DATA_WIDTH                  {256}
        set_instance_parameter_value      isp_rob_se_vfr_lres       ENABLE_SLAVE_PORT           {0}
        set_instance_parameter_value      isp_rob_se_vfr_lres       MASTER_ADDRESS_DEF          {0}
        set_instance_parameter_value      isp_rob_se_vfr_lres       MASTER_ADDRESS_WIDTH        {33}
        set_instance_parameter_value      isp_rob_se_vfr_lres       MAX_PENDING_READS           {8}
        set_instance_parameter_value      isp_rob_se_vfr_lres       SLAVE_ADDRESS_WIDTH         {26}
        set_instance_parameter_value      isp_rob_se_vfr_lres       SUB_WINDOW_COUNT            {1}
        set_instance_parameter_value      isp_rob_se_vfr_lres       SYNC_RESET                  {0}

        # isp_rob_irq_cdc_3
        set_instance_parameter_value      isp_rob_irq_cdc_3         IRQ_WIDTH                   {1}
        set_instance_parameter_value      isp_rob_irq_cdc_3         SYNC_RESET                  {0}
    }

    # isp_rob_pix_adapt_gs
    set_instance_parameter_value      isp_rob_pix_adapt_gs          BPS_IN                      ${v_vid_out_bps}
    set_instance_parameter_value      isp_rob_pix_adapt_gs          BPS_OUT                     ${v_rob_bps}
    set_instance_parameter_value      isp_rob_pix_adapt_gs          ENABLE_DEBUG                ${v_enable_debug}
    set_instance_parameter_value      isp_rob_pix_adapt_gs          EXTERNAL_MODE               {1}
    set_instance_parameter_value      isp_rob_pix_adapt_gs          NUMBER_OF_COLOR_PLANES      ${v_rob_cppp}
    set_instance_parameter_value      isp_rob_pix_adapt_gs          PIPELINE_READY              ${v_pipeline_ready}
    set_instance_parameter_value      isp_rob_pix_adapt_gs          PIXELS_IN_PARALLEL          ${v_pip}
    set_instance_parameter_value      isp_rob_pix_adapt_gs          P_CORE_CTRL_ID              {0}
    set_instance_parameter_value      isp_rob_pix_adapt_gs          P_UPDATE_CMD_SUPPORTED      {0}
    set_instance_parameter_value      isp_rob_pix_adapt_gs          RUNTIME_CONTROL             {0}
    set_instance_parameter_value      isp_rob_pix_adapt_gs          SEPARATE_SLAVE_CLOCK        {0}
    set_instance_parameter_value      isp_rob_pix_adapt_gs          SLAVE_PROTOCOL              {Avalon}

    # isp_rob_csc_gs
    set_instance_parameter_value      isp_rob_csc_gs                BPS_IN                      ${v_rob_bps}
    set_instance_parameter_value      isp_rob_csc_gs                BPS_OUT                     ${v_rob_bps}
    set_instance_parameter_value      isp_rob_csc_gs                COEFFICIENT_INT_BITS        {0}
    set_instance_parameter_value      isp_rob_csc_gs                COEFFICIENT_SIGNED          {1}
    set_instance_parameter_value      isp_rob_csc_gs                COEF_SUM_FRACTION_BITS      {8}
    set_instance_parameter_value      isp_rob_csc_gs                C_OMNI_CAP_ID_ASSOCIATED    {0}
    set_instance_parameter_value      isp_rob_csc_gs                C_OMNI_CAP_ID_COMPONENT     {1}
    set_instance_parameter_value      isp_rob_csc_gs                C_OMNI_CAP_IRQ              {255}
    set_instance_parameter_value      isp_rob_csc_gs                C_OMNI_CAP_IRQ_ENABLE       {0}
    set_instance_parameter_value      isp_rob_csc_gs                C_OMNI_CAP_IRQ_ENABLE_EN    {0}
    set_instance_parameter_value      isp_rob_csc_gs                C_OMNI_CAP_IRQ_STATUS       {0}
    set_instance_parameter_value      isp_rob_csc_gs                C_OMNI_CAP_IRQ_STATUS_EN    {0}
    set_instance_parameter_value      isp_rob_csc_gs                C_OMNI_CAP_TAG              {0}
    set_instance_parameter_value      isp_rob_csc_gs                C_OMNI_CAP_TYPE             {559}
    set_instance_parameter_value      isp_rob_csc_gs                C_OMNI_CAP_VERSION          {1}
    set_instance_parameter_value      isp_rob_csc_gs                ENABLE_DEBUG                ${v_enable_debug}
    set_instance_parameter_value      isp_rob_csc_gs                EXTERNAL_MODE               {1}
    set_instance_parameter_value      isp_rob_csc_gs                MOVE_BINARY_POINT_RIGHT     {0}
    set_instance_parameter_value      isp_rob_csc_gs                OUTPUT_COLORSPACE           {1}
    set_instance_parameter_value      isp_rob_csc_gs                PIPELINE_READY              ${v_pipeline_ready}
    set_instance_parameter_value      isp_rob_csc_gs                PIXELS_IN_PARALLEL          ${v_pip}
    set_instance_parameter_value      isp_rob_csc_gs                P_CORE_CTRL_ID              {0}
    set_instance_parameter_value      isp_rob_csc_gs                P_UPDATE_CMD_SUPPORTED      {0}
    set_instance_parameter_value      isp_rob_csc_gs                REMOVE_FRACTION_METHOD      {1}
    set_instance_parameter_value      isp_rob_csc_gs                REQ_FCOEFF_A0               {0.439}
    set_instance_parameter_value      isp_rob_csc_gs                REQ_FCOEFF_A1               {0.062}
    set_instance_parameter_value      isp_rob_csc_gs                REQ_FCOEFF_A2               {-0.04}
    set_instance_parameter_value      isp_rob_csc_gs                REQ_FCOEFF_B0               {-0.338}
    set_instance_parameter_value      isp_rob_csc_gs                REQ_FCOEFF_B1               {0.614}
    set_instance_parameter_value      isp_rob_csc_gs                REQ_FCOEFF_B2               {-0.399}
    set_instance_parameter_value      isp_rob_csc_gs                REQ_FCOEFF_C0               {-0.101}
    set_instance_parameter_value      isp_rob_csc_gs                REQ_FCOEFF_C1               {0.183}
    set_instance_parameter_value      isp_rob_csc_gs                REQ_FCOEFF_C2               {0.439}
    set_instance_parameter_value      isp_rob_csc_gs                REQ_FCOEFF_S0               {128.0}
    set_instance_parameter_value      isp_rob_csc_gs                REQ_FCOEFF_S1               {16.0}
    set_instance_parameter_value      isp_rob_csc_gs                REQ_FCOEFF_S2               {128.0}
    set_instance_parameter_value      isp_rob_csc_gs                RUNTIME_CONTROL             {0}
    set_instance_parameter_value      isp_rob_csc_gs                SEPARATE_SLAVE_CLOCK        {1}
    set_instance_parameter_value      isp_rob_csc_gs                SLAVE_PROTOCOL              {Avalon}
    set_instance_parameter_value      isp_rob_csc_gs                SUMMAND_INT_BITS            {8}
    set_instance_parameter_value      isp_rob_csc_gs                SUMMAND_SIGNED              {0}

    # isp_rob_cpm_gs
    set_instance_parameter_value      isp_rob_cpm_gs                BPS                         ${v_rob_bps}
    set_instance_parameter_value      isp_rob_cpm_gs                C_OMNI_CAP_ID_ASSOCIATED    {0}
    set_instance_parameter_value      isp_rob_cpm_gs                C_OMNI_CAP_ID_COMPONENT     {0}
    set_instance_parameter_value      isp_rob_cpm_gs                C_OMNI_CAP_IRQ              {255}
    set_instance_parameter_value      isp_rob_cpm_gs                C_OMNI_CAP_IRQ_ENABLE       {0}
    set_instance_parameter_value      isp_rob_cpm_gs                C_OMNI_CAP_IRQ_ENABLE_EN    {0}
    set_instance_parameter_value      isp_rob_cpm_gs                C_OMNI_CAP_IRQ_STATUS       {0}
    set_instance_parameter_value      isp_rob_cpm_gs                C_OMNI_CAP_IRQ_STATUS_EN    {0}
    set_instance_parameter_value      isp_rob_cpm_gs                C_OMNI_CAP_TAG              {0}
    set_instance_parameter_value      isp_rob_cpm_gs                C_OMNI_CAP_TYPE             {571}
    set_instance_parameter_value      isp_rob_cpm_gs                C_OMNI_CAP_VERSION          {1}
    set_instance_parameter_value      isp_rob_cpm_gs                ENABLE_DEBUG                ${v_enable_debug}
    set_instance_parameter_value      isp_rob_cpm_gs                EXTERNAL_MODE               {1}
    set_instance_parameter_value      isp_rob_cpm_gs                IP0_KEEP_COLOR_PLANE_0      {1}
    set_instance_parameter_value      isp_rob_cpm_gs                IP0_KEEP_COLOR_PLANE_1      {1}
    set_instance_parameter_value      isp_rob_cpm_gs                IP0_KEEP_COLOR_PLANE_2      {1}
    set_instance_parameter_value      isp_rob_cpm_gs                IP0_KEEP_COLOR_PLANE_3      {1}
    set_instance_parameter_value      isp_rob_cpm_gs                IP1_KEEP_COLOR_PLANE_0      {1}
    set_instance_parameter_value      isp_rob_cpm_gs                IP1_KEEP_COLOR_PLANE_1      {1}
    set_instance_parameter_value      isp_rob_cpm_gs                IP1_KEEP_COLOR_PLANE_2      {1}
    set_instance_parameter_value      isp_rob_cpm_gs                IP1_KEEP_COLOR_PLANE_3      {1}
    set_instance_parameter_value      isp_rob_cpm_gs                KEEP_IP1_AUX_PACKETS        {1}
    set_instance_parameter_value      isp_rob_cpm_gs                KEEP_OP1_AUX_PACKETS        {1}
    set_instance_parameter_value      isp_rob_cpm_gs                MAPPING_COLOR_PLANE_0       {1}
    set_instance_parameter_value      isp_rob_cpm_gs                MAPPING_COLOR_PLANE_1       {1}
    set_instance_parameter_value      isp_rob_cpm_gs                MAPPING_COLOR_PLANE_2       {1}
    set_instance_parameter_value      isp_rob_cpm_gs                MAPPING_COLOR_PLANE_3       {0}
    set_instance_parameter_value      isp_rob_cpm_gs                MODE                        {SPLIT}
    set_instance_parameter_value      isp_rob_cpm_gs                NUMBER_OF_COLOR_PLANES_IN0  ${v_rob_cppp}
    set_instance_parameter_value      isp_rob_cpm_gs                NUMBER_OF_COLOR_PLANES_IN1  {2}
    set_instance_parameter_value      isp_rob_cpm_gs                NUMBER_OF_COLOR_PLANES_OUT0 {1}
    set_instance_parameter_value      isp_rob_cpm_gs                NUMBER_OF_COLOR_PLANES_OUT1 {2}
    set_instance_parameter_value      isp_rob_cpm_gs                OP0_KEEP_COLOR_PLANE_0      {0}
    set_instance_parameter_value      isp_rob_cpm_gs                OP0_KEEP_COLOR_PLANE_1      {1}
    set_instance_parameter_value      isp_rob_cpm_gs                OP0_KEEP_COLOR_PLANE_2      {0}
    set_instance_parameter_value      isp_rob_cpm_gs                OP0_KEEP_COLOR_PLANE_3      {0}
    set_instance_parameter_value      isp_rob_cpm_gs                OP1_KEEP_COLOR_PLANE_0      {1}
    set_instance_parameter_value      isp_rob_cpm_gs                OP1_KEEP_COLOR_PLANE_1      {0}
    set_instance_parameter_value      isp_rob_cpm_gs                OP1_KEEP_COLOR_PLANE_2      {1}
    set_instance_parameter_value      isp_rob_cpm_gs                OP1_KEEP_COLOR_PLANE_3      {0}
    set_instance_parameter_value      isp_rob_cpm_gs                PADDING_COLOR_PLANE_0       {0}
    set_instance_parameter_value      isp_rob_cpm_gs                PADDING_COLOR_PLANE_1       {0}
    set_instance_parameter_value      isp_rob_cpm_gs                PADDING_COLOR_PLANE_2       {0}
    set_instance_parameter_value      isp_rob_cpm_gs                PADDING_COLOR_PLANE_3       {0}
    set_instance_parameter_value      isp_rob_cpm_gs                PIXELS_IN_PARALLEL          ${v_pip}
    set_instance_parameter_value      isp_rob_cpm_gs                P_CORE_CTRL_ID              {0}
    set_instance_parameter_value      isp_rob_cpm_gs                P_UPDATE_CMD_SUPPORTED      {0}
    set_instance_parameter_value      isp_rob_cpm_gs                RUNTIME_CONTROL             {0}
    set_instance_parameter_value      isp_rob_cpm_gs                SEPARATE_SLAVE_CLOCK        {1}
    set_instance_parameter_value      isp_rob_cpm_gs                SLAVE_PROTOCOL              {Avalon}

    # isp_rob_vfw_lres_gs
    set_instance_parameter_value      isp_rob_vfw_lres_gs           BPS                         ${v_rob_bps}
    set_instance_parameter_value      isp_rob_vfw_lres_gs           CLOCKS_ARE_SEPARATE         {1}
    set_instance_parameter_value      isp_rob_vfw_lres_gs           C_OMNI_CAP_ID_ASSOCIATED    {0}
    set_instance_parameter_value      isp_rob_vfw_lres_gs           C_OMNI_CAP_ID_COMPONENT     {2}
    set_instance_parameter_value      isp_rob_vfw_lres_gs           C_OMNI_CAP_IRQ              {255}
    set_instance_parameter_value      isp_rob_vfw_lres_gs           C_OMNI_CAP_IRQ_ENABLE       {0}
    set_instance_parameter_value      isp_rob_vfw_lres_gs           C_OMNI_CAP_IRQ_ENABLE_EN    {0}
    set_instance_parameter_value      isp_rob_vfw_lres_gs           C_OMNI_CAP_IRQ_STATUS       {0}
    set_instance_parameter_value      isp_rob_vfw_lres_gs           C_OMNI_CAP_IRQ_STATUS_EN    {0}
    set_instance_parameter_value      isp_rob_vfw_lres_gs           C_OMNI_CAP_TAG              {0}
    set_instance_parameter_value      isp_rob_vfw_lres_gs           C_OMNI_CAP_TYPE             {585}
    set_instance_parameter_value      isp_rob_vfw_lres_gs           C_OMNI_CAP_VERSION          {1}
    set_instance_parameter_value      isp_rob_vfw_lres_gs           ENABLE_DEBUG                ${v_enable_debug}
    set_instance_parameter_value      isp_rob_vfw_lres_gs           EXTERNAL_MODE               {1}
    set_instance_parameter_value      isp_rob_vfw_lres_gs           MAX_HEIGHT                  ${v_lt_rob_height_max}
    set_instance_parameter_value      isp_rob_vfw_lres_gs           MAX_WIDTH                   ${v_lt_rob_width_max}
    set_instance_parameter_value      isp_rob_vfw_lres_gs           NUMBER_OF_COLOR_PLANES      {1}
    set_instance_parameter_value      isp_rob_vfw_lres_gs           PACKING                     {PERFECT}
    set_instance_parameter_value      isp_rob_vfw_lres_gs           PIXELS_IN_PARALLEL          ${v_pip}
    set_instance_parameter_value      isp_rob_vfw_lres_gs           P_AV_MM_ADDR_WIDTH          {32}
    set_instance_parameter_value      isp_rob_vfw_lres_gs           P_AV_MM_DATA_WIDTH          {256}
    set_instance_parameter_value      isp_rob_vfw_lres_gs           SEPARATE_SLAVE_CLOCK        {1}
    set_instance_parameter_value      isp_rob_vfw_lres_gs           SLAVE_PROTOCOL              {Avalon}
    set_instance_parameter_value      isp_rob_vfw_lres_gs           WRITE_BURST_TARGET          {8}
    set_instance_parameter_value      isp_rob_vfw_lres_gs           WRITE_FIFO_DEPTH            {512}

    # isp_rob_se_vfw_lres_gs
    set_instance_parameter_value      isp_rob_se_vfw_lres_gs        BURSTCOUNT_WIDTH            {6}
    set_instance_parameter_value      isp_rob_se_vfw_lres_gs        DATA_WIDTH                  {256}
    set_instance_parameter_value      isp_rob_se_vfw_lres_gs        ENABLE_SLAVE_PORT           {0}
    set_instance_parameter_value      isp_rob_se_vfw_lres_gs        MASTER_ADDRESS_DEF          {0}
    set_instance_parameter_value      isp_rob_se_vfw_lres_gs        MASTER_ADDRESS_WIDTH        {33}
    set_instance_parameter_value      isp_rob_se_vfw_lres_gs        MAX_PENDING_READS           {8}
    set_instance_parameter_value      isp_rob_se_vfw_lres_gs        SLAVE_ADDRESS_WIDTH         {26}
    set_instance_parameter_value      isp_rob_se_vfw_lres_gs        SUB_WINDOW_COUNT            {1}
    set_instance_parameter_value      isp_rob_se_vfw_lres_gs        SYNC_RESET                  {0}

    # isp_rob_irq_cdc_gs
    set_instance_parameter_value      isp_rob_irq_cdc_gs             IRQ_WIDTH                   {1}
    set_instance_parameter_value      isp_rob_irq_cdc_gs             SYNC_RESET                  {0}

    ## Create Connections ##

    # isp_rob_cpu_clk_bridge
    add_connection      isp_rob_cpu_clk_bridge.out_clk               isp_rob_cpu_rst_bridge.clk
    add_connection      isp_rob_cpu_clk_bridge.out_clk               isp_rob_mm_bridge.clk
    if {${v_aruco_mk_en}} {
        add_connection      isp_rob_cpu_clk_bridge.out_clk               isp_rob_mixer.agent_clock
        add_connection      isp_rob_cpu_clk_bridge.out_clk               isp_rob_aruco_ctrl.clk
    }
    add_connection      isp_rob_cpu_clk_bridge.out_clk               isp_rob_clipper.agent_clock
    add_connection      isp_rob_cpu_clk_bridge.out_clk               isp_rob_scaler_down.agent_clock
    add_connection      isp_rob_cpu_clk_bridge.out_clk               isp_rob_vfb.control_clock
    add_connection      isp_rob_cpu_clk_bridge.out_clk               isp_rob_vfw_lres.control_clock
    if {${v_en_frame_rd}} {
        add_connection      isp_rob_cpu_clk_bridge.out_clk               isp_rob_vfr_lres.control_clock
    }
    add_connection      isp_rob_cpu_clk_bridge.out_clk               isp_rob_vfw_lres_gs.control_clock

    # isp_rob_cpu_rst_bridge
    add_connection      isp_rob_cpu_rst_bridge.out_reset             isp_rob_mm_bridge.reset
    if {${v_aruco_mk_en}} {
        add_connection      isp_rob_cpu_rst_bridge.out_reset             isp_rob_mixer.agent_reset
        add_connection      isp_rob_cpu_rst_bridge.out_reset             isp_rob_aruco_ctrl.reset
    }
    add_connection      isp_rob_cpu_rst_bridge.out_reset             isp_rob_clipper.agent_reset
    add_connection      isp_rob_cpu_rst_bridge.out_reset             isp_rob_scaler_down.agent_reset
    add_connection      isp_rob_cpu_rst_bridge.out_reset             isp_rob_vfb.control_reset
    add_connection      isp_rob_cpu_rst_bridge.out_reset             isp_rob_vfw_lres.control_reset
    if {${v_en_frame_rd}} {
        add_connection      isp_rob_cpu_rst_bridge.out_reset             isp_rob_vfr_lres.control_reset
    }
    add_connection      isp_rob_cpu_rst_bridge.out_reset             isp_rob_vfw_lres_gs.control_reset


    if {${v_en_hps_host}} {
        # isp_rob_hps_clk_bridge
        add_connection      isp_rob_hps_clk_bridge.out_clk               isp_rob_hps_rst_bridge.clk
        add_connection      isp_rob_hps_clk_bridge.out_clk               hps_fpga_rob_cc_bridge.m0_clk
        add_connection      isp_rob_cpu_clk_bridge.out_clk               hps_fpga_rob_cc_bridge.s0_clk

        # isp_rob_cpu_rst_bridge
        add_connection      isp_rob_hps_rst_bridge.out_reset             hps_fpga_rob_cc_bridge.m0_reset
        add_connection      isp_rob_cpu_rst_bridge.out_reset             hps_fpga_rob_cc_bridge.s0_reset
    }

    # handling the interrupt connection if the HPS is present (you can manage which interrupt host)
    if {(${v_lres_vfw_irq_host} == "hps_subsystem") && (${v_en_hps_host})} {
        add_connection      isp_rob_hps_clk_bridge.out_clk               isp_rob_irq_cdc_2.sender_clk
        add_connection      isp_rob_hps_rst_bridge.out_reset             isp_rob_irq_cdc_2.sender_clk_reset
        add_connection      isp_rob_hps_clk_bridge.out_clk               isp_rob_irq_cdc_gs.sender_clk
        add_connection      isp_rob_hps_rst_bridge.out_reset             isp_rob_irq_cdc_gs.sender_clk_reset
        if {${v_en_frame_rd}} {
            add_connection      isp_rob_hps_clk_bridge.out_clk               isp_rob_irq_cdc_3.sender_clk
            add_connection      isp_rob_hps_rst_bridge.out_reset             isp_rob_irq_cdc_3.sender_clk_reset
        }
    } else {
        add_connection      isp_rob_cpu_clk_bridge.out_clk               isp_rob_irq_cdc_2.sender_clk
        add_connection      isp_rob_cpu_rst_bridge.out_reset             isp_rob_irq_cdc_2.sender_clk_reset
        add_connection      isp_rob_cpu_clk_bridge.out_clk               isp_rob_irq_cdc_gs.sender_clk
        add_connection      isp_rob_cpu_rst_bridge.out_reset             isp_rob_irq_cdc_gs.sender_clk_reset
        if {${v_en_frame_rd}} {
            add_connection      isp_rob_cpu_clk_bridge.out_clk               isp_rob_irq_cdc_3.sender_clk
            add_connection      isp_rob_cpu_rst_bridge.out_reset             isp_rob_irq_cdc_3.sender_clk_reset
        }
    }

    # isp_rob_vid_clk_bridge
    add_connection         isp_rob_vid_clk_bridge.out_clk            isp_rob_vid_rst_bridge.clk
    add_connection         isp_rob_vid_clk_bridge.out_clk            isp_rob_axi4s_bcast_full.vid_clock
    if {${v_aruco_mk_en}} {
        add_connection         isp_rob_vid_clk_bridge.out_clk            isp_rob_mixer.main_clock
        for {set i 0} {$i < ${v_aruco_mk_qty}} {incr i} {
            add_connection         isp_rob_vid_clk_bridge.out_clk   isp_rob_aruco_mk_gen_${i}.main_clock
        }
    }
    add_connection         isp_rob_vid_clk_bridge.out_clk            isp_rob_clipper.main_clock
    add_connection         isp_rob_vid_clk_bridge.out_clk            isp_rob_scaler_down.main_clock
    add_connection         isp_rob_vid_clk_bridge.out_clk            isp_rob_axi4s_bcast.vid_clock
    add_connection         isp_rob_vid_clk_bridge.out_clk            isp_rob_vfb.main_clock
    add_connection         isp_rob_vid_clk_bridge.out_clk            isp_rob_pix_adapt.main_clock
    add_connection         isp_rob_vid_clk_bridge.out_clk            isp_rob_vfw_lres.main_clock
    add_connection         isp_rob_vid_clk_bridge.out_clk            isp_rob_irq_cdc_2.receiver_clk
    if {${v_en_frame_rd}} {
        add_connection         isp_rob_vid_clk_bridge.out_clk            isp_rob_vfr_lres.main_clock
        add_connection         isp_rob_vid_clk_bridge.out_clk            isp_rob_pix_adapt2.main_clock
        add_connection         isp_rob_vid_clk_bridge.out_clk            isp_rob_irq_cdc_3.receiver_clk
    }
    add_connection         isp_rob_vid_clk_bridge.out_clk            isp_rob_pix_adapt_gs.main_clock
    add_connection         isp_rob_vid_clk_bridge.out_clk            isp_rob_csc_gs.main_clock
    add_connection         isp_rob_vid_clk_bridge.out_clk            isp_rob_cpm_gs.main_clock
    add_connection         isp_rob_vid_clk_bridge.out_clk            isp_rob_vfw_lres_gs.main_clock
    add_connection         isp_rob_vid_clk_bridge.out_clk            isp_rob_irq_cdc_gs.receiver_clk
    if {${v_en_gs_output}} {
        add_connection         isp_rob_vid_clk_bridge.out_clk            isp_rob_csc_gs_out.main_clock
        add_connection         isp_rob_vid_clk_bridge.out_clk            isp_rob_cpm_gs_out.main_clock
    }

    # isp_rob_vid_rst_bridge
    add_connection         isp_rob_vid_rst_bridge.out_reset         isp_rob_axi4s_bcast_full.vid_reset
    if {${v_aruco_mk_en}} {
        add_connection         isp_rob_vid_rst_bridge.out_reset           isp_rob_mixer.main_reset
        for {set i 0} {$i < ${v_aruco_mk_qty}} {incr i} {
            add_connection         isp_rob_vid_rst_bridge.out_reset           isp_rob_aruco_mk_gen_${i}.main_reset
        }
    }
    add_connection         isp_rob_vid_rst_bridge.out_reset          isp_rob_clipper.main_reset
    add_connection         isp_rob_vid_rst_bridge.out_reset          isp_rob_scaler_down.main_reset
    add_connection         isp_rob_vid_rst_bridge.out_reset          isp_rob_axi4s_bcast.vid_reset
    add_connection         isp_rob_vid_rst_bridge.out_reset          isp_rob_vfb.main_reset
    add_connection         isp_rob_vid_rst_bridge.out_reset          isp_rob_pix_adapt.main_reset
    add_connection         isp_rob_vid_rst_bridge.out_reset          isp_rob_vfw_lres.main_reset
    add_connection         isp_rob_vid_rst_bridge.out_reset          isp_rob_irq_cdc_2.receiver_clk_reset
    if {${v_en_frame_rd}} {
        add_connection         isp_rob_vid_rst_bridge.out_reset          isp_rob_vfr_lres.main_reset
        add_connection         isp_rob_vid_rst_bridge.out_reset          isp_rob_pix_adapt2.main_reset
        add_connection         isp_rob_vid_rst_bridge.out_reset          isp_rob_irq_cdc_3.receiver_clk_reset
    }
    add_connection         isp_rob_vid_rst_bridge.out_reset          isp_rob_pix_adapt_gs.main_reset
    add_connection         isp_rob_vid_rst_bridge.out_reset          isp_rob_csc_gs.main_reset
    add_connection         isp_rob_vid_rst_bridge.out_reset          isp_rob_cpm_gs.main_reset
    add_connection         isp_rob_vid_rst_bridge.out_reset          isp_rob_vfw_lres_gs.main_reset
    add_connection         isp_rob_vid_rst_bridge.out_reset          isp_rob_irq_cdc_gs.receiver_clk_reset
    if {${v_en_gs_output}} {
        add_connection         isp_rob_vid_rst_bridge.out_reset          isp_rob_csc_gs_out.main_reset
        add_connection         isp_rob_vid_rst_bridge.out_reset          isp_rob_cpm_gs_out.main_reset
    }

    # isp_rob_emif_clk_bridge
    add_connection         isp_rob_emif_clk_bridge.out_clk           isp_rob_emif_rst_bridge.clk
    add_connection         isp_rob_emif_clk_bridge.out_clk           isp_rob_vfb.mem_clock
    add_connection         isp_rob_emif_clk_bridge.out_clk           isp_rob_se_vfb.clock
    add_connection         isp_rob_emif_clk_bridge.out_clk           isp_rob_vfw_lres.mem_clock
    add_connection         isp_rob_emif_clk_bridge.out_clk           isp_rob_se_vfw_lres.clock
    if {${v_en_frame_rd}} {
        add_connection         isp_rob_emif_clk_bridge.out_clk           isp_rob_vfr_lres.mem_clock
        add_connection         isp_rob_emif_clk_bridge.out_clk           isp_rob_se_vfr_lres.clock
    }
    add_connection         isp_rob_emif_clk_bridge.out_clk           isp_rob_vfw_lres_gs.mem_clock
    add_connection         isp_rob_emif_clk_bridge.out_clk           isp_rob_se_vfw_lres_gs.clock

    # isp_rob_emif_rst_bridge
    add_connection         isp_rob_emif_rst_bridge.out_reset         isp_rob_vfb.mem_reset
    add_connection         isp_rob_emif_rst_bridge.out_reset         isp_rob_se_vfb.reset
    add_connection         isp_rob_emif_rst_bridge.out_reset         isp_rob_vfw_lres.mem_reset
    add_connection         isp_rob_emif_rst_bridge.out_reset         isp_rob_se_vfw_lres.reset
    if {${v_en_frame_rd}} {
        add_connection         isp_rob_emif_rst_bridge.out_reset         isp_rob_vfr_lres.mem_reset
        add_connection         isp_rob_emif_rst_bridge.out_reset         isp_rob_se_vfr_lres.reset
    }
    add_connection         isp_rob_emif_rst_bridge.out_reset         isp_rob_vfw_lres_gs.mem_reset
    add_connection         isp_rob_emif_rst_bridge.out_reset         isp_rob_se_vfw_lres_gs.reset

    # isp_rob_mm_bridge
    if {${v_aruco_mk_en}} {
        add_connection          isp_rob_mm_bridge.m0                     isp_rob_mixer.av_mm_control_agent
        add_connection          isp_rob_mm_bridge.m0                     isp_rob_aruco_ctrl.s1
    }
    add_connection          isp_rob_mm_bridge.m0                     isp_rob_clipper.av_mm_control_agent
    add_connection          isp_rob_mm_bridge.m0                     isp_rob_scaler_down.av_mm_control_agent
    add_connection          isp_rob_mm_bridge.m0                     isp_rob_vfb.av_mm_control_agent
    add_connection          isp_rob_mm_bridge.m0                     isp_rob_vfw_lres.av_mm_control_agent
    if {${v_en_frame_rd}} {
        add_connection          isp_rob_mm_bridge.m0                     isp_rob_vfr_lres.av_mm_control_agent
    }
    add_connection          isp_rob_mm_bridge.m0                     isp_rob_vfw_lres_gs.av_mm_control_agent

    if {${v_en_hps_host}} {
    # hps_fpga_rob_cc_bridge
        if {${v_aruco_mk_en}} {
            add_connection          hps_fpga_rob_cc_bridge.m0                     isp_rob_mixer.av_mm_control_agent
            add_connection          hps_fpga_rob_cc_bridge.m0                     isp_rob_aruco_ctrl.s1
        }
        add_connection          hps_fpga_rob_cc_bridge.m0                isp_rob_clipper.av_mm_control_agent
        add_connection          hps_fpga_rob_cc_bridge.m0                isp_rob_scaler_down.av_mm_control_agent
        add_connection          hps_fpga_rob_cc_bridge.m0                isp_rob_vfb.av_mm_control_agent
        add_connection          hps_fpga_rob_cc_bridge.m0                isp_rob_vfw_lres.av_mm_control_agent
        if {${v_en_frame_rd}} {
            add_connection          hps_fpga_rob_cc_bridge.m0                isp_rob_vfr_lres.av_mm_control_agent
        }
        add_connection          hps_fpga_rob_cc_bridge.m0                isp_rob_vfw_lres_gs.av_mm_control_agent
    }

    if {${v_aruco_mk_en}} {
        # isp_rob_mixer
        add_connection      isp_rob_axi4s_bcast_full.axi4s_vid_out_1     isp_rob_mixer.axi4s_vid_0_in
        add_connection      isp_rob_mixer.axi4s_vid_out                  isp_rob_axi4s_bcast_full.axi4s_vid_in
        for {set i 0} {$i < ${v_aruco_mk_qty}} {incr i} {
             add_connection      isp_rob_aruco_mk_gen_${i}.axi4s_vid_out      isp_rob_mixer.axi4s_vid_[expr ${i}+1]_in
        }
    }

    #isp_rob_axi4s_bcast_full 1
    add_connection          isp_rob_axi4s_bcast_full.axi4s_vid_out_1     isp_rob_clipper.axi4s_vid_in

    # isp_rob_clipper
    add_connection          isp_rob_clipper.axi4s_vid_out           isp_rob_scaler_down.axi4s_vid_in

    #isp_rob_scaler_down
    add_connection          isp_rob_scaler_down.axi4s_vid_out       isp_rob_axi4s_bcast.axi4s_vid_in

    # isp_rob_pix_adapt
    add_connection          isp_rob_axi4s_bcast.axi4s_vid_out_1     isp_rob_pix_adapt.axi4s_vid_in
    add_connection          isp_rob_pix_adapt.axi4s_vid_out         isp_rob_vfw_lres.axi4s_vid_in

    #isp_rob_vfb
    if {${v_en_gs_output}} {
        #need some blocks if GS
        add_connection          isp_rob_axi4s_bcast.axi4s_vid_out_0     isp_rob_csc_gs_out.axi4s_vid_in
        # isp_rob_csc_gs_out
        add_connection          isp_rob_csc_gs_out.axi4s_vid_out        isp_rob_cpm_gs_out.axi4s_vid_in
        # isp_rob_cpm_gs_out
        add_connection          isp_rob_cpm_gs_out.axi4s_vid_out        isp_rob_vfb.axi4s_vid_in
    } else {
        #direct connection if RGB
        add_connection          isp_rob_axi4s_bcast.axi4s_vid_out_0     isp_rob_vfb.axi4s_vid_in
    }
    add_connection          isp_rob_vfb.av_mm_mem_write_host        isp_rob_se_vfb.windowed_slave
    add_connection          isp_rob_vfb.av_mm_mem_read_host         isp_rob_se_vfb.windowed_slave

    # isp_rob_vfw_lres
    add_connection          isp_rob_vfw_lres.av_mm_mem_write_host   isp_rob_se_vfw_lres.windowed_slave

    # isp_rob_irq_cdc
    add_connection          isp_rob_irq_cdc_2.receiver              isp_rob_vfw_lres.frame_writer_int

    # isp_rob_vfr_lres
    if {${v_en_frame_rd}} {
        add_connection          isp_rob_vfr_lres.av_mm_mem_read_host    isp_rob_se_vfr_lres.windowed_slave
        add_connection          isp_rob_vfr_lres.axi4s_vid_out          isp_rob_pix_adapt2.axi4s_vid_in
        add_connection          isp_rob_irq_cdc_3.receiver              isp_rob_vfr_lres.frame_reader_int
    }

    # isp_rob_pix_adapt_gs
    add_connection          isp_rob_axi4s_bcast.axi4s_vid_out_2     isp_rob_pix_adapt_gs.axi4s_vid_in
    add_connection          isp_rob_pix_adapt_gs.axi4s_vid_out      isp_rob_csc_gs.axi4s_vid_in

    # isp_rob_csc_gs
    add_connection          isp_rob_csc_gs.axi4s_vid_out            isp_rob_cpm_gs.axi4s_vid_in

    # isp_rob_cpm_gs
    add_connection          isp_rob_cpm_gs.axi4s_vid_out_0          isp_rob_vfw_lres_gs.axi4s_vid_in

    # isp_rob_vfw_lres_gs
    add_connection          isp_rob_vfw_lres_gs.av_mm_mem_write_host   isp_rob_se_vfw_lres_gs.windowed_slave

    # isp_rob_irq_cdc_gs
    add_connection          isp_rob_irq_cdc_gs.receiver              isp_rob_vfw_lres_gs.frame_writer_int

    ### Create Exports ###

    # isp_rob_cpu_clk_bridge
    add_interface           cpu_clk_in    clock       sink
    set_interface_property  cpu_clk_in    EXPORT_OF   isp_rob_cpu_clk_bridge.in_clk

    # isp_rob_cpu_rst_bridge
    add_interface           cpu_rst_in    reset       sink
    set_interface_property  cpu_rst_in    EXPORT_OF   isp_rob_cpu_rst_bridge.in_reset

    if {${v_en_hps_host}} {
        # isp_rob_hps_clk_bridge
        add_interface           hps_clk_in    clock       sink
        set_interface_property  hps_clk_in    EXPORT_OF   isp_rob_hps_clk_bridge.in_clk

        # isp_rob_hps_rst_bridge
        add_interface           hps_rst_in    reset       sink
        set_interface_property  hps_rst_in    EXPORT_OF   isp_rob_hps_rst_bridge.in_reset
    }

    # isp_rob_vid_clk_bridge
    set_interface_property  vid_clk_in    EXPORT_OF   isp_rob_vid_clk_bridge.in_clk

    # isp_rob_vid_rst_bridge
    set_interface_property  vid_rst_in    EXPORT_OF   isp_rob_vid_rst_bridge.in_reset

    # isp_rob_emif_clk_bridge
    set_interface_property  emif_clk_in   EXPORT_OF   isp_rob_emif_clk_bridge.in_clk

    # isp_rob_emif_rst_bridge
    set_interface_property  emif_rst_in   EXPORT_OF   isp_rob_emif_rst_bridge.in_reset

    # isp_rob_mm_bridge
    add_interface           mm_ctrl_in    avalon      slave
    set_interface_property  mm_ctrl_in    EXPORT_OF   isp_rob_mm_bridge.s0

    if {${v_en_hps_host}} {
        # hps_fpga_rob_cc_bridge
        add_interface           mm_ctrl_in_hps    avalon      slave
        set_interface_property  mm_ctrl_in_hps    EXPORT_OF   hps_fpga_rob_cc_bridge.s0
    }

    # isp_rob_clipper / mixer
    if {${v_aruco_mk_en}} {
        add_interface           isp_out_s_vid_axis    axi4stream  subordinate
        set_interface_property  isp_out_s_vid_axis    EXPORT_OF   isp_rob_mixer.axi4s_vid_0_in
    } else {
        # broadcaster_full
        add_interface           isp_out_s_vid_axis    axi4stream  subordinate
        set_interface_property  isp_out_s_vid_axis    EXPORT_OF   isp_rob_axi4s_bcast_full.axi4s_vid_in
    }

    add_interface           isp_out_m_vid_axis_0  axi4stream  manager
    set_interface_property  isp_out_m_vid_axis_0  EXPORT_OF   isp_rob_axi4s_bcast_full.axi4s_vid_out_0

    # isp_rob_se_vfb
    set_interface_property  av_mm_host_se_vfb     EXPORT_OF   isp_rob_se_vfb.expanded_master

    # isp_rob_vfb
    add_interface           rob_res_out_m_vid_axis_0          axi4stream  manager
    set_interface_property  rob_res_out_m_vid_axis_0          EXPORT_OF   isp_rob_vfb.axi4s_vid_out

    # isp_rob_irq_cdc
    set_interface_property    lres_vfw_int        EXPORT_OF   isp_rob_irq_cdc_2.sender

    # isp_rob_se_vfw_lres
    set_interface_property  av_mm_host_lres_vfw   EXPORT_OF   isp_rob_se_vfw_lres.expanded_master

    # isp_rob_vfr_lres
    if {${v_en_frame_rd}} {
        add_interface           rob_res_out_m_vid_axis_1    axi4stream  manager
        set_interface_property  rob_res_out_m_vid_axis_1    EXPORT_OF   isp_rob_pix_adapt2.axi4s_vid_out

        # isp_rob_se_vfr_lres
        set_interface_property  av_mm_host_lres_vfr    EXPORT_OF   isp_rob_se_vfr_lres.expanded_master
        # isp_rob_irq_cdc
        set_interface_property    lres_vfr_int         EXPORT_OF   isp_rob_irq_cdc_3.sender
    }

    # isp_rob_irq_cdc_gs
    set_interface_property    lres_vfw_gs_int        EXPORT_OF   isp_rob_irq_cdc_gs.sender

    # isp_rob_se_vfw_lres_gs
    set_interface_property  av_mm_host_lres_gs_vfw   EXPORT_OF   isp_rob_se_vfw_lres_gs.expanded_master

    # isp_rob_cpm_gs
    set_interface_property  cpm_gs_vid_out_1         EXPORT_OF   isp_rob_cpm_gs.axi4s_vid_out_1

    ### Assign Base Addresses ###

    if {${v_aruco_mk_en}} {
         set_connection_parameter_value isp_rob_mm_bridge.m0/isp_rob_mixer.av_mm_control_agent      baseAddress "0x00001000"
         set_connection_parameter_value isp_rob_mm_bridge.m0/isp_rob_aruco_ctrl.s1                  baseAddress "0x00000600"
    }
    set_connection_parameter_value isp_rob_mm_bridge.m0/isp_rob_scaler_down.av_mm_control_agent     baseAddress "0x00000000"
    set_connection_parameter_value isp_rob_mm_bridge.m0/isp_rob_clipper.av_mm_control_agent         baseAddress "0x00000400"
    set_connection_parameter_value isp_rob_mm_bridge.m0/isp_rob_vfb.av_mm_control_agent             baseAddress "0x00000C00"
    set_connection_parameter_value isp_rob_mm_bridge.m0/isp_rob_vfw_lres.av_mm_control_agent        baseAddress "0x00000200"
    set_connection_parameter_value isp_rob_mm_bridge.m0/isp_rob_vfw_lres_gs.av_mm_control_agent     baseAddress "0x00000800"

    if {${v_en_hps_host}} {
        if {${v_aruco_mk_en}} {
            set_connection_parameter_value hps_fpga_rob_cc_bridge.m0/isp_rob_mixer.av_mm_control_agent   baseAddress "0x00001000"
            set_connection_parameter_value hps_fpga_rob_cc_bridge.m0/isp_rob_aruco_ctrl.s1               baseAddress "0x00000600"
        }
        set_connection_parameter_value hps_fpga_rob_cc_bridge.m0/isp_rob_scaler_down.av_mm_control_agent baseAddress "0x00000000"
        set_connection_parameter_value hps_fpga_rob_cc_bridge.m0/isp_rob_clipper.av_mm_control_agent     baseAddress "0x00000400"
        set_connection_parameter_value hps_fpga_rob_cc_bridge.m0/isp_rob_vfb.av_mm_control_agent         baseAddress "0x00000C00"
        set_connection_parameter_value hps_fpga_rob_cc_bridge.m0/isp_rob_vfw_lres.av_mm_control_agent    baseAddress "0x00000200"
        set_connection_parameter_value hps_fpga_rob_cc_bridge.m0/isp_rob_vfw_lres_gs.av_mm_control_agent baseAddress "0x00000800"
    }

    if {${v_en_frame_rd}} {
        set_connection_parameter_value isp_rob_mm_bridge.m0/isp_rob_vfr_lres.av_mm_control_agent          baseAddress "0x00000800"
        if {${v_en_hps_host}} {
            set_connection_parameter_value hps_fpga_rob_cc_bridge.m0/isp_rob_vfr_lres.av_mm_control_agent     baseAddress "0x00000800"
        }
    }

    if {${v_aruco_mk_en}} {
        lock_avalon_base_address  isp_rob_mixer.av_mm_control_agent
        lock_avalon_base_address  isp_rob_aruco_ctrl.s1
    }
    lock_avalon_base_address  isp_rob_clipper.av_mm_control_agent
    lock_avalon_base_address  isp_rob_scaler_down.av_mm_control_agent
    lock_avalon_base_address  isp_rob_vfb.av_mm_control_agent
    lock_avalon_base_address  isp_rob_vfw_lres.av_mm_control_agent
    if {${v_en_frame_rd}} {
    lock_avalon_base_address  isp_rob_vfr_lres.av_mm_control_agent
    }
    lock_avalon_base_address  isp_rob_vfw_lres_gs.av_mm_control_agent

    ### Sync / Validation ###

    sync_sysinfo_parameters
    save_system

}


proc edit_top_level_qsys {} {

    set v_project_name        [get_shell_parameter PROJECT_NAME]
    set v_project_path        [get_shell_parameter PROJECT_PATH]
    set v_instance_name       [get_shell_parameter INSTANCE_NAME]

    load_system ${v_project_path}/rtl/${v_project_name}_qsys.qsys

    add_instance ${v_instance_name}   ${v_instance_name}

    # rob_cmp_gs_vid_out_1
    add_interface             "${v_instance_name}_cpm_gs_vid_out_1"            axi4stream   master
    set_interface_property    "${v_instance_name}_cpm_gs_vid_out_1" \
                              export_of   ${v_instance_name}.cpm_gs_vid_out_1

    sync_sysinfo_parameters
    save_system

}

proc add_auto_connections {} {

    set v_instance_name               [get_shell_parameter INSTANCE_NAME]
    set v_avmm_host                   [get_shell_parameter AVMM_HOST]
    set v_avmm_host_2                 [get_shell_parameter AVMM_HOST_2]
    set v_vid_out_rate                [get_shell_parameter VID_OUT_RATE]
    set v_pip                         [get_shell_parameter PIP]
    set v_lres_vfw_irq_priority       [get_shell_parameter LRES_VFW_IRQ_PRIORITY]
    set v_lres_vfw_irq_host           [get_shell_parameter LRES_VFW_IRQ_HOST]
    set v_en_frame_rd                 [get_shell_parameter EN_FRAME_RD]
    set v_lres_vfr_irq_priority       [get_shell_parameter LRES_VFR_IRQ_PRIORITY]
    set v_lres_vfr_irq_host           [get_shell_parameter LRES_VFR_IRQ_HOST]
    set v_lres_vfw_gs_irq_priority    [get_shell_parameter LRES_VFW_GS_IRQ_PRIORITY]
    set v_lres_vfw_gs_irq_host        [get_shell_parameter LRES_VFW_GS_IRQ_HOST]
    set v_en_hps_host                 [get_shell_parameter EN_HPS_HOST]
    set v_fpga_emif_async             [get_shell_parameter FPGA_EMIF_ASYNC]
    set v_fpga_emif_async_clk_hz      [get_shell_parameter FPGA_EMIF_ASYNC_CLK_HZ]

    add_auto_connection   ${v_instance_name}    cpu_clk_in        100000000
    add_auto_connection   ${v_instance_name}    cpu_rst_in        100000000

    if {${v_en_hps_host}} {
        add_auto_connection   ${v_instance_name}    hps_clk_in        200000000
        add_auto_connection   ${v_instance_name}    hps_rst_in        200000000
    }


    if {(${v_vid_out_rate} == "p60") || (${v_pip} == 1)} {
        add_auto_connection   ${v_instance_name}    vid_clk_in        297000000
        add_auto_connection   ${v_instance_name}    vid_rst_in        297000000
    } else {
        add_auto_connection   ${v_instance_name}    vid_clk_in        148500000
        add_auto_connection   ${v_instance_name}    vid_rst_in        148500000
    }

    if {${v_fpga_emif_async}} {
        add_auto_connection    ${v_instance_name}    emif_clk_in         ${v_fpga_emif_async_clk_hz}
    } else {
        add_auto_connection    ${v_instance_name}    emif_clk_in         emif_user_clk
    }

    add_auto_connection   ${v_instance_name}    emif_rst_in             emif_user_rst
    add_auto_connection   ${v_instance_name}    av_mm_host_se_vfb       emif_user_data
    add_auto_connection   ${v_instance_name}    av_mm_host_lres_vfw     emif_user_data
    add_auto_connection   ${v_instance_name}    av_mm_host_lres_gs_vfw  emif_user_data
    if {${v_en_frame_rd}} {
        add_auto_connection   ${v_instance_name}    av_mm_host_lres_vfr     emif_user_data
    }

    # from isp
    add_auto_connection   ${v_instance_name}    isp_out_s_vid_axis          isp_lite_out_vid

    # to isp out
    add_auto_connection   ${v_instance_name}    isp_out_m_vid_axis_0        isp_out_vid_axis_0
    add_auto_connection   ${v_instance_name}    rob_res_out_m_vid_axis_0    to_mix_vid_axis_0


    if {${v_en_frame_rd}} {
        add_auto_connection   ${v_instance_name}    rob_res_out_m_vid_axis_1    to_mix_vid_axis_1
    }

    # NIOS to mm bridge
    add_avmm_connections  mm_ctrl_in          ${v_avmm_host}

    # HPS to mm bridge
    if {${v_en_hps_host}} {
        add_avmm_connections  mm_ctrl_in_hps      ${v_avmm_host_2}
    }

    # HPS to mm bridge

    # Interrupts
    if {(${v_lres_vfw_irq_host} != "NONE") && (${v_lres_vfw_irq_host} != "")} {
        add_irq_connection ${v_instance_name}   "lres_vfw_int" \
                                          ${v_lres_vfw_irq_priority}  ${v_lres_vfw_irq_host}_irq
    }
    if {${v_en_frame_rd}} {
        if {(${v_lres_vfr_irq_host} != "NONE") && (${v_lres_vfr_irq_host} != "")} {
            add_irq_connection ${v_instance_name}   "lres_vfr_int" \
                                            ${v_lres_vfr_irq_priority}  ${v_lres_vfr_irq_host}_irq
        }
    }
    if {(${v_lres_vfw_gs_irq_host} != "NONE") && (${v_lres_vfw_gs_irq_host} != "")} {
        add_irq_connection ${v_instance_name}   "lres_vfw_gs_int" \
                                          ${v_lres_vfw_gs_irq_priority}  ${v_lres_vfw_gs_irq_host}_irq
    }

}

proc edit_top_v_file {} {

    set v_instance_name             [get_shell_parameter INSTANCE_NAME]

    add_qsys_inst_exports_list  ${v_instance_name}_cpm_gs_vid_out_1_tdata           ""
    add_qsys_inst_exports_list  ${v_instance_name}_cpm_gs_vid_out_1_tvalid          ""
    add_qsys_inst_exports_list  ${v_instance_name}_cpm_gs_vid_out_1_tready          "1'b1"
    add_qsys_inst_exports_list  ${v_instance_name}_cpm_gs_vid_out_1_tlast           ""
    add_qsys_inst_exports_list  ${v_instance_name}_cpm_gs_vid_out_1_tuser           ""

}
