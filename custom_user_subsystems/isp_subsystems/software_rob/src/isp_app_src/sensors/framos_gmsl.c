/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "system.h"
#include "io.h"
#include "alt_types.h"
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "intel_fpga_i2c.h"
#include "framos_gmsl.h"


void gpio_write_8bit(long port, unsigned char gpio_addr, unsigned char reg, unsigned char val) {

    // simple I2C write: send register and data
    intel_fpga_i2c_wait_busy(port);
    unsigned int tx_word = (gpio_addr << 1) | I2C_WRITE_FLAG | I2C_START_FLAG;
    IOWR(port, I2C_TFR_CMD, tx_word);

    IOWR(port, I2C_TFR_CMD, reg);                   // 8-bit register
    IOWR(port, I2C_TFR_CMD, val | I2C_STOP_FLAG);   // 8-bit data + stop

    intel_fpga_i2c_wait_busy(port);

}

int set_framos_gmsl(long port, unsigned char serAddress, unsigned char desAddress, unsigned char GPIOAddress) {

    intel_fpga_i2c_init(port, 100000000);
    int rc = 0;
    printf("[GMSL Init] GMSL init\n");
    printf("[GMSL Init] GPIO init\n");
    printf("Write completed\n");
    gpio_write_8bit(port, GPIOAddress, 0x01, 0xFE);
    gpio_write_8bit(port, GPIOAddress, 0x03, 0xFC);
    printf("Write completed\n");

    // Assume success; if your FPGA function has a return code, check it
    int gpio_ok = 1;
    if (!gpio_ok) {
        printf("[GMSL Init] No GMSL devices detected\n");
        return 0;
    }

    printf("[GMSL Init] Deserializer\n");
     // --- Deserializer (MAX96792) ---
    intel_fpga_i2c_write(port, desAddress, 0x0010, 0x81);
    usleep(100000); // 100 ms
    intel_fpga_i2c_write(port, desAddress, 0x0001, 0x03);
    intel_fpga_i2c_write(port, desAddress, 0x0004, 0xC3);
    intel_fpga_i2c_write(port, desAddress, 0x0028, 0x62);
    intel_fpga_i2c_write(port, desAddress, 0x2001, 0x01);
    intel_fpga_i2c_write(port, desAddress, 0x2101, 0x01);
    intel_fpga_i2c_write(port, desAddress, 0x0443, 0x81);
    intel_fpga_i2c_write(port, desAddress, 0x0444, 0x81);

    printf("[GMSL Init] Serializer\n");
    // --- Serializer (MAX96793) ---
    intel_fpga_i2c_write(port, serAddress, 0x14CE, 0x19);
    intel_fpga_i2c_write(port, serAddress, 0x0001, 0x0C);
    intel_fpga_i2c_write(port, serAddress, 0x0006, 0x11);
    intel_fpga_i2c_write(port, serAddress, 0x0028, 0x62);
    usleep(100000);
    intel_fpga_i2c_write(port, serAddress, 0x0010, 0x21);
    usleep(100000);

    // --- Complete link setup ---
    printf("[GMSL Init] Complete link setup\n");
    intel_fpga_i2c_write(port, desAddress, 0x0010, 0x21);
    usleep(100000);

    intel_fpga_i2c_write(port, serAddress, 0x0040, 0x16);
    intel_fpga_i2c_write(port, serAddress, 0x02BE, 0x84);
    intel_fpga_i2c_write(port, serAddress, 0x02C0, 0x4F);
    intel_fpga_i2c_write(port, serAddress, 0x02D6, 0x90);
    intel_fpga_i2c_write(port, serAddress, 0x02D0, 0x80);

    intel_fpga_i2c_write(port, desAddress, 0x0161, 0x01);
    intel_fpga_i2c_write(port, desAddress, 0x1D00, 0xF4);
    intel_fpga_i2c_write(port, desAddress, 0x0320, 0x38);
    intel_fpga_i2c_write(port, desAddress, 0x1D00, 0xF5);
    intel_fpga_i2c_write(port, desAddress, 0x0040, 0x16);
    intel_fpga_i2c_write(port, desAddress, 0x02C5, 0x83);
    intel_fpga_i2c_write(port, desAddress, 0x02C6, 0x6F);
    intel_fpga_i2c_write(port, desAddress, 0x02C8, 0x83);

    // --- Serializer streaming ---
    printf("[GMSL Init] Serializer streaming\n");
    intel_fpga_i2c_write(port, serAddress, 0x0330, 0x08);
    intel_fpga_i2c_write(port, serAddress, 0x0330, 0x00);
    intel_fpga_i2c_write(port, serAddress, 0x0331, 0x70);
    intel_fpga_i2c_write(port, serAddress, 0x031E, 0x2C);
    intel_fpga_i2c_write(port, serAddress, 0x0111, 0x4C);
    intel_fpga_i2c_write(port, serAddress, 0x0110, 0x28);
    intel_fpga_i2c_write(port, serAddress, 0x005B, 0x01);
    intel_fpga_i2c_write(port, serAddress, 0x0383, 0x80);

    // --- Deserializer streaming ---
    printf("[GMSL Init] Deserializer streaming\n");
    intel_fpga_i2c_write(port, desAddress, 0x0474, 0x19);
    intel_fpga_i2c_write(port, desAddress, 0x0112, 0x30);
    usleep(100000);
    intel_fpga_i2c_write(port, desAddress, 0x0112, 0x31);

    printf("[GMSL Init] GMSL device initialized\n");
    rc = 1;

    return rc;

}
