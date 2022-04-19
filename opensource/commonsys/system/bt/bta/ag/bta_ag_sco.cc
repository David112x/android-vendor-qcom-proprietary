/******************************************************************************
 *  Copyright (c) 2017-2018, The Linux Foundation. All rights reserved.
 *  Not a Contribution.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted (subject to the limitations in the
 *  disclaimer below) provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 *  * Neither the name of The Linux Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
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
 *  This file contains functions for managing the SCO connection used in AG.
 *
 ******************************************************************************/

#include <stddef.h>

#include "bt_common.h"
#include "bta_ag_api.h"
#include "bta_ag_co.h"
#include "bta_ag_int.h"
#include "bta_api.h"
#if (BTM_SCO_HCI_INCLUDED == TRUE)
#include "bta_dm_co.h"
#endif
#include "btm_api.h"
#include "device/include/controller.h"
#include "device/include/esco_parameters.h"
#include "osi/include/osi.h"
#include "utl.h"
#include "osi/include/properties.h"
#include "device/include/interop.h"
#if (TWS_AG_ENABLED == TRUE)
#include "bta_ag_twsp.h"
#include "bta_ag_twsp_dev.h"
#endif
#if (SWB_ENABLED == TRUE)
#include <hardware/vendor_hf.h>
#include "bta_ag_swb.h"
#endif

#include <btcommon_interface_defs.h>

bool sco_init_rmt_xfer = false;

/* Codec negotiation timeout */
#ifndef BTA_AG_CODEC_NEGOTIATION_TIMEOUT_MS
#define BTA_AG_CODEC_NEGOTIATION_TIMEOUT_MS (5 * 1000) /* 5 seconds */
#endif

#ifndef BTA_AG_XSCO_COLLISION_TIMEOUT_MS
#define BTA_AG_XSCO_COLLISION_TIMEOUT_MS (2 * 1000) /* 2 seconds */
#endif

static bool sco_allowed = true;
static RawAddress active_device_addr;

#define BTA_AG_NO_EDR_ESCO                                       \
  (ESCO_PKT_TYPES_MASK_NO_2_EV3 | ESCO_PKT_TYPES_MASK_NO_3_EV3 | \
   ESCO_PKT_TYPES_MASK_NO_2_EV5 | ESCO_PKT_TYPES_MASK_NO_3_EV5)


/**
 * Check if bd_addr is the current active device.
 *
 * @param bd_addr target device address
 * @return True if bd_addr is the current active device, False otherwise or if
 * no active device is set (i.e. active_device_addr is empty)
 */
bool bta_ag_sco_is_active_device(const RawAddress& bd_addr) {
  bool ret = false;
#if (TWS_AG_ENABLED == TRUE)
  RawAddress p_addr;
  if (is_twsp_device(bd_addr)) {
      ret = BTM_SecGetTwsPlusPeerDev(bd_addr, p_addr);
      if (ret) {
        ret = !active_device_addr.IsEmpty() &&
               (active_device_addr == bd_addr || active_device_addr == p_addr);
      }
  } else {
#endif
      ret = !active_device_addr.IsEmpty() && active_device_addr == bd_addr;
#if (TWS_AG_ENABLED == TRUE)
  }
#endif
  APPL_TRACE_DEBUG("%s: returns %d", __func__, ret);
  return ret;
}

static void bta_ag_create_pending_sco(tBTA_AG_SCB* p_scb, bool is_local);

/*******************************************************************************
 *
 * Function         bta_ag_sco_conn_cback
 *
 * Description      BTM SCO connection callback.
 *
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_ag_sco_conn_cback(uint16_t sco_idx) {
  uint16_t handle;
  tBTA_AG_SCB* p_scb;
  APPL_TRACE_DEBUG("%s:%d", __func__,sco_idx);
          /* Check if SLC is up */
  handle = bta_ag_idx_by_bdaddr(BTM_ReadScoBdAddr(sco_idx));
  p_scb = bta_ag_scb_by_idx(handle);
  if (p_scb && !p_scb->svc_conn) handle = 0;

  APPL_TRACE_DEBUG("%s: handle is : %x", __func__,handle);
  if (handle != 0) {
    BT_HDR* p_buf = (BT_HDR*)osi_malloc(sizeof(BT_HDR));
    p_buf->event = BTA_AG_SCO_OPEN_EVT;
    p_buf->layer_specific = handle;
    bta_sys_sendmsg(p_buf);
  } else {
    /* no match found; disconnect sco, init sco variables */
    bta_ag_cb.sco.p_curr_scb = NULL;
    bta_ag_cb.sco.state = BTA_AG_SCO_SHUTDOWN_ST;
#if (TWS_AG_ENABLED == TRUE)
    bta_ag_cb.main_sm_scb = NULL;
#endif
    BTM_RemoveSco(sco_idx);
  }
}

/*******************************************************************************
 *
 * Function         bta_ag_xsco_collision_timer_cback
 *
 * Description      xSCO collision timer call back that gets called after
 *                  collision timer expires and attempts for xSCO connection.
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_ag_xsco_collision_timer_cback(void* data) {
  APPL_TRACE_DEBUG("%s", __func__);
  tBTA_AG_SCB* p_scb = (tBTA_AG_SCB*)data;

  bta_ag_sco_open(p_scb, NULL);
  bta_ag_cback_sco(p_scb, BTA_AG_AUDIO_OPENING_EVT);
}

/*******************************************************************************
 *
 * Function         bta_ag_sco_disc_cback
 *
 * Description      BTM SCO disconnection callback.
 *
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_ag_sco_disc_cback(uint16_t sco_idx) {
  uint16_t handle = 0;
  tBTA_AG_SCB* curr_scb;
  uint8_t status = BTM_ReadScoDiscReason();
  bt_soc_type_t soc_type = controller_get_interface()->get_soc_type();

  APPL_TRACE_DEBUG("%s: soc_type: %d", __func__, soc_type);

  APPL_TRACE_IMP(
      "bta_ag_sco_disc_cback(): sco_idx: 0x%x  p_cur_scb: 0x%08x  sco.state: "
      "%d, status %x",
      sco_idx, bta_ag_cb.sco.p_curr_scb, bta_ag_cb.sco.state, status);

  APPL_TRACE_DEBUG(
      "bta_ag_sco_disc_cback(): scb[0] addr: 0x%08x  in_use: %u  sco_idx: 0x%x "
      " sco state: %u",
      &bta_ag_cb.scb[0], bta_ag_cb.scb[0].in_use, bta_ag_cb.scb[0].sco_idx,
      bta_ag_cb.scb[0].state);
  APPL_TRACE_DEBUG(
      "bta_ag_sco_disc_cback(): scb[1] addr: 0x%08x  in_use: %u  sco_idx: 0x%x "
      " sco state: %u",
      &bta_ag_cb.scb[1], bta_ag_cb.scb[1].in_use, bta_ag_cb.scb[1].sco_idx,
      bta_ag_cb.scb[1].state);

     /* match callback to scb */
    /* We only care about callbacks for the active SCO */
    if (bta_ag_cb.sco.p_curr_scb != NULL && bta_ag_cb.sco.p_curr_scb->in_use &&
       (bta_ag_cb.sco.p_curr_scb->sco_idx == sco_idx)) {
           handle = bta_ag_scb_to_idx(bta_ag_cb.sco.p_curr_scb);
    }
#if (TWS_AG_ENABLED == TRUE)
    else if  (bta_ag_cb.main_sm_scb != NULL
         && bta_ag_cb.main_sm_scb->sco_idx == sco_idx)  {
         APPL_TRACE_DEBUG("%s: Primary Twsp device dis", __func__);
         handle = bta_ag_scb_to_idx(bta_ag_cb.main_sm_scb);
    }
    else if (bta_ag_cb.sec_sm_scb != NULL
         && bta_ag_cb.sec_sm_scb->sco_idx == sco_idx)  {
         APPL_TRACE_DEBUG("%s: Secondary Twsp device dis", __func__);
         handle = bta_ag_scb_to_idx(bta_ag_cb.sec_sm_scb);
    }
#endif
    else {
            if (bta_ag_cb.sco.p_curr_scb != NULL  &&
                bta_ag_cb.sco.p_curr_scb->sco_idx != 0xFFFF) {
                APPL_TRACE_ERROR("%s: Invalid sco idx : %d",
                       __func__,bta_ag_cb.sco.p_curr_scb->sco_idx);
                return;
            }
    }

  if (handle != 0) {
#if (BTM_SCO_HCI_INCLUDED == TRUE)

    tBTM_STATUS status =
        BTM_ConfigScoPath(ESCO_DATA_PATH_PCM, NULL, NULL, true);
    APPL_TRACE_DEBUG("%s: sco close config status = %d", __func__, status);
    /* SCO clean up here */
    bta_dm_sco_co_close();
#endif
    curr_scb = bta_ag_scb_by_idx(handle);
    /* Restore settings */
#if (TWS_AG_ENABLED == TRUE)
    /*No codec fallback for TWS+ device*/
    if (is_twsp_device(curr_scb->peer_addr) != true) {
#endif
    if (bta_ag_cb.sco.p_curr_scb != NULL &&
           (bta_ag_cb.sco.p_curr_scb->inuse_codec == BTA_AG_CODEC_MSBC
#if (SWB_ENABLED == TRUE)
        || (bta_ag_cb.sco.p_curr_scb->inuse_codec == BTA_AG_SCO_SWB_SETTINGS_Q0 &&
            bta_ag_cb.sco.p_curr_scb->is_swb_codec)
#endif
       )) {
      /* Bypass vendor specific and voice settings if enhanced eSCO supported */

    if (!(controller_get_interface()
              ->supports_enhanced_setup_synchronous_connection() &&
          (soc_type == BT_SOC_TYPE_CHEROKEE || soc_type == BT_SOC_TYPE_HASTINGS))) {
#if (BLUETOOTH_QTI_SW == FALSE) /* This change is not needed.*/
        BTM_WriteVoiceSettings(BTM_VOICE_SETTING_CVSD);
#endif
      }

      /* If SCO open was initiated by AG and failed for mSBC T2, try mSBC T1
       * 'Safe setting' first. If T1 also fails, try CVSD */
      if (bta_ag_sco_is_opening(bta_ag_cb.sco.p_curr_scb)) {
        bta_ag_cb.sco.p_curr_scb->state = BTA_AG_SCO_CODEC_ST;
#if (BLUETOOTH_QTI_SW == FALSE) /* This change is not needed.*/
        if (bta_ag_cb.sco.p_curr_scb->codec_msbc_settings ==
            BTA_AG_SCO_MSBC_SETTINGS_T2) {
          APPL_TRACE_WARNING(
              "%s: eSCO/SCO failed to open, falling back to mSBC T1 settings",
              __func__);
          bta_ag_cb.sco.p_curr_scb->codec_msbc_settings =
              BTA_AG_SCO_MSBC_SETTINGS_T1;
           } else
#else
           {
               APPL_TRACE_WARNING(
                   "%s: eSCO/SCO failed to open, falling back to CVSD", __func__);
               bta_ag_cb.sco.p_curr_scb->codec_fallback = true;
           }
#endif
         }
      } else if (bta_ag_sco_is_opening(curr_scb)) {
          APPL_TRACE_ERROR("%s: eSCO/SCO failed to open, no more fall back",
                       __func__);
    }
#if (TWS_AG_ENABLED == TRUE)
    } else {
        APPL_TRACE_ERROR("%s: No codec fallback for Twsp devices", __func__);
    }
#endif

    APPL_TRACE_DEBUG("%s: Calling SCO_CLOSE for %d", __func__, handle);
    if(curr_scb != NULL) {
      curr_scb->inuse_codec = BTA_AG_CODEC_NONE;
    }

    BT_HDR* p_buf = (BT_HDR*)osi_malloc(sizeof(BT_HDR));
    p_buf->event = BTA_AG_SCO_CLOSE_EVT;
    p_buf->layer_specific = handle;
    bta_sys_sendmsg(p_buf);

    if ( status == HCI_ERR_DIFF_TRANSACTION_COLLISION &&
         bta_ag_cb.sco.p_curr_scb != NULL &&
         bta_ag_cb.sco.p_curr_scb->no_of_xsco_trials == 0 ) {

      APPL_TRACE_IMP("%s: xSCO disc status is %x, retry xSCO after %x secs",
                     __func__, status, BTA_AG_XSCO_COLLISION_TIMEOUT_MS);
      alarm_set_on_mloop(bta_ag_cb.sco.p_curr_scb->xsco_conn_collision_timer,
                        BTA_AG_XSCO_COLLISION_TIMEOUT_MS,
                        bta_ag_xsco_collision_timer_cback,
                        bta_ag_cb.sco.p_curr_scb);
      bta_ag_cb.sco.p_curr_scb->no_of_xsco_trials++;
    }
  } else {
    /* no match found */
    APPL_TRACE_DEBUG("no scb for ag_sco_disc_cback");

    /* sco could be closed after scb dealloc'ed */
    if (bta_ag_cb.sco.p_curr_scb != NULL) {
      bta_ag_cb.sco.p_curr_scb->sco_idx = BTM_INVALID_SCO_INDEX;
      bta_ag_cb.sco.p_curr_scb = NULL;
      bta_ag_cb.sco.state = BTA_AG_SCO_SHUTDOWN_ST;
    }

#if (TWS_AG_ENABLED == TRUE)
    if (bta_ag_cb.twsp_sec_sco.p_curr_scb != NULL &&
        bta_ag_cb.twsp_sec_sco.p_curr_scb->sco_idx == sco_idx) {
        bta_ag_cb.twsp_sec_sco.p_curr_scb->sco_idx = BTM_INVALID_SCO_INDEX;
        bta_ag_cb.twsp_sec_sco.p_curr_scb = NULL;
        bta_ag_cb.twsp_sec_sco.state = BTA_AG_SCO_SHUTDOWN_ST;
    }
#endif
  }
}
#if (BTM_SCO_HCI_INCLUDED == TRUE)
/*******************************************************************************
 *
 * Function         bta_ag_sco_read_cback
 *
 * Description      Callback function is the callback function for incoming
 *                  SCO data over HCI.
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_ag_sco_read_cback(uint16_t sco_inx, BT_HDR* p_data,
                                  tBTM_SCO_DATA_FLAG status) {
  if (status != BTM_SCO_DATA_CORRECT) {
    APPL_TRACE_DEBUG("%s: status %d", __func__, status);
  }

  /* Callout function must free the data. */
  bta_dm_sco_co_in_data(p_data, status);
}
#endif
/*******************************************************************************
 *
 * Function         bta_ag_remove_sco
 *
 * Description      Removes the specified SCO from the system.
 *                  If only_active is true, then SCO is only removed if
 *                  connected
 *
 * Returns          bool   - true if SCO removal was started
 *
 ******************************************************************************/
