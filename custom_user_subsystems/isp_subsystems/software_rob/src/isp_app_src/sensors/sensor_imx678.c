/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.

SPDX-License-Identifier: GPL-2.0-only
*******************************************************************************/

#include "system.h"
#include "io.h"
#include "alt_types.h"
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "intel_fpga_i2c.h"
#include "sensor_imx678.h"

#define STANDBY     0x3000
#define LANEMODE    0x3040
#define INCK_SEL    0x3014
#define XMSTA       0x3002
#define WINMODE     0x3018

const uint16_t IMX678_TABLE_WAIT_MS  = 0;
const uint16_t IMX678_WAIT_MS        = 10;
const uint16_t IMX678_TABLE_END      = 1;


unsigned char read_sensor_imx678(long port, unsigned char address, unsigned int reg) {

    unsigned char ret_rd = 0xff;
    // SLAMODE0 | SLAMODE1 | I2C Address
    // 0       |   0      | 0x1A (Sensor # 1 on MDK)
    // 1       |   0      | 0x10
    // 0       |   1      | 0x36
    // 1       |   1      | 0x37 (Sensor # 0 on MDK)

    intel_fpga_i2c_init(port, 100000000);
    ret_rd = intel_fpga_i2c_read(port, address, reg);
    return ret_rd;

}

