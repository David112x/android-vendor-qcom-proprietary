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

#include <string.h>
#include "bt_common.h"
#include "bta_api.h"
#include "bta_sys.h"
#include "btif_bat.h"
#include "sdp_api.h"
#include "hcimsgs.h"
#include "bta_bat.h"
#include "btm_api.h"
#include "hcidefs.h"

#include "osi/include/allocator.h"

#define MAX_COMMANDS     10
#define START_LT_ADDR    2
#define MAX_LT_ADDR      7
static uint8_t bat_lt_addr = START_LT_ADDR;

typedef struct {
    uint32_t sdp_handle;
    uint16_t pending_cmds[MAX_COMMANDS];
    uint16_t num_cmd_pending;
    uint16_t ack_pending_req;// message to send to btif once all cmds are sent
    uint8_t enc_key[ENCRYPTION_KEY_LEN];
    uint8_t div_key[DIV_KEY_LEN];
    uint8_t curr_vol_level;
    uint8_t curr_playing_state;
}bta_ba_cb_t;

bta_ba_cb_t bta_ba_cb;
static uint8_t bat_sdp_uuid[16] = {0x3d,0x6d,0x40,0x0e,0xaa,0xd7,0xf8,0xac,0x43,0x43,0x4d,0x5d,0xc9,0xbe,0x18,0xba};
bool bta_ba_hdl_msg(BT_HDR* p_msg);

#ifndef CASE_RETURN_STR
#define CASE_RETURN_STR(const) \
  case const:                  \
    return #const;
#endif

const char* dump_ba_event(uint16_t event) {
  switch (event) {
    CASE_RETURN_STR(BTA_BA_REGISTER_REQ)
    CASE_RETURN_STR(BTA_BA_ENABLE_REQ)
    CASE_RETURN_STR(BTA_BA_PAUSE_REQ)
    CASE_RETURN_STR(BTA_BA_STREAM_REQ)
    CASE_RETURN_STR(BTA_BA_STOP_REQ)
    CASE_RETURN_STR(BTA_BA_SET_VOL_REQ)
    CASE_RETURN_STR(BTA_BA_SET_ENC_KEY)
    CASE_RETURN_STR(BTA_BA_CMD_SET_LT_ADDR)
    CASE_RETURN_STR(BTA_BA_CMD_DELETE_LT_ADDR)
    CASE_RETURN_STR(BTA_BA_CMD_VS_TX_CONFIG)
    CASE_RETURN_STR(BTA_BA_CMD_ENABLE_CSB)
    CASE_RETURN_STR(BTA_BA_CMD_START_SYNC_TRAIN)
    CASE_RETURN_STR(BTA_BA_RSP_SET_LT_ADDR)
    CASE_RETURN_STR(BTA_BA_RSP_VS_TX_CONFIG)
    CASE_RETURN_STR(BTA_BA_RSP_VS_VOL)
    CASE_RETURN_STR(BTA_BA_RSP_ENABLE_CSB)
    CASE_RETURN_STR(BTA_BA_RSP_START_SYNC_TRAIN)
    CASE_RETURN_STR(BTA_BA_DEREGISTER_REQ)
    CASE_RETURN_STR(BTA_BA_CMD_SEND_SYNC_TRAIN_PARAM)
    CASE_RETURN_STR(BTA_BA_RSP_SEND_SYNC_TRAIN_PARAM)
    CASE_RETURN_STR(BTA_BA_HCI_EVT_CSB_TIMEOUT)
    default:
      return "UNKNOWN_EVENT";
  }
}

const char* dump_btif_event(uint16_t event) {
  switch (event) {
    CASE_RETURN_STR(BTIF_BA_RSP_STOP_DONE_EVT)
    CASE_RETURN_STR(BTIF_BA_RSP_PAUSE_DONE_EVT)
    CASE_RETURN_STR(BTIF_BA_RSP_STREAM_DONE_EVT)
    CASE_RETURN_STR(BTIF_BA_RSP_VOL_UPDATE_DONE_EVT)
    CASE_RETURN_STR(BTIF_BA_RSP_ENC_KEY_UPDATE_DONE_EVT)
    default:
      return "UNKNOWN_EVENT";
  }
}

