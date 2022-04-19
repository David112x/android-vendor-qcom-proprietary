/*
 * Copyright (C) 2016 The Linux Foundation. All rights reserved
 * Not a Contribution.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the
 * disclaimer below) provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *
 * * Neither the name of The Linux Foundation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
 * GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
 * HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  Copyright (C) 2009-2012 Broadcom Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/

/************************************************************************************
 *
 *  Filename:      btif_vendor.cc
 *
 *  Description:   Vendor Bluetooth Interface
 *
 *
 ***********************************************************************************/

#include <hardware/vendor.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#define LOG_TAG "bt_btif_vendor"

#include <cutils/properties.h>
#include <base/bind.h>
#include <base/callback.h>
#include <base/location.h>
#include "bt_utils.h"
#include "btif_common.h"
#include "btif_util.h"
#include "btif_profile_queue.h"
#include "stack_manager.h"
#include "l2cdefs.h"
#include "l2c_api.h"
#include "stack_config.h"
#include "btm_api.h"
#include "profile_config.h"
#include "btif_tws_plus.h"
#include "btif_api.h"
#include "device/include/controller.h"

#if TEST_APP_INTERFACE == TRUE
#include <bt_testapp.h>
#endif

#define BTA_SERVICE_ID_TO_SERVICE_MASK(id)  ((tBTA_SERVICE_MASK)1 << (id))
#define CALLBACK_TIMER_PERIOD_MS      (60000)
#define BTIF_VENDOR_BREDR_CLEANUP 1
#define BTIF_VENDOR_HCI_CLOSE 2

typedef struct {
  RawAddress bd_addr; /* BD address peer device. */
  uint16_t error;
  uint16_t error_info;
  uint32_t event_mask;
  uint8_t power_level;
  int8_t rssi;
  uint8_t link_quality;
  uint16_t glitch_count;
  uint8_t lmp_ver;
  uint16_t lmp_subver;
  uint16_t manufacturer_id;
  bool is_valid;
} BTIF_VND_IOT_INFO_CB_DATA;

extern bt_status_t btif_in_execute_service_request(tBTA_SERVICE_ID service_id,
                                               bool b_enable);
extern bt_status_t btif_storage_get_remote_device_property(const RawAddress *remote_bd_addr,
                                               bt_property_t *property);
extern tBTM_STATUS BTM_ReadRemoteVersion(const RawAddress& addr, uint8_t* lmp_version,
                                  uint16_t* manufacturer,
                                  uint16_t* lmp_sub_version);
extern bool interface_ready(void);
extern void set_prop_callouts(bt_property_callout_t *callouts);
void btif_vendor_cleanup_iot_broadcast_timer(void);

btvendor_callbacks_t *bt_vendor_callbacks = NULL;
static alarm_t *broadcast_cb_timer = NULL;
static void btif_broadcast_timer_cb(UNUSED_ATTR void *data);
BTIF_VND_IOT_INFO_CB_DATA broadcast_cb_data;

#if TEST_APP_INTERFACE == TRUE
extern const btl2cap_interface_t *btif_l2cap_get_interface(void);
extern const btrfcomm_interface_t *btif_rfcomm_get_interface(void);
extern const btgatt_test_interface_t *btif_gatt_test_get_interface(void);
extern const btsmp_interface_t *btif_smp_get_interface(void);
extern const btgap_interface_t *btif_gap_get_interface(void);
#endif

/*******************************************************************************
** VENDOR INTERFACE FUNCTIONS
*******************************************************************************/

/*******************************************************************************
**
** Function         btif_vendor_init
**
** Description     initializes the vendor interface
**
** Returns         bt_status_t
**
*******************************************************************************/
static bt_status_t init( btvendor_callbacks_t* callbacks)
{
    bt_vendor_callbacks = callbacks;
    broadcast_cb_timer = alarm_new("btif_vnd.cb_timer");
    LOG_INFO(LOG_TAG,"init");
    LOG_INFO(LOG_TAG,"init done");
    return BT_STATUS_SUCCESS;
}

static void btif_vendor_bredr_cleanup_event(uint16_t event, char *p_param)
{
    tBTA_SERVICE_MASK service_mask;
    uint32_t i;
    service_mask = btif_get_enabled_services_mask();
    for (i = 0; i <= BTA_MAX_SERVICE_ID; i++)
    {
        if (i != BTA_BLE_SERVICE_ID && (service_mask &
              (tBTA_SERVICE_MASK)(BTA_SERVICE_ID_TO_SERVICE_MASK(i))))
        {
            btif_reset_service(i);
            btif_in_execute_service_request(i, FALSE);
        }
    }
    btif_queue_release();
    btif_dm_bredr_disable();
    BTA_DmBredrCleanup();
    HAL_CBACK(bt_vendor_callbacks, bredr_cleanup_cb, true);
}