int set_sensor_imx678(long port, unsigned char address) {

    int ret = 0;
    // SLAMODE0 | SLAMODE1 | I2C Address
    // 0       |   0      | 0x1A (Sensor # 1 on MDK)
    // 1       |   0      | 0x10
    // 0       |   1      | 0x36
    // 1       |   1      | 0x37 (Sensor # 0 on MDK)

    intel_fpga_i2c_init(port, 100000000);

    // Stop Sequence
    intel_fpga_i2c_write(port, address, XMSTA,                0x01);
    intel_fpga_i2c_write(port, address, IMX678_TABLE_WAIT_MS, 30);
    intel_fpga_i2c_write(port, address, STANDBY,              0x01);
    intel_fpga_i2c_write(port, address, IMX678_TABLE_WAIT_MS, IMX678_WAIT_MS);
    intel_fpga_i2c_write(port, address, IMX678_TABLE_END,     0x00);

    // imx678_init_settings Sequence
    intel_fpga_i2c_write(port, address, LANEMODE,  0x03);
    intel_fpga_i2c_write(port, address, INCK_SEL,  0x01);

    // Master mode
    intel_fpga_i2c_write(port, address, XMSTA, 	0x00);
    intel_fpga_i2c_write(port, address, 0x3001,	0x00);
    intel_fpga_i2c_write(port, address, 0x3002,	0x00);
    intel_fpga_i2c_write(port, address, 0x3014,	0x01);
    intel_fpga_i2c_write(port, address, 0x3015,	0x02);
    intel_fpga_i2c_write(port, address, 0x3018,	0x04);
    intel_fpga_i2c_write(port, address, 0x3019,	0x00);
    intel_fpga_i2c_write(port, address, 0x301A,	0x00);
    intel_fpga_i2c_write(port, address, 0x301B,	0x00);
    intel_fpga_i2c_write(port, address, 0x301C,	0x00);
    intel_fpga_i2c_write(port, address, 0x301E,	0x01);
    intel_fpga_i2c_write(port, address, 0x3020,	0x00);
    intel_fpga_i2c_write(port, address, 0x3021,	0x00);
    intel_fpga_i2c_write(port, address, 0x3022,	0x01);
    intel_fpga_i2c_write(port, address, 0x3023,	0x01);
    intel_fpga_i2c_write(port, address, 0x3028,	0xCA);
    intel_fpga_i2c_write(port, address, 0x3029,	0x08);
    intel_fpga_i2c_write(port, address, 0x302A,	0x00);
    intel_fpga_i2c_write(port, address, 0x302C,	0x28);
    intel_fpga_i2c_write(port, address, 0x302D,	0x05);
    intel_fpga_i2c_write(port, address, 0x3030,	0x00);
    intel_fpga_i2c_write(port, address, 0x3031,	0x00);
    intel_fpga_i2c_write(port, address, 0x3032,	0x00);
    intel_fpga_i2c_write(port, address, 0x303C,	0x00);
    intel_fpga_i2c_write(port, address, 0x303D,	0x00);
    intel_fpga_i2c_write(port, address, 0x303E,	0x00);
    intel_fpga_i2c_write(port, address, 0x303F,	0x0F);
    intel_fpga_i2c_write(port, address, 0x3040,	0x03);
    intel_fpga_i2c_write(port, address, 0x3042,	0x00);
    intel_fpga_i2c_write(port, address, 0x3043,	0x00);
    intel_fpga_i2c_write(port, address, 0x3044,	0x00);
    intel_fpga_i2c_write(port, address, 0x3045,	0x00);
    intel_fpga_i2c_write(port, address, 0x3046,	0x70);
    intel_fpga_i2c_write(port, address, 0x3047,	0x08);
    intel_fpga_i2c_write(port, address, 0x3050,	0xE8);
    intel_fpga_i2c_write(port, address, 0x3051,	0x03);
    intel_fpga_i2c_write(port, address, 0x3052,	0x00);
    intel_fpga_i2c_write(port, address, 0x3054,	0x0E);
    intel_fpga_i2c_write(port, address, 0x3055,	0x00);
    intel_fpga_i2c_write(port, address, 0x3056,	0x00);
    intel_fpga_i2c_write(port, address, 0x3058,	0x8A);
    intel_fpga_i2c_write(port, address, 0x3059,	0x01);
    intel_fpga_i2c_write(port, address, 0x305A,	0x00);
    intel_fpga_i2c_write(port, address, 0x3060,	0x16);
    intel_fpga_i2c_write(port, address, 0x3061,	0x01);
    intel_fpga_i2c_write(port, address, 0x3062,	0x00);
    intel_fpga_i2c_write(port, address, 0x3064,	0xC4);
    intel_fpga_i2c_write(port, address, 0x3065,	0x0C);
    intel_fpga_i2c_write(port, address, 0x3066,	0x00);
    intel_fpga_i2c_write(port, address, 0x3069,	0x00);
    intel_fpga_i2c_write(port, address, 0x306B,	0x00);
    intel_fpga_i2c_write(port, address, 0x3070,	0x00);
    intel_fpga_i2c_write(port, address, 0x3071,	0x00);
    intel_fpga_i2c_write(port, address, 0x3072,	0x00);
    intel_fpga_i2c_write(port, address, 0x3073,	0x00);
    intel_fpga_i2c_write(port, address, 0x3074,	0x00);
    intel_fpga_i2c_write(port, address, 0x3075,	0x00);
    intel_fpga_i2c_write(port, address, 0x3081,	0x00);
    intel_fpga_i2c_write(port, address, 0x308C,	0x00);
    intel_fpga_i2c_write(port, address, 0x308D,	0x01);
    intel_fpga_i2c_write(port, address, 0x3094,	0x00);
    intel_fpga_i2c_write(port, address, 0x3095,	0x00);
    intel_fpga_i2c_write(port, address, 0x309C,	0x00);
    intel_fpga_i2c_write(port, address, 0x309D,	0x00);
    intel_fpga_i2c_write(port, address, 0x30A4,	0xAA);
    intel_fpga_i2c_write(port, address, 0x30A6,	0x00);
    intel_fpga_i2c_write(port, address, 0x30CC,	0x00);
    intel_fpga_i2c_write(port, address, 0x30CD,	0x00);
    intel_fpga_i2c_write(port, address, 0x30DC,	0x32);
    intel_fpga_i2c_write(port, address, 0x30DD,	0x40);
    intel_fpga_i2c_write(port, address, 0x3400,	0x01);
    intel_fpga_i2c_write(port, address, 0x3460,	0x22);
    intel_fpga_i2c_write(port, address, 0x355A,	0x64);
    intel_fpga_i2c_write(port, address, 0x3A02,	0x7A);
    intel_fpga_i2c_write(port, address, 0x3A10,	0xEC);
    intel_fpga_i2c_write(port, address, 0x3A12,	0x71);
    intel_fpga_i2c_write(port, address, 0x3A14,	0xDE);
    intel_fpga_i2c_write(port, address, 0x3A20,	0x2B);
    intel_fpga_i2c_write(port, address, 0x3A24,	0x22);
    intel_fpga_i2c_write(port, address, 0x3A25,	0x25);
    intel_fpga_i2c_write(port, address, 0x3A26,	0x2A);
    intel_fpga_i2c_write(port, address, 0x3A27,	0x2C);
    intel_fpga_i2c_write(port, address, 0x3A28,	0x39);
    intel_fpga_i2c_write(port, address, 0x3A29,	0x38);
    intel_fpga_i2c_write(port, address, 0x3A30,	0x04);
    intel_fpga_i2c_write(port, address, 0x3A31,	0x04);
    intel_fpga_i2c_write(port, address, 0x3A32,	0x03);
    intel_fpga_i2c_write(port, address, 0x3A33,	0x03);
    intel_fpga_i2c_write(port, address, 0x3A34,	0x09);
    intel_fpga_i2c_write(port, address, 0x3A35,	0x06);
    intel_fpga_i2c_write(port, address, 0x3A38,	0xCD);
    intel_fpga_i2c_write(port, address, 0x3A3A,	0x4C);
    intel_fpga_i2c_write(port, address, 0x3A3C,	0xB9);
    intel_fpga_i2c_write(port, address, 0x3A3E,	0x30);
    intel_fpga_i2c_write(port, address, 0x3A40,	0x2C);
    intel_fpga_i2c_write(port, address, 0x3A42,	0x39);
    intel_fpga_i2c_write(port, address, 0x3A4E,	0x00);
    intel_fpga_i2c_write(port, address, 0x3A52,	0x00);
    intel_fpga_i2c_write(port, address, 0x3A56,	0x00);
    intel_fpga_i2c_write(port, address, 0x3A5A,	0x00);
    intel_fpga_i2c_write(port, address, 0x3A5E,	0x00);
    intel_fpga_i2c_write(port, address, 0x3A62,	0x00);
    intel_fpga_i2c_write(port, address, 0x3A64,	0x00);
    intel_fpga_i2c_write(port, address, 0x3A6E,	0xA0);
    intel_fpga_i2c_write(port, address, 0x3A70,	0x50);
    intel_fpga_i2c_write(port, address, 0x3A8C,	0x04);
    intel_fpga_i2c_write(port, address, 0x3A8D,	0x03);
    intel_fpga_i2c_write(port, address, 0x3A8E,	0x09);
    intel_fpga_i2c_write(port, address, 0x3A90,	0x38);
    intel_fpga_i2c_write(port, address, 0x3A91,	0x42);
    intel_fpga_i2c_write(port, address, 0x3A92,	0x3C);
    intel_fpga_i2c_write(port, address, 0x3B0E,	0xF3);
    intel_fpga_i2c_write(port, address, 0x3B12,	0xE5);
    intel_fpga_i2c_write(port, address, 0x3B27,	0xC0);
    intel_fpga_i2c_write(port, address, 0x3B2E,	0xEF);
    intel_fpga_i2c_write(port, address, 0x3B30,	0x6A);
    intel_fpga_i2c_write(port, address, 0x3B32,	0xF6);
    intel_fpga_i2c_write(port, address, 0x3B36,	0xE1);
    intel_fpga_i2c_write(port, address, 0x3B3A,	0xE8);
    intel_fpga_i2c_write(port, address, 0x3B5A,	0x17);
    intel_fpga_i2c_write(port, address, 0x3B5E,	0xEF);
    intel_fpga_i2c_write(port, address, 0x3B60,	0x6A);
    intel_fpga_i2c_write(port, address, 0x3B62,	0xF6);
    intel_fpga_i2c_write(port, address, 0x3B66,	0xE1);
    intel_fpga_i2c_write(port, address, 0x3B6A,	0xE8);
    intel_fpga_i2c_write(port, address, 0x3B88,	0xEC);
    intel_fpga_i2c_write(port, address, 0x3B8A,	0xED);
    intel_fpga_i2c_write(port, address, 0x3B94,	0x71);
    intel_fpga_i2c_write(port, address, 0x3B96,	0x72);
    intel_fpga_i2c_write(port, address, 0x3B98,	0xDE);
    intel_fpga_i2c_write(port, address, 0x3B9A,	0xDF);
    intel_fpga_i2c_write(port, address, 0x3C0F,	0x06);
    intel_fpga_i2c_write(port, address, 0x3C10,	0x06);
    intel_fpga_i2c_write(port, address, 0x3C11,	0x06);
    intel_fpga_i2c_write(port, address, 0x3C12,	0x06);
    intel_fpga_i2c_write(port, address, 0x3C13,	0x06);
    intel_fpga_i2c_write(port, address, 0x3C18,	0x20);
    intel_fpga_i2c_write(port, address, 0x3C37,	0x10);
    intel_fpga_i2c_write(port, address, 0x3C3A,	0x7A);
    intel_fpga_i2c_write(port, address, 0x3C40,	0xF4);
    intel_fpga_i2c_write(port, address, 0x3C48,	0xE6);
    intel_fpga_i2c_write(port, address, 0x3C54,	0xCE);
    intel_fpga_i2c_write(port, address, 0x3C56,	0xD0);
    intel_fpga_i2c_write(port, address, 0x3C6C,	0x53);
    intel_fpga_i2c_write(port, address, 0x3C6E,	0x55);
    intel_fpga_i2c_write(port, address, 0x3C70,	0xC0);
    intel_fpga_i2c_write(port, address, 0x3C72,	0xC2);
    intel_fpga_i2c_write(port, address, 0x3C7E,	0xCE);
    intel_fpga_i2c_write(port, address, 0x3C8C,	0xCF);
    intel_fpga_i2c_write(port, address, 0x3C8E,	0xEB);
    intel_fpga_i2c_write(port, address, 0x3C98,	0x54);
    intel_fpga_i2c_write(port, address, 0x3C9A,	0x70);
    intel_fpga_i2c_write(port, address, 0x3C9C,	0xC1);
    intel_fpga_i2c_write(port, address, 0x3C9E,	0xDD);
    intel_fpga_i2c_write(port, address, 0x3CB0,	0x7A);
    intel_fpga_i2c_write(port, address, 0x3CB2,	0xBA);
    intel_fpga_i2c_write(port, address, 0x3CC8,	0xBC);
    intel_fpga_i2c_write(port, address, 0x3CCA,	0x7C);
    intel_fpga_i2c_write(port, address, 0x3CD4,	0xEA);
    intel_fpga_i2c_write(port, address, 0x3CD5,	0x01);
    intel_fpga_i2c_write(port, address, 0x3CD6,	0x4A);
    intel_fpga_i2c_write(port, address, 0x3CD8,	0x00);
    intel_fpga_i2c_write(port, address, 0x3CD9,	0x00);
    intel_fpga_i2c_write(port, address, 0x3CDA,	0xFF);
    intel_fpga_i2c_write(port, address, 0x3CDB,	0x03);
    intel_fpga_i2c_write(port, address, 0x3CDC,	0x00);
    intel_fpga_i2c_write(port, address, 0x3CDD,	0x00);
    intel_fpga_i2c_write(port, address, 0x3CDE,	0xFF);
    intel_fpga_i2c_write(port, address, 0x3CDF,	0x03);
    intel_fpga_i2c_write(port, address, 0x3CE4,	0x4C);
    intel_fpga_i2c_write(port, address, 0x3CE6,	0xEC);
    intel_fpga_i2c_write(port, address, 0x3CE7,	0x01);
    intel_fpga_i2c_write(port, address, 0x3CE8,	0xFF);
    intel_fpga_i2c_write(port, address, 0x3CE9,	0x03);
    intel_fpga_i2c_write(port, address, 0x3CEA,	0x00);
    intel_fpga_i2c_write(port, address, 0x3CEB,	0x00);
    intel_fpga_i2c_write(port, address, 0x3CEC,	0xFF);
    intel_fpga_i2c_write(port, address, 0x3CED,	0x03);
    intel_fpga_i2c_write(port, address, 0x3CEE,	0x00);
    intel_fpga_i2c_write(port, address, 0x3CEF,	0x00);
    intel_fpga_i2c_write(port, address, 0x3CF2, 0xFF);
    intel_fpga_i2c_write(port, address, 0x3CF3,	0x03);
    intel_fpga_i2c_write(port, address, 0x3CF4,	0x00);
    intel_fpga_i2c_write(port, address, 0x3E28,	0x82);
    intel_fpga_i2c_write(port, address, 0x3E2A,	0x80);
    intel_fpga_i2c_write(port, address, 0x3E30,	0x85);
    intel_fpga_i2c_write(port, address, 0x3E32,	0x7D);
    intel_fpga_i2c_write(port, address, 0x3E5C,	0xCE);
    intel_fpga_i2c_write(port, address, 0x3E5E,	0xD3);
    intel_fpga_i2c_write(port, address, 0x3E70,	0x53);
    intel_fpga_i2c_write(port, address, 0x3E72,	0x58);
    intel_fpga_i2c_write(port, address, 0x3E74,	0xC0);
    intel_fpga_i2c_write(port, address, 0x3E76,	0xC5);
    intel_fpga_i2c_write(port, address, 0x3E78,	0xC0);
    intel_fpga_i2c_write(port, address, 0x3E79,	0x01);
    intel_fpga_i2c_write(port, address, 0x3E7A,	0xD4);
    intel_fpga_i2c_write(port, address, 0x3E7B,	0x01);
    intel_fpga_i2c_write(port, address, 0x3EB4,	0x0B);
    intel_fpga_i2c_write(port, address, 0x3EB5,	0x02);
    intel_fpga_i2c_write(port, address, 0x3EB6,	0x4D);
    intel_fpga_i2c_write(port, address, 0x3EB7, 0x42);
    intel_fpga_i2c_write(port, address, 0x3EEC,	0xF3);
    intel_fpga_i2c_write(port, address, 0x3EEE,	0xE7);
    intel_fpga_i2c_write(port, address, 0x3F01,	0x01);
    intel_fpga_i2c_write(port, address, 0x3F24,	0x10);
    intel_fpga_i2c_write(port, address, 0x3F28,	0x2D);
    intel_fpga_i2c_write(port, address, 0x3F2A,	0x2D);
    intel_fpga_i2c_write(port, address, 0x3F2C,	0x2D);
    intel_fpga_i2c_write(port, address, 0x3F2E,	0x2D);
    intel_fpga_i2c_write(port, address, 0x3F30,	0x23);
    intel_fpga_i2c_write(port, address, 0x3F38,	0x2D);
    intel_fpga_i2c_write(port, address, 0x3F3A,	0x2D);
    intel_fpga_i2c_write(port, address, 0x3F3C,	0x2D);
    intel_fpga_i2c_write(port, address, 0x3F3E,	0x28);
    intel_fpga_i2c_write(port, address, 0x3F40,	0x1E);
    intel_fpga_i2c_write(port, address, 0x3F48,	0x2D);
    intel_fpga_i2c_write(port, address, 0x3F4A,	0x2D);
    intel_fpga_i2c_write(port, address, 0x3F4C, 0x00);
    intel_fpga_i2c_write(port, address, 0x4004,	0xE4);
    intel_fpga_i2c_write(port, address, 0x4006,	0xFF);
    intel_fpga_i2c_write(port, address, 0x4018,	0x69);
    intel_fpga_i2c_write(port, address, 0x401A,	0x84);
    intel_fpga_i2c_write(port, address, 0x401C,	0xD6);
    intel_fpga_i2c_write(port, address, 0x401E,	0xF1);
    intel_fpga_i2c_write(port, address, 0x4038,	0xDE);
    intel_fpga_i2c_write(port, address, 0x403A,	0x00);
    intel_fpga_i2c_write(port, address, 0x403B,	0x01);
    intel_fpga_i2c_write(port, address, 0x404C,	0x63);
    intel_fpga_i2c_write(port, address, 0x404E,	0x85);
    intel_fpga_i2c_write(port, address, 0x4050,	0xD0);
    intel_fpga_i2c_write(port, address, 0x4052,	0xF2);
    intel_fpga_i2c_write(port, address, 0x4108,	0xDD);
    intel_fpga_i2c_write(port, address, 0x410A,	0xF7);
    intel_fpga_i2c_write(port, address, 0x411C,	0x62);
    intel_fpga_i2c_write(port, address, 0x411E,	0x7C);
    intel_fpga_i2c_write(port, address, 0x4120,	0xCF);
    intel_fpga_i2c_write(port, address, 0x4122,	0xE9);
    intel_fpga_i2c_write(port, address, 0x4138,	0xE6);
    intel_fpga_i2c_write(port, address, 0x413A,	0xF1);
    intel_fpga_i2c_write(port, address, 0x414C,	0x6B);
    intel_fpga_i2c_write(port, address, 0x414E,	0x76);
    intel_fpga_i2c_write(port, address, 0x4150,	0xD8);
    intel_fpga_i2c_write(port, address, 0x4152,	0xE3);
    intel_fpga_i2c_write(port, address, 0x417E,	0x03);
    intel_fpga_i2c_write(port, address, 0x417F,	0x01);
    intel_fpga_i2c_write(port, address, 0x4186,	0xE0);
    intel_fpga_i2c_write(port, address, 0x4190,	0xF3);
    intel_fpga_i2c_write(port, address, 0x4192,	0xF7);
    intel_fpga_i2c_write(port, address, 0x419C,	0x78);
    intel_fpga_i2c_write(port, address, 0x419E,	0x7C);
    intel_fpga_i2c_write(port, address, 0x41A0,	0xE5);
    intel_fpga_i2c_write(port, address, 0x41A2,	0xE9);
    intel_fpga_i2c_write(port, address, 0x41C8,	0xE2);
    intel_fpga_i2c_write(port, address, 0x41CA,	0xFD);
    intel_fpga_i2c_write(port, address, 0x41DC,	0x67);
    intel_fpga_i2c_write(port, address, 0x41DE,	0x82);
    intel_fpga_i2c_write(port, address, 0x41E0,	0xD4);
    intel_fpga_i2c_write(port, address, 0x41E2,	0xEF);
    intel_fpga_i2c_write(port, address, 0x4200,	0xDE);
    intel_fpga_i2c_write(port, address, 0x4202,	0xDA);
    intel_fpga_i2c_write(port, address, 0x4218,	0x63);
    intel_fpga_i2c_write(port, address, 0x421A,	0x5F);
    intel_fpga_i2c_write(port, address, 0x421C,	0xD0);
    intel_fpga_i2c_write(port, address, 0x421E,	0xCC);
    intel_fpga_i2c_write(port, address, 0x425A,	0x82);
    intel_fpga_i2c_write(port, address, 0x425C,	0xEF);
    intel_fpga_i2c_write(port, address, 0x4348,	0xFE);
    intel_fpga_i2c_write(port, address, 0x4349,	0x06);
    intel_fpga_i2c_write(port, address, 0x4352,	0xCE);
    intel_fpga_i2c_write(port, address, 0x4420,	0x0B);
    intel_fpga_i2c_write(port, address, 0x4421,	0x02);
    intel_fpga_i2c_write(port, address, 0x4422,	0x4D);
    intel_fpga_i2c_write(port, address, 0x4423, 0x0A);
    intel_fpga_i2c_write(port, address, 0x4426,	0xF5);
    intel_fpga_i2c_write(port, address, 0x442A,	0xE7);
    intel_fpga_i2c_write(port, address, 0x4432,	0xF5);
    intel_fpga_i2c_write(port, address, 0x4436,	0xE7);
    intel_fpga_i2c_write(port, address, 0x4466,	0xB4);
    intel_fpga_i2c_write(port, address, 0x446E,	0x32);
    intel_fpga_i2c_write(port, address, 0x449F,	0x1C);
    intel_fpga_i2c_write(port, address, 0x44A4,	0x2C);
    intel_fpga_i2c_write(port, address, 0x44A6,	0x2C);
    intel_fpga_i2c_write(port, address, 0x44A8,	0x2C);
    intel_fpga_i2c_write(port, address, 0x44AA,	0x2C);
    intel_fpga_i2c_write(port, address, 0x44B4,	0x2C);
    intel_fpga_i2c_write(port, address, 0x44B6,	0x2C);
    intel_fpga_i2c_write(port, address, 0x44B8,	0x2C);
    intel_fpga_i2c_write(port, address, 0x44BA,	0x2C);
    intel_fpga_i2c_write(port, address, 0x44C4,	0x2C);
    intel_fpga_i2c_write(port, address, 0x44C6,	0x2C);
    intel_fpga_i2c_write(port, address, 0x44C8,	0x2C);
    intel_fpga_i2c_write(port, address, 0x4506,	0xF3);
    intel_fpga_i2c_write(port, address, 0x450E,	0xE5);
    intel_fpga_i2c_write(port, address, 0x4516,	0xF3);
    intel_fpga_i2c_write(port, address, 0x4522,	0xE5);
    intel_fpga_i2c_write(port, address, 0x4524,	0xF3);
    intel_fpga_i2c_write(port, address, 0x452C,	0xE5);
    intel_fpga_i2c_write(port, address, 0x453C,	0x22);
    intel_fpga_i2c_write(port, address, 0x453D,	0x1B);
    intel_fpga_i2c_write(port, address, 0x453E,	0x1B);
    intel_fpga_i2c_write(port, address, 0x453F,	0x15);
    intel_fpga_i2c_write(port, address, 0x4540,	0x15);
    intel_fpga_i2c_write(port, address, 0x4541,	0x15);
    intel_fpga_i2c_write(port, address, 0x4542,	0x15);
    intel_fpga_i2c_write(port, address, 0x4543,	0x15);
    intel_fpga_i2c_write(port, address, 0x4544,	0x15);
    intel_fpga_i2c_write(port, address, 0x4548,	0x00);
    intel_fpga_i2c_write(port, address, 0x4549,	0x01);
    intel_fpga_i2c_write(port, address, 0x454A,	0x01);
    intel_fpga_i2c_write(port, address, 0x454B,	0x06);
    intel_fpga_i2c_write(port, address, 0x454C,	0x06);
    intel_fpga_i2c_write(port, address, 0x454D,	0x06);
    intel_fpga_i2c_write(port, address, 0x454E,	0x06);
    intel_fpga_i2c_write(port, address, 0x454F,	0x06);
    intel_fpga_i2c_write(port, address, 0x4550,	0x06);
    intel_fpga_i2c_write(port, address, 0x4554,	0x55);
    intel_fpga_i2c_write(port, address, 0x4555,	0x02);
    intel_fpga_i2c_write(port, address, 0x4556,	0x42);
    intel_fpga_i2c_write(port, address, 0x4557,	0x05);
    intel_fpga_i2c_write(port, address, 0x4558,	0xFD);
    intel_fpga_i2c_write(port, address, 0x4559,	0x05);
    intel_fpga_i2c_write(port, address, 0x455A,	0x94);
    intel_fpga_i2c_write(port, address, 0x455B,	0x06);
    intel_fpga_i2c_write(port, address, 0x455D,	0x06);
    intel_fpga_i2c_write(port, address, 0x455E,	0x49);
    intel_fpga_i2c_write(port, address, 0x455F,	0x07);
    intel_fpga_i2c_write(port, address, 0x4560,	0x7F);
    intel_fpga_i2c_write(port, address, 0x4561,	0x07);
    intel_fpga_i2c_write(port, address, 0x4562,	0xA5);
    intel_fpga_i2c_write(port, address, 0x4564,	0x55);
    intel_fpga_i2c_write(port, address, 0x4565,	0x02);
    intel_fpga_i2c_write(port, address, 0x4566,	0x42);
    intel_fpga_i2c_write(port, address, 0x4567,	0x05);
    intel_fpga_i2c_write(port, address, 0x4568,	0xFD);
    intel_fpga_i2c_write(port, address, 0x4569,	0x05);
    intel_fpga_i2c_write(port, address, 0x456A,	0x94);
    intel_fpga_i2c_write(port, address, 0x456B,	0x06);
    intel_fpga_i2c_write(port, address, 0x456D,	0x06);
    intel_fpga_i2c_write(port, address, 0x456E,	0x49);
    intel_fpga_i2c_write(port, address, 0x456F,	0x07);
    intel_fpga_i2c_write(port, address, 0x4572,	0xA5);
    intel_fpga_i2c_write(port, address, 0x460C,	0x7D);
    intel_fpga_i2c_write(port, address, 0x460E,	0xB1);
    intel_fpga_i2c_write(port, address, 0x4614,	0xA8);
    intel_fpga_i2c_write(port, address, 0x4616,	0xB2);
    intel_fpga_i2c_write(port, address, 0x461C,	0x7E);
    intel_fpga_i2c_write(port, address, 0x461E,	0xA7);
    intel_fpga_i2c_write(port, address, 0x4624,	0xA8);
    intel_fpga_i2c_write(port, address, 0x4626,	0xB2);
    intel_fpga_i2c_write(port, address, 0x462C,	0x7E);
    intel_fpga_i2c_write(port, address, 0x462E,	0x8A);
    intel_fpga_i2c_write(port, address, 0x4630,	0x94);
    intel_fpga_i2c_write(port, address, 0x4632,	0xA7);
    intel_fpga_i2c_write(port, address, 0x4634,	0xFB);
    intel_fpga_i2c_write(port, address, 0x4636,	0x2F);
    intel_fpga_i2c_write(port, address, 0x4638,	0x81);
    intel_fpga_i2c_write(port, address, 0x4639,	0x01);
    intel_fpga_i2c_write(port, address, 0x463A,	0xB5);
    intel_fpga_i2c_write(port, address, 0x463B,	0x01);
    intel_fpga_i2c_write(port, address, 0x463C,	0x26);
    intel_fpga_i2c_write(port, address, 0x463E,	0x30);
    intel_fpga_i2c_write(port, address, 0x4640,	0xAC);
    intel_fpga_i2c_write(port, address, 0x4641,	0x01);
    intel_fpga_i2c_write(port, address, 0x4642,	0xB6);
    intel_fpga_i2c_write(port, address, 0x4643,	0x01);
    intel_fpga_i2c_write(port, address, 0x4644,	0xFC);
    intel_fpga_i2c_write(port, address, 0x4646,	0x25);
    intel_fpga_i2c_write(port, address, 0x4648,	0x82);
    intel_fpga_i2c_write(port, address, 0x4649,	0x01);
    intel_fpga_i2c_write(port, address, 0x464A,	0xAB);
    intel_fpga_i2c_write(port, address, 0x464B,	0x01);
    intel_fpga_i2c_write(port, address, 0x464C,	0x26);
    intel_fpga_i2c_write(port, address, 0x464E,	0x30);
    intel_fpga_i2c_write(port, address, 0x4654,	0xFC);
    intel_fpga_i2c_write(port, address, 0x4656,	0x08);
    intel_fpga_i2c_write(port, address, 0x4658,	0x12);
    intel_fpga_i2c_write(port, address, 0x465A,	0x25);
    intel_fpga_i2c_write(port, address, 0x4662,	0xFC);
    intel_fpga_i2c_write(port, address, 0x46A2,	0xFB);
    intel_fpga_i2c_write(port, address, 0x46D6,	0xF3);
    intel_fpga_i2c_write(port, address, 0x46E6,	0x00);
    intel_fpga_i2c_write(port, address, 0x46E8,	0xFF);
    intel_fpga_i2c_write(port, address, 0x46E9,	0x03);
    intel_fpga_i2c_write(port, address, 0x46EC,	0x7A);
    intel_fpga_i2c_write(port, address, 0x46EE,	0xE5);
    intel_fpga_i2c_write(port, address, 0x46F4,	0xEE);
    intel_fpga_i2c_write(port, address, 0x46F6,	0xF2);
    intel_fpga_i2c_write(port, address, 0x470C,	0xFF);
    intel_fpga_i2c_write(port, address, 0x470D,	0x03);
    intel_fpga_i2c_write(port, address, 0x470E,	0x00);
    intel_fpga_i2c_write(port, address, 0x4714,	0xE0);
    intel_fpga_i2c_write(port, address, 0x4716,	0xE4);
    intel_fpga_i2c_write(port, address, 0x471E,	0xED);
    intel_fpga_i2c_write(port, address, 0x472E,	0x00);
    intel_fpga_i2c_write(port, address, 0x4730,	0xFF);
    intel_fpga_i2c_write(port, address, 0x4731,	0x03);
    intel_fpga_i2c_write(port, address, 0x4734,	0x7B);
    intel_fpga_i2c_write(port, address, 0x4736,	0xDF);
    intel_fpga_i2c_write(port, address, 0x4754,	0x7D);
    intel_fpga_i2c_write(port, address, 0x4756,	0x8B);
    intel_fpga_i2c_write(port, address, 0x4758,	0x93);
    intel_fpga_i2c_write(port, address, 0x475A,	0xB1);
    intel_fpga_i2c_write(port, address, 0x475C,	0xFB);
    intel_fpga_i2c_write(port, address, 0x475E,	0x09);
    intel_fpga_i2c_write(port, address, 0x4760,	0x11);
    intel_fpga_i2c_write(port, address, 0x4762,	0x2F);
    intel_fpga_i2c_write(port, address, 0x4766,	0xCC);
    intel_fpga_i2c_write(port, address, 0x4776,	0xCB);
    intel_fpga_i2c_write(port, address, 0x477E,	0x4A);
    intel_fpga_i2c_write(port, address, 0x478E,	0x49);
    intel_fpga_i2c_write(port, address, 0x4794,	0x7C);
    intel_fpga_i2c_write(port, address, 0x4796,	0x8F);
    intel_fpga_i2c_write(port, address, 0x4798,	0xB3);
    intel_fpga_i2c_write(port, address, 0x4799,	0x00);
    intel_fpga_i2c_write(port, address, 0x479A,	0xCC);
    intel_fpga_i2c_write(port, address, 0x479C,	0xC1);
    intel_fpga_i2c_write(port, address, 0x479E,	0xCB);
    intel_fpga_i2c_write(port, address, 0x47A4,	0x7D);
    intel_fpga_i2c_write(port, address, 0x47A6,	0x8E);
    intel_fpga_i2c_write(port, address, 0x47A8,	0xB4);
    intel_fpga_i2c_write(port, address, 0x47A9,	0x00);
    intel_fpga_i2c_write(port, address, 0x47AA,	0xC0);
    intel_fpga_i2c_write(port, address, 0x47AC,	0xFA);
    intel_fpga_i2c_write(port, address, 0x47AE,	0x0D);
    intel_fpga_i2c_write(port, address, 0x47B0,	0x31);
    intel_fpga_i2c_write(port, address, 0x47B1,	0x01);
    intel_fpga_i2c_write(port, address, 0x47B2,	0x4A);
    intel_fpga_i2c_write(port, address, 0x47B3,	0x01);
    intel_fpga_i2c_write(port, address, 0x47B4,	0x3F);
    intel_fpga_i2c_write(port, address, 0x47B6,	0x49);
    intel_fpga_i2c_write(port, address, 0x47BC,	0xFB);
    intel_fpga_i2c_write(port, address, 0x47BE,	0x0C);
    intel_fpga_i2c_write(port, address, 0x47C0,	0x32);
    intel_fpga_i2c_write(port, address, 0x47C1,	0x01);
    intel_fpga_i2c_write(port, address, 0x47C2,	0x3E);
    intel_fpga_i2c_write(port, address, 0x47C3,	0x01);
    //Enable cropping
    intel_fpga_i2c_write(port, address, WINMODE,    0x04);
    //3840 width
    intel_fpga_i2c_write(port, address, 0x303E,    0x00);
    intel_fpga_i2c_write(port, address, 0x303F,    0x0F);
    //2160 height
    intel_fpga_i2c_write(port, address, 0x3046,    0x70);
    intel_fpga_i2c_write(port, address, 0x3047,    0x08);
    intel_fpga_i2c_write(port, address, 0x4E3C,    0x07);
    intel_fpga_i2c_write(port, address, IMX678_TABLE_WAIT_MS, IMX678_WAIT_MS);
    intel_fpga_i2c_write(port, address, IMX678_TABLE_END,     0x00);

    // Set h clock to 60hz mode Sequence
    //intel_fpga_i2c_write(port, address, 0x302C, 0x26);
    //intel_fpga_i2c_write(port, address, 0x302D, 0x02);
    //intel_fpga_i2c_write(port, address, IMX678_TABLE_END, 0x00);

    // Set h clock to 60hz mode Sequence
    intel_fpga_i2c_write(port, address, 0x302C, 0x4C);
    intel_fpga_i2c_write(port, address, 0x302D, 0x04);
    intel_fpga_i2c_write(port, address, IMX678_TABLE_END, 0x0000);

    intel_fpga_i2c_write(port, address, STANDBY, 0x00);
    intel_fpga_i2c_write(port, address, STANDBY, 0x01);
    intel_fpga_i2c_write(port, address, STANDBY, 0x00);

    ret = 1;
    return ret;

}

