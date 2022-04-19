/******************************************************************************
 *
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
 *  Copyright (C) 2003-2012 Broadcom Corporation
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
 *  This file contains action functions for the audio gateway.
 *
 ******************************************************************************/

#include <string.h>

#include "bta_ag_api.h"
#include "bta_ag_co.h"
#include "bta_ag_int.h"
#include "bta_api.h"
#include "bta_dm_api.h"
#include "bta_sys.h"
#include "l2c_api.h"
#include "osi/include/osi.h"
#include "port_api.h"
#include "utl.h"
#include "btif/include/btif_config.h"
#include "device/include/interop_config.h"
#include <cutils/properties.h>
#if (TWS_AG_ENABLED == TRUE)
#include "bta_ag_twsp_dev.h"
#include "bta_ag_twsp.h"
#endif
#include "device/include/device_iot_config.h"
#if (SWB_ENABLED == TRUE)
#include "bta_ag_swb.h"
#endif

/*****************************************************************************
 *  Constants
 ****************************************************************************/

/* maximum length of data to read from RFCOMM */
#define BTA_AG_RFC_READ_MAX 512

/* maximum AT command length */
#define BTA_AG_CMD_MAX 512

const uint16_t bta_ag_uuid[BTA_AG_NUM_IDX] = {
    UUID_SERVCLASS_HEADSET_AUDIO_GATEWAY, UUID_SERVCLASS_AG_HANDSFREE};

const uint8_t bta_ag_sec_id[BTA_AG_NUM_IDX] = {BTM_SEC_SERVICE_HEADSET_AG,
                                               BTM_SEC_SERVICE_AG_HANDSFREE};

const tBTA_SERVICE_ID bta_ag_svc_id[BTA_AG_NUM_IDX] = {BTA_HSP_SERVICE_ID,
                                                       BTA_HFP_SERVICE_ID};

const tBTA_SERVICE_MASK bta_ag_svc_mask[BTA_AG_NUM_IDX] = {
    BTA_HSP_SERVICE_MASK, BTA_HFP_SERVICE_MASK};

typedef void (*tBTA_AG_ATCMD_CBACK)(tBTA_AG_SCB* p_scb, uint16_t cmd,
                                    uint8_t arg_type, char* p_arg, char* p_end,
                                    int16_t int_arg);

const tBTA_AG_ATCMD_CBACK bta_ag_at_cback_tbl[BTA_AG_NUM_IDX] = {
    bta_ag_at_hsp_cback, bta_ag_at_hfp_cback};

/*******************************************************************************
 *
 * Function         bta_ag_cback_open
 *
 * Description      Send open callback event to application.
 *
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_ag_cback_open(tBTA_AG_SCB* p_scb, tBTA_AG_DATA* p_data,
                              tBTA_AG_STATUS status) {
  tBTA_AG_OPEN open;

  /* call app callback with open event */
  open.hdr.handle = bta_ag_scb_to_idx(p_scb);
  open.hdr.app_id = p_scb->app_id;
  open.status = status;
  open.service_id = bta_ag_svc_id[p_scb->conn_service];
  VLOG(1) << __func__ << "p_scb addr:" << p_scb->peer_addr;
  if (p_data) {
    /* if p_data is provided then we need to pick the bd address from the open
     * api structure */
    open.bd_addr = p_data->api_open.bd_addr;
  } else {
    open.bd_addr = p_scb->peer_addr;
  }
    VLOG(1) << __func__ << "open.bd_addr:" << open.bd_addr;
  (*bta_ag_cb.p_cback)(BTA_AG_OPEN_EVT, (tBTA_AG*)&open);
}

/*******************************************************************************
 *
 * Function         bta_ag_register
 *
 * Description      This function initializes values of the AG cb and sets up
 *                  the SDP record for the services.
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_ag_register(tBTA_AG_SCB* p_scb, tBTA_AG_DATA* p_data) {
  tBTA_AG_REGISTER reg;

  /* initialize control block */
  p_scb->reg_services = p_data->api_register.services;
  p_scb->serv_sec_mask = p_data->api_register.sec_mask;
  p_scb->features = p_data->api_register.features;
  p_scb->app_id = p_data->api_register.app_id;

  /* create SDP records */
  bta_ag_create_records(p_scb, p_data);

  /* start RFCOMM servers */
  bta_ag_start_servers(p_scb, p_scb->reg_services);

  /* call app callback with register event */
  reg.hdr.handle = bta_ag_scb_to_idx(p_scb);
  reg.hdr.app_id = p_scb->app_id;
  reg.status = BTA_AG_SUCCESS;
  (*bta_ag_cb.p_cback)(BTA_AG_REGISTER_EVT, (tBTA_AG*)&reg);
}

/*******************************************************************************
 *
 * Function         bta_ag_deregister
 *
 * Description      This function removes the sdp records, closes the RFCOMM
 *                  servers, and deallocates the service control block.
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_ag_deregister(tBTA_AG_SCB* p_scb, tBTA_AG_DATA* p_data) {
  /* set dealloc */
  p_scb->dealloc = true;

  /* remove sdp records */
  bta_ag_del_records(p_scb, p_data);

  /* remove rfcomm servers */
  bta_ag_close_servers(p_scb, p_scb->reg_services);

  /* dealloc */
  bta_ag_scb_dealloc(p_scb);
}

