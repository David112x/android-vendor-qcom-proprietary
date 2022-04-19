/******************************************************************************
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
 *  This is the main implementation file for the BTA audio gateway.
 *
 ******************************************************************************/

#include <string.h>

#include "bta_ag_co.h"
#include "bta_ag_int.h"
#include "bta_api.h"
#include "bta_sys.h"
#include "osi/include/osi.h"
#include "utl.h"

/*****************************************************************************
 * Constants and types
 ****************************************************************************/

static const char* bta_ag_evt_str(uint16_t event);
static const char* bta_ag_state_str(uint8_t state);
static const char* bta_ag_res_str(tBTA_AG_RES result);

/* state machine action enumeration list */
enum {
  BTA_AG_REGISTER,
  BTA_AG_DEREGISTER,
  BTA_AG_START_OPEN,
  BTA_AG_RFC_DO_OPEN,
  BTA_AG_RFC_DO_CLOSE,
  BTA_AG_START_DEREG,
  BTA_AG_START_CLOSE,
  BTA_AG_RFC_OPEN,
  BTA_AG_OPEN_FAIL,
  BTA_AG_RFC_ACP_OPEN,
  BTA_AG_RFC_CLOSE,
  BTA_AG_RFC_FAIL,
  BTA_AG_RFC_DATA,
  BTA_AG_DISC_INT_RES,
  BTA_AG_DISC_FAIL,
  BTA_AG_DISC_ACP_RES,
  BTA_AG_FREE_DB,
  BTA_AG_SCO_CONN_OPEN,
  BTA_AG_SCO_CONN_CLOSE,
  BTA_AG_SCO_LISTEN,
  BTA_AG_SCO_OPEN,
  BTA_AG_SCO_CLOSE,
  BTA_AG_SCO_SHUTDOWN,
  BTA_AG_POST_SCO_OPEN,
  BTA_AG_POST_SCO_CLOSE,
  BTA_AG_SVC_CONN_OPEN,
  BTA_AG_RESULT,
  BTA_AG_SETCODEC,
  BTA_AG_SEND_RING,
  BTA_AG_CI_SCO_DATA,
  BTA_AG_CI_RX_DATA,
  BTA_AG_RCVD_SLC_READY,
  BTA_AG_HANDLE_COLLISION,
  BTA_AG_NUM_ACTIONS
};

#define BTA_AG_IGNORE BTA_AG_NUM_ACTIONS

/* type for action functions */
typedef void (*tBTA_AG_ACTION)(tBTA_AG_SCB* p_scb, tBTA_AG_DATA* p_data);

/* action functions */
const tBTA_AG_ACTION bta_ag_action[] = {
    bta_ag_register,       bta_ag_deregister,    bta_ag_start_open,
    bta_ag_rfc_do_open,    bta_ag_rfc_do_close,  bta_ag_start_dereg,
    bta_ag_start_close,    bta_ag_rfc_open,      bta_ag_open_fail,
    bta_ag_rfc_acp_open,   bta_ag_rfc_close,     bta_ag_rfc_fail,
    bta_ag_rfc_data,       bta_ag_disc_int_res,  bta_ag_disc_fail,
    bta_ag_disc_acp_res,   bta_ag_free_db,       bta_ag_sco_conn_open,
    bta_ag_sco_conn_close, bta_ag_sco_listen,    bta_ag_sco_open,
    bta_ag_sco_close,      bta_ag_sco_shutdown,  bta_ag_post_sco_open,
    bta_ag_post_sco_close, bta_ag_svc_conn_open, bta_ag_result,
    bta_ag_setcodec,       bta_ag_send_ring,     bta_ag_ci_sco_data,
    bta_ag_ci_rx_data,     bta_ag_rcvd_slc_ready,bta_ag_handle_collision};

/* state table information */
#define BTA_AG_ACTIONS 2    /* number of actions */
#define BTA_AG_NEXT_STATE 2 /* position of next state */
#define BTA_AG_NUM_COLS 3   /* number of columns in state tables */

