/******************************************************************************
 *
 *  Copyright (C) 1999-2012 Broadcom Corporation
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

/******************************************************************************
 *
 *  This file contains functions that handle the SDP server functions.
 *  This is mainly dealing with client requests
 *
 ******************************************************************************/


#include <cutils/log.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bt_common.h"
#include "bt_types.h"
#include "bt_utils.h"
#include "bt_trace.h"
#include "btu.h"
#include "avdt_api.h"

#include "hcidefs.h"
#include "hcimsgs.h"
#include "l2cdefs.h"
#include "avrc_defs.h"
#include <errno.h>
#include "device/include/interop.h"
#include "btif/include/btif_storage.h"
#include "device/include/profile_config.h"
#include <cutils/properties.h>
#include <hardware/bluetooth.h>
#include <hardware/vendor.h>
#include "device/include/interop.h"
#include "osi/include/osi.h"
#include "osi/include/properties.h"
#include "sdp_api.h"
#include "sdpint.h"
#include "btif/include/btif_config.h"
//#include "service/logging_helpers.h"

#include <cutils/properties.h>

#if (SDP_SERVER_ENABLED == TRUE)

/* Maximum number of bytes to reserve out of SDP MTU for response data */
#define SDP_MAX_SERVICE_RSPHDR_LEN 12
#define SDP_MAX_SERVATTR_RSPHDR_LEN 10
#define SDP_MIN_ATTR_REQ_MAX_BYTE_COUNT 7
#define SDP_MAX_ATTR_RSPHDR_LEN 10
#define PBAP_GOEP_L2CAP_PSM_LEN    0x06
#define PBAP_SUPP_FEA_LEN          0x08
#define PROFILE_VERSION_POSITION 7
#define SDP_PROFILE_DESC_LENGTH 8
#define AVRCP_SUPPORTED_FEATURES_POSITION 1
#define AVRCP_PLAYER_APP_SETTINGS_SUPPORT_BITMASK 0x10
#define AVRCP_BROWSE_SUPPORT_BITMASK 0x40
#define AVRCP_MULTI_PLAYER_SUPPORT_BITMASK 0x80
#define AVRCP_CA_SUPPORT_BITMASK 0x01

/******************************************************************************/
/*            L O C A L    F U N C T I O N     P R O T O T Y P E S            */
/******************************************************************************/
static void process_service_search(tCONN_CB* p_ccb, uint16_t trans_num,
                                   uint16_t param_len, uint8_t* p_req,
                                   uint8_t* p_req_end);

static void process_service_attr_req(tCONN_CB* p_ccb, uint16_t trans_num,
                                     uint16_t param_len, uint8_t* p_req,
                                     uint8_t* p_req_end);

static void process_service_search_attr_req(tCONN_CB* p_ccb, uint16_t trans_num,
                                            uint16_t param_len, uint8_t* p_req,
                                            uint8_t* p_req_end);

static bool is_device_blacklisted_for_pbap (RawAddress remote_address,
                                            bool check_for_1_2);

static uint16_t sdp_update_pbap_blacklist_len(tCONN_CB* p_ccb, tSDP_ATTR_SEQ* attr_seq,
                                            tSDP_UUID_SEQ* uid_seq);

static tSDP_RECORD *sdp_upgrade_pse_record(tSDP_RECORD *p_rec,
                                      RawAddress remote_address);

static bool check_remote_pbap_version_102(RawAddress remote_addr);

static tSDP_RECORD *sdp_upgrade_mse_record(tSDP_RECORD *p_rec,
                                      RawAddress remote_address);

static bool check_remote_map_version_104(RawAddress remote_addr);
static bool is_map_adv_enabled();
static bool is_pbap_adv_enabled();

/******************************************************************************/
/*                E R R O R   T E X T   S T R I N G S                         */
/*                                                                            */
/* The default is to have no text string, but we allow the strings to be      */
/* configured in target.h if people want them.                                */
/******************************************************************************/
#ifndef SDP_TEXT_BAD_HEADER
#define SDP_TEXT_BAD_HEADER NULL
#endif

#ifndef SDP_TEXT_BAD_PDU
#define SDP_TEXT_BAD_PDU NULL
#endif

#ifndef SDP_TEXT_BAD_UUID_LIST
#define SDP_TEXT_BAD_UUID_LIST NULL
#endif

#ifndef SDP_TEXT_BAD_HANDLE
#define SDP_TEXT_BAD_HANDLE NULL
#endif

#ifndef SDP_TEXT_BAD_ATTR_LIST
#define SDP_TEXT_BAD_ATTR_LIST NULL
#endif

#ifndef SDP_TEXT_BAD_CONT_LEN
#define SDP_TEXT_BAD_CONT_LEN NULL
#endif

#ifndef SDP_TEXT_BAD_CONT_INX
#define SDP_TEXT_BAD_CONT_INX NULL
#endif

#ifndef SDP_TEXT_BAD_MAX_RECORDS_LIST
#define SDP_TEXT_BAD_MAX_RECORDS_LIST NULL
#endif

#ifndef SDP_TEXT_BAD_MAX_ATTR_LIST
#define SDP_TEXT_BAD_MAX_ATTR_LIST   NULL
#endif

#ifndef SDP_ENABLE_PTS_PBAP
#define SDP_ENABLE_PTS_PBAP  "vendor.bt.pts.pbap"
#endif

#define PBAP_1_2 0x0102
#define PBAP_1_2_BL_LEN 14

#ifndef SDP_ENABLE_PTS_MAP
#define SDP_ENABLE_PTS_MAP  "vendor.bt.pts.map"
#endif

#define MAP_1_4 0x0104

struct blacklist_entry
{
    int ver;
    char addr[3];
};

struct dynamic_upgrade_entry
{
    uint16_t ver;
    char addr[3];
    char rebonded;
};

uint16_t get_dut_avrcp_version() {
    // This api get avrcp version stored in property
    uint16_t profile_version = AVRC_REV_1_0;
    char avrcp_version[PROPERTY_VALUE_MAX] = {0};
    property_get(AVRCP_VERSION_PROPERTY, avrcp_version,
                     AVRCP_1_6_STRING);
    SDP_TRACE_DEBUG(" %s AVRCP version used for sdp: \"%s\"",
             __func__,avrcp_version);

    if (!strncmp(AVRCP_1_6_STRING, avrcp_version,
                 sizeof(AVRCP_1_6_STRING))) {
      profile_version = AVRC_REV_1_6;
    } else if (!strncmp(AVRCP_1_5_STRING, avrcp_version,
                        sizeof(AVRCP_1_5_STRING))) {
      profile_version = AVRC_REV_1_5;
    } else if (!strncmp(AVRCP_1_4_STRING, avrcp_version,
                        sizeof(AVRCP_1_4_STRING))) {
      profile_version = AVRC_REV_1_4;
    } else {
      profile_version = AVRC_REV_1_3;
    }
    return profile_version;
}

/****************************************************************************
**
** Function         sdp_get_stored_avrc_tg_version
**
** Description      This function is called to fetch stored remote's
**                  avrcp version
**
** Returns          int
**
*******************************************************************************/
int sdp_get_stored_avrc_tg_version(RawAddress addr)
{
    uint16_t avrcp_version = 0;
    uint16_t avrcp_features = 0;
    SDP_TRACE_DEBUG("%s target BD Addr: %s",\
             __func__, addr.ToString().c_str());
    bool feature = profile_feature_fetch(AVRCP_ID, AVRCP_COVERART_SUPPORT);
    if (btif_config_get_uint16(addr.ToString().c_str(), AV_REM_CTRL_VERSION_CONFIG_KEY,
                (uint16_t*)&avrcp_version)) {
      SDP_TRACE_DEBUG("%s : Entry found in pairing database, remote version: 0x%x", __func__, avrcp_version);
      if (btif_config_get_uint16(addr.ToString().c_str(), AV_REM_CTRL_FEATURES_CONFIG_KEY, (uint16_t*)&avrcp_features)) {
        bool CA_AVRC_BIT = ((AVRCP_FEAT_CA_BIT & avrcp_features) == AVRCP_FEAT_CA_BIT);
        bool BRW_AVRC_BIT = ((AVRCP_FEAT_BRW_BIT & avrcp_features) == AVRCP_FEAT_BRW_BIT);
        uint16_t dut_avrcp_version = get_dut_avrcp_version();
        uint16_t ver = (dut_avrcp_version > avrcp_version) ?
                           avrcp_version : dut_avrcp_version;
        if (CA_AVRC_BIT & feature)
          ver = AVRCP_MASK_CA_BIT | ver;
        if (BRW_AVRC_BIT)
          ver = AVRCP_MASK_BRW_BIT | ver;
        SDP_TRACE_DEBUG("%s: return version with (CA+BRW): 0x%x", __func__, ver);
        return ver;
      } else {
        SDP_TRACE_DEBUG("%s: failed to fetch features from pairing database, returning AVRC_1_3", __func__);
        return AVRC_REV_1_3;
      }
    } else {
      SDP_TRACE_DEBUG("%s: failed to fetch version from pairing database, returning AVRC_1_3", __func__);
      return AVRC_REV_1_3;
    }
}

/****************************************************************************
**
** Function         sdp_dev_blacklisted_for_avrcp15
**
** Description      This function is called to check if Remote device
**                  is blacklisted for Avrcp version.
**
** Returns          BOOLEAN
**
*******************************************************************************/
bool sdp_dev_blacklisted_for_avrcp15 (RawAddress addr)
{
    if (interop_match_addr(INTEROP_ADV_AVRCP_VER_1_3, &addr)) {
        bt_property_t prop_name;
        bt_bdname_t bdname;

        BTIF_STORAGE_FILL_PROPERTY(&prop_name, BT_PROPERTY_BDNAME,
                               sizeof(bt_bdname_t), &bdname);
        if (btif_storage_get_remote_device_property(&addr,
                                              &prop_name) != BT_STATUS_SUCCESS)
        {
            SDP_TRACE_ERROR("%s: BT_PROPERTY_BDNAME failed, returning false", __func__);
            return FALSE;
        }

        if (strlen((const char *)bdname.name) != 0 &&
            interop_match_name(INTEROP_ADV_AVRCP_VER_1_3, (const char *)bdname.name))
        {
            SDP_TRACE_DEBUG("%s: advertise AVRCP version 1.3 for device", __func__);
            return TRUE;
        }
    }

    return FALSE;
}

