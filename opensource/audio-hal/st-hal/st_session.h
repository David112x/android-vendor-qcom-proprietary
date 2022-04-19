/* st_session.h
 *
 * This file contains a sound trigger user session abstraction. This
 * abstraction represents a single st session from the application/framework
 * point of view.
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

#ifndef ST_SESSION_H
#define ST_SESSION_H

#include <stdint.h>
#include <pthread.h>
#include "st_hw_session.h"
#include "sound_trigger_platform.h"
#include "st_common_defs.h"

/* Below are the states that can be requested from the client */
enum client_states_t {
    ST_STATE_IDLE,
    ST_STATE_LOADED,
    ST_STATE_ACTIVE
};

typedef enum {
    ST_DET_LOW_POWER_MODE,
    ST_DET_HIGH_PERF_MODE,
    ST_DET_UNKNOWN_MODE = 0xFF,
}  st_det_perf_mode_t;

typedef enum st_session_event_id {
    ST_SES_EV_LOAD_SM,
    ST_SES_EV_UNLOAD_SM,
    ST_SES_EV_START,
    ST_SES_EV_RESTART,
    ST_SES_EV_STOP,
    ST_SES_EV_DETECTED,
    ST_SES_EV_READ_PCM,
    ST_SES_EV_END_BUFFERING,
    ST_SES_EV_SET_EXEC_MODE,
    ST_SES_EV_SSR_OFFLINE,
    ST_SES_EV_SSR_ONLINE,
    ST_SES_EV_PAUSE,
    ST_SES_EV_RESUME,
    ST_SES_EV_SEND_CHMIX_COEFF,
    ST_SES_EV_SET_DEVICE,
    ST_SES_EV_GET_PARAM_DATA,
    ST_SES_EV_DEFERRED_STOP,
    ST_SES_EV_REQUEST_DET,
} st_session_event_id_t;

struct sound_trigger_device;
struct st_session_ev;
typedef struct st_session_ev st_session_ev_t;

typedef struct st_session st_session_t;
typedef struct st_proxy_session st_proxy_session_t;
typedef int (*st_proxy_session_state_fn_t)(st_proxy_session_t*,
                                           st_session_ev_t *ev);

struct sound_model_info {
    unsigned char *sm_data;
    unsigned int sm_size;
    sound_trigger_sound_model_type_t sm_type;
    unsigned int num_keyphrases;
    unsigned int num_users;
    char **keyphrases;
    char **users;
    char **cf_levels_kw_users;
    unsigned char *cf_levels;
    unsigned char *det_cf_levels;
    unsigned int cf_levels_size;
    bool sm_merged;
};

struct st_session {
    /* TODO: decouple device below from session */
    struct listnode list_node;
    struct listnode transit_list_node;
    struct listnode hw_list_node;

    struct sound_trigger_device *stdev;
    struct st_vendor_info *vendor_uuid_info;

    pthread_mutex_t lock;
    st_exec_mode_t exec_mode;
    st_exec_mode_t ssr_transit_exec_mode;
    struct sound_trigger_phrase_sound_model *phrase_sm;
    struct sound_trigger_recognition_config *rc_config;
    sound_trigger_sound_model_type_t sm_type;
    sound_model_handle_t sm_handle;
    recognition_callback_t callback;
    void *cookie;
    audio_io_handle_t capture_handle;
    bool capture_requested;

    unsigned int num_phrases;
    unsigned int num_users;
    unsigned int recognition_mode;
    enum client_states_t state;
    bool paused;
    bool pending_stop;
    bool pending_load;
    bool pending_set_device;
    bool detection_sent;
    st_det_perf_mode_t client_req_det_mode;
    unsigned int hist_buf_duration;
    unsigned int preroll_duration;

    struct listnode second_stage_list;
    uint32_t conf_levels_intf_version;
    void *st_conf_levels;

    st_proxy_session_t *hw_proxy_ses;
    struct sound_model_info sm_info;
};

struct st_proxy_session {
    struct listnode clients_list; /* Attached client sessions */
    struct sound_trigger_device *stdev;
    struct st_vendor_info *vendor_uuid_info;

