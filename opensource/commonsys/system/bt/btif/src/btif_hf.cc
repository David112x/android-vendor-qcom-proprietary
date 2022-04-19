/******************************************************************************
 * Copyright (C) 2017-2018, The Linux Foundation. All rights reserved.
 * Not a Contribution.
 Redistribution and use in source and binary forms, with or without
modification, are permitted (subject to the limitations in the
disclaimer below) provided that the following conditions are met:
   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above
     copyright notice, this list of conditions and the following
     disclaimer in the documentation and/or other materials provided
     with the distribution.
   * Neither the name of The Linux Foundation nor the names of its
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
******************************************************************************
 *
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

/*******************************************************************************
 *
 *  Filename:      btif_hf.c
 *
 *  Description:   Handsfree Profile Bluetooth Interface
 *
 *
 ******************************************************************************/

#define LOG_TAG "bt_btif_hf"

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <bta/include/bta_ag_api.h>
#include <hardware/bluetooth.h>
#include <hardware/bluetooth_headset_callbacks.h>
#include <hardware/bluetooth_headset_interface.h>
#include <hardware/bt_hf.h>
#include <log/log.h>

#include "bta/include/bta_ag_api.h"
#if (SWB_ENABLED == TRUE)
#include "bta_ag_swb.h"
#include <hardware/vendor_hf.h>
#endif
#include "bta/include/utl.h"
#include "bta_ag_api.h"
#include "btif_common.h"
#include "btif_hf.h"
#include "btif_profile_queue.h"
#include "btif_util.h"
#include "osi/include/properties.h"
#include <cutils/properties.h>
#include "device/include/controller.h"
#include "btif_storage.h"
#if (TWS_AG_ENABLED == TRUE)
#include "btif_tws_plus.h"
#include "btif_twsp_hf.h"
#include "bta_ag_twsp.h"
#endif
#include "device/include/device_iot_config.h"