static void btif_vendor_hci_close_event(uint16_t event, char *p_param)
{
    btif_hci_close();
}
static void btif_vendor_send_iot_info_cb(uint16_t event, char *p_param)
{
    broadcast_cb_data.is_valid = false;
    HAL_CBACK(bt_vendor_callbacks, iot_device_broadcast_cb,
            &broadcast_cb_data.bd_addr, broadcast_cb_data.error,
            broadcast_cb_data.error_info, broadcast_cb_data.event_mask,
            broadcast_cb_data.lmp_ver, broadcast_cb_data.lmp_subver,
            broadcast_cb_data.manufacturer_id, broadcast_cb_data.power_level,
            broadcast_cb_data.rssi, broadcast_cb_data.link_quality,
            broadcast_cb_data.glitch_count);
}

void btif_vendor_update_add_on_features() {
    uint8_t add_on_features_len = 0;
    bt_vendor_property_t vnd_prop;
    char buf[8];
    vnd_prop.len = 0;
    const controller_t* controller = controller_get_interface();
    if(controller) {
        const bt_device_features_t* dev_features = controller->get_add_on_features(
                                    &add_on_features_len);

        vnd_prop.type = BT_VENDOR_PROPERTY_SOC_ADD_ON_FEATURES;
        vnd_prop.val = (void*)buf;
        if(dev_features && add_on_features_len > 0) {
            vnd_prop.len = add_on_features_len;
            memcpy(vnd_prop.val, dev_features, add_on_features_len);
        }
        HAL_CBACK(bt_vendor_callbacks, adapter_vendor_prop_cb,
                               BT_STATUS_SUCCESS, 1, &vnd_prop);
    }
}

void btif_broadcast_timer_cb(UNUSED_ATTR void *data) {
    btif_transfer_context(btif_vendor_send_iot_info_cb, 1, NULL, 0, NULL);
}

void btif_vendor_cleanup_iot_broadcast_timer()
{
    if(broadcast_cb_timer) {
        alarm_free(broadcast_cb_timer);
        broadcast_cb_timer = NULL;
    }
}

static void btif_vendor_get_remote_version(const RawAddress* bd_addr,
        uint8_t* lmp_version, uint16_t* manufacturer, uint16_t* lmp_sub_version)
{
    bt_property_t prop;
    bt_remote_version_t info;
    uint8_t tmp_lmp_ver = 0;
    uint16_t tmp_manufacturer = 0;
    uint16_t tmp_lmp_subver = 0;
    tBTM_STATUS status;

    if (bd_addr == NULL)
        return;

    status = BTM_ReadRemoteVersion(*bd_addr, &tmp_lmp_ver, &tmp_manufacturer, &tmp_lmp_subver);
    if (status == BTM_SUCCESS && (tmp_lmp_ver || tmp_manufacturer || tmp_lmp_subver)) {
        if (lmp_version) *lmp_version = tmp_lmp_ver;
        if (manufacturer) *manufacturer = tmp_manufacturer;
        if (lmp_sub_version) *lmp_sub_version = tmp_lmp_subver;
        return;
    }

    prop.type = BT_PROPERTY_REMOTE_VERSION_INFO;
    prop.len = sizeof(bt_remote_version_t);
    prop.val = (void*)&info;

    if (btif_storage_get_remote_device_property(bd_addr, &prop) ==
            BT_STATUS_SUCCESS) {
        if (lmp_version) *lmp_version = (uint8_t)info.version;
        if (manufacturer) *manufacturer = (uint16_t)info.manufacturer;
        if (lmp_sub_version) *lmp_sub_version = (uint16_t)info.sub_ver;
    }
}

