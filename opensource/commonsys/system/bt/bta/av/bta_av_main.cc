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
 *  This is the main implementation file for the BTA advanced audio/video.
 *
 ******************************************************************************/

#define LOG_TAG "bt_bta_av"

#include <base/logging.h>
#include <string.h>

#include "bt_target.h"
#include "osi/include/log.h"
#include "osi/include/osi.h"
#include "osi/include/properties.h"

#include "bta_av_co.h"
#include "bta_av_int.h"
#include "btm_int.h"
#include "btif/include/btif_av_co.h"
#include "l2c_api.h"
#include "l2cdefs.h"
#include "utl.h"

#if (BTA_AR_INCLUDED == TRUE)
#include "bta_ar_api.h"
#endif

/*****************************************************************************
 * Constants and types
 ****************************************************************************/

#ifndef BTA_AV_RET_TOUT
#define BTA_AV_RET_TOUT 4
#endif

#ifndef BTA_AV_SIG_TOUT
#define BTA_AV_SIG_TOUT 4
#endif

#ifndef BTA_AV_IDLE_TOUT
#define BTA_AV_IDLE_TOUT 10
#endif

/* the delay time in milliseconds to retry role switch */
#ifndef BTA_AV_RS_TIME_VAL
#define BTA_AV_RS_TIME_VAL 1000
#endif

extern bool tws_state_supported;
/* state machine states */
enum { BTA_AV_INIT_ST, BTA_AV_OPEN_ST };

/* state machine action enumeration list */
enum {
  BTA_AV_DISABLE,
  BTA_AV_RC_OPENED,
  BTA_AV_RC_REMOTE_CMD,
  BTA_AV_RC_VENDOR_CMD,
  BTA_AV_RC_VENDOR_RSP,
  BTA_AV_RC_FREE_RSP,
  BTA_AV_RC_FREE_BROWSE_MSG,
  BTA_AV_RC_META_RSP,
  BTA_AV_RC_MSG,
  BTA_AV_RC_CLOSE,
  BTA_AV_RC_BROWSE_CLOSE,
  BTA_AV_NUM_ACTIONS
};

#define BTA_AV_IGNORE BTA_AV_NUM_ACTIONS

/* type for action functions */
typedef void (*tBTA_AV_ACTION)(tBTA_AV_CB* p_cb, tBTA_AV_DATA* p_data);

/* action functions */
const tBTA_AV_ACTION bta_av_action[] = {
    bta_av_disable,
    bta_av_rc_opened,
    bta_av_rc_remote_cmd,
    bta_av_rc_vendor_cmd,
    bta_av_rc_vendor_rsp,
    bta_av_rc_free_rsp,
    bta_av_rc_free_browse_msg,
    bta_av_rc_meta_rsp,
    bta_av_rc_msg,
    bta_av_rc_close,
};

/* state table information */
#define BTA_AV_ACTION_COL 0 /* position of actions */
#define BTA_AV_NEXT_STATE 1 /* position of next state */
#define BTA_AV_NUM_COLS 2   /* number of columns in state tables */

/* state table for init state */
static const uint8_t bta_av_st_init[][BTA_AV_NUM_COLS] = {
    /* Event                     Action 1                   Next state */
    /* API_DISABLE_EVT */ {BTA_AV_DISABLE, BTA_AV_INIT_ST},
    /* API_REMOTE_CMD_EVT */ {BTA_AV_IGNORE, BTA_AV_INIT_ST},
    /* API_VENDOR_CMD_EVT */ {BTA_AV_IGNORE, BTA_AV_INIT_ST},
    /* API_VENDOR_RSP_EVT */ {BTA_AV_IGNORE, BTA_AV_INIT_ST},
    /* API_META_RSP_EVT */ {BTA_AV_RC_FREE_RSP, BTA_AV_INIT_ST},
    /* API_RC_CLOSE_EVT */ {BTA_AV_RC_CLOSE, BTA_AV_INIT_ST},
    /* AVRC_OPEN_EVT */ {BTA_AV_RC_OPENED, BTA_AV_OPEN_ST},
    /* AVRC_MSG_EVT */ {BTA_AV_RC_FREE_BROWSE_MSG, BTA_AV_INIT_ST},
    /* AVRC_NONE_EVT */ {BTA_AV_IGNORE, BTA_AV_INIT_ST},
};

/* state table for open state */
static const uint8_t bta_av_st_open[][BTA_AV_NUM_COLS] = {
    /* Event                     Action 1                   Next state */
    /* API_DISABLE_EVT */ {BTA_AV_DISABLE, BTA_AV_INIT_ST},
    /* API_REMOTE_CMD_EVT */ {BTA_AV_RC_REMOTE_CMD, BTA_AV_OPEN_ST},
    /* API_VENDOR_CMD_EVT */ {BTA_AV_RC_VENDOR_CMD, BTA_AV_OPEN_ST},
    /* API_VENDOR_RSP_EVT */ {BTA_AV_RC_VENDOR_RSP, BTA_AV_OPEN_ST},
    /* API_META_RSP_EVT */ {BTA_AV_RC_META_RSP, BTA_AV_OPEN_ST},
    /* API_RC_CLOSE_EVT */ {BTA_AV_RC_CLOSE, BTA_AV_OPEN_ST},
    /* AVRC_OPEN_EVT */ {BTA_AV_RC_OPENED, BTA_AV_OPEN_ST},
    /* AVRC_MSG_EVT */ {BTA_AV_RC_MSG, BTA_AV_OPEN_ST},
    /* AVRC_NONE_EVT */ {BTA_AV_IGNORE, BTA_AV_INIT_ST},
};

/* type for state table */
typedef const uint8_t (*tBTA_AV_ST_TBL)[BTA_AV_NUM_COLS];

/* state table */
static const tBTA_AV_ST_TBL bta_av_st_tbl[] = {bta_av_st_init, bta_av_st_open};

typedef void (*tBTA_AV_NSM_ACT)(tBTA_AV_DATA* p_data);
static void bta_av_api_enable(tBTA_AV_DATA* p_data);
static void bta_av_api_register(tBTA_AV_DATA* p_data);
static void bta_av_ci_data(tBTA_AV_DATA* p_data);
#if (AVDT_REPORTING == TRUE)
static void bta_av_rpc_conn(tBTA_AV_DATA* p_data);
#endif
static void bta_av_api_to_ssm(tBTA_AV_DATA* p_data);

static void bta_av_sco_chg_cback(tBTA_SYS_CONN_STATUS status, uint8_t id,
                                 uint8_t app_id, const RawAddress& peer_addr);
static void bta_av_sys_rs_cback(tBTA_SYS_CONN_STATUS status, uint8_t id,
                                uint8_t app_id, const RawAddress& peer_addr);

static void bta_av_api_enable_multicast(tBTA_AV_DATA *p_data);
static void bta_av_api_update_max_av_clients(tBTA_AV_DATA * p_data);

bool bta_av_multiple_streams_started(void);

extern int btif_get_is_remote_started_idx();
extern int btif_max_av_clients;
#if (TWS_ENABLED == TRUE)
#if (TWS_STATE_ENABLED == TRUE)
static void bta_av_api_set_tws_earbud_state(tBTA_AV_DATA * p_data);
#endif
static void bta_av_api_set_tws_earbud_role(tBTA_AV_DATA * p_data);
static void bta_av_api_set_is_tws_device(tBTA_AV_DATA * p_data);
#endif

/* action functions */
const tBTA_AV_NSM_ACT bta_av_nsm_act[] = {
    bta_av_api_enable,       /* BTA_AV_API_ENABLE_EVT */
    bta_av_api_register,     /* BTA_AV_API_REGISTER_EVT */
    bta_av_api_deregister,   /* BTA_AV_API_DEREGISTER_EVT */
    bta_av_api_disconnect,   /* BTA_AV_API_DISCONNECT_EVT */
    bta_av_ci_data,          /* BTA_AV_CI_SRC_DATA_READY_EVT */
    bta_av_sig_chg,          /* BTA_AV_SIG_CHG_EVT */
    bta_av_signalling_timer, /* BTA_AV_SIGNALLING_TIMER_EVT */
    bta_av_rc_disc_done,     /* BTA_AV_SDP_AVRC_DISC_EVT */
    bta_av_rc_closed,        /* BTA_AV_AVRC_CLOSE_EVT */
    bta_av_rc_browse_opened, /* BTA_AV_AVRC_BROWSE_OPEN_EVT */
    bta_av_rc_browse_closed, /* BTA_AV_AVRC_BROWSE_CLOSE_EVT */
    bta_av_rc_retry_disc,    /* BTA_AV_AVRC_RETRY_DISC_EVT */
    bta_av_conn_chg,         /* BTA_AV_CONN_CHG_EVT */
    bta_av_dereg_comp,       /* BTA_AV_DEREG_COMP_EVT */
    bta_av_active_browse,    /* BTA_AV_BROWSE_ACTIVE_EVT */
#if (AVDT_REPORTING == TRUE)
    bta_av_rpc_conn, /* BTA_AV_AVDT_RPT_CONN_EVT */
#endif
    bta_av_api_to_ssm, /* BTA_AV_API_START_EVT */
    bta_av_api_to_ssm, /* BTA_AV_API_STOP_EVT */
    bta_av_api_update_max_av_clients,
    bta_av_api_enable_multicast,    /* BTA_AV_ENABLE_MULTICAST_EVT */
    bta_av_rc_collission_detected, /* BTA_AV_RC_COLLISSION_DETECTED_EVT */
    bta_av_update_enc_mode, /* BTA_AV_UPDATE_ENCODER_MODE_EVT */
    bta_av_update_aptx_data,     /* BTA_AV_UPDATE_APTX_DATA_EVT */
#if (TWS_ENABLED == TRUE)
#if (TWS_STATE_ENABLED == TRUE)
    bta_av_api_set_tws_earbud_state, /* BTA_AV_SET_EARBUD_STATE_EVT */
#endif
    bta_av_api_set_tws_earbud_role, /* BTA_AV_SET_EARBUD_ROLE_EVT */
    bta_av_api_set_is_tws_device, /* BTA_AV_SET_TWS_DEVICE_EVT */
#endif
};

