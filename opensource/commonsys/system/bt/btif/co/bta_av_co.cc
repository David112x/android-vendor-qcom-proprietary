/******************************************************************************
 * Copyright (C) 2017, The Linux Foundation. All rights reserved.
 * Not a Contribution.
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted (subject to the limitations in the
 *  disclaimer below) provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.

    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

 *  NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
 *  GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
 *  HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 *  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 *  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 *  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 *  GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 *  IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 *  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 *  IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/
/******************************************************************************
 *
 *  Copyright (C) 2004-2012 Broadcom Corporation
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
 *  This is the advanced audio/video call-out function implementation for
 *  BTIF.
 *
 ******************************************************************************/

#include "bta_av_co.h"
#include <base/logging.h>
#include <string.h>
#include "a2dp_api.h"
#include "a2dp_sbc.h"
#include "bt_target.h"
#include "bta_av_api.h"
#include "bta_av_ci.h"
#include "bta_sys.h"

#include "btif_av.h"
#include "btif_av_co.h"
#include "btif_util.h"
#include "osi/include/mutex.h"
#include "osi/include/osi.h"
#include "osi/include/properties.h"
#include "device/include/interop.h"
#include "device/include/controller.h"
#include "btif/include/btif_storage.h"
#include <hardware/bt_gatt.h>
#include "btif/include/btif_a2dp_source.h"
#include "device/include/device_iot_config.h"
#include "btif/include/btif_config.h"
#include "stack/include/a2dp_vendor_ldac_constants.h"
#define MAX_2MBPS_AVDTP_MTU 663
#define A2DP_LDAC_CODEC_BEST_SAMPLE_RATE 96000
extern const btgatt_interface_t* btif_gatt_get_interface();

bool isDevUiReq = false;
btav_a2dp_codec_config_t saved_codec_user_config;
std::string supported_codecs = "";
/*****************************************************************************
 **  Constants
 *****************************************************************************/

/* Macro to retrieve the number of elements in a statically allocated array */
#define BTA_AV_CO_NUM_ELEMENTS(__a) (sizeof(__a) / sizeof((__a)[0]))

/* Macro to convert audio handle to index and vice versa */
#define BTA_AV_CO_AUDIO_HNDL_TO_INDX(hndl) (((hndl) & (~BTA_AV_CHNL_MSK)) - 1)
#define BTA_AV_CO_AUDIO_INDX_TO_HNDL(indx) (((indx) + 1) | BTA_AV_CHNL_AUDIO)

static void bta_av_co_free_peer(tBTA_AV_CO_PEER* p_peer);
/* SCMS-T protect info */
const uint8_t bta_av_co_cp_scmst[AVDT_CP_INFO_LEN] = {0x02, 0x02, 0x00};

/*****************************************************************************
 *  Local data
 ****************************************************************************/
class BtaAvCoCb {
 public:
  BtaAvCoCb() :  codec_config{} { reset(); }

  /* Connected peer information */
  tBTA_AV_CO_PEER peers[BTA_AV_NUM_STRS];
  /* Current codec configuration - access to this variable must be protected */
  uint8_t codec_config[AVDT_CODEC_SIZE];
  tBTA_AV_CO_CP cp;
  std::vector<btav_a2dp_codec_config_t> default_codec_priorities;

  void reset() {
    // TODO: Ugly leftover reset from the original C code. Should go away once
    // the rest of the code in this file migrates to C++.
    //memset(peers, 0, sizeof(peers));
    memset(codec_config, 0, sizeof(codec_config));
    memset(&cp, 0, sizeof(cp));

    // Initialize the handles
    for (size_t i = 0; i < BTA_AV_CO_NUM_ELEMENTS(peers); i++) {
      tBTA_AV_CO_PEER* p_peer = &peers[i];
      bta_av_co_free_peer(p_peer);
      p_peer->handle = BTA_AV_CO_AUDIO_INDX_TO_HNDL(i);
    }
  }
};

/* Control block instance */
static BtaAvCoCb bta_av_co_cb;

static bool bta_av_co_cp_is_scmst(const uint8_t* p_protect_info);
static bool bta_av_co_audio_protect_has_scmst(uint8_t num_protect,
                                              const uint8_t* p_protect_info);
static const tBTA_AV_CO_SINK* bta_av_co_find_peer_src_supports_codec(
    const tBTA_AV_CO_PEER* p_peer);
static tBTA_AV_CO_SINK* bta_av_co_audio_set_codec(tBTA_AV_CO_PEER* p_peer);
static tBTA_AV_CO_SINK* bta_av_co_audio_codec_selected(
    A2dpCodecConfig& codec_config, tBTA_AV_CO_PEER* p_peer);
static void bta_av_co_save_new_codec_config(tBTA_AV_CO_PEER* p_peer,
                                            const uint8_t* new_codec_config,
                                            uint8_t num_protect,
                                            const uint8_t* p_protect_info);
static bool bta_av_co_set_codec_ota_config(tBTA_AV_CO_PEER* p_peer,
                                           const uint8_t* p_ota_codec_config,
                                           uint8_t num_protect,
                                           const uint8_t* p_protect_info,
                                           bool* p_restart_output);
static bool bta_av_co_audio_update_selectable_codec(
    A2dpCodecConfig& codec_config, const tBTA_AV_CO_PEER* p_peer);

/* externs */
extern int btif_max_av_clients;
extern tBTA_AV_HNDL btif_av_get_reconfig_dev_hndl();
extern void btif_av_reset_codec_reconfig_flag(RawAddress address);
extern bool bt_split_a2dp_enabled;
extern void btif_av_set_reconfig_flag(tBTA_AV_HNDL bta_handle);
/*******************************************************************************
 **
 ** Function         bta_av_co_cp_get_flag
 **
 ** Description      Get content protection flag
 **                  AVDT_CP_SCMS_COPY_NEVER
 **                  AVDT_CP_SCMS_COPY_ONCE
 **                  AVDT_CP_SCMS_COPY_FREE
 **
 ** Returns          The current flag value
 **
 ******************************************************************************/
uint8_t bta_av_co_cp_get_flag(void) { return bta_av_co_cb.cp.flag; }
/*******************************************************************************
 **
 ** Function         bta_av_co_cp_is_active
 **
 ** Description     Get the current configuration of content protection
 **
 ** Returns          TRUE if the current streaming has CP, FALSE otherwise
 **
 ******************************************************************************/
bool bta_av_co_cp_is_active(void) { return bta_av_co_cb.cp.active; }

/*******************************************************************************
 **
 ** Function         bta_av_co_cp_set_flag
 **
 ** Description      Set content protection flag
 **                  AVDT_CP_SCMS_COPY_NEVER
 **                  AVDT_CP_SCMS_COPY_ONCE
 **                  AVDT_CP_SCMS_COPY_FREE
 **
 ** Returns          true if setting the SCMS flag is supported else false
 **
 ******************************************************************************/
static bool bta_av_co_cp_set_flag(uint8_t cp_flag) {
  APPL_TRACE_DEBUG("%s: cp_flag = %d", __func__, cp_flag);

#if (BTA_AV_CO_CP_SCMS_T == TRUE)
#else
  if (cp_flag != AVDT_CP_SCMS_COPY_FREE) {
    return false;
  }
#endif
  bta_av_co_cb.cp.flag = cp_flag;
  return true;
}

/*******************************************************************************
 **
 ** Function         bta_av_co_get_peer
 **
 ** Description      find the peer entry for a given handle
 **
 ** Returns          the control block
 **
 ******************************************************************************/
static tBTA_AV_CO_PEER* bta_av_co_get_peer(tBTA_AV_HNDL hndl) {
  uint8_t index;

  index = BTA_AV_CO_AUDIO_HNDL_TO_INDX(hndl);

  APPL_TRACE_DEBUG("%s: handle = %d index = %d", __func__, hndl, index);

  /* Sanity check */
  if (index >= BTA_AV_CO_NUM_ELEMENTS(bta_av_co_cb.peers)) {
    APPL_TRACE_ERROR("%s: peer index out of bounds: %d", __func__, index);
    return NULL;
  }

  return &bta_av_co_cb.peers[index];
}

tBTA_AV_CO_PEER* bta_av_co_get_active_peer(void) {
  size_t i;
  for (i = 0; i < BTA_AV_CO_NUM_ELEMENTS(bta_av_co_cb.peers); i++) {
    tBTA_AV_CO_PEER* p_peer = &bta_av_co_cb.peers[i];
    if (p_peer->is_active_peer) break;
  }

  if (i >= BTA_AV_CO_NUM_ELEMENTS(bta_av_co_cb.peers)) {
    APPL_TRACE_ERROR("%s: peer index out of bounds: %d", __func__, i);
    return NULL;
  }

  APPL_TRACE_ERROR("%s: active peer index: %d", __func__, i);
  return &bta_av_co_cb.peers[i];
}

bool bta_av_co_is_active_peer () {
  return (bta_av_co_get_active_peer() != NULL);
}

bool bta_av_co_set_active_peer(const RawAddress& peer_address) {
  bool status = false;
  for (size_t i = 0; i < BTA_AV_CO_NUM_ELEMENTS(bta_av_co_cb.peers); i++) {
    tBTA_AV_CO_PEER* p_peer_tmp = &bta_av_co_cb.peers[i];
    if (peer_address.IsEmpty()) {
      p_peer_tmp->is_active_peer = false;
      status = true;
    } else if (p_peer_tmp->addr == peer_address) {
      p_peer_tmp->is_active_peer = true;
      status = true;

      if (p_peer_tmp->rcfg_pend_active) {
        APPL_TRACE_DEBUG("%s: set active peer, trigger reconfig again", __func__);
        const tBTA_AV_CO_SINK* p_sink = bta_av_co_audio_set_codec(p_peer_tmp);
        if (p_sink != NULL) {
          uint8_t num_protect = 0;
#if (BTA_AV_CO_CP_SCMS_T == TRUE)
          if (p_peer_tmp->cp_active) num_protect = AVDT_CP_INFO_LEN;
#endif
          if (p_peer_tmp->reconfig_needed) {
            APPL_TRACE_DEBUG("%s: call BTA_AvReconfig(x%x)", __func__, p_peer_tmp->handle);
            btif_av_set_reconfig_flag(p_peer_tmp->handle);
            BTA_AvReconfig(p_peer_tmp->handle, true, p_sink->sep_info_idx, p_peer_tmp->codec_config,
                            num_protect, bta_av_co_cp_scmst);
            p_peer_tmp->rcfg_done = true;
            p_peer_tmp->reconfig_needed = false;
          }
        }
        p_peer_tmp->rcfg_pend_active = false;
      }
    } else {
      p_peer_tmp->is_active_peer = false;
    }
  }
  return status;
}