int exp_gain_incr_sensor_imx678(long port, unsigned char address) {

    printf("//================================================= \n");
    printf("Exposure Gain: Increment \n");
    printf("//================================================= \n");

    unsigned char d_gain_p2 = read_sensor_imx678(port, address, 0x3051) & (0xff);
    unsigned char d_gain_p1 = read_sensor_imx678(port, address, 0x3050) & (0xff);
    unsigned int  d_gain_total = (d_gain_p2 << 8) | (d_gain_p1);

    unsigned char a_gain_total = read_sensor_imx678(port, address, 0x3070) & (0xff);

    // Digital gain range : 10 to 2050 => 0xA to 0x802
    printf("Old Digital Gain... %d\n", d_gain_total);
    // Digital gain range : 0 to 240
    printf("Old Analogue Gain... %d\n", a_gain_total);

    if (d_gain_total >= 10 && d_gain_total < 2050) {
        d_gain_total = d_gain_total + 10;
        d_gain_p2    = (d_gain_total & 0xf00) >> (8);
        d_gain_p1    = (d_gain_total & 0x0ff);

        intel_fpga_i2c_write(port, address, 0x3051,	d_gain_p2);
        intel_fpga_i2c_write(port, address, 0x3050,	d_gain_p1);
        printf("New Digital Gain... %d\n", d_gain_total);
        printf("New Analogue Gain... %d\n", a_gain_total);
    } else if ( (d_gain_total >= 2050) && (a_gain_total >= 0) && (a_gain_total < 240) ) {
        a_gain_total = a_gain_total + 10;

        intel_fpga_i2c_write(port, address, 0x3070, a_gain_total);
        printf("New Digital Gain... %d\n", d_gain_total);
        printf("New Analogue Gain... %d\n", a_gain_total);
    } else {
        printf("You have reached the maximum value for the Digital Gain... %d\n", d_gain_total);
        printf("You have reached the maximum value for the Analogue Gain... %d\n", a_gain_total);
    }
    printf("//================================================= \n");

    return 1;

}

