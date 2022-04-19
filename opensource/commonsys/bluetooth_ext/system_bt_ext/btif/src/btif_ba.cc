/*
 * Copyright (c) 2017, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of The Linux Foundation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
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

#include <hardware/bt_ba.h>
#include <stdlib.h>
#include <string.h>

#define LOG_TAG "bt_btif_vendor"
#include "btif_a2dp.h"
#include <cutils/properties.h>
#include "bt_utils.h"
#include "btif_common.h"
#include "btif_util.h"
#include "btif_profile_queue.h"
#include "stack_manager.h"
#include "l2cdefs.h"
#include "l2c_api.h"
#include "stack_config.h"
#include "btm_api.h"
#include "btif_av.h"
#include "btif_bat.h"
#include "sdp_api.h"
#include <time.h>
#include "btif_a2dp_audio_interface.h"
#include "bta_bat.h"
#include "btif_av.h"
#include "btif_config.h"

extern void btif_av_trigger_suspend();

static ba_transmitter_callbacks_t *ba_transmitter_callback = NULL;


#define MSG_QUE_LEN 100

typedef struct {
    uint8_t msg;
    uint8_t state;
}btif_ba_msg_t;
btif_ba_msg_t rem_msg[MSG_QUE_LEN];

typedef struct {
    btif_sm_handle_t sm_handle;
    uint8_t encryption_key[ENCRYPTION_KEY_LEN];
    uint8_t div[DIV_KEY_LEN];
    uint8_t active_stream_id;
    uint32_t bit_rate;
    uint8_t codec_type;
    uint8_t sampl_freq;
    uint8_t ch_mode;
    uint8_t frame_size;
    uint8_t complexity;
    uint8_t prediction_mode;
    uint8_t vbr_mode;
    uint8_t audio_cmd_pending;
    uint8_t curr_vol_level;
    uint8_t max_vol_level;
    uint8_t prev_state;
}btif_ba_cb_t;

static btif_ba_cb_t btif_ba_cb = {
    NULL,{0},{0},0,0,0,0,0,0,0,0,0,0,0,0,0
    };

static bool btif_ba_state_idle_audio_ns_handler(btif_sm_event_t event,
    void* data, int idx);
static bool btif_ba_state_idle_audio_pending_handler(btif_sm_event_t event,
    void* data, int idx);
static bool btif_ba_state_idle_audio_streaming_handler(btif_sm_event_t event,
    void* data, int idx);
static bool btif_ba_state_pending_audio_ns_handler(btif_sm_event_t event,
    void* data, int idx);
static bool btif_ba_state_paused_audio_ns_handler(btif_sm_event_t event,
    void* data, int idx);
static bool btif_ba_state_streaming_audio_ns_handler(btif_sm_event_t event,
    void* data, int idx);

static const btif_sm_handler_t btif_ba_state_handlers[] = {
    btif_ba_state_idle_audio_pending_handler,
    btif_ba_state_idle_audio_streaming_handler,
    btif_ba_state_idle_audio_ns_handler,
    btif_ba_state_pending_audio_ns_handler,
    btif_ba_state_paused_audio_ns_handler,
    btif_ba_state_streaming_audio_ns_handler
};

void dump_curr_codec_config();
/*******************************************************************************
** Broadcast Audio Interface Functions
*******************************************************************************/

/*******************************************************************************
**
** Function         init
**
** Description     initializes ba transmitter.
**
** Returns         bt_status_t
**
*******************************************************************************/
const char* dump_ba_sm_event_name(btif_ba_sm_event_t event) {
  switch ((int)event) {
    CASE_RETURN_STR(BTIF_BA_API_INIT_REQ_EVT)
    CASE_RETURN_STR(BTIF_BA_API_SET_STATE_START_REQ_EVT)
    CASE_RETURN_STR(BTIF_BA_API_SET_STATE_STOP_REQ_EVT)
    CASE_RETURN_STR(BTIF_BA_BT_A2DP_DISC_EVT)
    CASE_RETURN_STR(BTIF_BA_BT_A2DP_PAUSED_EVT)
    CASE_RETURN_STR(BTIF_BA_BT_A2DP_STARTED_EVT)
    CASE_RETURN_STR(BTIF_BA_BT_A2DP_STARTING_EVT)
    CASE_RETURN_STR(BTIF_BA_CMD_PAUSE_REQ_EVT)
    CASE_RETURN_STR(BTIF_BA_CMD_STREAM_REQ_EVT)
    CASE_RETURN_STR(BTIF_BA_CMD_STOP_REQ_EVT)
    CASE_RETURN_STR(BTIF_BA_RSP_STOP_DONE_EVT)
    CASE_RETURN_STR(BTIF_BA_RSP_PAUSE_DONE_EVT)
    CASE_RETURN_STR(BTIF_BA_RSP_STREAM_DONE_EVT)
    CASE_RETURN_STR(BTIF_BA_AUDIO_START_REQ_EVT)
    CASE_RETURN_STR(BTIF_BA_AUDIO_PAUSE_REQ_EVT)
    CASE_RETURN_STR(BTIF_BA_AUDIO_STOP_REQ_EVT)
    CASE_RETURN_STR(BTIF_BA_API_DEINIT_REQ_EVT)
    CASE_RETURN_STR(BTIF_BA_API_REFRESH_ENC_KEY_REQ_EVT)
    CASE_RETURN_STR(BTIF_BA_API_SET_VOL_LEVEL)
    CASE_RETURN_STR(BTIF_BA_CMD_SEND_VOL_UPDATE)
    CASE_RETURN_STR(BTIF_BA_RSP_VOL_UPDATE_DONE_EVT)
    CASE_RETURN_STR(BTIF_BA_CMD_UPDATE_ENC_KEY)
    CASE_RETURN_STR(BTIF_BA_RSP_ENC_KEY_UPDATE_DONE_EVT)
    CASE_RETURN_STR(BTIF_SM_ENTER_EVT)
    CASE_RETURN_STR(BTIF_SM_EXIT_EVT)
    default:
      return "UNKNOWN_EVENT";
  }
}

