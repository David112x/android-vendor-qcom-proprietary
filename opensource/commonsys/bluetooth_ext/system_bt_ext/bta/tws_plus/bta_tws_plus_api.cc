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
 *  This is the implementation of the API for TwsPlus subsystem
 *
 ******************************************************************************/

#include <string.h>

#include "bt_common.h"
#include "bta_api.h"
#include "bta_tws_plus_api.h"
#include "bta_tws_plus_int.h"
#include "bta_sys.h"

/*****************************************************************************
 *  Constants
 ****************************************************************************/

static const tBTA_SYS_REG bta_tws_plus_reg = {bta_tws_plus_sm_execute, NULL};

/*******************************************************************************
 *
 * Function         BTA_TwsPlusEnable
 *
 * Description      Enable the tws_plus I/F service. When the enable
 *                  operation is complete the callback function will be
 *                  called with a BTA_TWS_PLUS_ENABLE_EVT. This function must
 *                  be called before other functions in the TwsPlus API are
 *                  called.
 *
 * Returns          BTA_TWS_PLUS_SUCCESS if successful.
 *                  BTA_TWS_PLUS_FAIL if internal failure.
 *
 ******************************************************************************/
tBTA_TWS_PLUS_STATUS BTA_TwsPlusEnable(tBTA_TWS_PLUS_DM_CBACK* p_cback) {
  tBTA_TWS_PLUS_STATUS status = BTA_TWS_PLUS_FAILURE;

  APPL_TRACE_API(__func__);
  if (p_cback && false == bta_sys_is_register(BTA_ID_TWS_PLUS)) {
    memset(&bta_tws_plus_cb, 0, sizeof(tBTA_TWS_PLUS_CB));

    /* register with BTA system manager */
    bta_sys_register(BTA_ID_TWS_PLUS, &bta_tws_plus_reg);

    if (p_cback) {
      tBTA_TWS_PLUS_API_ENABLE* p_buf =
          (tBTA_TWS_PLUS_API_ENABLE*)osi_malloc(sizeof(tBTA_TWS_PLUS_API_ENABLE));
      p_buf->hdr.event = BTA_TWS_PLUS_API_ENABLE_EVT;
      p_buf->p_cback = p_cback;
      bta_sys_sendmsg(p_buf);
      status = BTA_TWS_PLUS_SUCCESS;
    }
  }
  return status;
}

/*******************************************************************************
 *
 * Function         BTA_TwsPlusDisable
 *
 * Description      Enable the tws_plus I/F service. When the enable
 *                  operation is complete the callback function will be
 *                  called with a BTA_TWS_PLUS_ENABLE_EVT. This function must
 *                  be called before other functions in the TwsPlus API are
 *                  called.
 *
 * Returns          BTA_TWS_PLUS_SUCCESS if successful.
 *                  BTA_TWS_PLUS_FAIL if internal failure.
 *
 ******************************************************************************/
tBTA_TWS_PLUS_STATUS BTA_TwsPlusDisable(void) {
  tBTA_TWS_PLUS_STATUS status = BTA_TWS_PLUS_FAILURE;

  APPL_TRACE_API(__func__);
  if (bta_sys_is_register(BTA_ID_TWS_PLUS)) {

    /* de-register with BTA system manager */
    bta_sys_deregister(BTA_ID_TWS_PLUS);
    BT_HDR* p_buf = (BT_HDR*)osi_malloc(sizeof(BT_HDR));
    if(p_buf) {
        p_buf->event = BTA_TWS_PLUS_API_DISABLE_EVT;
        bta_sys_sendmsg(p_buf);
        status = BTA_TWS_PLUS_SUCCESS;
    }
  }
  return status;
}

