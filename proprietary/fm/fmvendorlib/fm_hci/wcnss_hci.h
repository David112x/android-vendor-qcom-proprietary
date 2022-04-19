/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Copyright (c) 2015, The Linux Foundation. All rights reserved.
 *
 */

#ifndef __WCNSS_HCI__
#define __WCNSS_HCI__
static void vendor_fwcfg_cb(bt_vendor_op_result_t result) {
}

static void vendor_scocfg_cb(bt_vendor_op_result_t result) {
}

static void vendor_lpm_vnd_cb(bt_vendor_op_result_t result) {
}

static void sco_audiostate_cb(bt_vendor_op_result_t result) {
}

static void* vendor_alloc(int size) {
    return NULL;
}

static void vendor_dealloc(void *p_buf) {
}

static uint8_t vendor_xmit_cb(uint16_t opcode, void *p_buf, tINT_CMD_CBACK p_cback) {
    return 0;
}

static void vendor_epilog_cb(bt_vendor_op_result_t result) {
}


static const bt_vendor_callbacks_t fm_vendor_callbacks = {
  sizeof(fm_vendor_callbacks),
   vendor_fwcfg_cb,
   vendor_scocfg_cb,
   vendor_lpm_vnd_cb,
   sco_audiostate_cb,
   vendor_alloc,
   vendor_dealloc,
   vendor_xmit_cb,
   vendor_epilog_cb
};
#endif
