/******************************************************************************
 *  Copyright (C) 2017, The Linux Foundation. All rights reserved.
 *  Not a Contribution.
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
 *  Copyright (C) 2011-2012 Broadcom Corporation
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
 *  This is the implementation of the API for the advanced audio/video (AV)
 *  subsystem of BTA, Broadcom's Bluetooth application layer for mobile
 *  phones.
 *
 ******************************************************************************/

#include <base/logging.h>

#include "bt_target.h"

#include <string.h>
#include "bt_common.h"
#include "bta_api.h"
#include "bta_av_api.h"
#include "bta_av_int.h"
#include "bta_sys.h"
#include "btif/include/btif_av.h"

#include "osi/include/allocator.h"

/*****************************************************************************
 *  Constants
 ****************************************************************************/

static const tBTA_SYS_REG bta_av_reg = {bta_av_hdl_event, BTA_AvDisable};

/*******************************************************************************
 *
 * Function         BTA_AvEnable
 *
 * Description      Enable the advanced audio/video service. When the enable
 *                  operation is complete the callback function will be
 *                  called with a BTA_AV_ENABLE_EVT. This function must
 *                  be called before other function in the AV API are
 *                  called.
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_AvEnable(tBTA_SEC sec_mask, tBTA_AV_FEAT features,
                  tBTA_AV_CBACK* p_cback) {
  APPL_TRACE_ERROR("%s:", __func__);
  tBTA_AV_API_ENABLE* p_buf =
      (tBTA_AV_API_ENABLE*)osi_malloc(sizeof(tBTA_AV_API_ENABLE));

  /* register with BTA system manager */
  bta_sys_register(BTA_ID_AV, &bta_av_reg);

  p_buf->hdr.event = BTA_AV_API_ENABLE_EVT;
  p_buf->p_cback = p_cback;
  p_buf->features = features;
  p_buf->sec_mask = sec_mask;

  bta_sys_sendmsg(p_buf);
}