/*******************************************************************************
 *
 * Function         BTA_TwsPlusSdpSearch
 *
 * Description      This function performs service discovery for a specific
 *                  service on given peer device. When the operation is
 *                  completed the tBTA_TWS_PLUS_DM_CBACK callback function will be
 *                  called with a BTA_TWS_PLUS_SEARCH_COMPLETE_EVT.
 *
 * Returns          BTA_TWS_PLUS_SUCCESS, if the request is being processed.
 *                  BTA_TWS_PLUS_FAILURE, otherwise.
 *
 ******************************************************************************/
tBTA_TWS_PLUS_STATUS BTA_TwsPlusSdpSearch(RawAddress bd_addr) {
  tBTA_TWS_PLUS_API_SDP_SEARCH* p_msg =
      (tBTA_TWS_PLUS_API_SDP_SEARCH*)osi_malloc(sizeof(tBTA_TWS_PLUS_API_SDP_SEARCH));

  APPL_TRACE_API("%s", __func__);

  p_msg->hdr.event = BTA_TWS_PLUS_API_SDP_SEARCH_EVT;
  p_msg->bd_addr = bd_addr;
  bta_sys_sendmsg(p_msg);

  return BTA_TWS_PLUS_SUCCESS;
}

/*******************************************************************************
 *
 * Function         BTA_TwsPlusDeriveLinkKey
 *
 * Description      This function performs service discovery for a specific
 *                  service on given peer device. When the operation is
 *                  completed the tBTA_TWS_PLUS_DM_CBACK callback function will be
 *                  called with a BTA_TWS_PLUS_SEARCH_COMPLETE_EVT.
 *
 * Returns          BTA_TWS_PLUS_SUCCESS, if the request is being processed.
 *                  BTA_TWS_PLUS_FAILURE, otherwise.
 *
 ******************************************************************************/
tBTA_TWS_PLUS_STATUS BTA_TwsPlusDeriveLinkKey(RawAddress eb_addr,
          RawAddress peer_eb_addr, LinkKey key, uint8_t reason) {
  tBTA_TWS_PLUS_API_DERIVE_LINKKEY* p_msg =
      (tBTA_TWS_PLUS_API_DERIVE_LINKKEY*)osi_malloc(sizeof(tBTA_TWS_PLUS_API_DERIVE_LINKKEY));

  APPL_TRACE_API("%s", __func__);

  p_msg->hdr.event = BTA_TWS_PLUS_API_DERIVE_LINK_KEY_EVT;
  p_msg->peer_eb_addr =  peer_eb_addr;
  p_msg->bd_addr = eb_addr;
  p_msg->key = key;
  p_msg->reason = reason;
  bta_sys_sendmsg(p_msg);

  return BTA_TWS_PLUS_SUCCESS;
}

/*******************************************************************************
 *
 * Function         BTA_TwsPlusUpdatePeerEbAddr
 *
 * Description      This function performs service discovery for a specific
 *                  service on given peer device. When the operation is
 *                  completed the tBTA_TWS_PLUS_DM_CBACK callback function will be
 *                  called with a BTA_TWS_PLUS_SEARCH_COMPLETE_EVT.
 *
 * Returns          BTA_TWS_PLUS_SUCCESS, if the request is being processed.
 *                  BTA_TWS_PLUS_FAILURE, otherwise.
 *
 ******************************************************************************/
tBTA_TWS_PLUS_STATUS BTA_TwsPlusUpdatePeerEbAddr(RawAddress eb_addr,
                                                   RawAddress peer_eb_addr) {
  tBTA_TWS_PLUS_API_UPDATE_PEER_EB_ADDR* p_msg =
      (tBTA_TWS_PLUS_API_UPDATE_PEER_EB_ADDR*)osi_malloc(sizeof(tBTA_TWS_PLUS_API_UPDATE_PEER_EB_ADDR));

  APPL_TRACE_API("%s", __func__);

  p_msg->hdr.event = BTA_TWS_PLUS_API_UPDATE_PEER_EB_ADDR_EVT;
  p_msg->peer_eb_addr = peer_eb_addr;
  p_msg->bd_addr = eb_addr;
  bta_sys_sendmsg(p_msg);

  return BTA_TWS_PLUS_SUCCESS;
}