void btif_vendor_iot_device_broadcast_event(RawAddress* bd_addr,
                uint16_t error, uint16_t error_info, uint32_t event_mask,
                uint8_t power_level, int8_t rssi, uint8_t link_quality,
                uint16_t glitch_count)
{
    uint8_t lmp_ver = 0;
    uint16_t lmp_subver = 0;
    uint16_t manufacturer_id = 0;

    btif_vendor_get_remote_version(bd_addr, &lmp_ver, &manufacturer_id, &lmp_subver);
    if((error == BT_SOC_A2DP_GLITCH || error == BT_HOST_A2DP_GLITCH) && (glitch_count == 0))
    {
        if(broadcast_cb_data.is_valid)
        {
            broadcast_cb_data.error_info = broadcast_cb_data.error_info|error_info;
            if(error == BT_SOC_A2DP_GLITCH && broadcast_cb_data.error == BT_HOST_A2DP_GLITCH)
            {
                broadcast_cb_data.event_mask = broadcast_cb_data.event_mask|event_mask;
                broadcast_cb_data.power_level = power_level;
                broadcast_cb_data.rssi = rssi;
                broadcast_cb_data.link_quality = link_quality;
            }
            broadcast_cb_data.glitch_count += glitch_count;
            return;
        }
        else if(broadcast_cb_timer)
        {
            broadcast_cb_data.bd_addr = *bd_addr;
            broadcast_cb_data.error = error;
            broadcast_cb_data.error_info = error_info;
            broadcast_cb_data.event_mask = event_mask;
            broadcast_cb_data.power_level = power_level;
            broadcast_cb_data.rssi = rssi;
            broadcast_cb_data.link_quality = link_quality;
            broadcast_cb_data.glitch_count = glitch_count;
            broadcast_cb_data.lmp_ver = lmp_ver;
            broadcast_cb_data.lmp_subver = lmp_subver;
            broadcast_cb_data.manufacturer_id = manufacturer_id;
            broadcast_cb_data.is_valid = true;

            alarm_set(broadcast_cb_timer, CALLBACK_TIMER_PERIOD_MS, btif_broadcast_timer_cb, NULL);
            return;
        }
    }

    HAL_CBACK(bt_vendor_callbacks, iot_device_broadcast_cb, bd_addr,
            error, error_info, event_mask, lmp_ver, lmp_subver,
            manufacturer_id, power_level, rssi, link_quality,
            glitch_count);
}

void btif_vendor_bqr_delivery_event(const RawAddress* bd_addr, const uint8_t* bqr_raw_data,
        uint32_t bqr_raw_data_len)
{
    if (bd_addr == NULL) {
        LOG_ERROR(LOG_TAG, "%s: addr is null", __func__);
        return;
    }

    if (bqr_raw_data == NULL) {
        LOG_ERROR(LOG_TAG, "%s: bqr data is null", __func__);
        return;
    }

    std::vector<uint8_t> raw_data;
    raw_data.insert(raw_data.begin(), bqr_raw_data, bqr_raw_data + bqr_raw_data_len);

    uint8_t lmp_ver = 0;
    uint16_t lmp_subver = 0;
    uint16_t manufacturer_id = 0;
    btif_vendor_get_remote_version(bd_addr, &lmp_ver, &manufacturer_id, &lmp_subver);

    LOG_INFO(LOG_TAG, "%s: len: %d, addr: %s, lmp_ver: %d, manufacturer_id: %d, lmp_subver: %d",
            __func__, bqr_raw_data_len, bd_addr->ToString().c_str(), lmp_ver,
            manufacturer_id, lmp_subver);

    do_in_jni_thread(
        FROM_HERE,
        base::Bind(
            [](RawAddress addr, uint8_t lmp_ver, uint16_t lmp_subver, uint16_t manufacturer_id,
                std::vector<uint8_t> raw_data) {
                HAL_CBACK(bt_vendor_callbacks, bqr_delivery_cb, &addr,
                    lmp_ver, lmp_subver, manufacturer_id, std::move(raw_data));
            },
            *bd_addr, lmp_ver, lmp_subver, manufacturer_id, std::move(raw_data)));
}

static void bredrstartup(void)
{
    LOG_INFO(LOG_TAG,"bredrstartup");
    BTA_DmBredrStartup();
}

static void bredrcleanup(void)
{
    LOG_INFO(LOG_TAG,"bredrcleanup");
    btif_transfer_context(btif_vendor_bredr_cleanup_event,BTIF_VENDOR_BREDR_CLEANUP,
                          NULL, 0, NULL);
}

static void hciclose(void)
{
    LOG_INFO(LOG_TAG,"hciclose");
    btif_transfer_context(btif_vendor_hci_close_event,BTIF_VENDOR_HCI_CLOSE,
                          NULL, 0, NULL);
}


