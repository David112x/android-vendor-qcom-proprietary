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

#ifndef BTA_BA_H
#define BTA_BA_H

#include "bta_sys.h"

enum {
    BTA_BA_REGISTER_REQ = BTA_SYS_EVT_START(BTA_ID_BAT),
    BTA_BA_ENABLE_REQ,// send SET_LT_ADDR, VS_TX_CONFIG, CONN_SLAVE_BROADCAST, START_SYNC_TRAIN
    BTA_BA_PAUSE_REQ, // send VS_TX_CONFIG and CONN_SLAVE_BROADCAST
    BTA_BA_STREAM_REQ,
    BTA_BA_STOP_REQ,
    BTA_BA_SET_VOL_REQ,
    BTA_BA_SET_ENC_KEY,
    BTA_BA_CMD_SET_LT_ADDR,// all cmds come first and rsp come later
    BTA_BA_CMD_SEND_SYNC_TRAIN_PARAM,
    BTA_BA_CMD_VS_TX_CONFIG,
    BTA_BA_CMD_ENABLE_CSB,
    BTA_BA_CMD_START_SYNC_TRAIN,
    BTA_BA_CMD_DISABLE_CSB,
    BTA_BA_CMD_DELETE_LT_ADDR,
    BTA_BA_CMD_VS_VOL,
    BTA_BA_RSP_SET_LT_ADDR,
    BTA_BA_RSP_DELETE_LT_ADDR,
    BTA_BA_RSP_SEND_SYNC_TRAIN_PARAM,
    BTA_BA_RSP_VS_TX_CONFIG,
    BTA_BA_RSP_ENABLE_CSB,
    BTA_BA_RSP_START_SYNC_TRAIN,
    BTA_BA_RSP_DISABLE_CSB,
    BTA_BA_RSP_VS_VOL,
    BTA_BA_HCI_EVT_CSB_TIMEOUT,
    BTA_BA_DEREGISTER_REQ
};

// VS command parameters
// comamand opcode
#define VS_BA_CMD_OPCODE         0x0023
// sub opcode
#define VS_HCI_BAT_TX_CONFIG     0x01
#define VS_HCI_BAT_TX_VOL        0x02
// stream IDs
#define VS_HCI_STREAM_ID_VALID    0x01
#define VS_HCI_STREAM_ID_INVALID  0xFF
//codec Type
#define VS_HCI_CODEC_TYPE_CELT    0x00
//sampling frequency
#define VS_HCI_SAMPLING_FREQ_44   0x00
#define VS_HCI_SAMPLING_FREQ_48   0x01
// max packet size
#define VS_HCI_MAX_PACKET_SIZE_48 0xBA
// encoding scheme
#define VS_HCI_ENCODING_SCHEME_2_5 0x00
// TTP Offset
#define VS_HCI_TTP_OFFSET          0x03D090
//TX power level: HOST will always send FF and SOC
// would determine the right value based on soc type
#define VS_HCI_TX_POWER_LEVEL     0xFF
// Sample size
#define VS_HCI_SAMPLE_SIZE  512

#define HCI_ENABLE_CSB            0x01
#define HCI_DISABLE_CSB           0x00
#define HCI_CSB_PACKET_TYPE       0xEE1C

#define HCI_CSB_INTEVAL_LONG_MIN      1452
#define HCI_CSB_INTEVAL_LONG_MAX      1584
#define HCI_CSB_INTEVAL_SHORT     22
#define HCI_CSB_TIMEOUT           0xFFFE

#define HCI_SYNC_TRAIN_MIN_INTERVAL 0x0058
#define HCI_SYNC_TRAIN_MAX_INTERVAL 0x0630
#define HCI_SYNC_TRAIN_TIMEOUT      0x07FFFFFE

#define BTA_BA_STATE_DISABLED   1
#define BTA_BA_STATE_PAUSED    2
#define BTA_BA_STATE_STREAMING 3

#define BTA_HCI_CMD_SUCCESS 0
#define BTA_HCI_CMD_RETRY 1
#define BTA_HCI_CMD_FAILURE 2
/*******************************************************************************
 *  BTIF BA API
 ******************************************************************************/

//bta public apis, would be called from btif_ba
void BTA_PauseBA(bool, uint8_t* , uint8_t* ,uint8_t);
void BTA_BAEnable();
void BTA_BADisable();
void BTA_BARegister();
void BTA_BADeregister();
void BTA_StreamBA();
void BTA_StopBA();
void BTA_SetVol(uint8_t curr_vol_level);
void BTA_SetEncKey(uint8_t* enc_key);

void bta_ba_handle_hci_event(uint16_t event, uint8_t result, uint8_t* p_data,
                                               uint8_t data_len);
#endif /* BTA_BA_H */

