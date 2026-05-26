/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

// Include cores present in the system
#include "intel_vvp_tpg.h"
#include "intel_vvp_switch.h"
#include "intel_vvp_blc.h"
#include "intel_vvp_wbc.h"
#include "intel_vvp_demosaic.h"
#include "intel_vvp_csc.h"
#include "intel_vvp_vfb.h"
#include "intel_vvp_vfw.h"
#ifdef ISP_ROB_SUBSYSTEM_ISP_ROB_VFR_LRES_BASE
#include "intel_vvp_vfr.h"
#endif
#include "intel_vvp_mixer.h"
#include "intel_vvp_1d_lut.h"
#include "intel_vvp_pip_conv.h"
#include "intel_vvp_protocol_conv.h"
#include "intel_vvp_clipper.h"
#include "intel_vvp_scaler.h"

#define INIT_HSIZE                  3840
#define INIT_VSIZE                  2160
#define ICON_HSIZE                  144
#define ICON_VSIZE                  144
#define IMG_INFO_COLORSPACE_RGB     0
#define IMG_INFO_SUBSAMPLING_444    3
#define BAYER_PHASE                 0
#define TPG_BAYER_SWITCH_IN         0
#define MIPI_BAYER_SWITCH_IN        1
#define BAYER_SWITCH_OUT            0

extern intel_vvp_tpg_instance input_tpg;
extern intel_vvp_switch_instance input_switch;
extern intel_vvp_blc_instance blc;
extern intel_vvp_wbc_instance wbc;
extern intel_vvp_demosaic_instance demosaic;
extern intel_vvp_csc_instance csc;
extern intel_vvp_vfb_instance vfb;
extern intel_vvp_tpg_instance tpg_base_layer;
extern intel_vvp_mixer_instance mixer;
extern intel_vvp_1d_lut_instance vvp_1d_lut;
extern intel_vvp_pip_conv_instance pip_conv_1to2;
extern intel_vvp_protocol_conv_instance proto_lite_to_full;
extern intel_vvp_pip_conv_instance pip_conv_mipi;
extern intel_vvp_clipper_instance clipper_rob;
extern intel_vvp_scaler_instance scaler_rob;
#ifdef ISP_ROB_SUBSYSTEM_ISP_ROB_MIXER_BASE
extern intel_vvp_mixer_instance mixer_rob;
#endif
extern intel_vvp_vfw_instance vfw;
#ifdef ISP_ROB_SUBSYSTEM_ISP_ROB_VFR_LRES_BASE
extern intel_vvp_vfr_instance vfr;
#endif
extern intel_vvp_vfb_instance vfb_rob;

extern unsigned int int_ccm_cnt;
extern int icon_tgl;
extern unsigned int int_tpg_cnt;
extern unsigned int int_wbc_cnt;

void printf_csc_coeffs(intel_vvp_coefficients intp_ccm_coeffs);

// Functions to initialize VVP cores in the subsystems
void vvp_cores_cfg_out_subsystem ();
void vvp_cores_cfg_rob_subsystem ();
void vvp_cores_cfg_isp_subsystem ();
void vvp_cores_cfg_mipi_in_subsystem();
void arUco_ctrl ();

// Functions for the Menu
void ccm_rgb_mode ();
void ccm_bypass_mode ();
void restore_vvp_core_default ();
void toggle_input_switch ();
void blc_on_rggb();
void blc_bypass();
void wbc_on_rggb();
void wbc_bypass();
void demosaic_rggb_mode();
void demosaic_bypass();
void bt709_1d_lut_mode();
void bypass_1dlut();
void toggle_vfb_icon();
