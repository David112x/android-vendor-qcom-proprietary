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
 *  Copyright (C) 2004-2016 Broadcom Corporation
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
 *  This file contains action functions for advanced audio/video main state
 *  machine.
 *
 ******************************************************************************/

#define LOG_TAG "bt_bta_av"
#include "bt_target.h"

#include <base/logging.h>
#include <string.h>
#include <vector>
#include <algorithm>

#include "bta_av_api.h"
#include "bta_av_int.h"
#include "l2c_api.h"
#include "log/log.h"
#include "osi/include/list.h"
#include "osi/include/log.h"
#include "osi/include/osi.h"
#include "osi/include/properties.h"
#include "utl.h"
#include <errno.h>
#include <hardware/vendor.h>
#include "device/include/interop.h"
#include "device/include/profile_config.h"
#include "device/include/device_iot_config.h"
#include "controller.h"
#include "btif/include/btif_av.h"
#include "stack/sdp/sdpint.h"

#if (BTA_AR_INCLUDED == TRUE)
#include "bta_ar_api.h"
#endif

/*****************************************************************************
 *  Constants
 ****************************************************************************/
/* the timeout to wait for open req after setconfig for incoming connections */
#ifndef BTA_AV_SIGNALLING_TIMEOUT_MS
#define BTA_AV_SIGNALLING_TIMEOUT_MS (8 * 1000) /* 8 seconds */
#endif

#ifndef BTA_AV_BROWSINIG_CHANNEL_INT_TIMEOUT_MS
#define BTA_AV_BROWSINIG_CHANNEL_INT_TIMEOUT_MS (1000) /* 1 second */
#endif

/* Time to wait for signalling from SNK when it is initiated from SNK. */
/* If not, we will start signalling from SRC. */
#ifndef BTA_AV_ACCEPT_SIGNALLING_TIMEOUT_MS
#define BTA_AV_ACCEPT_SIGNALLING_TIMEOUT_MS (2 * 1000) /* 2 seconds */
#endif

#ifndef AVRC_CONNECT_RETRY_DELAY_MS
#define AVRC_CONNECT_RETRY_DELAY_MS 2000
#endif

#ifndef BTA_AV_RC_DISC_RETRY_DELAY_MS
#define BTA_AV_RC_DISC_RETRY_DELAY_MS 3500
#endif

struct blacklist_entry
{
    int ver;
    char addr[3];
};

std::vector<RawAddress> active_device_priority_list;
extern fixed_queue_t* btu_bta_alarm_queue;
extern bool btif_av_is_split_a2dp_enabled(void);
static void bta_av_browsing_channel_open_retry(uint8_t handle);
static void bta_av_accept_signalling_timer_cback(void* data);
static void bta_av_browsing_channel_open_timer_cback(void* data);
static int browse_conn_retry_count = 1;
#ifndef AVRC_MIN_META_CMD_LEN
#define AVRC_MIN_META_CMD_LEN 20
#endif

#define AVRC_L2CAP_MIN_CONN_FAILURE_CODE 2 /*same as L2CAP_CONN_NO_PSM*/

/* state machine states */
enum {
  BTA_AV_INIT_SST,
  BTA_AV_INCOMING_SST,
  BTA_AV_OPENING_SST,
  BTA_AV_OPEN_SST,
  BTA_AV_RCFG_SST,
  BTA_AV_CLOSING_SST
};

/*******************************************************************************
 *
 * Function         bta_av_get_rcb_by_shdl
 *
 * Description      find the RCB associated with the given SCB handle.
 *
 * Returns          tBTA_AV_RCB
 *
 ******************************************************************************/
tBTA_AV_RCB* bta_av_get_rcb_by_shdl(uint8_t shdl) {
  tBTA_AV_RCB* p_rcb = NULL;
  int i;

  for (i = 0; i < BTA_AV_NUM_RCB; i++) {
    if (bta_av_cb.rcb[i].shdl == shdl &&
        bta_av_cb.rcb[i].handle != BTA_AV_RC_HANDLE_NONE) {
      p_rcb = &bta_av_cb.rcb[i];
      break;
    }
  }
  return p_rcb;
}
#define BTA_AV_STS_NO_RSP 0xFF /* a number not used by tAVRC_STS */

static void bta_av_set_peer_browse_active(uint8_t rc_handle) {
  APPL_TRACE_WARNING("%s set browse active for %d and reset others", __func__, rc_handle);
  tBTA_AV_CB *p_cb = &bta_av_cb;
  if (rc_handle < 0 || rc_handle >= BTA_AV_NUM_RCB) {
    APPL_TRACE_WARNING("%s Invalid RC handle", __func__);
    return;
  }

  p_cb->rcb[rc_handle].is_browse_active = true;
  for (int i = 0; i < BTA_AV_NUM_RCB; i++) {
    if ((i != rc_handle) && p_cb->rcb[i].is_browse_active) {
      APPL_TRACE_WARNING("%s reset browse %d ", __func__, i);
      p_cb->rcb[i].is_browse_active = false;
      AVRC_CloseBrowse(i);
    }
  }
}

int active_device_priority_list_get_idx(RawAddress bd_addr) {
  std::vector<RawAddress>::iterator it;
  it = std::find(active_device_priority_list.begin(), active_device_priority_list.end(), bd_addr);
  return (it != active_device_priority_list.end())?(it - active_device_priority_list.begin()):-1;
}

/*******************************************************************************
 *
 * Function         bta_av_del_rc
 *
 * Description      delete the given AVRC handle.
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_av_del_rc(tBTA_AV_RCB* p_rcb) {
  tBTA_AV_SCB* p_scb;
  uint8_t rc_handle; /* connected AVRCP handle */

  p_scb = NULL;
  if (p_rcb->handle != BTA_AV_RC_HANDLE_NONE) {
    if (p_rcb->shdl) {
      /* Validate array index*/
      if ((p_rcb->shdl - 1) < BTA_AV_NUM_STRS) {
        p_scb = bta_av_cb.p_scb[p_rcb->shdl - 1];
      }
      if (p_scb) {
        APPL_TRACE_DEBUG("bta_av_del_rc shdl:%d, srch:%d rc_handle:%d",
                         p_rcb->shdl, p_scb->rc_handle, p_rcb->handle);
        if (p_scb->rc_handle == p_rcb->handle)
          p_scb->rc_handle = BTA_AV_RC_HANDLE_NONE;
          p_scb->rc_ccb_alloc_handle = p_scb->rc_handle;
        /* just in case the RC timer is active
        if (bta_av_cb.features & BTA_AV_FEAT_RCCT && p_scb->chnl ==
        BTA_AV_CHNL_AUDIO) */
        alarm_cancel(p_scb->avrc_ct_timer);
      }
    }

    APPL_TRACE_IMP(
        "bta_av_del_rc  handle: %d status=0x%x, rc_acp_handle:%d, idx:%d",
        p_rcb->handle, p_rcb->status, bta_av_cb.rc_acp_handle,
        bta_av_cb.rc_acp_idx);
    rc_handle = p_rcb->handle;
    if (!(p_rcb->status & BTA_AV_RC_CONN_MASK) ||
        ((p_rcb->status & BTA_AV_RC_ROLE_MASK) == BTA_AV_RC_ROLE_INT)) {
      p_rcb->status = 0;
      p_rcb->handle = BTA_AV_RC_HANDLE_NONE;
      p_rcb->shdl = 0;
      p_rcb->lidx = 0;
      p_rcb->is_browse_active = false;
    }
    p_rcb->rc_opened = false;
    p_rcb->peer_addr = RawAddress::kEmpty;
    /* else ACP && connected. do not clear the handle yet */
    AVRC_Close(rc_handle);
    if (rc_handle == bta_av_cb.rc_acp_handle)
      bta_av_cb.rc_acp_handle = BTA_AV_RC_HANDLE_NONE;
    APPL_TRACE_EVENT(
        "end del_rc handle: %d status=0x%x, rc_acp_handle:%d, lidx:%d",
        p_rcb->handle, p_rcb->status, bta_av_cb.rc_acp_handle, p_rcb->lidx);
  }
}

/*******************************************************************************
 *
 * Function         bta_av_close_all_rc
 *
 * Description      close the all AVRC handle.
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_av_close_all_rc(tBTA_AV_CB* p_cb) {
  int i;
  active_device_priority_list.clear();
  for (i = 0; i < BTA_AV_NUM_RCB; i++) {
    if ((p_cb->disabling == true) || (bta_av_cb.rcb[i].shdl != 0))
      bta_av_del_rc(&bta_av_cb.rcb[i]);
  }
}

/*******************************************************************************
 *
 * Function         bta_av_del_sdp_rec
 *
 * Description      delete the given SDP record handle.
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_av_del_sdp_rec(uint32_t* p_sdp_handle) {
  if (*p_sdp_handle != 0) {
    SDP_DeleteRecord(*p_sdp_handle);
    *p_sdp_handle = 0;
  }
}

/*******************************************************************************
 *
 * Function         bta_av_avrc_sdp_cback
 *
 * Description      AVRCP service discovery callback.
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_av_avrc_sdp_cback(UNUSED_ATTR uint16_t status) {
  BT_HDR* p_msg = (BT_HDR*)osi_malloc(sizeof(BT_HDR));

  p_msg->event = BTA_AV_SDP_AVRC_DISC_EVT;

  bta_sys_sendmsg(p_msg);
}

/*******************************************************************************
 *
 * Function         bta_av_rc_ctrl_cback
 *
 * Description      AVRCP control callback.
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_av_rc_ctrl_cback(uint8_t handle, uint8_t event,
                                 UNUSED_ATTR uint16_t result,
                                 const RawAddress* peer_addr) {
  uint16_t msg_event = 0;
  tBTA_AV_CB* p_cb = &bta_av_cb;
  uint8_t rc_handle = BTA_AV_RC_HANDLE_NONE;
  if(handle < AVCT_NUM_LINKS)
    rc_handle = p_cb->rcb[handle].handle;

  APPL_TRACE_IMP("%s handle: %d, result %d, event=0x%x", __func__, handle, result, event);
  if (event == AVRC_OPEN_IND_EVT) {
    /* save handle of opened connection
    bta_av_cb.rc_handle = handle;*/

    msg_event = BTA_AV_AVRC_OPEN_EVT;
  } else if (event == AVRC_CLOSE_IND_EVT) {
    msg_event = BTA_AV_AVRC_CLOSE_EVT;
  } else if (event == AVRC_BROWSE_OPEN_IND_EVT) {
      if (result != 0 && (rc_handle != BTA_AV_RC_HANDLE_NONE)) {
        if (browse_conn_retry_count <= 1) {
          browse_conn_retry_count++;
          bta_av_browsing_channel_open_retry(handle);
          p_cb->rcb[handle].browse_open = true;
        } else {
          browse_conn_retry_count = 1;
          APPL_TRACE_IMP("%s Browse Connection Retry count exceeded", __func__);
        }
        return;
      }
      else {
        msg_event = BTA_AV_AVRC_BROWSE_OPEN_EVT;
      }
  } else if (event == AVRC_BROWSE_CLOSE_IND_EVT) {
    msg_event = BTA_AV_AVRC_BROWSE_CLOSE_EVT;
  }

  if (msg_event) {
    tBTA_AV_RC_CONN_CHG* p_msg =
        (tBTA_AV_RC_CONN_CHG*)osi_malloc(sizeof(tBTA_AV_RC_CONN_CHG));
    p_msg->hdr.event = msg_event;
    p_msg->handle = handle;
    p_msg->status = result;
    if (peer_addr) p_msg->peer_addr = *peer_addr;
    bta_sys_sendmsg(p_msg);
  }
}

/*******************************************************************************
 *
 * Function         bta_av_rc_msg_cback
 *
 * Description      AVRCP message callback.
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_av_rc_msg_cback(uint8_t handle, uint8_t label, uint8_t opcode,
                                tAVRC_MSG* p_msg) {
  uint8_t* p_data_src = NULL;
  uint16_t data_len = 0;

  APPL_TRACE_IMP("%s handle: %u opcode=0x%x", __func__, handle, opcode);

  /* Copy avrc packet into BTA message buffer (for sending to BTA state machine)
   */

  /* Get size of payload data  (for vendor and passthrough messages only; for
   * browsing
   * messages, use zero-copy) */
  if (opcode == AVRC_OP_VENDOR && p_msg->vendor.p_vendor_data != NULL) {
    p_data_src = p_msg->vendor.p_vendor_data;
    data_len = (uint16_t)p_msg->vendor.vendor_len;
  } else if (opcode == AVRC_OP_PASS_THRU && p_msg->pass.p_pass_data != NULL) {
    p_data_src = p_msg->pass.p_pass_data;
    data_len = (uint16_t)p_msg->pass.pass_len;
  }

  APPL_TRACE_IMP("%s data_len: %u", __func__, data_len);
  /* Create a copy of the message */
  tBTA_AV_RC_MSG* p_buf =
      (tBTA_AV_RC_MSG*)osi_malloc(sizeof(tBTA_AV_RC_MSG) + data_len);

  p_buf->hdr.event = BTA_AV_AVRC_MSG_EVT;
  p_buf->handle = handle;
  p_buf->label = label;
  p_buf->opcode = opcode;
  memcpy(&p_buf->msg, p_msg, sizeof(tAVRC_MSG));
  /* Copy the data payload, and set the pointer to it */
  if (p_data_src != NULL) {
    uint8_t* p_data_dst = (uint8_t*)(p_buf + 1);
    memcpy(p_data_dst, p_data_src, data_len);

    /* Update bta message buffer to point to payload data */
    /* (Note AVRC_OP_BROWSING uses zero-copy: p_buf->msg.browse.p_browse_data
     * already points to original avrc buffer) */
    if (opcode == AVRC_OP_VENDOR)
      p_buf->msg.vendor.p_vendor_data = p_data_dst;
    else if (opcode == AVRC_OP_PASS_THRU)
      p_buf->msg.pass.p_pass_data = p_data_dst;
  }

  if (opcode == AVRC_OP_BROWSE) {
    /* set p_pkt to NULL, so avrc would not free the buffer */
    APPL_TRACE_IMP("%s browse packet length: %d", __func__, p_msg->browse.browse_len);
    p_msg->browse.p_browse_pkt = NULL;
  }

  bta_sys_sendmsg(p_buf);
}

/*******************************************************************************
 *
 * Function         bta_av_rc_create
 *
 * Description      alloc RCB and call AVRC_Open
 *
 * Returns          the created rc handle
 *
 ******************************************************************************/
