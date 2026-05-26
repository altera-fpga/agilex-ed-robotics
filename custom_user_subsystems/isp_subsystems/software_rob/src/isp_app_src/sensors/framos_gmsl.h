/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

//Functions to setup the GMSL module
int set_framos_gmsl(long port, unsigned char serAddress, unsigned char desAddress, unsigned char GPIOAddress);
unsigned char read_framos_gmsl(long port, unsigned char address, unsigned int reg);
void gpio_write_8bit(long port, unsigned char gpio_addr, unsigned char reg, unsigned char val);