static void btif_ba_handle_event(uint16_t event, char* p_param) {
    BTIF_TRACE_EVENT("%s  event = %d", __FUNCTION__, event);
    btif_sm_dispatch(btif_ba_cb.sm_handle, event, (void*)p_param);
}

void btif_ba_bta_callback(uint16_t event, uint8_t result)
{
    BTIF_TRACE_DEBUG(" %s  event = %d, result = %x ",__FUNCTION__, event, result);
    if(result != BT_STATUS_SUCCESS)
      result = BT_STATUS_FAIL;// right now we are using just success and fail
    btif_transfer_context(btif_ba_handle_event, event,
        (char*)&result, sizeof(uint8_t), NULL);
}
static bt_status_t bat_init( ba_transmitter_callbacks_t* callbacks)
{
    LOG_INFO(LOG_TAG,"bat_init");
    ba_transmitter_callback = callbacks;
    btif_ba_cb.sm_handle = btif_sm_init(
     (const btif_sm_handler_t*)btif_ba_state_handlers, BTIF_BA_STATE_IDLE_AUDIO_NS, 0);
    btif_transfer_context(btif_ba_handle_event, BTIF_BA_API_INIT_REQ_EVT,
        NULL, 0, NULL);
    btif_ba_cb.prev_state = BTIF_BA_STATE_IDLE_AUDIO_NS;
    return BT_STATUS_SUCCESS;
}

static bt_status_t bat_setstate( uint8_t state )
{
    LOG_INFO(LOG_TAG,"%s state = %d", __FUNCTION__, state);
    if (state == 0) // STOP BA
    {
        btif_transfer_context(btif_ba_handle_event,
          BTIF_BA_API_SET_STATE_STOP_REQ_EVT, NULL, 0, NULL);
    }
    if (state == 1) // START BA
    {
        btif_transfer_context(btif_ba_handle_event,
          BTIF_BA_API_SET_STATE_START_REQ_EVT, NULL, 0, NULL);
    }
    return BT_STATUS_SUCCESS;
}

static bt_status_t refresh_enc_key()
{
    BTIF_TRACE_DEBUG(" %s ", __FUNCTION__);
    btif_transfer_context(btif_ba_handle_event,
          BTIF_BA_API_REFRESH_ENC_KEY_REQ_EVT, NULL, 0, NULL);
    return BT_STATUS_SUCCESS;
}

static bt_status_t set_volume(uint8_t vol, uint8_t max_vol)
{
    BTIF_TRACE_DEBUG(" %s vol = %d max_vol = %d", __FUNCTION__, vol, max_vol);
    btif_transfer_context(btif_ba_handle_event,
        BTIF_BA_API_SET_VOL_LEVEL, (char*)&vol, sizeof(uint8_t), NULL);
    btif_ba_cb.max_vol_level = max_vol;
    return BT_STATUS_SUCCESS;
}

static void cleanup(void)
{
    LOG_INFO(LOG_TAG,"cleanup");
    if (ba_transmitter_callback)
        ba_transmitter_callback = NULL;
    btif_transfer_context(btif_ba_handle_event, BTIF_BA_API_DEINIT_REQ_EVT,
        NULL, 0, NULL);
}

static const ba_transmitter_interface_t batInterface = {
    sizeof(ba_transmitter_interface_t),
    bat_init,
    bat_setstate,
    refresh_enc_key,
    set_volume,
    cleanup,
};

/*******************************************************************************
** LOCAL FUNCTIONS
*******************************************************************************/

/*******************************************************************************
**
** Function         btif_vendor_get_interface
**
** Description      Get the vendor callback interface
**
** Returns          btvendor_interface_t
**
*******************************************************************************/
const ba_transmitter_interface_t *btif_bat_get_interface()
{
    BTIF_TRACE_EVENT("%s", __FUNCTION__);
    return &batInterface;
}

bool btif_ba_is_active()
{
    bool ret = false;
    if (btif_ba_get_state() > BTIF_BA_STATE_IDLE_AUDIO_NS)
        ret = true;
    else
        ret = false;
    LOG_INFO(LOG_TAG,"%s: %d",__func__, ret);
    return ret;
}

btif_ba_state_t btif_ba_get_state()
{
    if (btif_ba_cb.sm_handle == NULL) {
        BTIF_TRACE_EVENT(" %s BA not iniitlized ", __FUNCTION__);
        return BTIF_BA_STATE_IDLE_AUDIO_NS;
    }
    btif_ba_state_t curr_state =
        (btif_ba_state_t)btif_sm_get_state(btif_ba_cb.sm_handle);
    BTIF_TRACE_EVENT("%s curr state = %d", __FUNCTION__, curr_state);
    return curr_state;
}

uint16_t btif_get_ba_latency() {
    BTIF_TRACE_EVENT(" %s  val = %d", __FUNCTION__,(VS_HCI_TTP_OFFSET/ 1000));
    return (VS_HCI_TTP_OFFSET/1000);
}

