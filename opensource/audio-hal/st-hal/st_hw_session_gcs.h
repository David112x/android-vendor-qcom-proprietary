/* st_hw_session_gcs.h
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
#ifndef ST_HW_SESSION_GCS
#define ST_HW_SESSION_GCS

#include <pthread.h>
#include "sound_trigger_platform.h"
#include "st_hw_session.h"
#include "st_common_defs.h"

#define ST_GRAPHITE_LAB_PERIOD_COUNT (4)
#define ST_BYTES_PER_SAMPLE (2)
#define MAX_DETECTION_PAYLOAD_SZ 256

#define ST_GRAPHITE_LAB_PERIOD_SIZE_IN_SAMPLES \
    CALCULATE_PERIOD_SIZE(ST_GRAPHITE_LAB_BUF_DURATION_MS, \
     SOUND_TRIGGER_SAMPLING_RATE_16000, ST_GRAPHITE_LAB_PERIOD_COUNT, 32)

#define ST_GCS_READ_BUF_SIZE (ST_GRAPHITE_LAB_PERIOD_SIZE_IN_SAMPLES * \
                             ST_BYTES_PER_SAMPLE)

/*
 * The chosen theshold for determining FTRT vs. RT data is half the buffer
 * duration. There can be frames received that are partially FTRT and
 * partially RT, so the threshold should be less than the full buffer duration
 * to account for that usecase. However, if the threshold is too small, then
 * some issue in the lower layers could lead to false identification of RT
 * data.
 */
#define GCS_MAX_LAB_FTRT_FRAME_RD_TIME_NS \
    ((ST_GRAPHITE_LAB_BUF_DURATION_MS * NSECS_PER_MSEC) \
    / (2 * ST_GRAPHITE_LAB_PERIOD_COUNT))

struct sound_trigger_device;

typedef struct st_hw_session_gcs {
    st_hw_session_t common;
    struct st_gcs_params* gcs_usecase;
    uint32_t graph_handle;
    uint32_t loaded_sm_handle;
    void* nonpersistent_cal;
    size_t nonpersistent_cal_size;
    struct gcs_det_engine_start_custom_config *start_engine_cal;
    size_t start_engine_cal_size;

    uint8_t detect_payload[MAX_DETECTION_PAYLOAD_SZ];
    size_t detect_payload_size;

    bool detection_signaled;
    bool exit_buffering;
    bool exit_detection;
    bool lab_processing_active;

    pthread_t callback_thread;
    pthread_cond_t callback_thread_cond;
    pthread_mutex_t callback_thread_lock;

    pthread_mutex_t lock;
    pthread_cond_t cond;
    uint32_t read_rsp_cnt; /* keeps track of how many read responses were
        received so we know how many read_req to send when sender thread runs */
    uint32_t unread_bytes;
    uint32_t bytes_written;
    bool move_client_ptr;
    uint64_t frame_send_time;
    uint64_t frame_receive_time;

    uint8_t *mulaw_op_buf;
    FILE *lab_fp_gcs;
    FILE *lab_fp_client;
} st_hw_session_gcs_t;

int st_hw_sess_gcs_init(st_hw_session_t *const p_ses,
   hw_ses_event_callback_t cb, void *cookie, st_exec_mode_t exec_mode,
   struct st_vendor_info *v_info, sound_model_handle_t sm_handle,
   struct sound_trigger_device *stdev);

void st_hw_sess_gcs_deinit(st_hw_session_t *const p_ses);

int st_hw_gcs_init(void);

void st_hw_gcs_deinit(void);

int st_hw_gcs_load_wdsp(bool load);
#endif /* ST_HW_SESSION_GCS */
