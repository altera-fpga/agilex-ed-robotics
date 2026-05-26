/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "aruco_square_placement.h"
#include "vvp_cores_cfg.h"
#include "csc_coeff.h"
#include "cfg.h"

intel_vvp_tpg_instance input_tpg;
intel_vvp_switch_instance input_switch;
intel_vvp_blc_instance blc;
intel_vvp_wbc_instance wbc;
intel_vvp_demosaic_instance demosaic;
intel_vvp_csc_instance csc;
intel_vvp_vfb_instance vfb;
intel_vvp_tpg_instance tpg_base_layer;
intel_vvp_mixer_instance mixer;
intel_vvp_1d_lut_instance vvp_1d_lut;
intel_vvp_pip_conv_instance pip_conv_1to2;
intel_vvp_protocol_conv_instance proto_lite_to_full;
intel_vvp_pip_conv_instance pip_conv_mipi;
intel_vvp_clipper_instance clipper_rob;
intel_vvp_scaler_instance scaler_rob;
#ifdef ISP_ROB_SUBSYSTEM_ISP_ROB_MIXER_BASE
intel_vvp_mixer_instance mixer_rob;
#endif
intel_vvp_vfw_instance vfw;
#ifdef ISP_ROB_SUBSYSTEM_ISP_ROB_VFR_LRES_BASE
intel_vvp_vfr_instance vfr;
#endif
intel_vvp_vfb_instance vfb_rob;
intel_vvp_vfw_instance vfw_gs;

unsigned int int_ccm_cnt = 1;
int icon_tgl = 0;
unsigned int int_tpg_cnt = 0;
unsigned int int_wbc_cnt = 1;

void printf_csc_coeffs(intel_vvp_coefficients intp_ccm_coeffs) {
    for(int r = 0; r < 3; ++r)
    {
        // Set the coefficients for each channel
        printf("Colour plane (%zu): %f, %f, %f\n", r,
                intp_ccm_coeffs.coeffs[r].c1, intp_ccm_coeffs.coeffs[r].c2, intp_ccm_coeffs.coeffs[r].c3);
    }

}

void vvp_cores_cfg_out_subsystem() {
    // Protocol converter (VVP-Full)
    intel_vvp_protocol_conv_init(&proto_lite_to_full, (intel_vvp_core_base)ISP_LITE_OUT_SUBSYSTEM_ISP_OUT_PROTO_CONV_BASE);
    intel_vvp_core_set_img_info_width(&proto_lite_to_full.core_instance, INIT_HSIZE);
    intel_vvp_core_set_img_info_height(&proto_lite_to_full.core_instance, INIT_VSIZE);
    intel_vvp_core_set_img_info_interlace(&proto_lite_to_full.core_instance, 3);
    intel_vvp_core_set_img_info_colorspace(&proto_lite_to_full.core_instance, IMG_INFO_COLORSPACE_RGB);
    intel_vvp_core_set_img_info_subsampling(&proto_lite_to_full.core_instance, IMG_INFO_SUBSAMPLING_444);
    intel_vvp_core_set_img_info_cositing(&proto_lite_to_full.core_instance, 0);
    intel_vvp_protocol_conv_enable(&proto_lite_to_full, true);

    // PiP Converter (DP Tx) 1 to 2
    intel_vvp_pip_conv_init(&pip_conv_1to2, (intel_vvp_core_base)ISP_LITE_OUT_SUBSYSTEM_VID_OUT_PIP_CONV_BASE);
    intel_vvp_core_set_img_info_width(&pip_conv_1to2.core_instance, INIT_HSIZE);
    intel_vvp_core_set_img_info_height(&pip_conv_1to2.core_instance, INIT_VSIZE);
    intel_vvp_core_set_img_info_interlace(&pip_conv_1to2.core_instance, 3);
    intel_vvp_core_set_img_info_colorspace(&pip_conv_1to2.core_instance, IMG_INFO_COLORSPACE_RGB);
    intel_vvp_core_set_img_info_subsampling(&pip_conv_1to2.core_instance, IMG_INFO_SUBSAMPLING_444);
    intel_vvp_core_set_img_info_cositing(&pip_conv_1to2.core_instance, 0);

    // 1D LUT
    intel_vvp_1d_lut_init(&vvp_1d_lut, (intel_vvp_core_base)ISP_LITE_OUT_SUBSYSTEM_ISP_1D_LUT_BASE);
    intel_vvp_core_set_img_info_width(&vvp_1d_lut.core_instance, INIT_HSIZE);
    intel_vvp_core_set_img_info_height(&vvp_1d_lut.core_instance, INIT_VSIZE);
    printf("1D LUT write array status... %d\n", intel_vvp_1d_lut_write_data_array(&vvp_1d_lut, gamma_oetf_bt709_1dlut, 1536));
    intel_vvp_1d_lut_set_bypass(&vvp_1d_lut, true);

    // Mixer
    intel_vvp_mixer_init(&mixer, (intel_vvp_core_base)ISP_LITE_OUT_SUBSYSTEM_ISP_MIXER_BASE);
    intel_vvp_core_set_img_info_width(&mixer.core_instance, INIT_HSIZE);
    intel_vvp_core_set_img_info_height(&mixer.core_instance, INIT_VSIZE);
    intel_vvp_core_set_img_info_subsampling(&mixer.core_instance, IMG_INFO_SUBSAMPLING_444);
    // Mixer Input Layer # 1 : input video from VfB
    intel_vvp_mixer_set_blend_mode(&mixer, 1, kIntelVvpMixerBlendOpaque);
    intel_vvp_mixer_set_horiz_offset(&mixer, 1, 0);
    intel_vvp_mixer_set_vert_offset(&mixer, 1, 0);
    intel_vvp_mixer_set_width(&mixer, 1, INIT_HSIZE);
    intel_vvp_mixer_set_height(&mixer, 1, INIT_VSIZE);
    // Mixer Input Layer # 3 : input video from Icon
    intel_vvp_mixer_set_blend_mode(&mixer, 2, kIntelVvpMixerBlendOpaque);
    intel_vvp_mixer_set_horiz_offset(&mixer, 2, 0);
    intel_vvp_mixer_set_vert_offset(&mixer, 2, 0);
    intel_vvp_mixer_set_width(&mixer, 2, ICON_HSIZE);
    intel_vvp_mixer_set_height(&mixer, 2, ICON_VSIZE);
    // Mixer Input Layer #3 : frame buffer output
    intel_vvp_mixer_set_blend_mode(&mixer, 3, kIntelVvpMixerBlendOpaque);
    intel_vvp_mixer_set_horiz_offset(&mixer, 3, 0);       // left edge
    intel_vvp_mixer_set_vert_offset(&mixer, 3, 1620);     // bottom-left
    intel_vvp_mixer_set_width(&mixer, 3, 960);
    intel_vvp_mixer_set_height(&mixer, 3, 540);
    // Mixer Input Layer #4 : frame reader output
#ifdef ISP_ROB_SUBSYSTEM_ISP_ROB_VFR_LRES_BASE
    // Mixer Input Layer #4 : frame reader output
    intel_vvp_mixer_set_blend_mode(&mixer, 4, kIntelVvpMixerBlendOpaque);
    intel_vvp_mixer_set_horiz_offset(&mixer, 4, 0);       // left edge
    intel_vvp_mixer_set_vert_offset(&mixer, 4, 500);     // bottom-left
    intel_vvp_mixer_set_width(&mixer, 4, 960);
    intel_vvp_mixer_set_height(&mixer, 4, 540);
#endif
    // Mixer Enable Input->Output
    intel_vvp_mixer_set_input_mode(&mixer, 1, true, false, true); //enable, consume, soft start
    intel_vvp_mixer_set_input_mode(&mixer, 2, true, false, true); //enable, consume, soft start
    intel_vvp_mixer_set_input_mode(&mixer, 3, true, false, true); //enable, consume, soft start
#ifdef ISP_ROB_SUBSYSTEM_ISP_ROB_VFR_LRES_BASE
    intel_vvp_mixer_set_input_mode(&mixer, 4, true, false, true); //enable, consume, soft start
#endif
    intel_vvp_mixer_commit_writes(&mixer);

    // 10-bit TPG for base layer with solid colour + Colour bars
    intel_vvp_tpg_init(&tpg_base_layer, (intel_vvp_core_base)ISP_LITE_OUT_SUBSYSTEM_ISP_TPG_BASE);
    intel_vvp_tpg_stop(&tpg_base_layer);
    intel_vvp_core_set_img_info_width(&tpg_base_layer.core_instance, INIT_HSIZE);
    intel_vvp_core_set_img_info_height(&tpg_base_layer.core_instance, INIT_VSIZE);
    intel_vvp_core_set_img_info_interlace(&tpg_base_layer.core_instance, 3);
    intel_vvp_tpg_set_pattern(&tpg_base_layer, 0);          // Colour bars = 1, Solid colours = 0
    intel_vvp_tpg_set_colors(&tpg_base_layer, 1023, 0, 0);  // BGR
    intel_vvp_tpg_commit_writes(&tpg_base_layer);
    intel_vvp_tpg_start(&tpg_base_layer);

    // VfB
    intel_vvp_vfb_init(&vfb, (intel_vvp_core_base)ISP_LITE_OUT_SUBSYSTEM_ISP_VFB_BASE);
    intel_vvp_core_set_img_info_width(&vfb.core_instance, INIT_HSIZE);
    intel_vvp_core_set_img_info_height(&vfb.core_instance, INIT_VSIZE);
    intel_vvp_core_set_img_info_interlace(&vfb.core_instance, 3);
    intel_vvp_core_set_img_info_colorspace(&vfb.core_instance, IMG_INFO_COLORSPACE_RGB);
    intel_vvp_core_set_img_info_subsampling(&vfb.core_instance, IMG_INFO_SUBSAMPLING_444);
    intel_vvp_core_set_img_info_cositing(&vfb.core_instance, 0);
    intel_vvp_vfb_output_enable(&vfb, true);

}