void bta_ba_handle_register_req()
{
    APPL_TRACE_DEBUG(" %s ", __func__);
    bta_ba_cb.sdp_handle = SDP_CreateRecord();
    if(bta_ba_cb.sdp_handle == 0)
    {
        APPL_TRACE_DEBUG(" Not able to allocate SDP Handle ");
        return;
    }
    SDP_AddAttribute(bta_ba_cb.sdp_handle, ATTR_ID_SERVICE_CLASS_ID_LIST,
                                             UUID_DESC_TYPE, 16, bat_sdp_uuid);
    bta_ba_cb.num_cmd_pending = 0;
    bta_ba_cb.ack_pending_req = 0;
    for (int i = 0; i < MAX_COMMANDS; i++) {
        bta_ba_cb.pending_cmds[i] = 0;
    }

}

void bta_ba_handle_deregister_req()
{
    APPL_TRACE_DEBUG(" %s ", __func__);
    SDP_DeleteRecord(bta_ba_cb.sdp_handle);
    bta_ba_cb.sdp_handle = 0;
    bta_ba_cb.num_cmd_pending = 0;
    bta_ba_cb.ack_pending_req = 0;
    for (int i = 0; i < MAX_COMMANDS; i++) {
        bta_ba_cb.pending_cmds[i] = 0;
    }
}

// this api just picks first command from top of the pending_cmd list
// and send message to bta_ba.
void process_hci_cmds(uint16_t event) {
    APPL_TRACE_DEBUG(" %s pending commands = %d event = %d top_cmd = %d",
        __func__,bta_ba_cb.num_cmd_pending, event, bta_ba_cb.pending_cmds[0]);

    if (event != bta_ba_cb.pending_cmds[0]) {
        return;
    }

    BT_HDR* p_buf =
      (BT_HDR*)osi_malloc(sizeof(BT_HDR));
    p_buf->event = event;

     bta_sys_sendmsg(p_buf);
}

void  bta_ba_handle_set_enc_key_req() {
    APPL_TRACE_DEBUG(" %s pending commands = %d ack_cmd = %d ", __func__,
                  bta_ba_cb.num_cmd_pending, bta_ba_cb.ack_pending_req);
    if(bta_ba_cb.num_cmd_pending !=0) {
        APPL_TRACE_ERROR(" %s cmds already pending, bail out",__func__);
        return;
    }
    bta_ba_cb.ack_pending_req = BTIF_BA_RSP_ENC_KEY_UPDATE_DONE_EVT;
    bta_ba_cb.num_cmd_pending = 1;
    bta_ba_cb.pending_cmds[0] = BTA_BA_CMD_VS_TX_CONFIG;
    process_hci_cmds(bta_ba_cb.pending_cmds[0]);
}

void  bta_ba_handle_set_vol_req() {
    APPL_TRACE_DEBUG(" %s pending commands = %d ack_cmd = %d ", __func__,
                  bta_ba_cb.num_cmd_pending, bta_ba_cb.ack_pending_req);
    if(bta_ba_cb.num_cmd_pending !=0) {
        APPL_TRACE_ERROR(" %s cmds already pending, bail out",__func__);
        return;
    }
    bta_ba_cb.ack_pending_req = BTIF_BA_RSP_VOL_UPDATE_DONE_EVT;
    bta_ba_cb.num_cmd_pending = 1;
    bta_ba_cb.pending_cmds[0] = BTA_BA_CMD_VS_VOL;
    process_hci_cmds(bta_ba_cb.pending_cmds[0]);
}

void bta_ba_handle_stop_req() {
    APPL_TRACE_DEBUG(" %s pending commands = %d ack_cmd = %d ", __func__,
                    bta_ba_cb.num_cmd_pending, bta_ba_cb.ack_pending_req);
    if(bta_ba_cb.num_cmd_pending !=0) {
        APPL_TRACE_ERROR(" %s cmds already pending, bail out",__func__);
        return;
    }
    bta_ba_cb.ack_pending_req = BTIF_BA_RSP_STOP_DONE_EVT;
    bta_ba_cb.num_cmd_pending = 2;
    bta_ba_cb.pending_cmds[0] = BTA_BA_CMD_DISABLE_CSB;
    bta_ba_cb.pending_cmds[1] = BTA_BA_CMD_DELETE_LT_ADDR;
    process_hci_cmds(bta_ba_cb.pending_cmds[0]);
}

