/******************************************************************************
 * Copyright (C) 2017, The Linux Foundation. All rights reserved.
 * Not a Contribution.
 ******************************************************************************/
/******************************************************************************
 *
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

/*******************************************************************************
 *
 *  Filename:      btif_av.h
 *
 *  Description:   Main API header file for all BTIF AV functions accessed
 *                 from internal stack.
 *
 ******************************************************************************/

#ifndef BTIF_AV_H
#define BTIF_AV_H

#include "bta_av_api.h"
#include "btif_common.h"
#include "btif_sm.h"

#define SOFT_HANDOFF 1
#define RECONFIG_A2DP_PARAM 2

#define APTX_HQ 0X1000
#define APTX_LL 0X2000
#define APTX_ULL_S 0X4000
#define APTX_ULL 0X6000
#define APTX_MODE_MASK 0X7000
#define APTX_SCAN_CONTROL_MASK 0X8000
#define APTX_BATTERY_INFO 0X0F

#define APTX_HQ_LATENCY 2000
#define APTX_LL_LATENCY 700
#define APTX_ULL_LATENCY 700

/*******************************************************************************
 *  Type definitions for callback functions
 ******************************************************************************/

typedef enum {
  /* Reuse BTA_AV_XXX_EVT - No need to redefine them here */
  BTIF_AV_CONNECT_REQ_EVT = BTA_AV_MAX_EVT,
  BTIF_AV_DISCONNECT_REQ_EVT,
  BTIF_AV_START_STREAM_REQ_EVT,
  BTIF_AV_STOP_STREAM_REQ_EVT,
  BTIF_AV_SUSPEND_STREAM_REQ_EVT,
  BTIF_AV_SOURCE_CONFIG_REQ_EVT,
  BTIF_AV_SOURCE_CONFIG_UPDATED_EVT,
  BTIF_AV_SINK_CONFIG_REQ_EVT,
  BTIF_AV_OFFLOAD_START_REQ_EVT,
  BTIF_AV_CLEANUP_REQ_EVT,
  BTIF_AV_REMOTE_SUSPEND_STREAM_REQ_EVT,
  BTIF_AV_RESET_REMOTE_STARTED_FLAG_EVT,
  BTIF_AV_RESET_REMOTE_STARTED_FLAG_UPDATE_AUDIO_STATE_EVT,
  BTIF_AV_INIT_REQ_EVT,
  BTIF_AV_REINIT_AUDIO_IF,
  BTIF_AV_SETUP_CODEC_REQ_EVT,
  BTIF_AV_TRIGGER_HANDOFF_REQ_EVT,
  BTIF_AV_SET_SILENT_REQ_EVT,
  BTIF_AV_ENCODER_MODE_CHANGED_EVT,
  BTIF_AV_SINK_QUICK_HANDOFF_EVT,
  BTIF_AV_PROCESS_HIDL_REQ_EVT,
  BTIF_AV_CHECK_PENDING_PLAY_EVT,
  BTIF_AV_REPORT_AUDIO_STATE_EVT,
} btif_av_sm_event_t;

/*******************************************************************************
 *  BTIF AV API
 ******************************************************************************/

bool btif_av_is_handoff_set();
/*******************************************************************************
 *
 * Function         btif_av_get_addr
 *
 * Description      Fetches current AV BD address
 *
 * Returns          BD address
 *
 ******************************************************************************/

RawAddress btif_av_get_addr(RawAddress address);

/*******************************************************************************
 * Function         btif_av_is_sink_enabled
 *
 * Description      Checks if A2DP Sink is enabled or not
 *
 * Returns          true if A2DP Sink is enabled, false otherwise
 *
 ******************************************************************************/

bool btif_av_is_sink_enabled(void);

/*******************************************************************************
 *
 * Function         btif_av_stream_ready
 *
 * Description      Checks whether AV is ready for starting a stream
 *
 * Returns          None
 *
 ******************************************************************************/

bool btif_av_stream_ready(void);

/*******************************************************************************
 *
 * Function         btif_av_stream_started_ready
 *
 * Description      Checks whether AV ready for media start in streaming state
 *
 * Returns          None
 *
 ******************************************************************************/

bool btif_av_stream_started_ready(void);

/*******************************************************************************
**
** Function         btif_av_is_start_ack_pending
**
** Description      Checks whether start ack is pending
**
** Returns          None
**
*******************************************************************************/

bool btif_av_is_start_ack_pending(void);


/*******************************************************************************
 *
 * Function         btif_dispatch_sm_event
 *
 * Description      Send event to AV statemachine
 *
 * Returns          None
 *
 ******************************************************************************/

/* used to pass events to AV statemachine from other tasks */
void btif_dispatch_sm_event(btif_av_sm_event_t event, void *p_data, int len);

/*******************************************************************************
 *
 * Function         btif_av_init
 *
 * Description      Initializes btif AV if not already done
 *
 * Returns          bt_status_t
 *
 ******************************************************************************/

bt_status_t btif_av_init(int service_id);

