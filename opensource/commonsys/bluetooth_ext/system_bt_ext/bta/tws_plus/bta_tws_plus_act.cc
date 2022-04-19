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
 *  This file contains action functions for SDP search.
 ******************************************************************************/

#include <arpa/inet.h>
#include <hardware/bluetooth.h>
#include <hardware/bt_sdp.h>
#include <stdlib.h>
#include <string.h>

#include "bt_common.h"
#include "bt_types.h"
#include "bta_api.h"
#include "bta_tws_plus_api.h"
#include "bta_tws_plus_int.h"
#include "bta_sys.h"
#include "btm_api.h"
#include "osi/include/allocator.h"
#include "sdp_api.h"
#include "utl.h"

/*****************************************************************************
 *  Constants
 ****************************************************************************/

 #ifndef BTA_TWS_PLUS_SDP_DB_SIZE
#define BTA_TWS_PLUS_SDP_DB_SIZE 500
#endif

static uint8_t __attribute__((aligned(4))) bta_tws_plus_sdp_db_data[BTA_TWS_PLUS_SDP_DB_SIZE];

/* TwsPlus SDP configuration structure */
const tBTA_TWS_PLUS_SDP_CFG bta_tws_plus_sdp_cfg = {
    BTA_TWS_PLUS_SDP_DB_SIZE,
    (tSDP_DISCOVERY_DB*)
        bta_tws_plus_sdp_db_data /* The data buffer to keep SDP database */
};

tBTA_TWS_PLUS_SDP_CFG* p_bta_tws_plus_sdp_cfg = (tBTA_TWS_PLUS_SDP_CFG*)&bta_tws_plus_sdp_cfg;

static const uint8_t UUID_TWS_PLUS_SINK[] = {0xea, 0xb1, 0x09, 0x38, 0x4c, 0xd7,
                                        0x4f, 0x9d, 0xbc, 0x56, 0xde, 0x3b,
                                        0xac, 0x83, 0x8a, 0x40};
static const uint8_t UUID_TWS_PLUS_SRC[] = {0x3c, 0xa5, 0xf1, 0x6f, 0x1b, 0xeb,
                                        0x4f, 0x53, 0x97, 0xae, 0x6d, 0xd2,
                                        0xc5, 0x65, 0x0c, 0x93};
static const uint8_t UUID_SERVCLASS_TWS_PLUS[] = {0x64, 0x02, 0x02, 0x2b, 0x30, 0xf3,
                                        0x43, 0xfe, 0xba, 0x40, 0xe5, 0x1f,
                                        0xa2, 0x76, 0x6c, 0xb9};

#define UUID_MAX_LENGTH 16
#define IS_UUID(u1, u2) !memcmp(u1, u2, UUID_MAX_LENGTH)

static bool bta_tws_plus_get_str_attr(tSDP_DISC_REC* p_rec, uint16_t attr_id,
                       RawAddress& addr);
bool TWS_Plus_AddRecord(void);

bool TWS_Plus_AddRecord(void) {
  bool result = false;
  uint32_t sdp_handle = 0;

  APPL_TRACE_DEBUG("%s() -", __func__);

  bta_tws_plus_cb.sdp_tws_plus_handle = sdp_handle = SDP_CreateRecord();
  if(sdp_handle) {
      /* add service class id list */
      result = SDP_AddServiceClassIdListUuid128(sdp_handle,
                     (uint8_t * ) UUID_TWS_PLUS_SRC);
      APPL_TRACE_DEBUG("%s() - result %d ", __func__, result);
      /* add profile descriptor list   */
      result &= SDP_AddProfileDescriptorListUuid128(
        sdp_handle, (uint8_t * ) UUID_SERVCLASS_TWS_PLUS, TWS_VERSION);
      APPL_TRACE_DEBUG("%s() - result %d ", __func__, result);
  }
  return result;
}

static bool bta_tws_plus_get_str_attr(tSDP_DISC_REC* p_rec, uint16_t attr_id,
                       RawAddress& addr) {
  tSDP_DISC_ATTR* p_attr, *p_sattr;
  bool status = true;

  p_attr = SDP_FindAttributeInRec(p_rec, attr_id);
  if ((p_attr != NULL) &&
     (SDP_DISC_ATTR_TYPE(p_attr->attr_len_type) == DATA_ELE_SEQ_DESC_TYPE)){
    APPL_TRACE_DEBUG("%s() - EB addr attribute found ", __func__);

      for (p_sattr = p_attr->attr_value.v.p_sub_attr; p_sattr;
           p_sattr = p_sattr->p_next_attr) {
        APPL_TRACE_DEBUG("%s() attr type %d", __func__, SDP_DISC_ATTR_TYPE(p_sattr->attr_len_type));
        APPL_TRACE_DEBUG("%s() attr len type %d", __func__, SDP_DISC_ATTR_LEN(p_sattr->attr_len_type));
        if (SDP_DISC_ATTR_TYPE(p_sattr->attr_len_type) ==  UINT_DESC_TYPE) {
          if (SDP_DISC_ATTR_LEN(p_sattr->attr_len_type) == 1) {
            addr.address[3] = p_sattr->attr_value.v.u8;
            APPL_TRACE_DEBUG("%s() - EB addr UAP copied", __func__);
          } else if (SDP_DISC_ATTR_LEN(p_sattr->attr_len_type) == 2) {
            APPL_TRACE_DEBUG("%s() - EB addr NAP copied", __func__);
            memcpy(&addr.address[4], &p_sattr->attr_value.v.u16, 2);
          } else if (SDP_DISC_ATTR_LEN(p_sattr->attr_len_type) ==
                     4) {
            APPL_TRACE_DEBUG("%s() - EB addr LAP copied", __func__);
            memcpy(&addr.address[0], &p_sattr->attr_value.v.u32, 3);
          }
        }
     }
  }

  return status;
}