uint8_t* bta_av_co_get_peer_codec_info(tBTA_AV_HNDL hndl) {
  uint8_t index;
  index = BTA_AV_CO_AUDIO_HNDL_TO_INDX(hndl);
  APPL_TRACE_DEBUG("%s: handle = %d index = %d", __func__, hndl, index);
  /* Sanity check */
  if (index >= BTA_AV_CO_NUM_ELEMENTS(bta_av_co_cb.peers)) {
    APPL_TRACE_ERROR("%s: peer index out of bounds: %d", __func__, index);
    return NULL;
  }
  APPL_TRACE_ERROR("%s ", __func__);
  for (int i = 0; i < AVDT_CODEC_SIZE; i++)
    APPL_TRACE_ERROR("%d ", bta_av_co_cb.peers[index].codec_config[i]);
  return bta_av_co_cb.peers[index].codec_config;
}

/*******************************************************************************
 **
 ** Function         bta_av_co_audio_init
 **
 ** Description      This callout function is executed by AV when it is
 **                  started by calling BTA_AvRegister().  This function can be
 **                  used by the phone to initialize audio paths or for other
 **                  initialization purposes.
 **
 **
 ** Returns          Stream codec and content protection capabilities info.
 **
 ******************************************************************************/
bool bta_av_co_audio_init(btav_a2dp_codec_index_t codec_index,
                          tAVDT_CFG* p_cfg) {
  return A2DP_InitCodecConfig(codec_index, p_cfg);
}

/*******************************************************************************
 **
 ** Function         bta_av_co_audio_disc_res
 **
 ** Description      This callout function is executed by AV to report the
 **                  number of stream end points (SEP) were found during the
 **                  AVDT stream discovery process.
 **
 **
 ** Returns          void.
 **
 ******************************************************************************/
void bta_av_co_audio_disc_res(tBTA_AV_HNDL hndl, uint8_t num_seps,
                              uint8_t num_sink, uint8_t num_src,
                              const RawAddress& addr, uint16_t uuid_local) {
  tBTA_AV_CO_PEER* p_peer;

  APPL_TRACE_DEBUG("%s: h:x%x num_seps:%d num_sink:%d num_src:%d", __func__,
                   hndl, num_seps, num_sink, num_src);

  /* Find the peer info */
  p_peer = bta_av_co_get_peer(hndl);
  if (p_peer == NULL) {
    APPL_TRACE_ERROR("%s: could not find peer entry", __func__);
    return;
  }

  /* Sanity check : this should never happen */
  if (p_peer->opened) {
    APPL_TRACE_ERROR("%s: peer already opened", __func__);
  }

  /* Copy the discovery results */
  p_peer->addr = addr;
  p_peer->num_sinks = num_sink;
  p_peer->num_srcs = num_src;
  p_peer->num_seps = num_seps;
  p_peer->rcfg_done = false;
  p_peer->num_rx_sinks = 0;
  p_peer->num_rx_srcs = 0;
  p_peer->num_sup_sinks = 0;
  p_peer->rcfg_pend_getcap = false;
  p_peer->getcap_pending = false;
  p_peer->rcfg_pend_active = false;
  if (uuid_local == UUID_SERVCLASS_AUDIO_SINK)
    p_peer->uuid_to_connect = UUID_SERVCLASS_AUDIO_SOURCE;
  else if (uuid_local == UUID_SERVCLASS_AUDIO_SOURCE)
    p_peer->uuid_to_connect = UUID_SERVCLASS_AUDIO_SINK;
}

/*******************************************************************************
 **
 ** Function         bta_av_audio_sink_getconfig
 **
 ** Description      This callout function is executed by AV to retrieve the
 **                  desired codec and content protection configuration for the
 **                  A2DP Sink audio stream in Initiator.
 **
 **
 ** Returns          Pass or Fail for current getconfig.
 **
 ******************************************************************************/
static tA2DP_STATUS bta_av_audio_sink_getconfig(
    tBTA_AV_HNDL hndl, uint8_t* p_codec_info, uint8_t* p_sep_info_idx,
    uint8_t seid, uint8_t* p_num_protect, uint8_t* p_protect_info) {
  tA2DP_STATUS result = A2DP_FAIL;
  tBTA_AV_CO_PEER* p_peer;

  APPL_TRACE_DEBUG("%s: handle:0x%x codec:%s seid:%d", __func__, hndl,
                   A2DP_CodecName(p_codec_info), seid);
  APPL_TRACE_DEBUG("%s: num_protect:0x%02x protect_info:0x%02x%02x%02x",
                   __func__, *p_num_protect, p_protect_info[0],
                   p_protect_info[1], p_protect_info[2]);

  /* Retrieve the peer info */
  p_peer = bta_av_co_get_peer(hndl);
  if (p_peer == NULL) {
    APPL_TRACE_ERROR("%s: could not find peer entry", __func__);
    return A2DP_FAIL;
  }

  APPL_TRACE_DEBUG("%s: peer(o=%d,n_sinks=%d,n_rx_sinks=%d,n_sup_sinks=%d)",
                   __func__, p_peer->opened, p_peer->num_srcs,
                   p_peer->num_rx_srcs, p_peer->num_sup_srcs);

  p_peer->num_rx_srcs++;

  /* Check the peer's SOURCE codec */
  if (A2DP_IsPeerSourceCodecValid(p_codec_info)) {
    /* If there is room for a new one */
    if (p_peer->num_sup_srcs < BTA_AV_CO_NUM_ELEMENTS(p_peer->srcs)) {
      tBTA_AV_CO_SINK* p_src = &p_peer->srcs[p_peer->num_sup_srcs++];

      APPL_TRACE_DEBUG("%s: saved caps[%x:%x:%x:%x:%x:%x]", __func__,
                       p_codec_info[1], p_codec_info[2], p_codec_info[3],
                       p_codec_info[4], p_codec_info[5], p_codec_info[6]);

      memcpy(p_src->codec_caps, p_codec_info, AVDT_CODEC_SIZE);
      p_src->sep_info_idx = *p_sep_info_idx;
      p_src->seid = seid;
      p_src->num_protect = *p_num_protect;
      memcpy(p_src->protect_info, p_protect_info, AVDT_CP_INFO_LEN);
    } else {
      APPL_TRACE_ERROR("%s: no more room for SRC info", __func__);
    }
  }

  /* If last SINK get capabilities or all supported codec caps retrieved */
  if ((p_peer->num_rx_srcs == p_peer->num_srcs) ||
      (p_peer->num_sup_srcs == BTA_AV_CO_NUM_ELEMENTS(p_peer->srcs))) {
    APPL_TRACE_DEBUG("%s: last SRC reached", __func__);

    /* Protect access to bta_av_co_cb.codec_config */
    mutex_global_lock();

    /* Find a src that matches the codec config */
    const tBTA_AV_CO_SINK* p_src =
        bta_av_co_find_peer_src_supports_codec(p_peer);
    if (p_src != NULL) {
      uint8_t pref_config[AVDT_CODEC_SIZE];
      APPL_TRACE_DEBUG("%s: codec supported", __func__);

      /* Build the codec configuration for this sink */
      /* Save the new configuration */
      p_peer->p_src = p_src;
      /* get preferred config from src_caps */
      if (A2DP_BuildSrc2SinkConfig(p_src->codec_caps, pref_config) !=
          A2DP_SUCCESS) {
        mutex_global_unlock();
        return A2DP_FAIL;
      }
      memcpy(p_peer->codec_config, pref_config, AVDT_CODEC_SIZE);

      APPL_TRACE_IMP("%s: p_codec_info[%x:%x:%x:%x:%x:%x]", __func__,
                       p_peer->codec_config[1], p_peer->codec_config[2],
                       p_peer->codec_config[3], p_peer->codec_config[4],
                       p_peer->codec_config[5], p_peer->codec_config[6]);
      /* By default, no content protection */
      *p_num_protect = 0;

#if (BTA_AV_CO_CP_SCMS_T == TRUE)
      p_peer->cp_active = false;
      bta_av_co_cb.cp.active = false;
#endif

      *p_sep_info_idx = p_src->sep_info_idx;
      memcpy(p_codec_info, p_peer->codec_config, AVDT_CODEC_SIZE);
      result = A2DP_SUCCESS;
    }
    /* Protect access to bta_av_co_cb.codec_config */
    mutex_global_unlock();
  }
  return result;
}

#if (BT_IOT_LOGGING_ENABLED == TRUE)
static void bta_av_co_store_peer_codectype(const tBTA_AV_CO_PEER* p_peer)
{
  int index, peer_codec_type = 0;
  const tBTA_AV_CO_SINK* p_sink;
  APPL_TRACE_DEBUG("%s", __func__);
  for (index = 0; index < p_peer->num_sup_sinks; index++) {
    p_sink = &p_peer->sinks[index];
    peer_codec_type |= A2DP_IotGetPeerSinkCodecType(p_sink->codec_caps);
  }

  device_iot_config_addr_set_hex(p_peer->addr,
          IOT_CONF_KEY_A2DP_CODECTYPE, peer_codec_type, IOT_CONF_BYTE_NUM_1);
}
#endif

/*******************************************************************************
 **
 ** Function         bta_av_co_audio_getconfig
 **
 ** Description      This callout function is executed by AV to retrieve the
 **                  desired codec and content protection configuration for the
 **                  audio stream.
 **
 **
 ** Returns          Stream codec and content protection configuration info.
 **
 ******************************************************************************/
