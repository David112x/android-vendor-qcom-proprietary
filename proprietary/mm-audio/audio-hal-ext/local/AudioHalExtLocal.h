/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef _AUDIO_HAL_EXT_LOCAL_H_
#define _AUDIO_HAL_EXT_LOCAL_H_

#include <log/log.h>
#include <stdlib.h>
#include <string>

#define DISALLOW_COPY_AND_ASSIGN(name) \
    name(const name&); \
    name &operator=(const name&)

typedef struct {
    bool snd_monitor_enabled;
    bool compress_capture_enabled;
    bool source_track_enabled;
    bool ssrec_enabled;
    bool audiosphere_enabled;
    bool afe_proxy_enabled;
    bool use_deep_buffer_as_primary_output;
    bool hdmi_edid_enabled;
    bool keep_alive_enabled;
    bool hifi_audio_enabled;
    bool receiver_aided_stereo;
    bool kpi_optimize_enabled;
    bool display_port_enabled;
    bool fluence_enabled;
    bool custom_stereo_enabled;
    bool anc_headset_enabled;
    bool spkr_prot_enabled;
    bool fm_power_opt_enabled;
    bool ext_qdsp_enabled;
    bool ext_spkr_enabled;
    bool ext_spkr_tfa_enabled;
    bool hwdep_cal_enabled;
    bool dsm_feedback_enabled;
    bool usb_offload_enabled;
    bool usb_offload_burst_mode;
    bool usb_offload_sidetone_vol_enabled;
    bool a2dp_offload_enabled;
    bool hfp_enabled;
    bool vbat_enabled;
    bool wsa_enabled;
    bool ext_hw_plugin_enabled;
    bool record_play_concurrency;
    bool hdmi_passthrough_enabled;
    bool concurrent_capture_enabled;
    bool compress_in_enabled;
    bool battery_listener_enabled;
    bool maxx_audio_enabled;
    bool compress_metadata_needed;
    bool incall_music_enabled;
    bool compress_voip_enabled;
    bool dynamic_ecns_enabled;
    bool audio_zoom_enabled;
} AHalValues_t;

struct AudioHalExtLocal {
public:
    AudioHalExtLocal() {};
    virtual ~AudioHalExtLocal() {};

    // to append local get config methods
    static AHalValues_t* getAHalConfigs();

private:
    //static ConfigParser *inst;
    static AHalValues_t ahal_configs;

    DISALLOW_COPY_AND_ASSIGN(AudioHalExtLocal);
};

extern "C" AHalValues_t* getAudioHalExtConfigs();

#endif  // _AUDIO_HAL_EXT_LOCAL_H_
