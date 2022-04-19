/*
 * Copyright (c) 2019, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ***********************************************************************/

/************************************************************************************
 *
 *  Filename:      btif_vendor_hf.cc
 *
 *  Description:   Vendor Bluetooth Interface
 *
 *
 ***********************************************************************************/

#define LOG_TAG "bt_btif_vendor_hf"

#include <hardware/vendor_hf.h>
#include "bta_ag_swb.h"
#include "btif_twsp_hf.h"
#include "btif_api.h"

btvendor_hf_callbacks_t *bt_vendor_hf_callbacks = NULL;
static bool swb_codec_status = false;

/*******************************************************************************
** VENDOR INTERFACE FUNCTIONS
*******************************************************************************/

/*******************************************************************************
**
** Function         init
**
** Description     initializes the vendor interface for HF
**
** Returns         bt_status_t
**
*******************************************************************************/

static bt_status_t init( btvendor_hf_callbacks_t* callbacks)
{
    bt_vendor_hf_callbacks = callbacks;
    LOG_INFO(LOG_TAG,"init done");
    return BT_STATUS_SUCCESS;
}

static void cleanup(void)
{
    LOG_INFO(LOG_TAG,"cleanup");
    if (bt_vendor_hf_callbacks)
        bt_vendor_hf_callbacks = NULL;
}

int enable_swb(bool enable) {
  LOG_INFO(LOG_TAG,"%s: %d", __func__, enable);
  swb_codec_status = enable;
  return 0;
}

static const btvendor_hf_interface_t btvendorhfInterface = {
    sizeof(btvendorhfInterface),
    init,
    cleanup,
    enable_swb,
};

void btif_handle_vendor_hf_events(uint16_t event, tBTA_AG* data,
                                  RawAddress *bd_addr) {
    BTIF_TRACE_EVENT("%s: event: %d", __func__, event)
    switch (event) {
        case BTA_AG_SWB_EVT:
            HAL_CBACK(bt_vendor_hf_callbacks, swb_codec_cb, data->val.num,
                        bd_addr);
            break;
        case BTA_AG_AT_QCS_EVT:
            HAL_CBACK(bt_vendor_hf_callbacks, swb_codec_cb, data->val.num,
                        bd_addr);
            break;
        case BTA_AG_TWSP_BATTERY_UPDATE:
            HAL_CBACK(bt_vendor_hf_callbacks, twsp_batt_status_cb,
                        data->val.str, bd_addr);
            break;
        default:
            BTIF_TRACE_EVENT("%s: unknown vendor hf event: %d", __func__,event);
    }
}

bool get_swb_codec_status() {
    return swb_codec_status;
}

/*******************************************************************************
**
** Function         btif_vendor_hf_get_interface
**
** Description      Get the vendor callback interface
**
** Returns          btvendor_hf_interface_t
**
*******************************************************************************/
const btvendor_hf_interface_t *btif_vendor_hf_get_interface()
{
    BTIF_TRACE_EVENT("%s", __FUNCTION__);
    return &btvendorhfInterface;
}