/*************************************************************************************
**
** Function        sdp_reset_avrcp_browsing_bit
**
** Description     Checks if Service Class ID is AV Remote Control TG, attribute id
**                 is Supported features and remote BD address
**                 matches device blacklist, reset Browsing Bit
**
** Returns         bool
**
***************************************************************************************/
bool sdp_reset_avrcp_browsing_bit (tSDP_ATTRIBUTE attr, tSDP_ATTRIBUTE *p_attr,
                                      RawAddress remote_address)
{
    if ((p_attr->id == ATTR_ID_SUPPORTED_FEATURES) && (attr.id == ATTR_ID_SERVICE_CLASS_ID_LIST) &&
        (((attr.value_ptr[1] << 8) | (attr.value_ptr[2])) == UUID_SERVCLASS_AV_REM_CTRL_TARGET))
    {
        int version;
        bool browsing_supported = false;
        if (sdp_dev_blacklisted_for_avrcp15 (remote_address))
        {
            SDP_TRACE_ERROR("Reset Browse feature bitmask");
            p_attr->value_ptr[AVRCP_SUPPORTED_FEATURES_POSITION] &= ~AVRCP_BROWSE_SUPPORT_BITMASK;
            p_attr->value_ptr[AVRCP_SUPPORTED_FEATURES_POSITION] &=
                    ~AVRCP_MULTI_PLAYER_SUPPORT_BITMASK;
            p_attr->value_ptr[AVRCP_SUPPORTED_FEATURES_POSITION] &=
                    ~AVRCP_PLAYER_APP_SETTINGS_SUPPORT_BITMASK;
            return TRUE;
        }
        version = sdp_get_stored_avrc_tg_version (remote_address);
        browsing_supported = ((AVRCP_MASK_BRW_BIT & version) == AVRCP_MASK_BRW_BIT);
        version = (AVRCP_VERSION_BIT_MASK & version);
        SDP_TRACE_ERROR("Stored AVRC TG version: 0x%x", version);

        if ((version < AVRC_REV_1_4) || (interop_match_addr_or_name(
                INTEROP_DISABLE_PLAYER_APPLICATION_SETTING_CMDS, &remote_address)))
        {
            p_attr->value_ptr[AVRCP_SUPPORTED_FEATURES_POSITION] &=
                    ~AVRCP_PLAYER_APP_SETTINGS_SUPPORT_BITMASK;
        } else {
            p_attr->value_ptr[AVRCP_SUPPORTED_FEATURES_POSITION] |=
                    AVRCP_PLAYER_APP_SETTINGS_SUPPORT_BITMASK;
        }

#if (defined(AVRC_QTI_V1_3_OPTIONAL_FEAT) && AVRC_QTI_V1_3_OPTIONAL_FEAT == TRUE)
#else
        p_attr->value_ptr[AVRCP_SUPPORTED_FEATURES_POSITION] &=
                ~AVRCP_PLAYER_APP_SETTINGS_SUPPORT_BITMASK;
#endif

        if (version < AVRC_REV_1_4 || !browsing_supported)
        {
            SDP_TRACE_ERROR("Reset Browse feature bitmask");
            p_attr->value_ptr[AVRCP_SUPPORTED_FEATURES_POSITION] &= ~AVRCP_BROWSE_SUPPORT_BITMASK;
            p_attr->value_ptr[AVRCP_SUPPORTED_FEATURES_POSITION] &=
                    ~AVRCP_MULTI_PLAYER_SUPPORT_BITMASK;
            return TRUE;
        }
        if (version >= AVRC_REV_1_4 && browsing_supported)
        {
            SDP_TRACE_ERROR("Set Browse feature bitmask");
            p_attr->value_ptr[AVRCP_SUPPORTED_FEATURES_POSITION] |= AVRCP_BROWSE_SUPPORT_BITMASK;
            p_attr->value_ptr[AVRCP_SUPPORTED_FEATURES_POSITION] |=
                    AVRCP_MULTI_PLAYER_SUPPORT_BITMASK;
            return FALSE;
        }
    }
    return FALSE;
}
/*************************************************************************************
**
** Function        sdp_reset_avrcp_cover_art_bit
**
** Description     Checks if Service Class ID is AV Remote Control TG, attribute id
**                 is Supported features and remote BD address
**                 matches device blacklist, reset Cover Art Bit
**
** Returns         BOOLEAN
**
***************************************************************************************/

bool sdp_reset_avrcp_cover_art_bit (tSDP_ATTRIBUTE attr, tSDP_ATTRIBUTE *p_attr,
                                                 RawAddress remote_address)
{
    if ((p_attr->id == ATTR_ID_SUPPORTED_FEATURES) && (attr.id == ATTR_ID_SERVICE_CLASS_ID_LIST) &&
        (((attr.value_ptr[1] << 8) | (attr.value_ptr[2])) == UUID_SERVCLASS_AV_REM_CTRL_TARGET))
    {
        int version;
        bool cover_art_supported = false;
        version = sdp_get_stored_avrc_tg_version (remote_address);
        cover_art_supported = ((AVRCP_MASK_CA_BIT & version) == AVRCP_MASK_CA_BIT);
        version = (AVRCP_VERSION_BIT_MASK & version);
        SDP_TRACE_ERROR("Stored AVRC TG version: 0x%x", version);
        if (version < AVRC_REV_1_6 || !cover_art_supported)
        {
            SDP_TRACE_ERROR("Reset Cover Art feature bitmask +1, 0x%x", p_attr->value_ptr[AVRCP_SUPPORTED_FEATURES_POSITION+1]);
            SDP_TRACE_ERROR("Reset Cover Art feature bitmask -1, 0x%x", p_attr->value_ptr[AVRCP_SUPPORTED_FEATURES_POSITION-1]);
            p_attr->value_ptr[AVRCP_SUPPORTED_FEATURES_POSITION-1] &= ~AVRCP_CA_SUPPORT_BITMASK;
            SDP_TRACE_ERROR("Reset Cover Art feature bitmask, new -1, 0x%x", p_attr->value_ptr[AVRCP_SUPPORTED_FEATURES_POSITION-1]);
            return TRUE;
        }
        if (version == AVRC_REV_1_6 && cover_art_supported)
        {
            SDP_TRACE_ERROR("Set Cover Art feature bitmask +1, 0x%x", p_attr->value_ptr[AVRCP_SUPPORTED_FEATURES_POSITION+1]);
            SDP_TRACE_ERROR("Set Cover Art feature bitmask -1, 0x%x", p_attr->value_ptr[AVRCP_SUPPORTED_FEATURES_POSITION-1]);
            p_attr->value_ptr[AVRCP_SUPPORTED_FEATURES_POSITION-1] |= AVRCP_CA_SUPPORT_BITMASK;
            SDP_TRACE_ERROR("Set Cover Art feature bitmask, new -1, 0x%x", p_attr->value_ptr[AVRCP_SUPPORTED_FEATURES_POSITION-1]);
            return FALSE;
        }
    }
    return FALSE;
}

/*************************************************************************************
**
** Function        sdp_change_hfp_version
**
** Description     Checks if UUID is AG_HANDSFREE, attribute id
**                 is Profile descriptor list and remote BD address
**                 matches device blacklist, change hfp version to 1.7
**
** Returns         BOOLEAN
**
+***************************************************************************************/
bool sdp_change_hfp_version (tSDP_ATTRIBUTE *p_attr, RawAddress remote_address)
{
    bool is_blacklisted = FALSE;
    char value[PROPERTY_VALUE_MAX];
    if ((p_attr->id == ATTR_ID_BT_PROFILE_DESC_LIST) &&
        (p_attr->len >= SDP_PROFILE_DESC_LENGTH))
    {
        /* As per current DB implementation UUID is condidered as 16 bit */
        if (((p_attr->value_ptr[3] << 8) | (p_attr->value_ptr[4])) ==
                UUID_SERVCLASS_HF_HANDSFREE)
        {
            is_blacklisted = interop_match_addr_or_name(INTEROP_HFP_1_7_BLACKLIST,
                                                           &remote_address);
            SDP_TRACE_DEBUG("%s: HF version is 1.7 for BD addr: %s",\
                           __func__, remote_address.ToString().c_str());
            /* For PTS we should show AG's HFP version as 1.7 */
            if (is_blacklisted ||
                (property_get("vendor.bt.pts.certification", value, "false") &&
                strcmp(value, "true") == 0))
            {
                p_attr->value_ptr[PROFILE_VERSION_POSITION] = 0x07; // Update HFP version as 1.7
                SDP_TRACE_ERROR("SDP Change HFP Version = 0x%x",
                         p_attr->value_ptr[PROFILE_VERSION_POSITION]);
                return TRUE;
            }
        }
    }
    return FALSE;
}

/*******************************************************************************
 *
 * Function         sdp_server_handle_client_req
 *
 * Description      This is the main dispatcher of the SDP server. It is called
 *                  when any data is received from L2CAP, and dispatches the
 *                  request to the appropriate handler.
 *
 * Returns          void
 *
 ******************************************************************************/
void sdp_server_handle_client_req(tCONN_CB* p_ccb, BT_HDR* p_msg) {
  uint8_t* p_req = (uint8_t*)(p_msg + 1) + p_msg->offset;
  uint8_t* p_req_end = p_req + p_msg->len;
  uint8_t pdu_id;
  uint16_t trans_num, param_len;

  /* Start inactivity timer */
  alarm_set_on_mloop(p_ccb->sdp_conn_timer, SDP_INACT_TIMEOUT_MS,
                     sdp_conn_timer_timeout, p_ccb);

  if (p_req + sizeof(pdu_id) + sizeof(trans_num) > p_req_end) {
    android_errorWriteLog(0x534e4554, "69384124");
    trans_num = 0;
    sdpu_build_n_send_error(p_ccb, trans_num, SDP_INVALID_REQ_SYNTAX,
                            SDP_TEXT_BAD_HEADER);
  }

  /* The first byte in the message is the pdu type */
  pdu_id = *p_req++;

  /* Extract the transaction number and parameter length */
  BE_STREAM_TO_UINT16(trans_num, p_req);

  if (p_req + sizeof(param_len) > p_req_end) {
    android_errorWriteLog(0x534e4554, "69384124");
    sdpu_build_n_send_error(p_ccb, trans_num, SDP_INVALID_REQ_SYNTAX,
                            SDP_TEXT_BAD_HEADER);
  }

  BE_STREAM_TO_UINT16(param_len, p_req);

  if ((p_req + param_len) != p_req_end) {
    sdpu_build_n_send_error(p_ccb, trans_num, SDP_INVALID_PDU_SIZE,
                            SDP_TEXT_BAD_HEADER);
    return;
  }

  switch (pdu_id) {
    case SDP_PDU_SERVICE_SEARCH_REQ:
      process_service_search(p_ccb, trans_num, param_len, p_req, p_req_end);
      break;

    case SDP_PDU_SERVICE_ATTR_REQ:
      process_service_attr_req(p_ccb, trans_num, param_len, p_req, p_req_end);
      break;

    case SDP_PDU_SERVICE_SEARCH_ATTR_REQ:
      process_service_search_attr_req(p_ccb, trans_num, param_len, p_req,
                                      p_req_end);
      break;

    default:
      sdpu_build_n_send_error(p_ccb, trans_num, SDP_INVALID_REQ_SYNTAX,
                              SDP_TEXT_BAD_PDU);
      SDP_TRACE_WARNING("SDP - server got unknown PDU: 0x%x", pdu_id);
      break;
  }
}

/*******************************************************************************
 *
 * Function         process_service_search
 *
 * Description      This function handles a service search request from the
 *                  client. It builds a reply message with info from the
 *                  database, and sends the reply back to the client.
 *
 * Returns          void
 *
 ******************************************************************************/