/*******************************************************************************
 *
 * Function         bta_ag_start_dereg
 *
 * Description      Start a deregister event.
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_ag_start_dereg(tBTA_AG_SCB* p_scb, tBTA_AG_DATA* p_data) {
  /* set dealloc */
  p_scb->dealloc = true;

  /* remove sdp records */
  bta_ag_del_records(p_scb, p_data);
}

/*******************************************************************************
 *
 * Function         bta_ag_start_open
 *
 * Description      This starts an AG open.
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_ag_start_open(tBTA_AG_SCB* p_scb, tBTA_AG_DATA* p_data) {
  RawAddress pending_bd_addr;
  int rfcomm_conn_status = 0;
  tBTA_SERVICE_MASK services;

  /* store parameters */
  if (p_data) {
    p_scb->peer_addr = p_data->api_open.bd_addr;
    p_scb->open_services = p_data->api_open.services;
    p_scb->cli_sec_mask = p_data->api_open.sec_mask;
  }

  services = p_scb->reg_services >> BTA_HSP_SERVICE_ID;
  for (int i = 0; i < BTA_AG_NUM_IDX && services != 0; i++, services >>= 1) {
    /* if service is set in mask */
    if (services & 1) {
      rfcomm_conn_status = PORT_GetStateBySCN(p_scb->peer_addr,
                                           bta_ag_cb.profile[i].scn, true);
      APPL_TRACE_WARNING("%s: rfcomm connection status %d for device %s, scn %x",
             __func__, rfcomm_conn_status, p_scb->peer_addr.ToString().c_str(),
             bta_ag_cb.profile[i].scn);

      if (rfcomm_conn_status == PORT_STATE_OPENED)
        break;
    }
  }

  /* Check if RFCOMM has any incoming connection to avoid collision. */
  if (PORT_IsOpening(pending_bd_addr) || rfcomm_conn_status == PORT_STATE_OPENED) {
    if (bta_ag_cb.max_hf_clients > 1)
    {
      // Abort the outgoing connection if incoming connection is from the same device
      if (pending_bd_addr == p_scb->peer_addr)
      {
        APPL_TRACE_WARNING("%s: p_scb %x, abort outgoing conn, there is"\
             " an incoming conn from dev %s, moving to BTA_AG_INIT_ST state",
              __func__, p_scb, p_scb->peer_addr.ToString().c_str());
        // send ourselves close event for clean up
        bta_ag_cback_open(p_scb, NULL, BTA_AG_FAIL_RFCOMM);
        p_scb->state = 0;
        return;
      }
    }
    /* Let the incoming connection goes through.                        */
    /* Issue collision for this scb for now.                            */
    /* We will decide what to do when we find incoming connetion later. */
    bta_ag_collision_cback(0, BTA_ID_AG, 0, p_scb->peer_addr);
    return;
  }

  /* close servers */
  bta_ag_close_servers(p_scb, p_scb->reg_services);

  /* set role */
  p_scb->role = BTA_AG_INT;

  /* do service search */
  bta_ag_do_disc(p_scb, p_scb->open_services);
}

/*******************************************************************************
 *
 * Function         bta_ag_disc_int_res
 *
 * Description      This function handles a discovery result when initiator.
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_ag_disc_int_res(tBTA_AG_SCB* p_scb, tBTA_AG_DATA* p_data) {
  uint16_t event = BTA_AG_DISC_FAIL_EVT;

  APPL_TRACE_DEBUG("bta_ag_disc_int_res: Status: %d",
                   p_data->disc_result.status);

  /* if found service */
  if (p_data->disc_result.status == SDP_SUCCESS ||
      p_data->disc_result.status == SDP_DB_FULL) {
    /* get attributes */
    if (bta_ag_sdp_find_attr(p_scb, p_scb->open_services)) {
      /* set connected service */
      p_scb->conn_service = bta_ag_service_to_idx(p_scb->open_services);

      /* send ourselves sdp ok event */
      event = BTA_AG_DISC_OK_EVT;

#if (BT_IOT_LOGGING_ENABLED == TRUE)
      device_iot_config_addr_set_hex_if_greater(p_scb->peer_addr,
              IOT_CONF_KEY_HFP_VERSION, p_scb->peer_version, IOT_CONF_BYTE_NUM_2);
#endif
    }
  }

  /* free discovery db */
  bta_ag_free_db(p_scb, p_data);
  APPL_TRACE_DEBUG(" %s: event %d ", __func__,event);

  /* if service not found check if we should search for other service */
  if ((event == BTA_AG_DISC_FAIL_EVT) &&
      (p_data->disc_result.status == SDP_SUCCESS ||
       p_data->disc_result.status == SDP_DB_FULL ||
       p_data->disc_result.status == SDP_NO_RECS_MATCH)) {
    if ((p_scb->open_services & BTA_HFP_SERVICE_MASK) &&
        (p_scb->open_services & BTA_HSP_SERVICE_MASK)) {
      /* search for HSP */
      p_scb->open_services &= ~BTA_HFP_SERVICE_MASK;
      bta_ag_do_disc(p_scb, p_scb->open_services);
    } else if ((p_scb->open_services & BTA_HSP_SERVICE_MASK) &&
               (p_scb->hsp_version == HSP_VERSION_1_2)) {
      /* search for UUID_SERVCLASS_HEADSET instead */
      p_scb->hsp_version = HSP_VERSION_1_0;
      bta_ag_do_disc(p_scb, p_scb->open_services);
    } else {
      /* send ourselves sdp ok/fail event */
      bta_ag_sm_execute(p_scb, event, p_data);
    }
  } else {
    /* send ourselves sdp ok/fail event */
    bta_ag_sm_execute(p_scb, event, p_data);
  }
}