/*****************************************************************************
 * Global data
 ****************************************************************************/

/* AV control block */
tBTA_AV_CB bta_av_cb;
tBT_VENDOR_A2DP_OFFLOAD offload_start;
static const char* bta_av_st_code(uint8_t state);

static bool is_multicast_enabled = false;
/*******************************************************************************
 *
 * Function         bta_av_api_enable
 *
 * Description      Handle an API enable event.
 *
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_av_api_enable(tBTA_AV_DATA* p_data) {
  /* initialize control block */
  memset(&bta_av_cb, 0, sizeof(tBTA_AV_CB));

  for (int i = 0; i < BTA_AV_NUM_RCB; i++)
    bta_av_cb.rcb[i].handle = BTA_AV_RC_HANDLE_NONE;

  bta_av_cb.rc_acp_handle = BTA_AV_RC_HANDLE_NONE;

  /*
   * TODO: The "disable" event handling is missing - there we need
   * to alarm_free() the alarms below.
   */
  bta_av_cb.link_signalling_timer = alarm_new("bta_av.link_signalling_timer");
  for (int j = 0; j < BTA_AV_NUM_STRS; j++)
  {
    bta_av_cb.accept_signalling_timer[j] =
      alarm_new("bta_av.accept_signalling_timer");
  }

  bta_av_cb.browsing_channel_open_timer =
  alarm_new("bta_av.browsing_channel_open_timer");
  /* store parameters */
  bta_av_cb.p_cback = p_data->api_enable.p_cback;
  bta_av_cb.features = p_data->api_enable.features;
  bta_av_cb.sec_mask = p_data->api_enable.sec_mask;

  tBTA_AV_ENABLE enable;
  enable.features = bta_av_cb.features;

  /* Register for SCO change event */
  if (!(bta_av_cb.features & BTA_AV_FEAT_NO_SCO_SSPD)) {
    bta_sys_sco_register(bta_av_sco_chg_cback);
  }

  /* call callback with enable event */
  tBTA_AV bta_av_data;
  bta_av_data.enable = enable;
  (*bta_av_cb.p_cback)(BTA_AV_ENABLE_EVT, &bta_av_data);
}

/*******************************************************************************
 *
 * Function         bta_av_addr_to_scb
 *
 * Description      find the stream control block by the peer addr
 *
 * Returns          void
 *
 ******************************************************************************/
tBTA_AV_SCB* bta_av_addr_to_scb(const RawAddress& bd_addr) {
  tBTA_AV_SCB* p_scb = NULL;
  int xx;

  for (xx = 0; xx < BTA_AV_NUM_STRS; xx++) {
    if (bta_av_cb.p_scb[xx]) {
      if (bd_addr == bta_av_cb.p_scb[xx]->peer_addr) {
        p_scb = bta_av_cb.p_scb[xx];
        break;
      }
    }
  }
  return p_scb;
}

/*******************************************************************************
 *
 * Function         bta_av_hndl_to_scb
 *
 * Description      find the stream control block by the handle
 *
 * Returns          void
 *
 ******************************************************************************/
tBTA_AV_SCB* bta_av_hndl_to_scb(uint16_t handle) {
  tBTA_AV_HNDL hndl = (tBTA_AV_HNDL)handle;
  tBTA_AV_SCB* p_scb = NULL;
  uint8_t idx = (hndl & BTA_AV_HNDL_MSK);

  if (idx && (idx <= BTA_AV_NUM_STRS)) {
    p_scb = bta_av_cb.p_scb[idx - 1];
  }
  return p_scb;
}

/*******************************************************************************
**
** Function         bta_avk_is_avdt_sync
**
** Description      If the current connection supports AVDT1.3
**
** Returns          true for supports AVDT1.3, false for not.
**
*******************************************************************************/
bool bta_avk_is_avdt_sync(uint16_t handle) {
  tBTA_AV_SCB* p_scb = bta_av_hndl_to_scb(handle);
  if (p_scb && (p_scb->avdt_version >= AVDT_VERSION_SYNC))
    return true;
  else
    return false;
}

/*******************************************************************************
 *
 * Function         bta_av_alloc_scb
 *
 * Description      allocate stream control block,
 *                  register the service to stack
 *                  create SDP record
 *
 * Returns          void
 *
 ******************************************************************************/
static tBTA_AV_SCB* bta_av_alloc_scb(tBTA_AV_CHNL chnl) {
  tBTA_AV_SCB* p_ret = NULL;
  int xx;
  tBTA_AV_STATUS sts = BTA_AV_SUCCESS;

  if (chnl == BTA_AV_CHNL_VIDEO) {
    if (p_bta_av_cfg->p_act_tbl == NULL || p_bta_av_cfg->p_reg == NULL) {
      APPL_TRACE_ERROR("Video streaming not supported");
      sts = BTA_AV_FAIL;
    } else {
      /* allow only one Video channel */
      if (bta_av_cb.reg_video) {
        APPL_TRACE_ERROR("Already registered");
        sts = BTA_AV_FAIL;
      }
    }
  } else if (chnl != BTA_AV_CHNL_AUDIO) {
    APPL_TRACE_ERROR("bad channel: %d", chnl);
    sts = BTA_AV_FAIL;
  }

  if (sts == BTA_AV_SUCCESS) {
    for (xx = 0; xx < BTA_AV_NUM_STRS; xx++) {
      if (bta_av_cb.p_scb[xx] == NULL) {
        /* found an empty spot */
        p_ret = (tBTA_AV_SCB*)osi_calloc(sizeof(tBTA_AV_SCB));
        p_ret->rc_handle = BTA_AV_RC_HANDLE_NONE;
        p_ret->chnl = chnl;
        p_ret->hndl = (tBTA_AV_HNDL)((xx + 1) | chnl);
        p_ret->hdi = xx;
        p_ret->a2dp_list = list_new(NULL);
        p_ret->avrc_ct_timer = alarm_new("bta_av.avrc_ct_timer");
        p_ret->cache_setconfig = NULL;
        p_ret->rc_ccb_alloc_handle = BTA_AV_RC_HANDLE_NONE;
        bta_av_cb.p_scb[xx] = p_ret;
        APPL_TRACE_EVENT("AV: Alloc success, handle is =%d", p_ret->hndl);
        break;
      }
    }
  }
  return p_ret;
}

/*******************************************************************************
 ******************************************************************************/
void bta_av_conn_cback(UNUSED_ATTR uint8_t handle, const RawAddress* bd_addr,
                       uint8_t event, tAVDT_CTRL* p_data) {
  uint16_t evt = 0;
  tBTA_AV_SCB* p_scb = NULL;

#if (BTA_AR_INCLUDED == TRUE)
  if (event == BTA_AR_AVDT_CONN_EVT || event == AVDT_CONNECT_IND_EVT ||
      event == AVDT_DISCONNECT_IND_EVT)
#else
  if (event == AVDT_CONNECT_IND_EVT || event == AVDT_DISCONNECT_IND_EVT)
#endif
  {
    evt = BTA_AV_SIG_CHG_EVT;
    if (event == AVDT_DISCONNECT_IND_EVT) {
      p_scb = bta_av_addr_to_scb(*bd_addr);
    } else if (event == AVDT_CONNECT_IND_EVT) {
      APPL_TRACE_DEBUG("%s: CONN_IND is ACP:%d", __func__,
                       p_data->hdr.err_param);
    }

    tBTA_AV_STR_MSG* p_msg =
        (tBTA_AV_STR_MSG*)osi_malloc(sizeof(tBTA_AV_STR_MSG));
    p_msg->hdr.event = evt;
    p_msg->hdr.layer_specific = event;
    p_msg->hdr.offset = p_data->hdr.err_param;
    p_msg->bd_addr = *bd_addr;
    if (p_scb) {
      APPL_TRACE_DEBUG("scb hndl x%x, role x%x", p_scb->hndl, p_scb->role);
    }
    VLOG(1) << "conn_cback bd_addr:" << bd_addr;
    bta_sys_sendmsg(p_msg);
  }
}

#if (AVDT_REPORTING == TRUE)
/*******************************************************************************
 *
 * Function         bta_av_a2dp_report_cback
 *
 * Description      A2DP report callback.
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_av_a2dp_report_cback(UNUSED_ATTR uint8_t handle,
                                     UNUSED_ATTR AVDT_REPORT_TYPE type,
                                     UNUSED_ATTR tAVDT_REPORT_DATA* p_data) {
  /* Do not need to handle report data for now.
   * This empty function is here for conformance reasons. */
}
#endif