void vvp_cores_cfg_rob_subsystem() {

#ifdef ISP_ROB_SUBSYSTEM_ISP_ROB_MIXER_BASE
    // Mixer
    intel_vvp_mixer_init(&mixer_rob, (intel_vvp_core_base)ISP_ROB_SUBSYSTEM_ISP_ROB_MIXER_BASE);
    intel_vvp_core_set_img_info_width(&mixer_rob.core_instance, INIT_HSIZE);
    intel_vvp_core_set_img_info_height(&mixer_rob.core_instance, INIT_VSIZE);
    intel_vvp_core_set_img_info_subsampling(&mixer_rob.core_instance, IMG_INFO_SUBSAMPLING_444);
    // Mixer Input Layer # 0 : input video from pipeline 4k
    //intel_vvp_mixer_set_blend_mode(&mixer_rob, 0, kIntelVvpMixerBlendOpaque);
    //intel_vvp_mixer_set_horiz_offset(&mixer_rob, 0, 0);
    //intel_vvp_mixer_set_vert_offset(&mixer_rob, 0, 0);
    //intel_vvp_mixer_set_width(&mixer_rob, 0, INIT_HSIZE);
    //intel_vvp_mixer_set_height(&mixer_rob, 0, INIT_VSIZE);
    // Mixer Input Layer # 1 : input video from Aruco 1
    intel_vvp_mixer_set_blend_mode(&mixer_rob, 1, kIntelVvpMixerBlendOpaque);
    intel_vvp_mixer_set_horiz_offset(&mixer_rob, 1, 1120);
    intel_vvp_mixer_set_vert_offset(&mixer_rob, 1, 700);
    intel_vvp_mixer_set_width(&mixer_rob, 1, ICON_HSIZE);
    intel_vvp_mixer_set_height(&mixer_rob, 1, ICON_VSIZE);
     // Mixer Input Layer # 2 : input video from Aruco 2
    intel_vvp_mixer_set_blend_mode(&mixer_rob, 2, kIntelVvpMixerBlendOpaque);
    intel_vvp_mixer_set_horiz_offset(&mixer_rob, 2, 1450);
    intel_vvp_mixer_set_vert_offset(&mixer_rob, 2, 980);
    intel_vvp_mixer_set_width(&mixer_rob, 2, ICON_HSIZE);
    intel_vvp_mixer_set_height(&mixer_rob, 2, ICON_VSIZE);
     // Mixer Input Layer # 3 : input video from Aruco 3
    intel_vvp_mixer_set_blend_mode(&mixer_rob, 3, kIntelVvpMixerBlendOpaque);
    intel_vvp_mixer_set_horiz_offset(&mixer_rob, 3, 1810);
    intel_vvp_mixer_set_vert_offset(&mixer_rob, 3, 650);
    intel_vvp_mixer_set_width(&mixer_rob, 3, ICON_HSIZE);
    intel_vvp_mixer_set_height(&mixer_rob, 3, ICON_VSIZE);
     // Mixer Input Layer # 4 : input video from Aruco 4
    intel_vvp_mixer_set_blend_mode(&mixer_rob, 4, kIntelVvpMixerBlendOpaque);
    intel_vvp_mixer_set_horiz_offset(&mixer_rob, 4, 2240);
    intel_vvp_mixer_set_vert_offset(&mixer_rob, 4, 1160);
    intel_vvp_mixer_set_width(&mixer_rob, 4, ICON_HSIZE);
    intel_vvp_mixer_set_height(&mixer_rob, 4, ICON_VSIZE);
     // Mixer Input Layer # 5 : input video from Aruco 5
    intel_vvp_mixer_set_blend_mode(&mixer_rob, 5, kIntelVvpMixerBlendOpaque);
    intel_vvp_mixer_set_horiz_offset(&mixer_rob, 5, 2580);
    intel_vvp_mixer_set_vert_offset(&mixer_rob, 5, 820);
    intel_vvp_mixer_set_width(&mixer_rob, 5, ICON_HSIZE);
    intel_vvp_mixer_set_height(&mixer_rob, 5, ICON_VSIZE);
    // Mixer Enable Input->Output
    intel_vvp_mixer_set_input_mode(&mixer_rob, 1, true, false, true); //enable, consume, soft start
    intel_vvp_mixer_set_input_mode(&mixer_rob, 2, true, false, true); //enable, consume, soft start
    intel_vvp_mixer_set_input_mode(&mixer_rob, 3, true, false, true); //enable, consume, soft start
    intel_vvp_mixer_set_input_mode(&mixer_rob, 4, true, false, true); //enable, consume, soft start
    intel_vvp_mixer_set_input_mode(&mixer_rob, 5, true, false, true); //enable, consume, soft start

    intel_vvp_mixer_commit_writes(&mixer_rob);

#endif

    //Scaler
    intel_vvp_scaler_init(&scaler_rob, (intel_vvp_core_base)ISP_ROB_SUBSYSTEM_ISP_ROB_SCALER_DOWN_BASE);
    intel_vvp_core_set_img_info_width(&scaler_rob.core_instance, 1920);
    intel_vvp_core_set_img_info_height(&scaler_rob.core_instance, 1080);
    intel_vvp_core_set_img_info_interlace(&scaler_rob.core_instance, 3);
    intel_vvp_core_set_img_info_colorspace(&scaler_rob.core_instance, IMG_INFO_COLORSPACE_RGB);
    intel_vvp_core_set_img_info_subsampling(&scaler_rob.core_instance, IMG_INFO_SUBSAMPLING_444);
    intel_vvp_core_set_img_info_cositing(&scaler_rob.core_instance, 0);
    intel_vvp_scaler_set_output_width(&scaler_rob, 960);
    intel_vvp_scaler_set_output_height(&scaler_rob, 540);
    intel_vvp_scaler_commit_writes(&scaler_rob);

    //Clipper
    intel_vvp_clipper_init(&clipper_rob,(intel_vvp_core_base)ISP_ROB_SUBSYSTEM_ISP_ROB_CLIPPER_BASE);
    intel_vvp_core_set_img_info_width(&clipper_rob.core_instance, 3840);
    intel_vvp_core_set_img_info_height(&clipper_rob.core_instance, 2160);
    intel_vvp_core_set_img_info_interlace(&clipper_rob.core_instance, 3);
    intel_vvp_core_set_img_info_colorspace(&clipper_rob.core_instance, IMG_INFO_COLORSPACE_RGB);
    intel_vvp_core_set_img_info_subsampling(&clipper_rob.core_instance, IMG_INFO_SUBSAMPLING_444);
    intel_vvp_core_set_img_info_cositing(&clipper_rob.core_instance, 0);
    intel_vvp_clipper_set_clip_area (&clipper_rob, 960, 540, 1920, 1080);
    intel_vvp_clipper_commit_writes(&clipper_rob);

    // Vfw
    intel_vvp_vfw_init(&vfw, (intel_vvp_core_base)ISP_ROB_SUBSYSTEM_ISP_ROB_VFW_LRES_BASE);
    intel_vvp_core_set_img_info_width(&vfw.core_instance, 960);
    intel_vvp_core_set_img_info_height(&vfw.core_instance, 540);
    intel_vvp_core_set_img_info_interlace(&vfw.core_instance, 3);
    intel_vvp_core_set_img_info_colorspace(&vfw.core_instance, IMG_INFO_COLORSPACE_RGB);
    intel_vvp_core_set_img_info_subsampling(&vfw.core_instance, IMG_INFO_SUBSAMPLING_444);
    intel_vvp_core_set_img_info_cositing(&vfw.core_instance, 0);
    intel_vvp_vfw_set_base_addr (&vfw,0);
    intel_vvp_vfw_set_num_buffers (&vfw,1);
    intel_vvp_vfw_set_inter_line_offset(&vfw,2880);
    intel_vvp_vfw_set_inter_buffer_offset(&vfw,1555200);
    intel_vvp_vfw_overwrite_broken_fields(&vfw, true);
    intel_vvp_vfw_set_run_mode (&vfw, kIntelVvpVfwFreeRunning);
    intel_vvp_vfw_commit_writes (&vfw);

#ifdef ISP_ROB_SUBSYSTEM_ISP_ROB_VFR_LRES_BASE
    // Vfr
    intel_vvp_vfr_init(&vfr, (intel_vvp_core_base)ISP_ROB_SUBSYSTEM_ISP_ROB_VFR_LRES_BASE);
    intel_vvp_core_set_img_info_width(&vfr.core_instance, 960);
    intel_vvp_core_set_img_info_height(&vfr.core_instance, 540);
    intel_vvp_core_set_img_info_interlace(&vfr.core_instance, 3);
    intel_vvp_core_set_img_info_colorspace(&vfr.core_instance, IMG_INFO_COLORSPACE_RGB);
    intel_vvp_core_set_img_info_subsampling(&vfr.core_instance, IMG_INFO_SUBSAMPLING_444);
    intel_vvp_core_set_img_info_cositing(&vfr.core_instance, 0);
    intel_vvp_vfr_set_buffer_mode(&vfr,kIntelVvpVfrSingleSet);
    intel_vvp_vfr_set_starting_buffer_set(&vfr,0);
    intel_vvp_vfr_set_run_mode (&vfr, kIntelVvpVfrFreeRunning);
    intel_vvp_vfr_set_bufset_base_addr(&vfr, 0,0);
    intel_vvp_vfr_set_bufset_inter_buffer_offset(&vfr,0,1555200);
    intel_vvp_vfr_set_bufset_inter_line_offset(&vfr,0,2880);
    intel_vvp_vfr_set_bufset_width (&vfr, 0, 960);
    intel_vvp_vfr_set_bufset_height (&vfr, 0, 540);
    intel_vvp_vfr_set_bufset_interlace (&vfr, 0, 3);
    intel_vvp_vfr_set_bufset_colorspace(&vfr, 0, IMG_INFO_COLORSPACE_RGB);
    intel_vvp_vfr_set_bufset_subsampling(&vfr, 0, IMG_INFO_SUBSAMPLING_444);
    intel_vvp_vfr_set_bufset_cositing(&vfr, 0, 0);
    intel_vvp_vfr_set_bufset_bps(&vfr, 0, 10);
    intel_vvp_vfr_commit_writes (&vfr);
#endif

    //vfb
    intel_vvp_vfb_init(&vfb_rob, (intel_vvp_core_base)ISP_ROB_SUBSYSTEM_ISP_ROB_VFB_BASE);
    intel_vvp_core_set_img_info_width(&vfb_rob.core_instance, 960);
    intel_vvp_core_set_img_info_height(&vfb_rob.core_instance, 540);
    intel_vvp_core_set_img_info_interlace(&vfb_rob.core_instance, 3);
    intel_vvp_core_set_img_info_colorspace(&vfb_rob.core_instance, IMG_INFO_COLORSPACE_RGB);
    intel_vvp_core_set_img_info_subsampling(&vfb_rob.core_instance, IMG_INFO_SUBSAMPLING_444);
    intel_vvp_core_set_img_info_cositing(&vfb_rob.core_instance, 0);
    intel_vvp_vfb_output_enable(&vfb_rob, true);

    // Vfw_gs
    intel_vvp_vfw_init(&vfw_gs, (intel_vvp_core_base)ISP_ROB_SUBSYSTEM_ISP_ROB_VFW_LRES_GS_BASE);
    intel_vvp_core_set_img_info_width(&vfw_gs.core_instance, 960);
    intel_vvp_core_set_img_info_height(&vfw_gs.core_instance, 540);
    intel_vvp_core_set_img_info_interlace(&vfw_gs.core_instance, 1);
    intel_vvp_core_set_img_info_colorspace(&vfw_gs.core_instance, IMG_INFO_COLORSPACE_RGB);
    intel_vvp_core_set_img_info_subsampling(&vfw_gs.core_instance, IMG_INFO_SUBSAMPLING_444);
    intel_vvp_core_set_img_info_cositing(&vfw_gs.core_instance, 0);
    intel_vvp_vfw_set_base_addr (&vfw_gs,0x00200000);
    intel_vvp_vfw_set_num_buffers (&vfw_gs,1);
    intel_vvp_vfw_set_inter_line_offset(&vfw_gs,960);
    intel_vvp_vfw_set_inter_buffer_offset(&vfw_gs,518400);
    intel_vvp_vfw_overwrite_broken_fields(&vfw_gs, true);
    intel_vvp_vfw_set_run_mode (&vfw_gs, kIntelVvpVfwFreeRunning);
    intel_vvp_vfw_commit_writes (&vfw_gs);

}