uint8_t bta_av_rc_create(tBTA_AV_CB* p_cb, uint8_t role, uint8_t shdl,
                         uint8_t lidx, RawAddress *addr) {
  tAVRC_CONN_CB ccb;
  RawAddress bda = RawAddress::kAny;
  uint8_t status = BTA_AV_RC_ROLE_ACP;
  tBTA_AV_SCB* p_scb = NULL;
  int i;
  uint8_t rc_handle;
  tBTA_AV_RCB* p_rcb;

  if (role == AVCT_INT) {
    p_scb = p_cb->p_scb[shdl - 1];
    bda = p_scb->peer_addr;
    status = BTA_AV_RC_ROLE_INT;
#if (BT_IOT_LOGGING_ENABLED == TRUE)
    device_iot_config_addr_int_add_one(p_scb->peer_addr, IOT_CONF_KEY_AVRCP_CONN_COUNT);
#endif
  } else {
    p_rcb = bta_av_get_rcb_by_shdl(shdl);
    if (addr)
    bda = *addr;
    if (p_rcb != NULL) {
      APPL_TRACE_ERROR("bta_av_rc_create ACP handle exist for shdl:%d", shdl);
      return p_rcb->handle;
    }
  }

  ccb.p_ctrl_cback = bta_av_rc_ctrl_cback;
  ccb.p_msg_cback = bta_av_rc_msg_cback;
  ccb.company_id = p_bta_av_cfg->company_id;
  ccb.conn = role;
  /* note: BTA_AV_FEAT_RCTG = AVRC_CT_TARGET, BTA_AV_FEAT_RCCT = AVRC_CT_CONTROL
   */
  ccb.control = p_cb->features & (BTA_AV_FEAT_RCTG | BTA_AV_FEAT_RCCT |
                                  BTA_AV_FEAT_METADATA | AVRC_CT_PASSIVE);

  if (AVRC_Open(&rc_handle, &ccb, bda) != AVRC_SUCCESS) {
#if (BT_IOT_LOGGING_ENABLED == TRUE)
    if (p_scb != NULL)
      device_iot_config_addr_int_add_one(p_scb->peer_addr, IOT_CONF_KEY_AVRCP_CONN_FAIL_COUNT);
#endif
    return BTA_AV_RC_HANDLE_NONE;
  }
  if (rc_handle >= BTA_AV_NUM_RCB) {
    APPL_TRACE_ERROR("bta_av_rc_create found invalid handle:%d", rc_handle);
    return BTA_AV_RC_HANDLE_NONE;
  }
  i = rc_handle;
  p_rcb = &p_cb->rcb[i];
  //no need to over write rc hadle detail for duplicate handle
  if (p_rcb->handle != BTA_AV_RC_HANDLE_NONE) {
    APPL_TRACE_ERROR("bta_av_rc_create found duplicated handle:%d", rc_handle);
  }

  APPL_TRACE_WARNING("%s RC handle %d is connected", __func__, rc_handle);
  p_rcb->handle = rc_handle;
  p_rcb->status = status;
  p_rcb->shdl = shdl;
  p_rcb->lidx = lidx;
  p_rcb->peer_features = 0;
  p_rcb->cover_art_psm = 0;
  p_rcb->is_browse_active = false;
  p_rcb->peer_addr = bda;
  /* handle 0 is reserved for acceptor handle so no need to change acceptor handle */
  if (role == AVCT_ACP && rc_handle == 0 && lidx == (BTA_AV_NUM_LINKS + 1)) {
    /* this LIDX is reserved for the AVRCP ACP connection */
    p_cb->rc_acp_handle = p_rcb->handle;
    p_cb->rc_acp_idx = (i + 1);
    APPL_TRACE_IMP("rc_acp_handle: %d idx: %d", p_cb->rc_acp_handle,
                     p_cb->rc_acp_idx);
  }
  APPL_TRACE_IMP(
      "%s create %d, role: %d, shdl:%d, rc_handle:%d, lidx:%d, status:0x%x",
      __func__, i, role, shdl, p_rcb->handle, lidx, p_rcb->status);

  return rc_handle;
}
#if (TWS_ENABLED == TRUE)
/*******************************************************************************
 *
 * Function         bta_av_valid_twsplus_command
 *
 * Description      Check if it is TWS Pluse command
 *
 * Returns          BTA_AV_RSP_ACCEPT or BTA_AV_RSP_NOT_IMPL.
 *
 *****************************************************************************/
static tBTA_AV_CODE bta_av_is_twsplus_command(uint8_t *p_data) {
 tBTA_AV_CODE ret = BTA_AV_RSP_NOT_IMPL;
  uint8_t* p_ptr = p_data;
  uint32_t u32;

  BTA_AV_BE_STREAM_TO_CO_ID(u32, p_ptr);
  if (u32 == AVRC_CO_QTI) {
    APPL_TRACE_IMP("%s: TWS passthrough cmd",__func__);
    ret = BTA_AV_RSP_ACCEPT;
  }
  return ret;
}
#endif
/*******************************************************************************
 *
 * Function         bta_av_valid_group_navi_msg
 *
 * Description      Check if it is Group Navigation Msg for Metadata
 *
 * Returns          BTA_AV_RSP_ACCEPT or BTA_AV_RSP_NOT_IMPL.
 *
 ******************************************************************************/
static tBTA_AV_CODE bta_av_group_navi_supported(uint8_t len, uint8_t* p_data,
                                                bool is_inquiry) {
  tBTA_AV_CODE ret = BTA_AV_RSP_NOT_IMPL;
  uint8_t* p_ptr = p_data;
  uint16_t u16;
  uint32_t u32;

  if (p_bta_av_cfg->avrc_group && len == BTA_GROUP_NAVI_MSG_OP_DATA_LEN) {
    BTA_AV_BE_STREAM_TO_CO_ID(u32, p_ptr);
    BE_STREAM_TO_UINT16(u16, p_ptr);

    if (u32 == AVRC_CO_METADATA) {
      if (is_inquiry) {
        if (u16 <= AVRC_PDU_PREV_GROUP) ret = BTA_AV_RSP_IMPL_STBL;
      } else {
        if (u16 <= AVRC_PDU_PREV_GROUP)
          ret = BTA_AV_RSP_ACCEPT;
        else
          ret = BTA_AV_RSP_REJ;
      }
    }
  }

  return ret;
}

/*******************************************************************************
 *
 * Function         bta_av_op_supported
 *
 * Description      Check if remote control operation is supported.
 *
 * Returns          BTA_AV_RSP_ACCEPT of supported, BTA_AV_RSP_NOT_IMPL if not.
 *
 ******************************************************************************/
static tBTA_AV_CODE bta_av_op_supported(tBTA_AV_RC rc_id, bool is_inquiry) {
  tBTA_AV_CODE ret_code = BTA_AV_RSP_NOT_IMPL;

  if (p_bta_av_rc_id) {
    if (is_inquiry) {
      if (p_bta_av_rc_id[rc_id >> 4] & (1 << (rc_id & 0x0F))) {
        ret_code = BTA_AV_RSP_IMPL_STBL;
      }
    } else {
      if (p_bta_av_rc_id[rc_id >> 4] & (1 << (rc_id & 0x0F))) {
        ret_code = BTA_AV_RSP_ACCEPT;
      } else if ((p_bta_av_cfg->rc_pass_rsp == BTA_AV_RSP_INTERIM) &&
                 p_bta_av_rc_id_ac) {
        if (p_bta_av_rc_id_ac[rc_id >> 4] & (1 << (rc_id & 0x0F))) {
          ret_code = BTA_AV_RSP_INTERIM;
        }
      }
    }
  }
  return ret_code;
}

/*******************************************************************************
 *
 * Function         bta_av_find_lcb
 *
 * Description      Given BD_addr, find the associated LCB.
 *
 * Returns          NULL, if not found.
 *
 ******************************************************************************/
tBTA_AV_LCB* bta_av_find_lcb(const RawAddress& addr, uint8_t op) {
  tBTA_AV_CB* p_cb = &bta_av_cb;
  int xx;
  uint8_t mask;
  tBTA_AV_LCB* p_lcb = NULL;

  for (xx = 0; xx < BTA_AV_NUM_LINKS; xx++) {
    mask = 1 << xx; /* the used mask for this lcb */
    if ((mask & p_cb->conn_lcb) && p_cb->lcb[xx].addr == addr) {
      p_lcb = &p_cb->lcb[xx];
      if (op == BTA_AV_LCB_FREE) {
        p_cb->conn_lcb &= ~mask; /* clear the connect mask */
        APPL_TRACE_DEBUG("%s:conn_lcb: 0x%x", __func__, p_cb->conn_lcb);
      }
      break;
    }
  }
  return p_lcb;
}

/*******************************************************************************
 *
 * Function         bta_av_rc_opened
 *
 * Description      Set AVRCP state to opened.
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_av_rc_opened(tBTA_AV_CB* p_cb, tBTA_AV_DATA* p_data) {
  tBTA_AV_RC_OPEN rc_open;
  tBTA_AV_SCB* p_scb;
  int i;
  uint8_t shdl = 0;
  tBTA_AV_LCB* p_lcb;
  tBTA_AV_RCB* p_rcb;
  uint8_t tmp;
  uint8_t disc = 0;

  /* find the SCB & stop the timer */
  APPL_TRACE_DEBUG("bta_av_rc_opened addr %s", p_data->rc_conn_chg.peer_addr.ToString().c_str());
  for (i = 0; i < BTA_AV_NUM_STRS; i++) {
    p_scb = p_cb->p_scb[i];
    if (p_scb && p_scb->peer_addr == p_data->rc_conn_chg.peer_addr) {
      p_scb->rc_handle = p_data->rc_conn_chg.handle;
      APPL_TRACE_DEBUG("bta_av_rc_opened shdl:%d, srch %d", i + 1,
                       p_scb->rc_handle);
      shdl = i + 1;
      LOG_INFO(LOG_TAG, "%s allow incoming AVRCP connections:%d", __func__,
               p_scb->use_rc);
      alarm_cancel(p_scb->avrc_ct_timer);
      disc = p_scb->hndl;
      break;
    }
  }

  i = p_data->rc_conn_chg.handle;
  if ((i >= BTA_AV_NUM_RCB) || p_cb->rcb[i].handle == BTA_AV_RC_HANDLE_NONE) {
    APPL_TRACE_ERROR("not a valid handle:%d any more", i);
    return;
  }

  APPL_TRACE_DEBUG("%s local features %d peer features %d", __func__,
                   p_cb->features, p_cb->rcb[i].peer_features);

  /* listen to browsing channel when the connection is open,
   * if peer initiated AVRCP connection and local device supports browsing
   * channel */
  AVRC_OpenBrowse(p_data->rc_conn_chg.handle, AVCT_ACP);

  if (p_cb->rcb[i].lidx == (BTA_AV_NUM_LINKS + 1) && shdl != 0) {
    /* rc is opened on the RC only ACP channel, but is for a specific
     * SCB -> need to switch RCBs */
    p_rcb = bta_av_get_rcb_by_shdl(shdl);
    if (p_rcb) {
      p_rcb->shdl = p_cb->rcb[i].shdl;
      tmp = p_rcb->lidx;
      p_rcb->lidx = p_cb->rcb[i].lidx;
      p_cb->rcb[i].lidx = tmp;
      p_cb->rc_acp_handle = p_rcb->handle;
      p_cb->rc_acp_idx = (p_rcb - p_cb->rcb) + 1;
      APPL_TRACE_DEBUG("switching RCB rc_acp_handle:%d idx:%d",
                       p_cb->rc_acp_handle, p_cb->rc_acp_idx);
    }
  }

  p_cb->rcb[i].shdl = shdl;
  p_cb->rcb[i].browse_open = true;
  rc_open.rc_handle = i;
  p_cb->rcb[i].peer_addr = p_data->rc_conn_chg.peer_addr;
  p_cb->rcb[i].rc_opened = true;
  APPL_TRACE_ERROR("bta_av_rc_opened rcb[%d] shdl:%d lidx:%d/%d", i, shdl,
                   p_cb->rcb[i].lidx, p_cb->lcb[BTA_AV_NUM_LINKS].lidx);

  p_cb->rcb[i].status |= BTA_AV_RC_CONN_MASK;

  if (!shdl && 0 == p_cb->lcb[BTA_AV_NUM_LINKS].lidx) {
    /* no associated SCB -> connected to an RC only device
     * update the index to the extra LCB */
    p_lcb = &p_cb->lcb[BTA_AV_NUM_LINKS];
    p_lcb->addr = p_data->rc_conn_chg.peer_addr;
    VLOG(1) << "rc_only bd_addr:" << p_lcb->addr;
    p_lcb->lidx = BTA_AV_NUM_LINKS + 1;
    p_cb->rcb[i].lidx = p_lcb->lidx;
    p_lcb->conn_msk = 1;
    APPL_TRACE_ERROR("rcb[%d].lidx=%d, lcb.conn_msk=x%x", i, p_cb->rcb[i].lidx,
                     p_lcb->conn_msk);
    disc = p_data->rc_conn_chg.handle | BTA_AV_CHNL_MSK;
  }

  rc_open.peer_addr = p_data->rc_conn_chg.peer_addr;
  rc_open.peer_features = p_cb->rcb[i].peer_features;
  rc_open.cover_art_psm = p_cb->rcb[i].cover_art_psm;
  rc_open.status = BTA_AV_SUCCESS;
  APPL_TRACE_DEBUG("%s local features:x%x peer_features:x%x", __func__,
                   p_cb->features, rc_open.peer_features);
  if (rc_open.peer_features == 0) {
    /* we have not done SDP on peer RC capabilities.
     * peer must have initiated the RC connection
     */
    if (p_cb->features & BTA_AV_FEAT_RCCT)
      rc_open.peer_features |= BTA_AV_FEAT_RCTG;
    if (p_cb->features & BTA_AV_FEAT_RCTG)
      rc_open.peer_features |= BTA_AV_FEAT_RCCT;

    if (bta_av_cb.disc != 0) {
      if ((bta_av_cb.disc & BTA_AV_HNDL_MSK) != (disc & BTA_AV_HNDL_MSK)) {
        /* AVRC discover db is in use */
        APPL_TRACE_IMP("%s avrcp sdp is in progress, sdhl=%d, disc=%d", __func__, shdl, disc);
        if (shdl != 0 && disc != 0)
          bta_sys_start_timer(p_scb->avrc_ct_timer, BTA_AV_RC_DISC_RETRY_DELAY_MS,
                              BTA_AV_AVRC_RETRY_DISC_EVT, disc);
      } else {
        APPL_TRACE_DEBUG("%s: avrcp sdp is in progress for same handle, skip sdp", __func__);
      }
    } else {
      bta_av_rc_disc(disc);
    }
  }
  tBTA_AV bta_av_data;
  bta_av_data.rc_open = rc_open;
  (*p_cb->p_cback)(BTA_AV_RC_OPEN_EVT, &bta_av_data);

  /* if local initiated AVRCP connection and both peer and locals device support
   * browsing channel, open the browsing channel now
   * TODO (sanketa): Some TG would not broadcast browse feature hence check
   * inter-op. */
  if ((p_cb->features & BTA_AV_FEAT_BROWSE) &&
      (rc_open.peer_features & BTA_AV_FEAT_BROWSE) &&
      ((p_cb->rcb[i].status & BTA_AV_RC_ROLE_MASK) == BTA_AV_RC_ROLE_INT)) {
    APPL_TRACE_DEBUG("%s opening AVRC Browse channel", __func__);

    if (interop_match_addr_or_name(INTEROP_AVRCP_BROWSE_OPEN_CHANNEL_COLLISION, &p_data->rc_conn_chg.peer_addr)) {
      alarm_set_on_mloop(p_cb->browsing_channel_open_timer,
                                 BTA_AV_BROWSINIG_CHANNEL_INT_TIMEOUT_MS,
                                 bta_av_browsing_channel_open_timer_cback,
                                 UINT_TO_PTR(p_data->rc_conn_chg.handle));
    }
    else {
      AVRC_OpenBrowse(p_data->rc_conn_chg.handle, AVCT_INT);
    }
  }
}
/*******************************************************************************
 *
 * Function         bta_av_browsing_channel_open_timer_cback
 *
 * Description      Send AVRCP command to open browsing channel after timer expires.
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_av_browsing_channel_open_timer_cback(void* data) {
  uint32_t handle = PTR_TO_UINT(data);
  AVRC_OpenBrowse(handle, AVCT_INT);
}

/*******************************************************************************
 *
 * Function         bta_av_rc_remote_cmd
 *
 * Description      Send an AVRCP remote control command.
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_av_rc_remote_cmd(tBTA_AV_CB* p_cb, tBTA_AV_DATA* p_data) {
  tBTA_AV_RCB* p_rcb;
  if (p_cb->features & BTA_AV_FEAT_RCCT) {
    if (p_data->hdr.layer_specific < BTA_AV_NUM_RCB) {
      p_rcb = &p_cb->rcb[p_data->hdr.layer_specific];
      if (p_rcb->status & BTA_AV_RC_CONN_MASK) {
        AVRC_PassCmd(p_rcb->handle, p_data->api_remote_cmd.label,
                     &p_data->api_remote_cmd.msg);
      }
    }
  }
}

/*******************************************************************************
 *
 * Function         bta_av_rc_vendor_cmd
 *
 * Description      Send an AVRCP vendor specific command.
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_av_rc_vendor_cmd(tBTA_AV_CB* p_cb, tBTA_AV_DATA* p_data) {
  tBTA_AV_RCB* p_rcb;
  if ((p_cb->features & (BTA_AV_FEAT_RCCT | BTA_AV_FEAT_VENDOR)) ==
      (BTA_AV_FEAT_RCCT | BTA_AV_FEAT_VENDOR)) {
    if (p_data->hdr.layer_specific < BTA_AV_NUM_RCB) {
      p_rcb = &p_cb->rcb[p_data->hdr.layer_specific];
      AVRC_VendorCmd(p_rcb->handle, p_data->api_vendor.label,
                     &p_data->api_vendor.msg);
    }
  }
}

/*******************************************************************************
 *
 * Function         bta_av_rc_vendor_rsp
 *
 * Description      Send an AVRCP vendor specific response.
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_av_rc_vendor_rsp(tBTA_AV_CB* p_cb, tBTA_AV_DATA* p_data) {
  tBTA_AV_RCB* p_rcb;
  if ((p_cb->features & (BTA_AV_FEAT_RCTG | BTA_AV_FEAT_VENDOR)) ==
      (BTA_AV_FEAT_RCTG | BTA_AV_FEAT_VENDOR)) {
    if (p_data->hdr.layer_specific < BTA_AV_NUM_RCB) {
      p_rcb = &p_cb->rcb[p_data->hdr.layer_specific];
      AVRC_VendorRsp(p_rcb->handle, p_data->api_vendor.label,
                     &p_data->api_vendor.msg);
    }
  }
}

/*******************************************************************************
 *
 * Function         bta_av_rc_meta_rsp
 *
 * Description      Send an AVRCP metadata/advanced control command/response.
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_av_rc_meta_rsp(tBTA_AV_CB* p_cb, tBTA_AV_DATA* p_data) {
  tBTA_AV_RCB* p_rcb;
  bool do_free = true;

  if ((p_cb->features & BTA_AV_FEAT_METADATA) &&
      (p_data->hdr.layer_specific < BTA_AV_NUM_RCB)) {
    if ((p_data->api_meta_rsp.is_rsp && (p_cb->features & BTA_AV_FEAT_RCTG)) ||
        (!p_data->api_meta_rsp.is_rsp && (p_cb->features & BTA_AV_FEAT_RCCT))) {
      p_rcb = &p_cb->rcb[p_data->hdr.layer_specific];
      if (p_rcb->handle != BTA_AV_RC_HANDLE_NONE) {
        AVRC_MsgReq(p_rcb->handle, p_data->api_meta_rsp.label,
                    p_data->api_meta_rsp.rsp_code, p_data->api_meta_rsp.p_pkt);
        do_free = false;
      }
    }
  }

  if (do_free) osi_free_and_reset((void**)&p_data->api_meta_rsp.p_pkt);
}

/*******************************************************************************
 *
 * Function         bta_av_rc_free_rsp
 *
 * Description      free an AVRCP metadata command buffer.
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_av_rc_free_rsp(UNUSED_ATTR tBTA_AV_CB* p_cb, tBTA_AV_DATA* p_data) {
  osi_free_and_reset((void**)&p_data->api_meta_rsp.p_pkt);
}

/*******************************************************************************
 *
 * Function         bta_av_rc_free_browse_msg
 *
 * Description      free an AVRCP browse message buffer.
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_av_rc_free_browse_msg(UNUSED_ATTR tBTA_AV_CB* p_cb,
                               tBTA_AV_DATA* p_data) {
  if (p_data->rc_msg.opcode == AVRC_OP_BROWSE) {
    osi_free_and_reset((void**)&p_data->rc_msg.msg.browse.p_browse_pkt);
  }
}

/*******************************************************************************
 *
 * Function         bta_av_chk_notif_evt_id
 *
 * Description      make sure the requested player id is valid.
 *
 * Returns          BTA_AV_STS_NO_RSP, if no error
 *
 ******************************************************************************/