/*******************************************************************************
 *
 * Function         bta_ag_disc_acp_res
 *
 * Description      This function handles a discovery result when acceptor.
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_ag_disc_acp_res(tBTA_AG_SCB* p_scb, tBTA_AG_DATA* p_data) {
  /* if found service */
  if (p_data->disc_result.status == SDP_SUCCESS ||
      p_data->disc_result.status == SDP_DB_FULL) {
    /* get attributes */
    bta_ag_sdp_find_attr(p_scb, bta_ag_svc_mask[p_scb->conn_service]);
#if (BT_IOT_LOGGING_ENABLED == TRUE)
    device_iot_config_addr_set_hex_if_greater(p_scb->peer_addr,
            IOT_CONF_KEY_HFP_VERSION, p_scb->peer_version, IOT_CONF_BYTE_NUM_2);
#endif
  }

  /* free discovery db */
  bta_ag_free_db(p_scb, p_data);
}

/*******************************************************************************
 *
 * Function         bta_ag_disc_fail
 *
 * Description      This function handles a discovery failure.
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_ag_disc_fail(tBTA_AG_SCB* p_scb, UNUSED_ATTR tBTA_AG_DATA* p_data) {
  /* reopen registered servers */
  bta_ag_start_servers(p_scb, p_scb->reg_services);

  /* reinitialize stuff */

  /* call open cback w. failure */
  bta_ag_cback_open(p_scb, NULL, BTA_AG_FAIL_SDP);

  /* clear the remote BD address */
  p_scb->peer_addr = RawAddress::kEmpty;

}

/*******************************************************************************
 *
 * Function         bta_ag_open_fail
 *
 * Description      open connection failed.
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_ag_open_fail(tBTA_AG_SCB* p_scb, tBTA_AG_DATA* p_data) {
  /* call open cback w. failure */
  bta_ag_cback_open(p_scb, p_data, BTA_AG_FAIL_RESOURCES);
}

/*******************************************************************************
 *
 * Function         bta_ag_rfc_fail
 *
 * Description      RFCOMM connection failed.
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_ag_rfc_fail(tBTA_AG_SCB* p_scb, UNUSED_ATTR tBTA_AG_DATA* p_data) {
  RawAddress peer_addr = p_scb->peer_addr;
  tBTA_AG_DATA data;

  memset(&data, 0, sizeof(data));
  /* reinitialize stuff */
  p_scb->conn_handle = 0;
  p_scb->conn_service = 0;
  p_scb->peer_features = 0;
  p_scb->peer_codecs = BTA_AG_CODEC_CVSD;
  p_scb->sco_codec = BTA_AG_CODEC_CVSD;
#if (SWB_ENABLED == TRUE)
  p_scb->is_swb_codec = false;
#endif
  p_scb->role = 0;
  p_scb->svc_conn = false;
  p_scb->hsp_version = HSP_VERSION_1_2;
  /*Clear the BD address*/
  p_scb->peer_addr = RawAddress::kEmpty;

  /* reopen registered servers */
  bta_ag_start_servers(p_scb, p_scb->reg_services);

  data.api_open.bd_addr = peer_addr;
  /* call open cback w. failure */
  bta_ag_cback_open(p_scb, &data, BTA_AG_FAIL_RFCOMM);
}

/*******************************************************************************
 *
 * Function         bta_ag_rfc_close
 *
 * Description      RFCOMM connection closed.
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_ag_rfc_close(tBTA_AG_SCB* p_scb, UNUSED_ATTR tBTA_AG_DATA* p_data) {
  tBTA_AG_CLOSE close;
  tBTA_SERVICE_MASK services;
  int i, num_active_conn = 0;

  /* reinitialize stuff */
  p_scb->conn_service = 0;
  p_scb->peer_features = 0;
  p_scb->peer_codecs = BTA_AG_CODEC_CVSD;
  p_scb->sco_codec = BTA_AG_CODEC_CVSD;
  /* Clear these flags upon SLC teardown */
  p_scb->codec_updated = false;
  p_scb->codec_fallback = false;
  p_scb->codec_msbc_settings = BTA_AG_SCO_MSBC_SETTINGS_T2;
#if (SWB_ENABLED == TRUE)
  p_scb->codec_swb_settings = BTA_AG_SCO_SWB_SETTINGS_Q0;
  p_scb->is_swb_codec = false;
#endif
  p_scb->role = 0;
  p_scb->post_sco = BTA_AG_POST_SCO_NONE;
  p_scb->svc_conn = false;
  p_scb->hsp_version = HSP_VERSION_1_2;
  bta_ag_at_reinit(&p_scb->at_cb);

  memset(&(p_scb->peer_hf_indicators), 0, sizeof(p_scb->peer_hf_indicators));
  memset(&(p_scb->local_hf_indicators), 0, sizeof(p_scb->local_hf_indicators));

  /* stop timers */
  alarm_cancel(p_scb->ring_timer);
  alarm_cancel(p_scb->codec_negotiation_timer);
  alarm_cancel(p_scb->xsco_conn_collision_timer);

  close.hdr.handle = bta_ag_scb_to_idx(p_scb);
  close.hdr.app_id = p_scb->app_id;
  close.bd_addr = p_scb->peer_addr;

  bta_sys_conn_close(BTA_ID_AG, p_scb->app_id, p_scb->peer_addr);

  /* call close call-out */
  bta_ag_co_data_close(close.hdr.handle);

  /* call close cback */
  (*bta_ag_cb.p_cback)(BTA_AG_CLOSE_EVT, (tBTA_AG*)&close);
