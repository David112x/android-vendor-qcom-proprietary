/* st_hw_extn.c
 * This file contains extended functionality of
 * sound_trigger_hw.
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

#define LOG_TAG "sound_trigger_hw_extn"
/* #define LOG_NDEBUG 0 */
#define LOG_NDDEBUG 0

/* #define VERY_VERBOSE_LOGGING */
#ifdef VERY_VERBOSE_LOGGING
#define ALOGVV ALOGV
#else
#define ALOGVV(a...) do { } while(0)
#endif

#include <errno.h>
#include <stdlib.h>
#include <cutils/log.h>
#include <inttypes.h>

#include "st_session.h"
#include "st_hw_defs.h"

static int generate_sound_trigger_phrase_recognition_event_v3
(
    const struct sound_trigger_phrase_sound_model *phrase_sm,
    const struct sound_trigger_recognition_config *rc_config,
    const void *payload,
    unsigned int payload_size,
    qsthw_recognition_event_type_t event_type,
    void **out_rc_event
)
{
    unsigned int i = 0, j = 0, user_id = 0;
    int rc = 0;

    ALOGD("%s: Enter payload_size %d", __func__, payload_size);

    if (!payload || !phrase_sm || !rc_config || !out_rc_event) {
        ALOGE("%s: Null params", __func__);
        return -EINVAL;
    }

    *out_rc_event = NULL;

    switch (event_type) {
    case QSTHW_RC_EVENT_TYPE_TIMESTAMP: {
        struct qsthw_phrase_recognition_event *event;
        struct sound_trigger_phrase_recognition_event *phrase_event;
        event = calloc(1, sizeof(*event) + payload_size);
        if (!event) {
            ALOGE("%s: event allocation failed size %d",
                   __func__, payload_size);
            rc = -ENODEV;
            break;
        }

        phrase_event = &event->phrase_event;

        phrase_event->num_phrases = rc_config->num_phrases;
        phrase_event->common.data_offset = sizeof(*event);
        phrase_event->common.data_size = payload_size;

        /* fill confidence levels */
        for (i = 0; i < rc_config->num_phrases; i++) {
             phrase_event->phrase_extras[i].id = rc_config->phrases[i].id;
             phrase_event->phrase_extras[i].recognition_modes =
                                 phrase_sm->phrases[0].recognition_mode;
             phrase_event->phrase_extras[i].confidence_level =
                                 ((char *)payload)[i];
             phrase_event->phrase_extras[i].num_levels =
                                 rc_config->phrases[i].num_levels;
             for (j = 0; j < rc_config->phrases[i].num_levels; j++) {
                  user_id = rc_config->phrases[i].levels[j].user_id;
                  phrase_event->phrase_extras[i].levels[j].user_id = user_id;
                  phrase_event->phrase_extras[i].levels[j].level =
                                             ((char *)payload)[user_id];
             }
        }

        /* Copy payload to event using offset generated above */
        memcpy((char *)event + phrase_event->common.data_offset,
               payload, payload_size);

        *out_rc_event = (struct qsthw_phrase_recognition_event *)event;
        break;
    }
    default:
        ALOGE("%s: Invalid event type passed %d", __func__, event_type);
        rc = -EINVAL;
        break;
    }
    return rc;
}

bool sthw_extn_check_process_det_ev_support()
{
    return true;
}

/* recognition event with timestamp */
int sthw_extn_process_detection_event_keyphrase(
    st_proxy_session_t *st_ses, uint64_t timestamp, int detect_status,
    void *payload, size_t payload_size,
    struct sound_trigger_phrase_recognition_event **event)
{
    st_hw_session_t *st_hw_ses = st_ses->hw_ses_current;
    st_session_t *stc_ses = st_ses->det_stc_ses;
    struct qsthw_phrase_recognition_event *local_event = NULL;
    struct st_vendor_info *v_info = st_ses->vendor_uuid_info;
    struct sound_trigger_phrase_sound_model *phrase_sm = stc_ses->phrase_sm;

    int status = 0;
    unsigned int i, j;
    qsthw_recognition_event_type_t event_type = QSTHW_RC_EVENT_TYPE_TIMESTAMP;