/*******************************************************************************
 *
 * Function     bta_tws_plus_search_cback
 *
 * Description  Callback from btm after search is completed
 *
 * Returns      void
 *
 ******************************************************************************/
static void bta_tws_plus_search_cback(uint16_t result, void* user_data) {
  tSDP_DISC_REC* p_rec = NULL;
  tBTA_TWS_PLUS evt_data;
  tBTA_TWS_PLUS_STATUS status = BTA_TWS_PLUS_FAILURE;
  bluetooth::Uuid& uuid = *(reinterpret_cast<bluetooth::Uuid*>(user_data));
  APPL_TRACE_DEBUG("%s() -  res: 0x%x", __func__, result);

  memset(&evt_data, 0, sizeof(evt_data));
  bta_tws_plus_cb.sdp_active = BTA_TWS_PLUS_SDP_ACTIVE_NONE;

  if (bta_tws_plus_cb.p_dm_cback == NULL) return;

  evt_data.sdp_search_comp.eb_addr = bta_tws_plus_cb.remote_addr;

  if (result == SDP_SUCCESS || result == SDP_DB_FULL) {
      p_rec = SDP_FindServiceUUIDInDb(p_bta_tws_plus_sdp_cfg->p_sdp_db, uuid, p_rec);
      /* generate the matching record data pointer */
      if ((p_rec != NULL) && ( uuid == bluetooth::Uuid::From128BitBE(UUID_TWS_PLUS_SINK))) {
        if(bta_tws_plus_get_str_attr(p_rec, ATTR_ID_TWS_PLUS_LINKED_EARBUD,
                           evt_data.sdp_search_comp.peer_eb_addr)) {
            status = BTA_TWS_PLUS_SUCCESS;
        }
        APPL_TRACE_DEBUG("%s() - found TWS_PLUS SINK uuid", __func__);
      } else {
        APPL_TRACE_DEBUG("%s() - UUID not found", __func__);
      }
      evt_data.sdp_search_comp.status = status;

      bta_tws_plus_cb.p_dm_cback(BTA_TWS_PLUS_SDP_SEARCH_COMP_EVT, &evt_data);
      osi_free(user_data);  // We no longer need the user data to track the search
  }
}

/*******************************************************************************
 *
 * Function     bta_tws_plus_enable
 *
 * Description  Initializes the SDP I/F
 *
 * Returns      void
 *
 ******************************************************************************/
void bta_tws_plus_enable(tBTA_TWS_PLUS_MSG* p_data) {
  APPL_TRACE_DEBUG("%s in, sdp_active:%d", __func__, bta_tws_plus_cb.sdp_active);
  tBTA_TWS_PLUS evt_data;
  evt_data.status = BTA_TWS_PLUS_SUCCESS;
  bta_tws_plus_cb.p_dm_cback = p_data->enable.p_cback;
  if(!TWS_Plus_AddRecord()) {
    evt_data.status = BTA_TWS_PLUS_FAILURE;
  }
  if(bta_tws_plus_cb.p_dm_cback)  {
    bta_tws_plus_cb.p_dm_cback(BTA_TWS_PLUS_ENABLE_EVT, &evt_data);
  }
}

/*******************************************************************************
 *
 * Function     bta_tws_plus_disable
 *
 * Description  Initializes the SDP I/F
 *
 * Returns      void
 *
 ******************************************************************************/
void bta_tws_plus_disable(tBTA_TWS_PLUS_MSG* p_data) {
  APPL_TRACE_DEBUG("%s in, sdp_active:%d", __func__, bta_tws_plus_cb.sdp_active);

  if(bta_tws_plus_cb.sdp_active == BTA_TWS_PLUS_SDP_ACTIVE_YES)  {
    SDP_CancelServiceSearch(p_bta_tws_plus_sdp_cfg->p_sdp_db);
  }

  if(bta_tws_plus_cb.sdp_tws_plus_handle) {
    SDP_DeleteRecord(bta_tws_plus_cb.sdp_tws_plus_handle);
    bta_tws_plus_cb.sdp_tws_plus_handle = 0;
  }
  memset(&bta_tws_plus_cb, 0, sizeof(bta_tws_plus_cb));
}