void ba_send_message(uint8_t event, uint8_t size, char* ptr, bool is_btif_thread)
{
    if (ba_transmitter_callback == NULL)
        return;// in case BA service is not even up, don't process further
    if (( event == BTIF_BA_AUDIO_STOP_REQ_EVT) ||
        ( event == BTIF_BA_AUDIO_PAUSE_REQ_EVT) ||
        ( event == BTIF_BA_AUDIO_START_REQ_EVT))
      {
          btif_ba_cb.audio_cmd_pending = event;
      }
    if (event == BTIF_BA_AUDIO_STOP_REQ_EVT) {
        event = BTIF_BA_AUDIO_PAUSE_REQ_EVT;
    }
    // internally STOP and PAUSE does same action, pause the BA stream.
    BTIF_TRACE_DEBUG(" %s  event  = %d", __FUNCTION__, event);
    if(is_btif_thread) {
        btif_ba_handle_event(event, NULL);
    } else {
        btif_transfer_context(btif_ba_handle_event, event, ptr, 0, NULL);
    }
}

void ba_acknowledge_audio_cmd(uint8_t pending_cmd, uint8_t result)
{
    BTIF_TRACE_DEBUG(" %s pending_cmd = %d audio_cmd_pending = %d result= %d",
             __FUNCTION__, pending_cmd, btif_ba_cb.audio_cmd_pending, result);
    if (btif_ba_cb.audio_cmd_pending == 0)
    {
        BTIF_TRACE_ERROR(" no audio cmd pending, bail out");
        return;
    }
    if (pending_cmd == BTIF_BA_AUDIO_PAUSE_REQ_EVT)
    {
        if ( btif_ba_cb.audio_cmd_pending == BTIF_BA_AUDIO_PAUSE_REQ_EVT)
        {
            btif_ba_audio_on_suspended(result);
        }
        else if ( btif_ba_cb.audio_cmd_pending == BTIF_BA_AUDIO_STOP_REQ_EVT)
        {
            btif_ba_audio_on_stopped(result);
        }
        else
        {
            BTIF_TRACE_ERROR(" mismatch !!! ");
        }
    }
    if (pending_cmd == BTIF_BA_AUDIO_START_REQ_EVT)
    {
        if ( btif_ba_cb.audio_cmd_pending == BTIF_BA_AUDIO_START_REQ_EVT)
        {
            btif_ba_audio_on_started(result);
        }
        else
        {
            BTIF_TRACE_ERROR(" mismatch !!! ");
        }
    }
}

void getBACodecConfig(uint8_t* p_codec_config)
{
    dump_curr_codec_config();
    uint32_t bit_rate = btif_ba_cb.bit_rate;
    //media codec header
    *p_codec_config = 10; p_codec_config ++;// len of codec values
    *p_codec_config = 0; p_codec_config ++;// media_type = 0: Audio
    *p_codec_config = btif_ba_cb.codec_type; p_codec_config ++;// codec_type
    // codec info element
    *p_codec_config = btif_ba_cb.sampl_freq;// freq
    *p_codec_config |= btif_ba_cb.ch_mode; // channel mode
    p_codec_config ++;
    *p_codec_config = btif_ba_cb.frame_size; // fr_aize
    *p_codec_config |= btif_ba_cb.complexity; // complexity
    p_codec_config ++;
    *p_codec_config = btif_ba_cb.prediction_mode;//prediction mode.
    *p_codec_config |= btif_ba_cb.vbr_mode; // vbr mask
    p_codec_config ++;
    // next 4 bytes for bit rate.
    *p_codec_config = bit_rate >> 24;p_codec_config ++;
    *p_codec_config = bit_rate >> 16;p_codec_config ++;
    *p_codec_config = bit_rate >> 8;p_codec_config ++;
    *p_codec_config = bit_rate;p_codec_config ++;
}

uint8_t btif_ba_get_sample_rate()
{
  return btif_ba_cb.sampl_freq;
}

uint8_t btif_ba_get_channel_mode()
{
  return btif_ba_cb.ch_mode;
}

uint8_t btif_ba_get_frame_size()
{
  return btif_ba_cb.frame_size;
}

uint8_t btif_ba_get_complexity()
{
  return btif_ba_cb.complexity;
}

uint8_t btif_ba_get_prediction_mode()
{
  return btif_ba_cb.prediction_mode;
}

uint8_t btif_ba_get_vbr_flag()
{
  return btif_ba_cb.vbr_mode;
}

uint32_t btif_ba_get_bitrate()
{
  return btif_ba_cb.bit_rate;
}

static void memorize_msg(uint8_t event, btif_ba_state_t state)
{
    BTIF_TRACE_DEBUG(" %s  event = %s, state = %d", __FUNCTION__,
        dump_ba_sm_event_name((btif_ba_sm_event_t)event), state);
    uint8_t i = 0;
    for (i = 0; i < MSG_QUE_LEN; i++) {
        if (rem_msg[i].msg == 0) {
            rem_msg[i].msg = event;
            rem_msg[i].state = state;
            BTIF_TRACE_DEBUG(" %s adding @ %d", __FUNCTION__, i);
            return;
        }
    }
    BTIF_TRACE_DEBUG(" %s, MSG List Full index = %d", __FUNCTION__, i);
}

static void handle_memorized_msgs() {
    // first figure out start and end of msg que
    int i = 0;
    uint8_t end_index = 0;
    for (i = 0; i< MSG_QUE_LEN; i++) {
        if (rem_msg[i].msg == 0) {
            break;
        }
    }
    end_index = i;
    BTIF_TRACE_DEBUG(" %s end_index = %d", __FUNCTION__, end_index);
    if ( end_index == 0)// no messages in list.
        return;
    // under assumption that we have enough space to accomodate even if
    // all messages are re memorized as part of processing memorized msgs.
    for (i = 0; i < end_index; i++) {
         btif_sm_dispatch(btif_ba_cb.sm_handle, rem_msg[i].msg, NULL);
    }
    // we have acted upon all rememberd messages, make everything 0 now.
    for(i = (end_index -1) ; i>= 0; i--) {
        rem_msg[i].msg = 0;
    }

    BTIF_TRACE_DEBUG(" %s out of function", __FUNCTION__);
}

