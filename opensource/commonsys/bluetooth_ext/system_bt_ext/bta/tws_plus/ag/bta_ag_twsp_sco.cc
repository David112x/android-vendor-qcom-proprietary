/*
 * Copyright (c) 2017-2018, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the
 * disclaimer below) provided that the following conditions are met:
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
 */
/*This will add sub SM for  secondary earbud's SCO connection */

#include "bta_ag_int.h"
#include "btif_hf.h"
#include "internal_include/bt_trace.h"
#include "device/include/controller.h"
#include "bta_ag_twsp.h"
#include "btm_api.h"
#include <cutils/properties.h>
#include "bta_ag_twsp_dev.h"

#if (TWS_AG_ENABLED == TRUE)

void send_twsp_esco_setup (const RawAddress& left_eb_addr, const RawAddress& rght_eb_addr,
    uint8_t selected_mic);
void print_bdaddr(const RawAddress& addr);

bool is_rfc_connected (tBTA_AG_SCB* p_scb) {
    return (p_scb && p_scb->conn_handle);
}

bool is_svc_connected (tBTA_AG_SCB* p_scb) {
    return (p_scb && p_scb->svc_conn);
}

tBTA_AG_SCB* get_scb(const RawAddress& addr) {
    uint16_t handle;
    tBTA_AG_SCB* p_scb;
    handle = bta_ag_idx_by_bdaddr(&addr);
    if (handle == 0) {
        APPL_TRACE_ERROR("%s: Handle is NULL", __func__);
        return NULL;
    }
    p_scb = bta_ag_scb_by_idx(handle);
    if (p_scb == NULL) {
        APPL_TRACE_ERROR("%s: p_scb is NULL", __func__);
        return NULL;
    }
    return p_scb;
}

bool get_peer_twsp_addr(const RawAddress& addr, RawAddress& p_addr) {
   bool ret = FALSE;
   APPL_TRACE_DEBUG("%s:>", __func__);

   ret = BTM_SecGetTwsPlusPeerDev(addr, p_addr);
   APPL_TRACE_DEBUG("%s: peer bd aadr\n", __func__);
   print_bdaddr(p_addr);
   return ret;
}

tBTA_AG_SCB* get_other_twsp_scb (const RawAddress& addr) {
    RawAddress other_eb_addr;
    get_peer_twsp_addr (addr, other_eb_addr);
    return get_scb (other_eb_addr);
}

bool twsp_sco_not_active(tBTA_AG_SCB* p_scb) {
    bool ret = false;
    if (p_scb == bta_ag_cb.main_sm_scb) {
        if (bta_ag_cb.sco.state == BTA_AG_SCO_LISTEN_ST ||
            bta_ag_cb.sco.state == BTA_AG_SCO_SHUTDOWN_ST)
            ret = true;
    } else if(p_scb == bta_ag_cb.sec_sm_scb) {
        if (bta_ag_cb.twsp_sec_sco.state == BTA_AG_SCO_LISTEN_ST ||
            bta_ag_cb.twsp_sec_sco.state == BTA_AG_SCO_SHUTDOWN_ST)
            ret = true;
    } else {
        APPL_TRACE_ERROR("%s: Invalid scb : %x", __func__, p_scb);
    }
    APPL_TRACE_DEBUG("%s: returning : %d", __func__, ret);
    return ret;
}

bool twsp_sco_active(tBTA_AG_SCB* p_scb) {
    bool ret = false;
    if (p_scb == bta_ag_cb.main_sm_scb) {
        if (bta_ag_cb.sco.state == BTA_AG_SCO_OPEN_ST)
            ret = true;
    } else if(p_scb == bta_ag_cb.sec_sm_scb) {
        if (bta_ag_cb.twsp_sec_sco.state == BTA_AG_SCO_OPEN_ST)
            ret = true;
    } else {
        APPL_TRACE_ERROR("%s: Invalid scb : %x", __func__, p_scb);
    }
    APPL_TRACE_DEBUG("%s: returning : %d", __func__, ret);
    return ret;
}

