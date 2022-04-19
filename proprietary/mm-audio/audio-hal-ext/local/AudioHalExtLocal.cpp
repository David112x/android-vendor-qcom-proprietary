/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#define LOG_TAG "AudioHalExtLocal"
//#define LOG_NDDEBUG 0

#include "AudioHalExtLocal.h"
#include "ConfigParser.h"

inline
bool toBool(const char *string, bool default_value) {
    bool result = default_value;

    int len = strlen(string);
    if (len == 1) {
        if (string[0] == '0' || string[0] == 'n')
            result = false;
        else if (string[0] == '1' || string[0] == 'y')
            result = true;
    } else if (len > 1) {
        if (!strcmp(string, "no") ||
            !strcmp(string, "false") ||
            !strcmp(string, "off") ||
            !strcmp(string, "disabled"))
            result = false;
        else if (!strcmp(string, "yes") ||
                 !strcmp(string, "true") ||
                 !strcmp(string, "on") ||
                 !strcmp(string, "enabled"))
            result = true;
    }

    return result;
}

/* initialization of static member veriables */
AHalValues_t AudioHalExtLocal::ahal_configs = {
                  true,  // snd_monitor_enabled
                  false, // compress_capture_enabled
                  false, // source_track_enabled
                  false, // ssrec_enabled
                  false, // audiosphere_enabled
                  false, // afe_proxy_enabled
                  false, // use_deep_buffer_as_primary_output
                  false, // hdmi_edid_enabled
                  false, // keep_alive_enabled
                  false, // hifi_audio_enabled
                  false, // receiver_aided_stereo
                  false, // kpi_optimize_enabled
                  false, // display_port_enabled
                  false, // fluence_enabled
                  false, // custom_stereo_enabled
                  false, // anc_headset_enabled
                  true,  // spkr_prot_enabled
                  false, // fm_power_opt_enabled
                  true,  // ext_qdsp_enabled
                  true,  // ext_spkr_enabled
                  false, // ext_spkr_tfa_enabled
                  true,  // hwdep_cal_enabled
                  false, // dsm_feedback_enabled
                  true,  // usb_offload_enabled
                  false, // usb_offload_burst_mode
                  false, // usb_offload_sidetone_vol_enabled
                  true,  // a2dp_offload_enabled
                  true,  // hfp_enabled
                  false, // vbat_enabled
                  false, // wsa_enabled
                  true,  // ext_hw_plugin_enabled
                  false, // record_play_concurrency
                  true,  // hdmi_passthrough_enabled
                  true,  // concurrent_capture_enabled
                  true,  // compress_in_enabled
                  true,  // battery_listener_enabled
                  false, // maxx_audio_enabled
                  false, // compress_metadata_needed
                  true,  // incall_music_enabled
                  false, // compress_voip_enabled
                  false, // dynamic_ecns_enabled
                  false, // audio_zoom_enabled
              };