#if (TWS_AG_ENABLED == TRUE)
  if (is_twsp_device(p_scb->peer_addr)) {
      reset_twsp_device(twsp_get_idx_by_scb(p_scb));
  }
#endif

  /* if not deregistering (deallocating) reopen registered servers */
  if (p_scb->dealloc == false) {
    /* Cancel SDP if it had been started. */
    if (p_scb->p_disc_db) {
      APPL_TRACE_DEBUG(" %s CancelSDP ",__func__);
      (void)SDP_CancelServiceSearch(p_scb->p_disc_db);
      bta_ag_free_db(p_scb, NULL);
    }
    APPL_TRACE_DEBUG("%s: sending sco_shutdown", __func__);
    /* Make sure SCO state is BTA_AG_SCO_SHUTDOWN_ST */
    bta_ag_sco_shutdown(p_scb, NULL);

    /* Clear peer bd_addr so instance can be reused */
    p_scb->peer_addr = RawAddress::kEmpty;

    if (bta_ag_get_active_device() == p_scb->peer_addr) {
       bta_clear_active_device();
    }

    /* start only unopened server */
    services = p_scb->reg_services;
    for (i = 0; i < BTA_AG_NUM_IDX && services != 0; i++) {
      if (p_scb->serv_handle[i])
        services &= ~((tBTA_SERVICE_MASK)1 << (BTA_HSP_SERVICE_ID + i));
    }
    bta_ag_start_servers(p_scb, services);

    p_scb->conn_handle = 0;


    /* Check if all the SLCs are down */
    for (i = 0; i < BTA_AG_MAX_NUM_CLIENTS; i++) {
      if (bta_ag_cb.scb[i].in_use && bta_ag_cb.scb[i].svc_conn)
        num_active_conn++;
    }

    if (!num_active_conn) {
      bta_sys_sco_unuse(BTA_ID_AG, p_scb->app_id, p_scb->peer_addr);
    }

  }
  /* else close port and deallocate scb */
  else {
    RFCOMM_RemoveServer(p_scb->conn_handle);
    bta_ag_scb_dealloc(p_scb);
  }
}

/*******************************************************************************
 *
 * Function         bta_ag_rfc_open
 *
 * Description      Handle RFCOMM channel open.
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_ag_rfc_open(tBTA_AG_SCB* p_scb, tBTA_AG_DATA* p_data) {
  /* initialize AT feature variables */
  p_scb->clip_enabled = false;
  p_scb->ccwa_enabled = false;
  p_scb->cmer_enabled = false;
  p_scb->cmee_enabled = false;
  p_scb->inband_enabled =
      ((p_scb->features & BTA_AG_FEAT_INBAND) == BTA_AG_FEAT_INBAND);
  APPL_TRACE_DEBUG("%s: p_scb->inband_enabled: %d p_scb->conn_service: %d", __func__,
                        p_scb->inband_enabled, p_scb->conn_service);
  VLOG(1) << __func__ << " p_scb addr:" << p_scb->peer_addr;
  /* set up AT command interpreter */
  p_scb->at_cb.p_at_tbl = (tBTA_AG_AT_CMD*)bta_ag_at_tbl[p_scb->conn_service];
  p_scb->at_cb.p_cmd_cback =
      (tBTA_AG_AT_CMD_CBACK*)bta_ag_at_cback_tbl[p_scb->conn_service];
  p_scb->at_cb.p_err_cback = (tBTA_AG_AT_ERR_CBACK*)bta_ag_at_err_cback;
  p_scb->at_cb.p_user = p_scb;
  p_scb->at_cb.cmd_max_len = BTA_AG_CMD_MAX;
  bta_ag_at_init(&p_scb->at_cb);

  /* call app open call-out */
  bta_ag_co_data_open(bta_ag_scb_to_idx(p_scb),
                      bta_ag_svc_id[p_scb->conn_service]);

  bta_sys_conn_open(BTA_ID_AG, p_scb->app_id, p_scb->peer_addr);

  bta_ag_cback_open(p_scb, NULL, BTA_AG_SUCCESS);

  if (p_scb->conn_service == BTA_AG_HFP) {
    /* if hfp start timer for service level conn */
    bta_sys_start_timer(p_scb->ring_timer, p_bta_ag_cfg->conn_tout,
                        BTA_AG_SVC_TIMEOUT_EVT, bta_ag_scb_to_idx(p_scb));
#if (TWS_AG_ENABLED == TRUE)
    //Update TWS+ data structure
    if (is_twsp_device(p_scb->peer_addr)) {
        update_twsp_device(p_scb);
    }
#endif
  } else {
    /* else service level conn is open */
    bta_ag_svc_conn_open(p_scb, p_data);
  }
}