#if HCI_RAW_CMD_INCLUDED == TRUE
// Callback invoked on receiving HCI event
static void btif_vendor_hci_event_callback ( tBTM_RAW_CMPL *p)
{
  if((p != NULL) && (bt_vendor_callbacks!= NULL)
      && (bt_vendor_callbacks->hci_event_recv_cb != NULL)) {

    BTIF_TRACE_DEBUG("%s", __FUNCTION__);
    HAL_CBACK(bt_vendor_callbacks, hci_event_recv_cb, p->event_code, p->p_param_buf,
                                                                p->param_len);
  }
}
#endif

int hci_cmd_send(uint16_t opcode, uint8_t* buf, uint8_t len)
{
    BTIF_TRACE_DEBUG("hci_cmd_send");
    return BTA_DmHciRawCommand(opcode, len, buf, btif_vendor_hci_event_callback);
}

static void set_wifi_state(bool status)
{
    LOG_INFO(LOG_TAG,"setWifiState :%d", status);
    BTA_DmSetWifiState(status);
}

static void set_Power_back_off_state(bool status)
{
    LOG_INFO(LOG_TAG,"setPowerBackOffState :%d ", status);
    BTA_DmPowerBackOff(status);
}

static bool get_profile_info(profile_t profile, profile_info_t feature_name)
{
    LOG_INFO(LOG_TAG,"get_profile_info :%d", profile);
    return profile_feature_fetch(profile,feature_name);
}

static void set_property_callouts(bt_property_callout_t* property_callouts)
{
    set_prop_callouts(property_callouts);
}

static void cleanup(void)
{
    LOG_INFO(LOG_TAG,"cleanup");
    if (bt_vendor_callbacks)
        bt_vendor_callbacks = NULL;
}

static void set_voip_network_type_wifi_hci_cmd_complete(tBTM_VSC_CMPL* p_data)
{
    LOG_INFO(LOG_TAG,"In set_voip_network_type_wifi_hci_cmd_complete");
    uint8_t         *stream,  status, subcmd;
    uint16_t        opcode, length;

    if (p_data && (stream = (uint8_t*)p_data->p_param_buf))
    {
        opcode = p_data->opcode;
        length = p_data->param_len;
        STREAM_TO_UINT8(status, stream);
        STREAM_TO_UINT8(subcmd, stream);
        BTIF_TRACE_DEBUG("%s opcode = 0x%04X, length = %d, status = %d, subcmd = %d",
                __FUNCTION__, opcode, length, status, subcmd);
        if (status == HCI_SUCCESS)
        {
            BTIF_TRACE_DEBUG("btm_SetVoipNetworkTypeWifi status success");
        }
    }
}

/*******************************************************************************
**
** Function         voip_network_type_wifi
**
** Description      BT app updates the connectivity network used for VOIP as Wifi
**
** Returns          bt_status_t
**
*******************************************************************************/
bt_status_t voip_network_type_wifi(bthf_voip_state_t isVoipStarted,
                                           bthf_voip_call_network_type_t isNetworkWifi)
{
    LOG_INFO(LOG_TAG,"In voip_network_type_wifi");
    uint8_t           cmd[3], *p_cursor;
    uint8_t           sub_cmd = HCI_VSC_SUBCODE_VOIP_NETWORK_WIFI;

    p_cursor = cmd;
    memset(cmd, 0, 3);

    *p_cursor++ = sub_cmd;
    *p_cursor++ = isVoipStarted;
    *p_cursor++ = isNetworkWifi;

    BTM_VendorSpecificCommand(HCI_VSC_VOIP_NETWORK_WIFI_OCF, sizeof(cmd),
            cmd, set_voip_network_type_wifi_hci_cmd_complete);
    return BT_STATUS_SUCCESS;
}

/*******************************************************************************
**
** Function         clock_sync_cback
**
** Description      callback function for set_clock_sync_config and start_clock_sync
**
** Returns          None
**
*******************************************************************************/
static void clock_sync_cback(tBTM_VSC_CMPL *param)
{
    CHECK(param->param_len > 1);

    BTIF_TRACE_DEBUG("%s: opcode=%x, subopcode=%x, status=%d",
        __func__, param->opcode, param->p_param_buf[1], param->p_param_buf[0]);
}

