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

#include "bt_common.h"
#include "bt_target.h"
#include "btu.h"
#include "hcidefs.h"
#include "hcimsgs.h"

#include <stddef.h>
#include <string.h>

#include "btm_int.h"

/* Connectionless Broadcast */
void btsnd_hcic_set_reserved_lt_addr(uint8_t lt_addr) {
  BT_HDR* p = (BT_HDR*)osi_malloc(HCI_CMD_BUF_SIZE);
  uint8_t* pp = (uint8_t*)(p + 1);

  p->len = HCIC_PREAMBLE_SIZE + 1;
  p->offset = 0;

  UINT16_TO_STREAM(pp, HCI_SET_RESERVED_LT_ADDR);
  UINT8_TO_STREAM(pp, p->len - HCIC_PREAMBLE_SIZE);

  UINT8_TO_STREAM(pp, lt_addr);

  btu_hcif_send_cmd(LOCAL_BR_EDR_CONTROLLER_ID, p);
}

void btsnd_hcic_delete_reserved_lt_addr(uint8_t lt_addr) {
  BT_HDR* p = (BT_HDR*)osi_malloc(HCI_CMD_BUF_SIZE);
  uint8_t* pp = (uint8_t*)(p + 1);

  p->len = HCIC_PREAMBLE_SIZE + 1;
  p->offset = 0;

  UINT16_TO_STREAM(pp, HCI_DELETE_RESERVED_LT_ADDR);
  UINT8_TO_STREAM(pp, p->len - HCIC_PREAMBLE_SIZE);

  UINT8_TO_STREAM(pp, lt_addr);

  btu_hcif_send_cmd(LOCAL_BR_EDR_CONTROLLER_ID, p);
}

void btsnd_hcic_start_synch_train() {
  BT_HDR* p = (BT_HDR*)osi_malloc(HCI_CMD_BUF_SIZE);
  uint8_t* pp = (uint8_t*)(p + 1);

  p->len = HCIC_PREAMBLE_SIZE;
  p->offset = 0;

  UINT16_TO_STREAM(pp, HCI_START_SYNC_TRAIN);
  UINT8_TO_STREAM(pp, 0);

  btu_hcif_send_cmd(LOCAL_BR_EDR_CONTROLLER_ID, p);
}

void btsnd_hcic_set_csb(uint8_t enable, uint8_t lt_addr, uint8_t lpo_allowed,
                        uint16_t pkt_type, int16_t min_interval,
                        uint16_t max_interval, uint16_t csb_supvisnto) {
  BT_HDR* p = (BT_HDR*)osi_malloc(HCI_CMD_BUF_SIZE);
  uint8_t* pp = (uint8_t*)(p + 1);

  p->len = HCIC_PREAMBLE_SIZE + HCIC_PARAM_SIZE_SET_CSB;
  p->offset = 0;

  UINT16_TO_STREAM(pp, HCI_SET_CLB);
  UINT8_TO_STREAM(pp, HCIC_PARAM_SIZE_SET_CSB);

  UINT8_TO_STREAM(pp, enable);
  UINT8_TO_STREAM(pp, lt_addr);
  UINT8_TO_STREAM(pp, lpo_allowed);
  UINT16_TO_STREAM(pp, pkt_type);
  UINT16_TO_STREAM(pp, min_interval);
  UINT16_TO_STREAM(pp, max_interval);
  UINT16_TO_STREAM(pp, csb_supvisnto);

  btu_hcif_send_cmd(LOCAL_BR_EDR_CONTROLLER_ID, p);
}

void btsnd_hcic_set_csb_data(uint8_t lt_addr, uint8_t fragment,
                             uint8_t data_len, uint8_t* data) {
  BT_HDR* p = (BT_HDR*)osi_malloc(HCI_CMD_BUF_SIZE);
  uint8_t* pp = (uint8_t*)(p + 1);

  p->len = HCIC_PREAMBLE_SIZE + 3 + data_len;
  p->offset = 0;

  UINT16_TO_STREAM(pp, HCI_WRITE_CLB_DATA);
  UINT8_TO_STREAM(pp, p->len - HCIC_PREAMBLE_SIZE);

  UINT8_TO_STREAM(pp, lt_addr);
  UINT8_TO_STREAM(pp, fragment);
  UINT8_TO_STREAM(pp, data_len);
  ARRAY_TO_STREAM  (pp, data, data_len);

  btu_hcif_send_cmd(LOCAL_BR_EDR_CONTROLLER_ID, p);
}

void btsnd_hcic_write_sync_train_param(uint16_t min_interval, uint16_t max_interval,
                             uint32_t sync_train, uint8_t data) {
  BT_HDR* p = (BT_HDR*)osi_malloc(HCI_CMD_BUF_SIZE);
  uint8_t* pp = (uint8_t*)(p + 1);

  p->len = HCIC_PREAMBLE_SIZE + HCIC_PARAM_SIZE_SYNC_TRAIN;
  p->offset = 0;

  UINT16_TO_STREAM(pp, HCI_WRITE_SYNC_TRAIN_PARAM);
  UINT8_TO_STREAM(pp,  HCIC_PARAM_SIZE_SYNC_TRAIN);

  UINT16_TO_STREAM(pp, min_interval);
  UINT16_TO_STREAM(pp, max_interval);
  UINT32_TO_STREAM(pp, sync_train);
  UINT8_TO_STREAM(pp, data);

  btu_hcif_send_cmd(LOCAL_BR_EDR_CONTROLLER_ID, p);
}