/*******************************************************************************
 *
 * Function         BTA_AvDisable
 *
 * Description      Disable the advanced audio/video service.
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_AvDisable(void) {
  if (!bta_sys_is_register(BTA_ID_AV)) {
    APPL_TRACE_ERROR("BTA AV is already disabled, ignoring ...");
    return;
  }

  BT_HDR* p_buf = (BT_HDR*)osi_malloc(sizeof(BT_HDR));

  bta_sys_deregister(BTA_ID_AV);
  p_buf->event = BTA_AV_API_DISABLE_EVT;

  bta_sys_sendmsg(p_buf);
}

/*******************************************************************************
 *
 * Function         BTA_AvRegister
 *
 * Description      Register the audio or video service to stack. When the
 *                  operation is complete the callback function will be
 *                  called with a BTA_AV_REGISTER_EVT. This function must
 *                  be called before AVDT stream is open.
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_AvRegister(tBTA_AV_CHNL chnl, const char* p_service_name,
                    uint8_t app_id, tBTA_AV_SINK_DATA_CBACK* p_sink_data_cback,
                    uint16_t service_uuid) {
  tBTA_AV_API_REG* p_buf =
      (tBTA_AV_API_REG*)osi_malloc(sizeof(tBTA_AV_API_REG));

  p_buf->hdr.layer_specific = chnl;
  p_buf->hdr.event = BTA_AV_API_REGISTER_EVT;
  if (p_service_name)
    strlcpy(p_buf->p_service_name, p_service_name, BTA_SERVICE_NAME_LEN + 1);
  else
    p_buf->p_service_name[0] = 0;
  p_buf->app_id = app_id;
  p_buf->p_app_sink_data_cback = p_sink_data_cback;
  p_buf->service_uuid = service_uuid;

  bta_sys_sendmsg(p_buf);
}

/*******************************************************************************
 *
 * Function         BTA_AvDeregister
 *
 * Description      Deregister the audio or video service
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_AvDeregister(tBTA_AV_HNDL hndl) {
  BT_HDR* p_buf = (BT_HDR*)osi_malloc(sizeof(BT_HDR));

  p_buf->layer_specific = hndl;
  p_buf->event = BTA_AV_API_DEREGISTER_EVT;

  bta_sys_sendmsg(p_buf);
}

/*******************************************************************************
 *
 * Function         BTA_AvOpen
 *
 * Description      Opens an advanced audio/video connection to a peer device.
 *                  When connection is open callback function is called
 *                  with a BTA_AV_OPEN_EVT.
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_AvOpen(const RawAddress& bd_addr, tBTA_AV_HNDL handle, bool use_rc,
                tBTA_SEC sec_mask, uint16_t uuid) {
  tBTA_AV_API_OPEN* p_buf =
      (tBTA_AV_API_OPEN*)osi_malloc(sizeof(tBTA_AV_API_OPEN));

  p_buf->hdr.event = BTA_AV_API_OPEN_EVT;
  p_buf->hdr.layer_specific = handle;
  p_buf->bd_addr = bd_addr;
  p_buf->use_rc = use_rc;
  p_buf->sec_mask = sec_mask;
  p_buf->switch_res = BTA_AV_RS_NONE;
  p_buf->uuid = uuid;

  bta_sys_sendmsg(p_buf);
}

/*******************************************************************************
 *
 * Function         BTA_AvClose
 *
 * Description      Close the current streams.
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_AvClose(tBTA_AV_HNDL handle) {
  BT_HDR* p_buf = (BT_HDR*)osi_malloc(sizeof(BT_HDR));

  p_buf->event = BTA_AV_API_CLOSE_EVT;
  p_buf->layer_specific = handle;

  bta_sys_sendmsg(p_buf);
}

/*******************************************************************************
 *
 * Function         BTA_AvDisconnect
 *
 * Description      Close the connection to the address.
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_AvDisconnect(const RawAddress& bd_addr) {
  tBTA_AV_API_DISCNT* p_buf =
      (tBTA_AV_API_DISCNT*)osi_malloc(sizeof(tBTA_AV_API_DISCNT));

  p_buf->hdr.event = BTA_AV_API_DISCONNECT_EVT;
  p_buf->bd_addr = bd_addr;

  bta_sys_sendmsg(p_buf);
}

/*******************************************************************************
 *
 * Function         BTA_AvStart
 *
 * Description      Start audio/video stream data transfer.
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_AvStart(tBTA_AV_HNDL handle) {
  BT_HDR* p_buf = (BT_HDR*)osi_malloc(sizeof(BT_HDR));

  p_buf->event = BTA_AV_API_START_EVT;
  p_buf->layer_specific = handle;

  bta_sys_sendmsg(p_buf);
}

/*******************************************************************************
 *
 * Function         BTA_AvOffloadStart
 *
 * Description      Start a2dp audio offloading.
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_AvOffloadStart(tBTA_AV_HNDL hndl, bool do_scrambling) {
  tBTA_AV_API_OFFLOAD_START* p_buf =
     (tBTA_AV_API_OFFLOAD_START*)osi_malloc(sizeof(tBTA_AV_API_OFFLOAD_START));

  p_buf->hdr.event = BTA_AV_API_OFFLOAD_START_EVT;
  p_buf->hdr.layer_specific = hndl;
  p_buf->do_scrambling = do_scrambling;

  bta_sys_sendmsg(p_buf);
}

/*******************************************************************************
 *
 * Function         BTA_AvOffloadStartRsp
 *
 * Description      Response from vendor lib for A2DP Offload Start request.
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_AvOffloadStartRsp(tBTA_AV_HNDL hndl, tBTA_AV_STATUS status) {
  tBTA_AV_API_STATUS_RSP* p_buf =
      (tBTA_AV_API_STATUS_RSP*)osi_malloc(sizeof(tBTA_AV_API_STATUS_RSP));

  p_buf->hdr.event = BTA_AV_API_OFFLOAD_START_RSP_EVT;
  p_buf->hdr.layer_specific = hndl;
  p_buf->status = status;

  bta_sys_sendmsg(p_buf);
}

/*******************************************************************************
 *
 * Function         BTA_AvStop
 *
 * Description      Stop audio/video stream data transfer.
 *                  If suspend is true, this function sends AVDT suspend signal
 *                  to the connected peer(s).
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_AvStop(bool suspend, tBTA_AV_HNDL handle) {
  tBTA_AV_API_STOP* p_buf =
      (tBTA_AV_API_STOP*)osi_malloc(sizeof(tBTA_AV_API_STOP));

  p_buf->hdr.event = BTA_AV_API_STOP_EVT;
  p_buf->flush = true;
  p_buf->suspend = suspend;
  p_buf->reconfig_stop = false;
  p_buf->hdr.layer_specific = handle;

  bta_sys_sendmsg(p_buf);
}

/*******************************************************************************
 *
 * Function         BTA_AvEnableMultiCast
 *
 * Description      Enable/Disable Avdtp MultiCast
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_AvEnableMultiCast(bool state, tBTA_AV_HNDL handle)
{
  tBTA_AV_ENABLE_MULTICAST  *p_buf;

  if ((p_buf = (tBTA_AV_ENABLE_MULTICAST *)osi_malloc(sizeof(tBTA_AV_ENABLE_MULTICAST))) != NULL) {
    p_buf->hdr.event = BTA_AV_ENABLE_MULTICAST_EVT;
    p_buf->hdr.layer_specific   = handle;
    p_buf->is_multicast_enabled = state;
    bta_sys_sendmsg(p_buf);
  }
}

#if (TWS_ENABLED == TRUE)
#if (TWS_STATE_ENABLED == TRUE)
void BTA_AVSetEarbudState(uint8_t state, tBTA_AV_HNDL handle)
{
  APPL_TRACE_DEBUG("%s",__func__);
  tBTA_AV_TWS_SET_EARBUD_STATE *p_buf;

  if ((p_buf = (tBTA_AV_TWS_SET_EARBUD_STATE *)osi_malloc(sizeof(tBTA_AV_TWS_SET_EARBUD_STATE))) != NULL) {
    p_buf->hdr.event = BTA_AV_SET_EARBUD_STATE_EVT;
    p_buf->hdr.layer_specific = handle;
    p_buf->eb_state = state;
    bta_sys_sendmsg(p_buf);
  }
}
#endif
void BTA_AVSetEarbudRole(uint8_t role, tBTA_AV_HNDL handle)
{
  APPL_TRACE_DEBUG("%s",__func__);
  tBTA_AV_TWS_SET_EARBUD_ROLE *p_buf;

  if ((p_buf = (tBTA_AV_TWS_SET_EARBUD_ROLE *)osi_malloc(sizeof(tBTA_AV_TWS_SET_EARBUD_ROLE))) != NULL) {
    p_buf->hdr.event = BTA_AV_SET_EARBUD_ROLE_EVT;
    p_buf->hdr.layer_specific = handle;
    p_buf->chn_mode = role;
    bta_sys_sendmsg(p_buf);
  }
}
void BTA_AvUpdateTWSDevice(bool state, tBTA_AV_HNDL handle)
{
  tBTA_AV_SET_TWS_DEVICE *p_buf;

  if ((p_buf = (tBTA_AV_SET_TWS_DEVICE *)osi_malloc(sizeof(tBTA_AV_SET_TWS_DEVICE))) != NULL) {
    p_buf->hdr.event = BTA_AV_SET_TWS_DEVICE_EVT;
    p_buf->hdr.layer_specific = handle;
    p_buf->is_tws_device = state;
    bta_sys_sendmsg(p_buf);
  }
}
#endif
/*******************************************************************************
 *
 * Function         BTA_AvUpdateMaxAVClient
 *
 * Description      Update max av connections supported simultaneously
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_AvUpdateMaxAVClient(uint8_t max_clients)
{
  tBTA_AV_MAX_CLIENT *p_buf;

  if ((p_buf = (tBTA_AV_MAX_CLIENT *) osi_malloc(sizeof(tBTA_AV_MAX_CLIENT))) != NULL) {
    p_buf->hdr.event = BTA_AV_UPDATE_MAX_AV_CLIENTS_EVT;
    p_buf->max_clients = max_clients;
    bta_sys_sendmsg(p_buf);
  }
}

/*******************************************************************************
 *
 * Function         BTA_AvReconfig
 *
 * Description      Reconfigure the audio/video stream.
 *                  If suspend is true, this function tries the
 *                  suspend/reconfigure procedure first.
 *                  If suspend is false or when suspend/reconfigure fails,
 *                  this function closes and re-opens the AVDT connection.
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_AvReconfig(tBTA_AV_HNDL hndl, bool suspend, uint8_t sep_info_idx,
                    uint8_t* p_codec_info, uint8_t num_protect,
                    const uint8_t* p_protect_info) {
  tBTA_AV_API_RCFG* p_buf =
      (tBTA_AV_API_RCFG*)osi_malloc(sizeof(tBTA_AV_API_RCFG) + num_protect);

  p_buf->hdr.layer_specific = hndl;
  p_buf->hdr.event = BTA_AV_API_RECONFIG_EVT;
  p_buf->num_protect = num_protect;
  p_buf->suspend = suspend;
  p_buf->sep_info_idx = sep_info_idx;
  p_buf->p_protect_info = (uint8_t*)(p_buf + 1);
  memcpy(p_buf->codec_info, p_codec_info, AVDT_CODEC_SIZE);
  memcpy(p_buf->p_protect_info, p_protect_info, num_protect);

  bta_sys_sendmsg(p_buf);
}

/*******************************************************************************
 *
 * Function         BTA_AvUpdateEncoderMode
 *
 * Description      Update current encoder mode to SoC by sending
 *                  Vendor Specific Command. It is called based on
 *                  Encoder feedback of Low Latency and High Quality
 *                  modes
 * Returns          void
 *
 ******************************************************************************/