/*******************************************************************************
 *
 * Function         bta_ag_rfc_acp_open
 *
 * Description      Handle RFCOMM channel open when accepting connection.
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_ag_rfc_acp_open(tBTA_AG_SCB* p_scb, tBTA_AG_DATA* p_data) {
  uint16_t lcid;
  int i;
  tBTA_AG_SCB *ag_scb;
  RawAddress dev_addr;
  int status;
  uint16_t hfp_version = 0;

  /* set role */
  p_scb->role = BTA_AG_ACP;

  APPL_TRACE_IMP ("bta_ag_rfc_acp_open: serv_handle0 = %d serv_handle1 = %d",
                   p_scb->serv_handle[0], p_scb->serv_handle[1]);

  /* get bd addr of peer */
  if (PORT_SUCCESS != (status = PORT_CheckConnection(p_data->rfc.port_handle,
                                                     dev_addr, &lcid))) {
    APPL_TRACE_ERROR(
        "bta_ag_rfc_acp_open error PORT_CheckConnection returned status %d",
        status);
  }

  /* Collision Handling */
  for (i = 0, ag_scb = &bta_ag_cb.scb[0]; i < BTA_AG_MAX_NUM_CLIENTS;
       i++, ag_scb++) {
    if (ag_scb->in_use) {

      VLOG(1) << __func__ << "ag_scb addr:" << ag_scb->peer_addr;
      if (dev_addr == ag_scb->peer_addr) {
        if (bta_ag_cb.max_hf_clients > 1 && ag_scb != p_scb)
        {
          /* If incoming and outgoing device are same, nothing more to do.*/
          /* Outgoing conn will be aborted because we have successful incoming conn.*/
          APPL_TRACE_WARNING("%s: ag_scb %x, abort outgoing conn,"\
            "there is an incoming conn from dev %s, i %x, ag_scb->state %x",
           __func__, ag_scb, dev_addr.ToString().c_str(), i, ag_scb->state);

          // if outgoing conn is waiting for SDP or RFCOMM conn to open
          if (ag_scb->state == BTA_AG_OPENING_ST) {
            bta_ag_handle_collision(ag_scb, NULL);
            ag_scb->state = BTA_AG_INIT_ST;
            ag_scb->peer_addr = RawAddress::kEmpty;
          }
          /* Outgoing RFCOMM is just connected, SLC didn't finish.
             If there is an incoming RFCOMM conn from the same device,
             treat it as collision and disconnect outgoing HF connection */
          else if(ag_scb->state == BTA_AG_OPEN_ST && !ag_scb->svc_conn) {
            // RFCOMM closure takes care of moving BTA to INIT, btif cleanup
            bta_ag_start_close(ag_scb, NULL);
          }

          break;
        }
      }
    }
  }

  p_scb->peer_addr = dev_addr;
  VLOG(1) << __func__ << " p_scb addr:" << p_scb->peer_addr;

  /* determine connected service from port handle */
  for (i = 0; i < BTA_AG_NUM_IDX; i++) {
    APPL_TRACE_DEBUG("%s: i = %d serv_handle = %d port_handle = %d", __func__, i,
                      p_scb->serv_handle[i], p_data->rfc.port_handle);
    if (p_scb->serv_handle[i] == p_data->rfc.port_handle) {
      p_scb->conn_service = i;
      p_scb->conn_handle = p_data->rfc.port_handle;
      break;
    }
  }

  APPL_TRACE_IMP("%s: conn_service = %d conn_handle = %d", __func__,
                   p_scb->conn_service, p_scb->conn_handle);

  /* close any unopened server */
  bta_ag_close_servers(
      p_scb, (p_scb->reg_services & ~bta_ag_svc_mask[p_scb->conn_service]));

  bool get_version = btif_config_get_uint16(
                     p_scb->peer_addr.ToString().c_str(), HFP_VERSION_CONFIG_KEY,
                     &hfp_version);
  if (p_scb->conn_service == BTA_AG_HFP && get_version) {
      p_scb->peer_version = hfp_version;
      APPL_TRACE_DEBUG(
       "%s: Avoid SDP for HFP device and fetch the peer_version: %04x "
        "from config file", __func__, p_scb->peer_version);
      /* Remote supports 1.7, store it in the file */
      if (p_scb->peer_version == HFP_VERSION_1_7) {
         APPL_TRACE_DEBUG("%s: version is 1.7, store in a file", __func__);
         interop_database_add_addr(INTEROP_HFP_1_7_BLACKLIST,
                          &p_scb->peer_addr, 3);
      }
#if (BT_IOT_LOGGING_ENABLED == TRUE)
    device_iot_config_addr_set_hex_if_greater(p_scb->peer_addr,
        IOT_CONF_KEY_HFP_VERSION, p_scb->peer_version, IOT_CONF_BYTE_NUM_2);
#endif
  } else {
      //do service discovery to get features for HSP and also for HFP
      //if the peer version can't be fetched from the config file
      APPL_TRACE_DEBUG(
      "%s: Do SDP for HSP/version couldn't be fetched from the config file",
       __func__);
      bta_ag_do_disc(p_scb, bta_ag_svc_mask[p_scb->conn_service]);
  }

  /* continue with common open processing */
  bta_ag_rfc_open(p_scb, p_data);
}

