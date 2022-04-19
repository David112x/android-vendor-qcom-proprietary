/*  Copyright (C) 2016-2017, The Linux Foundation. All rights reserved.
 *
 *  Not a Contribution
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted (subject to the limitations in the
 *  disclaimer below) provided that the following conditions are met:

      * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.

      * Redistributions in binary form must reproduce the above
        copyright notice, this list of conditions and the following
        disclaimer in the documentation and/or other materials provided
        with the distribution.

      * Neither the name of The Linux Foundation nor the names of its
        contributors may be used to endorse or promote products derived
        from this software without specific prior written permission.

 *  NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
 *  GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
 *  HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 *  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 *****************************************************************************/
/*****************************************************************************
 *  Copyright (C) 2009-2012 Broadcom Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/
/*      bthost_ipc.c
 *
 *  Description:   Implements IPC interface between HAL and BT host
 *
 *****************************************************************************/
#include <time.h>
#include <unistd.h>

#include "ldac_level_bit_rate_lookup.h"
#include "bthost_ipc.h"
#include <errno.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <system/audio.h>
#include <hardware/audio.h>

#include <hardware/hardware.h>
#include <log/log.h>
#include <cutils/properties.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "bthost_ipc"

bool DEBUG = false;
static int bt_split_a2dp_enabled = 0;
/*****************************************************************************
**  Constants & Macros
******************************************************************************/
/* Below two values adds up to 8 sec retry to address IOT issues*/
#define STREAM_START_MAX_RETRY_COUNT 5
#define STREAM_START_MAX_RETRY_LOOPER 6
#define CTRL_CHAN_RETRY_COUNT 3
#define CHECK_A2DP_READY_MAX_COUNT 5

#define CASE_RETURN_STR(const) case const: return #const;

#define FNLOG()             ALOGW(LOG_TAG, "%s", __FUNCTION__);
#define DEBUG(fmt, ...)     ALOGD(LOG_TAG, "%s: " fmt,__FUNCTION__, ## __VA_ARGS__)
#define INFO(fmt, ...)      ALOGI(LOG_TAG, "%s: " fmt,__FUNCTION__, ## __VA_ARGS__)
#define WARN(fmt, ...)      ALOGW(LOG_TAG, "%s: " fmt,__FUNCTION__, ## __VA_ARGS__)
#define ERROR(fmt, ...)     ALOGE(LOG_TAG, "%s: " fmt,__FUNCTION__, ## __VA_ARGS__)

#define ASSERTC(cond, msg, val) if (!(cond)) {ERROR("### ASSERT : %s line %d %s (%d) ###", __FILE__, __LINE__, msg, val);}

/*****************************************************************************
**  Local type definitions
******************************************************************************/

struct a2dp_stream_common audio_stream;
bt_lib_callback_t *stack_cb = NULL;
static volatile unsigned char ack_recvd = 0;
pthread_cond_t ack_cond = PTHREAD_COND_INITIALIZER;
static int test = 0;
static bool update_initial_sink_latency = false;
int wait_for_stack_response(uint8_t time_to_wait);
bool resp_received = false;
uint8_t tws_channelmode = 0;
static char a2dp_hal_imp[PROPERTY_VALUE_MAX] = "false";
static char AAC_frame_ctrl_val[PROPERTY_VALUE_MAX] = "false";
/*****************************************************************************
**  Static functions
******************************************************************************/

audio_sbc_encoder_config_t sbc_codec;
audio_aptx_encoder_config_t aptx_codec;
audio_aptx_adaptive_encoder_config_t aptx_adaptive_codec;
audio_aptx_tws_encoder_config_t aptx_tws_codec;
audio_aac_encoder_config_t aac_codec;
audio_aac_encoder_config_v2_t aac_codec_v2;
audio_ldac_encoder_config_t ldac_codec;
audio_celt_encoder_config_t celt_codec;
/*****************************************************************************
**  Functions
******************************************************************************/
void a2dp_open_ctrl_path(struct a2dp_stream_common *common);
void ldac_codec_parser(uint8_t *codec_cfg);
/*****************************************************************************
**   Miscellaneous helper functions
******************************************************************************/
static const char* dump_a2dp_ctrl_event(char event)
{
    switch(event)
    {
        CASE_RETURN_STR(A2DP_CTRL_CMD_NONE)
        CASE_RETURN_STR(A2DP_CTRL_CMD_CHECK_READY)
        CASE_RETURN_STR(A2DP_CTRL_CMD_START)
        CASE_RETURN_STR(A2DP_CTRL_CMD_STOP)
        CASE_RETURN_STR(A2DP_CTRL_CMD_SUSPEND)
        CASE_RETURN_STR(A2DP_CTRL_CMD_OFFLOAD_SUPPORTED)
        CASE_RETURN_STR(A2DP_CTRL_CMD_OFFLOAD_NOT_SUPPORTED)
        CASE_RETURN_STR(A2DP_CTRL_CMD_CHECK_STREAM_STARTED)
        CASE_RETURN_STR(A2DP_CTRL_GET_CODEC_CONFIG)
        CASE_RETURN_STR(A2DP_CTRL_GET_MULTICAST_STATUS)
        CASE_RETURN_STR(A2DP_CTRL_GET_CONNECTION_STATUS)
        default:
            return "UNKNOWN MSG ID";
    }
}

static const char* dump_a2dp_ctrl_ack(tA2DP_CTRL_ACK resp)
{
    switch(resp)
    {
        CASE_RETURN_STR(A2DP_CTRL_ACK_SUCCESS)
        CASE_RETURN_STR(A2DP_CTRL_ACK_FAILURE)
        CASE_RETURN_STR(A2DP_CTRL_ACK_INCALL_FAILURE)
        CASE_RETURN_STR(A2DP_CTRL_ACK_UNSUPPORTED)
        CASE_RETURN_STR(A2DP_CTRL_ACK_PENDING)
        CASE_RETURN_STR(A2DP_CTRL_ACK_DISCONNECT_IN_PROGRESS)
        CASE_RETURN_STR(A2DP_CTRL_ACK_PREVIOUS_COMMAND_PENDING)
        CASE_RETURN_STR(A2DP_CTRL_SKT_DISCONNECTED)
        CASE_RETURN_STR(A2DP_CTRL_ACK_UNKNOWN)
        default:
            return "UNKNOWN ACK ID";
    }
}

