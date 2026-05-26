# Robotics with Vision (Image Signal Processing + arUco marker generation) and 6-Axis (3x2) Drive-On-Chip based on NIOSV/g with Hard Processor System - Hardware Repository (**HW**).
<br>

## Overview

This repository contains the necessary files and collateral to build and create a Quartus Prime Project variant of the
Robotics with Vision with an HPS instance for higher-level robot manipulation control.

The products of this repository is a SOF file.

**Note:** To produce the JIC/RBF pair files known as "Flash image for HPS First configuration bitstream, phase 1" and
"HPS First configuration bitstream, phase 2" respectively; a **u-boot-spl-dtb.hex** file is needed which is a product of
the **"YOCTO"** component compilation.

<br>

## Software Requirements

The Modular Design Toolkit (MDT) requires the following software and versions (in Linux):

* Altera® Quartus® Prime Pro version (26.1), including open-source tools to
  compile software targeting NiosV soft-processors.
* Device Support.
* FPGA NiosV/g Open-Source Tools 26.1 (installed with Quartus® Prime).

Optional

* DSP Builder for Altera® FPGAs Pro Edition v26.1.

<br>

## Hardware Requirements

To operate this variant on hardware, ensure you have the suitable board or development kit that is
compatible with your chosen `board_subsystem`. Additionally, you may need relevant accessories such as
daughter cards and other components.
<br>
<br>

## Creating and compiling the hardware

### Create this hardware design using the Modular Design Toolkit (MDT)

Follow the next steps to create the Quartus® and Platform Designer Project for
[Agilex™ 5 FPGA and SoC E-Series 065B Modular Development Kit](https://www.altera.com/products/devkit/po-3274/agilex-5-fpga-and-soc-e-series-065b-modular-development-kit)
You can select any other available XML targeting other boards from the `HPS_ISP_VIS_DOC3x2_ROBOTICS` directory.

* Create your workspace and clone the repository using `--recurse-submodules`:

```bash
cd <workspace>
git clone -b <QUARTUS_REL_TAG> --recurse-submodules https://github.com/altera-fpga/agilex-ed-robotics.git altera-robotics
```

* Navigate to `altera-robotics` directory and create your project in a location of your choice `\<project\>`

```bash
cd altera-robotics
quartus_sh -t ./modular_design_toolkit/scripts/create/create_shell.tcl \
   -xml_path ./HPS_ISP_VIS_DOC3x2_ROBOTICS/AGX_5E_Modular_Devkit_HPS_ISP_VIS_DOC3x2_ROB.xml \
   -proj_path <project> -o
```

**Note:** optionally use `-jvm_max_heap 20G` at the end of the previous command to allocate more memory to
Platform Designer subsystem creation.

This will create your Quartus® Prime and Platform Designer Project in \<project>. The folder structure is consistent
with the "Modular Design Toolkit" methodology.

<br>

### Build this hardware design using the Modular Design Toolkit (MDT)

* Using the command line and the Modular Design Toolkit build script and the project created in the previous step,
  build the Quartus® project:

```bash
cd <project>/scripts
quartus_sh -t ./build_shell.tcl -hw_compile -sw_compile
```

<br>

The FPGA programming file is located in `\<project\>/quartus/output_files/top.sof`. Note the `-sw_compile`
generates the corresponding **`.elf`** and **`.hex`** files for any embedded NiosV/g processor in the
`cpu_subsystem` and is located in `\<project\>/software/dniosv_subsystem`

**Note:** You must generate the RBF/JIC pair to boot and program this design example into a board
using the output `top.sof` and a `u-boot-spl-dtb.hex` from the YOCTO and u-boot compilation process

<br>