/* state table for init state */
const uint8_t bta_ag_st_init[][BTA_AG_NUM_COLS] = {
    /* Event                    Action 1                Action 2 Next state */
    /* API_REGISTER_EVT */ {BTA_AG_REGISTER, BTA_AG_IGNORE, BTA_AG_INIT_ST},
    /* API_DEREGISTER_EVT */ {BTA_AG_DEREGISTER, BTA_AG_IGNORE, BTA_AG_INIT_ST},
    /* API_OPEN_EVT */ {BTA_AG_START_OPEN, BTA_AG_IGNORE, BTA_AG_OPENING_ST},
    /* API_CLOSE_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_INIT_ST},
    /* API_AUDIO_OPEN_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_INIT_ST},
    /* API_AUDIO_CLOSE_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_INIT_ST},
    /* API_RESULT_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_INIT_ST},
    /* API_SETCODEC_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_INIT_ST},
    /* RFC_OPEN_EVT */ {BTA_AG_RFC_ACP_OPEN, BTA_AG_SCO_LISTEN, BTA_AG_OPEN_ST},
    /* RFC_CLOSE_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_INIT_ST},
    /* RFC_SRV_CLOSE_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_INIT_ST},
    /* RFC_DATA_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_INIT_ST},
    /* SCO_OPEN_EVT */ {BTA_AG_SCO_CONN_OPEN, BTA_AG_IGNORE, BTA_AG_INIT_ST},
    /* SCO_CLOSE_EVT */ {BTA_AG_SCO_CONN_CLOSE, BTA_AG_IGNORE, BTA_AG_INIT_ST},
    /* DISC_ACP_RES_EVT */ {BTA_AG_FREE_DB, BTA_AG_IGNORE, BTA_AG_INIT_ST},
    /* DISC_INT_RES_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_INIT_ST},
    /* DISC_OK_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_INIT_ST},
    /* DISC_FAIL_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_INIT_ST},
    /* CI_RX_WRITE_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_INIT_ST},
    /* RING_TOUT_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_INIT_ST},
    /* SVC_TOUT_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_INIT_ST},
    /* CI_SCO_DATA_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_INIT_ST},
    /* CI_SLC_READY_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_INIT_ST},
    /* COLLISION_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_INIT_ST}};

/* state table for opening state */
const uint8_t bta_ag_st_opening[][BTA_AG_NUM_COLS] = {
    /* Event                    Action 1                Action 2 Next state */
    /* API_REGISTER_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_OPENING_ST},
    /* API_DEREGISTER_EVT */ {BTA_AG_RFC_DO_CLOSE, BTA_AG_START_DEREG,
                              BTA_AG_CLOSING_ST},
    /* API_OPEN_EVT */ {BTA_AG_OPEN_FAIL, BTA_AG_IGNORE, BTA_AG_OPENING_ST},
    /* API_CLOSE_EVT */ {BTA_AG_RFC_DO_CLOSE, BTA_AG_IGNORE, BTA_AG_CLOSING_ST},
    /* API_AUDIO_OPEN_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_OPENING_ST},
    /* API_AUDIO_CLOSE_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_OPENING_ST},
    /* API_RESULT_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_OPENING_ST},
    /* API_SETCODEC_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_OPENING_ST},
    /* RFC_OPEN_EVT */ {BTA_AG_RFC_OPEN, BTA_AG_SCO_LISTEN, BTA_AG_OPEN_ST},
    /* RFC_CLOSE_EVT */ {BTA_AG_RFC_FAIL, BTA_AG_IGNORE, BTA_AG_INIT_ST},
    /* RFC_SRV_CLOSE_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_OPENING_ST},
    /* RFC_DATA_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_OPENING_ST},
    /* SCO_OPEN_EVT */ {BTA_AG_SCO_CONN_OPEN, BTA_AG_IGNORE, BTA_AG_OPENING_ST},
    /* SCO_CLOSE_EVT */ {BTA_AG_SCO_CONN_CLOSE, BTA_AG_IGNORE,
                         BTA_AG_OPENING_ST},
    /* DISC_ACP_RES_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_OPENING_ST},
    /* DISC_INT_RES_EVT */ {BTA_AG_DISC_INT_RES, BTA_AG_IGNORE,
                            BTA_AG_OPENING_ST},
    /* DISC_OK_EVT */ {BTA_AG_RFC_DO_OPEN, BTA_AG_IGNORE, BTA_AG_OPENING_ST},
    /* DISC_FAIL_EVT */ {BTA_AG_DISC_FAIL, BTA_AG_IGNORE, BTA_AG_INIT_ST},
    /* CI_RX_WRITE_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_OPENING_ST},
    /* RING_TOUT_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_OPENING_ST},
    /* SVC_TOUT_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_OPENING_ST},
    /* CI_SCO_DATA_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_OPENING_ST},
    /* CI_SLC_READY_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_OPENING_ST},
    /* COLLISION_EVT */ {BTA_AG_HANDLE_COLLISION, BTA_AG_IGNORE, BTA_AG_INIT_ST}};

/* state table for open state */
const uint8_t bta_ag_st_open[][BTA_AG_NUM_COLS] = {
    /* Event                    Action 1                Action 2 Next state */
    /* API_REGISTER_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_OPEN_ST},
    /* API_DEREGISTER_EVT */ {BTA_AG_START_CLOSE, BTA_AG_START_DEREG,
                              BTA_AG_CLOSING_ST},
    /* API_OPEN_EVT */ {BTA_AG_OPEN_FAIL, BTA_AG_IGNORE, BTA_AG_OPEN_ST},
    /* API_CLOSE_EVT */ {BTA_AG_START_CLOSE, BTA_AG_IGNORE, BTA_AG_CLOSING_ST},
    /* API_AUDIO_OPEN_EVT */ {BTA_AG_SCO_OPEN, BTA_AG_IGNORE, BTA_AG_OPEN_ST},
    /* API_AUDIO_CLOSE_EVT */ {BTA_AG_SCO_CLOSE, BTA_AG_IGNORE, BTA_AG_OPEN_ST},
    /* API_RESULT_EVT */ {BTA_AG_RESULT, BTA_AG_IGNORE, BTA_AG_OPEN_ST},
    /* API_SETCODEC_EVT */ {BTA_AG_SETCODEC, BTA_AG_IGNORE, BTA_AG_OPEN_ST},
    /* RFC_OPEN_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_OPEN_ST},
    /* RFC_CLOSE_EVT */ {BTA_AG_RFC_CLOSE, BTA_AG_IGNORE, BTA_AG_INIT_ST},
    /* RFC_SRV_CLOSE_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_OPEN_ST},
    /* RFC_DATA_EVT */ {BTA_AG_RFC_DATA, BTA_AG_IGNORE, BTA_AG_OPEN_ST},
    /* SCO_OPEN_EVT */ {BTA_AG_SCO_CONN_OPEN, BTA_AG_POST_SCO_OPEN,
                        BTA_AG_OPEN_ST},
    /* SCO_CLOSE_EVT */ {BTA_AG_SCO_CONN_CLOSE, BTA_AG_POST_SCO_CLOSE,
                         BTA_AG_OPEN_ST},
    /* DISC_ACP_RES_EVT */ {BTA_AG_DISC_ACP_RES, BTA_AG_IGNORE, BTA_AG_OPEN_ST},
    /* DISC_INT_RES_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_OPEN_ST},
    /* DISC_OK_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_OPEN_ST},
    /* DISC_FAIL_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_OPEN_ST},
    /* CI_RX_WRITE_EVT */ {BTA_AG_CI_RX_DATA, BTA_AG_IGNORE, BTA_AG_OPEN_ST},
    /* RING_TOUT_EVT */ {BTA_AG_SEND_RING, BTA_AG_IGNORE, BTA_AG_OPEN_ST},
    /* SVC_TOUT_EVT */ {BTA_AG_START_CLOSE, BTA_AG_IGNORE, BTA_AG_CLOSING_ST},
    /* CI_SCO_DATA_EVT */ {BTA_AG_CI_SCO_DATA, BTA_AG_IGNORE, BTA_AG_OPEN_ST},
    /* CI_SLC_READY_EVT */ {BTA_AG_RCVD_SLC_READY, BTA_AG_IGNORE, BTA_AG_OPEN_ST},
    /* COLLISION_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_OPEN_ST}};

/* state table for closing state */
const uint8_t bta_ag_st_closing[][BTA_AG_NUM_COLS] = {
    /* Event                    Action 1                Action 2 Next state */
    /* API_REGISTER_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_CLOSING_ST},
    /* API_DEREGISTER_EVT */ {BTA_AG_START_DEREG, BTA_AG_IGNORE,
                              BTA_AG_CLOSING_ST},
    /* API_OPEN_EVT */ {BTA_AG_OPEN_FAIL, BTA_AG_IGNORE, BTA_AG_CLOSING_ST},
    /* API_CLOSE_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_CLOSING_ST},
    /* API_AUDIO_OPEN_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_CLOSING_ST},
    /* API_AUDIO_CLOSE_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_CLOSING_ST},
    /* API_RESULT_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_CLOSING_ST},
    /* API_SETCODEC_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_CLOSING_ST},
    /* RFC_OPEN_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_CLOSING_ST},
    /* RFC_CLOSE_EVT */ {BTA_AG_RFC_CLOSE, BTA_AG_IGNORE, BTA_AG_INIT_ST},
    /* RFC_SRV_CLOSE_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_CLOSING_ST},
    /* RFC_DATA_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_CLOSING_ST},
    /* SCO_OPEN_EVT */ {BTA_AG_SCO_CONN_OPEN, BTA_AG_IGNORE, BTA_AG_CLOSING_ST},
    /* SCO_CLOSE_EVT */ {BTA_AG_SCO_CONN_CLOSE, BTA_AG_POST_SCO_CLOSE,
                         BTA_AG_CLOSING_ST},
    /* DISC_ACP_RES_EVT */ {BTA_AG_FREE_DB, BTA_AG_IGNORE, BTA_AG_CLOSING_ST},
    /* DISC_INT_RES_EVT */ {BTA_AG_FREE_DB, BTA_AG_IGNORE, BTA_AG_INIT_ST},
    /* DISC_OK_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_CLOSING_ST},
    /* DISC_FAIL_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_CLOSING_ST},
    /* CI_RX_WRITE_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_CLOSING_ST},
    /* RING_TOUT_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_CLOSING_ST},
    /* SVC_TOUT_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_CLOSING_ST},
    /* CI_SCO_DATA_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_CLOSING_ST},
    /* CI_SLC_READY_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_CLOSING_ST},
    /* COLLISION_EVT */ {BTA_AG_IGNORE, BTA_AG_IGNORE, BTA_AG_CLOSING_ST}};

/* type for state table */
typedef const uint8_t (*tBTA_AG_ST_TBL)[BTA_AG_NUM_COLS];

/* state table */
const tBTA_AG_ST_TBL bta_ag_st_tbl[] = {bta_ag_st_init, bta_ag_st_opening,
                                        bta_ag_st_open, bta_ag_st_closing};

/*****************************************************************************
 * Global data
 ****************************************************************************/

/* AG control block */
tBTA_AG_CB bta_ag_cb;

/*******************************************************************************
 *
 * Function         bta_ag_scb_alloc
 *
 * Description      Allocate an AG service control block.
 *
 *
 * Returns          pointer to the scb, or NULL if none could be allocated.
 *
 ******************************************************************************/
static tBTA_AG_SCB* bta_ag_scb_alloc(void) {
  tBTA_AG_SCB* p_scb = &bta_ag_cb.scb[0];
  int i;

  for (i = 0; i < BTA_AG_MAX_NUM_CLIENTS; i++, p_scb++) {
    if (!p_scb->in_use) {
      /* initialize variables */
      p_scb->in_use = true;
      p_scb->sco_idx = BTM_INVALID_SCO_INDEX;
      p_scb->codec_updated = false;
      p_scb->codec_fallback = false;
      p_scb->peer_codecs = BTA_AG_CODEC_CVSD;
      p_scb->sco_codec = BTA_AG_CODEC_CVSD;
      p_scb->no_of_xsco_trials = 0;
      /* set up timers */
      p_scb->ring_timer = alarm_new("bta_ag.scb_ring_timer");
      p_scb->collision_timer = alarm_new("bta_ag.scb_collision_timer");
      p_scb->codec_negotiation_timer =
          alarm_new("bta_ag.scb_codec_negotiation_timer");
      p_scb->xsco_conn_collision_timer =
          alarm_new("bta_ag.scb_xsco_conn_collision_timer");
      /* set eSCO mSBC setting to T2 as the preferred */
      p_scb->codec_msbc_settings = BTA_AG_SCO_MSBC_SETTINGS_T2;
#if (SWB_ENABLED == TRUE)
      /* set eSCO SWB setting to Q0 as the preferred */
      p_scb->codec_swb_settings = BTA_AG_SCO_SWB_SETTINGS_Q0;
      p_scb->is_swb_codec = false;
#endif
      APPL_TRACE_DEBUG("bta_ag_scb_alloc %d", bta_ag_scb_to_idx(p_scb));
      break;
    }
  }

  if (i == BTA_AG_MAX_NUM_CLIENTS) {
    /* out of scbs */
    p_scb = NULL;
    APPL_TRACE_WARNING("%s: Out of scbs", __func__);
  }
  return p_scb;
}

/*******************************************************************************
 *
 * Function         bta_ag_scb_dealloc
 *
 * Description      Deallocate a service control block.
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_ag_scb_dealloc(tBTA_AG_SCB* p_scb) {
  uint8_t idx;
  bool allocated = false;

  APPL_TRACE_DEBUG("bta_ag_scb_dealloc %d", bta_ag_scb_to_idx(p_scb));

  if (p_scb->p_disc_db) {
    APPL_TRACE_DEBUG(" %s Cancel pending SDP ",__func__);
    (void)SDP_CancelServiceSearch(p_scb->p_disc_db);
    bta_ag_free_db(p_scb, NULL);
  }

  /* stop and free timers */
  alarm_free(p_scb->ring_timer);
  alarm_free(p_scb->codec_negotiation_timer);
  alarm_free(p_scb->collision_timer);
  alarm_free(p_scb->xsco_conn_collision_timer);

  /* initialize control block */
  memset(p_scb, 0, sizeof(tBTA_AG_SCB));
  p_scb->sco_idx = BTM_INVALID_SCO_INDEX;

  /* If all scbs are deallocated, callback with disable event */
  if (!bta_sys_is_register(BTA_ID_AG)) {
    for (idx = 0; idx < BTA_AG_MAX_NUM_CLIENTS; idx++) {
      if (bta_ag_cb.scb[idx].in_use) {
        allocated = true;
        break;
      }
    }

    if (!allocated) {
      (*bta_ag_cb.p_cback)(BTA_AG_DISABLE_EVT, NULL);
    }
  }
}

/*******************************************************************************
 *
 * Function         bta_ag_scb_to_idx
 *
 * Description      Given a pointer to an scb, return its index.
 *
 *
 * Returns          Index of scb.
 *
 ******************************************************************************/
uint16_t bta_ag_scb_to_idx(tBTA_AG_SCB* p_scb) {
  /* use array arithmetic to determine index */
  return ((uint16_t)(p_scb - bta_ag_cb.scb)) + 1;
}

/*******************************************************************************
 *
 * Function         bta_ag_scb_by_idx
 *
 * Description      Given an scb index return pointer to scb.
 *
 *
 * Returns          Pointer to scb or NULL if not allocated.
 *
 ******************************************************************************/
tBTA_AG_SCB* bta_ag_scb_by_idx(uint16_t idx) {
  tBTA_AG_SCB* p_scb;

  /* verify index */
  if (idx > 0 && idx <= BTA_AG_MAX_NUM_CLIENTS) {
    p_scb = &bta_ag_cb.scb[idx - 1];
    if (!p_scb->in_use) {
      p_scb = NULL;
      APPL_TRACE_WARNING("ag scb idx %d not allocated", idx);
    }
  } else {
    p_scb = NULL;
    APPL_TRACE_DEBUG("ag scb idx %d out of range", idx);
  }
  return p_scb;
}

/*******************************************************************************
 *
 * Function         bta_ag_service_to_idx
 *
 * Description      Given a BTA service mask convert to profile index.
 *
 *
 * Returns          Profile ndex of scb.
 *
 ******************************************************************************/
uint8_t bta_ag_service_to_idx(tBTA_SERVICE_MASK services) {
  if (services & BTA_HFP_SERVICE_MASK) {
    return BTA_AG_HFP;
  } else {
    return BTA_AG_HSP;
  }
}

/*******************************************************************************
 *
 * Function         bta_ag_idx_by_bdaddr
 *
 * Description      Find SCB associated with peer BD address.
 *
 *
 * Returns          Index of SCB or zero if none found.
 *
 ******************************************************************************/
uint16_t bta_ag_idx_by_bdaddr(const RawAddress* peer_addr) {
  tBTA_AG_SCB* p_scb = &bta_ag_cb.scb[0];
  uint16_t i;

  if (peer_addr != NULL) {
    for (i = 0; i < BTA_AG_MAX_NUM_CLIENTS; i++, p_scb++) {
      if (p_scb->in_use && *peer_addr == p_scb->peer_addr) {
        return (i + 1);
      }
    }
  }

  /* no scb found */
  APPL_TRACE_WARNING("No ag scb for peer addr");
  return 0;
}

/*******************************************************************************
 *
 * Function         bta_ag_other_scb_open
 *
 * Description      Check whether any other scb is in open state.
 *
 *
 * Returns          true if another scb is in open state, false otherwise.
 *
 ******************************************************************************/
bool bta_ag_other_scb_open(tBTA_AG_SCB* p_curr_scb) {
  tBTA_AG_SCB* p_scb = &bta_ag_cb.scb[0];
  int i;

  for (i = 0; i < BTA_AG_MAX_NUM_CLIENTS; i++, p_scb++) {
    if (p_scb->in_use && p_scb != p_curr_scb &&
#if (TWS_AG_ENABLED == TRUE)
        p_scb != bta_ag_cb.sec_sm_scb && /*ignore sec tws scb*/
#endif
        p_scb->state == BTA_AG_OPEN_ST) {
      return true;
    }
  }

  /* no other scb found */
  APPL_TRACE_DEBUG("No other ag scb open");
  return false;
}

/*******************************************************************************
 *
 * Function         bta_ag_scb_open
 *
 * Description      Check whether given scb is in open state.
 *
 *
 * Returns          true if scb is in open state, false otherwise.
 *
 ******************************************************************************/
bool bta_ag_scb_open(tBTA_AG_SCB* p_curr_scb) {
  if (p_curr_scb && p_curr_scb->in_use && p_curr_scb->state == BTA_AG_OPEN_ST) {
    return true;
  }

  return false;
}

/*******************************************************************************
 *
 * Function         bta_ag_get_other_idle_scb
 *
 * Description      Return other scb if it is in INIT st.
 *
 *
 * Returns          Pointer to other scb if INIT st, NULL otherwise.
 *
 ******************************************************************************/
tBTA_AG_SCB* bta_ag_get_other_idle_scb(tBTA_AG_SCB* p_curr_scb) {
  tBTA_AG_SCB* p_scb = &bta_ag_cb.scb[0];
  uint8_t xx;

  for (xx = 0; xx < BTA_AG_MAX_NUM_CLIENTS; xx++, p_scb++) {
    if (p_scb->in_use && (p_scb != p_curr_scb) &&
        (p_scb->state == BTA_AG_INIT_ST)) {
      return p_scb;
    }
  }

  /* no other scb found */
  APPL_TRACE_DEBUG("bta_ag_get_other_idle_scb: No idle AG scb");
  return NULL;
}

/*******************************************************************************
 *
 * Function         bta_ag_collision_cback
 *
 * Description      Get notified about collision.
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_ag_collision_cback(UNUSED_ATTR tBTA_SYS_CONN_STATUS status, uint8_t id,
                            UNUSED_ATTR uint8_t app_id,
                            const RawAddress& peer_addr) {
  uint16_t handle;
  tBTA_AG_SCB* p_scb;

  /* Check if we have opening scb for the peer device. */
  handle = bta_ag_idx_by_bdaddr(&peer_addr);
  p_scb = bta_ag_scb_by_idx(handle);

  if (p_scb && (p_scb->state == BTA_AG_OPENING_ST)) {
    if (id == BTA_ID_SYS) /* ACL collision */
    {
      LOG(WARNING) << __func__ << " AG found collision (ACL) for handle "
                   << unsigned(handle) << " device " << peer_addr;
    } else if (id == BTA_ID_AG) /* RFCOMM collision */
    {
      LOG(WARNING) << __func__ << " AG found collision (RFCOMM) for handle "
                   << unsigned(handle) << " device " << peer_addr;
    } else {
      LOG(WARNING) << __func__ << " AG found collision (UNKNOWN) for handle "
                   << unsigned(handle) << " device " << peer_addr;
    }
    bta_ag_sm_execute(p_scb, BTA_AG_COLLISION_EVT, NULL);
  }
}