static void process_service_search(tCONN_CB* p_ccb, uint16_t trans_num,
                                   uint16_t param_len, uint8_t* p_req,
                                   uint8_t* p_req_end) {
  uint16_t max_replies, cur_handles, rem_handles, cont_offset;
  tSDP_UUID_SEQ uid_seq;
  uint8_t *p_rsp, *p_rsp_start, *p_rsp_param_len;
  uint16_t rsp_param_len, num_rsp_handles, xx;
  uint32_t rsp_handles[SDP_MAX_RECORDS] = {0};
  tSDP_RECORD* p_rec = NULL;
  bool is_cont = false;

  p_req = sdpu_extract_uid_seq(p_req, param_len, &uid_seq);

  if ((!p_req) || (!uid_seq.num_uids)) {
    sdpu_build_n_send_error(p_ccb, trans_num, SDP_INVALID_REQ_SYNTAX,
                            SDP_TEXT_BAD_UUID_LIST);
    return;
  }

  /* Get the max replies we can send. Cap it at our max anyways. */
  if (p_req + sizeof(max_replies) > p_req_end) {
    android_errorWriteLog(0x534e4554, "69384124");
    sdpu_build_n_send_error(p_ccb, trans_num, SDP_INVALID_REQ_SYNTAX,
                            SDP_TEXT_BAD_MAX_RECORDS_LIST);
    return;
  }
  BE_STREAM_TO_UINT16(max_replies, p_req);
  if (!max_replies) {
    sdpu_build_n_send_error (p_ccb, trans_num, SDP_INVALID_REQ_SYNTAX,
                             SDP_TEXT_BAD_MAX_ATTR_LIST);
    return;
  }

  if (max_replies > SDP_MAX_RECORDS) max_replies = SDP_MAX_RECORDS;

  /* Get a list of handles that match the UUIDs given to us */
  for (num_rsp_handles = 0; num_rsp_handles < max_replies;) {
    p_rec = sdp_db_service_search(p_rec, &uid_seq);

    if (p_rec)
      rsp_handles[num_rsp_handles++] = p_rec->record_handle;
    else
      break;
  }

  /* Check if this is a continuation request */
  if (*p_req) {
    if (*p_req++ != SDP_CONTINUATION_LEN ||
        (p_req + sizeof(cont_offset) > p_req_end)) {
      sdpu_build_n_send_error(p_ccb, trans_num, SDP_INVALID_CONT_STATE,
                              SDP_TEXT_BAD_CONT_LEN);
      return;
    }
    BE_STREAM_TO_UINT16(cont_offset, p_req);

    if (cont_offset != p_ccb->cont_offset || num_rsp_handles < cont_offset) {
      sdpu_build_n_send_error(p_ccb, trans_num, SDP_INVALID_CONT_STATE,
                              SDP_TEXT_BAD_CONT_INX);
      return;
    }

    if (p_req != p_req_end) {
      sdpu_build_n_send_error (p_ccb, trans_num, SDP_INVALID_PDU_SIZE, SDP_TEXT_BAD_HEADER);
      return;
    }
    rem_handles =
        num_rsp_handles - cont_offset; /* extract the remaining handles */
  } else {
    if (p_req+1 != p_req_end) {
      sdpu_build_n_send_error (p_ccb, trans_num, SDP_INVALID_PDU_SIZE, SDP_TEXT_BAD_HEADER);
      return;
    }
    rem_handles = num_rsp_handles;
    cont_offset = 0;
    p_ccb->cont_offset = 0;
  }

  /* Calculate how many handles will fit in one PDU */
  cur_handles =
      (uint16_t)((p_ccb->rem_mtu_size - SDP_MAX_SERVICE_RSPHDR_LEN) / 4);

  if (rem_handles <= cur_handles)
    cur_handles = rem_handles;
  else /* Continuation is set */
  {
    p_ccb->cont_offset += cur_handles;
    is_cont = true;
  }

  /* Get a buffer to use to build the response */
  BT_HDR* p_buf = (BT_HDR*)osi_malloc(SDP_DATA_BUF_SIZE);
  p_buf->offset = L2CAP_MIN_OFFSET;
  p_rsp = p_rsp_start = (uint8_t*)(p_buf + 1) + L2CAP_MIN_OFFSET;

  /* Start building a rsponse */
  UINT8_TO_BE_STREAM(p_rsp, SDP_PDU_SERVICE_SEARCH_RSP);
  UINT16_TO_BE_STREAM(p_rsp, trans_num);

  /* Skip the length, we need to add it at the end */
  p_rsp_param_len = p_rsp;
  p_rsp += 2;

  /* Put in total and current number of handles, and handles themselves */
  UINT16_TO_BE_STREAM(p_rsp, num_rsp_handles);
  UINT16_TO_BE_STREAM(p_rsp, cur_handles);

  /*  SDP_TRACE_DEBUG("SDP Service Rsp: tothdl %d, curhdlr %d, start %d, end %d,
     cont %d",
                       num_rsp_handles, cur_handles, cont_offset,
                       cont_offset + cur_handles-1, is_cont); */
  for (xx = cont_offset; xx < cont_offset + cur_handles; xx++)
    UINT32_TO_BE_STREAM(p_rsp, rsp_handles[xx]);

  if (is_cont) {
    UINT8_TO_BE_STREAM(p_rsp, SDP_CONTINUATION_LEN);
    UINT16_TO_BE_STREAM(p_rsp, p_ccb->cont_offset);
  } else
    UINT8_TO_BE_STREAM(p_rsp, 0);

  /* Go back and put the parameter length into the buffer */
  rsp_param_len = p_rsp - p_rsp_param_len - 2;
  UINT16_TO_BE_STREAM(p_rsp_param_len, rsp_param_len);

  /* Set the length of the SDP data in the buffer */
  p_buf->len = p_rsp - p_rsp_start;

  /* Send the buffer through L2CAP */
  L2CA_DataWrite(p_ccb->connection_id, p_buf);
}

/*******************************************************************************
 *
 * Function         process_service_attr_req
 *
 * Description      This function handles an attribute request from the client.
 *                  It builds a reply message with info from the database,
 *                  and sends the reply back to the client.
 *
 * Returns          void
 *
 ******************************************************************************/