bool bta_ag_remove_sco(tBTA_AG_SCB* p_scb, bool only_active) {
  APPL_TRACE_DEBUG("%s: p_scb: %x, only_active: %x", __func__, p_scb, only_active);
  int cur_idx = BTM_INVALID_SCO_INDEX;
#if (TWS_AG_ENABLED == TRUE)
  bool is_peer_twsp_eb = false;
#endif
  if (p_scb->sco_idx != BTM_INVALID_SCO_INDEX) {
#if (TWS_AG_ENABLED == TRUE)
    if (is_twsp_device(p_scb->peer_addr)) {
       if (p_scb == bta_ag_cb.main_sm_scb) {
          cur_idx = bta_ag_cb.sco.cur_idx;
       } else if (p_scb == bta_ag_cb.sec_sm_scb){
          cur_idx = bta_ag_cb.twsp_sec_sco.cur_idx;
          is_peer_twsp_eb = true;
       } else {
          APPL_TRACE_ERROR("%s: Invalid SCB handle", __func__);
       }
   } else {
#endif
       cur_idx = bta_ag_cb.sco.cur_idx;
#if (TWS_AG_ENABLED == TRUE)
   }
#endif

    APPL_TRACE_DEBUG("%s: sco_idx: %d, curr_idx: %d\n", __func__, p_scb->sco_idx, cur_idx);
    if (!only_active || p_scb->sco_idx == cur_idx) {
      tBTM_STATUS status = BTM_RemoveSco(p_scb->sco_idx);
      APPL_TRACE_IMP("%s: SCO index 0x%04x, status %d", __func__,
                       p_scb->sco_idx, status);
      if (status == BTM_CMD_STARTED) {
        /* SCO is connected; set current control block */
#if (TWS_AG_ENABLED == TRUE)
        if (is_peer_twsp_eb) {
            bta_ag_cb.twsp_sec_sco.p_curr_scb = p_scb;
        } else {
#endif
            bta_ag_cb.sco.p_curr_scb = p_scb;
#if (TWS_AG_ENABLED == TRUE)
        }
#endif
        return true;
      } else if ((status == BTM_SUCCESS) || (status == BTM_UNKNOWN_ADDR)) {
        /* If no connection reset the SCO handle */
        p_scb->sco_idx = BTM_INVALID_SCO_INDEX;
      }
    }
  }
  return false;
}

/*******************************************************************************
 *
 * Function         bta_ag_esco_connreq_cback
 *
 * Description      BTM eSCO connection requests and eSCO change requests
 *                  Only the connection requests are processed by BTA.
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_ag_esco_connreq_cback(tBTM_ESCO_EVT event,
                                      tBTM_ESCO_EVT_DATA* p_data) {
  /* Only process connection requests */
  if (event == BTM_ESCO_CONN_REQ_EVT) {
    uint16_t sco_inx = p_data->conn_evt.sco_inx;
    const RawAddress* remote_bda = BTM_ReadScoBdAddr(sco_inx);
    tBTA_AG_SCB* p_scb = bta_ag_scb_by_idx(bta_ag_idx_by_bdaddr(remote_bda));

    if (p_scb == NULL) {
      APPL_TRACE_IMP("%s: p_scb is NULL", __func__);
    } else {
      APPL_TRACE_IMP("%s: p_scb->svc_conn %x, peer_addr is %s",
        __func__, p_scb->svc_conn, p_scb->peer_addr.ToString().c_str());
    }

    if (remote_bda == NULL) {
      APPL_TRACE_IMP("%s: remote_bda is NULL", __func__);
    } else {
      APPL_TRACE_IMP("%s: remote_bda %s, bta_ag_sco_is_active_device is %x",
                      __func__, (*remote_bda).ToString().c_str(),
                      bta_ag_sco_is_active_device(*remote_bda));
    }

#if (TWS_AG_ENABLED == TRUE)
    if (p_scb != NULL)
      APPL_TRACE_IMP("%s: is_twsp_device(p_scb->peer_addr) is %x",
               __func__, is_twsp_device(p_scb->peer_addr));
#endif

    if (
       p_scb &&
       remote_bda && bta_ag_sco_is_active_device(*remote_bda) &&
        p_scb->svc_conn) {
      p_scb->sco_idx = sco_inx;
#if (TWS_AG_ENABLED == TRUE)
      if (is_twsp_device(p_scb->peer_addr)) {
          p_scb->rmt_sco_req = TRUE;
          /* If no other SCO active, allow this one */
          /*OR if it is TWS+ device and this is secondary SCO*/
          if (bta_ag_cb.main_sm_scb == p_scb)
          {
              APPL_TRACE_IMP("%s: Accept Conn Request (sco_inx 0x%04x)", __func__,
                         sco_inx);
              bta_ag_sco_conn_rsp(p_scb, &p_data->conn_evt);

              bta_ag_cb.sco.state = BTA_AG_SCO_OPENING_ST;
              bta_ag_cb.sco.p_curr_scb = p_scb;
              bta_ag_cb.sco.cur_idx = p_scb->sco_idx;
          } else {
              if (bta_ag_cb.sec_sm_scb == p_scb) {

                APPL_TRACE_EVENT("%s: TWS: Accept Conn Request (sco_inx 0x%04x)", __func__,
                         sco_inx);
                bta_ag_cb.twsp_sec_sco.p_curr_scb = p_scb;
                bta_ag_cb.twsp_sec_sco.cur_idx = p_scb->sco_idx;
                bta_ag_cb.twsp_sec_sco.state = BTA_AG_SCO_OPENING_ST;

               bta_ag_sco_conn_rsp(p_scb, &p_data->conn_evt);
              }
          }
      } else { /*legacy device*/
#endif
         if (!bta_ag_cb.sco.p_curr_scb) {
              APPL_TRACE_IMP("%s: Accept Conn Request (sco_inx 0x%04x)", __func__,
                         sco_inx);
              bta_ag_sco_conn_rsp(p_scb, &p_data->conn_evt);

              bta_ag_cb.sco.state = BTA_AG_SCO_OPENING_ST;
              bta_ag_cb.sco.p_curr_scb = p_scb;
              bta_ag_cb.sco.cur_idx = p_scb->sco_idx;
         } else {
               /* Begin a transfer: Close current SCO before responding */
               APPL_TRACE_DEBUG("bta_ag_esco_connreq_cback: Begin XFER");
               bta_ag_cb.sco.p_xfer_scb = p_scb;
               bta_ag_cb.sco.conn_data = p_data->conn_evt;
               bta_ag_cb.sco.state = BTA_AG_SCO_OPEN_XFER_ST;

               if (!bta_ag_remove_sco(bta_ag_cb.sco.p_curr_scb, true)) {
                 APPL_TRACE_ERROR(
                 "%s: Nothing to remove,so accept Conn Request(sco_inx 0x%04x)",
                 __func__, sco_inx);
                  bta_ag_cb.sco.p_xfer_scb = NULL;
                  bta_ag_cb.sco.state = BTA_AG_SCO_LISTEN_ST;
                  bta_ag_sco_conn_rsp(p_scb, &p_data->conn_evt);
               }
          }
#if (TWS_AG_ENABLED == TRUE)
      }
#endif
    } else {
      /* If error occurred send reject response immediately */
      LOG(WARNING) << __func__
                   << ": reject incoming SCO connection, remote_bda="
                   << (remote_bda ? *remote_bda : RawAddress::kEmpty)
                   << ", active_bda=" << active_device_addr << ", current_bda="
                   << (p_scb ? p_scb->peer_addr : RawAddress::kEmpty);
      BTM_EScoConnRsp(p_data->conn_evt.sco_inx, HCI_ERR_HOST_REJECT_RESOURCES,
                      (enh_esco_params_t*)NULL);
    }
  } else if (event == BTM_ESCO_CHG_EVT) {
    /* Received a change in the esco link */
    APPL_TRACE_EVENT(
        "%s: eSCO change event (inx %d): rtrans %d, "
        "rxlen %d, txlen %d, txint %d",
        __func__, p_data->chg_evt.sco_inx, p_data->chg_evt.retrans_window,
        p_data->chg_evt.rx_pkt_len, p_data->chg_evt.tx_pkt_len,
        p_data->chg_evt.tx_interval);
  }
}

/*******************************************************************************
 *
 * Function         bta_ag_cback_sco
 *
 * Description      Call application callback function with SCO event.
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_ag_cback_sco(tBTA_AG_SCB* p_scb, uint8_t event) {

  tBTA_AG_HDR sco;

  sco.handle = bta_ag_scb_to_idx(p_scb);
  sco.app_id = p_scb->app_id;

  /* call close cback */
  (*bta_ag_cb.p_cback)(event, (tBTA_AG*)&sco);
}

/*******************************************************************************
 *
 * Function         bta_ag_create_sco
 *
 * Description      Create a SCO connection for a given control block
 *                  p_scb : Pointer to the target AG control block
 *                  is_orig : Whether to initiate or listen for SCO connection
 *
 * Returns          void
 *
 ******************************************************************************/