static tAVRC_STS bta_av_chk_notif_evt_id(tAVRC_MSG_VENDOR* p_vendor) {
  tAVRC_STS status = BTA_AV_STS_NO_RSP;
  uint8_t xx;
  uint16_t u16;
  uint8_t* p = p_vendor->p_vendor_data + 2;

  BE_STREAM_TO_UINT16(u16, p);
  /* double check the fixed length */
  if ((u16 != 5) || (p_vendor->vendor_len != 9)) {
    status = AVRC_STS_INTERNAL_ERR;
  } else {
    /* make sure the player_id is valid */
    for (xx = 0; xx < p_bta_av_cfg->num_evt_ids; xx++) {
      if (*p == p_bta_av_cfg->p_meta_evt_ids[xx]) {
        break;
      }
    }
    if (xx == p_bta_av_cfg->num_evt_ids) {
      status = AVRC_STS_BAD_PARAM;
    }
  }

  return status;
}

/*******************************************************************************
 *
 * Function         bta_av_proc_meta_cmd
 *
 * Description      Process an AVRCP metadata command from the peer.
 *
 * Returns          true to respond immediately
 *
 ******************************************************************************/
tBTA_AV_EVT bta_av_proc_meta_cmd(tAVRC_RESPONSE* p_rc_rsp,
                                 tBTA_AV_RC_MSG* p_msg, uint8_t* p_ctype) {
  tBTA_AV_EVT evt = BTA_AV_META_MSG_EVT;
  uint8_t u8, pdu, *p;
  uint16_t u16;
  tAVRC_MSG_VENDOR* p_vendor = &p_msg->msg.vendor;
  bool is_dev_avrcpv_blacklisted = FALSE;
  int i;
  RawAddress     addr;

#if (AVRC_METADATA_INCLUDED == TRUE)
  pdu = *(p_vendor->p_vendor_data);
  p_rc_rsp->pdu = pdu;
  *p_ctype = AVRC_RSP_REJ;

  /* Check to ansure a  valid minimum meta data length */
  if ((AVRC_MIN_META_CMD_LEN + p_vendor->vendor_len) > AVRC_META_CMD_BUF_SIZE) {
    /* reject it */
    p_rc_rsp->rsp.status = AVRC_STS_BAD_PARAM;
    APPL_TRACE_ERROR("%s Invalid meta-command length: %d", __func__,
                     p_vendor->vendor_len);
    return 0;
  }

  /* Metadata messages only use PANEL sub-unit type */
  if (p_vendor->hdr.subunit_type != AVRC_SUB_PANEL) {
    APPL_TRACE_DEBUG("SUBUNIT must be PANEL");
    /* reject it */
    evt = 0;
    p_vendor->hdr.ctype = BTA_AV_RSP_NOT_IMPL;
    p_vendor->vendor_len = 0;
    p_rc_rsp->rsp.status = AVRC_STS_BAD_PARAM;
  } else if (!AVRC_IsValidAvcType(pdu, p_vendor->hdr.ctype)) {
    APPL_TRACE_DEBUG("Invalid pdu/ctype: 0x%x, %d", pdu, p_vendor->hdr.ctype);
    /* reject invalid message without reporting to app */
    evt = 0;
    p_rc_rsp->rsp.status = AVRC_STS_BAD_CMD;
  } else {
    switch (pdu) {
      case AVRC_PDU_GET_CAPABILITIES:
        /* process GetCapabilities command without reporting the event to app */
        evt = 0;
        if (p_vendor->vendor_len != 5) {
          android_errorWriteLog(0x534e4554, "111893951");
          p_rc_rsp->get_caps.status = AVRC_STS_INTERNAL_ERR;
          break;
        }
        u8 = *(p_vendor->p_vendor_data + 4);
        p = p_vendor->p_vendor_data + 2;
        p_rc_rsp->get_caps.capability_id = u8;
        BE_STREAM_TO_UINT16(u16, p);
        if (u16 != 1) {
          p_rc_rsp->get_caps.status = AVRC_STS_INTERNAL_ERR;
        } else {
          p_rc_rsp->get_caps.status = AVRC_STS_NO_ERROR;
          if (u8 == AVRC_CAP_COMPANY_ID) {
            *p_ctype = AVRC_RSP_IMPL_STBL;
            p_rc_rsp->get_caps.count = p_bta_av_cfg->num_co_ids;
            memcpy(p_rc_rsp->get_caps.param.company_id,
                   p_bta_av_cfg->p_meta_co_ids,
                   (p_bta_av_cfg->num_co_ids << 2));
          } else if (u8 == AVRC_CAP_EVENTS_SUPPORTED) {
            *p_ctype = AVRC_RSP_IMPL_STBL;
            p_rc_rsp->get_caps.count = p_bta_av_cfg->num_evt_ids;
            /* DUT has blacklisted few remote dev for Avrcp Version hence
             * respose for event supported should not have AVRCP 1.5/1.4
             * version events
             */
            if (avct_get_peer_addr_by_ccb(p_msg->handle, addr) == TRUE)
            {
                is_dev_avrcpv_blacklisted = SDP_Dev_Blacklisted_For_Avrcp15(addr);
                BTIF_TRACE_ERROR("Blacklist for AVRCP1.5 = %d", is_dev_avrcpv_blacklisted);
            }

            bool is_blacklisted_player_app_settings = interop_match_addr_or_name(
                    INTEROP_DISABLE_PLAYER_APPLICATION_SETTING_CMDS, &addr);
            BTIF_TRACE_ERROR("BL for player app setting = %d", is_blacklisted_player_app_settings);
            uint16_t profile_version = sdp_get_stored_avrc_tg_version(addr);
            uint16_t version = (AVRCP_VERSION_BIT_MASK & profile_version);
            bool is_browse_bit_set =
                    ((AVRCP_MASK_BRW_BIT & profile_version) == AVRCP_MASK_BRW_BIT);
            BTIF_TRACE_DEBUG(LOG_TAG, "AVRCP version used for SDP 0x%x,browse supported %d",
                    version, is_browse_bit_set);

            if ((version < AVRC_REV_1_4) || (!is_browse_bit_set) ||
                    (is_dev_avrcpv_blacklisted == TRUE))
            {
                for (i = 0; i < p_bta_av_cfg->num_evt_ids; ++i)
                {
                   if (version < AVRC_REV_1_4 || is_dev_avrcpv_blacklisted == TRUE) {
                     if (p_bta_av_cfg->p_meta_evt_ids[i] == AVRC_EVT_AVAL_PLAYERS_CHANGE)
                       break;
                   } else {
                     if (p_bta_av_cfg->p_meta_evt_ids[i] == AVRC_EVT_NOW_PLAYING_CHANGE)
                       break;
                   }
                }
                int evt_count = i;
                APPL_TRACE_WARNING("event count: 0x%x", evt_count);
                if ((version < AVRC_REV_1_4) || is_dev_avrcpv_blacklisted == TRUE)
                {
#if (defined(AVRC_QTI_V1_3_OPTIONAL_FEAT) && AVRC_QTI_V1_3_OPTIONAL_FEAT == TRUE)
                  evt_count = i - 1;
#endif
                  p_rc_rsp->get_caps.count = evt_count;
                  memcpy(p_rc_rsp->get_caps.param.event_id, p_bta_av_cfg->p_meta_evt_ids,
                         evt_count);
                } else {
                    if (is_blacklisted_player_app_settings) {
#if (defined(AVRC_QTI_V1_3_OPTIONAL_FEAT) && AVRC_QTI_V1_3_OPTIONAL_FEAT == TRUE)
                      int i, j;
                      for (i = 0, j = 0; i < evt_count; ++i)
                        if (p_bta_av_cfg->p_meta_evt_ids[i] != AVRC_EVT_APP_SETTING_CHANGE)
                          p_rc_rsp->get_caps.param.event_id[j++] = p_bta_av_cfg->p_meta_evt_ids[i];
                      p_rc_rsp->get_caps.count = j;
#else
                      p_rc_rsp->get_caps.count = evt_count;
                      memcpy(p_rc_rsp->get_caps.param.event_id, p_bta_av_cfg->p_meta_evt_ids,
                             evt_count);
#endif
                    } else {
                      p_rc_rsp->get_caps.count = evt_count;
                      memcpy(p_rc_rsp->get_caps.param.event_id, p_bta_av_cfg->p_meta_evt_ids,
                             evt_count);
                    }
                }
            }
            else
            {
                if (is_blacklisted_player_app_settings)
                {
#if (defined(AVRC_QTI_V1_3_OPTIONAL_FEAT) && AVRC_QTI_V1_3_OPTIONAL_FEAT == TRUE)
                    int i, j;
                    for (i = 0, j = 0; i < p_bta_av_cfg->num_evt_ids; ++i)
                      if (p_bta_av_cfg->p_meta_evt_ids[i] != AVRC_EVT_APP_SETTING_CHANGE)
                        p_rc_rsp->get_caps.param.event_id[j++] = p_bta_av_cfg->p_meta_evt_ids[i];
                    p_rc_rsp->get_caps.count = j;
#else
                    memcpy(p_rc_rsp->get_caps.param.event_id, p_bta_av_cfg->p_meta_evt_ids,
                           p_bta_av_cfg->num_evt_ids);
#endif
                } else {
                    memcpy(p_rc_rsp->get_caps.param.event_id, p_bta_av_cfg->p_meta_evt_ids,
                           p_bta_av_cfg->num_evt_ids);
                }
            }
          } else {
            APPL_TRACE_DEBUG("Invalid capability ID: 0x%x", u8);
            /* reject - unknown capability ID */
            p_rc_rsp->get_caps.status = AVRC_STS_BAD_PARAM;
          }
        }
        break;

      case AVRC_PDU_REGISTER_NOTIFICATION:
        /* make sure the event_id is implemented */
        p_rc_rsp->rsp.status = bta_av_chk_notif_evt_id(p_vendor);
        if (p_rc_rsp->rsp.status != BTA_AV_STS_NO_RSP) evt = 0;
        break;
    }
  }
#else
  APPL_TRACE_IMP("AVRCP 1.3 Metadata not supporteed. Reject command.");
  /* reject invalid message without reporting to app */
  evt = 0;
  p_rc_rsp->rsp.status = AVRC_STS_BAD_CMD;
#endif

  return evt;
}