/*******************************************************************************
 *
 * Function         btif_av_is_connected
 *
 * Description      Checks if av has a connected sink
 *
 * Returns          bool
 *
 ******************************************************************************/

bool btif_av_is_connected(void);

/*******************************************************************************
 *
 * Function         btif_av_get_peer_sep
 *
 * Description      Get the stream endpoint type.
 *
 * Returns          The stream endpoint type: either AVDT_TSEP_SRC or
 *                  AVDT_TSEP_SNK.
 *
 ******************************************************************************/

uint8_t btif_av_get_peer_sep();

/*******************************************************************************
 *
 * Function         btif_av_is_peer_edr
 *
 * Description      Check if the connected a2dp device supports
 *                  EDR or not. Only when connected this function
 *                  will accurately provide a true capability of
 *                  remote peer. If not connected it will always be false.
 *
 * Returns          true if remote device is capable of EDR
 *
 ******************************************************************************/

bool btif_av_is_peer_edr(void);

/******************************************************************************
 *
 * Function         btif_av_clear_remote_suspend_flag
 *
 * Description      Clears remote suspended flag
 *
 * Returns          Void
 ******************************************************************************/
void btif_av_clear_remote_suspend_flag(void);

/*******************************************************************************
 *
 * Function         btif_av_peer_supports_3mbps
 *
 * Description      Check if the connected A2DP device supports
 *                  3 Mbps EDR. This function will only work while connected.
 *                  If not connected it will always return false.
 *
 * Returns          true if remote device is EDR and supports 3 Mbps
 *
 ******************************************************************************/
bool btif_av_peer_supports_3mbps(void);

/*******************************************************************************
**
** Function         btif_av_check_flag_remote_suspend
**
** Description      Check whether remote suspend flag is set or not
**
** Returns          TRUE if remote suspen flag set
**
*******************************************************************************/
bool btif_av_check_flag_remote_suspend(int index);

/*******************************************************************************
 *
 * Function         btif_av_is_split_a2dp_enabled
 *
 * Description      Check if split a2dp is enabled.
 *
 * Returns          TRUE if split a2dp is enabled, FALSE otherwise
 *
 ******************************************************************************/
bool btif_av_is_split_a2dp_enabled(void);

/*******************************************************************************
**
** Function         btif_av_any_br_peer
**
** Description      Check if the any of connected devices is BR device.
**
** Returns          TRUE if connected to any BR device, FALSE otherwise.
**
*******************************************************************************/
bool btif_av_any_br_peer(void);

/*******************************************************************************
**
** Function         btif_av_get_multicast_state
**
** Description      Check if A2DP multicast is enabled
**
** Returns          TRUE if a2dp multicast is enabled
**
*******************************************************************************/
bool btif_av_get_multicast_state();

/*******************************************************************************
**
** Function         btif_av_is_multicast_supported
**
** Description      Check if A2DP multicast is supported
**
** Returns          TRUE if a2dp multicast is supported
**
*******************************************************************************/
bool btif_av_is_multicast_supported();

/******************************************************************************
 *
 * Function         btif_av_get_peer_addr
 *
 * Description      Returns active peer device address
 *
 * Returns          peer address
 *******************************************************************************/
void btif_av_get_active_peer_addr(RawAddress *peer_bda);

/*******************************************************************************
 *
 * Function         btif_av_clear_remote_start_timer
 *
 * Description      Clear latest av start timer
 *
 * Returns          bool
 *
 ******************************************************************************/
void  btif_av_clear_remote_start_timer(int index);

/******************************************************************************
 *
 * Function         btif_get_latest_playing_device_idx
 *
 * Description      Get the index of AV where streaming is happening
 *
 * Returns          index
 *******************************************************************************/
int btif_av_get_latest_playing_device_idx();

/******************************************************************************
 *
 * Function         btif_get_latest_playing_device_idx
 *
 * Description      Get the index of AV where streaming is happening but not
                    remote started index
 *
 * Returns          index
 *******************************************************************************/
int btif_av_get_latest_stream_device_idx();

/******************************************************************************
 *
 * Function         btif_av_is_device_connected
 *
 * Description      Checks if the A2DP device is connected
 *
 * Returns          true/false
 *******************************************************************************/
bool btif_av_is_device_connected(RawAddress address);

/******************************************************************************
 *
 * Function         btif_av_is_playing
 *
 * Description      Checks if the A2DP stream is playing
 *
 * Returns          true/false
 *******************************************************************************/
bool btif_av_is_playing();

/*******************************************************************************
 *
 * Function         btif_av_get_latest_playing_device_idx
 *
 * Description      Get the index of AV where streaming is happening
 *
 * Returns          int
 *
 ******************************************************************************/
int btif_av_get_latest_playing_device_idx();

/*******************************************************************************
 *
 * Function         btif_av_trigger_dual_handoff
 *
 * Description      Trigger the DUAL HANDOFF. This function will trigger remote
 *                  suspend for currently playing device and then initiate START
 *                  on Handoff device whose address is passed as an argument.
 *
 * Returns          void
 *
 ******************************************************************************/