tA2DP_STATUS bta_av_co_audio_getconfig(tBTA_AV_HNDL hndl, uint8_t* p_codec_info,
                                       uint8_t* p_sep_info_idx, uint8_t seid,
                                       uint8_t* p_num_protect,
                                       uint8_t* p_protect_info) {
  tBTA_AV_CO_PEER* p_peer;

  APPL_TRACE_DEBUG("%s", __func__);
  A2DP_DumpCodecInfo(p_codec_info);

  /* Retrieve the peer info */
  p_peer = bta_av_co_get_peer(hndl);
  if (p_peer == NULL) {
    APPL_TRACE_ERROR("%s: could not find peer entry", __func__);
    return A2DP_FAIL;
  }

  if (p_peer->uuid_to_connect == UUID_SERVCLASS_AUDIO_SOURCE) {
    return bta_av_audio_sink_getconfig(hndl, p_codec_info, p_sep_info_idx, seid,
                                       p_num_protect, p_protect_info);
  }
  APPL_TRACE_DEBUG("%s: handle:0x%x codec:%s seid:%d", __func__, hndl,
                   A2DP_CodecName(p_codec_info), seid);
  APPL_TRACE_DEBUG("%s: num_protect:0x%02x protect_info:0x%02x%02x%02x",
                   __func__, *p_num_protect, p_protect_info[0],
                   p_protect_info[1], p_protect_info[2]);
  APPL_TRACE_DEBUG("%s: peer(o=%d, n_sinks=%d, n_rx_sinks=%d, n_sup_sinks=%d)",
                   __func__, p_peer->opened, p_peer->num_sinks,
                   p_peer->num_rx_sinks, p_peer->num_sup_sinks);

  p_peer->num_rx_sinks++;

  /* Check the peer's SINK codec */
  if (A2DP_IsPeerSinkCodecValid(p_codec_info)) {
    /* If there is room for a new one */
    if (p_peer->num_sup_sinks < BTA_AV_CO_NUM_ELEMENTS(p_peer->sinks)) {
      tBTA_AV_CO_SINK* p_sink = &p_peer->sinks[p_peer->num_sup_sinks++];

      APPL_TRACE_DEBUG("%s: saved caps[%x:%x:%x:%x:%x:%x]", __func__,
                       p_codec_info[1], p_codec_info[2], p_codec_info[3],
                       p_codec_info[4], p_codec_info[5], p_codec_info[6]);

      memcpy(p_sink->codec_caps, p_codec_info, AVDT_CODEC_SIZE);
      p_sink->sep_info_idx = *p_sep_info_idx;
      p_sink->seid = seid;
      p_sink->num_protect = *p_num_protect;
      memcpy(p_sink->protect_info, p_protect_info, AVDT_CP_INFO_LEN);
    } else {
      APPL_TRACE_ERROR("%s: no more room for SINK info", __func__);
    }
  }

  std::string remote_bd_addr_str = p_peer->addr.ToString();
  const char* remote_bdstr  = remote_bd_addr_str.c_str();
  if (supported_codecs.empty()) {
      supported_codecs.append(A2DP_CodecName(p_codec_info));
      APPL_TRACE_DEBUG("%s: First codec entry %s",__func__,supported_codecs.c_str());
  } else {
      supported_codecs.append(",");
      supported_codecs.append(A2DP_CodecName(p_codec_info));
      APPL_TRACE_DEBUG("%s: Next codec entry %s",__func__,supported_codecs.c_str());
  }
  // Check if this is the last SINK get capabilities or all supported codec
  // capabilities are retrieved.
  if ((p_peer->num_rx_sinks != p_peer->num_sinks) &&
      (p_peer->num_sup_sinks != BTA_AV_CO_NUM_ELEMENTS(p_peer->sinks))) {
    return A2DP_FAIL;
  }

  //store peer supported codecs in bt_config.conf file
  btif_config_set_str(remote_bdstr, BTIF_STORAGE_KEY_FOR_SUPPORTED_CODECS, supported_codecs.c_str());
  supported_codecs.clear();

  APPL_TRACE_DEBUG("%s: last sink reached", __func__);
#if (BT_IOT_LOGGING_ENABLED == TRUE)
  bta_av_co_store_peer_codectype(p_peer);
#endif

  const tBTA_AV_CO_SINK* p_sink = bta_av_co_audio_set_codec(p_peer);
  if (p_sink == NULL) {
    APPL_TRACE_ERROR("%s: cannot set up codec for the peer SINK", __func__);
    return A2DP_FAIL;
  }

  // By default, no content protection
  *p_num_protect = 0;
#if (BTA_AV_CO_CP_SCMS_T == TRUE)
  if (p_peer->cp_active) {
    *p_num_protect = AVDT_CP_INFO_LEN;
    memcpy(p_protect_info, bta_av_co_cp_scmst, AVDT_CP_INFO_LEN);
  }
#endif

  // If acceptor -> reconfig otherwise reply for configuration.
  if (p_peer->acp) {
    // Stop fetching caps once we retrieved a supported codec.
    APPL_TRACE_EVENT("%s: no need to fetch more SEPs", __func__);
    *p_sep_info_idx = p_peer->num_seps;
    APPL_TRACE_EVENT("%s: p_peer->reconfig_needed %d p_peer->rcfg_pend_getcap %d ",
            __func__, p_peer->reconfig_needed, p_peer->rcfg_pend_getcap);

    if (p_peer->reconfig_needed || p_peer->rcfg_pend_getcap) {
      APPL_TRACE_DEBUG("%s: call BTA_AvReconfig(x%x)", __func__, hndl);
      btif_av_set_reconfig_flag(hndl);
      uint8_t index = BTA_AV_CO_AUDIO_HNDL_TO_INDX(hndl);
      btif_av_clear_remote_start_timer(index);
      BTA_AvReconfig(hndl, true, p_sink->sep_info_idx, p_peer->codec_config,
                     *p_num_protect, bta_av_co_cp_scmst);
      p_peer->rcfg_done = true;
      p_peer->reconfig_needed = false;
    }
    if (p_peer->getcap_pending) {
      APPL_TRACE_DEBUG("%s: send event BTIF_MEDIA_SOURCE_ENCODER_USER_CONFIG_UPDATE to update",
                          __func__);
      btif_a2dp_source_encoder_user_config_update_req(saved_codec_user_config,
                                                           p_peer->addr.address);
      memset(&saved_codec_user_config, 0, sizeof(btav_a2dp_codec_config_t));
    }
  } else {
    *p_sep_info_idx = p_sink->sep_info_idx;
    memcpy(p_codec_info, p_peer->codec_config, AVDT_CODEC_SIZE);
  }
  p_peer->rcfg_pend_getcap = false;
  p_peer->getcap_pending = false;

  return A2DP_SUCCESS;
}

/*******************************************************************************
 **
 ** Function         bta_av_co_audio_setconfig
 **
 ** Description      This callout function is executed by AV to set the codec
 **                  and content protection configuration of the audio stream.
 **
 **
 ** Returns          void
 **
 ******************************************************************************/
void bta_av_co_audio_setconfig(tBTA_AV_HNDL hndl, const uint8_t* p_codec_info,
                               uint8_t seid,
                               const RawAddress& addr, uint8_t num_protect,
                               const uint8_t* p_protect_info,
                               uint8_t t_local_sep, uint8_t avdt_handle) {
  tBTA_AV_CO_PEER* p_peer;
  tA2DP_STATUS status = A2DP_SUCCESS;
  uint8_t category = A2DP_SUCCESS;
  bool reconfig_needed = false;
  char value[PROPERTY_VALUE_MAX] = "false";

  std::string addrstr = addr.ToString();
  const char* bd_addr_str = addrstr.c_str();
  APPL_TRACE_DEBUG("%s: Device [%s], seid = %d", __func__, bd_addr_str, seid);
  APPL_TRACE_IMP("%s: p_codec_info[%x:%x:%x:%x:%x:%x]", __func__,
                   p_codec_info[1], p_codec_info[2], p_codec_info[3],
                   p_codec_info[4], p_codec_info[5], p_codec_info[6]);
  APPL_TRACE_DEBUG("num_protect:0x%02x protect_info:0x%02x%02x%02x",
                   num_protect, p_protect_info[0], p_protect_info[1],
                   p_protect_info[2]);
  A2DP_DumpCodecInfo(p_codec_info);

  /* Retrieve the peer info */
  p_peer = bta_av_co_get_peer(hndl);
  if (p_peer == NULL) {
    APPL_TRACE_ERROR("%s: could not find peer entry", __func__);
    /* Call call-in rejecting the configuration */
    bta_av_ci_setconfig(hndl, A2DP_BUSY, AVDT_ASC_CODEC, 0, &seid, false,
                        avdt_handle);
    return;
  }

  APPL_TRACE_DEBUG("%s: peer(o=%d, n_sinks=%d, n_rx_sinks=%d, n_sup_sinks=%d)",
                   __func__, p_peer->opened, p_peer->num_sinks,
                   p_peer->num_rx_sinks, p_peer->num_sup_sinks);

  p_peer->isIncoming = bta_av_get_is_peer_state_incoming(addr);
  APPL_TRACE_DEBUG("%s: isIncoming: %d", __func__, p_peer->isIncoming);
  if (p_peer->isIncoming) {
    p_peer->codecIndextoCompare = A2DP_SourceCodecIndex(p_codec_info);
    APPL_TRACE_DEBUG("%s: codecIndextoCompare: %d", __func__, p_peer->codecIndextoCompare);
  }

  /* Sanity check: should not be opened at this point */
  if (p_peer->opened) {
    APPL_TRACE_ERROR("%s: peer already in use", __func__);
  }

  if (num_protect != 0) {
#if (BTA_AV_CO_CP_SCMS_T == TRUE)
    /* If CP is supported */
    if ((num_protect != 1) ||
        (bta_av_co_cp_is_scmst(p_protect_info) == false)) {
      APPL_TRACE_ERROR("%s: wrong CP configuration", __func__);
      status = A2DP_BAD_CP_TYPE;
      category = AVDT_ASC_PROTECT;
    }
#else
    /* Do not support content protection for the time being */
    APPL_TRACE_ERROR("%s: wrong CP configuration", __func__);
    status = A2DP_BAD_CP_TYPE;
    category = AVDT_ASC_PROTECT;
#endif
  }

  if (status == A2DP_SUCCESS) {
    bool codec_config_supported = false;

    if (t_local_sep == AVDT_TSEP_SNK) {
      APPL_TRACE_DEBUG("%s: peer is A2DP SRC", __func__);
      codec_config_supported = A2DP_IsSinkCodecSupported(p_codec_info);
      if (codec_config_supported) {
        // If Peer is SRC, and our config subset matches with what is
        // requested by peer, then just accept what peer wants.
        bta_av_co_save_new_codec_config(p_peer, p_codec_info, num_protect,
                                        p_protect_info);
      }
    }
    if (t_local_sep == AVDT_TSEP_SRC) {
      APPL_TRACE_DEBUG("%s: peer is A2DP SINK", __func__);
      bool restart_output = false;
      p_peer->addr = addr;
      if ((p_peer->codecs == nullptr) ||
          !bta_av_co_set_codec_ota_config(p_peer, p_codec_info, num_protect,
                                          p_protect_info, &restart_output)) {
        APPL_TRACE_DEBUG("%s: cannot set source codec %s", __func__,
                         A2DP_CodecName(p_codec_info));
      } else {
        codec_config_supported = true;
        APPL_TRACE_DEBUG("%s: restart_output: %d", __func__, restart_output);
        // Check if reconfiguration is needed
        if (restart_output ||
            ((num_protect == 1) && (!bta_av_co_cb.cp.active))) {
          reconfig_needed = true;
        }
      }
    }

    /* Check if codec configuration is supported */
    if (!codec_config_supported) {
      category = AVDT_ASC_CODEC;
      status = A2DP_WRONG_CODEC;
    }
  }

  if (status != A2DP_SUCCESS) {
    APPL_TRACE_ERROR("%s: reject s=%d c=%d", __func__, status, category);
    /* Call call-in rejecting the configuration */
    bta_av_ci_setconfig(hndl, status, category, 0, &seid, false, avdt_handle);
    return;
  }
  property_get("persist.vendor.bt.a2dp.ldac_96k_support", value, "false");
  if (!strncmp("true", value, 4)) {
    btav_a2dp_codec_index_t ota_codec_index =
        A2DP_SourceCodecIndex(p_codec_info);
    if (p_peer->isIncoming && ota_codec_index == BTAV_A2DP_CODEC_INDEX_SOURCE_LDAC
        && (A2DP_GetTrackSampleRate(p_codec_info) != A2DP_LDAC_CODEC_BEST_SAMPLE_RATE)) {
      APPL_TRACE_DEBUG("%s: Selected codec is LDAC and reconfig is needed", __func__);
      reconfig_needed = true;
    }
  }
  /* Mark that this is an acceptor peer */
  p_peer->acp = true;
  p_peer->reconfig_needed = reconfig_needed;
  APPL_TRACE_DEBUG("%s: accept reconf=%d", __func__, reconfig_needed);
  /* Call call-in accepting the configuration */
  bta_av_ci_setconfig(hndl, A2DP_SUCCESS, A2DP_SUCCESS, 0, &seid,
                      reconfig_needed, avdt_handle);
}