void  dispatch_event_twsp_peer_device(tBTA_AG_SCB *p_scb, uint8_t event) {
    tBTA_AG_SCB *other_scb = get_other_twsp_scb((p_scb->peer_addr));
    if (other_scb) {
        if (is_svc_connected(other_scb)) {
           if (event == BTA_AG_SCO_OPEN_E) {
              APPL_TRACE_WARNING("%s: Inform BTIF on SCO init", __func__);
              bta_ag_cback_sco(other_scb, BTA_AG_AUDIO_OPENING_EVT);
           }
           bta_ag_twsp_sco_event(other_scb, event);
        } else {
           APPL_TRACE_WARNING("%s: RFC is not up yet", __func__);
        }
    } else {
        APPL_TRACE_WARNING("%s: other_scb is NULL", __func__);
    }
}

void  dispatch_event_primary_peer_device(tBTA_AG_SCB *p_scb, uint8_t event) {
    tBTA_AG_SCB *other_scb = get_other_twsp_scb((p_scb->peer_addr));
    APPL_TRACE_DEBUG("%s: other_scb: %x", other_scb);
    if (other_scb) {
        if (is_svc_connected(other_scb)) {
           if (event == BTA_AG_SCO_OPEN_E) {
              APPL_TRACE_WARNING("%s: Inform BTIF on SCO init", __func__);
              bta_ag_cback_sco(other_scb, BTA_AG_AUDIO_OPENING_EVT);
           }
           bta_ag_sco_event(other_scb, event);
        } else {
           APPL_TRACE_WARNING("%s: RFC is not up yet", __func__);
        }
    } else {
        APPL_TRACE_WARNING("%s: other_scb is NULL", __func__);
    }
}

void bta_ag_twsp_hfp_result(tBTA_AG_SCB* p_scb, tBTA_AG_API_RESULT* p_result) {
    APPL_TRACE_EVENT("bta_ag_twsp_hfp_result : res = %d", p_result->result);

    switch (p_result->result) {
        case BTA_AG_TWS_QES_RES: {
                bta_ag_send_result(p_scb, BTA_AG_TWS_QES_RES, nullptr, 0);
            } break;
        case BTA_AG_TWS_QMQ_RES: {
                bta_ag_send_result(p_scb, BTA_AG_TWS_QMQ_RES, nullptr, 0);
            } break;
        case BTA_AG_TWS_QBC_RES: {
                bta_ag_send_result(p_scb, BTA_AG_TWS_QBC_RES, nullptr, 0);
           } break;
        case BTA_AG_TWS_QDSP_RES: {
                bta_ag_send_result(p_scb, BTA_AG_TWS_QDSP_RES, nullptr, 0);
           } break;
   }
}