void BTA_AvUpdateEncoderMode(uint16_t enc_mode) {
  tBTA_AV_ENC_MODE* p_buf =
      (tBTA_AV_ENC_MODE*)osi_malloc(sizeof(tBTA_AV_ENC_MODE));

  p_buf->hdr.event = BTA_AV_UPDATE_ENCODER_MODE_EVT;
  p_buf->enc_mode = enc_mode;

  bta_sys_sendmsg(p_buf);
}

void BTA_AvUpdateAptxData(uint32_t data) {
  bool battery_info = (data & APTX_BATTERY_INFO);
  uint16_t aptx_mode = (uint16_t)(data & APTX_MODE_MASK);
  if(battery_info) {
    tBTA_AV_APTX_DATA* p_buf_battery =
        (tBTA_AV_APTX_DATA*)osi_malloc(sizeof(tBTA_AV_APTX_DATA));
    p_buf_battery->type = 4;
    p_buf_battery->data = (uint16_t)data;
    p_buf_battery->hdr.event = BTA_AV_UPDATE_APTX_DATA_EVT;
    bta_sys_sendmsg(p_buf_battery);
  }
  if(aptx_mode == APTX_ULL || aptx_mode == APTX_ULL_S) {
    tBTA_AV_APTX_DATA* p_buf_ull =
        (tBTA_AV_APTX_DATA*)osi_malloc(sizeof(tBTA_AV_APTX_DATA));
    p_buf_ull->type = 3;
    p_buf_ull->data = 1;
    p_buf_ull->hdr.event = BTA_AV_UPDATE_APTX_DATA_EVT;
    bta_sys_sendmsg(p_buf_ull);
  }
  if(aptx_mode == APTX_HQ || aptx_mode == APTX_LL) {
    tBTA_AV_APTX_DATA* p_buf_ull =
        (tBTA_AV_APTX_DATA*)osi_malloc(sizeof(tBTA_AV_APTX_DATA));
    p_buf_ull->type = 3;
    p_buf_ull->data = 0;
    p_buf_ull->hdr.event = BTA_AV_UPDATE_APTX_DATA_EVT;
    bta_sys_sendmsg(p_buf_ull);
  }
}