/*******************************************************************************
 **
 ** Function         bta_av_co_audio_open
 **
 ** Description      This function is called by AV when the audio stream
 **                  connection is opened.
 **
 **
 ** Returns          void
 **
 ******************************************************************************/
void bta_av_co_audio_open(tBTA_AV_HNDL hndl, uint16_t mtu) {
  tBTA_AV_CO_PEER* p_peer;

  APPL_TRACE_DEBUG("%s: handle: %d mtu:%d", __func__, hndl, mtu);

  /* Retrieve the peer info */
  p_peer = bta_av_co_get_peer(hndl);
  if (p_peer == NULL) {
    APPL_TRACE_ERROR("%s: could not find peer entry", __func__);
  } else {
    p_peer->opened = true;
    p_peer->mtu = mtu;
    p_peer->handle = hndl;
  }
}

/*******************************************************************************
 **
 ** Function         bta_av_co_audio_close
 **
 ** Description      This function is called by AV when the audio stream
 **                  connection is closed.
 **
 **
 ** Returns          void
 **
 ******************************************************************************/
void bta_av_co_audio_close(tBTA_AV_HNDL hndl) {
  tBTA_AV_CO_PEER* p_peer;
  uint8_t index;

  index = BTA_AV_CO_AUDIO_HNDL_TO_INDX(hndl);

  APPL_TRACE_DEBUG("%s hndl = 0x%x, index = %d", __func__, hndl, index);
  btif_av_reset_audio_delay(hndl);

  /* Retrieve the peer info */
  p_peer = bta_av_co_get_peer(hndl);
  if (p_peer && (index < BTA_AV_CO_NUM_ELEMENTS(bta_av_co_cb.peers))) {
    /* Mark the peer closed and clean the peer info */
    bta_av_co_free_peer(p_peer);
    APPL_TRACE_DEBUG("%s call bta_av_co_peer_init", __func__);
    bta_av_co_peer_init(bta_av_co_cb.default_codec_priorities, index);
  } else {
    APPL_TRACE_ERROR("%s: could not find peer entry", __func__);
  }
}

/*******************************************************************************
 **
 ** Function         bta_av_co_free_peer
 **
 ** Description      This function free up and clears peer data when the audio
 **                  stream connection is closed.
 **
 **
 ** Returns          void
 **
 ******************************************************************************/
void bta_av_co_free_peer(tBTA_AV_CO_PEER* p_peer) {
  memset(&p_peer->addr, 0, sizeof(RawAddress));
  memset(p_peer->sinks, 0, sizeof(p_peer->sinks));
  memset(p_peer->srcs, 0, sizeof(p_peer->srcs));
  p_peer->num_sinks = 0;
  p_peer->num_srcs = 0;
  p_peer->num_seps = 0;
  p_peer->num_rx_sinks = 0;
  p_peer->num_rx_srcs = 0;
  p_peer->num_sup_sinks = 0;
  p_peer->num_sup_srcs = 0;
  memset(p_peer->codec_config, 0, sizeof(p_peer->codec_config));
  p_peer->cp_active = 0;
  p_peer->acp = 0;
  p_peer->reconfig_needed = 0;
  p_peer->rcfg_done = 0;
  p_peer->opened = 0;
  p_peer->mtu = 0;
  p_peer->uuid_to_connect = 0;
  p_peer->handle = 0;
  p_peer->is_active_peer = 0;
  p_peer->rcfg_pend_getcap = 0;
  p_peer->isIncoming = 0;
  p_peer->codecIndextoCompare = (btav_a2dp_codec_index_t)0;
  p_peer->getcap_pending = 0;
}


/*******************************************************************************
 **
 ** Function         bta_av_co_audio_start
 **
 ** Description      This function is called by AV when the audio streaming data
 **                  transfer is started.
 **
 **
 ** Returns          void
 **
 ******************************************************************************/
void bta_av_co_audio_start(UNUSED_ATTR tBTA_AV_HNDL hndl,
                           UNUSED_ATTR uint8_t* p_codec_info,
                           UNUSED_ATTR bool* p_no_rtp_hdr) {
  APPL_TRACE_DEBUG("%s", __func__);
}

/*******************************************************************************
 **
 ** Function         bta_av_co_audio_stop
 **
 ** Description      This function is called by AV when the audio streaming data
 **                  transfer is stopped.
 **
 **
 ** Returns          void
 **
 ******************************************************************************/
void bta_av_co_audio_stop(UNUSED_ATTR tBTA_AV_HNDL hndl) {
  APPL_TRACE_DEBUG("%s", __func__);
}

/*******************************************************************************
 **
 ** Function         bta_av_co_audio_src_data_path
 **
 ** Description      This function is called to manage data transfer from
 **                  the audio codec to AVDTP.
 **
 ** Returns          Pointer to the GKI buffer to send, NULL if no buffer to
 **                  send
 **
 ******************************************************************************/
void* bta_av_co_audio_src_data_path(const uint8_t* p_codec_info,
                                    uint32_t* p_timestamp) {
  BT_HDR* p_buf;

  APPL_TRACE_DEBUG("%s: codec: %s", __func__, A2DP_CodecName(p_codec_info));

  p_buf = btif_a2dp_source_audio_readbuf();
  if (p_buf == NULL) {
    APPL_TRACE_DEBUG("%s: p_buf is null, return", __func__);
    return NULL;
  }

  /*
   * Retrieve the timestamp information from the media packet,
   * and set up the packet header.
   *
   * In media packet, the following information is available:
   * p_buf->layer_specific : number of audio frames in the packet
   * p_buf->word[0] : timestamp
   */
  if (!A2DP_GetPacketTimestamp(p_codec_info, (const uint8_t*)(p_buf + 1),
                               p_timestamp) ||
      !A2DP_BuildCodecHeader(p_codec_info, p_buf, p_buf->layer_specific)) {
    APPL_TRACE_ERROR("%s: unsupported codec type (%d)", __func__,
                     A2DP_GetCodecType(p_codec_info));
  }

#if (BTA_AV_CO_CP_SCMS_T == TRUE)
  if (bta_av_co_cb.cp.active) {
    p_buf->len++;
    p_buf->offset--;
    uint8_t* p = (uint8_t*)(p_buf + 1) + p_buf->offset;
    *p = bta_av_co_cp_get_flag();
  }
#endif

  return p_buf;
}

/*******************************************************************************
 **
 ** Function         bta_av_co_audio_drop
 **
 ** Description      An Audio packet is dropped. .
 **                  It's very likely that the connected headset with this
 **                  handle is moved far away. The implementation may want to
 **                  reduce the encoder bit rate setting to reduce the packet
 **                  size.
 **
 ** Returns          void
 **
 ******************************************************************************/
void bta_av_co_audio_drop(tBTA_AV_HNDL hndl) {
  APPL_TRACE_ERROR("%s: dropped audio packet on handle 0x%x", __func__, hndl);
}

/*******************************************************************************
 **
 ** Function         bta_av_co_audio_delay
 **
 ** Description      This function is called by AV when the audio stream
 **                  connection needs to send the initial delay report to the
 **                  connected SRC.
 **
 **
 ** Returns          void
 **
 ******************************************************************************/
void bta_av_co_audio_delay(tBTA_AV_HNDL hndl, uint16_t delay) {
  APPL_TRACE_ERROR("%s: handle: x%x, delay:0x%x", __func__, hndl, delay);
  btif_av_set_audio_delay(delay, hndl);
}

void bta_av_co_audio_update_mtu(tBTA_AV_HNDL hndl, uint16_t mtu) {
  tBTA_AV_CO_PEER* p_peer;

  APPL_TRACE_DEBUG("%s: handle: %d mtu: %d", __func__, hndl, mtu);

  /* Retrieve the peer info */
  p_peer = bta_av_co_get_peer(hndl);
  if (p_peer == NULL) {
    APPL_TRACE_ERROR("%s: could not find peer entry", __func__);
    return;
  }
  p_peer->mtu = mtu;
}

/*******************************************************************************
 **
 ** Function         bta_av_co_cp_is_scmst
 **
 ** Description      Check if a content protection service is SCMS-T
 **
 ** Returns          true if this CP is SCMS-T, false otherwise
 **
 ******************************************************************************/
static bool bta_av_co_cp_is_scmst(const uint8_t* p_protect_info) {
  APPL_TRACE_DEBUG("%s", __func__);

  if (*p_protect_info >= AVDT_CP_LOSC) {
    uint16_t cp_id;

    p_protect_info++;
    STREAM_TO_UINT16(cp_id, p_protect_info);
    if (cp_id == AVDT_CP_SCMS_T_ID) {
      APPL_TRACE_DEBUG("%s: SCMS-T found", __func__);
      return true;
    }
  }

  return false;
}

// Check if audio protect info contains SCMS-T Copy Protection
// Returns true if |p_protect_info| contains SCMS-T, otherwise false.
static bool bta_av_co_audio_protect_has_scmst(uint8_t num_protect,
                                              const uint8_t* p_protect_info) {
  APPL_TRACE_DEBUG("%s", __func__);

  while (num_protect--) {
    if (bta_av_co_cp_is_scmst(p_protect_info)) return true;
    /* Move to the next SC */
    p_protect_info += *p_protect_info + 1;
  }
  APPL_TRACE_DEBUG("%s: SCMS-T not found", __func__);
  return false;
}

/*******************************************************************************
 **
 ** Function         bta_av_co_audio_is_aac_wl_enabled
 **
 ** Description      Check if AAC white-list is enabled or not
 **
 ** Returns          returns default value true, false if property is disabled
 **
 ******************************************************************************/
bool bta_av_co_audio_is_aac_wl_enabled(const RawAddress *remote_bdaddr) {

  int retval;
  bool res = FALSE;
  char is_whitelist_by_default[255] = "false";
  retval = property_get("persist.vendor.bt.a2dp.aac_whitelist", is_whitelist_by_default, "true");
  BTIF_TRACE_DEBUG("%s: property_get: bt.a2dp.aac_whitelist: %s, retval: %d",
                                  __func__, is_whitelist_by_default, retval);
  if (!strncmp(is_whitelist_by_default, "true", 4)) {
      res = TRUE;
  }
  return res;
}