AHalValues_t* AudioHalExtLocal::getAHalConfigs() {
    ALOGV("AudioHalExtLocal::getAHalConfigs");
    ConfigParser *inst = ConfigParser::getInstance();
    if (inst == NULL) {
        ALOGE("failed to acquire config instance");
        return NULL;
    }

    // flags
    if (inst->mConfigMap.find("snd_monitor_enabled") != inst->mConfigMap.end()) {
        ahal_configs.snd_monitor_enabled =
            toBool(inst->mConfigMap["snd_monitor_enabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("compress_capture_enabled") != inst->mConfigMap.end()) {
        ahal_configs.compress_capture_enabled =
            toBool(inst->mConfigMap["compress_capture_enabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("source_track_enabled") != inst->mConfigMap.end()) {
        ahal_configs.source_track_enabled =
            toBool(inst->mConfigMap["source_track_enabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("ssrec_enabled") != inst->mConfigMap.end()) {
        ahal_configs.ssrec_enabled =
            toBool(inst->mConfigMap["ssrec_enabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("audiosphere_enabled") != inst->mConfigMap.end()) {
        ahal_configs.audiosphere_enabled =
            toBool(inst->mConfigMap["audiosphere_enabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("afe_proxy_enabled") != inst->mConfigMap.end()) {
        ahal_configs.afe_proxy_enabled =
            toBool(inst->mConfigMap["afe_proxy_enabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("use_deep_buffer_as_primary_output") != inst->mConfigMap.end()) {
        ahal_configs.use_deep_buffer_as_primary_output =
            toBool(inst->mConfigMap["use_deep_buffer_as_primary_output"].c_str(), false);
    }
    if (inst->mConfigMap.find("hdmi_edid_enabled") != inst->mConfigMap.end()) {
        ahal_configs.hdmi_edid_enabled =
            toBool(inst->mConfigMap["hdmi_edid_enabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("keep_alive_enabled") != inst->mConfigMap.end()) {
        ahal_configs.keep_alive_enabled =
            toBool(inst->mConfigMap["keep_alive_enabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("hifi_audio_enabled") != inst->mConfigMap.end()) {
        ahal_configs.hifi_audio_enabled =
            toBool(inst->mConfigMap["hifi_audio_enabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("receiver_aided_stereo") != inst->mConfigMap.end()) {
        ahal_configs.receiver_aided_stereo =
            toBool(inst->mConfigMap["receiver_aided_stereo"].c_str(), false);
    }
    if (inst->mConfigMap.find("kpi_optimize_enabled") != inst->mConfigMap.end()) {
        ahal_configs.kpi_optimize_enabled =
            toBool(inst->mConfigMap["kpi_optimize_enabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("display_port_enabled") != inst->mConfigMap.end()) {
        ahal_configs.display_port_enabled =
            toBool(inst->mConfigMap["display_port_enabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("fluence_enabled") != inst->mConfigMap.end()) {
        ahal_configs.fluence_enabled =
            toBool(inst->mConfigMap["fluence_enabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("custom_stereo_enabled") != inst->mConfigMap.end()) {
        ahal_configs.custom_stereo_enabled =
            toBool(inst->mConfigMap["custom_stereo_enabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("anc_headset_enabled") != inst->mConfigMap.end()) {
        ahal_configs.anc_headset_enabled =
            toBool(inst->mConfigMap["anc_headset_enabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("spkr_prot_enabled") != inst->mConfigMap.end()) {
        ahal_configs.spkr_prot_enabled =
            toBool(inst->mConfigMap["spkr_prot_enabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("fm_power_opt") != inst->mConfigMap.end()) {
        ahal_configs.fm_power_opt_enabled =
            toBool(inst->mConfigMap["fm_power_opt"].c_str(), false);
    }
    if (inst->mConfigMap.find("ext_qdsp_enabled") != inst->mConfigMap.end()) {
        ahal_configs.ext_qdsp_enabled =
            toBool(inst->mConfigMap["ext_qdsp_enabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("ext_spkr_enabled") != inst->mConfigMap.end()) {
        ahal_configs.ext_spkr_enabled =
            toBool(inst->mConfigMap["ext_spkr_enabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("ext_spkr_tfa_enabled") != inst->mConfigMap.end()) {
        ahal_configs.ext_spkr_tfa_enabled =
            toBool(inst->mConfigMap["ext_spkr_tfa_enabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("hwdep_cal_enabled") != inst->mConfigMap.end()) {
        ahal_configs.hwdep_cal_enabled =
            toBool(inst->mConfigMap["hwdep_cal_enabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("dsm_feedback_enabled") != inst->mConfigMap.end()) {
        ahal_configs.dsm_feedback_enabled =
            toBool(inst->mConfigMap["dsm_feedback_enabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("usb_offload_enabled") != inst->mConfigMap.end()) {
        ahal_configs.usb_offload_enabled =
            toBool(inst->mConfigMap["usb_offload_enabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("usb_offload_burst_mode") != inst->mConfigMap.end()) {
        ahal_configs.usb_offload_burst_mode =
            toBool(inst->mConfigMap["usb_offload_burst_mode"].c_str(), false);
    }
    if (inst->mConfigMap.find("usb_offload_sidetone_vol_enabled") != inst->mConfigMap.end()) {
        ahal_configs.usb_offload_sidetone_vol_enabled =
            toBool(inst->mConfigMap["usb_offload_sidetone_vol_enabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("a2dp_offload_enabled") != inst->mConfigMap.end()) {
        ahal_configs.a2dp_offload_enabled =
            toBool(inst->mConfigMap["a2dp_offload_enabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("hfp_enabled") != inst->mConfigMap.end()) {
        ahal_configs.hfp_enabled =
            toBool(inst->mConfigMap["hfp_enabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("vbat_enabled") != inst->mConfigMap.end()) {
        ahal_configs.vbat_enabled =
            toBool(inst->mConfigMap["vbat_enabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("wsa_enabled") != inst->mConfigMap.end()) {
        ahal_configs.wsa_enabled =
            toBool(inst->mConfigMap["wsa_enabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("ext_hw_plugin_enabled") != inst->mConfigMap.end()) {
        ahal_configs.ext_hw_plugin_enabled =
            toBool(inst->mConfigMap["ext_hw_plugin_enabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("record_play_concurrency") != inst->mConfigMap.end()) {
        ahal_configs.record_play_concurrency =
            toBool(inst->mConfigMap["record_play_concurrency"].c_str(), false);
    }
    if (inst->mConfigMap.find("hdmi_passthrough_enabled") != inst->mConfigMap.end()) {
        ahal_configs.hdmi_passthrough_enabled =
            toBool(inst->mConfigMap["hdmi_passthrough_enabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("concurrent_capture_enabled") != inst->mConfigMap.end()) {
        ahal_configs.concurrent_capture_enabled =
            toBool(inst->mConfigMap["concurrent_capture_enabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("compress_in_enabled") != inst->mConfigMap.end()) {
        ahal_configs.compress_in_enabled =
            toBool(inst->mConfigMap["compress_in_enabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("battery_listener_enabled") != inst->mConfigMap.end()) {
        ahal_configs.battery_listener_enabled =
            toBool(inst->mConfigMap["battery_listener_enabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("maxx_audio_enabled") != inst->mConfigMap.end()) {
        ahal_configs.maxx_audio_enabled =
            toBool(inst->mConfigMap["maxx_audio_enabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("compress_metadata_needed") != inst->mConfigMap.end()) {
        ahal_configs.compress_metadata_needed =
            toBool(inst->mConfigMap["compress_metadata_needed"].c_str(), false);
    }
    if (inst->mConfigMap.find("incall_music_enabled") != inst->mConfigMap.end()) {
        ahal_configs.incall_music_enabled =
            toBool(inst->mConfigMap["incall_music_enabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("compress_voip_enabled") != inst->mConfigMap.end()) {
        ahal_configs.compress_voip_enabled =
            toBool(inst->mConfigMap["compress_voip_enabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("dynamic_ecns_enabled") != inst->mConfigMap.end()) {
        ahal_configs.dynamic_ecns_enabled =
            toBool(inst->mConfigMap["dynamic_ecns_enabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("audio_zoom_enabled") != inst->mConfigMap.end()) {
        ahal_configs.audio_zoom_enabled =
            toBool(inst->mConfigMap["audio_zoom_enabled"].c_str(), false);
    }

    return &ahal_configs;
}

AHalValues_t* getAudioHalExtConfigs() {
    return AudioHalExtLocal::getAHalConfigs();
}