static const char* dump_a2dp_hal_state(int event)
{
    switch(event)
    {
        CASE_RETURN_STR(AUDIO_A2DP_STATE_STARTING)
        CASE_RETURN_STR(AUDIO_A2DP_STATE_STARTED)
        CASE_RETURN_STR(AUDIO_A2DP_STATE_STOPPING)
        CASE_RETURN_STR(AUDIO_A2DP_STATE_STOPPED)
        CASE_RETURN_STR(AUDIO_A2DP_STATE_SUSPENDED)
        CASE_RETURN_STR(AUDIO_A2DP_STATE_STANDBY)
        default:
            return "UNKNOWN STATE ID";
    }
}
static void* a2dp_codec_parser(uint8_t *codec_cfg, audio_format_t *codec_type,
                                                   uint32_t *sample_freq)
{
    char byte,len;
    uint8_t *p_cfg = codec_cfg;
    ALOGW("%s: codec_type = %x",__func__, codec_cfg[CODEC_OFFSET]);
    if (codec_cfg[CODEC_OFFSET] == CODEC_TYPE_PCM)
    {
        *codec_type = AUDIO_FORMAT_PCM_16_BIT;
        //For the time being Audio does not require any param to be passed for PCM so returning null
        return NULL;
    }
    else if (codec_cfg[CODEC_OFFSET] == CODEC_TYPE_SBC)
    {
        memset(&sbc_codec,0,sizeof(audio_sbc_encoder_config_t));
        p_cfg++;//skip dev idx
        len = *p_cfg++;
        p_cfg++;//skip media type
        len--;
        p_cfg++;
        len--;
        byte = *p_cfg++;
        len--;
        switch (byte & A2D_SBC_FREQ_MASK)
        {
            case A2D_SBC_SAMP_FREQ_48:
                 sbc_codec.sampling_rate = 48000;
                 break;
            case A2D_SBC_SAMP_FREQ_44:
                 sbc_codec.sampling_rate = 44100;
                 break;
            case A2D_SBC_SAMP_FREQ_32:
                 sbc_codec.sampling_rate = 3200;
                 break;
            case A2D_SBC_SAMP_FREQ_16:
                 sbc_codec.sampling_rate = 16000;
                 break;
            default:
                 ALOGE("SBC:Unkown sampling rate");
        }

        switch (byte & A2D_SBC_CHN_MASK)
        {
            case A2D_SBC_CH_MD_JOINT:
                 sbc_codec.channels = 3;
                 break;
            case A2D_SBC_CH_MD_STEREO:
                 sbc_codec.channels = 2;
                 break;
            case A2D_SBC_CH_MD_DUAL:
                 sbc_codec.channels = 1;
                 break;
            case A2D_SBC_CH_MD_MONO:
                 sbc_codec.channels = 0;
                 break;
            default:
                 ALOGE("SBC:Unknow channel mode");
        }
        byte = *p_cfg++;
        len--;
        switch (byte & A2D_SBC_BLK_MASK)
        {
            case A2D_SBC_BLOCKS_16:
                sbc_codec.blk_len = 16;
                break;
            case A2D_SBC_BLOCKS_12:
                sbc_codec.blk_len = 12;
                break;
            case A2D_SBC_BLOCKS_8:
                sbc_codec.blk_len = 8;
                break;
            case A2D_SBC_BLOCKS_4:
                sbc_codec.blk_len = 4;
                break;
            default:
                ALOGE("SBD:Unknown block length");
        }

        switch (byte & A2D_SBC_SUBBAND_MASK)
        {
            case A2D_SBC_SUBBAND_8:
                sbc_codec.subband = 8;
                break;
            case A2D_SBC_SUBBAND_4:
                sbc_codec.subband = 4;
                break;
            default:
                ALOGE("SBD:Unknown subband");
        }
        switch (byte & A2D_SBC_ALLOC_MASK)
        {
            case A2D_SBC_ALLOC_MD_L:
                sbc_codec.alloc = 1;
                break;
            case A2D_SBC_ALLOC_MD_S:
                sbc_codec.alloc = 2;
            default:
                ALOGE("SBD:Unknown alloc method");
        }
        sbc_codec.min_bitpool = *p_cfg++;
        len--;
        sbc_codec.max_bitpool = *p_cfg++;
        len--;
        if (len == 0)
        {
            ALOGW("Copied codec config");
        }
        p_cfg += 2; //skip mtu
        sbc_codec.bitrate = *p_cfg++;
        sbc_codec.bitrate |= (*p_cfg++ << 8);
        sbc_codec.bitrate |= (*p_cfg++ << 16);
        sbc_codec.bitrate |= (*p_cfg++ << 24);
        sbc_codec.bits_per_sample = *(uint32_t *)p_cfg;
        *codec_type = AUDIO_FORMAT_SBC;

        if(sample_freq) *sample_freq = sbc_codec.sampling_rate;

        ALOGW("SBC: Done copying full codec config bits_per_sample: %d", sbc_codec.bits_per_sample);
        return ((void *)(&sbc_codec));
    } else if (codec_cfg[CODEC_OFFSET] == CODEC_TYPE_AAC)
    {
        bool is_AAC_frame_ctrl_enable = false;
        property_get("persist.vendor.bt.aac_frm_ctl.enabled", AAC_frame_ctrl_val, "false");
        if (!strcmp(AAC_frame_ctrl_val, "true"))
          is_AAC_frame_ctrl_enable = true;
        ALOGW("%s: AAC frame control enabled: %d", __func__, is_AAC_frame_ctrl_enable);
        if (is_AAC_frame_ctrl_enable) {
          uint16_t aac_samp_freq = 0;
          uint32_t aac_bit_rate = 0;
          memset(&aac_codec_v2,0,sizeof(audio_aac_encoder_config_v2_t));
          p_cfg++;//skip dev idx
          len = *p_cfg++;
          p_cfg++;//skip media type
          len--;
          p_cfg++;//skip codec type
          len--;
          byte = *p_cfg++;
          len--;
          switch (byte & A2D_AAC_IE_OBJ_TYPE_MSK)
          {
              case A2D_AAC_IE_OBJ_TYPE_MPEG_2_AAC_LC:
                  aac_codec_v2.enc_mode = AUDIO_FORMAT_AAC_SUB_LC;
                  break;
              case A2D_AAC_IE_OBJ_TYPE_MPEG_4_AAC_LC:
                  aac_codec_v2.enc_mode = AUDIO_FORMAT_AAC_SUB_LC;
                  break;
              case A2D_AAC_IE_OBJ_TYPE_MPEG_4_AAC_LTP:
                  aac_codec_v2.enc_mode = AUDIO_FORMAT_AAC_SUB_LTP;
                  break;
              case A2D_AAC_IE_OBJ_TYPE_MPEG_4_AAC_SCA:
                  aac_codec_v2.enc_mode = AUDIO_FORMAT_AAC_SUB_SCALABLE;
                  break;
              default:
                  ALOGE("AAC:Unknown encoder mode");
          }
          //USE 0 (AAC_LC) as hardcoded value till Audio
          //define constants
          aac_codec_v2.enc_mode = 0;
          //USE LOAS(1) or LATM(4) hardcoded values till
          //Audio define proper constants
          aac_codec_v2.format_flag = 4;
          byte = *p_cfg++;
          len--;
          aac_samp_freq = byte << 8; //1st byte of sample_freq
          byte = *p_cfg++;
          len--;
          aac_samp_freq |= byte & 0x00F0; //1st nibble of second byte of samp_freq

          switch (aac_samp_freq) {
              case 0x8000: aac_codec_v2.sampling_rate = 8000; break;
              case 0x4000: aac_codec_v2.sampling_rate = 11025; break;
              case 0x2000: aac_codec_v2.sampling_rate = 12000; break;
              case 0x1000: aac_codec_v2.sampling_rate = 16000; break;
              case 0x0800: aac_codec_v2.sampling_rate = 22050; break;
              case 0x0400: aac_codec_v2.sampling_rate = 24000; break;
              case 0x0200: aac_codec_v2.sampling_rate = 32000; break;
              case 0x0100: aac_codec_v2.sampling_rate = 44100; break;
              case 0x0080: aac_codec_v2.sampling_rate = 48000; break;
              case 0x0040: aac_codec_v2.sampling_rate = 64000; break;
              case 0x0020: aac_codec_v2.sampling_rate = 88200; break;
              case 0x0010: aac_codec_v2.sampling_rate = 96000; break;
              default:
                  ALOGE("Invalid sample_freq: %x", aac_samp_freq);
          }

          switch (byte & A2D_AAC_IE_CHANNELS_MSK)
          {
              case A2D_AAC_IE_CHANNELS_1:
                   aac_codec_v2.channels = 1;
                   break;
              case A2D_AAC_IE_CHANNELS_2:
                   aac_codec_v2.channels = 2;
                   break;
              default:
                   ALOGE("AAC:Unknown channel mode");
          }
          byte = *p_cfg++; //Move to VBR byte
          len--;
          switch (byte & A2D_AAC_IE_VBR_MSK)
          {
              case A2D_AAC_IE_VBR:
                  break;
              default:
                  ALOGE("AAC:VBR not supported");
          }
          aac_bit_rate = 0x7F&byte;
          //Move it 2nd byte of 32 bit word. leaving the VBR bit
          aac_bit_rate = aac_bit_rate << 16;
          byte = *p_cfg++; //Move to 2nd byteof bitrate
          len--;

          //Move it to 3rd byte of 32bit word
          aac_bit_rate |= 0x0000FF00 & (((uint32_t)byte)<<8);
          byte = *p_cfg++; //Move to 3rd byte of bitrate
          len--;

          aac_bit_rate |= 0x000000FF & (((uint32_t)byte));
          aac_codec_v2.bitrate = aac_bit_rate;
          ALOGW("%s: Final AAC bitrate: %d",__func__, aac_bit_rate);

          aac_codec_v2.frame_ctl.ctl_type = A2D_AAC_FRAME_PEAK_MTU;
          aac_codec_v2.frame_ctl.ctl_value = *(uint16_t *)p_cfg;
          //(p_cfg(+2 is because of mtu filled in stack is of 2bytes)
          p_cfg = p_cfg + 2;

          aac_codec_v2.bitrate = *(uint32_t *)p_cfg;
          //p_cfg(+4 is because of bitrate filled in stack is occupying 4 bytes.
          p_cfg = p_cfg + 4;
          ALOGW("%s: AAC bitrate overwritten with actual value fetched from stack: %d",
                      __func__, aac_codec_v2.bitrate);

          aac_codec_v2.bits_per_sample = *(uint32_t *)p_cfg;
          *codec_type = AUDIO_FORMAT_AAC;

          if(sample_freq) *sample_freq = aac_codec_v2.sampling_rate;
          ALOGW("%s: Copied full codec config bits_per_sample : %d, ctl_type = %d, ctl_value = %d",
                 __func__, aac_codec_v2.bits_per_sample, aac_codec_v2.frame_ctl.ctl_type,
                           aac_codec_v2.frame_ctl.ctl_value );
          return ((void *)(&aac_codec_v2));
        } else {
          uint16_t aac_samp_freq = 0;
          uint32_t aac_bit_rate = 0;
          memset(&aac_codec,0,sizeof(audio_aac_encoder_config_t));
          p_cfg++;//skip dev idx
          len = *p_cfg++;
          p_cfg++;//skip media type
          len--;
          p_cfg++;//skip codec type
          len--;
          byte = *p_cfg++;
          len--;
          switch (byte & A2D_AAC_IE_OBJ_TYPE_MSK)
          {
              case A2D_AAC_IE_OBJ_TYPE_MPEG_2_AAC_LC:
                  aac_codec.enc_mode = AUDIO_FORMAT_AAC_SUB_LC;
                  break;
              case A2D_AAC_IE_OBJ_TYPE_MPEG_4_AAC_LC:
                  aac_codec.enc_mode = AUDIO_FORMAT_AAC_SUB_LC;
                  break;
              case A2D_AAC_IE_OBJ_TYPE_MPEG_4_AAC_LTP:
                  aac_codec.enc_mode = AUDIO_FORMAT_AAC_SUB_LTP;
                  break;
              case A2D_AAC_IE_OBJ_TYPE_MPEG_4_AAC_SCA:
                  aac_codec.enc_mode = AUDIO_FORMAT_AAC_SUB_SCALABLE;
                  break;
              default:
                  ALOGE("AAC:Unknown encoder mode");
          }
          //USE 0 (AAC_LC) as hardcoded value till Audio
          //define constants
          aac_codec.enc_mode = 0;
          //USE LOAS(1) or LATM(4) hardcoded values till
          //Audio define proper constants
          aac_codec.format_flag = 4;
          byte = *p_cfg++;
          len--;
          aac_samp_freq = byte << 8; //1st byte of sample_freq
          byte = *p_cfg++;
          len--;
          aac_samp_freq |= byte & 0x00F0; //1st nibble of second byte of samp_freq

          switch (aac_samp_freq) {
              case 0x8000: aac_codec.sampling_rate = 8000; break;
              case 0x4000: aac_codec.sampling_rate = 11025; break;
              case 0x2000: aac_codec.sampling_rate = 12000; break;
              case 0x1000: aac_codec.sampling_rate = 16000; break;
              case 0x0800: aac_codec.sampling_rate = 22050; break;
              case 0x0400: aac_codec.sampling_rate = 24000; break;
              case 0x0200: aac_codec.sampling_rate = 32000; break;
              case 0x0100: aac_codec.sampling_rate = 44100; break;
              case 0x0080: aac_codec.sampling_rate = 48000; break;
              case 0x0040: aac_codec.sampling_rate = 64000; break;
              case 0x0020: aac_codec.sampling_rate = 88200; break;
              case 0x0010: aac_codec.sampling_rate = 96000; break;
              default:
                  ALOGE("Invalid sample_freq: %x", aac_samp_freq);
          }

          switch (byte & A2D_AAC_IE_CHANNELS_MSK)
          {
              case A2D_AAC_IE_CHANNELS_1:
                   aac_codec.channels = 1;
                   break;
              case A2D_AAC_IE_CHANNELS_2:
                   aac_codec.channels = 2;
                   break;
              default:
                   ALOGE("AAC:Unknown channel mode");
          }
          byte = *p_cfg++; //Move to VBR byte
          len--;
          switch (byte & A2D_AAC_IE_VBR_MSK)
          {
              case A2D_AAC_IE_VBR:
                  break;
              default:
                  ALOGE("AAC:VBR not supported");
          }
          aac_bit_rate = 0x7F&byte;
          //Move it 2nd byte of 32 bit word. leaving the VBR bit
          aac_bit_rate = aac_bit_rate << 16;
          byte = *p_cfg++; //Move to 2nd byteof bitrate
          len--;

          //Move it to 3rd byte of 32bit word
          aac_bit_rate |= 0x0000FF00 & (((uint32_t)byte)<<8);
          byte = *p_cfg++; //Move to 3rd byte of bitrate
          len--;

          aac_bit_rate |= 0x000000FF & (((uint32_t)byte));
          aac_codec.bitrate = aac_bit_rate;
          ALOGW("%s: Final AAC bitrate: %d",__func__, aac_bit_rate);
          //+2 because 2 bytes occupying by MTU
          //+4 because 4 bytes occupying by bitrate
          p_cfg = p_cfg + 2 + 4;
          aac_codec.bits_per_sample = *(uint32_t *)p_cfg;
          *codec_type = AUDIO_FORMAT_AAC;

          if(sample_freq) *sample_freq = aac_codec.sampling_rate;
          ALOGW("%s: AAC: Done copying full codec config bits_per_sample : %d",
                               __func__, aac_codec.bits_per_sample );
          return ((void *)(&aac_codec));
        }
    }
    else if (codec_cfg[CODEC_OFFSET] == NON_A2DP_CODEC_TYPE)
    {
        uint32_t vendor_ldac_id = 0x0;
        vendor_ldac_id =  (codec_cfg[VENDOR_ID_OFFSET] & 0x000000FF) |
                   ((codec_cfg[VENDOR_ID_OFFSET + 1]) << 8 & 0x0000FF00) |
                   ((codec_cfg[VENDOR_ID_OFFSET + 2]) << 16 & 0x00FF0000) |
                   ((codec_cfg[VENDOR_ID_OFFSET + 3]) << 24 & 0xFF000000);

        if (codec_cfg[VENDOR_ID_OFFSET] == VENDOR_APTX &&
            codec_cfg[CODEC_ID_OFFSET] == APTX_CODEC_ID)
        {
            ALOGW("AptX-classic codec");
            *codec_type = AUDIO_FORMAT_APTX;
        }
        if (codec_cfg[VENDOR_ID_OFFSET] == VENDOR_APTX_HD &&
            codec_cfg[CODEC_ID_OFFSET] == APTX_HD_CODEC_ID)
        {
            ALOGW("AptX-HD codec");
            *codec_type = AUDIO_FORMAT_APTX_HD;
        }

        if (codec_cfg[VENDOR_ID_OFFSET] == VENDOR_APTX_ADAPTIVE &&
            codec_cfg[CODEC_ID_OFFSET] == APTX_ADAPTIVE_CODEC_ID)
        {
            ALOGW("AptX-Adaptive codec");
            *codec_type = ENC_CODEC_TYPE_APTX_ADAPTIVE;

            memset(&aptx_adaptive_codec, 0, sizeof(audio_aptx_adaptive_encoder_config_t));
            p_cfg++; //skip dev_idx
            len = *p_cfg++;//LOSC
            p_cfg++; // Skip media type
            len--;
            p_cfg++; //codec_type
            len--;
            p_cfg+=4;//skip vendor id
            len -= 4;
            p_cfg += 2; //skip codec id
            len -= 2;

            switch(*p_cfg++ & A2D_APTX_ADAPTIVE_SAMP_FREQ_MASK)
            {
                case A2DP_APTX_ADAPTIVE_SAMPLERATE_44100:
                     aptx_adaptive_codec.sampling_rate = 0x2;
                     if(sample_freq) *sample_freq = 44100;
                     break;
                case A2DP_APTX_ADAPTIVE_SAMPLERATE_48000:
                     aptx_adaptive_codec.sampling_rate = 0x1;
                     if(sample_freq) *sample_freq = 48000;
                     break;
                case A2DP_APTX_ADAPTIVE_SAMPLERATE_88000:
                     aptx_adaptive_codec.sampling_rate = 0;
                     break;
                case A2DP_APTX_ADAPTIVE_SAMPLERATE_192000:
                     aptx_adaptive_codec.sampling_rate = 0;
                     break;
                default:
                     ALOGE("Unknown sampling rate");
            }
            len--;

            switch(*p_cfg++ & A2D_APTX_ADAPTIVE_CHAN_MASK)
            {
                case A2DP_APTX_ADAPTIVE_CHANNELS_MONO:
                     aptx_adaptive_codec.channel_mode = 1;
                     break;
                case A2DP_APTX_ADAPTIVE_CHANNELS_TWS_MONO:
                     aptx_adaptive_codec.channel_mode = 2;
                     break;
                case A2DP_APTX_ADAPTIVE_CHANNELS_JOINT_STEREO:
                     aptx_adaptive_codec.channel_mode = 0;
                     break;
                case A2DP_APTX_ADAPTIVE_CHANNELS_TWS_STEREO:
                     aptx_adaptive_codec.channel_mode = 4;
                     break;
                default:
                     ALOGE("Unknown channel id");
            }
            len--;

            aptx_adaptive_codec.min_sink_buffering_LL = 20; // gghai temp setting to default value
            aptx_adaptive_codec.max_sink_buffering_LL = 50;
            aptx_adaptive_codec.min_sink_buffering_HQ = 20;
            aptx_adaptive_codec.max_sink_buffering_HQ = 50;
            aptx_adaptive_codec.min_sink_buffering_TWS = 20;
            aptx_adaptive_codec.max_sink_buffering_TWS = 50;

            aptx_adaptive_codec.TTP_LL_low = *(p_cfg ++);
            aptx_adaptive_codec.TTP_LL_high = *(p_cfg ++);
            aptx_adaptive_codec.TTP_HQ_low = *(p_cfg ++);
            aptx_adaptive_codec.TTP_HQ_high = *(p_cfg ++);
            aptx_adaptive_codec.TTP_TWS_low = *(p_cfg ++);
            aptx_adaptive_codec.TTP_TWS_high = *(p_cfg ++);
            len -= 6;

            p_cfg += 3; // ignoring eoc bits
            len -= 3;
            p_cfg += APTX_ADAPTIVE_RESERVED_BITS;
            len -= APTX_ADAPTIVE_RESERVED_BITS;
            ALOGW("%s: ## aptXAdaptive ## sampleRate 0x%x", __func__, aptx_adaptive_codec.sampling_rate);
            ALOGW("%s: ## aptXAdaptive ## channelMode 0x%x", __func__, aptx_adaptive_codec.channel_mode);
            ALOGW("%s: ## aptXAdaptive ## ttp_ll_0 0x%x", __func__, aptx_adaptive_codec.TTP_LL_low);
            ALOGW("%s: ## aptXAdaptive ## ttp_ll_1 0x%x", __func__, aptx_adaptive_codec.TTP_LL_high);
            ALOGW("%s: ## aptXAdaptive ## ttp_hq_0 0x%x", __func__, aptx_adaptive_codec.TTP_HQ_low);
            ALOGW("%s: ## aptXAdaptive ## ttp_hq_1 0x%x", __func__, aptx_adaptive_codec.TTP_HQ_high);
            ALOGW("%s: ## aptXAdaptive ## ttp_tws_0 0x%x", __func__, aptx_adaptive_codec.TTP_TWS_low);
            ALOGW("%s: ## aptXAdaptive ## ttp_tws_1 0x%x", __func__, aptx_adaptive_codec.TTP_TWS_high);

            if(len == 0)
                ALOGW("%s: codec config copied", __func__);
            else
                ALOGW("%s: codec config length error: %d", __func__, len);

            aptx_adaptive_codec.mtu = *(uint16_t *)p_cfg;
            p_cfg += 6;
            aptx_adaptive_codec.bits_per_sample = *(uint32_t *)p_cfg;
            p_cfg += 4;
            aptx_adaptive_codec.aptx_mode= *(uint16_t *)p_cfg;

            ALOGW("%s: ## aptXAdaptive ## MTU =  %d", __func__, aptx_adaptive_codec.mtu);
            ALOGW("%s: ## aptXAdaptive ## Bits Per Sample =  %d", __func__, aptx_adaptive_codec.bits_per_sample);
            ALOGW("%s: ## aptXAdaptive ## Mode =  %d", __func__, aptx_adaptive_codec.aptx_mode);

            return ((void *)&aptx_adaptive_codec);
        }

        if (vendor_ldac_id == VENDOR_LDAC &&
            codec_cfg[CODEC_ID_OFFSET] == LDAC_CODEC_ID)
        {
            ALOGW("LDAC codec");
            *codec_type = AUDIO_FORMAT_LDAC;
            ldac_codec_parser(codec_cfg);
            if (sample_freq) *sample_freq = ldac_codec.sampling_rate;
            return ((void *)&ldac_codec);
        }
        if (codec_cfg[VENDOR_ID_OFFSET] == VENDOR_APTX_HD &&
            codec_cfg[CODEC_ID_OFFSET] == APTX_TWS_CODEC_ID)
        {
            ALOGW("AptX-TWS codec");
            *codec_type = ENC_CODEC_TYPE_APTX_DUAL_MONO;
            //aptx_codec.sync_mode = 0x01;
        }
        memset(&aptx_codec,0,sizeof(audio_aptx_encoder_config_t));
        p_cfg++; //skip dev_idx
        len = *p_cfg++;//LOSC
        p_cfg++; // Skip media type
        len--;
        p_cfg++; //codec_type
        len--;
        p_cfg+=4;//skip vendor id
        len -= 4;
        p_cfg += 2; //skip codec id
        len -= 2;
        byte = *p_cfg++;
        len--;
        switch (byte & A2D_APTX_SAMP_FREQ_MASK)
        {
            case A2D_APTX_SAMP_FREQ_48:
                 aptx_codec.sampling_rate = 48000;
                 break;
            case A2D_APTX_SAMP_FREQ_44:
                 aptx_codec.sampling_rate = 44100;
                 break;
            default:
                 ALOGE("Unknown sampling rate");
        }
        switch (byte & A2D_APTX_CHAN_MASK)
        {
            case A2D_APTX_CHAN_STEREO:
            case A2D_APTX_TWS_CHAN_MODE:
                 aptx_codec.channels = 2;
                 break;
            case A2D_APTX_CHAN_MONO:
                 aptx_codec.channels = 1;
                 break;
            default:
                 ALOGE("Unknown channel mode");
        }
        if (*codec_type == AUDIO_FORMAT_APTX_HD) {
            p_cfg += 4;
            len -= 4;//ignore 4 bytes not used
        }
        if (len == 0)
        {
            ALOGW("Codec config copied");
        }
        p_cfg += 2; //skip mtu

        aptx_codec.bitrate = *p_cfg++;
        aptx_codec.bitrate |= (*p_cfg++ << 8);
        aptx_codec.bitrate |= (*p_cfg++ << 16);
        aptx_codec.bitrate |= (*p_cfg++ << 24);
        aptx_codec.bits_per_sample = *(uint32_t *)p_cfg;
        tws_channelmode = *(p_cfg+4);
        ALOGW("APTx: tws channel mode =%d\n", tws_channelmode);
        if(sample_freq) *sample_freq = aptx_codec.sampling_rate;
        ALOGW("APTx: Done copying full codec config bits_per_sample : %d", aptx_codec.bits_per_sample);
        if (*codec_type == ENC_CODEC_TYPE_APTX_DUAL_MONO)
        {
            memset(&aptx_tws_codec, 0, sizeof(audio_aptx_tws_encoder_config_t));
            memcpy(&aptx_tws_codec, &aptx_codec, sizeof(aptx_codec));
            aptx_tws_codec.sync_mode = 0x02;
            return ((void *)&aptx_tws_codec);
        }
        return ((void *)&aptx_codec);
    }
    else if (codec_cfg[CODEC_OFFSET] == CODEC_TYPE_CELT)
    {
        uint8_t celt_samp_freq = 0;
        uint32_t celt_bit_rate = 0;
        memset(&celt_codec,0,sizeof(audio_celt_encoder_config_t));
        switch(codec_cfg[4] & A2D_CELT_SAMP_FREQ_MASK)
        {
        case A2D_CELT_SAMP_FREQ_48:
            celt_codec.sampling_rate = 48000;
            break;
        case A2D_CELT_SAMP_FREQ_44:
            celt_codec.sampling_rate = 44100;
            break;
        case A2D_CELT_SAMP_FREQ_32:
            celt_codec.sampling_rate = 32000;
            break;
        default:
            ALOGE("CELT: unknown sampl freq");
        }
        switch(codec_cfg[4] & A2D_CELT_CHANNEL_MASK)
        {
        case A2D_CELT_CH_MONO:
            celt_codec.channels = 1;
            break;
        case A2D_CELT_CH_STEREO:
            celt_codec.channels = 2;
            break;
        default:
            ALOGE("CELT: unknown channel");
        }
        switch(codec_cfg[5] & A2D_CELT_FRAME_SIZE_MASK)
        {
        case A2D_CELT_FRAME_SIZE_64:
            celt_codec.frame_size = 64;
            break;
        case A2D_CELT_FRAME_SIZE_128:
            celt_codec.frame_size = 128;
            break;
        case A2D_CELT_FRAME_SIZE_256:
            celt_codec.frame_size = 256;
            break;
        case A2D_CELT_FRAME_SIZE_512:
            celt_codec.frame_size = 512;
            break;
        default:
            ALOGE("CELT: unknown frame size");
        }
        celt_codec.complexity = codec_cfg[5] & A2D_CELT_COMPLEXITY_MASK;
        celt_codec.prediction_mode =
                (codec_cfg[6] & A2D_CELT_PREDICTION_MODE_MASK) >> 4;
        celt_codec.vbr_flag = codec_cfg[6] & A2D_CELT_VBR_MASK;

        celt_codec.bitrate |= codec_cfg[7];
        celt_codec.bitrate = celt_codec.bitrate << 8;
        celt_codec.bitrate |= codec_cfg[8];
        celt_codec.bitrate = celt_codec.bitrate << 8;
        celt_codec.bitrate |= codec_cfg[9];
        celt_codec.bitrate = celt_codec.bitrate << 8;
        celt_codec.bitrate |= codec_cfg[10];
        *codec_type = AUDIO_CODEC_TYPE_CELT;

        ALOGE("CELT Bitrate: 0%x", celt_codec.bitrate);
        ALOGE("CELT channel: 0%x", celt_codec.channels);
        ALOGE("CELT complexity: 0%x", celt_codec.complexity);
        ALOGE("CELT frame_size: 0%x", celt_codec.frame_size);
        ALOGE("CELT prediction_mode: 0%x", celt_codec.prediction_mode);
        ALOGE("CELT sampl_freq: 0%x", celt_codec.sampling_rate);
        ALOGE("CELT vbr_flag: 0%x", celt_codec.vbr_flag);
        ALOGE("CELT codec_type: 0%x", codec_type);
        return ((void *)(&celt_codec));
    }
    return NULL;
}