void vvp_cores_cfg_isp_subsystem() {

    // Colour Space Converter
    intel_vvp_csc_init(&csc, (intel_vvp_core_base)ISP_LITE_SUBSYSTEM_ISP_CCM_BASE);
    intel_vvp_core_set_img_info_width(&csc.core_instance, INIT_HSIZE);
    intel_vvp_core_set_img_info_height(&csc.core_instance, INIT_VSIZE);
    intel_vvp_core_set_img_info_interlace(&csc.core_instance, 3);
    intel_vvp_core_set_img_info_colorspace(&csc.core_instance, IMG_INFO_COLORSPACE_RGB);
    intel_vvp_core_set_img_info_subsampling(&csc.core_instance, IMG_INFO_SUBSAMPLING_444);
    intel_vvp_core_set_img_info_cositing(&csc.core_instance, 0);
    intel_vvp_csc_set_coeff_data(&csc, &ccm_4000k, 10);
    intel_vvp_csc_set_output_color_space(&csc, kIntelVvpCsRgb);
    intel_vvp_csc_commit_writes(&csc);

    // Demosaic
    intel_vvp_demosaic_init(&demosaic, (intel_vvp_core_base)ISP_LITE_SUBSYSTEM_ISP_DMS_BASE);
    intel_vvp_core_set_img_info_width(&demosaic.core_instance, INIT_HSIZE);
    intel_vvp_core_set_img_info_height(&demosaic.core_instance, INIT_VSIZE);
    intel_vvp_demosaic_set_cfa_phase(&demosaic, BAYER_PHASE); // RGGB = 0 (Framos IMX678), BGGR = 3 (Raspberry pi hq camera)
    intel_vvp_demosaic_set_bypass(&demosaic, false);

    // White Balance Correction

    intel_vvp_wbc_init(&wbc, (intel_vvp_core_base)ISP_LITE_SUBSYSTEM_ISP_WBC_BASE);
    intel_vvp_core_set_img_info_width(&wbc.core_instance, INIT_HSIZE);
    intel_vvp_core_set_img_info_height(&wbc.core_instance, INIT_VSIZE);

    // Default to 5000K
    intel_vvp_wbc_set_cfa_00_color_scaler(&wbc, 3236);
    intel_vvp_wbc_set_cfa_01_color_scaler(&wbc, 2048);
    intel_vvp_wbc_set_cfa_10_color_scaler(&wbc, 2048);
    intel_vvp_wbc_set_cfa_11_color_scaler(&wbc, 4687);
    intel_vvp_wbc_set_cfa_phase(&wbc, BAYER_PHASE);  // RGGB = 0, BGGR = 3
    intel_vvp_wbc_set_bypass(&wbc, false);
    intel_vvp_wbc_commit(&wbc);

    // Black Balance Correction
    intel_vvp_blc_init(&blc, (intel_vvp_core_base)ISP_LITE_SUBSYSTEM_ISP_BLC_BASE);
    intel_vvp_core_set_img_info_width(&blc.core_instance, INIT_HSIZE);
    intel_vvp_core_set_img_info_height(&blc.core_instance, INIT_VSIZE);
    intel_vvp_blc_set_cfa_00_black_pedestal(&blc, 200);
    intel_vvp_blc_set_cfa_00_color_scaler(&blc, 13460);
    intel_vvp_blc_set_cfa_01_black_pedestal(&blc, 200);
    intel_vvp_blc_set_cfa_01_color_scaler(&blc, 13460);
    intel_vvp_blc_set_cfa_10_black_pedestal(&blc, 200);
    intel_vvp_blc_set_cfa_10_color_scaler(&blc, 13460);
    intel_vvp_blc_set_cfa_11_black_pedestal(&blc, 200);
    intel_vvp_blc_set_cfa_11_color_scaler(&blc, 13460);
    intel_vvp_blc_set_cfa_phase(&blc, BAYER_PHASE); // RGGB = 0, BGGR = 3
    intel_vvp_blc_set_bypass(&blc, false);
    intel_vvp_blc_commit(&blc);

    // Input Switch (VVP-Full) 2x1
    intel_vvp_switch_init(&input_switch, (intel_vvp_core_base)ISP_IN_SUBSYSTEM_ISP_IN_SWITCH_BASE);
#if USE_SENSOR
    intel_vvp_switch_set_input_config(&input_switch, TPG_BAYER_SWITCH_IN, kIntelVvpSwitchInputConsumed); // TPG
    intel_vvp_switch_set_input_config(&input_switch, MIPI_BAYER_SWITCH_IN, kIntelVvpSwitchInputEnabled); // IMX Sensor
    intel_vvp_switch_set_output_config(&input_switch, BAYER_SWITCH_OUT, true, MIPI_BAYER_SWITCH_IN);
#else
    intel_vvp_switch_set_input_config(&input_switch, TPG_BAYER_SWITCH_IN, kIntelVvpSwitchInputEnabled);   // TPG
    intel_vvp_switch_set_input_config(&input_switch, MIPI_BAYER_SWITCH_IN, kIntelVvpSwitchInputConsumed); // IMX Sensor
    intel_vvp_switch_set_output_config(&input_switch, BAYER_SWITCH_OUT, true, TPG_BAYER_SWITCH_IN);
#endif
    intel_vvp_switch_commit_writes(&input_switch);

    // Remosaic Phase Configuration
    printf("Remosaic Phase RGGB \n");
    // RGGB = 0x94 -> 1001_0100 (Framos IMX678)
    // BGGR = 0x16 -> 0001_0110 (Raspberry pi hq camera)
    IOWR(ISP_IN_SUBSYSTEM_ISP_IN_RMS_BASE, 1, 0x94);

    // 10-bit TPG for base layer with solid colour + Colour bars
    intel_vvp_tpg_init(&input_tpg, (intel_vvp_core_base)ISP_IN_SUBSYSTEM_ISP_IN_TPG_BASE);
    intel_vvp_tpg_stop(&input_tpg);
    intel_vvp_core_set_img_info_width(&input_tpg.core_instance, INIT_HSIZE);
    intel_vvp_core_set_img_info_height(&input_tpg.core_instance, INIT_VSIZE);
    intel_vvp_core_set_img_info_interlace(&input_tpg.core_instance, 3);
    intel_vvp_tpg_set_pattern(&input_tpg, 0); // Colour bars = 0, Solid colours = 1
    intel_vvp_tpg_set_bars_type(&input_tpg, kIntelVvpTpgColorBars);
    intel_vvp_tpg_set_colors(&input_tpg, 0, 4095, 0); // BGR
    intel_vvp_tpg_commit_writes(&input_tpg);
    intel_vvp_tpg_start(&input_tpg);

}