/*******************************************************************************
 *
 * Function         bta_ag_resume_open
 *
 * Description      Resume opening process.
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_ag_resume_open(tBTA_AG_SCB* p_scb) {
  if (p_scb->state == BTA_AG_INIT_ST) {
    LOG(INFO) << __func__ << ": Resume connection to " << p_scb->peer_addr
              << ", handle" << bta_ag_scb_to_idx(p_scb);
    tBTA_AG_DATA open_data = {.api_open.bd_addr = p_scb->peer_addr,
                              .api_open.services = p_scb->open_services,
                              .api_open.sec_mask = p_scb->cli_sec_mask};
   // TODO: check if we don't need handle

    bta_ag_sm_execute(p_scb, BTA_AG_API_OPEN_EVT, &open_data);
  } else {
    VLOG(1) << __func__ << ": device " << p_scb->peer_addr
            << " is already in state " << std::to_string(p_scb->state);
  }
}

/*******************************************************************************
 *
 * Function         bta_ag_api_enable
 *
 * Description      Handle an API enable event.
 *
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_ag_api_enable(tBTA_AG_DATA* p_data) {
  /* initialize control block */
  for (size_t i = 0; i < BTA_AG_MAX_NUM_CLIENTS; i++) {
    alarm_free(bta_ag_cb.scb[i].ring_timer);
    alarm_free(bta_ag_cb.scb[i].codec_negotiation_timer);
    alarm_free(bta_ag_cb.scb[i].collision_timer);
    alarm_free(bta_ag_cb.scb[i].xsco_conn_collision_timer);
  }
  memset(&bta_ag_cb, 0, sizeof(tBTA_AG_CB));

  /* store callback function */
  bta_ag_cb.p_cback = p_data->api_enable.p_cback;
  bta_ag_cb.parse_mode = p_data->api_enable.parse_mode;

  /* call init call-out */
  bta_ag_co_init();

  bta_sys_collision_register(BTA_ID_AG, bta_ag_collision_cback);

  /* call callback with enable event */
  (*bta_ag_cb.p_cback)(BTA_AG_ENABLE_EVT, NULL);
}