int a2dp_read_codec_config(struct a2dp_stream_common *common,uint8_t idx)
{
    char cmd[2];//,ack;
    int i,len = 0;
    uint8_t *p_codec_cfg = common->codec_cfg;
    cmd[0] = A2DP_CTRL_GET_CODEC_CONFIG;
    cmd[1] = idx;
    ALOGW("%s",__func__);
    memset(p_codec_cfg,0,MAX_CODEC_CFG_SIZE);
    tA2DP_CTRL_ACK status = A2DP_CTRL_ACK_FAILURE;

    if(stack_cb)
    {
        ALOGW("Calling get_codec_cfg_cb");
        resp_received = false;
        stack_cb->get_codec_cfg_cb();
        ack_recvd = 0;
        if (resp_received == false)
        {
            ALOGW("%s: stack resp not received",__func__);
            wait_for_stack_response(1);
        }
        status = common->ack_status;
        common->ack_status = A2DP_CTRL_ACK_UNKNOWN;
        ALOGW("get_codec_cfg_cb returned: status = %s",dump_a2dp_ctrl_ack(status));
    }
    return status;
}

void a2dp_get_multicast_status(uint8_t *mcast_status)
{
    ALOGW("%s",__func__);
    if (stack_cb)
    {
        resp_received = false;
        stack_cb->get_mcast_status_cb();
        ack_recvd = 0;
        if (resp_received == false)
        {
            ALOGW("%s: stack resp not received",__func__);
            wait_for_stack_response(1);
        }
        *mcast_status = audio_stream.multicast;
    }
    else
        *mcast_status  = 0;
}

