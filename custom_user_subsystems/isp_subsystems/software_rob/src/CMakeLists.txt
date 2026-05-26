# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the
# License.
# *******************************************************************************/

cmake_minimum_required(VERSION 3.14)

add_subdirectory(../build/bsp bsp)
add_subdirectory(btc_dptx_syslib/release_mst/btc_dptx_syslib btc_dptx_syslib)
add_subdirectory(btc_dptxll_syslib/release/btc_dptxll_syslib btc_dptxll_syslib)

include(../build/bsp/toolchain.cmake)

project(app)

enable_language(ASM)
enable_language(C)
enable_language(CXX)

add_executable(app.elf)

target_sources(app.elf
    PRIVATE
        isp_app_src/display_port/aux_decoder.c
        isp_app_src/display_port/dptx_formats.c
        isp_app_src/display_port/intel_axi2cv.c
        isp_app_src/display_port/intel_vab_core.c
        isp_app_src/display_port/tx_utils.c
        isp_app_src/display_port/debug.c
        isp_app_src/display_port/dp_support.c
        isp_app_src/sensors/sensor_imx678.c
        isp_app_src/sensors/framos_gmsl.c
        isp_app_src/sensors/intel_fpga_i2c.c
        isp_app_src/vvp_cores/vvp_cores_cfg.c
        isp_app_src/arUco_helpers/aruco_square_placement.c
        isp_app_src/board.c
        isp_app_src/main.c

)

target_include_directories(app.elf
    PRIVATE
        .
        isp_app_src
        isp_app_src/display_port
        isp_app_src/sensors
        isp_app_src/vvp_cores
        isp_app_src/arUco_helpers
    PUBLIC
)

target_link_libraries(app.elf
    PRIVATE
        -T "${BspLinkerScript}" -nostdlib
        btc_dptx_syslib
        btc_dptxll_syslib
        "${ExtraArchiveLibraries}"
        -Wl,--start-group "${BspLibraryName}" -lc -lstdc++ -lgcc -lm -Wl,--end-group
)

# Create objdump from ELF.
set(objdump app.elf.objdump)
add_custom_command(
    OUTPUT "${objdump}"
    DEPENDS app.elf
    COMMAND "${ToolchainObjdump}" "${ToolchainObjdumpFlags}" app.elf >
            "${objdump}"
    COMMENT "Creating ${objdump}."
    VERBATIM
)
add_custom_target(create-objdump ALL DEPENDS "${objdump}")

# Report space free for stack + heap. Note that the file below is never created
# so the report is always output on build.
set(stack_report_file app.elf.stack_report)
add_custom_command(
    OUTPUT "${stack_report_file}"
    DEPENDS app.elf
    COMMAND niosv-stack-report -p "${ToolchainPrefix}" app.elf
    COMMENT "Reporting memory available for stack + heap in app.elf."
    VERBATIM
)
add_custom_target(niosv-stack-report ALL DEPENDS "${stack_report_file}")