/*******************************************************************************
 *
 * Function         bta_ag_api_disable
 *
 * Description      Handle an API disable event.
 *
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_ag_api_disable(tBTA_AG_DATA* p_data) {
  /* deregister all scbs in use */
  tBTA_AG_SCB* p_scb = &bta_ag_cb.scb[0];
  bool do_dereg = false;
  int i;

  if (!bta_sys_is_register(BTA_ID_AG)) {
    APPL_TRACE_ERROR("BTA AG is already disabled, ignoring ...");
    return;
  }

  /* De-register with BTA system manager */
  bta_sys_deregister(BTA_ID_AG);

  for (i = 0; i < BTA_AG_MAX_NUM_CLIENTS; i++, p_scb++) {
    if (p_scb->in_use) {
      bta_ag_sm_execute(p_scb, BTA_AG_API_DEREGISTER_EVT, p_data);
      do_dereg = true;
    }
  }

  if (!do_dereg) {
    /* Done, send callback evt to app */
    (*bta_ag_cb.p_cback)(BTA_AG_DISABLE_EVT, NULL);
  }

  bta_sys_collision_register(BTA_ID_AG, NULL);
}

/*******************************************************************************
 *
 * Function         bta_ag_api_register
 *
 * Description      Handle an API event registers a new service.
 *
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_ag_api_register(tBTA_AG_DATA* p_data) {
  tBTA_AG_SCB* p_scb;
  tBTA_AG_REGISTER reg;

  /* allocate an scb */
  p_scb = bta_ag_scb_alloc();
  if (p_scb != NULL) {
    APPL_TRACE_DEBUG("bta_ag_api_register: p_scb 0x%08x ", p_scb);
    bta_ag_sm_execute(p_scb, p_data->hdr.event, p_data);
  } else {
    reg.status = BTA_AG_FAIL_RESOURCES;
    (*bta_ag_cb.p_cback)(BTA_AG_REGISTER_EVT, (tBTA_AG*)&reg);
  }
}

