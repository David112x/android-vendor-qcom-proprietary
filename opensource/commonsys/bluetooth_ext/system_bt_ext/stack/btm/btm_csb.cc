/*
 * Copyright (c) 2017, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of The Linux Foundation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
 
#include <base/logging.h>
#include <stddef.h>

#include <stdlib.h>
#include <string.h>
#include "bt_types.h"
#include "bt_utils.h"
#include "btcore/include/module.h"
#include "btm_int.h"
#include "btu.h"
#include "device/include/controller.h"
#include "hci_layer.h"
#include "hcimsgs.h"
#include "osi/include/osi.h"
#include "bta_bat.h"

/*******************************************************************************
 *
 * Function         btm_hci_csb_timeout_evt
 *
 * Description      Process event HCI_CSB_TIMEOUT_EVT
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void btm_hci_csb_timeout_evt(uint8_t* p) {
  RawAddress  bda;
  uint8_t status;
  uint8_t lt_addr;

  BTM_TRACE_DEBUG("BTM Event: Connectionless Slave Broadcast Timeout");
  STREAM_TO_UINT8(status, p);
  STREAM_TO_BDADDR(bda, p);
  STREAM_TO_UINT8(lt_addr, p);

  bta_ba_handle_hci_event(BTA_BA_HCI_EVT_CSB_TIMEOUT, status, NULL, 0);
}

/*******************************************************************************
 *
 * Function         btm_hci_start_sync_train_complete
 *
 * Description      Process event HCI_START_SYNC_TRAIN
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void btm_hci_start_sync_train_complete(uint8_t* p) {
  uint8_t status;

  BTM_TRACE_DEBUG("BTM Event: btm_hci_start_sync_train_complete");
  STREAM_TO_UINT8 (status, p);

  bta_ba_handle_hci_event(BTA_BA_RSP_START_SYNC_TRAIN, status, NULL, 0);
}

/*******************************************************************************
 *
 * Function         btm_hci_set_reserved_lt_addr_complete
 *
 * Description      Process event HCI_SET_RESERVED_LT_ADDR
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void btm_hci_set_reserved_lt_addr_complete(uint8_t* p) {
  uint8_t status;

  STREAM_TO_UINT8(status, p);

  bta_ba_handle_hci_event(BTA_BA_RSP_SET_LT_ADDR, status, NULL, 0);
}

/*******************************************************************************
 *
 * Function         btm_hci_delete_reserved_lt_addr_complete
 *
 * Description      Process event HCI_DELETE_RESERVED_LT_ADDR
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void btm_hci_delete_reserved_lt_addr_complete(uint8_t* p) {
  uint8_t status;

  STREAM_TO_UINT8(status, p);

  bta_ba_handle_hci_event(BTA_BA_RSP_DELETE_LT_ADDR, status, NULL, 0);
}


/*******************************************************************************
 *
 * Function         btm_hci_write_sync_train_param_complete
 *
 * Description      Process event HCI_SYNCHRONIZATION_TRAIN_COMPLETE_EVENT
 *
 *
 * Returns          void
 *
 ******************************************************************************/
 void btm_hci_write_sync_train_param_complete(uint8_t* p){
 uint8_t status;

  BTM_TRACE_DEBUG("BTM Event: btm_hci_write_sync_train_param_complete");
  STREAM_TO_UINT8 (status, p);

  bta_ba_handle_hci_event(BTA_BA_RSP_SEND_SYNC_TRAIN_PARAM, status, NULL, 0);
}

/*******************************************************************************
 *
 * Function         btm_hci_set_csb_complete
 *
 * Description      Process event HCI_SET_CLB
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void btm_hci_set_csb_complete(uint8_t* p) {
  uint8_t status;

  BTM_TRACE_DEBUG("BTM Event: btm_hci_set_csb_complete");
  STREAM_TO_UINT8 (status, p);

  bta_ba_handle_hci_event(BTA_BA_RSP_ENABLE_CSB, status, (p), 3);
}