/*******************************************************************************
 *
 * Function         BTA_AvProtectReq
 *
 * Description      Send a content protection request.  This function can only
 *                  be used if AV is enabled with feature BTA_AV_FEAT_PROTECT.
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_AvProtectReq(tBTA_AV_HNDL hndl, uint8_t* p_data, uint16_t len) {
  tBTA_AV_API_PROTECT_REQ* p_buf = (tBTA_AV_API_PROTECT_REQ*)osi_malloc(
      sizeof(tBTA_AV_API_PROTECT_REQ) + len);

  p_buf->hdr.layer_specific = hndl;
  p_buf->hdr.event = BTA_AV_API_PROTECT_REQ_EVT;
  p_buf->len = len;
  if (p_data == NULL) {
    p_buf->p_data = NULL;
  } else {
    p_buf->p_data = (uint8_t*)(p_buf + 1);
    memcpy(p_buf->p_data, p_data, len);
  }

  bta_sys_sendmsg(p_buf);
}

/*******************************************************************************
 *
 * Function         BTA_AvProtectRsp
 *
 * Description      Send a content protection response.  This function must
 *                  be called if a BTA_AV_PROTECT_REQ_EVT is received.
 *                  This function can only be used if AV is enabled with
 *                  feature BTA_AV_FEAT_PROTECT.
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_AvProtectRsp(tBTA_AV_HNDL hndl, uint8_t error_code, uint8_t* p_data,
                      uint16_t len) {
  tBTA_AV_API_PROTECT_RSP* p_buf = (tBTA_AV_API_PROTECT_RSP*)osi_malloc(
      sizeof(tBTA_AV_API_PROTECT_RSP) + len);

  p_buf->hdr.layer_specific = hndl;
  p_buf->hdr.event = BTA_AV_API_PROTECT_RSP_EVT;
  p_buf->len = len;
  p_buf->error_code = error_code;
  if (p_data == NULL) {
    p_buf->p_data = NULL;
  } else {
    p_buf->p_data = (uint8_t*)(p_buf + 1);
    memcpy(p_buf->p_data, p_data, len);
  }

  bta_sys_sendmsg(p_buf);
}

/*******************************************************************************
 *
 * Function         BTA_AvRemoteCmd
 *
 * Description      Send a remote control command.  This function can only
 *                  be used if AV is enabled with feature BTA_AV_FEAT_RCCT.
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_AvRemoteCmd(uint8_t rc_handle, uint8_t label, tBTA_AV_RC rc_id,
                     tBTA_AV_STATE key_state) {
  tBTA_AV_API_REMOTE_CMD* p_buf =
      (tBTA_AV_API_REMOTE_CMD*)osi_malloc(sizeof(tBTA_AV_API_REMOTE_CMD));

  p_buf->hdr.event = BTA_AV_API_REMOTE_CMD_EVT;
  p_buf->hdr.layer_specific = rc_handle;
  p_buf->msg.op_id = rc_id;
  p_buf->msg.state = key_state;
  p_buf->msg.p_pass_data = NULL;
  p_buf->msg.pass_len = 0;
  p_buf->label = label;

  bta_sys_sendmsg(p_buf);
}

/*******************************************************************************
 *
 * Function         BTA_AvRemoteVendorUniqueCmd
 *
 * Description      Send a remote control command with Vendor Unique rc_id.
 *                  This function can only be used if AV is enabled with
 *                  feature BTA_AV_FEAT_RCCT.
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_AvRemoteVendorUniqueCmd(uint8_t rc_handle, uint8_t label,
                                 tBTA_AV_STATE key_state, uint8_t* p_msg,
                                 uint8_t buf_len) {
  tBTA_AV_API_REMOTE_CMD* p_buf = (tBTA_AV_API_REMOTE_CMD*)osi_malloc(
      sizeof(tBTA_AV_API_REMOTE_CMD) + buf_len);

  p_buf->label = label;
  p_buf->hdr.event = BTA_AV_API_REMOTE_CMD_EVT;
  p_buf->hdr.layer_specific = rc_handle;
  p_buf->msg.op_id = AVRC_ID_VENDOR;
  p_buf->msg.state = key_state;
  p_buf->msg.pass_len = buf_len;
  if (p_msg == NULL) {
    p_buf->msg.p_pass_data = NULL;
  } else {
    p_buf->msg.p_pass_data = (uint8_t*)(p_buf + 1);
    memcpy(p_buf->msg.p_pass_data, p_msg, buf_len);
  }
  bta_sys_sendmsg(p_buf);
}

/*******************************************************************************
 *
 * Function         BTA_AvVendorCmd
 *
 * Description      Send a vendor dependent remote control command.  This
 *                  function can only be used if AV is enabled with feature
 *                  BTA_AV_FEAT_VENDOR.
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_AvVendorCmd(uint8_t rc_handle, uint8_t label, tBTA_AV_CODE cmd_code,
                     uint8_t* p_data, uint16_t len) {
  tBTA_AV_API_VENDOR* p_buf =
      (tBTA_AV_API_VENDOR*)osi_malloc(sizeof(tBTA_AV_API_VENDOR) + len);

  p_buf->hdr.event = BTA_AV_API_VENDOR_CMD_EVT;
  p_buf->hdr.layer_specific = rc_handle;
  p_buf->msg.hdr.ctype = cmd_code;
  p_buf->msg.hdr.subunit_type = AVRC_SUB_PANEL;
  p_buf->msg.hdr.subunit_id = 0;
  p_buf->msg.company_id = p_bta_av_cfg->company_id;
  p_buf->label = label;
  p_buf->msg.vendor_len = len;
  if (p_data == NULL) {
    p_buf->msg.p_vendor_data = NULL;
  } else {
    p_buf->msg.p_vendor_data = (uint8_t*)(p_buf + 1);
    memcpy(p_buf->msg.p_vendor_data, p_data, len);
  }

  bta_sys_sendmsg(p_buf);
}

/*******************************************************************************
 *
 * Function         BTA_AvVendorRsp
 *
 * Description      Send a vendor dependent remote control response.
 *                  This function must be called if a BTA_AV_VENDOR_CMD_EVT
 *                  is received. This function can only be used if AV is
 *                  enabled with feature BTA_AV_FEAT_VENDOR.
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_AvVendorRsp(uint8_t rc_handle, uint8_t label, tBTA_AV_CODE rsp_code,
                     uint8_t* p_data, uint16_t len, uint32_t company_id) {
  tBTA_AV_API_VENDOR* p_buf =
      (tBTA_AV_API_VENDOR*)osi_malloc(sizeof(tBTA_AV_API_VENDOR) + len);

  p_buf->hdr.event = BTA_AV_API_VENDOR_RSP_EVT;
  p_buf->hdr.layer_specific = rc_handle;
  p_buf->msg.hdr.ctype = rsp_code;
  p_buf->msg.hdr.subunit_type = AVRC_SUB_PANEL;
  p_buf->msg.hdr.subunit_id = 0;
  if (company_id)
    p_buf->msg.company_id = company_id;
  else
    p_buf->msg.company_id = p_bta_av_cfg->company_id;
  p_buf->label = label;
  p_buf->msg.vendor_len = len;
  if (p_data == NULL) {
    p_buf->msg.p_vendor_data = NULL;
  } else {
    p_buf->msg.p_vendor_data = (uint8_t*)(p_buf + 1);
    memcpy(p_buf->msg.p_vendor_data, p_data, len);
  }

  bta_sys_sendmsg(p_buf);
}

/*******************************************************************************
 *
 * Function         BTA_AvOpenRc
 *
 * Description      Open an AVRCP connection toward the device with the
 *                  specified handle
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_AvOpenRc(tBTA_AV_HNDL handle) {
  tBTA_AV_API_OPEN_RC* p_buf =
      (tBTA_AV_API_OPEN_RC*)osi_malloc(sizeof(tBTA_AV_API_OPEN_RC));

  p_buf->hdr.event = BTA_AV_API_RC_OPEN_EVT;
  p_buf->hdr.layer_specific = handle;

  bta_sys_sendmsg(p_buf);
}

/*******************************************************************************
 *
 * Function         BTA_AvCloseRc
 *
 * Description      Close an AVRCP connection
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_AvCloseRc(uint8_t rc_handle) {
  tBTA_AV_API_CLOSE_RC* p_buf =
      (tBTA_AV_API_CLOSE_RC*)osi_malloc(sizeof(tBTA_AV_API_CLOSE_RC));

  p_buf->hdr.event = BTA_AV_API_RC_CLOSE_EVT;
  p_buf->hdr.layer_specific = rc_handle;

  bta_sys_sendmsg(p_buf);
}

 /*******************************************************************************
  *
  * Function         BTA_AvBrowseActive
  *
  * Description      Set Active Browse AVRCP
  *
  * Returns          void
  ******************************************************************************/