/*******************************************************************************
 *
 * Function         bta_ag_api_result
 *
 * Description      Handle an API result event.
 *
 *
 * Returns          void
 *
 ******************************************************************************/
static void bta_ag_api_result(tBTA_AG_DATA* p_data) {
  tBTA_AG_SCB* p_scb;
  int i;

  if (p_data->hdr.layer_specific != BTA_AG_HANDLE_ALL) {
    p_scb = bta_ag_scb_by_idx(p_data->hdr.layer_specific);
    if (p_scb != NULL) {
      APPL_TRACE_IMP("bta_ag_api_result: p_scb 0x%08x ", p_scb);
      bta_ag_sm_execute(p_scb, BTA_AG_API_RESULT_EVT, p_data);
    }
  } else {
    for (i = 0, p_scb = &bta_ag_cb.scb[0]; i < BTA_AG_MAX_NUM_CLIENTS;
         i++, p_scb++) {
      if (p_scb->in_use && p_scb->svc_conn) {
        APPL_TRACE_DEBUG("bta_ag_api_result p_scb 0x%08x ", p_scb);
        bta_ag_sm_execute(p_scb, BTA_AG_API_RESULT_EVT, p_data);
      }
    }
  }
}

/*******************************************************************************
 *
 * Function         bta_ag_sm_execute
 *
 * Description      State machine event handling function for AG
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_ag_sm_execute(tBTA_AG_SCB* p_scb, uint16_t event,
                       tBTA_AG_DATA* p_data) {
  tBTA_AG_ST_TBL state_table;
  uint8_t action;
  int i;
  uint16_t previous_event = event;
  uint8_t previous_state = p_scb->state;
  APPL_TRACE_EVENT(
      "%s: handle=0x%04x, bd_addr=%s, state=%s(0x%02x), "
      "event=%s(0x%04x)",
      __func__, bta_ag_scb_to_idx(p_scb), p_scb->peer_addr.ToString().c_str(),
      bta_ag_state_str(p_scb->state), p_scb->state, bta_ag_evt_str(event),
      event);

  if (p_data != NULL) {
    APPL_TRACE_EVENT("result=%s(0x%02x)",
      bta_ag_res_str(p_data->api_result.result), p_data->api_result.result);
  }

  event &= 0x00FF;
  if (event >= (BTA_AG_MAX_EVT & 0x00FF)) {
    APPL_TRACE_ERROR("%s: event out of range, ignored", __func__);
    return;
  }

  /* look up the state table for the current state */
  state_table = bta_ag_st_tbl[p_scb->state];

  /* set next state */
  p_scb->state = state_table[event][BTA_AG_NEXT_STATE];

  /* execute action functions */
  for (i = 0; i < BTA_AG_ACTIONS; i++) {
    action = state_table[event][i];
    if (action != BTA_AG_IGNORE) {
      (*bta_ag_action[action])(p_scb, p_data);
    } else {
      break;
    }
  }
  if (p_scb->state != previous_state) {
    APPL_TRACE_EVENT(
        "%s: handle=0x%04x, bd_addr=%s, state_change[%s(0x%02x)]->[%s(0x%02x)],"
        " event[%s(0x%04x)]",
        __func__, bta_ag_scb_to_idx(p_scb), p_scb->peer_addr.ToString().c_str(),
        bta_ag_state_str(previous_state), previous_state,
        bta_ag_state_str(p_scb->state), p_scb->state,
        bta_ag_evt_str(previous_event), previous_event);

    if (p_data != NULL) {
       APPL_TRACE_EVENT("result=%s(0x%02x)",
        bta_ag_res_str(p_data->api_result.result), p_data->api_result.result);
    }
  }
}