void a2dp_get_num_connected_devices(uint8_t *num_dev)
{
    ALOGW("%s",__func__);
    if (stack_cb)
    {
        resp_received = false;
        stack_cb->get_connected_device_cb();
        ack_recvd = 0;
        if (resp_received == false)
        {
            ALOGW("%s: stack resp not received",__func__);
            wait_for_stack_response(1);
        }
        *num_dev = 1;
    }
}
/*****************************************************************************
**
** AUDIO DATA PATH
**
*****************************************************************************/

void a2dp_stream_common_init(struct a2dp_stream_common *common)
{
    pthread_mutexattr_t lock_attr;

    //FNLOG();
    ALOGW("%s",__func__);

    pthread_mutexattr_init(&lock_attr);
    pthread_mutexattr_settype(&lock_attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&common->lock, &lock_attr);
    pthread_mutexattr_destroy(&lock_attr);
    common->state = AUDIO_A2DP_STATE_STOPPED;
    common->sink_latency = A2DP_DEFAULT_SINK_LATENCY;

    bt_split_a2dp_enabled = false;
}

int wait_for_stack_response(uint8_t time_to_wait)
{
    ALOGW("wait_for_stack_response");
    struct timespec now,wait_time;
    uint8_t retry = 0;
    pthread_mutex_lock(&audio_stream.ack_lock);
    if (stack_cb == NULL)
    {
        ALOGE("stack deinitialized");
        pthread_mutex_unlock(&audio_stream.ack_lock);
        return retry;
    }
    // in race condition, ack_status is updated as SUCCESS
    // without ack_recvd made 0.
    if (audio_stream.ack_status == A2DP_CTRL_ACK_SUCCESS)
    {
        ALOGE("ACK Success, no need to wait");
        pthread_mutex_unlock(&audio_stream.ack_lock);
        return retry;
    }
    while (retry < CTRL_CHAN_RETRY_COUNT &&
              ack_recvd == 0)
    {
        ALOGW("entering coditional wait: retry = %d, ack_recvd = %d",retry,ack_recvd);
        clock_gettime(CLOCK_REALTIME, &now);
        now.tv_sec += time_to_wait;
        pthread_cond_timedwait(&ack_cond, &audio_stream.ack_lock, &now);
        retry++;
    }
    pthread_mutex_unlock(&audio_stream.ack_lock);
    if (ack_recvd) {
        ALOGV("wait_for_stack_response: ack received");
    }
    ALOGV("wait_for_stack_response returning retry = %d",retry);
    return retry;
}
static void copy_status(tA2DP_CTRL_ACK status)
{
    ALOGW("copy_status: status = %d",status);
    pthread_mutex_lock(&audio_stream.ack_lock);
    audio_stream.ack_status = status;
    if (!ack_recvd)
    {
        ack_recvd = 1;
        pthread_cond_signal(&ack_cond);
    }
    pthread_mutex_unlock(&audio_stream.ack_lock);
}
void bt_stack_init(bt_lib_callback_t *lib_cb)
{
    ALOGW("bt_stack_init");
    int ret = 0;
    stack_cb = lib_cb;
}
void bt_stack_deinit(tA2DP_CTRL_ACK status)
{
    ALOGW("bt_stack_deinit");
    pthread_mutex_lock(&audio_stream.ack_lock);
    stack_cb = NULL;
    audio_stream.ack_status = status;
    if (!ack_recvd)
    {
        ack_recvd = 1;
        pthread_cond_signal(&ack_cond);
    }
    pthread_mutex_unlock(&audio_stream.ack_lock);
}

