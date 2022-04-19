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
 *  Copyright (C) 2015 The Android Open Source Project
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
 *  This is the public interface file for the BTA TWS_PLUS I/F
 *
 ******************************************************************************/
#ifndef BTA_TWS_PLUS_API_H
#define BTA_TWS_PLUS_API_H

#include <hardware/bt_sdp.h>

#include "bt_target.h"
#include "bt_types.h"
#include "bta_api.h"
#include "btm_api.h"

/* status values */
#define BTA_TWS_PLUS_SUCCESS 0 /* Successful operation. */
#define BTA_TWS_PLUS_FAILURE 1 /* Generic failure. */
#define BTA_TWS_PLUS_BUSY 2    /* Temporarily can not handle this request. */

typedef uint8_t tBTA_TWS_PLUS_STATUS;

/* SDP I/F callback events */
/* events received by tBTA_TWS_PLUS_DM_CBACK */
#define BTA_TWS_PLUS_ENABLE_EVT 0             /* SDP service i/f enabled */
#define BTA_TWS_PLUS_DISABLE_EVT 1            /* SDP service i/f disabled */
#define BTA_TWS_PLUS_SDP_SEARCH_EVT 2         /* TwsPlus SDP Service started */
#define BTA_TWS_PLUS_SDP_SEARCH_COMP_EVT 3    /* TwsPlus SDP search complete */
#define BTA_TWS_PLUS_LK_DERIVED_EVT 4         /* TwsPlus LK derived for 2nd EB */
#define BTA_TWS_PLUS_MAX_EVT 5                /* max number of SDP events */

typedef uint16_t tBTA_TWS_PLUS_EVT;

/* data associated with tBTA_TWS_PLUS_LK_DERIVED_EVT */
typedef struct {
  tBTA_TWS_PLUS_STATUS status;
  RawAddress bd_addr;
  RawAddress  peer_eb_addr;
  LinkKey key;                /* Link key associated with peer device. */
  uint8_t reason;
} tBTA_TWS_PLUS_LK_DERIVED;

typedef struct {
  tBTA_TWS_PLUS_STATUS status;
  RawAddress eb_addr;
  RawAddress peer_eb_addr;

} tBTA_TWS_PLUS_SDP_SEARCH_COMP;

typedef union {
  tBTA_TWS_PLUS_STATUS status;                   /* BTA_TWS_PLUS_SEARCH_EVT */
  tBTA_TWS_PLUS_SDP_SEARCH_COMP sdp_search_comp; /* BTA_TWS_PLUS_SEARCH_COMP_EVT */
  tBTA_TWS_PLUS_LK_DERIVED lk_derived;           /* BTA_TWS_PLUS_LK_DERIVED_EVT */
} tBTA_TWS_PLUS;

/* SDP DM Interface callback */
typedef void(tBTA_TWS_PLUS_DM_CBACK)(tBTA_TWS_PLUS_EVT event, tBTA_TWS_PLUS* p_data);

/* TWS_PLUS configuration structure */
typedef struct {
  uint16_t sdp_db_size;        /* The size of p_sdp_db */
  tSDP_DISCOVERY_DB* p_sdp_db; /* The data buffer to keep SDP database */
} tBTA_TWS_PLUS_SDP_CFG;

/*******************************************************************************
 *
 * Function         BTA_TwsPlusEnable
 *
 * Description      Enable the SDP I/F service. When the enable
 *                  operation is complete the callback function will be
 *                  called with a BTA_TWS_PLUS_ENABLE_EVT. This function must
 *                  be called before other functions in the TWS_PLUS API are
 *                  called.
 *
 * Returns          BTA_TWS_PLUS_SUCCESS if successful.
 *                  BTA_TWS_PLUS_FAIL if internal failure.
 *
 ******************************************************************************/
extern tBTA_TWS_PLUS_STATUS BTA_TwsPlusEnable(tBTA_TWS_PLUS_DM_CBACK* p_cback);

/*******************************************************************************
 *
 * Function         BTA_TwsPlusDisable
 *
 * Description      Enable the SDP I/F service. When the enable
 *                  operation is complete the callback function will be
 *                  called with a BTA_TWS_PLUS_DISABLE_EVT. This function must
 *                  be called before other functions in the TWS_PLUS API are
 *                  called.
 *
 * Returns          BTA_TWS_PLUS_SUCCESS if successful.
 *                  BTA_TWS_PLUS_FAIL if internal failure.
 *
 ******************************************************************************/
extern tBTA_TWS_PLUS_STATUS BTA_TwsPlusDisable(void);

/*******************************************************************************
 *
 * Function         BTA_TwsPlusSdpSearch
 *
 * Description      Start a search for sdp records for a specific RawAddress with a
 *                  specific profile uuid.
 *                  When the search operation is completed, the callback
 *                  function will be called with a BTA_TWS_PLUS_SEARCH_EVT.
 * Returns          BTA_TWS_PLUS_SUCCESS if successful.
 *                  BTA_TWS_PLUS_FAIL if internal failure.
 *
 ******************************************************************************/
extern tBTA_TWS_PLUS_STATUS BTA_TwsPlusSdpSearch(RawAddress bd_addr);

extern tBTA_TWS_PLUS_STATUS BTA_TwsPlusDeriveLinkKey(RawAddress eb_addr,
                    RawAddress peer_eb_addr, LinkKey key, uint8_t reason);

extern tBTA_TWS_PLUS_STATUS BTA_TwsPlusUpdatePeerEbAddr(RawAddress eb_addr,
                                                   RawAddress peer_eb_addr);
extern tBTA_TWS_PLUS_STATUS BTA_TwsPlusDisable(void);

#endif /* BTA_TWS_PLUS_API_H */