/*******************************************************************************
 **
 ** Function         bta_av_co_audio_device_addr_check_is_enabled
 **
 ** Description      Check if address based property is enabled or not
 **
 ** Returns          returns default value true, false if property is disabled
 **
 ******************************************************************************/
bool bta_av_co_audio_device_addr_check_is_enabled(const RawAddress *remote_bdaddr) {

  int retval;
  bool res = FALSE;
  char is_addr_check_enabled_by_default[255] = "false";
  retval = property_get("persist.vendor.bt.a2dp.addr_check_enabled_for_aac", is_addr_check_enabled_by_default, "true");
  BTIF_TRACE_DEBUG("%s: property_get: bt.a2dp.addr_check_enabled_for_aac: %s, retval: %d",
                                  __func__, is_addr_check_enabled_by_default, retval);
  if (!strncmp(is_addr_check_enabled_by_default, "true", 4)) {
      res = TRUE;
  }
  return res;
}

/*******************************************************************************
 **
 ** Function         bta_av_co_audio_sink_supports_cp
 **
 ** Description      Check if a sink supports the current content protection
 **
 ** Returns          true if the sink supports this CP, false otherwise
 **
 ******************************************************************************/
static bool bta_av_co_audio_sink_supports_cp(const tBTA_AV_CO_SINK* p_sink) {
  APPL_TRACE_DEBUG("%s", __func__);

  /* Check if content protection is enabled for this stream */
  if (bta_av_co_cp_get_flag() != AVDT_CP_SCMS_COPY_FREE) {
    return bta_av_co_audio_protect_has_scmst(p_sink->num_protect,
                                             p_sink->protect_info);
  }

  APPL_TRACE_DEBUG("%s: not required", __func__);
  return true;
}

/*******************************************************************************
 **
 ** Function         bta_av_co_find_peer_src_supports_codec
 **
 ** Description      Find a peer acting as src that supports codec config
 **
 ** Returns          The peer source that supports the codec, otherwise NULL.
 **
 ******************************************************************************/
static const tBTA_AV_CO_SINK* bta_av_co_find_peer_src_supports_codec(
    const tBTA_AV_CO_PEER* p_peer) {
  APPL_TRACE_DEBUG("%s: peer num_sup_srcs = %d", __func__,
                   p_peer->num_sup_srcs);

  for (size_t index = 0; index < p_peer->num_sup_srcs; index++) {
    const uint8_t* p_codec_caps = p_peer->srcs[index].codec_caps;
    if (A2DP_CodecTypeEquals(bta_av_co_cb.codec_config, p_codec_caps) &&
        A2DP_IsPeerSourceCodecSupported(p_codec_caps)) {
      return &p_peer->srcs[index];
    }
  }
  return NULL;
}

/*******************************************************************************
 **
 ** Function         bta_av_co_check_peer_eligible_for_aac_codec
 **
 ** Description      Find if peer device full fill requirements for aac support
 **
 ** Returns          true if eligible for aac else false
 **
 ******************************************************************************/
static bool bta_av_co_check_peer_eligible_for_aac_codec(
    const tBTA_AV_CO_PEER* p_peer) {
  char remote_name[BTM_MAX_REM_BD_NAME_LEN] = "";
  uint16_t vendor = 0;
  uint16_t product = 0;
  uint16_t version = 0;
  bool vndr_prdt_ver_present = false;
  bool aac_support = false;
  if (btif_config_get_uint16(p_peer->addr.ToString().c_str(), PNP_VENDOR_ID_CONFIG_KEY,
      (uint16_t*)&vendor) && btif_config_get_uint16(p_peer->addr.ToString().c_str(),
      PNP_PRODUCT_ID_CONFIG_KEY, (uint16_t*)&product) && btif_config_get_uint16(p_peer->addr.ToString().c_str(),
      PNP_PRODUCT_VERSION_CONFIG_KEY, (uint16_t*)&version)) {
    APPL_TRACE_DEBUG("%s: vendor: 0x%04x product: 0x%04x version: 0x%04x", __func__, vendor, product, version);
    vndr_prdt_ver_present = true;
  }
  if (vndr_prdt_ver_present && interop_database_match_version(INTEROP_ENABLE_AAC_CODEC, version) &&
      interop_match_vendor_product_ids(INTEROP_ENABLE_AAC_CODEC, vendor, product)) {
    APPL_TRACE_DEBUG("%s: vendor id, product id and version info matching with conf file", __func__);
    vndr_prdt_ver_present = false;
    aac_support = true;
  } else {
    if (bta_av_co_audio_is_aac_wl_enabled(&p_peer->addr)) {
      if (bta_av_co_audio_device_addr_check_is_enabled(&p_peer->addr)) {
        if (btif_storage_get_stored_remote_name(p_peer->addr, remote_name) &&
            interop_match_addr(INTEROP_ENABLE_AAC_CODEC, &p_peer->addr) &&
            interop_match_name(INTEROP_ENABLE_AAC_CODEC, remote_name)) {
          APPL_TRACE_DEBUG("%s: AAC is supported for this WL remote device", __func__);
          aac_support = true;
        } else {
          APPL_TRACE_DEBUG("%s: RD is not present in name and address based check for AAC WL.",
                             __func__);
        }
      } else {
        if (btif_storage_get_stored_remote_name(p_peer->addr, remote_name) &&
            interop_match_name(INTEROP_ENABLE_AAC_CODEC, remote_name)) {
          APPL_TRACE_DEBUG("%s: AAC is supported for this WL remote device", __func__);
          aac_support = true;
        } else {
          APPL_TRACE_DEBUG("%s: RD is not present in name based check for AAC WL.",
                            __func__);
        }
      }
    } else {
      if (bta_av_co_audio_device_addr_check_is_enabled(&p_peer->addr)) {
        if (interop_match_addr_or_name(INTEROP_DISABLE_AAC_CODEC, &p_peer->addr)) {
          APPL_TRACE_DEBUG("AAC is not supported for this BL remote device");
        } else {
          APPL_TRACE_DEBUG("%s: AAC is supported for this remote device", __func__);
          aac_support = true;
        }
      } else {
        if (btif_storage_get_stored_remote_name(p_peer->addr, remote_name) &&
            interop_match_name(INTEROP_DISABLE_AAC_CODEC, remote_name)) {
          APPL_TRACE_DEBUG("AAC is not supported for this BL remote device");
        } else {
          APPL_TRACE_DEBUG("%s: AAC is supported for this remote device", __func__);
          aac_support = true;
        }
      }
    }
  }
  return aac_support;
}

//
// Select the current codec configuration based on peer codec support.
// Furthermore, the local state for the remaining non-selected codecs is
// updated to reflect whether the codec is selectable.
// Return a pointer to the corresponding |tBTA_AV_CO_SINK| sink entry
// on success, otherwise NULL.
//
static tBTA_AV_CO_SINK* bta_av_co_audio_set_codec(tBTA_AV_CO_PEER* p_peer) {
  tBTA_AV_CO_SINK* p_sink = NULL;
  // Update all selectable codecs.
  // This is needed to update the selectable parameters for each codec.
  // NOTE: The selectable codec info is used only for informational purpose.
  if (p_peer->codecs == nullptr) {
    APPL_TRACE_ERROR("%s: p_peer->codecs is null", __func__);
    return NULL;
  }
  for (const auto& iter : p_peer->codecs->orderedSourceCodecs()) {
    APPL_TRACE_DEBUG("%s: updating selectable codec %s", __func__,
                     iter->name().c_str());
#if (TWS_ENABLED == TRUE)
    if ((!strcmp(iter->name().c_str(),"aptX-TWS")) && !BTM_SecIsTwsPlusDev(p_peer->addr)) {
        APPL_TRACE_DEBUG("%s:Non-TWS+ device, skip update selectable aptX-TWS codec",__func__);
        continue;
    }
#endif
    if (!strcmp(iter->name().c_str(),"AAC")) {
      if (bta_av_co_check_peer_eligible_for_aac_codec(p_peer)) {
        APPL_TRACE_DEBUG("%s: Show AAC support in selectable capability", __func__);
        bta_av_co_audio_update_selectable_codec(*iter, p_peer);
      } else {
        APPL_TRACE_DEBUG("%s: Do not show AAC support in selectable capability", __func__);
        continue;
      }
    } else {
      bta_av_co_audio_update_selectable_codec(*iter, p_peer);
    }
  }
  // Select the codec
  for (const auto& iter : p_peer->codecs->orderedSourceCodecs()) {
    APPL_TRACE_DEBUG("%s: trying codec %s", __func__, iter->name().c_str());
#if (TWS_ENABLED == TRUE)
    if ((!strcmp(iter->name().c_str(),"aptX-TWS")) && !BTM_SecIsTwsPlusDev(p_peer->addr)) {
      APPL_TRACE_DEBUG("%s:Non-TWS+ device, skip aptX-TWS codec",__func__);
      continue;
    }
#endif
    if (!strcmp(iter->name().c_str(),"AAC")) {
      if (bta_av_co_check_peer_eligible_for_aac_codec(p_peer)) {
        APPL_TRACE_DEBUG("%s: AAC codec is added in sink capability", __func__);
        p_sink = bta_av_co_audio_codec_selected(*iter, p_peer);
      } else {
        APPL_TRACE_DEBUG("%s: Do not add AAC codec in sink capability", __func__);
      }
    } else {
      APPL_TRACE_DEBUG("%s: non-AAC codec has been selected.", __func__);
      p_sink = bta_av_co_audio_codec_selected(*iter, p_peer);
    }

    if (p_sink != NULL) {
      APPL_TRACE_DEBUG("%s: selected codec %s", __func__, iter->name().c_str());
      if ((p_peer->num_rx_sinks != p_peer->num_sinks) &&
          (p_peer->num_sup_sinks != BTA_AV_CO_NUM_ELEMENTS(p_peer->sinks))) {
        APPL_TRACE_DEBUG("%s: GetCap did not complete, avoid sending codec config updated evt", __func__);
      } else {
        if (p_peer->isIncoming) {
          btav_a2dp_codec_index_t current_peer_codec_index = A2DP_SourceCodecIndex(p_peer->codec_config);
          APPL_TRACE_DEBUG("%s: current_peer_codec_index: %d, isIncoming: %d",
                              __func__, current_peer_codec_index, p_peer->isIncoming);
          if (current_peer_codec_index != p_peer->codecIndextoCompare) {
            if (p_peer->is_active_peer) {
              p_peer->reconfig_needed = true;
              p_peer->isIncoming = false;
              p_peer->rcfg_pend_active = false;
              APPL_TRACE_DEBUG("%s: incoming codec Idx mismatched with outgoing codec Idx: %d",
                                   __func__, p_peer->reconfig_needed);
            } else {
              p_peer->rcfg_pend_active = true;
              APPL_TRACE_DEBUG("%s: bta av stream of peer is not opened: %d, pending reconfig",
                                   __func__, p_peer->rcfg_pend_active);
              break;
            }
          }
        }
        // NOTE: Conditionally dispatch the event to make sure a callback with
        // the most recent codec info is generated.
        btif_dispatch_sm_event(BTIF_AV_SOURCE_CONFIG_UPDATED_EVT, (void *)p_peer->addr.address,
                         sizeof(RawAddress));
      }
      break;
    }
    APPL_TRACE_DEBUG("%s: cannot use codec %s", __func__, iter->name().c_str());
  }
  APPL_TRACE_DEBUG("%s BDA:%s", __func__, p_peer->addr.ToString().c_str());
  return p_sink;
}