void bt_stack_on_stream_started(tA2DP_CTRL_ACK status)
{
    ALOGW("bt_stack_on_stream_started: status = %d",status);
    pthread_mutex_lock(&audio_stream.ack_lock);
    if ((audio_stream.ack_status != A2DP_CTRL_ACK_UNKNOWN) && (status == A2DP_CTRL_ACK_PENDING)) {
        ALOGW("status already changed to = %d, don't update pending",audio_stream.ack_status);
    }
    else {
        audio_stream.ack_status = status;
    }
    resp_received = true;
    if (!ack_recvd)
    {
        ack_recvd = 1;
        pthread_cond_signal(&ack_cond);
    }
    pthread_mutex_unlock(&audio_stream.ack_lock);
}

void bt_stack_on_stream_suspended(tA2DP_CTRL_ACK status)
{
    ALOGW("bt_stack_on_stream_suspended status = %d, ack_status = %d ", status, audio_stream.ack_status);
    pthread_mutex_lock(&audio_stream.ack_lock);
    if ((audio_stream.ack_status != A2DP_CTRL_ACK_UNKNOWN) && (status == A2DP_CTRL_ACK_PENDING)) {
        ALOGW("status already changed to = %d, don't update pending",audio_stream.ack_status);
    }
    else {
        audio_stream.ack_status = status;
        ALOGW("bt_stack_on_stream_suspended updating  ack_status = %d ", audio_stream.ack_status);
    }
    resp_received = true;
    if (!ack_recvd)
    {
        ack_recvd = 1;
        ALOGW("bt_stack_on_stream_suspended signalling pthread ");
        pthread_cond_signal(&ack_cond);
    }
    pthread_mutex_unlock(&audio_stream.ack_lock);
    ALOGW("bt_stack_on_stream_suspended mutex unlocked ");
}

void bt_stack_on_stream_stopped(tA2DP_CTRL_ACK status)
{
    ALOGW("bt_stack_on_stream_stopped");
    pthread_mutex_lock(&audio_stream.ack_lock);
    if ((audio_stream.ack_status != A2DP_CTRL_ACK_UNKNOWN) && (status == A2DP_CTRL_ACK_PENDING)) {
        ALOGW("status already changed to = %d, don't update pending",audio_stream.ack_status);
    }
    else {
        audio_stream.ack_status = status;
    }
    resp_received = true;
    if (!ack_recvd)
    {
        ack_recvd = 1;
        pthread_cond_signal(&ack_cond);
    }
    pthread_mutex_unlock(&audio_stream.ack_lock);
}

void bt_stack_on_get_codec_cfg(tA2DP_CTRL_ACK status, const char *p_cfg,
                                  size_t len)
{
    ALOGW("bt_stack_on_get_codec_config status %s",dump_a2dp_ctrl_ack(status));
    if (status != A2DP_CTRL_ACK_PENDING)
    {
        pthread_mutex_lock(&audio_stream.ack_lock);
        audio_stream.ack_status = status;
        ALOGW("bt_stack_on_get_codec_config len = %d",len);
        if (len > MAX_CODEC_CFG_SIZE) {
          ALOGE("codec config length > MAX_CODEC_CFG_SIZE");
          status = A2DP_CTRL_ACK_FAILURE;
        }
        if (status == A2DP_CTRL_ACK_SUCCESS)
        {
            memcpy(audio_stream.codec_cfg,p_cfg,len);
            for (int i = 0; i < len; i++) {
                ALOGV("audio_stream.codec_cfg[%d] = %x",i,audio_stream.codec_cfg[i]);
            }
        }
        resp_received = true;
        if (!ack_recvd)
        {
            ack_recvd = 1;
            pthread_cond_signal(&ack_cond);
        }
        pthread_mutex_unlock(&audio_stream.ack_lock);
    }
    else
    {
        ALOGW("bt_stack_on_get_codec_cfg status pending");
    }
}

void bt_stack_on_get_mcast_status(uint8_t status)
{
    ALOGW("bt_stack_on_get_mcast_status");
    pthread_mutex_lock(&audio_stream.ack_lock);
    audio_stream.multicast = status;
    resp_received = true;
    if (!ack_recvd)
    {
        ack_recvd = 1;
        pthread_cond_signal(&ack_cond);
    }
    pthread_mutex_unlock(&audio_stream.ack_lock);
}

void bt_stack_on_get_num_connected_devices(uint8_t num_dev)
{
    ALOGW("bt_stack_on_get_num_connected_devices");
    pthread_mutex_lock(&audio_stream.ack_lock);
    audio_stream.num_conn_dev = num_dev;
    resp_received = true;
    if (!ack_recvd)
    {
        ack_recvd = 1;
        pthread_cond_signal(&ack_cond);
    }
    pthread_mutex_unlock(&audio_stream.ack_lock);
}

void bt_stack_on_get_connection_status(tA2DP_CTRL_ACK status)
{
    ALOGW("bt_stack_on_get_connection_status");
    pthread_mutex_lock(&audio_stream.ack_lock);
    audio_stream.ack_status = status;
    resp_received = true;
    if (!ack_recvd)
    {
        ack_recvd = 1;
        pthread_cond_signal(&ack_cond);
    }
    pthread_mutex_unlock(&audio_stream.ack_lock);
}
void bt_stack_on_check_a2dp_ready(tA2DP_CTRL_ACK status)
{
    ALOGW("bt_stack_on_check_a2dp_ready");
    pthread_mutex_lock(&audio_stream.ack_lock);
    audio_stream.ack_status = status;
    resp_received = true;
    if (!ack_recvd)
    {
        ack_recvd = 1;
        pthread_cond_signal(&ack_cond);
    }
    pthread_mutex_unlock(&audio_stream.ack_lock);
}

void bt_stack_on_get_sink_latency(uint16_t latency)
{
    if(update_initial_sink_latency == false)
    {
        ALOGW("bt_stack_on_get_sink_latency: Async Latency Update");
        audio_stream.sink_latency = latency;
        return;
    }

    ALOGW("bt_stack_on_get_sink_latency: %d", latency);
    pthread_mutex_lock(&audio_stream.ack_lock);
    audio_stream.sink_latency = latency;
    resp_received = true;
    if (!ack_recvd)
    {
        ack_recvd = 1;
        pthread_cond_signal(&ack_cond);
    }
    pthread_mutex_unlock(&audio_stream.ack_lock);
}