void vvp_cores_cfg_mipi_in_subsystem() {

    // PiP Converter MIPI Rx
    intel_vvp_pip_conv_init(&pip_conv_mipi, (intel_vvp_core_base)MIPI_IN_SUBSYSTEM_MIPI_IN_PIP_CONV_0_BASE);
    intel_vvp_core_set_img_info_width(&pip_conv_mipi.core_instance, INIT_HSIZE);
    intel_vvp_core_set_img_info_height(&pip_conv_mipi.core_instance, INIT_VSIZE);
    intel_vvp_core_set_img_info_interlace(&pip_conv_mipi.core_instance, 3);
    intel_vvp_core_set_img_info_colorspace(&pip_conv_mipi.core_instance, IMG_INFO_COLORSPACE_RGB);
    intel_vvp_core_set_img_info_subsampling(&pip_conv_mipi.core_instance, IMG_INFO_SUBSAMPLING_444);
    intel_vvp_core_set_img_info_cositing(&pip_conv_mipi.core_instance, 0);

}

#if defined(ISP_ROB_SUBSYSTEM_ISP_ROB_MIXER_BASE) \
    && defined(ISP_ROB_SUBSYSTEM_ISP_ROB_ARUCO_CTRL_BASE)

#define MAX_SQUARES 6
PixelPlacementCfg cfg = {
    .h_start = 960,
    .h_end = 2880,
    .v_start = 540,
    .v_end = 1620,
    .margin = 50,
    .square_side = 144,
};