void refresh_encryption_key(bool overwrite) {
    uint8_t i = 0;
    bool enc_key_available = false;
    bool ret = false;
    size_t enc_key_len = ENCRYPTION_KEY_LEN;
    enc_key_available = btif_config_has_section("Adapter");
    if (enc_key_available) {
        enc_key_available = btif_config_exist("Adapter", "BA_ENC_KEY");
    }
    BTIF_TRACE_DEBUG(" %s key_avail = %d ovewrite = %d ", __FUNCTION__,
                                                enc_key_available,overwrite);
    BTIF_TRACE_DEBUG(" %s time = %d ", __FUNCTION__,time(NULL));
    if (!overwrite && enc_key_available) {
        // overwrite is false and key is available.just copy value locally
        ret = btif_config_get_bin("Adapter", "BA_ENC_KEY",
                      (uint8_t*)&btif_ba_cb.encryption_key[0], &enc_key_len);
        BTIF_TRACE_DEBUG(" reading from config returns %d ", ret);
        for (i = 0; i < ENCRYPTION_KEY_LEN; i++) {
            BTIF_TRACE_DEBUG(" ENC key[%d] = %d ", i,
                                    btif_ba_cb.encryption_key[i]);
        }
        return;
    }
    // we have to overwrite value. get the randomized value and put in config
    srand(time(NULL));
    for (i = 0; i < ENCRYPTION_KEY_LEN; i++) {
        btif_ba_cb.encryption_key[i] = (uint8_t)(rand() % 256);
    }
    for (i = 0; i < ENCRYPTION_KEY_LEN; i++) {
        BTIF_TRACE_DEBUG(" ENC key[%d] = %d ", i, btif_ba_cb.encryption_key[i]);
    }
    ret = btif_config_set_bin("Adapter", "BA_ENC_KEY",
                            &btif_ba_cb.encryption_key[0], enc_key_len);
    BTIF_TRACE_DEBUG(" writing to config returns %d ", ret);
}

void refresh_div(bool enable_ba)
{
    uint8_t i = 0;
    BTIF_TRACE_DEBUG(" %s enable_ba = %d time = %d ", __FUNCTION__,enable_ba,
                                                                 time(NULL));
    if(!enable_ba)
    {
        // BA is disabled, make div as zero.
        for (i=0; i < DIV_KEY_LEN; i++)
        {
            btif_ba_cb.div[i] = 0;
        }
        return;
    }
    srand(time(NULL));
    for (i = 0; i < DIV_KEY_LEN; i++) {
        btif_ba_cb.div[i] = (uint8_t)(rand() % 256);
    }
    for (i = 0; i < DIV_KEY_LEN; i++) {
        BTIF_TRACE_DEBUG(" DIV key[%d] = %d ", i, btif_ba_cb.div[i]);
    }
}

void refresh_stream_id(bool ba_streaming)
{
    BTIF_TRACE_DEBUG(" %s ba_streaming  = %d ", __FUNCTION__, ba_streaming);
    if (!ba_streaming)
    {
        // as per new spec, we have to send same stream ID always now.
        btif_ba_cb.active_stream_id = STREAM_ID_48;
    }
    else
    {
        btif_ba_cb.active_stream_id = STREAM_ID_48;
    }
    HAL_CBACK(ba_transmitter_callback, stream_id_update_cb,
          btif_ba_cb.active_stream_id);
}

void dump_curr_codec_config() {
    BTIF_TRACE_DEBUG(" codec_type = %x = %d", btif_ba_cb.codec_type, btif_ba_cb.codec_type);
    BTIF_TRACE_DEBUG(" bit_rate = %x = %d", btif_ba_cb.bit_rate, btif_ba_cb.bit_rate);
    BTIF_TRACE_DEBUG(" channel mode = %x = %d", btif_ba_cb.ch_mode, btif_ba_cb.ch_mode);
    BTIF_TRACE_DEBUG(" complexity = %x = %d", btif_ba_cb.complexity, btif_ba_cb.complexity);
    BTIF_TRACE_DEBUG(" fr_size = %x = %d", btif_ba_cb.frame_size, btif_ba_cb.frame_size);
    BTIF_TRACE_DEBUG(" prediction_mode = %x = %d", btif_ba_cb.prediction_mode, btif_ba_cb.prediction_mode);
    BTIF_TRACE_DEBUG(" sampl_freq = %x = %d", btif_ba_cb.sampl_freq, btif_ba_cb.sampl_freq);
    BTIF_TRACE_DEBUG(" vbr_mode = %x = %d", btif_ba_cb.vbr_mode, btif_ba_cb.vbr_mode);
}