int audio_start_stream()
{
    int i, j, ack_ret;
    tA2DP_CTRL_ACK status = A2DP_CTRL_ACK_SUCCESS;
    ALOGW("%s: state = %s",__func__,dump_a2dp_hal_state(audio_stream.state));
    pthread_mutex_lock(&audio_stream.lock);
    if (audio_stream.state == AUDIO_A2DP_STATE_SUSPENDED)
    {
        ALOGW("stream suspended");
        pthread_mutex_unlock(&audio_stream.lock);
        return -1;
    }
    if (property_get("persist.vendor.bluetooth.a2dp.hal.implementation", a2dp_hal_imp, "false") &&
            !strcmp(a2dp_hal_imp, "true"))
    {
      if (audio_stream.state == AUDIO_A2DP_STATE_STARTED)
      {
          INFO("stream already started");
          pthread_mutex_unlock(&audio_stream.lock);
          return 0;
      }
    }
    for (j = 0; j <STREAM_START_MAX_RETRY_LOOPER; j++) {
        for (i = 0; i < STREAM_START_MAX_RETRY_COUNT; i++)
        {
            if (stack_cb)
            {
                audio_stream.ack_status = A2DP_CTRL_ACK_UNKNOWN;
                resp_received = false;
                stack_cb->start_req_cb();
                ack_recvd = 0;
                if (!resp_received)
                {
                    ack_ret = wait_for_stack_response(1);
                    if (ack_ret == CTRL_CHAN_RETRY_COUNT && !ack_recvd)
                    {
                        ALOGE("audio_start_stream: Failed to get ack from stack");
                        status = -1;
                        goto end;
                    }
                }
                status = audio_stream.ack_status;
                audio_stream.ack_status = A2DP_CTRL_ACK_UNKNOWN;
                ALOGW("audio_start_stream status = %s",dump_a2dp_ctrl_ack(status));
                if (status == A2DP_CTRL_ACK_PENDING)
                {
                    ALOGW("waiting in pending");
                    ack_recvd = 0;
                    if (property_get("persist.vendor.bluetooth.a2dp.hal.implementation", a2dp_hal_imp, "false") &&
                            !strcmp(a2dp_hal_imp, "true"))
                    {
                        wait_for_stack_response(1);
                        if (audio_stream.ack_status == A2DP_CTRL_ACK_UNKNOWN)
                        {
                            ALOGW("audio_start_stream ack not received, fake as success");
                            status = A2DP_CTRL_ACK_SUCCESS;
                        }
                        else
                        {
                            status = audio_stream.ack_status;
                        }
                    }
                    else
                    {
                        wait_for_stack_response(5);
                        status = audio_stream.ack_status;
                    }
                    ALOGW("done waiting in pending status = %s",dump_a2dp_ctrl_ack(status));
                    audio_stream.ack_status = A2DP_CTRL_ACK_UNKNOWN;
                }

                if (status == A2DP_CTRL_ACK_SUCCESS)
                {
                    ALOGW("a2dp stream started successfully");
                    audio_stream.state = AUDIO_A2DP_STATE_STARTED;
                    goto end;
                }
                else if (status == A2DP_CTRL_ACK_INCALL_FAILURE ||
                         status == A2DP_CTRL_ACK_UNSUPPORTED ||
                         status == A2DP_CTRL_ACK_DISCONNECT_IN_PROGRESS ||
                         status == A2DP_CTRL_ACK_UNKNOWN)
                {
                    ALOGW("a2dp stream start failed: status = %s",dump_a2dp_ctrl_ack(status));
                    audio_stream.state = AUDIO_A2DP_STATE_STOPPED;
                    goto end;
                }
                else if (property_get("persist.vendor.bluetooth.a2dp.hal.implementation", a2dp_hal_imp, "false") &&
                        !strcmp(a2dp_hal_imp, "true") &&
                        status == A2DP_CTRL_ACK_PREVIOUS_COMMAND_PENDING)
                {
                    ALOGW("a2dp stream start exited as prev command is pending, fake as success");
                    audio_stream.state = AUDIO_A2DP_STATE_STARTED;
                    goto end;
                }
                else if (status == A2DP_CTRL_ACK_FAILURE)
                {
                    ALOGW("a2dp stream start failed: generic failure");
                }
            }
            else
            {
                ALOGW("%s:Stack shutdown",__func__);
                pthread_mutex_unlock(&audio_stream.lock);
                return A2DP_CTRL_SKT_DISCONNECTED;
            }
            ALOGW("%s: a2dp stream not started,wait 100mse & retry", __func__);
            usleep(100000);
        }
        ALOGW("%s: Check if valid connection is still up or not", __func__);

        // For every 1 sec check if a2dp is still up, to avoid
        // blocking the audio thread forever if a2dp connection is closed
        // for some reason
        audio_stream.ack_status = A2DP_CTRL_ACK_UNKNOWN;
        resp_received = false;
        stack_cb->get_connection_status_cb();
        ack_recvd = 0;
        if (!resp_received)
        {
            ack_ret = wait_for_stack_response(1);
            if (ack_ret == CTRL_CHAN_RETRY_COUNT && !ack_recvd)
            {
                ALOGE("audio_start_stream: Failed to get ack from stack");
                status = -1;
                goto end;
            }
        }
        status = audio_stream.ack_status;
        audio_stream.ack_status = A2DP_CTRL_ACK_UNKNOWN;
        if (status != A2DP_CTRL_ACK_SUCCESS)
        {
            ALOGE("%s: No valid a2dp connection\n", __func__);
            pthread_mutex_unlock(&audio_stream.lock);
            return -1;
        }
    }
end:
    if (audio_stream.state != AUDIO_A2DP_STATE_STARTED)
    {
        ALOGE("%s: Failed to start a2dp stream", __func__);
        pthread_mutex_unlock(&audio_stream.lock);
        return status;
    }
    pthread_mutex_unlock(&audio_stream.lock);
    INFO("stream successfully started");
    return status;
}

int audio_stream_open()
{
    ALOGW("%s",__func__);
    a2dp_stream_common_init(&audio_stream);
    bt_split_a2dp_enabled = true;
    if (stack_cb != NULL)
    {
        ALOGW("audio_stream_open: Success");
        return 0;
    }
    ALOGW("audio_stream_open: Failed");
    return -1;
}

int audio_stream_close()
{
    ALOGW("%s",__func__);
    tA2DP_CTRL_ACK status = A2DP_CTRL_ACK_SUCCESS;
    pthread_mutex_lock(&audio_stream.lock);
    if (audio_stream.state == AUDIO_A2DP_STATE_STARTED ||
        audio_stream.state == AUDIO_A2DP_STATE_STOPPING)
    {
        ALOGW("%s: Suspending audio stream",__func__);
        if (stack_cb)
        {
            int ack_ret = 0;
            audio_stream.ack_status = A2DP_CTRL_ACK_UNKNOWN;
            resp_received = false;
            stack_cb->suspend_req_cb();
            ack_recvd = 0;
            if (!resp_received)
            {
                ack_ret = wait_for_stack_response(1);
                if (ack_ret == 3 &&
                    audio_stream.ack_status == A2DP_CTRL_ACK_UNKNOWN)
                {
                    ALOGE("audio_stream_close: Failed to get ack from stack");
                    pthread_mutex_unlock(&audio_stream.lock);
                    return -1;
                }
            }
        }
    }
    pthread_mutex_unlock(&audio_stream.lock);
    return 0;
}
int audio_stop_stream()
{
    ALOGW("%s",__func__);
    int ret = -1;
    tA2DP_CTRL_ACK status;
    pthread_mutex_lock(&audio_stream.lock);
    if (stack_cb)
    {
        if (audio_stream.state != AUDIO_A2DP_STATE_SUSPENDED)
        {
            int ack_ret = 0;
            ack_recvd = 0;
            resp_received = false;
            audio_stream.ack_status = A2DP_CTRL_ACK_UNKNOWN;
            stack_cb->suspend_req_cb();
            if (!resp_received)
            {
                ack_ret = wait_for_stack_response(1);
                if (ack_ret == CTRL_CHAN_RETRY_COUNT && !ack_recvd)
                {
                    ALOGE("audio_stop_stream: Failed to get ack from stack");
                    pthread_mutex_unlock(&audio_stream.lock);
                    return -1;
                }
            }
            status = audio_stream.ack_status;
            audio_stream.ack_status = A2DP_CTRL_ACK_UNKNOWN;
            ALOGW("audio_stop_stream: ack status = %s",dump_a2dp_ctrl_ack(status));
            if (status == A2DP_CTRL_ACK_PENDING)
            {
                ack_recvd = 0;
                if (property_get("persist.vendor.bluetooth.a2dp.hal.implementation", a2dp_hal_imp, "false") &&
                        !strcmp(a2dp_hal_imp, "true"))
                {
                    wait_for_stack_response(1);
                }
                else
                {
                    wait_for_stack_response(5);
                }
                status = audio_stream.ack_status;
                audio_stream.ack_status = A2DP_CTRL_ACK_UNKNOWN;
                if (status == A2DP_CTRL_ACK_SUCCESS) ret = 0;
            }

            if (status == A2DP_CTRL_ACK_SUCCESS)
            {
                ALOGW("audio stop stream successful");
                audio_stream.state = AUDIO_A2DP_STATE_STANDBY;
                pthread_mutex_unlock(&audio_stream.lock);
                return 0;
            }
            else if (property_get("persist.vendor.bluetooth.a2dp.hal.implementation", a2dp_hal_imp, "false") &&
                    !strcmp(a2dp_hal_imp, "true") &&
                    status == A2DP_CTRL_ACK_PREVIOUS_COMMAND_PENDING)
            {
                ALOGW("a2dp stream stop exited as prev command is pending, fake as success");
                audio_stream.state = AUDIO_A2DP_STATE_STANDBY;
                pthread_mutex_unlock(&audio_stream.lock);
                return 0;
            }
            else
            {
                ALOGW("audio stop stream failed");
                audio_stream.state = AUDIO_A2DP_STATE_STOPPED;
                pthread_mutex_unlock(&audio_stream.lock);
                return -1;
            }
        }
    }
    else
        ALOGW("stack is down");
    audio_stream.state = AUDIO_A2DP_STATE_STOPPED;
    pthread_mutex_unlock(&audio_stream.lock);
    return ret;
}