void BTA_AvBrowseActive(uint8_t rc_handle, const RawAddress& bd_addr,
                        uint8_t browse_device_evt) {
  APPL_TRACE_DEBUG("%s: Send Browse Active msg",__func__);
  tBTA_AV_API_ACTIVE_BROWSE_RC* p_buf =
      (tBTA_AV_API_ACTIVE_BROWSE_RC*)osi_malloc(sizeof(tBTA_AV_API_ACTIVE_BROWSE_RC));

  p_buf->hdr.event = BTA_AV_BROWSE_ACTIVE_EVT;
  p_buf->hdr.layer_specific = rc_handle;
  p_buf->peer_addr = bd_addr;
  p_buf->browse_device_evt = browse_device_evt;

  bta_sys_sendmsg(p_buf);
}
/*******************************************************************************
 *
 * Function         BTA_AvMetaRsp
 *
 * Description      Send a Metadata/Advanced Control response. The message
 *                  contained in p_pkt can be composed with AVRC utility
 *                  functions.
 *                  This function can only be used if AV is enabled with feature
 *                  BTA_AV_FEAT_METADATA.
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_AvMetaRsp(uint8_t rc_handle, uint8_t label, tBTA_AV_CODE rsp_code,
                   BT_HDR* p_pkt) {
  tBTA_AV_API_META_RSP* p_buf =
      (tBTA_AV_API_META_RSP*)osi_malloc(sizeof(tBTA_AV_API_META_RSP));

  p_buf->hdr.event = BTA_AV_API_META_RSP_EVT;
  p_buf->hdr.layer_specific = rc_handle;
  p_buf->rsp_code = rsp_code;
  p_buf->p_pkt = p_pkt;
  p_buf->is_rsp = true;
  p_buf->label = label;

  bta_sys_sendmsg(p_buf);
}

/*******************************************************************************
 *
 * Function         BTA_AvMetaCmd
 *
 * Description      Send a Metadata/Advanced Control command. The message
*contained
 *                  in p_pkt can be composed with AVRC utility functions.
 *                  This function can only be used if AV is enabled with feature
 *                  BTA_AV_FEAT_METADATA.
 *                  This message is sent only when the peer supports the TG
*role.
*8                  The only command makes sense right now is the absolute
*volume command.
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_AvMetaCmd(uint8_t rc_handle, uint8_t label, tBTA_AV_CMD cmd_code,
                   BT_HDR* p_pkt) {
  tBTA_AV_API_META_RSP* p_buf =
      (tBTA_AV_API_META_RSP*)osi_malloc(sizeof(tBTA_AV_API_META_RSP));

  p_buf->hdr.event = BTA_AV_API_META_RSP_EVT;
  p_buf->hdr.layer_specific = rc_handle;
  p_buf->p_pkt = p_pkt;
  p_buf->rsp_code = cmd_code;
  p_buf->is_rsp = false;
  p_buf->label = label;

  bta_sys_sendmsg(p_buf);
}

void bta_av_sniff_enable(bool policy_enable, const RawAddress& peer_addr) {
  APPL_TRACE_DEBUG("%s: sniff policy: %d, peer_addr: %s: ",
                       __func__, policy_enable, peer_addr.ToString().c_str());
  uint8_t policy = HCI_ENABLE_SNIFF_MODE;

  if (policy_enable)
    bta_sys_set_policy(BTA_ID_AV, policy, peer_addr);
  else
    bta_sys_clear_policy(BTA_ID_AV, policy, peer_addr);
}