/*******************************************************************************
 *
 * Function     bta_tws_plus_sdp_search
 *
 * Description  Discovers all sdp records for an uuid on remote device
 *
 * Returns      void
 *
 ******************************************************************************/
void bta_tws_plus_sdp_search(tBTA_TWS_PLUS_MSG* p_data) {
  if (p_data == NULL) {
    APPL_TRACE_DEBUG("SDP control block handle is null");
    return;
  }
  tBTA_TWS_PLUS_STATUS status = BTA_TWS_PLUS_FAILURE;

  APPL_TRACE_DEBUG("%s in, sdp_active:%d", __func__, bta_tws_plus_cb.sdp_active);

  if (bta_tws_plus_cb.sdp_active != BTA_TWS_PLUS_SDP_ACTIVE_NONE) {
    /* SDP is still in progress */
    status = BTA_TWS_PLUS_BUSY;
    if (bta_tws_plus_cb.p_dm_cback) {
      tBTA_TWS_PLUS result;
      memset(&result, 0, sizeof(result));
      result.sdp_search_comp.eb_addr = p_data->sdp_search.bd_addr;
      result.sdp_search_comp.status = status;
      bta_tws_plus_cb.p_dm_cback(BTA_TWS_PLUS_SDP_SEARCH_COMP_EVT, &result);
    }
    return;
  }

  bta_tws_plus_cb.sdp_active = BTA_TWS_PLUS_SDP_ACTIVE_YES;
  bta_tws_plus_cb.remote_addr = p_data->sdp_search.bd_addr;
  /* set the uuid used in the search */
  bluetooth::Uuid *bta_tws_plus_search_uuid =  (bluetooth::Uuid *)osi_malloc(sizeof(bluetooth::Uuid));
  *bta_tws_plus_search_uuid = bluetooth::Uuid::From128BitBE(UUID_TWS_PLUS_SINK);

  /* initialize the search for the uuid */
  SDP_InitDiscoveryDb(p_bta_tws_plus_sdp_cfg->p_sdp_db, p_bta_tws_plus_sdp_cfg->sdp_db_size, 1,
                      bta_tws_plus_search_uuid, 0, NULL);

  if (!SDP_ServiceSearchAttributeRequest2(
          p_data->sdp_search.bd_addr, p_bta_tws_plus_sdp_cfg->p_sdp_db,
          bta_tws_plus_search_cback, (void*) bta_tws_plus_search_uuid)) {
    bta_tws_plus_cb.sdp_active = BTA_TWS_PLUS_SDP_ACTIVE_NONE;

    /* failed to start SDP. report the failure right away */
    if (bta_tws_plus_cb.p_dm_cback) {
      tBTA_TWS_PLUS result;
      memset(&result, 0, sizeof(result));
      result.sdp_search_comp.eb_addr =  p_data->sdp_search.bd_addr;
      result.sdp_search_comp.status = status;
      bta_tws_plus_cb.p_dm_cback(BTA_TWS_PLUS_SDP_SEARCH_COMP_EVT, &result);
    }
  }
  /*
  else report the result when the cback is called
  */
}

/*******************************************************************************
 *
 * Function     bta_tws_plus_derive_linkkey
 *
 * Description
 *
 * Returns      void
 *
 ******************************************************************************/
void bta_tws_plus_derive_linkkey(tBTA_TWS_PLUS_MSG* p_data) {
  tBTA_TWS_PLUS_STATUS status = BTA_TWS_PLUS_FAILURE;
  tBTA_TWS_PLUS result;
  memset(&result, 0, sizeof(result));
  APPL_TRACE_DEBUG("%s in, sdp_active:%d", __func__, bta_tws_plus_cb.sdp_active);

  int i = 0;
  result.lk_derived.key = SMP_DeriveBrEdrLinkKey(p_data->derive_lk.peer_eb_addr,
       p_data->derive_lk.key);

  APPL_TRACE_DEBUG("%s link key for 2nd device derived", __func__);
  for(i = 0; i< 16 ; i++)
    APPL_TRACE_DEBUG("%s result.key [%d] is %d", __func__, i, result.lk_derived.key[i]);
  status = BTA_TWS_PLUS_SUCCESS;

  if (bta_tws_plus_cb.p_dm_cback) {
    result.lk_derived.peer_eb_addr =  p_data->derive_lk.peer_eb_addr;
    result.lk_derived.bd_addr = p_data->derive_lk.bd_addr;
    result.lk_derived.status = status;
    result.lk_derived.reason = p_data->derive_lk.reason;
    bta_tws_plus_cb.p_dm_cback(BTA_TWS_PLUS_LK_DERIVED_EVT, &result);
  }
}

/*******************************************************************************
 *
 * Function     bta_tws_plus_update_peer_eb_addr
 *
 * Description
 *
 * Returns      void
 *
 ******************************************************************************/
void bta_tws_plus_update_peer_eb_addr(tBTA_TWS_PLUS_MSG* p_data) {

  BTM_SecUpdateTwsPeerAddr(p_data->update_peer_eb.bd_addr,
                    p_data->update_peer_eb.peer_eb_addr);
}