static void process_service_attr_req(tCONN_CB* p_ccb, uint16_t trans_num,
                                     uint16_t param_len, uint8_t* p_req,
                                     uint8_t* p_req_end) {
  uint16_t max_list_len, len_to_send, cont_offset;
  int16_t rem_len;
  char a2dp_role[PROPERTY_VALUE_MAX] = "false";
  tSDP_ATTR_SEQ attr_seq, attr_seq_sav;
  uint8_t *p_rsp, *p_rsp_start, *p_rsp_param_len;
  uint16_t rsp_param_len, xx;
  uint32_t rec_handle;
  tSDP_RECORD* p_rec;
  tSDP_ATTRIBUTE* p_attr;
  bool is_cont = false;
  bool is_hfp_fallback = FALSE;
  uint16_t attr_len;
  uint16_t profile_version;
  bool is_avrcp_browse_bit_set = FALSE;
  bool is_avrcp_cover_bit_set = FALSE;
  if (p_req + sizeof(rec_handle) + sizeof(max_list_len) > p_req_end) {
    android_errorWriteLog(0x534e4554, "69384124");
    sdpu_build_n_send_error(p_ccb, trans_num, SDP_INVALID_SERV_REC_HDL,
                            SDP_TEXT_BAD_HANDLE);
    return;
  }

  /* Extract the record handle */
  BE_STREAM_TO_UINT32(rec_handle, p_req);
  param_len -= sizeof(rec_handle);

  /* Get the max list length we can send. Cap it at MTU size minus overhead */
  BE_STREAM_TO_UINT16(max_list_len, p_req);
  param_len -= sizeof(max_list_len);

    if (max_list_len < SDP_MIN_ATTR_REQ_MAX_BYTE_COUNT)
    {
        sdpu_build_n_send_error (p_ccb, trans_num, SDP_INVALID_REQ_SYNTAX,
                                 SDP_TEXT_BAD_MAX_ATTR_LIST);
        return;
    }

    if (max_list_len > (p_ccb->rem_mtu_size - SDP_MAX_ATTR_RSPHDR_LEN))
        max_list_len = p_ccb->rem_mtu_size - SDP_MAX_ATTR_RSPHDR_LEN;

  param_len = static_cast<uint16_t>(p_req_end - p_req);
  p_req = sdpu_extract_attr_seq(p_req, param_len, &attr_seq);

  if ((!p_req) || (!attr_seq.num_attr)) {
    sdpu_build_n_send_error(p_ccb, trans_num, SDP_INVALID_REQ_SYNTAX,
                            SDP_TEXT_BAD_ATTR_LIST);
    return;
  }

  memcpy(&attr_seq_sav, &attr_seq, sizeof(tSDP_ATTR_SEQ));

  /* Find a record with the record handle */
  p_rec = sdp_db_find_record(rec_handle);
  if (!p_rec) {
    sdpu_build_n_send_error(p_ccb, trans_num, SDP_INVALID_SERV_REC_HDL,
                            SDP_TEXT_BAD_HANDLE);
    return;
  }
  if (is_map_adv_enabled()) {
    p_rec = sdp_upgrade_mse_record(p_rec, p_ccb->device_address);
  }
  if(is_pbap_adv_enabled()) {
    p_rec = sdp_upgrade_pse_record(p_rec, p_ccb->device_address);
  }
  /* Free and reallocate buffer */
  osi_free(p_ccb->rsp_list);
  p_ccb->rsp_list = (uint8_t*)osi_malloc(max_list_len);

  /* Check if this is a continuation request */
  if (*p_req) {
    if (*p_req++ != SDP_CONTINUATION_LEN ||
        (p_req + sizeof(cont_offset) > p_req_end)) {
      sdpu_build_n_send_error(p_ccb, trans_num, SDP_INVALID_CONT_STATE,
                              SDP_TEXT_BAD_CONT_LEN);
      return;
    }
    BE_STREAM_TO_UINT16(cont_offset, p_req);

    if (cont_offset != p_ccb->cont_offset) {
      sdpu_build_n_send_error(p_ccb, trans_num, SDP_INVALID_CONT_STATE,
                              SDP_TEXT_BAD_CONT_INX);
      return;
    }
    if (p_req != p_req_end) {
      sdpu_build_n_send_error (p_ccb, trans_num, SDP_INVALID_PDU_SIZE, SDP_TEXT_BAD_HEADER);
      return;
    }
    is_cont = true;

    /* Initialise for continuation response */
    p_rsp = &p_ccb->rsp_list[0];
    attr_seq.attr_entry[p_ccb->cont_info.next_attr_index].start =
        p_ccb->cont_info.next_attr_start_id;
  } else {
    if (p_req+1 != p_req_end) {
      sdpu_build_n_send_error (p_ccb, trans_num, SDP_INVALID_PDU_SIZE, SDP_TEXT_BAD_HEADER);
      return;
    }

    p_ccb->cont_offset = 0;
    p_rsp = &p_ccb->rsp_list[3]; /* Leave space for data elem descr */

    /* Reset continuation parameters in p_ccb */
    p_ccb->cont_info.prev_sdp_rec = NULL;
    p_ccb->cont_info.curr_sdp_rec = NULL;
    p_ccb->cont_info.next_attr_index = 0;
    p_ccb->cont_info.attr_offset = 0;
  }
  /* Search for attributes that match the list given to us */
  for (xx = p_ccb->cont_info.next_attr_index; xx < attr_seq.num_attr; xx++) {
    p_attr = sdp_db_find_attr_in_rec(p_rec, attr_seq.attr_entry[xx].start,
                                     attr_seq.attr_entry[xx].end);

    if (p_attr) {
    /*
     * If DUT profile version is 1.6, we are going to show 1.6.
     * Entry in file would be remote's actual version, but no action would be taken
     */
     /*
      *  There is no point in resetting CA bit, because if DUT 1.6, we have to show 1.6
      *  even if remote misbhevaes. If we DUT is not 1.6 then there would be no ca bit
      */
      if (sdp_reset_avrcp_browsing_bit(p_rec->attribute[1], p_attr, p_ccb->device_address))
        is_avrcp_browse_bit_set = FALSE;
      if (sdp_reset_avrcp_cover_art_bit(p_rec->attribute[1], p_attr, p_ccb->device_address))
        is_avrcp_cover_bit_set = FALSE;
      if ((p_attr->id == ATTR_ID_BT_PROFILE_DESC_LIST) &&
             (p_attr->len >= SDP_PROFILE_DESC_LENGTH)) {
        if (((p_attr->value_ptr[3] << 8) | (p_attr->value_ptr[4])) ==
                UUID_SERVCLASS_AV_REMOTE_CONTROL) {
          property_get("persist.vendor.service.bt.a2dp.sink", a2dp_role, "false");
          if (!strncmp("false", a2dp_role, 5)) {
            profile_version = sdp_get_stored_avrc_tg_version(p_ccb->device_address);
            uint16_t ver = (AVRCP_VERSION_BIT_MASK & profile_version);
            if (ver >= AVRC_REV_1_4) {
              p_attr->value_ptr[PROFILE_VERSION_POSITION] = (uint8_t)(ver & 0x00ff);
              SDP_TRACE_DEBUG("%s :Showing AVRCP version in SDP = 0x%x", __func__,
                               p_attr->value_ptr[PROFILE_VERSION_POSITION]);
            } else {
              p_attr->value_ptr[PROFILE_VERSION_POSITION] = 0x03; // Update AVRCP version as 1.3
              SDP_TRACE_DEBUG("%s : remote doesn't support cover art and browsing, showing AVRCP Version = 0x%x", __func__,
                                 p_attr->value_ptr[PROFILE_VERSION_POSITION]);
            }
            if (sdp_dev_blacklisted_for_avrcp15 (p_ccb->device_address)) {
              p_attr->value_ptr[PROFILE_VERSION_POSITION] = 0x03; // Update AVRCP version as 1.3
              SDP_TRACE_DEBUG("%s :Device is blacklisted updating AVRCP Version = 0x%x", __func__,
                                 p_attr->value_ptr[PROFILE_VERSION_POSITION]);
            }
          }
        }
      }
      is_hfp_fallback = sdp_change_hfp_version (p_attr, p_ccb->device_address);
      /* Check if attribute fits. Assume 3-byte value type/length */
      rem_len = max_list_len - (int16_t)(p_rsp - &p_ccb->rsp_list[0]);

      /* just in case */
      if (rem_len <= 0) {
        p_ccb->cont_info.next_attr_index = xx;
        p_ccb->cont_info.next_attr_start_id = p_attr->id;
        break;
      }

      attr_len = sdpu_get_attrib_entry_len(p_attr);
      /* if there is a partial attribute pending to be sent */
      if (p_ccb->cont_info.attr_offset) {
        if (attr_len < p_ccb->cont_info.attr_offset) {
          android_errorWriteLog(0x534e4554, "79217770");
          LOG(ERROR) << "offset is bigger than attribute length";
          sdpu_build_n_send_error(p_ccb, trans_num, SDP_INVALID_CONT_STATE,
                                  SDP_TEXT_BAD_CONT_LEN);
          return;
        }
        p_rsp = sdpu_build_partial_attrib_entry(p_rsp, p_attr, rem_len,
                                                &p_ccb->cont_info.attr_offset);

        /* If the partial attrib could not been fully added yet */
        if (p_ccb->cont_info.attr_offset != attr_len)
          break;
        else /* If the partial attrib has been added in full by now */
          p_ccb->cont_info.attr_offset = 0; /* reset attr_offset */
      } else if (rem_len <
                 attr_len) /* Not enough space for attr... so add partially */
      {
        if (attr_len >= SDP_MAX_ATTR_LEN) {
          SDP_TRACE_ERROR("SDP attr too big: max_list_len=%d,attr_len=%d",
                          max_list_len, attr_len);
          sdpu_build_n_send_error(p_ccb, trans_num, SDP_NO_RESOURCES, NULL);
          return;
        }

        /* add the partial attribute if possible */
        p_rsp = sdpu_build_partial_attrib_entry(
            p_rsp, p_attr, (uint16_t)rem_len, &p_ccb->cont_info.attr_offset);

        p_ccb->cont_info.next_attr_index = xx;
        p_ccb->cont_info.next_attr_start_id = p_attr->id;
        break;
      } else /* build the whole attribute */
        p_rsp = sdpu_build_attrib_entry(p_rsp, p_attr);

      /* If doing a range, stick with this one till no more attributes found */
      if (attr_seq.attr_entry[xx].start != attr_seq.attr_entry[xx].end) {
        /* Update for next time through */
        attr_seq.attr_entry[xx].start = p_attr->id + 1;

        xx--;
      }
      if (is_hfp_fallback) {
          SDP_TRACE_ERROR("Restore HFP version to 1.6");
          /* Update HFP version back to 1.6 */
          p_attr->value_ptr[PROFILE_VERSION_POSITION] = 0x06;
          is_hfp_fallback = FALSE;
      }
    }
  }
  if (is_hfp_fallback) {
      SDP_TRACE_ERROR("Restore HFP version to 1.6");
      /* Update HFP version back to 1.6 */
      p_attr->value_ptr[PROFILE_VERSION_POSITION] = 0x06;
      is_hfp_fallback = FALSE;
  }

  /* If all the attributes have been accomodated in p_rsp,
     reset next_attr_index */
  if (xx == attr_seq.num_attr) p_ccb->cont_info.next_attr_index = 0;

  len_to_send = (uint16_t)(p_rsp - &p_ccb->rsp_list[0]);
  cont_offset = 0;

  if (!is_cont) {
    p_ccb->list_len = sdpu_get_attrib_seq_len(p_rec, &attr_seq_sav) + 3;
    /* Put in the sequence header (2 or 3 bytes) */
    if (p_ccb->list_len > 255) {
      p_ccb->rsp_list[0] =
          (uint8_t)((DATA_ELE_SEQ_DESC_TYPE << 3) | SIZE_IN_NEXT_WORD);
      p_ccb->rsp_list[1] = (uint8_t)((p_ccb->list_len - 3) >> 8);
      p_ccb->rsp_list[2] = (uint8_t)(p_ccb->list_len - 3);
    } else {
      cont_offset = 1;

      p_ccb->rsp_list[1] =
          (uint8_t)((DATA_ELE_SEQ_DESC_TYPE << 3) | SIZE_IN_NEXT_BYTE);
      p_ccb->rsp_list[2] = (uint8_t)(p_ccb->list_len - 3);

      p_ccb->list_len--;
      len_to_send--;
    }
  }

  /* Get a buffer to use to build the response */
  BT_HDR* p_buf = (BT_HDR*)osi_malloc(SDP_DATA_BUF_SIZE);
  p_buf->offset = L2CAP_MIN_OFFSET;
  p_rsp = p_rsp_start = (uint8_t*)(p_buf + 1) + L2CAP_MIN_OFFSET;

  /* Start building a rsponse */
  UINT8_TO_BE_STREAM(p_rsp, SDP_PDU_SERVICE_ATTR_RSP);
  UINT16_TO_BE_STREAM(p_rsp, trans_num);

  /* Skip the parameter length, add it when we know the length */
  p_rsp_param_len = p_rsp;
  p_rsp += 2;

  UINT16_TO_BE_STREAM(p_rsp, len_to_send);

  memcpy(p_rsp, &p_ccb->rsp_list[cont_offset], len_to_send);
  p_rsp += len_to_send;

  p_ccb->cont_offset += len_to_send;

  /* If anything left to send, continuation needed */
  if (p_ccb->cont_offset < p_ccb->list_len) {
    is_cont = true;

    UINT8_TO_BE_STREAM(p_rsp, SDP_CONTINUATION_LEN);
    UINT16_TO_BE_STREAM(p_rsp, p_ccb->cont_offset);
  } else
    UINT8_TO_BE_STREAM(p_rsp, 0);

  /* Go back and put the parameter length into the buffer */
  rsp_param_len = p_rsp - p_rsp_param_len - 2;
  UINT16_TO_BE_STREAM(p_rsp_param_len, rsp_param_len);

  /* Set the length of the SDP data in the buffer */
  p_buf->len = p_rsp - p_rsp_start;

  /* Send the buffer through L2CAP */
  L2CA_DataWrite(p_ccb->connection_id, p_buf);
}

/*******************************************************************************
 *
 * Function         process_service_search_attr_req
 *
 * Description      This function handles a combined service search and
 *                  attribute read request from the client. It builds a reply
 *                  message with info from the database, and sends the reply
 *                  back to the client.
 *
 * Returns          void
 *
 ******************************************************************************/
