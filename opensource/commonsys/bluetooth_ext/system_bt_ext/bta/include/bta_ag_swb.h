/*
 * Copyright (c) 2019, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
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

#ifndef _BTA_AG_SWB_H_
#define _BTA_AG_SWB_H_

#include "bta_ag_int.h"
#include "device/include/esco_parameters.h"

#define BTA_AG_SCO_SWB_SETTINGS_Q0_MASK 4
#define BTA_AG_SCO_SWB_SETTINGS_Q1_MASK 8
#define BTA_AG_SCO_SWB_SETTINGS_Q2_MASK 16
#define BTA_AG_SCO_SWB_SETTINGS_Q3_MASK 32

/* Events originated from HF side */
#define  BTA_AG_AT_QAC_EVT 253
#define  BTA_AG_AT_QCS_EVT 254
#define  BTA_AG_SWB_EVT 100 /* SWB SCO codec info */
#define  BTA_AG_LOCAL_RES_QAC 0x108
#define  BTA_AG_LOCAL_RES_QCS 0x109

#define LEGACY_CODECS 2
#define SWB_ESCO_NUM_CODECS 4

void bta_ag_swb_handle_vs_at_events(tBTA_AG_SCB* p_scb, uint16_t cmd, int16_t int_arg, tBTA_AG_VAL val);
void bta_ag_send_qac(tBTA_AG_SCB* p_scb, tBTA_AG_DATA* p_data);
void bta_ag_send_qcs(tBTA_AG_SCB* p_scb, tBTA_AG_DATA* p_data);
tBTA_AG_PEER_CODEC bta_ag_parse_qac(tBTA_AG_SCB* p_scb, char* p_s);