// Select an open device for the preferred codec specified by |codec_config|.
// Return the corresponding peer that supports the codec, otherwise NULL.
static tBTA_AV_CO_SINK* bta_av_co_audio_codec_selected(
    A2dpCodecConfig& codec_config, tBTA_AV_CO_PEER* p_peer) {
  uint8_t new_codec_config[AVDT_CODEC_SIZE];
  APPL_TRACE_DEBUG("%s: num_sup_sinks: %d", __func__, p_peer->num_sup_sinks);

  // Find the peer sink for the codec
  tBTA_AV_CO_SINK* p_sink = NULL;
  for (size_t index = 0; index < p_peer->num_sup_sinks; index++) {
    btav_a2dp_codec_index_t peer_codec_index =
        A2DP_SourceCodecIndex(p_peer->sinks[index].codec_caps);

    APPL_TRACE_DEBUG("%s ind: %d, peer_codec_index : %d :: codec_config.codecIndex() : %d",
           __func__, index, peer_codec_index, codec_config.codecIndex());

    if (peer_codec_index != codec_config.codecIndex()) {
      continue;
    }
    if (!bta_av_co_audio_sink_supports_cp(&p_peer->sinks[index])) {
      APPL_TRACE_DEBUG(
          "%s: peer sink for codec %s does not support "
          "Copy Protection",
          __func__, codec_config.name().c_str());
      continue;
    }
    p_sink = &p_peer->sinks[index];
    break;
  }
  if (p_sink == NULL) {
    APPL_TRACE_DEBUG("%s: peer sink for codec %s not found", __func__,
                     codec_config.name().c_str());
    return NULL;
  }
  if ((p_peer->codecs == nullptr) || !p_peer->codecs->setCodecConfig(
          p_sink->codec_caps, true /* is_capability */, new_codec_config,
          true /* select_current_codec */)) {
    APPL_TRACE_DEBUG("%s: cannot set source codec %s", __func__,
                     codec_config.name().c_str());
    return NULL;
  }

  /*
  ** In case of acceptor or remote initiated connection need to update enocder
  ** and audio config with codec config details as sent by remote during set config
  */
  if (p_peer->acp == true && isDevUiReq != true  && p_peer->rcfg_done != true) {
    /* check if sample rate is non-zero (remote set-conf) and
     * cache peer config for possibility of reconfiguation for better config */
    A2dpCodecConfig* current_codec_ = p_peer->codecs->getCurrentCodecConfig();
    if (current_codec_ != nullptr) {
      btav_a2dp_codec_config_t update_codec_config = current_codec_->getCodecConfig();
      /* check if sample rate is non-zero (remote set-conf) and
       * cache peer config for possibility of reconfiguation for better config */
      if (A2DP_SourceCodecIndex(codec_config.ota_codec_peer_config_) ==
        BTAV_A2DP_CODEC_INDEX_SOURCE_LDAC && codec_config.ota_codec_peer_config_[9] != 0) {
        uint8_t sampleRate = codec_config.ota_codec_peer_config_[9];
        if (sampleRate & A2DP_LDAC_SAMPLING_FREQ_44100) {
          update_codec_config.sample_rate = BTAV_A2DP_CODEC_SAMPLE_RATE_44100;
        } else if (sampleRate & A2DP_LDAC_SAMPLING_FREQ_48000) {
          update_codec_config.sample_rate = BTAV_A2DP_CODEC_SAMPLE_RATE_48000;
        } else if (sampleRate & A2DP_LDAC_SAMPLING_FREQ_88200) {
          update_codec_config.sample_rate = BTAV_A2DP_CODEC_SAMPLE_RATE_88200;
        } else if (sampleRate & A2DP_LDAC_SAMPLING_FREQ_96000) {
          update_codec_config.sample_rate = BTAV_A2DP_CODEC_SAMPLE_RATE_96000;
        }
        APPL_TRACE_WARNING("%s: Update encoder config with peer config ", __func__);
        if (p_peer->codecs->updateCodecConfig(update_codec_config))
          memcpy(codec_config.ota_codec_config_, codec_config.ota_codec_peer_config_, AVDT_CODEC_SIZE);
      }
    } else {
      APPL_TRACE_WARNING("%s: current codec is NULL, do not update encoder and audio config ", __func__);
    }
  }

  p_peer->p_sink = p_sink;

  bta_av_co_save_new_codec_config(p_peer, new_codec_config, p_sink->num_protect,
                                  p_sink->protect_info);
  // NOTE: Event BTIF_AV_SOURCE_CONFIG_UPDATED_EVT is dispatched by the caller

  return p_sink;
}

// Update a selectable codec |codec_config| with the corresponding codec
// information from a peer device |p_peer|.
// Returns true if the codec is updated, otherwise false.
static bool bta_av_co_audio_update_selectable_codec(
    A2dpCodecConfig& codec_config, const tBTA_AV_CO_PEER* p_peer) {
  uint8_t new_codec_config[AVDT_CODEC_SIZE];

  APPL_TRACE_DEBUG("%s: num_sup_sinks: %d", __func__, p_peer->num_sup_sinks);

  // Find the peer sink for the codec
  const tBTA_AV_CO_SINK* p_sink = NULL;
  for (size_t index = 0; index < p_peer->num_sup_sinks; index++) {
    btav_a2dp_codec_index_t peer_codec_index =
        A2DP_SourceCodecIndex(p_peer->sinks[index].codec_caps);
    APPL_TRACE_DEBUG("%s ind: %d, peer_codec_index : %d :: codec_config.codecIndex() : %d",
           __func__, index, peer_codec_index, codec_config.codecIndex());
    if (peer_codec_index != codec_config.codecIndex()) {
      continue;
    }
    if (!bta_av_co_audio_sink_supports_cp(&p_peer->sinks[index])) {
      APPL_TRACE_DEBUG(
          "%s: peer sink for codec %s does not support "
          "Copy Protection",
          __func__, codec_config.name().c_str());
      continue;
    }
    p_sink = &p_peer->sinks[index];
    break;
  }
  if (p_sink == NULL) {
    // The peer sink device does not support this codec
    return false;
  }
  if ((p_peer->codecs == nullptr) || !p_peer->codecs->setCodecConfig(
          p_sink->codec_caps, true /* is_capability */, new_codec_config,
          false /* select_current_codec */)) {
    APPL_TRACE_DEBUG("%s: cannot update source codec %s", __func__,
                     codec_config.name().c_str());
    return false;
  }
  return true;
}

static void bta_av_co_save_new_codec_config(tBTA_AV_CO_PEER* p_peer,
                                            const uint8_t* new_codec_config,
                                            uint8_t num_protect,
                                            const uint8_t* p_protect_info) {
  APPL_TRACE_DEBUG("%s", __func__);
  A2DP_DumpCodecInfo(new_codec_config);

  // Protect access to bta_av_co_cb.codec_config
  mutex_global_lock();

  memcpy(bta_av_co_cb.codec_config, new_codec_config,
         sizeof(bta_av_co_cb.codec_config));
  memcpy(p_peer->codec_config, new_codec_config, AVDT_CODEC_SIZE);

#if (BTA_AV_CO_CP_SCMS_T == TRUE)
  /* Check if this sink supports SCMS */
  bool cp_active =
      bta_av_co_audio_protect_has_scmst(num_protect, p_protect_info);
  bta_av_co_cb.cp.active = cp_active;
  p_peer->cp_active = cp_active;
#endif

  // Protect access to bta_av_co_cb.codec_config
  mutex_global_unlock();
}

void bta_av_co_get_peer_params(tA2DP_ENCODER_INIT_PEER_PARAMS* p_peer_params) {
  uint16_t min_mtu = 0xFFFF;
  int index = btif_max_av_clients;
  const tBTA_AV_CO_PEER* p_peer;

  APPL_TRACE_DEBUG("%s", __func__);
  CHECK(p_peer_params != nullptr);

  /* Protect access to bta_av_co_cb.codec_config */
  mutex_global_lock();

  /* Compute the MTU */
  for (size_t i = 0; i < BTA_AV_CO_NUM_ELEMENTS(bta_av_co_cb.peers); i++) {
    p_peer = &bta_av_co_cb.peers[i];
    if (!p_peer->opened) continue;
    if (p_peer->mtu < min_mtu) min_mtu = p_peer->mtu;
    APPL_TRACE_DEBUG("%s: peer MTU %d for connected index %d", __func__, p_peer->mtu, i);
  }

  index = btif_av_get_current_playing_dev_idx();
  if ((index < btif_max_av_clients) && (!btif_av_is_multicast_supported())) {
    p_peer = &bta_av_co_cb.peers[index];
    min_mtu = p_peer->mtu;
    if (min_mtu > BTA_AV_MAX_A2DP_MTU)
      min_mtu = BTA_AV_MAX_A2DP_MTU;
    if (min_mtu == 0) {
      APPL_TRACE_WARNING("%s min_mtu received as 0, updating to: %d",
                               __func__, MAX_2MBPS_AVDTP_MTU);
      min_mtu = MAX_2MBPS_AVDTP_MTU;
    }
    bool is_AAC_frame_ctrl_stack_enable = controller_get_interface()->supports_aac_frame_ctl();

    APPL_TRACE_DEBUG("%s: Stack AAC frame control enabled: %d", __func__, is_AAC_frame_ctrl_stack_enable);
    if (is_AAC_frame_ctrl_stack_enable && btif_av_is_peer_edr() &&
                               (btif_av_peer_supports_3mbps() == FALSE)) {
      // This condition would be satisfied only if the remote device is
      // EDR and supports only 2 Mbps, but the effective AVDTP MTU size
      // exceeds the 2DH5 packet size.
      APPL_TRACE_DEBUG("%s The remote devce is EDR but does not support 3 Mbps", __func__);
      if (min_mtu > MAX_2MBPS_AVDTP_MTU) {
        min_mtu = MAX_2MBPS_AVDTP_MTU;
        APPL_TRACE_WARNING("%s Restricting AVDTP MTU size to %d", __func__, min_mtu);
      }
    }
    APPL_TRACE_DEBUG("%s updating peer MTU to %d for index %d",
                                    __func__, min_mtu, index);
  }

  p_peer_params->peer_mtu = min_mtu;
  p_peer_params->is_peer_edr = btif_av_is_peer_edr();
  p_peer_params->peer_supports_3mbps = btif_av_peer_supports_3mbps();

  /* Protect access to bta_av_co_cb.codec_config */
  mutex_global_unlock();
}