/*******************************************************************************
 *
 * Function         bta_ag_rfc_data
 *
 * Description      Read and process data from RFCOMM.
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_ag_rfc_data(tBTA_AG_SCB* p_scb, UNUSED_ATTR tBTA_AG_DATA* p_data) {
  uint16_t len;
  char buf[BTA_AG_RFC_READ_MAX+1];

  memset(buf, 0, BTA_AG_RFC_READ_MAX+1);

  APPL_TRACE_DEBUG("%s", __func__);

  /* do the following */
  for (;;) {
    /* read data from rfcomm; if bad status, we're done */
    if (PORT_ReadData(p_scb->conn_handle, buf, BTA_AG_RFC_READ_MAX, &len) !=
        PORT_SUCCESS) {
      break;
    }

    /* if no data, we're done */
    if (len == 0) {
      break;
    }

    if (strstr(buf, "AT+IPHONEACCEV") != NULL) {
        APPL_TRACE_IMP("%s: AT+IPHONEACCEV received, not coming out of sniff", __func__);
    } else if (strstr(buf, "AT+CHUP") != NULL || strstr(buf, "ATA") != NULL ||
               strstr(buf, "ATD") != NULL || strstr(buf, "AT+BLDN") != NULL) {
        APPL_TRACE_IMP("%s: AT+CHUP/ATA/ATD/AT+BLDN received, not coming out of sniff", __func__);
    } else {
        APPL_TRACE_IMP("%s: setting sys busy", __func__);
        bta_sys_busy(BTA_ID_AG, p_scb->app_id, p_scb->peer_addr);
    }
    /* run AT command interpreter on data */
    bta_ag_at_parse(&p_scb->at_cb, buf, len);
    if ((p_scb->sco_idx != BTM_INVALID_SCO_INDEX) &&
        bta_ag_sco_is_open(p_scb)) {
      APPL_TRACE_IMP("%s change link policy for SCO", __func__);
      bta_sys_sco_open(BTA_ID_AG, p_scb->app_id, p_scb->peer_addr);
    } else {
      if (strstr(buf, "AT+IPHONEACCEV") != NULL) {
          APPL_TRACE_IMP("%s: AT+IPHONEACCEV received, not setting idle", __func__);
      } else if (strstr(buf, "AT+CHUP") != NULL || strstr(buf, "ATA") != NULL ||
                 strstr(buf, "ATD") != NULL || strstr(buf, "AT+BLDN") != NULL) {
          APPL_TRACE_IMP("%s: AT+CHUP/ATA/ATD/AT+BLDN received, not setting idle", __func__);
      } else {
          APPL_TRACE_IMP("%s: resetting idle timer", __func__);
          bta_sys_idle(BTA_ID_AG, p_scb->app_id, p_scb->peer_addr);
      }
    }

    /* no more data to read, we're done */
    if (len < BTA_AG_RFC_READ_MAX) {
      break;
    }
  }
}

/*******************************************************************************
 *
 * Function         bta_ag_start_close
 *
 * Description      Start the process of closing SCO and RFCOMM connection.
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_ag_start_close(tBTA_AG_SCB* p_scb, tBTA_AG_DATA* p_data) {
  /* Take the link out of sniff and set L2C idle time to 0 */
  bta_dm_pm_active(p_scb->peer_addr);

  if (p_scb->svc_conn) {
    APPL_TRACE_WARNING("%s: SLC is up, probably user initiated HF disconnection" \
                       " setting L2CAP idle timer to 0 from AG", __func__);
    L2CA_SetIdleTimeoutByBdAddr(p_scb->peer_addr, 0, BT_TRANSPORT_BR_EDR);
  }

  /* if SCO is open close SCO and wait on RFCOMM close */
  if (bta_ag_sco_is_open(p_scb)) {
    p_scb->post_sco = BTA_AG_POST_SCO_CLOSE_RFC;
  } else {
    p_scb->post_sco = BTA_AG_POST_SCO_NONE;
    bta_ag_rfc_do_close(p_scb, p_data);
  }

  /* always do SCO shutdown to handle all SCO corner cases */
  bta_ag_sco_shutdown(p_scb, p_data);
}

