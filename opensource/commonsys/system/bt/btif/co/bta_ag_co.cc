/******************************************************************************
 *
 *  Copyright (C) 2009-2012 Broadcom Corporation
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

#define LOG_TAG "bt_btif_bta_ag"

#include "bta/include/bta_ag_co.h"
#include "bta/ag/bta_ag_int.h"
#include "bta/include/bta_ag_api.h"
#include "bta/include/bta_ag_ci.h"
#include "osi/include/osi.h"

/*******************************************************************************
 *
 * Function         bta_ag_co_init
 *
 * Description      This callout function is executed by AG when it is
 *                  started by calling BTA_AgEnable().  This function can be
 *                  used by the phone to initialize audio paths or for other
 *                  initialization purposes.
 *
 *
 * Returns          Void.
 *
 ******************************************************************************/
void bta_ag_co_init(void) { BTM_WriteVoiceSettings(AG_VOICE_SETTINGS); }

/*******************************************************************************
 *
 * Function         bta_ag_co_data_open
 *
 * Description      This function is executed by AG when a service level
 *                  connection is opened.  The phone can use this function to
 *                  set up data paths or perform any required initialization or
 *                  set up particular to the connected service.
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_ag_co_data_open(uint16_t handle, tBTA_SERVICE_ID service) {
  BTIF_TRACE_DEBUG("bta_ag_co_data_open handle:%d service:%d", handle, service);
}

/*******************************************************************************
 *
 * Function         bta_ag_co_data_close
 *
 * Description      This function is called by AG when a service level
 *                  connection is closed
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_ag_co_data_close(uint16_t handle) {
  BTIF_TRACE_DEBUG("bta_ag_co_data_close handle:%d", handle);
}

/*******************************************************************************
 **
 ** Function         bta_ag_co_tx_write
 **
 ** Description      This function is called by the AG to send data to the
 **                  phone when the AG is configured for AT command
 **                  pass-through. The implementation of this function must copy
 **                  the data to the phones memory.
 **
 ** Returns          void
 **
 ******************************************************************************/
void bta_ag_co_tx_write(uint16_t handle, UNUSED_ATTR uint8_t* p_data,
                        uint16_t len) {
  BTIF_TRACE_DEBUG("bta_ag_co_tx_write: handle: %d, len: %d", handle, len);
}