/*******************************************************************************
 *
 * Function         bta_av_api_register
 *
 * Description      allocate stream control block,
 *                  register the service to stack
 *                  create SDP record
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_av_api_register(tBTA_AV_DATA* p_data) {
  tBTA_AV_REGISTER registr;
  tBTA_AV_SCB* p_scb; /* stream control block */
  tAVDT_REG reg;
  tAVDT_CS cs;
  char* p_service_name;
  tBTA_UTL_COD cod;

  memset(&cs, 0, sizeof(tAVDT_CS));

  registr.status = BTA_AV_FAIL_RESOURCES;
  registr.app_id = p_data->api_reg.app_id;
  registr.chnl = (tBTA_AV_CHNL)p_data->hdr.layer_specific;

  uint16_t profile_initialized = p_data->api_reg.service_uuid;
  if (profile_initialized == UUID_SERVCLASS_AUDIO_SINK) {
    p_bta_av_cfg = (tBTA_AV_CFG*)&bta_avk_cfg;
  } else if (profile_initialized == UUID_SERVCLASS_AUDIO_SOURCE) {
    p_bta_av_cfg = (tBTA_AV_CFG*)&bta_av_cfg;
  }

  APPL_TRACE_DEBUG("%s(): profile: 0x%x, channle: 0x%x", __func__,
                   profile_initialized, registr.chnl);
  if (p_bta_av_cfg == NULL) {
    APPL_TRACE_ERROR("AV configuration is null!");
    return;
  }

  do {
    p_scb = bta_av_alloc_scb(registr.chnl);
    cs.registration_id = p_scb->hdi;
    if (p_scb == NULL) {
      APPL_TRACE_ERROR("failed to alloc SCB");
      break;
    }

    registr.hndl = p_scb->hndl;
    p_scb->app_id = registr.app_id;

    /* initialize the stream control block */
    registr.status = BTA_AV_SUCCESS;

    if ((bta_av_cb.reg_audio + bta_av_cb.reg_video) == 0) {
      /* the first channel registered. register to AVDTP */
      reg.ctrl_mtu = p_bta_av_cfg->sig_mtu;
      reg.ret_tout = BTA_AV_RET_TOUT;
      reg.sig_tout = BTA_AV_SIG_TOUT;
      reg.idle_tout = BTA_AV_IDLE_TOUT;
      reg.sec_mask = bta_av_cb.sec_mask;
#if (BTA_AR_INCLUDED == TRUE)
      bta_ar_reg_avdt(&reg, bta_av_conn_cback, BTA_ID_AV);
#endif
      bta_sys_role_chg_register(&bta_av_sys_rs_cback);

      /* create remote control TG service if required */
      if (bta_av_cb.features & (BTA_AV_FEAT_RCTG)) {
/* register with no authorization; let AVDTP use authorization instead */
#if (BTA_AR_INCLUDED == TRUE)
#if (BTA_AV_WITH_AVCTP_AUTHORIZATION == TRUE)
        bta_ar_reg_avct(p_bta_av_cfg->avrc_mtu, p_bta_av_cfg->avrc_br_mtu,
                        bta_av_cb.sec_mask, BTA_ID_AV);
#else
        bta_ar_reg_avct(p_bta_av_cfg->avrc_mtu, p_bta_av_cfg->avrc_br_mtu,
                        (uint8_t)(bta_av_cb.sec_mask & (~BTA_SEC_AUTHORIZE)),
                        BTA_ID_AV);
#endif

        /* For the Audio Sink role we support additional TG 1.3 to support
         * absolute volume.
         */
        uint16_t profile_version = AVRC_REV_1_0;

        if (profile_initialized == UUID_SERVCLASS_AUDIO_SOURCE) {
          profile_version = AVRC_REV_1_6;
        } else if (profile_initialized == UUID_SERVCLASS_AUDIO_SINK) {
          // Initialize AVRCP1.4 to provide Absolute Volume control.
          profile_version = AVRC_REV_1_4;
        }

        bta_ar_reg_avrc(
            UUID_SERVCLASS_AV_REM_CTRL_TARGET, "AV Remote Control Target", NULL,
            p_bta_av_cfg->avrc_tg_cat, BTA_ID_AV,
            (bta_av_cb.features & BTA_AV_FEAT_BROWSE), profile_version);
#endif
      }

      /* Set the Capturing service class bit */
      if (profile_initialized == UUID_SERVCLASS_AUDIO_SOURCE)
        cod.service = BTM_COD_SERVICE_CAPTURING;
      else if (profile_initialized == UUID_SERVCLASS_AUDIO_SINK)
        cod.service = BTM_COD_SERVICE_RENDERING;
      utl_set_device_class(&cod, BTA_UTL_SET_COD_SERVICE_CLASS);
    } /* if 1st channel */

    /* get stream configuration and create stream */
    cs.cfg.num_codec = 1;
    cs.nsc_mask =
        AVDT_NSC_RECONFIG |
        ((bta_av_cb.features & BTA_AV_FEAT_PROTECT) ? 0 : AVDT_NSC_SECURITY);
    APPL_TRACE_DEBUG("nsc_mask: 0x%x", cs.nsc_mask);

    if (p_data->api_reg.p_service_name[0] == 0) {
      p_service_name = NULL;
    } else {
      p_service_name = p_data->api_reg.p_service_name;
    }

    p_scb->suspend_sup = true;
    p_scb->recfg_sup = true;
    p_scb->skip_sdp = false;

    cs.p_ctrl_cback = bta_av_dt_cback[p_scb->hdi];
    if (registr.chnl == BTA_AV_CHNL_AUDIO) {
      /* set up the audio stream control block */
      APPL_TRACE_EVENT("AV: set up the audio stream control block ");
      p_scb->p_act_tbl = (const tBTA_AV_ACT*)bta_av_a2dp_action;
      p_scb->p_cos = &bta_av_a2dp_cos;
      p_scb->media_type = AVDT_MEDIA_TYPE_AUDIO;
      cs.cfg.psc_mask = AVDT_PSC_TRANS;
      cs.media_type = AVDT_MEDIA_TYPE_AUDIO;
      cs.mtu = p_bta_av_cfg->audio_mtu;
      cs.flush_to = L2CAP_DEFAULT_FLUSH_TO;
      btav_a2dp_codec_index_t codec_index_min =
          BTAV_A2DP_CODEC_INDEX_SOURCE_MIN;
      btav_a2dp_codec_index_t codec_index_max =
          BTAV_A2DP_CODEC_INDEX_SOURCE_MAX;

#if (AVDT_REPORTING == TRUE)
      if (bta_av_cb.features & BTA_AV_FEAT_REPORT) {
        cs.cfg.psc_mask |= AVDT_PSC_REPORT;
        cs.p_report_cback = bta_av_a2dp_report_cback;
      }
#endif
      if (bta_av_cb.features & BTA_AV_FEAT_DELAY_RPT) {
        cs.cfg.psc_mask |= AVDT_PSC_DELAY_RPT;
        a2dp_set_avdt_sdp_ver(AVDT_VERSION_SYNC);
        a2dp_set_a2dp_sdp_ver(A2DP_VERSION_SYNC);
      }

      if (profile_initialized == UUID_SERVCLASS_AUDIO_SOURCE) {
        cs.tsep = AVDT_TSEP_SRC;
        codec_index_min = BTAV_A2DP_CODEC_INDEX_SOURCE_MIN;
        codec_index_max = BTAV_A2DP_CODEC_INDEX_SOURCE_MAX;
      } else if (profile_initialized == UUID_SERVCLASS_AUDIO_SINK) {
        cs.tsep = AVDT_TSEP_SNK;
        cs.p_sink_data_cback = bta_av_sink_data_cback;
        codec_index_min = BTAV_A2DP_CODEC_INDEX_SINK_MIN;
        codec_index_max = BTAV_A2DP_CODEC_INDEX_SINK_MAX;
      }

      /* Initialize handles to zero */
      for (int xx = 0; xx < BTAV_A2DP_CODEC_INDEX_MAX; xx++) {
        p_scb->seps[xx].av_handle = 0;
      }

      /* keep the configuration in the stream control block */
      memcpy(&p_scb->cfg, &cs.cfg, sizeof(tAVDT_CFG));
      for (int i = codec_index_min; i < codec_index_max; i++) {
        btav_a2dp_codec_index_t codec_index =
            static_cast<btav_a2dp_codec_index_t>(i);
        APPL_TRACE_DEBUG("%s: codec_index = %d", __func__, codec_index);
        A2dpCodecs* a2dp_codecs = bta_av_get_a2dp_codecs();
        if (a2dp_codecs != nullptr) {
          std::list<A2dpCodecConfig*> list_codec =
                        a2dp_codecs->orderedSourceCodecs();
          bool found = false;
          for (auto it = list_codec.begin(); it != list_codec.end(); it++) {
            if ((*it)->codecIndex() == codec_index) {
              APPL_TRACE_DEBUG("%s: Hit the codec in ordered source", __func__);
              found = true;
              break;
            }
          }
          if (!found) {
            APPL_TRACE_DEBUG("%s: Can't support codec in ordered source", __func__);
            continue;
          }
        }

        if (!(*bta_av_a2dp_cos.init)(codec_index, &cs.cfg)) {
          continue;
        }
        if (AVDT_CreateStream(&p_scb->seps[codec_index].av_handle, &cs) !=
            AVDT_SUCCESS) {
          continue;
        }
        /* Save a copy of the codec */
        memcpy(p_scb->seps[codec_index].codec_info, cs.cfg.codec_info,
               AVDT_CODEC_SIZE);
        p_scb->seps[codec_index].tsep = cs.tsep;
        if (cs.tsep == AVDT_TSEP_SNK) {
          p_scb->seps[codec_index].p_app_sink_data_cback =
              p_data->api_reg.p_app_sink_data_cback;
        } else {
          /* In case of A2DP SOURCE we don't need a callback to
           * handle media packets.
           */
          p_scb->seps[codec_index].p_app_sink_data_cback = NULL;
        }
      }

      if (!bta_av_cb.reg_audio) {
        bta_av_cb.sdp_a2dp_handle = 0;
        bta_av_cb.sdp_a2dp_snk_handle = 0;
        if (profile_initialized == UUID_SERVCLASS_AUDIO_SOURCE) {
          /* create the SDP records on the 1st audio channel */
          bta_av_cb.sdp_a2dp_handle = SDP_CreateRecord();
          A2DP_AddRecord(UUID_SERVCLASS_AUDIO_SOURCE, p_service_name, NULL,
                         A2DP_SUPF_PLAYER, bta_av_cb.sdp_a2dp_handle);
          bta_sys_add_uuid(UUID_SERVCLASS_AUDIO_SOURCE);
        } else if (profile_initialized == UUID_SERVCLASS_AUDIO_SINK) {
#if (BTA_AV_SINK_INCLUDED == TRUE)
          bta_av_cb.sdp_a2dp_snk_handle = SDP_CreateRecord();
          A2DP_AddRecord(UUID_SERVCLASS_AUDIO_SINK, p_service_name, NULL,
                         A2DP_SUPF_PLAYER, bta_av_cb.sdp_a2dp_snk_handle);
          bta_sys_add_uuid(UUID_SERVCLASS_AUDIO_SINK);
#endif
        }
        /* start listening when A2DP is registered */
        if (bta_av_cb.features & BTA_AV_FEAT_RCTG)
          bta_av_rc_create(&bta_av_cb, AVCT_ACP, p_scb->hdi, BTA_AV_NUM_LINKS + 1, NULL);

        /* if the AV and AVK are both supported, it cannot support the CT role
         */
        if (bta_av_cb.features & (BTA_AV_FEAT_RCCT)) {
          /* if TG is not supported, we need to register to AVCT now */
          if ((bta_av_cb.features & (BTA_AV_FEAT_RCTG)) == 0) {
#if (BTA_AR_INCLUDED == TRUE)
#if (BTA_AV_WITH_AVCTP_AUTHORIZATION == TRUE)
            bta_ar_reg_avct(p_bta_av_cfg->avrc_mtu, p_bta_av_cfg->avrc_br_mtu,
                            bta_av_cb.sec_mask, BTA_ID_AV);
#else
            bta_ar_reg_avct(
                p_bta_av_cfg->avrc_mtu, p_bta_av_cfg->avrc_br_mtu,
                (uint8_t)(bta_av_cb.sec_mask & (~BTA_SEC_AUTHORIZE)),
                BTA_ID_AV);
#endif
#endif
            bta_av_rc_create(&bta_av_cb, AVCT_ACP, 0, BTA_AV_NUM_LINKS + 1, NULL);
          }
#if (BTA_AR_INCLUDED == TRUE)
          /* create an SDP record as AVRC CT. We create 1.3 for SOURCE
           * because we rely on feature bits being scanned by external
           * devices more than the profile version itself.
           *
           * We create 1.4 for SINK since we support browsing.
           */
          if ((profile_initialized == UUID_SERVCLASS_AUDIO_SOURCE) ||
              (profile_initialized == UUID_SERVCLASS_AUDIO_SINK)) {
            bta_ar_reg_avrc(UUID_SERVCLASS_AV_REMOTE_CONTROL, NULL, NULL,
                            p_bta_av_cfg->avrc_ct_cat, BTA_ID_AV,
                            (bta_av_cb.features & BTA_AV_FEAT_BROWSE),
                            AVRC_REV_1_6);
          }
#endif
        }
      }
      bta_av_cb.reg_audio |= BTA_AV_HNDL_TO_MSK(p_scb->hdi);
      APPL_TRACE_DEBUG("reg_audio: 0x%x", bta_av_cb.reg_audio);
    } else {
      bta_av_cb.reg_video = BTA_AV_HNDL_TO_MSK(p_scb->hdi);
      bta_av_cb.sdp_vdp_handle = SDP_CreateRecord();
      /* register the video channel */
      /* no need to verify the function pointer here. it's verified prior */
      (*p_bta_av_cfg->p_reg)(&cs, p_service_name, p_scb);
    }
  } while (0);

  /* call callback with register event */
  tBTA_AV bta_av_data;
  bta_av_data.registr = registr;
  (*bta_av_cb.p_cback)(BTA_AV_REGISTER_EVT, &bta_av_data);
}

