/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "intel_axi2cv.h"
#include "dptx_formats.h"

#undef DEBUG
#ifdef DEBUG
#define DPTX_PRINT(...)  printf(__VA_ARGS__)
#else
#define DPTX_PRINT(...)
#undef BITEC_STATUS_DEBUG
#endif /* DEBUG */

#define AXI_OFFSET 0x300

// Get the core capabilities (defined in QSYS and ported to system.h)
#define TX_MAX_LINK_RATE      DP_TX_DP_SOURCE_BITEC_CFG_TX_MAX_LINK_RATE
#define TX_MAX_LANE_COUNT     DP_TX_DP_SOURCE_BITEC_CFG_TX_MAX_LANE_COUNT

void update_supported_formats_from_dtd(const uint8_t* dtd);
void process_sink_edid();
void program_cvo_timing(intel_axi2cv_instance* cvo, const cvo_timing_info_t* t);
uint32_t get_tx_link_rate();
int init_dp_tx();
void dp_tx_loop ();