    pthread_mutex_t lock;
    st_exec_mode_t exec_mode;
    bool enable_trans;

    struct sound_trigger_recognition_config *rc_config;
    sound_trigger_sound_model_type_t sm_type;
    sound_model_handle_t sm_handle;
    bool lab_enabled;
    unsigned int recognition_mode;

    st_hw_session_t *hw_ses_cpe; /* cpe hw session */
    st_hw_session_t *hw_ses_adsp; /* adsp hw session */
    st_hw_session_t *hw_ses_arm; /* arm hw session */
    st_hw_session_t *hw_ses_current; /* current hw session, this is set every
        time there is an exec_mode change and points to one of the above
        hw sessions */
    st_hw_session_t *hw_ses_prev; /* cached hw_ses_current,
        used for WDSP<->ADSP transitions */
    st_session_t *det_stc_ses; /* Current detected client */

    /*
     * flag gets set if user restarts
     * session right after detection before we have a chance to stop the
     * session
     */
    bool hw_session_started;

    st_proxy_session_state_fn_t current_state;
    bool device_disabled;

    pthread_t aggregator_thread;
    pthread_mutex_t ss_detections_lock;
    pthread_cond_t ss_detections_cond;
    bool aggregator_thread_created;
    bool exit_aggregator_loop;
    bool enable_second_stage;
    st_session_ev_t *det_session_ev;
    int rc_config_update_counter;
    bool detection_requested;

    struct sound_model_info sm_info;
    FILE *lab_fp;
    uint64_t detection_event_time;
};

/*
 * Initialzies a sound trigger session. Must be called before
 * any other opertaions.
 * Parameters:
 * use_gcs TRUE indicates that GCS should be used for CPE HW
 * session (WCD9340 and beyond) otherwise use
 * LSM(WCD9335 and earlier)
 * exec_mode Indicates initial execution mode for the st
 *  session, whether it is in CPE or ADSP
 */
int st_session_init(st_session_t *st_ses, struct sound_trigger_device *stdev,
    st_exec_mode_t exec_mode, sound_model_handle_t sm_handle);
int st_session_deinit(st_session_t *);
int st_session_ss_init(st_session_t *st_ses);
int st_session_ss_deinit(st_session_t *st_ses);

int st_session_load_sm(st_session_t *st_ses);
int st_session_start(st_session_t *st_ses);
int st_session_unload_sm(st_session_t *st_ses);
int st_session_stop(st_session_t *st_ses);
int st_session_read_pcm(st_session_t *st_ses, uint8_t *buff,
    size_t buff_size, /*out*/ size_t *read_size);
int st_session_stop_lab(st_session_t *st_ses);

int st_session_ssr_offline(st_session_t *st_ses,
   enum ssr_event_status ssr_type);
int st_session_ssr_online(st_session_t *st_ses,
   enum ssr_event_status ssr_type);
int st_session_pause(st_session_t *st_ses);
int st_session_resume(st_session_t *st_ses);
int st_session_restart(st_session_t *st_ses);
int st_session_send_custom_chmix_coeff(st_session_t *st_ses, char *str);
int st_session_get_config(st_session_t *st_ses, struct pcm_config *config);
int st_session_enable_device(st_session_t *st_ses);
int st_session_disable_device(st_session_t *st_ses);
bool st_session_is_detected(st_session_t *st_ses);
bool st_session_is_active(st_session_t *st_ses);
bool st_session_is_buffering(st_session_t *st_ses);
bool st_session_is_ssr_state(st_session_t *st_ses);
int st_session_set_exec_mode(st_session_t *st_ses, st_exec_mode_t exec);
int st_session_get_param_data(st_session_t *st_ses, const char *param,
    void *payload, size_t payload_size, size_t *param_data_size);
int st_session_request_detection(st_session_t *st_ses);
int st_session_update_recongition_config(st_session_t *st_ses);
int st_session_get_preroll(st_session_t *st_ses);

int process_detection_event_keyphrase_v2(
    st_proxy_session_t *st_ses, int detect_status,
    void *payload, size_t payload_size,
    struct sound_trigger_phrase_recognition_event **event);

#endif /* ST_SESSION_H */