/*******************************************************************************
 *
 * Function         bta_av_api_deregister
 *
 * Description      de-register a channel
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_av_api_deregister(tBTA_AV_DATA* p_data) {
  tBTA_AV_SCB* p_scb = bta_av_hndl_to_scb(p_data->hdr.layer_specific);

  if (p_scb) {
    p_scb->deregistring = true;
    bta_av_ssm_execute(p_scb, BTA_AV_API_CLOSE_EVT, p_data);
  } else {
    bta_av_dereg_comp(p_data);
  }
}

/*******************************************************************************
 *
 * Function         bta_av_ci_data
 *
 * Description      Forward the BTA_AV_CI_SRC_DATA_READY_EVT to stream state
 *                  machine.
 *
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_av_ci_data(tBTA_AV_DATA* p_data) {
  tBTA_AV_SCB* p_scb;
  int i;
  uint8_t chnl = (uint8_t)p_data->hdr.layer_specific;
  APPL_TRACE_DEBUG("%s: chnl: 0x%x", __func__, chnl);

  for (i = 0; i < btif_max_av_clients; i++) {
    p_scb = bta_av_cb.p_scb[i];

    /* Check if the Stream is in Started state before sending data
     * in Dual Handoff mode, get SCB where START is done.
     */
    if (p_scb && (p_scb->chnl == chnl) && (p_scb->started)) {
      APPL_TRACE_DEBUG("%s: p_scb->chnl: 0x%x, started: %d, hdi: %d",
                        __func__, p_scb->chnl, p_scb->started, p_scb->hdi);
      if (p_scb->hdi == btif_get_is_remote_started_idx()) {
          APPL_TRACE_WARNING("%s: Not to send data to remote Started index %d",
            __func__, p_scb->hdi);
      } else if ((!bta_av_is_multicast_enabled()) &&
            (bta_av_multiple_streams_started()) &&
            (btif_get_is_remote_started_idx() == btif_max_av_clients)) {
            // Hit when suspend is sent for remote start but ack not received yet
            APPL_TRACE_WARNING("%s: Remote Start update delayed, drop data for index %d",
              __func__, p_scb->hdi);
      } else {
          bta_av_ssm_execute(p_scb, BTA_AV_SRC_DATA_READY_EVT, p_data);
      }
    }
  }
}

/*******************************************************************************
 *
 * Function         bta_av_rpc_conn
 *
 * Description      report report channel open
 *
 * Returns          void
 *
 ******************************************************************************/
#if (AVDT_REPORTING == TRUE)
static void bta_av_rpc_conn(UNUSED_ATTR tBTA_AV_DATA* p_data) {}
#endif

/*******************************************************************************
 *
 * Function         bta_av_multiple_streams_started
 *
 * Description      Are multiple streams started (multicast)
 *
 * Returns          bool
 *
 ******************************************************************************/
bool bta_av_multiple_streams_started(void)
{
  int xx, stream_count = 0;

  for (xx = 0; xx < BTA_AV_NUM_STRS; xx++)
    if ((bta_av_cb.p_scb[xx] != NULL) && bta_av_cb.p_scb[xx]->started == true)
            stream_count++;

  return (stream_count > 1);
}

/*******************************************************************************
 *
 * Function         bta_av_api_to_ssm
 *
 * Description      forward the API request to stream state machine
 *
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_av_api_to_ssm(tBTA_AV_DATA* p_data) {
  int xx;
  uint16_t event =
      p_data->hdr.event - BTA_AV_FIRST_A2S_API_EVT + BTA_AV_FIRST_A2S_SSM_EVT;

  /* Multicast: Corner case handling for multicast state getting
   * updated for ACL connected during the stream start where both
   * streams are not yet started. We need to take care of this
   * during suspend to ensure we suspend both streams.
   */
#if (TWS_ENABLED == TRUE)
  int tws_device = 0;
  tBTA_AV_SCB *p_scb = bta_av_hndl_to_scb(p_data->hdr.layer_specific);
  RawAddress tws_pair_addr;
  bool tws_pair_found = false;
  if (p_scb == NULL) {
    APPL_TRACE_ERROR("failed to alloc SCB");
    return;
  }
  if (p_scb->tws_device) {
    tws_device++;
    if (BTM_SecGetTwsPlusPeerDev(p_scb->peer_addr, tws_pair_addr) == true) {
      APPL_TRACE_DEBUG("%s: tws pair found",__func__);
      tws_pair_found = true;
    } else {
      APPL_TRACE_DEBUG("%s:tws pair not found",__func__);
      tws_pair_found = false;
    }
    for (int i = 0; i < BTA_AV_NUM_STRS; i++) {
      if (bta_av_cb.p_scb[i] != NULL &&
        bta_av_cb.p_scb[i] != p_scb) {
        if (bta_av_cb.p_scb[i]->tws_device &&
          bta_av_cb.p_scb[i]->peer_addr == tws_pair_addr) {
          tws_device++;
        }
      }
    }
  }
  APPL_TRACE_DEBUG("bta_av_api_to_ssm: num tws devices = %d",tws_device);