/*******************************************************************************
 *
 * Function         bta_ag_post_sco_open
 *
 * Description      Perform post-SCO open action, if any
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_ag_post_sco_open(tBTA_AG_SCB* p_scb, tBTA_AG_DATA* p_data) {
  switch (p_scb->post_sco) {
    case BTA_AG_POST_SCO_RING:
      bta_ag_send_ring(p_scb, p_data);
      p_scb->post_sco = BTA_AG_POST_SCO_NONE;
      break;

    case BTA_AG_POST_SCO_CALL_CONN:
      bta_ag_send_call_inds(p_scb, BTA_AG_IN_CALL_CONN_RES);
      p_scb->post_sco = BTA_AG_POST_SCO_NONE;
      break;

    default:
      break;
  }
}

/*******************************************************************************
 *
 * Function         bta_ag_post_sco_close
 *
 * Description      Perform post-SCO close action, if any
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_ag_post_sco_close(tBTA_AG_SCB* p_scb, tBTA_AG_DATA* p_data) {
  switch (p_scb->post_sco) {
    case BTA_AG_POST_SCO_CLOSE_RFC:
      bta_ag_rfc_do_close(p_scb, p_data);
      p_scb->post_sco = BTA_AG_POST_SCO_NONE;
      break;

    case BTA_AG_POST_SCO_CALL_CONN:
      bta_ag_send_call_inds(p_scb, BTA_AG_IN_CALL_CONN_RES);
      p_scb->post_sco = BTA_AG_POST_SCO_NONE;
      break;

    case BTA_AG_POST_SCO_CALL_ORIG:
      bta_ag_send_call_inds(p_scb, BTA_AG_OUT_CALL_ORIG_RES);
      p_scb->post_sco = BTA_AG_POST_SCO_NONE;
      break;

    case BTA_AG_POST_SCO_CALL_END:
      for (size_t i = 0; i < BTA_AG_MAX_NUM_CLIENTS; i++) {
        if (bta_ag_cb.scb[i].in_use &&
            bta_ag_cb.scb[i].svc_conn &&
            bta_ag_cb.scb[i].post_sco == BTA_AG_POST_SCO_CALL_END) {
           bta_ag_send_call_inds(&bta_ag_cb.scb[i], BTA_AG_END_CALL_RES);
           bta_ag_cb.scb[i].post_sco = BTA_AG_POST_SCO_NONE;
           APPL_TRACE_IMP("%s: sending call end indicators after SCO close for scb" \
              " on index %x, device %s",
               __func__, i, bta_ag_cb.scb[i].peer_addr.ToString().c_str());
        }
      }
      break;

    case BTA_AG_POST_SCO_CALL_END_INCALL:
      bta_ag_send_call_inds(p_scb, BTA_AG_END_CALL_RES);

      /* Sending callsetup IND and Ring were defered to after SCO close. */
      bta_ag_send_call_inds(p_scb, BTA_AG_IN_CALL_RES);

      if (bta_ag_inband_enabled(p_scb) &&
          !(p_scb->features & BTA_AG_FEAT_NOSCO)) {
        p_scb->post_sco = BTA_AG_POST_SCO_RING;
        bta_ag_sco_open(p_scb, p_data);
      } else {
        p_scb->post_sco = BTA_AG_POST_SCO_NONE;
        bta_ag_send_ring(p_scb, p_data);
      }
      break;

    default:
      break;
  }
}

/*******************************************************************************
 *
 * Function         bta_ag_svc_conn_open
 *
 * Description      Service level connection opened
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_ag_svc_conn_open(tBTA_AG_SCB* p_scb,
                          UNUSED_ATTR tBTA_AG_DATA* p_data) {
  tBTA_AG_CONN evt;

  if (!p_scb->svc_conn) {
    /* set state variable */
    p_scb->svc_conn = true;

    /* Clear AT+BIA mask from previous SLC if any. */
    p_scb->bia_masked_out = 0;

    alarm_cancel(p_scb->ring_timer);

    /* call callback */
    evt.hdr.handle = bta_ag_scb_to_idx(p_scb);
    evt.hdr.app_id = p_scb->app_id;
    evt.peer_feat = p_scb->peer_features;
    evt.bd_addr = p_scb->peer_addr;
    evt.peer_codec = p_scb->peer_codecs;

    if ((p_scb->call_ind != BTA_AG_CALL_INACTIVE) ||
        (p_scb->callsetup_ind != BTA_AG_CALLSETUP_NONE)) {
      bta_sys_sco_use(BTA_ID_AG, p_scb->app_id, p_scb->peer_addr);
    }
    if (bta_ag_get_active_device().IsEmpty()) {
      tBTA_AG_DATA data = {};
      data.api_set_active_device.active_device_addr = p_scb->peer_addr;
      bta_ag_api_set_active_device(&data);
    }
    (*bta_ag_cb.p_cback)(BTA_AG_CONN_EVT, (tBTA_AG*)&evt);

#if (TWS_AG_ENABLED == TRUE)
    /* if this is TWSP device and SCO is active on primary sm
     * trigger SCO_OPEN for this scb*/
    if (is_twsp_device(p_scb->peer_addr)) {
        tBTA_AG_SCB* other_scb = NULL;
        other_scb = get_other_twsp_scb(p_scb->peer_addr);
        if (other_scb != NULL) {
            tBTA_AG_SCO_CB *related_sco = NULL;
            if (other_scb == bta_ag_cb.main_sm_scb) {
                if (bta_ag_cb.sco.p_curr_scb == bta_ag_cb.main_sm_scb) {
                    related_sco = &(bta_ag_cb.sco);
                }
            } else if(other_scb == bta_ag_cb.sec_sm_scb) {
                APPL_TRACE_DEBUG("%s:TWS+ peer SCO is selected", __func__);
                related_sco = &(bta_ag_cb.twsp_sec_sco);
            } else {
                APPL_TRACE_ERROR("%s: Invalid SCB", __func__);
            }

            if (related_sco != NULL && (related_sco->state ==  BTA_AG_SCO_OPENING_ST
                     || related_sco->state == BTA_AG_SCO_OPEN_ST)) {
                APPL_TRACE_DEBUG("%s: triggering secondary twsp sco cn", __func__);
                bta_ag_sco_open(p_scb, p_data);
            } else {
                APPL_TRACE_DEBUG("other SCB is not initialized, no need of tws sco trigger");
            }
        }
    }
#endif
    }
}

