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
 */

#include <hardware/bluetooth.h>
#include "btif_common.h"
#include "btif_hf.h"
#include "btif_tws_plus.h"
#include "btif_twsp_hf.h"
#include "vendor.h"
#include <cutils/properties.h>


#if (TWS_AG_ENABLED == TRUE)
//#define TEST_TWSP 1


namespace bluetooth {
namespace headset {

bool is_twsp_device_connected() {
#ifdef TEST_TWSP
    return true;
#else
    int i;
    for (i = 0; i < btif_max_hf_clients; ++i) {
        if (((btif_hf_cb[i].state == BTHF_CONNECTION_STATE_CONNECTED) ||
            (btif_hf_cb[i].state == BTHF_CONNECTION_STATE_SLC_CONNECTED)) &&
            btif_is_tws_plus_device(&btif_hf_cb[i].connected_bda))
            return true;
  }
  return false;
#endif
}

int get_idx_primary_eb(RawAddress* bd_addr) {
   static char primary[PROPERTY_VALUE_MAX];
   int val = 0;
   int idx = -1;
   if (property_get("persist.bt.tws.hf.primary", primary, NULL)) {
      val = atoi(primary);
      BTIF_TRACE_DEBUG("%s: primary eb value : %d", __func__, val);
   }

#ifdef TEST_TWSP
   return 0;
#else
   int i, type;
   bool both_eb_connected = true;
   for (i = 0; i < btif_max_hf_clients; ++i) {
        BTIF_TRACE_DEBUG("%s: i: %d", __func__,i);
        if (btif_is_tws_plus_device(&btif_hf_cb[i].connected_bda)) {
            btif_tws_plus_get_dev_type(&btif_hf_cb[i].connected_bda, &type);
            if (type == TWS_PLUS_DEV_TYPE_PRIMARY) {
                idx = i;
            }
            if (!is_connected(&btif_hf_cb[i].connected_bda)) {
                both_eb_connected = false;
            }
        } else {
            both_eb_connected = false;
        }
  }
  BTIF_TRACE_DEBUG("%s: idx: %d, both_eb_connected: %d", __func__, idx, both_eb_connected);
  if (both_eb_connected)
      idx = 0;
#endif
  BTIF_TRACE_DEBUG("%s returning : idx = %d", __func__,idx);
  return idx;
}

/*******************************************************************************
**
** Function         btif_hf_get_other_connected_twsp_index
**
** Description      Returns for other connected TWSP index
**
** Returns          Other connected index
**
*******************************************************************************/

int btif_hf_get_other_connected_twsp_index(int current_index)
{
    int i;
    for (i = 0; i < btif_max_hf_clients; i++)
    {
        if (i != current_index)
        {
            if (btif_is_tws_plus_device(&btif_hf_cb[i].connected_bda) &&
                 ((btif_hf_cb[i].state == BTHF_CONNECTION_STATE_CONNECTED) ||
                 (btif_hf_cb[i].state == BTHF_CONNECTION_STATE_SLC_CONNECTED)))
                return i;
        }
    }
    return btif_max_hf_clients;
}

/*******************************************************************************
**
** Function         btif_hf_twsp_send_bvra_update
**
** Description      Send BVRA update for connected TWS+ peer earbud
**
** Returns          void
**
*******************************************************************************/

void btif_hf_twsp_send_bvra_update(int current_index, tBTA_AG_RES_DATA* ag_res) {
    int peer_eb_idx = btif_hf_get_other_connected_twsp_index(current_index);
    if ((peer_eb_idx >= 0) && (peer_eb_idx < btif_max_hf_clients) &&
        (btif_hf_cb[peer_eb_idx].peer_feat & BTA_AG_PEER_FEAT_VREC)) {
        BTIF_TRACE_DEBUG("%s: Send BVRA update for peer earbud, idx: %d", __func__, peer_eb_idx);
        BTA_AgResult(btif_hf_cb[peer_eb_idx].handle, BTA_AG_BVRA_RES, ag_res);
    }
}

bool btif_hf_check_twsp_state_for_sco(int idx) {
    BTIF_TRACE_DEBUG("%s: Current earbud state: %d", __func__, btif_hf_cb[idx].twsp_state);
    if (btif_hf_cb[idx].twsp_state == TWSPLUS_EB_STATE_OUT_OF_EAR ||
        btif_hf_cb[idx].twsp_state == TWSPLUS_EB_STATE_INCASE) {
        int peer_eb_idx = btif_hf_get_other_connected_twsp_index(idx);
        if ((peer_eb_idx < 0) || (peer_eb_idx >= btif_max_hf_clients) ||
            (btif_hf_cb[peer_eb_idx].twsp_state == TWSPLUS_EB_STATE_OUT_OF_EAR) ||
            (btif_hf_cb[peer_eb_idx].twsp_state == TWSPLUS_EB_STATE_INCASE)) {
            BTIF_TRACE_DEBUG("%s: No any connected earbud in ear", __func__);
            btif_transfer_context(btif_in_hf_generic_evt, BTIF_HFP_CB_AUDIO_CONNECTING,
                            (char*)(&btif_hf_cb[idx].connected_bda), sizeof(RawAddress), NULL);
            btif_transfer_context(btif_in_hf_generic_evt, BTIF_HFP_CB_AUDIO_DISCONNECTED,
                            (char*)(&btif_hf_cb[idx].connected_bda), sizeof(RawAddress), NULL);
        }
        return false;
    }
    return true;
}

}  // namespace headset
}  // namespace bluetooth

#endif//#if (TWS_AG_ENABLED == TRUE)