/*******************************************************************************
 *
 * Function         bta_av_rc_msg
 *
 * Description      Process an AVRCP message from the peer.
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_av_rc_msg(tBTA_AV_CB* p_cb, tBTA_AV_DATA* p_data) {
  tBTA_AV_EVT evt = 0;
  tBTA_AV av;
  BT_HDR* p_pkt = NULL;
  tAVRC_MSG_VENDOR* p_vendor = &p_data->rc_msg.msg.vendor;
  bool is_inquiry = ((p_data->rc_msg.msg.hdr.ctype == AVRC_CMD_SPEC_INQ) ||
                     p_data->rc_msg.msg.hdr.ctype == AVRC_CMD_GEN_INQ);
#if (AVRC_METADATA_INCLUDED == TRUE)
  uint8_t ctype = 0;
  tAVRC_RESPONSE rc_rsp;

  rc_rsp.rsp.status = BTA_AV_STS_NO_RSP;
#endif

  if (NULL == p_data) {
    APPL_TRACE_ERROR("Message from peer with no data in %s", __func__);
    return;
  }

  APPL_TRACE_DEBUG("%s: opcode=%x, ctype=%x", __func__, p_data->rc_msg.opcode,
                   p_data->rc_msg.msg.hdr.ctype);

  if (p_data->rc_msg.opcode == AVRC_OP_PASS_THRU) {
    /* if this is a pass thru command */
    if ((p_data->rc_msg.msg.hdr.ctype == AVRC_CMD_CTRL) ||
        (p_data->rc_msg.msg.hdr.ctype == AVRC_CMD_SPEC_INQ) ||
        (p_data->rc_msg.msg.hdr.ctype == AVRC_CMD_GEN_INQ)) {
      /* check if operation is supported */
      char avrcp_ct_support[PROPERTY_VALUE_MAX];
      osi_property_get("bluetooth.pts.avrcp_ct.support", avrcp_ct_support,
                       "false");
      if (p_data->rc_msg.msg.pass.op_id == AVRC_ID_VENDOR) {
        p_data->rc_msg.msg.hdr.ctype = BTA_AV_RSP_NOT_IMPL;
#if (TWS_ENABLED == TRUE)
        if (p_data->rc_msg.msg.pass.p_pass_data != NULL) {
          p_data->rc_msg.msg.hdr.ctype = bta_av_is_twsplus_command(
                                          p_data->rc_msg.msg.pass.p_pass_data);
        }
#endif
#if (AVRC_METADATA_INCLUDED == TRUE)
        if (p_cb->features & BTA_AV_FEAT_METADATA
#if (TWS_ENABLED == TRUE)
          &&
          p_data->rc_msg.msg.hdr.ctype == BTA_AV_RSP_NOT_IMPL
#endif
          )
          p_data->rc_msg.msg.hdr.ctype = bta_av_group_navi_supported(
              p_data->rc_msg.msg.pass.pass_len,
              p_data->rc_msg.msg.pass.p_pass_data, is_inquiry);
#endif
      } else if (((p_data->rc_msg.msg.pass.op_id == AVRC_ID_VOL_UP) ||
                  (p_data->rc_msg.msg.pass.op_id == AVRC_ID_VOL_DOWN)) &&
                 !strcmp(avrcp_ct_support, "true")) {
        p_data->rc_msg.msg.hdr.ctype = BTA_AV_RSP_ACCEPT;
      } else {
        p_data->rc_msg.msg.hdr.ctype =
            bta_av_op_supported(p_data->rc_msg.msg.pass.op_id, is_inquiry);
      }

      APPL_TRACE_DEBUG("ctype %d", p_data->rc_msg.msg.hdr.ctype)

      /* send response */
      if (p_data->rc_msg.msg.hdr.ctype != BTA_AV_RSP_INTERIM)
        AVRC_PassRsp(p_data->rc_msg.handle, p_data->rc_msg.label,
                     &p_data->rc_msg.msg.pass);

      /* set up for callback if supported */
      if (p_data->rc_msg.msg.hdr.ctype == BTA_AV_RSP_ACCEPT ||
          p_data->rc_msg.msg.hdr.ctype == BTA_AV_RSP_INTERIM) {
        evt = BTA_AV_REMOTE_CMD_EVT;
        av.remote_cmd.rc_id = p_data->rc_msg.msg.pass.op_id;
        av.remote_cmd.key_state = p_data->rc_msg.msg.pass.state;
        av.remote_cmd.p_data = p_data->rc_msg.msg.pass.p_pass_data;
        av.remote_cmd.len = p_data->rc_msg.msg.pass.pass_len;
        memcpy(&av.remote_cmd.hdr, &p_data->rc_msg.msg.hdr, sizeof(tAVRC_HDR));
        av.remote_cmd.label = p_data->rc_msg.label;
      }
    }
    /* else if this is a pass thru response */
    /* id response type is not impl, we have to release label */
    else if (p_data->rc_msg.msg.hdr.ctype >= AVRC_RSP_NOT_IMPL) {
      /* set up for callback */
      evt = BTA_AV_REMOTE_RSP_EVT;
      av.remote_rsp.rc_id = p_data->rc_msg.msg.pass.op_id;
      av.remote_rsp.key_state = p_data->rc_msg.msg.pass.state;
      av.remote_rsp.rsp_code = p_data->rc_msg.msg.hdr.ctype;
      av.remote_rsp.label = p_data->rc_msg.label;

      /* If this response is for vendor unique command  */
      if ((p_data->rc_msg.msg.pass.op_id == AVRC_ID_VENDOR) &&
          (p_data->rc_msg.msg.pass.pass_len > 0)) {
        av.remote_rsp.p_data =
            (uint8_t*)osi_malloc(p_data->rc_msg.msg.pass.pass_len);
        APPL_TRACE_DEBUG("Vendor Unique data len = %d",
                         p_data->rc_msg.msg.pass.pass_len);
        memcpy(av.remote_rsp.p_data, p_data->rc_msg.msg.pass.p_pass_data,
               p_data->rc_msg.msg.pass.pass_len);
      }
    }
    /* must be a bad ctype -> reject*/
    else {
      p_data->rc_msg.msg.hdr.ctype = BTA_AV_RSP_REJ;
      AVRC_PassRsp(p_data->rc_msg.handle, p_data->rc_msg.label,
                   &p_data->rc_msg.msg.pass);
    }
  }
  /* else if this is a vendor specific command or response */
  else if (p_data->rc_msg.opcode == AVRC_OP_VENDOR) {
    /* set up for callback */
    av.vendor_cmd.code = p_data->rc_msg.msg.hdr.ctype;
    av.vendor_cmd.company_id = p_vendor->company_id;
    av.vendor_cmd.label = p_data->rc_msg.label;
    av.vendor_cmd.p_data = p_vendor->p_vendor_data;
    av.vendor_cmd.len = p_vendor->vendor_len;

    /* if configured to support vendor specific and it's a command */
    if ((p_cb->features & BTA_AV_FEAT_VENDOR) &&
        p_data->rc_msg.msg.hdr.ctype <= AVRC_CMD_GEN_INQ) {
#if (AVRC_METADATA_INCLUDED == TRUE)
      if ((p_cb->features & BTA_AV_FEAT_METADATA) &&
          (p_vendor->company_id == AVRC_CO_METADATA)) {
        av.meta_msg.p_msg = &p_data->rc_msg.msg;
        rc_rsp.rsp.status = BTA_AV_STS_NO_RSP;
        evt = bta_av_proc_meta_cmd(&rc_rsp, &p_data->rc_msg, &ctype);
      } else
#endif
        evt = BTA_AV_VENDOR_CMD_EVT;
    }
    /* else if configured to support vendor specific and it's a response */
    else if ((p_cb->features & BTA_AV_FEAT_VENDOR) &&
             p_data->rc_msg.msg.hdr.ctype >= AVRC_RSP_NOT_IMPL) {
#if (AVRC_METADATA_INCLUDED == TRUE)
      if ((p_cb->features & BTA_AV_FEAT_METADATA) &&
          (p_vendor->company_id == AVRC_CO_METADATA)) {
        av.meta_msg.p_msg = &p_data->rc_msg.msg;
        evt = BTA_AV_META_MSG_EVT;
      } else
#endif
        evt = BTA_AV_VENDOR_RSP_EVT;

    }
    /* else if not configured to support vendor specific and it's a command */
    else if (!(p_cb->features & BTA_AV_FEAT_VENDOR) &&
             p_data->rc_msg.msg.hdr.ctype <= AVRC_CMD_GEN_INQ) {
      if (p_data->rc_msg.msg.vendor.p_vendor_data[0] == AVRC_PDU_INVALID) {
        /* reject it */
        p_data->rc_msg.msg.hdr.ctype = BTA_AV_RSP_REJ;
        p_data->rc_msg.msg.vendor.p_vendor_data[4] = AVRC_STS_BAD_CMD;
      } else
        p_data->rc_msg.msg.hdr.ctype = BTA_AV_RSP_NOT_IMPL;
      AVRC_VendorRsp(p_data->rc_msg.handle, p_data->rc_msg.label,
                     &p_data->rc_msg.msg.vendor);
    }
  } else if (p_data->rc_msg.opcode == AVRC_OP_BROWSE) {
    /* set up for callback */
    av.meta_msg.rc_handle = p_data->rc_msg.handle;
    av.meta_msg.company_id = p_vendor->company_id;
    av.meta_msg.code = p_data->rc_msg.msg.hdr.ctype;
    av.meta_msg.label = p_data->rc_msg.label;
    av.meta_msg.p_msg = &p_data->rc_msg.msg;
    av.meta_msg.p_data = p_data->rc_msg.msg.browse.p_browse_data;
    av.meta_msg.len = p_data->rc_msg.msg.browse.browse_len;
    APPL_TRACE_DEBUG("%s: meta msg length: %d", __func__, av.meta_msg.len);
    evt = BTA_AV_META_MSG_EVT;
  }

#if (AVRC_METADATA_INCLUDED == TRUE)
  if (evt == 0 && rc_rsp.rsp.status != BTA_AV_STS_NO_RSP) {
    if (!p_pkt) {
      rc_rsp.rsp.opcode = p_data->rc_msg.opcode;
      AVRC_BldResponse(0, &rc_rsp, &p_pkt);
    }
    if (p_pkt)
      AVRC_MsgReq(p_data->rc_msg.handle, p_data->rc_msg.label, ctype, p_pkt);
  }
#endif

  /* call callback */
  if (evt != 0) {
    av.remote_cmd.rc_handle = p_data->rc_msg.handle;
    (*p_cb->p_cback)(evt, &av);
    /* If browsing message, then free the browse message buffer */
    bta_av_rc_free_browse_msg(p_cb, p_data);
  }
}

/*******************************************************************************
 *
 * Function         bta_av_rc_close
 *
 * Description      close the specified AVRC handle.
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_av_rc_close(tBTA_AV_CB* p_cb, tBTA_AV_DATA* p_data) {
  uint16_t handle = p_data->hdr.layer_specific;
  tBTA_AV_SCB* p_scb;
  tBTA_AV_RCB* p_rcb;

  if (handle < BTA_AV_NUM_RCB) {
    p_rcb = &p_cb->rcb[handle];

    APPL_TRACE_DEBUG("%s handle: %d, status=0x%x", __func__, p_rcb->handle,
                     p_rcb->status);
    if (p_rcb->handle != BTA_AV_RC_HANDLE_NONE) {
      if (p_rcb->shdl) {
        p_scb = bta_av_cb.p_scb[p_rcb->shdl - 1];
        if (p_scb) {
          /* just in case the RC timer is active
          if (bta_av_cb.features & BTA_AV_FEAT_RCCT &&
             p_scb->chnl == BTA_AV_CHNL_AUDIO) */
          alarm_cancel(p_scb->avrc_ct_timer);
        }
      }

      AVRC_Close(p_rcb->handle);
    }
  }
}

/*******************************************************************************
 *
 * Function         bta_av_rc_browse_close
 *
 * Description      Empty placeholder.
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_av_rc_browse_close(tBTA_AV_CB* p_cb, tBTA_AV_DATA* p_data) {
  APPL_TRACE_WARNING("%s empty placeholder does nothing!", __func__);
}

/*******************************************************************************
 *
 * Function         bta_av_get_shdl
 *
 * Returns          The index to p_scb[]
 *
 ******************************************************************************/
static uint8_t bta_av_get_shdl(tBTA_AV_SCB* p_scb) {
  int i;
  uint8_t shdl = 0;
  /* find the SCB & stop the timer */
  for (i = 0; i < BTA_AV_NUM_STRS; i++) {
    if (p_scb == bta_av_cb.p_scb[i]) {
      shdl = i + 1;
      break;
    }
  }
  return shdl;
}

/*******************************************************************************
 *
 * Function         bta_av_stream_chg
 *
 * Description      audio streaming status changed.
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_av_stream_chg(tBTA_AV_SCB* p_scb, bool started) {
  uint8_t started_msk;
  int i;
  uint8_t* p_streams;
  bool no_streams = false;
  tBTA_AV_SCB* p_scbi;

  started_msk = BTA_AV_HNDL_TO_MSK(p_scb->hdi);
  APPL_TRACE_DEBUG("bta_av_stream_chg started:%d started_msk:x%x chnl:x%x",
                   started, started_msk, p_scb->chnl);
  if (BTA_AV_CHNL_AUDIO == p_scb->chnl)
    p_streams = &bta_av_cb.audio_streams;
  else
    p_streams = &bta_av_cb.video_streams;

  if (started) {
    /* Let L2CAP know this channel is processed with high priority */
    if (!btif_av_is_split_a2dp_enabled()) {
      L2CA_SetAclPriority(p_scb->peer_addr, L2CAP_PRIORITY_HIGH);
    }
    (*p_streams) |= started_msk;
  } else {
    (*p_streams) &= ~started_msk;
  }

  if (!started) {
    i = 0;
    if (BTA_AV_CHNL_AUDIO == p_scb->chnl) {
      if (bta_av_cb.video_streams == 0) no_streams = true;
    } else {
      no_streams = true;
      if (bta_av_cb.audio_streams) {
        for (; i < BTA_AV_NUM_STRS; i++) {
          p_scbi = bta_av_cb.p_scb[i];
          /* scb is used and started */
          if (p_scbi && (bta_av_cb.audio_streams & BTA_AV_HNDL_TO_MSK(i)) &&
              p_scbi->peer_addr == p_scb->peer_addr) {
            no_streams = false;
            break;
          }
        }
      }
    }

    APPL_TRACE_DEBUG("no_streams:%d i:%d, audio_streams:x%x, video_streams:x%x",
                     no_streams, i, bta_av_cb.audio_streams,
                     bta_av_cb.video_streams);
    if (no_streams) {
      /* Let L2CAP know this channel is processed with low priority */
      if (!btif_av_is_split_a2dp_enabled())
        L2CA_SetAclPriority(p_scb->peer_addr, L2CAP_PRIORITY_NORMAL);
    }
  }
}