bool bta_ag_create_sco(tBTA_AG_SCB* p_scb, bool is_orig) {
  APPL_TRACE_DEBUG(
      "%s: BEFORE codec_updated=%d, codec_fallback=%d, "
      "sco_codec=%d, peer_codec=%d, msbc_settings=%d, device=%s",
      __func__, p_scb->codec_updated, p_scb->codec_fallback, p_scb->sco_codec,
      p_scb->peer_codecs, p_scb->codec_msbc_settings,
      p_scb->peer_addr.ToString().c_str());
  tBTA_AG_PEER_CODEC esco_codec = BTA_AG_CODEC_CVSD;

  if (!bta_ag_sco_is_active_device(p_scb->peer_addr)) {
    LOG(WARNING) << __func__ << ": device " << p_scb->peer_addr
                 << " is not active, active_device=" << active_device_addr;
    return false;
  }

  /* Make sure this SCO handle is not already in use */
  if (p_scb->sco_idx != BTM_INVALID_SCO_INDEX) {
    APPL_TRACE_ERROR("%s: device %s, index 0x%04x already in use!", __func__,
                     p_scb->peer_addr.ToString().c_str(), p_scb->sco_idx);
    return false;
  }

  if ((p_scb->sco_codec == BTA_AG_CODEC_MSBC) && !p_scb->codec_fallback)
    esco_codec = BTA_AG_CODEC_MSBC;

#if (SWB_ENABLED == TRUE)
  if ((p_scb->sco_codec == BTA_AG_SCO_SWB_SETTINGS_Q0) && !p_scb->codec_fallback)
    esco_codec = BTA_AG_SCO_SWB_SETTINGS_Q0;
#endif

  if (p_scb->codec_fallback) {
    p_scb->codec_fallback = false;
    /* Force AG to send +BCS for the next audio connection. */
    p_scb->codec_updated = true;
    /* Reset mSBC settings to T2 for the next audio connection */
    p_scb->codec_msbc_settings = BTA_AG_SCO_MSBC_SETTINGS_T2;
#if (SWB_ENABLED == TRUE)
    /* Reset SWB settings to Q3 for the next audio connection */
    p_scb->codec_swb_settings = BTA_AG_SCO_SWB_SETTINGS_Q0;
#endif
  }

  esco_codec_t codec_index = ESCO_CODEC_CVSD;
  /* If WBS included, use CVSD by default, index is 0 for CVSD by
   * initialization. If eSCO codec is mSBC, index is T2 or T1 */
  if (esco_codec == BTA_AG_CODEC_MSBC) {
    if (p_scb->codec_msbc_settings == BTA_AG_SCO_MSBC_SETTINGS_T2) {
      codec_index = ESCO_CODEC_MSBC_T2;
    } else {
      codec_index = ESCO_CODEC_MSBC_T1;
    }
  }

#if (SWB_ENABLED == TRUE)
  if (p_scb->is_swb_codec == true && !p_scb->codec_updated) {
    if (p_scb->codec_swb_settings == BTA_AG_SCO_SWB_SETTINGS_Q3) {
      codec_index = ESCO_CODEC_SWB_Q3;
    } else if (p_scb->codec_swb_settings == BTA_AG_SCO_SWB_SETTINGS_Q2) {
      codec_index = ESCO_CODEC_SWB_Q2;
    } else if (p_scb->codec_swb_settings == BTA_AG_SCO_SWB_SETTINGS_Q1) {
      codec_index = ESCO_CODEC_SWB_Q1;
    } else if (p_scb->codec_swb_settings == BTA_AG_SCO_SWB_SETTINGS_Q0) {
      codec_index = ESCO_CODEC_SWB_Q0;
    }
  }
#endif
  /* Initialize eSCO parameters */
  enh_esco_params_t params = esco_parameters_for_codec(codec_index);
  /* For CVSD */
  if (esco_codec == BTM_SCO_CODEC_CVSD) {
    /* Use the applicable packet types
      (3-EV3 not allowed due to errata 2363) */
    params.packet_types =
        p_bta_ag_cfg->sco_pkt_types | ESCO_PKT_TYPES_MASK_NO_3_EV3;
    if ((!(p_scb->features & BTA_AG_FEAT_ESCO)) ||
        (!(p_scb->peer_features & BTA_AG_PEER_FEAT_ESCO))) {
      params.max_latency_ms = 10;
      params.retransmission_effort = ESCO_RETRANSMISSION_POWER;
    }
  }

#if (TWS_AG_ENABLED == TRUE)
  if (is_twsp_device(p_scb->peer_addr)) {
      APPL_TRACE_DEBUG("%s: set Max Latency to 14 for TWS eSCO", __func__);
      params.max_latency_ms = 14;
  }
#endif

  /* If initiating, setup parameters to start SCO/eSCO connection */
  if (is_orig) {
    bta_ag_cb.sco.is_local = true;
    /* Set eSCO Mode */
    BTM_SetEScoMode(&params);

#if (TWS_AG_ENABLED == TRUE)
    if (is_twsp_device(p_scb->peer_addr)) {
        if (bta_ag_cb.main_sm_scb == p_scb) {
            bta_ag_cb.sco.p_curr_scb = p_scb;
        } else if (bta_ag_cb.sec_sm_scb == p_scb){
            bta_ag_cb.twsp_sec_sco.p_curr_scb = p_scb;
        } else {
            APPL_TRACE_DEBUG("%s: FataL: no scb match!", __func__);
        }
    } else {
#endif
        bta_ag_cb.sco.p_curr_scb = p_scb;
#if (TWS_AG_ENABLED == TRUE)
    }
#endif
    /* save the current codec as sco_codec can be updated while SCO is open. */
    p_scb->inuse_codec = esco_codec;

    /* tell sys to stop av if any */
    bta_sys_sco_use(BTA_ID_AG, p_scb->app_id, p_scb->peer_addr);

    /* Send pending commands to create SCO connection to peer */
    bta_ag_create_pending_sco(p_scb, bta_ag_cb.sco.is_local);
    APPL_TRACE_API("%s: orig %d, inx 0x%04x, pkt types 0x%04x", __func__,
                   is_orig, p_scb->sco_idx, params.packet_types);
  } else {
    /* Not initiating, go to listen mode */
    tBTM_STATUS status = BTM_CreateSco(
        &p_scb->peer_addr, false, params.packet_types, &p_scb->sco_idx,
        bta_ag_sco_conn_cback, bta_ag_sco_disc_cback);
    if (status == BTM_CMD_STARTED) {
      BTM_RegForEScoEvts(p_scb->sco_idx, bta_ag_esco_connreq_cback);
    }

    APPL_TRACE_API("%s: orig %d, inx 0x%04x, status 0x%x, pkt types 0x%04x",
                   __func__, is_orig, p_scb->sco_idx, status,
                   params.packet_types);
  }
  APPL_TRACE_DEBUG(
      "%s: AFTER codec_updated=%d, codec_fallback=%d, "
      "sco_codec=%d, peer_codec=%d, msbc_settings=%d, device=%s",
      __func__, p_scb->codec_updated, p_scb->codec_fallback, p_scb->sco_codec,
      p_scb->peer_codecs, p_scb->codec_msbc_settings,
      p_scb->peer_addr.ToString().c_str());
  return true;
}

/*******************************************************************************
 *
 * Function         bta_ag_create_pending_sco
 *
 * Description      This Function is called after the pre-SCO vendor setup is
 *                  done for the BTA to continue and send the HCI Commands for
 *                  creating/accepting SCO connection with peer based on the
 *                  is_local parameter.
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_ag_create_pending_sco(tBTA_AG_SCB* p_scb, bool is_local) {
  if (p_scb == NULL) {
    APPL_TRACE_ERROR("%s: Invalid p_scb", __func__);
    return;
  }
  tBTA_AG_PEER_CODEC esco_codec = p_scb->inuse_codec;
  enh_esco_params_t params;
  bt_soc_type_t soc_type = controller_get_interface()->get_soc_type();

  APPL_TRACE_DEBUG("%s: soc_type: %d", __func__, soc_type);


  /* If there is timer running for xSCO setup, cancel it */
  alarm_cancel(p_scb->xsco_conn_collision_timer);

#if (TWS_AG_ENABLED == TRUE)
  if (is_twsp_device(p_scb->peer_addr)) {
      APPL_TRACE_DEBUG("%s: sco_curr_scb %x, twsp_sec_sco.curr_scb : %x p_scb : %x",
      __func__, bta_ag_cb.sco.p_curr_scb, bta_ag_cb.twsp_sec_sco.p_curr_scb, p_scb);

      APPL_TRACE_DEBUG("%s: main_sm_scb %x, sec_sm_scb : %x",
      __func__, bta_ag_cb.main_sm_scb, bta_ag_cb.sec_sm_scb);
      if (bta_ag_cb.main_sm_scb == p_scb) {
          bta_ag_cb.sco.p_curr_scb = p_scb;
          bta_ag_cb.sco.cur_idx = p_scb->sco_idx;
      } else if (bta_ag_cb.sec_sm_scb == p_scb) {
          APPL_TRACE_DEBUG("%s: It is TWS+ peer connection", __func__);
          bta_ag_cb.twsp_sec_sco.p_curr_scb = p_scb;
          bta_ag_cb.twsp_sec_sco.cur_idx = p_scb->sco_idx;
      }
  } else {
#endif
      bta_ag_cb.sco.p_curr_scb = p_scb;
      bta_ag_cb.sco.cur_idx = p_scb->sco_idx;
#if (TWS_AG_ENABLED == TRUE)
  }
#endif

  /* Local device requested SCO connection to peer */
  if (is_local) {
#if (SWB_ENABLED == TRUE)
    if (p_scb->is_swb_codec == true && !p_scb->codec_updated) {
      if (p_scb->codec_swb_settings == BTA_AG_SCO_SWB_SETTINGS_Q3) {
        params = esco_parameters_for_codec(ESCO_CODEC_SWB_Q3);
      } else if (p_scb->codec_swb_settings == BTA_AG_SCO_SWB_SETTINGS_Q2) {
        params = esco_parameters_for_codec(ESCO_CODEC_SWB_Q2);
      } else if (p_scb->codec_swb_settings == BTA_AG_SCO_SWB_SETTINGS_Q1) {
        params = esco_parameters_for_codec(ESCO_CODEC_SWB_Q1);
      } else if (p_scb->codec_swb_settings == BTA_AG_SCO_SWB_SETTINGS_Q0) {
        params = esco_parameters_for_codec(ESCO_CODEC_SWB_Q0);
      }
    }
#endif
    if (esco_codec == BTA_AG_CODEC_MSBC) {
      if (p_scb->codec_msbc_settings == BTA_AG_SCO_MSBC_SETTINGS_T2) {
        params = esco_parameters_for_codec(ESCO_CODEC_MSBC_T2);
      } else
        params = esco_parameters_for_codec(ESCO_CODEC_MSBC_T1);
    } else {
      params = esco_parameters_for_codec(ESCO_CODEC_CVSD);
      if ((!(p_scb->features & BTA_AG_FEAT_ESCO)) ||
          (!(p_scb->peer_features & BTA_AG_PEER_FEAT_ESCO))) {
        params.max_latency_ms = 10;
        params.retransmission_effort = ESCO_RETRANSMISSION_POWER;
      }
    }

    /* Bypass voice settings if enhanced SCO setup command is supported */
    if (!(controller_get_interface()
              ->supports_enhanced_setup_synchronous_connection() &&
          (soc_type == BT_SOC_TYPE_CHEROKEE || soc_type == BT_SOC_TYPE_HASTINGS))) {
#if (BLUETOOTH_QTI_SW == FALSE) /* These changes are not needed*/
      if (esco_codec == BTA_AG_CODEC_MSBC)
        BTM_WriteVoiceSettings(BTM_VOICE_SETTING_TRANS);
      else
        BTM_WriteVoiceSettings(BTM_VOICE_SETTING_CVSD);
#endif
    }

#if (BTM_SCO_HCI_INCLUDED == TRUE)
    /* initialize SCO setup, no voice setting for AG, data rate <==> sample
     * rate */
    BTM_ConfigScoPath(params.input_data_path, bta_ag_sco_read_cback, NULL,
                      TRUE);
#endif

    tBTM_STATUS status = BTM_CreateSco(
        &p_scb->peer_addr, true, params.packet_types, &p_scb->sco_idx,
        bta_ag_sco_conn_cback, bta_ag_sco_disc_cback);
    if (status == BTM_CMD_STARTED) {
      /* Initiating the connection, set the current sco handle */
#if (TWS_AG_ENABLED == TRUE)
      if (is_twsp_device(p_scb->peer_addr)) {
          if (p_scb == bta_ag_cb.sec_sm_scb)
              bta_ag_cb.twsp_sec_sco.cur_idx = p_scb->sco_idx;
          else if (p_scb == bta_ag_cb.main_sm_scb){
              APPL_TRACE_DEBUG("%s: updating cur_idx of sco to: %d", __func__,  p_scb->sco_idx);
              bta_ag_cb.sco.cur_idx = p_scb->sco_idx;
          } else {
              APPL_TRACE_ERROR("%s: Invalid p_scb: %d", __func__, p_scb);
          }
      } else {
#endif
          bta_ag_cb.sco.cur_idx = p_scb->sco_idx;
#if (TWS_AG_ENABLED == TRUE)
      }
#endif
    }
    APPL_TRACE_DEBUG("%s: initiated SCO connection", __func__);
  } else {
    /* Local device accepted SCO connection from peer */
    params = esco_parameters_for_codec(ESCO_CODEC_CVSD);
    if ((!(p_scb->features & BTA_AG_FEAT_ESCO)) ||
        (!(p_scb->peer_features & BTA_AG_PEER_FEAT_ESCO))) {
      params.max_latency_ms = 10;
      params.retransmission_effort = ESCO_RETRANSMISSION_POWER;
    }

    BTM_EScoConnRsp(p_scb->sco_idx, HCI_SUCCESS, &params);
    APPL_TRACE_DEBUG("%s: listening for SCO connection", __func__);
  }
}

/*******************************************************************************
 *
 * Function         bta_ag_codec_negotiation_timer_cback
 *
 * Description
 *
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_ag_codec_negotiation_timer_cback(void* data) {
  APPL_TRACE_DEBUG("%s", __func__);
  tBTA_AG_SCB* p_scb = (tBTA_AG_SCB*)data;
  bool is_blacklisted = interop_match_addr(INTEROP_DISABLE_CODEC_NEGOTIATION,
                                           &p_scb->peer_addr);
  /* Announce that codec negotiation failed. */
  bta_ag_sco_codec_nego(p_scb, false);
#if (TWS_AG_ENABLED == TRUE)
  if (is_twsp_device(p_scb->peer_addr)) {
     APPL_TRACE_IMP("%s: tws device %s  codec negotiation fail.. skip blacklist",
                    __func__, p_scb->peer_addr.ToString().c_str());
  } else {
#endif
  // add the device to blacklisting to disable codec negotiation
    if (is_blacklisted == false) {
      APPL_TRACE_IMP("%s: blacklisting device %s for codec negotiation",
                    __func__, p_scb->peer_addr.ToString().c_str());
#if (SWB_ENABLED == TRUE)
      if (p_scb->is_swb_codec == false) {
#endif
        interop_database_add(INTEROP_DISABLE_CODEC_NEGOTIATION,
                         &p_scb->peer_addr, 3);
#if (SWB_ENABLED == TRUE)
      } else {
        APPL_TRACE_IMP("%s: Ignore blacklisting SWB device  %s for codec negotiation",
                      __func__, p_scb->peer_addr.ToString().c_str());
      }
#endif
    } else {
       APPL_TRACE_IMP("%s: dev %s is already blacklisted for codec negotiation",
                     __func__, p_scb->peer_addr.ToString().c_str());
    }
#if (TWS_AG_ENABLED == TRUE)
  }
#endif
  /* call app callback */
  bta_ag_cback_sco(p_scb, BTA_AG_AUDIO_CLOSE_EVT);
}