/*******************************************************************************
 *
 * Function         bta_ag_ci_rx_data
 *
 * Description      Send result code
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_ag_ci_rx_data(tBTA_AG_SCB* p_scb, tBTA_AG_DATA* p_data) {
  uint16_t len;
  tBTA_AG_CI_RX_WRITE* p_rx_write_msg = (tBTA_AG_CI_RX_WRITE*)p_data;
  char* p_data_area =
      (char*)(p_rx_write_msg + 1); /* Point to data area after header */

  APPL_TRACE_DEBUG("bta_ag_ci_rx_data:");
  /* send to RFCOMM */
  bta_sys_busy(BTA_ID_AG, p_scb->app_id, p_scb->peer_addr);
  PORT_WriteData(p_scb->conn_handle, p_data_area, strlen(p_data_area), &len);
  if ((p_scb->sco_idx != BTM_INVALID_SCO_INDEX) && bta_ag_sco_is_open(p_scb)) {
    APPL_TRACE_IMP("bta_ag_rfc_data, change link policy for SCO");
    bta_sys_sco_open(BTA_ID_AG, p_scb->app_id, p_scb->peer_addr);
  } else {
    bta_sys_idle(BTA_ID_AG, p_scb->app_id, p_scb->peer_addr);
  }
}

/*******************************************************************************
 *
 * Function         bta_ag_rcvd_slc_ready
 *
 * Description      Handles SLC ready call-in in case of pass-through mode.
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_ag_rcvd_slc_ready(tBTA_AG_SCB* p_scb,
                           UNUSED_ATTR tBTA_AG_DATA* p_data) {
  APPL_TRACE_DEBUG("bta_ag_rcvd_slc_ready: handle = %d",
                   bta_ag_scb_to_idx(p_scb));

  if (bta_ag_cb.parse_mode == BTA_AG_PASS_THROUGH) {
    /* In pass-through mode, BTA knows that SLC is ready only through call-in.
     */
    bta_ag_svc_conn_open(p_scb, NULL);
  }
}

/*******************************************************************************
 *
 * Function         bta_ag_setcodec
 *
 * Description      Handle API SetCodec
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_ag_setcodec(tBTA_AG_SCB* p_scb, tBTA_AG_DATA* p_data) {
  tBTA_AG_PEER_CODEC codec_type = p_data->api_setcodec.codec;
  tBTA_AG_VAL val;
  val.hdr.handle = bta_ag_scb_to_idx(p_scb);
  val.hdr.app_id = p_scb->app_id;
  val.bd_addr = p_scb->peer_addr;

  /* Check if the requested codec type is valid */
  if ((codec_type != BTA_AG_CODEC_NONE) && (codec_type != BTA_AG_CODEC_CVSD) &&
      (codec_type != BTA_AG_CODEC_MSBC)
#if (SWB_ENABLED == TRUE)
      && (codec_type != BTA_AG_SCO_SWB_SETTINGS_Q0)
#endif
    ) {
    val.num = codec_type;
    val.hdr.status = BTA_AG_FAIL_RESOURCES;
    APPL_TRACE_ERROR("bta_ag_setcodec error: unsupported codec type %d",
                     codec_type);
    (*bta_ag_cb.p_cback)(BTA_AG_WBS_EVT, (tBTA_AG*)&val);
#if (SWB_ENABLED == TRUE)
    (*bta_ag_cb.p_cback)(BTA_AG_SWB_EVT, (tBTA_AG*)&val);
#endif
    return;
  }

  if ((p_scb->peer_codecs & codec_type) || (codec_type == BTA_AG_CODEC_NONE) ||
      (codec_type == BTA_AG_CODEC_CVSD)
#if (SWB_ENABLED == TRUE)
      || (codec_type == BTA_AG_SCO_SWB_SETTINGS_Q0)
#endif
    ) {
    p_scb->sco_codec = codec_type;
    p_scb->codec_updated = true;
    val.num = codec_type;
    val.hdr.status = BTA_AG_SUCCESS;
    APPL_TRACE_DEBUG("bta_ag_setcodec: Updated codec type %d", codec_type);
  } else {
    val.num = codec_type;
    val.hdr.status = BTA_AG_FAIL_RESOURCES;
    APPL_TRACE_ERROR("bta_ag_setcodec error: unsupported codec type %d",
                     codec_type);
  }
#if (SWB_ENABLED == TRUE)
  if (codec_type == BTA_AG_SCO_SWB_SETTINGS_Q0)
  {
    (*bta_ag_cb.p_cback)(BTA_AG_SWB_EVT, (tBTA_AG*)&val);
  } else
#endif
  {
    (*bta_ag_cb.p_cback)(BTA_AG_WBS_EVT, (tBTA_AG*)&val);
  }
}

void bta_ag_handle_collision(tBTA_AG_SCB* p_scb,
                             tBTA_AG_DATA* data) {
  /* Cancel SDP if it had been started. */
  if (p_scb->p_disc_db) {
    SDP_CancelServiceSearch(p_scb->p_disc_db);
    bta_ag_free_db(p_scb, NULL);
  }

  APPL_TRACE_IMP("%s: sending RFCOMM fail event to btif for dev %s",
                  __func__, p_scb->peer_addr.ToString().c_str())
  bta_ag_cback_open(p_scb, NULL, BTA_AG_FAIL_RFCOMM);
  APPL_TRACE_DEBUG("%s: clear peer_addr so that instance can be reused", __func__);
  p_scb->peer_addr = RawAddress::kEmpty;

  /* reopen registered servers */
  /* Collision may be detected before or after we close servers. */
  if (bta_ag_is_server_closed(p_scb)) {
    bta_ag_start_servers(p_scb, p_scb->reg_services);
  }

  /* connection is retried from apps, no need for connection attempt again*/
}