static bool btif_ba_state_idle_audio_pending_handler(btif_sm_event_t event,
        void* p_data, int index) {
    BTIF_TRACE_EVENT("%s event = %s ", __FUNCTION__,
        dump_ba_sm_event_name((btif_ba_sm_event_t)event));
    switch(event)
    {
        case BTIF_SM_ENTER_EVT:
            // no need for fetching memmorized messages, we can enter in this
            // only from BA_I_A_NS. We don't remember any message there.
            break;
        case BTIF_BA_API_INIT_REQ_EVT:
            BTIF_TRACE_ERROR(" %s Should not receive it here ", __FUNCTION__);
            break;
        case BTIF_BA_API_DEINIT_REQ_EVT:
            BTA_BADeregister();
            BTA_BADisable();
            break;
        case BTIF_BA_API_SET_STATE_START_REQ_EVT:
            memorize_msg(event, BTIF_BA_STATE_IDLE_AUDIO_PENDING);
            break;
        case BTIF_BA_BT_A2DP_STARTED_EVT:
            btif_sm_change_state(btif_ba_cb.sm_handle,
                                        BTIF_BA_STATE_IDLE_AUDIO_STREAMING);
            break;
        case BTIF_BA_BT_A2DP_DISC_EVT:
        case BTIF_BA_BT_A2DP_PAUSED_EVT:
            if( !btif_av_is_playing()) {
                btif_sm_change_state(btif_ba_cb.sm_handle,
                                               BTIF_BA_STATE_IDLE_AUDIO_NS);
            }
            break;
        case BTIF_BA_API_REFRESH_ENC_KEY_REQ_EVT:
            // as BA is not enabled, just change enc key here
            // new value would anyways be sent when we enable BA.
            refresh_encryption_key(true);
            break;
        case BTIF_BA_API_SET_VOL_LEVEL:
             // just cache vol level here.
             btif_ba_cb.curr_vol_level = *((uint8_t*)p_data);
             break;
        case BTIF_BA_AUDIO_START_REQ_EVT:
            ba_acknowledge_audio_cmd(BTIF_BA_AUDIO_START_REQ_EVT,
                                                    BA_CTRL_ACK_FAILURE);
            FALLTHROUGH;
        case BTIF_BA_AUDIO_PAUSE_REQ_EVT:
            BTIF_TRACE_DEBUG(" %s already in paused state ", __FUNCTION__);
            ba_acknowledge_audio_cmd(BTIF_BA_AUDIO_PAUSE_REQ_EVT,
                                                    BA_CTRL_ACK_SUCCESS);
            FALLTHROUGH;
        case BTIF_SM_EXIT_EVT:
             btif_ba_cb.prev_state = BTIF_BA_STATE_IDLE_AUDIO_PENDING;
            break;
        default:
            BTIF_TRACE_DEBUG(" %s  UNHANDLED event ", __FUNCTION__);
            break;
    }
    return true;
}

static bool btif_ba_state_idle_audio_streaming_handler(btif_sm_event_t event,
        void* p_data, int index) {
    BTIF_TRACE_EVENT("%s event = %s ", __FUNCTION__,
        dump_ba_sm_event_name((btif_ba_sm_event_t)event));
    switch(event)
    {
        case BTIF_SM_ENTER_EVT:
            handle_memorized_msgs();
            break;
        case BTIF_BA_API_SET_STATE_START_REQ_EVT:
            memorize_msg(event, BTIF_BA_STATE_IDLE_AUDIO_STREAMING);
            btif_av_trigger_suspend();
            break;
        case BTIF_BA_API_DEINIT_REQ_EVT:
            BTA_BADeregister();
            BTA_BADisable();
            break;
        case BTIF_BA_BT_A2DP_DISC_EVT:
        case BTIF_BA_BT_A2DP_PAUSED_EVT:
            if( !btif_av_is_playing()) {
                btif_sm_change_state(btif_ba_cb.sm_handle,
                                           BTIF_BA_STATE_IDLE_AUDIO_NS);
            }
            break;
        case BTIF_BA_API_SET_VOL_LEVEL:
             // just cache vol level here.
             btif_ba_cb.curr_vol_level = *((uint8_t*)p_data);
             break;
        case BTIF_BA_API_REFRESH_ENC_KEY_REQ_EVT:
            // as BA is not enabled, just change enc key here
            // new value would anyways be sent when we enable BA.
            refresh_encryption_key(true);
            break;
        case BTIF_BA_AUDIO_START_REQ_EVT:
            ba_acknowledge_audio_cmd(BTIF_BA_AUDIO_START_REQ_EVT,
                                                    BA_CTRL_ACK_FAILURE);
            FALLTHROUGH;
        case BTIF_BA_AUDIO_PAUSE_REQ_EVT:
            BTIF_TRACE_DEBUG(" %s already in paused state ", __FUNCTION__);
            ba_acknowledge_audio_cmd(BTIF_BA_AUDIO_PAUSE_REQ_EVT,
                                                    BA_CTRL_ACK_SUCCESS);
            FALLTHROUGH;
        case BTIF_SM_EXIT_EVT:
            btif_ba_cb.prev_state = BTIF_BA_STATE_IDLE_AUDIO_STREAMING;
            break;
    }
    return true;
}

