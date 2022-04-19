/* st_graphite_api.h
 *
 * This file defines all the APIs for interacting with Graphite service and
 * modules.
 *
 * Copyright (c) 2016-2019, The Linux Foundation. All rights reserved.
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
#include "graphite_common.h"

#define GCS_DETECTION_ENGINE_CONFIG_KW_DET_ENABLE 0x1
#define GCS_DETECTION_ENGINE_CONFIG_USER_VER_ENABLE 0x2
#define GCS_DETECTION_ENGINE_CONFIG_FAILURE_DET_ENABLE 0x4

#define GCS_DETECTION_ENGINE_EVENT_DETECTED 0
#define GCS_DETECTION_ENGINE_EVENT_FAILED 1

#define GCS_READ_CMDRSP_STATUS_SUCCESS 0

struct gcs_det_engine_config_param {
    struct graphite_cal_header cal_hdr;
    uint16_t mode;
    uint16_t custom_payload_sz;
} __packed;

struct gcs_det_engine_custom_config_param {
    struct graphite_cal_header cal_hdr;
};

struct gcs_det_engine_event {
    uint16_t status;
    uint16_t custom_payload_sz;
    /* followed by uint8_t*custom_payload_sz for conf levels */
};

struct gcs_cmd_read_payload_t {
    uint32_t size_in_bytes;
};

struct gcs_cmd_readrsp_payload_t {
    uint32_t status;
    uint32_t flags;
    uint32_t timestamp_lsw;
    uint32_t timestapm_msw;
};

struct gcs_data_cmd_t {
    struct graphite_data_cmd_hdr hdr;
    union {
        struct gcs_cmd_read_payload_t read;
    } payload;
} __packed;

#define MAX_BRDIGE_BUF_PORTS (8)

struct gcs_bridge_buf_payload {
    uint32_t minor_version;
    uint32_t addr_circbuf_info[MAX_BRDIGE_BUF_PORTS];
} __packed;

struct gcs_bridge_buf_params {
    struct graphite_cal_header cal_hdr;
    struct gcs_bridge_buf_payload payload;
} __packed;

/* custom params defined by STHAL to VA engine */

/*
 * start and stop individual engines in single
 * module
 */
struct gcs_det_engine_start_custom_config {
    struct graphite_cal_header cal_hdr;
    uint16_t minor_version; /* Param version. Currently fixed value 1 */
    uint16_t enable; /* 1: start engine 0: stop engine */
    uint32_t token; /* unique token per engine */
} __packed;

struct gcs_det_engine_start_params {
    uint16_t minor_version; /* Param version. Currently fixed value 1 */
    uint32_t token; /* unique token per engine sent in
                       @gcs_det_engine_start_custom_config*/
} __packed;

struct gcs_det_event_type_custom_config {
    struct graphite_cal_header cal_hdr;
    uint32_t minor_version; /* Param version. Currently fixed value 1 */
    uint32_t event_type; /* Value 1 selects generic event */
    uint32_t mode; /* Bit 0 enables confidence levels. Bit 1 enables
                        keyword start/end indices */
} __packed;

/*
 * Expected format of event when used with
 * @gcs_det_engine_start_custom_config
 */
struct gcs_det_engine_extended_event {
    struct gcs_det_engine_event ev_params;
    struct gcs_det_engine_start_params start_params;
    /* followed by VA engine specific custom payload */
} __packed;