static void process_service_search_attr_req(tCONN_CB* p_ccb, uint16_t trans_num,
                                            uint16_t param_len, uint8_t* p_req,
                                            uint8_t* p_req_end) {
  uint16_t max_list_len;
  int16_t rem_len;
  char a2dp_role[PROPERTY_VALUE_MAX] = "false";
  uint16_t len_to_send, cont_offset;
  tSDP_UUID_SEQ uid_seq;
  uint8_t *p_rsp, *p_rsp_start, *p_rsp_param_len;
  uint16_t rsp_param_len, xx;
  tSDP_RECORD* p_rec;
  tSDP_RECORD* p_prev_rec;
  tSDP_ATTR_SEQ attr_seq, attr_seq_sav;
  tSDP_ATTRIBUTE* p_attr;
  bool maxxed_out = false, is_cont = false;
  uint8_t* p_seq_start = NULL;
  bool is_hfp_fallback = FALSE;
  uint16_t seq_len, attr_len;
  uint16_t profile_version;
  bool is_avrcp_browse_bit_set = FALSE;
  bool is_avrcp_cover_bit_set = FALSE;
  /* Extract the UUID sequence to search for */
  p_req = sdpu_extract_uid_seq(p_req, param_len, &uid_seq);

  if ((!p_req) || (!uid_seq.num_uids) ||
      (p_req + sizeof(uint16_t) > p_req_end)) {
    sdpu_build_n_send_error(p_ccb, trans_num, SDP_INVALID_REQ_SYNTAX,
                            SDP_TEXT_BAD_UUID_LIST);
    return;
  }

  /* Get the max list length we can send. Cap it at our max list length. */
  BE_STREAM_TO_UINT16(max_list_len, p_req);

    if (max_list_len < SDP_MIN_ATTR_REQ_MAX_BYTE_COUNT)
    {
        sdpu_build_n_send_error (p_ccb, trans_num, SDP_INVALID_REQ_SYNTAX,
                                 SDP_TEXT_BAD_MAX_ATTR_LIST);
        return;
    }

    if (max_list_len > (p_ccb->rem_mtu_size - SDP_MAX_SERVATTR_RSPHDR_LEN))
        max_list_len = p_ccb->rem_mtu_size - SDP_MAX_SERVATTR_RSPHDR_LEN;

  param_len = static_cast<uint16_t>(p_req_end - p_req);
  p_req = sdpu_extract_attr_seq(p_req, param_len, &attr_seq);

  if ((!p_req) || (!attr_seq.num_attr)) {
    sdpu_build_n_send_error(p_ccb, trans_num, SDP_INVALID_REQ_SYNTAX,
                            SDP_TEXT_BAD_ATTR_LIST);
    return;
  }

  memcpy(&attr_seq_sav, &attr_seq, sizeof(tSDP_ATTR_SEQ));

  /* Free and reallocate buffer */
  osi_free(p_ccb->rsp_list);
  p_ccb->rsp_list = (uint8_t*)osi_malloc(max_list_len);

  /* Check if this is a continuation request */
  if (*p_req) {
    if (*p_req++ != SDP_CONTINUATION_LEN ||
        (p_req + sizeof(uint16_t) > p_req_end)) {
      sdpu_build_n_send_error(p_ccb, trans_num, SDP_INVALID_CONT_STATE,
                              SDP_TEXT_BAD_CONT_LEN);
      return;
    }
    BE_STREAM_TO_UINT16(cont_offset, p_req);

    if (cont_offset != p_ccb->cont_offset) {
      sdpu_build_n_send_error(p_ccb, trans_num, SDP_INVALID_CONT_STATE,
                              SDP_TEXT_BAD_CONT_INX);
      return;
    }
    if (p_req != p_req_end) {
      sdpu_build_n_send_error (p_ccb, trans_num, SDP_INVALID_PDU_SIZE, SDP_TEXT_BAD_HEADER);
      return;
    }
    is_cont = true;

    /* Initialise for continuation response */
    p_rsp = &p_ccb->rsp_list[0];
    attr_seq.attr_entry[p_ccb->cont_info.next_attr_index].start =
        p_ccb->cont_info.next_attr_start_id;
  } else {
    if (p_req+1 != p_req_end) {
      sdpu_build_n_send_error (p_ccb, trans_num, SDP_INVALID_PDU_SIZE, SDP_TEXT_BAD_HEADER);
      return;
    }
    p_ccb->cont_offset = 0;
    p_rsp = &p_ccb->rsp_list[3]; /* Leave space for data elem descr */

    /* Reset continuation parameters in p_ccb */
    p_ccb->cont_info.prev_sdp_rec = NULL;
    p_ccb->cont_info.curr_sdp_rec = NULL;
    p_ccb->cont_info.next_attr_index = 0;
    p_ccb->cont_info.last_attr_seq_desc_sent = false;
    p_ccb->cont_info.attr_offset = 0;
  }
  bool is_mse_adv_enabled = is_map_adv_enabled();
  bool is_pse_adv_enabled = is_pbap_adv_enabled();
  /* Get a list of handles that match the UUIDs given to us */
  for (p_rec = sdp_db_service_search(p_ccb->cont_info.prev_sdp_rec, &uid_seq);
       p_rec; p_rec = sdp_db_service_search(p_rec, &uid_seq)) {
    p_ccb->cont_info.curr_sdp_rec = p_rec;
    /* Store the actual record pointer which would be reused later */
    p_prev_rec = p_rec;
    if (is_mse_adv_enabled) {
      p_rec = sdp_upgrade_mse_record(p_rec, p_ccb->device_address);
    }
    if (is_pse_adv_enabled) {
      p_rec = sdp_upgrade_pse_record(p_rec, p_ccb->device_address);
    }
    /* Allow space for attribute sequence type and length */
    p_seq_start = p_rsp;
    if (p_ccb->cont_info.last_attr_seq_desc_sent == false) {
      /* See if there is enough room to include a new service in the current
       * response */
      rem_len = max_list_len - (int16_t)(p_rsp - &p_ccb->rsp_list[0]);
      if (rem_len < 3) {
        /* Not enough room. Update continuation info for next response */
        p_ccb->cont_info.next_attr_index = 0;
        p_ccb->cont_info.next_attr_start_id = attr_seq.attr_entry[0].start;
        break;
      }
      p_rsp += 3;
    }
    /* Get a list of handles that match the UUIDs given to us */
    for (xx = p_ccb->cont_info.next_attr_index; xx < attr_seq.num_attr; xx++) {
      p_attr = sdp_db_find_attr_in_rec(p_rec, attr_seq.attr_entry[xx].start,
                                       attr_seq.attr_entry[xx].end);

      if (p_attr) {
        /*
         * If DUT profile version is 1.6, we are going to show 1.6.
         * Entry in file would be remote's actual version, but no action would be taken
         */
         /*
          *  There is no point in resetting CA bit, because if DUT 1.6, we have to show 1.6
          *  even if remote misbhevaes. If we DUT is not 1.6 then there would be no ca bit
          */

        if (sdp_reset_avrcp_browsing_bit(p_rec->attribute[1], p_attr, p_ccb->device_address))
          is_avrcp_browse_bit_set = FALSE;
        if (sdp_reset_avrcp_cover_art_bit(p_rec->attribute[1], p_attr, p_ccb->device_address))
          is_avrcp_cover_bit_set = FALSE;
        if ((p_attr->id == ATTR_ID_BT_PROFILE_DESC_LIST) &&
               (p_attr->len >= SDP_PROFILE_DESC_LENGTH)) {
          if (((p_attr->value_ptr[3] << 8) | (p_attr->value_ptr[4])) ==
                  UUID_SERVCLASS_AV_REMOTE_CONTROL) {
            property_get("persist.vendor.service.bt.a2dp.sink", a2dp_role, "false");
            if (!strncmp("false", a2dp_role, 5)) {
              profile_version = sdp_get_stored_avrc_tg_version(p_ccb->device_address);
              uint16_t ver = (AVRCP_VERSION_BIT_MASK & profile_version);
              if (ver >= AVRC_REV_1_4) {
                p_attr->value_ptr[PROFILE_VERSION_POSITION] = (uint8_t)(ver & 0x00ff);
                SDP_TRACE_DEBUG("%s : Showing AVRCP version in SDP = 0x%x", __func__,
                                 p_attr->value_ptr[PROFILE_VERSION_POSITION]);
              } else {
                p_attr->value_ptr[PROFILE_VERSION_POSITION] = 0x03; // Update AVRCP version as 1.3
                SDP_TRACE_DEBUG("%s : remote doesn't support cover art and browsing, showing AVRCP Version = 0x%x", __func__,
                                   p_attr->value_ptr[PROFILE_VERSION_POSITION]);
              }
              if (sdp_dev_blacklisted_for_avrcp15 (p_ccb->device_address)) {
                p_attr->value_ptr[PROFILE_VERSION_POSITION] = 0x03; // Update AVRCP version as 1.3
                SDP_TRACE_DEBUG("%s :Device is blacklisted updating AVRCP Version = 0x%x", __func__,
                                   p_attr->value_ptr[PROFILE_VERSION_POSITION]);
              }
            }
          }
        }
        is_hfp_fallback = sdp_change_hfp_version (p_attr, p_ccb->device_address);
        /* Check if attribute fits. Assume 3-byte value type/length */
        rem_len = max_list_len - (int16_t)(p_rsp - &p_ccb->rsp_list[0]);

        /* just in case */
        if (rem_len <= 0) {
          p_ccb->cont_info.next_attr_index = xx;
          p_ccb->cont_info.next_attr_start_id = p_attr->id;
          maxxed_out = true;
          break;
        }

        attr_len = sdpu_get_attrib_entry_len(p_attr);
        /* if there is a partial attribute pending to be sent */
        if (p_ccb->cont_info.attr_offset) {
          if (attr_len < p_ccb->cont_info.attr_offset) {
            android_errorWriteLog(0x534e4554, "79217770");
            LOG(ERROR) << "offset is bigger than attribute length";
            sdpu_build_n_send_error(p_ccb, trans_num, SDP_INVALID_CONT_STATE,
                                    SDP_TEXT_BAD_CONT_LEN);
            return;
          }
          p_rsp = sdpu_build_partial_attrib_entry(
              p_rsp, p_attr, rem_len, &p_ccb->cont_info.attr_offset);

          /* If the partial attrib could not been fully added yet */
          if (p_ccb->cont_info.attr_offset != attr_len) {
            maxxed_out = true;
            break;
          } else /* If the partial attrib has been added in full by now */
            p_ccb->cont_info.attr_offset = 0; /* reset attr_offset */
        } else if (rem_len <
                   attr_len) /* Not enough space for attr... so add partially */
        {
          if (attr_len >= SDP_MAX_ATTR_LEN) {
            SDP_TRACE_ERROR("SDP attr too big: max_list_len=%d,attr_len=%d",
                            max_list_len, attr_len);
            sdpu_build_n_send_error(p_ccb, trans_num, SDP_NO_RESOURCES, NULL);
            return;
          }

          /* add the partial attribute if possible */
          p_rsp = sdpu_build_partial_attrib_entry(
              p_rsp, p_attr, (uint16_t)rem_len, &p_ccb->cont_info.attr_offset);

          p_ccb->cont_info.next_attr_index = xx;
          p_ccb->cont_info.next_attr_start_id = p_attr->id;
          maxxed_out = true;
          break;
        } else /* build the whole attribute */
          p_rsp = sdpu_build_attrib_entry(p_rsp, p_attr);

        /* If doing a range, stick with this one till no more attributes found
         */
        if (attr_seq.attr_entry[xx].start != attr_seq.attr_entry[xx].end) {
          /* Update for next time through */
          attr_seq.attr_entry[xx].start = p_attr->id + 1;

          xx--;
        }
        if (is_hfp_fallback) {
            SDP_TRACE_ERROR("Restore HFP version to 1.6");
            /* Update HFP version back to 1.6 */
            p_attr->value_ptr[PROFILE_VERSION_POSITION] = 0x06;
            is_hfp_fallback = FALSE;
        }
      }
    }
    if (is_hfp_fallback) {
        SDP_TRACE_ERROR("Restore HFP version to 1.6");
        /* Update HFP version back to 1.6 */
        p_attr->value_ptr[PROFILE_VERSION_POSITION] = 0x06;
        is_hfp_fallback = FALSE;
    }

    /* Go back and put the type and length into the buffer */
    if (p_ccb->cont_info.last_attr_seq_desc_sent == false) {
      seq_len = sdpu_get_attrib_seq_len(p_rec, &attr_seq_sav);
      if (seq_len != 0) {
      if (p_seq_start) {
        UINT8_TO_BE_STREAM(p_seq_start,
                           (DATA_ELE_SEQ_DESC_TYPE << 3) | SIZE_IN_NEXT_WORD);
        UINT16_TO_BE_STREAM(p_seq_start, seq_len);
      } else {
        SDP_TRACE_DEBUG("SDP service and attribute rsp: Attribute sequence p_seq_start is NULL");
      }

        if (maxxed_out) p_ccb->cont_info.last_attr_seq_desc_sent = true;
      } else
        p_rsp = p_seq_start;
    }

    if (maxxed_out) break;

    /* Restore the attr_seq to look for in the next sdp record */
    memcpy(&attr_seq, &attr_seq_sav, sizeof(tSDP_ATTR_SEQ));

    /* Reset the next attr index */
    p_ccb->cont_info.next_attr_index = 0;
    /* restore the record pointer.*/
    p_rec = p_prev_rec;
    p_ccb->cont_info.prev_sdp_rec = p_rec;
    p_ccb->cont_info.last_attr_seq_desc_sent = false;
  }

  /* response length */
  len_to_send = (uint16_t)(p_rsp - &p_ccb->rsp_list[0]);
  cont_offset = 0;

  // The current SDP server design has a critical flaw where it can run into
  // an infinite request/response loop with the client. Here's the scenario:
  // - client makes SDP request
  // - server returns the first fragment of the response with a continuation
  //   token
  // - an SDP record is deleted from the server
  // - client issues another request with previous continuation token
  // - server has nothing to send back because the record is unavailable but
  //   in the first fragment, it had specified more response bytes than are
  //   now available
  // - server sends back no additional response bytes and returns the same
  //   continuation token
  // - client issues another request with the continuation token, and the
  //   process repeats
  //
  // We work around this design flaw here by checking if we will make forward
  // progress (i.e. we will send > 0 response bytes) on a continued request.
  // If not, we must have run into the above situation and we tell the peer an
  // error occurred.
  //
  // TODO(sharvil): rewrite SDP server.
  if (is_cont && len_to_send == 0) {
    sdpu_build_n_send_error(p_ccb, trans_num, SDP_INVALID_CONT_STATE, NULL);
    return;
  }

  /* If first response, insert sequence header */
  if (!is_cont) {
    /* Get the total list length for requested uid and attribute sequence */
    p_ccb->list_len = sdpu_get_list_len(&uid_seq, &attr_seq_sav) + 3;
    /* Get the length of blacklisted attributes to be updated if device is blacklisted */
    p_ccb->bl_update_len = sdp_update_pbap_blacklist_len(p_ccb, &attr_seq_sav, &uid_seq);
    SDP_TRACE_DEBUG("%s p_ccb->list_len = %d bl_update_len = %d",__func__,
        p_ccb->list_len, p_ccb->bl_update_len);

    /* Put in the sequence header (2 or 3 bytes) */
    if (p_ccb->list_len > 255) {
      p_ccb->rsp_list[0] =
          (uint8_t)((DATA_ELE_SEQ_DESC_TYPE << 3) | SIZE_IN_NEXT_WORD);
      p_ccb->rsp_list[1] = (uint8_t)((p_ccb->list_len - 3 + p_ccb->bl_update_len) >> 8);
      p_ccb->rsp_list[2] = (uint8_t)(p_ccb->list_len - 3 + p_ccb->bl_update_len);
    } else {
      cont_offset = 1;

      p_ccb->rsp_list[1] =
          (uint8_t)((DATA_ELE_SEQ_DESC_TYPE << 3) | SIZE_IN_NEXT_BYTE);
      p_ccb->rsp_list[2] = (uint8_t)(p_ccb->list_len - 3 + p_ccb->bl_update_len);

      p_ccb->list_len--;
      len_to_send--;
    }
  }

  /* Get a buffer to use to build the response */
  BT_HDR* p_buf = (BT_HDR*)osi_malloc(SDP_DATA_BUF_SIZE);
  p_buf->offset = L2CAP_MIN_OFFSET;
  p_rsp = p_rsp_start = (uint8_t*)(p_buf + 1) + L2CAP_MIN_OFFSET;

  /* Start building a rsponse */
  UINT8_TO_BE_STREAM(p_rsp, SDP_PDU_SERVICE_SEARCH_ATTR_RSP);
  UINT16_TO_BE_STREAM(p_rsp, trans_num);

  /* Skip the parameter length, add it when we know the length */
  p_rsp_param_len = p_rsp;
  p_rsp += 2;

  /* Stream the list length to send */
  UINT16_TO_BE_STREAM(p_rsp, len_to_send);

  /* copy from rsp_list to the actual buffer to be sent */
  memcpy(p_rsp, &p_ccb->rsp_list[cont_offset], len_to_send);
  p_rsp += len_to_send;

  p_ccb->cont_offset += len_to_send;

  SDP_TRACE_DEBUG("%s: p_ccb->bl_update_len %d, cont_offset = %d, p_ccb->list_len = %d",
      __func__, p_ccb->bl_update_len, p_ccb->cont_offset, p_ccb->list_len + p_ccb->bl_update_len);

  /* If anything left to send, continuation needed */
  if (p_ccb->cont_offset < (p_ccb->list_len + p_ccb->bl_update_len)) {
    is_cont = true;
    UINT8_TO_BE_STREAM(p_rsp, SDP_CONTINUATION_LEN);
    UINT16_TO_BE_STREAM(p_rsp, p_ccb->cont_offset);
  } else {
    UINT8_TO_BE_STREAM(p_rsp, 0);
    if (p_ccb->bl_update_len) {
      p_ccb->bl_update_len = 0;
    }
  }

  /* Go back and put the parameter length into the buffer */
  rsp_param_len = p_rsp - p_rsp_param_len - 2;
  UINT16_TO_BE_STREAM(p_rsp_param_len, rsp_param_len);

  /* Set the length of the SDP data in the buffer */
  p_buf->len = p_rsp - p_rsp_start;

  /* Send the buffer through L2CAP */
  L2CA_DataWrite(p_ccb->connection_id, p_buf);
}