void bta_ba_handle_pause_req() {
    APPL_TRACE_DEBUG(" %s pending commands = %d ack_cmd = %d ", __func__,
                    bta_ba_cb.num_cmd_pending, bta_ba_cb.ack_pending_req);
    if(bta_ba_cb.num_cmd_pending !=0) {
        APPL_TRACE_ERROR(" %s cmds already pending, bail out",__func__);
        return;
    }
    bta_ba_cb.ack_pending_req = BTIF_BA_RSP_PAUSE_DONE_EVT;
    bta_ba_cb.num_cmd_pending = 2;
    bta_ba_cb.pending_cmds[0] = BTA_BA_CMD_ENABLE_CSB;
    bta_ba_cb.pending_cmds[1] = BTA_BA_CMD_VS_TX_CONFIG;

    process_hci_cmds(bta_ba_cb.pending_cmds[0]);
}

void bta_ba_handle_stream_req(){
    APPL_TRACE_DEBUG(" %s pending commands = %d ack_cmd = %d ", __func__,
                      bta_ba_cb.num_cmd_pending, bta_ba_cb.ack_pending_req);
    if (bta_ba_cb.num_cmd_pending !=0) {
        APPL_TRACE_ERROR(" %s cmds already pending, bail out",__func__);
        return;
    }
    bta_ba_cb.ack_pending_req = BTIF_BA_RSP_STREAM_DONE_EVT;
    bta_ba_cb.num_cmd_pending = 2;
    bta_ba_cb.pending_cmds[0] = BTA_BA_CMD_ENABLE_CSB;
    bta_ba_cb.pending_cmds[1] = BTA_BA_CMD_VS_TX_CONFIG;
    process_hci_cmds(bta_ba_cb.pending_cmds[0]);
}

void bta_ba_handle_enable_req()
{
    APPL_TRACE_DEBUG(" %s pending commands = %d ack_cmd = %d ", __func__,
                        bta_ba_cb.num_cmd_pending, bta_ba_cb.ack_pending_req);
    if (bta_ba_cb.num_cmd_pending !=0) {
        APPL_TRACE_ERROR(" %s cmds already pending, bail out",__func__);
        return;
    }
    bta_ba_cb.ack_pending_req = BTIF_BA_RSP_PAUSE_DONE_EVT;
    bta_ba_cb.num_cmd_pending = 5;
    bat_lt_addr = START_LT_ADDR;
    bta_ba_cb.pending_cmds[0] = BTA_BA_CMD_SET_LT_ADDR;
    bta_ba_cb.pending_cmds[1] = BTA_BA_CMD_ENABLE_CSB;
    bta_ba_cb.pending_cmds[2] = BTA_BA_CMD_SEND_SYNC_TRAIN_PARAM;
    bta_ba_cb.pending_cmds[3] = BTA_BA_CMD_VS_TX_CONFIG;
    bta_ba_cb.pending_cmds[4] = BTA_BA_CMD_START_SYNC_TRAIN;
    process_hci_cmds(bta_ba_cb.pending_cmds[0]);
}
void ba_vs_cmd_cback(tBTM_VSC_CMPL *param){
    uint8_t status = 0;
    uint8_t sub_opcode = 0;
    uint16_t opcode = param->opcode;
    uint16_t param_len = param->param_len;

    APPL_TRACE_DEBUG(" %s opcode  = %x param_len = %d",__func__,
                                                   opcode, param_len);
    status = param->p_param_buf[0];
    sub_opcode = param->p_param_buf[1];
    //status = HCI_SUCCESS;// HACK till we get proper response from soc.
    APPL_TRACE_DEBUG(" %s pencking_ack  = %d status = %x, sub_opcode = %x",
               __func__,bta_ba_cb.ack_pending_req, status, sub_opcode);

    if (status != HCI_SUCCESS) {
      // in case of error subopcode might not be correct, use ack_pending_req
      if (bta_ba_cb.ack_pending_req == BTIF_BA_RSP_VOL_UPDATE_DONE_EVT)
        sub_opcode = VS_HCI_BAT_TX_VOL;
      else
        sub_opcode = VS_HCI_BAT_TX_CONFIG;
    }

    switch(sub_opcode) {
      case VS_HCI_BAT_TX_CONFIG:
        bta_ba_handle_hci_event(BTA_BA_RSP_VS_TX_CONFIG,status, NULL, 0);
        break;
      case VS_HCI_BAT_TX_VOL:
        bta_ba_handle_hci_event(BTA_BA_RSP_VS_VOL,status, NULL, 0);
        break;
    }
}
/*******************************************************************************
 *
 * Function         bta_ba_hdl_msg
 *
 * Description      BA main event handling function.
 *
 *
 * Returns          bool
 *
 ******************************************************************************/