int audio_suspend_stream()
{
    ALOGW("%s",__func__);
    tA2DP_CTRL_ACK status;

    pthread_mutex_lock(&audio_stream.lock);
    if (stack_cb)
    {
        if (audio_stream.state != AUDIO_A2DP_STATE_SUSPENDED)
        {
            int ack_ret = 0;
            ack_recvd = 0;
            resp_received = false;
            audio_stream.ack_status = A2DP_CTRL_ACK_UNKNOWN;
            stack_cb->suspend_req_cb();
            if (!resp_received)
            {
                ack_ret = wait_for_stack_response(1);
                if (ack_ret == CTRL_CHAN_RETRY_COUNT && !ack_recvd)
                {
                    ALOGE("audio_suspend_stream: Failed to get ack from stack");
                    pthread_mutex_unlock(&audio_stream.lock);
                    return -1;
                }
            }
            status = audio_stream.ack_status;
            audio_stream.ack_status = A2DP_CTRL_ACK_UNKNOWN;
            ALOGW("audio_suspend_stream: ack status = %s",dump_a2dp_ctrl_ack(status));
            if (status == A2DP_CTRL_ACK_PENDING)
            {
                //TODO wait for the response;
                ack_recvd = 0;
                if (property_get("persist.vendor.bluetooth.a2dp.hal.implementation", a2dp_hal_imp, "false") &&
                        !strcmp(a2dp_hal_imp, "true"))
                {
                    wait_for_stack_response(1);
                }
                else
                {
                    wait_for_stack_response(5);
                }
                status = audio_stream.ack_status;
                audio_stream.ack_status = A2DP_CTRL_ACK_UNKNOWN;
            }

            if (status == A2DP_CTRL_ACK_SUCCESS)
            {
                ALOGW("audio suspend stream successful");
                audio_stream.state = AUDIO_A2DP_STATE_SUSPENDED;
                pthread_mutex_unlock(&audio_stream.lock);
                return 0;
            }
            else if (property_get("persist.vendor.bluetooth.a2dp.hal.implementation", a2dp_hal_imp, "false") &&
                    !strcmp(a2dp_hal_imp, "true") &&
                    status == A2DP_CTRL_ACK_PREVIOUS_COMMAND_PENDING)
            {
                ALOGW("a2dp stream suspend exited as prev command is pending, fake as success");
                pthread_mutex_unlock(&audio_stream.lock);
                audio_stream.state = AUDIO_A2DP_STATE_SUSPENDED;
                return 0;
            }
            else
            {
                ALOGW("audio suspend stream failed");
                pthread_mutex_unlock(&audio_stream.lock);
                return -1;
            }
        }
    }
    else
        ALOGW("stack is down");
    pthread_mutex_unlock(&audio_stream.lock);
    return -1;
}

void audio_handoff_triggered()
{
    ALOGW("%s state = %s",__func__,dump_a2dp_hal_state(audio_stream.state));
    pthread_mutex_lock(&audio_stream.lock);
    if (audio_stream.state != AUDIO_A2DP_STATE_STOPPED ||
        audio_stream.state != AUDIO_A2DP_STATE_STOPPING)
    {
        audio_stream.state = AUDIO_A2DP_STATE_STOPPED;
    }
    pthread_mutex_unlock(&audio_stream.lock);
}

void clear_a2dpsuspend_flag()
{
    ALOGW("%s: state = %s",__func__,dump_a2dp_hal_state(audio_stream.state));
    pthread_mutex_lock(&audio_stream.lock);
    if (audio_stream.state == AUDIO_A2DP_STATE_SUSPENDED)
        audio_stream.state = AUDIO_A2DP_STATE_STOPPED;
    pthread_mutex_unlock(&audio_stream.lock);
}

void * audio_get_codec_config(uint8_t *multicast_status, uint8_t *num_dev,
                              audio_format_t *codec_type)
{
    int i, status;
    ALOGW("%s: state = %s",__func__,dump_a2dp_hal_state(audio_stream.state));

    pthread_mutex_lock(&audio_stream.lock);
    a2dp_get_multicast_status(multicast_status);
    if (*multicast_status)
    {
        a2dp_get_num_connected_devices(num_dev);
    }
    else
        *num_dev = 1;
    ALOGW("got multicast status = %d dev = %d",*multicast_status,*num_dev);
    update_initial_sink_latency = true;

    for (i = 0; i < STREAM_START_MAX_RETRY_COUNT; i++)
    {
        status = a2dp_read_codec_config(&audio_stream, 0);
        if (status == A2DP_CTRL_ACK_SUCCESS)
        {
            pthread_mutex_unlock(&audio_stream.lock);
            if (stack_cb == NULL) {
               ALOGW("get codec config returned due to stack deinit");
               return NULL;
            }
            return (a2dp_codec_parser(&audio_stream.codec_cfg[0], codec_type, NULL));
        }
        INFO("%s: a2dp stream not configured,wait 100mse & retry", __func__);
        usleep(100000);
    }
    pthread_mutex_unlock(&audio_stream.lock);
    return NULL;
}

void* audio_get_next_codec_config(uint8_t idx, audio_format_t *codec_type)
{
    int i, status;
    ALOGW("%s",__func__);
    pthread_mutex_lock(&audio_stream.lock);
    for (i = 0; i < STREAM_START_MAX_RETRY_COUNT; i++)
    {
        status = a2dp_read_codec_config(&audio_stream,idx);
        if (status == A2DP_CTRL_ACK_SUCCESS)
        {
            pthread_mutex_unlock(&audio_stream.lock);
            return (a2dp_codec_parser(&audio_stream.codec_cfg[0], codec_type, NULL));
        }
        INFO("%s: a2dp stream not configured,wait 100mse & retry", __func__);
        usleep(100000);
    }
    pthread_mutex_unlock(&audio_stream.lock);
    return NULL;
}

int audio_check_a2dp_ready()
{
    int i, ack_ret;
    ALOGW("audio_check_a2dp_ready: state %s", dump_a2dp_hal_state(audio_stream.state));
    tA2DP_CTRL_ACK status;
    pthread_mutex_lock(&audio_stream.lock);
    if (property_get("persist.vendor.bluetooth.a2dp.hal.implementation", a2dp_hal_imp, "false") &&
            !strcmp(a2dp_hal_imp, "true") &&
            audio_stream.state == AUDIO_A2DP_STATE_SUSPENDED)
    {
        INFO("stream not ready to start");
        pthread_mutex_unlock(&audio_stream.lock);
        return 0;
    }

    for (i = 0; i < CHECK_A2DP_READY_MAX_COUNT; i++)
    {
        pthread_mutex_lock(&audio_stream.ack_lock);
        if (stack_cb != NULL) {
            audio_stream.ack_status = A2DP_CTRL_ACK_UNKNOWN;
            ack_recvd = 0;
            stack_cb->a2dp_check_ready_cb();
            pthread_mutex_unlock(&audio_stream.ack_lock);
        } else {
            ALOGW("audio_check_a2dp_ready = NOT ready - callbacks not registered");
            pthread_mutex_unlock(&audio_stream.ack_lock);
            pthread_mutex_unlock(&audio_stream.lock);
            return 0;
        }

        ack_ret = wait_for_stack_response(1);
        status = audio_stream.ack_status;
        if (status == A2DP_CTRL_ACK_SUCCESS)
        {
            ALOGW("audio_check_a2dp_ready : %s",dump_a2dp_ctrl_ack(status));
            pthread_mutex_unlock(&audio_stream.lock);
            return 1;
        }
        if (ack_ret == CTRL_CHAN_RETRY_COUNT && !ack_recvd)
        {
            ALOGE("audio_check_a2dp_ready: Failed to get ack from stack");
            pthread_mutex_unlock(&audio_stream.lock);
            return 0;
        }
        ALOGW("audio_check_a2dp_ready(): a2dp stream not ready, wait 200msec & retry");
        usleep(200000);
    }
    audio_stream.ack_status = A2DP_CTRL_ACK_UNKNOWN;
    ALOGW("audio_check_a2dp_ready = %s",dump_a2dp_ctrl_ack(status));

    pthread_mutex_unlock(&audio_stream.lock);
    return status == A2DP_CTRL_ACK_SUCCESS;
}

uint16_t audio_get_a2dp_sink_latency()
{
    ALOGD_IF(DEBUG, "%s: state = %s",__func__,dump_a2dp_hal_state(audio_stream.state));
    pthread_mutex_lock(&audio_stream.lock);
    if (update_initial_sink_latency)
    {
        if (stack_cb)
        {
            resp_received = false;
            stack_cb->get_sink_latency_cb();
            ack_recvd = 0;
            if (resp_received == false)
                wait_for_stack_response(1);
        }
        else
            audio_stream.sink_latency = A2DP_DEFAULT_SINK_LATENCY;
        update_initial_sink_latency = false;
    }
    pthread_mutex_unlock(&audio_stream.lock);
    return audio_stream.sink_latency;
}