#endif //TWS_ENABLED
  if (is_multicast_enabled == TRUE
#if (TWS_ENABLED == TRUE)
     ||  tws_device > 1
#endif
     ) {
    /* Send START request to all Open Stream connections.*/
    for (xx=0; xx < BTA_AV_NUM_STRS; xx++)
      if (is_multicast_enabled) {
        bta_av_ssm_execute(bta_av_cb.p_scb[xx], event, p_data);
      }
#if (TWS_ENABLED == TRUE)
      else {
          if (tws_device > 1 && bta_av_cb.p_scb[xx] != NULL && bta_av_cb.p_scb[xx]->tws_device &&
           ((bta_av_cb.p_scb[xx]->peer_addr != p_scb->peer_addr) ||
            (tws_pair_found && bta_av_cb.p_scb[xx]->peer_addr != tws_pair_addr))) {
            APPL_TRACE_DEBUG("%s:peer_addr: %s eb state: %d",__func__,
                bta_av_cb.p_scb[xx]->peer_addr.ToString().c_str(),bta_av_cb.p_scb[xx]->eb_state);
#if (TWS_STATE_ENABLED == TRUE)
            if (event == BTA_AV_AP_START_EVT && tws_state_supported &&
              bta_av_cb.p_scb[xx]->eb_state == TWSP_EB_STATE_OUT_OF_EAR) {
              APPL_TRACE_DEBUG("%s:EB not in ear skip start",__func__);
              continue;
            }
#endif
            APPL_TRACE_DEBUG("%s:Execute event %d",__func__,event);
            bta_av_ssm_execute(bta_av_cb.p_scb[xx], event, p_data);
          }
      }
#endif
  } else {
    /* In Dual A2dp Handoff, process this fucntion on specific handles.*/
    APPL_TRACE_DEBUG("bta_av_api_to_ssm: on Handle 0x%x",p_data->hdr.layer_specific);
    bta_av_ssm_execute(bta_av_hndl_to_scb(p_data->hdr.layer_specific), event, p_data);
  }
}

/*******************************************************************************
 *
 * Function         bta_av_api_enable_multicast
 *
 * Description      Enable/Disable Avdtp multicast
 *
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_av_api_enable_multicast(tBTA_AV_DATA *p_data)
{
  is_multicast_enabled = p_data->multicast_state.is_multicast_enabled;
  APPL_TRACE_DEBUG("%s(): is_multicast_enabled :%d", __func__,
                   is_multicast_enabled);
}

#if (TWS_ENABLED == TRUE)
#if (TWS_STATE_ENABLED == TRUE)
static void bta_av_api_set_tws_earbud_state(tBTA_AV_DATA * p_data)
{
  APPL_TRACE_DEBUG("%s:EB_STATE=%d",__func__,p_data->tws_set_earbud_state.eb_state);
  tBTA_AV_SCB *p_scb = bta_av_hndl_to_scb(p_data->hdr.layer_specific);
  RawAddress tws_pair_addr;
  if (p_scb == NULL) {
    APPL_TRACE_ERROR("bta_av_api_set_tws_earbud_state: scb not found");
    return;
  }
  uint8_t previous_state = p_scb->eb_state;
  p_scb->eb_state = p_data->tws_set_earbud_state.eb_state;
  if (previous_state == TWSP_EB_STATE_UNKNOWN &&
    p_scb->eb_state == TWSP_EB_STATE_OUT_OF_EAR &&
    p_scb->started) {
    bta_av_ssm_execute(p_scb, BTA_AV_AP_STOP_EVT, p_data);
    return;
  }

  for (int i = 0; i < BTA_AV_NUM_STRS; i++) {
    tBTA_AV_SCB *p_scbi;
    p_scbi = bta_av_cb.p_scb[i];
    if (p_scbi == NULL || p_scbi == p_scb) continue;
    APPL_TRACE_DEBUG("%s:p_scbi is tws dev = %d",__func__,p_scbi->tws_device);
    if (p_scbi->tws_device &&
      (BTM_SecGetTwsPlusPeerDev(p_scb->peer_addr,
                             tws_pair_addr) == true)) {
      if ((tws_pair_addr == p_scbi->peer_addr) && (p_scbi->started || p_scbi->start_pending)) {
        if (p_scb->eb_state == TWSP_EB_STATE_IN_EAR && !p_scb->started) {
          APPL_TRACE_ERROR("%:streaming on other eb, sending start to eb",__func__);
          bta_av_ssm_execute(p_scb, BTA_AV_AP_START_EVT, p_data);
          return;
        }
      }
    }
  }
}
#endif
static void bta_av_api_set_tws_earbud_role(tBTA_AV_DATA * p_data)
{
  APPL_TRACE_DEBUG("bta_av_api_set_earbud_role = %d",p_data->tws_set_earbud_role.chn_mode);
  tBTA_AV_SCB *p_scb = bta_av_hndl_to_scb(p_data->hdr.layer_specific);
  RawAddress tws_pair_addr;
  tBTA_AV_SCB *p_scbi;
  int i;
  if (p_scb == NULL) {
    APPL_TRACE_ERROR("bta_av_api_set_tws_earbud_role: scb not found");
    return;
  }
  if (p_scb->started) {
    APPL_TRACE_ERROR("%:already streaming,not overwriting ch role",__func__);
    return;
  }

  for (i = 0; i < BTA_AV_NUM_STRS; i++) {
    p_scbi = bta_av_cb.p_scb[i];
    if (p_scbi == NULL || p_scbi == p_scb) continue;
    APPL_TRACE_DEBUG("%s:p_scbi is tws dev = %d",__func__,p_scbi->tws_device);
    if (p_scbi->tws_device) {
      if (BTM_SecGetTwsPlusPeerDev(p_scb->peer_addr,
                               tws_pair_addr) == true) {
        if ((tws_pair_addr == p_scbi->peer_addr) && p_scbi->started) {
          APPL_TRACE_ERROR("%:already streaming in paired earbud,not overwriting ch role",__func__);
          return;
        }
      }
    }
  }

  p_scb->channel_mode = p_data->tws_set_earbud_role.chn_mode;
  bta_av_set_tws_chn_mode(p_scb, true);
//  p_scb->tws_device = true;
}
static void bta_av_api_set_is_tws_device(tBTA_AV_DATA * p_data)
{
  APPL_TRACE_DEBUG("bta_av_api_set_is_tws_device = %d", p_data->tws_set_device.is_tws_device);
  tBTA_AV_SCB *p_scb = bta_av_hndl_to_scb(p_data->hdr.layer_specific);
  if (p_scb == NULL) {
    APPL_TRACE_ERROR("bta_av_api_set_is_tws_device: scb not found");
    return;
  }
  p_scb->tws_device = p_data->tws_set_device.is_tws_device;
}
#endif
/*******************************************************************************
 *
 * Function         bta_av_is_multicast_enabled
 *
 * Description      return status of Avdtp multicast
 *
 * Returns          bool
 *
 ******************************************************************************/
bool bta_av_is_multicast_enabled()
{
  return is_multicast_enabled;
}

/*******************************************************************************
 *
 * Function         bta_av_api_update_max_av_client
 *
 * Description      Update max simultaneous AV connections supported
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_av_api_update_max_av_clients(tBTA_AV_DATA *p_data)
{
  int bta_av_max_clients = p_data->max_av_clients.max_clients;
  APPL_TRACE_DEBUG("bta_av_max_clients:%d",bta_av_max_clients);
  AVDT_UpdateMaxAvClients(bta_av_max_clients);
}

/*******************************************************************************
 *
 * Function         bta_av_chk_start
 *
 * Description      if this is audio channel, check if more than one audio
 *                  channel is connected & already started.
 *
 * Returns          true, if need api_start
 *
 ******************************************************************************/
