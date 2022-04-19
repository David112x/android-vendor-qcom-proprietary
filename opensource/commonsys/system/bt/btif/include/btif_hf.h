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
 *  Copyright (C) 2016 Google, Inc.
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

#ifndef BTIF_HF_H
#define BTIF_HF_H

#include <stdbool.h>
#include <hardware/bluetooth_headset_interface.h>
#include <bta/include/bta_ag_api.h>

namespace bluetooth {
namespace headset {

Interface* GetInterface();

//#include "bta_ag_api.h"

/* Number of BTIF-HF control blocks */
typedef uint16_t tBTA_AG_PEER_FEAT;

extern uint16_t btif_max_hf_clients;

/* BTIF-HF control block to map bdaddr to BTA handle */
typedef struct _btif_hf_cb {
  uint16_t handle;
  RawAddress connected_bda;
  bthf_connection_state_t state;
  bthf_vr_state_t vr_state;
  tBTA_AG_PEER_FEAT peer_feat;
  int num_active;
  int num_held;
  struct timespec call_end_timestamp;
  struct timespec connected_timestamp;
  bthf_call_state_t call_setup_state;
  bthf_audio_state_t audio_state;
  tBTA_SERVICE_ID service_id;
#if (TWS_AG_ENABLED == TRUE)
  uint8_t twsp_state;
#endif
} btif_hf_cb_t;

extern btif_hf_cb_t btif_hf_cb[BTA_AG_MAX_NUM_CLIENTS];

// Check whether there is a Hands-Free call in progress.
// Returns true if no call is in progress.
bool btif_hf_is_call_idle(void);
bool btif_hf_is_call_vr_idle(void);
bt_status_t btif_hf_execute_service(bool b_enable);
bt_status_t btif_hf_check_if_sco_connected();
bool is_connected(RawAddress* bd_addr);
void btif_in_hf_generic_evt(uint16_t event, char* p_param);


}  // namespace headset
}  // na

#endif /* BTIF_HF_H */