const tA2DP_ENCODER_INTERFACE* bta_av_co_get_encoder_interface(void) {
  /* Protect access to bta_av_co_cb.codec_config */
  mutex_global_lock();

  const tA2DP_ENCODER_INTERFACE* encoder_interface =
      A2DP_GetEncoderInterface(bta_av_co_cb.codec_config);

  /* Protect access to bta_av_co_cb.codec_config */
  mutex_global_unlock();

  return encoder_interface;
}

bool bta_av_co_set_codec_user_config(
    const btav_a2dp_codec_config_t& codec_user_config,
    const RawAddress& bd_addr) {
  uint8_t result_codec_config[AVDT_CODEC_SIZE];
  RawAddress bt_addr = bd_addr;
  const tBTA_AV_CO_SINK* p_sink = nullptr;
  bool restart_input = false;
  bool restart_output = false;
  bool config_updated = false;
  bool success = true;
  tBTA_AV_HNDL hndl = btif_av_get_reconfig_dev_hndl();
  // Find the peer that is currently open
  tBTA_AV_CO_PEER* p_peer = nullptr;
  if (hndl > 0)
    p_peer = bta_av_co_get_peer(hndl);
  else {
    for (size_t i = 0; i < BTA_AV_CO_NUM_ELEMENTS(bta_av_co_cb.peers); i++) {
      tBTA_AV_CO_PEER* p_peer_tmp = &bta_av_co_cb.peers[i];
      if (p_peer_tmp->addr == bd_addr) {
        p_peer = p_peer_tmp;
        break;
      }
    }
  }
  if (p_peer == nullptr) {
    APPL_TRACE_ERROR("%s: no open peer to configure", __func__);
    success = false;
    goto done;
  }

  if(codec_user_config.codec_type == BTAV_A2DP_CODEC_INDEX_SOURCE_APTX_ADAPTIVE) {
    if (codec_user_config.codec_specific_4 > 0 ) {
      //ENCODER_BTSCAN_AND_BATTERY_LEVEL_MASK is ORing of
      //Battery level indication(0x0000-0x000F) and BTcontrol(0x8000)
      const uint16_t ENCODER_MODE_MASK = 0x3000;
      uint16_t encoder_mode = codec_user_config.codec_specific_4 & ENCODER_MODE_MASK;
      if (encoder_mode == APTX_HQ || encoder_mode == APTX_LL) {
        APPL_TRACE_DEBUG("%s: Updating Encoder Mode to: %x", __func__, encoder_mode);
        BTA_AvUpdateEncoderMode(encoder_mode);
      }
      if(codec_user_config.codec_specific_4 > 0) {
        APPL_TRACE_DEBUG("%s: Updating Extended Aptx Data: %x", __func__, codec_user_config.codec_specific_4);
        BTA_AvUpdateAptxData(codec_user_config.codec_specific_4);
      }

      bool ble_scan_off = ((codec_user_config.codec_specific_4 & APTX_SCAN_CONTROL_MASK) == APTX_SCAN_CONTROL_MASK);
      if(!ble_scan_off)
        return success;

      switch(codec_user_config.codec_specific_4 & APTX_MODE_MASK) {
        case APTX_ULL:
        case APTX_LL:
          APPL_TRACE_DEBUG("%s: Disabling BLE Scanning", __func__);
          btif_gatt_get_interface()->scanner->Scan(false);
          break;
      }
      return success;
    }
  }

  // Find the peer SEP codec to use
  if (codec_user_config.codec_type < BTAV_A2DP_CODEC_INDEX_MAX) {
    for (size_t index = 0; index < p_peer->num_sup_sinks; index++) {
      btav_a2dp_codec_index_t peer_codec_index =
          A2DP_SourceCodecIndex(p_peer->sinks[index].codec_caps);
      if (peer_codec_index != codec_user_config.codec_type) continue;
      if (!bta_av_co_audio_sink_supports_cp(&p_peer->sinks[index])) continue;
      p_sink = &p_peer->sinks[index];
      break;
    }
  } else {
    // Use the current sink codec
    p_sink = p_peer->p_sink;
  }
  if (p_sink == nullptr) {
    APPL_TRACE_ERROR("%s: cannot find peer SEP to configure for codec type %d",
                     __func__, codec_user_config.codec_type);
    //check whether all remote supported SEPs, Get_caps done or not.
    //So that we need to decide whether we need to trigger this set_codec_user_config
    // path, by posting BTIF_AV_SOURCE_CONFIG_UPDATED_EVT when DUT done all remote
    //support SEP SNK capabilities.
    if ((p_peer->num_rx_sinks != p_peer->num_sinks) &&
        (p_peer->num_sup_sinks != BTA_AV_CO_NUM_ELEMENTS(p_peer->sinks))) {
      APPL_TRACE_WARNING("%s: All peer's capabilities have not been retrieved",
                         __func__);
      p_peer->getcap_pending = true;
      saved_codec_user_config = codec_user_config;
    }
    success = false;
    goto done;
  }

  tA2DP_ENCODER_INIT_PEER_PARAMS peer_params;
  bta_av_co_get_peer_params(&peer_params);
  if ((p_peer->codecs == nullptr) || !p_peer->codecs->setCodecUserConfig(
          codec_user_config, &peer_params, p_sink->codec_caps,
          result_codec_config, &restart_input, &restart_output,
          &config_updated)) {
    success = false;
    goto done;
  }

  if (restart_output) {
    uint8_t num_protect = 0;
#if (BTA_AV_CO_CP_SCMS_T == TRUE)
    if (p_peer->cp_active) num_protect = AVDT_CP_INFO_LEN;
#endif

    p_sink = bta_av_co_audio_set_codec(p_peer);
    if (p_sink == NULL) {
      APPL_TRACE_ERROR("%s: cannot set up codec for the peer SINK", __func__);
      success = false;
      goto done;
    }
    // Don't call BTA_AvReconfig() prior to retrieving all peer's capabilities
    if ((p_peer->num_rx_sinks != p_peer->num_sinks) &&
        (p_peer->num_sup_sinks != BTA_AV_CO_NUM_ELEMENTS(p_peer->sinks))) {
      APPL_TRACE_WARNING("%s: not all peer's capabilities have been retrieved",
                         __func__);
      p_peer->rcfg_pend_getcap = true;
      success = false;
      goto done;
    }

    if (isDevUiReq) {
      p_peer->acp = false;
      isDevUiReq = false;
    }
    btif_av_set_reconfig_flag(hndl);
    uint8_t index = BTA_AV_CO_AUDIO_HNDL_TO_INDX(hndl);
    btif_av_clear_remote_start_timer(index);
    APPL_TRACE_DEBUG("%s: call BTA_AvReconfig(x%x)", __func__, p_peer->handle);
    BTA_AvReconfig(p_peer->handle, true, p_sink->sep_info_idx,
                   p_peer->codec_config, num_protect, bta_av_co_cp_scmst);
    p_peer->rcfg_done = true;
    p_peer->reconfig_needed = false;
  }

done:
  // NOTE: We uncoditionally send the upcall even if there is no change
  // or the user config failed. Thus, the caller would always know whether the
  // request succeeded or failed.
  // NOTE: Currently, the input is restarted by sending an upcall
  // and informing the Media Framework about the change.
  if (p_peer == nullptr) {
    bt_addr = RawAddress::kAny;
    btif_dispatch_sm_event(BTIF_AV_SOURCE_CONFIG_UPDATED_EVT, (void *)bt_addr.address, sizeof(RawAddress));
    APPL_TRACE_DEBUG("%s BDA: %s", __func__, bt_addr.ToString().c_str());
  } else {
    btif_dispatch_sm_event(BTIF_AV_SOURCE_CONFIG_UPDATED_EVT, (void *)p_peer->addr.address,
                         sizeof(RawAddress));
    APPL_TRACE_DEBUG("%s BDA: %s", __func__, p_peer->addr.ToString().c_str());
  }
  if (!success || !restart_output) {
    APPL_TRACE_DEBUG("%s:reseting codec reconfig flag",__func__);
    btif_av_reset_codec_reconfig_flag(bt_addr);
  }
  return success;
}

// Sets the Over-The-Air preferred codec configuration.
// The OTA prefered codec configuration is ignored if the current
// codec configuration contains explicit user configuration, or if the
// codec configuration for the same codec contains explicit user
// configuration.
// |p_peer| is the peer device that sent the OTA codec configuration.
// |p_ota_codec_config| contains the received OTA A2DP codec configuration
// from the remote peer. Note: this is not the peer codec capability,
// but the codec configuration that the peer would like to use.
// |num_protect| is the number of content protection methods to use.
// |p_protect_info| contains the content protection information to use.
// If there is a change in the encoder configuration tht requires restarting
// of the A2DP connection, flag |p_restart_output| is set to true.
// Returns true on success, otherwise false.
static bool bta_av_co_set_codec_ota_config(tBTA_AV_CO_PEER* p_peer,
                                           const uint8_t* p_ota_codec_config,
                                           uint8_t num_protect,
                                           const uint8_t* p_protect_info,
                                           bool* p_restart_output) {
  uint8_t result_codec_config[AVDT_CODEC_SIZE];
  bool restart_input = false;
  bool restart_output = false;
  bool config_updated = false;

  APPL_TRACE_DEBUG("%s", __func__);
  A2DP_DumpCodecInfo(p_ota_codec_config);

  *p_restart_output = false;

  // Find the peer SEP codec to use
  btav_a2dp_codec_index_t ota_codec_index =
      A2DP_SourceCodecIndex(p_ota_codec_config);
  if (ota_codec_index == BTAV_A2DP_CODEC_INDEX_MAX) {
    APPL_TRACE_WARNING("%s: invalid peer codec config", __func__);
    return false;
  }
  const tBTA_AV_CO_SINK* p_sink = nullptr;
  for (size_t index = 0; index < p_peer->num_sup_sinks; index++) {
    btav_a2dp_codec_index_t peer_codec_index =
        A2DP_SourceCodecIndex(p_peer->sinks[index].codec_caps);
    if (peer_codec_index != ota_codec_index) continue;
    if (!bta_av_co_audio_sink_supports_cp(&p_peer->sinks[index])) continue;
    p_sink = &p_peer->sinks[index];
    break;
  }
  if ((p_peer->num_sup_sinks > 0) && (p_sink == nullptr)) {
    // There are no peer SEPs if we didn't do the discovery procedure yet.
    // We have all the information we need from the peer, so we can
    // proceed with the OTA codec configuration.
    APPL_TRACE_ERROR("%s: cannot find peer SEP to configure", __func__);
    return false;
  }

  tA2DP_ENCODER_INIT_PEER_PARAMS peer_params;
  bta_av_co_get_peer_params(&peer_params);
  if ((p_peer->codecs == nullptr) || !p_peer->codecs->setCodecOtaConfig(
          p_ota_codec_config, &peer_params, result_codec_config, &restart_input,
          &restart_output, &config_updated)) {
    APPL_TRACE_ERROR("%s: cannot set OTA config", __func__);
    return false;
  }

  APPL_TRACE_DEBUG("%s: restart_output:%d, restart_input:%d, config_updated:%d",
                       __func__, restart_output, restart_input, config_updated);
  if (restart_output) {
    APPL_TRACE_DEBUG("%s: restart output", __func__);
    A2DP_DumpCodecInfo(result_codec_config);

    *p_restart_output = true;
    p_peer->p_sink = p_sink;
    bta_av_co_save_new_codec_config(p_peer, result_codec_config, num_protect,
                                    p_protect_info);
  }

  if (restart_input || config_updated) {
    // NOTE: Currently, the input is restarted by sending an upcall
    // and informing the Media Framework about the change.
    btif_dispatch_sm_event(BTIF_AV_SOURCE_CONFIG_UPDATED_EVT, (void *)p_peer->addr.address,
                           sizeof(RawAddress));
    APPL_TRACE_DEBUG("%s BDA:%s", __func__, p_peer->addr.ToString().c_str());
  }

  return true;
}

