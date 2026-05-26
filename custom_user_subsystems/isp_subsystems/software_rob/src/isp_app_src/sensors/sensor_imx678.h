/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.

SPDX-License-Identifier: GPL-2.0-only
*******************************************************************************/

#ifndef __SENSOR_IMX678_H__
#define __SENSOR_IMX678_H__

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

// bool probe_sensor_imx678(long port);
int set_sensor_imx678(long port, unsigned char address);
unsigned char read_sensor_imx678(long port, unsigned char address, unsigned int reg);
int exp_gain_incr_sensor_imx678(long port, unsigned char address);
int exp_gain_decr_sensor_imx678(long port, unsigned char address);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SENSOR_IMX678_H__ */