/*************************************************************************************
**
** Function        is_device_blacklisted_for_pbap
**
** Description     Checks if given PBAP record is for PBAP PSE and blacklisted
**
** Returns         BOOLEAN
**
***************************************************************************************/
static bool is_device_blacklisted_for_pbap (RawAddress remote_address, bool check_for_1_2)
{
  if (check_for_1_2 && interop_match_addr_or_name(INTEROP_ADV_PBAP_VER_1_1, &remote_address)) {
    SDP_TRACE_DEBUG("%s: device is blacklisted for pbap version < 1.2 ", __func__);
    return true;
  }
  if (!check_for_1_2 && interop_match_addr_or_name(INTEROP_ADV_PBAP_VER_1_2, &remote_address)) {
    SDP_TRACE_DEBUG("%s: device is blacklisted for pbap version 1.2 ", __func__);
    return true;
  }
  return false;
}

/*************************************************************************************
**
** Function        check_remote_pbap_version_102
**
** Description     checks if remote supports PBAP 1.2
**
** Returns         true/false depending on remote PBAP version support found in file.
**                 Returns false if 1.2 entry is stored but device is not re-paired with
**                 remote.
**                 Returns true if 1.2 entry is stored and device is re-paired with
**                 remote.
**
***************************************************************************************/
static bool check_remote_pbap_version_102(RawAddress remote_addr) {
  bool entry_found = FALSE;
  struct dynamic_upgrade_entry entry;
  FILE *fp = fopen(PCE_PEER_VERSION_CONF_FILE, "r+b");
  if (!fp) {
    APPL_TRACE_ERROR("%s unable to open PBAP PCE Conf file for read: error: (%s)",\
                                                      __func__, strerror(errno));
  } else {
    while (fread(&entry, sizeof(dynamic_upgrade_entry), 1, fp) != 0)
    {
      APPL_TRACE_DEBUG("Entry: addr = %x:%x:%x, ver = 0x%x rebonded: %c",\
              entry.addr[0], entry.addr[1], entry.addr[2], entry.ver, entry.rebonded);
      if(!memcmp(&remote_addr, entry.addr, 3))
      {
          entry_found = (entry.rebonded == 'Y' && entry.ver >= PBAP_1_2 )? TRUE : FALSE;
          APPL_TRACE_DEBUG("remote bd address matched, isRebonded=%c entry_found = %d",
                  entry.rebonded, entry_found);
          break;
      }
    }
    fclose(fp);
  }
  return entry_found;
}

/*************************************************************************************
**
** Function        sdp_update_pbap_blacklist_len
**
** Description     Updates the blacklist length to be updated from the SDP response
**
** Returns         void
**
***************************************************************************************/
static uint16_t sdp_update_pbap_blacklist_len(tCONN_CB* p_ccb, tSDP_ATTR_SEQ* attr_seq,
                                            tSDP_UUID_SEQ* uid_seq) {
  if (!p_ccb || !attr_seq || !uid_seq) return 0;
  tSDP_RECORD* p_rec;

  p_ccb->bl_update_len = 0;

  // Check to validate if 1.2 record is getting sent
  bool is_pbap_102_supported = check_remote_pbap_version_102(p_ccb->device_address);
  bool is_pbap_101_blacklisted = is_device_blacklisted_for_pbap(p_ccb->device_address, false);
  bool is_pbap_102_blacklisted = is_device_blacklisted_for_pbap(p_ccb->device_address, true);
  bool running_pts = false;
  char pts_property[6];
  osi_property_get(SDP_ENABLE_PTS_PBAP, pts_property, "false");
  if (!strncmp("true", pts_property, 4)) {
    SDP_TRACE_DEBUG("%s pts running= %d", __func__, pts_property);
    running_pts = true;
  }
  SDP_TRACE_DEBUG("%s remote BD Addr : %s is_pbap_102_supported : %d "
      "is_pbap_1_1__blacklisted = %d is_pbap_1_2__blacklisted = %d "
      "running_pts = %d", __func__,
      p_ccb->device_address.ToString().c_str(), is_pbap_102_supported,
      is_pbap_101_blacklisted, is_pbap_102_blacklisted, running_pts);

  if (is_pbap_102_blacklisted
      || (!is_pbap_102_supported && !is_pbap_101_blacklisted && !running_pts)) {
    // Send Length without any update
    return p_ccb->bl_update_len;
  }

  int xx;
  tSDP_ATTRIBUTE attr;
  if (uid_seq) {
    for (p_rec = sdp_db_service_search(NULL, uid_seq); p_rec;
      p_rec = sdp_db_service_search(p_rec, uid_seq)) {
      attr = p_rec->attribute[1];
      if ((attr.id == ATTR_ID_SERVICE_CLASS_ID_LIST) &&
          (((attr.value_ptr[1] << 8) | (attr.value_ptr[2])) == UUID_SERVCLASS_PBAP_PSE)) {
        // PBAP PSE Record
        p_rec = sdp_upgrade_pse_record(p_rec, p_ccb->device_address);
        SDP_TRACE_DEBUG("%s: response has PBAP PSE record for BL device", __func__);

        int att_index;
        bool l2cap_psm_len_included = false, supp_attr_len_included = false;
        for (xx = p_ccb->cont_info.next_attr_index; xx < attr_seq->num_attr; xx++) {
          SDP_TRACE_DEBUG("%s: xx = %d attr_seq->num_attr = %d, "
              "attr_seq->attr_entry[xx].start = %d , attr_seq->attr_entry[xx].end = %d",
              __func__, xx, attr_seq->num_attr, attr_seq->attr_entry[xx].start,
              attr_seq->attr_entry[xx].end);

          for (att_index = 0; att_index < p_rec->num_attributes; att_index++) {
            tSDP_ATTRIBUTE cur_attr = p_rec->attribute[att_index];
            if (cur_attr.id == ATTR_ID_GOEP_L2CAP_PSM
                && !l2cap_psm_len_included
                &&  cur_attr.id >= attr_seq->attr_entry[xx].start
                &&  cur_attr.id <= attr_seq->attr_entry[xx].end) {
              l2cap_psm_len_included = true;
              p_ccb->bl_update_len += PBAP_GOEP_L2CAP_PSM_LEN;
              SDP_TRACE_ERROR("%s: ATTR_ID_GOEP_L2CAP_PSM requested,"
                  " need to change length by %d", __func__,
                  p_ccb->bl_update_len);
            } else if (cur_attr.id == ATTR_ID_PBAP_SUPPORTED_FEATURES
                &&  !supp_attr_len_included
                &&  cur_attr.id >= attr_seq->attr_entry[xx].start
                &&  cur_attr.id <= attr_seq->attr_entry[xx].end) {
              supp_attr_len_included = true;
              p_ccb->bl_update_len += PBAP_SUPP_FEA_LEN;
              SDP_TRACE_DEBUG("%s: ATTR_ID_PBAP_SUPPORTED_FEATURES requested,"
                  " need to change length by %d", __func__,
                  p_ccb->bl_update_len);
            }
          }
          if (p_ccb->bl_update_len == PBAP_1_2_BL_LEN) break;
        }
        break;
      }
    }
  }
  SDP_TRACE_DEBUG("%s: bl_update_len = %d", __func__, p_ccb->bl_update_len);
  return p_ccb->bl_update_len;
}


