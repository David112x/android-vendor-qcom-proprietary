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
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_INCLUDE_BT_VENDOR_H
#define ANDROID_INCLUDE_BT_VENDOR_H

#include <hardware/bluetooth.h>
#include <vector>

__BEGIN_DECLS

#define BT_PROFILE_VENDOR_ID "vendor"

// Profile related Enums & function
typedef enum {
  AVRCP_ID = 1,
  PBAP_ID,
  MAP_ID,
  MAX_POW_ID,
  END_OF_PROFILE_LIST
} profile_t;


typedef enum {
 VERSION = 1,
 AVRCP_COVERART_SUPPORT,
 AVRCP_0103_SUPPORT,
 USE_SIM_SUPPORT,
 MAP_EMAIL_SUPPORT,
 PBAP_0102_SUPPORT,
 MAP_0104_SUPPORT,
 BR_MAX_POW_SUPPORT,
 EDR_MAX_POW_SUPPORT,
 BLE_MAX_POW_SUPPORT,
 END_OF_FEATURE_LIST
 } profile_info_t;

// Vendor Callbacks
#define TWS_PLUS_DEV_TYPE_NONE      (0x00)
#define TWS_PLUS_DEV_TYPE_PRIMARY   (0x01)
#define TWS_PLUS_DEV_TYPE_SECONDARY (0x02)
typedef uint8_t tTWS_PLUS_DEVICE_TYPE;

typedef enum {
    BT_VENDOR_PROPERTY_TWS_PLUS_DEVICE_TYPE = 0x01,
    BT_VENDOR_PROPERTY_TWS_PLUS_PEER_ADDR,
    BT_VENDOR_PROPERTY_TWS_PLUS_AUTO_CONNECT,
    BT_VENDOR_PROPERTY_HOST_ADD_ON_FEATURES,
    BT_VENDOR_PROPERTY_SOC_ADD_ON_FEATURES
} bt_vendor_property_type_t;

typedef enum {
    BTHF_VOIP_CALL_NETWORK_TYPE_MOBILE = 0,
    BTHF_VOIP_CALL_NETWORK_TYPE_WIFI
} bthf_voip_call_network_type_t;

typedef enum {
    BTHF_VOIP_STATE_STOPPED = 0,
    BTHF_VOIP_STATE_STARTED
} bthf_voip_state_t;

typedef struct {
    bt_vendor_property_type_t type;
    int len;
    void *val;
} bt_vendor_property_t;

/** Callback when bredr cleanup is done.
 */
typedef void (*  btvendor_bredr_cleanup_callback)(bool status);
typedef void (*  btvendor_iot_device_broadcast_callback)(RawAddress* remote_bd_addr, uint16_t error,
                        uint16_t error_info, uint32_t event_mask, uint8_t lmp_ver, uint16_t lmp_subver,
                        uint16_t manufacturer_id, uint8_t power_level, int8_t rssi, uint8_t link_quality,
                        uint16_t glitch_count );
typedef void (* btvendor_bqr_delivery_callback)(RawAddress* remote_bd_addr,
                        uint8_t lmp_ver, uint16_t lmp_subver, uint16_t manufacturer_id,
                        std::vector<uint8_t> bqr_raw_data);
typedef void (* btvendor_bredr_cleanup_callback)(bool status);

/** Callback to notify the remote device vendor properties.
 */
typedef void (* remote_dev_prop_callback)(bt_status_t status,
                          RawAddress *bd_addr, int num_properties,
                          bt_vendor_property_t *properties);

/** Callback to handle SSR */
typedef void (* ssr_vendor_callback)(void);

/** Bluetooth HCI event Callback */
/* Receive any HCI event from controller for raw commands */
typedef void (* hci_event_recv_callback)(uint8_t event_code, uint8_t *buf, uint8_t len);

/** Callback to notify the remote device vendor properties.
 */
typedef void (* adapter_vendor_prop_callback)(bt_status_t status,
                          int num_properties,
                          bt_vendor_property_t *properties);

/** BT-Vendor callback structure. */
typedef struct {
    /** set to sizeof(BtVendorCallbacks) */
    size_t      size;
    btvendor_bredr_cleanup_callback  bredr_cleanup_cb;
    btvendor_iot_device_broadcast_callback iot_device_broadcast_cb;
    btvendor_bqr_delivery_callback bqr_delivery_cb;
    remote_dev_prop_callback         rmt_dev_prop_cb;
    hci_event_recv_callback  hci_event_recv_cb;
    adapter_vendor_prop_callback     adapter_vendor_prop_cb;
    ssr_vendor_callback         ssr_vendor_cb;
} btvendor_callbacks_t;

typedef int (*property_set_callout)(const char* key, const char* value);
typedef int (*property_get_callout)(const char* key, char* value, const char* default_value);
typedef int32_t (*property_get_int32_callout)(const char* key, int32_t default_value);

typedef struct {
  size_t size;

  property_set_callout bt_set_property;
  property_get_callout bt_get_property;
  property_get_int32_callout bt_get_property_int32;
} bt_property_callout_t;

/** Represents the standard BT-Vendor interface.
 */
typedef struct {

    /** set to sizeof(BtVendorInterface) */
    size_t  size;

    /**
     * Register the BtVendor callbacks
     */
    bt_status_t (*init)( btvendor_callbacks_t* callbacks );

    /* Send any test HCI command to the controller. */
    int (*hci_cmd_send)(uint16_t opcode, uint8_t *buf, uint8_t len);

    /** test interface. */
    const void* (*get_testapp_interface)(int);

    /** Does BREDR cleanup */
    void (*bredrcleanup)(void);

    /** Does BREDR startup */
    void (*bredrstartup)(void);

    /** set wifi state */
    void (*set_wifi_state)(bool);

    /** set Power_back_off state */
    void (*set_Power_back_off_state)(bool);

    /** get profile info */
    bool (*get_profile_info)(profile_t, profile_info_t);

    void (*set_property_callouts)(bt_property_callout_t* property_callouts);

    /** Closes the interface. */
    void  (*cleanup)( void );

    /** Sends connectivity network type used by Voip currently to stack */
    bt_status_t (*voip_network_type_wifi) (bthf_voip_state_t is_voip_started,
                                           bthf_voip_call_network_type_t is_network_wifi);
    void (*hciclose)(void);

    /** enable/disable clock sync protocol */
    bool (*set_clock_sync_config)(bool enable, int mode, int adv_interval,
        int channel, int jitter, int offset);

    /** start clock sync protocol */
    void (*start_clock_sync)(void);

} btvendor_interface_t;

__END_DECLS

#endif /* ANDROID_INCLUDE_BT_VENDOR_H */