/*******************************************************************************
 *
 * Function         bta_ag_codec_negotiate
 *
 * Description      Initiate codec negotiation by sending AT command.
 *                  If not necessary, skip negotiation.
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_ag_codec_negotiate(tBTA_AG_SCB* p_scb) {
  APPL_TRACE_DEBUG("%s p_scb", __func__, p_scb);

#if (TWS_AG_ENABLED == TRUE)
  if (is_twsp_device(p_scb->peer_addr)) {
     if (bta_ag_cb.main_sm_scb == p_scb)
        bta_ag_cb.sco.p_curr_scb = p_scb;
     else if (bta_ag_cb.sec_sm_scb == p_scb){
        APPL_TRACE_DEBUG("%s:TWS codec nego : %x  %x",
             __func__, bta_ag_cb.twsp_sec_sco.p_curr_scb, p_scb);
        bta_ag_cb.twsp_sec_sco.p_curr_scb = p_scb;
     } else {
        APPL_TRACE_ERROR("%s: Invalid TWSP case",__func__);
     }
  } else {
#endif
     bta_ag_cb.sco.p_curr_scb = p_scb;
#if (TWS_AG_ENABLED == TRUE)
  }
#endif

  if (((p_scb->codec_updated || p_scb->codec_fallback) &&
      (p_scb->peer_features & BTA_AG_PEER_FEAT_CODEC))
#if (SWB_ENABLED == TRUE)
      || ((get_swb_codec_status() || p_scb->is_swb_codec)
      && (p_scb->peer_codecs & BTA_AG_SCO_SWB_SETTINGS_Q0_MASK))
#endif
     ) {
    /* Change the power mode to Active until SCO open is completed. */
    bta_sys_busy(BTA_ID_AG, p_scb->app_id, p_scb->peer_addr);


#if (SWB_ENABLED == TRUE)
    if (get_swb_codec_status() && (p_scb->peer_codecs & BTA_AG_SCO_SWB_SETTINGS_Q0_MASK)) {
      if (p_scb->is_swb_codec == false) {
        p_scb->sco_codec = BTA_AG_SCO_SWB_SETTINGS_Q0;
        p_scb->is_swb_codec = true;
      }
      /* Send +QCS to the peer */
      bta_ag_send_qcs(p_scb, NULL);
    } else
#endif
    {
#if (SWB_ENABLED == TRUE)
      if (p_scb->is_swb_codec == true  && (p_scb->peer_codecs & BTA_AG_SCO_SWB_SETTINGS_Q0_MASK)) {
        p_scb->sco_codec = BTM_SCO_CODEC_MSBC;
        p_scb->is_swb_codec = false;
      }
#endif
      /* Send +BCS to the peer */
      bta_ag_send_bcs(p_scb, NULL);
    }

    /* Start timer to handle timeout */
    alarm_set_on_mloop(p_scb->codec_negotiation_timer,
                       BTA_AG_CODEC_NEGOTIATION_TIMEOUT_MS,
                       bta_ag_codec_negotiation_timer_cback, p_scb);
  } else {
    /* use same codec type as previous SCO connection, skip codec negotiation */
    APPL_TRACE_DEBUG(
        "use same codec type as previous SCO connection,skip codec "
        "negotiation");
    bta_ag_sco_codec_nego(p_scb, true);
  }
}

bool bta_ag_is_other_legacy_device_connected(tBTA_AG_SCB* p_curr_scb) {
  tBTA_AG_SCB* p_scb = &bta_ag_cb.scb[0];
  int i;

  for (i = 0; i < BTA_AG_MAX_NUM_CLIENTS; i++, p_scb++) {
    if (p_scb->in_use && p_scb != p_curr_scb &&
#if (TWS_AG_ENABLED == TRUE)
        !is_twsp_device(p_scb->peer_addr) &&  /*ignore tws scb*/
#endif
        p_scb->state == BTA_AG_OPEN_ST) {
      return true;
    }
  }

  /* no other scb found */
  APPL_TRACE_DEBUG("%s: No other legacy device connected", __func__);
  return false;
}


/*******************************************************************************
 *
 * Function         bta_ag_sco_event
 *
 * Description
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_ag_sco_event(tBTA_AG_SCB* p_scb, uint8_t event) {
  tBTA_AG_SCO_CB* p_sco = &bta_ag_cb.sco;
  uint8_t in_state;
#if (BTM_SCO_HCI_INCLUDED == TRUE)
  BT_HDR* p_buf;
#endif

  APPL_TRACE_IMP("%s: index=0x%04x, device=%s, state=%s[%d], event=%s[%d]",
                   __func__, p_scb->sco_idx,
                   p_scb->peer_addr.ToString().c_str(),
                   bta_ag_sco_state_str(p_sco->state), p_sco->state,
                   bta_ag_sco_evt_str(event), event);

#if (BTM_SCO_HCI_INCLUDED == TRUE)
  if (event == BTA_AG_SCO_CI_DATA_E) {
    while (true) {
      bta_dm_sco_co_out_data(&p_buf);
      if (p_buf) {
        if (p_sco->state == BTA_AG_SCO_OPEN_ST)
          BTM_WriteScoData(p_sco->p_curr_scb->sco_idx, p_buf);
        else
          osi_free(p_buf);
      } else
        break;
    }

    return;
  }
#endif
  APPL_TRACE_DEBUG("%s: p_scb: %x\n", __func__, p_scb);
  in_state = p_sco->state;

  switch (p_sco->state) {
    case BTA_AG_SCO_SHUTDOWN_ST:
      switch (event) {
        case BTA_AG_SCO_LISTEN_E:
#if (TWS_AG_ENABLED == TRUE)
          if (is_twsp_device(p_scb->peer_addr)) {
              bta_ag_cb.main_sm_scb = p_scb;
          }
#endif
          /* create sco listen connection */
          bta_ag_create_sco(p_scb, false);
          p_sco->state = BTA_AG_SCO_LISTEN_ST;
#if (TWS_AG_ENABLED == TRUE)
          /* If the current device is TWS+
           * then trigger sending VS for eSCO setup*/
          if (is_twsp_device(p_scb->peer_addr)) {
              twsp_select_microphone(get_other_twsp_scb(p_scb->peer_addr), p_scb);
          }