void bta_ag_twsp_sco_event(tBTA_AG_SCB* p_scb, uint8_t event) {
   //PONTING TO SECONDARY sco cb
   tBTA_AG_SCO_CB* p_sco = &bta_ag_cb.twsp_sec_sco;
   tBTA_AG_SCB *other_scb = NULL;
   uint8_t in_state = p_sco->state;
   APPL_TRACE_IMP("%s: SCO Index 0x%04x, State %d, Event %d", __func__,
                     p_scb->sco_idx, p_sco->state, event);

   APPL_TRACE_IMP("%s: TWS+ peer p_scb: %x", __func__, p_scb);

   switch (p_sco->state) {
       case BTA_AG_SCO_SHUTDOWN_ST:
           switch (event) {
              case BTA_AG_SCO_LISTEN_E:
              bta_ag_cb.sec_sm_scb = p_scb;
              /* create sco listen connection */
              p_sco->p_curr_scb = p_scb;

              bta_ag_create_sco(p_scb, false);
              p_sco->state = BTA_AG_SCO_LISTEN_ST;
              break;
       case BTA_AG_SCO_OPEN_E:
              //Honor OPEN request as this could be active
              //call state
              bta_ag_cb.sec_sm_scb = p_scb;
              p_sco->p_curr_scb = p_scb;
              p_sco->state = BTA_AG_SCO_CODEC_ST;
              bta_ag_codec_negotiate(p_scb);
              break;
       default:
              APPL_TRACE_WARNING("%s:BTA_AG_SCO_SHUTDOWN_ST: Ignoring event %d",
                             __func__, event);
       } break;

       case BTA_AG_SCO_LISTEN_ST:
          switch(event) {
              case BTA_AG_SCO_OPEN_E:
                  /* remove listening connection */
                  bta_ag_remove_sco(p_scb, false);
                  p_sco->state = BTA_AG_SCO_CODEC_ST;
                  bta_ag_codec_negotiate(p_scb);
              break;
              case BTA_AG_SCO_SHUTDOWN_E:
                  bta_ag_remove_sco(p_scb, false);
                  p_sco->state = BTA_AG_SCO_SHUTDOWN_ST;
                  bta_ag_cb.sec_sm_scb = NULL;

              break;
              case BTA_AG_SCO_CONN_CLOSE_E:
                  /* sco failed; create sco listen connection */
                  bta_ag_create_sco(p_scb, false);
                  p_sco->state = BTA_AG_SCO_LISTEN_ST;
              break;
              default:
                  APPL_TRACE_WARNING("%s: BTA_AG_SCO_LISTEN_ST: Ignoring event %d",
                             __func__, event);
          } break;
        case BTA_AG_SCO_CODEC_ST:
          switch(event) {
            case BTA_AG_SCO_CLOSE_E:
                /* sco open is not started yet. just go back to listening */
                p_sco->state = BTA_AG_SCO_LISTEN_ST;
                APPL_TRACE_WARNING("%s: perform post sco close action to send indicator", __func__);
                bta_ag_post_sco_close(p_scb, NULL);
                /*call app callback so that btif and app state
                / go back to audio disconnected state*/
                APPL_TRACE_WARNING("%s: SCO close during codec negotiation", __func__);
                bta_ag_cback_sco(p_scb, BTA_AG_AUDIO_CLOSE_EVT);
            break;
            case BTA_AG_SCO_CN_DONE_E:
                if (is_twsp_device(p_scb->peer_addr)) {
                    twsp_select_microphone(get_other_twsp_scb(p_scb->peer_addr), p_scb);
                }
                /* create sco connection to peer */
                if (bta_ag_create_sco(p_scb, true)) {
                    p_sco->state = BTA_AG_SCO_OPENING_ST;
                } else {
                    APPL_TRACE_WARNING("%s: create sco connection failed", __func__);
                    p_sco->state = BTA_AG_SCO_LISTEN_ST;
                    bta_ag_cback_sco(p_scb, BTA_AG_AUDIO_CLOSE_EVT);
                }
            break;
            case BTA_AG_SCO_CONN_CLOSE_E:
                //SCO failed back to LISTEN
                bta_ag_create_sco(p_scb, false);
                p_sco->state = BTA_AG_SCO_LISTEN_ST;
                break;
            case BTA_AG_SCO_SHUTDOWN_E:
                /* remove listening connection */
                bta_ag_remove_sco(p_scb, false);

                if (p_scb == p_sco->p_curr_scb) p_sco->p_curr_scb = NULL;

                p_sco->state = BTA_AG_SCO_SHUTDOWN_ST;
             break;
             default:
                  APPL_TRACE_WARNING("%s: BTA_AG_SCO_CODEC_ST: Ignoring event %d",
                             __func__, event);
        } break;
        case BTA_AG_SCO_OPENING_ST:
          switch(event) {
            case BTA_AG_SCO_CLOSE_E:
                p_sco->state = BTA_AG_SCO_OPEN_CL_ST;
            break;
            case BTA_AG_SCO_SHUTDOWN_E:
                /* If not opening scb, just close it */
                if (p_scb != p_sco->p_curr_scb) {
                    /* remove listening connection */
                    bta_ag_remove_sco(p_scb, false);
                } else
                   p_sco->state = BTA_AG_SCO_SHUTTING_ST;
            break;
            case BTA_AG_SCO_CONN_OPEN_E:
                /*SCO is up!*/
                p_sco->state = BTA_AG_SCO_OPEN_ST;
                other_scb = get_other_twsp_scb((p_scb->peer_addr));
                if (other_scb && twsp_sco_active(other_scb) == false &&
                        get_twsp_state(other_scb) != TWSPLUS_EB_STATE_OUT_OF_EAR &&
                        get_twsp_state(other_scb) != TWSPLUS_EB_STATE_INCASE) {
                    APPL_TRACE_WARNING("Calling SCO open");
                    dispatch_event_primary_peer_device(p_scb, BTA_AG_SCO_OPEN_E);
                }
            break;
            case BTA_AG_SCO_CONN_CLOSE_E:
                /*sco failed, create sco listen connection*/
                bta_ag_create_sco(p_scb, false);
                p_sco->state = BTA_AG_SCO_LISTEN_ST;
                other_scb = get_other_twsp_scb((p_scb->peer_addr));
                if (other_scb && twsp_sco_active(other_scb) == false &&
                        get_twsp_state(other_scb) != TWSPLUS_EB_STATE_OUT_OF_EAR &&
                        get_twsp_state(other_scb) != TWSPLUS_EB_STATE_INCASE) {
                    //Atleast try bringing up the other EB eSCO
                    APPL_TRACE_WARNING("Calling SCO open for other EB");
                    dispatch_event_primary_peer_device(p_scb, BTA_AG_SCO_OPEN_E);
                }
            break;
            default:
                APPL_TRACE_WARNING("%s: BTA_AG_SCO_OPENING_ST: Ignoring event %d",
                             __func__, event);
        } break;
    case BTA_AG_SCO_OPEN_CL_ST:
      switch (event) {
        case BTA_AG_SCO_OPEN_E:
          p_sco->state = BTA_AG_SCO_OPENING_ST;
          break;
        case BTA_AG_SCO_SHUTDOWN_E:
          /* If not opening scb, just close it */
          if (p_scb != p_sco->p_curr_scb) {
            /* remove listening connection */
            bta_ag_remove_sco(p_scb, false);
          } else
            p_sco->state = BTA_AG_SCO_SHUTTING_ST;
          break;
        case BTA_AG_SCO_CONN_OPEN_E:
          /* close sco connection */
          bta_ag_remove_sco(p_scb, true);
          p_sco->state = BTA_AG_SCO_CLOSING_ST;
          break;

        case BTA_AG_SCO_CONN_CLOSE_E:
          /* sco failed; create sco listen connection */
          p_sco->state = BTA_AG_SCO_LISTEN_ST;
          // SCO CLOSE must have lead to this state
          // keep back in Listen state
          break;
        default:
          APPL_TRACE_WARNING("%s: BTA_AG_SCO_OPEN_CL_ST: Ignoring event %d",
                             __func__, event);
          break;
      }
      break;
    case BTA_AG_SCO_OPEN_ST:
      switch (event) {
        case BTA_AG_SCO_CLOSE_E:
          /* close sco connection if active */
          if (bta_ag_remove_sco(p_scb, true)) {
            p_sco->state = BTA_AG_SCO_CLOSING_ST;
          }
          break;

        case BTA_AG_SCO_SHUTDOWN_E:
          /* remove all listening connections */
          bta_ag_remove_sco(p_scb, false);

          /* If SCO was active on this scb, close it */
          if (p_scb == p_sco->p_curr_scb) {
            p_sco->state = BTA_AG_SCO_SHUTTING_ST;
          }
          break;

        case BTA_AG_SCO_CONN_CLOSE_E:
          /* peer closed sco; create sco listen connection */
          bta_ag_create_sco(p_scb, false);
          p_sco->state = BTA_AG_SCO_LISTEN_ST;

          break;
        default:
          APPL_TRACE_WARNING("%s: BTA_AG_SCO_OPEN_ST: Ignoring event %d",
                             __func__, event);
          break;
     }
     break;
     case BTA_AG_SCO_CLOSING_ST:
      switch (event) {
        case BTA_AG_SCO_OPEN_E:
          p_sco->state = BTA_AG_SCO_CLOSE_OP_ST;
          break;

        case BTA_AG_SCO_SHUTDOWN_E:
          /* If not closing scb, just close it */
          if (p_scb != p_sco->p_curr_scb) {
            /* remove listening connection */
            bta_ag_remove_sco(p_scb, false);
          } else
            p_sco->state = BTA_AG_SCO_SHUTTING_ST;

          break;

        case BTA_AG_SCO_CONN_CLOSE_E:
          /* peer closed sco; create sco listen connection */
          bta_ag_create_sco(p_scb, false);
          p_sco->state = BTA_AG_SCO_LISTEN_ST;
          //Closing the other eSCO as well
          APPL_TRACE_WARNING("Calling SCO Close for SSM");
          dispatch_event_primary_peer_device(p_scb, BTA_AG_SCO_CLOSE_E);
          break;
        default:
          APPL_TRACE_WARNING("%s: BTA_AG_SCO_CLOSING_ST: Ignoring event %d",
                             __func__, event);
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
          /* start codec negotiation */
          p_sco->state = BTA_AG_SCO_CODEC_ST;
          bta_ag_codec_negotiate(p_scb);
          break;
        default:
          APPL_TRACE_WARNING("%s: BTA_AG_SCO_CLOSE_OP_ST: Ignoring event %d",
                             __func__, event);
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
            p_sco->state = BTA_AG_SCO_SHUTDOWN_ST;
            bta_ag_cb.sec_sm_scb = NULL;

          /* CHECK: If SCO closed for other HS which is not being disconnected,
             then create listen sco connection for it as scb still open */
          if (bta_ag_scb_open(p_scb)) {
            bta_ag_create_sco(p_scb, false);
            p_sco->state = BTA_AG_SCO_LISTEN_ST;
          }

          if (p_scb == p_sco->p_curr_scb) {
            p_sco->p_curr_scb->sco_idx = BTM_INVALID_SCO_INDEX;
            p_sco->p_curr_scb = NULL;
          }
          break;

        case BTA_AG_SCO_SHUTDOWN_E:
          p_sco->state = BTA_AG_SCO_SHUTDOWN_ST;
          bta_ag_cb.sec_sm_scb = NULL;

          if (p_scb == p_sco->p_curr_scb) {
            p_sco->p_curr_scb->sco_idx = BTM_INVALID_SCO_INDEX;
            p_sco->p_curr_scb = NULL;
          }
          break;

        default:
          APPL_TRACE_WARNING("%s: BTA_AG_SCO_SHUTTING_ST: Ignoring event %d",
                             __func__, event);
          break;
      }
      break;

    default:
      break;
  }
  if (p_sco->state != in_state) {
    APPL_TRACE_IMP("BTA AG TWS SCO State Change: [%s] -> [%s] after Event [%s]",
                     bta_ag_sco_state_str(in_state),
                     bta_ag_sco_state_str(p_sco->state),
                     bta_ag_sco_evt_str(event));
  }
}