PixelsRect squares[MAX_SQUARES];

static alt_u8 aruco_ctrl_read_byte(alt_u8 byte_index) {
    alt_u32 reg = IORD(ISP_ROB_SUBSYSTEM_ISP_ROB_ARUCO_CTRL_BASE, 0);
    alt_u32 shift = (alt_u32)byte_index * 8u;
    return (alt_u8)((reg >> shift) & 0xFFu);
}

static void aruco_ctrl_write_byte(alt_u8 byte_index, alt_u8 value) {
    alt_u32 shift = (alt_u32)byte_index * 8u;
    alt_u32 mask = (alt_u32)0xFFu << shift;
    alt_u32 reg = IORD(ISP_ROB_SUBSYSTEM_ISP_ROB_ARUCO_CTRL_BASE, 0);
    reg = (reg & ~mask) | ((alt_u32)value << shift);
    IOWR(ISP_ROB_SUBSYSTEM_ISP_ROB_ARUCO_CTRL_BASE, 0, reg);
}
#endif


void arUco_ctrl() {

#if defined(ISP_ROB_SUBSYSTEM_ISP_ROB_MIXER_BASE) \
    && defined(ISP_ROB_SUBSYSTEM_ISP_ROB_ARUCO_CTRL_BASE)

    alt_u8 request     = aruco_ctrl_read_byte(0);
    alt_u8 aruco_stat  = aruco_ctrl_read_byte(2);

    int n_layers = intel_vvp_mixer_get_num_layers(&mixer_rob);
    aruco_ctrl_write_byte(1, (alt_u8)(n_layers - 1));

    if (request == 1) {

        int placed = pixels_place_squares_random(&cfg, n_layers, squares, 2u);
        for (int i = 0; i < placed; i++) {
            /* squares[i].x, squares[i].y top-left; .w and .h == 144 */
            intel_vvp_mixer_set_horiz_offset(&mixer_rob, i + 1, squares[i].x);
            intel_vvp_mixer_set_vert_offset(&mixer_rob, i + 1, squares[i].y);
        }
        intel_vvp_mixer_commit_writes(&mixer_rob);
        /* Bits 0 .. placed-1 => blocks we just positioned (matches mixer layers 1..placed) */
        alt_u8 stat_update = (placed > 0)
            ? (alt_u8)(((1u << placed) - 1u) & 0xFFu)
            : (alt_u8)0;
        aruco_ctrl_write_byte(2, (alt_u8)stat_update);
        /* Value read at entry is stale vs this write; drive mixer from updated mask this pass. */
        aruco_stat = stat_update;
        aruco_ctrl_write_byte(0, (alt_u8)0);
    }

    /* aruco_stat: bit i == 1 => ArUco block id i present; layer i maps to mixer layer i+1 */
    for (int block_id = 0; block_id < n_layers; block_id++) {
        bool present = (aruco_stat & ((alt_u8)1u << block_id)) != 0;
        int layer = block_id + 1;

        intel_vvp_mixer_set_input_mode(&mixer_rob, layer, present, false, true);
    }
    intel_vvp_mixer_commit_writes(&mixer_rob);

    if (aruco_stat == 0) {
        aruco_ctrl_write_byte(0, (alt_u8)1);
    }

#endif

}