#endif
          break;
        case BTA_AG_SCO_OPEN_E:
          // send SCO close event to btif to ensure proper cleanup
          bta_ag_cback_sco(p_scb, BTA_AG_AUDIO_CLOSE_EVT);
          APPL_TRACE_WARNING(
              "%s: BTA_AG_SCO_SHUTDOWN_ST: Ignoring event %s[%d]", __func__,
              bta_ag_sco_evt_str(event), event);
          break;

        default:
          APPL_TRACE_WARNING(
              "%s: BTA_AG_SCO_SHUTDOWN_ST: Ignoring event %s[%d]", __func__,
              bta_ag_sco_evt_str(event), event);
          break;
      }
      break;

    case BTA_AG_SCO_LISTEN_ST:
      switch (event) {
        case BTA_AG_SCO_LISTEN_E:
          /* create sco listen connection (Additional channel) */
#if (TWS_AG_ENABLED == TRUE)
        /* If One earbud connected main_sm_scb will not be NULL*/
        if (is_twsp_device(p_scb->peer_addr)) {
                if (bta_ag_cb.main_sm_scb != NULL) {
                    if(is_rfc_connected(p_scb)) {
                        //trigger twsp peer listen state
                        //As the SCO state is in listen state, it must be for the TWS+ peerdevice
                        bta_ag_twsp_sco_event(p_scb, BTA_AG_SCO_LISTEN_E);
                    } else {
                        APPL_TRACE_WARNING("%s: twsp peer rfc is not connected", __func__);
                    }
                } else {
                    bta_ag_cb.main_sm_scb = p_scb;
                }
        } else {
#endif
            bta_ag_create_sco(p_scb, false);
#if (TWS_AG_ENABLED == TRUE)
        }
#endif
         break;

        case BTA_AG_SCO_OPEN_E:
          /* remove listening connection */
          bta_ag_remove_sco(p_scb, false);

          /* start codec negotiation */
          if (p_scb->peer_codecs != BTA_AG_CODEC_NONE)
          {
              p_sco->state = BTA_AG_SCO_CODEC_ST;
              bta_ag_codec_negotiate(p_scb);
          }
          else
          {
              if (bta_ag_create_sco(p_scb, TRUE)) {
                  p_sco->state = BTA_AG_SCO_OPENING_ST;
              } else {
                  APPL_TRACE_WARNING("%s: create sco connection failed", __func__);
                  p_sco->state = BTA_AG_SCO_LISTEN_ST;
                  bta_ag_cback_sco(p_scb, BTA_AG_AUDIO_CLOSE_EVT);
              }
          }
          break;

        case BTA_AG_SCO_SHUTDOWN_E:
          /* remove listening connection */
          bta_ag_remove_sco(p_scb, false);


          /* If last SCO instance then finish shutting down */
          if (!bta_ag_other_scb_open(p_scb)) {
            p_sco->state = BTA_AG_SCO_SHUTDOWN_ST;
          } else /* Other instance is still listening */
          {
            p_sco->state = BTA_AG_SCO_LISTEN_ST;
          }
#if (TWS_AG_ENABLED == TRUE)
          if (is_twsp_device(p_scb->peer_addr)) {
              if (p_scb == p_sco->p_curr_scb) {
                  if (p_scb == bta_ag_cb.main_sm_scb) {
                      if(bta_ag_is_other_legacy_device_connected(p_scb) != true) {
                          p_sco->state = BTA_AG_SCO_SHUTDOWN_ST;
                          APPL_TRACE_WARNING("%s: moved to shutdown st",__func__);
                      } else {
                          //If there is Legacy Connection
                          //Keep it back in LISTEN_ST
                          p_sco->state = BTA_AG_SCO_LISTEN_ST;
                      }
                  }
              }
              if (p_scb == bta_ag_cb.main_sm_scb) {
                  bta_ag_cb.main_sm_scb = NULL;
              }
          }
#endif
          if (p_scb == p_sco->p_curr_scb) {
            p_sco->p_curr_scb = NULL;
          }

          break;

        case BTA_AG_SCO_CLOSE_E:
          /* remove listening connection */
          /* Ignore the event. Keep listening SCO for the active SLC
           */
          APPL_TRACE_WARNING("%s: BTA_AG_SCO_LISTEN_ST: Ignoring event %s[%d]",
                             __func__, bta_ag_sco_evt_str(event), event);
          break;

        case BTA_AG_SCO_CONN_CLOSE_E:
          /* sco failed; create sco listen connection */
          bta_ag_create_sco(p_scb, false);
          p_sco->state = BTA_AG_SCO_LISTEN_ST;
          break;

        default:
          APPL_TRACE_WARNING("%s: BTA_AG_SCO_LISTEN_ST: Ignoring event %s[%d]",
                             __func__, bta_ag_sco_evt_str(event), event);
          break;
      }
      break;

    case BTA_AG_SCO_CODEC_ST:
      switch (event) {
        case BTA_AG_SCO_LISTEN_E:
#if (TWS_AG_ENABLED == TRUE)
        /* If One earbud connected main_sm_scb will not be NULL*/
        if (is_twsp_device(p_scb->peer_addr)) {
                if (bta_ag_cb.main_sm_scb != NULL) {
                    if(is_rfc_connected(p_scb)) {
                        //trigger twsp peer listen state
                        //As the SCO state is in listen state, it must be for the TWS+ peerdevice
                        bta_ag_twsp_sco_event(p_scb, BTA_AG_SCO_LISTEN_E);
                    } else {
                        APPL_TRACE_WARNING("%s: twsp peer rfc is not connected", __func__);
                    }
                } else {
                    bta_ag_cb.main_sm_scb = p_scb;
                }
          } else {
#endif
             /* create sco listen connection (Additional channel) */
             bta_ag_create_sco(p_scb, false);
#if (TWS_AG_ENABLED == TRUE)
          }
#endif
          break;

        case BTA_AG_SCO_CN_DONE_E:
          /* create sco connection to peer */
#if (TWS_AG_ENABLED == TRUE)
          if (is_twsp_device(p_scb->peer_addr)) {
              twsp_select_microphone(get_other_twsp_scb(p_scb->peer_addr), p_scb);
          }
#endif
          if (bta_ag_create_sco(p_scb, true)) {
              p_sco->state = BTA_AG_SCO_OPENING_ST;
          } else {
              APPL_TRACE_WARNING("%s: create sco connection failed", __func__);
              p_sco->state = BTA_AG_SCO_LISTEN_ST;
              bta_ag_cback_sco(p_scb, BTA_AG_AUDIO_CLOSE_EVT);
          }
          break;

        case BTA_AG_SCO_XFER_E:
          /* save xfer scb */
          if (!sco_init_rmt_xfer) {
              p_sco->p_xfer_scb = p_scb;
              p_sco->state = BTA_AG_SCO_CLOSE_XFER_ST;
          } else {
              //IGN SCO XFER if it is in the middle
              //remote initiated transfer
              APPL_TRACE_WARNING("%s:Ignoring SCO XFER @ state: %d",
                                            __func__, p_sco->state);
          }
          break;

        case BTA_AG_SCO_SHUTDOWN_E:
          /* remove listening connection */
          bta_ag_remove_sco(p_scb, false);

          /* If last SCO instance then finish shutting down */
          if (!bta_ag_other_scb_open(p_scb)) {
            p_sco->state = BTA_AG_SCO_SHUTDOWN_ST;
          } else if (p_scb == p_sco->p_curr_scb) {
            p_sco->state = BTA_AG_SCO_LISTEN_ST;
          }

#if (TWS_AG_ENABLED == TRUE)
          if (is_twsp_device(p_scb->peer_addr)) {
             if (p_scb == p_sco->p_curr_scb) {
                 if (p_scb == bta_ag_cb.main_sm_scb) {
                     if(bta_ag_is_other_legacy_device_connected(p_scb) != true) {
                         p_sco->state = BTA_AG_SCO_SHUTDOWN_ST;
                     } else {
                         //keep main SM back in LISTEN
                         //as there is some legac Connection
                         p_sco->state = BTA_AG_SCO_LISTEN_ST;
                     }
                 }
             }
             if (p_scb == bta_ag_cb.main_sm_scb) {
                 bta_ag_cb.main_sm_scb = NULL;
             }
          }
#endif
          if (p_scb == p_sco->p_curr_scb) {
            p_sco->p_curr_scb = NULL;
          }
          break;

        case BTA_AG_SCO_CLOSE_E:
          /* sco open is not started yet. just go back to listening */
          p_sco->state = BTA_AG_SCO_LISTEN_ST;

          if (p_scb == p_sco->p_curr_scb) {
             p_sco->p_curr_scb->sco_idx = BTM_INVALID_SCO_INDEX;
             p_sco->p_curr_scb = NULL;
          }
          APPL_TRACE_WARNING("%s: perform post sco close action to send indicator", __func__);
          bta_ag_post_sco_close(p_scb, NULL);
          /* call app callback so that btif and app state
          /  go back to audio disconnected state */
          APPL_TRACE_WARNING("%s: SCO close during codec negotiation", __func__);
          bta_ag_cback_sco(p_scb, BTA_AG_AUDIO_CLOSE_EVT);
          break;

        case BTA_AG_SCO_CONN_CLOSE_E:
          /* sco failed; create sco listen connection */
          bta_ag_create_sco(p_scb, false);
          p_sco->state = BTA_AG_SCO_LISTEN_ST;

#if (TWS_AG_ENABLED == TRUE)
          //Atleast try connecting to 2nd twsp
          if (is_twsp_device(p_scb->peer_addr) && !p_scb->rmt_sco_req) {
             tBTA_AG_SCB *other_scb = get_other_twsp_scb((p_scb->peer_addr));
             if (other_scb && twsp_sco_active(other_scb) == false &&
                     get_twsp_state(other_scb) != TWSPLUS_EB_STATE_OUT_OF_EAR &&
                     get_twsp_state(other_scb) != TWSPLUS_EB_STATE_INCASE) {
                 dispatch_event_twsp_peer_device(p_scb, BTA_AG_SCO_OPEN_E);
             }
          }
          p_scb->rmt_sco_req = FALSE;
#endif
          break;

        default:
          APPL_TRACE_WARNING("%s: BTA_AG_SCO_CODEC_ST: Ignoring event %s[%d]",
                             __func__, bta_ag_sco_evt_str(event), event);
          break;
      }
      break;

    case BTA_AG_SCO_OPENING_ST:
      switch (event) {
        case BTA_AG_SCO_LISTEN_E:
          /* second headset has now joined */
          /* create sco listen connection (Additional channel) */
          if (p_scb != bta_ag_cb.sco.p_curr_scb) {
#if (TWS_AG_ENABLED == TRUE)
          /* If One earbud connected main_sm_scb will not be NULL*/
          if (is_twsp_device(p_scb->peer_addr)) {
              if (bta_ag_cb.main_sm_scb != NULL) {
                 if(is_rfc_connected(p_scb)) {
                      //trigger twsp peer listen state
                      //As the SCO state is in listen state, it must be for the TWS+ peerdevice
                      bta_ag_twsp_sco_event(p_scb, BTA_AG_SCO_LISTEN_E);
                 } else {
                      APPL_TRACE_WARNING("%s: twsp peer rfc is not connected", __func__);
                 }
              } else {
                    bta_ag_cb.main_sm_scb = p_scb;
              }
          }
          else {
#endif
                 bta_ag_create_sco(p_scb, false);
#if (TWS_AG_ENABLED == TRUE)
              }
#endif
          }
          break;

        case BTA_AG_SCO_REOPEN_E:
          /* start codec negotiation */
          p_sco->state = BTA_AG_SCO_CODEC_ST;
          bta_ag_codec_negotiate(p_scb);
          break;

        case BTA_AG_SCO_XFER_E:
          if (!sco_init_rmt_xfer) {
              /* save xfer scb */
              p_sco->p_xfer_scb = p_scb;
              p_sco->state = BTA_AG_SCO_CLOSE_XFER_ST;
          } else {
              //IGN SCO XFER if it is in the middle
              //remote initiated transfer
              APPL_TRACE_WARNING("%s:Ignoring SCO XFER @ state: %d",
                                               __func__, p_sco->state);
          }
          break;

        case BTA_AG_SCO_CLOSE_E:
#if (TWS_AG_ENABLED == TRUE)
          if (is_twsp_device(p_scb->peer_addr) &&
                p_scb != bta_ag_cb.sco.p_curr_scb) {
              //If this request is for a TWSPLUS device
              //and It is different from curr_scb. Ignore the request
             APPL_TRACE_WARNING("%s: TWS+: SCO_CLOSE is ignored as it is not matching", __func__);
          } else {
#endif
              p_sco->state = BTA_AG_SCO_OPEN_CL_ST;
#if (TWS_AG_ENABLED == TRUE)
          }
#endif
          break;

        case BTA_AG_SCO_SHUTDOWN_E:
          /* remove listening connection */
          bta_ag_remove_sco(p_scb, FALSE);

          /* If last SCO instance then finish shutting down */
          if (!bta_ag_other_scb_open(p_scb)) {
            p_sco->state = BTA_AG_SCO_SHUTDOWN_ST;
          } else /* Other instance is still listening */
          {
            p_sco->state = BTA_AG_SCO_LISTEN_ST;
          }
#if (TWS_AG_ENABLED == TRUE)
          if (is_twsp_device(p_scb->peer_addr)) {
             /* if the current device is TWS move it to SHUTDOWN as sms
              * operate independently*/
             if (p_scb == p_sco->p_curr_scb) {
                 if (bta_ag_cb.main_sm_scb == p_scb) {
                     if(bta_ag_is_other_legacy_device_connected(p_scb) != true) {
                         p_sco->state = BTA_AG_SCO_SHUTDOWN_ST;
                     } else {
                         p_sco->state = BTA_AG_SCO_LISTEN_ST;
                     }
                 }
             }
             if (bta_ag_cb.main_sm_scb == p_scb) {
                 bta_ag_cb.main_sm_scb = NULL;
             }
          }
#endif
          if (p_scb == p_sco->p_curr_scb) {
            p_sco->p_curr_scb = NULL;
          }

          break;

        case BTA_AG_SCO_CONN_OPEN_E:
          p_sco->state = BTA_AG_SCO_OPEN_ST;
          sco_init_rmt_xfer = false;
#if (TWS_AG_ENABLED == TRUE)
          //Once SCO connected
          if (is_twsp_device(p_scb->peer_addr) && !p_scb->rmt_sco_req) {
              tBTA_AG_SCB *other_scb = get_other_twsp_scb((p_scb->peer_addr));
              if (other_scb && twsp_sco_active(other_scb) == false &&
                      get_twsp_state(other_scb) != TWSPLUS_EB_STATE_OUT_OF_EAR &&
                      get_twsp_state(other_scb) != TWSPLUS_EB_STATE_INCASE) {
                  //trigger the secondary SCO connection for TWS
                  dispatch_event_twsp_peer_device(p_scb, BTA_AG_SCO_OPEN_E);
              }
          }
          p_scb->rmt_sco_req = FALSE;
#endif
          break;
        case BTA_AG_SCO_CONN_CLOSE_E:
          /* sco failed; create sco listen connection */
          bta_ag_create_sco(p_scb, false);
          p_sco->state = BTA_AG_SCO_LISTEN_ST;
#if (TWS_AG_ENABLED == TRUE)
          /* 1st earbud SCO is closed
             atleast try opening the secondary SCO */
          if (is_twsp_device(p_scb->peer_addr)&&!p_scb->rmt_sco_req) {
              tBTA_AG_SCB *other_scb = get_other_twsp_scb((p_scb->peer_addr));
              if (other_scb && twsp_sco_active(other_scb) == false &&
                      get_twsp_state(other_scb) != TWSPLUS_EB_STATE_OUT_OF_EAR &&
                      get_twsp_state(other_scb) != TWSPLUS_EB_STATE_INCASE) {
                 dispatch_event_twsp_peer_device(p_scb, BTA_AG_SCO_OPEN_E);
              }
          }
          p_scb->rmt_sco_req = FALSE;
#endif
          break;

        default:
          APPL_TRACE_WARNING("%s: BTA_AG_SCO_OPENING_ST: Ignoring event %s[%d]",
                             __func__, bta_ag_sco_evt_str(event), event);
          break;
      }
      break;

    case BTA_AG_SCO_OPEN_CL_ST:
      switch (event) {
        case BTA_AG_SCO_XFER_E:
          /* save xfer scb */
          p_sco->p_xfer_scb = p_scb;

          p_sco->state = BTA_AG_SCO_CLOSE_XFER_ST;
          break;

        case BTA_AG_SCO_OPEN_E:
          p_sco->state = BTA_AG_SCO_OPENING_ST;
          break;

        case BTA_AG_SCO_SHUTDOWN_E:
          /* remove listening connection */
          bta_ag_remove_sco(p_scb, FALSE);

          /* If last SCO instance then finish shutting down */
          if (!bta_ag_other_scb_open(p_scb)) {
            p_sco->state = BTA_AG_SCO_SHUTDOWN_ST;
          } else /* Other instance is still listening */
          {
            p_sco->state = BTA_AG_SCO_LISTEN_ST;
          }
#if (TWS_AG_ENABLED == TRUE)
          if (is_twsp_device(p_scb->peer_addr)) {
             if (p_scb == p_sco->p_curr_scb) {
                 if (p_scb == bta_ag_cb.main_sm_scb) {
                     if(bta_ag_is_other_legacy_device_connected(p_scb) != true) {
                         p_sco->state = BTA_AG_SCO_SHUTDOWN_ST;
                     } else {
                         p_sco->state = BTA_AG_SCO_LISTEN_ST;
                     }
                 }
             }
          }
          if (bta_ag_cb.main_sm_scb == p_scb) {
              bta_ag_cb.main_sm_scb = NULL;
             /* if the current device is TWS move it to SHUTDOWN as sms
              * operate independently*/
             p_sco->state = BTA_AG_SCO_SHUTDOWN_ST;
          }
#endif

          if (p_scb == p_sco->p_curr_scb) {
            p_sco->p_curr_scb = NULL;
          }
          break;

        case BTA_AG_SCO_CONN_OPEN_E:
          /* close sco connection */
          bta_ag_remove_sco(p_scb, true);

          p_sco->state = BTA_AG_SCO_CLOSING_ST;
          break;

        case BTA_AG_SCO_CONN_CLOSE_E:
          /* sco failed; create sco listen connection */

          p_sco->state = BTA_AG_SCO_LISTEN_ST;
          break;

        default:
          APPL_TRACE_WARNING("%s: BTA_AG_SCO_OPEN_CL_ST: Ignoring event %s[%d]",
                             __func__, bta_ag_sco_evt_str(event), event);
          break;
      }
      break;

    case BTA_AG_SCO_OPEN_XFER_ST:
      switch (event) {
        case BTA_AG_SCO_CLOSE_E:
          /* close sco connection */
          bta_ag_remove_sco(p_scb, true);

          p_sco->state = BTA_AG_SCO_CLOSING_ST;
          break;

        case BTA_AG_SCO_SHUTDOWN_E:
          /* remove all connection */
          bta_ag_remove_sco(p_scb, false);
          p_sco->state = BTA_AG_SCO_SHUTTING_ST;

          break;

        case BTA_AG_SCO_CONN_CLOSE_E:
          /* closed sco; place in listen mode and
             accept the transferred connection */
          bta_ag_create_sco(p_scb, false); /* Back into listen mode */

          /* Accept sco connection with xfer scb */
          bta_ag_sco_conn_rsp(p_sco->p_xfer_scb, &p_sco->conn_data);
          p_sco->state = BTA_AG_SCO_OPENING_ST;
          p_sco->p_curr_scb = p_sco->p_xfer_scb;
          p_sco->cur_idx = p_sco->p_xfer_scb->sco_idx;
          p_sco->p_xfer_scb = NULL;
          sco_init_rmt_xfer = true;
          break;

        default:
          APPL_TRACE_WARNING(
              "%s: BTA_AG_SCO_OPEN_XFER_ST: Ignoring event %s[%d]", __func__,
              bta_ag_sco_evt_str(event), event);
          break;
      }
      break;

    case BTA_AG_SCO_OPEN_ST:
      switch (event) {
        case BTA_AG_SCO_LISTEN_E:
          /* second headset has now joined */
          /* create sco listen connection (Additional channel) */
          if (p_scb != bta_ag_cb.sco.p_curr_scb) {
#if (TWS_AG_ENABLED == TRUE)
          /* If One earbud connected main_sm_scb will not be NULL*/
          if (is_twsp_device(p_scb->peer_addr)) {
              if (bta_ag_cb.main_sm_scb != NULL) {
                  if(is_rfc_connected(p_scb)) {
                      //trigger twsp peer listen state
                      //As the SCO state is in listen state, it must be for the TWS+ peerdevice
                      bta_ag_twsp_sco_event(p_scb, BTA_AG_SCO_LISTEN_E);
                  } else {
                      APPL_TRACE_WARNING("%s: twsp peer rfc is not connected", __func__);
                  }
              } else {
                    bta_ag_cb.main_sm_scb = p_scb;
              }
          } else {
#endif
                 bta_ag_create_sco(p_scb, false);
#if (TWS_AG_ENABLED == TRUE)
          }
#endif
          }
          break;

        case BTA_AG_SCO_XFER_E:
          /* close current sco connection */
          bta_ag_remove_sco(p_sco->p_curr_scb, true);

          /* save xfer scb */
          p_sco->p_xfer_scb = p_scb;

          p_sco->state = BTA_AG_SCO_CLOSE_XFER_ST;
          break;

        case BTA_AG_SCO_CLOSE_E:
          /* close sco connection if active */
          if (bta_ag_remove_sco(p_scb, true)) {
            p_sco->state = BTA_AG_SCO_CLOSING_ST;
          }
          break;

        case BTA_AG_SCO_SHUTDOWN_E: {
          /* remove all listening connections */
          bool sco_disc_init = bta_ag_remove_sco(p_scb, false);

#if (TWS_AG_ENABLED == TRUE)
          if (is_twsp_device(p_scb->peer_addr)) {

              if (p_scb == p_sco->p_curr_scb) {
                  if (sco_disc_init) {
                      p_sco->state = BTA_AG_SCO_SHUTTING_ST;
                  } else {
                      if (p_scb == bta_ag_cb.main_sm_scb) {
                          bta_ag_cb.main_sm_scb = NULL;
                      }

                      if(bta_ag_is_other_legacy_device_connected(p_scb) != true) {
                          p_sco->state = BTA_AG_SCO_SHUTDOWN_ST;
                      } else {
                          p_sco->state = BTA_AG_SCO_LISTEN_ST;
                      }
                  }
              } else {
                  if (p_scb == bta_ag_cb.main_sm_scb) {
                      bta_ag_cb.main_sm_scb = NULL;
                  }
              }
          }
          else {
#endif
          /* If SCO was active on this scb, close it */
          if (p_scb == p_sco->p_curr_scb) {
            if (sco_disc_init && p_scb->svc_conn) {
                p_sco->state = BTA_AG_SCO_SHUTTING_ST;
            } else {
                if (!bta_ag_other_scb_open(p_scb)) {
                    p_sco->state = BTA_AG_SCO_SHUTDOWN_ST;
                } else {
                   p_sco->state = BTA_AG_SCO_LISTEN_ST;
                }
            }
          }
#if (TWS_AG_ENABLED == TRUE)
          }
#endif

            if (p_scb == p_sco->p_curr_scb) {
              p_sco->p_curr_scb = NULL;
            }
          }
          break;

        case BTA_AG_SCO_CONN_CLOSE_E:
          /* peer closed sco; create sco listen connection */
          bta_ag_create_sco(p_scb, false);
          p_sco->state = BTA_AG_SCO_LISTEN_ST;

          break;

        default:
          APPL_TRACE_WARNING("%s: BTA_AG_SCO_OPEN_ST: Ignoring event %s[%d]",
                             __func__, bta_ag_sco_evt_str(event), event);
          break;
      }
      break;

    case BTA_AG_SCO_CLOSING_ST:
      switch (event) {
        case BTA_AG_SCO_LISTEN_E:
          /* create sco listen connection (Additional channel) */
          if (p_scb != bta_ag_cb.sco.p_curr_scb) {
#if (TWS_AG_ENABLED == TRUE)
          if (is_twsp_device(p_scb->peer_addr)) {
              if (bta_ag_cb.main_sm_scb != NULL) {
                  if(is_rfc_connected(p_scb)) {
                      //trigger twsp peer listen state
                      //As the SCO state is in listen state, it must be for the TWS+ peerdevice
                      bta_ag_twsp_sco_event(p_scb, BTA_AG_SCO_LISTEN_E);
                  } else {
                      APPL_TRACE_WARNING("%s: twsp peer rfc is not connected", __func__);
                  }
              } else {
                    bta_ag_cb.main_sm_scb = p_scb;
              }
          } else {
#endif
                  bta_ag_create_sco(p_scb, false);
#if (TWS_AG_ENABLED == TRUE)
              }
#endif
          } break;

        case BTA_AG_SCO_OPEN_E:
          p_sco->state = BTA_AG_SCO_CLOSE_OP_ST;
          break;

        case BTA_AG_SCO_XFER_E:
          /* save xfer scb */
          p_sco->p_xfer_scb = p_scb;

          p_sco->state = BTA_AG_SCO_CLOSE_XFER_ST;
          break;

        case BTA_AG_SCO_SHUTDOWN_E:
#if (TWS_AG_ENABLED == TRUE)
          if (is_twsp_device(p_scb->peer_addr)) {
              if (p_scb == p_sco->p_curr_scb) {
                  if(bta_ag_is_other_legacy_device_connected(p_scb) != true) {
                      p_sco->state = BTA_AG_SCO_SHUTTING_ST;
                  }
              } else {
                  if (bta_ag_cb.main_sm_scb == p_scb) {
                      bta_ag_cb.main_sm_scb = NULL;
                  }
              }
          }
          else {
#endif
          /* If not closing scb, just close it */
          if (p_scb != p_sco->p_curr_scb) {
            /* remove listening connection */
            bta_ag_remove_sco(p_scb, false);
          } else
            p_sco->state = BTA_AG_SCO_SHUTTING_ST;

#if (TWS_AG_ENABLED == TRUE)
          }
#endif

          if (p_scb == p_sco->p_curr_scb) {
            p_sco->p_curr_scb = NULL;
          }
          break;

        case BTA_AG_SCO_CONN_CLOSE_E:
          /* peer closed sco; create sco listen connection */
          bta_ag_create_sco(p_scb, false);

          p_sco->state = BTA_AG_SCO_LISTEN_ST;

#if (TWS_AG_ENABLED == TRUE)
          if (is_twsp_device(p_scb->peer_addr) &&
                 is_rfc_connected(get_other_twsp_scb((p_scb->peer_addr)))) {
                 dispatch_event_twsp_peer_device(p_scb, BTA_AG_SCO_CLOSE_E);
          }
#endif
          break;

        default:
          APPL_TRACE_WARNING("%s: BTA_AG_SCO_CLOSING_ST: Ignoring event %s[%d]",
                             __func__, bta_ag_sco_evt_str(event), event);
          break;
      }
      break;

    case BTA_AG_SCO_CLOSE_OP_ST:
      switch (event) {
        case BTA_AG_SCO_CLOSE_E:
          p_sco->state = BTA_AG_SCO_CLOSING_ST;
          break;

        case BTA_AG_SCO_SHUTDOWN_E:
          p_sco->state = BTA_AG_SCO_SHUTTING_ST;
          break;

        case BTA_AG_SCO_CONN_CLOSE_E:
          if (p_scb->peer_codecs != BTA_AG_CODEC_NONE)
          {
              /* start codec negotiation */
              p_sco->state = BTA_AG_SCO_CODEC_ST;
              bta_ag_codec_negotiate(p_scb);
          }
          else
          {
              if (bta_ag_create_sco(p_scb, TRUE)) {
                  p_sco->state = BTA_AG_SCO_OPENING_ST;
              } else {
                  APPL_TRACE_WARNING("%s: create sco connection failed", __func__);
                  p_sco->state = BTA_AG_SCO_LISTEN_ST;
                  bta_ag_cback_sco(p_scb, BTA_AG_AUDIO_CLOSE_EVT);
              }
          }
          break;

        case BTA_AG_SCO_LISTEN_E:
          /* create sco listen connection (Additional channel) */
          if (p_scb != p_sco->p_curr_scb) {
#if (TWS_AG_ENABLED == TRUE)
              if (is_twsp_device(p_scb->peer_addr) &&
                    is_rfc_connected(p_scb)) {
                 bta_ag_twsp_sco_event(p_scb, BTA_AG_SCO_LISTEN_E);
              } else {
#endif
                 bta_ag_create_sco(p_scb, false);
#if (TWS_AG_ENABLED == TRUE)
              }
#endif
          }
          break;

        default:
          APPL_TRACE_WARNING(
              "%s: BTA_AG_SCO_CLOSE_OP_ST: Ignoring event %s[%d]", __func__,
              bta_ag_sco_evt_str(event), event);
          break;
      }
      break;

    case BTA_AG_SCO_CLOSE_XFER_ST:
      switch (event) {
        case BTA_AG_SCO_CONN_OPEN_E:
          /* close sco connection so headset can be transferred
             Probably entered this state from "opening state" */
          bta_ag_remove_sco(p_scb, true);
          break;

        case BTA_AG_SCO_CLOSE_E:
          /* clear xfer scb */
          p_sco->p_xfer_scb = NULL;

          p_sco->state = BTA_AG_SCO_CLOSING_ST;
          break;

        case BTA_AG_SCO_SHUTDOWN_E:
          /* clear xfer scb */
          p_sco->p_xfer_scb = NULL;

          p_sco->state = BTA_AG_SCO_SHUTTING_ST;
          break;

        case BTA_AG_SCO_CONN_CLOSE_E: {
          /* closed sco; place old sco in listen mode,
             take current sco out of listen, and
             create originating sco for current */
          bta_ag_create_sco(p_scb, false);
#if (TWS_AG_ENABLED == TRUE)
          if (is_twsp_device(p_scb->peer_addr) &&
              is_rfc_connected(get_other_twsp_scb((p_scb->peer_addr)))
              ) {
              dispatch_event_twsp_peer_device(p_scb, BTA_AG_SCO_CLOSE_E);
          }
#endif

          bta_ag_remove_sco(p_sco->p_xfer_scb, false);

          if (p_scb->peer_codecs != BTA_AG_CODEC_NONE)
          {
              /* start codec negotiation */
              p_sco->state = BTA_AG_SCO_CODEC_ST;
              tBTA_AG_SCB* p_cn_scb = p_sco->p_xfer_scb;
              p_sco->p_xfer_scb = NULL;
              bta_ag_codec_negotiate(p_cn_scb);
          }
          else
          {
              bta_ag_create_sco(p_sco->p_xfer_scb, TRUE);
              p_sco->p_xfer_scb = NULL;
              p_sco->state = BTA_AG_SCO_OPENING_ST;
          }
          break;
        }

        default:
          APPL_TRACE_WARNING(
              "%s: BTA_AG_SCO_CLOSE_XFER_ST: Ignoring event %s[%d]", __func__,
              bta_ag_sco_evt_str(event), event);
          break;
      }
      break;

    case BTA_AG_SCO_SHUTTING_ST:
      switch (event) {
        case BTA_AG_SCO_CONN_OPEN_E:
          /* close sco connection; wait for conn close event */
          bta_ag_remove_sco(p_scb, true);
          break;

        case BTA_AG_SCO_CONN_CLOSE_E:
#if (TWS_AG_ENABLED == TRUE)
          if (is_twsp_device(p_scb->peer_addr) ) {
              if(bta_ag_is_other_legacy_device_connected(p_scb) != true) {
                  p_sco->state = BTA_AG_SCO_SHUTDOWN_ST;
              } else {
                 p_sco->state = BTA_AG_SCO_LISTEN_ST;
              }
              if (p_scb == bta_ag_cb.main_sm_scb) {
                 bta_ag_cb.main_sm_scb = NULL;
              }
          }
          else {
#endif
          /* If last SCO instance then finish shutting down */
          if (!bta_ag_other_scb_open(p_scb)) {
            p_sco->state = BTA_AG_SCO_SHUTDOWN_ST;
          } else /* Other instance is still listening */
          {
            p_sco->state = BTA_AG_SCO_LISTEN_ST;
          }

          /* If SCO closed for other HS which is not being disconnected,
             then create listen sco connection for it as scb still open */
          if (bta_ag_scb_open(p_scb)) {
            bta_ag_create_sco(p_scb, false);
            p_sco->state = BTA_AG_SCO_LISTEN_ST;
          }

          if (p_scb == p_sco->p_curr_scb) {
            p_sco->p_curr_scb->sco_idx = BTM_INVALID_SCO_INDEX;
            p_sco->p_curr_scb = NULL;
          }
#if (TWS_AG_ENABLED == TRUE)
          }
#endif
          break;

        case BTA_AG_SCO_LISTEN_E:
          /* create sco listen connection (Additional channel) */
          if (p_scb != bta_ag_cb.sco.p_curr_scb) {
#if (TWS_AG_ENABLED == TRUE)
          if (is_twsp_device(p_scb->peer_addr)) {
              if (bta_ag_cb.main_sm_scb != NULL) {
                  if(is_rfc_connected(p_scb)) {
                      //trigger twsp peer listen state
                      //As the SCO state is in listen state, it must be for the TWS+ peerdevice
                      bta_ag_twsp_sco_event(p_scb, BTA_AG_SCO_LISTEN_E);
                  } else {
                      APPL_TRACE_WARNING("%s: twsp peer rfc is not connected", __func__);
                  }
              } else {
                    bta_ag_cb.main_sm_scb = p_scb;
              }
          } else {
#endif
                  bta_ag_create_sco(p_scb, false);
#if (TWS_AG_ENABLED == TRUE)
              }
#endif
          }
          break;

        case BTA_AG_SCO_SHUTDOWN_E:
#if (TWS_AG_ENABLED == TRUE)
          if (is_twsp_device(p_scb->peer_addr)) {
             /* if the current device is TWS move it to SHUTDOWN as sms
              * operate independently*/
             if (p_scb == p_sco->p_curr_scb) {
                 if(bta_ag_is_other_legacy_device_connected(p_scb) != true) {
                     p_sco->state = BTA_AG_SCO_SHUTDOWN_ST;
                 } else {
                     p_sco->state = BTA_AG_SCO_LISTEN_ST;
                 }
              }
              if (p_scb == bta_ag_cb.main_sm_scb) {
                   bta_ag_cb.main_sm_scb = NULL;
              }
          }
          else {
#endif

          if (!bta_ag_other_scb_open(p_scb)) {
            p_sco->state = BTA_AG_SCO_SHUTDOWN_ST;
          } else /* Other instance is still listening */
          {
            p_sco->state = BTA_AG_SCO_LISTEN_ST;
          }

          if (p_scb == p_sco->p_curr_scb) {
            p_sco->p_curr_scb->sco_idx = BTM_INVALID_SCO_INDEX;
            p_sco->p_curr_scb = NULL;
          }
#if (TWS_AG_ENABLED == TRUE)
          }
#endif
          break;

        default:
          APPL_TRACE_WARNING(
              "%s: BTA_AG_SCO_SHUTTING_ST: Ignoring event %s[%d]", __func__,
              bta_ag_sco_evt_str(event), event);
          break;
      }
      break;

    default:
      break;
  }
  APPL_TRACE_IMP("BTA AG SCO State Change: [%s] -> [%s] after Event [%s]",
                           bta_ag_sco_state_str(in_state),
                           bta_ag_sco_state_str(p_sco->state),
                           bta_ag_sco_evt_str(event));
}