// intilized to this state.
static bool btif_ba_state_idle_audio_ns_handler(btif_sm_event_t event,
        void* p_data, int index) {
    BTIF_TRACE_EVENT("%s event = %s ", __FUNCTION__,
        dump_ba_sm_event_name((btif_ba_sm_event_t)event));
    uint8_t enable_sync_train = 0;
    switch(event)
    {
        case BTIF_SM_ENTER_EVT:
             btif_ba_cb.active_stream_id = STREAM_ID_PAUSED;
             btif_ba_cb.bit_rate = A2D_CELT_BIT_RATE_48_2_5;
             btif_ba_cb.ch_mode = A2D_CELT_CH_STEREO;
             btif_ba_cb.codec_type = CODEC_TYPE_CELT;
             btif_ba_cb.complexity = A2D_CELT_COMPLEXITY_BA;
             btif_ba_cb.frame_size = A2D_CELT_FRAME_SIZE_512;
             btif_ba_cb.prediction_mode = 0;
             btif_ba_cb.sampl_freq = A2D_CELT_SAMP_FREQ_48;
             btif_ba_cb.vbr_mode = 0;
             HAL_CBACK(ba_transmitter_callback, state_cb, BA_STATE_IDLE);
             handle_memorized_msgs();
             dump_curr_codec_config();
            break;
        case BTIF_BA_API_INIT_REQ_EVT:
            BTA_BAEnable();
            BTA_BARegister();
            refresh_encryption_key(false);
            break;
        case BTIF_BA_API_DEINIT_REQ_EVT:
            BTA_BADeregister();
            BTA_BADisable();
            break;
        case BTIF_BA_API_SET_VOL_LEVEL:
             // just cache vol level here. BA is not enabled even here
             btif_ba_cb.curr_vol_level = *((uint8_t*)p_data);
             break;
        case BTIF_BA_API_SET_STATE_START_REQ_EVT:
            refresh_div(true);
            enable_sync_train = 1;
            HAL_CBACK(ba_transmitter_callback, state_cb, BA_STATE_PENDING);
            btif_sm_change_state(btif_ba_cb.sm_handle,
                                              BTIF_BA_STATE_PENDING_AUDIO_NS);
            btif_sm_dispatch(btif_ba_cb.sm_handle, BTIF_BA_CMD_PAUSE_REQ_EVT,
                                                     (char*)&enable_sync_train);
            break;
        case BTIF_BA_BT_A2DP_STARTING_EVT:
            btif_sm_change_state(btif_ba_cb.sm_handle,
                                          BTIF_BA_STATE_IDLE_AUDIO_PENDING);
            break;
        case BTIF_BA_BT_A2DP_STARTED_EVT:
            btif_sm_change_state(btif_ba_cb.sm_handle,
                                        BTIF_BA_STATE_IDLE_AUDIO_STREAMING);
            break;
        case BTIF_BA_API_REFRESH_ENC_KEY_REQ_EVT:
            // as BA is not enabled, just change enc key here
            // new value would anyways be sent when we enable BA.
            refresh_encryption_key(true);
            break;
        case BTIF_BA_AUDIO_START_REQ_EVT:
            ba_acknowledge_audio_cmd(BTIF_BA_AUDIO_START_REQ_EVT,
                                                    BA_CTRL_ACK_FAILURE);
            FALLTHROUGH;
        case BTIF_BA_AUDIO_PAUSE_REQ_EVT:
            BTIF_TRACE_DEBUG(" %s already in paused state ", __FUNCTION__);
            ba_acknowledge_audio_cmd(BTIF_BA_AUDIO_PAUSE_REQ_EVT,
                                                    BA_CTRL_ACK_SUCCESS);
            FALLTHROUGH;
        case BTIF_SM_EXIT_EVT:
            btif_ba_cb.prev_state = BTIF_BA_STATE_IDLE_AUDIO_NS;
            break;
    }
    return true;
}