/*******************************************************************************
 *
 * Function         bta_av_conn_chg
 *
 * Description      connetion status changed.
 *                  Open an AVRCP acceptor channel, if new conn.
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_av_conn_chg(tBTA_AV_DATA* p_data) {
  tBTA_AV_CB* p_cb = &bta_av_cb;
  tBTA_AV_SCB* p_scb = NULL;
  tBTA_AV_SCB* p_scbi;
  uint8_t mask;
  uint8_t conn_msk;
  uint8_t old_msk;
  int i;
  int index = (p_data->hdr.layer_specific & BTA_AV_HNDL_MSK) - 1;
  tBTA_AV_LCB* p_lcb;
  tBTA_AV_LCB* p_lcb_rc;
  tBTA_AV_RCB *p_rcb, *p_rcb2;
  bool chk_restore = false;

  /* Validate array index*/
  if (index < BTA_AV_NUM_STRS) {
    p_scb = p_cb->p_scb[index];
  }
  mask = BTA_AV_HNDL_TO_MSK(index);
  p_lcb = bta_av_find_lcb(p_data->conn_chg.peer_addr, BTA_AV_LCB_FIND);
  conn_msk = 1 << (index + 1);
  APPL_TRACE_DEBUG("%s: conn_msk: %x is_up: %d, index: %d, mask: %x",
       __func__, conn_msk, p_data->conn_chg.is_up, index, mask);
  if (p_data->conn_chg.is_up) {
    /* set the conned mask for this channel */
    if (p_scb) {
      if (p_lcb) {
        p_lcb->conn_msk |= conn_msk;
        for (i = 0; i < BTA_AV_NUM_RCB; i++) {
          if (bta_av_cb.rcb[i].lidx == p_lcb->lidx) {
            bta_av_cb.rcb[i].shdl = index + 1;
            APPL_TRACE_DEBUG(
                "conn_chg up[%d]: %d, status=0x%x, shdl:%d, lidx:%d", i,
                bta_av_cb.rcb[i].handle, bta_av_cb.rcb[i].status,
                bta_av_cb.rcb[i].shdl, bta_av_cb.rcb[i].lidx);
            break;
          }
        }
      }
      if (p_scb->chnl == BTA_AV_CHNL_AUDIO) {
        old_msk = p_cb->conn_audio;
        p_cb->conn_audio |= mask;
      } else {
        old_msk = p_cb->conn_video;
        p_cb->conn_video |= mask;
      }

      if ((old_msk & mask) == 0) {
        /* increase the audio open count, if not set yet */
        bta_av_cb.audio_open_cnt++;
      }

      APPL_TRACE_DEBUG("%s: rc_acp_handle:%d rc_acp_idx:%d", __func__, p_cb->rc_acp_handle,
                       p_cb->rc_acp_idx);
      /* check if the AVRCP ACP channel is already connected */
      if (p_lcb && p_cb->rc_acp_handle != BTA_AV_RC_HANDLE_NONE &&
          p_cb->rc_acp_idx) {
        p_lcb_rc = &p_cb->lcb[BTA_AV_NUM_LINKS];
        APPL_TRACE_DEBUG("%s: rc_acp is connected && conn_chg on same addr "
                         "p_lcb_rc->conn_msk:x%x, p_lcb_rc->addr: %s,"
                         "conn_chg.peer_addr: %s", __func__,
                         p_lcb_rc->conn_msk, p_lcb_rc->addr.ToString().c_str(),
                         p_data->conn_chg.peer_addr.ToString().c_str());
        /* check if the RC is connected to the scb addr */
        if (p_lcb_rc->conn_msk &&
            p_lcb_rc->addr == p_data->conn_chg.peer_addr) {
          /* AVRCP is already connected.
           * need to update the association betwen SCB and RCB */
          if (p_cb->rc_acp_handle < BTA_AV_NUM_RCB &&
               !p_cb->rcb[p_cb->rc_acp_handle].rc_opened) {
            if (p_cb->rcb[p_cb->rc_acp_handle].peer_addr != p_lcb_rc->addr) {
              APPL_TRACE_ERROR("%s:RC is not open, stale entry",__func__);
            }
          }
          p_lcb_rc->conn_msk = 0; /* indicate RC ONLY is not connected */
          p_lcb_rc->lidx = 0;
          p_scb->rc_handle = p_cb->rc_acp_handle;
          p_rcb = &p_cb->rcb[p_cb->rc_acp_idx - 1];
          p_rcb->shdl = bta_av_get_shdl(p_scb);
          APPL_TRACE_DEBUG("%s: update rc_acp shdl:%d/%d srch:%d",
              __func__, index + 1, p_rcb->shdl, p_scb->rc_handle);

          p_rcb2 = bta_av_get_rcb_by_shdl(p_rcb->shdl);
          if (p_rcb2) {
            /* found the RCB that was created to associated with this SCB */
            p_cb->rc_acp_handle = p_rcb2->handle;
            p_cb->rc_acp_idx = (p_rcb2 - p_cb->rcb) + 1;
            APPL_TRACE_DEBUG("%s: new rc_acp_handle:%d, idx:%d", __func__,
                             p_cb->rc_acp_handle, p_cb->rc_acp_idx);
            p_rcb2->lidx = (BTA_AV_NUM_LINKS + 1);
            APPL_TRACE_DEBUG("%s: rc2 handle:%d lidx:%d/%d", p_rcb2->handle,
                             __func__, p_rcb2->lidx, p_cb->lcb[p_rcb2->lidx - 1].lidx);
          }
          p_rcb->lidx = p_lcb->lidx;
          APPL_TRACE_DEBUG("%s: rc handle:%d lidx:%d/%d", __func__, p_rcb->handle,
                           p_rcb->lidx, p_cb->lcb[p_rcb->lidx - 1].lidx);
        }
      }
    }
  } else {
    if ((p_cb->conn_audio & mask) && bta_av_cb.audio_open_cnt) {
      /* this channel is still marked as open. decrease the count */
      bta_av_cb.audio_open_cnt--;
    }

    /* clear the conned mask for this channel */
    p_cb->conn_audio &= ~mask;
    p_cb->conn_video &= ~mask;
    if (p_scb) {
      /* the stream is closed.
       * clear the peer address, so it would not mess up the AVRCP for the next
       * round of operation */
      p_scb->peer_addr = RawAddress::kEmpty;
      if (p_scb->chnl == BTA_AV_CHNL_AUDIO) {
        if (p_lcb) {
          p_lcb->conn_msk &= ~conn_msk;
        }
        /* audio channel is down. make sure the INT channel is down */
        /* just in case the RC timer is active
        if (p_cb->features & BTA_AV_FEAT_RCCT) */
        { alarm_cancel(p_scb->avrc_ct_timer); }
        /* one audio channel goes down. check if we need to restore high
         * priority */
        chk_restore = true;
      }
    }

    APPL_TRACE_DEBUG("%s: shdl:%d", __func__, index + 1);
    for (i = 0; i < BTA_AV_NUM_RCB; i++) {
      APPL_TRACE_DEBUG("%s: conn_chg dn[%d]: %d, status=0x%x, shdl:%d, lidx:%d",
                       __func__, i,
                       bta_av_cb.rcb[i].handle, bta_av_cb.rcb[i].status,
                       bta_av_cb.rcb[i].shdl, bta_av_cb.rcb[i].lidx);
      if (bta_av_cb.rcb[i].shdl == index + 1) {
        bta_av_del_rc(&bta_av_cb.rcb[i]);
        int idx = active_device_priority_list_get_idx(p_data->conn_chg.peer_addr);
        std::vector<RawAddress>::iterator it = active_device_priority_list.begin();
        if (idx != -1) {
          APPL_TRACE_WARNING("%s: Remove Addr: %s, (it + idx): %s", __func__,
                                           p_data->conn_chg.peer_addr.ToString().c_str(),
                                           (*(it + idx)).ToString().c_str());
          active_device_priority_list.erase(it + idx);
        }
        /* since the connection is already down and info was removed, clean
         * reference */
        bta_av_cb.rcb[i].shdl = 0;
        break;
      }
    }

    if (p_cb->conn_audio == 0 && p_cb->conn_video == 0 && (p_cb->conn_lcb & ~mask) == 0) {
      APPL_TRACE_WARNING("%s: No other AV connection up, close all RC",__func__);
      bta_av_close_all_rc(p_cb);
    }

    /* if the AVRCP is no longer listening, create the listening channel */
    if (bta_av_cb.rc_acp_handle == BTA_AV_RC_HANDLE_NONE &&
        bta_av_cb.features & BTA_AV_FEAT_RCTG)
      bta_av_rc_create(&bta_av_cb, AVCT_ACP, 0, BTA_AV_NUM_LINKS + 1, NULL);
  }

  APPL_TRACE_DEBUG(
      "%s: audio:%x video:%x up:%d conn_msk:0x%x chk_restore:%d "
      "audio_open_cnt:%d p_cb->conn_lcb:0x%x mask:0x%x", __func__,
      p_cb->conn_audio, p_cb->conn_video, p_data->conn_chg.is_up, conn_msk,
      chk_restore, p_cb->audio_open_cnt, p_cb->conn_lcb, mask);

  if (chk_restore) {
    if (p_cb->audio_open_cnt == 1) {
      /* one audio channel goes down and there's one audio channel remains open.
       * restore the switch role in default link policy */
      bta_sys_set_default_policy(BTA_ID_AV, HCI_ENABLE_MASTER_SLAVE_SWITCH);
      /* allow role switch, if this is the last connection */
      bta_av_restore_switch();
    }
    if (p_cb->audio_open_cnt) {
      /* adjust flush timeout settings to longer period */
      for (i = 0; i < BTA_AV_NUM_STRS; i++) {
        p_scbi = bta_av_cb.p_scb[i];
        if (p_scbi && p_scbi->chnl == BTA_AV_CHNL_AUDIO && p_scbi->co_started) {
          /* may need to update the flush timeout of this already started stream
           */
          if (p_scbi->co_started != bta_av_cb.audio_open_cnt) {
            p_scbi->co_started = bta_av_cb.audio_open_cnt;
            L2CA_SetFlushTimeout(
                p_scbi->peer_addr,
                p_bta_av_cfg->p_audio_flush_to[p_scbi->co_started - 1]);
          }
        }
      }
    }
  }
}

/*******************************************************************************
 *
 * Function         bta_av_disable
 *
 * Description      disable AV.
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_av_disable(tBTA_AV_CB* p_cb, UNUSED_ATTR tBTA_AV_DATA* p_data) {
  BT_HDR hdr;
  uint16_t xx;

  p_cb->disabling = true;

  bta_av_close_all_rc(p_cb);

  if (p_cb->p_disc_db) {
    SDP_CancelServiceSearch(p_cb->p_disc_db);
    osi_free_and_reset((void **)&p_cb->p_disc_db);
  }

  /* disable audio/video - de-register all channels,
   * expect BTA_AV_DEREG_COMP_EVT when deregister is complete */
  for (xx = 0; xx < BTA_AV_NUM_STRS; xx++) {
    if (p_cb->p_scb[xx] != NULL) {
      hdr.layer_specific = xx + 1;
      bta_av_api_deregister((tBTA_AV_DATA*)&hdr);
    }
    alarm_free(p_cb->accept_signalling_timer[xx]);
    p_cb->accept_signalling_timer[xx] = NULL;
  }

  alarm_free(p_cb->browsing_channel_open_timer);
  p_cb->browsing_channel_open_timer = NULL;
  alarm_free(p_cb->link_signalling_timer);
  p_cb->link_signalling_timer = NULL;
}

/*******************************************************************************
 *
 * Function         bta_av_api_disconnect
 *
 * Description      .
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_av_api_disconnect(tBTA_AV_DATA* p_data) {
  AVDT_DisconnectReq(p_data->api_discnt.bd_addr, bta_av_conn_cback);
  alarm_cancel(bta_av_cb.link_signalling_timer);
  alarm_cancel(bta_av_cb.browsing_channel_open_timer);
}

static uint16_t bta_sink_time_out() {
  char value[PROPERTY_VALUE_MAX] = {0};
  uint16_t pts_bta_accept_timeout = 5000;
  osi_property_get("vendor.bt.pts.certification", value, "false");
  if(!strcmp(value, "true")){
      return pts_bta_accept_timeout; // increase timeout value to pass PTS;
  }
  return BTA_AV_ACCEPT_SIGNALLING_TIMEOUT_MS;
}

/*function is to check if rc connection is already created or not*/
static tBTA_AV_RCB* bta_av_map_scb_rc(RawAddress bd_addr, tBTA_AV_RCB* rcb) {
  tBTA_AV_RCB *tmp_rcb = NULL;
  int i;
  for (i = 0; i < BTA_AV_NUM_STRS; i++) {
    if (bd_addr == rcb[i].peer_addr) {
      tmp_rcb = rcb;
      APPL_TRACE_DEBUG("bta_av_map_scb_rc index %d,",i);
      break;
    }
  }
  return tmp_rcb;
}

/*******************************************************************************
 *
 * Function         bta_av_sig_chg
 *
 * Description      process AVDT signal channel up/down.
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_av_sig_chg(tBTA_AV_DATA* p_data) {
  uint16_t event = p_data->str_msg.hdr.layer_specific;
  tBTA_AV_CB* p_cb = &bta_av_cb;
  uint32_t xx;
  uint8_t mask;
  uint16_t timeout = 0;
  tBTA_AV_LCB* p_lcb = NULL;
  uint8_t handle;
  tBTA_AV_RCB *tmp_rcb = NULL;
  APPL_TRACE_IMP("%s:event: %d, conn_acp: %d addr %s", __func__, event,
                 p_data->hdr.offset, p_data->str_msg.bd_addr.ToString().c_str());
  if (event == AVDT_CONNECT_IND_EVT) {
    p_lcb = bta_av_find_lcb(p_data->str_msg.bd_addr, BTA_AV_LCB_FIND);
    if (!p_lcb) {
      if (p_cb->conn_lcb > 0)
        APPL_TRACE_DEBUG("%s: Already connected to LCBs: 0x%x",
                                __func__, p_cb->conn_lcb);
      /* Check if busy processing a connection, if yes, Reject the
       * new incoming connection.
       * This is very rare case to happen as the timeout to start
       * signalling procedure is just 2 sec.
       * Also sink initiators will have retry machanism.
       * Even though busy flag is set during outgoing connection to
       * reject incoming connection at L2CAP connect request, there
       * is a chance to get here if the incoming connection has passed
       * the L2CAP connection stage.
       */
      if ((p_data->hdr.offset == AVDT_ACP) &&
          (AVDT_GetServiceBusyState() == true)) {
        APPL_TRACE_ERROR("%s(): Incoming conn while processing another.. Reject",
                         __func__);
        AVDT_DisconnectReq(p_data->str_msg.bd_addr, NULL);
        return;
      }
      /* if the address does not have an LCB yet, alloc one */
      for (xx = 0; xx < BTA_AV_NUM_LINKS; xx++) {
        mask = 1 << xx;
        APPL_TRACE_DEBUG("%s: The current conn_lcb: 0x%x", __func__, p_cb->conn_lcb);
        /* look for a p_lcb with its p_scb registered */
        if ((!(mask & p_cb->conn_lcb)) && (p_cb->p_scb[xx] != NULL)) {
          /* Check if the SCB is Free before using for
           * ACP connection
           */
          if ((p_data->hdr.offset == AVDT_ACP) &&
              (p_cb->p_scb[xx]->state != BTA_AV_INIT_SST)) {
            APPL_TRACE_DEBUG("%s: SCB in use %d", __func__, xx);
            continue;
          }
#if (TWS_ENABLED == TRUE)
          else if ((p_data->hdr.offset == AVDT_INT) &&
              (p_cb->p_scb[xx]->state == BTA_AV_INIT_SST)) {
            APPL_TRACE_DEBUG("%s: Invalid SCB %d", __func__, xx);
            continue;
          }
#endif
          APPL_TRACE_DEBUG("%s: Found a free p_lcb : 0x%x", __func__, xx);
          p_lcb = &p_cb->lcb[xx];
          p_lcb->lidx = xx + 1;
          /* start listening when the signal channel is open */
          if ((p_cb->features & BTA_AV_FEAT_RCTG) &&
              (bta_av_map_scb_rc(p_data->str_msg.bd_addr, p_cb->rcb) == NULL)) {
            if ((handle = bta_av_rc_create(p_cb, AVCT_ACP, 0, p_lcb->lidx, &p_data->str_msg.bd_addr)) != 0 &&
                 (handle != BTA_AV_RC_HANDLE_NONE)) {
              p_cb->p_scb[xx]->rc_ccb_alloc_handle = handle;
            }
          }
          p_lcb->addr = p_data->str_msg.bd_addr;
          p_lcb->conn_msk = 0; /* clear the connect mask */
          /* this entry is not used yet. */
          p_cb->conn_lcb |= mask; /* mark it as used */
          APPL_TRACE_DEBUG("%s: start sig timer %d", __func__, p_data->hdr.offset);
          if (p_data->hdr.offset == AVDT_ACP) {
            APPL_TRACE_DEBUG("%s: Incoming L2CAP acquired, set state as incoming",
                             __func__);
            p_cb->p_scb[xx]->peer_addr = p_data->str_msg.bd_addr;
            p_cb->p_scb[xx]->use_rc =
                true; /* allowing RC for incoming connection */
            bta_av_ssm_execute(p_cb->p_scb[xx], BTA_AV_ACP_CONNECT_EVT, p_data);
            AVDT_AssociateScb(p_cb->p_scb[xx]->hndl,  p_cb->p_scb[xx]->peer_addr);
            /* The Pending Event should be sent as soon as the L2CAP signalling
             * channel
             * is set up, which is NOW. Earlier this was done only after
             * BTA_AV_SIGNALLING_TIMEOUT_MS.
             * The following function shall send the event and start the
             * recurring timer
             */
            bta_av_signalling_timer(NULL);

            APPL_TRACE_DEBUG("%s: Re-start timer for AVDTP service", __func__);
            bta_sys_conn_open(BTA_ID_AV, p_cb->p_scb[xx]->app_id,
                              p_cb->p_scb[xx]->peer_addr);
            /* Possible collision : need to avoid outgoing processing while the
             * timer is running */
            p_cb->p_scb[xx]->coll_mask = BTA_AV_COLL_INC_TMR;
            timeout = bta_sink_time_out();
            /* Add 500msec offset to timeout if there is an outstanding
             * incoming connection */
            for (uint32_t i = 0; i < BTA_AV_NUM_LINKS; i++) {
              if ((p_cb->p_scb[i] != NULL &&
                  p_cb->p_scb[i]->coll_mask & BTA_AV_COLL_INC_TMR) && i != xx)
                timeout += 500;
            }
            APPL_TRACE_DEBUG("%s: AV signalling timer started for index = %d", __func__, xx);
            APPL_TRACE_DEBUG("%s: Remote Addr: %s", __func__,
                            p_cb->p_scb[xx]->peer_addr.ToString().c_str());
            alarm_set_on_mloop(p_cb->accept_signalling_timer[xx],
                               //bta_sink_time_out(),
                               timeout,
                               bta_av_accept_signalling_timer_cback,
                               UINT_TO_PTR(xx));
          }
          break;
        }
      }

      /* check if we found something */
      if (xx == BTA_AV_NUM_LINKS) {
        /* We do not have scb for this avdt connection.     */
        /* Silently close the connection.                   */
        APPL_TRACE_ERROR("%s: av scb not available for avdt connection", __func__);
        AVDT_DisconnectReq(p_data->str_msg.bd_addr, NULL);
        return;
      }
    }
  }
#if (BTA_AR_INCLUDED == TRUE)
  else if (event == BTA_AR_AVDT_CONN_EVT) {
    alarm_cancel(bta_av_cb.link_signalling_timer);
  }
#endif
  else {
    /* disconnected. */
    APPL_TRACE_DEBUG("%s: bta_av_cb.conn_lcb is %d", __func__,
                     bta_av_cb.conn_lcb);

    p_lcb = bta_av_find_lcb(p_data->str_msg.bd_addr, BTA_AV_LCB_FREE);
    if (p_lcb) {
      APPL_TRACE_DEBUG("%s: conn_lcb: 0x%x, conn_msk: 0x%x", __func__,
                         bta_av_cb.conn_lcb, p_lcb->conn_msk);
      /* clean up ssm  */
      for (xx = 0; xx < BTA_AV_NUM_STRS; xx++) {
        if (p_cb->p_scb[xx] &&
            p_cb->p_scb[xx]->peer_addr == p_data->str_msg.bd_addr) {
          APPL_TRACE_DEBUG("%s: Closing timer for AVDTP service", __func__);
          bta_sys_conn_close(BTA_ID_AV, p_cb->p_scb[xx]->app_id,
                             p_cb->p_scb[xx]->peer_addr);
        }
        mask = 1 << (xx + 1);
        if (p_cb->p_scb[xx] &&
            p_cb->p_scb[xx]->peer_addr == p_data->str_msg.bd_addr) {
          APPL_TRACE_WARNING("%s: Sending AVDT_DISCONNECT_EVT peer_addr=%s",
                             __func__,
                             p_cb->p_scb[xx]->peer_addr.ToString().c_str());
          bta_av_ssm_execute(p_cb->p_scb[xx], BTA_AV_AVDT_DISCONNECT_EVT, NULL);
        }
      }
    }
    if (((tmp_rcb = bta_av_map_scb_rc(p_data->str_msg.bd_addr, p_cb->rcb))
         != NULL) && tmp_rcb->handle != BTA_AV_RC_HANDLE_NONE) {
      //get rcb as per bd address to initiate cleanup
      APPL_TRACE_IMP("%s:bta_av_sig_chg event: closing unmaped RC handle %d",
        __func__, tmp_rcb->handle)
      if (tmp_rcb->handle) {
        // rc handle 0 is reserved for acceptor, need to cleanup from old path
        AVRC_Close(tmp_rcb->handle);
      }
    }
  }
  APPL_TRACE_DEBUG("%s: sig_chg conn_lcb: 0x%x", __func__, p_cb->conn_lcb);
}