bool bta_av_chk_start(tBTA_AV_SCB* p_scb) {
  bool start = false;
  tBTA_AV_SCB* p_scbi;
  int i;
#if (TWS_ENABLED == TRUE)
  RawAddress tws_pair_addr;
  bool tws_pair_found = false;
  if (p_scb->tws_device) {
    if (BTM_SecGetTwsPlusPeerDev(p_scb->peer_addr,
                                 tws_pair_addr) == false) {
      APPL_TRACE_DEBUG("%s:tws pair not found", __func__);
      tws_pair_found = false;
    } else {
      APPL_TRACE_DEBUG("%s:tws pair found", __func__);
      tws_pair_found = true;
    }
  }
#endif
  APPL_TRACE_DEBUG("%s(): Audio open count: 0x%x", __func__,
                   bta_av_cb.audio_open_cnt);
  if (p_scb->chnl == BTA_AV_CHNL_AUDIO) {
    if ((bta_av_cb.audio_open_cnt >= 2) &&
        ((0 ==
          (p_scb->role & BTA_AV_ROLE_AD_ACP)) || /* Outgoing connection or   */
         (bta_av_cb.features &
          BTA_AV_FEAT_ACP_START))) /* auto-starting option     */
    {
      /* more than one audio channel is connected */
      /* if this is the 2nd stream as ACP, give INT a chance to issue the START
       * command */
      for (i = 0; i < BTA_AV_NUM_STRS; i++) {
        p_scbi = bta_av_cb.p_scb[i];
        if (p_scbi && p_scbi->chnl == BTA_AV_CHNL_AUDIO && p_scbi->co_started) {
          if (is_multicast_enabled == TRUE
#if (TWS_ENABLED == TRUE)
              || ((p_scb->tws_device && p_scbi->tws_device && tws_pair_found &&
                 p_scbi->peer_addr == tws_pair_addr)
#if (TWS_STATE_ENABLED == TRUE)
              &&(!tws_state_supported || (tws_state_supported &&
              p_scb->eb_state == TWSP_EB_STATE_IN_EAR))
#endif
            )
#endif
            )
            start = true;
          else {
            start = false;
            APPL_TRACE_DEBUG("%s: Already playing", __func__);
            break;
          }
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
  return start;
}

/*******************************************************************************
 *
 * Function         bta_av_restore_switch
 *
 * Description      assume that the caller of this function already makes
 *                  sure that there's only one ACL connection left
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_av_restore_switch(void) {
  tBTA_AV_CB* p_cb = &bta_av_cb;
  int i;
  uint8_t mask;

  APPL_TRACE_DEBUG("reg_audio: 0x%x", bta_av_cb.reg_audio);
  for (i = 0; i < BTA_AV_NUM_STRS; i++) {
    mask = BTA_AV_HNDL_TO_MSK(i);
    if (p_cb->conn_audio == mask) {
      if (p_cb->p_scb[i]) {
        bta_sys_set_policy(BTA_ID_AV, HCI_ENABLE_MASTER_SLAVE_SWITCH,
                           p_cb->p_scb[i]->peer_addr);
      }
      break;
    }
  }
}

/*******************************************************************************
 *
 * Function         bta_av_sys_rs_cback
 *
 * Description      Receives the role change event from dm
 *
 * Returns          (BTA_SYS_ROLE_CHANGE, new_role, hci_status, p_bda)
 *
 ******************************************************************************/
static void bta_av_sys_rs_cback(UNUSED_ATTR tBTA_SYS_CONN_STATUS status,
                                uint8_t id, uint8_t app_id,
                                const RawAddress& peer_addr) {
  int i;
  tBTA_AV_SCB* p_scb = NULL;
  uint8_t cur_role;
  uint8_t peer_idx = 0;

  APPL_TRACE_DEBUG("bta_av_sys_rs_cback: %d", bta_av_cb.rs_idx);
  for (i = 0; i < BTA_AV_NUM_STRS; i++) {
    /* loop through all the SCBs to find matching peer addresses and report the
     * role change event */
    /* note that more than one SCB (a2dp & vdp) maybe waiting for this event */
    p_scb = bta_av_cb.p_scb[i];
    if (p_scb && p_scb->peer_addr == peer_addr) {
      tBTA_AV_ROLE_RES* p_buf =
          (tBTA_AV_ROLE_RES*)osi_malloc(sizeof(tBTA_AV_ROLE_RES));
      APPL_TRACE_DEBUG("new_role:%d, hci_status:x%x hndl: x%x", id, app_id,
                       p_scb->hndl);
      /*
      if ((id != BTM_ROLE_MASTER) && (app_id != HCI_SUCCESS))
      {
          bta_sys_set_policy(BTA_ID_AV,
      (HCI_ENABLE_MASTER_SLAVE_SWITCH|HCI_ENABLE_SNIFF_MODE), p_scb->peer_addr);
      }
      */
      p_buf->hdr.event = BTA_AV_ROLE_CHANGE_EVT;
      p_buf->hdr.layer_specific = p_scb->hndl;
      p_buf->new_role = id;
      p_buf->hci_status = app_id;
      bta_sys_sendmsg(p_buf);

      peer_idx = p_scb->hdi + 1; /* Handle index for the peer_addr */
    }
  }

  /* restore role switch policy, if role switch failed */
  if ((HCI_SUCCESS != app_id) &&
      (BTM_GetRole(peer_addr, &cur_role) == BTM_SUCCESS) &&
      (cur_role == BTM_ROLE_SLAVE)) {
    bta_sys_set_policy(BTA_ID_AV, HCI_ENABLE_MASTER_SLAVE_SWITCH, peer_addr);
  }

  /* if BTA_AvOpen() was called for other device, which caused the role switch
   * of the peer_addr,  */
  /* we need to continue opening process for the BTA_AvOpen(). */
  if ((bta_av_cb.rs_idx != 0) && (bta_av_cb.rs_idx != peer_idx)) {
    if ((bta_av_cb.rs_idx - 1) < BTA_AV_NUM_STRS) {
      p_scb = bta_av_cb.p_scb[bta_av_cb.rs_idx - 1];
    }
    if (p_scb && p_scb->q_tag == BTA_AV_Q_TAG_OPEN) {
      APPL_TRACE_DEBUG("bta_av_sys_rs_cback: rs_idx(%d), hndl:x%x q_tag: %d",
                       bta_av_cb.rs_idx, p_scb->hndl, p_scb->q_tag);

      /* Multicast:
       * As per Multicast feature implementation, fallback
       * happens to soft hand-off when DUT is in scatternet
       * scenario. Hence, don't fail the connection if
       * role switch fails because of remote disallowing.
       * Set switch_res to BTA_AV_RS_DONE on failure.
       */
      if (HCI_SUCCESS == app_id || HCI_ERR_NO_CONNECTION == app_id)
        p_scb->q_info.open.switch_res = BTA_AV_RS_OK;
      else
        p_scb->q_info.open.switch_res = BTA_AV_RS_DONE;

      /* Continue av open process */
      bta_av_do_disc_a2dp(p_scb, (tBTA_AV_DATA*)&(p_scb->q_info.open));
    }

    bta_av_cb.rs_idx = 0;
  }
}

/*******************************************************************************
 *
 * Function         bta_av_sco_chg_cback
 *
 * Description      receive & process the SCO connection up/down event from sys.
 *                  call setup also triggers this callback, to suspend av before
 *                  SCO activity happens, or to resume av once call ends.
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_av_sco_chg_cback(tBTA_SYS_CONN_STATUS status, uint8_t id,
                                 UNUSED_ATTR uint8_t app_id,
                                 UNUSED_ATTR const RawAddress& peer_addr) {
  tBTA_AV_SCB* p_scb;
  int i;
  tBTA_AV_API_STOP stop;

  APPL_TRACE_DEBUG("bta_av_sco_chg_cback:%d status:%d", id, status);
  if (id) {
    bta_av_cb.sco_occupied = true;

    /* either BTA_SYS_SCO_OPEN or BTA_SYS_SCO_CLOSE with remaining active SCO */
    for (i = 0; i < BTA_AV_NUM_STRS; i++) {
      p_scb = bta_av_cb.p_scb[i];

      if (p_scb && p_scb->co_started && (p_scb->sco_suspend == false)) {
        APPL_TRACE_DEBUG("suspending scb:%d", i);
        /* scb is used and started, not suspended automatically */
        p_scb->sco_suspend = true;
        stop.flush = false;
        stop.suspend = true;
        stop.reconfig_stop = false;
        bta_av_ssm_execute(p_scb, BTA_AV_AP_STOP_EVT, (tBTA_AV_DATA*)&stop);
      }
    }
  } else {
    bta_av_cb.sco_occupied = false;

    for (i = 0; i < BTA_AV_NUM_STRS; i++) {
      p_scb = bta_av_cb.p_scb[i];

      if (p_scb && p_scb->sco_suspend) /* scb is used and suspended for SCO */
      {
        APPL_TRACE_DEBUG("starting scb:%d", i);
        bta_av_ssm_execute(p_scb, BTA_AV_AP_START_EVT, NULL);
      }
    }
  }
}

/*******************************************************************************
 *
 * Function         bta_av_switch_if_needed
 *
 * Description      This function checks if there is another existing AV
 *                  channel that is local as slave role.
 *                  If so, role switch and remove it from link policy.
 *
 * Returns          true, if role switch is done
 *
 ******************************************************************************/
bool bta_av_switch_if_needed(tBTA_AV_SCB* p_scb) {
  uint8_t role;
  bool needed = false;
  tBTA_AV_SCB* p_scbi;
  int i;
  tBTM_STATUS ret;
  uint8_t mask;

  for (i = 0; i < BTA_AV_NUM_STRS; i++) {
    mask = BTA_AV_HNDL_TO_MSK(i);
    p_scbi = bta_av_cb.p_scb[i];
    if (p_scbi && (p_scb->hdi != i) &&    /* not the original channel */
        ((bta_av_cb.conn_audio & mask) || /* connected audio */
         (bta_av_cb.conn_video & mask)))  /* connected video */
    {
      BTM_GetRole(p_scbi->peer_addr, &role);
      /* this channel is open - clear the role switch link policy for this link
       */
      APPL_TRACE_DEBUG("%s: Role %d for index %d", __func__, role, i);
      if (BTM_ROLE_MASTER != role) {
        if (bta_av_cb.features & BTA_AV_FEAT_MASTER)
          bta_sys_clear_policy(BTA_ID_AV, HCI_ENABLE_MASTER_SLAVE_SWITCH,
                               p_scbi->peer_addr);
        ret = BTM_SwitchRole(p_scbi->peer_addr, BTM_ROLE_MASTER, NULL);
        APPL_TRACE_IMP("%s: AV Role switch request returns: %d", __func__, ret);
        if ((ret == BTM_REPEATED_ATTEMPTS) ||
            (ret == BTM_MODE_UNSUPPORTED) ||
            (ret == BTM_UNKNOWN_ADDR) ||
            (ret == BTM_SUCCESS) ||
            ((ret == BTM_NO_RESOURCES) &&
              btm_is_sco_active_by_bdaddr(p_scbi->peer_addr)))
          return false;

        if (BTM_CMD_STARTED != ret) {
          /* can not switch role on SCBI
           * start the timer on SCB - because this function is ONLY called when
           * SCB gets API_OPEN */
          bta_sys_start_timer(p_scb->avrc_ct_timer, BTA_AV_RS_TIME_VAL,
                              BTA_AV_AVRC_TIMER_EVT, p_scb->hndl);
        }
        needed = true;
        /* mark the original channel as waiting for RS result */
        bta_av_cb.rs_idx = p_scb->hdi + 1;
        break;
      }
    }
  }
  return needed;
}

/*******************************************************************************
 *
 * Function         bta_av_link_role_ok
 *
 * Description      This function checks if the SCB has existing ACL connection
 *                  If so, check if the link role fits the requirements.
 *
 * Returns          true, if role is ok
 *
 ******************************************************************************/
bool bta_av_link_role_ok(tBTA_AV_SCB* p_scb, uint8_t bits) {
  uint8_t role;
  bool is_ok = true;
  tBTM_STATUS ret;

  if (BTM_GetRole(p_scb->peer_addr, &role) == BTM_SUCCESS) {
    LOG_INFO(LOG_TAG, "%s hndl:x%x role:%d conn_audio:x%x bits:%d features:x%x",
             __func__, p_scb->hndl, role, bta_av_cb.conn_audio, bits,
             bta_av_cb.features);
    if (BTM_ROLE_MASTER != role &&
        (A2DP_BitsSet(bta_av_cb.conn_audio) > bits ||
         (bta_av_cb.features & BTA_AV_FEAT_MASTER) ||
          BTM_GetWifiState())) {
      if (bta_av_cb.features & BTA_AV_FEAT_MASTER)
        bta_sys_clear_policy(BTA_ID_AV, HCI_ENABLE_MASTER_SLAVE_SWITCH,
                             p_scb->peer_addr);
      ret = BTM_SwitchRole(p_scb->peer_addr, BTM_ROLE_MASTER, NULL);
      /* We have already reached maximum attempts,
       * If we try again it will anyways fail
       * return from here
       */
      APPL_TRACE_IMP("%s: AV Role switch request returns: %d", __func__, ret);
      if ((ret == BTM_REPEATED_ATTEMPTS) ||
          (ret == BTM_MODE_UNSUPPORTED) ||
          (ret == BTM_UNKNOWN_ADDR) ||
          (ret == BTM_SUCCESS) ||
          ((ret == BTM_NO_RESOURCES) &&
            btm_is_sco_active_by_bdaddr(p_scb->peer_addr)))
        return true;

      if (BTM_CMD_STARTED != ret) {
        /* can not switch role on SCB - start the timer on SCB */
      }
      is_ok = false;
      p_scb->wait |= BTA_AV_WAIT_ROLE_SW_RES_START;
    }
  }

  return is_ok;
}

/*******************************************************************************
 *
 * Function         bta_av_chk_mtu
 *
 * Description      if this is audio channel, check if more than one audio
 *                  channel is connected.
 *
 * Returns          The smallest mtu of the connected audio channels
 *
 ******************************************************************************/
uint16_t bta_av_chk_mtu(tBTA_AV_SCB* p_scb, UNUSED_ATTR uint16_t mtu) {
  uint16_t ret_mtu = BTA_AV_MAX_A2DP_MTU - AVDT_MEDIA_HDR_SIZE;
  tBTA_AV_SCB* p_scbi;
  int i;
  uint8_t mask;

    /* If multicast is not enabled we do not need to assign lesser MTU even if multi connections are
  avaialable as when active device changes it configures encoder accordingly with proper MTU size
  This change is required only if multicast is enabled as that requires duplication of same packetes
  to two different channels and this logic helps to choose lesser MTU to avoid fragmentation*/

    if (!is_multicast_enabled)
    {
      APPL_TRACE_DEBUG("%s: Non-multicast, conn_audio:0x%x, ret:%d",
                                               __func__, bta_av_cb.conn_audio, mtu);
      if (mtu > (BTA_AV_MAX_A2DP_MTU - AVDT_MEDIA_HDR_SIZE)) {
        mtu = BTA_AV_MAX_A2DP_MTU - AVDT_MEDIA_HDR_SIZE;
        APPL_TRACE_DEBUG("%s: After deduction AVDT_header, updated mtu: %d", __func__, mtu);
      }
      return mtu;
    }

  /* TODO_MV mess with the mtu according to the number of EDR/non-EDR headsets
   */
  if (p_scb->chnl == BTA_AV_CHNL_AUDIO) {
    if (bta_av_cb.audio_open_cnt >= 2) {
      /* more than one audio channel is connected */
      for (i = 0; i < BTA_AV_NUM_STRS; i++) {
        p_scbi = bta_av_cb.p_scb[i];
        if ((p_scb != p_scbi) && p_scbi &&
            (p_scbi->chnl == BTA_AV_CHNL_AUDIO)) {
          mask = BTA_AV_HNDL_TO_MSK(i);
          APPL_TRACE_DEBUG("%s: [%d] mtu: %d, mask:0x%x", __func__, i, p_scbi->stream_mtu,
                           mask);
          if (bta_av_cb.conn_audio & mask) {
            if (ret_mtu > p_scbi->stream_mtu) ret_mtu = p_scbi->stream_mtu;
          }
        }
      }
    }
    APPL_TRACE_DEBUG("bta_av_chk_mtu audio count:%d, conn_audio:0x%x, ret:%d",
                     bta_av_cb.audio_open_cnt, bta_av_cb.conn_audio, ret_mtu);
  }
  return ret_mtu;
}

/*******************************************************************************
 *
 * Function         bta_av_dup_audio_buf
 *
 * Description      dup the audio data to the q_info.a2dp of other audio
 *                  channels
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_av_dup_audio_buf(tBTA_AV_SCB* p_scb, BT_HDR* p_buf) {
  /* Test whether there is more than one audio channel connected */
  if ((p_buf == NULL) || (bta_av_cb.audio_open_cnt < 2)
    || (!bta_av_is_multicast_enabled())) {
      APPL_TRACE_DEBUG("bta_av_dup_audio_buf: data not to dup ");
    return;
  }

  uint16_t copy_size = BT_HDR_SIZE + p_buf->len + p_buf->offset;
  for (int i = 0; i < BTA_AV_NUM_STRS; i++) {
    tBTA_AV_SCB* p_scbi = bta_av_cb.p_scb[i];

    if (i == p_scb->hdi) continue; /* Ignore the original channel */
    if ((p_scbi == NULL) || !p_scbi->co_started)
      continue; /* Ignore if SCB is not used or started */
    if (!(bta_av_cb.conn_audio & BTA_AV_HNDL_TO_MSK(i)))
      continue; /* Audio is not connected */

    /* Enqueue the data */
    BT_HDR* p_new = (BT_HDR*)osi_malloc(copy_size);
    memcpy(p_new, p_buf, copy_size);
    list_append(p_scbi->a2dp_list, p_new);

    if (list_length(p_scbi->a2dp_list) > p_bta_av_cfg->audio_mqs) {
      // Drop the oldest packet
      bta_av_co_audio_drop(p_scbi->hndl);
      BT_HDR* p_buf_drop = static_cast<BT_HDR*>(list_front(p_scbi->a2dp_list));
      list_remove(p_scbi->a2dp_list, p_buf_drop);
      osi_free(p_buf_drop);
    }
  }
}

/*******************************************************************************
 *
 * Function         bta_av_sm_execute
 *
 * Description      State machine event handling function for AV
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_av_sm_execute(tBTA_AV_CB* p_cb, uint16_t event, tBTA_AV_DATA* p_data) {
  tBTA_AV_ST_TBL state_table;
  uint8_t action;

  APPL_TRACE_EVENT("%s: AV event=0x%x(%s) state=%d(%s)", __func__, event,
                   bta_av_evt_code(event), p_cb->state,
                   bta_av_st_code(p_cb->state));

  /* look up the state table for the current state */
  state_table = bta_av_st_tbl[p_cb->state];

  event &= 0x00FF;

  /* set next state */
  p_cb->state = state_table[event][BTA_AV_NEXT_STATE];
  APPL_TRACE_EVENT("next state=%d event offset:%d", p_cb->state, event);

  /* execute action functions */
  action = state_table[event][BTA_AV_ACTION_COL];
  if (action != BTA_AV_IGNORE) {
    APPL_TRACE_EVENT("%s action executed %d", __func__, action);
    (*bta_av_action[action])(p_cb, p_data);
  }
}