static bool btif_ba_state_pending_audio_ns_handler(btif_sm_event_t event,
        void* p_data, int index) {
    BTIF_TRACE_EVENT("%s event = %s ", __FUNCTION__,
        dump_ba_sm_event_name((btif_ba_sm_event_t)event));
    switch(event)
    {
        case BTIF_SM_ENTER_EVT:
            /*
             * NO HAL_CBACK here, because there can be 2 entry points for
             * Pending state. Audio_pending and Enable_pending.Better send
             * from place where its triggered from
             */
            break;
        case BTIF_BA_API_SET_STATE_START_REQ_EVT:
        case BTIF_BA_API_SET_STATE_STOP_REQ_EVT:
        case BTIF_BA_AUDIO_START_REQ_EVT:
        case BTIF_BA_AUDIO_PAUSE_REQ_EVT:
        case BTIF_BA_CSB_TIMEOUT_EVT:
            memorize_msg(event, BTIF_BA_STATE_PENDING_AUDIO_NS);
            break;
        case BTIF_BA_API_DEINIT_REQ_EVT:
            BTA_BADeregister();
            BTA_BADisable();
            break;
        case BTIF_BA_BT_A2DP_STARTED_EVT:
        case BTIF_BA_BT_A2DP_STARTING_EVT:
            BTIF_TRACE_DEBUG("  %s This shld not happen ", __FUNCTION__);
            break;
            // we might be waiting for some commands from BTA. so memorize
            // this command.
        case BTIF_BA_API_SET_VOL_LEVEL:
             btif_ba_cb.curr_vol_level = *((uint8_t*)p_data);
             BTIF_TRACE_DEBUG("%s: curr_vol_level: %d",
                              __FUNCTION__, btif_ba_cb.curr_vol_level);
             memorize_msg(event, BTIF_BA_STATE_PENDING_AUDIO_NS);
             break;
        case BTIF_BA_CMD_PAUSE_REQ_EVT:
            if (*((uint8_t*)p_data))
              BTA_PauseBA(true, &btif_ba_cb.encryption_key[0],
                                &btif_ba_cb.div[0], btif_ba_cb.curr_vol_level);
            else
              BTA_PauseBA(false, &btif_ba_cb.encryption_key[0],
                               &btif_ba_cb.div[0], btif_ba_cb.curr_vol_level);
            break;
        case BTIF_BA_RSP_PAUSE_DONE_EVT:

            if (*(uint8_t*)p_data != BT_STATUS_SUCCESS) {
                switch(btif_ba_cb.prev_state) {
                  // this was trigerred as part of API_START, and failure happened
                  case BTIF_BA_STATE_IDLE_AUDIO_NS:
                    btif_sm_dispatch(btif_ba_cb.sm_handle,
                       BTIF_BA_CMD_STOP_REQ_EVT, NULL);
                    break;
                  // pause was sent on request from audio, we met failure
                  case BTIF_BA_STATE_STREAMING_AUDIO_NS:
                    ba_acknowledge_audio_cmd(BTIF_BA_AUDIO_PAUSE_REQ_EVT,
                                                     BA_CTRL_ACK_FAILURE);
                    btif_sm_dispatch(btif_ba_cb.sm_handle,
                       BTIF_BA_CMD_STOP_REQ_EVT, NULL);
                    break;
                }
            }
            else {
                switch(btif_ba_cb.prev_state) {
                  // this was trigerred as part of API_START, and failure happened
                  case BTIF_BA_STATE_IDLE_AUDIO_NS:
                    initialize_audio_hidl();
                    HAL_CBACK(ba_transmitter_callback, enc_update_key_cb,
                         ENCRYPTION_KEY_LEN, &(btif_ba_cb.encryption_key[0]));
                    HAL_CBACK(ba_transmitter_callback, div_update_cb, DIV_KEY_LEN,
                                                       &(btif_ba_cb.div[0]));
                    btif_sm_change_state(btif_ba_cb.sm_handle,
                                           BTIF_BA_STATE_PAUSED_AUDIO_NS);
                    break;
                  // pause was sent on request from audio, we met failure
                  case BTIF_BA_STATE_STREAMING_AUDIO_NS:
                    btif_sm_change_state(btif_ba_cb.sm_handle,
                                           BTIF_BA_STATE_PAUSED_AUDIO_NS);
                    ba_acknowledge_audio_cmd(BTIF_BA_AUDIO_PAUSE_REQ_EVT,
                                                        BA_CTRL_ACK_SUCCESS);
                    break;
                }
            }
            break;
        case BTIF_BA_CMD_STREAM_REQ_EVT:
            BTA_StreamBA();
            break;
        case BTIF_BA_RSP_STREAM_DONE_EVT:
            btif_sm_change_state(btif_ba_cb.sm_handle,
                                     BTIF_BA_STATE_STREAMING_AUDIO_NS);
            ba_acknowledge_audio_cmd(BTIF_BA_AUDIO_START_REQ_EVT,
                                                    BA_CTRL_ACK_SUCCESS);
            break;
        case BTIF_BA_CMD_STOP_REQ_EVT:
            refresh_div(false);
            BTA_StopBA();
            break;
        case BTIF_BA_RSP_STOP_DONE_EVT:
            deinit_audio_hal();
            btif_sm_change_state(btif_ba_cb.sm_handle,
                                        BTIF_BA_STATE_IDLE_AUDIO_NS);
            HAL_CBACK(ba_transmitter_callback, div_update_cb, DIV_KEY_LEN,
                                                      &(btif_ba_cb.div[0]));
            break;
        case BTIF_BA_CMD_SEND_VOL_UPDATE:
            BTA_SetVol(btif_ba_cb.curr_vol_level);
            break;
        case BTIF_BA_RSP_VOL_UPDATE_DONE_EVT:
            BTIF_TRACE_EVENT(" %s moving to %d state ",__FUNCTION__,
                                        btif_ba_cb.prev_state);
            btif_sm_change_state(btif_ba_cb.sm_handle,
                                   btif_ba_cb.prev_state);
            break;
        case BTIF_BA_CMD_UPDATE_ENC_KEY:
            BTA_SetEncKey(&btif_ba_cb.encryption_key[0]);
            break;
        case BTIF_BA_RSP_ENC_KEY_UPDATE_DONE_EVT:
            BTIF_TRACE_EVENT(" %s moving to %d state ",__FUNCTION__,
                                      btif_ba_cb.prev_state);
            btif_sm_change_state(btif_ba_cb.sm_handle,
                                 btif_ba_cb.prev_state);
            HAL_CBACK(ba_transmitter_callback, enc_update_key_cb,
                ENCRYPTION_KEY_LEN, &(btif_ba_cb.encryption_key[0]));
            break;
        case BTIF_BA_API_REFRESH_ENC_KEY_REQ_EVT:
            // memorize here, as we are in transition because of some other
            // request
            refresh_encryption_key(true);
            memorize_msg(event, BTIF_BA_STATE_PENDING_AUDIO_NS);
            break;
        case BTIF_SM_EXIT_EVT:
            btif_ba_cb.prev_state = BTIF_BA_STATE_PENDING_AUDIO_NS;
            break;
        default:
            BTIF_TRACE_DEBUG(" %s  UNHANDLED event ", __FUNCTION__);
            break;
    }
    return true;
}

static bool btif_ba_state_paused_audio_ns_handler(btif_sm_event_t event,
        void* p_data, int index) {
    BTIF_TRACE_EVENT("%s event = %s ", __FUNCTION__,
        dump_ba_sm_event_name((btif_ba_sm_event_t)event));
    switch(event)
    {
        case BTIF_SM_ENTER_EVT:
            HAL_CBACK(ba_transmitter_callback, state_cb, BA_STATE_PAUSED);
            handle_memorized_msgs();
            break;
        case BTIF_BA_CSB_TIMEOUT_EVT:
        case BTIF_BA_API_SET_STATE_STOP_REQ_EVT:
            HAL_CBACK(ba_transmitter_callback, state_cb, BA_STATE_PENDING);
            btif_sm_change_state(btif_ba_cb.sm_handle,
                                              BTIF_BA_STATE_PENDING_AUDIO_NS);
            btif_sm_dispatch(btif_ba_cb.sm_handle, BTIF_BA_CMD_STOP_REQ_EVT,
                                                                      NULL);
            break;
        case BTIF_BA_AUDIO_START_REQ_EVT:
            HAL_CBACK(ba_transmitter_callback, state_cb, BA_STATE_AUDIO_PENDING);
            btif_sm_change_state(btif_ba_cb.sm_handle,
                                            BTIF_BA_STATE_PENDING_AUDIO_NS);
            btif_sm_dispatch(btif_ba_cb.sm_handle, BTIF_BA_CMD_STREAM_REQ_EVT,
                                                                    NULL);
            break;
        case BTIF_BA_API_SET_STATE_START_REQ_EVT:
            BTIF_TRACE_DEBUG(" %s already in paused state ", __FUNCTION__);
            break;
        case BTIF_BA_AUDIO_PAUSE_REQ_EVT:
            BTIF_TRACE_DEBUG(" %s already in paused state ", __FUNCTION__);
            ba_acknowledge_audio_cmd(BTIF_BA_AUDIO_PAUSE_REQ_EVT,
                                                    BA_CTRL_ACK_SUCCESS);
            break;
        case BTIF_BA_BT_A2DP_STARTED_EVT:
        case BTIF_BA_BT_A2DP_STARTING_EVT:
            BTIF_TRACE_DEBUG("  %s This shld not happen ", __FUNCTION__);
            break;
        case BTIF_BA_API_DEINIT_REQ_EVT:
            BTA_BADeregister();
            BTA_BADisable();
            break;
        case BTIF_BA_API_REFRESH_ENC_KEY_REQ_EVT:
            refresh_encryption_key(true);
            btif_sm_change_state(btif_ba_cb.sm_handle,
                                          BTIF_BA_STATE_PENDING_AUDIO_NS);
            btif_sm_dispatch(btif_ba_cb.sm_handle, BTIF_BA_CMD_UPDATE_ENC_KEY,
                                                 NULL);
            break;
        case BTIF_BA_API_SET_VOL_LEVEL:
            btif_ba_cb.curr_vol_level = *((uint8_t*)p_data);
            btif_sm_change_state(btif_ba_cb.sm_handle,
                                          BTIF_BA_STATE_PENDING_AUDIO_NS);
            btif_sm_dispatch(btif_ba_cb.sm_handle, BTIF_BA_CMD_SEND_VOL_UPDATE,
                                                 NULL);
            break;
        case BTIF_SM_EXIT_EVT:
            btif_ba_cb.prev_state = BTIF_BA_STATE_PAUSED_AUDIO_NS;
            break;
        default:
            BTIF_TRACE_DEBUG(" %s  UNHANDLED event = %d ", __FUNCTION__, event);
            break;
    }
    return true;
}