void print_bdaddr(const RawAddress& addr) {
    APPL_TRACE_ERROR("%s", addr.ToString().c_str());
}

bool is_twsp_connected() {
    bool ret = false;
    APPL_TRACE_DEBUG("%s: max indicies %d\n", __func__, BTA_AG_MAX_NUM_CLIENTS);
    for (int i=0; i <BTA_AG_MAX_NUM_CLIENTS; i++) {
        APPL_TRACE_DEBUG("%s: %s", __func__,
            bta_ag_cb.scb[i].peer_addr.ToString().c_str());
        if (BTM_SecIsTwsPlusDev(bta_ag_cb.scb[i].peer_addr)) {
           ret = true;
           break;
        }
    }
    APPL_TRACE_DEBUG("%s: returning %d", __func__, ret);
    return ret;
}

bool is_twsp_device(const RawAddress& addr) {
    APPL_TRACE_DEBUG("%s:>", __func__);

    print_bdaddr(addr);
    bool val = BTM_SecIsTwsPlusDev(addr);
    APPL_TRACE_DEBUG("%s: BTM_SecIsTwsPlusDev returning %d", __func__, val);
    return val;
}

bool is_twsp_set(const RawAddress& addr1, const RawAddress& addr2) {
   bool ret = false;
   APPL_TRACE_DEBUG("%s:>", __func__);
   RawAddress p_addr;
   ret = BTM_SecGetTwsPlusPeerDev(addr1, p_addr);
   if (ret) {
       if (addr2 == p_addr) {
           ret = true;
       } else {
           ret = false;
       }
   } else {
      APPL_TRACE_ERROR("%s: Error getting peer bd addr: %d", __func__, ret);
   }
   return ret;
}