/*******************************************************************************
 *
 * Function         bta_ag_sco_is_open
 *
 * Description      Check if sco is open for this scb.
 *
 *
 * Returns          true if sco open for this scb, false otherwise.
 *
 ******************************************************************************/
bool bta_ag_sco_is_open(tBTA_AG_SCB* p_scb) {
  tBTA_AG_SCO_CB *sco_hdl = NULL;
#if (TWS_AG_ENABLED == TRUE)
  if (is_twsp_device(p_scb->peer_addr)) {
     if (p_scb == bta_ag_cb.main_sm_scb) {
        sco_hdl = &bta_ag_cb.sco;
     } else if (p_scb == bta_ag_cb.sec_sm_scb) {
        APPL_TRACE_DEBUG("%s:It is TWSP sco handle", __func__);
        sco_hdl = &bta_ag_cb.twsp_sec_sco;
     } else {
        APPL_TRACE_ERROR("%s: Invalid scb handle", __func__);
        return false;
     }
  } else {
#endif
      sco_hdl = &bta_ag_cb.sco;
#if (TWS_AG_ENABLED == TRUE)
  }
#endif

  APPL_TRACE_DEBUG("%s: returning : %d", __func__, ((sco_hdl->state == BTA_AG_SCO_OPEN_ST) &&
          (sco_hdl->p_curr_scb == p_scb)));
  APPL_TRACE_DEBUG("%s: pscb: %x, curr_scb: %x", __func__, p_scb, sco_hdl->p_curr_scb);
  return ((sco_hdl->state == BTA_AG_SCO_OPEN_ST) &&
          (sco_hdl->p_curr_scb == p_scb));
}