/*******************************************************************************
 *
 * Function         bta_av_signalling_timer
 *
 * Description      process the signal channel timer. This timer is started
 *                  when the AVDTP signal channel is connected. If no profile
 *                  is connected, the timer goes off every
 *                  BTA_AV_SIGNALLING_TIMEOUT_MS.
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_av_signalling_timer(UNUSED_ATTR tBTA_AV_DATA* p_data) {
  tBTA_AV_CB* p_cb = &bta_av_cb;
  int xx;
  uint8_t mask;
  tBTA_AV_LCB* p_lcb = NULL;

  APPL_TRACE_DEBUG("%s", __func__);
  for (xx = 0; xx < BTA_AV_NUM_LINKS; xx++) {
    mask = 1 << xx;
    if (mask & p_cb->conn_lcb) {
      /* this entry is used. check if it is connected */
      p_lcb = &p_cb->lcb[xx];
      if (!p_lcb->conn_msk) {
        APPL_TRACE_DEBUG("bta_av_sig_timer on IDX = %d",xx);
        bta_sys_start_timer(p_cb->link_signalling_timer,
                            BTA_AV_SIGNALLING_TIMEOUT_MS,
                            BTA_AV_SIGNALLING_TIMER_EVT, 0);
        tBTA_AV_PEND pend;
        pend.bd_addr = p_lcb->addr;
        pend.hndl = p_cb->p_scb[xx]->hndl;
        (*p_cb->p_cback)(BTA_AV_PENDING_EVT, (tBTA_AV*)&pend);
      }
    }
  }
}

/*******************************************************************************
 *
 * Function         bta_av_accept_signalling_timer_cback
 *
 * Description      Process the timeout when SRC is accepting connection
 *                  and SNK did not start signalling.
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_av_accept_signalling_timer_cback(void* data) {
  uint32_t inx = PTR_TO_UINT(data);
  tBTA_AV_CB* p_cb = &bta_av_cb;
  tBTA_AV_SCB* p_scb = NULL;
  if (inx < BTA_AV_NUM_STRS) {
    p_scb = p_cb->p_scb[inx];
  }
  if (p_scb) {
    APPL_TRACE_DEBUG("%s coll_mask = 0x%02X index = %d", __func__, p_scb->coll_mask, inx);
    APPL_TRACE_DEBUG("%s: Remote Addr: %0s", __func__,
                     p_scb->peer_addr.ToString().c_str());
    if (p_scb->coll_mask & BTA_AV_COLL_INC_TMR) {
      p_scb->coll_mask &= ~BTA_AV_COLL_INC_TMR;

      APPL_TRACE_DEBUG("%s: stream state opening: SDP started = %d", __func__,
                       p_scb->sdp_discovery_started);
      if (p_scb->sdp_discovery_started) {
        /* We are still doing SDP. Run the timer again. */
        p_scb->coll_mask |= BTA_AV_COLL_INC_TMR;
        alarm_set_on_mloop(p_cb->accept_signalling_timer[inx],
                           bta_sink_time_out(),
                           bta_av_accept_signalling_timer_cback,
                           UINT_TO_PTR(inx));
        APPL_TRACE_DEBUG("%s:sdp in progress,starting timer loop",__func__);
        return;
      }
      if (bta_av_is_scb_opening(p_scb)) {
          /* SNK did not start signalling, resume signalling process. */
          bta_av_discover_req(p_scb, NULL);
      } else if (bta_av_is_scb_incoming(p_scb)) {
        /* Stay in incoming state if SNK does not start signalling */

        APPL_TRACE_DEBUG("%s: stream state incoming", __func__);
        /* API open was called right after SNK opened L2C connection. */
        if (p_scb->coll_mask & BTA_AV_COLL_API_CALLED) {
          p_scb->coll_mask &= ~BTA_AV_COLL_API_CALLED;

          /* BTA_AV_API_OPEN_EVT */
          tBTA_AV_API_OPEN* p_buf =
              (tBTA_AV_API_OPEN*)osi_malloc(sizeof(tBTA_AV_API_OPEN));
          memcpy(p_buf, &(p_scb->open_api), sizeof(tBTA_AV_API_OPEN));
          bta_sys_sendmsg(p_buf);
        }
      }
    }
  }
}

uint16_t bta_get_dut_avrcp_version() {
    // This api get avrcp version stored in property
    uint16_t profile_version = AVRC_REV_1_0;
    char avrcp_version[PROPERTY_VALUE_MAX] = {0};
    osi_property_get(AVRCP_VERSION_PROPERTY, avrcp_version,
                     AVRCP_1_6_STRING);

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
    APPL_TRACE_DEBUG(" %s AVRCP version used for sdp: \"%s\"",
             __func__,avrcp_version);
    return profile_version;
}

#if (BT_IOT_LOGGING_ENABLED == TRUE)
static void bta_av_store_peer_rc_version() {
  tBTA_AV_CB* p_cb = &bta_av_cb;
  tSDP_DISC_REC* p_rec = NULL;
  uint16_t peer_rc_version = 0; /*Assuming Default peer version as 1.3*/

  if ((p_rec = SDP_FindServiceInDb(
      p_cb->p_disc_db, UUID_SERVCLASS_AV_REMOTE_CONTROL, NULL)) != NULL) {
    if ((SDP_FindAttributeInRec(p_rec, ATTR_ID_BT_PROFILE_DESC_LIST)) != NULL) {
      /* get profile version (if failure, version parameter is not updated) */
      SDP_FindProfileVersionInRec(p_rec, UUID_SERVCLASS_AV_REMOTE_CONTROL,
                                 &peer_rc_version);
    }
    if (peer_rc_version != 0)
      device_iot_config_addr_set_hex_if_greater(p_rec->remote_bd_addr,
              IOT_CONF_KEY_AVRCP_CTRL_VERSION, peer_rc_version, IOT_CONF_BYTE_NUM_2);
  }

  peer_rc_version = 0;
  if ((p_rec = SDP_FindServiceInDb(
      p_cb->p_disc_db, UUID_SERVCLASS_AV_REM_CTRL_TARGET, NULL)) != NULL) {
    if ((SDP_FindAttributeInRec(p_rec, ATTR_ID_BT_PROFILE_DESC_LIST)) != NULL) {
      /* get profile version (if failure, version parameter is not updated) */
      SDP_FindProfileVersionInRec(p_rec, UUID_SERVCLASS_AV_REMOTE_CONTROL,
                                 &peer_rc_version);
    }
    if (peer_rc_version != 0)
      device_iot_config_addr_set_hex_if_greater(p_rec->remote_bd_addr,
              IOT_CONF_KEY_AVRCP_TG_VERSION, peer_rc_version, IOT_CONF_BYTE_NUM_2);
  }
}
#endif

/*******************************************************************************
 *
 * Function         bta_av_check_peer_features
 *
 * Description      check supported features on the peer device from the SDP
 *                  record and return the feature mask
 *
 * Returns          tBTA_AV_FEAT peer device feature mask
 *
 ******************************************************************************/
tBTA_AV_FEAT bta_av_check_peer_features(uint16_t service_uuid) {
  tBTA_AV_FEAT peer_features = 0;
  tBTA_AV_CB* p_cb = &bta_av_cb;
  tSDP_DISC_REC* p_rec = NULL;
  tSDP_DISC_ATTR* p_attr;
  uint16_t peer_rc_version = 0;
  uint16_t categories = 0;

  APPL_TRACE_DEBUG("bta_av_check_peer_features service_uuid:x%x", service_uuid);
  /* loop through all records we found */
  while (true) {
    /* get next record; if none found, we're done */
    p_rec = SDP_FindServiceInDb(p_cb->p_disc_db, service_uuid, p_rec);
    if (p_rec == NULL) {
      break;
    }

    if ((SDP_FindAttributeInRec(p_rec, ATTR_ID_SERVICE_CLASS_ID_LIST)) !=
        NULL) {
      /* find peer features */
      if (SDP_FindServiceInDb(p_cb->p_disc_db, UUID_SERVCLASS_AV_REMOTE_CONTROL,
                              NULL)) {
        peer_features |= BTA_AV_FEAT_RCCT;
      }
      if (SDP_FindServiceInDb(p_cb->p_disc_db,
                              UUID_SERVCLASS_AV_REM_CTRL_TARGET, NULL)) {
        peer_features |= BTA_AV_FEAT_RCTG;
      }
    }

    if ((SDP_FindAttributeInRec(p_rec, ATTR_ID_BT_PROFILE_DESC_LIST)) != NULL) {
      /* get profile version (if failure, version parameter is not updated) */
      SDP_FindProfileVersionInRec(p_rec, UUID_SERVCLASS_AV_REMOTE_CONTROL,
                                  &peer_rc_version);

      if (interop_match_addr_or_name(INTEROP_ADV_AVRCP_VER_1_3,
              &p_rec->remote_bd_addr))
      {
          peer_rc_version = AVRC_REV_1_3;
          APPL_TRACE_DEBUG("changing peer_rc_version as part of blacklisting to 0x%x",
                  peer_rc_version);
      }
      else if (interop_match_addr_or_name(INTEROP_STORE_REMOTE_AVRCP_VERSION_1_4,
              &p_rec->remote_bd_addr))
      {
          peer_rc_version = AVRC_REV_1_4;
          APPL_TRACE_DEBUG("changing peer_rc_version as part of blacklisting to 0x%x",
                  peer_rc_version);
      }

      APPL_TRACE_DEBUG("peer_rc_version 0x%x", peer_rc_version);

      if (peer_rc_version >= AVRC_REV_1_3)
        peer_features |= (BTA_AV_FEAT_VENDOR | BTA_AV_FEAT_METADATA);

      if (peer_rc_version >= AVRC_REV_1_4) {
#if (defined(AVRC_QTI_V1_3_OPTIONAL_FEAT) && AVRC_QTI_V1_3_OPTIONAL_FEAT == TRUE)
        peer_features |= (BTA_AV_FEAT_APP_SETTING);
#endif
        /* get supported categories */
        p_attr = SDP_FindAttributeInRec(p_rec, ATTR_ID_SUPPORTED_FEATURES);
        if (p_attr != NULL) {
          categories = p_attr->attr_value.v.u16;
          APPL_TRACE_DEBUG("peer categories: 0x%x", categories);
          if (categories & AVRC_SUPF_CT_CAT2)
            peer_features |= (BTA_AV_FEAT_ADV_CTRL);
          if (categories & AVRC_SUPF_CT_BROWSE)
            peer_features |= (BTA_AV_FEAT_BROWSE);
          uint16_t dut_avrcp_version = bta_get_dut_avrcp_version();
          if (((categories & AVRC_SUPF_CT_COVER_ART_GET_IMAGE) ||
              (categories & AVRC_SUPF_CT_COVER_ART_GET_THUMBNAIL))
              && (dut_avrcp_version == AVRC_REV_1_6))
          {
              peer_features |= (BTA_AV_FEAT_CA);
              APPL_TRACE_DEBUG("peer supports cover art");
          }
        }
      }
    }
  }
  APPL_TRACE_DEBUG("peer_features:x%x", peer_features);
  return peer_features;
}

/*******************************************************************************
 *
 * Function         bta_avk_check_peer_features
 *
 * Description      check supported features on the peer device from the SDP
 *                  record and return the feature mask
 *
 * Returns          tBTA_AV_FEAT peer device feature mask
 *
 ******************************************************************************/
tBTA_AV_FEAT bta_avk_check_peer_features(uint16_t service_uuid, uint16_t *p_psm) {
  tBTA_AV_FEAT peer_features = 0;
  tBTA_AV_CB* p_cb = &bta_av_cb;
  tSDP_DISC_REC *p_rec = NULL;
  tSDP_DISC_ATTR *p_attr;
  uint16_t peer_rc_version = 0;
  uint16_t categories = 0;
  bool val;


  APPL_TRACE_DEBUG("%s service_uuid:x%x", __func__, service_uuid);
  /* get next record; if none found, we're done */
  p_rec = SDP_FindServiceInDb(p_cb->p_disc_db, service_uuid, p_rec);
  if (p_rec != NULL) {
      APPL_TRACE_DEBUG("%s found Service record for x%x", __func__, service_uuid);
      if (service_uuid == UUID_SERVCLASS_AV_REMOTE_CONTROL)
          peer_features |= BTA_AV_FEAT_RCCT;
      else if (service_uuid == UUID_SERVCLASS_AV_REM_CTRL_TARGET)
          peer_features |= BTA_AV_FEAT_RCTG;
      if ((SDP_FindAttributeInRec(p_rec, ATTR_ID_BT_PROFILE_DESC_LIST)) != NULL)
      {
          /* profile version (if failure, version parameter is not updated) */
          val = SDP_FindProfileVersionInRec(p_rec,
          UUID_SERVCLASS_AV_REMOTE_CONTROL, &peer_rc_version);
          APPL_TRACE_DEBUG("%s rc_version for TG 0x%x, profile_found %d", __FUNCTION__,
                                               peer_rc_version, val);

          if (peer_rc_version < AVRC_REV_1_3)
              return peer_features;

          p_attr = SDP_FindAttributeInRec(p_rec, ATTR_ID_SUPPORTED_FEATURES);
          if (p_attr == NULL)
              return peer_features;

          categories = p_attr->attr_value.v.u16;

          if (service_uuid == UUID_SERVCLASS_AV_REM_CTRL_TARGET)
          {
              peer_features |= (BTA_AV_FEAT_VENDOR | BTA_AV_FEAT_METADATA);
              /* get supported categories */
              if (categories & AVRC_SUPF_CT_BROWSE)
                  peer_features |= BTA_AV_FEAT_BROWSE;
              if (categories & AVRC_SUPF_CT_APP_SETTINGS)
                  peer_features |= BTA_AV_FEAT_APP_SETTING;
              if ((peer_rc_version >= AVRC_REV_1_6) &&
                    (categories & AVRC_SUPF_TG_PLAYER_COVER_ART))
              {
                  peer_features |= BTA_AV_FEAT_CA;
                  if ((p_attr = SDP_FindAttributeInRec(p_rec,
                          ATTR_ID_ADDITION_PROTO_DESC_LISTS)) != NULL)
                  {
                      if (SDP_FindAvrcpCoverArtPSM(p_attr, p_psm) != TRUE)
                          *p_psm = 0;

                      APPL_TRACE_DEBUG(" %s PSM for cover art obex l2cap channel 0x%04X",
                            __FUNCTION__, *p_psm);
                  }
              }
          }
          /*
           * Absolute Volume came after in 1.4 and above.
           * but there are few devices in market which supports.
           * absoluteVolume and they are still 1.3 To avoid inter-operatibility issuses with.
           * those devices, we check for 1.3 as minimum version.
           */
          else if (service_uuid == UUID_SERVCLASS_AV_REMOTE_CONTROL)
          {
              if (categories & AVRC_SUPF_CT_CAT2)
              {
                  APPL_TRACE_DEBUG(" %s Remote supports ABS Vol", __FUNCTION__);
                  peer_features |= BTA_AV_FEAT_ADV_CTRL;
              }
          }
    }
  }
  APPL_TRACE_DEBUG("%s peer_features:x%x", __func__, peer_features);
  return peer_features;
}