/*******************************************************************************
 *
 * Function         bta_ag_hdl_event
 *
 * Description      Data gateway main event handling function.
 *
 *
 * Returns          bool
 *
 ******************************************************************************/
bool bta_ag_hdl_event(BT_HDR* p_msg) {
  tBTA_AG_SCB* p_scb;

  APPL_TRACE_DEBUG("bta_ag_hdl_event: Event 0x%04x ", p_msg->event);
  switch (p_msg->event) {
    case BTA_AG_API_ENABLE_EVT:
      bta_ag_api_enable((tBTA_AG_DATA*)p_msg);
      break;

    case BTA_AG_API_DISABLE_EVT:
      bta_ag_api_disable((tBTA_AG_DATA*)p_msg);
      break;

    case BTA_AG_API_REGISTER_EVT:
      bta_ag_api_register((tBTA_AG_DATA*)p_msg);
      break;

    case BTA_AG_API_RESULT_EVT:
      bta_ag_api_result((tBTA_AG_DATA*)p_msg);
      break;

    case BTA_AG_API_SET_SCO_ALLOWED_EVT:
      bta_ag_set_sco_allowed((tBTA_AG_DATA*)p_msg);
      break;

    case BTA_AG_API_SET_ACTIVE_DEVICE_EVT:
      bta_ag_api_set_active_device((tBTA_AG_DATA*)p_msg);
      break;

    /* all others reference scb by handle */
    default:
      p_scb = bta_ag_scb_by_idx(p_msg->layer_specific);
      if (p_scb != NULL) {
        APPL_TRACE_IMP("bta_ag_hdl_event: p_scb 0x%08x ", p_scb);
        bta_ag_sm_execute(p_scb, p_msg->event, (tBTA_AG_DATA*)p_msg);
      }
      break;
  }
  return true;
}