bool bta_ba_hdl_msg(BT_HDR* p_msg) {
    APPL_TRACE_DEBUG(" %s event = %s", __func__, dump_ba_event(p_msg->event));
    uint8_t param[40];
    uint8_t index = 0;
    switch(p_msg->event)
    {
      // request from btif
    case BTA_BA_REGISTER_REQ:
        bta_ba_handle_register_req();
        break;
    case BTA_BA_DEREGISTER_REQ:
        bta_ba_handle_deregister_req();
        break;
    case BTA_BA_ENABLE_REQ:
        bta_ba_handle_enable_req();
        bta_ba_cb.curr_playing_state = BTA_BA_STATE_DISABLED;
        break;
    case BTA_BA_STREAM_REQ:
        bta_ba_handle_stream_req();
        break;
    case BTA_BA_PAUSE_REQ:
        bta_ba_handle_pause_req();
        break;
    case BTA_BA_STOP_REQ:
        bta_ba_handle_stop_req();
        break;
    case BTA_BA_SET_VOL_REQ:
        bta_ba_handle_set_vol_req();
        break;
    case BTA_BA_SET_ENC_KEY:
        bta_ba_handle_set_enc_key_req();
        break;
        // cmds send to HCI
    case BTA_BA_CMD_SET_LT_ADDR:
        btsnd_hcic_set_reserved_lt_addr(bat_lt_addr);
        break;
    case BTA_BA_CMD_VS_TX_CONFIG:
        param[index++] = VS_HCI_BAT_TX_CONFIG;
        // first parameter is Stream ID
        if (bta_ba_cb.ack_pending_req == BTIF_BA_RSP_PAUSE_DONE_EVT) {
            param[index++] = VS_HCI_STREAM_ID_INVALID;
        }
        else if (bta_ba_cb.ack_pending_req == BTIF_BA_RSP_STREAM_DONE_EVT) {
            param[index++] = VS_HCI_STREAM_ID_VALID;
        }
        else if (bta_ba_cb.ack_pending_req == BTIF_BA_RSP_ENC_KEY_UPDATE_DONE_EVT) {
            switch(bta_ba_cb.curr_playing_state) {
              case BTA_BA_STATE_PAUSED:
              case BTA_BA_STATE_DISABLED:
                 param[index++] = VS_HCI_STREAM_ID_INVALID;
                 break;
              case BTA_BA_STATE_STREAMING:
                 param[index++] = VS_HCI_STREAM_ID_VALID;
                 break;
            }
        }
        //LT_ADDR
        param[index++] = bat_lt_addr;
        //CODEC type
        param[index++] = VS_HCI_CODEC_TYPE_CELT;
        //sampling frequency
        param[index++] = VS_HCI_SAMPLING_FREQ_48;
        // packet size: 2 bytes
        param[index++] = VS_HCI_MAX_PACKET_SIZE_48;
        param[index++] = 0x00;
        // encoding scheme
        param[index++] = VS_HCI_ENCODING_SCHEME_2_5;
        // TTP offset: 3 bytes
        param[index++] = (VS_HCI_TTP_OFFSET) & 0x0000FF;
        param[index++] = (VS_HCI_TTP_OFFSET >> 8) & 0x0000FF;
        param[index++] = (VS_HCI_TTP_OFFSET >> 16);
        param[index++] = 0;
        // Initialization vector
        memcpy(&param[index], bta_ba_cb.enc_key, ENCRYPTION_KEY_LEN);
        index = index + ENCRYPTION_KEY_LEN;
        //Initialization vector
        //memcpy(&param[index], bta_ba_cb.div_key, DIV_KEY_LEN);
        //index += DIV_KEY_LEN
        param[index++] = bta_ba_cb.div_key[1];
        param[index++] = bta_ba_cb.div_key[0];
        //
        param[index++] = 0xBE;
        param[index++] = 0xEF;
        param[index++] = 0xEE;
        param[index++] = 0xFA;
        param[index++] = 0xCA;
        param[index++] = 0xDE;
        //vol level
        // because vol level for MM ( 0-15 ). BA (0-31)
        param[index++] = 2*bta_ba_cb.curr_vol_level;
        // Power Level
        param[index++] = VS_HCI_TX_POWER_LEVEL;
        // sample size
        param[index++] = VS_HCI_SAMPLE_SIZE & 0x00FF;
        param[index++] = (VS_HCI_SAMPLE_SIZE >> 8) & 0x00FF;

        BTM_VendorSpecificCommand(VS_BA_CMD_OPCODE, index, param, ba_vs_cmd_cback);
        APPL_TRACE_DEBUG(" %s param_len = %d",__func__, index);
        break;
    case BTA_BA_CMD_ENABLE_CSB:
        if (bta_ba_cb.ack_pending_req == BTIF_BA_RSP_PAUSE_DONE_EVT) {
            btsnd_hcic_set_csb(HCI_ENABLE_CSB, bat_lt_addr, 0,
                HCI_CSB_PACKET_TYPE, HCI_CSB_INTEVAL_LONG_MIN, HCI_CSB_INTEVAL_LONG_MAX, HCI_CSB_TIMEOUT);
        }
        else if (bta_ba_cb.ack_pending_req == BTIF_BA_RSP_STREAM_DONE_EVT) {
            btsnd_hcic_set_csb(HCI_ENABLE_CSB, bat_lt_addr, 0,
                HCI_CSB_PACKET_TYPE, HCI_CSB_INTEVAL_SHORT, HCI_CSB_INTEVAL_SHORT, HCI_CSB_TIMEOUT);
        }
        break;
    case BTA_BA_CMD_SEND_SYNC_TRAIN_PARAM:
        btsnd_hcic_write_sync_train_param(HCI_SYNC_TRAIN_MIN_INTERVAL,
                 HCI_SYNC_TRAIN_MAX_INTERVAL, HCI_SYNC_TRAIN_TIMEOUT, 0);
        break;
    case BTA_BA_CMD_START_SYNC_TRAIN:
        btsnd_hcic_start_synch_train();
        break;
    case BTA_BA_CMD_DISABLE_CSB:
         btsnd_hcic_set_csb(HCI_DISABLE_CSB, bat_lt_addr, 0,
           HCI_CSB_PACKET_TYPE, HCI_CSB_INTEVAL_LONG_MAX, HCI_CSB_INTEVAL_LONG_MAX, HCI_CSB_TIMEOUT);
        break;
    case BTA_BA_CMD_DELETE_LT_ADDR:
        btsnd_hcic_delete_reserved_lt_addr(bat_lt_addr);
        break;
    case BTA_BA_CMD_VS_VOL:
        param[index++] = VS_HCI_BAT_TX_VOL;
        param[index++] = 2*bta_ba_cb.curr_vol_level;
        BTM_VendorSpecificCommand(VS_BA_CMD_OPCODE, index, param, ba_vs_cmd_cback);
        break;
    }
  return true;
}