void ccm_rgb_mode() {

    printf("CCM: RGB Mode \n");
    if (int_ccm_cnt <= 6) {
        int_ccm_cnt = int_ccm_cnt + 1;
    } else {
        int_ccm_cnt = 1;
    }
    if (int_ccm_cnt == 1) {             // # 3000K
        intel_vvp_csc_set_coeff_data(&csc, &ccm_3000k, 10);
        printf("CCM: Set for 3000K \n");
        printf_csc_coeffs(ccm_3000k);
    } else if (int_ccm_cnt == 2) {      // # 4000K
        intel_vvp_csc_set_coeff_data(&csc, &ccm_4000k, 10);
        printf("CCM: Set for 4000K \n");
        printf_csc_coeffs(ccm_4000k);
    } else if (int_ccm_cnt == 3) {      // # 5000K
        intel_vvp_csc_set_coeff_data(&csc, &ccm_5000k, 10);
        printf("CCM: Set for 5000K \n");
        printf_csc_coeffs(ccm_5000k);
    } else if (int_ccm_cnt == 4) {      // # 6000K
        intel_vvp_csc_set_coeff_data(&csc, &ccm_6000k, 10);
        printf("CCM: Set for 6000K \n");
        printf_csc_coeffs(ccm_6000k);
    } else if (int_ccm_cnt == 5) {      // # 7000K
        intel_vvp_csc_set_coeff_data(&csc, &ccm_7000k, 10);
        printf("CCM: Set for 7000K \n");
        printf_csc_coeffs(ccm_7000k);
    } else if (int_ccm_cnt == 6) {      // # 8000K
        intel_vvp_csc_set_coeff_data(&csc, &ccm_8000k, 10);
        printf("CCM: Set for 8000K \n");
        printf_csc_coeffs(ccm_8000k);
    } else if (int_ccm_cnt == 7) {      // # 9000K
        intel_vvp_csc_set_coeff_data(&csc, &ccm_9000k, 10);
        printf("CCM: Set for 9000K \n");
        printf_csc_coeffs(ccm_9000k);
    }
    intel_vvp_csc_commit_writes(&csc);
}

void ccm_bypass_mode (){
    printf("CCM: Bypass Mode \n");
    intel_vvp_csc_set_coeff_data(&csc, &ccm_passthrough, 0);
    intel_vvp_csc_commit_writes(&csc);
}

void restore_vvp_core_default() {

    // Protocol converter (VVP-Full)
    intel_vvp_core_set_img_info_width(&proto_lite_to_full.core_instance, INIT_HSIZE);
    intel_vvp_core_set_img_info_height(&proto_lite_to_full.core_instance, INIT_VSIZE);
    intel_vvp_core_set_img_info_interlace(&proto_lite_to_full.core_instance, 3);
    intel_vvp_core_set_img_info_colorspace(&proto_lite_to_full.core_instance, IMG_INFO_COLORSPACE_RGB);
    intel_vvp_core_set_img_info_subsampling(&proto_lite_to_full.core_instance, IMG_INFO_SUBSAMPLING_444);
    intel_vvp_core_set_img_info_cositing(&proto_lite_to_full.core_instance, 0);
    intel_vvp_protocol_conv_enable(&proto_lite_to_full, true);

    // PiP Converter (DP Tx) 1 to 2
    intel_vvp_core_set_img_info_width(&pip_conv_1to2.core_instance, INIT_HSIZE);
    intel_vvp_core_set_img_info_height(&pip_conv_1to2.core_instance, INIT_VSIZE);
    intel_vvp_core_set_img_info_interlace(&pip_conv_1to2.core_instance, 3);
    intel_vvp_core_set_img_info_colorspace(&pip_conv_1to2.core_instance, IMG_INFO_COLORSPACE_RGB);
    intel_vvp_core_set_img_info_subsampling(&pip_conv_1to2.core_instance, IMG_INFO_SUBSAMPLING_444);
    intel_vvp_core_set_img_info_cositing(&pip_conv_1to2.core_instance, 0);

    // 1D LUT
    intel_vvp_1d_lut_set_bypass(&vvp_1d_lut, false);

    // Mixer Input Layer # 1 : input video from VfB
    intel_vvp_mixer_set_horiz_offset(&mixer, 1, 0);
    intel_vvp_mixer_set_vert_offset(&mixer, 1, 0);
    intel_vvp_mixer_set_width(&mixer, 1, INIT_HSIZE);
    intel_vvp_mixer_set_height(&mixer, 1, INIT_VSIZE);

    // Mixer Input Layer # 3 : input video from Icon
    intel_vvp_mixer_set_horiz_offset(&mixer, 2, 0);
    intel_vvp_mixer_set_vert_offset(&mixer, 2, 0);
    intel_vvp_mixer_set_width(&mixer, 2, ICON_HSIZE);
    intel_vvp_mixer_set_height(&mixer, 2, ICON_VSIZE);

    intel_vvp_mixer_set_input_mode(&mixer, 1, true, false, true); //enable, consume, soft start
    intel_vvp_mixer_set_input_mode(&mixer, 2, true, false, true); //enable, consume, soft start
    intel_vvp_mixer_commit_writes(&mixer);

    // 10-bit TPG for base layer with solid colour + Colour bars
    intel_vvp_tpg_stop(&tpg_base_layer);
    intel_vvp_core_set_img_info_width(&tpg_base_layer.core_instance, INIT_HSIZE);
    intel_vvp_core_set_img_info_height(&tpg_base_layer.core_instance, INIT_VSIZE);
    intel_vvp_tpg_set_pattern(&tpg_base_layer, 0); // Colour bars = 1, Solid colours = 0
    intel_vvp_tpg_set_colors(&tpg_base_layer, 1023, 0, 0); // BGR
    intel_vvp_tpg_commit_writes(&tpg_base_layer);
    intel_vvp_tpg_start(&tpg_base_layer);

    // VfB
    intel_vvp_core_set_img_info_width(&vfb.core_instance, INIT_HSIZE);
    intel_vvp_core_set_img_info_height(&vfb.core_instance, INIT_VSIZE);
    intel_vvp_vfb_output_enable(&vfb, true);

    // Colour Space Converter
    int_ccm_cnt = 2;
    intel_vvp_core_set_img_info_width(&csc.core_instance, INIT_HSIZE);
    intel_vvp_core_set_img_info_height(&csc.core_instance, INIT_VSIZE);
    intel_vvp_csc_set_coeff_data(&csc, &ccm_5000k, 10);
    intel_vvp_csc_set_output_color_space(&csc, kIntelVvpCsRgb);
    intel_vvp_csc_commit_writes(&csc);

    // Demosaic
    intel_vvp_core_set_img_info_width(&demosaic.core_instance, INIT_HSIZE);
    intel_vvp_core_set_img_info_height(&demosaic.core_instance, INIT_VSIZE);
    intel_vvp_demosaic_set_cfa_phase(&demosaic, BAYER_PHASE); // RGGB = 0, BGGR = 3
    intel_vvp_demosaic_set_bypass(&demosaic, false);

    // White Balance Correction
    int_wbc_cnt = 1;
    intel_vvp_core_set_img_info_width(&wbc.core_instance, INIT_HSIZE);
    intel_vvp_core_set_img_info_height(&wbc.core_instance, INIT_VSIZE);

    // Default to 5000K: from 3000K to 9000K
    intel_vvp_wbc_set_cfa_00_color_scaler(&wbc, 0x1d89);
    intel_vvp_wbc_set_cfa_01_color_scaler(&wbc, 0x0800);
    intel_vvp_wbc_set_cfa_10_color_scaler(&wbc, 0x0800);
    intel_vvp_wbc_set_cfa_11_color_scaler(&wbc, 0x0de9);
    intel_vvp_wbc_set_cfa_phase(&wbc, BAYER_PHASE); // RGGB = 0, BGGR = 3
    intel_vvp_wbc_set_bypass(&wbc, false);
    intel_vvp_wbc_commit(&wbc);

    // Black Balance Correction
    intel_vvp_core_set_img_info_width(&blc.core_instance, INIT_HSIZE);
    intel_vvp_core_set_img_info_height(&blc.core_instance, INIT_VSIZE);
    intel_vvp_blc_set_cfa_00_black_pedestal(&blc, 256);
    intel_vvp_blc_set_cfa_00_color_scaler(&blc, 17496);
    intel_vvp_blc_set_cfa_01_black_pedestal(&blc, 256);
    intel_vvp_blc_set_cfa_01_color_scaler(&blc, 17496);
    intel_vvp_blc_set_cfa_10_black_pedestal(&blc, 256);
    intel_vvp_blc_set_cfa_10_color_scaler(&blc, 17496);
    intel_vvp_blc_set_cfa_11_black_pedestal(&blc, 256);
    intel_vvp_blc_set_cfa_11_color_scaler(&blc, 17496);
    intel_vvp_blc_set_cfa_phase(&blc, BAYER_PHASE); // RGGB = 0, BGGR = 3
    intel_vvp_blc_set_bypass(&blc, false);
    intel_vvp_blc_commit(&blc);

    // Input Switch (VVP-Full) 2x1
    intel_vvp_switch_set_input_config(&input_switch, TPG_BAYER_SWITCH_IN, kIntelVvpSwitchInputConsumed); // TPG
    intel_vvp_switch_set_input_config(&input_switch, MIPI_BAYER_SWITCH_IN, kIntelVvpSwitchInputEnabled); // IMX Sensor
    intel_vvp_switch_set_output_config(&input_switch, BAYER_SWITCH_OUT, true, MIPI_BAYER_SWITCH_IN);
    intel_vvp_switch_commit_writes(&input_switch);

    // Remosaic Phase Configuration
    printf("Remosaic Phase RGGB \n");
    // RGGB = 0x94 -> 1001_0100 (Framos IMX678)
    // BGGR = 0x94 -> 0001_0110 (Raspberry pi hq camera)
    IOWR(ISP_IN_SUBSYSTEM_ISP_IN_RMS_BASE, 1, 0x16);

    // 10-bit TPG for base layer with solid colour + Colour bars
    intel_vvp_tpg_stop(&input_tpg);
    intel_vvp_core_set_img_info_width(&input_tpg.core_instance, INIT_HSIZE);
    intel_vvp_core_set_img_info_height(&input_tpg.core_instance, INIT_VSIZE);
    intel_vvp_core_set_img_info_interlace(&input_tpg.core_instance, 3);
    intel_vvp_tpg_set_pattern(&input_tpg, 0); // Colour bars = 1, Solid colours = 0
    intel_vvp_tpg_set_colors(&input_tpg, 0, 1023, 0); // BGR
    intel_vvp_tpg_commit_writes(&input_tpg);
    intel_vvp_tpg_start(&input_tpg);

}

