/*
 * Copyright (C) 2017, The Linux Foundation. All rights reserved.
 * Not a Contribution.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the
 * disclaimer below) provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *
 * * Neither the name of The Linux Foundation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
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
 */

/******************************************************************************
 *
 *  Copyright (C) 2014 The Android Open Source Project

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
 *  This is the main implementation file for the BTA TWS_PLUS I/F
 *
 ******************************************************************************/

#include <stdlib.h>

#include "bta_api.h"
#include "bta_tws_plus_api.h"
#include "bta_tws_plus_int.h"
#include "bta_sys.h"

/*****************************************************************************
 * Constants and types
 ****************************************************************************/

tBTA_TWS_PLUS_CB bta_tws_plus_cb;

/* state machine action enumeration list */
#define BTA_TWS_PLUS_NUM_ACTIONS (BTA_TWS_PLUS_MAX_INT_EVT & 0x00ff)

/* type for action functions */
typedef void (*tBTA_TWS_PLUS_ACTION)(tBTA_TWS_PLUS_MSG* p_data);

/* action function list */
const tBTA_TWS_PLUS_ACTION bta_tws_plus_action[] = {
    bta_tws_plus_enable,            /* BTA_TWS_PLUS_API_ENABLE_EVT */
    bta_tws_plus_disable,            /* BTA_TWS_PLUS_API_DISABLE_EVT */
    bta_tws_plus_sdp_search,        /* BTA_TWS_PLUS_API_SDP_SEARCH_EVT */
    bta_tws_plus_derive_linkkey,    /* BTA_TWS_PLUS_API_DERIVE_LINK_KEY_EVT */
    bta_tws_plus_update_peer_eb_addr /* BTA_TWS_PLUS_API_UPDATE_PEER_EB_ADDR_EVT */
};

/*******************************************************************************
 * Function         bta_tws_plus_sm_execute
 *
 * Description      State machine event handling function for SDP search
 *
 * Returns          void
 ******************************************************************************/
bool bta_tws_plus_sm_execute(BT_HDR* p_msg) {
  if (p_msg == NULL) return false;

  bool ret = false;
  uint16_t action = (p_msg->event & 0x00ff);

  /* execute action functions */
  if (action < BTA_TWS_PLUS_NUM_ACTIONS) {
    (*bta_tws_plus_action[action])((tBTA_TWS_PLUS_MSG*)p_msg);
    ret = true;
  }

  return (ret);
}