static const tBTA_SYS_REG bta_ba_reg = {bta_ba_hdl_msg, BTA_BADisable};

// as this api is also called from btm, which is same as btu thread context
// we can directly acnknowledge btif from this api.
void  bta_ba_handle_hci_event(uint16_t event, uint8_t result, uint8_t* p_data,
                              uint8_t data_len) {
    APPL_TRACE_DEBUG(" %s event= %s result = %x data_len = %d", __func__,
                                  dump_ba_event(event), result, data_len);
    APPL_TRACE_DEBUG(" pending_cmd[0] = %s ack_pending = %s num_cmds_pending = %d"
     ,dump_ba_event(bta_ba_cb.pending_cmds[0])
     ,dump_btif_event(bta_ba_cb.ack_pending_req), bta_ba_cb.num_cmd_pending);

    uint16_t topmost_pending_cmd = 0;
    uint8_t command_status = BTA_HCI_CMD_SUCCESS;
    switch(event) {
      case BTA_BA_RSP_SET_LT_ADDR:
         topmost_pending_cmd = BTA_BA_CMD_SET_LT_ADDR;
         if (result == HCI_ERR_CONNECTION_EXISTS) {
             bat_lt_addr ++;
             if (bat_lt_addr > MAX_LT_ADDR) {
                 // we have tried all possible lt_addrs.treat this as
                 // command failure and return.
               command_status =  BTA_HCI_CMD_FAILURE;
               bat_lt_addr = START_LT_ADDR;
             } else {
               command_status = BTA_HCI_CMD_RETRY;
             }
         }
         break;
      case BTA_BA_RSP_SEND_SYNC_TRAIN_PARAM:
        topmost_pending_cmd = BTA_BA_CMD_SEND_SYNC_TRAIN_PARAM;
        break;
      case BTA_BA_RSP_VS_TX_CONFIG:
        topmost_pending_cmd = BTA_BA_CMD_VS_TX_CONFIG;
        if (result != HCI_SUCCESS) {
            command_status = BTA_HCI_CMD_FAILURE;
        }
        break;
        // even when we disable CSB, we would always receive only ENABLE_CSB
        // from hci.
      case BTA_BA_RSP_ENABLE_CSB:
        if(bta_ba_cb.ack_pending_req == BTIF_BA_RSP_STOP_DONE_EVT) {
            topmost_pending_cmd = BTA_BA_CMD_DISABLE_CSB;
        }
        else {
            topmost_pending_cmd = BTA_BA_CMD_ENABLE_CSB;
            //get value of interval
            APPL_TRACE_DEBUG(" data_received = %x %x %x", p_data[0],
                                     p_data[1],p_data[2]);
            uint16_t interval = p_data[2];
            interval = interval << 8;
            interval = interval | p_data[1];
            APPL_TRACE_DEBUG(" interval received = %x  ack_pending = %d",
                                       interval, bta_ba_cb.ack_pending_req);
        }
        if (result != HCI_SUCCESS) {
            command_status = BTA_HCI_CMD_FAILURE;
        }
        break;
      case BTA_BA_RSP_START_SYNC_TRAIN:
        topmost_pending_cmd = BTA_BA_CMD_START_SYNC_TRAIN;
        break;
      case BTA_BA_RSP_VS_VOL:
        topmost_pending_cmd = BTA_BA_CMD_VS_VOL;
        // even if a error occurs, don't do anything.Let it process like
        // normal sequence
        break;
      case BTA_BA_RSP_DELETE_LT_ADDR:
        topmost_pending_cmd = BTA_BA_CMD_DELETE_LT_ADDR;
        break;
      case BTA_BA_HCI_EVT_CSB_TIMEOUT:
        command_status = BTA_HCI_CMD_FAILURE;
        result = HCI_ERR_HOST_REJECT_RESOURCES;
        if (bta_ba_cb.ack_pending_req == 0) {
            // CSB timeout happened while we were not sending any command.
            bta_ba_cb.ack_pending_req = BTIF_BA_CSB_TIMEOUT_EVT;
        }
        break;
      default:
        APPL_TRACE_ERROR(" %s UNKNOWN event ", __func__);
        break;
    }
    APPL_TRACE_DEBUG(" %s topmost_pending_cmd = %d ", __func__,
                                              topmost_pending_cmd);
    if ((topmost_pending_cmd != bta_ba_cb.pending_cmds[0]) &&
        (event != BTA_BA_HCI_EVT_CSB_TIMEOUT)){
        APPL_TRACE_ERROR(" %s cmd event mismatch ", __func__);
        return;
    }
    if (command_status == BTA_HCI_CMD_SUCCESS) {
        // topmost cmd and expected event matches.
        int i = 0;
        for (i = 1; i< bta_ba_cb.num_cmd_pending; i++) {
            bta_ba_cb.pending_cmds[i -1] = bta_ba_cb.pending_cmds[i];
        }
        // decrement num_cmd_pending
        bta_ba_cb.num_cmd_pending--;
    }

    if (command_status == BTA_HCI_CMD_FAILURE) {
       // one of the commands failed, fail the whole set
       for (int i = bta_ba_cb.num_cmd_pending; i>=0; i--) {
           bta_ba_cb.pending_cmds[i] = 0;
       }
       bta_ba_cb.num_cmd_pending = 0;
    }

    if (bta_ba_cb.num_cmd_pending == 0) {
        // if theere is a failure, don't change state.
        if (command_status == BTA_HCI_CMD_SUCCESS) {
            switch(bta_ba_cb.ack_pending_req) {
              case BTIF_BA_RSP_PAUSE_DONE_EVT:
                  bta_ba_cb.curr_playing_state = BTA_BA_STATE_PAUSED;
              break;
              case BTIF_BA_RSP_STOP_DONE_EVT:
                  bta_ba_cb.curr_playing_state = BTA_BA_STATE_DISABLED;
              break;
              case BTIF_BA_RSP_STREAM_DONE_EVT:
                  bta_ba_cb.curr_playing_state = BTA_BA_STATE_STREAMING;
              break;
            }
        }
        // no more HCI command to process. acknowledge btif from here..
        btif_ba_bta_callback(bta_ba_cb.ack_pending_req, result);
        bta_ba_cb.ack_pending_req = 0;
        return;
    }
    process_hci_cmds(bta_ba_cb.pending_cmds[0]);
}
/*******************************************************************************
 *
 * Function         BTA_BAEnable
 *
 * Description      Enable the Broadcast Audio Transmitter profile.
 *                  We register SDP record as part of enable
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_BAEnable()
{
    /* register with BTA system manager */
    bta_sys_register(BTA_ID_BAT, &bta_ba_reg);
}