namespace bluetooth {
namespace headset {

/*******************************************************************************
 *  Constants & Macros
 ******************************************************************************/
#ifndef BTIF_HSAG_SERVICE_NAME
#define BTIF_HSAG_SERVICE_NAME ("Headset Gateway")
#endif

#ifndef BTIF_HFAG_SERVICE_NAME
#define BTIF_HFAG_SERVICE_NAME ("Handsfree Gateway")
#endif

#ifndef BTIF_HF_SERVICES
#define BTIF_HF_SERVICES (BTA_HSP_SERVICE_MASK | BTA_HFP_SERVICE_MASK)
#endif

#ifndef BTIF_HF_SERVICE_NAMES
#define BTIF_HF_SERVICE_NAMES \
  { BTIF_HSAG_SERVICE_NAME, BTIF_HFAG_SERVICE_NAME }
#endif

#ifndef BTIF_HF_SECURITY
#define BTIF_HF_SECURITY (BTA_SEC_AUTHENTICATE | BTA_SEC_ENCRYPT)
#endif

#ifndef BTIF_HF_FEATURES
#define BTIF_HF_FEATURES                                       \
  (BTA_AG_FEAT_3WAY | BTA_AG_FEAT_ECNR | BTA_AG_FEAT_REJECT |  \
   BTA_AG_FEAT_ECS | BTA_AG_FEAT_EXTERR | BTA_AG_FEAT_VREC |   \
   BTA_AG_FEAT_HF_IND | BTA_AG_FEAT_ESCO | BTA_AG_FEAT_UNAT)
#endif

/* HF features supported at runtime */
static uint32_t btif_hf_features = BTIF_HF_FEATURES;

#define BTIF_HF_CALL_END_TIMEOUT 6

#define BTIF_HF_INVALID_IDX (-1)

/* Assigned number for mSBC codec */
#define BTA_AG_MSBC_CODEC 5

/* Max HF clients supported from App */
uint16_t btif_max_hf_clients = 1;
static RawAddress active_bda = {};

/*******************************************************************************
 *  Local type definitions
 ******************************************************************************/

/*******************************************************************************
 *  Static variables
 ******************************************************************************/
static Callbacks* bt_hf_callbacks = NULL;

#define CHECK_BTHF_INIT()                                             \
  do {                                                                \
    if (bt_hf_callbacks == NULL) {                                    \
      BTIF_TRACE_WARNING("BTHF: %s: BTHF not initialized", __func__); \
      return BT_STATUS_NOT_READY;                                     \
    } else {                                                          \
      BTIF_TRACE_EVENT("BTHF: %s", __func__);                         \
    }                                                                 \
  } while (0)

#define HAL_HF_CBACK(P_CB, P_CBACK, ...)                \
  do {                                                  \
    if (P_CB != NULL ) {                                \
      BTIF_TRACE_ERROR("HAL %s->%s", #P_CB, #P_CBACK);  \
      (P_CB)->P_CBACK(__VA_ARGS__);                     \
    } else {                                            \
      ASSERTC(0, "Callback is NULL", 0);                \
    }                                                   \
  } while (0)

btif_hf_cb_t btif_hf_cb[BTA_AG_MAX_NUM_CLIENTS];

/*******************************************************************************
 *  Static functions
 ******************************************************************************/

/*******************************************************************************
 *  Externs
 ******************************************************************************/
/* By default, even though codec negotiation is enabled, we will not use WBS as
 * the default
 * codec unless this variable is set to true.
 */
#ifndef BTIF_HF_WBS_PREFERRED
#define BTIF_HF_WBS_PREFERRED true
#endif

static bool btif_conf_hf_force_wbs = BTIF_HF_WBS_PREFERRED;
#if (TWS_AG_ENABLED == TRUE)
static bool btif_twsp_send_bvra_update = FALSE;
static int btif_twsp_bvra_peer_eb_state = BTHF_VR_STATE_STOPPED;
#endif

/*******************************************************************************
 *  Functions
 ******************************************************************************/

const char* dump_hf_call_state(bthf_call_state_t call_state) {
  switch (call_state) {
    CASE_RETURN_STR(BTHF_CALL_STATE_IDLE)
    CASE_RETURN_STR(BTHF_CALL_STATE_HELD)
    CASE_RETURN_STR(BTHF_CALL_STATE_DIALING)
    CASE_RETURN_STR(BTHF_CALL_STATE_ALERTING)
    CASE_RETURN_STR(BTHF_CALL_STATE_INCOMING)
    CASE_RETURN_STR(BTHF_CALL_STATE_WAITING)
    CASE_RETURN_STR(BTHF_CALL_STATE_ACTIVE)
    CASE_RETURN_STR(BTHF_CALL_STATE_DISCONNECTED)
    default:
      return "UNKNOWN CALL STATE";
  }
}

/**
 * Check if bd_addr is the current active device.
 *
 * @param bd_addr target device address
 * @return True if bd_addr is the current active device, False otherwise or if
 * no active device is set (i.e. active_device_addr is empty)
 */
static bool is_active_device(const RawAddress& bd_addr) {
  bool ret = false;
#if (TWS_AG_ENABLED == TRUE)
  RawAddress peer_eb_addr;
  RawAddress cur_bd_addr = bd_addr;
  if (btif_is_tws_plus_device(&bd_addr)) {
      btif_tws_plus_get_peer_eb_addr(&cur_bd_addr, &peer_eb_addr);
      ret = !active_bda.IsEmpty() &&
                 (active_bda == bd_addr || peer_eb_addr == active_bda);
  } else {
#endif
      ret = !active_bda.IsEmpty() && active_bda == bd_addr;
#if (TWS_AG_ENABLED == TRUE)
  }
#endif
  BTIF_TRACE_EVENT("%s: returns: %d", __func__, ret);
  return ret;
}

/*******************************************************************************
 *
 * Function         is_connected
 *
 * Description      Internal function to check if HF is connected
 *
 * Returns          true if connected
 *
 ******************************************************************************/
bool is_connected(RawAddress* bd_addr) {
  int i;
  for (i = 0; i < btif_max_hf_clients; ++i) {
    if (((btif_hf_cb[i].state == BTHF_CONNECTION_STATE_CONNECTED) ||
         (btif_hf_cb[i].state == BTHF_CONNECTION_STATE_SLC_CONNECTED)) &&
        (!bd_addr || *bd_addr == btif_hf_cb[i].connected_bda))
      return true;
  }
  return false;
}

/*******************************************************************************
 *
 * Function         btif_hf_idx_by_bdaddr
 *
 * Description      Internal function to get idx by bdaddr
 *
 * Returns          idx
 *
 ******************************************************************************/
static int btif_hf_idx_by_bdaddr(RawAddress* bd_addr) {
  int i;
  for (i = 0; i < btif_max_hf_clients; ++i) {
    if ( (btif_hf_cb[i].state == BTHF_CONNECTION_STATE_CONNECTED ||
          btif_hf_cb[i].state == BTHF_CONNECTION_STATE_SLC_CONNECTED) &&
         (*bd_addr == btif_hf_cb[i].connected_bda))
      return i;
  }
  return BTIF_HF_INVALID_IDX;
}

/*******************************************************************************
 *
 * Function         callstate_to_callsetup
 *
 * Description      Converts HAL call state to BTA call setup indicator value
 *
 * Returns          BTA call indicator value
 *
 ******************************************************************************/
static uint8_t callstate_to_callsetup(bthf_call_state_t call_state) {
  uint8_t call_setup = 0;
  if (call_state == BTHF_CALL_STATE_INCOMING) call_setup = 1;
  if (call_state == BTHF_CALL_STATE_DIALING) call_setup = 2;
  if (call_state == BTHF_CALL_STATE_ALERTING) call_setup = 3;

  return call_setup;
}

/*******************************************************************************
 *
 * Function         send_at_result
 *
 * Description      Send AT result code (OK/ERROR)
 *
 * Returns          void
 *
 ******************************************************************************/
static void send_at_result(uint8_t ok_flag, uint16_t errcode, int idx) {
  tBTA_AG_RES_DATA ag_res;
  memset(&ag_res, 0, sizeof(ag_res));

  ag_res.ok_flag = ok_flag;
  if (ok_flag == BTA_AG_OK_ERROR) {
    ag_res.errcode = errcode;
  }

  BTA_AgResult(btif_hf_cb[idx].handle, BTA_AG_UNAT_RES, &ag_res);
}

/*******************************************************************************
 *
 * Function         send_indicator_update
 *
 * Description      Send indicator update (CIEV)
 *
 * Returns          void
 *
 ******************************************************************************/
static void send_indicator_update(const btif_hf_cb_t& control_block,
                                  uint16_t indicator, uint16_t value) {
  tBTA_AG_RES_DATA ag_res;

  memset(&ag_res, 0, sizeof(tBTA_AG_RES_DATA));
  ag_res.ind.id = indicator;
  ag_res.ind.value = value;

  BTA_AgResult(control_block.handle, BTA_AG_IND_RES, &ag_res);
}

void clear_phone_state_multihf(int idx) {
  btif_hf_cb[idx].call_setup_state = BTHF_CALL_STATE_IDLE;
  btif_hf_cb[idx].num_active = btif_hf_cb[idx].num_held = 0;
}

/**
 * Check if Service Level Connection (SLC) is established for bd_addr
 *
 * @param bd_addr remote device address
 * @return true if SLC is established for bd_addr
 */
static bool IsSlcConnected(RawAddress* bd_addr) {
  if (!bd_addr) {
    LOG(WARNING) << __func__ << ": bd_addr is null";
    return false;
  }
  int idx = btif_hf_idx_by_bdaddr(bd_addr);
  if (idx < 0 || idx >= BTA_AG_MAX_NUM_CLIENTS) {
    LOG(WARNING) << __func__ << ": invalid index " << idx << " for "
                 << *bd_addr;
    return false;
  }
  return btif_hf_cb[idx].state == BTHF_CONNECTION_STATE_SLC_CONNECTED;
}

/*******************************************************************************
 *
 * Function         btif_hf_check_if_sco_connected
 *
 * Description      Returns BT_STATUS_SUCCESS if SCO is up for any HF
 *
 * Returns          bt_status_t
 *
 ******************************************************************************/
bt_status_t btif_hf_check_if_sco_connected() {
  if (bt_hf_callbacks == NULL) {
    BTIF_TRACE_WARNING("BTHF: %s(): BTHF not initialized. ", __func__);
    return BT_STATUS_NOT_READY;
  } else {
    for (int i = 0; i < btif_max_hf_clients; i++) {
      if ((btif_hf_cb[i].audio_state == BTHF_AUDIO_STATE_CONNECTED) ||
             (btif_hf_cb[i].audio_state == BTHF_AUDIO_STATE_CONNECTING)) {
        BTIF_TRACE_EVENT("BTHF: %s(): sco connected/connecting for idx = %d",
                         __func__, i);
        return BT_STATUS_SUCCESS;
      }
    }
    BTIF_TRACE_WARNING("BTHF: %s(): No SCO connection up", __func__);
    return BT_STATUS_NOT_READY;
  }
}

/*****************************************************************************
 *   Section name (Group of functions)
 ****************************************************************************/

/*****************************************************************************
 *
 *   btif hf api functions (no context switch)
 *
 ****************************************************************************/

/*******************************************************************************
 *
 * Function         btif_hf_upstreams_evt
 *
 * Description      Executes HF UPSTREAMS events in btif context
 *
 * Returns          void
 *
 ******************************************************************************/
static void btif_hf_upstreams_evt(uint16_t event, char* p_param) {
  tBTA_AG* p_data = (tBTA_AG*)p_param;
  int idx;
  bool ignore_rfc_fail = false;
  RawAddress bd_addr;
#if (TWS_AG_ENABLED == TRUE)
  RawAddress peer_eb_addr;
  int peer_eb_dev_type;
#endif

  BTIF_TRACE_IMP("%s: event=%s", __func__, dump_hf_event(event));
  // for BTA_AG_ENABLE_EVT/BTA_AG_DISABLE_EVT, p_data is NULL
  if (event == BTA_AG_ENABLE_EVT || event == BTA_AG_DISABLE_EVT)
    return;

  // p_data is NULL for any other event, return
  if (p_data == NULL)
  {
    BTIF_TRACE_ERROR("%s: data is NULL", __FUNCTION__);
    return;
  }

  idx = p_data->hdr.handle - 1;
  if ((idx < 0) || (idx >= BTA_AG_MAX_NUM_CLIENTS)) {
    BTIF_TRACE_ERROR("%s: Invalid index %d", __func__, idx);
    return;
  }

  BTIF_TRACE_DEBUG("%s:idx:%d",__func__, idx);
  switch (event) {
    case BTA_AG_REGISTER_EVT:
      btif_hf_cb[idx].handle = p_data->reg.hdr.handle;
      BTIF_TRACE_DEBUG(
          "%s: BTA_AG_REGISTER_EVT,"
          "btif_hf_cb.handle = %d",
          __func__, btif_hf_cb[idx].handle);
      break;

    case BTA_AG_OPEN_EVT:
      BTIF_TRACE_DEBUG("%s:p_data->open.status:%d,btif_hf_cb[idx].state:%d,btif_max_hf_clients:%d",
                         __func__, p_data->open.status, btif_hf_cb[idx].state, btif_max_hf_clients);
      BTIF_TRACE_DEBUG("%s: service_id: %d", __func__, p_data->open.service_id);
      if (p_data->open.status == BTA_AG_SUCCESS) {
        btif_hf_cb[idx].connected_bda = p_data->open.bd_addr;
#if (BT_IOT_LOGGING_ENABLED == TRUE)
        if (btif_hf_cb[idx].state != BTHF_CONNECTION_STATE_CONNECTING) {
          device_iot_config_addr_set_int(btif_hf_cb[idx].connected_bda,
              IOT_CONF_KEY_HFP_ROLE, IOT_CONF_VAL_HFP_ROLE_CLIENT);
          device_iot_config_addr_int_add_one(btif_hf_cb[idx].connected_bda,
              IOT_CONF_KEY_HFP_SLC_CONN_COUNT);
        }
#endif
        btif_hf_cb[idx].state = BTHF_CONNECTION_STATE_CONNECTED;
        btif_hf_cb[idx].peer_feat = 0;
        clear_phone_state_multihf(idx);
        btif_hf_cb[idx].service_id = p_data->open.service_id;
      } else if (btif_hf_cb[idx].state == BTHF_CONNECTION_STATE_CONNECTING) {
        /* In Multi-hf, if outgoing RFCOMM connection fails due to collision,
         * ignore the failure if HF is already connected.
         */
        if ( (btif_max_hf_clients > 1) &&
             (p_data->open.status == BTA_AG_FAIL_RFCOMM) &&
             (is_connected(&btif_hf_cb[idx].connected_bda)) )
        {
          BTIF_TRACE_WARNING("%s: Ignoring RFCOMM failure due to collision for dev %s",
                        __func__, btif_hf_cb[idx].connected_bda.ToString().c_str());
          ignore_rfc_fail = true;
        }

#if (BT_IOT_LOGGING_ENABLED == TRUE)
        if (!ignore_rfc_fail) {
          device_iot_config_addr_int_add_one(btif_hf_cb[idx].connected_bda,
              IOT_CONF_KEY_HFP_SLC_CONN_FAIL_COUNT);
        }
#endif
        LOG(ERROR) << __func__ << ": AG open failed for "
                   << btif_hf_cb[idx].connected_bda << ", status "
                   << unsigned(p_data->open.status);
        btif_hf_cb[idx].state = BTHF_CONNECTION_STATE_DISCONNECTED;
      } else {
        LOG(WARNING) << __func__ << ": AG open failed for "
                     << p_data->open.bd_addr << ", error "
                     << std::to_string(p_data->open.status)
                     << ", local device is " << btif_hf_cb[idx].connected_bda
                     << ". Ignoring as not expecting to open";
        break;
      }
      if (ignore_rfc_fail != true)
      {
        VLOG(1) << __func__ << "btif_hf_cb[idx].connected_bda:" << btif_hf_cb[idx].connected_bda;
        HAL_HF_CBACK(bt_hf_callbacks, ConnectionStateCallback, btif_hf_cb[idx].state,
                 &btif_hf_cb[idx].connected_bda);
      }
#if (TWS_AG_ENABLED == TRUE)
      bd_addr = btif_hf_cb[idx].connected_bda;
      //if the ACL connected for earbud and if peer is not connectec yet
      //designate connected as primary and peer as secondary
      btif_tws_plus_get_peer_eb_addr(&bd_addr, &peer_eb_addr);
      BTIF_TRACE_DEBUG("AG_OPEN : addr %s", bd_addr.ToString().c_str());

      BTIF_TRACE_DEBUG("AG_OPEN : peer bd addr %s", peer_eb_addr.ToString().c_str());
      //conn_state = btif_dm_get_connection_state(&peer_eb_addr);
      if (btif_is_tws_plus_device(&bd_addr) && !(is_connected(&peer_eb_addr))) {
          bool ret = btif_tws_plus_set_dev_type(&bd_addr, TWS_PLUS_DEV_TYPE_PRIMARY);
          ASSERTC(ret == TRUE, "Making TWS_PLUS dev type as primary failed", ret);
          ret = btif_tws_plus_set_dev_type(&peer_eb_addr, TWS_PLUS_DEV_TYPE_SECONDARY);
          ASSERTC(ret == TRUE, "Adding TWS_PLUS dev type failed", ret);
          BTIF_TRACE_DEBUG("%s: peer TWS_PLUS device is designated as SECONDARY_EB", __func__);
      }
#endif
      bd_addr = btif_hf_cb[idx].connected_bda;
      if (btif_hf_cb[idx].state == BTHF_CONNECTION_STATE_DISCONNECTED)
        btif_hf_cb[idx].connected_bda = RawAddress::kAny;

      if (p_data->open.status != BTA_AG_SUCCESS)
        btif_queue_advance_by_uuid(UUID_SERVCLASS_AG_HANDSFREE, &bd_addr);
      break;

    case BTA_AG_CLOSE_EVT:
#if (BT_IOT_LOGGING_ENABLED == TRUE)
      if (btif_hf_cb[idx].state == BTHF_CONNECTION_STATE_CONNECTED) {
        device_iot_config_addr_int_add_one(btif_hf_cb[idx].connected_bda,
            IOT_CONF_KEY_HFP_SLC_CONN_FAIL_COUNT);
      }
#endif

      btif_hf_cb[idx].state = BTHF_CONNECTION_STATE_DISCONNECTED;

      BTIF_TRACE_DEBUG("%s: Moving the audio_state to DISCONNECTED for device %s",
                       __FUNCTION__, btif_hf_cb[idx].connected_bda.ToString().c_str());
      btif_hf_cb[idx].audio_state = BTHF_AUDIO_STATE_DISCONNECTED;
#if (TWS_AG_ENABLED == TRUE)
      btif_hf_cb[idx].twsp_state = TWSPLUS_EB_STATE_UNKNOWN;
#endif
      BTIF_TRACE_DEBUG(
          "%s: BTA_AG_CLOSE_EVT,"
          "idx = %d, btif_hf_cb.handle = %d",
          __func__, idx, btif_hf_cb[idx].handle);

      /* Ignore AG Close event if HF is conntected via another Rfcomm DLC connection
         due to collision */
      if (!((btif_max_hf_clients > 1) && (is_connected(&btif_hf_cb[idx].connected_bda))))
      {
        HAL_HF_CBACK(bt_hf_callbacks, ConnectionStateCallback, btif_hf_cb[idx].state,
                  &btif_hf_cb[idx].connected_bda);
      }
#if (TWS_AG_ENABLED == TRUE)
      bd_addr = btif_hf_cb[idx].connected_bda;
      //if the ACL disconnected for earbud and if that was designated as primary
      //check if the secondary earbud is connected, if connected set that as
      //primary
      btif_tws_plus_get_peer_eb_addr(&bd_addr, &peer_eb_addr);
      btif_tws_plus_get_dev_type(&bd_addr, &peer_eb_dev_type);
      BTIF_TRACE_DEBUG("peer dev type : %d\n", peer_eb_dev_type);
      if(btif_is_tws_plus_device(&bd_addr) && peer_eb_dev_type == TWS_PLUS_DEV_TYPE_PRIMARY) {
        bool ret = btif_tws_plus_set_dev_type(&bd_addr, TWS_PLUS_DEV_TYPE_SECONDARY);
        //conn_state = btif_dm_get_connection_state(&peer_eb_addr);
        ASSERTC(ret == TRUE, "Making TWS_PLUS dev type as secondary failed", ret);
        if (is_connected(&peer_eb_addr)) {
            bool ret = btif_tws_plus_set_dev_type(&peer_eb_addr, TWS_PLUS_DEV_TYPE_PRIMARY);
            ASSERTC(ret == TRUE, "Adding TWS_PLUS dev type failed", ret);
            BTIF_TRACE_DEBUG("%s: TWS_PLUS device is designated as PRIMARY_EB", __func__);
        }
      }
#endif
      bd_addr = btif_hf_cb[idx].connected_bda;
      btif_hf_cb[idx].connected_bda = RawAddress::kAny;
      btif_hf_cb[idx].peer_feat = 0;
      clear_phone_state_multihf(idx);
      /* Not clear active device if HFP is conntected via another Rfcomm DLC connection
         due to collision */
      if (!((btif_max_hf_clients > 1) && (is_connected(&bd_addr)))) {
        //If the active device is disconnected, clear the active device
        if (is_active_device(bd_addr)) {
          bool is_twsp_dev = btif_is_tws_plus_device(&bd_addr);
          /*Clear active device only if given tws+ is active*/
          if (!is_twsp_dev || (is_twsp_dev && active_bda == bd_addr)) {
              active_bda = RawAddress::kEmpty;
              BTIF_TRACE_IMP("%s: Active device is disconnected, clear the active device %s",
                  __func__, active_bda.ToString().c_str());
              BTA_AgSetActiveDevice(active_bda);
          } else {
              BTIF_TRACE_IMP("%s:non-active TWS+ device disconnected",__func__);
          }
        }
      }
      /* If AG_OPEN was received but SLC was not setup in a specified time (10
       *seconds),
       ** then AG_CLOSE may be received. We need to advance the queue here
       */
      btif_queue_advance_by_uuid(UUID_SERVCLASS_AG_HANDSFREE, &bd_addr);
      break;

    case BTA_AG_CONN_EVT:
      BTIF_TRACE_DEBUG("%s: BTA_AG_CONN_EVT, idx = %d ", __func__, idx);

#if (BT_IOT_LOGGING_ENABLED == TRUE)
      device_iot_config_addr_set_hex(btif_hf_cb[idx].connected_bda, IOT_CONF_KEY_HFP_CODECTYPE,
          p_data->conn.peer_codec == 0x03 ? IOT_CONF_VAL_HFP_CODECTYPE_CVSDMSBC :
          IOT_CONF_VAL_HFP_CODECTYPE_CVSD, IOT_CONF_BYTE_NUM_1);
      device_iot_config_addr_set_hex(btif_hf_cb[idx].connected_bda, IOT_CONF_KEY_HFP_FEATURES,
          p_data->conn.peer_feat, IOT_CONF_BYTE_NUM_2);
#endif

      btif_hf_cb[idx].peer_feat = p_data->conn.peer_feat;
      btif_hf_cb[idx].state = BTHF_CONNECTION_STATE_SLC_CONNECTED;

      HAL_HF_CBACK(bt_hf_callbacks, ConnectionStateCallback, btif_hf_cb[idx].state,
                &btif_hf_cb[idx].connected_bda);
      btif_queue_advance_by_uuid(UUID_SERVCLASS_AG_HANDSFREE, &btif_hf_cb[idx].connected_bda);
      break;

    case BTA_AG_AUDIO_OPENING_EVT:
      BTIF_TRACE_DEBUG("%s:  Moving the audio_state to CONNECTING for device %s",
                      __FUNCTION__, btif_hf_cb[idx].connected_bda.ToString().c_str());
      btif_hf_cb[idx].audio_state = BTHF_AUDIO_STATE_CONNECTING;
      HAL_HF_CBACK(bt_hf_callbacks, AudioStateCallback, BTHF_AUDIO_STATE_CONNECTING,
                &btif_hf_cb[idx].connected_bda);
      break;

    case BTA_AG_AUDIO_OPEN_EVT:
      BTIF_TRACE_DEBUG("%s: Moving the audio_state to CONNECTED for device %s",
                      __FUNCTION__, btif_hf_cb[idx].connected_bda.ToString().c_str());
      btif_hf_cb[idx].audio_state = BTHF_AUDIO_STATE_CONNECTED;
      HAL_HF_CBACK(bt_hf_callbacks, AudioStateCallback, BTHF_AUDIO_STATE_CONNECTED,
                &btif_hf_cb[idx].connected_bda);
      break;

    case BTA_AG_AUDIO_CLOSE_EVT:
      BTIF_TRACE_DEBUG("%s: Moving the audio_state to DISCONNECTED for device %s",
                       __FUNCTION__, btif_hf_cb[idx].connected_bda.ToString().c_str());
#if (BT_IOT_LOGGING_ENABLED == TRUE)
      if (btif_hf_cb[idx].audio_state != BTHF_AUDIO_STATE_CONNECTED) {
        device_iot_config_addr_int_add_one(btif_hf_cb[idx].connected_bda,
            IOT_CONF_KEY_HFP_SCO_CONN_FAIL_COUNT);
      }
#endif
      btif_hf_cb[idx].audio_state = BTHF_AUDIO_STATE_DISCONNECTED;

      // Ignore SCO disconnection event if SLC is already disconnected
      if (btif_hf_cb[idx].state == BTHF_CONNECTION_STATE_SLC_CONNECTED) {
        HAL_HF_CBACK(bt_hf_callbacks, AudioStateCallback, BTHF_AUDIO_STATE_DISCONNECTED,
                &btif_hf_cb[idx].connected_bda);
      }
      break;

    /* BTA auto-responds, silently discard */
    case BTA_AG_SPK_EVT:
        FALLTHROUGH;
    case BTA_AG_MIC_EVT:
      HAL_HF_CBACK(bt_hf_callbacks, VolumeControlCallback,
                (event == BTA_AG_SPK_EVT) ? BTHF_VOLUME_TYPE_SPK
                                          : BTHF_VOLUME_TYPE_MIC,
                p_data->val.num, &btif_hf_cb[idx].connected_bda);
      break;

    case BTA_AG_AT_A_EVT:
      HAL_HF_CBACK(bt_hf_callbacks, AnswerCallCallback, &btif_hf_cb[idx].connected_bda);
      break;

    /* Java needs to send OK/ERROR for these commands */
    case BTA_AG_AT_BLDN_EVT:
    case BTA_AG_AT_D_EVT:
      HAL_HF_CBACK(bt_hf_callbacks, DialCallCallback,
                (event == BTA_AG_AT_D_EVT) ? p_data->val.str : NULL,
                &btif_hf_cb[idx].connected_bda);
      break;

    case BTA_AG_AT_CHUP_EVT:
      HAL_HF_CBACK(bt_hf_callbacks, HangupCallCallback, &btif_hf_cb[idx].connected_bda);
      break;

    case BTA_AG_AT_CIND_EVT:
      HAL_HF_CBACK(bt_hf_callbacks, AtCindCallback, &btif_hf_cb[idx].connected_bda);
      break;

    case BTA_AG_AT_VTS_EVT:
      HAL_HF_CBACK(bt_hf_callbacks, DtmfCmdCallback, p_data->val.str[0],
                &btif_hf_cb[idx].connected_bda);
      break;

    case BTA_AG_AT_BVRA_EVT:
      HAL_HF_CBACK(bt_hf_callbacks, VoiceRecognitionCallback,
                (p_data->val.num == 1) ? BTHF_VR_STATE_STARTED
                                       : BTHF_VR_STATE_STOPPED,
                &btif_hf_cb[idx].connected_bda);
#if (TWS_AG_ENABLED == TRUE)
      if (btif_is_tws_plus_device(&btif_hf_cb[idx].connected_bda)) {
        BTIF_TRACE_DEBUG("%s: VR is initiated from TWS+ device, idx: %d", __FUNCTION__, idx);
        btif_twsp_send_bvra_update = TRUE;
        btif_twsp_bvra_peer_eb_state = p_data->val.num;
      }
#endif
      break;

    case BTA_AG_AT_NREC_EVT:
      HAL_HF_CBACK(bt_hf_callbacks, NoiseReductionCallback,
                (p_data->val.num == 1) ? BTHF_NREC_START : BTHF_NREC_STOP,
                &btif_hf_cb[idx].connected_bda);
      break;

    /* TODO: Add a callback for CBC */
    case BTA_AG_AT_CBC_EVT:
      break;

    case BTA_AG_AT_CKPD_EVT:
      HAL_HF_CBACK(bt_hf_callbacks, KeyPressedCallback, &btif_hf_cb[idx].connected_bda);
      break;

    case BTA_AG_WBS_EVT:
      BTIF_TRACE_DEBUG(
          "BTA_AG_WBS_EVT Set codec status %d codec %d 1=CVSD 2=MSBC",
          p_data->val.hdr.status, p_data->val.num);
      if (p_data->val.num == BTA_AG_CODEC_CVSD) {
        HAL_HF_CBACK(bt_hf_callbacks, WbsCallback, BTHF_WBS_NO,
                  &btif_hf_cb[idx].connected_bda);
      } else if (p_data->val.num == BTA_AG_CODEC_MSBC) {
        HAL_HF_CBACK(bt_hf_callbacks, WbsCallback, BTHF_WBS_YES,
                  &btif_hf_cb[idx].connected_bda);
      } else {
        HAL_HF_CBACK(bt_hf_callbacks, WbsCallback, BTHF_WBS_NONE,
                  &btif_hf_cb[idx].connected_bda);
      }
      break;

#if SWB_ENABLED
    case BTA_AG_SWB_EVT:
      BTIF_TRACE_DEBUG("%s: AG final selected SWB codec is 0x%02x 0=Q0 4=Q1 6=Q3 7=Q4",
                       __func__, p_data->val.num);
      btif_handle_vendor_hf_events(event, p_data, &btif_hf_cb[idx].connected_bda);
      break;
#endif

    /* Java needs to send OK/ERROR for these commands */
    case BTA_AG_AT_CHLD_EVT:
      HAL_HF_CBACK(bt_hf_callbacks, AtChldCallback,
                (bthf_chld_type_t)atoi(p_data->val.str),
                &btif_hf_cb[idx].connected_bda);
      break;

    case BTA_AG_AT_CLCC_EVT:
      HAL_HF_CBACK(bt_hf_callbacks, AtClccCallback, &btif_hf_cb[idx].connected_bda);
      break;

    case BTA_AG_AT_COPS_EVT:
      HAL_HF_CBACK(bt_hf_callbacks, AtCopsCallback, &btif_hf_cb[idx].connected_bda);
      break;

    case BTA_AG_AT_UNAT_EVT:
      HAL_HF_CBACK(bt_hf_callbacks, UnknownAtCallback, p_data->val.str,
                &btif_hf_cb[idx].connected_bda);
      break;

    case BTA_AG_AT_CNUM_EVT:
      HAL_HF_CBACK(bt_hf_callbacks, AtCnumCallback, &btif_hf_cb[idx].connected_bda);
      break;

    /* TODO: Some of these commands may need to be sent to app. For now respond
     * with error */
    case BTA_AG_AT_BINP_EVT:
    case BTA_AG_AT_BTRH_EVT:
      send_at_result(BTA_AG_OK_ERROR, BTA_AG_ERR_OP_NOT_SUPPORTED, idx);
      break;
    case BTA_AG_AT_BAC_EVT:
      BTIF_TRACE_DEBUG("AG Bitmap of peer-codecs %d", p_data->val.num);
      /* If the peer supports mSBC and the BTIF preferred codec is also mSBC,
      then
      we should set the BTA AG Codec to mSBC. This would trigger a +BCS to mSBC
      at the time
      of SCO connection establishment */
      if ((btif_conf_hf_force_wbs == true) &&
          (p_data->val.num & BTA_AG_CODEC_MSBC)) {
        BTIF_TRACE_EVENT("%s: btif_hf override-Preferred Codec to MSBC",
                         __func__);
        BTA_AgSetCodec(btif_hf_cb[idx].handle, BTA_AG_CODEC_MSBC);
      } else {
        BTIF_TRACE_EVENT("%s btif_hf override-Preferred Codec to CVSD",
                         __func__);
        BTA_AgSetCodec(btif_hf_cb[idx].handle, BTA_AG_CODEC_CVSD);
      }
      break;
    case BTA_AG_AT_BCS_EVT:
      BTIF_TRACE_DEBUG("%s: AG final selected codec is 0x%02x 1=CVSD 2=MSBC",
                       __func__, p_data->val.num);
      /* No BTHF_WBS_NONE case, because HF1.6 supported device can send BCS */
      /* Only CVSD is considered narrow band speech */
      HAL_HF_CBACK(bt_hf_callbacks, WbsCallback,
              (p_data->val.num == BTA_AG_CODEC_CVSD) ? BTHF_WBS_NO : BTHF_WBS_YES,
              &btif_hf_cb[idx].connected_bda);
      break;

#if (SWB_ENABLED == TRUE)
    case BTA_AG_AT_QAC_EVT:
      BTIF_TRACE_DEBUG("QAC-EVT: AG Bitmap of peer-codecs %d", p_data->val.num);
      /* If the peer supports SWB and the BTIF preferred codec is also SWB,
      then we should set the BTA AG Codec to SWB. This would trigger a +QCS
      to SWB at the time of SCO connection establishment */
      if (!get_swb_codec_status()) break;

      if (p_data->val.num == BTA_AG_SCO_SWB_SETTINGS_Q0) {
        BTIF_TRACE_EVENT("%s: btif_hf override-Preferred Codec to SWB",
                         __func__);
        BTA_AgSetCodec(btif_hf_cb[idx].handle, BTA_AG_SCO_SWB_SETTINGS_Q0);
      } else {
        BTIF_TRACE_EVENT("%s btif_hf override-Preferred Codec to MSBC",
                         __func__);
        BTA_AgSetCodec(btif_hf_cb[idx].handle, BTA_AG_CODEC_MSBC);
      }
      break;

    case BTA_AG_AT_QCS_EVT:
      BTIF_TRACE_DEBUG("%s: AG final selected SWB codec is 0x%02x 0=Q0 4=Q1 6=Q3 7=Q4",
                       __func__, p_data->val.num);
      btif_handle_vendor_hf_events(event, p_data, &btif_hf_cb[idx].connected_bda);
      break;
#endif

    case BTA_AG_AT_BIND_EVT:
      if (p_data->val.hdr.status == BTA_AG_SUCCESS) {
        HAL_HF_CBACK(bt_hf_callbacks, AtBindCallback, p_data->val.str,
                &btif_hf_cb[idx].connected_bda);
      }
      break;

    case BTA_AG_AT_BIEV_EVT:
      if (p_data->val.hdr.status == BTA_AG_SUCCESS) {
        HAL_HF_CBACK(bt_hf_callbacks, AtBievCallback, (bthf_hf_ind_type_t)p_data->val.lidx,
                (int)p_data->val.num, &btif_hf_cb[idx].connected_bda);
      }
      break;

    case BTA_AG_TWSP_STATE_UPDATE:
      BTIF_TRACE_DEBUG("%s: Twsp state update idx: %d, state: %d",
                      __func__, idx, p_data->val.num);
      btif_hf_cb[idx].twsp_state = p_data->val.num;
      break;

    case BTA_AG_TWSP_BATTERY_UPDATE: {
        BTIF_TRACE_DEBUG("%s: Twsp battery status update : %s",
                                           __func__, p_data->val.str);
        btif_handle_vendor_hf_events(event, p_data,
                               &btif_hf_cb[idx].connected_bda);
    }
    break;

    default:
      BTIF_TRACE_WARNING("%s: Unhandled event: %d", __func__, event);
      break;
  }
}

/*******************************************************************************
 *
 * Function         bte_hf_evt
 *
 * Description      Switches context from BTE to BTIF for all HF events
 *
 * Returns          void
 *
 ******************************************************************************/

static void bte_hf_evt(tBTA_AG_EVT event, tBTA_AG* p_data) {
  bt_status_t status;
  int param_len = 0;

  /* TODO: BTA sends the union members and not tBTA_AG. If using
   * param_len=sizeof(tBTA_AG), we get a crash on memcpy */
  if (BTA_AG_REGISTER_EVT == event)
    param_len = sizeof(tBTA_AG_REGISTER);
  else if (BTA_AG_OPEN_EVT == event)
    param_len = sizeof(tBTA_AG_OPEN);
  else if (BTA_AG_CONN_EVT == event)
    param_len = sizeof(tBTA_AG_CONN);
  else if ((BTA_AG_CLOSE_EVT == event) || (BTA_AG_AUDIO_OPEN_EVT == event) ||
           (BTA_AG_AUDIO_CLOSE_EVT == event))
    param_len = sizeof(tBTA_AG_HDR);
  else if (p_data)
    param_len = sizeof(tBTA_AG_VAL);

  /* switch context to btif task context (copy full union size for convenience)
   */
  status = btif_transfer_context(btif_hf_upstreams_evt, (uint16_t)event,
                                 (char*)p_data, param_len, NULL);

  /* catch any failed context transfers */
  ASSERTC(status == BT_STATUS_SUCCESS, "context transfer failed", status);
}

/*******************************************************************************
 *
 * Function         btif_in_hf_generic_evt
 *
 * Description     Processes generic events to be sent to JNI that are not
 *                      triggered from the BTA.
 *                      Always runs in BTIF context
 *
 * Returns          void
 *
 ******************************************************************************/
void btif_in_hf_generic_evt(uint16_t event, char* p_param) {
  int idx = btif_hf_idx_by_bdaddr((RawAddress*)p_param);

  BTIF_TRACE_EVENT("%s: event=%d", __func__, event);

  if ((idx < 0) || (idx >= BTA_AG_MAX_NUM_CLIENTS)) {
    BTIF_TRACE_ERROR("%s: Invalid index %d", __func__, idx);
    return;
  }

  BTIF_TRACE_DEBUG("%s: audio state = %d for device: %s", __func__,
       btif_hf_cb[idx].audio_state, btif_hf_cb[idx].connected_bda.ToString().c_str());
  switch (event) {
    case BTIF_HFP_CB_AUDIO_CONNECTING: {
      if (btif_hf_cb[idx].audio_state != BTHF_AUDIO_STATE_CONNECTED) {
        BTIF_TRACE_DEBUG("%s: Moving the audio_state to CONNECTING for device %s",
                  __FUNCTION__, btif_hf_cb[idx].connected_bda.ToString().c_str());
        btif_hf_cb[idx].audio_state = BTHF_AUDIO_STATE_CONNECTING;
        HAL_HF_CBACK(bt_hf_callbacks, AudioStateCallback, BTHF_AUDIO_STATE_CONNECTING,
                &btif_hf_cb[idx].connected_bda);
      } else {
        BTIF_TRACE_WARNING("%s :unexpected audio state", __func__);
      }
    } break;
    case BTIF_HFP_CB_AUDIO_DISCONNECTED: {
      BTIF_TRACE_DEBUG("%s: Moving the audio_state to DISCONNECTED for device %s",
                __FUNCTION__, btif_hf_cb[idx].connected_bda.ToString().c_str());
      btif_hf_cb[idx].audio_state = BTHF_AUDIO_STATE_DISCONNECTED;
      HAL_HF_CBACK(bt_hf_callbacks, AudioStateCallback, BTHF_AUDIO_STATE_DISCONNECTED,
              &btif_hf_cb[idx].connected_bda);
    } break;
    default: {
      BTIF_TRACE_WARNING("%s : Unknown event 0x%x", __func__, event);
    } break;
  }
}

class HeadsetInterface : Interface {
 public:
  static Interface* GetInstance() {
    static Interface* instance = new HeadsetInterface();
    return instance;
  }
  bt_status_t Init(Callbacks* callbacks, int max_hf_clients,
                   bool inband_ringing_enabled) override;
  bt_status_t Connect(RawAddress* bd_addr) override;
  bt_status_t Disconnect(RawAddress* bd_addr) override;
  bt_status_t ConnectAudio(RawAddress* bd_addr) override;
  bt_status_t DisconnectAudio(RawAddress* bd_addr) override;
  bt_status_t StartVoiceRecognition(RawAddress* bd_addr) override;
  bt_status_t StopVoiceRecognition(RawAddress* bd_addr) override;
  bt_status_t VolumeControl(bthf_volume_type_t type, int volume,
                            RawAddress* bd_addr) override;
  bt_status_t DeviceStatusNotification(bthf_network_state_t ntk_state,
                                       bthf_service_type_t svc_type, int signal,
                                       int batt_chg, RawAddress* bd_addr) override;
  bt_status_t CopsResponse(const char* cops, RawAddress* bd_addr) override;
  bt_status_t CindResponse(int svc, int num_active, int num_held,
                           bthf_call_state_t call_setup_state, int signal,
                           int roam, int batt_chg,
                           RawAddress* bd_addr) override;
  bt_status_t FormattedAtResponse(const char* rsp,
                                  RawAddress* bd_addr) override;
  bt_status_t AtResponse(bthf_at_response_t response_code, int error_code,
                         RawAddress* bd_addr) override;
  bt_status_t ClccResponse(int index, bthf_call_direction_t dir,
                           bthf_call_state_t state, bthf_call_mode_t mode,
                           bthf_call_mpty_type_t mpty, const char* number,
                           bthf_call_addrtype_t type,
                           RawAddress* bd_addr) override;
  bt_status_t PhoneStateChange(int num_active, int num_held,
                               bthf_call_state_t call_setup_state,
                               const char* number, bthf_call_addrtype_t type,
                               const char* name, RawAddress* bd_addr) override;

  void Cleanup() override;
  bt_status_t SetScoAllowed(bool value) override;
  bt_status_t SendBsir(bool value, RawAddress* bd_addr) override;
  bt_status_t SetActiveDevice(RawAddress* active_device_addr) override;
};

/*******************************************************************************
 *
 * Function         Init
 *
 * Description     initializes the hf interface
 *
 * Returns         bt_status_t
 *
 ******************************************************************************/
bt_status_t HeadsetInterface::Init(Callbacks* callbacks, int max_hf_clients,
                                   bool inband_ringing_enabled) {
  if (inband_ringing_enabled) {
    btif_hf_features |= BTA_AG_FEAT_INBAND;
  } else {
    btif_hf_features &= ~BTA_AG_FEAT_INBAND;
  }

  CHECK_LE(max_hf_clients, BTA_AG_MAX_NUM_CLIENTS)
      << __func__
      << "Too many HF clients,"
         " maximum is "
      << BTA_AG_MAX_NUM_CLIENTS << " was given " << max_hf_clients;

  btif_max_hf_clients = max_hf_clients;

  // let bta also know about the max hf clients
  BTA_AgSetMaxHfClients(max_hf_clients);

  BTIF_TRACE_DEBUG(
      "%s: btif_hf_features=%zu, max_hf_clients=%d, inband_ringing_enabled=%d",
      __func__, btif_hf_features, btif_max_hf_clients, inband_ringing_enabled);
  bt_hf_callbacks = callbacks;
  for (btif_hf_cb_t& hf_cb : btif_hf_cb) {
    hf_cb = {};
  }

/* Invoke the enable service API to the core to set the appropriate service_id
 * Internally, the HSP_SERVICE_ID shall also be enabled if HFP is enabled
 * (phone)
 * othwerwise only HSP is enabled (tablet)
 */
#if (defined(BTIF_HF_SERVICES) && (BTIF_HF_SERVICES & BTA_HFP_SERVICE_MASK))
  btif_enable_service(BTA_HFP_SERVICE_ID);
#else
  btif_enable_service(BTA_HSP_SERVICE_ID);
#endif

  for (int i = 0; i < btif_max_hf_clients; i++) clear_phone_state_multihf(i);

  return BT_STATUS_SUCCESS;
}

/*******************************************************************************
 *
 * Function         connect_int
 *
 * Description     connect to headset
 *
 * Returns         bt_status_t
 *
 ******************************************************************************/
static bt_status_t connect_int(RawAddress* bd_addr, uint16_t uuid) {
  CHECK_BTHF_INIT();
  int i;
  for (i = 0; i < btif_max_hf_clients;) {
    if (((btif_hf_cb[i].state == BTHF_CONNECTION_STATE_CONNECTED) ||
         (btif_hf_cb[i].state == BTHF_CONNECTION_STATE_SLC_CONNECTED)))
      i++;
    else
      break;
  }

  if (i == btif_max_hf_clients) {
    BTIF_TRACE_WARNING(
        "%s: Cannot connect %s: maximum %d clients already connected", __func__,
        bd_addr->ToString().c_str(), btif_max_hf_clients);
    return BT_STATUS_BUSY;
  }

  if (btif_storage_is_device_bonded(bd_addr) != BT_STATUS_SUCCESS) {
    BTIF_TRACE_WARNING("HF %s()## connect_int ## Device Not Bonded %s \n", __func__,
                         bd_addr->ToString().c_str());
    btif_hf_cb[i].state = BTHF_CONNECTION_STATE_DISCONNECTED;
    /* inform the application of the disconnection as the connection is not processed */
    HAL_HF_CBACK(bt_hf_callbacks, ConnectionStateCallback, btif_hf_cb[i].state,
                  bd_addr);
    btif_queue_advance_by_uuid(UUID_SERVCLASS_AG_HANDSFREE, bd_addr);
    return BT_STATUS_SUCCESS;
  }

  if (is_connected(bd_addr)) {
    BTIF_TRACE_WARNING("%s: device %s is already connected", __func__,
                       bd_addr->ToString().c_str());
    return BT_STATUS_BUSY;
  }

  btif_hf_cb[i].state = BTHF_CONNECTION_STATE_CONNECTING;
  btif_hf_cb[i].connected_bda = *bd_addr;

  BTA_AgOpen(btif_hf_cb[i].handle, btif_hf_cb[i].connected_bda,
               BTIF_HF_SECURITY, BTIF_HF_SERVICES);

#if (BT_IOT_LOGGING_ENABLED == TRUE)
  device_iot_config_addr_set_int(btif_hf_cb[i].connected_bda,
      IOT_CONF_KEY_HFP_ROLE, IOT_CONF_VAL_HFP_ROLE_CLIENT);
  device_iot_config_addr_int_add_one(btif_hf_cb[i].connected_bda,
      IOT_CONF_KEY_HFP_SLC_CONN_COUNT);
#endif
  return BT_STATUS_SUCCESS;
}


static void UpdateCallStates(btif_hf_cb_t* control_block, int num_active,
                             int num_held, bthf_call_state_t call_setup_state) {
  control_block->num_active = num_active;
  control_block->num_held = num_held;
  control_block->call_setup_state = call_setup_state;
}

bt_status_t HeadsetInterface::Connect(RawAddress* bd_addr) {
  CHECK_BTHF_INIT();
  return btif_queue_connect(UUID_SERVCLASS_AG_HANDSFREE, *bd_addr, connect_int,
                                  btif_max_hf_clients);
}

/*******************************************************************************
 *
 * Function         Disconnect
 *
 * Description      disconnect from headset
 *
 * Returns         bt_status_t
 *
 ******************************************************************************/
bt_status_t HeadsetInterface::Disconnect(RawAddress* bd_addr) {
  CHECK_BTHF_INIT();

  int idx = btif_hf_idx_by_bdaddr(bd_addr);

  if ((idx < 0) || (idx >= BTA_AG_MAX_NUM_CLIENTS)) {
    BTIF_TRACE_ERROR("%s: Invalid index %d", __func__, idx);
    return BT_STATUS_FAIL;
  }

  if (idx != BTIF_HF_INVALID_IDX) {
    BTA_AgClose(btif_hf_cb[idx].handle);
    return BT_STATUS_SUCCESS;
  }

  return BT_STATUS_FAIL;
}

/*******************************************************************************
 *
 * Function         ConnectAudio
 *
 * Description     create an audio connection
 *
 * Returns         bt_status_t
 *
 ******************************************************************************/
bt_status_t HeadsetInterface::ConnectAudio(RawAddress* bd_addr) {
  CHECK_BTHF_INIT();

  int idx = btif_hf_idx_by_bdaddr(bd_addr);

  if ((idx < 0) || (idx >= BTA_AG_MAX_NUM_CLIENTS)) {
    BTIF_TRACE_ERROR("%s: Invalid index %d", __func__, idx);
    return BT_STATUS_FAIL;
  }

  /* Check if SLC is connected */
  if (!IsSlcConnected(bd_addr)) {
    LOG(ERROR) << ": SLC not connected for " << *bd_addr;
    return BT_STATUS_NOT_READY;
  }

  /* Check if SCO is already connecting/connected for the same device*/
  if (btif_hf_cb[idx].audio_state == BTHF_AUDIO_STATE_CONNECTING ||
     btif_hf_cb[idx].audio_state == BTHF_AUDIO_STATE_CONNECTED) {
    BTIF_TRACE_WARNING("%s: Ignore SCO connection as audio_state for device %s: is %d",
    __func__, btif_hf_cb[idx].connected_bda.ToString().c_str(),
    btif_hf_cb[idx].audio_state);
    return BT_STATUS_SUCCESS;
  }

  // if SCO is setting up, don't allow SCO connection
  for (int i = 0; i < btif_max_hf_clients; i++) {
    if (btif_hf_cb[i].audio_state == BTHF_AUDIO_STATE_CONNECTING) {
      BTIF_TRACE_ERROR("%s: SCO setting up with %s, not allowing SCO connection with %s",
      __func__, btif_hf_cb[i].connected_bda.ToString().c_str(),
      bd_addr->ToString().c_str());
      return BT_STATUS_FAIL;
    }
  }

  if (
#if (TWS_AG_ENABLED == TRUE)
      !BTM_SecIsTwsPlusDev(*bd_addr) &&
#endif
      !is_active_device(*bd_addr)) {
    LOG(ERROR) << "HF: ConnectAudio is called for inactive device, returning"
               << *bd_addr;
    return BT_STATUS_FAIL;
  }

#if (TWS_AG_ENABLED == TRUE)
  if (btif_is_tws_plus_device(bd_addr) &&
      !btif_hf_check_twsp_state_for_sco(idx)) {
    RawAddress peer_eb_addr;
    int peer_idx;

    BTIF_TRACE_DEBUG("%s: Not create SCO since earbud is not in ear",__FUNCTION__);
    btif_tws_plus_get_peer_eb_addr(bd_addr, &peer_eb_addr);
    peer_idx = btif_hf_idx_by_bdaddr(&peer_eb_addr);

    if (peer_idx != BTIF_HF_INVALID_IDX && IsSlcConnected(&peer_eb_addr) &&
        btif_hf_cb[peer_idx].twsp_state != TWSPLUS_EB_STATE_OUT_OF_EAR &&
        btif_hf_cb[peer_idx].twsp_state != TWSPLUS_EB_STATE_INCASE) {
      BTIF_TRACE_DEBUG("%s: Create SCO with the other earbud in ear", __FUNCTION__);
      BTA_AgAudioOpen(btif_hf_cb[peer_idx].handle);

#if (BT_IOT_LOGGING_ENABLED == TRUE)
      if (btif_hf_cb[peer_idx].audio_state != BTHF_AUDIO_STATE_CONNECTING)
        device_iot_config_addr_int_add_one(peer_eb_addr, IOT_CONF_KEY_HFP_SCO_CONN_COUNT);
#endif

      btif_transfer_context(btif_in_hf_generic_evt, BTIF_HFP_CB_AUDIO_CONNECTING,
                            (char*)&peer_eb_addr, sizeof(RawAddress), NULL);
    }
    return BT_STATUS_FAIL;
  }
#endif

  if (idx != BTIF_HF_INVALID_IDX) {
    BTA_AgAudioOpen(btif_hf_cb[idx].handle);

#if (BT_IOT_LOGGING_ENABLED == TRUE)
    if (btif_hf_cb[idx].audio_state != BTHF_AUDIO_STATE_CONNECTING)
      device_iot_config_addr_int_add_one(*bd_addr, IOT_CONF_KEY_HFP_SCO_CONN_COUNT);
#endif

    /* Inform the application that the audio connection has been initiated
     * successfully */
    btif_transfer_context(btif_in_hf_generic_evt, BTIF_HFP_CB_AUDIO_CONNECTING,
                          (char*)bd_addr, sizeof(RawAddress), NULL);
    return BT_STATUS_SUCCESS;
  }

  return BT_STATUS_FAIL;
}

/*******************************************************************************
 *
 * Function         DisconnectAudio
 *
 * Description      close the audio connection
 *
 * Returns         bt_status_t
 *
 ******************************************************************************/
bt_status_t HeadsetInterface::DisconnectAudio(RawAddress* bd_addr) {
  CHECK_BTHF_INIT();

  int idx = btif_hf_idx_by_bdaddr(bd_addr);

  if ((idx < 0) || (idx >= BTA_AG_MAX_NUM_CLIENTS)) {
    BTIF_TRACE_ERROR("%s: Invalid index %d", __func__, idx);
    return BT_STATUS_FAIL;
  }

  if (idx != BTIF_HF_INVALID_IDX) {
    BTA_AgAudioClose(btif_hf_cb[idx].handle);
    return BT_STATUS_SUCCESS;
  }

  return BT_STATUS_FAIL;
}

/*******************************************************************************
 *
 * Function         StartVoiceRecognition
 *
 * Description      start voice recognition
 *
 * Returns          bt_status_t
 *
 ******************************************************************************/
bt_status_t HeadsetInterface::StartVoiceRecognition(RawAddress* bd_addr) {
    CHECK_BTHF_INIT();
    int idx = btif_hf_idx_by_bdaddr(bd_addr);

    if ((idx < 0) || (idx >= BTA_AG_MAX_NUM_CLIENTS)) {
      BTIF_TRACE_ERROR("%s: Invalid index %d", __func__, idx);
      return BT_STATUS_FAIL;
    }
    if (!is_connected(bd_addr)) {
      BTIF_TRACE_ERROR("%s: %s is not connected", __func__,
                       bd_addr->ToString().c_str());
      return BT_STATUS_NOT_READY;
    }
    if (!(btif_hf_cb[idx].peer_feat & BTA_AG_PEER_FEAT_VREC)) {
      BTIF_TRACE_ERROR("%s: voice recognition not supported, features=0x%x",
                       __func__, btif_hf_cb[idx].peer_feat);
      return BT_STATUS_UNSUPPORTED;
    }

    tBTA_AG_RES_DATA ag_res = {};
    ag_res.state = true;
    BTA_AgResult(btif_hf_cb[idx].handle, BTA_AG_BVRA_RES, &ag_res);

#if (TWS_AG_ENABLED == TRUE)
    if (btif_is_tws_plus_device(bd_addr)) {
        btif_hf_twsp_send_bvra_update(idx, &ag_res);
    }
#endif
    return BT_STATUS_SUCCESS;
}


/*******************************************************************************
 *
 * Function         StopVoiceRecognition
 *
 * Description      stop voice recognition
 *
 * Returns          bt_status_t
 *
 ******************************************************************************/
bt_status_t HeadsetInterface::StopVoiceRecognition(RawAddress* bd_addr) {
    CHECK_BTHF_INIT();

    int idx = btif_hf_idx_by_bdaddr(bd_addr);

    if ((idx < 0) || (idx >= BTA_AG_MAX_NUM_CLIENTS)) {
      BTIF_TRACE_ERROR("%s: Invalid index %d", __func__, idx);
      return BT_STATUS_FAIL;
    }
    if (!is_connected(bd_addr)) {
      BTIF_TRACE_ERROR("%s: %s is not connected", __func__,
                       bd_addr->ToString().c_str());
      return BT_STATUS_NOT_READY;
    }
    if (!(btif_hf_cb[idx].peer_feat & BTA_AG_PEER_FEAT_VREC)) {
      BTIF_TRACE_ERROR("%s: voice recognition not supported, features=0x%x",
                       __func__, btif_hf_cb[idx].peer_feat);
      return BT_STATUS_UNSUPPORTED;
    }

    tBTA_AG_RES_DATA ag_res = {};
    ag_res.state = false;
    BTA_AgResult(btif_hf_cb[idx].handle, BTA_AG_BVRA_RES, &ag_res);

#if (TWS_AG_ENABLED == TRUE)
    if (btif_is_tws_plus_device(bd_addr)) {
        btif_hf_twsp_send_bvra_update(idx, &ag_res);
    }
#endif
    return BT_STATUS_SUCCESS;
}


/*******************************************************************************
 *
 * Function         VolumeControl
 *
 * Description      volume control
 *
 * Returns          bt_status_t
 *
 ******************************************************************************/
bt_status_t HeadsetInterface::VolumeControl(bthf_volume_type_t type, int volume,
                                  RawAddress* bd_addr) {
  CHECK_BTHF_INIT();

  int idx = btif_hf_idx_by_bdaddr(bd_addr);

  if ((idx < 0) || (idx >= BTA_AG_MAX_NUM_CLIENTS)) {
    BTIF_TRACE_ERROR("%s: Invalid index %d", __func__, idx);
    return BT_STATUS_FAIL;
  }

  tBTA_AG_RES_DATA ag_res;
  memset(&ag_res, 0, sizeof(tBTA_AG_RES_DATA));
  if (idx != BTIF_HF_INVALID_IDX) {
    ag_res.num = volume;
    BTA_AgResult(
        btif_hf_cb[idx].handle,
        (type == BTHF_VOLUME_TYPE_SPK) ? BTA_AG_SPK_RES : BTA_AG_MIC_RES,
        &ag_res);
    return BT_STATUS_SUCCESS;
  }
  return BT_STATUS_FAIL;
}

/*******************************************************************************
 *
 * Function         DeviceStatusNotification
 *
 * Description      Combined device status change notification
 *
 * Returns          bt_status_t
 *
 ******************************************************************************/
bt_status_t HeadsetInterface::DeviceStatusNotification(
                                bthf_network_state_t ntk_state,
                                bthf_service_type_t svc_type,
                                int signal, int batt_chg, RawAddress* bd_addr) {
  CHECK_BTHF_INIT();
  if (!bd_addr) {
    BTIF_TRACE_WARNING("%s: bd_addr is null", __func__);
    return BT_STATUS_FAIL;
  }
  int idx = btif_hf_idx_by_bdaddr(bd_addr);
  if (idx < 0 || idx >= BTA_AG_MAX_NUM_CLIENTS) {
    BTIF_TRACE_WARNING("%s: invalid index %d for %s", __func__, idx,
                       bd_addr->ToString().c_str());
    return BT_STATUS_FAIL;
  }
  const btif_hf_cb_t& control_block = btif_hf_cb[idx];

  if (is_connected(NULL)) {
    /* send all indicators to BTA.
    ** BTA will make sure no duplicates are sent out
    */
    send_indicator_update(control_block, BTA_AG_IND_SERVICE,
                          (ntk_state == BTHF_NETWORK_STATE_AVAILABLE) ? 1 : 0);
    send_indicator_update(control_block, BTA_AG_IND_ROAM,
                          (svc_type == BTHF_SERVICE_TYPE_HOME) ? 0 : 1);
    send_indicator_update(control_block, BTA_AG_IND_SIGNAL, signal);
    send_indicator_update(control_block, BTA_AG_IND_BATTCHG, batt_chg);
    return BT_STATUS_SUCCESS;
  }

  return BT_STATUS_SUCCESS;
}


/*******************************************************************************
 *
 * Function         CopsResponse
 *
 * Description      Response for COPS command
 *
 * Returns          bt_status_t
 *
 ******************************************************************************/
bt_status_t HeadsetInterface::CopsResponse(const char* cops, RawAddress* bd_addr) {
  CHECK_BTHF_INIT();

  int idx = btif_hf_idx_by_bdaddr(bd_addr);

  if ((idx < 0) || (idx >= BTA_AG_MAX_NUM_CLIENTS)) {
    BTIF_TRACE_ERROR("%s: Invalid index %d", __func__, idx);
    return BT_STATUS_FAIL;
  }

  if (idx != BTIF_HF_INVALID_IDX) {
    tBTA_AG_RES_DATA ag_res;

    /* Format the response */
    snprintf(ag_res.str, sizeof(ag_res.str), "0,0,\"%.16s\"", cops);
    ag_res.ok_flag = BTA_AG_OK_DONE;

    BTA_AgResult(btif_hf_cb[idx].handle, BTA_AG_COPS_RES, &ag_res);
    return BT_STATUS_SUCCESS;
  }
  return BT_STATUS_FAIL;
}

/*******************************************************************************
 *
 * Function         cind_response
 *
 * Description      Response for CIND command
 *
 * Returns          bt_status_t
 *
 ******************************************************************************/
bt_status_t HeadsetInterface::CindResponse(int svc, int num_active, int num_held,
                                 bthf_call_state_t call_setup_state, int signal,
                                 int roam, int batt_chg, RawAddress* bd_addr) {
  CHECK_BTHF_INIT();

  int idx = btif_hf_idx_by_bdaddr(bd_addr);

  if ((idx < 0) || (idx >= BTA_AG_MAX_NUM_CLIENTS)) {
    BTIF_TRACE_ERROR("%s: Invalid index %d", __func__, idx);
    return BT_STATUS_FAIL;
  }

  if (idx != BTIF_HF_INVALID_IDX) {
    tBTA_AG_RES_DATA ag_res;

    memset(&ag_res, 0, sizeof(ag_res));
    /* per the errata 2043, call=1 implies atleast one call is in progress
     *(active/held)
     ** https://www.bluetooth.org/errata/errata_view.cfm?errata_id=2043
     **/
    snprintf(
        ag_res.str, sizeof(ag_res.str), "%d,%d,%d,%d,%d,%d,%d",
        (num_active + num_held) ? 1 : 0,          /* Call state */
        callstate_to_callsetup(call_setup_state), /* Callsetup state */
        svc,                                      /* network service */
        signal,                                   /* Signal strength */
        roam,                                     /* Roaming indicator */
        batt_chg,                                 /* Battery level */
        ((num_held == 0) ? 0 : ((num_active == 0) ? 2 : 1))); /* Call held */

    BTA_AgResult(btif_hf_cb[idx].handle, BTA_AG_CIND_RES, &ag_res);

    return BT_STATUS_SUCCESS;
  }

  return BT_STATUS_FAIL;
}

bt_status_t HeadsetInterface::SetScoAllowed(bool value) {
  CHECK_BTHF_INIT();

  BTA_AgSetScoAllowed(value);
  return BT_STATUS_SUCCESS;
}

/*******************************************************************************
 *
 * Function         FormattedAtResponse
 *
 * Description      Pre-formatted AT response, typically in response to unknown
 *                  AT cmd
 *
 * Returns          bt_status_t
 *
 ******************************************************************************/
bt_status_t HeadsetInterface::FormattedAtResponse(const char* rsp,
                                        RawAddress* bd_addr) {
  CHECK_BTHF_INIT();
  tBTA_AG_RES_DATA ag_res;
  int idx = btif_hf_idx_by_bdaddr(bd_addr);

  if ((idx < 0) || (idx >= BTA_AG_MAX_NUM_CLIENTS)) {
    BTIF_TRACE_ERROR("%s: Invalid index %d", __func__, idx);
    return BT_STATUS_FAIL;
  }

  if (idx != BTIF_HF_INVALID_IDX) {
    /* Format the response and send */
    memset(&ag_res, 0, sizeof(ag_res));
    strlcpy(ag_res.str, rsp, BTA_AG_AT_MAX_LEN + 1);
    BTA_AgResult(btif_hf_cb[idx].handle, BTA_AG_UNAT_RES, &ag_res);

    return BT_STATUS_SUCCESS;
  }

  return BT_STATUS_FAIL;
}

/*******************************************************************************
 *
 * Function         AtResponse
 *
 * Description      ok/error response
 *
 * Returns          bt_status_t
 *
 ******************************************************************************/
bt_status_t HeadsetInterface::AtResponse(bthf_at_response_t response_code,
                               int error_code, RawAddress* bd_addr) {
  CHECK_BTHF_INIT();

  int idx = btif_hf_idx_by_bdaddr(bd_addr);

  if ((idx < 0) || (idx >= BTA_AG_MAX_NUM_CLIENTS)) {
    BTIF_TRACE_ERROR("%s: Invalid index %d", __func__, idx);
    return BT_STATUS_FAIL;
  }

  if (idx != BTIF_HF_INVALID_IDX) {
    send_at_result((response_code == BTHF_AT_RESPONSE_OK) ? BTA_AG_OK_DONE
                                                          : BTA_AG_OK_ERROR,
                   error_code, idx);
#if (TWS_AG_ENABLED == TRUE)
    if (response_code == BTHF_AT_RESPONSE_OK && btif_twsp_send_bvra_update &&
        btif_is_tws_plus_device(bd_addr)) {
        tBTA_AG_RES_DATA ag_res;
        memset(&ag_res, 0, sizeof(ag_res));
        ag_res.state = btif_twsp_bvra_peer_eb_state;
        btif_hf_twsp_send_bvra_update(idx, &ag_res);
        btif_twsp_send_bvra_update = FALSE;
    }
#endif
    return BT_STATUS_SUCCESS;
  }

  return BT_STATUS_FAIL;
}

/*******************************************************************************
 *
 * Function         clcc_response
 *
 * Description      response for CLCC command
 *                  Can be iteratively called for each call index. Call index
 *                  of 0 will be treated as NULL termination (Completes
 *                  response)
 *
 * Returns          bt_status_t
 *
 ******************************************************************************/
bt_status_t HeadsetInterface::ClccResponse(int index, bthf_call_direction_t dir,
                                 bthf_call_state_t state, bthf_call_mode_t mode,
                                 bthf_call_mpty_type_t mpty, const char* number,
                                 bthf_call_addrtype_t type,
                                 RawAddress* bd_addr) {
  CHECK_BTHF_INIT();

  int idx = btif_hf_idx_by_bdaddr(bd_addr);

  if ((idx < 0) || (idx >= BTA_AG_MAX_NUM_CLIENTS)) {
    BTIF_TRACE_ERROR("%s: Invalid index %d", __func__, idx);
    return BT_STATUS_FAIL;
  }

  if (idx != BTIF_HF_INVALID_IDX) {
    tBTA_AG_RES_DATA ag_res;
    memset(&ag_res, 0, sizeof(ag_res));

    /* Format the response */
    if (index == 0) {
      ag_res.ok_flag = BTA_AG_OK_DONE;
    } else {
      BTIF_TRACE_EVENT(
          "clcc_response: [%d] dir %d state %d mode %d number = %s type = %d",
          index, dir, state, mode, number, type);
      int res_strlen =
          snprintf(ag_res.str, sizeof(ag_res.str), "%d,%d,%d,%d,%d", index, dir,
                   state, mode, mpty);

      if (number) {
        size_t rem_bytes = sizeof(ag_res.str) - res_strlen;
        char dialnum[sizeof(ag_res.str)];
        size_t newidx = 0;
        if (type == BTHF_CALL_ADDRTYPE_INTERNATIONAL && *number != '+') {
          dialnum[newidx++] = '+';
        }
        for (size_t i = 0; number[i] != 0; i++) {
          if (newidx >= (sizeof(dialnum) - res_strlen - 1)) {
            android_errorWriteLog(0x534e4554, "79266386");
            break;
          }
          if (utl_isdialchar(number[i])) {
            dialnum[newidx++] = number[i];
          }
        }
        dialnum[newidx] = 0;
        // Reserve 4 bytes for [,][3_digit_type]
        snprintf(&ag_res.str[res_strlen], rem_bytes - 4, ",\"%s\"", dialnum);
        std::stringstream remaining_string;
        remaining_string << "," << type;
        strlcat(ag_res.str, remaining_string.str().c_str(), sizeof(ag_res.str));
        BTIF_TRACE_EVENT("clcc_response: The CLCC response is, ag_res.str: %s", ag_res.str);
      }
    }
    BTA_AgResult(btif_hf_cb[idx].handle, BTA_AG_CLCC_RES, &ag_res);

    return BT_STATUS_SUCCESS;
  }

  return BT_STATUS_FAIL;
}

/*******************************************************************************
 *
 * Function         PhoneStateChange
 *
 * Description      notify of a call state change
 *                  number & type: valid only for incoming & waiting call
 *
 * Returns          bt_status_t
 *
 ******************************************************************************/

bt_status_t HeadsetInterface::PhoneStateChange(
    int num_active, int num_held, bthf_call_state_t call_setup_state,
    const char* number, bthf_call_addrtype_t type, const char* name,
    RawAddress* bd_addr) {
  CHECK_BTHF_INIT();
  if (!bd_addr) {
    BTIF_TRACE_WARNING("%s: bd_addr is null", __func__);
    return BT_STATUS_FAIL;
  }
  int idx = btif_hf_idx_by_bdaddr(bd_addr);
  if (idx < 0 || idx >= BTA_AG_MAX_NUM_CLIENTS) {
    BTIF_TRACE_WARNING("%s: invalid index %d for %s", __func__, idx,
                       bd_addr->ToString().c_str());
    return BT_STATUS_FAIL;
  }
#if (BT_IOT_LOGGING_ENABLED == TRUE)
  bthf_audio_state_t current_audio_state;
  current_audio_state = btif_hf_cb[idx].audio_state;
#endif

  btif_hf_cb_t& control_block = btif_hf_cb[idx];
  if (!IsSlcConnected(bd_addr)) {
    LOG(WARNING) << ": SLC not connected for " << *bd_addr;
    return BT_STATUS_NOT_READY;
  }
  if (call_setup_state == BTHF_CALL_STATE_DISCONNECTED) {
    // HFP spec does not handle cases when a call is being disconnected.
    // Since DISCONNECTED state must lead to IDLE state, ignoring it here.
    LOG(WARNING) << __func__
              << ": Ignore call state change to DISCONNECTED, idx=" << idx
              << ", addr=" << *bd_addr << ", num_active=" << num_active
              << ", num_held=" << num_held;
    return BT_STATUS_SUCCESS;
  }
  LOG(INFO) << __func__ << ": idx=" << idx << ", addr=" << *bd_addr
            << ", active_bda=" << active_bda << ", num_active=" << num_active
            << ", prev_num_active" << control_block.num_active
            << ", num_held=" << num_held
            << ", prev_num_held=" << control_block.num_held
            << ", call_state=" << dump_hf_call_state(call_setup_state)
            << ", prev_call_state="
            << dump_hf_call_state(control_block.call_setup_state);

  tBTA_AG_RES res = 0xff;
  tBTA_AG_RES_DATA ag_res;
  bt_status_t status = BT_STATUS_SUCCESS;
  bool active_call_updated = false;

  memset(&ag_res, 0, sizeof(ag_res));

  BTIF_TRACE_IMP(
      "phone_state_change: num_active=%d [prev: %d]  num_held=%d[prev: %d]"
      " call_setup=%s [prev: %s]",
      num_active, btif_hf_cb[idx].num_active, num_held,
      btif_hf_cb[idx].num_held, dump_hf_call_state(call_setup_state),
      dump_hf_call_state(btif_hf_cb[idx].call_setup_state));

  /* if all indicators are 0, send end call and return */
  if (num_active == 0 && num_held == 0 &&
      call_setup_state == BTHF_CALL_STATE_IDLE) {
    BTA_AgResult(control_block.handle, BTA_AG_END_CALL_RES, NULL);

    /* if held call was present, reset that as well */
    if (control_block.num_held)
      send_indicator_update(control_block, BTA_AG_IND_CALLHELD, 0);

    UpdateCallStates(&btif_hf_cb[idx], num_active, num_held, call_setup_state);
    return status;
  }

  /* active state can change when:
  ** 1. an outgoing/incoming call was answered
  ** 2. an held was resumed
  ** 3. without callsetup notifications, call became active
  ** (3) can happen if call is active and a headset connects to us
  **
  ** In the case of (3), we will have to notify the stack of an active
  ** call, instead of sending an indicator update. This will also
  ** force the SCO to be setup. Handle this special case here prior to
  ** call setup handling
  */
  if (((num_active + num_held) > 0) && (control_block.num_active == 0) &&
      (control_block.num_held == 0) &&
      (control_block.call_setup_state == BTHF_CALL_STATE_IDLE)) {
    BTIF_TRACE_IMP(
        "%s: Active/Held call notification received without call setup update",
        __func__);

    memset(&ag_res, 0, sizeof(tBTA_AG_RES_DATA));
    //Change the audio state during the call only for HFP device
    if (is_active_device(*bd_addr) && btif_hf_cb[idx].service_id == BTA_HFP_SERVICE_ID) {
       // initiate SCO only if it is not connected already
       if (btif_hf_cb[idx].audio_state != BTHF_AUDIO_STATE_CONNECTED) {
#if (TWS_AG_ENABLED == TRUE)
           if (btif_is_tws_plus_device(bd_addr) &&
               !btif_hf_check_twsp_state_for_sco(idx)) {
               BTIF_TRACE_DEBUG("%s: Not create SCO since earbud is not in ear", __FUNCTION__);
           } else {
#endif
               ag_res.audio_handle = control_block.handle;
               btif_transfer_context(btif_in_hf_generic_evt, BTIF_HFP_CB_AUDIO_CONNECTING,
                                 (char*)(&btif_hf_cb[idx].connected_bda), sizeof(RawAddress), NULL);
#if (TWS_AG_ENABLED == TRUE)
           }
#endif
       } else {
           BTIF_TRACE_IMP("%s: SCO is already connected with device %s, not intiating SCO",
            __func__, bd_addr->ToString().c_str());
           ag_res.audio_handle = BTA_AG_HANDLE_SCO_NO_CHANGE;
       }
    } else {
       ag_res.audio_handle = BTA_AG_HANDLE_SCO_NO_CHANGE;
       BTIF_TRACE_IMP("%s: Don't create SCO since non-active device or HSP device is connected",
                            __FUNCTION__);
    }

    /* Addition call setup with the Active call
    ** CIND response should have been updated.
    ** just open SCO connection.
    */
    if (call_setup_state != BTHF_CALL_STATE_IDLE)
      res = BTA_AG_MULTI_CALL_RES;
    else
      res = BTA_AG_OUT_CALL_CONN_RES;
    BTA_AgResult(control_block.handle, res, &ag_res);
    active_call_updated = true;
  }

  /* Ringing call changed? */
  if (call_setup_state != control_block.call_setup_state) {
    BTIF_TRACE_IMP("%s: Call setup states changed. old: %s new: %s", __func__,
                     dump_hf_call_state(control_block.call_setup_state),
                     dump_hf_call_state(call_setup_state));
    memset(&ag_res, 0, sizeof(tBTA_AG_RES_DATA));
    ag_res.audio_handle = BTA_AG_HANDLE_SCO_NO_CHANGE;

    switch (call_setup_state) {
      case BTHF_CALL_STATE_IDLE: {
        switch (control_block.call_setup_state) {
          case BTHF_CALL_STATE_INCOMING:
            if (num_active > control_block.num_active) {
              res = BTA_AG_IN_CALL_CONN_RES;
              if (is_active_device(*bd_addr)) {
                // initiate SCO only if it is not connected already
                if (btif_hf_cb[idx].audio_state != BTHF_AUDIO_STATE_CONNECTED) {
#if (TWS_AG_ENABLED == TRUE)
                  if (btif_is_tws_plus_device(bd_addr) &&
                      !btif_hf_check_twsp_state_for_sco(idx)) {
                    BTIF_TRACE_DEBUG("%s: Not create SCO since earbud is not in ear",__FUNCTION__);
                  } else {
#endif
                    ag_res.audio_handle = control_block.handle;
                    btif_transfer_context(btif_in_hf_generic_evt, BTIF_HFP_CB_AUDIO_CONNECTING,
                                 (char*)(&btif_hf_cb[idx].connected_bda), sizeof(RawAddress), NULL);
#if (TWS_AG_ENABLED == TRUE)
                  }
#endif
                } else {
                  BTIF_TRACE_IMP("%s: SCO is already connected with device %s, not intiating SCO",
                         __func__, bd_addr->ToString().c_str());
                  ag_res.audio_handle = BTA_AG_HANDLE_SCO_NO_CHANGE;
                }
              }
            } else if (num_held > control_block.num_held)
              res = BTA_AG_IN_CALL_HELD_RES;
            else
              res = BTA_AG_CALL_CANCEL_RES;
            break;
          case BTHF_CALL_STATE_DIALING:
          case BTHF_CALL_STATE_ALERTING:
            if (num_active > control_block.num_active) {
              res = BTA_AG_OUT_CALL_CONN_RES;
            } else
              res = BTA_AG_CALL_CANCEL_RES;
            break;
          default:
            BTIF_TRACE_ERROR("%s: Incorrect call state prev=%d, now=%d",
                             __func__, control_block.call_setup_state,
                             call_setup_state);
            status = BT_STATUS_PARM_INVALID;
            break;
        }
      } break;

      case BTHF_CALL_STATE_INCOMING:
        if (num_active || num_held) {
          res = BTA_AG_CALL_WAIT_RES;
        } else {
          res = BTA_AG_IN_CALL_RES;
          if (is_active_device(*bd_addr) &&
              (btif_hf_features & BTA_AG_FEAT_INBAND)) {
            ag_res.audio_handle = control_block.handle;
            if (BTA_AgInbandEnabled(control_block.handle)) {
              btif_transfer_context(btif_in_hf_generic_evt, BTIF_HFP_CB_AUDIO_CONNECTING,
                (char*)(&btif_hf_cb[idx].connected_bda), sizeof(RawAddress), NULL);
            }
          }
        }
        if (number) {
          std::ostringstream call_number_stream;
          if ((type == BTHF_CALL_ADDRTYPE_INTERNATIONAL) && (*number != '+')) {
            call_number_stream << "\"+";
          } else {
            call_number_stream << "\"";
          }

          std::string name_str;
          if (name) {
            name_str.append(name);
          }
          std::string number_str(number);
          // 13 = ["][+]["][,][3_digit_type][,,,]["]["][null_terminator]
          int overflow_size =
              13 + static_cast<int>(number_str.length() + name_str.length()) -
              static_cast<int>(sizeof(ag_res.str));
          if (overflow_size > 0) {
            android_errorWriteLog(0x534e4554, "79431031");
            int extra_overflow_size =
                overflow_size - static_cast<int>(name_str.length());
            if (extra_overflow_size > 0) {
              number_str.resize(number_str.length() - extra_overflow_size);
              name_str.clear();
            } else {
              name_str.resize(name_str.length() - overflow_size);
            }
          }
          call_number_stream << number_str << "\"";

          // Store caller id string and append type info.
          // Make sure type info is valid, otherwise add 129 as default type
          ag_res.num = static_cast<uint16_t>(type);
          if ((ag_res.num < BTA_AG_CLIP_TYPE_MIN) ||
              (ag_res.num > BTA_AG_CLIP_TYPE_MAX)) {
            if (ag_res.num != BTA_AG_CLIP_TYPE_VOIP) {
              ag_res.num = BTA_AG_CLIP_TYPE_DEFAULT;
            }
          }

          if (res == BTA_AG_CALL_WAIT_RES || name_str.empty()) {
            call_number_stream << "," << std::to_string(ag_res.num);
          } else {
            call_number_stream << "," << std::to_string(ag_res.num) << ",,,\""
                               << name_str << "\"";
          }
          snprintf(ag_res.str, sizeof(ag_res.str), "%s",
                   call_number_stream.str().c_str());
          BTIF_TRACE_EVENT("%s: The CLIP response is number: %s, name: %s, ag_res.str: %s",
                            __func__ , number, name, ag_res.str);
        }
        break;
      case BTHF_CALL_STATE_DIALING:
        if (!(num_active + num_held) && is_active_device(*bd_addr))
        {
#if (TWS_AG_ENABLED == TRUE)
          if (btif_is_tws_plus_device(bd_addr) &&
              !btif_hf_check_twsp_state_for_sco(idx)) {
            BTIF_TRACE_DEBUG("%s: Not create SCO since earbud is not in ear", __FUNCTION__);
          } else {
#endif
            ag_res.audio_handle = control_block.handle;

            btif_transfer_context(btif_in_hf_generic_evt, BTIF_HFP_CB_AUDIO_CONNECTING,
                                  (char*)(&btif_hf_cb[idx].connected_bda), sizeof(RawAddress), NULL);
#if (TWS_AG_ENABLED == TRUE)
          }
#endif
        }
        else
        {
          BTIF_TRACE_DEBUG("%s: Already in a call, don't set audio handle", __FUNCTION__);
        }
        res = BTA_AG_OUT_CALL_ORIG_RES;
        break;
      case BTHF_CALL_STATE_ALERTING:
        /* if we went from idle->alert, force SCO setup here if SCO is not connected already.
         * Dialing usually triggers it */
        if ((control_block.call_setup_state == BTHF_CALL_STATE_IDLE) &&
            !(num_active + num_held) && is_active_device(*bd_addr) &&
            (btif_hf_cb[idx].audio_state != BTHF_AUDIO_STATE_CONNECTED)) {
#if (TWS_AG_ENABLED == TRUE)
          if (btif_is_tws_plus_device(bd_addr) &&
              !btif_hf_check_twsp_state_for_sco(idx)) {
            BTIF_TRACE_DEBUG("%s: Not create SCO since earbud is not in ear", __FUNCTION__);
          } else {
#endif
            ag_res.audio_handle = control_block.handle;

            btif_transfer_context(btif_in_hf_generic_evt, BTIF_HFP_CB_AUDIO_CONNECTING,
                                  (char*)(&btif_hf_cb[idx].connected_bda), sizeof(RawAddress), NULL);
#if (TWS_AG_ENABLED == TRUE)
          }
#endif
        }
        else
        {
          BTIF_TRACE_DEBUG("%s: Already in a call or prev call state was dialing,"
                  " don't set audio handle", __FUNCTION__);
        }
        res = BTA_AG_OUT_CALL_ALERT_RES;
        break;
      default:
        BTIF_TRACE_ERROR("%s: Incorrect call state prev=%d, now=%d", __func__,
                         control_block.call_setup_state, call_setup_state);
        status = BT_STATUS_PARM_INVALID;
        break;
    }
    BTIF_TRACE_IMP("%s: Call setup state changed. res=%d, audio_handle=%d",
                     __func__, res, ag_res.audio_handle);

    if (res)
      BTA_AgResult(control_block.handle, res, &ag_res);

    /* if call setup is idle, we have already updated call indicator, jump out
     */
    if (call_setup_state == BTHF_CALL_STATE_IDLE) {
      /* check & update callheld */
      if ((num_held > 0) && (num_active > 0))
        send_indicator_update(control_block, BTA_AG_IND_CALLHELD, 1);

      UpdateCallStates(&btif_hf_cb[idx], num_active, num_held,
                       call_setup_state);
      return status;
    }
  }

  memset(&ag_res, 0, sizeof(tBTA_AG_RES_DATA));

  /* per the errata 2043, call=1 implies atleast one call is in progress
   *(active/held)
   ** https://www.bluetooth.org/errata/errata_view.cfm?errata_id=2043
   ** Handle call indicator change
   **/
  if (!active_call_updated &&
      ((num_active + num_held) !=
       (control_block.num_active + control_block.num_held))) {
    BTIF_TRACE_IMP("%s: Active call states changed. old: %d new: %d",
                     __func__, control_block.num_active, num_active);
    send_indicator_update(control_block, BTA_AG_IND_CALL,
                          ((num_active + num_held) > 0) ? BTA_AG_CALL_ACTIVE
                                                        : BTA_AG_CALL_INACTIVE);
  }

  /* Held Changed? */
  if (num_held != control_block.num_held ||
      ((num_active == 0) && ((num_held + control_block.num_held) > 1))) {
    BTIF_TRACE_IMP("%s: Held call states changed. old: %d new: %d", __func__,
                     control_block.num_held, num_held);
    send_indicator_update(control_block, BTA_AG_IND_CALLHELD,
                          ((num_held == 0) ? 0 : ((num_active == 0) ? 2 : 1)));
  }

  /* Calls Swapped? */
  if ((call_setup_state == control_block.call_setup_state) &&
      (num_active && num_held) && (num_active == control_block.num_active) &&
      (num_held == control_block.num_held)) {
    BTIF_TRACE_IMP("%s: Calls swapped", __func__);
    send_indicator_update(control_block, BTA_AG_IND_CALLHELD, 1);
  }

  /* When call is hung up and still there is another call is in active,
   * some of the HF cannot acquire the call states by its own. If HF try
   * to terminate a call, it may not send the command AT+CHUP because the
   * call states are not updated properly. HF should get informed the call
   * status forcibly.
   */
  if ((control_block.num_active == num_active && num_active != 0) &&
      (control_block.num_held != num_held && num_held == 0)) {
    memset(&ag_res, 0, sizeof(tBTA_AG_RES_DATA));
    ag_res.ind.id = BTA_AG_IND_CALL;
    ag_res.ind.value = num_active;
    BTA_AgResult(control_block.handle, BTA_AG_IND_RES_ON_DEMAND, &ag_res);
  }

  UpdateCallStates(&btif_hf_cb[idx], num_active, num_held, call_setup_state);

#if (BT_IOT_LOGGING_ENABLED == TRUE)
  if (current_audio_state != BTHF_AUDIO_STATE_CONNECTING &&
      btif_hf_cb[idx].audio_state == BTHF_AUDIO_STATE_CONNECTING)
    device_iot_config_addr_int_add_one(btif_hf_cb[idx].connected_bda,
        IOT_CONF_KEY_HFP_SCO_CONN_COUNT);
#endif
  return status;
}

/*******************************************************************************
*
 * Function         btif_hf_is_call_vr_idle
 *
 * Description      returns true if no call is in progress
 *
 * Returns          bool
 *
 ******************************************************************************/
bool btif_hf_is_call_vr_idle() {
  int i, j = 1;

  if (bt_hf_callbacks == NULL)
    return true;
  for (i = 0; i < btif_max_hf_clients; i++) {
    BTIF_TRACE_EVENT("%s: call_setup_state: %d for handle: %d",
                     __func__, btif_hf_cb[i].call_setup_state,
                     btif_hf_cb[i].handle);
    BTIF_TRACE_EVENT("num_held: %d, num_active: %d for handle: %d",
                     btif_hf_cb[i].num_held, btif_hf_cb[i].num_active,
                     btif_hf_cb[i].handle);
    j &= ((btif_hf_cb[i].call_setup_state == BTHF_CALL_STATE_IDLE) &&
            ((btif_hf_cb[i].num_held + btif_hf_cb[i].num_active) == 0));
  }

  if (j && (btif_hf_check_if_sco_connected() != BT_STATUS_SUCCESS)) {
    BTIF_TRACE_EVENT("%s: call state idle and no sco connected.", __func__);
    return true;
  } else
    return false;

  return true;
}

/*******************************************************************************
 *
 * Function         Cleanup
 *
 * Description      Closes the HF interface
 *
 * Returns          bt_status_t
 *
 ******************************************************************************/
void HeadsetInterface::Cleanup(void) {
  BTIF_TRACE_EVENT("%s", __func__);

  btif_queue_cleanup(UUID_SERVCLASS_AG_HANDSFREE);
#if (defined(BTIF_HF_SERVICES) && (BTIF_HF_SERVICES & BTA_HFP_SERVICE_MASK))
    btif_disable_service(BTA_HFP_SERVICE_ID);
#else
    btif_disable_service(BTA_HSP_SERVICE_ID);
#endif
}

/*******************************************************************************
 *
 * Function         configure_wbs
 *
 * Description      set to over-ride the current WBS configuration.
 *                  It will not send codec setting cmd to the controller now.
 *                  It just change the configure.
 *
 * Returns          bt_status_t
 *
 ******************************************************************************/
/*static bt_status_t configure_wbs(RawAddress* bd_addr,
                                 bthf_wbs_config_t config) {
  CHECK_BTHF_INIT();

  int idx = btif_hf_idx_by_bdaddr(bd_addr);

  if ((idx < 0) || (idx >= BTA_AG_MAX_NUM_CLIENTS)) {
    BTIF_TRACE_ERROR("%s: Invalid index %d", __func__, idx);
    return BT_STATUS_FAIL;
  }

  BTIF_TRACE_EVENT("%s config is %d", __func__, config);
  if (config == BTHF_WBS_YES)
    BTA_AgSetCodec(btif_hf_cb[idx].handle, BTA_AG_CODEC_MSBC);
  else if (config == BTHF_WBS_NO)
    BTA_AgSetCodec(btif_hf_cb[idx].handle, BTA_AG_CODEC_CVSD);
  else
    BTA_AgSetCodec(btif_hf_cb[idx].handle, BTA_AG_CODEC_NONE);

  return BT_STATUS_SUCCESS;
} */ // commenting temp because of HAL change

#ifdef BT_HF_VOIP_FEATURE
static void set_voip_network_type_wifi_hci_cmd_complete(tBTM_VSC_CMPL* p_data)
{
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

/*
/*******************************************************************************
**
** Function         voip_network_type_wifi
**
** Description      BT app updates the connectivity network used for VOIP as Wifi
**
** Returns          bt_status_t
**
*******************************************************************************/
static bt_status_t voip_network_type_wifi(bthf_voip_state_t isVoipStarted,
                                           bthf_voip_call_network_type_t isNetworkWifi)
{
    uint8_t           cmd[3], *p_cursor;
    uint8_t           sub_cmd = HCI_VSC_SUBCODE_VOIP_NETWORK_WIFI;
    int             idx = btif_hf_latest_connected_idx();

    CHECK_BTHF_INIT();

    if ((idx < 0) || (idx >= BTA_AG_MAX_NUM_CLIENTS))
    {
        BTIF_TRACE_ERROR("%s: Invalid index %d", __FUNCTION__, idx);
        return BT_STATUS_FAIL;
    }

    p_cursor = cmd;
    memset(cmd, 0, 3);

    *p_cursor++ = sub_cmd;
    *p_cursor++ = isVoipStarted;
    *p_cursor++ = isNetworkWifi;

    BTM_VendorSpecificCommand(HCI_VSC_VOIP_NETWORK_WIFI_OCF, sizeof(cmd),
            cmd, set_voip_network_type_wifi_hci_cmd_complete);
    return BT_STATUS_SUCCESS;
}
#endif

bt_status_t HeadsetInterface::SendBsir(bool value, RawAddress* bd_addr) {
  CHECK_BTHF_INIT();
  int idx = btif_hf_idx_by_bdaddr(bd_addr);
  if ((idx < 0) || (idx >= BTA_AG_MAX_NUM_CLIENTS)) {
    BTIF_TRACE_ERROR("%s: Invalid index %d for %s", __func__, idx,
                     bd_addr->ToString().c_str());
    return BT_STATUS_FAIL;
  }
  if (!is_connected(bd_addr)) {
    BTIF_TRACE_ERROR("%s: %s not connected", __func__,
                     bd_addr->ToString().c_str());
    return BT_STATUS_FAIL;
  }
  tBTA_AG_RES_DATA ag_res;
  memset(&ag_res, 0, sizeof(tBTA_AG_RES_DATA));
  ag_res.state = value;
  BTA_AgResult(btif_hf_cb[idx].handle, BTA_AG_INBAND_RING_RES, &ag_res);
  return BT_STATUS_SUCCESS;
}

bt_status_t HeadsetInterface::SetActiveDevice(RawAddress* active_device_addr) {
  CHECK_BTHF_INIT();

  if (!active_device_addr->IsEmpty()) {
    //If the app is setting a device as active, which is already active in stack,
    //return success
    if (*active_device_addr == active_bda) {
      BTIF_TRACE_IMP(
        "%s: Allow app to set device: %s as active, which is already active in stack",
         __func__, active_device_addr->ToString().c_str());
      return BT_STATUS_SUCCESS;
    }
    // if SCO is setting up, don't allow active device switch
    for (int i = 0; i < btif_max_hf_clients; i++) {
      if (btif_hf_cb[i].audio_state == BTHF_AUDIO_STATE_CONNECTING) {
         BTIF_TRACE_ERROR("%s: SCO setting up with %s, not allowing active device switch to %s",
          __func__, btif_hf_cb[i].connected_bda.ToString().c_str(),
         active_device_addr->ToString().c_str());
         return BT_STATUS_FAIL;
      }
    }

    active_bda = *active_device_addr;
  } else {
    BTIF_TRACE_IMP("%s: set active bda to Empty", __func__);
    active_bda = RawAddress::kEmpty;
  }
  BTA_AgSetActiveDevice(active_bda);
  return BT_STATUS_SUCCESS;
}

/*******************************************************************************
 *
 * Function         btif_hf_execute_service
 *
 * Description      Initializes/Shuts down the service
 *
 * Returns          BT_STATUS_SUCCESS on success, BT_STATUS_FAIL otherwise
 *
 ******************************************************************************/
bt_status_t btif_hf_execute_service(bool b_enable) {
  const char* p_service_names[] = BTIF_HF_SERVICE_NAMES;
  int i;
  uint8_t no_of_codecs = 0;
  uint8_t* codecs;
  char value[PROPERTY_VALUE_MAX];

  BTIF_TRACE_EVENT("%s: enable: %d", __FUNCTION__, b_enable);

  if (b_enable) {
    /* Enable and register with BTA-AG */
    BTA_AgEnable(BTA_AG_PARSE, bte_hf_evt);
    codecs = controller_get_interface()->get_local_supported_codecs(&no_of_codecs);
    if (codecs != NULL)
    {
        for (i = 0; i < no_of_codecs; i++)
        {
            if (codecs[i] == BTA_AG_MSBC_CODEC)
            {
                btif_hf_features |= BTA_AG_FEAT_CODEC;
                break;
            }
        }
    }
    else
    {
        /* Read the property if local supported codecs commands is not supported */
        if (property_get("ro.vendor.btstack.hfp.ver", value, "1.5") &&
               (!strcmp(value, "1.6") || !strcmp(value, "1.7")))
            btif_hf_features |= BTA_AG_FEAT_CODEC;
    }

    for (uint8_t app_id = 0; app_id < btif_max_hf_clients; app_id++) {
      BTA_AgRegister(BTIF_HF_SERVICES, BTIF_HF_SECURITY, btif_hf_features,
                     p_service_names, app_id);
    }
  } else {
    if (bt_hf_callbacks)
    {
        BTIF_TRACE_EVENT("%s: setting call backs to NULL", __FUNCTION__);
        bt_hf_callbacks = NULL;
    }
    /* De-register AG */
    for (i = 0; i < btif_max_hf_clients; i++) {
      BTA_AgDeregister(btif_hf_cb[i].handle);
    }
    /* Disable AG */
    BTA_AgDisable();
  }
  BTIF_TRACE_EVENT("%s: enable: %d completed", __FUNCTION__, b_enable);
  return BT_STATUS_SUCCESS;
}

/*******************************************************************************
 *
 * Function         GetInterface
 *
 * Description      Get the hf callback interface
 *
 * Returns          (HeadsetInterface)Interface*
 *
 ******************************************************************************/
Interface* GetInterface() {
  VLOG(0) << __func__;
  return HeadsetInterface::GetInstance();
}

}  // namespace headset
}  // n