void toggle_input_switch() {

    printf("Toggle Input Switch \n");
    if (int_tpg_cnt < 4) {
        int_tpg_cnt = int_tpg_cnt + 1;
    } else {
        int_tpg_cnt = 0;
    }

    if (int_tpg_cnt == 1) {
        intel_vvp_switch_set_input_config(&input_switch, TPG_BAYER_SWITCH_IN, kIntelVvpSwitchInputEnabled);   // TPG
        intel_vvp_switch_set_input_config(&input_switch, MIPI_BAYER_SWITCH_IN, kIntelVvpSwitchInputConsumed); // IMX Sensor
        intel_vvp_switch_set_output_config(&input_switch, BAYER_SWITCH_OUT, true, TPG_BAYER_SWITCH_IN);
        intel_vvp_tpg_set_pattern(&input_tpg, 0);           // Colour bars = 0, Solid colours = 1
        intel_vvp_tpg_set_bars_type(&input_tpg, kIntelVvpTpgColorBars);
        intel_vvp_tpg_set_colors(&input_tpg, 0, 4095, 0);   // BGR
        intel_vvp_tpg_commit_writes(&input_tpg);
        intel_vvp_switch_commit_writes(&input_switch);
        printf("Input Switch: Input TPG Color Bars \n");
    } else if (int_tpg_cnt == 2) {
        intel_vvp_switch_set_input_config(&input_switch, TPG_BAYER_SWITCH_IN, kIntelVvpSwitchInputEnabled);   // TPG
        intel_vvp_switch_set_input_config(&input_switch, MIPI_BAYER_SWITCH_IN, kIntelVvpSwitchInputConsumed); // IMX Sensor
        intel_vvp_switch_set_output_config(&input_switch, BAYER_SWITCH_OUT, true, TPG_BAYER_SWITCH_IN);
        intel_vvp_tpg_set_pattern(&input_tpg, 1);           // Colour bars = 0, Solid colours = 1
        intel_vvp_tpg_set_bars_type(&input_tpg, kIntelVvpTpgColorBars);
        intel_vvp_tpg_set_colors(&input_tpg, 4095, 0, 0);   // BGR
        intel_vvp_tpg_commit_writes(&input_tpg);
        intel_vvp_switch_commit_writes(&input_switch);
        printf("Input Switch: Input TPG Solid-Blue \n");
    } else if (int_tpg_cnt == 3) {
        intel_vvp_switch_set_input_config(&input_switch, TPG_BAYER_SWITCH_IN, kIntelVvpSwitchInputEnabled);   // TPG
        intel_vvp_switch_set_input_config(&input_switch, MIPI_BAYER_SWITCH_IN, kIntelVvpSwitchInputConsumed); // IMX Sensor
        intel_vvp_switch_set_output_config(&input_switch, BAYER_SWITCH_OUT, true, TPG_BAYER_SWITCH_IN);
        intel_vvp_tpg_set_pattern(&input_tpg, 1);           // Colour bars = 0, Solid colours = 1
        intel_vvp_tpg_set_bars_type(&input_tpg, kIntelVvpTpgColorBars);
        intel_vvp_tpg_set_colors(&input_tpg, 0, 4095, 0);   // BGR
        intel_vvp_tpg_commit_writes(&input_tpg);
        intel_vvp_switch_commit_writes(&input_switch);
        printf("Input Switch: Input TPG Solid-Green \n");
    } else if (int_tpg_cnt == 4) {
        intel_vvp_switch_set_input_config(&input_switch, TPG_BAYER_SWITCH_IN, kIntelVvpSwitchInputEnabled);   // TPG
        intel_vvp_switch_set_input_config(&input_switch, MIPI_BAYER_SWITCH_IN, kIntelVvpSwitchInputConsumed); // IMX Sensor
        intel_vvp_switch_set_output_config(&input_switch, BAYER_SWITCH_OUT, true, TPG_BAYER_SWITCH_IN);
        intel_vvp_tpg_set_pattern(&input_tpg, 1);           // Colour bars = 0, Solid colours = 1
        intel_vvp_tpg_set_bars_type(&input_tpg, kIntelVvpTpgColorBars);
        intel_vvp_tpg_set_colors(&input_tpg, 0, 0, 4095);   // BGR
        intel_vvp_tpg_commit_writes(&input_tpg);
        intel_vvp_switch_commit_writes(&input_switch);
        printf("Input Switch: Input TPG Solid-Red \n");
    } else {
        intel_vvp_switch_set_input_config(&input_switch, TPG_BAYER_SWITCH_IN, kIntelVvpSwitchInputConsumed); // TPG
        intel_vvp_switch_set_input_config(&input_switch, MIPI_BAYER_SWITCH_IN, kIntelVvpSwitchInputEnabled); // IMX Sensor
        intel_vvp_switch_set_output_config(&input_switch, BAYER_SWITCH_OUT, true, MIPI_BAYER_SWITCH_IN);
        intel_vvp_switch_commit_writes(&input_switch);
        printf("Input Switch: MIPI Rx \n");
    }

}