bool is_sco_connected_to_twsp() {
   bool ret = false;
   if (bta_ag_cb.sco.state == BTA_AG_SCO_OPEN_ST
         && bta_ag_cb.main_sm_scb != NULL
         && is_twsp_device(bta_ag_cb.main_sm_scb->peer_addr)) {
            //if main SM sco is connected
            //for twsp device
            ret = true;
   } else if (bta_ag_cb.twsp_sec_sco.state == BTA_AG_SCO_OPEN_ST
         && bta_ag_cb.sec_sm_scb != NULL
         && is_twsp_device(bta_ag_cb.sec_sm_scb->peer_addr )) {
            ret = true;
   }
   APPL_TRACE_DEBUG("%s: returns : %d", __func__, ret);
   return ret;
}

bool is_this_twsp_case(const RawAddress& addr) {
    bool ret = false;
    if (is_twsp_device(addr)
          || is_sco_connected_to_twsp()) {
        ret = true;
    }
    APPL_TRACE_DEBUG("%s: returns : %d", __func__, ret);
    return ret;
}

tBTA_AG_SCB* get_sco_connected_scb() {
    tBTA_AG_SCB* ret_scb = NULL;
    if (bta_ag_cb.sco.state == BTA_AG_SCO_OPEN_ST) {
        //if main SM sco is connected
        //for twsp device
        ret_scb = bta_ag_cb.main_sm_scb;
   } else if (bta_ag_cb.twsp_sec_sco.state == BTA_AG_SCO_OPEN_ST) {
        ret_scb = bta_ag_cb.sec_sm_scb;
   }
   APPL_TRACE_DEBUG("%s: returns : %x", __func__, ret_scb);
   return ret_scb;
}