/*******************************************************************************
**
** Function         set_clock_sync_config
**
** Description      Enable/Disable clock sync protocol and config clock sync paramters
**
**                  enable        : 0 - disable, 1 -enable
**                  mode          : 0x00 - GPIO sync, 0x01 - VSC sync
**                  adv_internal  : advertising interval, 0xA0 ~ 0x4000
**                  channel       : BIT0: channel 37, BIT1: channel 38, BIT2: channel 39
**                  jitter        : 0~8, 0 - random jitter, other - (jitter-1)*1.25
**                  offset        : -32768~32767us, timing between sync pulse and advert,
**                                  Positive value move the sync edge close to the advert
**                                  and therefore make the offset smaller.
**
** Return           None
**
*******************************************************************************/
static bool set_clock_sync_config(bool enable, int mode, int adv_interval,
    int channel, int jitter, int offset)
{
    uint16_t opcode = 0xfc35;
    uint8_t cmdbuf[128], *ptr = cmdbuf;
    uint32_t cmdlen = 0;

    BTIF_TRACE_DEBUG("%s", __func__);
    if (mode != 0 && mode != 1) {
      BTIF_TRACE_DEBUG("%s: invalid mode %d", __func__, mode);
      return false;
    }

    *ptr++ = (uint8_t)0; // subcode
    cmdlen ++;

    *ptr++ = (uint8_t) enable;  // enable
    cmdlen ++;

    *ptr++ = (uint8_t) mode;  // mode - 00: GPIO, 1: VSC
    cmdlen ++;

    if (adv_interval < 0xa0)
      adv_interval = 0xa0;
    if (adv_interval > 0x4000)
      adv_interval = 0x4000;
    *(uint16_t *)ptr = (uint16_t) adv_interval; // advertise interval
    ptr +=2;
    cmdlen += 2;

    channel &= 0x7;
    *ptr++ = (uint8_t) channel; // channel
    cmdlen ++;

    if (jitter < 0)
      jitter = 0;
    if (jitter > 8)
      jitter = 8;
    *ptr++ = (uint8_t) jitter;  // jitter
    cmdlen ++;

    if (offset < -32768)
      offset = -32768;
    if (offset > 32767)
      offset = 32767;
    *(uint16_t *)ptr = (uint16_t) offset; // offset
    ptr += 2;
    cmdlen += 2;

    BTM_VendorSpecificCommand(opcode, cmdlen, cmdbuf, clock_sync_cback);
    return true;
}

/*******************************************************************************
**
** Function         start_clock_sync
**
** Description      start clock sync
**
** Return           None
**
*******************************************************************************/
static void start_clock_sync(void)
{
    uint16_t opcode = 0xfc35;
    uint8_t cmdbuf[] = {0x01}; // subcode

    BTIF_TRACE_DEBUG("%s", __func__);
    BTM_VendorSpecificCommand(opcode, 1, cmdbuf, clock_sync_cback);
}

/*******************************************************************************
**
** Function         get_testapp_interface
**
** Description      Get the Test interface
**
** Returns          btvendor_interface_t
**
*******************************************************************************/
#if TEST_APP_INTERFACE == TRUE
static const void* get_testapp_interface(int test_app_profile)
{
    if (interface_ready() == FALSE) {
        return NULL;
    }
    ALOGI("get_testapp_interface %d", test_app_profile);
    switch(test_app_profile) {
        case TEST_APP_L2CAP:
            return btif_l2cap_get_interface();
        case TEST_APP_RFCOMM:
            return btif_rfcomm_get_interface();
        case TEST_APP_GATT:
           return btif_gatt_test_get_interface();
        case TEST_APP_SMP:
           return btif_smp_get_interface();
        case TEST_APP_GAP:
           return btif_gap_get_interface();
        default:
            return NULL;
    }
    return NULL;
}
#endif

static const btvendor_interface_t btvendorInterface = {
    sizeof(btvendorInterface),
    init,
    hci_cmd_send,
#if TEST_APP_INTERFACE == TRUE
    get_testapp_interface,
#else
    NULL,
#endif
    bredrcleanup,
    bredrstartup,
    set_wifi_state,
    set_Power_back_off_state,
    get_profile_info,
    set_property_callouts,
    cleanup,
    voip_network_type_wifi,
    hciclose,
    set_clock_sync_config,
    start_clock_sync,
};

/*******************************************************************************
** LOCAL FUNCTIONS
*******************************************************************************/

/*******************************************************************************
**
** Function         btif_vendor_get_interface
**
** Description      Get the vendor callback interface
**
** Returns          btvendor_interface_t
**
*******************************************************************************/
const btvendor_interface_t *btif_vendor_get_interface()
{
    BTIF_TRACE_EVENT("%s", __FUNCTION__);
    return &btvendorInterface;
}
