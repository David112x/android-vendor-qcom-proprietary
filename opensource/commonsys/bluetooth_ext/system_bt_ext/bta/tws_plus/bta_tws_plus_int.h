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
 *  This is the private interface file for the BTA TWS_PLUS I/F
 *
 ******************************************************************************/
#ifndef BTA_TWS_PLUS_INT_H
#define BTA_TWS_PLUS_INT_H

#include "bta_api.h"
#include "bta_tws_plus_api.h"
#include "bta_sys.h"

/*****************************************************************************
 *  Constants
 ****************************************************************************/

#define ATTR_ID_TWS_PLUS_LINKED_EARBUD 0x0200
#define TWS_VERSION 0x0500

enum {
  /* these events are handled by the state machine */
  BTA_TWS_PLUS_API_ENABLE_EVT = BTA_SYS_EVT_START(BTA_ID_TWS_PLUS),
  BTA_TWS_PLUS_API_DISABLE_EVT,
  BTA_TWS_PLUS_API_SDP_SEARCH_EVT,
  BTA_TWS_PLUS_API_DERIVE_LINK_KEY_EVT,
  BTA_TWS_PLUS_API_UPDATE_PEER_EB_ADDR_EVT,
  BTA_TWS_PLUS_MAX_INT_EVT
};

enum {
  BTA_TWS_PLUS_SDP_ACTIVE_NONE = 0,
  BTA_TWS_PLUS_SDP_ACTIVE_YES /* waiting for SDP result */
};

/* data type for BTA_TWS_PLUS_API_ENABLE_EVT */
typedef struct {
  BT_HDR hdr;
  tBTA_TWS_PLUS_DM_CBACK* p_cback;
} tBTA_TWS_PLUS_API_ENABLE;

/* data type for BTA_TWS_PLUS_API_SDP_SEARCH_EVT */
typedef struct {
  BT_HDR hdr;
  RawAddress bd_addr;
} tBTA_TWS_PLUS_API_SDP_SEARCH;

/* data type for tBTA_TWS_PLUS_LK_DERIVED_EVT */
typedef struct {
  BT_HDR hdr;
  RawAddress bd_addr;
  RawAddress peer_eb_addr;        /* peer eb addr for which lk has to derived */
  LinkKey key;                /* Link key associated with peer device. */
  uint8_t reason;
} tBTA_TWS_PLUS_API_DERIVE_LINKKEY;

/* data type for BTA_TWS_PLUS_API_UPDATE_PEER_EB_ADDR_EVT */
typedef struct {
  BT_HDR hdr;
  RawAddress bd_addr;
  RawAddress peer_eb_addr;
} tBTA_TWS_PLUS_API_UPDATE_PEER_EB_ADDR;

/* union of all data types */
typedef union {
  BT_HDR hdr;
  tBTA_TWS_PLUS_API_ENABLE enable;
  tBTA_TWS_PLUS_API_SDP_SEARCH sdp_search;
  tBTA_TWS_PLUS_API_DERIVE_LINKKEY derive_lk;
  tBTA_TWS_PLUS_API_UPDATE_PEER_EB_ADDR update_peer_eb;
} tBTA_TWS_PLUS_MSG;

/* SDP control block */
typedef struct {
  uint32_t sdp_tws_plus_handle;     /* SDP record handle for TWS+ */
  uint8_t sdp_active;               /* see BTA_TWS_PLUS_SDP_ACT_* */
  RawAddress remote_addr;
  tBTA_TWS_PLUS_DM_CBACK* p_dm_cback;
} tBTA_TWS_PLUS_CB;

/* SDP control block */
extern tBTA_TWS_PLUS_CB bta_tws_plus_cb;

/* config struct */
extern tBTA_TWS_PLUS_SDP_CFG* p_bta_tws_plus_sdp_cfg;

extern bool bta_tws_plus_sm_execute(BT_HDR* p_msg);

extern void bta_tws_plus_enable(tBTA_TWS_PLUS_MSG* p_data);
extern void bta_tws_plus_disable(tBTA_TWS_PLUS_MSG* p_data);
extern void bta_tws_plus_sdp_search(tBTA_TWS_PLUS_MSG* p_data);
extern void bta_tws_plus_derive_linkkey(tBTA_TWS_PLUS_MSG* p_data);
extern void bta_tws_plus_update_peer_eb_addr(tBTA_TWS_PLUS_MSG* p_data);
#endif /* BTA_TWS_PLUS_INT_H */