static bool btif_ba_state_streaming_audio_ns_handler(btif_sm_event_t event,
        void* p_data, int index) {
    BTIF_TRACE_EVENT("%s event = %s ", __FUNCTION__,
        dump_ba_sm_event_name((btif_ba_sm_event_t)event));
    uint8_t enable_sync_train = 0;
    switch(event)
    {
      case BTIF_SM_ENTER_EVT:
          HAL_CBACK(ba_transmitter_callback, state_cb, BA_STATE_STREAMING);
          refresh_stream_id(true);
          handle_memorized_msgs();
          break;
      case BTIF_BA_CSB_TIMEOUT_EVT:
      case BTIF_BA_API_SET_STATE_STOP_REQ_EVT:
          HAL_CBACK(ba_transmitter_callback, state_cb, BA_STATE_PENDING);
          btif_sm_change_state(btif_ba_cb.sm_handle,
                                            BTIF_BA_STATE_PENDING_AUDIO_NS);
          btif_sm_dispatch(btif_ba_cb.sm_handle, BTIF_BA_CMD_STOP_REQ_EVT,
                                                                    NULL);
          break;
      case BTIF_BA_AUDIO_PAUSE_REQ_EVT:
          enable_sync_train = 0;
          HAL_CBACK(ba_transmitter_callback, state_cb, BA_STATE_AUDIO_PENDING);
          btif_sm_change_state(btif_ba_cb.sm_handle,
                                          BTIF_BA_STATE_PENDING_AUDIO_NS);
          btif_sm_dispatch(btif_ba_cb.sm_handle, BTIF_BA_CMD_PAUSE_REQ_EVT,
                                                    (char*)&enable_sync_train);
          break;
      case BTIF_BA_API_SET_STATE_START_REQ_EVT:
          BTIF_TRACE_DEBUG(" %s already in streaming state ", __FUNCTION__);
          break;
      case BTIF_BA_AUDIO_START_REQ_EVT:
          BTIF_TRACE_DEBUG(" %s already in streaming state ", __FUNCTION__);
          ba_acknowledge_audio_cmd(BTIF_BA_AUDIO_START_REQ_EVT,
                                                  BA_CTRL_ACK_SUCCESS);
          break;
      case BTIF_BA_BT_A2DP_STARTED_EVT:
      case BTIF_BA_BT_A2DP_STARTING_EVT:
          BTIF_TRACE_DEBUG("  %s This shld not happen ", __FUNCTION__);
          break;
      case BTIF_BA_API_DEINIT_REQ_EVT:
          BTA_BADeregister();
          BTA_BADisable();
          break;
      case BTIF_BA_API_REFRESH_ENC_KEY_REQ_EVT:
          refresh_encryption_key(true);
          btif_sm_change_state(btif_ba_cb.sm_handle,
                                        BTIF_BA_STATE_PENDING_AUDIO_NS);
          btif_sm_dispatch(btif_ba_cb.sm_handle, BTIF_BA_CMD_UPDATE_ENC_KEY,
                                               NULL);
          break;
      case BTIF_BA_API_SET_VOL_LEVEL:
           if (p_data != NULL) {
             btif_ba_cb.curr_vol_level = *((uint8_t*)p_data);
           } else {
             BTIF_TRACE_DEBUG("%s: p_data is null, curr_vol_level: %d",
                              __FUNCTION__, btif_ba_cb.curr_vol_level);
           }
           btif_sm_change_state(btif_ba_cb.sm_handle,
                                           BTIF_BA_STATE_PENDING_AUDIO_NS);
           btif_sm_dispatch(btif_ba_cb.sm_handle, BTIF_BA_CMD_SEND_VOL_UPDATE,
                                                  NULL);
           break;
      case BTIF_SM_EXIT_EVT:
           refresh_stream_id(false);
           btif_ba_cb.prev_state = BTIF_BA_STATE_STREAMING_AUDIO_NS;
           break;
      default:
          BTIF_TRACE_DEBUG(" %s  UNHANDLED event ", __FUNCTION__);
          break;
    }
    return true;
}