/*******************************************************************************
 *
 * Function         bta_av_hdl_event
 *
 * Description      Advanced audio/video main event handling function.
 *
 *
 * Returns          bool
 *
 ******************************************************************************/
bool bta_av_hdl_event(BT_HDR* p_msg) {
  if (p_msg->event >= BTA_AV_LAST_EVT) {
    return true; /* to free p_msg */
  }
  if (p_msg->event >= BTA_AV_FIRST_NSM_EVT) {
    APPL_TRACE_VERBOSE("%s: AV nsm event=0x%x(%s) on handle = 0x%x", __func__,
          p_msg->event, bta_av_evt_code(p_msg->event), p_msg->layer_specific);
    /* non state machine events */
    (*bta_av_nsm_act[p_msg->event - BTA_AV_FIRST_NSM_EVT])(
        (tBTA_AV_DATA*)p_msg);
  } else if (p_msg->event >= BTA_AV_FIRST_SM_EVT &&
             p_msg->event <= BTA_AV_LAST_SM_EVT) {
    APPL_TRACE_VERBOSE("%s: AV sm event=0x%x(%s) on handle = 0x%x", __func__,
         p_msg->event, bta_av_evt_code(p_msg->event), p_msg->layer_specific);
    /* state machine events */
    bta_av_sm_execute(&bta_av_cb, p_msg->event, (tBTA_AV_DATA*)p_msg);
  } else {
      APPL_TRACE_VERBOSE("%s: AV ssm event=0x%x(%s) on handle = 0x%x", __func__,
        p_msg->event, bta_av_evt_code(p_msg->event), p_msg->layer_specific);
    /* stream state machine events */
    bta_av_ssm_execute(bta_av_hndl_to_scb(p_msg->layer_specific), p_msg->event,
                       (tBTA_AV_DATA*)p_msg);
  }
  return true;
}

