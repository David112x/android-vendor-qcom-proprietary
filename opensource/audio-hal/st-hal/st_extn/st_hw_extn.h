/* st_hw_extn.h
 * Interface for sound trigger hal and st extn
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

#ifndef SOUND_TRIGGER_HW_EXTN_H
#define SOUND_TRIGGER_HW_EXTN_H

#ifndef ST_EXTN_ENABLED
#define sthw_extn_process_detection_event_keyphrase(a, b, c, d, e, f) (0)
#define sthw_extn_check_process_det_ev_support() (false)
#else
int sthw_extn_process_detection_event_keyphrase(
    st_proxy_session_t *st_ses, uint64_t timestamp, int detect_status,
    void *payload, size_t payload_size,
    struct sound_trigger_phrase_recognition_event **event);
bool sthw_extn_check_process_det_ev_support();
#endif /* ST_EXTN_ENABLED */

enum sthw_extn_lpma_event_type {
    LPMA_EVENT_START,
    LPMA_EVENT_STOP,
    LPMA_EVENT_AUDIO_CONCURRENCY,
    LPMA_EVENT_ENABLE_DEVICE,
    LPMA_EVENT_DISABLE_DEVICE,
    LPMA_EVENT_CPE_STATUS_OFFLINE,
    LPMA_EVENT_CPE_STATUS_ONLINE,
    LPMA_EVENT_SLPI_STATUS_OFFLINE,
    LPMA_EVENT_SLPI_STATUS_ONLINE,
    LPMA_EVENT_TRANSIT_CPE_TO_APE,
    LPMA_EVENT_TRANSIT_APE_TO_CPE,
};

#ifndef ST_LMPA_EXTN_ENABLED
#define sthw_extn_lpma_present() (false)
#define sthw_extn_lpma_init(a) (0)
#define sthw_extn_lpma_deinit() (0)
#define sthw_extn_lpma_notify_event(event) (0)
#else
#define sthw_extn_lpma_present() (true)
int sthw_extn_lpma_init(struct sound_trigger_device *stdev);
void sthw_extn_lpma_deinit();
int sthw_extn_lpma_notify_event(enum sthw_extn_lpma_event_type event);
#endif

#endif /* SOUND_TRIGGER_HW_EXTN_H */