/*******************************************************************************
 *
 * Function         bta_av_rc_disc_done
 *
 * Description      Handle AVRCP service discovery results.  If matching
 *                  service found, open AVRCP connection.
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_av_rc_disc_done(UNUSED_ATTR tBTA_AV_DATA* p_data) {
  tBTA_AV_CB* p_cb = &bta_av_cb;
  tBTA_AV_SCB* p_scb = NULL;
  tBTA_AV_RC_OPEN rc_open;
  tBTA_AV_LCB* p_lcb;
  uint8_t rc_handle;
  uint16_t cover_art_psm = 0;
  tBTA_AV_FEAT peer_features = 0; /* peer features mask */

  APPL_TRACE_DEBUG("%s bta_av_rc_disc_done disc:x%x", __func__, p_cb->disc);
  if (!p_cb->disc) {
    return;
  }

  if ((p_cb->disc & BTA_AV_CHNL_MSK) == BTA_AV_CHNL_MSK) {
    /* this is the rc handle/index to tBTA_AV_RCB */
    rc_handle = p_cb->disc & (~BTA_AV_CHNL_MSK);
  } else {
    /* Validate array index*/
    if ((((p_cb->disc & BTA_AV_HNDL_MSK) - 1) < BTA_AV_NUM_STRS) &&
        (((p_cb->disc & BTA_AV_HNDL_MSK) - 1) >= 0)) {
      p_scb = p_cb->p_scb[(p_cb->disc & BTA_AV_HNDL_MSK) - 1];
    }
    if (p_scb) {
      rc_handle = p_scb->rc_handle;
    } else {
      p_cb->disc = 0;
      return;
    }
  }

  if (rc_handle == BTA_AV_RC_HANDLE_NONE)
  {
      if (p_scb != NULL && AVRC_CheckIncomingConn(p_scb->peer_addr) == TRUE)
      {
          bta_sys_start_timer(p_scb->avrc_ct_timer, AVRC_CONNECT_RETRY_DELAY_MS,
                                 BTA_AV_SDP_AVRC_DISC_EVT,p_scb->hndl);
          APPL_TRACE_DEBUG("%s: incoming connection in progress, reset sdp disc handle",__func__);
          p_cb->disc = 0;
          return;
      }
  }
#if (BTA_AV_SINK_INCLUDED == TRUE)
  if (p_cb->sdp_a2dp_snk_handle) {
    /* This is Sink + CT + TG(Abs Vol) */
      peer_features = bta_avk_check_peer_features(UUID_SERVCLASS_AV_REMOTE_CONTROL,
                                                  &cover_art_psm);
      peer_features |= bta_avk_check_peer_features(UUID_SERVCLASS_AV_REM_CTRL_TARGET,
                                                  &cover_art_psm);
      APPL_TRACE_DEBUG("final rc_features %x", peer_features);
  } else
#endif
      if (p_cb->sdp_a2dp_handle) {
    /* check peer version and whether support CT and TG role */
    peer_features =
        bta_av_check_peer_features(UUID_SERVCLASS_AV_REMOTE_CONTROL);
    if ((p_cb->features & BTA_AV_FEAT_ADV_CTRL) &&
        ((peer_features & BTA_AV_FEAT_ADV_CTRL) == 0)) {
      /* if we support advance control and peer does not, check their support on
       * TG role
       * some implementation uses 1.3 on CT ans 1.4 on TG */
      peer_features |=
          bta_av_check_peer_features(UUID_SERVCLASS_AV_REM_CTRL_TARGET);
    }
  }

#if (BT_IOT_LOGGING_ENABLED == TRUE)
  bta_av_store_peer_rc_version();
#endif

  p_cb->disc = 0;
  osi_free_and_reset((void**)&p_cb->p_disc_db);

  APPL_TRACE_DEBUG("peer_features 0x%x, features 0x%x", peer_features,
                   p_cb->features);

  /* if we have no rc connection */
  if (rc_handle == BTA_AV_RC_HANDLE_NONE) {
    if (p_scb) {
      /* if peer remote control service matches ours and USE_RC is true */
      if ((((p_cb->features & BTA_AV_FEAT_RCCT) &&
            (peer_features & BTA_AV_FEAT_RCTG)) ||
           ((p_cb->features & BTA_AV_FEAT_RCTG) &&
            (peer_features & BTA_AV_FEAT_RCCT)))) {
        p_lcb = bta_av_find_lcb(p_scb->peer_addr, BTA_AV_LCB_FIND);
        if (p_lcb) {
          if (p_scb->rc_ccb_alloc_handle != BTA_AV_RC_HANDLE_NONE) {
            APPL_TRACE_DEBUG("closing previous allocating unnecessary ccb %d", p_scb->rc_ccb_alloc_handle);
            AVRC_Close(p_scb->rc_ccb_alloc_handle);
            p_scb->rc_ccb_alloc_handle = BTA_AV_RC_HANDLE_NONE;
          }
          rc_handle = bta_av_rc_create(p_cb, AVCT_INT,
                                       (uint8_t)(p_scb->hdi + 1), p_lcb->lidx, NULL);
          if ((rc_handle != BTA_AV_RC_HANDLE_NONE) && (rc_handle < BTA_AV_NUM_RCB)) {
            p_cb->rcb[rc_handle].peer_features = peer_features;
            p_cb->rcb[rc_handle].cover_art_psm = cover_art_psm;
          } else {
            /* cannot create valid rc_handle for current device */
            APPL_TRACE_ERROR(" No link resources available");
            p_scb->use_rc = FALSE;
            rc_open.peer_addr = p_scb->peer_addr;
            rc_open.peer_features = 0;
            rc_open.status = BTA_AV_FAIL_RESOURCES;
            (*p_cb->p_cback)(BTA_AV_RC_OPEN_EVT, (tBTA_AV *) &rc_open);
          }
        } else {
          APPL_TRACE_ERROR("can not find LCB!!");
        }
      } else if (p_scb->use_rc) {
        /* can not find AVRC on peer device. report failure */
        p_scb->use_rc = false;
        rc_open.peer_addr = p_scb->peer_addr;
        rc_open.peer_features = 0;
        rc_open.status = BTA_AV_FAIL_SDP;
        tBTA_AV bta_av_data;
        bta_av_data.rc_open = rc_open;
        (*p_cb->p_cback)(BTA_AV_RC_OPEN_EVT, &bta_av_data);
      }
#if (BT_IOT_LOGGING_ENABLED == TRUE)
      if (peer_features != 0)
        device_iot_config_addr_set_hex(p_scb->peer_addr,
                IOT_CONF_KEY_AVRCP_FEATURES, peer_features, IOT_CONF_BYTE_NUM_2);
#endif
    }
  } else if(rc_handle < BTA_AV_NUM_RCB) {
    tBTA_AV_RC_FEAT rc_feat;
    p_cb->rcb[rc_handle].peer_features = peer_features;
    rc_feat.cover_art_psm = cover_art_psm;
    rc_feat.rc_handle = rc_handle;
    rc_feat.peer_features = peer_features;
    /*Assuming here incoming RC is connected before timer expired
      so previous allocated ccb is used*/
    if (p_scb != NULL &&
      p_scb->rc_ccb_alloc_handle != BTA_AV_RC_HANDLE_NONE) {
      p_scb->rc_ccb_alloc_handle = BTA_AV_RC_HANDLE_NONE;
    }
    if (p_scb == NULL) {
      /*
       * In case scb is not created by the time we are done with SDP
       * we still need to send RC feature event. So we need to get BD
       * from Message
       */
      rc_feat.peer_addr = p_cb->lcb[p_cb->rcb[rc_handle].lidx - 1].addr;
    } else {
      rc_feat.peer_addr = p_scb->peer_addr;
    }
    tBTA_AV bta_av_data;
    bta_av_data.rc_feat = rc_feat;
    (*p_cb->p_cback)(BTA_AV_RC_FEAT_EVT, &bta_av_data);
#if (BT_IOT_LOGGING_ENABLED == TRUE)
    if (peer_features != 0)
      device_iot_config_addr_set_hex(rc_feat.peer_addr,
              IOT_CONF_KEY_AVRCP_FEATURES, peer_features, IOT_CONF_BYTE_NUM_2);
#endif
  }
}

/*******************************************************************************
 *
 * Function         bta_av_rc_collission_detected
 *
 * Description      Update App on collision detected case
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_av_rc_collission_detected(tBTA_AV_DATA *p_data) {
  tBTA_AV_CB   *p_cb = &bta_av_cb;
  tBTA_AV_RC_COLL_DETECTED rc_coll;
  tBTA_AV_RC_COLLISSION_DETECTED *p_msg =
                (tBTA_AV_RC_COLLISSION_DETECTED *)p_data;
  rc_coll.rc_handle = p_msg->handle;
  rc_coll.peer_addr =  p_msg->peer_addr;
  (*p_cb->p_cback)(BTA_AV_RC_COLL_DETECTED_EVT, (tBTA_AV *) &rc_coll);
}

/*******************************************************************************
 *
 * Function         bta_av_active_browse
 *
 * Description      Set AVRCP Browse device to active
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_av_active_browse(tBTA_AV_DATA *p_data) {
  tBTA_AV_CB *p_cb = &bta_av_cb;
  tBTA_AV_API_ACTIVE_BROWSE_RC *p_msg = (tBTA_AV_API_ACTIVE_BROWSE_RC *)p_data;
  uint8_t rc_handle = p_msg->hdr.layer_specific;
  const RawAddress bd_addr = p_msg->peer_addr;
  uint8_t browse_evt = p_msg->browse_device_evt;
  APPL_TRACE_WARNING("%s hdl %d, handoff %d", __func__, rc_handle, browse_evt);
  APPL_TRACE_WARNING("%s: Remote Addr: %s", __func__, bd_addr.ToString().c_str());
  if (rc_handle >= 0 && rc_handle < BTA_AV_NUM_RCB) {
    APPL_TRACE_WARNING("%s feature %d isactive %d",__func__,
            p_cb->rcb[rc_handle].peer_features, p_cb->rcb[rc_handle].is_browse_active);
    if (p_cb->rcb[rc_handle].is_browse_active || bd_addr == RawAddress::kEmpty) {
      APPL_TRACE_WARNING("%s: Device hdl already browse active ignore", __func__);
      return;
    }
  }

  bool is_active_set = false;
  int device_list_size = active_device_priority_list.size();
  int found_idx = active_device_priority_list_get_idx(bd_addr);
  bool device_exist = (found_idx != -1);
  APPL_TRACE_WARNING("%s: Device exist %d, at idx = %d", __func__, device_exist, found_idx);

  switch(browse_evt) {
    case BTA_AV_BROWSE_CONNECT:
      if (device_exist) {
        int active_rc_hdl = -1;
        for (int i = 0; i < BTA_AV_NUM_RCB; i++) {
          if ((i != rc_handle) && p_cb->rcb[i].is_browse_active) {
            active_rc_hdl = i;
          }
        }
        if (active_rc_hdl != -1) {
          tBTA_AV_RCB* p_rcb;
          RawAddress prev_active_addr = RawAddress::kEmpty;
          for (int i = 0; i < BTA_AV_NUM_RCB; i++) {
            p_rcb = &p_cb->rcb[i];
            if (active_rc_hdl == p_rcb->handle) {
              if (p_rcb->shdl > 0) {
                tBTA_AV_SCB* p_scb = bta_av_cb.p_scb[p_rcb->shdl - 1];
                prev_active_addr = p_scb->peer_addr;
              }
            }
          }
          APPL_TRACE_WARNING("%s: Already active Addr: %s", __func__,
                  prev_active_addr.ToString().c_str());
          int active_idx = active_device_priority_list_get_idx(prev_active_addr);
          APPL_TRACE_WARNING("%s: prev active idx %d, cur idx %d", __func__, active_idx, found_idx);
          if (found_idx < active_idx) {
            APPL_TRACE_WARNING("%s: close for hdl %d", __func__, rc_handle);
            AVRC_CloseBrowse(rc_handle);
          } else {
            if (rc_handle >= 0 && rc_handle < BTA_AV_NUM_RCB) {
              is_active_set = true;
              bta_av_set_peer_browse_active(rc_handle);
            }
          }
        } else {
          if (rc_handle >= 0 && rc_handle < BTA_AV_NUM_RCB) {
            is_active_set = true;
            bta_av_set_peer_browse_active(rc_handle);
          }
        }
      }
      break;

    case BTA_AV_BROWSE_ACTIVE:
      if (device_exist) {
        if (rc_handle >= 0 && rc_handle < BTA_AV_NUM_RCB &&
            p_cb->rcb[rc_handle].peer_features & BTA_AV_FEAT_BROWSE) {
          int last = device_list_size - 1;
          std::swap(active_device_priority_list[found_idx], active_device_priority_list[last]);
          is_active_set = true;
          bta_av_set_peer_browse_active(rc_handle);
          APPL_TRACE_WARNING("%s: open browse for hdl %d", __func__, rc_handle);
          AVRC_OpenBrowse(rc_handle, AVCT_INT);
        }
      } else {
        if (rc_handle != BTA_AV_RC_HANDLE_NONE && device_list_size < BTA_AV_NUM_LINKS) {
          if (rc_handle >= 0 && rc_handle < BTA_AV_NUM_RCB &&
              p_cb->rcb[rc_handle].peer_features & BTA_AV_FEAT_BROWSE) {
            is_active_set = true;
            bta_av_set_peer_browse_active(rc_handle);
          }
        }
        if (device_list_size < BTA_AV_NUM_LINKS)
          active_device_priority_list.push_back(bd_addr);
      }
      break;

    case BTA_AV_BROWSE_HANDOFF:
      if (device_exist) {
        if (rc_handle >= 0 && rc_handle < BTA_AV_NUM_RCB &&
            p_cb->rcb[rc_handle].peer_features & BTA_AV_FEAT_BROWSE) {
          int last = device_list_size - 1;
          std::swap(active_device_priority_list[found_idx], active_device_priority_list[last]);
          is_active_set = true;
          bta_av_set_peer_browse_active(rc_handle);
          APPL_TRACE_WARNING("%s: open browse for handle %d", __func__, rc_handle);
          AVRC_OpenBrowse(rc_handle, AVCT_INT);
        }
      } else {
        if (rc_handle != BTA_AV_RC_HANDLE_NONE && device_list_size < BTA_AV_NUM_LINKS) {
          if (rc_handle >= 0 && rc_handle < BTA_AV_NUM_RCB &&
              p_cb->rcb[rc_handle].peer_features & BTA_AV_FEAT_BROWSE) {
            bta_av_set_peer_browse_active(rc_handle);
            is_active_set = true;
          }
        }
        if (device_list_size < BTA_AV_NUM_LINKS)
          active_device_priority_list.push_back(bd_addr);
      }
      break;

    default:
      break;
  }

  if (is_active_set) {
    APPL_TRACE_WARNING("%s: Send Browse active status to %s", __func__, bd_addr.ToString().c_str());
    tBTA_AV_RC_BROWSE_OPEN rc_browse_open;

    rc_browse_open.status = BTA_AV_SUCCESS_BR_HANDOFF;
    rc_browse_open.rc_handle = rc_handle;
    rc_browse_open.peer_addr = bd_addr;

    tBTA_AV bta_av_data;
    bta_av_data.rc_browse_open = rc_browse_open;
    (*p_cb->p_cback)(BTA_AV_RC_BROWSE_OPEN_EVT, &bta_av_data);
  }
}

/*******************************************************************************
 *
 * Function         bta_av_rc_closed
 *
 * Description      Set AVRCP state to closed.
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_av_rc_closed(tBTA_AV_DATA* p_data) {
  tBTA_AV_CB* p_cb = &bta_av_cb;
  tBTA_AV_RC_CLOSE rc_close;
  tBTA_AV_RC_CONN_CHG* p_msg = (tBTA_AV_RC_CONN_CHG*)p_data;
  tBTA_AV_RCB* p_rcb;
  tBTA_AV_SCB* p_scb;
  int i;
  bool conn = false;
  tBTA_AV_LCB* p_lcb;
  bool browse_support = false;

  rc_close.rc_handle = BTA_AV_RC_HANDLE_NONE;
  p_scb = NULL;
  APPL_TRACE_DEBUG("bta_av_rc_closed rc_handle:%d, peer_addr:%s",
                  p_msg->handle, p_msg->peer_addr.ToString().c_str());
  for (i = 0; i < BTA_AV_NUM_RCB; i++) {
    p_rcb = &p_cb->rcb[i];
    APPL_TRACE_DEBUG("bta_av_rc_closed rcb[%d] rc_handle:%d, status=0x%x", i,
                     p_rcb->handle, p_rcb->status);
    if (p_rcb->handle == p_msg->handle) {
      rc_close.rc_handle = i;
      p_rcb->status &= ~BTA_AV_RC_CONN_MASK;
      p_rcb->peer_features = 0;
      if(p_rcb->browse_open && (p_rcb->peer_features & BTA_AV_FEAT_BROWSE))
        browse_support = true;
      APPL_TRACE_DEBUG("shdl:%d, lidx:%d", p_rcb->shdl, p_rcb->lidx);
      if (p_rcb->shdl) {
        if ((p_rcb->shdl - 1) < BTA_AV_NUM_STRS) {
          p_scb = bta_av_cb.p_scb[p_rcb->shdl - 1];
        }
        if (p_scb) {
          rc_close.peer_addr = p_scb->peer_addr;
          if (p_scb->rc_handle == p_rcb->handle)
            p_scb->rc_handle = BTA_AV_RC_HANDLE_NONE;
            p_scb->rc_ccb_alloc_handle = p_scb->rc_handle;
          APPL_TRACE_DEBUG("shdl:%d, srch:%d", p_rcb->shdl, p_scb->rc_handle);
        } else {
          APPL_TRACE_DEBUG("%s: p_scb is NULL", __func__);
          rc_close.peer_addr = p_msg->peer_addr;
        }
        p_rcb->shdl = 0;
      } else if (p_rcb->lidx == (BTA_AV_NUM_LINKS + 1)) {
        /* if the RCB uses the extra LCB, use the addr for event and clean it */
        p_lcb = &p_cb->lcb[BTA_AV_NUM_LINKS];
        rc_close.peer_addr = p_msg->peer_addr;
        VLOG(1) << "rc_only closed bd_addr:" << p_msg->peer_addr;
        p_lcb->conn_msk = 0;
        p_lcb->lidx = 0;
      }
      p_rcb->lidx = 0;
      if (p_cb->disc && ((p_cb->disc & (~BTA_AV_CHNL_MSK)) == p_rcb->handle)) {
        APPL_TRACE_WARNING("%s: clear RC discovery in avrcp close disc: x%x",
                   __func__, p_cb->disc );
        SDP_CancelServiceSearch(p_cb->p_disc_db);
        p_cb->disc = 0;
        osi_free_and_reset((void **)&p_cb->p_disc_db);
      }

      if ((p_rcb->status & BTA_AV_RC_ROLE_MASK) == BTA_AV_RC_ROLE_INT) {
        /* AVCT CCB is deallocated */
        p_rcb->handle = BTA_AV_RC_HANDLE_NONE;
        p_rcb->status = 0;
        p_rcb->is_browse_active = false;
        p_rcb->peer_addr = RawAddress::kEmpty;
      } else {
        /* AVCT CCB is still there. dealloc */
        bta_av_del_rc(p_rcb);
      }
      int idx = active_device_priority_list_get_idx(rc_close.peer_addr);
      std::vector<RawAddress>::iterator it = active_device_priority_list.begin();
      if (idx != -1) {
        APPL_TRACE_WARNING("Remove Addr is %s", rc_close.peer_addr.ToString().c_str());
        APPL_TRACE_WARNING("Remove Addr is %s", (*(it + idx)).ToString().c_str());
        active_device_priority_list.erase(it + idx);
      }
    } else if ((p_rcb->handle != BTA_AV_RC_HANDLE_NONE) &&
               (p_rcb->status & BTA_AV_RC_CONN_MASK)) {
      /* at least one channel is still connected */
      conn = true;
    }
    APPL_TRACE_DEBUG("bta_av_rc_closed rcb[%d] rc_handle:%d, status=0x%x", i,
                     p_rcb->handle, p_rcb->status);
  }

  if (!conn) {
    /* no AVRC channels are connected, go back to INIT state */
    bta_av_sm_execute(p_cb, BTA_AV_AVRC_NONE_EVT, NULL);
  }

  if (rc_close.rc_handle == BTA_AV_RC_HANDLE_NONE) {
    rc_close.rc_handle = p_msg->handle;
    rc_close.peer_addr = p_msg->peer_addr;
  }
  APPL_TRACE_DEBUG("bta_av_rc_closed rc_close handle:%d, peer_addr:%s",
              rc_close.rc_handle, rc_close.peer_addr.ToString().c_str());
  tBTA_AV bta_av_data;
  bta_av_data.rc_close = rc_close;
  (*p_cb->p_cback)(BTA_AV_RC_CLOSE_EVT, &bta_av_data);

  /*blocking rc_create for acceptor handle if peer is supporting brwosing connection
  waiting to close browsing connection*/
  if (!browse_support && (bta_av_cb.rc_acp_handle == BTA_AV_RC_HANDLE_NONE) &&
     (bta_av_cb.features & BTA_AV_FEAT_RCTG))
      bta_av_rc_create(&bta_av_cb, AVCT_ACP, 0, BTA_AV_NUM_LINKS + 1, NULL);
  else if (p_msg->status) {
      bta_av_rc_create(&bta_av_cb, AVCT_ACP, 0, BTA_AV_NUM_LINKS + 1, &p_msg->peer_addr);
  }
}