void blc_on_rggb() {

    printf("BLC: ON RGGB \n");
    intel_vvp_blc_set_cfa_00_black_pedestal(&blc, 200);
    intel_vvp_blc_set_cfa_00_color_scaler(&blc, 13460);
    intel_vvp_blc_set_cfa_01_black_pedestal(&blc, 200);
    intel_vvp_blc_set_cfa_01_color_scaler(&blc, 13460);
    intel_vvp_blc_set_cfa_10_black_pedestal(&blc, 200);
    intel_vvp_blc_set_cfa_10_color_scaler(&blc, 13460);
    intel_vvp_blc_set_cfa_11_black_pedestal(&blc, 200);
    intel_vvp_blc_set_cfa_11_color_scaler(&blc, 13460);
    intel_vvp_blc_set_cfa_phase(&blc, BAYER_PHASE);
    intel_vvp_blc_set_bypass(&blc, false);
    intel_vvp_blc_commit(&blc);

}

void blc_bypass() {

    printf("BLC: Bypass \n");
    intel_vvp_blc_set_bypass(&blc, true);
    intel_vvp_blc_commit(&blc);

}

void wbc_on_rggb() {

    printf("WBC: ON RGGB  \n");
    if (int_wbc_cnt <= 6) {
        int_wbc_cnt = int_wbc_cnt + 1;
    } else {
        int_wbc_cnt = 1;
    }

    // Default to 5000K:  3000K to 9000K
    if (int_wbc_cnt == 1) {                 // # 3000K
        intel_vvp_wbc_set_cfa_00_color_scaler(&wbc, 2627);
        intel_vvp_wbc_set_cfa_11_color_scaler(&wbc, 5886);
        printf("WBC: Set for 3000K \n");
    } else if (int_wbc_cnt == 2) {          // # 4000K
        intel_vvp_wbc_set_cfa_00_color_scaler(&wbc, 3236);
        intel_vvp_wbc_set_cfa_11_color_scaler(&wbc, 4687);
        printf("WBC: Set for 4000K \n");
    } else if (int_wbc_cnt == 3) {          // # 5000K
        intel_vvp_wbc_set_cfa_00_color_scaler(&wbc, 3715);
        intel_vvp_wbc_set_cfa_11_color_scaler(&wbc, 4091);
        printf("WBC: Set for 5000K \n");
    } else if (int_wbc_cnt == 4) {          // # 6000K
        intel_vvp_wbc_set_cfa_00_color_scaler(&wbc, 4024);
        intel_vvp_wbc_set_cfa_11_color_scaler(&wbc, 3666);
        printf("WBC: Set for 6000K \n");
    } else if (int_wbc_cnt == 5) {          // # 7000K
        intel_vvp_wbc_set_cfa_00_color_scaler(&wbc, 4168);
        intel_vvp_wbc_set_cfa_11_color_scaler(&wbc, 3495);
        printf("WBC: Set for 7000K \n");
    } else if (int_wbc_cnt == 6) {          // # 8000K
        intel_vvp_wbc_set_cfa_00_color_scaler(&wbc, 4316);
        intel_vvp_wbc_set_cfa_11_color_scaler(&wbc, 3301);
        printf("WBC: Set for 8000K \n");
    } else if (int_wbc_cnt == 7) {          // # 9000K
        intel_vvp_wbc_set_cfa_00_color_scaler(&wbc, 4416);
        intel_vvp_wbc_set_cfa_11_color_scaler(&wbc, 3131);
        printf("WBC: Set for 9000K \n");
    }

    intel_vvp_wbc_set_cfa_01_color_scaler(&wbc, 2048);
    intel_vvp_wbc_set_cfa_10_color_scaler(&wbc, 2048);
    intel_vvp_wbc_set_cfa_phase(&wbc, BAYER_PHASE);
    intel_vvp_wbc_set_bypass(&wbc, false);
    intel_vvp_wbc_commit(&wbc);

}

void wbc_bypass() {

    printf("WBC: Bypass \n");
    intel_vvp_wbc_set_bypass(&wbc, true);
    intel_vvp_wbc_commit(&wbc);

}


void demosaic_rggb_mode() {

    printf("Demosaic: RGGB  Mode \n");
    intel_vvp_demosaic_set_cfa_phase(&demosaic, BAYER_PHASE);
    intel_vvp_demosaic_set_bypass(&demosaic, false);

}

void demosaic_bypass() {

    printf("Demosaic: Bypass Mode \n");
    intel_vvp_demosaic_set_cfa_phase(&demosaic, 3);
    intel_vvp_demosaic_set_bypass(&demosaic, true);

}

void bt709_1d_lut_mode() {

    printf("1D LUT: BT-709 Mode \n");
    intel_vvp_1d_lut_set_bypass(&vvp_1d_lut, false);

}

void bypass_1dlut() {

    printf("1D LUT: Bypass Mode \n");
    intel_vvp_1d_lut_set_bypass(&vvp_1d_lut, true);

}

void toggle_vfb_icon(){

    printf("Toggle VfB + Icon \n");
    if (icon_tgl == 0) {
        icon_tgl = 1;
        intel_vvp_mixer_set_input_mode(&mixer, 1, true, false, true); //enable, consume, soft start: VfB
        intel_vvp_mixer_set_input_mode(&mixer, 2, true, false, true); //enable, consume, soft start: Icon
        intel_vvp_mixer_commit_writes(&mixer);
        printf("Icon     : On \n");
    } else {
        icon_tgl = 0;
        intel_vvp_mixer_set_input_mode(&mixer, 1, true, false, true); //enable, consume, soft start: VfB
        intel_vvp_mixer_set_input_mode(&mixer, 2, true, true,  true); //enable, consume, soft start: Icon
        intel_vvp_mixer_commit_writes(&mixer);
        printf("Icon     : Off \n");
    }

}
