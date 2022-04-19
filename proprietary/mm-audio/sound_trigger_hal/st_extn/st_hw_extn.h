/* st_hw_extn.h
 * Interface for sound trigger hal and st extn
 *
 * Copyright (c) 2016-2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

#ifndef SOUND_TRIGGER_HW_EXTN_H
#define SOUND_TRIGGER_HW_EXTN_H

#ifndef ST_EXTN_ENABLED
#define sthw_extn_process_detection_event_keyphrase(a, b, c, d, e, f) (0)
#define sthw_extn_check_process_det_ev_support() (false)
#else
int sthw_extn_process_detection_event_keyphrase(
    st_session_t *st_ses, uint64_t timestamp, int detect_status,
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