/*******************************************************************************
 *
 * Function         bta_av_rc_browse_opened
 *
 * Description      AVRC browsing channel is opened
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_av_rc_browse_opened(tBTA_AV_DATA* p_data) {
  tBTA_AV_CB* p_cb = &bta_av_cb;
  tBTA_AV_RC_CONN_CHG* p_msg = (tBTA_AV_RC_CONN_CHG*)p_data;
  tBTA_AV_RC_BROWSE_OPEN rc_browse_open;

  VLOG(1) << "bta_av_rc_browse_opened bd_addr:" << p_msg->peer_addr;
  APPL_TRACE_DEBUG("bta_av_rc_browse_opened rc_handle:%d", p_msg->handle);

  rc_browse_open.status = BTA_AV_SUCCESS;
  rc_browse_open.rc_handle = p_msg->handle;
  rc_browse_open.peer_addr = p_msg->peer_addr;

  tBTA_AV bta_av_data;
  bta_av_data.rc_browse_open = rc_browse_open;
  (*p_cb->p_cback)(BTA_AV_RC_BROWSE_OPEN_EVT, &bta_av_data);
}

/*******************************************************************************
 *
 * Function         bta_av_rc_browse_closed
 *
 * Description      AVRC browsing channel is closed
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_av_rc_browse_closed(tBTA_AV_DATA* p_data) {
  tBTA_AV_CB* p_cb = &bta_av_cb;
  tBTA_AV_RC_CONN_CHG* p_msg = (tBTA_AV_RC_CONN_CHG*)p_data;
  tBTA_AV_RC_BROWSE_CLOSE rc_browse_close;
  tBTA_AV_RCB* p_rcb;

  VLOG(1) << "bta_av_rc_browse_closed bd_addr:" << p_msg->peer_addr;
  APPL_TRACE_DEBUG("bta_av_rc_browse_closed rc_handle:%d", p_msg->handle);

  rc_browse_close.rc_handle = p_msg->handle;
  rc_browse_close.peer_addr = p_msg->peer_addr;
  p_rcb = &p_cb->rcb[p_msg->handle];
  /*if handle in none means rc cleaup is triggered but due to
  browsing bcb it is not finished. So again celaning RC for handler*/
  if(p_rcb->handle == BTA_AV_RC_HANDLE_NONE) {
    p_rcb->handle = p_msg->handle;
    bta_av_del_rc(p_rcb);
    p_rcb->handle = BTA_AV_RC_HANDLE_NONE;
    if (p_rcb->browse_open && (bta_av_cb.rc_acp_handle ==
        BTA_AV_RC_HANDLE_NONE) && (bta_av_cb.features & BTA_AV_FEAT_RCTG))
      bta_av_rc_create(&bta_av_cb, AVCT_ACP, 0, BTA_AV_NUM_LINKS + 1, NULL);
  }
  p_rcb->browse_open = false;
  tBTA_AV bta_av_data;
  bta_av_data.rc_browse_close = rc_browse_close;
  (*p_cb->p_cback)(BTA_AV_RC_BROWSE_CLOSE_EVT, &bta_av_data);
}

/*******************************************************************************
 *
 * Function         bta_av_rc_disc
 *
 * Description      start AVRC SDP discovery.
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_av_rc_disc(uint8_t disc) {
  tBTA_AV_CB* p_cb = &bta_av_cb;
  tAVRC_SDP_DB_PARAMS db_params;
  uint16_t attr_list[] = {ATTR_ID_SERVICE_CLASS_ID_LIST,
                          ATTR_ID_PROTOCOL_DESC_LIST,
                          ATTR_ID_ADDITION_PROTO_DESC_LISTS,
                          ATTR_ID_BT_PROFILE_DESC_LIST,
                          ATTR_ID_SUPPORTED_FEATURES};
  uint8_t hdi;
  tBTA_AV_SCB* p_scb;
  RawAddress* p_addr = NULL;
  uint8_t rc_handle;

  APPL_TRACE_DEBUG("%s: 0x%x, %d", __func__, disc, bta_av_cb.disc);
  if ((bta_av_cb.disc != 0) || (disc == 0)) return;

  if ((disc & BTA_AV_CHNL_MSK) == BTA_AV_CHNL_MSK) {
    /* this is the rc handle/index to tBTA_AV_RCB */
    rc_handle = disc & (~BTA_AV_CHNL_MSK);
    if (p_cb->rcb[rc_handle].lidx) {
      p_addr = &p_cb->lcb[p_cb->rcb[rc_handle].lidx - 1].addr;
    }
  } else {
    hdi = (disc & BTA_AV_HNDL_MSK) - 1;
    APPL_TRACE_DEBUG("%s: hdi: %d", __func__, hdi);
    p_scb = p_cb->p_scb[hdi];

    if (p_scb) {
      APPL_TRACE_DEBUG("%s: rc_handle: %d, peer_addr: %s", __func__,
                p_scb->rc_handle, p_scb->peer_addr.ToString().c_str());
      p_addr = &p_scb->peer_addr;
    }
  }

  if (p_addr) {
    /* allocate discovery database */
    if (p_cb->p_disc_db == NULL)
      p_cb->p_disc_db = (tSDP_DISCOVERY_DB*)osi_malloc(BTA_AV_DISC_BUF_SIZE);

    /* set up parameters */
    db_params.db_len = BTA_AV_DISC_BUF_SIZE;
    db_params.num_attr = sizeof(attr_list) / sizeof(uint16_t);
    db_params.p_db = p_cb->p_disc_db;
    db_params.p_attrs = attr_list;

    /* searching for UUID_SERVCLASS_AV_REMOTE_CONTROL gets both TG and CT */
    if (AVRC_FindService(UUID_SERVCLASS_AV_REMOTE_CONTROL, *p_addr, &db_params,
                         bta_av_avrc_sdp_cback) == AVRC_SUCCESS) {
      p_cb->disc = disc;
      APPL_TRACE_DEBUG("disc %d", p_cb->disc);
    }
  }
}

void bta_av_rc_retry_disc(tBTA_AV_DATA* p_data)
{
  uint8_t disc = p_data->hdr.layer_specific;
  APPL_TRACE_IMP("%s: disc=%d", __func__, disc);
  bta_av_rc_disc(disc);
}

/*******************************************************************************
 *
 * Function         bta_av_dereg_comp
 *
 * Description      deregister complete. free the stream control block.
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_av_dereg_comp(tBTA_AV_DATA* p_data) {
  tBTA_AV_CB* p_cb = &bta_av_cb;
  tBTA_AV_SCB* p_scb;
  tBTA_UTL_COD cod;
  uint8_t mask;
  BT_HDR* p_buf;

  /* find the stream control block */
  p_scb = bta_av_hndl_to_scb(p_data->hdr.layer_specific);

  if (p_scb) {
    APPL_TRACE_DEBUG("deregistered %d(h%d)", p_scb->chnl, p_scb->hndl);
    mask = BTA_AV_HNDL_TO_MSK(p_scb->hdi);
    if (p_scb->chnl == BTA_AV_CHNL_AUDIO) {
      p_cb->reg_audio &= ~mask;
      if ((p_cb->conn_audio & mask) && bta_av_cb.audio_open_cnt) {
        /* this channel is still marked as open. decrease the count */
        bta_av_cb.audio_open_cnt--;
      }
      p_cb->conn_audio &= ~mask;

      if (p_scb->q_tag == BTA_AV_Q_TAG_STREAM && p_scb->a2dp_list) {
        /* make sure no buffers are in a2dp_list */
        while (!list_is_empty(p_scb->a2dp_list)) {
          p_buf = (BT_HDR*)list_front(p_scb->a2dp_list);
          list_remove(p_scb->a2dp_list, p_buf);
          osi_free(p_buf);
        }
      }

      /* remove the A2DP SDP record, if no more audio stream is left */
      if (!p_cb->reg_audio) {
#if (BTA_AR_INCLUDED == TRUE)
        bta_ar_dereg_avrc(UUID_SERVCLASS_AV_REMOTE_CONTROL, BTA_ID_AV);
#endif
        if (p_cb->sdp_a2dp_handle) {
          bta_av_del_sdp_rec(&p_cb->sdp_a2dp_handle);
          p_cb->sdp_a2dp_handle = 0;
          bta_sys_remove_uuid(UUID_SERVCLASS_AUDIO_SOURCE);
        }

#if (BTA_AV_SINK_INCLUDED == TRUE)
        if (p_cb->sdp_a2dp_snk_handle) {
          bta_av_del_sdp_rec(&p_cb->sdp_a2dp_snk_handle);
          p_cb->sdp_a2dp_snk_handle = 0;
          bta_sys_remove_uuid(UUID_SERVCLASS_AUDIO_SINK);
        }
#endif
      }
    } else {
      p_cb->reg_video &= ~mask;
      /* make sure that this channel is not connected */
      p_cb->conn_video &= ~mask;
      /* remove the VDP SDP record, (only one video stream at most) */
      bta_av_del_sdp_rec(&p_cb->sdp_vdp_handle);
      bta_sys_remove_uuid(UUID_SERVCLASS_VIDEO_SOURCE);
    }

    /* make sure that the timer is not active */
    alarm_cancel(p_scb->avrc_ct_timer);
    osi_free_and_reset((void**)&p_cb->p_scb[p_scb->hdi]);
  }

  APPL_TRACE_DEBUG("audio 0x%x, video: 0x%x, disable:%d", p_cb->reg_audio,
                   p_cb->reg_video, p_cb->disabling);
  /* if no stream control block is active */
  if ((p_cb->reg_audio + p_cb->reg_video) == 0) {
#if (BTA_AR_INCLUDED == TRUE)
    /* deregister from AVDT */
    bta_ar_dereg_avdt(BTA_ID_AV);

    /* deregister from AVCT */
    bta_ar_dereg_avrc(UUID_SERVCLASS_AV_REM_CTRL_TARGET, BTA_ID_AV);
    bta_ar_dereg_avct(BTA_ID_AV);
#endif

    if (p_cb->disabling) {
      p_cb->disabling = false;
      bta_av_cb.features = 0;
    }

    /* Clear the Capturing service class bit */
    cod.service = BTM_COD_SERVICE_CAPTURING;
    utl_set_device_class(&cod, BTA_UTL_CLR_COD_SERVICE_CLASS);
  }
}

/*******************************************************************************
 *
 * Function         bta_av_browsing_channel_open_retry
 *
 * Description      Retry to AVRCP command to open browsing channel.
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_av_browsing_channel_open_retry(uint8_t handle) {
  APPL_TRACE_IMP("%s Retry Browse connection", __func__);
  AVRC_OpenBrowse(handle, AVCT_INT);
}

/*******************************************************************************
 *
 * Function         bta_av_refresh_accept_signalling_timer
 *
 * Description      Refresh accept_signalling_timer
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_av_refresh_accept_signalling_timer(const RawAddress &remote_bdaddr) {
  tBTA_AV_SCB* p_scb = NULL;
  p_scb = bta_av_addr_to_scb(remote_bdaddr);
  if (p_scb == NULL) {
    APPL_TRACE_IMP("%s: p_scb is null, return", __func__);
    return;
  }
  APPL_TRACE_IMP("%s: add: %s hdi = %d", __func__,
                           remote_bdaddr.ToString().c_str(), p_scb->hdi);
  if (alarm_is_scheduled(bta_av_cb.accept_signalling_timer[p_scb->hdi])) {
    APPL_TRACE_IMP("%s:accept_signalling_timer is scheduled on p_scb->hdi: %d"
                     " cancel it, and restart", __func__, p_scb->hdi);
    alarm_set_on_mloop(bta_av_cb.accept_signalling_timer[p_scb->hdi],
              bta_sink_time_out(), bta_av_accept_signalling_timer_cback,
              UINT_TO_PTR(p_scb->hdi));
  }
}