bool bta_av_get_is_peer_state_incoming(const RawAddress& bd_addr) {
   std::string addrstr = bd_addr.ToString();
   const char* bd_addr_str = addrstr.c_str();
   APPL_TRACE_DEBUG("%s: Remote Device [%s]", __func__, bd_addr_str);
   if (bd_addr_str == NULL) {
      APPL_TRACE_DEBUG("%s: Remote device address is null, return false", __func__);
      return false;
   }
   tBTA_AV_SCB* p_scb = NULL;
   p_scb = bta_av_addr_to_scb(bd_addr);
   return bta_av_is_scb_incoming(p_scb);
}

/*****************************************************************************
 *  Debug Functions
 ****************************************************************************/
/*******************************************************************************
 *
 * Function         bta_av_st_code
 *
 * Description
 *
 * Returns          char *
 *
 ******************************************************************************/
static const char* bta_av_st_code(uint8_t state) {
  switch (state) {
    case BTA_AV_INIT_ST:
      return "INIT";
    case BTA_AV_OPEN_ST:
      return "OPEN";
    default:
      return "unknown";
  }
}
/*******************************************************************************
 *
 * Function         bta_av_evt_code
 *
 * Description
 *
 * Returns          char *
 *
 ******************************************************************************/
const char* bta_av_evt_code(uint16_t evt_code) {
  switch (evt_code) {
    case BTA_AV_API_DISABLE_EVT:
      return "API_DISABLE";
    case BTA_AV_API_REMOTE_CMD_EVT:
      return "API_REMOTE_CMD";
    case BTA_AV_API_VENDOR_CMD_EVT:
      return "API_VENDOR_CMD";
    case BTA_AV_API_VENDOR_RSP_EVT:
      return "API_VENDOR_RSP";
    case BTA_AV_API_META_RSP_EVT:
      return "API_META_RSP_EVT";
    case BTA_AV_API_RC_CLOSE_EVT:
      return "API_RC_CLOSE";
    case BTA_AV_AVRC_OPEN_EVT:
      return "AVRC_OPEN";
    case BTA_AV_AVRC_MSG_EVT:
      return "AVRC_MSG";
    case BTA_AV_AVRC_NONE_EVT:
      return "AVRC_NONE";

    case BTA_AV_API_OPEN_EVT:
      return "API_OPEN";
    case BTA_AV_API_CLOSE_EVT:
      return "API_CLOSE";
    case BTA_AV_AP_START_EVT:
      return "AP_START";
    case BTA_AV_AP_STOP_EVT:
      return "AP_STOP";
    case BTA_AV_API_RECONFIG_EVT:
      return "API_RECONFIG";
    case BTA_AV_API_PROTECT_REQ_EVT:
      return "API_PROTECT_REQ";
    case BTA_AV_API_PROTECT_RSP_EVT:
      return "API_PROTECT_RSP";
    case BTA_AV_API_RC_OPEN_EVT:
      return "API_RC_OPEN";
    case BTA_AV_SRC_DATA_READY_EVT:
      return "SRC_DATA_READY";
    case BTA_AV_CI_SETCONFIG_OK_EVT:
      return "CI_SETCONFIG_OK";
    case BTA_AV_CI_SETCONFIG_FAIL_EVT:
      return "CI_SETCONFIG_FAIL";
    case BTA_AV_SDP_DISC_OK_EVT:
      return "SDP_DISC_OK";
    case BTA_AV_SDP_DISC_FAIL_EVT:
      return "SDP_DISC_FAIL";
    case BTA_AV_STR_DISC_OK_EVT:
      return "STR_DISC_OK";
    case BTA_AV_STR_DISC_FAIL_EVT:
      return "STR_DISC_FAIL";
    case BTA_AV_STR_GETCAP_OK_EVT:
      return "STR_GETCAP_OK";
    case BTA_AV_STR_GETCAP_FAIL_EVT:
      return "STR_GETCAP_FAIL";
    case BTA_AV_STR_OPEN_OK_EVT:
      return "STR_OPEN_OK";
    case BTA_AV_STR_OPEN_FAIL_EVT:
      return "STR_OPEN_FAIL";
    case BTA_AV_STR_START_OK_EVT:
      return "STR_START_OK";
    case BTA_AV_STR_START_FAIL_EVT:
      return "STR_START_FAIL";
    case BTA_AV_STR_CLOSE_EVT:
      return "STR_CLOSE";
    case BTA_AV_STR_CONFIG_IND_EVT:
      return "STR_CONFIG_IND";
    case BTA_AV_STR_SECURITY_IND_EVT:
      return "STR_SECURITY_IND";
    case BTA_AV_STR_SECURITY_CFM_EVT:
      return "STR_SECURITY_CFM";
    case BTA_AV_STR_WRITE_CFM_EVT:
      return "STR_WRITE_CFM";
    case BTA_AV_STR_SUSPEND_CFM_EVT:
      return "STR_SUSPEND_CFM";
    case BTA_AV_STR_RECONFIG_CFM_EVT:
      return "STR_RECONFIG_CFM";
    case BTA_AV_AVRC_TIMER_EVT:
      return "AVRC_TIMER";
    case BTA_AV_AVDT_CONNECT_EVT:
      return "AVDT_CONNECT";
    case BTA_AV_AVDT_DISCONNECT_EVT:
      return "AVDT_DISCONNECT";
    case BTA_AV_ROLE_CHANGE_EVT:
      return "ROLE_CHANGE";
    case BTA_AV_AVDT_DELAY_RPT_EVT:
      return "AVDT_DELAY_RPT";
    case BTA_AV_ACP_CONNECT_EVT:
      return "ACP_CONNECT";

    case BTA_AV_API_ENABLE_EVT:
      return "API_ENABLE";
    case BTA_AV_API_REGISTER_EVT:
      return "API_REG";
    case BTA_AV_API_DEREGISTER_EVT:
      return "API_DEREG";
    case BTA_AV_API_DISCONNECT_EVT:
      return "API_DISCNT";
    case BTA_AV_CI_SRC_DATA_READY_EVT:
      return "CI_DATA_READY";
    case BTA_AV_SIG_CHG_EVT:
      return "SIG_CHG";
    case BTA_AV_SIGNALLING_TIMER_EVT:
      return "SIGNALLING_TIMER";
    case BTA_AV_SDP_AVRC_DISC_EVT:
      return "SDP_AVRC_DISC";
    case BTA_AV_AVRC_CLOSE_EVT:
      return "AVRC_CLOSE";
    case BTA_AV_AVRC_BROWSE_OPEN_EVT:
      return "AVRC_BROWSE_OPEN";
    case BTA_AV_AVRC_BROWSE_CLOSE_EVT:
      return "AVRC_BROWSE_CLOSE";
    case BTA_AV_CONN_CHG_EVT:
      return "CONN_CHG";
    case BTA_AV_DEREG_COMP_EVT:
      return "DEREG_COMP";
#if (AVDT_REPORTING == TRUE)
    case BTA_AV_AVDT_RPT_CONN_EVT:
      return "RPT_CONN";
#endif
    case BTA_AV_API_START_EVT:
      return "API_START";
    case BTA_AV_API_STOP_EVT:
      return "API_STOP";
    case BTA_AV_ENABLE_MULTICAST_EVT:
      return "MULTICAST_ENABLE";
    case BTA_AV_UPDATE_ENCODER_MODE_EVT:
      return "UPDATE_ENCODER_MODE";
    case BTA_AV_UPDATE_APTX_DATA_EVT:
      return "UPDATE_APTX_DATA";
    default:
      return "unknown";
  }
}
