/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#define LOG_TAG "AudioHalExtHidl"
#define LOG_NDDEBUG 0

#include "AudioHalExtHidl.h"

namespace vendor {
namespace qti {
namespace hardware {
namespace audiohalext {
namespace V1_0 {
namespace implementation {

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

AudioHalExtHidl::AudioHalExtHidl() {
    inst = ConfigParser::getInstance();
}

AudioHalExtHidl *AudioHalExtHidl::getInstance() {
    static AudioHalExtHidl instance;
    return &instance;
}

// Methods from ::vendor::qti::hardware::audiohalext::V1_0::IAudioHalExt follow.
Return<void> AudioHalExtHidl::getApmConfigs(getApmConfigs_cb _hidl_cb) {
    ApmConfigs configs = {false, {
                                  true,  // audio_offload_video
                                  false, // audio_offload_disable
                                  true,  // audio_deepbuffer_media
                                  false, // audio_av_streaming_offload_enable
                                  true,  // audio_offload_track_enable
                                  false, // audio_offload_multiple_enabled
                                  true,  // voice_dsd_playback_conc_disabled
                                  false, // audio_sva_conc_enabled
                                  false, // audio_va_concurrency_enabled
                                  false, // audio_rec_playback_conc_disabled
                                  true,  // voice_path_for_pcm_voip
                                  false, // voice_playback_conc_disabled
                                  false, // voice_record_conc_disabled
                                  false, // voice_voip_conc_disabled
                                  60,    // audio_offload_min_duration_secs
                                  "\0",  // voice_conc_fallbackpath
                                  true,  // audio_extn_hdmi_spk_enabled
                                  true,  // audio_extn_formats_enabled
                                  true,  // audio_extn_afe_proxy_enabled
                                  false, // compress_voip_enabled
                                  true,  // fm_power_opt
                                  false, // voice_concurrency
                                  false, // record_play_concurrency
                                  true,  // use_xml_audio_policy_conf
                                 }};

    // bool properties
    if (inst->mConfigMap.find("audio.offload.video") != inst->mConfigMap.end()) {
        configs.specified = true;
        configs.value.audio_offload_video =
            toBool(inst->mConfigMap["audio.offload.video"].c_str(), false);
    }
    if (inst->mConfigMap.find("audio.offload.disable") != inst->mConfigMap.end()) {
        configs.specified = true;
        configs.value.audio_offload_disable =
            toBool(inst->mConfigMap["audio.offload.disable"].c_str(), false);
    }
    if (inst->mConfigMap.find("audio.deep_buffer.media") != inst->mConfigMap.end()) {
        configs.specified = true;
        configs.value.audio_deepbuffer_media =
            toBool(inst->mConfigMap["audio.deep_buffer.media"].c_str(), false);
    }
    if (inst->mConfigMap.find("vendor.audio.av.streaming.offload.enable") !=
            inst->mConfigMap.end()) {
        configs.specified = true;
        configs.value.audio_av_streaming_offload_enable =
            toBool(inst->mConfigMap["vendor.audio.av.streaming.offload.enable"].c_str(), false);
    }
    if (inst->mConfigMap.find("vendor.audio.offload.track.enable") != inst->mConfigMap.end()) {
        configs.specified = true;
        configs.value.audio_offload_track_enable =
            toBool(inst->mConfigMap["vendor.audio.offload.track.enable"].c_str(), false);
    }
    if (inst->mConfigMap.find("vendor.audio.offload.multiple.enabled") != inst->mConfigMap.end()) {
        configs.specified = true;
        configs.value.audio_offload_multiple_enabled =
            toBool(inst->mConfigMap["vendor.audio.offload.multiple.enabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("vendor.voice.dsd.playback.conc.disabled") !=
            inst->mConfigMap.end()) {
        configs.specified = true;
        configs.value.voice_dsd_playback_conc_disabled =
            toBool(inst->mConfigMap["vendor.voice.dsd.playback.conc.disabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("persist.vendor.audio.sva.conc.enabled") != inst->mConfigMap.end()) {
        configs.specified = true;
        configs.value.audio_sva_conc_enabled =
            toBool(inst->mConfigMap["persist.vendor.audio.sva.conc.enabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("persist.vendor.audio.va_concurrency_enabled") !=
            inst->mConfigMap.end()) {
        configs.specified = true;
        configs.value.audio_va_concurrency_enabled =
            toBool(inst->mConfigMap["persist.vendor.audio.va_concurrency_enabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("vendor.audio.rec.playback.conc.disabled") !=
            inst->mConfigMap.end()) {
        configs.specified = true;
        configs.value.audio_rec_playback_conc_disabled =
            toBool(inst->mConfigMap["vendor.audio.rec.playback.conc.disabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("vendor.voice.path.for.pcm.voip") != inst->mConfigMap.end()) {
        configs.specified = true;
        configs.value.voice_path_for_pcm_voip =
            toBool(inst->mConfigMap["vendor.voice.path.for.pcm.voip"].c_str(), false);
    }
    if (inst->mConfigMap.find("vendor.voice.playback.conc.disabled") != inst->mConfigMap.end()) {
        configs.specified = true;
        configs.value.voice_playback_conc_disabled =
            toBool(inst->mConfigMap["vendor.voice.playback.conc.disabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("vendor.voice.record.conc.disabled") != inst->mConfigMap.end()) {
        configs.specified = true;
        configs.value.voice_record_conc_disabled =
            toBool(inst->mConfigMap["vendor.voice.record.conc.disabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("vendor.voice.voip.conc.disabled") != inst->mConfigMap.end()) {
        configs.specified = true;
        configs.value.voice_voip_conc_disabled =
            toBool(inst->mConfigMap["vendor.voice.voip.conc.disabled"].c_str(), false);
    }

    // uint properties
    if (inst->mConfigMap.find("audio.offload.min.duration.secs") != inst->mConfigMap.end()) {
        configs.specified = true;
        configs.value.audio_offload_min_duration_secs =
            (uint32_t)atoi(inst->mConfigMap["audio.offload.min.duration.secs"].c_str());
    }
    // string properties
    if (inst->mConfigMap.find("vendor.voice.conc.fallbackpath") != inst->mConfigMap.end()) {
        configs.specified = true;
        configs.value.voice_conc_fallbackpath = inst->mConfigMap["vendor.voice.conc.fallbackpath"];
    }

    // flags
    if (inst->mConfigMap.find("audio_extn_hdmi_spk_enabled") != inst->mConfigMap.end()) {
        configs.specified = true;
        configs.value.audio_extn_hdmi_spk_enabled =
            toBool(inst->mConfigMap["audio_extn_hdmi_spk_enabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("audio_extn_formats_enabled") != inst->mConfigMap.end()) {
        configs.specified = true;
        configs.value.audio_extn_formats_enabled =
            toBool(inst->mConfigMap["audio_extn_formats_enabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("afe_proxy_enabled") != inst->mConfigMap.end()) {
        configs.specified = true;
        configs.value.audio_extn_afe_proxy_enabled =
            toBool(inst->mConfigMap["afe_proxy_enabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("compress_voip_enabled") != inst->mConfigMap.end()) {
        configs.specified = true;
        configs.value.compress_voip_enabled =
            toBool(inst->mConfigMap["compress_voip_enabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("fm_power_opt") != inst->mConfigMap.end()) {
        configs.specified = true;
        configs.value.fm_power_opt =
            toBool(inst->mConfigMap["fm_power_opt"].c_str(), false);
    }
    if (inst->mConfigMap.find("voice_concurrency") != inst->mConfigMap.end()) {
        configs.specified = true;
        configs.value.voice_concurrency =
            toBool(inst->mConfigMap["voice_concurrency"].c_str(), false);
    }
    if (inst->mConfigMap.find("record_play_concurrency") != inst->mConfigMap.end()) {
        configs.specified = true;
        configs.value.record_play_concurrency =
            toBool(inst->mConfigMap["record_play_concurrency"].c_str(), false);
    }
    if (inst->mConfigMap.find("use_xml_audio_policy_conf") != inst->mConfigMap.end()) {
        configs.specified = true;
        configs.value.use_xml_audio_policy_conf =
            toBool(inst->mConfigMap["use_xml_audio_policy_conf"].c_str(), false);
    }

    _hidl_cb(configs);
    return Void();
}

Return<void> AudioHalExtHidl::getAVConfigs(getAVConfigs_cb _hidl_cb) {
    AVConfigs configs = {false, {
                                  true,  // audio_use_sw_alac_decoder
                                  true,  // audio_use_sw_ape_decoder
                                  false, // audio_use_sw_mpegh_decoder
                                  true,  // audio_flac_sw_decoder_24bit
                                  true,  // audio_hw_aac_encoder
                                  "\0",  // audio_debug_sf_noaudio
                                  0,     // audio_record_aggregate_buffer_duration
                                  true,  // aac_adts_offload_enabled
                                  true,  // alac_offload_enabled
                                  true,  // ape_offload_enabled
                                  true,  // flac_offload_enabled
                                  true,  // pcm_offload_enabled_16
                                  true,  // pcm_offload_enabled_24
                                  true,  // qti_flac_decoder
                                  true,  // vorbis_offload_enabled
                                  true,  // wma_offload_enabled
                                 }};

    // bool properties
    if (inst->mConfigMap.find("vendor.audio.use.sw.alac.decoder") != inst->mConfigMap.end()) {
        configs.specified = true;
        configs.value.audio_use_sw_alac_decoder =
            toBool(inst->mConfigMap["vendor.audio.use.sw.alac.decoder"].c_str(), false);
    }
    if (inst->mConfigMap.find("vendor.audio.use.sw.ape.decoder") != inst->mConfigMap.end()) {
        configs.specified = true;
        configs.value.audio_use_sw_ape_decoder =
            toBool(inst->mConfigMap["vendor.audio.use.sw.ape.decoder"].c_str(), false);
    }
    if (inst->mConfigMap.find("vendor.audio.use.sw.mpegh.decoder") != inst->mConfigMap.end()) {
        configs.specified = true;
        configs.value.audio_use_sw_mpegh_decoder =
            toBool(inst->mConfigMap["vendor.audio.use.sw.mpegh.decoder"].c_str(), false);
    }
    if (inst->mConfigMap.find("vendor.audio.flac.sw.decoder.24bit") != inst->mConfigMap.end()) {
        configs.specified = true;
        configs.value.audio_flac_sw_decoder_24bit =
            toBool(inst->mConfigMap["vendor.audio.flac.sw.decoder.24bit"].c_str(), false);
    }
    if (inst->mConfigMap.find("vendor.audio.hw.aac.encoder") != inst->mConfigMap.end()) {
        configs.specified = true;
        configs.value.audio_hw_aac_encoder =
            toBool(inst->mConfigMap["vendor.audio.hw.aac.encoder"].c_str(), false);
    }
    // string properties
    if (inst->mConfigMap.find("persist.vendor.audio.debug.sf.noaudio") != inst->mConfigMap.end()) {
        configs.specified = true;
        configs.value.audio_debug_sf_noaudio =
            inst->mConfigMap["persist.vendor.audio.debug.sf.noaudio"];
    }
    // uint properties
    if (inst->mConfigMap.find("vendor.audio.record.aggregate.buffer.duration") !=
            inst->mConfigMap.end()) {
        configs.specified = true;
        configs.value.audio_record_aggregate_buffer_duration =
            (uint32_t)atoi(
                    inst->mConfigMap["vendor.audio.record.aggregate.buffer.duration"].c_str());
    }
    // flags
    if (inst->mConfigMap.find("aac_adts_offload_enabled") != inst->mConfigMap.end()) {
        configs.specified = true;
        configs.value.aac_adts_offload_enabled =
            toBool(inst->mConfigMap["aac_adts_offload_enabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("alac_offload_enabled") != inst->mConfigMap.end()) {
        configs.specified = true;
        configs.value.alac_offload_enabled =
            toBool(inst->mConfigMap["alac_offload_enabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("ape_offload_enabled") != inst->mConfigMap.end()) {
        configs.specified = true;
        configs.value.ape_offload_enabled =
            toBool(inst->mConfigMap["ape_offload_enabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("flac_offload_enabled") != inst->mConfigMap.end()) {
        configs.specified = true;
        configs.value.flac_offload_enabled =
            toBool(inst->mConfigMap["flac_offload_enabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("pcm_offload_enabled_16") != inst->mConfigMap.end()) {
        configs.specified = true;
        configs.value.pcm_offload_enabled_16 =
            toBool(inst->mConfigMap["pcm_offload_enabled_16"].c_str(), false);
    }
    if (inst->mConfigMap.find("pcm_offload_enabled_24") != inst->mConfigMap.end()) {
        configs.specified = true;
        configs.value.pcm_offload_enabled_24 =
            toBool(inst->mConfigMap["pcm_offload_enabled_24"].c_str(), false);
    }
    if (inst->mConfigMap.find("qti_flac_decoder") != inst->mConfigMap.end()) {
        configs.specified = true;
        configs.value.qti_flac_decoder =
            toBool(inst->mConfigMap["qti_flac_decoder"].c_str(), false);
    }
    if (inst->mConfigMap.find("vorbis_offload_enabled") != inst->mConfigMap.end()) {
        configs.specified = true;
        configs.value.vorbis_offload_enabled =
            toBool(inst->mConfigMap["vorbis_offload_enabled"].c_str(), false);
    }
    if (inst->mConfigMap.find("wma_offload_enabled") != inst->mConfigMap.end()) {
        configs.specified = true;
        configs.value.wma_offload_enabled =
            toBool(inst->mConfigMap["wma_offload_enabled"].c_str(), false);
    }

    _hidl_cb(configs);
    return Void();
}
// Methods from ::android::hidl::base::V1_0::IBase follow.

IAudioHalExt* HIDL_FETCH_IAudioHalExt(const char* /* name */) {
    return AudioHalExtHidl::getInstance();
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace audiohalext
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