/*******************************************************************************
 *
 * Function         bta_ag_sco_is_opening
 *
 * Description      Check if sco is in Opening state.
 *
 *
 * Returns          true if sco is in Opening state for this scb, false
 *                  otherwise.
 *
 ******************************************************************************/
bool bta_ag_sco_is_opening(tBTA_AG_SCB* p_scb) {
  tBTA_AG_SCO_CB *sco_hdl = NULL;
#if (TWS_AG_ENABLED == TRUE)
  if (is_twsp_device(p_scb->peer_addr)) {
     if (p_scb == bta_ag_cb.main_sm_scb) {
        sco_hdl = &bta_ag_cb.sco;
     } else if (p_scb == bta_ag_cb.sec_sm_scb) {
        APPL_TRACE_DEBUG("%s:It is TWSP sco handle", __func__);
        sco_hdl = &bta_ag_cb.twsp_sec_sco;
     } else {
        APPL_TRACE_ERROR("%s: Invalid scb handle", __func__);
        return false;
     }
  } else {
#endif
      sco_hdl = &bta_ag_cb.sco;
#if (TWS_AG_ENABLED == TRUE)
  }
#endif

  APPL_TRACE_DEBUG("%s: returning : %d", __func__,
             ((sco_hdl->state == BTA_AG_SCO_OPENING_ST
           || sco_hdl->state == BTA_AG_SCO_CODEC_ST) &&
           (sco_hdl->p_curr_scb == p_scb)));

  return ((sco_hdl->state == BTA_AG_SCO_OPENING_ST
           || sco_hdl->state == BTA_AG_SCO_CODEC_ST) &&
           (sco_hdl->p_curr_scb == p_scb));
}

/*******************************************************************************
 *
 * Function         bta_ag_is_sco_present_on_any_device
 *
 * Description      Check if sco is present on any device.
 *
 *
 * Returns          true if sco is in Open/Opening/Closing state for any scb, false
 *                  otherwise.
 *
 ******************************************************************************/
bool bta_ag_is_sco_present_on_any_device() {
  tBTA_AG_SCO_CB *sco_hdl = NULL;
  bool ret_val = false;

  sco_hdl = &bta_ag_cb.sco;

  ret_val = sco_hdl->state == BTA_AG_SCO_OPEN_ST ||
            sco_hdl->state == BTA_AG_SCO_OPENING_ST ||
            sco_hdl->state == BTA_AG_SCO_CODEC_ST ||
            sco_hdl->state == BTA_AG_SCO_CLOSING_ST;

  APPL_TRACE_IMP("%s: returning : %d", __func__, ret_val);

  return ret_val;
}

/*******************************************************************************
 *
 * Function         bta_ag_sco_listen
 *
 * Description
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_ag_sco_listen(tBTA_AG_SCB* p_scb, UNUSED_ATTR tBTA_AG_DATA* p_data) {
  LOG(INFO) << __func__ << ": " << p_scb->peer_addr;
  bta_ag_sco_event(p_scb, BTA_AG_SCO_LISTEN_E);
}

/*******************************************************************************
 *
 * Function         bta_ag_sco_open
 *
 * Description
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_ag_sco_open(tBTA_AG_SCB* p_scb, UNUSED_ATTR tBTA_AG_DATA* p_data) {
  int event;
  if (!sco_allowed) {
    APPL_TRACE_DEBUG("%s not opening sco, by policy", __func__);
    return;
  }

  APPL_TRACE_DEBUG("%s: p_scb: %x", __func__, p_scb);
  /* if another scb using sco, this is a transfer */
#if (TWS_AG_ENABLED == TRUE)
  if (is_twsp_device(p_scb->peer_addr)) {
      if (bta_ag_cb.main_sm_scb != p_scb
        && p_scb == bta_ag_cb.sec_sm_scb) {
            if (bta_ag_cb.sco.state != BTA_AG_SCO_LISTEN_ST &&
                bta_ag_cb.sco.state != BTA_AG_SCO_SHUTDOWN_ST &&
                bta_ag_cb.sco.state != BTA_AG_SCO_OPEN_ST &&
                (bta_ag_cb.sco.p_curr_scb &&
                !is_twsp_device(bta_ag_cb.sco.p_curr_scb->peer_addr))) {
                APPL_TRACE_DEBUG("%s: primary sco SM is not in stable state",
                                                                 __func__);
                APPL_TRACE_DEBUG("%s: Ignore SCO request on secondary SM",
                                                                 __func__);
                //This should be part of QueryPhoneState where device on sec sco
                //SM queries phonestate when legacy device on primary SCO SM
                //in process of closing or in any other intermediate state
                //When Primary SCO SM is not in stable state and It is serving
                //legacy HF device, Ignore sco on secondary SM
                bta_ag_cback_sco(p_scb, BTA_AG_AUDIO_CLOSE_EVT);
            } else {
                //If primary SCO is in stable state and current req is TWS
                //peer address trigger secondary SCO open
                //Stable states: OPEN, LISTEN, SHUTDOWN
                bta_ag_twsp_sco_event(p_scb, BTA_AG_SCO_OPEN_E);
            }
            return;
      }
      else {
          if (bta_ag_cb.sco.p_curr_scb != NULL &&
                  bta_ag_cb.sco.p_curr_scb != p_scb &&
                  !is_twsp_device(bta_ag_cb.sco.p_curr_scb->peer_addr)) {
              /*main SM is occupied by a non-tws device
                consider It as SCO transfer
               */
              event = BTA_AG_SCO_XFER_E;
          } else {
              event = BTA_AG_SCO_OPEN_E;
          }
      }
  }
  /* else it is legacy */
  else {
     if (bta_ag_cb.twsp_sec_sco.state != BTA_AG_SCO_SHUTDOWN_ST &&
         bta_ag_cb.twsp_sec_sco.state != BTA_AG_SCO_LISTEN_ST) {
          APPL_TRACE_DEBUG("%s: Ignore SCO connection as secondary EB is not in stable state", __func__);
          //This should be part of QueryPhoneState where
          //secondary EB is still not done with Disconnection
          bta_ag_cback_sco(p_scb, BTA_AG_AUDIO_CLOSE_EVT);
          return;
      }
#endif
     if (bta_ag_cb.sco.p_curr_scb != NULL && bta_ag_cb.sco.p_curr_scb != p_scb) {
         LOG(INFO) << __func__ << ": tranfer " << bta_ag_cb.sco.p_curr_scb->peer_addr
              << " -> " << p_scb->peer_addr;
         event = BTA_AG_SCO_XFER_E;
     } else {
         /* else it is an open */
         LOG(INFO) << __func__ << ": open " << p_scb->peer_addr;
         event = BTA_AG_SCO_OPEN_E;
     }
#if (TWS_AG_ENABLED == TRUE)
  }
#endif
  bta_ag_sco_event(p_scb, event);
}

/*******************************************************************************
 *
 * Function         bta_ag_sco_close
 *
 * Description
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_ag_sco_close(tBTA_AG_SCB* p_scb, UNUSED_ATTR tBTA_AG_DATA* p_data) {
/* if scb is in use */
  /* sco_idx is not allocated in SCO_CODEC_ST, still need to move to listen
   * state. */
  tBTA_AG_SCO_CB *sco_hdl = NULL;

#if (TWS_AG_ENABLED == TRUE)
  if (is_twsp_device(p_scb->peer_addr)) {
    if (p_scb == bta_ag_cb.main_sm_scb) {
        sco_hdl = &bta_ag_cb.sco;
    }
    else if (p_scb == bta_ag_cb.sec_sm_scb) {
        sco_hdl = &bta_ag_cb.twsp_sec_sco;
    } else {
        APPL_TRACE_ERROR("%s: Invalid scb handle : %x", __func__, p_scb);
        return;
    }
  } else {
#endif
      sco_hdl = &bta_ag_cb.sco;
#if (TWS_AG_ENABLED == TRUE)
  }
#endif

  if ((p_scb->sco_idx != BTM_INVALID_SCO_INDEX) ||
      (sco_hdl->state == BTA_AG_SCO_CODEC_ST))
  {
#if (TWS_AG_ENABLED == TRUE)
    if (is_twsp_device(p_scb->peer_addr)) {
       APPL_TRACE_IMP("%s: p_scb:%x, bta_ag_cb.main_sm_scb:%x,bta_ag_cb.sec_sm_scb:%x",
                     __func__,p_scb, bta_ag_cb.main_sm_scb);
       APPL_TRACE_IMP("%s: bta_ag_cb.sec_sm_scb:%x", bta_ag_cb.sec_sm_scb);
       if (p_scb == bta_ag_cb.main_sm_scb) {
          bta_ag_sco_event(p_scb, BTA_AG_SCO_CLOSE_E);
       }
       else if (p_scb == bta_ag_cb.sec_sm_scb) {
          bta_ag_twsp_sco_event(p_scb, BTA_AG_SCO_CLOSE_E);
       } else {
          APPL_TRACE_ERROR("%s: Invalid scb handle", __func__);
       }
    } else {
#endif
       APPL_TRACE_IMP("bta_ag_sco_close: sco_inx = %d", p_scb->sco_idx);
       bta_ag_sco_event(p_scb, BTA_AG_SCO_CLOSE_E);
#if (TWS_AG_ENABLED == TRUE)
    }
#endif
  }
}

/*******************************************************************************
 *
 * Function         bta_ag_sco_codec_nego
 *
 * Description      Handles result of eSCO codec negotiation
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_ag_sco_codec_nego(tBTA_AG_SCB* p_scb, bool result) {
  if (result == true) {
    /* Subsequent SCO connection will skip codec negotiation */
    APPL_TRACE_DEBUG("%s: Succeeded for index 0x%04x, device %s", __func__,
                     p_scb->sco_idx, p_scb->peer_addr.ToString().c_str());
    p_scb->codec_updated = false;

#if (TWS_AG_ENABLED == TRUE)
    if (is_twsp_device(p_scb->peer_addr)) {
        APPL_TRACE_DEBUG("%s:scb: %x == p_curr_scb = %x",
           __func__, p_scb ,bta_ag_cb.sco.p_curr_scb);
        if (p_scb == bta_ag_cb.main_sm_scb) {
           bta_ag_sco_event(p_scb, BTA_AG_SCO_CN_DONE_E);
        } else if (p_scb == bta_ag_cb.sec_sm_scb){
           //TWS PEER SCO CN is done.
           APPL_TRACE_DEBUG("%s:TWS+ peer SCO CN success", __func__);
           bta_ag_twsp_sco_event(p_scb, BTA_AG_SCO_CN_DONE_E);
        } else {
           APPL_TRACE_ERROR("%s: CN result ignored", __func__);
        }
    } else {
#endif
        bta_ag_sco_event(p_scb, BTA_AG_SCO_CN_DONE_E);
#if (TWS_AG_ENABLED == TRUE)
    }
