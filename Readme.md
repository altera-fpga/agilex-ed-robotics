# Altera® Robotics - Hardware Repository (**HW**)

## Overview

This repository contains a collection of robotics oriented designs based on modular design
methodology. Different variants are targeted for various industrial and robotics use
cases. The project variants are described using XML files that add subsystems to the
designs.

## Available Variants

* **HPS_ISP_CAM_ROBOTICS**: Robotics Example Design with FRAMOS IMX678 sensor
  ingestion, basic ISP(Image Signal Processing) pipeline and display port controller.
  Suitable to integrate with higher-level software applications like ROS2 for
  robotics manipulation.
  See [HPS_ISP_CAM_ROBOTICS](./HPS_ISP_CAM_ROBOTICS) to create and build.
* **HPS_ISP_VIS_DOC3x2_ROBOTICS**: Robotics Vision pipeline implemented in FPGA with
  6-axis FOC. AruCO marker inyection in the video pipeline to mimic robot
  manipulation applications.
  See [HPS_ISP_VIS_DOC3x2_ROBOTICS](./HPS_ISP_VIS_DOC3x2_ROBOTICS) to create and build.

## Supporting Repositories

* **modular_design_toolkit**: This is an instance of the "Modular Design Toolkit" (MDT)
  repository to create and build Quartus® Prime and Platform Designer projects. You must use
  `--recurse-submodules` when cloning this repository to populate the MDT sources.
  See [modular_design_toolkit](https://github.com/intel-innersource/applications.fpga.reference-designs.projects.modular-design-toolkit)
  to know more.

MDT is currently available in Linux.