    if (st_hw_ses->is_generic_event) {
        /*
         * For generic event, reusing the st_session v2 event generation.
         * Note: free memory allocated from v2 event generation after forming local event.
         */
        struct sound_trigger_phrase_recognition_event *generic_phrase_event = NULL;

        status = process_detection_event_keyphrase_v2(st_ses, detect_status, payload,
                                                   payload_size, &generic_phrase_event);
        if (status) {
            ALOGE("%s: process_detection_event_keyphrase_v2 failed, size %zd", __func__,
                  payload_size);
            goto exit;
        }

        local_event = calloc(1, sizeof(*local_event) + generic_phrase_event->common.data_size);
        if (!local_event) {
            ALOGE("%s: event allocation failed, size %u", __func__,
                  generic_phrase_event->common.data_size);
            status = -ENOMEM;
            free(generic_phrase_event);
            goto exit;
        }

        memcpy(&local_event->phrase_event, generic_phrase_event,
               sizeof(*generic_phrase_event));
        local_event->phrase_event.common.data_offset = sizeof(*local_event);
        memcpy((char *)local_event + local_event->phrase_event.common.data_offset,
               (char *)generic_phrase_event + generic_phrase_event->common.data_offset,
               generic_phrase_event->common.data_size);
        free(generic_phrase_event);
        *event = &local_event->phrase_event;
        goto exit;
    } else if (v_info->is_qcva_uuid) {
        /* if smlib is present, get the event from it, else send the
         * DSP received payload as it is to App
         */
        status = generate_sound_trigger_phrase_recognition_event_v3(
            phrase_sm, stc_ses->rc_config, payload, payload_size,
            event_type, (void *)&local_event);

        if (status || !local_event) {
            ALOGW("%s: smlib fill recognition event failed, status %d",
                  __func__, status);
            goto exit;
        }
    } else {
        local_event = calloc(1, sizeof(*local_event) + payload_size);
        if (!local_event) {
            ALOGE("%s: event allocation failed, size %zd", __func__,
                  payload_size);
            status = -ENOMEM;
            goto exit;
        }

        memcpy(local_event->phrase_event.phrase_extras,
            stc_ses->rc_config->phrases,
            stc_ses->rc_config->num_phrases *
            sizeof(struct sound_trigger_phrase_recognition_extra));

        local_event->phrase_event.num_phrases = stc_ses->rc_config->num_phrases;
        local_event->phrase_event.common.data_offset = sizeof(*local_event);
        local_event->phrase_event.common.data_size = payload_size;
        memcpy((char *)local_event + local_event->phrase_event.common.data_offset,
               payload, payload_size);
    }

    /* fill the remaining recognition event parameters not specific
     * to soundmodel lib
     */
    local_event->timestamp = timestamp;

    local_event->phrase_event.common.status = detect_status;
    local_event->phrase_event.common.type = phrase_sm->common.type;;
    local_event->phrase_event.common.model = stc_ses->sm_handle;
    local_event->phrase_event.common.capture_available = stc_ses->capture_requested;
    local_event->phrase_event.common.capture_delay_ms = 0;
    local_event->phrase_event.common.capture_preamble_ms = 0;
    local_event->phrase_event.common.audio_config.sample_rate =
                 SOUND_TRIGGER_SAMPLING_RATE_16000;
    local_event->phrase_event.common.audio_config.channel_mask =
                 AUDIO_CHANNEL_IN_MONO;
    local_event->phrase_event.common.audio_config.format =
                 AUDIO_FORMAT_PCM_16_BIT;

    for (i = 0; i < local_event->phrase_event.num_phrases; i++) {
        ALOGVV("%s: [%d] kw_id %d level %d", __func__,i,
              local_event->phrase_event.phrase_extras[i].id,
              local_event->phrase_event.phrase_extras[i].confidence_level);
        for (j = 0; j < local_event->phrase_event.phrase_extras[i].num_levels; j++) {
            ALOGVV("%s: [%d] user_id %d level %d ", __func__, i,
                  local_event->phrase_event.phrase_extras[i].levels[j].user_id,
                  local_event->phrase_event.phrase_extras[i].levels[j].level);
        }
    }

    ALOGI("%s:[c%d] send keyphrase recognition event %d", __func__,
                     stc_ses->sm_handle, detect_status);
    ALOGV("%s:[c%d] status=%d, type=%d, model=%d, capture_avaiable=%d, "
           "num_phrases=%d, id=%d, timestamp = %" PRIu64,
           __func__, stc_ses->sm_handle, local_event->phrase_event.common.status,
           local_event->phrase_event.common.type, local_event->phrase_event.common.model,
           local_event->phrase_event.common.capture_available, local_event->phrase_event.num_phrases,
           local_event->phrase_event.phrase_extras[0].id, local_event->timestamp);

    *event = &local_event->phrase_event;
exit:
    return status;
}