/*************************************************************************************
**
** Function        sdp_upgrade_pbap_pse_record
**
** Description     updates pbap record to pbap 1.2 record if remote supports pbap 1.2
**
** Returns         the address of updated record
**
***************************************************************************************/
static tSDP_RECORD *sdp_upgrade_pse_record(tSDP_RECORD * p_rec,
        RawAddress remote_address) {
  static bool is_pbap_102_supported = FALSE;
  tSDP_ATTRIBUTE attr = p_rec->attribute[1];
  if (!((attr.id == ATTR_ID_SERVICE_CLASS_ID_LIST) &&
      (((attr.value_ptr[1] << 8) | (attr.value_ptr[2])) == UUID_SERVCLASS_PBAP_PSE))) {
    // Not a PBAP PSE Record
    return p_rec;
  }

  /* Check if remote supports PBAP 1.2 */
  is_pbap_102_supported = check_remote_pbap_version_102(remote_address);
  bool is_pbap_101_blacklisted = is_device_blacklisted_for_pbap(remote_address, false);
  bool is_pbap_102_blacklisted = is_device_blacklisted_for_pbap(remote_address, true);
  bool running_pts = false;
  char pts_property[6];
  osi_property_get(SDP_ENABLE_PTS_PBAP, pts_property, "false");
  if (!strncmp("true", pts_property, 4)) {
    SDP_TRACE_DEBUG("%s pts running= %d", __func__, pts_property);
    running_pts = true;
  }
  SDP_TRACE_DEBUG("%s remote BD Addr : %s is_pbap_102_supported : %d "
      "is_pbap_1_1__blacklisted = %d is_pbap_1_2__blacklisted = %d "
      "running_pts = %d", __func__,
      remote_address.ToString().c_str(), is_pbap_102_supported,
      is_pbap_101_blacklisted, is_pbap_102_blacklisted, running_pts);

  if (is_pbap_102_blacklisted
      || (!is_pbap_102_supported && !is_pbap_101_blacklisted && !running_pts)) {
    // Send 1.1 SDP Record
    return p_rec;
  }

  static tSDP_RECORD pbap_102_sdp_rec;
  memset(&pbap_102_sdp_rec, 0, sizeof(tSDP_RECORD));

  uint32_t supported_features = 0x021F; // PBAP 1.2 Features
  uint16_t pbap_0102 = PBAP_1_2; // Profile version
  uint32_t pbap_l2cap_psm = 0x1025; // Fixed L2CAP PSM
  tSDP_ATTRIBUTE  *p_attr = &p_rec->attribute[0];
  uint8_t temp[4], j;
  uint8_t* p_temp = temp;
  bool status = true;

  /* Copying contents of the PBAP 1.1 PSE record to a new 1.2 record */
  for (j = 0; j < p_rec->num_attributes; j++, p_attr++) {
    SDP_AddAttributeToRecord (&pbap_102_sdp_rec, p_attr->id,
      p_attr->type, p_attr->len, p_attr->value_ptr);
  }

  /* Add in the Bluetooth Profile Descriptor List */
  status &= SDP_AddProfileDescriptorListToRecord(
          &pbap_102_sdp_rec, UUID_SERVCLASS_PHONE_ACCESS, pbap_0102);

  /* Add PBAP 1.2 supported features 4 */
  UINT32_TO_BE_STREAM(p_temp, supported_features);
  status &= SDP_AddAttributeToRecord(&pbap_102_sdp_rec, ATTR_ID_PBAP_SUPPORTED_FEATURES,
                              UINT_DESC_TYPE, (uint32_t)4, temp);

  /* Add the L2CAP PSM */
  p_temp = temp;  // The macro modifies p_temp, hence rewind.
  UINT16_TO_BE_STREAM(p_temp, pbap_l2cap_psm);
  status &= SDP_AddAttributeToRecord(&pbap_102_sdp_rec, ATTR_ID_GOEP_L2CAP_PSM,
                           UINT_DESC_TYPE, (uint32_t)2, temp);

  if (!status) {
    SDP_TRACE_ERROR("%s: FAILED", __func__);
    return p_rec;
  }
  return &pbap_102_sdp_rec;
}

/*************************************************************************************
**
** Function        update_pce_entry_after_cancelling_bonding
**
** Description     Update PCE 1.2 entry by setting rebonded to true
**
***************************************************************************************/
void update_pce_entry_after_cancelling_bonding(RawAddress remote_addr) {
  SDP_TRACE_DEBUG("%s", __func__);
  struct dynamic_upgrade_entry entry;
  FILE *fp = fopen(PCE_PEER_VERSION_CONF_FILE, "r+b");
  if (!fp) {
    APPL_TRACE_ERROR("%s unable to open PBAP PCE Conf file for read: error: (%s)",\
                                                      __func__, strerror(errno));
  } else {
    int size_read = 0;
    int entry_size = sizeof(dynamic_upgrade_entry);
    while (fread(&entry, sizeof(entry), 1, fp) != 0)
    {
      size_read += sizeof(entry);
      APPL_TRACE_DEBUG("Entry: addr = %x:%x:%x, ver = 0x%x",\
              entry.addr[0], entry.addr[1], entry.addr[2], entry.ver);
      if(!memcmp(&remote_addr, entry.addr, 3))
      {
        APPL_TRACE_DEBUG("remote bd address matched, rebonded = %c,"
            " entry_size = %d, size_read = %d", entry.rebonded, entry_size, size_read);
        if (entry.rebonded == 'N' && entry_size <= size_read) {
            fseek(fp, -(entry_size), SEEK_CUR);
            entry.rebonded = 'Y';
            fwrite(&entry, sizeof(entry), 1, fp);
        }
        break;
      }
    }
    fclose(fp);
  }
}

/*********************************************************************
 ** Function : check_and_store_pce_profile_version
 **
 **  Description :
 **    This function checks remote PBAP profile version. If remote supports
 **    PBAP 1.2, entry will be added to database for remote this remote.
 **    address.
 **    Entry Format: [version, BD_ADDRESS, rebonded]
 **    Version: Remote PBAP Profile Version
 **    BD_ADDRESS: Bluetooth Address of the remote.
 **    rebonded: either 'N'/'Y'.
 **              N - When entry is created.
 **              Y - When device is rebonded
 **
 ********************************************************************/
void check_and_store_pce_profile_version(tSDP_DISC_REC* p_sdp_rec) {
  FILE *fp;
  bool has_entry = FALSE;
  struct dynamic_upgrade_entry entry;
  uint16_t peer_pce_version = 0;

  RawAddress remote_addr = p_sdp_rec->remote_bd_addr;
  SDP_FindProfileVersionInRec(p_sdp_rec, UUID_SERVCLASS_PHONE_ACCESS, &peer_pce_version);
  if (peer_pce_version != 0) {
    APPL_TRACE_DEBUG("%s: peer_pce_version : 0x%x", __func__, peer_pce_version);
    if (btif_config_set_uint16(remote_addr.ToString().c_str(),
                      PBAP_PCE_VERSION_CONFIG_KEY,
                      peer_pce_version)) {
      btif_config_save();
    } else {
      APPL_TRACE_WARNING("%s: Failed to store  peer_pce_version for %s",
                   __func__, remote_addr.ToString().c_str());
    }
  }

  bool is_pbap_102_blacklisted = is_device_blacklisted_for_pbap(remote_addr, true);
  APPL_TRACE_DEBUG("%s remote BD Addr: %s peer pce version: %x is_pbap_102_blacklisted = %d",
      __func__, remote_addr.ToString().c_str(), peer_pce_version, is_pbap_102_blacklisted);

  if (is_pbap_102_blacklisted) {
    return;
  }

  fp = fopen(PCE_PEER_VERSION_CONF_FILE, "r+b");
  if (!fp)
  {
    APPL_TRACE_ERROR("%s unable to open PBAP PCE Conf file for read: error: (%s)",\
                                                      __func__, strerror(errno));
  }
  else
  {
    while (fread(&entry, sizeof(entry), 1, fp) != 0)
    {
      APPL_TRACE_DEBUG("%s: Entry: addr = %x:%x:%x, ver = 0x%x",\
              __func__, entry.addr[0], entry.addr[1], entry.addr[2], entry.ver);
      if(!memcmp(&remote_addr, entry.addr, 3))
      {
        has_entry = TRUE;
        // Remote PBAP Version Downgraded from 1.2 to some older version
        if ((peer_pce_version < PBAP_1_2 && entry.ver >= PBAP_1_2) ||
            (peer_pce_version >= PBAP_1_2 && entry.ver < PBAP_1_2)) {
          APPL_TRACE_DEBUG("%s: Remote PBAP version is downgraded/Upgraded", __func__);
          // update file pce entry with older version and rebonded = 'N'
          fseek(fp, -(sizeof(dynamic_upgrade_entry)), SEEK_CUR);
          entry.ver = peer_pce_version;
          entry.rebonded = 'N';
          fwrite(&entry, sizeof(dynamic_upgrade_entry), 1, fp);
        }
        APPL_TRACE_DEBUG("Entry already present, break");
        break;
      }
    }
    fclose(fp);
  }
  // Store PCE PBAP version
  if (has_entry == FALSE && peer_pce_version >= PBAP_1_2)
  {
    fp = fopen(PCE_PEER_VERSION_CONF_FILE, "ab");
    if (!fp)
    {
      APPL_TRACE_ERROR("%s unable to open PCE Conf file for write: error: (%s)",\
                                                        __func__, strerror(errno));
    }
    else
    {
      entry.ver = peer_pce_version;
      entry.rebonded = 'N';
      memcpy(entry.addr, &remote_addr, 3);
      APPL_TRACE_DEBUG("PCE PBAP version to store = 0x%x rebonded = %c",
              peer_pce_version, entry.rebonded);
      fwrite(&entry, sizeof(entry), 1, fp);
      fclose(fp);
    }
  }
}