bool bta_av_co_set_codec_audio_config(
    const btav_a2dp_codec_config_t& codec_audio_config) {
  uint8_t result_codec_config[AVDT_CODEC_SIZE];
  bool restart_output = false;
  bool config_updated = false;

  // Find the peer that is currently Active.
  tBTA_AV_CO_PEER* p_peer = nullptr;
  p_peer = bta_av_co_get_active_peer();
  if (p_peer == nullptr) {
    APPL_TRACE_ERROR("%s: no active peer to configure", __func__);
    return false;
  }

  // Use the current sink codec
  const tBTA_AV_CO_SINK* p_sink = p_peer->p_sink;
  if (p_sink == nullptr) {
    APPL_TRACE_ERROR("%s: cannot find peer SEP to configure", __func__);
    return false;
  }

  tA2DP_ENCODER_INIT_PEER_PARAMS peer_params;
  bta_av_co_get_peer_params(&peer_params);
  if ((p_peer->codecs == nullptr) || !p_peer->codecs->setCodecAudioConfig(
          codec_audio_config, &peer_params, p_sink->codec_caps,
          result_codec_config, &restart_output, &config_updated)) {
    return false;
  }

  if (restart_output) {
    uint8_t num_protect = 0;
#if (BTA_AV_CO_CP_SCMS_T == TRUE)
    if (p_peer->cp_active) num_protect = AVDT_CP_INFO_LEN;
#endif

    bta_av_co_save_new_codec_config(p_peer, result_codec_config,
                                    p_sink->num_protect, p_sink->protect_info);

    // Don't call BTA_AvReconfig() prior to retrieving all peer's capabilities
    if ((p_peer->num_rx_sinks != p_peer->num_sinks) &&
        (p_peer->num_sup_sinks != BTA_AV_CO_NUM_ELEMENTS(p_peer->sinks))) {
      APPL_TRACE_WARNING("%s: not all peer's capabilities have been retrieved",
                         __func__);
    } else {
      btif_av_set_reconfig_flag(p_peer->handle);
      APPL_TRACE_DEBUG("%s: call BTA_AvReconfig(x%x)", __func__,
                       p_peer->handle);
      BTA_AvReconfig(p_peer->handle, true, p_sink->sep_info_idx,
                     p_peer->codec_config, num_protect, bta_av_co_cp_scmst);
      p_peer->rcfg_done = true;
      p_peer->reconfig_needed = false;
    }
  }

  if (config_updated) {
    // NOTE: Currently, the input is restarted by sending an upcall
    // and informing the Media Framework about the change.
    btif_dispatch_sm_event(BTIF_AV_SOURCE_CONFIG_UPDATED_EVT, (void *)p_peer->addr.address,
                           sizeof(RawAddress));
    APPL_TRACE_DEBUG("%s BDA: %s", __func__, p_peer->addr.ToString().c_str());
  }

  return true;
}

A2dpCodecs* bta_av_get_a2dp_codecs(void) {
  tBTA_AV_CO_PEER* p_active_peer;
  p_active_peer = bta_av_co_get_active_peer();
  if (p_active_peer != NULL) {
    return p_active_peer->codecs;
  }
  return nullptr;
}

A2dpCodecs* bta_av_get_peer_a2dp_codecs(const RawAddress& bd_addr) {
  tBTA_AV_CO_PEER* p_peer;
  size_t i;
  for (i = 0; i < BTA_AV_CO_NUM_ELEMENTS(bta_av_co_cb.peers); i++) {
    p_peer = &bta_av_co_cb.peers[i];
    if (p_peer->addr == bd_addr) break;
  }
  APPL_TRACE_ERROR("%s() i = %d", __func__, i);
  if (p_peer != NULL) {
    return p_peer->codecs;
  }
  return nullptr;
}

A2dpCodecConfig* bta_av_get_a2dp_current_codec(void) {
  A2dpCodecConfig* current_codec;
  tBTA_AV_CO_PEER* p_active_peer;

  p_active_peer = bta_av_co_get_active_peer();
  if (p_active_peer == NULL) {
    p_active_peer = &bta_av_co_cb.peers[0];
  }

  mutex_global_lock();
  if (p_active_peer != NULL && p_active_peer->codecs != nullptr) {
    current_codec = p_active_peer->codecs->getCurrentCodecConfig();
  } else {
    mutex_global_unlock();
    return nullptr;
  }
  mutex_global_unlock();

  return current_codec;
}

bt_status_t bta_av_set_a2dp_current_codec(tBTA_AV_HNDL hndl) {
  tBTA_AV_CO_PEER* p_peer;
  tBTA_AV_CO_SINK* p_sink;
  bt_status_t  status = BT_STATUS_SUCCESS;

  mutex_global_lock();
  p_peer = bta_av_co_get_peer(hndl);
  if (p_peer) {
    if (!bta_av_co_set_active_peer(p_peer->addr)) {
      BTIF_TRACE_WARNING("%s: unable to set active peer",__func__);
    }
    p_sink = bta_av_co_audio_set_codec(p_peer);
    if (p_sink == NULL) {
      APPL_TRACE_ERROR("%s() can not setup codec for the peer", __func__);
      status = BT_STATUS_FAIL;
    }
  } else {
    APPL_TRACE_ERROR("%s() peer not found", __func__);
    status = BT_STATUS_FAIL;
  }
  mutex_global_unlock();

  return status;
}

bool bta_av_co_is_scrambling_enabled() {
  uint8_t no_of_freqs = 0;
  uint8_t *freqs = NULL;

  freqs = controller_get_interface()->get_scrambling_supported_freqs(&no_of_freqs);

  if(no_of_freqs == 0) {
    return false;
  }
  return true;
}

bool bta_av_co_is_44p1kFreq_enabled() {
  uint8_t add_on_features_size = 0;
  const bt_device_features_t * add_on_features_list = NULL;

  add_on_features_list = controller_get_interface()->get_add_on_features(&add_on_features_size);
  if (add_on_features_size == 0) {
    BTIF_TRACE_WARNING(
        "BT controller doesn't add on features");
    return false;
  }

  if (add_on_features_list != NULL) {
    if (HCI_SPLIT_A2DP_44P1KHZ_SAMPLE_FREQ(add_on_features_list->as_array)) {
      return true;
    }
  }
  return false;
}

void bta_av_co_init(
    const std::vector<btav_a2dp_codec_config_t>& codec_priorities,
    std::vector<btav_a2dp_codec_config_t>& offload_enabled_codecs_config) {
  APPL_TRACE_DEBUG("%s", __func__);
  RawAddress bt_addr;
  const char *a2dp_ofload_cap = controller_get_interface()->get_a2dp_offload_cap();
  tBTA_AV_CO_PEER* p_peer;
  /* Protect access to bta_av_co_cb.codec_config */
  mutex_global_lock();
  /* Reset the control block */
  bta_av_co_cb.reset();

#if (BTA_AV_CO_CP_SCMS_T == TRUE)
  bta_av_co_cp_set_flag(AVDT_CP_SCMS_COPY_NEVER);
#else
  bta_av_co_cp_set_flag(AVDT_CP_SCMS_COPY_FREE);
#endif

/* SPLITA2DP */
  bool a2dp_offload = btif_av_is_split_a2dp_enabled();
  bool isScramblingSupported = bta_av_co_is_scrambling_enabled();
  bool is44p1kFreqSupported = bta_av_co_is_44p1kFreq_enabled();

  A2DP_SetOffloadStatus(a2dp_offload, a2dp_ofload_cap, isScramblingSupported,
                       is44p1kFreqSupported, offload_enabled_codecs_config);

/* SPLITA2DP */
  bool isMcastSupported = btif_av_is_multicast_supported();
  for (size_t i = 0; i < BTA_AV_CO_NUM_ELEMENTS(bta_av_co_cb.peers); i++) {
    p_peer = &bta_av_co_cb.peers[i];
    if (p_peer != NULL && p_peer->codecs == NULL)
      p_peer->codecs = new A2dpCodecs(codec_priorities);

    if (p_peer->codecs != nullptr)
      p_peer->codecs->init(isMcastSupported);

    p_peer->isIncoming = false;
  }
  A2DP_InitDefaultCodec(bta_av_co_cb.codec_config);
  bta_av_co_cb.default_codec_priorities = codec_priorities;
  mutex_global_unlock();

  // NOTE: Unconditionally dispatch the event to make sure a callback with
  // the most recent codec info is generated.
  bt_addr = RawAddress::kAny;
  btif_dispatch_sm_event(BTIF_AV_SOURCE_CONFIG_UPDATED_EVT, (void *)bt_addr.address, sizeof(RawAddress));
}


void bta_av_co_peer_init(
    const std::vector<btav_a2dp_codec_config_t>& codec_priorities, int index) {
  APPL_TRACE_DEBUG("%s", __func__);

  tBTA_AV_CO_PEER* p_peer;
  bool a2dp_offload = btif_av_is_split_a2dp_enabled();
  bool isMcastSupported = btif_av_is_multicast_supported();
  if (a2dp_offload) {
    isMcastSupported = false;
  }

  p_peer = &bta_av_co_cb.peers[index];
  if (p_peer != NULL &&  p_peer->codecs == NULL)
    p_peer->codecs = new A2dpCodecs(codec_priorities);

  if (p_peer->codecs != nullptr)
    p_peer->codecs->init(isMcastSupported);

  p_peer->isIncoming = false;
}