void get_left_eb_addr(RawAddress& eb_addr) {
   APPL_TRACE_ERROR("%s:>", __func__);
   twsp_get_left_eb_addr(eb_addr);
}

void get_right_eb_addr(RawAddress& eb_addr) {
   APPL_TRACE_ERROR("%s:>", __func__);
   twsp_get_right_eb_addr(eb_addr);
}

/*******************************************************************************
 *
 * Function         bta_ag_twsp_sco_conn_rsp
 *
 * Description      Process the SCO connection request
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_ag_twsp_sco_conn_rsp(tBTA_AG_SCB* p_scb,
                         tBTM_ESCO_CONN_REQ_EVT_DATA* p_data) {
  bta_ag_cb.sco.is_local = false;

  APPL_TRACE_DEBUG("%s: eSCO %d, state %d", __func__,
                   controller_get_interface()
                       ->supports_enhanced_setup_synchronous_connection(),
                   bta_ag_cb.sco.state);

  //Just send the Connection rsp, as SCO will be triggered later
  //based on primary SCO state
  //BTM_EScoConnRsp(p_scb->sco_idx, HCI_SUCCESS, &params);
}

void twsp_select_microphone(tBTA_AG_SCB *scb1,
                                        tBTA_AG_SCB *scb2) {

   APPL_TRACE_DEBUG("%s: scb1: %x, scb2: %x", __func__, scb1, scb2);
   tBTA_AG_SCB *best_scb = twsp_get_best_mic_scb();

   tBTA_AG_SCB* other_scb = (best_scb == scb1) ? scb2 : scb1;

   twsp_update_microphone_selection(other_scb, best_scb);

}

void twsp_update_microphone_selection(tBTA_AG_SCB *curr_scb,
                                        tBTA_AG_SCB *selected_scb) {
    RawAddress left_eb_addr;
    RawAddress right_eb_addr;
    APPL_TRACE_DEBUG("%s: curr_pscb: %x, selected_pscb: %x", __func__,
         curr_scb, selected_scb);
    if (selected_scb == NULL) {
        APPL_TRACE_DEBUG("%s: selected scb cant be NULL", __func__);
        return;
    }

   if (twsp_get_left_eb_addr(left_eb_addr) == true) {
       if (twsp_get_right_eb_addr(right_eb_addr) == false) {
          get_peer_twsp_addr(left_eb_addr, right_eb_addr);
       }
    } else {
       if (twsp_get_right_eb_addr(right_eb_addr) == true) {
          get_peer_twsp_addr(right_eb_addr, left_eb_addr);
       } else {
          APPL_TRACE_DEBUG("there are no valid scbs");
          return;
       }
    }

    bta_ag_send_result(selected_scb, BTA_AG_MIC_RES, nullptr, MIC_ENABLE);
    //Have a timer for MIC_PATH_ENABLE_DELAY
    usleep(MIC_PATH_ENABLE_DELAY*1000);
    int  role = get_twsp_role(selected_scb);
    if (role == TWSPLUS_EB_ROLE_INVALID) {
        APPL_TRACE_DEBUG("%s: invalid role, set to default left", __func__);
        role = TWSPLUS_EB_ROLE_LEFT;
    }

    send_twsp_esco_setup(left_eb_addr, right_eb_addr, role);

    if (curr_scb != NULL) {
        bta_ag_send_result(curr_scb, BTA_AG_MIC_RES, nullptr, MIC_DISABLE);
        APPL_TRACE_DEBUG("%s: Disabling Mic path for %x", __func__, curr_scb);
    }
}

/*
 * Callback handle for TWS SCo VS command
 */
