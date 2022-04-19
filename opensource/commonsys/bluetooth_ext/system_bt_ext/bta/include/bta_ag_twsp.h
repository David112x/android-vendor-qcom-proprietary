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
#ifndef _BTA_AG_TWS_H_
#define _BTA_AG_TWS_H_

#include "btif_twsp_hf.h"
#include "bta_ag_int.h"

#define VS_QHCI_TWS_ESCO_SETUP_OPCODE 0x0D
#define VS_QHCI_TWS_ESCO_SETUP_SUBOPCODE 0x00
#define VS_TWS_SCO_SETUP_CMD_LEN 14

/* Unsolicited AT commands frm AG side
 * Request earbud State */
#define BTA_AG_TWS_QES_RES 101
//Request earbud mic quality
#define BTA_AG_TWS_QMQ_RES 102
//Request earbud battery status
#define BTA_AG_TWS_QBC_RES 103
//Request earbud dsp feature status
#define BTA_AG_TWS_QDSP_RES 104

/* Events originated from HF side
 * earbud State update from HF */
#define BTA_AG_TWSP_AT_QES_EVT 0x2001
//earbud mic quality update from HF
#define BTA_AG_TWSP_AT_QMQ_EVT 0x2002
//earbud battery status update from HF
#define BTA_AG_TWSP_AT_QBC_EVT 0x2003
#define BTA_AG_TWSP_AT_QER_EVT 0x2004
#define BTA_AG_TWSP_AT_QMD_EVT 0x2005
#define BTA_AG_TWSP_AT_QDSP_EVT 0x2006

//Mic path enable delay be 10 ms
#define MIC_PATH_ENABLE_DELAY 10
#define MIC_ENABLE 15
#define MIC_DISABLE 0

void bta_ag_twsp_sco_event(tBTA_AG_SCB* p_scb, uint8_t event);
bool is_twsp_device(const RawAddress& addr);
bool is_twsp_set(const RawAddress& addr1, const RawAddress& addr2);
void bta_ag_twsp_sco_conn_rsp(tBTA_AG_SCB* p_scb,
                         tBTM_ESCO_CONN_REQ_EVT_DATA* p_data);
void bta_ag_twsp_hfp_result(tBTA_AG_SCB* p_scb, tBTA_AG_API_RESULT* p_result);
tBTA_AG_SCB* get_other_twsp_scb (const RawAddress& addr);
void  dispatch_event_twsp_peer_device(tBTA_AG_SCB *p_scb, uint8_t event);
bool is_rfc_connected (tBTA_AG_SCB* p_scb);
bool twsp_sco_not_active(tBTA_AG_SCB* p_scb);
bool twsp_sco_active(tBTA_AG_SCB* p_scb);
bool is_twsp_connected();
void twsp_select_microphone(tBTA_AG_SCB *scb1, tBTA_AG_SCB *scb2);
void twsp_update_microphone_selection(tBTA_AG_SCB *curr_pscb,
                                             tBTA_AG_SCB *selected_pscb);

#endif//_BTA_AG_TWS_H_