void btif_av_trigger_dual_handoff(bool handoff, int current_active_index, int previous_active_index);

/*******************************************************************************
 *
 * Function         btif_get_latest_playing_device
 *
 * Description      Get the index for the most recent source
 *
 * Returns          None
 *
 ******************************************************************************/
void btif_get_latest_playing_device(RawAddress* address);

/*******************************************************************************
 *
 * Function        btif_av_get_num_connected_devices
 *
 * Description     Return number of A2dp connected devices
 *
 * Returns         int
 *****************************************************************************/
uint16_t btif_av_get_num_connected_devices(void);

/******************************************************************************
 *
 * Function        btif_av_get_num_playing_devices
 *
 * Description     Return number of A2dp playing devices
 *
 * Returns         int
 *****************************************************************************/
uint16_t btif_av_get_num_playing_devices(void);

/******************************************************************************
 *
 * Function        btif_av_is_current_device
 *
 * Description     return true if this A2dp device is streaming
 *
 * Returns         true/false
 *****************************************************************************/
bool btif_av_is_current_device(RawAddress address);

/******************************************************************************
 *
 * Function        btif_av_get_latest_device_idx_to_start
 *
 * Description     Return latest device index to which start stream req will be
 *                 processed
 *
 * Returns         int
 *****************************************************************************/
int btif_av_get_latest_device_idx_to_start();

/******************************************************************************
**
** Function         btif_av_is_under_handoff
**
** Description     check if AV state is under handoff
**
** Returns         TRUE if handoff is triggered, FALSE otherwise
********************************************************************************/
bool btif_av_is_under_handoff();

/******************************************************************************
**
** Function        btif_av_get_sink_latency
**
** Description     get initial sink latency
**
** Returns         tBTA_AV_LATENCY
********************************************************************************/
tBTA_AV_LATENCY btif_av_get_sink_latency();

bool btif_av_is_scrambling_enabled();
bool btif_av_is_44p1kFreq_supported();


/******************************************************************************
**
** Function         btif_av_peer_config_dump
**
** Description
**
** Returns
********************************************************************************/
void btif_av_peer_config_dump();

/******************************************************************************
**
** Function         btif_av_get_current_playing_dev_idx()
**
** Description
**
** Returns
********************************************************************************/
int btif_av_get_current_playing_dev_idx();

/**
 * Set the audio delay for the stream.
 *
 * @param delay the delay to set in units of 1/10ms
 */
void btif_av_set_audio_delay(uint16_t delay, tBTA_AV_HNDL hndl);

/**
 * Reset the audio delay and count of audio bytes sent to zero.
 */
void btif_av_reset_audio_delay(tBTA_AV_HNDL hndl);

/**
 * Function         btif_av_get_audio_delay
 *
 * Description      Get the audio delay for the stream.
 *
 * Returns          uint16_t
 */
uint16_t btif_av_get_audio_delay(int index);

/**
 * Function         btif_av_get_aptx_mode_info
 *
 * Description      Return current aptx_mode for active index.
 *
 * Returns          uint16_t
 */
uint16_t btif_av_get_aptx_mode_info();


void initialize_audio_hidl();
void deinit_audio_hal();

RawAddress btif_av_get_addr_by_index(int idx);

/*******************************************************************************
**
** Function         btif_get_average_delay
**
** Description      Returns average of instantaneous delay values
**
** Returns          int64_t
*******************************************************************************/
int64_t btif_get_average_delay();

/*******************************************************************************
**
** Function         btif_is_a2dp_sink_handoff_required
**
** Description      To check if there is need for Soft-Handoff in A2DP Sink.
**
** Returns          bool
*******************************************************************************/
bool btif_is_a2dp_sink_handoff_required(int idx);

/*******************************************************************************
**
** Function         btif_initiate_sink_handoff
**
** Description      Intiates operations required to handle Soft-Handoff
**
** Returns          void
*******************************************************************************/
void btif_initiate_sink_handoff(int idx, bool audio_state_changed);

/*******************************************************************************
**
** Function         btif_get_max_allowable_sink_connections
**
** Description      Get maximum number of supported Sink Connections
**                  Currently, Default:2, Max:2
**                  TODO: Q: Range:{1,5} Deafault:3 Max:5
**
** Returns          void
*******************************************************************************/
int btif_get_max_allowable_sink_connections();

/*******************************************************************************
**
** Function         btif_av_get_hndl_by_addr
**
** Description      Get AV handle for the associated bd_addr
**
** Returns          tBTA_AV_HNDL
*******************************************************************************/
tBTA_AV_HNDL btif_av_get_hndl_by_addr(RawAddress peer_address);

void btif_av_signal_session_ready();
void btif_av_set_suspend_rsp_track_timer(int index);
void btif_av_set_suspend_rsp_track_timer_tout(void* data);
void btif_av_clear_suspend_rsp_track_timer(int index);
#endif /* BTIF_AV_H */