/*******************************************************************************
 *
 * Function         BTA_BARegister
 *
 * Description      Registers Broadcast Audio Transmitter profile.
 *                  We register SDP record as part of register
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_BARegister() {
    BT_HDR* p_buf =
      (BT_HDR*)osi_malloc(sizeof(BT_HDR));

    p_buf->event = BTA_BA_REGISTER_REQ;
    bta_sys_sendmsg(p_buf);
}

void BTA_PauseBA(bool start_sync_train, uint8_t* p_enc_key, uint8_t* p_div_key,
                                               uint8_t vol_level)
{
    /*
     *  this api will send list of command required to move to PAUSE_STATE
     *  It can be called either from OFF state or from streaming state.
     *  If called from off state, start_sync_train should be true.
     */
    APPL_TRACE_DEBUG(" %s start_sync_train =%d ", __func__, start_sync_train);
    BT_HDR* p_buf =
      (BT_HDR*)osi_malloc(sizeof(BT_HDR));

    if (start_sync_train)
    {
        p_buf->event = BTA_BA_ENABLE_REQ;
    }
    else
    {
        p_buf->event = BTA_BA_PAUSE_REQ;
    }
    memcpy(bta_ba_cb.enc_key, p_enc_key, ENCRYPTION_KEY_LEN);
    memcpy(bta_ba_cb.div_key, p_div_key, DIV_KEY_LEN);
    bta_ba_cb.curr_vol_level = vol_level;
    bta_sys_sendmsg(p_buf);
}