#endif
  } else {
    /* codec negotiation failed */
    APPL_TRACE_ERROR("%s: Failed for index 0x%04x", __func__, p_scb->sco_idx);
#if (TWS_AG_ENABLED == TRUE)
    if (is_twsp_device(p_scb->peer_addr)) {
       if (p_scb == bta_ag_cb.main_sm_scb)
          bta_ag_sco_event(p_scb, BTA_AG_SCO_CLOSE_E);
       else if(p_scb == bta_ag_cb.sec_sm_scb){
          APPL_TRACE_DEBUG("%s:TWS+ peer SCO CN failed", __func__);
          bta_ag_twsp_sco_event(p_scb, BTA_AG_SCO_CLOSE_E);
       } else {
          APPL_TRACE_ERROR("%s: CN failure ignored", __func__);
       }
    } else {
#endif
        bta_ag_sco_event(p_scb, BTA_AG_SCO_CLOSE_E);
#if (TWS_AG_ENABLED == TRUE)
    }
#endif
  }
}

/*******************************************************************************
 *
 * Function         bta_ag_sco_shutdown
 *
 * Description
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_ag_sco_shutdown(tBTA_AG_SCB* p_scb, UNUSED_ATTR tBTA_AG_DATA* p_data) {
  APPL_TRACE_DEBUG("%s: p_scb : %x", __func__, p_scb);

#if (TWS_AG_ENABLED == TRUE)
  if (is_twsp_device(p_scb->peer_addr)) {
     if (p_scb == bta_ag_cb.main_sm_scb) {
        bta_ag_sco_event(p_scb, BTA_AG_SCO_SHUTDOWN_E);
     } else if (p_scb == bta_ag_cb.sec_sm_scb){
        APPL_TRACE_DEBUG("%s:TWS+ peer SCO shutdown", __func__);
        bta_ag_twsp_sco_event(p_scb, BTA_AG_SCO_SHUTDOWN_E);
     } else {
        APPL_TRACE_ERROR("%s: ignored", __func__);
     }
  } else {
#endif
     bta_ag_sco_event(p_scb, BTA_AG_SCO_SHUTDOWN_E);
#if (TWS_AG_ENABLED == TRUE)
  }
#endif
}

/*******************************************************************************
 *
 * Function         bta_ag_sco_conn_open
 *
 * Description
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_ag_sco_conn_open(tBTA_AG_SCB* p_scb,
                          UNUSED_ATTR tBTA_AG_DATA* p_data) {


#if (TWS_AG_ENABLED == TRUE)
  if (is_twsp_device(p_scb->peer_addr)) {
     APPL_TRACE_DEBUG("%s: p_scb : %x sco.p_curr_scb: %x", __func__,
                       p_scb, bta_ag_cb.sco.p_curr_scb );
     if (p_scb == bta_ag_cb.main_sm_scb) {
        bta_ag_sco_event(p_scb, BTA_AG_SCO_CONN_OPEN_E);
     } else if(p_scb == bta_ag_cb.sec_sm_scb){
        APPL_TRACE_DEBUG("%s:TWS+ peer SCO CN Opened", __func__);
        bta_ag_twsp_sco_event(p_scb, BTA_AG_SCO_CONN_OPEN_E);
     } else {
        APPL_TRACE_ERROR("%s: ignored", __func__);
     }

  } else {
#endif
      bta_ag_sco_event(p_scb, BTA_AG_SCO_CONN_OPEN_E);
#if (TWS_AG_ENABLED == TRUE)
  }
#endif
    bta_sys_sco_open(BTA_ID_AG, p_scb->app_id, p_scb->peer_addr);

#if (BTM_SCO_HCI_INCLUDED == TRUE)
      /* open SCO codec if SCO is routed through transport */
    bta_dm_sco_co_open(bta_ag_scb_to_idx(p_scb), BTA_SCO_OUT_PKT_SIZE,
                     BTA_AG_CI_SCO_DATA_EVT);
#endif

    /* call app callback */
    bta_ag_cback_sco(p_scb, BTA_AG_AUDIO_OPEN_EVT);

    /* reset collision trials and timer */
    p_scb->no_of_xsco_trials = 0;
    alarm_cancel(p_scb->xsco_conn_collision_timer);

    /* reset to mSBC T2 settings as the preferred */
    p_scb->codec_msbc_settings = BTA_AG_SCO_MSBC_SETTINGS_T2;

#if (SWB_ENABLED == TRUE)
    /* reset to SWB Q0 settings as the preferred */
    p_scb->codec_swb_settings = BTA_AG_SCO_SWB_SETTINGS_Q0;
#endif
 }

/*******************************************************************************
 *
 * Function         bta_ag_sco_conn_close
 *
 * Description
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_ag_sco_conn_close(tBTA_AG_SCB* p_scb,
                           UNUSED_ATTR tBTA_AG_DATA* p_data) {

#if (TWS_AG_ENABLED == TRUE)
   if (is_twsp_connected()) {
       //Check if this is robust enough
       if (p_scb->peer_addr.IsEmpty()) {
         //if BD_ADDR is null just ignore
         APPL_TRACE_ERROR("%s: IGN: sco_close is ignored", __func__);
         return;
       }
   }
#endif

    /* Indicate if the closing of audio is because of transfer */
#if (TWS_AG_ENABLED == TRUE)
    if (is_twsp_device(p_scb->peer_addr)) {
       if (p_scb == bta_ag_cb.main_sm_scb) {
          bta_ag_cb.sco.p_curr_scb = NULL;
          p_scb->sco_idx = BTM_INVALID_SCO_INDEX;
          bta_ag_sco_event(p_scb, BTA_AG_SCO_CONN_CLOSE_E);
       } else if (p_scb == bta_ag_cb.sec_sm_scb){
          bta_ag_cb.twsp_sec_sco.p_curr_scb = NULL;
          p_scb->sco_idx = BTM_INVALID_SCO_INDEX;
          bta_ag_twsp_sco_event(p_scb, BTA_AG_SCO_CONN_CLOSE_E);
       }

    } else {
#endif
       /* clear current scb */
       bta_ag_cb.sco.p_curr_scb = NULL;
       p_scb->sco_idx = BTM_INVALID_SCO_INDEX;
       /* codec_fallback is set when AG is initiator and connection failed for mSBC.
        * OR if codec is msbc and T2 settings failed, then retry Safe T1 settings */
       if (p_scb->svc_conn &&
           (p_scb->codec_fallback ||
           (p_scb->sco_codec == BTM_SCO_CODEC_MSBC &&
            p_scb->codec_msbc_settings == BTA_AG_SCO_MSBC_SETTINGS_T1)
#if (SWB_ENABLED == TRUE)
            || (p_scb->codec_fallback && p_scb->sco_codec == BTA_AG_SCO_SWB_SETTINGS_Q0)
#endif
        )) {
              bta_ag_sco_event(p_scb, BTA_AG_SCO_REOPEN_E);
       } else {
           bta_ag_sco_event(p_scb, BTA_AG_SCO_CONN_CLOSE_E);
       }
#if (TWS_AG_ENABLED == TRUE)
    }
#endif

     bta_sys_sco_close(BTA_ID_AG, p_scb->app_id, p_scb->peer_addr);

     /* if av got suspended by this call, let it resume. */
     /* In case call stays alive regardless of sco, av should not be affected. */
     if (((p_scb->call_ind == BTA_AG_CALL_INACTIVE) &&
            (p_scb->callsetup_ind == BTA_AG_CALLSETUP_NONE)) ||
            (p_scb->post_sco == BTA_AG_POST_SCO_CALL_END)) {
           bta_sys_sco_unuse(BTA_ID_AG, p_scb->app_id, p_scb->peer_addr);
     }

     /* call app callback */
     bta_ag_cback_sco(p_scb, BTA_AG_AUDIO_CLOSE_EVT);
     p_scb->codec_msbc_settings = BTA_AG_SCO_MSBC_SETTINGS_T2;
#if (SWB_ENABLED == TRUE)
     p_scb->codec_swb_settings = BTA_AG_SCO_SWB_SETTINGS_Q0;
#endif
}

/*******************************************************************************
 *
 * Function         bta_ag_sco_conn_rsp
 *
 * Description      Process the SCO connection request
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_ag_sco_conn_rsp(tBTA_AG_SCB* p_scb,
                         tBTM_ESCO_CONN_REQ_EVT_DATA* p_data) {
  bta_ag_cb.sco.is_local = false;

  APPL_TRACE_DEBUG("%s: eSCO %d, state %d", __func__,
                   controller_get_interface()
                       ->supports_enhanced_setup_synchronous_connection(),
                   bta_ag_cb.sco.state);

  if (bta_ag_cb.sco.state == BTA_AG_SCO_LISTEN_ST ||
      bta_ag_cb.sco.state == BTA_AG_SCO_CLOSE_XFER_ST ||
      bta_ag_cb.sco.state == BTA_AG_SCO_OPEN_XFER_ST) {
    /* tell sys to stop av if any */
    bta_sys_sco_use(BTA_ID_AG, p_scb->app_id, p_scb->peer_addr);
    /* When HS initiated SCO, it cannot be WBS. */
#if (BTM_SCO_HCI_INCLUDED == TRUE)
    /* Configure the transport being used */
    BTM_ConfigScoPath(resp.input_data_path, bta_ag_sco_read_cback, NULL, TRUE);
#endif
  }

  /* If SCO open was initiated from HS, it must be CVSD */
  p_scb->inuse_codec = BTA_AG_CODEC_NONE;
  /* Send pending commands to create SCO connection to peer */
  bta_ag_create_pending_sco(p_scb, bta_ag_cb.sco.is_local);
}

/*******************************************************************************
 *
 * Function         bta_ag_ci_sco_data
 *
 * Description      Process the SCO data ready callin event
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_ag_ci_sco_data(UNUSED_ATTR tBTA_AG_SCB* p_scb,
                        UNUSED_ATTR tBTA_AG_DATA* p_data) {
#if (BTM_SCO_HCI_INCLUDED == TRUE)
  bta_ag_sco_event(p_scb, BTA_AG_SCO_CI_DATA_E);
#endif
}

void bta_ag_set_sco_allowed(tBTA_AG_DATA* p_data) {
  sco_allowed = ((tBTA_AG_API_SET_SCO_ALLOWED*)p_data)->value;
  APPL_TRACE_DEBUG(sco_allowed ? "sco now allowed" : "sco now not allowed");
}

const RawAddress& bta_ag_get_active_device() { return active_device_addr; }

void bta_clear_active_device() { active_device_addr = RawAddress::kEmpty; }

void bta_ag_api_set_active_device(tBTA_AG_DATA* p_data) {
  if (p_data->api_set_active_device.active_device_addr.IsEmpty()) {
    APPL_TRACE_ERROR("%s: empty device", __func__);
    return;
  }
  //When HFP active device is changed, exit sniff for the new active device
  active_device_addr = p_data->api_set_active_device.active_device_addr;
  tBTA_AG_SCB* p_scb = bta_ag_scb_by_idx(bta_ag_idx_by_bdaddr(&(active_device_addr)));
  if (p_scb == NULL) {
      APPL_TRACE_WARNING("%s: p_scb is NULL", __func__);
  } else {
      bta_sys_busy(BTA_ID_AG, p_scb->app_id, active_device_addr);
      bta_sys_idle(BTA_ID_AG, p_scb->app_id, active_device_addr);
  }
}

/*******************************************************************************
 *  Debugging functions
 ******************************************************************************/
#define CASE_RETURN_STR(const) \
  case const:                  \
    return #const;

const char* bta_ag_sco_evt_str(uint8_t event) {
  switch (event) {
    CASE_RETURN_STR(BTA_AG_SCO_LISTEN_E)
    CASE_RETURN_STR(BTA_AG_SCO_OPEN_E)
    CASE_RETURN_STR(BTA_AG_SCO_XFER_E)
    CASE_RETURN_STR(BTA_AG_SCO_CN_DONE_E)
    CASE_RETURN_STR(BTA_AG_SCO_REOPEN_E)
    CASE_RETURN_STR(BTA_AG_SCO_CLOSE_E)
    CASE_RETURN_STR(BTA_AG_SCO_SHUTDOWN_E)
    CASE_RETURN_STR(BTA_AG_SCO_CONN_OPEN_E)
    CASE_RETURN_STR(BTA_AG_SCO_CONN_CLOSE_E)
    default:
      return (char*)"Unknown SCO Event";
  }
}

const char* bta_ag_sco_state_str(uint8_t state) {
  switch (state) {
    CASE_RETURN_STR(BTA_AG_SCO_SHUTDOWN_ST)
    CASE_RETURN_STR(BTA_AG_SCO_LISTEN_ST)
    CASE_RETURN_STR(BTA_AG_SCO_CODEC_ST)
    CASE_RETURN_STR(BTA_AG_SCO_OPENING_ST)
    CASE_RETURN_STR(BTA_AG_SCO_OPEN_CL_ST)
    CASE_RETURN_STR(BTA_AG_SCO_OPEN_XFER_ST)
    CASE_RETURN_STR(BTA_AG_SCO_OPEN_ST)
    CASE_RETURN_STR(BTA_AG_SCO_CLOSING_ST)
    CASE_RETURN_STR(BTA_AG_SCO_CLOSE_OP_ST)
    CASE_RETURN_STR(BTA_AG_SCO_CLOSE_XFER_ST)
    CASE_RETURN_STR(BTA_AG_SCO_SHUTTING_ST)
    default:
      return (char*)"Unknown SCO State";
  }
}