void twsp_sco_setup_callback(tBTM_VSC_CMPL *param) {
    unsigned char status = 0;
    unsigned char sub_opcode = 0;
    APPL_TRACE_DEBUG("%s: param_len = %d subopcode = %d status = %d",
         __func__,param->param_len, param->p_param_buf[1], param->p_param_buf[0]);

    if (param->param_len)
    {
       status = param->p_param_buf[0];
    }
    sub_opcode =  param->p_param_buf[1];

    if (status == 0) {
        switch (sub_opcode) {
           case VS_QHCI_TWS_ESCO_SETUP_SUBOPCODE:
               APPL_TRACE_DEBUG("VS_QHCI_TWS_ESCO_SETUP_SUBOPCODE successful");
           break;
        default:
            APPL_TRACE_ERROR("invalid sub_opcode: %x", sub_opcode);
        }
    } else {
        APPL_TRACE_DEBUG("twsp soc vs command failed for subopcode= %d",param->p_param_buf[1]);
    }
}

/* send vendor specific command TWS_eSCO_Setup
 *
 *
 */
void send_twsp_esco_setup (const RawAddress& left_eb_addr, const RawAddress& rght_eb_addr,
    uint8_t selected_mic) {
   uint8_t param[20];
   uint8_t *p_param = param;
   std::string leb, reb;

   APPL_TRACE_DEBUG("%s: left_addr: %s, right_addr: %s, selected_mic: %d", __func__,
           left_eb_addr.ToString().c_str(), rght_eb_addr.ToString().c_str(), selected_mic);

   *p_param++ = VS_QHCI_TWS_ESCO_SETUP_SUBOPCODE;

   REVERSE_ARRAY_TO_STREAM(p_param,left_eb_addr.address,
                                  BD_ADDR_LEN);
   REVERSE_ARRAY_TO_STREAM(p_param,rght_eb_addr.address,
                                  BD_ADDR_LEN);
   *p_param++ = selected_mic;

   BTM_VendorSpecificCommand(VS_QHCI_TWS_ESCO_SETUP_OPCODE,
                              VS_TWS_SCO_SETUP_CMD_LEN,
                               param, twsp_sco_setup_callback);
}

#endif //TWS_AG_ENABLED