void BTA_StreamBA()
{
    APPL_TRACE_DEBUG(" %s ", __func__);
    BT_HDR* p_buf =
      (BT_HDR*)osi_malloc(sizeof(BT_HDR));
    p_buf->event = BTA_BA_STREAM_REQ;
    bta_sys_sendmsg(p_buf);
}

void BTA_StopBA()
{
    APPL_TRACE_DEBUG(" %s ", __func__);
    BT_HDR* p_buf =
      (BT_HDR*)osi_malloc(sizeof(BT_HDR));
    p_buf->event = BTA_BA_STOP_REQ;
    bta_sys_sendmsg(p_buf);
}

void BTA_SetVol(uint8_t curr_vol_level)
{
    APPL_TRACE_DEBUG(" %s ", __func__);
    BT_HDR* p_buf =
      (BT_HDR*)osi_malloc(sizeof(BT_HDR));
    p_buf->event = BTA_BA_SET_VOL_REQ;
    bta_ba_cb.curr_vol_level = curr_vol_level;
    bta_sys_sendmsg(p_buf);
}

void BTA_SetEncKey(uint8_t* enc_key)
{
    APPL_TRACE_DEBUG(" %s ", __func__);
    BT_HDR* p_buf =
      (BT_HDR*)osi_malloc(sizeof(BT_HDR));
    p_buf->event = BTA_BA_SET_ENC_KEY;
    memcpy(bta_ba_cb.enc_key, enc_key, ENCRYPTION_KEY_LEN);
    bta_sys_sendmsg(p_buf);
}
/*******************************************************************************
 *
 * Function         BTA_BADeregister
 *
 * Description      DeRegisters Broadcast Audio Transmitter profile.
 *                  We deregister SDP record as part of enable
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_BADeregister() {
    BT_HDR* p_buf = (BT_HDR*)osi_malloc(sizeof(BT_HDR));
    p_buf->event = BTA_BA_DEREGISTER_REQ;
    bta_sys_sendmsg(p_buf);
}

/*******************************************************************************
 *
 * Function         BTA_BADisable
 *
 * Description      Disable the Broadcast Audio.
 *
 * Returns          void
 *
 ******************************************************************************/
void BTA_BADisable(void) {
    bta_sys_deregister(BTA_ID_BAT);
}