int exp_gain_decr_sensor_imx678(long port, unsigned char address) {

    printf("//================================================= \n");
    printf("Exposure Gain: Decrement \n");
    printf("//================================================= \n");

    unsigned char d_gain_p2 = read_sensor_imx678(port, address, 0x3051) & (0xff);
    unsigned char d_gain_p1 = read_sensor_imx678(port, address, 0x3050) & (0xff);
    unsigned int d_gain_total = (d_gain_p2 << 8) | (d_gain_p1);

    unsigned char a_gain_total = read_sensor_imx678(port, address, 0x3070) & (0xff);

    // Digital gain range : 10 to 2050 => 0xA to 0x802
    printf("Old Digital Gain... %d\n", d_gain_total);
    // Digital gain range : 0 to 240
    printf("Old Analogue Gain... %d\n", a_gain_total);

    if ( (d_gain_total >= 10) && (a_gain_total > 0) && (a_gain_total <= 240) ) {
        a_gain_total = a_gain_total - 10;

        intel_fpga_i2c_write(port, address, 0x3070, a_gain_total);
        printf("New Digital Gain... %d\n", d_gain_total);
        printf("New Analogue Gain... %d\n", a_gain_total);
    } else if (d_gain_total > 10 && d_gain_total <= 2050) {
        d_gain_total = d_gain_total - 10;
        d_gain_p2    = (d_gain_total & 0xf00) >> (8);
        d_gain_p1    = (d_gain_total & 0x0ff);

        intel_fpga_i2c_write(port, address, 0x3051, d_gain_p2);
        intel_fpga_i2c_write(port, address, 0x3050, d_gain_p1);
        printf("New Digital Gain... %d\n", d_gain_total);
        printf("New Analogue Gain... %d\n", a_gain_total);
    } else {
        printf("You have reached the minimum value for the Digital Gain... %d\n", d_gain_total);
        printf("You have reached the minimum value for the Analogue Gain... %d\n", a_gain_total);
    }
    printf("//================================================= \n");

    return 1;

}
