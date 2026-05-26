/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <io.h>
#include <system.h>
#include <inttypes.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <sys/times.h>
#include <sys/alt_stdio.h>
#include "sys/alt_timestamp.h"
#include "alt_types.h"
#include "sys/alt_irq.h"
#include "debug.h"
#include "board.h"
#include "sensor_imx678.h"
#include "framos_gmsl.h"
#include "vvp_cores_cfg.h"
#include "dp_support.h"
#include "cfg.h"


#if USE_GMSL_MODULE
#define     SER_ADDRESS     0x42
#define     DES_ADDRESS     0x6A
#define     GPIO_ADDRESS    0x21
#define     CAM_ADDRESS     0x36
#else
#define     CAM_ADDRESS     0x37
#endif

int main() {
    // ------------ Enable non-blocking jtag uart -------------
    // --------------------------------------------------------
    int res = 0;
    res = fcntl(STDOUT_FILENO, F_SETFL, O_NONBLOCK);
    res = fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);

    if(res == -1) {
        printf("FCNTL Failed\n");
    }

    printf("Started...\n");

    // ------------------ Board configuration -----------------
    // --------------------------------------------------------
    printf("DP Tx was configured for on-board connectors\n ");
    board_configure();                                              // Perform Devkit Specific initializations
    printf("board_configure OK\n");
    printf("Waiting for calibration to complete...\n");
    usleep(15000000);                                               // just accounting for the EMIF calibration

#if USE_SENSOR
    // -------------- GMSL module configuration --------------
    // -------------------------------------------------------
    #if USE_GMSL_MODULE
    set_framos_gmsl(MIPI_IN_SUBSYSTEM_CAM_I2C_BASE, SER_ADDRESS, DES_ADDRESS, GPIO_ADDRESS);
    #endif
    printf("Continue\n ");

    // -------------- Check and setup the Sensor ---------------
    // ---------------------------------------------------------
    printf("Is Sensor on Stanby?... %x\n", read_sensor_imx678(MIPI_IN_SUBSYSTEM_CAM_I2C_BASE, CAM_ADDRESS, 0x3000));
    int retval = set_sensor_imx678(MIPI_IN_SUBSYSTEM_CAM_I2C_BASE, CAM_ADDRESS);
    if (retval == 0) {
        printf("Initial sensor IMX678 setup has failed, exiting\n");
        return 0;
    } else {
        printf("Initial sensor IMX678 setup configuration has passed\n");
    }
#endif
    // -------------- Display Port Initialization --------------
    // ---------------------------------------------------------
    init_dp_tx ();

    // -- Here we Initialize the VVP Cores (by PD subsystem) ---
    // ---------------------------------------------------------
    vvp_cores_cfg_out_subsystem();                                  // ISP out subsystem
    vvp_cores_cfg_rob_subsystem();                                  // ISP rob subsystem
    vvp_cores_cfg_isp_subsystem();                                  // ISP lite subsystem
    vvp_cores_cfg_mipi_in_subsystem();                              // MIPI in subsystem

    // -------------- Main Loop (DPTX and Menu) ----------------
    // ---------------------------------------------------------
    printf("Please press 'h' to see the menu\n");

    int last_cmd = EOF;
    while(1) {

        dp_tx_loop ();                                              // Display Port monitoring
#if !USE_SENSOR
        arUco_ctrl ();
#endif
        int cmd = alt_getchar();
        if (cmd == EOF) {
            last_cmd = EOF;  // no key pressed → ready for next
            printf("no key pressed\n");
            continue;
        }

        // Ignore CR/LF
        if (cmd == '\r' || cmd == '\n') continue;

        // Ignore repeated key if same as last processed
        if (cmd == last_cmd) continue;

        last_cmd = cmd;  // mark this key as processed

        switch (cmd) {
            case 'h': {
                printf("//================================================= \n");
                printf("Help Menu: \n");
                printf("//================================================= \n");
                printf("[s] -> Toggle between MIPIR Rx and Input TPG\n");
                printf("[r] -> ISP_Lite Camera: Default Configuration\n");
                printf("[t] -> Toggle Icon (On and Off)\n");
                printf("[1] -> BLC: RGGB Mode\n");
                printf("[2] -> BLC: Bypass Mode\n");
                printf("[3] -> WBC: RGGB Mode (From 3000K to 9000K)\n");
                printf("[4] -> WBC: Bypass Mode\n");
                printf("[5] -> Demosaic: RGGB Mode\n");
                printf("[6] -> Demosaic: Bypass Mode (Bayer Greyscale)\n");
                printf("[7] -> CCM: RGB Mode (From 3000K to 9000K)\n");
                printf("[8] -> CCM: Bypass Mode\n");
                printf("[9] -> 1D LUT: BT-709 Mode\n");
                printf("[0] -> 1D LUT: Bypass Mode\n");
                printf("[m] -> Exposure Gain: Increment\n");
                printf("[n] -> Exposure Gain: Decrement\n");
                printf("//================================================= \n");
                break;
            }
            case 's': { toggle_input_switch(); break; }
            case '1': { blc_on_rggb();         break; }
            case '2': { blc_bypass();          break; }
            case '3': { wbc_on_rggb();         break; }
            case '4': { wbc_bypass();          break; }
            case '5': { demosaic_rggb_mode();  break; }
            case '6': { demosaic_bypass();     break; }
            case '7': { ccm_rgb_mode();        break; }
            case '8': { ccm_bypass_mode();     break; }
            case '9': { bt709_1d_lut_mode();   break; }
            case '0': { bypass_1dlut();        break; }
            case 't': { toggle_vfb_icon ();    break; }
#if USE_SENSOR
            case 'm': { exp_gain_incr_sensor_imx678(MIPI_IN_SUBSYSTEM_BOARD_I2C_BASE, CAM_ADDRESS); break; }
            case 'n': { exp_gain_decr_sensor_imx678(MIPI_IN_SUBSYSTEM_BOARD_I2C_BASE, CAM_ADDRESS); break; }
            case 'r': {
                printf("ISP lite Camera Demo: Restore Default Settings \n");
                set_sensor_imx678(MIPI_IN_SUBSYSTEM_BOARD_I2C_BASE, CAM_ADDRESS);
                restore_vvp_core_default ();
                break;
            }
#endif
            default : {break;}
        }
    } // while
    return 0;

}