/*************************************************************************************
**
** Function        check_remote_map_version_104
**
** Description     checks if remote supports MAP 1.4
**
** Returns         true/false depending on remote MAP version support found in file.
**                 Returns false if 1.4 entry is stored but device is not re-paired with
**                 remote.
**                 Returns true if 1.4 entry is stored and device is re-paired with
**                 remote.
**
***************************************************************************************/
static bool check_remote_map_version_104(RawAddress remote_addr) {
  bool entry_found = FALSE;
  struct dynamic_upgrade_entry entry;
  FILE *fp = fopen(MCE_PEER_VERSION_CONF_FILE, "r+b");
  if (!fp) {
    APPL_TRACE_ERROR("%s unable to open MCE Conf file for read: reason: (%s)",\
                                                      __func__, strerror(errno));
  } else {
    while (fread(&entry, sizeof(dynamic_upgrade_entry), 1, fp) != 0)
    {
      APPL_TRACE_DEBUG("Entry: addr = %x:%x:%x, ver = 0x%x rebonded: %c",\
              entry.addr[0], entry.addr[1], entry.addr[2], entry.ver, entry.rebonded);
      if(!memcmp(&remote_addr, entry.addr, 3))
      {
          entry_found = (entry.rebonded == 'Y' && entry.ver >= MAP_1_4)? TRUE : FALSE;
          APPL_TRACE_DEBUG("remote bd address matched, isRebonded=%c entry_found = %d",
                  entry.rebonded, entry_found);
          break;
      }
    }
    fclose(fp);
  }
  return entry_found;
}

/*********************************************************************
 ** Function : check_and_store_mce_profile_version
 **
 **  Description :
 **    This function checks remote MAP profile version. If remote supports
 **    MAP 1.4, entry will be added to database for this remote.
 **    address.
 **    Entry Format: [version, BD_ADDRESS, rebonded]
 **    Version: Remote MAP Profile Version
 **    BD_ADDRESS: Bluetooth Address of the remote.
 **    rebonded: either 'N'/'Y'.
 **              N - When entry is created.
 **              Y - When device is rebonded
 **
 ********************************************************************/
void check_and_store_mce_profile_version(tSDP_DISC_REC* m_sdp_rec) {
  APPL_TRACE_WARNING("%s", __func__);
  FILE *fp;
  bool has_entry = FALSE;
  struct dynamic_upgrade_entry entry;
  uint16_t peer_mce_version = 0;
  RawAddress remote_addr = m_sdp_rec->remote_bd_addr;
  SDP_FindProfileVersionInRec(m_sdp_rec, UUID_SERVCLASS_MAP_PROFILE, &peer_mce_version);
  if (peer_mce_version != 0) {
    APPL_TRACE_WARNING("%s: peer_mce_version : 0x%x", __func__,peer_mce_version);
    if (btif_config_set_uint16(remote_addr.ToString().c_str(),
                      MAP_MCE_VERSION_CONFIG_KEY,
                      peer_mce_version)) {
      btif_config_save();
    } else {
      APPL_TRACE_WARNING("%s: Failed to store  peer_mce_version for %s",
                   __func__, remote_addr.ToString().c_str());
    }
  } else {
    APPL_TRACE_WARNING("%s: peer_mce_version invalid : 0x%x", __func__, peer_mce_version);
  }
  fp = fopen(MCE_PEER_VERSION_CONF_FILE, "r+b");
  if (!fp)
  {
    APPL_TRACE_ERROR("%s No existing MAP MCE Conf file for read reason: (%s)",\
                                                      __func__, strerror(errno));
  }
  else
  {
    while (fread(&entry, sizeof(entry), 1, fp) != 0)
    {
      APPL_TRACE_DEBUG("%s: Entry: addr = %x:%x:%x, ver = 0x%x",\
              __func__, entry.addr[0], entry.addr[1], entry.addr[2], entry.ver);
      if(!memcmp(&remote_addr, entry.addr, 3))
      {
        has_entry = TRUE;
        // Remote MAP Version Downgraded from 1.4 to some older version
        if ((peer_mce_version < MAP_1_4 && entry.ver >= MAP_1_4) ||
            (peer_mce_version >= MAP_1_4 && entry.ver < MAP_1_4)) {
          APPL_TRACE_DEBUG("%s: Remote MAP version is downgraded/Upgraded", __func__);
          // update file mce entry with older version and rebonded = 'N'
          fseek(fp, -(sizeof(dynamic_upgrade_entry)), SEEK_CUR);
          entry.ver = peer_mce_version;
          entry.rebonded = 'N';
          fwrite(&entry, sizeof(dynamic_upgrade_entry), 1, fp);
        }
        APPL_TRACE_DEBUG("Entry already present, break");
        break;
      }
    }
    fclose(fp);
  }
  // Store MCE MAP version
  if (has_entry == FALSE && peer_mce_version >= MAP_1_4)
  {
    fp = fopen(MCE_PEER_VERSION_CONF_FILE, "ab");
    if (!fp)
    {
      APPL_TRACE_ERROR("%s Unable to open/create MCE Conf file for write: reason: (%s)",\
                                                        __func__, strerror(errno));
    }
    else
    {
      entry.ver = peer_mce_version;
      entry.rebonded = 'N';
      memcpy(entry.addr, &remote_addr, 3);
      APPL_TRACE_DEBUG("MCE MAP version to store = 0x%x rebonded = %c",
              peer_mce_version, entry.rebonded);
      fwrite(&entry, sizeof(entry), 1, fp);
      fclose(fp);
    }
  } else {
      APPL_TRACE_DEBUG(" %s MCE MAP version not stored =>> 0x%x has_entry = %d",
                __func__,peer_mce_version, has_entry);
  }
}

/*************************************************************************************
**
** Function        update_mce_entry_after_cancelling_bonding
**
** Description     Update MCE 1.4 entry by setting rebonded to true
**
***************************************************************************************/
void update_mce_entry_after_cancelling_bonding(RawAddress remote_addr) {
  SDP_TRACE_DEBUG("%s", __func__);
  struct dynamic_upgrade_entry entry;
  FILE *fp = fopen(MCE_PEER_VERSION_CONF_FILE, "r+b");
  if (!fp) {
    APPL_TRACE_ERROR("%s unable to open MAP MCE Conf file for read: Reason: (%s)",\
                                                      __func__, strerror(errno));
  } else {
    int size_read = 0;
    int entry_size = sizeof(dynamic_upgrade_entry);
    while (fread(&entry, sizeof(entry), 1, fp) != 0)
    {
      size_read += sizeof(entry);
      APPL_TRACE_DEBUG("Entry: addr = %x:%x:%x, ver = 0x%x",\
              entry.addr[0], entry.addr[1], entry.addr[2], entry.ver);
      if(!memcmp(&remote_addr, entry.addr, 3))
      {
        APPL_TRACE_DEBUG("remote bd address matched, rebonded = %c,"
            " entry_size = %d, size_read = %d", entry.rebonded, entry_size, size_read);
        if (entry.rebonded == 'N' && entry_size <= size_read) {
            fseek(fp, -(entry_size), SEEK_CUR);
            entry.rebonded = 'Y';
            fwrite(&entry, sizeof(entry), 1, fp);
        }
        break;
      }
    }
    fclose(fp);
  }
}

/*************************************************************************************
**
** Function        sdp_upgrade_map_mse_record
**
** Description     updates map record to map 1.4 record if remote supports map 1.4
**
** Returns         the address of updated record
**
***************************************************************************************/
static tSDP_RECORD *sdp_upgrade_mse_record(tSDP_RECORD * p_rec,
        RawAddress remote_address) {
  static bool is_map_104_supported = FALSE;
  APPL_TRACE_ERROR("%s ",__func__);
  tSDP_ATTRIBUTE attr = p_rec->attribute[1];
  if (!((attr.id == ATTR_ID_SERVICE_CLASS_ID_LIST) &&
      (((attr.value_ptr[1] << 8) | (attr.value_ptr[2])) == UUID_SERVCLASS_MESSAGE_ACCESS))) {
    // Not a MAP MSE Record
    return p_rec;
  }
  /* Check if remote supports MAP 1.4 */
  is_map_104_supported = check_remote_map_version_104(remote_address);
  bool running_pts = false;
  char pts_property[6];
  osi_property_get(SDP_ENABLE_PTS_MAP, pts_property, "false");
  if (!strncmp("true", pts_property, 4)) {
    SDP_TRACE_DEBUG("%s pts running= %s", __func__, pts_property);
    running_pts = true;
  }
  APPL_TRACE_ERROR("%s remote BD Addr : %s is_map_104_supported : %d running_pts = %d",
      __func__,remote_address.ToString().c_str(),is_map_104_supported,running_pts);

  if (!is_map_104_supported && !running_pts) {
    // Send 1.2 SDP Record
    APPL_TRACE_ERROR("%s Send MAP 1.2, remote not supporting map 1.4  ",__func__);
    return p_rec;
  }
  tSDP_ATTRIBUTE  *p_attr = &p_rec->attribute[0];
  static tSDP_RECORD map_104_sdp_rec;
  bool status = true;
  uint8_t temp[4];
  uint8_t* p_temp = temp;
  memset(&map_104_sdp_rec, 0, sizeof(tSDP_RECORD));
  uint16_t map_0104 = MAP_1_4; // Profile version
  uint32_t supported_features = 0x603ff; // MAP 1.4 Features
  for (int j = 0; j < p_rec->num_attributes; j++, p_attr++) {
    SDP_AddAttributeToRecord (&map_104_sdp_rec, p_attr->id,
    p_attr->type, p_attr->len, p_attr->value_ptr);
  }
  /* Add in the Bluetooth Profile Descriptor List */
  status &= SDP_AddProfileDescriptorListToRecord(
    &map_104_sdp_rec, UUID_SERVCLASS_MAP_PROFILE, map_0104);
  /* Add MAP 1.4 supported features  */
  UINT32_TO_BE_STREAM(p_temp, supported_features);
  status &= SDP_AddAttributeToRecord(&map_104_sdp_rec,  ATTR_ID_MAP_SUPPORTED_FEATURES,
    UINT_DESC_TYPE, (uint32_t)4, temp);
  SDP_TRACE_DEBUG("%s update supported_features %x, version : %x"
    ,__func__, supported_features,map_0104);
  if (!status) {
    SDP_TRACE_ERROR("%s: FAILED", __func__);
    return p_rec;
  }
  SDP_TRACE_ERROR("%s: Success changed", __func__);
  return &map_104_sdp_rec;
}

static bool is_map_adv_enabled(){
  bool feature = profile_feature_fetch(MAP_ID, MAP_0104_SUPPORT);
  APPL_TRACE_ERROR("%s feature : %d",__func__,feature);
  return feature;
}

static bool is_pbap_adv_enabled(){
  bool feature = profile_feature_fetch(PBAP_ID, PBAP_0102_SUPPORT);
  APPL_TRACE_ERROR("%s feature : %d",__func__,feature);
  return feature;
}

#endif /* SDP_SERVER_ENABLED == TRUE */