#define CASE_RETURN_STR(const) \
  case const:                  \
    return #const;

static const char* bta_ag_res_str(tBTA_AG_RES result) {
  switch (result) {
    CASE_RETURN_STR(BTA_AG_SPK_RES)
    CASE_RETURN_STR(BTA_AG_MIC_RES)
    CASE_RETURN_STR(BTA_AG_INBAND_RING_RES)
    CASE_RETURN_STR(BTA_AG_CIND_RES)
    CASE_RETURN_STR(BTA_AG_BINP_RES)
    CASE_RETURN_STR(BTA_AG_IND_RES)
    CASE_RETURN_STR(BTA_AG_BVRA_RES)
    CASE_RETURN_STR(BTA_AG_CNUM_RES)
    CASE_RETURN_STR(BTA_AG_BTRH_RES)
    CASE_RETURN_STR(BTA_AG_CLCC_RES)
    CASE_RETURN_STR(BTA_AG_COPS_RES)
    CASE_RETURN_STR(BTA_AG_IN_CALL_RES)
    CASE_RETURN_STR(BTA_AG_IN_CALL_CONN_RES)
    CASE_RETURN_STR(BTA_AG_CALL_WAIT_RES)
    CASE_RETURN_STR(BTA_AG_OUT_CALL_ORIG_RES)
    CASE_RETURN_STR(BTA_AG_OUT_CALL_ALERT_RES)
    CASE_RETURN_STR(BTA_AG_OUT_CALL_CONN_RES)
    CASE_RETURN_STR(BTA_AG_CALL_CANCEL_RES)
    CASE_RETURN_STR(BTA_AG_END_CALL_RES)
    CASE_RETURN_STR(BTA_AG_IN_CALL_HELD_RES)
    CASE_RETURN_STR(BTA_AG_UNAT_RES)
    CASE_RETURN_STR(BTA_AG_MULTI_CALL_RES)
    CASE_RETURN_STR(BTA_AG_BIND_RES)
    CASE_RETURN_STR(BTA_AG_IND_RES_ON_DEMAND)
    default:
      return "Unknown AG Result";
  }
}
static const char* bta_ag_evt_str(uint16_t event) {
  switch (event) {
    CASE_RETURN_STR(BTA_AG_API_REGISTER_EVT)
    CASE_RETURN_STR(BTA_AG_API_DEREGISTER_EVT)
    CASE_RETURN_STR(BTA_AG_API_OPEN_EVT)
    CASE_RETURN_STR(BTA_AG_API_CLOSE_EVT)
    CASE_RETURN_STR(BTA_AG_API_AUDIO_OPEN_EVT)
    CASE_RETURN_STR(BTA_AG_API_AUDIO_CLOSE_EVT)
    CASE_RETURN_STR(BTA_AG_API_RESULT_EVT)
    CASE_RETURN_STR(BTA_AG_API_SETCODEC_EVT)
    CASE_RETURN_STR(BTA_AG_RFC_OPEN_EVT)
    CASE_RETURN_STR(BTA_AG_RFC_CLOSE_EVT)
    CASE_RETURN_STR(BTA_AG_RFC_SRV_CLOSE_EVT)
    CASE_RETURN_STR(BTA_AG_RFC_DATA_EVT)
    CASE_RETURN_STR(BTA_AG_SCO_OPEN_EVT)
    CASE_RETURN_STR(BTA_AG_SCO_CLOSE_EVT)
    CASE_RETURN_STR(BTA_AG_DISC_ACP_RES_EVT)
    CASE_RETURN_STR(BTA_AG_DISC_INT_RES_EVT)
    CASE_RETURN_STR(BTA_AG_DISC_OK_EVT)
    CASE_RETURN_STR(BTA_AG_DISC_FAIL_EVT)
    CASE_RETURN_STR(BTA_AG_RING_TIMEOUT_EVT)
    CASE_RETURN_STR(BTA_AG_SVC_TIMEOUT_EVT)
    default:
      return "Unknown AG Event";
  }
}

static const char* bta_ag_state_str(uint8_t state) {
  switch (state) {
    CASE_RETURN_STR(BTA_AG_INIT_ST)
    CASE_RETURN_STR(BTA_AG_OPENING_ST)
    CASE_RETURN_STR(BTA_AG_OPEN_ST)
    CASE_RETURN_STR(BTA_AG_CLOSING_ST)
    default:
      return "Unknown AG State";
  }
}