/* Returns true if TWS encoder to be configure with mono mode
        False if TWS encoder to be configured with stereo mode */
bool isTwsMonomodeEnable(void)
{
   if (tws_channelmode)
        return true;
   else
       return false;
}

bool audio_is_scrambling_enabled(void)
{
    audio_format_t codec_type = AUDIO_FORMAT_DEFAULT;
    int i;
    char value[PROPERTY_VALUE_MAX];
    uint8_t *codec_cfg = NULL;
    uint32_t sample_freq = 0;
    memset(value, '\0', sizeof(char)*PROPERTY_VALUE_MAX);
    ALOGW("audio_is_scrambling_enabled: state %s",
                    dump_a2dp_hal_state(audio_stream.state));
    tA2DP_CTRL_ACK status = A2DP_CTRL_ACK_UNKNOWN;

    if (stack_cb == NULL)
    {
        ALOGW("audio_is_scrambling_enabled returned false due to stack deinit");
        return false;
    }

    if( property_get("persist.vendor.bluetooth.soc.scram_freqs", value, "false") &&
        !strcmp(value, "false"))
    {
        property_get("persist.vendor.bt.soc.scram_freqs", value, "false");
        if(!strcmp(value, "false"))
        {
            ALOGW("persist.vendor.bt.soc.scram_freqs is not set");
            return false;
        }
    }
    else
    {
        ALOGE("Error in fetching persist.vendor.bluetooth.soc.scram_freqs property");
        return false;
    }
	ALOGE("scram_freqs Prop value = %s", value);

    pthread_mutex_lock(&audio_stream.lock);
    for (i = 0; i < STREAM_START_MAX_RETRY_COUNT; i++)
    {
        status = a2dp_read_codec_config(&audio_stream, 0);
        if (status == A2DP_CTRL_ACK_SUCCESS)
        {
            if(!a2dp_codec_parser(&audio_stream.codec_cfg[0],
                        &codec_type, &sample_freq)) {
                status = A2DP_CTRL_ACK_UNKNOWN;
            }
            break;
        }
        INFO("%s: a2dp stream not configured,wait 100mse & retry", __func__);
        usleep(100000);
    }
    if (codec_type == ENC_CODEC_TYPE_APTX_DUAL_MONO) {
        INFO("%s:TWSP codec, return false",__func__);
        pthread_mutex_unlock(&audio_stream.lock);
        return false;
    }
    if (codec_type == ENC_CODEC_TYPE_APTX_ADAPTIVE) {
        ALOGW("%s:aptX Adaptive codec, return false",__func__);
        pthread_mutex_unlock(&audio_stream.lock);
        return false;
    }

    if(status == A2DP_CTRL_ACK_SUCCESS) {

        if (codec_type == CODEC_TYPE_CELT) {
           INFO("%s: BA going on,return false", __func__);
           pthread_mutex_unlock(&audio_stream.lock);
           return false;
        }

        ALOGW("audio_is_scrambling_enabled sample_freq %ld",sample_freq);
        switch (sample_freq) {
            case 44100:
                if(!strstr(value, "441")) status = A2DP_CTRL_ACK_UNKNOWN;
                break;
            case 48000:
                if(!strstr(value, "48")) status = A2DP_CTRL_ACK_UNKNOWN;
                break;
            case 88200:
                if(!strstr(value, "882")) status = A2DP_CTRL_ACK_UNKNOWN;
                break;
            case 96000:
                if(!strstr(value, "96")) status = A2DP_CTRL_ACK_UNKNOWN;
                break;
            case 176400:
                if(!strstr(value, "1764")) status = A2DP_CTRL_ACK_UNKNOWN;
                break;
            case 192000:
                if(!strstr(value, "192")) status = A2DP_CTRL_ACK_UNKNOWN;
                break;
            default:
                ALOGE("Invalid sampling freqency, return A2DP_CTRL_ACK_UNKNOWN");
                status = A2DP_CTRL_ACK_UNKNOWN;
                break;
        }
    }
    ALOGW("audio_is_scrambling_enabled = %s",dump_a2dp_ctrl_ack(status));
    pthread_mutex_unlock(&audio_stream.lock);
    return status == A2DP_CTRL_ACK_SUCCESS;
}

void ldac_codec_parser(uint8_t *codec_cfg)
{
    char byte,len;
    uint8_t *p_cfg = codec_cfg;
    memset(&ldac_codec,0,sizeof(audio_ldac_encoder_config_t));
    p_cfg++; //skip dev_idx
    len = *p_cfg++;//LOSC
    p_cfg++; // Skip media type
    len--;
    p_cfg++; //codec_type
    len--;
    p_cfg+=4;//skip vendor id
    len -= 4;
    p_cfg += 2; //skip codec id
    len -= 2;
    byte = *p_cfg++;
    len--;
    switch (byte & A2D_LDAC_SAMP_FREQ_MASK)
    {
        case A2D_LDAC_SAMP_FREQ_44:
             ldac_codec.sampling_rate = 44100;
             break;
        case A2D_LDAC_SAMP_FREQ_48:
             ldac_codec.sampling_rate = 48000;
             break;
        case A2D_LDAC_SAMP_FREQ_88:
             ldac_codec.sampling_rate = 88200;
             break;
        case A2D_LDAC_SAMP_FREQ_96:
             ldac_codec.sampling_rate = 96000;
             break;
        case A2D_LDAC_SAMP_FREQ_176:
             ldac_codec.sampling_rate = 176400;
             break;
        case A2D_LDAC_SAMP_FREQ_192:
             ldac_codec.sampling_rate = 192000;
             break;
        default:
             ALOGE("Unknown sampling rate");
    }
    ALOGW("%s: LDAC: sample rate: %lu", __func__, ldac_codec.sampling_rate);
    byte = *p_cfg++;
    len--;
    ldac_codec.channel_mode = (byte & A2D_LDAC_CHAN_MASK);
    if (len == 0)
    {
        ALOGW("Codec config copied");
    }
    ldac_codec.mtu = DEFAULT_MTU_SIZE;
    p_cfg += 2;

    ldac_codec.bitrate = *p_cfg++;
    ldac_codec.bitrate |= (*p_cfg++ << 8);
    ldac_codec.bitrate |= (*p_cfg++ << 16);
    ldac_codec.bitrate |= (*p_cfg++ << 24);

    ldac_codec.is_abr_enabled = (ldac_codec.bitrate == 0);
    ldac_codec.bits_per_sample = *(uint32_t *)p_cfg;
    ALOGW("Create Lookup for %d with ABR %d, bits_per_sample %d", ldac_codec.sampling_rate, ldac_codec.is_abr_enabled, ldac_codec.bits_per_sample);
    if (ldac_codec.sampling_rate == 44100 ||
            ldac_codec.sampling_rate == 88200) {
        int num_of_level_entries =
            sizeof(bit_rate_level_44_1k_88_2k_database)/sizeof(bit_rate_level_44_1k_88_2k_table_t);
        ldac_codec.level_to_bitrate_map.num_levels = num_of_level_entries;
        if (ldac_codec.is_abr_enabled) {
         ldac_codec.bitrate = bit_rate_level_44_1k_88_2k_database[0].bit_rate_value;
         ALOGW("Send start highest bit-rate value %d", ldac_codec.bitrate);
        }
        for (int i = 0; i < num_of_level_entries; i++) {
            ldac_codec.level_to_bitrate_map.bit_rate_level_map[i].link_quality_level =
                bit_rate_level_44_1k_88_2k_database[i].level_value;
            ldac_codec.level_to_bitrate_map.bit_rate_level_map[i].bitrate =
                bit_rate_level_44_1k_88_2k_database[i].bit_rate_value;
            ALOGW("Level: %d, bit-rate: %d",
                ldac_codec.level_to_bitrate_map.bit_rate_level_map[i].link_quality_level,
                ldac_codec.level_to_bitrate_map.bit_rate_level_map[i].bitrate);
        }
    } else if (ldac_codec.sampling_rate == 48000 ||
            ldac_codec.sampling_rate == 96000) {
        int num_of_level_entries =
            sizeof(bit_rate_level_48k_96k_database)/sizeof(bit_rate_level_48k_96k_table_t);
        ldac_codec.level_to_bitrate_map.num_levels = num_of_level_entries;
        if (ldac_codec.is_abr_enabled) {
         ldac_codec.bitrate = bit_rate_level_48k_96k_database[0].bit_rate_value;
         ALOGW("Send start highest bit-rate value %d", ldac_codec.bitrate);
        }
        for (int i = 0; i < num_of_level_entries; i++) {
            ldac_codec.level_to_bitrate_map.bit_rate_level_map[i].link_quality_level =
                bit_rate_level_48k_96k_database[i].level_value;
            ldac_codec.level_to_bitrate_map.bit_rate_level_map[i].bitrate =
                bit_rate_level_48k_96k_database[i].bit_rate_value;
            ALOGW("Level: %d, bit-rate: %d",
                ldac_codec.level_to_bitrate_map.bit_rate_level_map[i].link_quality_level,
                ldac_codec.level_to_bitrate_map.bit_rate_level_map[i].bitrate);
        }
    } else {
        ALOGW("Unsupported Invalid frequency");
    }
    ALOGW("%s: LDAC: bitrate: %lu", __func__, ldac_codec.bitrate);
    ALOGW("LDAC: Done copying full codec config");
}