const enh_esco_params_t default_esco_swb_parameters[SWB_ESCO_NUM_CODECS] = {
     // ESCO_CODEC_SWB_Q0
     {.transmit_bandwidth = TXRX_64KBITS_RATE,
     .receive_bandwidth = TXRX_64KBITS_RATE,
     .transmit_coding_format = {.coding_format = ESCO_CODING_FORMAT_VS,
                                .company_id = 0x000A,
                                .vendor_specific_codec_id = 0x0000},
     .receive_coding_format = {.coding_format = ESCO_CODING_FORMAT_VS,
                               .company_id = 0x000A,
                               .vendor_specific_codec_id = 0x0000},
     .transmit_codec_frame_size = 60,
     .receive_codec_frame_size = 60,
     .input_bandwidth = INPUT_OUTPUT_128K_RATE,
     .output_bandwidth = INPUT_OUTPUT_128K_RATE,
     .input_coding_format = {.coding_format = ESCO_CODING_FORMAT_LINEAR,
                             .company_id = 0x0000,
                             .vendor_specific_codec_id = 0x0000},
     .output_coding_format = {.coding_format = ESCO_CODING_FORMAT_LINEAR,
                              .company_id = 0x0000,
                              .vendor_specific_codec_id = 0x0000},
     .input_coded_data_size = 16,
     .output_coded_data_size = 16,
     .input_pcm_data_format = ESCO_PCM_DATA_FORMAT_2_COMP,
     .output_pcm_data_format = ESCO_PCM_DATA_FORMAT_2_COMP,
     .input_pcm_payload_msb_position = 0,
     .output_pcm_payload_msb_position = 0,
     .input_data_path = ESCO_DATA_PATH_PCM,
     .output_data_path = ESCO_DATA_PATH_PCM,
     .input_transport_unit_size = 0x00,
     .output_transport_unit_size = 0x00,
     .max_latency_ms = 14,
     .packet_types = 0x0380,
     .retransmission_effort = ESCO_RETRANSMISSION_QUALITY},
     // ESCO_CODEC_SWB_Q1
     {.transmit_bandwidth = TXRX_64KBITS_RATE,
     .receive_bandwidth = TXRX_64KBITS_RATE,
     .transmit_coding_format = {.coding_format = ESCO_CODING_FORMAT_VS,
                                .company_id = 0x000A,
                                .vendor_specific_codec_id = 0x0000},
     .receive_coding_format = {.coding_format = ESCO_CODING_FORMAT_VS,
                               .company_id = 0x000A,
                               .vendor_specific_codec_id = 0x0000},
     .transmit_codec_frame_size = 60,
     .receive_codec_frame_size = 60,
     .input_bandwidth = INPUT_OUTPUT_128K_RATE,
     .output_bandwidth = INPUT_OUTPUT_128K_RATE,
     .input_coding_format = {.coding_format = ESCO_CODING_FORMAT_LINEAR,
                             .company_id = 0x0000,
                             .vendor_specific_codec_id = 0x0000},
     .output_coding_format = {.coding_format = ESCO_CODING_FORMAT_LINEAR,
                              .company_id = 0x0000,
                              .vendor_specific_codec_id = 0x0000},
     .input_coded_data_size = 16,
     .output_coded_data_size = 16,
     .input_pcm_data_format = ESCO_PCM_DATA_FORMAT_2_COMP,
     .output_pcm_data_format = ESCO_PCM_DATA_FORMAT_2_COMP,
     .input_pcm_payload_msb_position = 0,
     .output_pcm_payload_msb_position = 0,
     .input_data_path = ESCO_DATA_PATH_PCM,
     .output_data_path = ESCO_DATA_PATH_PCM,
     .input_transport_unit_size = 0x00,
     .output_transport_unit_size = 0x00,
     .max_latency_ms = 14,
     .packet_types = 0x0380,
     .retransmission_effort = ESCO_RETRANSMISSION_QUALITY},
     // ESCO_CODEC_SWB_Q2
     {.transmit_bandwidth = TXRX_64KBITS_RATE,
     .receive_bandwidth = TXRX_64KBITS_RATE,
     .transmit_coding_format = {.coding_format = ESCO_CODING_FORMAT_VS,
                                .company_id = 0x000A,
                                .vendor_specific_codec_id = 0x0000},
     .receive_coding_format = {.coding_format = ESCO_CODING_FORMAT_VS,
                               .company_id = 0x000A,
                               .vendor_specific_codec_id = 0x0000},
     .transmit_codec_frame_size = 60,
     .receive_codec_frame_size = 60,
     .input_bandwidth = INPUT_OUTPUT_128K_RATE,
     .output_bandwidth = INPUT_OUTPUT_128K_RATE,
     .input_coding_format = {.coding_format = ESCO_CODING_FORMAT_LINEAR,
                             .company_id = 0x0000,
                             .vendor_specific_codec_id = 0x0000},
     .output_coding_format = {.coding_format = ESCO_CODING_FORMAT_LINEAR,
                              .company_id = 0x0000,
                              .vendor_specific_codec_id = 0x0000},
     .input_coded_data_size = 16,
     .output_coded_data_size = 16,
     .input_pcm_data_format = ESCO_PCM_DATA_FORMAT_2_COMP,
     .output_pcm_data_format = ESCO_PCM_DATA_FORMAT_2_COMP,
     .input_pcm_payload_msb_position = 0,
     .output_pcm_payload_msb_position = 0,
     .input_data_path = ESCO_DATA_PATH_PCM,
     .output_data_path = ESCO_DATA_PATH_PCM,
     .input_transport_unit_size = 0x00,
     .output_transport_unit_size = 0x00,
     .max_latency_ms = 14,
     .packet_types = 0x0380,
     .retransmission_effort = ESCO_RETRANSMISSION_QUALITY},
     // ESCO_CODEC_SWB_Q3
     {.transmit_bandwidth = TXRX_64KBITS_RATE,
     .receive_bandwidth = TXRX_64KBITS_RATE,
     .transmit_coding_format = {.coding_format = ESCO_CODING_FORMAT_VS,
                                .company_id = 0x000A,
                                .vendor_specific_codec_id = 0x0000},
     .receive_coding_format = {.coding_format = ESCO_CODING_FORMAT_VS,
                               .company_id = 0x000A,
                               .vendor_specific_codec_id = 0x0000},
     .transmit_codec_frame_size = 60,
     .receive_codec_frame_size = 60,
     .input_bandwidth = INPUT_OUTPUT_128K_RATE,
     .output_bandwidth = INPUT_OUTPUT_128K_RATE,
     .input_coding_format = {.coding_format = ESCO_CODING_FORMAT_LINEAR,
                             .company_id = 0x0000,
                             .vendor_specific_codec_id = 0x0000},
     .output_coding_format = {.coding_format = ESCO_CODING_FORMAT_LINEAR,
                              .company_id = 0x0000,
                              .vendor_specific_codec_id = 0x0000},
     .input_coded_data_size = 16,
     .output_coded_data_size = 16,
     .input_pcm_data_format = ESCO_PCM_DATA_FORMAT_2_COMP,
     .output_pcm_data_format = ESCO_PCM_DATA_FORMAT_2_COMP,
     .input_pcm_payload_msb_position = 0,
     .output_pcm_payload_msb_position = 0,
     .input_data_path = ESCO_DATA_PATH_PCM,
     .output_data_path = ESCO_DATA_PATH_PCM,
     .input_transport_unit_size = 0x00,
     .output_transport_unit_size = 0x00,
     .max_latency_ms = 14,
     .packet_types = 0x0380,
     .retransmission_effort = ESCO_RETRANSMISSION_QUALITY}
};

#endif//_BTA_AG_SWB_H_
