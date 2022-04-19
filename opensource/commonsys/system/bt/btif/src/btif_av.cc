/******************************************************************************
 *  Copyright (C) 2017, The Linux Foundation. All rights reserved.
 *  Not a Contribution.
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
 *  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 *  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 *  GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 *  IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 *  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 *  IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/
/******************************************************************************
 *  Copyright (C) 2009-2016 Broadcom Corporation
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

#define LOG_TAG "btif_av"

#include "btif_av.h"

#include <base/logging.h>
#include <string.h>

#include <hardware/bluetooth.h>
#include <hardware/bt_av.h>
#include <hardware/bt_rc_ext.h>

#include "audio_a2dp_hw/include/audio_a2dp_hw.h"
#include "bt_common.h"
#include "bt_utils.h"
#include "bta_api.h"
#include "btif_storage.h"
#include "btif_a2dp.h"
#include "btif_hf.h"
#include "btif_a2dp_audio_interface.h"
#include "btif_a2dp_control.h"
#include "btif_a2dp_sink.h"
#include "btif_av_co.h"
#include "btif_profile_queue.h"
#include "btif_util.h"
#include "btu.h"
#include "osi/include/allocator.h"
#include "osi/include/osi.h"
#include "osi/include/properties.h"
#include "btif/include/btif_a2dp_source.h"
#include "device/include/interop.h"
#include "device/include/controller.h"
#include "btif_bat.h"
#include "bta/av/bta_av_int.h"
#include "device/include/device_iot_config.h"
#if (OFF_TARGET_TEST_ENABLED == FALSE)
#include "audio_hal_interface/a2dp_encoding.h"
#endif
#include "controller.h"
#if (OFF_TARGET_TEST_ENABLED == TRUE)
#include "log/log.h"
#include "service/a2dp_hal_sim/audio_a2dp_hal_stub.h"
#endif

extern bool isDevUiReq;
bool isBitRateChange = false;
bool isBitsPerSampleChange = false;
static int reconfig_a2dp_param_id = 0;
static int reconfig_a2dp_param_val = 0;
std::mutex session_wait_mutex_;
std::condition_variable session_wait_cv;
bool session_wait;
RawAddress ba_addr({0xCE, 0xFA, 0xCE, 0xFA, 0xCE, 0xFA});

#define BTIF_AV_ENABLE_MCAST_RESTRICTIONS FALSE
/*****************************************************************************
 *  Constants & Macros
 *****************************************************************************/
#define BTIF_AV_SERVICE_NAME "Advanced Audio"
#define BTIF_AVK_SERVICE_NAME "Advanced Audio Sink"

#define BTIF_TIMEOUT_AV_OPEN_ON_RC_MS (2 * 1000)
#define BTIF_SUSPEND_RSP_FROM_REMOTE_TOUT (2 * 1000)

/* Number of BTIF-AV control blocks */
/* Now supports Two AV connections. */
#define BTIF_AV_NUM_CB       5
#define HANDLE_TO_INDEX(x) ((x & BTA_AV_HNDL_MSK) - 1)
#define INVALID_INDEX        -1

typedef enum {
  BTIF_AV_STATE_IDLE = 0x0,
  BTIF_AV_STATE_OPENING,
  BTIF_AV_STATE_OPENED,
  BTIF_AV_STATE_STARTED,
  BTIF_AV_STATE_CLOSING
} btif_av_state_t;

/* Should not need dedicated suspend state as actual actions are no
   different than open state. Suspend flags are needed however to prevent
   media task from trying to restart stream during remote suspend or while
   we are in the process of a local suspend */

#define BTIF_AV_FLAG_LOCAL_SUSPEND_PENDING 0x1
#define BTIF_AV_FLAG_REMOTE_SUSPEND 0x2
#define BTIF_AV_FLAG_PENDING_START 0x4
#define BTIF_AV_FLAG_PENDING_STOP 0x8
#define BTIF_AV_FLAG_PENDING_DISCONNECT 0x10
#define BTIF_TIMEOUT_AV_COLL_DETECTED_MS (2 * 1000)
#define BTIF_TIMEOUT_AV_COLL_DETECTED_MS_2 (4 * 1000)
#define BTIF_ERROR_SRV_AV_CP_NOT_SUPPORTED   705

/* Host role definitions */
#define HOST_ROLE_MASTER                   0x00
#define HOST_ROLE_SLAVE                    0x01
#define HOST_ROLE_UNKNOWN                  0xff

/* Default sink latency value while delay report is not supported by SNK */
#define BTIF_AV_DEFAULT_SINK_LATENCY 0
#define BTIF_AV_DEFAULT_MULTICAST_SINK_LATENCY 200

#define DELAY_RECORD_COUNT 100 /* No of packets considered for delay reporting calculation */
#define MISC_RENDERING_DELAY 50 /* miscellaneous rendering delay from SINK API's*/
#define QUICK_SINK_SHO_IND_PKT_CNT 165 /* when previous streaming device starts playing
                                          within 3 sec*/
#define MAX_ALLOWED_SINK_CONNECTIONS 2 /* Max Sink connections are restricted to 2 */

/* Param id for bitrate and bits per sample */
#define BITRATE_PARAM_ID 1
#define BITSPERSAMPLE_PARAM_ID 2

/*****************************************************************************
 *  Local type definitions
 *****************************************************************************/

typedef struct {
  tBTA_AV_HNDL bta_handle;
  RawAddress peer_bda;
  bool self_initiated_connection;
  btif_sm_handle_t sm_handle;
  uint8_t flags;
  tBTA_AV_EDR edr;
  uint8_t peer_sep; /* sep type of peer device */
  std::vector<btav_a2dp_codec_config_t> codec_priorities;
  bool edr_3mbps;
  bool dual_handoff;
  bool current_playing;
  bool is_silenced;
  btif_sm_state_t state;
  int service;
  bool is_slave;
  bool is_device_playing;
  bool reconfig_pending; /* flag used for codec switch */
  tBTA_AV_LATENCY sink_latency;
  bool remote_started;
  bool is_suspend_for_remote_start;
  bool retry_rc_connect;
#if (TWS_ENABLED == TRUE)
  bool tws_device;
  bool offload_state;
#if (TWS_STATE_ENABLED == TRUE)
  uint8_t eb_state;
  bool tws_state_suspend;
#endif
#endif
  bool avdt_sync; /* for AVDT1.3 delay reporting */
  uint16_t codec_latency;
  uint16_t aptx_mode;
  uint16_t remote_delay;
  struct alarm_t *remote_start_alarm;
  btif_sm_event_t reconfig_event;
  tBTA_AV reconfig_data;
  struct alarm_t *suspend_rsp_track_timer;
  bool fake_suspend_rsp;
} btif_av_cb_t;

typedef struct {
  RawAddress* target_bda;
  uint16_t uuid;
} btif_av_connect_req_t;

typedef struct {
  RawAddress bd_addr;
  bool is_silent;
} btif_av_silent_req_t;

typedef struct {
  int sample_rate;
  int channel_count;
  RawAddress peer_bd;
} btif_av_sink_config_req_t;

typedef struct {
  RawAddress bd_addr;
  btav_a2dp_codec_config_t codec_config;
} btif_av_codec_config_req_t;

typedef struct {
  RawAddress bd_addr;
  uint8_t conn_retry_count;
  alarm_t *av_coll_detected_timer;
} btif_av_collision_detect_t;

/*****************************************************************************
 *  Static variables
 *****************************************************************************/
static btav_source_callbacks_t* bt_av_src_callbacks = NULL;
static btav_sink_callbacks_t* bt_av_sink_callbacks = NULL;
static btif_av_cb_t btif_av_cb[BTIF_AV_NUM_CB];
btif_av_collision_detect_t collision_detect[BTIF_AV_NUM_CB];

static alarm_t* av_open_on_rc_timer = NULL;
static btif_sm_event_t idle_rc_event;
static tBTA_AV idle_rc_data;
int btif_max_av_clients = 1;
static bool enable_multicast = false;
static bool is_multicast_supported = false;
static bool multicast_disabled = false;
static RawAddress codec_bda = {};
static bool isPeerA2dpSink = false;
static bool codec_config_update_enabled = false;
bool is_codec_config_dump = false;
static std::vector<btav_a2dp_codec_config_t> offload_enabled_codecs_config_;

/*SPLITA2DP */
bool bt_split_a2dp_enabled = false;
bool reconfig_a2dp = false;
bool btif_a2dp_audio_if_init = false;
bool codec_cfg_change = false;
bool audio_start_awaited = false;
extern bool enc_update_in_progress;
extern bool is_block_hal_start;
#if (TWS_ENABLED == TRUE)
bool tws_defaultmono_supported = false;
#endif
#if (TWS_STATE_ENABLED == TRUE)
bool tws_state_supported = false;
#endif
/*SPLITA2DP */
/* both interface and media task needs to be ready to alloc incoming request */
#define CHECK_BTAV_INIT()                                                    \
  do {                                                                       \
    if (((bt_av_src_callbacks == NULL) && (bt_av_sink_callbacks == NULL)) || \
        (btif_av_cb[0].sm_handle == NULL)) {                                    \
      BTIF_TRACE_WARNING("%s: BTAV not initialized", __func__);              \
      return BT_STATUS_NOT_READY;                                            \
    }                                                                        \
  } while (0)

/* Helper macro to avoid code duplication in the state machine handlers */
#define CHECK_RC_EVENT(e, d)       \
  case BTA_AV_RC_BROWSE_OPEN_EVT:  \
  case BTA_AV_RC_CLOSE_EVT:        \
  case BTA_AV_RC_BROWSE_CLOSE_EVT: \
  case BTA_AV_REMOTE_CMD_EVT:      \
  case BTA_AV_VENDOR_CMD_EVT:      \
  case BTA_AV_META_MSG_EVT:        \
  case BTA_AV_RC_FEAT_EVT:         \
  case BTA_AV_REMOTE_RSP_EVT: {    \
    btif_rc_handler(e, d);         \
  } break;

void btif_av_flow_spec_cmd(int index, int bitrate);

int64_t average_delay;
static int64_t delay_record[DELAY_RECORD_COUNT] = {0}; /* store latest packets delay */
static int delay_record_idx = 0;

static bool btif_av_state_idle_handler(btif_sm_event_t event, void* data, int idx);
static bool btif_av_state_opening_handler(btif_sm_event_t event, void* data, int idx);
static bool btif_av_state_opened_handler(btif_sm_event_t event, void* data, int idx);
static bool btif_av_state_started_handler(btif_sm_event_t event, void* data, int idx);
static bool btif_av_state_closing_handler(btif_sm_event_t event, void* data, int idx);

static bool btif_av_get_valid_idx(int idx);
int btif_av_idx_by_bdaddr(RawAddress *bd_addr);
static int btif_av_get_valid_idx_for_rc_events(RawAddress bd_addr, int rc_handle);
static int btif_get_conn_state_of_device(RawAddress address);
static void btif_av_set_browse_active(RawAddress peer_addr, uint8_t device_switch);
static bt_status_t connect_int(RawAddress *bd_addr, uint16_t uuid);
static void btif_av_check_rc_connection_priority(void *p_data);
static bt_status_t connect_int(RawAddress* bd_addr, uint16_t uuid);
int btif_get_is_remote_started_idx();
bool btif_av_is_state_opened(int i);
static void btif_av_reset_remote_started_flag();
extern void btif_a2dp_update_sink_latency_change();
extern bt_status_t send_av_passthrough_cmd(RawAddress* bd_addr, uint8_t key_code,
                                        uint8_t key_state);
static int other_device_media_packet_count = 0;
void btif_av_set_reconfig_flag(tBTA_AV_HNDL bta_handle);
void btif_av_clear_pending_start_flag();
bool isBATEnabled();

#if (TWS_ENABLED == TRUE)
bool btif_av_is_tws_device_playing(int index);
bool btif_av_is_tws_suspend_triggered(int index);
bool btif_av_is_tws_enabled_for_dev(const RawAddress& rc_addr);
bool btif_av_is_tws_connected(void);
bool btif_av_current_device_is_tws(void);
bool btif_av_is_idx_tws_device(int index);
int btif_av_get_tws_pair_idx(int index);
bool btif_av_is_tws_enable_monocfg(void);
bool btif_av_is_tws_pair_remote_started(int index);
#else
#define btif_av_is_tws_device_playing() 0
#define btif_av_is_tws_suspend_triggered() 0
#define btif_av_is_tws_enabled_for_dev() 0
#define btif_av_is_tws_connected() 0
#define btif_av_current_device_is_tws() 0
#define btif_av_is_idx_tws_device() 0
#define btif_av_get_tws_pair_idx() 0
#define btif_av_is_tws_enable_monocfg() 0
#define btif_av_is_tws_pair_remote_started() 0
#endif
#if (TWS_ENABLED == TRUE)
#if (TWS_STATE_ENABLED == TRUE)
#define TWSP_STATE_OUT_OF_EAR 2
#define TWSP_STATE_IN_EAR 3
bool btif_av_is_tws_ready_to_play();
#endif
#endif
#ifdef AVK_BACKPORT
void btif_av_request_audio_focus(bool enable);
#endif

static const btif_sm_handler_t btif_av_state_handlers[] = {
    btif_av_state_idle_handler, btif_av_state_opening_handler,
    btif_av_state_opened_handler, btif_av_state_started_handler,
    btif_av_state_closing_handler};

static void btif_av_event_free_data(btif_sm_event_t event, void* p_data);
void btif_av_set_offload_status(void);

/*************************************************************************
 * Extern functions
 ************************************************************************/
extern void btif_rc_handler(tBTA_AV_EVT event, tBTA_AV* p_data);
extern bool btif_rc_get_connected_peer(RawAddress* peer_addr);
extern uint8_t btif_rc_get_connected_peer_handle(const RawAddress& peer_addr);
extern RawAddress btif_rc_get_connected_peer_address(uint8_t handle);
extern void btif_rc_check_handle_pending_play(const RawAddress& peer_addr,
                                              bool bSendToApp);
extern void btif_rc_get_playing_device(RawAddress *address);
extern void btif_rc_clear_playing_state(bool play);
extern void btif_rc_clear_priority(RawAddress address);
extern void btif_rc_send_pause_command(RawAddress bda);
extern uint16_t btif_dm_get_br_edr_links();
extern uint16_t btif_dm_get_le_links();
extern void btif_a2dp_on_idle();
extern fixed_queue_t* btu_general_alarm_queue;
extern void btif_media_send_reset_vendor_state();
extern tBTIF_A2DP_SOURCE_VSC btif_a2dp_src_vsc;
extern uint8_t* bta_av_co_get_peer_codec_info(uint8_t hdl);
extern bool bta_avk_is_avdt_sync(uint16_t handle);

/*****************************************************************************
 * Local helper functions
 *****************************************************************************/
bool btif_av_is_connected_on_other_idx(int current_index);
bool btif_av_is_playing_on_other_idx(int current_index);
void btif_av_update_multicast_state(int index);
bool btif_av_get_ongoing_multicast();
tBTA_AV_HNDL btif_av_get_playing_device_hdl();
tBTA_AV_HNDL btif_av_get_av_hdl_from_idx(int idx);
int btif_av_get_other_connected_idx(int current_index);
bool btif_av_is_peer_silenced(RawAddress *bd_addr);
void btif_av_reset_reconfig_flag();
tBTA_AV_HNDL btif_av_get_reconfig_dev_hndl();
void btif_av_reset_codec_reconfig_flag(RawAddress address);
void btif_av_reinit_audio_interface();
bool btif_av_is_suspend_stop_pending_ack();
static void allow_connection(int is_valid, RawAddress *bd_addr);
bool btif_av_is_local_started_on_other_idx(int current_index);

const char* dump_av_sm_state_name(btif_av_state_t state) {
  switch (state) {
    CASE_RETURN_STR(BTIF_AV_STATE_IDLE)
    CASE_RETURN_STR(BTIF_AV_STATE_OPENING)
    CASE_RETURN_STR(BTIF_AV_STATE_OPENED)
    CASE_RETURN_STR(BTIF_AV_STATE_STARTED)
    CASE_RETURN_STR(BTIF_AV_STATE_CLOSING)
    default:
      return "UNKNOWN_STATE";
  }
}

const char* dump_av_sm_event_name(btif_av_sm_event_t event) {
  switch ((int)event) {
    CASE_RETURN_STR(BTA_AV_ENABLE_EVT)
    CASE_RETURN_STR(BTA_AV_REGISTER_EVT)
    CASE_RETURN_STR(BTA_AV_OPEN_EVT)
    CASE_RETURN_STR(BTA_AV_CLOSE_EVT)
    CASE_RETURN_STR(BTA_AV_START_EVT)
    CASE_RETURN_STR(BTA_AV_STOP_EVT)
    CASE_RETURN_STR(BTA_AV_PROTECT_REQ_EVT)
    CASE_RETURN_STR(BTA_AV_PROTECT_RSP_EVT)
    CASE_RETURN_STR(BTA_AV_RC_OPEN_EVT)
    CASE_RETURN_STR(BTA_AV_RC_CLOSE_EVT)
    CASE_RETURN_STR(BTA_AV_RC_BROWSE_OPEN_EVT)
    CASE_RETURN_STR(BTA_AV_RC_BROWSE_CLOSE_EVT)
    CASE_RETURN_STR(BTA_AV_REMOTE_CMD_EVT)
    CASE_RETURN_STR(BTA_AV_REMOTE_RSP_EVT)
    CASE_RETURN_STR(BTA_AV_VENDOR_CMD_EVT)
    CASE_RETURN_STR(BTA_AV_VENDOR_RSP_EVT)
    CASE_RETURN_STR(BTA_AV_RECONFIG_EVT)
    CASE_RETURN_STR(BTA_AV_SUSPEND_EVT)
    CASE_RETURN_STR(BTA_AV_PENDING_EVT)
    CASE_RETURN_STR(BTA_AV_META_MSG_EVT)
    CASE_RETURN_STR(BTA_AV_REJECT_EVT)
    CASE_RETURN_STR(BTA_AV_RC_FEAT_EVT)
    CASE_RETURN_STR(BTA_AV_OFFLOAD_START_RSP_EVT)
    CASE_RETURN_STR(BTA_AV_DELAY_REPORT_EVT)
    CASE_RETURN_STR(BTIF_SM_ENTER_EVT)
    CASE_RETURN_STR(BTIF_SM_EXIT_EVT)
    CASE_RETURN_STR(BTIF_AV_CONNECT_REQ_EVT)
    CASE_RETURN_STR(BTIF_AV_DISCONNECT_REQ_EVT)
    CASE_RETURN_STR(BTIF_AV_START_STREAM_REQ_EVT)
    CASE_RETURN_STR(BTIF_AV_STOP_STREAM_REQ_EVT)
    CASE_RETURN_STR(BTIF_AV_SUSPEND_STREAM_REQ_EVT)
    CASE_RETURN_STR(BTIF_AV_SOURCE_CONFIG_REQ_EVT)
    CASE_RETURN_STR(BTIF_AV_SOURCE_CONFIG_UPDATED_EVT)
    CASE_RETURN_STR(BTIF_AV_REMOTE_SUSPEND_STREAM_REQ_EVT)
    CASE_RETURN_STR(BTIF_AV_SINK_CONFIG_REQ_EVT)
    CASE_RETURN_STR(BTIF_AV_OFFLOAD_START_REQ_EVT)
    CASE_RETURN_STR(BTA_AV_OFFLOAD_STOP_RSP_EVT)
    CASE_RETURN_STR(BTIF_AV_SETUP_CODEC_REQ_EVT)
    default:
      return "UNKNOWN_EVENT";
  }
}

/****************************************************************************
 *  Local helper functions
 ****************************************************************************/
/*******************************************************************************
 *
 * Function         btif_initiate_av_open_timer_timeout
 *
 * Description      Timer to trigger AV open if the remote headset establishes
 *                  RC connection w/o AV connection. The timer is needed to IOP
 *                  with headsets that do establish AV after RC connection.
 *
 * Returns          void
 *
 ******************************************************************************/
static void btif_initiate_av_open_timer_timeout(void* data) {
  RawAddress peer_addr;
  btif_av_connect_req_t connect_req;
  RawAddress *bd_add = (RawAddress *)data;
  if (bd_add == nullptr) return;
  BTIF_TRACE_DEBUG("%s: bd_add: %s", __func__, bd_add->ToString().c_str());

  memset(&connect_req, 0, sizeof(btif_av_connect_req_t));
  /* is there at least one RC connection - There should be */
  if (btif_rc_get_connected_peer(&peer_addr)) {
    BTIF_TRACE_DEBUG("%s: peer_addr: %s", __func__, peer_addr.ToString().c_str());
    /* Check if this peer_addr is same as currently connected AV*/
    if ((*bd_add == peer_addr) &&
        (btif_get_conn_state_of_device(peer_addr) == BTIF_AV_STATE_OPENED)) {
      BTIF_TRACE_DEBUG("AV is already connected");
    } else {
      uint8_t rc_handle;
      int index;
      /* Multicast: Check if AV slot is available for connection
       * If not available, AV got connected to different devices.
       * Disconnect this RC connection without AV connection.
       */
      rc_handle = btif_rc_get_connected_peer_handle(*bd_add);
      index = btif_av_get_valid_idx_for_rc_events(*bd_add, rc_handle);
      if (index >= btif_max_av_clients) {
          BTIF_TRACE_ERROR("%s No slot free for AV connection, back off",
                            __func__);
          return;
      }
      BTIF_TRACE_DEBUG("%s Issuing connect to the remote RC peer", __func__);
    /* In case of AVRCP connection request, we will initiate SRC connection */
    connect_req.target_bda = bd_add;
    if (bt_av_sink_callbacks != NULL)
      connect_req.uuid = UUID_SERVCLASS_AUDIO_SINK;
    else if (bt_av_src_callbacks != NULL)
      connect_req.uuid = UUID_SERVCLASS_AUDIO_SOURCE;
    btif_dispatch_sm_event(BTIF_AV_CONNECT_REQ_EVT, (char*)&connect_req,
                           sizeof(connect_req));
    }
  } else {
    BTIF_TRACE_ERROR("%s: No connected RC peers", __func__);
  }
}

/*****************************************************************************
 *  Static functions
 *****************************************************************************/

/*******************************************************************************
 *
 * Function         btif_report_connection_state
 *
 * Description      Updates the components via the callbacks about the
 *                  connection state of a2dp connection.
 *
 * Returns          None
 *
 ******************************************************************************/
static void btif_report_connection_state(btav_connection_state_t state,
                                         RawAddress* bd_addr) {
  if (bt_av_sink_callbacks != NULL) {
    HAL_CBACK(bt_av_sink_callbacks, connection_state_cb, *bd_addr, state);
  } else if (bt_av_src_callbacks != NULL) {
    HAL_CBACK(bt_av_src_callbacks, connection_state_cb, *bd_addr, state);
  }
}

void btif_report_connection_state_to_ba(btav_connection_state_t state) {
  // send updates to BA as well
  if (state == BTAV_CONNECTION_STATE_DISCONNECTED)
  {
      ba_send_message(BTIF_BA_BT_A2DP_DISC_EVT, 0, NULL, true);
  }
}
/*******************************************************************************
 *
 * Function         btif_report_audio_state
 *
 * Description      Updates the components via the callbacks about the audio
 *                  state of a2dp connection. The state is updated when either
 *                  the remote ends starts streaming (started state) or whenever
 *                  it transitions out of started state (to opened or streaming)
 *                  state.
 *
 * Returns          None
 *
 ******************************************************************************/
static void btif_report_audio_state(btav_audio_state_t state,
                                    RawAddress* bd_addr) {
  if (bt_av_sink_callbacks != NULL) {
    HAL_CBACK(bt_av_sink_callbacks, audio_state_cb, *bd_addr, state);
  } else if (bt_av_src_callbacks != NULL) {
    HAL_CBACK(bt_av_src_callbacks, audio_state_cb, *bd_addr, state);
  }
}

void btif_report_audio_state_to_ba(btav_audio_state_t state) {
  if (state == BTAV_AUDIO_STATE_STARTED)
  {
      ba_send_message(BTIF_BA_BT_A2DP_STARTED_EVT, 0, NULL, true);
  }
  if ((state == BTAV_AUDIO_STATE_REMOTE_SUSPEND) ||
      (state == BTAV_AUDIO_STATE_STOPPED))
  {
      ba_send_message(BTIF_BA_BT_A2DP_PAUSED_EVT, 0, NULL, true);
  }
}
void btif_av_peer_config_dump()
{
   int index = 0;
   RawAddress bd_addr;
   btif_sm_state_t av_state;
   index = btif_av_get_latest_playing_device_idx();
   BTIF_TRACE_DEBUG("Latest playing device index: %d", index);
   btif_get_latest_playing_device(&bd_addr);
   BTIF_TRACE_DEBUG("TARGET BD ADDRESS %s", bd_addr.ToString().c_str());
   if (index == btif_max_av_clients)
   {
       BTIF_TRACE_DEBUG("%s: AV Index invalid", __FUNCTION__);
       return;
   }
   av_state = btif_get_conn_state_of_device(bd_addr);
   BTIF_TRACE_DEBUG("%s: Av_state: %d", __FUNCTION__, av_state);
   BTIF_TRACE_DEBUG("%s: index: %d flags: 0x%x edr: 0x%x SHO: %d current_playing: %d",
                    __FUNCTION__, index, btif_av_cb[index].flags, btif_av_cb[index].edr,
                     btif_av_cb[index].dual_handoff, btif_av_cb[index].current_playing);
   BTIF_TRACE_DEBUG("%s: is_slave: %d is_device_playing: %d",
          __FUNCTION__, btif_av_cb[index].is_slave, btif_av_cb[index].is_device_playing);
   A2dpCodecs* a2dp_codecs = bta_av_get_a2dp_codecs();
   if (a2dp_codecs != nullptr) {
       a2dp_codecs->debug_codec_dump(-1);
   }
}

static void btif_update_source_codec(void* p_data) {
  BTIF_TRACE_DEBUG("%s", __func__);

  btif_av_codec_config_req_t *req = (btif_av_codec_config_req_t *)p_data;
  btif_a2dp_source_encoder_user_config_update_req(req->codec_config, req->bd_addr);

  if (req->codec_config.codec_specific_4 > 0) {
    A2dpCodecConfig* current_codec = bta_av_get_a2dp_current_codec();
    if (current_codec != nullptr) {
      btav_a2dp_codec_config_t codec_config;
      codec_config = current_codec->getCodecConfig();
      if(codec_config.codec_type == BTAV_A2DP_CODEC_INDEX_SOURCE_APTX_ADAPTIVE) {
        int index = btif_max_av_clients;
        int tws_index = btif_max_av_clients;
        uint16_t encoder_mode = req->codec_config.codec_specific_4 & APTX_MODE_MASK;

        if (btif_av_stream_started_ready())
          index = btif_av_get_latest_playing_device_idx();
        else
          index = btif_av_get_latest_device_idx_to_start();

        if(index >= btif_max_av_clients) return;

        if(btif_av_cb[index].tws_device) {
          tws_index = btif_av_get_tws_pair_idx(index);
        }

        if(encoder_mode == APTX_HQ) {
          btif_av_cb[index].aptx_mode = APTX_HQ;
          btif_av_cb[index].codec_latency = APTX_HQ_LATENCY;
          if(tws_index < btif_max_av_clients) {
            btif_av_cb[tws_index].aptx_mode = APTX_HQ;
            btif_av_cb[tws_index].codec_latency = APTX_HQ_LATENCY;
          }
          btif_a2dp_update_sink_latency_change();
          BTIF_TRACE_DEBUG("%s: Aptx Adaptive mode = %d, codec_latency = %d", __func__,
                        btif_av_cb[index].aptx_mode, btif_av_cb[index].codec_latency);
        } else if (encoder_mode == APTX_LL) {
          btif_av_cb[index].aptx_mode = APTX_LL;
          btif_av_cb[index].codec_latency = APTX_LL_LATENCY;
          if(tws_index < btif_max_av_clients) {
            btif_av_cb[tws_index].aptx_mode = APTX_LL;
            btif_av_cb[tws_index].codec_latency = APTX_LL_LATENCY;
          }
          btif_a2dp_update_sink_latency_change();
          BTIF_TRACE_DEBUG("%s: Aptx Adaptive mode = %d, codec_latency = %d", __func__,
                        btif_av_cb[index].aptx_mode, btif_av_cb[index].codec_latency);
        } else if (encoder_mode == APTX_ULL) {
          btif_av_cb[index].aptx_mode = APTX_ULL;
          btif_av_cb[index].codec_latency = APTX_ULL_LATENCY;
          if(tws_index < btif_max_av_clients) {
            btif_av_cb[tws_index].aptx_mode = APTX_ULL;
            btif_av_cb[tws_index].codec_latency = APTX_ULL_LATENCY;
          }
          btif_a2dp_update_sink_latency_change();
          BTIF_TRACE_DEBUG("%s: Aptx Adaptive mode = %d, codec_latency = %d", __func__,
                        btif_av_cb[index].aptx_mode, btif_av_cb[index].codec_latency);
        }
      }
    }
  }
}

static void btif_report_source_codec_state(UNUSED_ATTR void* p_data,
                                            RawAddress* bd_addr) {
  btav_a2dp_codec_config_t codec_config;
  int index = btif_av_idx_by_bdaddr(bd_addr);
  std::vector<btav_a2dp_codec_config_t> codecs_local_capabilities;
  std::vector<btav_a2dp_codec_config_t> codecs_selectable_capabilities;

  A2dpCodecs* a2dp_codecs = bta_av_get_peer_a2dp_codecs(*bd_addr);
  if (a2dp_codecs == nullptr) return;
  if (!a2dp_codecs->getCodecConfigAndCapabilities(
          &codec_config, &codecs_local_capabilities,
          &codecs_selectable_capabilities)) {
    BTIF_TRACE_WARNING(
        "%s: error reporting audio source codec state: "
        "cannot get codec config and capabilities",
        __func__);
    return;
  }

  if (index < btif_max_av_clients) {
    if (codec_config.codec_type == BTAV_A2DP_CODEC_INDEX_SOURCE_APTX_ADAPTIVE) {
      if(btif_av_cb[index].codec_latency == 0)
        btif_av_cb[index].codec_latency = APTX_HQ_LATENCY;
    } else {
      btif_av_cb[index].codec_latency = 0;
    }
  }

  if (btif_a2dp_source_is_hal_v2_supported()) {
    //check for codec update for active device
    if(index < btif_max_av_clients && btif_av_cb[index].current_playing == TRUE) {
      if(btif_a2dp_source_is_restart_session_needed()) {
        RawAddress bt_addr = btif_av_cb[index].peer_bda;
        btif_a2dp_source_restart_session(bt_addr, bt_addr);
        codec_cfg_change = false;
        if (btif_av_cb[index].reconfig_pending) {
          BTIF_TRACE_DEBUG("%s:Set reconfig_a2dp true",__func__);
          reconfig_a2dp = true;
        }
      }
      btif_av_signal_session_ready();
    }
  }

  if (bt_av_src_callbacks != NULL) {
    BTIF_TRACE_DEBUG("%s codec config changed BDA:0x%02X%02X%02X%02X%02X%02X", __func__,
                   bd_addr->address[0], bd_addr->address[1], bd_addr->address[2],
                   bd_addr->address[3], bd_addr->address[4], bd_addr->address[5]);
    HAL_CBACK(bt_av_src_callbacks, audio_config_cb, *bd_addr, codec_config,
              codecs_local_capabilities, codecs_selectable_capabilities);
  }
}


static void btif_av_collission_timer_timeout(void *data) {
  int *arg = (int *)data;
  if (!arg) {
    BTIF_TRACE_ERROR("%s: index is null, return", __func__);
    return;
  }

  int collision_index = *arg;
  if ((collision_index < 0) && (collision_index >= btif_max_av_clients)) {
    BTIF_TRACE_ERROR("%s: collision index is not valid, return", __func__);
    return;
  }

  RawAddress target_bda = collision_detect[collision_index].bd_addr;
  btif_sm_state_t av_state;
  RawAddress av_address;
  BTIF_TRACE_DEBUG("%s: collision_index = %d target_bda: %s, conn_retry_count = %d", __func__,
                      collision_index, target_bda.ToString().c_str(),
                      collision_detect[collision_index].conn_retry_count);

  if (btif_storage_is_device_bonded(&target_bda) != BT_STATUS_SUCCESS){
    BTIF_TRACE_IMP("btif_av_collission_timer_timeout: not bonded device ");
    if (arg) {
      osi_free(arg);
    }
    return;
  } else {
    BTIF_TRACE_IMP("btif_av_collission_timer_timeout: bonded device ");
  }

  av_address = target_bda;
  av_state = btif_get_conn_state_of_device(av_address);
  BTIF_TRACE_DEBUG("%s: AV state: %d BD_Addr: %s", __func__,
                      av_state, av_address.ToString().c_str());

  if (av_state == BTIF_AV_STATE_IDLE &&
      collision_detect[collision_index].conn_retry_count <= 1) {
    if (bt_av_src_callbacks != NULL) {
      BTIF_TRACE_DEBUG("%s Starting A2dp connection", __FUNCTION__);
      collision_detect[collision_index].conn_retry_count++;
      btif_queue_connect(UUID_SERVCLASS_AUDIO_SOURCE, target_bda, connect_int,
                    btif_max_av_clients);
    } else {
      BTIF_TRACE_DEBUG("%s Aborting A2dp connection retry", __FUNCTION__);
    }
  } else if (btif_rc_get_connected_peer_handle(av_address) == BTRC_HANDLE_NONE &&
             collision_detect[collision_index].conn_retry_count <= 1) {
    tBTA_AV_HNDL handle;
    int idx = btif_av_idx_by_bdaddr(&av_address);
    if (idx == btif_max_av_clients) {
      BTIF_TRACE_ERROR("%s: Invalid handle",__func__);
      handle = (tBTA_AV_HNDL)INVALID_INDEX;
    } else {
      handle = btif_av_cb[idx].bta_handle;
    }
    BTIF_TRACE_DEBUG("%s Starting Avrcp connection for handle: %d", __FUNCTION__, handle);
    if ((handle != (tBTA_AV_HNDL)INVALID_INDEX) && (bt_av_src_callbacks != NULL)) {
      BTA_AvOpenRc(handle);
      collision_detect[collision_index].conn_retry_count++;
    } else {
      BTIF_TRACE_DEBUG("%s Aborting Avrcp connection retry", __FUNCTION__);
    }
  } else {
    if (collision_detect[collision_index].conn_retry_count > 1) {
      collision_detect[collision_index].conn_retry_count = 1;
      BTIF_TRACE_DEBUG("%s Connection Retry count exceeded for : %s",
                       __FUNCTION__, collision_detect[collision_index].bd_addr.ToString().c_str());
      memset(&collision_detect[collision_index].bd_addr, 0, sizeof(RawAddress));
      if (arg) {
        osi_free(arg);
      }
      return;
    }
    BTIF_TRACE_DEBUG("%s A2dp already connected", __FUNCTION__);
    BTIF_TRACE_DEBUG("%s Avrcp already connected on handle: %d", __FUNCTION__,
                   btif_rc_get_connected_peer_handle(av_address));
  }

  if (arg) {
    osi_free(arg);
  }
}

static void btif_av_check_and_start_collission_timer(int index) {
  int coll_i = 0;
  int *arg = NULL;
  arg = (int *) osi_malloc(sizeof(int));
  RawAddress target_bda = btif_av_get_addr_by_index(index);
  BTIF_TRACE_DEBUG("%s: index: %d ", __func__, index);

  //check for free index, to start timer
  for (coll_i = 0; coll_i < btif_max_av_clients; coll_i++) {
    if (alarm_is_scheduled(collision_detect[coll_i].av_coll_detected_timer)) {
      BTIF_TRACE_DEBUG("%s: collision alram is already scheduled on coll_i = %d,"
                       " coll_add: %s", __func__, coll_i,
                       collision_detect[coll_i].bd_addr.ToString().c_str());
      //When 2nd time collision timer started for target_bda as part of
      //conn_retry_count, in the middile of timer processconnectotherprofiles has
      //been triggered and connection has been initiated for the same bda.
      //If it fails, then once again timer has been going to start on the same
      //target_bda, by checking on the freed collision index. So, already due to
      //2nd time collision, timer already running on the same target_bda, it is going
      //to allocate one more collision index for the same bda and start the other
      //collision timer. So effectively 2 timers will run for the same bda with
      //different collision indices. So to avoid this, below check is needed.
      if (collision_detect[coll_i].bd_addr == target_bda) {
        BTIF_TRACE_DEBUG("%s: collision alram is already scheduled on target_bda = %s,"
                             "no need to fetch new collision index to start timer.",
                          __func__, target_bda.ToString().c_str());
        if (arg) {
          osi_free(arg);
        }
        break;
      }
      continue;
    } else {
      collision_detect[coll_i].bd_addr = target_bda;
      *arg = coll_i;
      /* Start collision detected timeout */
      BTIF_TRACE_DEBUG("%s: schedule collision alram on coll_i = %d, bd_add: %s",
                __func__, coll_i, collision_detect[coll_i].bd_addr.ToString().c_str());
      alarm_set_on_mloop(collision_detect[coll_i].av_coll_detected_timer,
                          BTIF_TIMEOUT_AV_COLL_DETECTED_MS_2,
                          btif_av_collission_timer_timeout,
                          (void *)arg);
      break;
    }
  }
  if (coll_i >= btif_max_av_clients) {
    if (arg) {
      osi_free(arg);
    }
    BTIF_TRACE_DEBUG("%s: All collision indices are full, no coll_i has been allocated, "
                      "for target_bda: %s",__func__, target_bda.ToString().c_str());
  }
}

static void btif_av_cache_src_codec_config(btif_sm_event_t event, void* p_data, int index) {
  BTIF_TRACE_DEBUG("%s: cache codec_config data to process when we move to proper state",
                                        __func__);
  btif_av_cb[index].reconfig_event = BTIF_AV_SOURCE_CONFIG_REQ_EVT;
  memcpy(&(btif_av_cb[index].reconfig_data), ((tBTA_AV*)p_data), sizeof(tBTA_AV));
}

static void btif_av_process_cached_src_codec_config(int index) {
  BTIF_TRACE_DEBUG("%s: process previousely stored codec config", __func__);
  btif_update_source_codec(&btif_av_cb[index].reconfig_data);
  btif_av_cb[index].reconfig_event = 0;
  memset(&btif_av_cb[index].reconfig_data, 0, sizeof(tBTA_AV));
}


/*****************************************************************************
 *
 * Function     btif_av_state_idle_handler
 *
 * Description  State managing disconnected AV link
 *
 * Returns      true if event was processed, false otherwise
 *
 ******************************************************************************/

static bool btif_av_state_idle_handler(btif_sm_event_t event, void* p_data, int index) {
  char a2dp_role[255] = "false";
  bool other_device_connected = false;
  BTIF_TRACE_IMP("%s event:%s flags %x on index %x", __func__,
                   dump_av_sm_event_name((btif_av_sm_event_t)event),
                   btif_av_cb[index].flags, index);

  switch (event) {
    case BTIF_SM_ENTER_EVT:
      /* clear the peer_bda */
      BTIF_TRACE_EVENT("%s: IDLE state for index: %d", __func__, index);
      memset(&btif_av_cb[index].peer_bda, 0, sizeof(RawAddress)); //TODO
      btif_av_cb[index].flags = 0;
      btif_av_cb[index].edr_3mbps = false;
      btif_av_cb[index].edr = 0;
      btif_av_cb[index].current_playing = false;
      btif_av_cb[index].is_slave = false;
      btif_av_cb[index].is_device_playing = false;
      btif_av_cb[index].reconfig_pending = false;
      btif_av_cb[index].sink_latency = 0;
      btif_av_cb[index].remote_started = false;
      btif_av_cb[index].remote_start_alarm = NULL;
      btif_av_cb[index].is_suspend_for_remote_start = false;
      btif_av_cb[index].retry_rc_connect = false;
#if (TWS_ENABLED == TRUE)
      BTIF_TRACE_EVENT("%s: reset tws_device flag in IDLE state", __func__);
      btif_av_cb[index].tws_device = false;
      btif_av_cb[index].offload_state = false;
#if (TWS_STATE_ENABLED == TRUE)
      btif_av_cb[index].eb_state = 0;
      btif_av_cb[index].tws_state_suspend = false;
#endif
#endif
      btif_av_cb[index].aptx_mode = APTX_HQ;
      btif_av_cb[index].codec_latency = 0;
      btif_av_cb[index].reconfig_event = 0;
      memset(&btif_av_cb[index].reconfig_data, 0, sizeof(tBTA_AV));
      if (alarm_is_scheduled(btif_av_cb[index].suspend_rsp_track_timer)) {
        BTIF_TRACE_DEBUG("%s: clear suspend_rsp_track_timer", __func__);
        alarm_cancel(btif_av_cb[index].suspend_rsp_track_timer);
      }
      btif_av_cb[index].fake_suspend_rsp = false;
      for (int i = 0; i < btif_max_av_clients; i++)
        btif_av_cb[i].dual_handoff = false;
      osi_property_get("persist.vendor.service.bt.a2dp.sink", a2dp_role, "false");
      if (!strncmp("false", a2dp_role, 5)) {
        btif_av_cb[index].peer_sep = AVDT_TSEP_SNK;
        isPeerA2dpSink = true;
      } else {
        btif_av_cb[index].peer_sep = AVDT_TSEP_SRC;
        isPeerA2dpSink = false;
      }
      /* This API will be called twice at initialization
      ** Idle can be moved when device is disconnected too.
      ** Take care of other connected device here.*/
      for (int i = 0; i < btif_max_av_clients; i++) {
        if ((i != index) && btif_av_get_valid_idx(i)) {
          other_device_connected = true;
          break;
        }
      }
      if (other_device_connected == false) {
        BTIF_TRACE_EVENT("%s: reset A2dp states in IDLE ", __func__);
        bta_av_co_init(btif_av_cb[index].codec_priorities, offload_enabled_codecs_config_);
        btif_a2dp_on_idle();
      } else {
        //There is another AV connection, update current playin
        BTIF_TRACE_EVENT("%s: idle state for index %d init_co", __func__, index);
        bta_av_co_peer_init(btif_av_cb[index].codec_priorities, index);
      }
      if (!btif_av_is_local_started_on_other_idx(index) &&
           btif_av_is_split_a2dp_enabled()) {
        BTIF_TRACE_EVENT("%s: reset Vendor flag A2DP state is IDLE", __func__);
        reconfig_a2dp = FALSE;
        btif_media_send_reset_vendor_state();
      }
      break;

    case BTIF_SM_EXIT_EVT:
      if (codec_config_update_enabled == false) {
        codec_config_update_enabled = true;
        BTIF_TRACE_IMP("%s: Codec config update enabled changed to true", __func__);
      }
      break;

    case BTA_AV_ENABLE_EVT:
      BTIF_TRACE_EVENT("AV is enabled now for index: %d", index);
      break;

    case BTA_AV_REGISTER_EVT:
      BTIF_TRACE_EVENT("The AV Handle:%d", ((tBTA_AV*)p_data)->registr.hndl);
      btif_av_cb[index].bta_handle = ((tBTA_AV*)p_data)->registr.hndl;
      break;

    case BTIF_AV_CONNECT_REQ_EVT: {
        btif_av_connect_req_t* connect_req_p = (btif_av_connect_req_t*)p_data;
        btif_av_cb[index].peer_bda = *connect_req_p->target_bda;
        BTA_AvOpen(btif_av_cb[index].peer_bda, btif_av_cb[index].bta_handle, true,
                   BTA_SEC_AUTHENTICATE, connect_req_p->uuid);
#if (BT_IOT_LOGGING_ENABLED == TRUE)
        device_iot_config_addr_set_int(btif_av_cb[index].peer_bda, IOT_CONF_KEY_A2DP_ROLE,
            (connect_req_p->uuid == UUID_SERVCLASS_AUDIO_SOURCE) ? IOT_CONF_VAL_A2DP_ROLE_SINK :
            IOT_CONF_VAL_A2DP_ROLE_SOURCE);
        device_iot_config_addr_int_add_one(btif_av_cb[index].peer_bda,
            IOT_CONF_KEY_A2DP_CONN_COUNT);
#endif
#if (TWS_ENABLED == TRUE)
      BTIF_TRACE_EVENT("update tws device status");
      btif_av_cb[index].tws_device = BTM_SecIsTwsPlusDev(btif_av_cb[index].peer_bda.address);
      BTA_AvUpdateTWSDevice(btif_av_cb[index].tws_device, btif_av_cb[index].bta_handle);
#endif
      btif_sm_change_state(btif_av_cb[index].sm_handle, BTIF_AV_STATE_OPENING);
      } break;

    case BTA_AV_PENDING_EVT:
    case BTA_AV_RC_OPEN_EVT:
      /* IOP_FIX: Jabra 620 only does RC open without AV open whenever it
       * connects. So
       * as per the AV WP, an AVRC connection cannot exist without an AV
       * connection. Therefore,
       * we initiate an AV connection if an RC_OPEN_EVT is received when we are
       * in AV_CLOSED state.
       * We initiate the AV connection after a small 3s timeout to avoid any
       * collisions from the
       * headsets, as some headsets initiate the AVRC connection first and then
       * immediately initiate the AV connection
       *
       * TODO: We may need to do this only on an AVRCP Play. FixMe
       */
      /* Check if connection allowed with this device
       * In Dual A2dp case, this event can come for both the headsets.
       * Reject second connection request as we are already checking
       * for device priority for first device and we cannot queue
       * incoming connections requests.
       */
      if (idle_rc_event != 0) {
        BTIF_TRACE_DEBUG("Processing another RC Event ");
        return false;
      }
      memcpy(&idle_rc_data, ((tBTA_AV*)p_data), sizeof(tBTA_AV));
      if (event == BTA_AV_RC_OPEN_EVT) {
        if (((tBTA_AV*)p_data)->rc_open.status == BTA_AV_SUCCESS) {
            btif_av_cb[index].peer_bda = ((tBTA_AV*)p_data)->rc_open.peer_addr;
        } else {
          idle_rc_event = 0;
          return true;
        }
      } else
        btif_av_cb[index].peer_bda = ((tBTA_AV*)p_data)->pend.bd_addr;

      // Only for AVDTP connection request move to opening state
      if (event == BTA_AV_PENDING_EVT) {
#if (BT_IOT_LOGGING_ENABLED == TRUE)
        device_iot_config_addr_int_add_one(btif_av_cb[index].peer_bda,
            IOT_CONF_KEY_A2DP_CONN_COUNT);
#endif
        btif_sm_change_state(btif_av_cb[index].sm_handle, BTIF_AV_STATE_OPENING);
      }

      if (bt_av_src_callbacks != NULL) {
        BTIF_TRACE_DEBUG("Calling connection priority callback ");
        idle_rc_event = event;
#ifdef BT_AV_SHO_FEATURE
        HAL_CBACK(bt_av_src_callbacks, connection_priority_cb, // call allow_conn for all callbacks
           &(btif_av_cb[index].peer_bda));
#else
        allow_connection(true, &(btif_av_cb[index].peer_bda));
#endif
      }
      if (bt_av_sink_callbacks != NULL) {
        if (event == BTA_AV_PENDING_EVT)
            BTA_AvOpen(btif_av_cb[index].peer_bda, btif_av_cb[index].bta_handle,
               true, BTA_SEC_AUTHENTICATE, UUID_SERVCLASS_AUDIO_SINK);
        else if (event == BTA_AV_RC_OPEN_EVT) {
          alarm_set_on_mloop(av_open_on_rc_timer,
          BTIF_TIMEOUT_AV_OPEN_ON_RC_MS,
          btif_initiate_av_open_timer_timeout, NULL);
          btif_rc_handler(event, (tBTA_AV *)p_data);
        }
      }
      break;

    case BTA_AV_RC_BROWSE_OPEN_EVT:
      BTIF_TRACE_DEBUG("%s: BTA_AV_RC_BROWSE_OPEN_EVT received", __func__);
      btif_rc_handler(event, (tBTA_AV*)p_data);
      break;

    case BTIF_AV_SOURCE_CONFIG_REQ_EVT:
      btif_update_source_codec(p_data);
      break;

    case BTIF_AV_SOURCE_CONFIG_UPDATED_EVT:
    {
      BTIF_TRACE_DEBUG("%s: BTIF_AV_SOURCE_CONFIG_UPDATED_EVT received, ignore", __func__);
    }
    break;

    /*
     * In case Signalling channel is not down
     * and remote started Streaming Procedure
     * we have to handle config and open event in
     * idle_state. We hit these scenarios while running
     * PTS test case for AVRCP Controller
     */
    case BTIF_AV_SINK_CONFIG_REQ_EVT: {
      btif_av_sink_config_req_t req;
      // copy to avoid alignment problems
      memcpy(&req, p_data, sizeof(req));

      BTIF_TRACE_WARNING(
          "%s: BTIF_AV_SINK_CONFIG_REQ_EVT sample_rate=%d "
          "channel_count=%d",
          __func__, req.sample_rate, req.channel_count);
      if (bt_av_sink_callbacks != NULL) {
        HAL_CBACK(bt_av_sink_callbacks, audio_config_cb, req.peer_bd,
                  req.sample_rate, req.channel_count);
      }
    } break;

    case BTA_AV_OPEN_EVT: {
      /* We get this event in Idle State if Signaling
       * channel is not closed, only Streaming channel was
       * closed earlier, and now only stream setup process is
       * initiated.
       */
      tBTA_AV *p_bta_data = (tBTA_AV*)p_data;
      btav_connection_state_t state;
      BTIF_TRACE_DEBUG("status:%d, edr 0x%x",p_bta_data->open.status,
                               p_bta_data->open.edr);

      if (p_bta_data->open.status == BTA_AV_SUCCESS) {
        state = BTAV_CONNECTION_STATE_CONNECTED;
        btif_av_cb[index].edr = p_bta_data->open.edr;
        if (p_bta_data->open.role == HOST_ROLE_SLAVE)
          btif_av_cb[index].is_slave = true;
        btif_av_cb[index].peer_sep = p_bta_data->open.sep;
#if (TWS_ENABLED == TRUE)
        BTIF_TRACE_DEBUG("is tws_device = %d",p_bta_data->open.tws_device);
        btif_av_cb[index].tws_device = p_bta_data->open.tws_device;
#endif
        if (p_bta_data->open.edr & BTA_AV_EDR_3MBPS) {
          BTIF_TRACE_DEBUG("remote supports 3 mbps");
          btif_av_cb[index].edr_3mbps = true;
        }

        btif_av_cb[index].peer_bda = ((tBTA_AV*)p_data)->open.bd_addr;
      } else {
        BTIF_TRACE_WARNING("BTA_AV_OPEN_EVT::FAILED status: %d",
                             p_bta_data->open.status );
        state = BTAV_CONNECTION_STATE_DISCONNECTED;
      }

      /* change state to open based on the status */
      if (p_bta_data->open.status == BTA_AV_SUCCESS) {
        /* inform the application of the event */
        if (!btif_a2dp_source_is_hal_v2_supported() &&
           btif_av_is_split_a2dp_enabled() &&btif_a2dp_audio_if_init != true) {
          BTIF_TRACE_DEBUG("Got OPEN_EVT in IDLE state, init audio interface");
          btif_a2dp_audio_interface_init();
          btif_a2dp_audio_if_init = true;
        }
        btif_report_connection_state(state, &(btif_av_cb[index].peer_bda));
        btif_sm_change_state(btif_av_cb[index].sm_handle, BTIF_AV_STATE_OPENED);
        btif_report_connection_state_to_ba(state);
        /* BTIF AV State updated, now check
         * and update multicast state
         */
        btif_av_update_multicast_state(index);
        if (btif_av_cb[index].peer_sep == AVDT_TSEP_SNK) {
          /* if queued PLAY command,  send it now */
          if (!btif_a2dp_source_is_hal_v2_supported())
            btif_rc_check_handle_pending_play(p_bta_data->open.bd_addr,
                   (p_bta_data->open.status == BTA_AV_SUCCESS));
        } else if (btif_av_cb[index].peer_sep == AVDT_TSEP_SRC) {
          /* if queued PLAY command,  send it now */
          btif_rc_check_handle_pending_play(p_bta_data->open.bd_addr, false);
          /* Bring up AVRCP connection too */
          BTA_AvOpenRc(btif_av_cb[index].bta_handle);
        }
      }

      btif_av_connect_req_t* connect_req_t = (btif_av_connect_req_t*)p_data;
      if (bt_av_sink_callbacks != NULL)
          connect_req_t->uuid = UUID_SERVCLASS_AUDIO_SINK;
      else if (bt_av_src_callbacks != NULL)
          connect_req_t->uuid = UUID_SERVCLASS_AUDIO_SOURCE;
      btif_queue_advance_by_uuid(connect_req_t->uuid, &(btif_av_cb[index].peer_bda));
    } break;

    case BTA_AV_REMOTE_CMD_EVT:
    case BTA_AV_VENDOR_CMD_EVT:
    case BTA_AV_META_MSG_EVT:
    case BTA_AV_RC_FEAT_EVT:
    case BTA_AV_REMOTE_RSP_EVT:
      btif_rc_handler(event, (tBTA_AV*)p_data);
      break;

    case BTA_AV_RC_CLOSE_EVT:
      BTIF_TRACE_DEBUG("%s: BTA_AV_RC_CLOSE_EVT: Stopping AV timer.", __func__);
      BTIF_TRACE_DEBUG("BTA_AV_RC_CLOSE_EVT: clear peer_bda.address index : %d", index);
      btif_av_cb[index].peer_bda =  RawAddress::kEmpty;
      BTIF_TRACE_DEBUG("BTA_AV_RC_CLOSE_EVT: Stopping AV timer.");
      alarm_cancel(av_open_on_rc_timer);
      btif_rc_handler(event, (tBTA_AV*)p_data);
      break;

    case BTIF_AV_OFFLOAD_START_REQ_EVT:
      BTIF_TRACE_ERROR(
          "%s: BTIF_AV_OFFLOAD_START_REQ_EVT: Stream not Started IDLE",
          __func__);
      btif_a2dp_on_offload_started(BTA_AV_FAIL);
      break;

    default:
      BTIF_TRACE_WARNING("%s: unhandled event=%s", __func__,
                         dump_av_sm_event_name((btif_av_sm_event_t)event));
      return false;
  }

  return true;
}
/*****************************************************************************
 *
 * Function        btif_av_state_opening_handler
 *
 * Description     Intermediate state managing events during establishment
 *                 of avdtp channel
 *
 * Returns         true if event was processed, false otherwise
 *
 ******************************************************************************/

static bool btif_av_state_opening_handler(btif_sm_event_t event, void* p_data,
                                          int index) {
  RawAddress* bt_addr = nullptr;
  A2dpCodecs* a2dp_codecs = nullptr;
  BTIF_TRACE_IMP("%s event:%s flags %x on index %x", __func__,
                   dump_av_sm_event_name((btif_av_sm_event_t)event),
                   btif_av_cb[index].flags, index);

  switch (event) {
    case BTIF_SM_ENTER_EVT:
      //When uncheck media audio from settings UI and try to connect from remote,
      //a2dp would fail. Then check the media audio from UI, then due to peer codec info
      //null, so it will not go for A2dp connection. So reinit peer codecs if null.
      a2dp_codecs = bta_av_get_peer_a2dp_codecs(btif_av_cb[index].peer_bda);
      if (a2dp_codecs == nullptr) {
        BTIF_TRACE_DEBUG("%s: initialize peer codecs for index %d ", __func__, index);
        bta_av_co_peer_init(btif_av_cb[index].codec_priorities, index);
      }
      /* inform the application that we are entering connecting state */
      if (bt_av_sink_callbacks != NULL)
        HAL_CBACK(bt_av_sink_callbacks, connection_state_cb,
                  btif_av_cb[index].peer_bda, BTAV_CONNECTION_STATE_CONNECTING);
      else if (bt_av_src_callbacks != NULL)
        HAL_CBACK(bt_av_src_callbacks, connection_state_cb,
                  btif_av_cb[index].peer_bda, BTAV_CONNECTION_STATE_CONNECTING);
      break;

    case BTIF_SM_EXIT_EVT:
      break;

    case BTA_AV_REJECT_EVT:
      BTIF_TRACE_WARNING("%s: Received BTA_AV_REJECT_EVT", __func__);
      btif_report_connection_state(BTAV_CONNECTION_STATE_DISCONNECTED,
                                   &(btif_av_cb[index].peer_bda));
      btif_sm_change_state(btif_av_cb[index].sm_handle, BTIF_AV_STATE_IDLE);
#if (BT_IOT_LOGGING_ENABLED == TRUE)
      device_iot_config_addr_int_add_one(btif_av_cb[index].peer_bda,
          IOT_CONF_KEY_A2DP_CONN_FAIL_COUNT);
#endif
      break;

    case BTA_AV_OPEN_EVT: {
      tBTA_AV* p_bta_data = (tBTA_AV*)p_data;
      btav_connection_state_t state;
      btif_sm_state_t av_state;
      BTIF_TRACE_DEBUG("status:%d, edr 0x%x, role: 0x%x", p_bta_data->open.status,
                       p_bta_data->open.edr, p_bta_data->open.role);

      if (p_bta_data->open.status == BTA_AV_SUCCESS) {
        state = BTAV_CONNECTION_STATE_CONNECTED;
        av_state = BTIF_AV_STATE_OPENED;
        RawAddress target_bda = btif_av_get_addr_by_index(index);
        for (int i = 0; i < btif_max_av_clients; i++) {
          if (target_bda == collision_detect[i].bd_addr) {
            BTIF_TRACE_DEBUG("%s: Collision and Target add are matched.", __func__);
            collision_detect[i].conn_retry_count = 1;
            if (alarm_is_scheduled(collision_detect[i].av_coll_detected_timer)) {
               alarm_cancel(collision_detect[i].av_coll_detected_timer);
               BTIF_TRACE_DEBUG("%s: clear av_coll_detected_timer on i = %d and add = %s",
                       __func__, i, collision_detect[i].bd_addr.ToString().c_str());
            }
            memset(&collision_detect[i].bd_addr, 0, sizeof(RawAddress));
            break;
          }
        }
/* SPLITA2DP */
        if (btif_a2dp_audio_if_init != true) {
          if (btif_av_is_split_a2dp_enabled() &&
             !btif_a2dp_source_is_hal_v2_supported()) {
            BTIF_TRACE_DEBUG("Split a2dp enabled with hal1.0: initialize interface ");
            btif_a2dp_audio_if_init = true;
            btif_a2dp_audio_interface_init();
          } else {
            BTIF_TRACE_DEBUG("Split a2dp not enabled or hal2.0 enabled");
          }
        } else {
          BTIF_TRACE_DEBUG("audio interface is already initialized");
        }
/* SPLITA2DP */
        btif_av_cb[index].edr = p_bta_data->open.edr;
        if (p_bta_data->open.role == HOST_ROLE_SLAVE)
          btif_av_cb[index].is_slave = true;
        btif_av_cb[index].peer_sep = p_bta_data->open.sep;
#if (TWS_ENABLED == TRUE)
        BTIF_TRACE_DEBUG("is tws_device = %d",p_bta_data->open.tws_device);
        btif_av_cb[index].tws_device = p_bta_data->open.tws_device;
        if (btif_av_cb[index].tws_device) {
          int idx = btif_av_get_tws_pair_idx(index);
          if (idx < btif_max_av_clients) {
            btif_av_cb[index].aptx_mode = btif_av_cb[idx].aptx_mode;
            BTIF_TRACE_DEBUG("Updating aptx mode to second connected earbud: %d",
                          btif_av_cb[index].aptx_mode);
          }
        }
#endif
        if (p_bta_data->open.edr & BTA_AV_EDR_3MBPS) {
          BTIF_TRACE_DEBUG("remote supports 3 mbps");
          btif_av_cb[index].edr_3mbps = true;
        }
        btif_av_cb[index].avdt_sync = bta_avk_is_avdt_sync(btif_av_cb[index].bta_handle);

        if (btif_av_cb[index].avdt_sync) {
          BTIF_TRACE_DEBUG("avdt_sync TRUE");
        } else {
          BTIF_TRACE_DEBUG("avdt_sync FALSE");
        }
      } else {
        BTIF_TRACE_WARNING("%s: BTA_AV_OPEN_EVT::FAILED status: %d", __func__,
                           p_bta_data->open.status);

        /* Multicast: Check if connected to AVRC only device
         * disconnect when Dual A2DP/Multicast is supported.
         */
        RawAddress peer_addr;
        uint8_t peer_handle = BTRC_HANDLE_NONE;
        if (btif_rc_get_connected_peer(&peer_addr) &&
            (btif_av_cb[index].peer_bda == peer_addr)) {
          /* Do not disconnect AVRCP connection if A2DP
           * connection failed due to SDP failure since remote
           * may not support A2DP. In such case we will keep
           * AVRCP only connection.
           */
          if (p_bta_data->open.status != BTA_AV_FAIL_SDP) {
            BTIF_TRACE_WARNING("%s: Disconnecting AVRCP: peer_addr=%s", __func__,
                             peer_addr.ToString().c_str());
            peer_handle = btif_rc_get_connected_peer_handle(peer_addr);
            if (peer_handle != BTRC_HANDLE_NONE) {
              BTA_AvCloseRc(peer_handle);
            }
          } else {
            BTIF_TRACE_WARNING("Keep AVRCP only connection");
          }
        }
        state = BTAV_CONNECTION_STATE_DISCONNECTED;
        av_state = BTIF_AV_STATE_IDLE;
#if (BT_IOT_LOGGING_ENABLED == TRUE)
        device_iot_config_addr_int_add_one(btif_av_cb[index].peer_bda,
            IOT_CONF_KEY_A2DP_CONN_FAIL_COUNT);
#endif
      }
      if (p_bta_data->open.status != BTA_AV_SUCCESS &&
              p_bta_data->open.status != BTA_AV_FAIL_SDP) {
          btif_av_check_and_start_collission_timer(index);
      }
      /* inform the application of the event */
      btif_report_connection_state(state, &(btif_av_cb[index].peer_bda));
      /* change state to open/idle based on the status */
      btif_sm_change_state(btif_av_cb[index].sm_handle, av_state);
      btif_report_connection_state_to_ba(state);
      /* Check if the other connected AV is playing. If YES, trigger DUAL Handoff */
      if (p_bta_data->open.status == BTA_AV_SUCCESS) {
        /* BTIF AV State updated, now check
         * and update multicast state
         */
        btif_av_update_multicast_state(index);
        if (btif_av_cb[index].peer_sep == AVDT_TSEP_SNK) {
          /* if queued PLAY command,  send it now */
          if (!btif_a2dp_source_is_hal_v2_supported())
            btif_rc_check_handle_pending_play(p_bta_data->open.bd_addr,
                      (p_bta_data->open.status == BTA_AV_SUCCESS));
        } else if (btif_av_cb[index].peer_sep == AVDT_TSEP_SRC) {
          /* if queued PLAY command,  send it now */
          btif_rc_check_handle_pending_play(p_bta_data->open.bd_addr, false);
          /* Bring up AVRCP connection too */
          BTA_AvOpenRc(btif_av_cb[index].bta_handle);
        }
      }
        btif_av_connect_req_t* connect_req_t = (btif_av_connect_req_t*)p_data;
        if (bt_av_sink_callbacks != NULL)
            connect_req_t->uuid = UUID_SERVCLASS_AUDIO_SINK;
        else if (bt_av_src_callbacks != NULL)
            connect_req_t->uuid = UUID_SERVCLASS_AUDIO_SOURCE;
        btif_queue_advance_by_uuid(connect_req_t->uuid, &(btif_av_cb[index].peer_bda));
    } break;

    case BTIF_AV_SOURCE_CONFIG_REQ_EVT:
      if(codec_cfg_change)
        btif_av_cache_src_codec_config(BTIF_AV_SOURCE_CONFIG_REQ_EVT, p_data, index);
      break;

    case BTIF_AV_SOURCE_CONFIG_UPDATED_EVT:
      if (p_data != NULL) {
          bt_addr = (RawAddress *)p_data;
      } else {
          BTIF_TRACE_DEBUG("%s: p_data not null", __func__);
          bt_addr = &btif_av_cb[index].peer_bda;
      }

      btif_report_source_codec_state(p_data, bt_addr);
      break;

    case BTIF_AV_SINK_CONFIG_REQ_EVT: {
      btif_av_sink_config_req_t req;
      // copy to avoid alignment problems
      memcpy(&req, p_data, sizeof(req));

      BTIF_TRACE_WARNING("BTIF_AV_SINK_CONFIG_REQ_EVT %d %d", req.sample_rate,
                         req.channel_count);
      if (btif_av_cb[index].peer_sep == AVDT_TSEP_SRC &&
          bt_av_sink_callbacks != NULL) {
        HAL_CBACK(bt_av_sink_callbacks, audio_config_cb, btif_av_cb[index].peer_bda,
                  req.sample_rate, req.channel_count);
      }
    } break;

    case BTIF_AV_CONNECT_REQ_EVT: {
      // Check for device, if same device which moved to opening then ignore
      // callback
      btif_av_connect_req_t* connect_req_p = (btif_av_connect_req_t*)p_data;
      if (memcmp(((btif_av_connect_req_t*)p_data)->target_bda,
                 &(btif_av_cb[index].peer_bda), sizeof(btif_av_cb[index].peer_bda)) == 0) {
        BTIF_TRACE_DEBUG(
            "%s: Same device moved to Opening state,ignore Connect Req",
            __func__);
      } else {
          RawAddress& target_bda = *connect_req_p->target_bda;
        BTIF_TRACE_WARNING(
            "%s: device %s is already connecting, reject Connect request to %s",
            __func__, btif_av_cb[index].peer_bda.ToString().c_str(),
            target_bda.ToString().c_str());
        btif_report_connection_state(BTAV_CONNECTION_STATE_DISCONNECTED,
                                     &target_bda);
        btif_report_connection_state_to_ba(BTAV_CONNECTION_STATE_DISCONNECTED);
      }
      // Ignore all connection request if we are already opening
      if (bt_av_sink_callbacks != NULL)
          connect_req_p->uuid = UUID_SERVCLASS_AUDIO_SINK;
      else if (bt_av_src_callbacks != NULL)
          connect_req_p->uuid = UUID_SERVCLASS_AUDIO_SOURCE;
      btif_queue_advance_by_uuid(connect_req_p->uuid, &(btif_av_cb[index].peer_bda));
    } break;

    case BTA_AV_PENDING_EVT: {
      // Check for device, if same device which moved to opening then ignore
      // callback
      if (((tBTA_AV*)p_data)->pend.bd_addr == btif_av_cb[index].peer_bda) {
        BTIF_TRACE_DEBUG(
            "%s: Same device moved to Opening state,ignore Pending Req",
            __func__);
        break;
      } else {
          const RawAddress& bd_addr = ((tBTA_AV*)p_data)->pend.bd_addr;
        BTIF_TRACE_WARNING(
            "%s: device %s is already connecting, reject incoming request "
            "from %s",
            __func__, btif_av_cb[index].peer_bda.ToString().c_str(),
            bd_addr.ToString().c_str());
        BTA_AvDisconnect(bd_addr);
      }
    } break;

    case BTIF_AV_OFFLOAD_START_REQ_EVT:
      BTIF_TRACE_ERROR(
          "%s: BTIF_AV_OFFLOAD_START_REQ_EVT: Stream not Started OPENING",
          __func__);
      btif_a2dp_on_offload_started(BTA_AV_FAIL);
      break;

    case BTA_AV_CLOSE_EVT: {
      /* avdtp link is closed. Check if any other device is playing
       * and this is not the one
       */
      if (!btif_av_is_playing())
        btif_a2dp_on_stopped(NULL);
      btif_av_cb[index].avdt_sync = FALSE;
      /* inform the application that we are disconnected */
      btif_report_connection_state(BTAV_CONNECTION_STATE_DISCONNECTED,
         &(btif_av_cb[index].peer_bda));
      btif_av_connect_req_t* connect_req_t = (btif_av_connect_req_t*)p_data;
      if (bt_av_sink_callbacks != NULL)
          connect_req_t->uuid = UUID_SERVCLASS_AUDIO_SINK;
      else if (bt_av_src_callbacks != NULL)
          connect_req_t->uuid = UUID_SERVCLASS_AUDIO_SOURCE;
      btif_queue_advance_by_uuid(connect_req_t->uuid, &(btif_av_cb[index].peer_bda));
      btif_av_check_and_start_collission_timer(index);
      btif_sm_change_state(btif_av_cb[index].sm_handle, BTIF_AV_STATE_IDLE);
      btif_report_connection_state_to_ba(BTAV_CONNECTION_STATE_DISCONNECTED);
#if (BT_IOT_LOGGING_ENABLED == TRUE)
      device_iot_config_addr_int_add_one(btif_av_cb[index].peer_bda,
          IOT_CONF_KEY_A2DP_CONN_FAIL_COUNT);
#endif
      } break;

    case BTIF_AV_DISCONNECT_REQ_EVT: {
       uint8_t peer_handle = BTRC_HANDLE_NONE;
       btif_report_connection_state(BTAV_CONNECTION_STATE_DISCONNECTED,
           &(btif_av_cb[index].peer_bda));
       if (!btif_av_cb[index].peer_bda.IsEmpty())
         peer_handle = btif_rc_get_connected_peer_handle(btif_av_cb[index].peer_bda);

       if (peer_handle != BTRC_HANDLE_NONE) {
         BTIF_TRACE_WARNING("%s: RC connected to %s, disc RC too since AV is being aborted",
                 __func__, btif_av_cb[index].peer_bda.ToString().c_str());
         BTA_AvCloseRc(peer_handle);
       }
       BTA_AvClose(btif_av_cb[index].bta_handle);
       btif_queue_advance();
       btif_sm_change_state(btif_av_cb[index].sm_handle, BTIF_AV_STATE_IDLE);
       btif_report_connection_state_to_ba(BTAV_CONNECTION_STATE_DISCONNECTED);
       } break;

    case BTA_AV_RC_OPEN_EVT:
       btif_rc_handler(event, (tBTA_AV*)p_data);;
       break;

    case BTA_AV_DELAY_REPORT_EVT:
      /* Initial delay report after avdtp stream configuration */
      BTIF_TRACE_DEBUG("%s : BTA_AV_DELAY_REPORT_EVT received", __func__);
      if (btif_av_cb[index].peer_sep == AVDT_TSEP_SNK) {
        btif_av_cb[index].sink_latency = ((tBTA_AV*)p_data)->delay_rpt.sink_delay;
      }
      break;

      CHECK_RC_EVENT(event, (tBTA_AV*)p_data);

    default:
      BTIF_TRACE_WARNING("%s: unhandled event=%s", __func__,
                         dump_av_sm_event_name((btif_av_sm_event_t)event));
      return false;
  }
  return true;
}

/*****************************************************************************
 *
 * Function        btif_av_state_closing_handler
 *
 * Description     Intermediate state managing events during closing
 *                 of avdtp channel
 *
 * Returns         true if event was processed, false otherwise
 *
 ******************************************************************************/

static bool btif_av_state_closing_handler(btif_sm_event_t event, void* p_data, int index) {
  RawAddress *bt_addr = nullptr;

  BTIF_TRACE_IMP("%s event:%s flags %x and index %x", __func__,
                   dump_av_sm_event_name((btif_av_sm_event_t)event),
                   btif_av_cb[index].flags, index);

  switch (event) {
    case BTIF_SM_ENTER_EVT:
      if (btif_av_cb[index].peer_sep == AVDT_TSEP_SNK) {
        /* Multicast/Soft Hand-off:
         * If MC/SHO is enabled we need to keep/start playing on
         * other device.
         */
        if (btif_av_is_connected_on_other_idx(index)) {
          if (btif_av_is_playing()) {
              APPL_TRACE_DEBUG("%s: Keep playing on other device", __func__);
          } else {
             if (btif_av_cb[index].flags & BTIF_AV_FLAG_LOCAL_SUSPEND_PENDING) {
                 APPL_TRACE_DEBUG("%s: Not playing on other device: Stop media task as local suspend pending", __func__);
                 btif_a2dp_on_stopped(NULL);
             } else if (btif_av_cb[index].current_playing) {
                APPL_TRACE_DEBUG("%s: Not playing on other device: Set Flush as active device", __func__);
                btif_a2dp_source_set_tx_flush(true);
                btif_a2dp_source_stop_audio_req();
             }
          }
        } else {
          /* Single connections scenario:
           * Immediately stop transmission of frames
           * wait for audioflinger to stop a2dp
           */
          APPL_TRACE_DEBUG("%s: setting tx_flush to true.", __func__);
          btif_a2dp_source_set_tx_flush(true);
        }
      }
      if (btif_av_cb[index].peer_sep == AVDT_TSEP_SRC && btif_av_cb[index].current_playing)
        btif_a2dp_sink_set_rx_flush(true);
      break;

    case BTA_AV_STOP_EVT:
    case BTIF_AV_STOP_STREAM_REQ_EVT:
      if (btif_av_cb[index].peer_sep == AVDT_TSEP_SNK) {
        /* Dont stop in DUAL A2dp connections, as
         * UIPC will keep waiting for Audio CTRL channel
         * to get closed which is not required in Dual A2dp.
         * We will stop only when only single A2dp conn is present.*/
        if (btif_av_is_connected_on_other_idx(index)) {
          if (!btif_av_is_playing() ||
            (is_multicast_supported && btif_av_cb[index].current_playing)) {
            APPL_TRACE_WARNING("%s: Suspend the AV Data channel", __func__);
            //Stop media task
            btif_a2dp_on_stopped(NULL);
            if (is_multicast_supported && btif_av_is_playing()) {
              btif_dispatch_sm_event(BTIF_AV_SUSPEND_STREAM_REQ_EVT, NULL, 0);
            }
          }
        } else {
          /* immediately flush any pending tx frames while suspend is pending */
          APPL_TRACE_WARNING("%s: Stop the AV Data channel", __func__);
          btif_a2dp_source_set_tx_flush(true);
          btif_a2dp_on_stopped(NULL);
        }
      }
      if (btif_av_cb[index].peer_sep == AVDT_TSEP_SRC) {
        btif_a2dp_sink_set_rx_flush(true);
        btif_a2dp_on_stopped(NULL);
      }
      break;

    case BTIF_SM_EXIT_EVT:
      break;

    case BTIF_AV_SOURCE_CONFIG_REQ_EVT:
      btif_update_source_codec(p_data);
      break;

    case BTIF_AV_SOURCE_CONFIG_UPDATED_EVT:
      if (p_data != NULL) {
          bt_addr = (RawAddress *)p_data;
      } else {
          BTIF_TRACE_DEBUG("%s: p_data not null", __func__);
          bt_addr = &btif_av_cb[index].peer_bda;
      }


      btif_report_source_codec_state(p_data, bt_addr);
      break;

    case BTA_AV_CLOSE_EVT:
      /* inform the application that we are disconnecting */
      btif_report_connection_state(BTAV_CONNECTION_STATE_DISCONNECTED,
                                   &(btif_av_cb[index].peer_bda));

      btif_sm_change_state(btif_av_cb[index].sm_handle, BTIF_AV_STATE_IDLE);
      btif_report_connection_state_to_ba(BTAV_CONNECTION_STATE_DISCONNECTED);
      break;

    case BTIF_AV_OFFLOAD_START_REQ_EVT:
      BTIF_TRACE_ERROR(
          "%s: BTIF_AV_OFFLOAD_START_REQ_EVT: Stream not Started Closing",
          __func__);
      btif_a2dp_on_offload_started(BTA_AV_FAIL);
      break;

    CHECK_RC_EVENT(event, (tBTA_AV*)p_data);

    default:
      BTIF_TRACE_WARNING("%s: unhandled event=%s", __func__,
                         dump_av_sm_event_name((btif_av_sm_event_t)event));
      return false;
  }
  return true;
}

void btif_av_signal_session_ready() {
  std::unique_lock<std::mutex> guard(session_wait_mutex_);
  if(!session_wait) {
    session_wait = true;
    session_wait_cv.notify_all();
  } else {
    BTIF_TRACE_WARNING("%s: already signalled ",__func__);
  }
}

void btif_av_update_reconfigure_event(int index) {

  if (btif_av_is_split_a2dp_enabled() &&
      btif_av_cb[index].current_playing == TRUE) {
    if (codec_cfg_change && !btif_a2dp_source_is_hal_v2_supported()) {
      codec_cfg_change = false;
      if (!isBitRateChange && !isBitsPerSampleChange)
        reconfig_a2dp = TRUE;
      isBitRateChange = false;
      isBitsPerSampleChange = false;
      btav_a2dp_codec_config_t codec_config;
      std::vector<btav_a2dp_codec_config_t> codecs_local_capabilities;
      std::vector<btav_a2dp_codec_config_t> codecs_selectable_capabilities;
      codec_config.codec_type = BTAV_A2DP_CODEC_INDEX_SOURCE_MAX;
      HAL_CBACK(bt_av_src_callbacks, audio_config_cb,
              (btif_av_cb[index].peer_bda), codec_config,
              codecs_local_capabilities, codecs_selectable_capabilities);
    }
  }
}

/*****************************************************************************
 *
 * Function     btif_av_state_opened_handler
 *
 * Description  Handles AV events while AVDTP is in OPEN state
 *
 * Returns      true if event was processed, false otherwise
 *
 ******************************************************************************/

static bool btif_av_state_opened_handler(btif_sm_event_t event, void* p_data,
                                         int index) {
  RawAddress * bt_addr = NULL;
  tBTA_AV* p_av = (tBTA_AV*)p_data;

  BTIF_TRACE_IMP("%s: event:%s flags %x peer_sep %x and index %x reconfig_event: %d",
     __func__, dump_av_sm_event_name((btif_av_sm_event_t)event),
     btif_av_cb[index].flags, btif_av_cb[index].peer_sep, index, btif_av_cb[index].reconfig_event);
  if (event == BTA_AV_RC_OPEN_EVT) {
    BTIF_TRACE_DEBUG("%s: Remote_add: %s", __func__,
        ((tBTA_AV*)p_data)->rc_open.peer_addr.ToString().c_str());
  }

  if ((event == BTA_AV_REMOTE_CMD_EVT) &&
      (p_av->remote_cmd.rc_id == BTA_AV_RC_PLAY)) {
    if (btif_av_check_flag_remote_suspend(index)) {
      BTIF_TRACE_EVENT("%s: Resetting remote suspend flag on RC PLAY", __func__);
      btif_av_clear_remote_suspend_flag();
      if (bluetooth::headset::btif_hf_is_call_vr_idle())
      {
        RawAddress addr = btif_rc_get_connected_peer_address(p_av->remote_cmd.rc_handle);
        if (!addr.IsEmpty() && btif_av_is_current_device(addr)) {
          BTIF_TRACE_EVENT("%s: No active call, start stream for active device ", __func__);
          btif_dispatch_sm_event(BTIF_AV_START_STREAM_REQ_EVT, NULL, 0);
        }
      }
    }
  }

  switch (event) {
    case BTIF_SM_ENTER_EVT: {
      btif_av_cb[index].flags &= ~BTIF_AV_FLAG_PENDING_STOP;
      if (btif_av_cb[index].reconfig_pending &&
        (btif_av_cb[index].flags &= BTIF_AV_FLAG_PENDING_START) != 0 &&
        !reconfig_a2dp) {
        // if codec switch did not happen and reconfig is called when the index
        // was in streaming state, keep pending start to trigger play after reset
        // if reconfig_a2dp is set, audio hal will suspend and start, no need to
        // retain pending start flag
        APPL_TRACE_IMP("%s: Pending start set for reconfig, do not reset",__func__);
      } else {
        btif_av_cb[index].flags &= ~BTIF_AV_FLAG_PENDING_START;
      }
      if (btif_av_cb[index].reconfig_event) {
        btif_av_process_cached_src_codec_config(index);
      }
#if (TWS_ENABLED == TRUE)
      if (btif_av_cb[index].tws_device) {
        APPL_TRACE_DEBUG("TWS+ device enter opened state");
        for(int i = 0; i < btif_max_av_clients; i++) {
          if (i != index && btif_av_cb[i].tws_device &&
              (btif_av_cb[i].flags & BTIF_AV_FLAG_PENDING_START) &&
              !(btif_av_cb[i].flags & BTIF_AV_FLAG_LOCAL_SUSPEND_PENDING)) {
            APPL_TRACE_DEBUG("The other TWS+ is pending start, sending bta av start, flags %x",
                              btif_av_cb[i].flags);
            btif_av_cb[index].flags |= BTIF_AV_FLAG_PENDING_START;
            BTA_AvStart(btif_av_cb[index].bta_handle);
          }
        }
      }
#endif
    } break;

    case BTIF_SM_EXIT_EVT: {
      btif_av_cb[index].flags &= ~BTIF_AV_FLAG_PENDING_START;
    } break;

    case BTIF_AV_START_STREAM_REQ_EVT: {
      /* update multicast state here if new device is connected
       * after A2dp connection. New A2dp device is connected
       * whlie playing */
      is_codec_config_dump = true;
      btif_av_update_multicast_state(index);
#if (TWS_ENABLED == TRUE)
#if (TWS_STATE_ENABLED == TRUE)
      if (btif_av_current_device_is_tws() && tws_state_supported &&
         !btif_av_is_tws_ready_to_play()) {
        BTIF_TRACE_ERROR("%s:TWSP device not ready to play, may be out of ear",__func__);
        tBTA_AV_START start;
        start.status = BTA_AV_FAIL;
        start.initiator = true;
        start.suspending = false;
        start.role = 0xFF;
        btif_a2dp_on_started(&start, false, btif_av_cb[index].bta_handle);
        if (btif_av_is_under_handoff()) {
          btif_av_reset_reconfig_flag();
        }
        break;
      }
#endif
#endif
      if (btif_av_cb[index].peer_sep == AVDT_TSEP_SRC) {
        BTA_AvStart(btif_av_cb[index].bta_handle);
        btif_av_cb[index].flags |= BTIF_AV_FLAG_PENDING_START;
      } else {
        bt_status_t status = BT_STATUS_FAIL;
        if (btif_a2dp_source_is_hal_v2_supported()) {
            status = BT_STATUS_SUCCESS;
        } else {
            status = btif_a2dp_source_setup_codec(btif_av_cb[index].bta_handle);
        }
        if (status == BT_STATUS_SUCCESS) {
          int idx = 0;
          BTA_AvStart(btif_av_cb[index].bta_handle);
          ba_send_message(BTIF_BA_BT_A2DP_STARTING_EVT, 0, NULL, true);
          if (enable_multicast == true
#if (TWS_ENABLED == TRUE)
             || btif_av_cb[index].tws_device
#endif
          ) {
            /* In A2dp Multicast, DUT initiated stream request
             * should be true for all connected A2dp devices
             */
            for (; idx < btif_max_av_clients; idx++)
#if (TWS_ENABLED == TRUE)
              if (btif_av_cb[idx].tws_device) {
                if (!tws_state_supported ||
                  (tws_state_supported &&
                  btif_av_cb[idx].eb_state == TWSP_STATE_IN_EAR))
                btif_av_cb[idx].flags |= BTIF_AV_FLAG_PENDING_START;
              } else if (enable_multicast == true) {
                btif_av_cb[idx].flags |= BTIF_AV_FLAG_PENDING_START;
              }
#else
              btif_av_cb[idx].flags |= BTIF_AV_FLAG_PENDING_START;
#endif
          } else
              btif_av_cb[index].flags |= BTIF_AV_FLAG_PENDING_START;
        } else {
          BTIF_TRACE_ERROR("## AV Disconnect## status : %x",status);
          BTA_AvDisconnect(btif_av_cb[index].peer_bda);
        }
      }
    } break;

    case BTA_AV_START_EVT: {
      BTIF_TRACE_DEBUG("%s: BTA_AV_START_EVT status: %d, suspending: %d, init: %d,"
            " role: %d", __func__, p_av->start.status, p_av->start.suspending,
            p_av->start.initiator, p_av->start.role);
      if (p_av->start.role == HOST_ROLE_SLAVE) {
        btif_av_cb[index].is_slave = true;
      } else {
        // update if we are master after role switch before start
        btif_av_cb[index].is_slave = false;
      }
      /* There can be role switch after device is connected,
       * hence check for role before starting multicast, and
       * disable if we are in slave role for any connection
       */
      btif_av_update_multicast_state(index);

      if ((p_av->start.status == BTA_SUCCESS) && (p_av->start.suspending == true))
        return true;

      /* if remote tries to start a2dp when call is in progress, suspend it right away */
      if ((!(btif_av_cb[index].flags & BTIF_AV_FLAG_PENDING_START)) &&
            (!bluetooth::headset::btif_hf_is_call_vr_idle())) {
        BTIF_TRACE_EVENT("%s: trigger suspend as call is in progress!!", __func__);
        btif_av_cb[index].flags &= ~BTIF_AV_FLAG_PENDING_START;
        btif_av_cb[index].remote_started = true;
        btif_sm_change_state(btif_av_cb[index].sm_handle, BTIF_AV_STATE_STARTED);
        btif_dispatch_sm_event(BTIF_AV_REMOTE_SUSPEND_STREAM_REQ_EVT, &index, sizeof(index));
        break;
      }

      if ((!btif_av_cb[index].current_playing) &&
          (btif_av_cb[index].flags & BTIF_AV_FLAG_PENDING_START)) {
#if (TWS_ENABLED == TRUE)
        bool active_tws = false;
        if(btif_av_cb[index].tws_device) {
          for (int idx = 0; idx < btif_max_av_clients; idx++) {
            if(btif_av_cb[idx].tws_device && idx != index
                 && btif_av_cb[idx].current_playing) {
              active_tws = true;
              break;
            }
          }
        }
        if(!active_tws && !enable_multicast)
#endif
        {
          BTIF_TRACE_EVENT("%s: Start event received for in-active device", __func__);
          btif_av_cb[index].flags &= ~BTIF_AV_FLAG_PENDING_START;
          btif_av_cb[index].flags |= BTIF_AV_FLAG_LOCAL_SUSPEND_PENDING;
          BTA_AvStop(true, btif_av_cb[index].bta_handle);
          break;
        }
      }

      /* if remote tries to start a2dp when DUT is a2dp source
       * then suspend. In case a2dp is sink and call is active
       * then disconnect the AVDTP channel
       */
      if (!(btif_av_cb[index].flags & BTIF_AV_FLAG_PENDING_START)) {
        if (btif_av_cb[index].peer_sep == AVDT_TSEP_SNK) {
          if (enable_multicast
#if (TWS_ENABLED == TRUE)
              || ((btif_av_cb[index].tws_device == true)
              &&  ((btif_av_is_tws_device_playing(index) == true) &&
                    btif_av_is_tws_pair_remote_started(index) == false) )
#endif
          ) {
            /* Stack will start the playback on newly connected
             * A2dp device, if the playback is already happening on
             * other connected device.*/
            if (btif_av_is_playing()
#if (TWS_ENABLED == TRUE)
                && (btif_av_cb[index].tws_device == false)
#endif
               )
              /* when HS2 is connected during HS1 playing, stack directly
               * sends start event hence update encoder so that least L2CAP
               *  MTU is selected.*/
              BTIF_TRACE_DEBUG("%s: A2dp Multicast playback",
                            __func__);
            /* initiate suspend if start is initiate by remote and multicast
             * is enabled.
             * Avoid suspend if stream is started as quick suspend-start
             * creates IOT issue, seen with SBH50.
             */
            if (!p_av->start.initiator && !btif_av_is_playing()) {
              BTIF_TRACE_DEBUG("%s: initiate suspend for remote start", __func__);
              btif_dispatch_sm_event(BTIF_AV_SUSPEND_STREAM_REQ_EVT, NULL, 0);
            }
          } else {
            if ((btif_av_cb[index].flags & BTIF_AV_FLAG_REMOTE_SUSPEND)) {
                BTIF_TRACE_DEBUG("%s: clear remote suspend flag on remote start",
                __func__);
                btif_av_cb[index].flags &= ~BTIF_AV_FLAG_REMOTE_SUSPEND;
            } else if (p_av->start.status == BTA_AV_SUCCESS) {
              BTIF_TRACE_DEBUG("%s: honor remote start on index %d",__func__, index);
              /*avoiding same index timer running*/
              if(!btif_av_cb[index].remote_started) {
                BTIF_TRACE_DEBUG("%s: honor remote start on index %d",__func__, index);
                btif_av_cb[index].remote_started = true;
                btif_a2dp_honor_remote_start(&btif_av_cb[index].remote_start_alarm, index);
              }
            }
          }
        }
      }
#if (TWS_ENABLED == TRUE)
      if (btif_av_cb[index].tws_device &&
        (btif_av_cb[index].flags & BTIF_AV_FLAG_LOCAL_SUSPEND_PENDING)) {
        /* Susupend initiated after start to earbuds but in some scenario
         * if one of the earbuds took more time to respond to start req or
         * earbud sent suspend for dut initiated start and then sent start again
         * We have to suspend stream
         */
        BTIF_TRACE_DEBUG("%s:Suspending pending for TWS pair",__func__);
        btif_dispatch_sm_event(BTIF_AV_SUSPEND_STREAM_REQ_EVT, NULL, 0);
      }
#endif
      /* remain in open state if status failed */
      /* Multicast-soft Handoff:
       * START failed, cleanup Handoff flag.
       */
      if (p_av->start.status != BTA_AV_SUCCESS)
      {
        int i;

        /* In case peer is A2DP SRC we do not want to ack commands on UIPC */
        if (btif_av_cb[index].peer_sep == AVDT_TSEP_SNK)
          if (btif_a2dp_on_started(&p_av->start,
              ((btif_av_cb[index].flags & BTIF_AV_FLAG_PENDING_START) != 0),
                btif_av_cb[index].bta_handle))
                  /* only clear pending flag after acknowledgement */
                  btif_av_cb[index].flags &= ~BTIF_AV_FLAG_PENDING_START;
          /* Clear dual handoff flag */
          for(i = 0; i < btif_max_av_clients; i++)
              btif_av_cb[i].dual_handoff = false;

          return false;
      }

#ifndef AVK_BACKPORT
      if (btif_av_cb[index].peer_sep == AVDT_TSEP_SRC)
          btif_a2dp_sink_set_rx_flush(false); /*  remove flush state, ready for streaming*/
#endif
      btif_sm_change_state(btif_av_cb[index].sm_handle, BTIF_AV_STATE_STARTED);
    } break;

    case BTIF_AV_SOURCE_CONFIG_REQ_EVT: {
      if ((btif_av_cb[index].flags & BTIF_AV_FLAG_PENDING_START) && codec_cfg_change) {
        btif_av_cache_src_codec_config(BTIF_AV_SOURCE_CONFIG_REQ_EVT, p_data, index);
      } else {
        btif_av_cb[index].reconfig_event = 0;
        memset(&btif_av_cb[index].reconfig_data, 0, sizeof(tBTA_AV));
        btif_update_source_codec(p_data);
      }
    } break;

    case BTIF_AV_SOURCE_CONFIG_UPDATED_EVT: {
      if (p_data != NULL) {
          bt_addr = (RawAddress *)p_data;
      } else {
          BTIF_TRACE_DEBUG("%s: p_data not null", __func__);
          bt_addr = &btif_av_cb[index].peer_bda;
      }

      btif_report_source_codec_state(p_data, bt_addr);
      btif_av_update_reconfigure_event(index);
    } break;

    case BTIF_AV_DISCONNECT_REQ_EVT: {
      BTA_AvClose(btif_av_cb[index].bta_handle);
      if (btif_av_cb[index].peer_sep == AVDT_TSEP_SRC) {
        BTA_AvCloseRc(btif_av_cb[index].bta_handle);
      }

/* SPLITA2DP */
      if (!btif_av_is_connected_on_other_idx(index)) {
        if (btif_av_is_split_a2dp_enabled() &&
           !btif_a2dp_source_is_hal_v2_supported()) {
          if (!isBATEnabled()) {
            btif_a2dp_audio_if_init = false;
           btif_a2dp_audio_interface_deinit();
          }
        }
      }
/* SPLITA2DP */
      /* change state to idle, send acknowledgement if start is pending */
      if (btif_av_cb[index].flags & BTIF_AV_FLAG_PENDING_START) {
        btif_a2dp_command_ack(A2DP_CTRL_ACK_FAILURE);
        btif_av_cb[index].flags &= ~BTIF_AV_FLAG_PENDING_START;
      }
      btif_sm_change_state(btif_av_cb[index].sm_handle, BTIF_AV_STATE_CLOSING);
      /* inform the application that we are disconnecting */
      btif_report_connection_state(BTAV_CONNECTION_STATE_DISCONNECTING,
                                   &(btif_av_cb[index].peer_bda));
    } break;

    case BTA_AV_CLOSE_EVT: {
      /* avdtp link is closed
       * Don't close the A2dp when Dual playback is happening
       */
      if (btif_av_is_connected_on_other_idx(index)) {
        APPL_TRACE_WARNING("%s: Conn is closing,close AV data channel", __func__);
        if ((!btif_av_is_split_a2dp_enabled() ||
             (btif_av_is_split_a2dp_enabled() && !reconfig_a2dp)) &&
            ((btif_av_cb[index].flags & BTIF_AV_FLAG_REMOTE_SUSPEND) != 0) &&
            ((btif_av_cb[index].flags & BTIF_AV_FLAG_LOCAL_SUSPEND_PENDING) == 0)) {
          /* fake handoff state to switch streaming to other codec device */
          btif_av_cb[index].dual_handoff = true;
        } else if ((!btif_av_is_playing() || is_multicast_supported)
                  && btif_av_cb[index].current_playing) {
          APPL_TRACE_WARNING("%s: Suspend the AV Data channel for active device", __func__);
          /* ensure tx frames are immediately suspended */
          btif_a2dp_source_set_tx_flush(true);
          btif_a2dp_source_stop_audio_req();
          if (is_multicast_supported && btif_av_is_playing()) {
            APPL_TRACE_WARNING("%s:Mcast streaming,suspend other playing device",__func__);
            btif_dispatch_sm_event(BTIF_AV_SUSPEND_STREAM_REQ_EVT, NULL, 0);
          }
        }
      } else {
        APPL_TRACE_WARNING("%s: Stop the AV Data channel", __func__);
/* SPLITA2DP */
        if (btif_av_is_split_a2dp_enabled() &&
           !btif_a2dp_source_is_hal_v2_supported()) {
          if (btif_a2dp_audio_if_init) {
            if (!isBATEnabled()) {
              btif_a2dp_audio_if_init = false;
              btif_a2dp_audio_interface_deinit();
            }
          }
        }
/* SPLITA2DP */
        if (is_multicast_supported && !btif_av_cb[index].current_playing) {
          APPL_TRACE_WARNING("%s:Non active device disconnected,continue streaming",__func__);
        } else {
          btif_a2dp_on_stopped(NULL);
        }
      }

      /* inform the application that we are disconnected */
      btif_av_cb[index].flags |= BTIF_AV_FLAG_PENDING_DISCONNECT;
      btif_report_connection_state(BTAV_CONNECTION_STATE_DISCONNECTED,
                                        &(btif_av_cb[index].peer_bda));
      btif_av_cb[index].flags &= ~BTIF_AV_FLAG_PENDING_DISCONNECT;

      /* change state to idle, send acknowledgement if start is pending */
      if (btif_av_cb[index].flags & BTIF_AV_FLAG_PENDING_START) {
        btif_a2dp_command_ack(A2DP_CTRL_ACK_FAILURE);
        btif_av_cb[index].flags &= ~BTIF_AV_FLAG_PENDING_START;
      }

      btif_sm_change_state(btif_av_cb[index].sm_handle, BTIF_AV_STATE_IDLE);
      btif_report_connection_state_to_ba(BTAV_CONNECTION_STATE_DISCONNECTED);
    } break;

    case BTA_AV_RECONFIG_EVT: {
      if ((btif_av_cb[index].flags & BTIF_AV_FLAG_PENDING_START) &&
          (p_av->reconfig.status == BTA_AV_SUCCESS)) {
        APPL_TRACE_WARNING("reconfig done BTA_AVstart()");
        if (btif_a2dp_source_is_restart_session_needed()) {
          btif_report_source_codec_state(p_data, &btif_av_cb[index].peer_bda);
        } else {
          BTA_AvStart(btif_av_cb[index].bta_handle);
          ba_send_message(BTIF_BA_BT_A2DP_STARTING_EVT, 0, NULL, true);
        }
      } else if (btif_av_cb[index].flags & BTIF_AV_FLAG_PENDING_START) {
        btif_av_cb[index].flags &= ~BTIF_AV_FLAG_PENDING_START;
        btif_a2dp_command_ack(A2DP_CTRL_ACK_FAILURE);
      }
      btif_av_cb[index].reconfig_pending = false;
      bluetooth::audio::a2dp::update_session_params(SessionParamType::MTU);
    } break;

    case BTIF_AV_CONNECT_REQ_EVT: {
      if (memcmp(((btif_av_connect_req_t*)p_data)->target_bda, &(btif_av_cb[index].peer_bda),
                 sizeof(btif_av_cb[index].peer_bda)) == 0) {
        BTIF_TRACE_DEBUG("%s: Ignore BTIF_AV_CONNECT_REQ_EVT for same device",
                         __func__);
      } else {
          btif_av_connect_req_t* connect_req_p = (btif_av_connect_req_t*)p_data;
          RawAddress& target_bda = *connect_req_p->target_bda;
        BTIF_TRACE_WARNING(
            "%s: Moved to opened by Other incoming Connect request: "
            "target_bda=%s",
            __func__, target_bda.ToString().c_str());
        btif_report_connection_state(BTAV_CONNECTION_STATE_DISCONNECTED,
                                     &target_bda);
      }
      btif_av_connect_req_t* connect_req_t = (btif_av_connect_req_t*)p_data;
      if (bt_av_sink_callbacks != NULL)
          connect_req_t->uuid = UUID_SERVCLASS_AUDIO_SINK;
      else if (bt_av_src_callbacks != NULL)
          connect_req_t->uuid = UUID_SERVCLASS_AUDIO_SOURCE;
      btif_queue_advance_by_uuid(connect_req_t->uuid, &(btif_av_cb[index].peer_bda));
      btif_report_connection_state_to_ba(BTAV_CONNECTION_STATE_DISCONNECTED);
    } break;

    case BTIF_AV_OFFLOAD_START_REQ_EVT: {
      BTIF_TRACE_ERROR(
          "%s: BTIF_AV_OFFLOAD_START_REQ_EVT: Stream not Started Opened",
          __func__);
      if (btif_av_cb[index].flags & BTIF_AV_FLAG_REMOTE_SUSPEND) {
        APPL_TRACE_WARNING("%s: Ack success to MM although VSCs of START exchange didn't start : %d",
                               __func__, btif_av_cb[index].flags);
        btif_a2dp_on_offload_started(BTA_AV_FAIL_UNSUPPORTED);
        break;
      }
      btif_a2dp_on_offload_started(BTA_AV_FAIL);
    } break;

    case BTA_AV_OFFLOAD_START_RSP_EVT:
      tBTA_AV_STATUS status;

      if (!btif_a2dp_src_vsc.multi_vsc_support)
        status = p_av->offload_rsp.status;
      else
        status = p_av->status;
      APPL_TRACE_WARNING("Offload Start Rsp is unsupported in opened state, status = %d", status);
      if (btif_av_cb[index].flags & BTIF_AV_FLAG_REMOTE_SUSPEND) {
        if (status == BTA_AV_SUCCESS) {
          btif_a2dp_src_vsc.tx_started = TRUE;
          bta_av_vendor_offload_stop(NULL);
        }
        APPL_TRACE_WARNING("%s: Ack success to MM although VSCs of STRAT Failed: %d",
                               __func__, btif_av_cb[index].flags);
        btif_a2dp_on_offload_started(BTA_AV_FAIL_UNSUPPORTED);
      }
      break;

    case BTA_AV_RC_OPEN_EVT: {
      btif_av_check_rc_connection_priority(p_data);
    }  break;

    case BTIF_AV_SETUP_CODEC_REQ_EVT:{
      uint8_t hdl = btif_av_get_av_hdl_from_idx(index);
      APPL_TRACE_DEBUG("%s: hdl = %d, btif_a2dp_source_is_hal_v2_supported() %d",
            __func__, hdl, btif_a2dp_source_is_hal_v2_supported());
      if (hdl >= 0 && !btif_a2dp_source_is_hal_v2_supported())
        btif_a2dp_source_setup_codec(hdl);
      }
      break;

    case BTA_AV_DELAY_REPORT_EVT:
      /* Initial delay report after avdtp stream configuration */
      BTIF_TRACE_DEBUG("%s : BTA_AV_DELAY_REPORT_EVT received", __func__);
      if (btif_av_cb[index].peer_sep == AVDT_TSEP_SNK) {
        btif_av_cb[index].sink_latency = ((tBTA_AV*)p_data)->delay_rpt.sink_delay;
      }
      break;

    case BTA_AV_SUSPEND_EVT:
      if (btif_av_cb[index].tws_device &&
        (btif_av_cb[index].flags & BTIF_AV_FLAG_PENDING_START)) {
        //Dont clear suspend pending flag, remote will be suspended after start complete
        BTIF_TRACE_DEBUG("%s: BTA_AV_SUSPEND_EVT for TWS+ remote when pending start",__func__);
      } else {
        btif_av_cb[index].flags &= ~BTIF_AV_FLAG_LOCAL_SUSPEND_PENDING;
        btif_av_cb[index].fake_suspend_rsp = false;
        BTIF_TRACE_DEBUG("%s: BTA_AV_SUSPEND_EVT received in opened state for index: %d, ignore.",
                            __func__, index);
      }
      break;

      CHECK_RC_EVENT(event, (tBTA_AV*)p_data);

    default: {
      BTIF_TRACE_WARNING("%s : unhandled event:%s", __func__,
                          dump_av_sm_event_name((btif_av_sm_event_t)event));
      return false;
    }
  }
  return true;
}

/*****************************************************************************
 *
 * Function     btif_av_state_started_handler
 *
 * Description  Handles AV events while A2DP stream is started
 *
 * Returns      true if event was processed, false otherwise
 *
 ******************************************************************************/

static bool btif_av_state_started_handler(btif_sm_event_t event, void* p_data,
                                          int index) {
  tBTA_AV* p_av = (tBTA_AV*)p_data;
  RawAddress *bt_addr = nullptr;

  btif_sm_state_t state = BTIF_AV_STATE_IDLE;
  int i;
  bool hal_suspend_pending = false;
  tA2DP_CTRL_CMD pending_cmd = A2DP_CTRL_CMD_NONE;
  if (!btif_a2dp_source_is_hal_v2_supported()) {
    pending_cmd = btif_a2dp_control_get_pending_command();
  }
  bool remote_start_cancelled = false;
  BTIF_TRACE_IMP("%s event:%s flags %x  index =%d, reconfig_event: %d", __func__,
                   dump_av_sm_event_name((btif_av_sm_event_t)event),
                   btif_av_cb[index].flags, index, btif_av_cb[index].reconfig_event);

  switch (event) {
    case BTIF_SM_ENTER_EVT:
      /* Ack from entry point of started handler instead of open state to avoid
       * race condition
       */
      if (btif_av_cb[index].peer_sep == AVDT_TSEP_SNK) {
        if (!((!enable_multicast)&&((btif_av_cb[index].remote_started)
            && ((btif_av_cb[index].flags & BTIF_AV_FLAG_PENDING_START) == 0)))) {
          BTIF_TRACE_IMP("%s update media task on DUT initiated start for index =%d", __func__, index);
          if (btif_a2dp_on_started(&p_av->start,
              ((btif_av_cb[index].flags & BTIF_AV_FLAG_PENDING_START) != 0),
              btif_av_cb[index].bta_handle)) {
            /* only clear pending flag after acknowledgement */
            btif_av_cb[index].flags &= ~BTIF_AV_FLAG_PENDING_START;
          }
        } else {
          BTIF_TRACE_IMP("%s Do not update media task on remote start for index =%d", __func__, index);
        }
      }

      /* Already changed state to started, send acknowledgement
       * if start is pending
       */
      if (btif_av_cb[index].flags & BTIF_AV_FLAG_PENDING_START) {
          if (btif_av_cb[index].peer_sep == AVDT_TSEP_SNK)
              btif_a2dp_on_started(NULL, true, btif_av_cb[index].bta_handle);
          btif_av_cb[index].flags &= ~BTIF_AV_FLAG_PENDING_START;
      }

      /* we are again in started state, clear any remote suspend flags */
      btif_av_cb[index].flags &= ~BTIF_AV_FLAG_REMOTE_SUSPEND;

      if ((!enable_multicast)&&((btif_av_cb[index].remote_started)
          && (btif_av_is_playing_on_other_idx(index)))) {
        BTIF_TRACE_IMP("%s Do not update audio state change to app for index =%d", __func__, index);
      } else {
        /**
            * Report to components above that we have entered the streaming
            * stage, this should usually be followed by focus grant.
            * see update_audio_focus_state()
            */
        if (btif_av_cb[index].peer_sep == AVDT_TSEP_SRC) {
          /** if DUT is in idle then only update the AVDTP_START to App
            * else don't update to app any kind of touchtone, dialpadtones etc
            * which will not trigger actual playback
            */
          bool is_playing = false;
          for (int i = 0; i < btif_max_av_clients; i++) {
            if (btif_av_cb[i].current_playing) {
              is_playing = true;
              BTIF_TRACE_DEBUG("%s: current playing index: %d, is_playing: %d",
                           __func__, i, is_playing);
              break;
            }
          }
          BTIF_TRACE_DEBUG("%s: is_playing: %d", __func__, is_playing);
          if (!is_playing) {
            BTIF_TRACE_DEBUG("%s: start streaming when both are in opened state", __func__);
            btif_initiate_sink_handoff(index, true);
            btif_report_audio_state(BTAV_AUDIO_STATE_STARTED, &(btif_av_cb[index].peer_bda));
          }
        } else {
          if (btif_av_cb[index].tws_device) {
            BTIF_TRACE_DEBUG("%s:tws device, notify audio state after offload", __func__);
          } else {
            btif_report_audio_state(BTAV_AUDIO_STATE_STARTED, &(btif_av_cb[index].peer_bda));
          }
        }
      }
      btif_av_cb[index].is_device_playing = true;
      btif_report_audio_state_to_ba(BTAV_AUDIO_STATE_STARTED);

#ifdef AVK_BACKPORT
      if (btif_av_cb[index].peer_sep == AVDT_TSEP_SRC)
        btif_av_request_audio_focus(true);
#endif
      // Clear Dual Handoff for all SCBs for DUT initiated Start.
      //If split a2dp is enabled, clear dual handoff flag in offload resp evt.
      if ((!btif_av_is_split_a2dp_enabled()) &&
                    (!(btif_av_cb[index].remote_started))) {
        for(i = 0; i < btif_max_av_clients; i++) {
          btif_av_cb[i].dual_handoff = false;
        }
        BTIF_TRACE_IMP("%s Setting device as current playing for index = %d",
                                        __func__, index);
      } else {
          BTIF_TRACE_IMP("%s Remote Start, Not updating current playing for index = %d",
                                          __func__, index);
      }
#if (TWS_ENABLED == TRUE)
     if (btif_av_current_device_is_tws() &&
       btif_av_cb[index].tws_device) {
       btif_av_cb[index].current_playing = true;
     }
#endif
     if (!btif_av_is_split_a2dp_enabled() && btif_av_cb[index].reconfig_event) {
       btif_av_process_cached_src_codec_config(index);
     }
     break;

    case BTIF_SM_EXIT_EVT:
      break;

    case BTIF_AV_START_STREAM_REQ_EVT:
      /* we were remotely started, just ack back the local request */
      if (btif_av_cb[index].remote_started) btif_av_cb[index].remote_started = false;
      if (btif_av_cb[index].peer_sep == AVDT_TSEP_SNK) {
        uint8_t hdl = btif_av_get_av_hdl_from_idx(index);
        if (hdl >= 0) {
          bt_status_t status = BT_STATUS_FAIL;
          if (btif_a2dp_source_is_hal_v2_supported()) {
            status = BT_STATUS_SUCCESS;
          } else {
            status = btif_a2dp_source_setup_codec(hdl);
          }
          BTIF_TRACE_DEBUG("%s: hdl = %u, status : %x, enc_update_in_progress = %d",
                                __func__, hdl, status, enc_update_in_progress);

          if (status == BT_STATUS_SUCCESS) {
            enc_update_in_progress = TRUE;
            btif_a2dp_on_started(NULL, true, btif_av_cb[index].bta_handle);
            btif_report_audio_state(BTAV_AUDIO_STATE_STARTED, &(btif_av_cb[index].peer_bda));
            btif_av_cb[index].is_device_playing = true;
          } else {
            BTIF_TRACE_ERROR("## AV Disconnect## status : %x",status);
            BTA_AvDisconnect(btif_av_cb[index].peer_bda.address);
          }
          btif_av_reset_reconfig_flag();
        }
      }
      break;

    case BTIF_AV_SOURCE_CONFIG_REQ_EVT:
      if(codec_cfg_change) {
        btif_av_cb[index].reconfig_pending = true;
      }
      btif_av_flow_spec_cmd(index, reconfig_a2dp_param_val);
      if ((btif_av_cb[index].flags & BTIF_AV_FLAG_LOCAL_SUSPEND_PENDING) && codec_cfg_change) {
        btif_av_cache_src_codec_config(BTIF_AV_SOURCE_CONFIG_REQ_EVT, p_data, index);
      } else {
        btif_av_cb[index].reconfig_event = 0;
        memset(&btif_av_cb[index].reconfig_data, 0, sizeof(tBTA_AV));
        btif_update_source_codec(p_data);
      }
      break;

    case BTIF_AV_SOURCE_CONFIG_UPDATED_EVT:
      if (p_data != NULL) {
          bt_addr = (RawAddress *)p_data;
      } else {
          BTIF_TRACE_DEBUG("%s: p_data not null", __func__);
          bt_addr = &btif_av_cb[index].peer_bda;
      }

      btif_report_source_codec_state(p_data, bt_addr);
      btif_av_update_reconfigure_event(index);
      break;

    /* fixme -- use suspend = true always to work around issue with BTA AV */
    case BTIF_AV_STOP_STREAM_REQ_EVT:
    case BTIF_AV_SUSPEND_STREAM_REQ_EVT:
      /* set pending flag to ensure btif task is not trying to restart
       * stream while suspend is in progress.
       * Multicast: If streaming is happening on both devices, we need
       * to update flag for both connections as SUSPEND request will
       * be sent to only one stream as internally BTA takes care of
       * suspending both streams.
       */
      if (enable_multicast
#if (TWS_ENABLED == TRUE)
          || (btif_av_cb[index].tws_device
          && ((btif_av_cb[index].flags & BTIF_AV_FLAG_LOCAL_SUSPEND_PENDING) == 0))
#endif
         ) {
        //TODO check for tws pair
        for(i = 0; i < btif_max_av_clients; i++) {
          state = btif_sm_get_state(btif_av_cb[i].sm_handle);
#if (TWS_ENABLED == TRUE)
          if ((btif_av_cb[index].tws_device && i != index &&
              btif_av_cb[i].tws_device &&
              (btif_av_cb[i].flags & BTIF_AV_FLAG_PENDING_START ||
              state == BTIF_AV_STATE_STARTED)) || (i == index)) {
            btif_av_cb[i].flags |= BTIF_AV_FLAG_LOCAL_SUSPEND_PENDING;
          } else if (enable_multicast && state == BTIF_AV_STATE_STARTED) {
#else
          if (state == BTIF_AV_STATE_STARTED) {
#endif
            btif_av_cb[i].flags |= BTIF_AV_FLAG_LOCAL_SUSPEND_PENDING;
          }
        }
      } else {
        btif_av_cb[index].flags |= BTIF_AV_FLAG_LOCAL_SUSPEND_PENDING;
      }

      /* if we were remotely suspended but suspend locally, local suspend
       * always overrides
       */
      btif_av_cb[index].flags &= ~BTIF_AV_FLAG_REMOTE_SUSPEND;

      if (btif_av_cb[index].peer_sep == AVDT_TSEP_SNK)
      {
          if (btif_av_is_connected_on_other_idx(index)) {
            APPL_TRACE_WARNING("%s: Suspend the AV Data channel, remote_started: %d",
                            __func__, btif_av_cb[index].remote_started);
            //Flush and close media channel if this index is DUT started and streaming
            if (!btif_av_cb[index].remote_started) {
              APPL_TRACE_WARNING("%s: index is DUT started, flush the stream", __func__);
              btif_a2dp_source_set_tx_flush(true);
            } else if (btif_av_is_playing_on_other_idx(index)) {
              //index is remote started, other index streaming
              APPL_TRACE_WARNING("%s: Not flushing as one link is already streaming", __func__);
            }
          } else {
            /* immediately flush any pending tx frames while suspend is pending */
            APPL_TRACE_WARNING("%s: Stop the AV Data channel", __func__);
            btif_a2dp_source_set_tx_flush(true);
          }
      }

      if (btif_av_cb[index].remote_started) {
        if (btif_a2dp_source_is_remote_start()) {
          BTIF_TRACE_DEBUG("%s:cancel remote start timer",__func__);
          if (btif_a2dp_source_last_remote_start_index() == index)
            btif_a2dp_source_cancel_remote_start();
        }
        btif_av_cb[index].remote_started = false;
      }

      if (btif_av_cb[index].peer_sep == AVDT_TSEP_SRC) {
        btif_a2dp_sink_set_rx_flush(true);
        btif_a2dp_on_stopped(NULL);
      }

      BTA_AvStop(true, btif_av_cb[index].bta_handle);
      break;

    case BTIF_AV_DISCONNECT_REQ_EVT:
      // Now it is not the current playing
      // request avdtp to close
      BTA_AvClose(btif_av_cb[index].bta_handle);
      if (btif_av_cb[index].peer_sep == AVDT_TSEP_SRC)
          BTA_AvCloseRc(btif_av_cb[index].bta_handle);

      if (btif_av_is_split_a2dp_enabled() &&
        btif_av_is_connected_on_other_idx(index)) {
        /*Fake handoff state to switch streaming to other coddeced
          device */
        //TODO change will be removed after official patch
        //btif_av_cb[index].dual_handoff = true;
      } else {
        if (btif_a2dp_audio_if_init && !btif_a2dp_source_is_hal_v2_supported()) {
          if (!isBATEnabled()) {
            btif_a2dp_audio_if_init = false;
            btif_a2dp_audio_interface_deinit();
          }
        }
      }
#if (TWS_ENABLED == TRUE)
      if (btif_av_cb[index].dual_handoff &&
        btif_av_cb[index].tws_device &&
        btif_av_is_tws_device_playing(index)) {
        BTIF_TRACE_DEBUG("%s:Playing on TWSP pair, do not reconfig",__func__);
        btif_av_cb[index].dual_handoff = false;
      }
#endif
      // inform the application that we are disconnecting
      btif_report_connection_state(BTAV_CONNECTION_STATE_DISCONNECTING, &(btif_av_cb[index].peer_bda));

      // wait in closing state until fully closed
      btif_sm_change_state(btif_av_cb[index].sm_handle, BTIF_AV_STATE_CLOSING);
      break;

    case BTA_AV_SUSPEND_EVT:
      if (btif_a2dp_source_is_hal_v2_supported()) {
        pending_cmd =  bluetooth::audio::a2dp::get_pending_command();
      }
      BTIF_TRACE_EVENT("%s: BTA_AV_SUSPEND_EVT: index: %d, status %d, init %d, flag %d,"
          "pending_cmd: %d, dual_handoff: %d,  fake_suspend_rsp: %d", __func__, index,
       p_av->suspend.status, p_av->suspend.initiator, btif_av_cb[index].flags, pending_cmd,
       btif_av_cb[index].dual_handoff, btif_av_cb[index].fake_suspend_rsp);
#if (TWS_ENABLED == TRUE && TWS_STATE_ENABLED == TRUE)
      if (tws_state_supported && btif_av_cb[index].tws_device &&
        btif_av_cb[index].tws_state_suspend) {
        BTIF_TRACE_EVENT("%s:Suspending out of ear EB",__func__);
        btif_av_cb[index].flags |= BTIF_AV_FLAG_LOCAL_SUSPEND_PENDING;
        btif_av_cb[index].tws_state_suspend = false;
      }
#endif
      if (alarm_is_scheduled(btif_av_cb[index].suspend_rsp_track_timer)) {
        BTIF_TRACE_DEBUG("%s: BTA_AV_SUSPEND_EVT is received, clear suspend_rsp_track_timer",
                             __func__);
        btif_av_cb[index].fake_suspend_rsp = false;
        alarm_cancel(btif_av_cb[index].suspend_rsp_track_timer);
      }

      // Check if this suspend is due to DUAL_Handoff
      if ((btif_av_cb[index].dual_handoff) &&
          (p_av->suspend.status == BTA_AV_SUCCESS)) {
        if (!btif_av_is_split_a2dp_enabled() &&
            !btif_a2dp_source_is_hal_v2_supported()){
          uint8_t curr_hdl = btif_av_cb[index].bta_handle;
          uint8_t* cur_codec_cfg = NULL;
          uint8_t* old_codec_cfg = NULL;
          cur_codec_cfg = bta_av_co_get_peer_codec_info(curr_hdl);
          if (cur_codec_cfg != NULL) {
           BTIF_TRACE_EVENT("BTA_AV_SUSPEND_EVT: Current codec = ");
           for (int i = 0; i < AVDT_CODEC_SIZE; i++)
             BTIF_TRACE_EVENT("%d ",cur_codec_cfg[i]);
          }
          int other_index = btif_av_get_other_connected_idx(index);
          if (other_index != INVALID_INDEX) {
            uint8_t other_hdl = btif_av_cb[other_index].bta_handle;
            old_codec_cfg = bta_av_co_get_peer_codec_info(other_hdl);
          }
          if (old_codec_cfg != NULL) {
           BTIF_TRACE_EVENT("%s: BTA_AV_SUSPEND_EVT: Old codec = ", __func__);
           for (int i = 0; i < AVDT_CODEC_SIZE; i++)
             BTIF_TRACE_EVENT("%d ",old_codec_cfg[i]);
          }
          /*In P implementation Audio is sending suspend for same
            codec SHO and different codec SHO*/
          btif_dispatch_sm_event(BTIF_AV_SETUP_CODEC_REQ_EVT, NULL, 0);
          is_block_hal_start = true;
          btif_trigger_unblock_audio_start_recovery_timer();
          BTIF_TRACE_EVENT("%s: BTA_AV_SUSPEND_EVT: Wait for Audio Start in non-split", __func__);

        } else {
          BTIF_TRACE_EVENT("%s: BTA_AV_SUSPEND_EVT:SplitA2DP/NS 2.0 Disallow stack"
                 "start wait Audio to Start", __func__);
          audio_start_awaited = true;
        }
      } else {
        for (int i = 0; i < btif_max_av_clients; i++) {
          if (i != index && (btif_av_cb[i].dual_handoff == true))
            btif_av_cb[i].dual_handoff = false;
        }
      }
      if (btif_av_cb[index].remote_started) {
        if (btif_a2dp_source_is_remote_start()) {
          BTIF_TRACE_DEBUG("%s:cancel remote start timer",__func__);
          if(btif_a2dp_source_last_remote_start_index() == index)
            btif_a2dp_source_cancel_remote_start();
          /*
           * Remote sent avdtp start followed by avdtp suspend, setting
           * the flag not to update the play state to app
           */
           //remote_start_cancelled = true;
        }
        btif_av_cb[index].remote_started = false;
      }
      else if (p_av->suspend.initiator != true) {
        /* remote suspend, notify HAL and await audioflinger to
         * suspend/stop stream
         * set remote suspend flag to block media task from restarting
         * stream only if we did not already initiate a local suspend
         * set remote suspend flag before suspending stream as in race conditions
         * when stream is suspended, but flag is things ge tossed up
         */
        BTIF_TRACE_EVENT("%s: Clear before suspending", __func__);
        if ((btif_av_cb[index].flags & BTIF_AV_FLAG_LOCAL_SUSPEND_PENDING) == 0) {
          btif_av_cb[index].flags |= BTIF_AV_FLAG_REMOTE_SUSPEND;
          bta_av_sniff_enable(false, btif_av_cb[index].peer_bda);
        }
        for (int i = 0; i < btif_max_av_clients; i++)
          if ((i != index) && btif_av_get_ongoing_multicast()) {
            multicast_disabled = true;
            btif_av_update_multicast_state(index);
            BTIF_TRACE_EVENT("%s: Initiate suspend for other HS also", __func__);
            btif_sm_dispatch(btif_av_cb[i].sm_handle,
                    BTIF_AV_SUSPEND_STREAM_REQ_EVT, NULL);
          }
      }

      /* a2dp suspended, stop media task until resumed
       * Multicast: If streaming on other device, don't call onsuspended
       * as it unblocks the audio process and audio process may send
       * subsequent commands and create problem during the time where we
       * still did not receive response for SUSPEND sent to other device.
       * Keep the suspend failure handling untouched and handle
       * only success case to check and avoid calling onsuspended.
       */
      if (pending_cmd == A2DP_CTRL_CMD_SUSPEND ||
          pending_cmd == A2DP_CTRL_CMD_STOP) {
        BTIF_TRACE_DEBUG("%s: HAL suspend/stop pending ack the suspend", __func__);
        hal_suspend_pending = true;
      }

      if ((p_av->suspend.status != BTA_AV_SUCCESS) || hal_suspend_pending) {
        BTIF_TRACE_DEBUG("%s: Suspend failed or HAL pending ack, ack the suspend",__func__);
        btif_a2dp_on_suspended(&p_av->suspend);
      } else if (!btif_av_is_local_started_on_other_idx(index)) {
        BTIF_TRACE_DEBUG("%s: Other device not locally started",__func__);
        if (btif_av_check_flag_remote_suspend(index)){
          BTIF_TRACE_DEBUG("%s: Remote suspended on index = %d, don't ack the suspend ", __func__, index);
        } else{
          BTIF_TRACE_DEBUG("%s: ack the suspend ", __func__);
          btif_a2dp_on_suspended(&p_av->suspend);
        }
      } else {
        BTIF_TRACE_DEBUG("%s: Other device locally started, don't ack the suspend", __func__);
      }

      BTIF_TRACE_DEBUG("%s: local suspend flag: %d, reconfig_event: %d", __func__,
              btif_av_cb[index].flags & BTIF_AV_FLAG_LOCAL_SUSPEND_PENDING,
              btif_av_cb[index].reconfig_event);

      if ((!enable_multicast)&& btif_av_cb[index].is_suspend_for_remote_start
            && (btif_av_is_playing_on_other_idx(index)))
      {
        BTIF_TRACE_IMP("%s Don't update audio state change to app for idx =%d", __func__, index);
        btif_av_cb[index].is_device_playing = false;
      } else {
        if (!((btif_av_cb[index].flags & BTIF_AV_FLAG_LOCAL_SUSPEND_PENDING)
                                          || (p_av->suspend.initiator == true)))
        {
          btif_report_audio_state(BTAV_AUDIO_STATE_REMOTE_SUSPEND, &(btif_av_cb[index].peer_bda));
        }
        else
        {
          btif_report_audio_state(BTAV_AUDIO_STATE_STOPPED, &(btif_av_cb[index].peer_bda));
        }
      }

      remote_start_cancelled = false;
      // if not successful, remain in current state
      if (p_av->suspend.status != BTA_AV_SUCCESS) {
        if (btif_av_cb[index].is_suspend_for_remote_start) {
          BTIF_TRACE_DEBUG("%s: Suspend sent for remote start failed", __func__);
          btif_av_cb[index].is_suspend_for_remote_start = false;
          if (!btif_av_is_playing_on_other_idx(index) &&
                  (index == btif_av_get_latest_device_idx_to_start())) {
            BTIF_TRACE_DEBUG("%s: other index not playing, setup codec", __func__);
            btif_dispatch_sm_event(BTIF_AV_SETUP_CODEC_REQ_EVT, NULL, 0);
          }
        }

        if (!btif_av_cb[index].fake_suspend_rsp)
          btif_av_cb[index].flags &= ~BTIF_AV_FLAG_LOCAL_SUSPEND_PENDING;

        if (btif_av_cb[index].peer_sep == AVDT_TSEP_SNK) {
          BTIF_TRACE_DEBUG("%s: resetting tx_flush flag", __func__);
          // suspend failed, reset back tx flush state
          btif_a2dp_source_set_tx_flush(false);
        }
        return false;
      }
      btif_av_cb[index].is_suspend_for_remote_start = false;
      btif_av_cb[index].is_device_playing = false;
      btif_sm_change_state(btif_av_cb[index].sm_handle, BTIF_AV_STATE_OPENED);
      btif_report_audio_state_to_ba(BTAV_AUDIO_STATE_REMOTE_SUSPEND);

      // suspend completed and state changed, clear pending status
      if (!btif_av_cb[index].fake_suspend_rsp)
        btif_av_cb[index].flags &= ~BTIF_AV_FLAG_LOCAL_SUSPEND_PENDING;

      if (btif_av_cb[index].reconfig_event) {
        btif_av_process_cached_src_codec_config(index);
      }

      /* clear delay record array when stream suspended */
      BTIF_TRACE_DEBUG("clear delay record array when stream suspended");
      delay_record_idx = 0;
      average_delay = 0;
      memset(delay_record, 0, sizeof(int64_t) * DELAY_RECORD_COUNT);
      break;

    case BTA_AV_STOP_EVT:
      btif_av_cb[index].flags |= BTIF_AV_FLAG_PENDING_STOP;
      BTIF_TRACE_DEBUG("%s: Stop the AV Data channel", __func__);
      if (is_multicast_supported && !btif_av_cb[index].current_playing) {
        APPL_TRACE_WARNING("%s:Non active device disconnected,continue streaming",__func__);
      } else {
        btif_a2dp_on_stopped(&p_av->suspend);
      }
      btif_av_cb[index].is_device_playing = false;

      if ((!enable_multicast)&& btif_av_cb[index].is_suspend_for_remote_start
          && (btif_av_is_playing_on_other_idx(index))) {
        BTIF_TRACE_IMP("%s Do not update audio state change to app for index =%d", __FUNCTION__, index);
        btif_av_cb[index].is_device_playing = FALSE;
      } else {
        btif_report_audio_state(BTAV_AUDIO_STATE_STOPPED, &(btif_av_cb[index].peer_bda));
      }
      if (btif_av_cb[index].remote_started) {
        if (btif_a2dp_source_is_remote_start()) {
          BTIF_TRACE_DEBUG("%s:cancel remote start timer",__func__);
          if(btif_a2dp_source_last_remote_start_index() == index)
            btif_a2dp_source_cancel_remote_start();
        }
        btif_av_cb[index].remote_started = false;
      }
      btif_av_cb[index].is_suspend_for_remote_start = FALSE;
      // if stop was successful, change state to open
      if (p_av->suspend.status == BTA_AV_SUCCESS)
        btif_sm_change_state(btif_av_cb[index].sm_handle, BTIF_AV_STATE_OPENED);

      btif_report_audio_state_to_ba(BTAV_AUDIO_STATE_STOPPED);
      break;

    case BTA_AV_CLOSE_EVT:
      btif_av_cb[index].flags |= BTIF_AV_FLAG_PENDING_STOP;

/* SPLITA2DP */
      if (!btif_av_is_connected_on_other_idx(index)) {
        if (btif_av_is_split_a2dp_enabled() &&
           !btif_a2dp_source_is_hal_v2_supported()) {
          if (!isBATEnabled()) {
            btif_a2dp_audio_if_init = false;
            btif_a2dp_audio_interface_deinit();
          }
        }
      }
/* SPLITA2DP */
      /* avdtp link is closed */
      btif_a2dp_on_stopped(NULL);

      /* inform the application that we are disconnected */
      btif_report_connection_state(BTAV_CONNECTION_STATE_DISCONNECTED,
                                   &(btif_av_cb[index].peer_bda));

      btif_sm_change_state(btif_av_cb[index].sm_handle, BTIF_AV_STATE_IDLE);
      btif_report_connection_state_to_ba(BTAV_CONNECTION_STATE_DISCONNECTED);
      break;

    case BTA_AV_RC_OPEN_EVT:
      btif_av_check_rc_connection_priority(p_data);
      break;

    case BTIF_AV_OFFLOAD_START_REQ_EVT:
      if (btif_av_cb[index].flags != BTIF_AV_FLAG_LOCAL_SUSPEND_PENDING &&
          btif_av_cb[index].flags != BTIF_AV_FLAG_REMOTE_SUSPEND &&
          btif_av_cb[index].remote_started == false)
      {
          bool is_scrambling_enabled = btif_av_is_scrambling_enabled();

          BTIF_TRACE_WARNING("%s:is_scrambling_enabled %d",__func__,
                                    is_scrambling_enabled);

          bool is_44p1kFreq_supported = btif_av_is_44p1kFreq_supported();

          BTIF_TRACE_WARNING("%s:is_44p1kFreq_supported %d",__func__,
                                    is_44p1kFreq_supported);

          BTA_AvOffloadStart(btif_av_cb[index].bta_handle, is_scrambling_enabled);
      }
      else if (btif_av_cb[index].remote_started)
      {
          BTIF_TRACE_ERROR("%s:remote started for this index, ignore req",__func__);
      }
      else
      {
          BTIF_TRACE_ERROR("%s:Suspend pending, cannot start offload",__func__);
          btif_a2dp_on_offload_started(BTA_AV_FAIL);
      }
      break;

    case BTA_AV_OFFLOAD_START_RSP_EVT:
      tBTA_AV_STATUS status;
      if (!btif_a2dp_src_vsc.multi_vsc_support)
        status = p_av->offload_rsp.status;
      else
        status = p_av->status;

      btif_a2dp_on_offload_started(status);
      if (btif_av_cb[index].tws_device) {
        btif_report_audio_state(BTAV_AUDIO_STATE_STARTED, &(btif_av_cb[index].peer_bda));
      }
      if (btif_av_cb[index].reconfig_event) {
        btif_av_process_cached_src_codec_config(index);
      }

      for(i = 0; i < btif_max_av_clients; i++) {
        btif_av_cb[i].dual_handoff = false;
      }
      break;

    case BTIF_AV_SETUP_CODEC_REQ_EVT: {
      uint8_t hdl = btif_av_get_av_hdl_from_idx(index);
      APPL_TRACE_DEBUG("%s: hdl = %d, btif_a2dp_source_is_hal_v2_supported() %d",
                __func__, hdl, btif_a2dp_source_is_hal_v2_supported());
      if (hdl >= 0 && !btif_a2dp_source_is_hal_v2_supported())
        btif_a2dp_source_setup_codec(hdl);
      }
      break;

    case BTA_AV_DELAY_REPORT_EVT:
      /* Todo Implement: Delay report during A2dp streaming
       * Update latency to A2dp StateMachine if need support
       * dynamic audio/video sync */
      break;

      CHECK_RC_EVENT(event, (tBTA_AV*)p_data);

    default:
      BTIF_TRACE_WARNING("%s: unhandled event=%s", __func__,
                         dump_av_sm_event_name((btif_av_sm_event_t)event));
      return false;
  }

  return true;
}

/*****************************************************************************
 *  Local event handlers
 *****************************************************************************/

static void btif_av_handle_event(uint16_t event, char* p_param) {
  int index = 0;
  tBTA_AV *p_bta_data = (tBTA_AV*)p_param;
  RawAddress *bt_addr = nullptr;
  uint8_t role;
  int uuid;
  bool active_device_selected = false;
  int previous_active_index = INVALID_INDEX;
  int now_active_index = INVALID_INDEX;

  switch (event) {
    case BTIF_AV_INIT_REQ_EVT:
      BTIF_TRACE_DEBUG("%s: BTIF_AV_INIT_REQ_EVT", __func__);
      if (btif_a2dp_source_startup())
        btif_a2dp_sink_on_init();
      break;

    case BTIF_AV_CLEANUP_REQ_EVT: // Clean up to be called on default index
      BTIF_TRACE_DEBUG("%s: BTIF_AV_CLEANUP_REQ_EVT", __func__);
      uuid = (int)*p_param;
      if (btif_a2dp_audio_if_init && !btif_a2dp_source_is_hal_v2_supported()) {
        btif_a2dp_audio_if_init = false;
        btif_a2dp_audio_interface_deinit();
      }
      if (uuid == BTA_A2DP_SOURCE_SERVICE_ID) {
        if (bt_av_src_callbacks) {
          bt_av_src_callbacks = NULL;
          if (bt_av_sink_callbacks != NULL)
            break;
        }
      } else {
        if (bt_av_sink_callbacks) {
          bt_av_sink_callbacks = NULL;
          if (bt_av_src_callbacks != NULL)
            break;
        }
      }
      btif_a2dp_src_vsc.multi_vsc_support = false;//reset multi vsc support flag
      btif_a2dp_source_shutdown();
      btif_a2dp_sink_shutdown();
      return;

    case BTIF_AV_CONNECT_REQ_EVT:
      if (p_param != NULL) {
        btif_av_connect_req_t* connect_req_p = (btif_av_connect_req_t*)p_param;
        bt_addr = connect_req_p->target_bda;
        index = btif_av_idx_by_bdaddr(bt_addr);
        if (index == btif_max_av_clients) {
          index = 0;
        }
        BTIF_TRACE_DEBUG("%s: BTIF_AV_CONNECT_REQ_EVT on idx = %d", __func__, index);
      }
      break;

    case BTIF_AV_SOURCE_CONFIG_REQ_EVT:
        BTIF_TRACE_DEBUG("BTIF_AV_SOURCE_CONFIG_REQ_EVT more than one device connected");
        index = btif_max_av_clients;
        if (!codec_bda.IsEmpty())
          index = btif_av_idx_by_bdaddr(&codec_bda);

        if (index == btif_max_av_clients)
        {
          if (btif_av_stream_started_ready())
            index = btif_av_get_latest_playing_device_idx();
          else
            index = btif_av_get_latest_device_idx_to_start();
        }
        BTIF_TRACE_IMP("BTIF_AV_SOURCE_CONFIG_REQ_EVT on idx = %d", index);
        codec_bda = {};
      break;
    case BTIF_AV_SOURCE_CONFIG_UPDATED_EVT:
      if (codec_config_update_enabled == false)
      {
        BTIF_TRACE_IMP("ignore updating BTIF_AV_SOURCE_CONFIG_UPDATED_EVT");
        return;
      }
      if (p_param != NULL) {
        bt_addr = (RawAddress *)p_param;
        index = btif_av_idx_by_bdaddr(bt_addr);
        if (index == btif_max_av_clients) {
            index = 0;
        }
      }
      BTIF_TRACE_IMP("Process BTIF_AV_SOURCE_CONFIG_UPDATED_EVT on idx = %d", index);
      break;

    case BTIF_AV_DISCONNECT_REQ_EVT:
      // Bd address passed should help us in getting the handle
      bt_addr = (RawAddress *)p_param;
      index = btif_av_idx_by_bdaddr(bt_addr);
      break;

    case BTIF_AV_SINK_CONFIG_REQ_EVT: {
      btif_av_sink_config_req_t *config = (btif_av_sink_config_req_t *)p_param;
      index = btif_av_idx_by_bdaddr(&config->peer_bd);
      BTIF_TRACE_DEBUG("%s: device %s index = %d", __func__,
          config->peer_bd.ToString().c_str(), index);
      } break;

    case BTIF_AV_CHECK_PENDING_PLAY_EVT:
      bt_addr = (RawAddress *)p_param;
      BTIF_TRACE_WARNING("%s: device %s ",__func__, (*bt_addr).ToString().c_str());
      if (*bt_addr != RawAddress::kEmpty)
        btif_rc_check_handle_pending_play(*bt_addr, true);
      break;

    case BTIF_AV_TRIGGER_HANDOFF_REQ_EVT:
      bt_addr = (RawAddress *)p_param;
      BTIF_TRACE_WARNING("%s: device %s ",__func__, (*bt_addr).ToString().c_str());
      previous_active_index = btif_av_get_latest_device_idx_to_start();
      /* 1. SetActive Device -> Null */
      if (*bt_addr == RawAddress::kEmpty)
      {
        int streaming_index = INVALID_INDEX;
        for(int i = 0; i < btif_max_av_clients; i++)
        {
          if (btif_av_cb[i].current_playing == TRUE)
            btif_av_cb[i].current_playing = FALSE;
        }
        BTIF_TRACE_IMP("Reset all Current Playing for Device -> Null");
        if (btif_a2dp_source_is_hal_v2_supported()) {
          if (!btif_a2dp_source_end_session(
            btif_av_get_addr_by_index(previous_active_index))) {
            BTIF_TRACE_IMP("Device -> Null, btif_a2dp_source_end_session failed");
          }
          streaming_index =  btif_av_get_latest_stream_device_idx();
          if (previous_active_index < btif_max_av_clients) {
            if (streaming_index == previous_active_index) {
              BTIF_TRACE_IMP("Send suspend to previous active streaming device & stop media alarm");
              btif_sm_dispatch(btif_av_cb[previous_active_index].sm_handle,
                             BTIF_AV_SUSPEND_STREAM_REQ_EVT, NULL);
              btif_a2dp_source_stop_audio_req();
            } else if (streaming_index < btif_max_av_clients &&
                    streaming_index == btif_av_get_tws_pair_idx(previous_active_index)) {
               BTIF_TRACE_IMP("Send suspend to streaming device which is pair of previously active device");
               btif_sm_dispatch(btif_av_cb[streaming_index].sm_handle,
                              BTIF_AV_SUSPEND_STREAM_REQ_EVT, NULL);
               btif_a2dp_source_stop_audio_req();
            }
          }
          btif_av_signal_session_ready();
        }
        break;
      }

      /* 2. SetActive Null -> Device */
      for(int i = 0; i < btif_max_av_clients; i++)
        if (btif_av_cb[i].current_playing == TRUE)
          active_device_selected = true;

      index = btif_av_idx_by_bdaddr(bt_addr);
      if (index == btif_max_av_clients) {
        //Device is disconnected before setting it as active device
        BTIF_TRACE_IMP("%s:Invalid index to set active device",__func__);
        btif_av_signal_session_ready();
        break;
      }

      if ((btif_av_cb[index].retry_rc_connect == true) && (bt_av_src_callbacks != NULL)
          && (btif_av_cb[index].bta_handle != (tBTA_AV_HNDL)INVALID_INDEX))
      {
        BTIF_TRACE_IMP("Retry trigger RC connect for index = %d dropped earlier", index);
        BTA_AvOpenRc(btif_av_cb[index].bta_handle);
        btif_av_cb[index].retry_rc_connect = false;
      }

      if (active_device_selected == false)
      {
        BTIF_TRACE_IMP("For Null -> Device set current playing and don't trigger handoff");
        btif_av_cb[index].current_playing = TRUE;
        btif_av_set_browse_active(*bt_addr, BTA_AV_BROWSE_ACTIVE);
        if (btif_a2dp_source_is_hal_v2_supported()) {
          //btif_av_set_offload_status();
          if (!btif_a2dp_source_restart_session(
               btif_av_get_addr_by_index(previous_active_index),
               btif_av_get_addr_by_index(index))) {
            BTIF_TRACE_IMP("Null -> Device, btif_a2dp_source_restart_session failed");
          }
        }
        break;
      }

      /* 3. SetActive Device -> Device */
      if (btif_av_cb[index].current_playing == TRUE)
      {
        BTIF_TRACE_IMP("Trigger handoff for same device %d discard it", index);
        if (btif_av_current_device_is_tws() && btif_av_cb[index].tws_device)
          btif_av_signal_session_ready();
        break;
      }

      BTIF_TRACE_IMP("BTIF_AV_TRIGGER_HANDOFF_REQ_EVT on index %d", index);
      now_active_index = index;
#if (TWS_ENABLED == TRUE)
      if (btif_av_cb[index].tws_device &&
        btif_av_is_tws_device_playing(index)) {
        btif_av_cb[index].current_playing = TRUE;
        btif_av_signal_session_ready();
        BTIF_TRACE_DEBUG("TWSP device, do not trigger handoff");
        return;
      }
#endif
      btif_av_set_browse_active(*bt_addr, BTA_AV_BROWSE_HANDOFF);
      if (index >= 0 && index < btif_max_av_clients)
      {
        if ((btif_av_cb[index].retry_rc_connect == true)
            && (bt_av_src_callbacks != NULL)
            && (btif_av_cb[index].bta_handle != (tBTA_AV_HNDL)INVALID_INDEX))
        {
          BTIF_TRACE_IMP("Retry trigger RC connect for idx = %d dropped earlier", index);
          BTA_AvOpenRc(btif_av_cb[index].bta_handle);
          btif_av_cb[index].retry_rc_connect = false;
        }

        for(int i = 0; i < btif_max_av_clients; i++)
        {
          if (i == index)
            btif_av_cb[i].current_playing = TRUE;
          else
            btif_av_cb[i].current_playing = FALSE;

          BTIF_TRACE_IMP("current_playing for index %d: %d", i,
                               btif_av_cb[i].current_playing);
        }
        BTIF_TRACE_IMP("going to active index %d: previous active index %d",
                            now_active_index, previous_active_index);
        /*  RC play state is to be cleared to make sure the same when retained
         *  does not impact UI initiated play*/
        if (btif_a2dp_source_is_hal_v2_supported()) {
          //btif_av_set_offload_status();
          if (!btif_a2dp_source_restart_session(
               btif_av_get_addr_by_index(previous_active_index),
               btif_av_get_addr_by_index(now_active_index))) {
            BTIF_TRACE_IMP("Device1 -> Device2, \
                    btif_a2dp_source_restart_session failed");
          }
        }
        if (is_multicast_supported && btif_av_is_playing_on_other_idx(index)) {
          BTIF_TRACE_WARNING("Multicast streaming, do not trigger handoff");
        } else {
          btif_av_trigger_dual_handoff(TRUE, now_active_index,
                                  previous_active_index);
        }
      }
      else
      {
        BTIF_TRACE_WARNING("Device is no longer connected, device switch failed");
      }
      break;
    case BTIF_AV_SET_SILENT_REQ_EVT: {
      btif_av_silent_req_t *silent_req = (btif_av_silent_req_t *)p_param;
      RawAddress bt_addr = silent_req->bd_addr;
      bool is_silent = silent_req->is_silent;
      int idx = btif_av_idx_by_bdaddr(&bt_addr);
      if (idx == btif_max_av_clients) {
        BTIF_TRACE_IMP("%s:Invalid index to set active device",__func__);
        break;
      }
      btif_av_cb[idx].is_silenced = is_silent;
    } break;
    case BTIF_AV_START_STREAM_REQ_EVT:
      /* Get the last connected device on which START can be issued
       * Get the Dual A2dp Handoff Device first, if none is present,
       * go for lastest connected.
       * In A2dp Multicast, the index selected can be any of the
       * connected device. Stack will ensure to START the steaming
       * on both the devices. */
      if (isBATEnabled()) {
          // if BA is enabled, don't process this start.
          BTIF_TRACE_DEBUG(" BA enabled and we received START_REQ ");
          tA2DP_CTRL_CMD pend_cmd = A2DP_CTRL_CMD_NONE;
          if (btif_a2dp_source_is_hal_v2_supported()) {
            pend_cmd =  bluetooth::audio::a2dp::get_pending_command();
          } else {
            pend_cmd = btif_a2dp_audio_interface_get_pending_cmd();
          }
          if (pend_cmd == A2DP_CTRL_CMD_START) {
              BTIF_TRACE_DEBUG(" Audio Waiting for Start ");
              // in this case, send start req to BA and let BA
              // ack back audio once BA starts it.
              ba_send_message(BTIF_BA_AUDIO_START_REQ_EVT, 0, NULL, true);
          }
          return;
      }
      index = btif_av_get_latest_device_idx_to_start();
#if (TWS_ENABLED == TRUE)
      if (btif_av_current_device_is_tws()) {
        int started_index = btif_av_get_latest_playing_device_idx();
        if (btif_av_stream_started_ready() &&
          started_index == index) {
          index = btif_av_get_tws_pair_idx(started_index);
          BTIF_TRACE_DEBUG("Switching start req from index %d to %d",started_index,index);
        }
      }
#endif
      break;
    case BTIF_AV_STOP_STREAM_REQ_EVT:
    case BTIF_AV_SUSPEND_STREAM_REQ_EVT:
      // Should be handled by current STARTED
      if(p_param == NULL)
        index = btif_av_get_latest_stream_device_idx();
      else
        index = (int)*p_param;
      BTIF_TRACE_DEBUG("BTIF_AV_SUSPEND_STREAM_REQ_EVT:revising effective index %d", index);
      break;
    case BTIF_AV_OFFLOAD_START_REQ_EVT:
      {
        //AV handle passed by a2dp on started
        tBTA_AV_HNDL hdl = (tBTA_AV_HNDL)*p_param;
        index = HANDLE_TO_INDEX(hdl);
      }
      break;
    case BTIF_AV_REINIT_AUDIO_IF:
      btif_av_reinit_audio_interface();
      return;
    case BTIF_AV_REMOTE_SUSPEND_STREAM_REQ_EVT:
      index = (int) *p_param;
      if (index != INVALID_INDEX && index > btif_max_av_clients &&
        btif_av_cb[index].remote_started == false)
        index = btif_max_av_clients;
      if (index >= btif_max_av_clients) {
        BTIF_TRACE_ERROR("%s: Invalid index for connection", __func__);
        return;
      }
      BTIF_TRACE_IMP("%s: Remote Started set @ index = %d", __func__, index);
#ifdef BTA_AV_SPLIT_A2DP_ENABLED
      if ((btif_av_is_split_a2dp_enabled()) && (!btif_av_is_playing_on_other_idx(index))) {
        BTIF_TRACE_IMP("%s: Other index is not playing", __func__);
        bta_av_vendor_offload_stop(NULL);
      }
#endif
      event = BTIF_AV_SUSPEND_STREAM_REQ_EVT;
      break;
    case BTIF_AV_RESET_REMOTE_STARTED_FLAG_UPDATE_AUDIO_STATE_EVT:
      index = (int) *p_param;
      if (index >= 0 && index < btif_max_av_clients) {
        if (btif_av_cb[index].peer_sep == AVDT_TSEP_SNK)
        {
          BTIF_TRACE_IMP("%s: on remote start clean up update audio started state for index %d",
                          __func__, index);
          btif_report_audio_state(BTAV_AUDIO_STATE_STARTED, &(btif_av_cb[index].peer_bda));
        }
        btif_av_cb[index].is_device_playing = true;
      }
      FALLTHROUGH;
    case BTIF_AV_RESET_REMOTE_STARTED_FLAG_EVT:
      btif_av_reset_remote_started_flag();
      return;
    case BTIF_AV_REPORT_AUDIO_STATE_EVT:
      index = (int) *p_param;
      if (index < btif_max_av_clients && btif_av_cb[index].tws_device) {
        BTIF_TRACE_IMP("%s: report audio state for index %d",__func__,index);
        btif_report_audio_state(BTAV_AUDIO_STATE_STARTED, &(btif_av_cb[index].peer_bda));
      }
      return;
      // Events from the stack, BTA
    case BTA_AV_ENABLE_EVT:
      index = 0;
      break;

    case BTA_AV_REGISTER_EVT:
      index = HANDLE_TO_INDEX(p_bta_data->registr.hndl);
      break;

    case BTA_AV_OPEN_EVT:
      index = HANDLE_TO_INDEX(p_bta_data->open.hndl);
      break;

    case BTA_AV_ROLE_CHANGED_EVT:
      index = HANDLE_TO_INDEX(p_bta_data->role_changed.hndl);
      role = p_bta_data->role_changed.new_role;
      BTIF_TRACE_EVENT("Role change: 0x%x: new role: %s",
      p_bta_data->role_changed.hndl, (role == HOST_ROLE_SLAVE) ? "Slave" : "Master");
      if (index >= 0 && index < btif_max_av_clients) {
        btif_av_cb[index].is_slave = (role == HOST_ROLE_SLAVE) ? true : false;
        btif_av_update_multicast_state(index);
      } else {
        BTIF_TRACE_ERROR("%s: Invalid index for connection", __func__);
      }
      return;

    case BTA_AV_PENDING_EVT:
        /* In race conditions, outgoing and incoming connections
        * at same time check for BD address at index and if it
        * does not match then check for first avialable index.
        */
        index = HANDLE_TO_INDEX(p_bta_data->pend.hndl);
        if (index >= 0 && index < btif_max_av_clients &&
            (p_bta_data->pend.bd_addr == btif_av_cb[index].peer_bda))
       {
            BTIF_TRACE_EVENT("incomming connection at index %d", index);
       }
       else
       {
           RawAddress empty_addr = RawAddress::kEmpty;
           index = btif_av_idx_by_bdaddr(&empty_addr);
           if (index >= btif_max_av_clients)
           {
               BTIF_TRACE_ERROR("No free SCB available");
               BTA_AvDisconnect(p_bta_data->pend.bd_addr);
           }
           else
           {
               BTIF_TRACE_EVENT("updated index for connection %d", index);
           }
       }
       break;
    case BTA_AV_REJECT_EVT:
      index = HANDLE_TO_INDEX(p_bta_data->reject.hndl);
      break;

    case BTA_AV_STOP_EVT:
      index = HANDLE_TO_INDEX(p_bta_data->suspend.hndl);
      break;

    case BTA_AV_CLOSE_EVT:
      index = HANDLE_TO_INDEX(p_bta_data->close.hndl);
      break;

    case BTA_AV_START_EVT:
      index = HANDLE_TO_INDEX(p_bta_data->start.hndl);
      break;

    case BTA_AV_RECONFIG_EVT:
      index = HANDLE_TO_INDEX(p_bta_data->reconfig.hndl);
      break;

    case BTA_AV_SUSPEND_EVT:
      index = HANDLE_TO_INDEX(p_bta_data->suspend.hndl);
      break;

    case BTA_AV_OFFLOAD_START_RSP_EVT:
      if (!btif_a2dp_src_vsc.multi_vsc_support) {
        BTIF_TRACE_DEBUG("%s:hndl  = %x",__func__, p_bta_data->offload_rsp.hndl);
        index = HANDLE_TO_INDEX(p_bta_data->offload_rsp.hndl);
      } else {
        index = btif_av_get_latest_playing_device_idx();
      }

      if (index == btif_max_av_clients) {
        for (int i = 0; i < btif_max_av_clients; i++) {
          if (btif_av_check_flag_remote_suspend(i)) {
            index = i;
            break;
          }
        }
      }
      BTIF_TRACE_EVENT("index = %d, max connections = %d", index, btif_max_av_clients);
      break;

    case BTA_AV_OFFLOAD_STOP_RSP_EVT:
      if (p_bta_data->status == BTA_AV_SUCCESS) {
        btif_a2dp_src_vsc.tx_started = FALSE;
        btif_a2dp_src_vsc.tx_stop_initiated = FALSE;
        btif_media_send_reset_vendor_state();
      } else {
        btif_a2dp_src_vsc.tx_stop_initiated = FALSE;
      }
      break;
    case BTA_AV_DELAY_REPORT_EVT:
      index = HANDLE_TO_INDEX(p_bta_data->delay_rpt.hndl);
      break;

      /* Handle all RC events on default index. RC handling should take
       * care of the events. All events come with BD Address
       * Handled well in AV Opening, opened and started state
       * AV Idle handler needs to take care of this event properly.
       */
    case BTA_AV_RC_OPEN_EVT:
      index = btif_av_get_valid_idx_for_rc_events(p_bta_data->rc_open.peer_addr,
                p_bta_data->rc_open.rc_handle);
      break;

    case BTA_AV_RC_BROWSE_OPEN_EVT:
      BTIF_TRACE_ERROR("handle BTA_AV_RC_BROWSE_OPEN_EVT on Browse connect");
      if (p_bta_data->rc_browse_open.status == 0)
        btif_av_set_browse_active(p_bta_data->rc_browse_open.peer_addr, BTA_AV_BROWSE_CONNECT);
      break;

    case BTA_AV_RC_CLOSE_EVT:
      /* If there is no entry in the connection table
       * RC handler has to be called for cleanup.
       * Directly call the RC handler as we cannot
       * associate any AV handle to it.
       */
      index = btif_av_idx_by_bdaddr(&p_bta_data->rc_close.peer_addr);
      BTIF_TRACE_DEBUG("%s: BTA_AV_RC_CLOSE_EVT: peer_addr=%s, index=%d", __func__,
                  p_bta_data->rc_close.peer_addr.ToString().c_str(), index);
      if (index >= 0 && index < btif_max_av_clients) {
        if (btif_av_cb[index].current_playing == false &&
            btif_av_is_device_connected(p_bta_data->rc_close.peer_addr)) {
          BTIF_TRACE_IMP("Mark retry RC connect for inactive idx = %d drops RC",
                                                index);
          btif_av_cb[index].retry_rc_connect = true;
        }
      } else {
        BTIF_TRACE_DEBUG("Invalid index for device");
      }

      if (index == btif_max_av_clients || btif_av_cb[index].sm_handle == NULL)
        btif_rc_handler(event, (tBTA_AV*)p_bta_data);
      break;

    case BTA_AV_RC_COLL_DETECTED_EVT:
      index = btif_av_idx_by_bdaddr(&p_bta_data->rc_col_detected.peer_addr);
      break;

      /* Let the RC handler decide on these passthrough cmds
       * Use rc_handle to get the active AV device and use that mapping.
       */
    /* FALLTHROUGH */
    case BTA_AV_REMOTE_CMD_EVT:
    case BTA_AV_VENDOR_CMD_EVT:
    case BTA_AV_META_MSG_EVT:
    case BTA_AV_RC_FEAT_EVT:
      index = 0;
      BTIF_TRACE_EVENT("RC events: on index = %d", index);
      break;

    case BTIF_AV_SETUP_CODEC_REQ_EVT:
      index = btif_av_get_latest_device_idx_to_start();
      break;

    case BTIF_AV_PROCESS_HIDL_REQ_EVT:
      btif_a2dp_source_process_request((tA2DP_CTRL_CMD ) *p_param);
      break;

    default:
      BTIF_TRACE_ERROR("Unhandled event = %d", event);
      break;
  }
  BTIF_TRACE_DEBUG("Handle the AV event = %x on index = %d", event, index);
  if (index >= 0 && index < btif_max_av_clients)
      btif_sm_dispatch(btif_av_cb[index].sm_handle, event, (void*)p_param);
  else
      BTIF_TRACE_ERROR("Unhandled Index = %d", index);
  btif_av_event_free_data(event, p_param);

}

void btif_av_reset_remote_started_flag()
{
    int i;
    BTIF_TRACE_DEBUG("btif_av_reset_remote_started_flag");
    for (i = 0; i < btif_max_av_clients; i++)
    {
        if (btif_av_cb[i].remote_started)
            btif_av_cb[i].remote_started = false;
    }
}

/*******************************************************************************
 *
 * Function         btif_av_get_valid_idx
 *
 * Description      Check the validity of the current index for the connection
 *
 * Returns          Boolean
 *
 ******************************************************************************/
static bool btif_av_get_valid_idx(int idx) {
  btif_sm_state_t state = btif_sm_get_state(btif_av_cb[idx].sm_handle);
  return ((state == BTIF_AV_STATE_OPENED) ||
          (state ==  BTIF_AV_STATE_STARTED) ||
          (state == BTIF_AV_STATE_OPENING) ||
          (state == BTIF_AV_STATE_CLOSING));
}

/*******************************************************************************
 *
 * Function         btif_av_idx_by_bdaddr
 *
 * Description      Get the index corresponding to BD addr
 *
 * Returns          int
 *
 ******************************************************************************/
int btif_av_idx_by_bdaddr(RawAddress *bd_addr) {
  int i;
  for (i = 0; i < btif_max_av_clients; i++)
    if (*bd_addr == btif_av_cb[i].peer_bda)
      return i;
  return i;
}

bool btif_av_is_current_device(RawAddress address) {
  int index;

  index = btif_av_idx_by_bdaddr(&address);
  if ((index < btif_max_av_clients) && (btif_av_cb[index].current_playing)
      && (!btif_av_cb[index].remote_started))
    return true;
  return false;
}

/*******************************************************************************
 *
 * Function         btif_av_get_latest_device_idx_to_start
 *
 * Description      Get the index of the AV where streaming is to be started
 *
 * Returns          int
 *
 ******************************************************************************/
int btif_av_get_latest_device_idx_to_start() {
  int i;
  for (i = 0; i < btif_max_av_clients; i++)
    if (btif_av_cb[i].current_playing)
      break;
  if (i == btif_max_av_clients)
    BTIF_TRACE_ERROR("%s:No valid active device found",__func__);
  return i;
}

/*******************************************************************************
 *
 * Function         btif_av_get_latest_playing_device_idx
 *
 * Description      Get the index of AV where streaming is happening
 *
 * Returns          int
 *
 ******************************************************************************/
int btif_av_get_latest_playing_device_idx() {
  int i;
  btif_sm_state_t state;
  for (i = 0; i < btif_max_av_clients; i++) {
    state = btif_sm_get_state(btif_av_cb[i].sm_handle);
    if (state == BTIF_AV_STATE_STARTED) {
      BTIF_TRACE_IMP("Latest playing device index %d", i);
      break;
    }
  }
  return i;
}

/*******************************************************************************
 *
 * Function         btif_av_get_latest_stream_device_idx
 *
 * Description      Get the index of AV where streaming is happening but not
                    remote started index
 *
 * Returns          int
 *
 ******************************************************************************/
int btif_av_get_latest_stream_device_idx() {
  int i;
  btif_sm_state_t state;
  for (i = 0; i < btif_max_av_clients; i++) {
    state = btif_sm_get_state(btif_av_cb[i].sm_handle);
    if ((state == BTIF_AV_STATE_STARTED) && !(btif_av_cb[i].remote_started)) {
      BTIF_TRACE_ERROR("Latest playing device index is %d ", i);
      break;
    }
  }
  return i;
}


/*******************************************************************************
 *
 * Function         btif_get_is_remote_started_idx
 *
 * Description      Get the index of AV where last remote Start is received
 *
 * Returns          int
 *
 ******************************************************************************/
int btif_get_is_remote_started_idx() {
  int index = btif_a2dp_source_last_remote_start_index();
  BTIF_TRACE_DEBUG("%s: last remote started index: %d", __func__, index);
  if ((index == -1) ||
      ((index != -1) && !btif_av_cb[index].remote_started))
    index = btif_max_av_clients;
  return index;
}

/*******************************************************************************
 *
 * Function         btif_av_is_playing
 *
 * Description      Is AV in streaming state
 *
 * Returns          bool
 *
 ******************************************************************************/
bool btif_av_is_playing() {
  int i;
  for (i = 0; i < btif_max_av_clients; i++) {
    btif_av_cb[i].state = btif_sm_get_state(btif_av_cb[i].sm_handle);
    if (btif_av_cb[i].state == BTIF_AV_STATE_STARTED) {
      BTIF_TRACE_EVENT("btif_av_is_playing on index= %d", i);
      return true;
    }
  }
  return false;
}

/*******************************************************************************
 *
 * Function         btif_get_conn_state_of_device
 *
 * Description      Returns the state of AV scb
 *
 * Returns          int
 *
 ******************************************************************************/
static int btif_get_conn_state_of_device(RawAddress address) {
  btif_sm_state_t state = BTIF_AV_STATE_IDLE;
  int i;
  for (i = 0; i < btif_max_av_clients; i++) {
    if (address == btif_av_cb[i].peer_bda) {
      state = btif_sm_get_state(btif_av_cb[i].sm_handle);
      BTIF_TRACE_EVENT("%s: index = %d, BD Found: %s, state: %s",
           __func__, i, btif_av_cb[i].peer_bda.ToString().c_str(),
           dump_av_sm_state_name((btif_av_state_t)state));
      return state;
    }
  }
  return state;
}

/*******************************************************************************
 *
 * Function         btif_av_get_valid_idx_for_rc_events
 *
 * Description      gets th valid index for the RC event address
 *
 * Returns          int
 *
 ******************************************************************************/
static int btif_av_get_valid_idx_for_rc_events(RawAddress bd_addr, int rc_handle) {
  int index = 0;
  /* First try to find if it is first event in AV IF
   * both the handles would be in IDLE state, pick the first
   * If we get second RC event while processing the priority
   * for the first, reject the second connection.
   */

  /* Get the index from connected SCBs */
  index = btif_av_idx_by_bdaddr(&bd_addr);
  if (index == btif_max_av_clients) {
    /* None of the SCBS matched * Allocate free SCB, null address SCB*/
    RawAddress empty_addr = RawAddress::kEmpty;
    index = btif_av_idx_by_bdaddr(&empty_addr);
    BTIF_TRACE_EVENT("btif_av_get_valid_idx_for_rc_events is %d", index);
    if (index >= btif_max_av_clients) {
      BTIF_TRACE_EVENT("disconnect only AVRCP device rc_handle %d", rc_handle);
      BTA_AvCloseRc(rc_handle);
    }
  }
  return index;
}

/*******************************************************************************
 *
 * Function         btif_av_check_rc_connection_priority
 *
 * Description      Handles Priority callback for RC connections
 *
 * Returns          void
 *
 ******************************************************************************/
static void btif_av_check_rc_connection_priority(void *p_data) {
  RawAddress peer_bda;

  BTIF_TRACE_DEBUG("%s: %s:", __func__,
                 ((tBTA_AV*)p_data)->rc_open.peer_addr.ToString().c_str());
  /*Check if it is for same AV device*/
  if (btif_av_is_device_connected(((tBTA_AV*)p_data)->rc_open.peer_addr)) {
    /*AV is connected */
    BTIF_TRACE_DEBUG("AV is connected, process RC connect event");
    btif_rc_handler(BTA_AV_RC_OPEN_EVT, (tBTA_AV*)p_data);
    return;
  }
  peer_bda = ((tBTA_AV*)p_data)->rc_open.peer_addr;

  if (idle_rc_event != 0) {
    BTIF_TRACE_DEBUG("Processing another RC Event ");
    return;
  }
  idle_rc_event = BTA_AV_RC_OPEN_EVT;
  memcpy(&idle_rc_data, ((tBTA_AV*)p_data), sizeof(tBTA_AV));
  if (((tBTA_AV*)p_data)->rc_open.status == BTA_AV_SUCCESS) {
        BTIF_TRACE_DEBUG("RC conn is success ");
    if (bt_av_src_callbacks != NULL) {
      BTIF_TRACE_DEBUG(" Check Device priority");
#ifdef BT_AV_SHO_FEATURE
      HAL_CBACK(bt_av_src_callbacks, connection_priority_cb, &peer_bda);
#else
      allow_connection(true, &peer_bda);
#endif
    }
  } else {
    idle_rc_event = 0;
    memset(&idle_rc_data, 0, sizeof(tBTA_AV));
  }
  return;
}



void btif_av_event_deep_copy(uint16_t event, char* p_dest, char* p_src) {
  BTIF_TRACE_DEBUG("%s: event: %d", __func__, event);

  if (!p_dest || !p_src) {
    BTIF_TRACE_DEBUG("%s: p_src is null", __func__);
    return;
  }
  switch (event) {
    case BTA_AV_META_MSG_EVT: //17
      {
        BTIF_TRACE_DEBUG("%s: BTA_AV_META_MSG_EVT", __func__);
        tBTA_AV_META_MSG* av_src_meta_msg = (tBTA_AV_META_MSG*)p_src;
        tBTA_AV_META_MSG* av_dest_meta_msg = (tBTA_AV_META_MSG*)p_dest;
        BTIF_TRACE_DEBUG("%s: event: %d, size: %d", __func__, event, sizeof(*av_src_meta_msg));

        // First copy the structure
        maybe_non_aligned_memcpy(av_dest_meta_msg, av_src_meta_msg, sizeof(*av_src_meta_msg));
        if (av_src_meta_msg->p_data && av_src_meta_msg->len) {
            av_dest_meta_msg->p_data = (uint8_t*)osi_calloc(av_src_meta_msg->len);
          memcpy(av_dest_meta_msg->p_data, av_src_meta_msg->p_data,
                 av_src_meta_msg->len);
        }

        if (av_src_meta_msg->p_msg) {
          av_dest_meta_msg->p_msg = (tAVRC_MSG*)osi_calloc(sizeof(tAVRC_MSG));
          memcpy(av_dest_meta_msg->p_msg, av_src_meta_msg->p_msg,
                 sizeof(tAVRC_MSG));

          tAVRC_MSG* p_msg_src = av_src_meta_msg->p_msg;
          tAVRC_MSG* p_msg_dest = av_dest_meta_msg->p_msg;

          BTIF_TRACE_DEBUG("%s: opcode: 0x%x, vendor_len: %d, p_msg_src->browse.browse_len: %d",
            __func__, p_msg_src->hdr.opcode, p_msg_src->vendor.vendor_len, p_msg_src->browse.browse_len);
          if ((p_msg_src->hdr.opcode == AVRC_OP_VENDOR) &&
              (p_msg_src->vendor.p_vendor_data && p_msg_src->vendor.vendor_len)) {
            BTIF_TRACE_DEBUG("%s: AVRC_OP_VENDOR:", __func__);
            p_msg_dest->vendor.p_vendor_data =
                (uint8_t*)osi_calloc(p_msg_src->vendor.vendor_len);
            memcpy(p_msg_dest->vendor.p_vendor_data,
                   p_msg_src->vendor.p_vendor_data, p_msg_src->vendor.vendor_len);
          }
          if ((p_msg_src->hdr.opcode == AVRC_OP_BROWSE) &&
              p_msg_src->browse.p_browse_data && p_msg_src->browse.browse_len) {
            BTIF_TRACE_DEBUG("%s: AVRC_OP_BROWSE:", __func__);
            p_msg_dest->browse.p_browse_data =
               (uint8_t*)osi_calloc(p_msg_src->browse.browse_len);
            memcpy(p_msg_dest->browse.p_browse_data,
                   p_msg_src->browse.p_browse_data, p_msg_src->browse.browse_len);
            android_errorWriteLog(0x534e4554, "109699112");
          }
        }
        break;
      }

    case BTA_AV_PENDING_EVT: //16
      {
        tBTA_AV_PEND* av_src_pend = (tBTA_AV_PEND*)p_src;
        tBTA_AV_PEND* av_dest_pend = (tBTA_AV_PEND*)p_dest;
        BTIF_TRACE_DEBUG("%s: event: %d, size: %d", __func__, event, sizeof(*av_src_pend));
        maybe_non_aligned_memcpy(av_dest_pend, av_src_pend, sizeof(*av_src_pend));
        break;
      }

    case BTA_AV_ENABLE_EVT: //0
      {
        tBTA_AV_ENABLE* av_src_enable = (tBTA_AV_ENABLE*)p_src;
        tBTA_AV_ENABLE* av_dest_enable = (tBTA_AV_ENABLE*)p_dest;
        BTIF_TRACE_DEBUG("%s: event: %d, size: %d", __func__, event, sizeof(*av_src_enable));
        maybe_non_aligned_memcpy(av_dest_enable, av_src_enable, sizeof(*av_src_enable));
        break;
      }

    case BTA_AV_REGISTER_EVT: //1
      {
        tBTA_AV_REGISTER* av_src_reg = (tBTA_AV_REGISTER*)p_src;
        tBTA_AV_REGISTER* av_dest_reg = (tBTA_AV_REGISTER*)p_dest;
        BTIF_TRACE_DEBUG("%s: event: %d, size: %d", __func__, event, sizeof(*av_src_reg));
        maybe_non_aligned_memcpy(av_dest_reg, av_src_reg, sizeof(*av_src_reg));
        break;
      }

    case BTA_AV_OPEN_EVT: //2
      {
        tBTA_AV_OPEN* av_src_open = (tBTA_AV_OPEN*)p_src;
        tBTA_AV_OPEN* av_dest_open = (tBTA_AV_OPEN*)p_dest;
        BTIF_TRACE_DEBUG("%s: event: %d, size: %d", __func__, event, sizeof(*av_src_open));
        maybe_non_aligned_memcpy(av_dest_open, av_src_open, sizeof(*av_src_open));
        break;
      }

    case BTA_AV_CLOSE_EVT: //3
      {
        tBTA_AV_CLOSE* av_src_close = (tBTA_AV_CLOSE*)p_src;
        tBTA_AV_CLOSE* av_dest_close = (tBTA_AV_CLOSE*)p_dest;
        BTIF_TRACE_DEBUG("%s: event: %d, size: %d", __func__, event, sizeof(*av_src_close));
        maybe_non_aligned_memcpy(av_dest_close, av_src_close, sizeof(*av_src_close));
        break;
      }

    case BTA_AV_START_EVT : //4
      {
        tBTA_AV_START* av_src_start = (tBTA_AV_START*)p_src;
        tBTA_AV_START* av_dest_start = (tBTA_AV_START*)p_dest;
        BTIF_TRACE_DEBUG("%s: event: %d, size: %d", __func__, event, sizeof(*av_src_start));
        maybe_non_aligned_memcpy(av_dest_start, av_src_start, sizeof(*av_src_start));
        break;
      }

    case BTA_AV_PROTECT_REQ_EVT: //6
      {
        tBTA_AV_PROTECT_REQ* av_src_protect = (tBTA_AV_PROTECT_REQ*)p_src;
        tBTA_AV_PROTECT_REQ* av_dest_protect = (tBTA_AV_PROTECT_REQ*)p_dest;
        BTIF_TRACE_DEBUG("%s: event: %d, size: %d", __func__, event, sizeof(*av_src_protect));
        maybe_non_aligned_memcpy(av_dest_protect, av_src_protect, sizeof(*av_src_protect));
        break;
      }

    case BTA_AV_PROTECT_RSP_EVT: //7
      {
        tBTA_AV_PROTECT_RSP* av_src_potect_rsp = (tBTA_AV_PROTECT_RSP*)p_src;
        tBTA_AV_PROTECT_RSP* av_dest_potect_rsp = (tBTA_AV_PROTECT_RSP*)p_dest;
        BTIF_TRACE_DEBUG("%s: event: %d, size: %d", __func__, event, sizeof(*av_src_potect_rsp));
        maybe_non_aligned_memcpy(av_dest_potect_rsp, av_src_potect_rsp, sizeof(*av_src_potect_rsp));
        break;
      }

    case BTA_AV_RC_OPEN_EVT: //8
      {
        tBTA_AV_RC_OPEN* av_src_avrc_open = (tBTA_AV_RC_OPEN*)p_src;
        tBTA_AV_RC_OPEN* av_dest_avrc_open = (tBTA_AV_RC_OPEN*)p_dest;
        BTIF_TRACE_DEBUG("%s: event: %d, size: %d", __func__, event, sizeof(*av_src_avrc_open));
        maybe_non_aligned_memcpy(av_dest_avrc_open, av_src_avrc_open, sizeof(*av_src_avrc_open));
        break;
      }

    case BTA_AV_RC_CLOSE_EVT: //9
      {
        tBTA_AV_RC_CLOSE* av_src_avrc_close = (tBTA_AV_RC_CLOSE*)p_src;
        tBTA_AV_RC_CLOSE* av_dest_avrc_close = (tBTA_AV_RC_CLOSE*)p_dest;
        BTIF_TRACE_DEBUG("%s: event: %d, size: %d", __func__, event, sizeof(*av_src_avrc_close));
        maybe_non_aligned_memcpy(av_dest_avrc_close, av_src_avrc_close, sizeof(*av_src_avrc_close));
        break;
      }

    case BTA_AV_REMOTE_CMD_EVT: //10
      {
        tBTA_AV_REMOTE_CMD* av_src_remote_cmd = (tBTA_AV_REMOTE_CMD*)p_src;
        tBTA_AV_REMOTE_CMD* av_dest_remote_cmd = (tBTA_AV_REMOTE_CMD*)p_dest;
        BTIF_TRACE_DEBUG("%s: event: %d, size: %d", __func__, event, sizeof(*av_src_remote_cmd));
        maybe_non_aligned_memcpy(av_dest_remote_cmd, av_src_remote_cmd, sizeof(*av_src_remote_cmd));
        if (av_src_remote_cmd->p_data && av_src_remote_cmd->len) {
          BTIF_TRACE_DEBUG("%s:BTA_AV_REMOTE_CMD_EVT vendor cmd",__func__);
          av_dest_remote_cmd->p_data = (uint8_t *)osi_calloc(av_src_remote_cmd->len);
          memcpy(av_dest_remote_cmd->p_data, av_src_remote_cmd->p_data, av_src_remote_cmd->len);
        }
        break;
      }

    case BTA_AV_REMOTE_RSP_EVT: //11
      {
        tBTA_AV_REMOTE_RSP* av_src_remote_rsp = (tBTA_AV_REMOTE_RSP*)p_src;
        tBTA_AV_REMOTE_RSP* av_dest_remote_rsp = (tBTA_AV_REMOTE_RSP*)p_dest;
        BTIF_TRACE_DEBUG("%s: event: %d, size: %d", __func__, event, sizeof(*av_src_remote_rsp));
        maybe_non_aligned_memcpy(av_dest_remote_rsp, av_src_remote_rsp, sizeof(*av_src_remote_rsp));
        break;
      }

    case BTA_AV_VENDOR_CMD_EVT: //12 /* fall through */
    case BTA_AV_VENDOR_RSP_EVT: //13
      {
        tBTA_AV_VENDOR* av_src_vendor = (tBTA_AV_VENDOR*)p_src;
        tBTA_AV_VENDOR* av_dest_vendor = (tBTA_AV_VENDOR*)p_dest;
        BTIF_TRACE_DEBUG("%s: event: %d, size: %d", __func__, event, sizeof(*av_src_vendor));
        maybe_non_aligned_memcpy(av_dest_vendor, av_src_vendor, sizeof(*av_src_vendor));
        break;
      }

    case BTA_AV_RECONFIG_EVT: //14
      {
        tBTA_AV_RECONFIG* av_src_reconfig = (tBTA_AV_RECONFIG*)p_src;
        tBTA_AV_RECONFIG* av_dest_reconfig = (tBTA_AV_RECONFIG*)p_dest;
        BTIF_TRACE_DEBUG("%s: event: %d, size: %d", __func__, event, sizeof(*av_src_reconfig));
        maybe_non_aligned_memcpy(av_dest_reconfig, av_src_reconfig, sizeof(*av_src_reconfig));
        break;
      }

    case BTA_AV_STOP_EVT: //5 /* fall through */
    case BTA_AV_SUSPEND_EVT: //15
      {
        tBTA_AV_SUSPEND* av_src_suspend = (tBTA_AV_SUSPEND*)p_src;
        tBTA_AV_SUSPEND* av_dest_suspend = (tBTA_AV_SUSPEND*)p_dest;
        BTIF_TRACE_DEBUG("%s: event: %d, size: %d", __func__, event, sizeof(*av_src_suspend));
        maybe_non_aligned_memcpy(av_dest_suspend, av_src_suspend, sizeof(*av_src_suspend));
        break;
      }

    case BTA_AV_REJECT_EVT: //18
      {
        tBTA_AV_REJECT* av_src_rej = (tBTA_AV_REJECT*)p_src;
        tBTA_AV_REJECT* av_dest_rej = (tBTA_AV_REJECT*)p_dest;
        BTIF_TRACE_DEBUG("%s: event: %d, size: %d", __func__, event, sizeof(*av_src_rej));
        maybe_non_aligned_memcpy(av_dest_rej, av_src_rej, sizeof(*av_src_rej));
        break;
      }

    case BTA_AV_RC_FEAT_EVT: //19
      {
        tBTA_AV_RC_FEAT* av_src_rc_feat = (tBTA_AV_RC_FEAT*)p_src;
        tBTA_AV_RC_FEAT* av_dest_rc_feat = (tBTA_AV_RC_FEAT*)p_dest;
        BTIF_TRACE_DEBUG("%s: event: %d, size: %d", __func__, event, sizeof(*av_src_rc_feat));
        maybe_non_aligned_memcpy(av_dest_rc_feat, av_src_rc_feat, sizeof(*av_src_rc_feat));
        break;
      }

    case BTA_AV_RC_BROWSE_OPEN_EVT: //23
      {
        tBTA_AV_RC_BROWSE_OPEN* av_src_browse_open = (tBTA_AV_RC_BROWSE_OPEN*)p_src;
        tBTA_AV_RC_BROWSE_OPEN* av_dest_browse_open = (tBTA_AV_RC_BROWSE_OPEN*)p_dest;
        BTIF_TRACE_DEBUG("%s: event: %d, size: %d", __func__, event, sizeof(*av_src_browse_open));
        maybe_non_aligned_memcpy(av_dest_browse_open, av_src_browse_open, sizeof(*av_src_browse_open));
        break;
      }

    case BTA_AV_RC_BROWSE_CLOSE_EVT: //24
      {
        tBTA_AV_RC_BROWSE_CLOSE* av_src_browse_close = (tBTA_AV_RC_BROWSE_CLOSE*)p_src;
        tBTA_AV_RC_BROWSE_CLOSE* av_dest_browse_close = (tBTA_AV_RC_BROWSE_CLOSE*)p_dest;
        BTIF_TRACE_DEBUG("%s: event: %d, size: %d", __func__, event, sizeof(*av_src_browse_close));
        maybe_non_aligned_memcpy(av_dest_browse_close, av_src_browse_close, sizeof(*av_src_browse_close));
        break;
      }

    case BTA_AV_ROLE_CHANGED_EVT: //25
      {
        tBTA_AV_ROLE_CHANGED* av_src_role_changed = (tBTA_AV_ROLE_CHANGED*)p_src;
        tBTA_AV_ROLE_CHANGED* av_dest_role_changed = (tBTA_AV_ROLE_CHANGED*)p_dest;
        BTIF_TRACE_DEBUG("%s: event: %d, size: %d", __func__, event, sizeof(*av_src_role_changed));
        maybe_non_aligned_memcpy(av_dest_role_changed, av_src_role_changed,
                                                           sizeof(*av_src_role_changed));
        break;
      }

    case BTA_AV_DELAY_REPORT_EVT: //27
      {
        tBTA_AV_DELAY_RPT* av_src_delay_rpt = (tBTA_AV_DELAY_RPT*)p_src;
        tBTA_AV_DELAY_RPT* av_dest_delay_rpt = (tBTA_AV_DELAY_RPT*)p_dest;
        BTIF_TRACE_DEBUG("%s: event: %d, size: %d", __func__, event, sizeof(*av_src_delay_rpt));
        maybe_non_aligned_memcpy(av_dest_delay_rpt, av_src_delay_rpt, sizeof(*av_src_delay_rpt));
        break;
      }

    case BTA_AV_OFFLOAD_START_RSP_EVT: //22 /* fall through */
    case BTA_AV_OFFLOAD_STOP_RSP_EVT: //28
      {
         if (event == BTA_AV_OFFLOAD_START_RSP_EVT && !btif_a2dp_src_vsc.multi_vsc_support) {
           tBTA_AV_OFFLOAD_RSP* av_src_offload_start_or_stop_rsp = (tBTA_AV_OFFLOAD_RSP*)p_src;
           tBTA_AV_OFFLOAD_RSP* av_dest_offload_start_or_stop_rsp = (tBTA_AV_OFFLOAD_RSP*)p_dest;

           BTIF_TRACE_DEBUG("%s: event: %d, size: %d", __func__, event,
                                           sizeof(*av_src_offload_start_or_stop_rsp));
           maybe_non_aligned_memcpy(av_dest_offload_start_or_stop_rsp,
                   av_src_offload_start_or_stop_rsp, sizeof(*av_src_offload_start_or_stop_rsp));
           break;
         } else {
           tBTA_AV* av_src_offload_start_or_stop_rsp = (tBTA_AV*)p_src;
           tBTA_AV* av_dest_offload_start_or_stop_rsp = (tBTA_AV*)p_dest;
           BTIF_TRACE_DEBUG("%s: event: %d, size: %d", __func__, event, sizeof(uint8_t));
           maybe_non_aligned_memcpy(av_dest_offload_start_or_stop_rsp,
                                       av_src_offload_start_or_stop_rsp, sizeof(uint8_t));
           break;
         }
      }

      case BTA_AV_RC_COLL_DETECTED_EVT: //26
      {
         tBTA_AV_RC_COLL_DETECTED* av_src_av_rc_collision_detect =
                                                 (tBTA_AV_RC_COLL_DETECTED*)p_src;
         tBTA_AV_RC_COLL_DETECTED* av_dest_av_rc_collision_detect =
                                                 (tBTA_AV_RC_COLL_DETECTED*)p_dest;
         BTIF_TRACE_DEBUG("%s: event: %d, size: %d", __func__, event,
                                      sizeof(*av_src_av_rc_collision_detect));
         maybe_non_aligned_memcpy(av_dest_av_rc_collision_detect,
                                  av_src_av_rc_collision_detect,
                                  sizeof(*av_src_av_rc_collision_detect));
         break;
      }

    default:
      {
        tBTA_AV* av_src_default = (tBTA_AV*)p_src;
        tBTA_AV* av_dest_default = (tBTA_AV*)p_dest;
        BTIF_TRACE_DEBUG("%s: event: %d, size: %d", __func__, event, sizeof(*av_src_default));

        // First copy the structure
        maybe_non_aligned_memcpy(av_dest_default, av_src_default, sizeof(*av_src_default));
        break;
      }
  }
}

static void btif_av_event_free_data(btif_sm_event_t event, void* p_data) {
  BTIF_TRACE_DEBUG("%s: event: %d", __func__, event);
  switch (event) {
    case BTA_AV_META_MSG_EVT: {
      tBTA_AV* av = (tBTA_AV*)p_data;
      BTIF_TRACE_DEBUG("%s: len: %d, opcode: 0x%x",
                 __func__, av->meta_msg.len, av->meta_msg.p_msg->hdr.opcode);
      if (av->meta_msg.p_data && av->meta_msg.len) {
        osi_free_and_reset((void**)&av->meta_msg.p_data);
      }

      if (av->meta_msg.p_msg) {
        BTIF_TRACE_DEBUG("%s: meta_msg.p_msg is not null: vendor_len: %d, browse_len: %d ",
          __func__, av->meta_msg.p_msg->vendor.vendor_len, av->meta_msg.p_msg->browse.browse_len);
        if (av->meta_msg.p_msg->hdr.opcode == AVRC_OP_VENDOR &&
            av->meta_msg.p_msg->vendor.vendor_len) {
          osi_free(av->meta_msg.p_msg->vendor.p_vendor_data);
        }
        if (av->meta_msg.p_msg->hdr.opcode == AVRC_OP_BROWSE &&
            av->meta_msg.p_msg->browse.browse_len) {
          osi_free(av->meta_msg.p_msg->browse.p_browse_data);
        }
        osi_free_and_reset((void**)&av->meta_msg.p_msg);
      }
    } break;
    case BTA_AV_REMOTE_CMD_EVT: {
      tBTA_AV* av = (tBTA_AV*)p_data;
      if (av->remote_cmd.p_data && av->remote_cmd.len) {
        BTIF_TRACE_DEBUG("%s:free vendor passthrough data len = %d",__func__,av->remote_cmd.len);
        osi_free((void**)av->remote_cmd.p_data);
      }
    } break;
    default:
      break;
  }
}

static void bte_av_callback(tBTA_AV_EVT event, tBTA_AV* p_data) {
  BTIF_TRACE_DEBUG("%s: event: %d", __func__, event);
  btif_transfer_context(btif_av_handle_event, event, (char*)p_data,
                        sizeof(tBTA_AV), btif_av_event_deep_copy);
}

static void bte_av_sink_media_callback(tBTA_AV_EVT event,
                                       tBTA_AV_MEDIA* p_data, RawAddress bd_addr) {
  int index = btif_av_idx_by_bdaddr(&bd_addr);
  int cur_playing_index = btif_av_get_current_playing_dev_idx();
  BTIF_TRACE_DEBUG("%s: index = %d, cur_playing_index: %d",
                       __func__, index, cur_playing_index);

  switch (event) {
    case BTA_AV_SINK_MEDIA_DATA_EVT: {
      btif_sm_state_t state = btif_sm_get_state(btif_av_cb[index].sm_handle);
      if (((state == BTIF_AV_STATE_STARTED) || (state == BTIF_AV_STATE_OPENED)) &&
            ((index == cur_playing_index) && (cur_playing_index != btif_max_av_clients))) {
        uint8_t queue_len = btif_a2dp_sink_enqueue_buf((BT_HDR*)p_data);
        BTIF_TRACE_DEBUG("%s: index = %d, packets in sink queue %d", __func__, index, queue_len);
      }
      break;
    }
    case BTA_AV_SINK_MEDIA_CFG_EVT: {
      btif_av_sink_config_req_t config_req;

      /* send a command to BT Media Task */
      btif_a2dp_sink_update_decoder((uint8_t*)(p_data->avk_config.codec_info));
      /* Switch to BTIF context */
      config_req.sample_rate =
          A2DP_GetTrackSampleRate(p_data->avk_config.codec_info);
      if (config_req.sample_rate == -1) {
        APPL_TRACE_ERROR("%s: cannot get the track frequency", __func__);
        break;
      }
      config_req.channel_count =
          A2DP_GetTrackChannelCount(p_data->avk_config.codec_info);
      if (config_req.channel_count == -1) {
        APPL_TRACE_ERROR("%s: cannot get the channel count", __func__);
        break;
      }

      config_req.peer_bd = p_data->avk_config.bd_addr;
      btif_transfer_context(btif_av_handle_event, BTIF_AV_SINK_CONFIG_REQ_EVT,
                            (char*)&config_req, sizeof(config_req), NULL);
      break;
    }
    default:
      break;
  }
}

/*******************************************************************************
 *
 * Function         btif_av_init
 *
 * Description      Initializes btif AV if not already done
 *
 * Returns          bt_status_t
 *
 ******************************************************************************/

bt_status_t btif_av_init(int service_id) {
  if (btif_av_cb[0].sm_handle == NULL) {
    alarm_free(av_open_on_rc_timer);
    av_open_on_rc_timer = alarm_new("btif_av.av_open_on_rc_timer");

    BTIF_TRACE_DEBUG("%s; service Id: %d", __func__, service_id);

    btif_av_cb[0].service = service_id;
    switch (service_id) {
      case BTA_A2DP_SOURCE_SERVICE_ID:
        if (btif_a2dp_source_media_task_is_running())
          return BT_STATUS_FAIL;  // Already running
        if (!btif_a2dp_source_startup())
          return BT_STATUS_FAIL;  // Already running
        break;
      case BTA_A2DP_SINK_SERVICE_ID:
        btif_max_av_clients = btif_get_max_allowable_sink_connections();
        if (!btif_a2dp_sink_startup())
          return BT_STATUS_FAIL;  // Already running
        break;
      default:
        break;
    }

    /* Also initialize the AV state machine */
    for (int i = 0; i < btif_max_av_clients; i++) {
      btif_av_cb[i].sm_handle = btif_sm_init(
        (const btif_sm_handler_t*)btif_av_state_handlers,
        BTIF_AV_STATE_IDLE, i);
      btif_av_cb[i].suspend_rsp_track_timer =
                       alarm_new("btif_av.suspend_rsp_track_timer");
      alarm_free(collision_detect[i].av_coll_detected_timer);
      collision_detect[i].av_coll_detected_timer =
                         alarm_new("btif_av.av_coll_detected_timer");
      memset(&collision_detect[i].bd_addr, 0, sizeof(RawAddress));
      collision_detect[i].conn_retry_count = 1;
    }

    btif_transfer_context(btif_av_handle_event, BTIF_AV_INIT_REQ_EVT,
        (char*)&service_id, sizeof(int), NULL);

    btif_enable_service(service_id);
    if (BTA_A2DP_SOURCE_SERVICE_ID == service_id) {
      btif_enable_service(BTA_TWS_PLUS_SERVICE_ID);
    }
  }

  return BT_STATUS_SUCCESS;
}

/*******************************************************************************
 *
 * Function         init_src
 *
 * Description      Initializes the AV interface for source mode
 *
 * Returns          bt_status_t
 *
 ******************************************************************************/
static bt_status_t init_src(
    btav_source_callbacks_t* callbacks,
    std::vector<btav_a2dp_codec_config_t> codec_priorities,
    std::vector<btav_a2dp_codec_config_t> offload_enabled_codecs,
    int max_a2dp_connections, int a2dp_multicast_state) {
  bt_status_t status = BT_STATUS_FAIL;
  BTIF_TRACE_EVENT("%s() with max conn = %d", __func__, max_a2dp_connections);
  char value[PROPERTY_VALUE_MAX] = {'\0'};

  bt_split_a2dp_enabled = controller_get_interface()->supports_spilt_a2dp();
  BTIF_TRACE_DEBUG("split_a2dp_status = %d",bt_split_a2dp_enabled);
  osi_property_get("persist.vendor.btstack.twsplus.defaultchannelmode",
                                    value, "mono");
  BTIF_TRACE_DEBUG("tws default channel mode = %s",value);
  tws_defaultmono_supported = (strcmp(value, "mono") == 0);
  BTIF_TRACE_DEBUG("default mono channel mode = %d",tws_defaultmono_supported);
  offload_enabled_codecs_config_ = offload_enabled_codecs;

#if (TWS_STATE_ENABLED == TRUE)
  //osi_property_get("persist.vendor.btstack.twsplus.state", value, "false");
  tws_state_supported =
       controller_get_interface()->supports_twsp_remote_state();
#endif
  if (bt_av_sink_callbacks != NULL)
        // already did btif_av_init()
        status = BT_STATUS_SUCCESS;
  else {
    if (a2dp_multicast_state)
      is_multicast_supported = true;
    btif_max_av_clients = max_a2dp_connections;
    BTIF_TRACE_EVENT("%s() with max conn changed to = %d", __func__,
                                btif_max_av_clients);
    if (btif_av_is_split_a2dp_enabled()) {
      btif_a2dp_src_vsc.multi_vsc_support = false;
    }

    for (int i = 0; i < btif_max_av_clients; i++)
      btif_av_cb[i].codec_priorities = codec_priorities;
    if (codec_config_update_enabled != false) {
        BTIF_TRACE_IMP("%s: Codec cfg update enabled changed to false", __func__);
        codec_config_update_enabled = false;
    }
    status = btif_av_init(BTA_A2DP_SOURCE_SERVICE_ID);
    if (status == BT_STATUS_SUCCESS) bt_av_src_callbacks = callbacks;
  }
  return status;
}

static bt_status_t init_src(
    btav_source_callbacks_t* callbacks,
    int max_connected_audio_devices,
    std::vector<btav_a2dp_codec_config_t> codec_priorities,
    std::vector<btav_a2dp_codec_config_t> offload_enabled_codecs) {
  int a2dp_multicast_state = controller_get_interface()->is_multicast_enabled();
  if(max_connected_audio_devices > BTIF_AV_NUM_CB) {
    BTIF_TRACE_ERROR("%s: App setting maximum allowable connections(%d) \
              to more than limit(%d)",
            __func__, max_connected_audio_devices, BTIF_AV_NUM_CB);
    max_connected_audio_devices = BTIF_AV_NUM_CB;
  }
  for (int i = 0; i < max_connected_audio_devices; i++) {
    memset(&btif_av_cb[i], 0, sizeof(btif_av_cb[i]));
    btif_av_cb[i].codec_priorities = codec_priorities;
    btif_av_cb[i].state = BTIF_AV_STATE_IDLE;
    btif_av_cb[i].service = BTA_A2DP_SOURCE_SERVICE_ID;
    btif_av_cb[i].aptx_mode = APTX_HQ;
    btif_av_cb[i].remote_start_alarm = NULL;
    btif_av_cb[i].suspend_rsp_track_timer = NULL;
    btif_av_cb[i].fake_suspend_rsp = false;
    memset(&collision_detect[i].bd_addr, 0, sizeof(RawAddress));
    collision_detect[i].conn_retry_count = 1;
    collision_detect[i].av_coll_detected_timer = NULL;
  }
  return init_src(callbacks, codec_priorities, offload_enabled_codecs,
                max_connected_audio_devices, a2dp_multicast_state);
}


/*******************************************************************************
 *
 * Function         init_sink
 *
 * Description      Initializes the AV interface for sink mode
 *
 * Returns          bt_status_t
 *
 ******************************************************************************/

static bt_status_t init_sink(btav_sink_callbacks_t* callbacks) {
  BTIF_TRACE_EVENT("%s", __func__);

  bt_status_t status = btif_av_init(BTA_A2DP_SINK_SERVICE_ID);
  if (status == BT_STATUS_SUCCESS) bt_av_sink_callbacks = callbacks;

  return status;
}

/*******************************************************************************
 *
 * Function         update_audio_focus_state
 *
 * Description      Updates the final focus state reported by components calling
 *                  this module.
 *
 * Returns          None
 *
 ******************************************************************************/
static void update_audio_focus_state(int state) {
  BTIF_TRACE_DEBUG("%s: state=%d", __func__, state);
  btif_a2dp_sink_set_focus_state_req((btif_a2dp_sink_focus_state_t)state);
}

/*******************************************************************************
 *
 * Function         update_audio_track_gain
 *
 * Description      Updates the track gain (used for ducking).
 *
 * Returns          None
 *
 ******************************************************************************/
static void update_audio_track_gain(float gain) {
  BTIF_TRACE_DEBUG("%s: gain=%f", __func__, gain);
  btif_a2dp_sink_set_audio_track_gain(gain);
}

/*******************************************************************************
 *
 * Function         btif_get_latest_playing_device
 *
 * Description      Get the index for the most recent source
 *
 * Returns          None
 *
 ******************************************************************************/
void btif_get_latest_playing_device(RawAddress *address) {
  int index;
  index = btif_av_get_latest_playing_device_idx();
  if (index < btif_max_av_clients)
        //copy bdaddrsss
    *address = btif_av_cb[index].peer_bda;
  else
    *address = RawAddress::kEmpty;
}

/*******************************************************************************
 *
 * Function         btif_av_is_device_connected
 *
 * Description      Check if the peer is connected(A2DP)
 *
 * Returns          true/false
 *
 ******************************************************************************/
bool btif_av_is_device_connected(RawAddress address) {
  btif_sm_state_t state = btif_get_conn_state_of_device(address);

  if ((state == BTIF_AV_STATE_OPENED) ||
      (state == BTIF_AV_STATE_STARTED))
    return true;
  else
    return false;
}

void btif_av_set_browse_active(RawAddress peer_addr, uint8_t device_switch) {
  uint8_t peer_handle = btif_rc_get_connected_peer_handle(peer_addr);
  BTIF_TRACE_DEBUG("%s Trigger device switch on rc hdl %d", __func__, peer_handle);
  BTA_AvBrowseActive(peer_handle, peer_addr, device_switch);
}

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
void btif_av_trigger_dual_handoff(bool handoff, int current_active_index, int previous_active_index) {
  BTIF_TRACE_DEBUG("%s()", __func__);

  /*clear remote suspend flag unconditionally, which was set when one remote does suspend*/
  btif_av_clear_remote_suspend_flag();

  if (current_active_index == btif_max_av_clients) {
    BTIF_TRACE_ERROR("Handoff on invalid index");
    return;
  } else if ((previous_active_index != btif_max_av_clients) && (previous_active_index != INVALID_INDEX)){
    if (btif_av_current_device_is_tws()) {
      int tws_index;
      tws_index = btif_av_get_tws_pair_idx(previous_active_index);
      if (tws_index != btif_max_av_clients &&
        btif_sm_get_state(btif_av_cb[previous_active_index].sm_handle) != BTIF_AV_STATE_STARTED) {
        if (btif_sm_get_state(btif_av_cb[tws_index].sm_handle) == BTIF_AV_STATE_STARTED) {
          BTIF_TRACE_DEBUG("%s:Chaning previous_active_index from %d to %d",__func__,
                            previous_active_index, tws_index);
          previous_active_index = tws_index;
        }
      }
    }
    if(btif_sm_get_state(btif_av_cb[previous_active_index].sm_handle) == BTIF_AV_STATE_STARTED) {
      BTIF_TRACE_DEBUG("Initiate SUSPEND for this device on index = %d", previous_active_index);
      btif_av_cb[previous_active_index].dual_handoff = handoff; /*Initiate Handoff*/
      if (btif_av_cb[previous_active_index].tws_device) {
        for (int i = 0; i < btif_max_av_clients; i++) {
          if (i != previous_active_index && btif_av_cb[i].tws_device) btif_av_cb[i].dual_handoff = handoff;
        }
      }
      btif_sm_dispatch(btif_av_cb[previous_active_index].sm_handle, BTIF_AV_SUSPEND_STREAM_REQ_EVT, NULL);
      btif_av_set_suspend_rsp_track_timer(previous_active_index);
    }
  }
  if (btif_av_is_split_a2dp_enabled()) {
      if ((current_active_index != btif_max_av_clients) && (current_active_index != INVALID_INDEX)) {
        BTIF_TRACE_DEBUG("Not sending Reconfig update to audio");
        reconfig_a2dp = true;
      }
    }
}

/*******************************************************************************
 *
 * Function         btif_av_trigger_suspend
 *
 * Description      Trigger suspend when multicast is ongoing for tuch tones
 *                  and new ACL is created.
 *
 * Returns          void
 *
 ******************************************************************************/
void btif_av_trigger_suspend() {
  int index;
  /*Get the current playing device*/
  BTIF_TRACE_DEBUG("%s()", __func__);
  index = btif_av_get_latest_playing_device_idx();
  if (index <= btif_max_av_clients) {
    /* Initiate SUSPEND for this device */
    BTIF_TRACE_DEBUG("Initiate SUSPEND for this device on index = %d", index);
    btif_sm_dispatch(btif_av_cb[index].sm_handle, BTIF_AV_SUSPEND_STREAM_REQ_EVT, NULL);
  } else
    BTIF_TRACE_ERROR("suspend on invalid index");
}

/*******************************************************************************
 *
 * Function         connect_int
 *
 * Description      Establishes the AV signalling channel with the remote
 *                  headset
 *
 * Returns          bt_status_t
 *
 ******************************************************************************/
static bt_status_t connect_int(RawAddress* bd_addr, uint16_t uuid) {
  btif_av_connect_req_t connect_req;
  int i;
  connect_req.target_bda = bd_addr;
  connect_req.uuid = uuid;
  BTIF_TRACE_EVENT("%s", __func__);

  if (btif_storage_is_device_bonded(bd_addr) != BT_STATUS_SUCCESS)
  {
    BTIF_TRACE_WARNING("%s()## connect_int ## Device Not Bonded %s \n", __func__,
                      bd_addr->ToString().c_str());
    /* inform the application of the disconnection as the connection is not processed */
    btif_report_connection_state(BTAV_CONNECTION_STATE_DISCONNECTED, bd_addr);
    btif_queue_advance_by_uuid(uuid, bd_addr);
    return BT_STATUS_SUCCESS;
  }
  for (i = 0; i < btif_max_av_clients; i++) {
    if (btif_av_get_valid_idx(i)) {
      if (*bd_addr == btif_av_cb[i].peer_bda) {
        BTIF_TRACE_ERROR("Attempting connection for non idle device.. back off ");
        btif_queue_advance_by_uuid(uuid, bd_addr);
        return BT_STATUS_SUCCESS;
      }
    }
  }
  for (i = 0; i < btif_max_av_clients; i++) {
    if (!btif_av_get_valid_idx(i)) {
      BTIF_TRACE_DEBUG("%s: index %d is idle", __func__, i);
      break;
    }
  }

  if (i == btif_max_av_clients) {
    uint8_t rc_handle;

    BTIF_TRACE_ERROR("%s: All indexes are full", __func__);

    btif_report_connection_state(BTAV_CONNECTION_STATE_DISCONNECTED, bd_addr);

    /* Multicast: Check if AV slot is available for connection
     * If not available, AV got connected to different devices.
     * Disconnect this RC connection without AV connection.
     */
    rc_handle = btif_rc_get_connected_peer_handle(bd_addr->address);
    if (rc_handle != BTRC_HANDLE_NONE)
      BTA_AvCloseRc(rc_handle);
    btif_queue_advance_by_uuid(uuid, bd_addr);
    return BT_STATUS_FAIL;
  }

  btif_sm_dispatch(btif_av_cb[i].sm_handle, BTIF_AV_CONNECT_REQ_EVT, (char*)&connect_req);

  return BT_STATUS_SUCCESS;
}

static bt_status_t src_connect_sink(const RawAddress& bd_addr) {
  BTIF_TRACE_EVENT("%s", __func__);
  CHECK_BTAV_INIT();

  return btif_queue_connect(UUID_SERVCLASS_AUDIO_SOURCE, bd_addr, connect_int,
                    btif_max_av_clients);
}

static bt_status_t sink_connect_src(const RawAddress& bd_addr) {
  BTIF_TRACE_EVENT("%s", __func__);
  CHECK_BTAV_INIT();

  return btif_queue_connect(UUID_SERVCLASS_AUDIO_SINK, bd_addr, connect_int,
                    btif_max_av_clients);
}

/*******************************************************************************
 *
 * Function         disconnect
 *
 * Description      Tears down the AV signalling channel with the remote headset
 *
 * Returns          bt_status_t
 *
 ******************************************************************************/
static bt_status_t disconnect(const RawAddress& bd_addr) {
  BTIF_TRACE_EVENT("%s", __func__);
  CHECK_BTAV_INIT();

  /* Switch to BTIF context */
  return btif_transfer_context(btif_av_handle_event, BTIF_AV_DISCONNECT_REQ_EVT,
                               (char *)&bd_addr, sizeof(RawAddress), NULL);
}

/*******************************************************************************
 *
 * Function         set_silence_device
 *
 * Description      Sets the connected device silence state
 *
 * Returns          bt_status_t
 *
 ******************************************************************************/
static bt_status_t set_silence_device(const RawAddress& bd_addr, bool silence) {
  BTIF_TRACE_EVENT("%s silence = %d", __func__, silence);
  CHECK_BTAV_INIT();

  btif_av_silent_req_t silent_req;
  silent_req.bd_addr = bd_addr;
  silent_req.is_silent = silence;
  return btif_transfer_context(btif_av_handle_event, BTIF_AV_SET_SILENT_REQ_EVT,
                               (char *)&silent_req, sizeof(silent_req), NULL);
}

/*******************************************************************************
 *
 * Function         set_active_device
 *
 * Description      Tears down the AV signalling channel with the remote headset
 *
 * Returns          bt_status_t
 *
 ******************************************************************************/
static bt_status_t set_active_device(const RawAddress& bd_addr) {
  CHECK_BTAV_INIT();

  int active_index = btif_av_get_latest_device_idx_to_start();
  int set_active_device_index = btif_av_idx_by_bdaddr(&(RawAddress&)bd_addr);
  int tws_pair_index = btif_max_av_clients;
  BTIF_TRACE_EVENT("%s: active_index: %d, set_active_device_index: %d",
               __func__, active_index, set_active_device_index);

  if(set_active_device_index < btif_max_av_clients) {
    BTIF_TRACE_EVENT("%s: set_active_device_index flags: %d",
                 __func__, btif_av_cb[set_active_device_index].flags &
                      BTIF_AV_FLAG_LOCAL_SUSPEND_PENDING);
  }

  if (active_index < btif_max_av_clients && btif_av_cb[active_index].tws_device) {
    tws_pair_index = btif_av_get_tws_pair_idx(active_index);
  }
  if (active_index < btif_max_av_clients &&
      (((btif_av_cb[active_index].flags & BTIF_AV_FLAG_PENDING_START) ||
      ((set_active_device_index < btif_max_av_clients) &&
       btif_av_cb[set_active_device_index].flags & BTIF_AV_FLAG_LOCAL_SUSPEND_PENDING)) ||
      (btif_av_cb[active_index].tws_device && tws_pair_index < btif_max_av_clients &&
      (btif_av_cb[tws_pair_index].flags & BTIF_AV_FLAG_PENDING_START)))) {
    BTIF_TRACE_ERROR("%s: Pending Start/Suspend Response on current device, Return Fail",__func__);
    return BT_STATUS_NOT_READY;
  }

  if (!bta_av_co_set_active_peer(bd_addr)) {
    BTIF_TRACE_WARNING("%s: unable to set active peer in BtaAvCo",__func__);
  }
  if (btif_a2dp_source_is_hal_v2_supported()) {
    std::unique_lock<std::mutex> guard(session_wait_mutex_);
    session_wait = false;
    /* Initiate handoff for the device with address in the argument*/
    btif_transfer_context(btif_av_handle_event, BTIF_AV_TRIGGER_HANDOFF_REQ_EVT,
                                 (char *)&bd_addr, sizeof(RawAddress), NULL);
    BTIF_TRACE_EVENT("%s: wating for signal",__func__);
    session_wait_cv.wait_for(guard, std::chrono::milliseconds(1000),
                      []{return session_wait;});
    BTIF_TRACE_EVENT("%s: done with signal",__func__);
    if (!bd_addr.IsEmpty())
      btif_transfer_context(btif_av_handle_event, BTIF_AV_CHECK_PENDING_PLAY_EVT,
                                    (char *)&bd_addr, sizeof(RawAddress), NULL);
    return BT_STATUS_SUCCESS;
  } else {
    /* Initiate handoff for the device with address in the argument*/
    return btif_transfer_context(btif_av_handle_event,
    BTIF_AV_TRIGGER_HANDOFF_REQ_EVT,(char *)&bd_addr, sizeof(RawAddress), NULL);
  }
}

static bt_status_t codec_config_src(const RawAddress& bd_addr,
    std::vector<btav_a2dp_codec_config_t> codec_preferences) {
  BTIF_TRACE_EVENT("%s", __func__);
  CHECK_BTAV_INIT();
  //RawAddress *bda = &bda;
  int index = btif_av_idx_by_bdaddr(const_cast<RawAddress*>(&bd_addr));
#if (TWS_ENABLED == TRUE)
  //check if current device is TWS and then return failure with SHO support
  /*if (index < btif_max_av_clients && btif_av_cb[index].tws_device) {
  //if (btif_av_is_tws_connected()) {
    BTIF_TRACE_DEBUG("%s:TWSP device connected, config change not allowed",__func__);
    return BT_STATUS_FAIL;
  //}
  } */
#endif
  btif_av_codec_config_req_t codec_req;
  isDevUiReq = false;
  codec_cfg_change = false;
  for (auto cp : codec_preferences) {
    BTIF_TRACE_DEBUG(
        "%s: codec_type=%d codec_priority=%d "
        "sample_rate=0x%x bits_per_sample=0x%x "
        "channel_mode=0x%x codec_specific_1=%d "
        "codec_specific_2=%d codec_specific_3=%d "
        "codec_specific_4=%d",
        __func__, cp.codec_type, cp.codec_priority, cp.sample_rate,
        cp.bits_per_sample, cp.channel_mode, cp.codec_specific_1,
        cp.codec_specific_2, cp.codec_specific_3, cp.codec_specific_4);

    A2dpCodecConfig* current_codec = bta_av_get_a2dp_current_codec();

    if (current_codec != nullptr) {
      btav_a2dp_codec_config_t codec_config;
      codec_config = current_codec->getCodecConfig();
      isBitRateChange = false;
      if ((codec_config.codec_specific_1 != cp.codec_specific_1) &&
            (codec_config.codec_type == BTAV_A2DP_CODEC_INDEX_SOURCE_LDAC)) {
        isBitRateChange = true;
        switch (cp.codec_specific_1)
        {
          case 1000:
            if ((codec_config.sample_rate == BTAV_A2DP_CODEC_SAMPLE_RATE_44100) ||
                (codec_config.sample_rate == BTAV_A2DP_CODEC_SAMPLE_RATE_88200))
              reconfig_a2dp_param_val = 909000;
            else
              reconfig_a2dp_param_val = 990000;
            break;
          case 1001:
            if ((codec_config.sample_rate == BTAV_A2DP_CODEC_SAMPLE_RATE_44100) ||
                  (codec_config.sample_rate == BTAV_A2DP_CODEC_SAMPLE_RATE_88200))
              reconfig_a2dp_param_val = 606000;
            else
              reconfig_a2dp_param_val = 660000;
            break;
          case 1002:
            if ((codec_config.sample_rate == BTAV_A2DP_CODEC_SAMPLE_RATE_44100) ||
                (codec_config.sample_rate == BTAV_A2DP_CODEC_SAMPLE_RATE_88200))
              reconfig_a2dp_param_val = 303000;
            else
              reconfig_a2dp_param_val = 330000;
            break;
          case 1003:
            reconfig_a2dp_param_val = 0;
            break;
        }
        if (cp.codec_specific_1 != 0) {
          reconfig_a2dp_param_id = BITRATE_PARAM_ID;
        }
      } else if ((codec_config.bits_per_sample != cp.bits_per_sample) &&
               (codec_config.codec_type == BTAV_A2DP_CODEC_INDEX_SOURCE_LDAC)) {
          switch (cp.bits_per_sample)
          {
            case BTAV_A2DP_CODEC_BITS_PER_SAMPLE_16:
              reconfig_a2dp_param_val = 16;
              break;
            case BTAV_A2DP_CODEC_BITS_PER_SAMPLE_24:
              reconfig_a2dp_param_val = 24;
              break;
            case BTAV_A2DP_CODEC_BITS_PER_SAMPLE_32:
              reconfig_a2dp_param_val = 32;
              break;
            case BTAV_A2DP_CODEC_BITS_PER_SAMPLE_NONE:
              break;

          }
          if ((cp.bits_per_sample != 0) && (codec_config.bits_per_sample != 0)) {
            reconfig_a2dp_param_id = BITSPERSAMPLE_PARAM_ID;
            isBitsPerSampleChange = true;
          } else {
            codec_cfg_change = true;
          }
        } else {
          codec_cfg_change = true;
        }
    }

    if(cp.codec_specific_4 > 0 &&
          cp.codec_type == BTAV_A2DP_CODEC_INDEX_SOURCE_APTX_ADAPTIVE) {
          codec_cfg_change = false;
      BTIF_TRACE_WARNING("%s: Reset codec_cfg_change for Aptx mode change call", __func__);
    }

    if (index < btif_max_av_clients && btif_av_cb[index].reconfig_pending && codec_cfg_change) {
      BTIF_TRACE_ERROR("%s:Reconfig Pending, dishonor codec switch",__func__);
      return BT_STATUS_FAIL;
    }

    isDevUiReq = true;
    if (!codec_bda.IsEmpty())
      BTIF_TRACE_DEBUG("%s: previous codec_bda: %s", __func__, codec_bda.ToString().c_str());

    codec_bda = bd_addr;
    BTIF_TRACE_DEBUG("%s: current codec_bda: %s", __func__, codec_bda.ToString().c_str());
    memcpy(&codec_req.bd_addr, (uint8_t *)&bd_addr, sizeof(RawAddress));
    memcpy(&codec_req.codec_config, (char*)(&cp), sizeof(btav_a2dp_codec_config_t));
    btif_transfer_context(btif_av_handle_event, BTIF_AV_SOURCE_CONFIG_REQ_EVT,
                            (char *)&codec_req, sizeof(codec_req), NULL);
  }

  return BT_STATUS_SUCCESS;
}


/*******************************************************************************
 *
 * Function         cleanup
 *
 * Description      Shuts down the AV interface and does the cleanup
 *
 * Returns          None
 *
 ******************************************************************************/
static void cleanup(int service_uuid) {
  BTIF_TRACE_EVENT("%s", __func__);

  btif_transfer_context(btif_av_handle_event, BTIF_AV_CLEANUP_REQ_EVT,
                        (char*)&service_uuid, sizeof(int), NULL);

  btif_disable_service(service_uuid);

  btif_disable_service(BTA_TWS_PLUS_SERVICE_ID);

  for (int i = 0; i < btif_max_av_clients; i++) {
    btif_av_clear_suspend_rsp_track_timer(i);
    collision_detect[i].conn_retry_count = 1;
    alarm_free(collision_detect[i].av_coll_detected_timer);
    collision_detect[i].av_coll_detected_timer = NULL;
    memset(&collision_detect[i].bd_addr, 0, sizeof(RawAddress));
  }

  alarm_free(av_open_on_rc_timer);
  av_open_on_rc_timer = NULL;
}

static void cleanup_src(void) {
  BTIF_TRACE_EVENT("%s:", __func__);

  btif_queue_cleanup(UUID_SERVCLASS_AUDIO_SOURCE);
  if (bt_av_sink_callbacks == NULL) cleanup(BTA_A2DP_SOURCE_SERVICE_ID);
}

static void cleanup_sink(void) {
  BTIF_TRACE_EVENT("%s:", __func__);

  btif_queue_cleanup(UUID_SERVCLASS_AUDIO_SINK);
  if (bt_av_src_callbacks == NULL) cleanup(BTA_A2DP_SINK_SERVICE_ID);
}

/*******************************************************************************
 *
 * Function         allow_connection
 *
 * Description      Shuts down the AV interface and does the cleanup
 *
 * Returns          None
 *
 ******************************************************************************/
static void allow_connection(int is_valid, RawAddress *bd_addr)
{
  int index = 0;

  if (*bd_addr == RawAddress::kEmpty) {
    BTIF_TRACE_ERROR("%s: bd_address is empty, return", __func__);
    return;
  }

  BTIF_TRACE_DEBUG(" %s() isValid is %d event %d", __func__,is_valid,idle_rc_event);
  switch (idle_rc_event) {
    case BTA_AV_RC_OPEN_EVT:
      if (is_valid) {
        BTIF_TRACE_DEBUG("allowconn for RC connection");
        alarm_set_on_mloop(av_open_on_rc_timer,
                           BTIF_TIMEOUT_AV_OPEN_ON_RC_MS,
                           btif_initiate_av_open_timer_timeout, bd_addr);
          btif_rc_handler(idle_rc_event, (tBTA_AV*)&idle_rc_data);
      } else {
        uint8_t rc_handle =  idle_rc_data.rc_open.rc_handle;
        BTA_AvCloseRc(rc_handle);
      }
      break;

   case BTA_AV_PENDING_EVT:
       index = btif_av_idx_by_bdaddr(bd_addr);
       if (index >= btif_max_av_clients) {
         BTIF_TRACE_DEBUG("Invalid index for device");
         break;
       }
       if (is_valid) {
       BTIF_TRACE_DEBUG("The connection is allowed for the device at index = %d", index);
#if (TWS_ENABLED == TRUE)
       btif_av_cb[index].tws_device = BTM_SecIsTwsPlusDev(btif_av_cb[index].peer_bda.address);
       BTA_AvUpdateTWSDevice(btif_av_cb[index].tws_device, btif_av_cb[index].bta_handle);
#endif
       BTA_AvOpen(btif_av_cb[index].peer_bda, btif_av_cb[index].bta_handle,
                     true, BTA_SEC_AUTHENTICATE, UUID_SERVCLASS_AUDIO_SOURCE);
       } else {
         BTIF_TRACE_IMP("Reject incoming AV connection on Index %d", index);
         btif_report_connection_state(BTAV_CONNECTION_STATE_DISCONNECTED,
             &(btif_av_cb[index].peer_bda));
         BTA_AvClose(btif_av_cb[index].bta_handle);
         btif_sm_change_state(btif_av_cb[index].sm_handle, BTIF_AV_STATE_IDLE);
       }
   break;

   default:
     BTIF_TRACE_DEBUG("%s : unhandled event:%s", __FUNCTION__,
                              dump_av_sm_event_name((btif_av_sm_event_t)idle_rc_event));
  }
  idle_rc_event = 0;
  memset(&idle_rc_data, 0, sizeof(tBTA_AV));
}

#ifdef BT_AV_SHO_FEATURE
static bt_status_t select_audio_device(RawAddress *bd_addr)
{
    BTIF_TRACE_EVENT("%s", __FUNCTION__);
    CHECK_BTAV_INIT();

    /* Switch to BTIF context */
    return btif_transfer_context(btif_av_handle_event,
            BTIF_AV_TRIGGER_HANDOFF_REQ_EVT,
            (char*)bd_addr, sizeof(RawAddress), NULL);
}
#endif

static const btav_source_interface_t bt_av_src_interface = {
    sizeof(btav_source_interface_t),
    init_src,
    src_connect_sink,
    disconnect,
    set_silence_device,
    set_active_device,
    codec_config_src,
    cleanup_src,
#ifdef BT_AV_SHO_FEATURE
    allow_connection,
    select_audio_device,
#endif
};

static const btav_sink_interface_t bt_av_sink_interface = {
    sizeof(btav_sink_interface_t),
    init_sink,
    sink_connect_src,
    disconnect,
    cleanup_sink,
    update_audio_focus_state,
    update_audio_track_gain,
};

RawAddress btif_av_get_addr_by_index(int idx) {
  return (idx < btif_max_av_clients) ? btif_av_cb[idx].peer_bda:RawAddress::kEmpty;
}

/*******************************************************************************
 *
 * Function         btif_av_get_addr
 *
 * Description      Fetches current AV BD address
 *
 * Returns          BD address
 *
 ******************************************************************************/
RawAddress btif_av_get_addr(RawAddress address) {
  int i;
  RawAddress not_found = RawAddress::kEmpty;
  for (i = 0; i < btif_max_av_clients; i++)
    if (btif_av_cb[i].peer_bda == address)
      return btif_av_cb[i].peer_bda;
  return not_found;
}

/******************************************************************************
 *
 * Function         btif_av_get_peer_addr
 *
 * Description      Returns peer device address
 *
 * Returns          void. Updates the peer_bda argument
 *******************************************************************************/
void btif_av_get_active_peer_addr(RawAddress* peer_bda) {
  // Find the peer that is currently Active.
  tBTA_AV_CO_PEER* p_peer = nullptr;
  p_peer = bta_av_co_get_active_peer();
  if (p_peer == nullptr) {
    APPL_TRACE_ERROR("%s: no active peer device found", __func__);
    return;
  }
  BTIF_TRACE_DEBUG("%s: active peer_add: %s ", __func__, p_peer->addr.ToString().c_str());
  *peer_bda = p_peer->addr;
}

/*******************************************************************************
 * Function         btif_av_is_sink_enabled
 *
 * Description      Checks if A2DP Sink is enabled or not
 *
 * Returns          true if A2DP Sink is enabled, false otherwise
 *
 ******************************************************************************/

bool btif_av_is_sink_enabled(void) {
  return (bt_av_sink_callbacks != NULL) ? true : false;
}

/*******************************************************************************
 *
 * Function         btif_av_stream_ready
 *
 * Description      Checks whether AV is ready for starting a stream
 *
 * Returns          None
 *
 ******************************************************************************/
bool btif_av_stream_ready(void) {
  int i;
  bool status = false;

  /* also make sure main adapter is enabled */
  if (btif_is_enabled() == 0) {
    BTIF_TRACE_EVENT("%s: main adapter not enabled", __func__);
    return false;
  }

  for (i = 0; i < btif_max_av_clients; i++) {
    btif_av_cb[i].state = btif_sm_get_state(btif_av_cb[i].sm_handle);
    BTIF_TRACE_DEBUG("%s : sm hdl %d, state %d, flags %x, handoff %d", __func__,
                   btif_av_cb[i].sm_handle, btif_av_cb[i].state,
                   btif_av_cb[i].flags, btif_av_cb[i].dual_handoff);
    /* Multicast:
     * If any of the stream is in pending suspend state when
     * we initiate start, it will result in inconsistent behavior
     * Check the pending SUSPEND flag and return failure
     * if suspend is in progress.
     */
    if (btif_av_cb[i].dual_handoff ||
        (btif_av_cb[i].flags & BTIF_AV_FLAG_LOCAL_SUSPEND_PENDING)) {
      status = false;
      break;
    } else if (btif_av_cb[i].flags &
        (BTIF_AV_FLAG_REMOTE_SUSPEND|BTIF_AV_FLAG_PENDING_STOP|BTIF_AV_FLAG_PENDING_DISCONNECT)) {
      status = false;
      break;
    } else if (btif_av_cb[i].state == BTIF_AV_STATE_OPENED)
      status = true;
  }
  BTIF_TRACE_DEBUG("%s(): %d", __func__, status);
  return status;
}


/*******************************************************************************
 *
 * Function         btif_av_clear_remote_start_timer
 *
 * Description      Clear latest av start timer
 *
 * Returns          bool
 *
 ******************************************************************************/
void  btif_av_clear_remote_start_timer(int index) {
  BTIF_TRACE_DEBUG("%s: index: %d", __func__, index);
  if (index < btif_max_av_clients && index >= 0) {
    if (btif_av_cb[index].remote_start_alarm != NULL &&
             btif_av_cb[index].remote_started)
      alarm_free(btif_av_cb[index].remote_start_alarm);
      btif_av_cb[index].remote_started = false;
      btif_av_cb[index].remote_start_alarm = NULL;
  }
}


/*******************************************************************************
 *
 * Function         btif_av_stream_started_ready
 *
 * Description      Checks whether AV ready for media start in streaming state
 *
 * Returns          bool
 *
 ******************************************************************************/

bool btif_av_stream_started_ready(void)
{
  int i;
  bool status = false;

  for (i = 0; i < btif_max_av_clients; i++) {
    btif_av_cb[i].state = btif_sm_get_state(btif_av_cb[i].sm_handle);
    if (btif_av_cb[i].dual_handoff) {
      BTIF_TRACE_ERROR("%s: Under Dual handoff ",__func__ );
      status = false;
      break;
    } else if (btif_av_cb[i].flags &
               (BTIF_AV_FLAG_LOCAL_SUSPEND_PENDING |
                BTIF_AV_FLAG_REMOTE_SUSPEND |
                BTIF_AV_FLAG_PENDING_STOP)) {
      status = false;
      break;
    } else if (btif_av_cb[i].state == BTIF_AV_STATE_STARTED)
      status = true;
  }
  BTIF_TRACE_DEBUG("btif_av_stream_started_ready: %d", status);
  return status;
}

/*******************************************************************************
**
** Function         btif_av_is_start_ack_pending
**
** Description      Checks whether start command is sent but not acked by remote
**
** Returns          None
**
*******************************************************************************/

bool btif_av_is_start_ack_pending(void)
{
    int i;
    bool status = false;

    for (i = 0; i < btif_max_av_clients; i++)
    {
        if (btif_av_cb[i].flags & BTIF_AV_FLAG_PENDING_START)
        {
            status = true;
            break;
        }
    }
    BTIF_TRACE_DEBUG("btif_av_is_start_ack_pending: %d", status);
    return status;
}

/*******************************************************************************
**
** Function         btif_av_is_suspend_stop_pending_ack
**
** Description      Checks whether suspend/stop is pending ack when state is started
**
** Returns          None
**
*******************************************************************************/
bool btif_av_is_suspend_stop_pending_ack(void)
{
    int i;
    bool status = FALSE;

    for (i = 0; i < btif_max_av_clients; i++)
    {
        btif_av_cb[i].state = btif_sm_get_state(btif_av_cb[i].sm_handle);
        BTIF_TRACE_DEBUG("btif_av_stream_ready flags: %d, state: %d",
                                btif_av_cb[i].flags, btif_av_cb[i].state);
        if ((btif_av_cb[i].flags &
            (BTIF_AV_FLAG_LOCAL_SUSPEND_PENDING |
            BTIF_AV_FLAG_PENDING_STOP)) &&
            (btif_av_cb[i].state == BTIF_AV_STATE_STARTED)) {
          status = true;
          break;
        }
    }
    BTIF_TRACE_DEBUG("btif_av_is_stream_suspend_pending_ack: %d", status);
    return status;
}

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
void btif_dispatch_sm_event(btif_av_sm_event_t event, void* p_data, int len) {
  /* Switch to BTIF context */
  BTIF_TRACE_IMP("%s: event: %d, len: %d", __FUNCTION__, event, len);
  btif_transfer_context(btif_av_handle_event, event, (char*)p_data, len, NULL);
  BTIF_TRACE_IMP("%s: event %d sent", __FUNCTION__, event);
}

/*******************************************************************************
 *
 * Function         btif_av_execute_service
 *
 * Description      Initializes/Shuts down the service
 *
 * Returns          BT_STATUS_SUCCESS on success, BT_STATUS_FAIL otherwise
 *
 ******************************************************************************/
bt_status_t btif_av_execute_service(bool b_enable) {
  int i;
  btif_sm_state_t state;
  bool delay_report_enabled = false;
  char value[PROPERTY_VALUE_MAX] = {'\0'};
  tBTA_AV_FEAT feat_delay_rpt = 0;
  char a2dp_role[255] = "false";
  tA2DP_CTRL_CMD pending_cmd = A2DP_CTRL_CMD_NONE;
  osi_property_get("persist.vendor.service.bt.a2dp.sink", a2dp_role, "false");
  BTIF_TRACE_DEBUG("%s(): enable: %d", __func__, b_enable);
  if (b_enable) {
    osi_property_get("persist.bluetooth.disabledelayreports", value, "false");
    delay_report_enabled = (strcmp(value, "false") == 0);
    if (delay_report_enabled)
      feat_delay_rpt = BTA_AV_FEAT_DELAY_RPT;

    /* TODO: Removed BTA_SEC_AUTHORIZE since the Java/App does not
     * handle this request in order to allow incoming connections to succeed.
     * We need to put this back once support for this is added
     */
    /* Added BTA_AV_FEAT_NO_SCO_SSPD - this ensures that the BTA does not
     * auto-suspend av streaming on AG events(SCO or Call). The suspend shall
     * be initiated by the app/audioflinger layers
     */
#if (AVRC_METADATA_INCLUDED == true)
    BTA_AvEnable(BTA_SEC_AUTHENTICATE,
      BTA_AV_FEAT_RCTG|BTA_AV_FEAT_METADATA|BTA_AV_FEAT_VENDOR|BTA_AV_FEAT_NO_SCO_SSPD
      |BTA_AV_FEAT_ACP_START|feat_delay_rpt
#if (AVRC_ADV_CTRL_INCLUDED == true)
      |BTA_AV_FEAT_RCCT
      |BTA_AV_FEAT_ADV_CTRL
      |BTA_AV_FEAT_BROWSE
#endif
      , bte_av_callback);
#else
    BTA_AvEnable(BTA_SEC_AUTHENTICATE, (BTA_AV_FEAT_RCTG | BTA_AV_FEAT_NO_SCO_SSPD
                 |BTA_AV_FEAT_ACP_START | feat_delay_rpt), bte_av_callback);
#endif

    for (i = 0; i < btif_max_av_clients; i++) {
      BTIF_TRACE_DEBUG("%s: BTA_AvRegister : %d", __FUNCTION__, i);
      BTA_AvRegister(BTA_AV_CHNL_AUDIO, BTIF_AV_SERVICE_NAME, 0,
                     bte_av_sink_media_callback,
      UUID_SERVCLASS_AUDIO_SOURCE);
    }
    BTA_AvUpdateMaxAVClient(btif_max_av_clients);
  } else {
    /* Also shut down the AV state machine */
    for (i = 0; i < btif_max_av_clients; i++ ) {
      if (btif_av_cb[i].sm_handle != NULL) {
        state = btif_sm_get_state(btif_av_cb[i].sm_handle);
        BTIF_TRACE_DEBUG("BT is shutting down, state=%d", state);
        if ((state == BTIF_AV_STATE_OPENING) || (state == BTIF_AV_STATE_OPENED) ||
            (state == BTIF_AV_STATE_STARTED)) {
          BTIF_TRACE_DEBUG("Moving State from opened/started to Idle due to BT ShutDown");
          if (btif_av_is_split_a2dp_enabled() &&
             !btif_a2dp_source_is_hal_v2_supported()) {
            btif_a2dp_audio_interface_deinit();
            btif_a2dp_audio_if_init = false;
          } else {
            if (btif_a2dp_source_is_hal_v2_supported()) {
              pending_cmd =  bluetooth::audio::a2dp::get_pending_command();
            } else {
              pending_cmd = btif_a2dp_control_get_pending_command();
            }
            BTIF_TRACE_DEBUG("%s: a2dp-ctrl-cmd : %s", __func__,
                                     audio_a2dp_hw_dump_ctrl_event(pending_cmd));
            if (pending_cmd) {
              if(btif_a2dp_source_is_hal_v2_supported()) {
                btif_a2dp_source_command_ack(pending_cmd,
                            A2DP_CTRL_ACK_DISCONNECT_IN_PROGRESS);
              } else {
                btif_a2dp_command_ack(A2DP_CTRL_ACK_FAILURE);
              }
            }
            btif_a2dp_source_on_stopped(NULL);
            btif_sm_change_state(btif_av_cb[i].sm_handle, BTIF_AV_STATE_IDLE);
            if (!strncmp("false", a2dp_role, 5)) {
              btif_queue_advance_by_uuid(UUID_SERVCLASS_AUDIO_SOURCE, &(btif_av_cb[i].peer_bda));
            } else {
              btif_queue_advance_by_uuid(UUID_SERVCLASS_AUDIO_SINK, &(btif_av_cb[i].peer_bda));
            }
          }
        }
        btif_sm_shutdown(btif_av_cb[i].sm_handle);
        btif_av_cb[i].sm_handle = NULL;
      }
    }
    for (i = 0; i < btif_max_av_clients; i++)
      BTA_AvDeregister(btif_av_cb[i].bta_handle);
    BTA_AvDisable();
  }
  BTIF_TRACE_DEBUG("%s: enable: %d completed", __func__, b_enable);
  return BT_STATUS_SUCCESS;
}

/*******************************************************************************
 *
 * Function         btif_av_sink_execute_service
 *
 * Description      Initializes/Shuts down the service
 *
 * Returns          BT_STATUS_SUCCESS on success, BT_STATUS_FAIL otherwise
 *
 ******************************************************************************/
bt_status_t btif_av_sink_execute_service(bool b_enable) {
  int i;
  BTIF_TRACE_DEBUG("%s(): enable: %d", __func__, b_enable);

  bool delay_report_enabled = false;
  char value[PROPERTY_VALUE_MAX] = {'\0'};
  tBTA_AV_FEAT feat_delay_rpt = 0;

  if (b_enable) {
    /* Added BTA_AV_FEAT_NO_SCO_SSPD - this ensures that the BTA does not
     * auto-suspend av streaming on AG events(SCO or Call). The suspend shall
     * be initiated by the app/audioflinger layers
     */

    osi_property_get("persist.vendor.bt.a2dp.delay_report_sink", value, "false");
    delay_report_enabled = (strcmp(value, "false") == 0);
    BTIF_TRACE_DEBUG("%s: delay_report_enabled = %d", __func__, delay_report_enabled);
    if (delay_report_enabled)
      feat_delay_rpt = BTA_AV_FEAT_DELAY_RPT;

    BTA_AvEnable(BTA_SEC_AUTHENTICATE, BTA_AV_FEAT_NO_SCO_SSPD|BTA_AV_FEAT_RCCT|
                                        BTA_AV_FEAT_METADATA|BTA_AV_FEAT_VENDOR|
                                        BTA_AV_FEAT_ADV_CTRL|BTA_AV_FEAT_RCTG|feat_delay_rpt,
                                        bte_av_callback);

    for (i = 0; i < btif_max_av_clients; i++) {
        BTA_AvRegister(BTA_AV_CHNL_AUDIO, BTIF_AVK_SERVICE_NAME, 0,
                bte_av_sink_media_callback, UUID_SERVCLASS_AUDIO_SINK);
    }
  } else {
    /* Also shut down the AV state machine */
    for (i = 0; i < btif_max_av_clients; i++ ) {
      if (btif_av_cb[i].sm_handle != NULL) {
        BTIF_TRACE_DEBUG("%s(): shutting down AV SM", __func__);
        btif_sm_shutdown(btif_av_cb[i].sm_handle);
        btif_av_cb[i].sm_handle = NULL;
      }
    }
    BTA_AvDeregister(btif_av_cb[0].bta_handle);
    BTA_AvDisable();
  }

  return BT_STATUS_SUCCESS;
}

/*******************************************************************************
 *
 * Function         btif_av_get_src_interface
 *
 * Description      Get the AV callback interface for A2DP source profile
 *
 * Returns          btav_source_interface_t
 *
 ******************************************************************************/
const btav_source_interface_t* btif_av_get_src_interface(void) {
  BTIF_TRACE_EVENT("%s", __func__);
  return &bt_av_src_interface;
}

/*******************************************************************************
 *
 * Function         btif_av_get_sink_interface
 *
 * Description      Get the AV callback interface for A2DP sink profile
 *
 * Returns          btav_sink_interface_t
 *
 ******************************************************************************/
const btav_sink_interface_t* btif_av_get_sink_interface(void) {
  BTIF_TRACE_EVENT("%s", __func__);
  return &bt_av_sink_interface;
}

/*******************************************************************************
 *
 * Function         btif_av_is_connected
 *
 * Description      Checks if av has a connected sink
 *
 * Returns          bool
 *
 ******************************************************************************/
bool btif_av_is_connected(void) {
  int i;
  bool status = false;
  for (i = 0; i < btif_max_av_clients; i++) {
    btif_av_cb[i].state = btif_sm_get_state(btif_av_cb[i].sm_handle);
    if ((btif_av_cb[i].state == BTIF_AV_STATE_OPENED) ||
        (btif_av_cb[i].state ==  BTIF_AV_STATE_STARTED)) {
      status = true;
      break;
    }
  }
  return status;
}

bool isBATEnabled()
{
    bool ret = false;
    if (btif_ba_get_state() > BTIF_BA_STATE_IDLE_AUDIO_NS)
        ret = true;
    else
        ret = false;
    LOG_INFO(LOG_TAG,"%s: %d",__func__, ret);
    return ret;
}

void initialize_audio_hidl() {
  if (!btif_a2dp_source_is_hal_v2_supported()) {
    BTIF_TRACE_DEBUG(" %s if_init = %d ",__func__, btif_a2dp_audio_if_init);
    if (btif_a2dp_audio_if_init == false) {
      if (btif_av_is_split_a2dp_enabled()) {
        BTIF_TRACE_DEBUG("%s initializing ", __func__);
        btif_a2dp_audio_if_init = true;
        btif_a2dp_audio_interface_init();
      } else {
        BTIF_TRACE_DEBUG("%s cannot initialize, split not enabled ",__func__);
      }
    } else {
      BTIF_TRACE_DEBUG("%s already initialized ", __func__);
    }
  } else {
    int index = btif_av_get_latest_device_idx_to_start();
    //if index < btif_max_av_clients, AV -> BA switch, else null -> BA switch
    btif_a2dp_source_restart_session(btif_av_get_addr_by_index(index), ba_addr);
  }
}

void deinit_audio_hal() {
  if (!btif_a2dp_source_is_hal_v2_supported()) {
    BTIF_TRACE_DEBUG(" %s if_init = %d ",__func__, btif_a2dp_audio_if_init);
    BTIF_TRACE_DEBUG(" %s split_enabled = %d av_connected = %d",__func__,
                    btif_av_is_split_a2dp_enabled(), btif_av_is_connected());
    if (btif_a2dp_audio_if_init == false)
      return;
    if (!btif_av_is_split_a2dp_enabled())
      return;
    // if av is connected, it would need interface active
    if (btif_av_is_connected())
      return;
    btif_a2dp_audio_if_init =  false;
    btif_a2dp_audio_interface_deinit();
  } else {
    int index = btif_av_get_latest_device_idx_to_start();
    if(index < btif_max_av_clients) {
      //restart session BA -> AV switch
      btif_a2dp_source_restart_session(ba_addr, btif_av_get_addr_by_index(index));
    } else {
      //end session BA -> null
      btif_a2dp_source_end_session(ba_addr);
    }
  }
}

/*******************************************************************************
**
** Function         btif_av_is_connected_on_other_idx
**
** Description      Checks if any other AV SCB is connected
**
** Returns          bool
**
*******************************************************************************/
bool btif_av_is_connected_on_other_idx(int current_index) {
  //return true if other IDx is connected
  btif_sm_state_t state = BTIF_AV_STATE_IDLE;
  int i;
  for (i = 0; i < btif_max_av_clients; i++) {
    if (i != current_index) {
      state = btif_sm_get_state(btif_av_cb[i].sm_handle);
      if ((state == BTIF_AV_STATE_OPENED) ||
          (state == BTIF_AV_STATE_STARTED))
        return true;
    }
  }
  return false;
}

/*******************************************************************************
**
** Function         btif_av_get_other_connected_idx
**
** Description      Checks if any AV SCB is connected other than the current
**                  index
**
** Returns          index
**
*******************************************************************************/
int btif_av_get_other_connected_idx(int current_index)
{
  btif_sm_state_t state = BTIF_AV_STATE_IDLE;
  int i;
  // return true if other IDx is connected
  for (i = 0; i < btif_max_av_clients; i++) {
    if (i != current_index) {
      state = btif_sm_get_state(btif_av_cb[i].sm_handle);
      if ((state == BTIF_AV_STATE_OPENED) ||
          (state == BTIF_AV_STATE_STARTED))
        return i;
    }
  }
  return INVALID_INDEX;
}

/*******************************************************************************
**
** Function         btif_av_is_playing_on_other_idx
**
** Description      Checks if any other AV SCB is connected
**
** Returns          bool
**
*******************************************************************************/
bool btif_av_is_playing_on_other_idx(int current_index)
{
  btif_sm_state_t state = BTIF_AV_STATE_IDLE;
  int i;
  // return true if other IDx is playing
  for (i = 0; i < btif_max_av_clients; i++) {
    if (i != current_index) {
      state = btif_sm_get_state(btif_av_cb[i].sm_handle);
      if (state == BTIF_AV_STATE_STARTED)
        return true;
    }
  }
  return false;
}

bool btif_av_is_peer_silenced(RawAddress *bd_addr)
{
  int index = btif_av_idx_by_bdaddr(bd_addr);
  if (index == btif_max_av_clients) {
    return false;
  }
  return btif_av_cb[index].is_silenced;
}

/*******************************************************************************
**
** Function         btif_av_is_local_started_on_other_idx
**
** Description      Checks if any other AV SCB is connected and
**                  remote started is not set
**
** Returns          bool
**
*******************************************************************************/
bool btif_av_is_local_started_on_other_idx(int current_index)
{
  btif_sm_state_t state = BTIF_AV_STATE_IDLE;
  BTIF_TRACE_DEBUG("%s: current_index = %d", __func__, current_index);
  int i;
  // return true if other IDx is playing
  for (i = 0; i < btif_max_av_clients; i++) {
    if (i != current_index) {
      state = btif_sm_get_state(btif_av_cb[i].sm_handle);
      BTIF_TRACE_DEBUG("%s: i = %d, state = %d, btif_av_cb[i].remote_started: %d",
                                  __func__, i, state, btif_av_cb[i].remote_started);
      if (state == BTIF_AV_STATE_STARTED && !btif_av_cb[i].remote_started)
        return true;
    }
  }
  return false;
}


/*******************************************************************************
**
** Function         btif_av_is_remote_started_set
**
** Description      Checks if remote started is not set for particular index
**
** Returns          bool
**
*******************************************************************************/
bool btif_av_is_remote_started_set(int index)
{
  if(index != INVALID_INDEX && index < btif_max_av_clients)
    return btif_av_cb[index].remote_started;
  return false;
}


/*******************************************************************************
 *
 * Function         btif_av_get_current_playing_dev_idx
 *
 * Description      get current playing device index
 *
 * Returns          int
 *
 ******************************************************************************/
int btif_av_get_current_playing_dev_idx(void)
{
  int i;

  for (i = 0; i < btif_max_av_clients; i++) {
    if (btif_av_cb[i].current_playing == TRUE) {
      BTIF_TRACE_DEBUG("current playing on index = %d", i);
      return i;
    }
  }
  BTIF_TRACE_DEBUG("current playing device not set on any idx");
  return i;
}

/******************************************************************************
**
** Function        btif_av_get_sink_latency
**
** Description     get initial sink latency
**
** Returns         tBTA_AV_LATENCY
**
********************************************************************************/
tBTA_AV_LATENCY btif_av_get_sink_latency() {
  int index = 0;
  int i;
  tBTA_AV_LATENCY latency = 0;
  tBTA_AV_LATENCY sink_latency = 0;

  if (enable_multicast == false) {
    index = btif_av_get_current_playing_dev_idx();
    /* Set sink latency to be latency reported from sink if delay reporting
     * is enabled, or else set sink latency to be default sink latency value */
    if ((index >= 0 && index < btif_max_av_clients) && btif_av_cb[index].sink_latency > 0)
      sink_latency = btif_av_cb[index].sink_latency;
    else
      sink_latency = BTIF_AV_DEFAULT_SINK_LATENCY;

  } else {
    BTIF_TRACE_DEBUG("%s, multicast enabled, calculate average sink latency", __func__);
    for (i = 0; i < btif_max_av_clients; i++) {
      if (btif_av_cb[i].sink_latency > 0)
        latency += btif_av_cb[i].sink_latency;
      else
        latency += BTIF_AV_DEFAULT_MULTICAST_SINK_LATENCY;
    }
    if (latency > 0)
      sink_latency = latency / btif_max_av_clients;
    else
      sink_latency = BTIF_AV_DEFAULT_MULTICAST_SINK_LATENCY;
  }

  BTIF_TRACE_DEBUG("%s, return sink latency: %d", __func__, sink_latency);
  return sink_latency;
}


/******************************************************************************
**
** Function        btif_av_is_scrambling_enabled
**
** Description     get scrambling is enabled from bluetooth.
**
** Returns         bool
**
********************************************************************************/
bool btif_av_is_scrambling_enabled() {
  uint8_t no_of_freqs = 0;
  uint8_t *freqs = NULL;
  btav_a2dp_codec_config_t codec_config;
  std::vector<btav_a2dp_codec_config_t> codecs_local_capabilities;
  std::vector<btav_a2dp_codec_config_t> codecs_selectable_capabilities;
  A2dpCodecs* a2dp_codecs = bta_av_get_a2dp_codecs();
  if (a2dp_codecs == nullptr) return false;

  if (!a2dp_codecs->getCodecConfigAndCapabilities(
          &codec_config, &codecs_local_capabilities,
          &codecs_selectable_capabilities)) {
    BTIF_TRACE_WARNING(
        "btif_av_is_scrambling_enabled failed: "
        "cannot get codec config and capabilities");
    return false;
  }
  freqs = controller_get_interface()->get_scrambling_supported_freqs(&no_of_freqs);
  if(no_of_freqs == 0) {
    BTIF_TRACE_WARNING(
        "BT controller doesn't support scrambling");
    return false;
  }

  if (freqs != NULL) {
    for ( uint8_t i = 0; i < no_of_freqs; i++) {
      if (freqs[i] ==  ( uint8_t ) codec_config.sample_rate ) {
         return true;
      }
    }
  }
  return false;
}

/******************************************************************************
**
** Function        btif_av_is_44p1kFreq_supported
**
** Description     get 44p1kFreq is enabled from bluetooth.
**
** Returns         bool
**
********************************************************************************/
bool btif_av_is_44p1kFreq_supported() {
  uint8_t add_on_features_size = 0;
  const bt_device_features_t * add_on_features_list = NULL;

  add_on_features_list = controller_get_interface()->get_add_on_features(&add_on_features_size);
  if (add_on_features_size == 0) {
    BTIF_TRACE_WARNING(
        "BT controller doesn't add on features");
    return false;
  }

  if (add_on_features_list != NULL) {
    if (HCI_SPLIT_A2DP_44P1KHZ_SAMPLE_FREQ(add_on_features_list->as_array)) {
      return true;
    }
  }
  return false;
}

uint8_t btif_av_get_peer_sep() {
  if (isPeerA2dpSink == true)
    return AVDT_TSEP_SNK;
  else
    return AVDT_TSEP_SRC;
}

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
bool btif_av_is_peer_edr(void) {
  btif_sm_state_t state;
  bool peer_edr = false;

  ASSERTC(btif_av_is_connected(), "No active a2dp connection", 0);

  /* If any of the remote in streaming state is BR
   * return false to ensure proper configuration
   * is used. Ideally, since multicast is not supported
   * if any of the connected device is BR device,
   * we should not see both devices in START state.
   */
  for (int index = 0; index < btif_max_av_clients; index ++) {
    state = btif_sm_get_state(btif_av_cb[index].sm_handle);
    if ((btif_av_cb[index].flags & BTIF_AV_FLAG_PENDING_START)
            || btif_av_cb[index].current_playing) {
      if (btif_av_cb[index].edr)
        peer_edr = true;
      else
        return false;
    }
  }
  return peer_edr;
}

/*******************************************************************************
 *
 * Function         btif_av_any_br_peer
 *
 * Description      Check if the any of connected devices is BR device.
 *
 * Returns          true if connected to any BR device, false otherwise.
 *
 ******************************************************************************/
bool btif_av_any_br_peer(void) {
  btif_sm_state_t state;
  for (int index = 0; index < btif_max_av_clients; index ++) {
    state = btif_sm_get_state(btif_av_cb[index].sm_handle);
    if (state >= BTIF_AV_STATE_OPENED) {
      if (!btif_av_cb[index].edr) {
        BTIF_TRACE_WARNING("%s : Connected to BR device :", __FUNCTION__);
        return true;
      }
    }
  }
  return false;
}

/*******************************************************************************
 *
 * Function         btif_av_peer_supports_3mbps
 *
 * Description      check if the connected a2dp device supports
 *                  3mbps edr. Only when connected this function
 *                  will accurately provide a true capability of
 *                  remote peer. If not connected it will always be false.
 *
 * Returns          true if remote device is EDR and supports 3mbps
 *
 ******************************************************************************/
bool btif_av_peer_supports_3mbps(void) {
  btif_sm_state_t state;
  ASSERTC(btif_av_is_connected(), "No active a2dp connection", 0);

  for (int index = 0; index < btif_max_av_clients; index ++) {
    state = btif_sm_get_state(btif_av_cb[index].sm_handle);
    if ((btif_av_cb[index].flags & BTIF_AV_FLAG_PENDING_START)
         || btif_av_cb[index].current_playing)
      if (btif_av_cb[index].edr_3mbps)
        return true;
  }
  return false;
}

/******************************************************************************
 *
 * Function        btif_av_clear_remote_suspend_flag
 *
 * Description     Clears btif_av_cd.flags if BTIF_AV_FLAG_REMOTE_SUSPEND is set
 *
 * Returns          void
 *****************************************************************************/
void btif_av_clear_remote_suspend_flag(void) {
  int i;
  for (i = 0; i < btif_max_av_clients; i++) {
    BTIF_TRACE_DEBUG("%s(): flag :%x", __func__, btif_av_cb[i].flags);
    btif_av_cb[i].flags  &= ~BTIF_AV_FLAG_REMOTE_SUSPEND;
    bta_av_sniff_enable(true, btif_av_cb[i].peer_bda);
  }
}

/*******************************************************************************
 *
 * Function         btif_av_move_idle
 *
 * Description      Opening state is intermediate state. It cannot handle
 *                  incoming/outgoing connect/disconnect requests.When ACL
 *                  is disconnected and we are in opening state then move back
 *                  to idle state which is proper to handle connections.
 *
 * Returns          Void
 *
 ******************************************************************************/
void btif_av_move_idle(RawAddress bd_addr) {
  int index =0;
  char a2dp_role[255] = "false";
  osi_property_get("persist.vendor.service.bt.a2dp.sink", a2dp_role, "false");
  /* inform the application that ACL is disconnected and move to idle state */
  index = btif_av_idx_by_bdaddr(&bd_addr);
  if (index == btif_max_av_clients) {
    BTIF_TRACE_IMP("btif_av_move_idle: Already in IDLE");
    return;
  }
  btif_sm_state_t state = btif_sm_get_state(btif_av_cb[index].sm_handle);
  BTIF_TRACE_IMP("ACL Disconnected state %d  is same device %d",state,
           memcmp (&bd_addr, &(btif_av_cb[index].peer_bda), sizeof(bd_addr)));
  if (state == BTIF_AV_STATE_OPENING &&
      (memcmp (&bd_addr, &(btif_av_cb[index].peer_bda), sizeof(bd_addr)) == 0)) {
    BTIF_TRACE_IMP("Moving BTIF State from Opening to Idle due to ACL disconnect");
    btif_report_connection_state(BTAV_CONNECTION_STATE_DISCONNECTED, &(btif_av_cb[index].peer_bda));
    BTA_AvClose(btif_av_cb[index].bta_handle);
    btif_av_check_and_start_collission_timer(index);
    btif_sm_change_state(btif_av_cb[index].sm_handle, BTIF_AV_STATE_IDLE);
    if (!strncmp("false", a2dp_role, 5)) {
      btif_queue_advance_by_uuid(UUID_SERVCLASS_AUDIO_SOURCE, &(btif_av_cb[index].peer_bda));
    } else {
      btif_queue_advance_by_uuid(UUID_SERVCLASS_AUDIO_SINK, &(btif_av_cb[index].peer_bda));
    }
    btif_report_connection_state_to_ba(BTAV_CONNECTION_STATE_DISCONNECTED);
  }
}

/******************************************************************************
 *
 * Function        btif_av_get_num_playing_devices
 *
 * Description     Return number of A2dp playing devices
 *
 * Returns         int
 *****************************************************************************/
uint16_t btif_av_get_num_playing_devices(void) {
  uint16_t i;
  uint16_t playing_devices = 0;
  for (i = 0; i < btif_max_av_clients; i++) {
    btif_av_cb[i].state = btif_sm_get_state(btif_av_cb[i].sm_handle);
    if (btif_av_cb[i].state ==  BTIF_AV_STATE_STARTED)
        playing_devices++;
  }
  BTIF_TRACE_DEBUG("AV devices playing: %d", playing_devices);

  return playing_devices;
}
/*******************************************************************************
 *
 * Function        btif_av_get_num_connected_devices
 *
 * Description     Return number of A2dp connected devices
 *
 * Returns         int
 *****************************************************************************/
uint16_t btif_av_get_num_connected_devices(void) {
  uint16_t i;
  uint16_t connected_devies = 0;
  for (i = 0; i < btif_max_av_clients; i++)
  {
    btif_av_cb[i].state = btif_sm_get_state(btif_av_cb[i].sm_handle);
    if ((btif_av_cb[i].state == BTIF_AV_STATE_OPENED) ||
        (btif_av_cb[i].state ==  BTIF_AV_STATE_STARTED))
      connected_devies++;
  }
  BTIF_TRACE_DEBUG("AV Connection count: %d", connected_devies);

  return connected_devies;
}

/******************************************************************************
**
** Function         btif_av_get_av_hdl_from_idx
**
** Description      Returns bta handle from the device index
**
** Returns         BTA HANDLE
********************************************************************************/
tBTA_AV_HNDL btif_av_get_av_hdl_from_idx(int idx)
{
    if (idx == btif_max_av_clients)
    {
        BTIF_TRACE_ERROR("%s: Invalid handle",__func__);
        return -1;
    }
    return btif_av_cb[idx].bta_handle;
}

/******************************************************************************
 *
 * Function        btif_av_update_multicast_state
 *
 * Description     Enable Multicast only if below conditions are satisfied
 *                 1. Connected to only 2 EDR HS.
 *                 2. Connected to both HS as master.
 *                 3. Connected to 2 EDR HS and one BLE device
 *                 Multicast will fall back to soft handsoff in below conditions
 *                 1. Number of ACL links is more than 2,like connected to HID
 *                    initiating connection for HS1 and HS2.
 *                 2. Connected to BR and EDR HS.
 *                 3. Connected to more then 1 BLE device
 *
 * Returns         void
*****************************************************************************/
void btif_av_update_multicast_state(int index) {
#if (BTIF_AV_ENABLE_MCAST_RESTRICTIONS == TRUE)
  uint16_t num_connected_br_edr_devices = 0;
  uint16_t num_connected_le_devices = 0;
  uint16_t i = 0;
  bool is_slave = false;
  bool is_br_hs_connected = false;
#endif
  uint16_t num_av_connected = 0;
  bool prev_multicast_state = enable_multicast;

  if (!is_multicast_supported) {
    BTIF_TRACE_DEBUG("%s() Multicast is Disabled", __func__);
    return;
  }

  if (multicast_disabled == true) {
    multicast_disabled = false;
    enable_multicast = false;
    BTA_AvEnableMultiCast(false, btif_av_cb[index].bta_handle);
    return;
  }

  BTIF_TRACE_DEBUG("%s() Multicast previous state : %s", __func__,
    enable_multicast ? "Enabled" : "Disabled" );
  num_av_connected = btif_av_get_num_connected_devices();
#if (BTIF_AV_ENABLE_MCAST_RESTRICTIONS == TRUE)
  num_connected_br_edr_devices = btif_dm_get_br_edr_links();
  num_connected_le_devices = btif_dm_get_le_links();
  num_av_connected = btif_av_get_num_connected_devices();
  is_br_hs_connected = btif_av_any_br_peer();

  for (i = 0; i < btif_max_av_clients; i++) {
    if (btif_av_cb[i].is_slave == true) {
      BTIF_TRACE_WARNING("Conected as slave");
      is_slave = true;
      break;
    }
  }

  if ((num_av_connected <= 2) && (is_br_hs_connected != true) &&
      (is_slave == false) && ((num_connected_br_edr_devices <= 2) &&
      (num_connected_le_devices <= 1)))
    enable_multicast = true;
  else
    enable_multicast = false;
#endif
  if (num_av_connected > 1)
     enable_multicast = true;
  else
     enable_multicast = false;
  BTIF_TRACE_DEBUG("%s() Multicast current state : %s", __func__,
        enable_multicast ? "Enabled" : "Disabled" );

  if (prev_multicast_state != enable_multicast) {
    BTA_AvEnableMultiCast(enable_multicast, btif_av_cb[index].bta_handle);
#ifdef BT_AV_SHO_FEATURE
    HAL_CBACK(bt_av_src_callbacks, multicast_state_cb, enable_multicast);
#endif
  }
}
#if (TWS_ENABLED == TRUE)
extern void btif_av_set_earbud_state(const RawAddress& address, uint8_t earbud_state);
extern void btif_av_set_earbud_role(const RawAddress& address, uint8_t earbud_role);
void btif_av_set_earbud_state(const RawAddress& address, uint8_t earbud_state) {
#if (TWS_STATE_ENABLED == TRUE)
  BTIF_TRACE_EVENT("btif_av_set_earbud_state = %d", earbud_state);
  if (tws_state_supported == false) {
    BTIF_TRACE_ERROR("%s:tws state not supported",__func__);
    return;
  }
  int index = btif_av_idx_by_bdaddr(&(RawAddress&)address);
  if (index == btif_max_av_clients) {
    BTIF_TRACE_ERROR("%s: invalid index",__func__);
    return;
  }
  if (earbud_state == TWSP_STATE_OUT_OF_EAR &&
    btif_av_cb[index].eb_state == 0 &&
    btif_av_cb[index].state == BTIF_AV_STATE_STARTED) {
    BTIF_TRACE_ERROR("%s:streaming started for reconn,suspending",__func__);
    btif_av_cb[index].tws_state_suspend = true;
  }
  btif_av_cb[index].eb_state = earbud_state;
  BTA_AVSetEarbudState(earbud_state, btif_av_cb[index].bta_handle);
#endif
}
void btif_av_set_earbud_role(const RawAddress& address, uint8_t earbud_role) {
  BTIF_TRACE_EVENT("btif_av_set_earbud_role = %d",earbud_role);
  int index = btif_av_idx_by_bdaddr(&(RawAddress&)address);
  if (index == btif_max_av_clients) {
    BTIF_TRACE_ERROR("%s: invalid index",__func__);
    return;
  }
  BTA_AVSetEarbudRole(earbud_role, btif_av_cb[index].bta_handle);
}
#endif

/******************************************************************************
**
** Function        btif_av_get_multicast_state
**
** Description     Returns true if multicast is enabled else false
**
** Returns         bool
******************************************************************************/
bool btif_av_get_multicast_state() {
  return enable_multicast;
}

/******************************************************************************
**
** Function        btif_av_get_ongoing_multicast
**
** Description     Returns true if multicast is ongoing
**
** Returns         bool
******************************************************************************/
bool btif_av_get_ongoing_multicast() {
  int i = 0, j = 0;
  if (!is_multicast_supported) {
    BTIF_TRACE_DEBUG("Multicast is Disabled");
    return false;
  }
  for (i = 0; i < btif_max_av_clients; i++)
    if (btif_av_cb[i].is_device_playing)
      j++;

  if (j == btif_max_av_clients)
    return true;
  else
    return false;
}

/******************************************************************************
**
** Function        btif_av_is_multicast_supported
**
** Description     Returns true if multicast is supported
**
** Returns         bool
******************************************************************************/
bool btif_av_is_multicast_supported() {
  return is_multicast_supported;
}

bool btif_av_check_flag_remote_suspend(int index) {
  if (index >= btif_max_av_clients || index < 0) {
    BTIF_TRACE_ERROR("%s(): Invalid index = %d",__func__, index);
    return false;
  }
  if (btif_av_cb[index].flags & BTIF_AV_FLAG_REMOTE_SUSPEND) {
    BTIF_TRACE_DEBUG("remote suspend flag set on index = %d",index);
    return true;
  } else {
    BTIF_TRACE_DEBUG("remote suspend flag not set on index = %d",index);
    return false;
  }
}

/* SPLITA2DP*/
/*******************************************************************************
 *
 * Function         btif_av_is_split_a2dp_enabled
 *
 * Description      Check if split a2dp is enabled.
 *
 * Returns          TRUE if split a2dp is enabled, FALSE otherwise
 *
 ******************************************************************************/
bool btif_av_is_split_a2dp_enabled() {
  BTIF_TRACE_DEBUG("btif_a2dp_source_is_hal_v2_supported %d ",
                        btif_a2dp_source_is_hal_v2_supported());
  if (is_multicast_supported) {
    BTIF_TRACE_ERROR("%s,Mulitcast enabled, default non-split mode",__func__);
    return false;
  }
  if (!btif_a2dp_source_is_hal_v2_supported()) {
    BTIF_TRACE_DEBUG("btif_av_is_split_a2dp_enabled: %d", bt_split_a2dp_enabled);
    return bt_split_a2dp_enabled;
  } else if(isBATEnabled()) {
    BTIF_TRACE_DEBUG("%s:  going for split as BA is active", __func__);
    return true;
  } else {
    if (!bta_av_co_is_active_peer()) {
      BTIF_TRACE_ERROR("%s:  No active peer codec config found, "
                        "by default splitmode", __func__);
      return true;
    }
    A2dpCodecConfig* a2dpCodecConfig = bta_av_get_a2dp_current_codec();
    if (a2dpCodecConfig == nullptr) {
      BTIF_TRACE_ERROR("%s: active peer set but still no current codec "
          "available, by default splitmode ", __func__);
      return true;
    }

    if(A2DP_IsCodecEnabledInOffload(a2dpCodecConfig->codecIndex())) {
      BTIF_TRACE_DEBUG("%s:  going for split ", __func__);
      return true;
    } else if(A2DP_IsCodecEnabledInSoftware(a2dpCodecConfig->codecIndex())) {
      BTIF_TRACE_DEBUG("%s:  going for non split ", __func__);
      return false;
    } else {
      BTIF_TRACE_ERROR("%s: current codec is not enabled either of modes"
                        " going ahead with split", __func__);
      return true;
    }
  }
}

/******************************************************************************
**
** Function         btif_av_is_under_handoff
**
** Description     check if AV state is under handoff
**
** Returns         TRUE if handoff is triggered, FALSE otherwise
********************************************************************************/
bool btif_av_is_under_handoff() {
  int i;
  btif_sm_state_t state = BTIF_AV_STATE_IDLE;

  BTIF_TRACE_DEBUG("btif_av_is_under_handoff");

  for (i = 0; i < btif_max_av_clients; i++) {
    state = btif_sm_get_state(btif_av_cb[i].sm_handle);
    if (btif_av_cb[i].dual_handoff &&
      (state == BTIF_AV_STATE_STARTED || state == BTIF_AV_STATE_OPENED)) {
      /* If a2dp reconfigure is triggered when playing device disconnect is
       * initiated locally then return false, otherwise wait till the suspend cfm
       * is received from the remote.
       */
       BTIF_TRACE_DEBUG("AV is under handoff");
       return true;
    }
  }
  return false;
}

bool btif_av_is_handoff_set() {
  for (int i = 0; i < btif_max_av_clients; i++) {
    if (btif_av_cb[i].dual_handoff) {
      BTIF_TRACE_DEBUG("%s: AV is under handoff for idx = %d",__func__, i);
      return true;
    }
  }
  return false;
}

void btif_av_reset_reconfig_flag() {
  int i;
  BTIF_TRACE_DEBUG("%s",__func__);
  reconfig_a2dp = FALSE;
  for (i = 0; i < btif_max_av_clients; i++)
  {
    if (btif_av_cb[i].dual_handoff) {
      BTIF_TRACE_DEBUG("%s resetting dual handoff flag for index %d",__func__, i);
      btif_av_cb[i].dual_handoff = FALSE;
    }
  }
}

/******************************************************************************
**
** Function         btif_av_get_reconfig_dev_hndl
**
** Description     Retrieve the bta handle for the device codec switch is
**                 triggered in the middle of streaming.
**
** Returns         BTA HANDLE if codec switch triggered during streaming
********************************************************************************/
tBTA_AV_HNDL btif_av_get_reconfig_dev_hndl() {
  int i = 0;
  BTIF_TRACE_DEBUG("%s",__func__);
  for (i = 0 ; i < btif_max_av_clients; i++) {
    if (btif_av_cb[i].reconfig_pending)
        return btif_av_cb[i].bta_handle;
  }
  return 0;
}

/******************************************************************************
**
** Function        btif_av_reset_codec_reconfig_flag
**
** Description     Reset codec reconfig flag if avdtp reconfig is not triggered
**                 for user provided codec config
**
** Returns         void
********************************************************************************/
void btif_av_reset_codec_reconfig_flag(RawAddress address) {
  int i;
  BTIF_TRACE_DEBUG("%s",__func__);
  i = btif_av_idx_by_bdaddr(&address);
  if (i < btif_max_av_clients) {
    if (btif_av_cb[i].reconfig_pending)
      btif_av_cb[i].reconfig_pending = false;
  } else {
  /*clearing all reconfig_pending when address will not match */
    for (i = 0; i < btif_max_av_clients; i++)
      if (btif_av_cb[i].reconfig_pending)
        btif_av_cb[i].reconfig_pending = false;
  }
}

/******************************************************************************
**
** Function        btif_av_reinit_audio_interface
**
** Description     Reinit audio interface,this function is called when BT
**                 audio hal server is died
**
** Returns         void
********************************************************************************/

void btif_av_reinit_audio_interface() {
  BTIF_TRACE_DEBUG(LOG_TAG,"btif_av_reint_audio_interface");
  if (!btif_a2dp_source_is_hal_v2_supported()) {
    btif_a2dp_audio_interface_init();
    btif_a2dp_audio_if_init = true;
  }
}

void btif_av_flow_spec_cmd(int index, int bitrate) {
  tBT_FLOW_SPEC flow_spec;
  memset(&flow_spec, 0x00, sizeof(flow_spec));
  flow_spec.flow_direction = 0x00;     /* flow direction - out going */
  flow_spec.service_type = 0x02;       /* Guaranteed */
  flow_spec.token_rate = 0x00;         /* bytes/second - no token rate is specified*/
  flow_spec.token_bucket_size = 0x00;  /* bytes - no token bucket is needed*/
  flow_spec.latency = 0xFFFFFFFF;      /* microseconds - default value */
  flow_spec.peak_bandwidth = bitrate/8;/*bytes per second */
  //Sending Flow_spec has been taken care whenever event:BTIF_AV_START_STREAM_REQ_EVT comes.
  //BTM_FlowSpec (btif_av_cb[index].peer_bda, &flow_spec, NULL);
  APPL_TRACE_DEBUG("%s peak_bandwidth %d",__func__, flow_spec.peak_bandwidth);
}

#if (TWS_ENABLED == TRUE)
bool btif_av_is_tws_device_playing(int index) {
  int i;
  btif_sm_state_t state = BTIF_AV_STATE_IDLE;
  for (i = 0; i < btif_max_av_clients; i++) {
    state = btif_sm_get_state(btif_av_cb[i].sm_handle);
    if (i != index && state == BTIF_AV_STATE_STARTED) {
      if (btif_av_cb[i].tws_device) {
        BTIF_TRACE_EVENT("btif_av_is_tws_device_playing on index = %d",i);
        return true;
      }
    }
  }
  return false;
}
bool btif_av_is_tws_pair_remote_started(int index) {
  int i;
  btif_sm_state_t state = BTIF_AV_STATE_IDLE;
  for (i = 0; i < btif_max_av_clients; i++) {
    state = btif_sm_get_state(btif_av_cb[i].sm_handle);
    if (i != index && state == BTIF_AV_STATE_STARTED) {
      if (btif_av_cb[i].tws_device && btif_av_cb[i].remote_started) {
        BTIF_TRACE_EVENT("%s: on index = %d",__func__,i);
        return true;
      }
    }
  }
  return false;
}
bool btif_av_is_tws_suspend_triggered(int index) {
  for (int i = 0; i < btif_max_av_clients; i++) {
    if (i != index && btif_av_cb[i].tws_device) {
    //TODO check for tws pair
      if (btif_av_cb[i].flags & BTIF_AV_FLAG_LOCAL_SUSPEND_PENDING) {
        BTIF_TRACE_EVENT("Local suspend triggered on tws pair");
        return true;
      }
    }
  }
  return false;
}
bool btif_av_is_tws_enabled_for_dev(const RawAddress& rc_addr) {
  for (int i = 0; i < btif_max_av_clients; i++) {
    if (btif_av_cb[i].peer_bda == rc_addr) {
      BTIF_TRACE_DEBUG("%s",__func__);
      return btif_av_cb[i].tws_device;
    }
  }
  return false;
}
bool btif_av_is_tws_connected() {
  for (int i = 0; i < btif_max_av_clients; i++) {
    if (btif_av_cb[i].tws_device) {
      BTIF_TRACE_DEBUG("%s",__func__);
      return true;
    }
  }
  return false;
}
bool btif_av_current_device_is_tws() {
  for (int i = 0; i < btif_max_av_clients; i++) {
    if (btif_av_cb[i].tws_device && btif_av_cb[i].current_playing) {
      BTIF_TRACE_DEBUG("%s",__func__);
      return true;
    }
  }
  return false;
}
bool btif_av_is_idx_tws_device(int index) {
  if (index == btif_max_av_clients) {
    BTIF_TRACE_ERROR("%s:invalid index",__func__);
    return false;
  }
  BTIF_TRACE_DEBUG("%s:%d",__func__, btif_av_cb[index].tws_device);
  return btif_av_cb[index].tws_device;
}
int btif_av_get_tws_pair_idx(int index) {
  BTIF_TRACE_DEBUG("%s",__func__);
  int idx = btif_max_av_clients;
  if (index == btif_max_av_clients) {
    BTIF_TRACE_ERROR("%s:invalid index",__func__);
    return false;
  }
  for (idx = 0; idx < btif_max_av_clients; idx++) {
    if (idx != index && btif_av_cb[idx].tws_device) {
      BTIF_TRACE_DEBUG("%s:found TWS+ pair index %d",__func__,idx);
      return idx;
    }
  }
  return idx;
}
#if (TWS_STATE_ENABLED == TRUE)
bool btif_av_is_tws_ready_to_play() {
  BTIF_TRACE_DEBUG("%s",__func__);
  for(int i = 0; i < btif_max_av_clients; i++) {
    if (btif_av_cb[i].tws_device &&
      btif_av_cb[i].eb_state == TWSP_STATE_IN_EAR) {
      BTIF_TRACE_DEBUG("%s:Ready to start stream",__func__);
      return true;
    }
  }
  return false;
}
#endif
bool btif_av_is_tws_enable_monocfg() {
  int i,index;
  BTIF_TRACE_DEBUG("%s",__func__);
  btif_sm_state_t state = BTIF_AV_STATE_IDLE;
  RawAddress tws_address_peer;
  i = btif_av_get_latest_playing_device_idx();
  if (btif_av_cb[i].tws_device) {
    if (BTM_SecGetTwsPlusPeerDev(btif_av_cb[i].peer_bda,tws_address_peer)== true) {
          index = btif_av_idx_by_bdaddr(&tws_address_peer);
          if (index < btif_max_av_clients) {
            state = btif_sm_get_state(btif_av_cb[index].sm_handle);
            if (state == BTIF_AV_STATE_STARTED || (btif_av_cb[index].flags & BTIF_AV_FLAG_PENDING_START))
               return false;
          }
       }
    }
  return true;
}
#endif
/*SPLITA2DP*/

bool btif_av_is_state_opened(int i) {
  if (i >= 0 && i < btif_max_av_clients)
    return (btif_sm_get_state(btif_av_cb[i].sm_handle) == BTIF_AV_STATE_OPENED);
  else
    return false;
}

void btif_av_set_audio_delay(uint16_t delay, tBTA_AV_HNDL hndl) {
  int index = HANDLE_TO_INDEX(hndl);
  bool isActive = (index == btif_av_get_current_playing_dev_idx());
  if (index >= 0 && index < btif_max_av_clients) {
    btif_av_cb[index].remote_delay = delay;
    btif_a2dp_control_set_audio_delay(delay, index);
  } else {
    BTIF_TRACE_ERROR("%s: Invalid index for connection", __func__);
    btif_a2dp_control_set_audio_delay(delay, 0);
  }

  if (btif_a2dp_source_is_hal_v2_supported() && isActive) {
    BTIF_TRACE_DEBUG("%s set delay/latency",__func__);
    btif_a2dp_update_sink_latency_change();
  }
}

void btif_av_reset_audio_delay(tBTA_AV_HNDL hndl) {
  int index = HANDLE_TO_INDEX(hndl);
  if (index >= 0 && index < btif_max_av_clients) {
    btif_a2dp_control_reset_audio_delay(index);
    btif_av_cb[index].remote_delay = 0;
  } else {
    BTIF_TRACE_ERROR("%s: Invalid index for connection", __func__);
    btif_a2dp_control_reset_audio_delay(0);
  }
}

uint16_t btif_av_get_audio_delay(int index) {
  A2dpCodecConfig* current_codec = bta_av_get_a2dp_current_codec();
  if (current_codec != nullptr) {
    if (current_codec->codecIndex() == BTAV_A2DP_CODEC_INDEX_SOURCE_APTX_ADAPTIVE) {
      if (index >= 0 && index < btif_max_av_clients) {
        BTIF_TRACE_WARNING("%s: Updating Aptx Adaptive specific delay: %d",
              __func__, btif_av_cb[index].codec_latency);
        return btif_av_cb[index].codec_latency;
      } else {
        return 0;
      }
    }
  }

  if (!btif_a2dp_source_is_hal_v2_supported()) {
    if (index >= 0 && index < btif_max_av_clients) {
      return btif_a2dp_control_get_audio_delay(index);
    } else {
      BTIF_TRACE_ERROR("%s: Invalid index for connection", __func__);
      return btif_a2dp_control_get_audio_delay(0);
    }
  } else {
    if (index >= 0 && index < btif_max_av_clients) {
      return btif_av_cb[index].remote_delay;
    } else {
      BTIF_TRACE_ERROR("%s: Invalid index for connection", __func__);
      return 0;
    }
  }
}

uint16_t btif_av_get_aptx_mode_info() {
  int index = btif_max_av_clients;
  if (btif_av_stream_started_ready())
    index = btif_av_get_latest_playing_device_idx();
  else
    index = btif_av_get_latest_device_idx_to_start();

  if(index >= btif_max_av_clients) return 0;

  return btif_av_cb[index].aptx_mode;
}

/*******************************************************************************
**
** Function         btif_update_reported_delay
**
** Description      Count average packet delay (include buffering, decoding,
**                  rending delay)
**
** Returns          delay value (nanosencond)
*******************************************************************************/
uint64_t btif_update_reported_delay(uint64_t inst_delay)
{
  average_delay = 0;
  if (delay_record_idx >= DELAY_RECORD_COUNT)
    delay_record_idx = 0;
  delay_record[delay_record_idx++] = inst_delay +
      ((get_audiotrack_latency() + MISC_RENDERING_DELAY) * 1000000);

  int64_t sum_delay = 0; int i = 0; int count = 0;
  for(; i < DELAY_RECORD_COUNT; i++) {
    if(delay_record[i] > 0) {
      sum_delay += delay_record[i];
      count++;
    }
    else
      break;
  }

  average_delay = (sum_delay / count);

  BTIF_TRACE_DEBUG("%s ~~inst_delay = [%09llu](ns) avg_delay = [%09llu](ns), Delay records = %d",
                    __func__,inst_delay, average_delay, count);

  return average_delay;
}

/*******************************************************************************
**
** Function         btif_is_sink_delay_report_supported
**
** Description      check if the connected a2dp device supports
**                  delay reporting.
**
** Returns          bool
*******************************************************************************/
bool btif_is_sink_delay_report_supported() {
  int index = 0;
  BTIF_TRACE_DEBUG("%s: %d" ,__func__, btif_av_cb[index].avdt_sync);
  return btif_av_cb[index].avdt_sync;
}

/*******************************************************************************
**
** Function         btif_get_average_delay
**
** Description      Returns average of instantaneous delay values
**
** Returns          int64_t
*******************************************************************************/
int64_t btif_get_average_delay() {
  return average_delay;
}

/*******************************************************************************
**
** Function         btif_get_average_delay
**
** Description      To check if device is supporting which A2DP Role (Sink/Source)
**
** Returns          bool
*******************************************************************************/
bool btif_device_in_sink_role() {
    char a2dp_role[6] = "false";
    osi_property_get("persist.vendor.service.bt.a2dp.sink", a2dp_role, "false");
    if (!strncmp("true", a2dp_role, 4))
        return true;
    return false;
}

/*******************************************************************************
**
** Function         btif_is_a2dp_sink_handoff_required
**
** Description      To check if there is need for Soft-Handoff in A2DP Sink.
**
** Returns          bool
*******************************************************************************/
bool btif_is_a2dp_sink_handoff_required(int idx) {
  int cur_playing_idx = btif_av_get_current_playing_dev_idx();
  BTIF_TRACE_DEBUG("%s: index = %d, cur_playing_idx= %d",
                          __func__, idx, cur_playing_idx);
  if (cur_playing_idx != btif_max_av_clients && idx != cur_playing_idx) {
    return true;
  }
  return false;
}

/*******************************************************************************
**
** Function         btif_initiate_sink_handoff
**
** Description      Intiates operations required to handle Soft-Handoff
**
** Returns          void
*******************************************************************************/
void btif_initiate_sink_handoff(int idx, bool audio_state_changed) {
  int cur_playing_idx = btif_av_get_current_playing_dev_idx();
  BTIF_TRACE_DEBUG("%s: index = %d, cur_playing_idx = %d",
                       __func__, idx, cur_playing_idx);

  // Mark idx as current playing index
  btif_av_cb[idx].current_playing = true;
  // Mark the other playing devices as not playing
  int i;
  for (i = 0; i < btif_max_av_clients; i++) {
    if (i != idx) {
      btif_av_cb[i].current_playing = false;
    }
  }
  other_device_media_packet_count = 0;
  // Update the decoder on soft-handoff if handoff is through audio state change
  if (audio_state_changed) {
      BTIF_TRACE_DEBUG("%s, updating decoder on SHO through audio state change", __func__);
      uint8_t* a2dp_codec_config = bta_av_co_get_peer_codec_info(btif_av_cb[idx].bta_handle);
      if (a2dp_codec_config != NULL) {
          btif_a2dp_sink_update_decoder(a2dp_codec_config);
      } else {
          BTIF_TRACE_DEBUG("%s, a2dp_codec_config is NULL", __func__);
      }
  }
}

/*******************************************************************************
**
** Function         btif_sink_ho_through_avrcp_pback_status
**
** Description      Initate SoftHandoff from AVRCP EVENT_PLAYBACK_STATUS_CHANGED
**                  (if recieved before AVDTP_START) Or in case of quick SHO
**                  i.e. prev streaming device started streaming again (within 3sec)
**                  and AVDTP_SUSPEND and AVDTP_START is not received but AVRCP cmd
**                  EVENT_PLAYBACK_STATUS_CHANGED is received.
**
** Returns          void
*******************************************************************************/
void btif_sink_ho_through_avrcp_pback_status(RawAddress bd_addr) {
    int index = btif_av_idx_by_bdaddr(&bd_addr);
    BTIF_TRACE_DEBUG("%s: index: %d", __func__, index);
    if (btif_is_a2dp_sink_handoff_required(index)){
        BTIF_TRACE_DEBUG("%s, handoff from avrcp playback status change event on index: %d",
                              __func__, index);
        btif_initiate_sink_handoff(index, true);
    }
}

/*******************************************************************************
**
** Function         btif_av_set_remote_playing_state
**
** Description      Sets remote device playing state when response is received after
**                  for passthrough command
**
** Returns          void
*******************************************************************************/
void btif_av_set_remote_playing_state(int index, bool playing_state) {
  BTIF_TRACE_DEBUG("%s: is_remote_playing: %d", __func__, playing_state);
  if (index >= btif_max_av_clients)
    return;
  btif_av_cb[index].is_device_playing = playing_state;
}


/*******************************************************************************
**
** Function         btif_get_max_allowable_sink_connections
**
** Description      Get maximum number of supported Sink Connections
**                  Currently, Default:1, Max:2
**                  TODO: Range:{1,5} Default:3 Max:5
**
** Returns          void
*******************************************************************************/
int btif_get_max_allowable_sink_connections() {
    int def_no_of_conn = 1;
    char number_of_conn[2];
    osi_property_get("persist.vendor.bt.a2dp.sink_conn", number_of_conn, "1");
    BTIF_TRACE_DEBUG("%s, number_of_conn = %s", __func__, number_of_conn);
    if (!strncmp(number_of_conn, "2", 1)) {
        return atoi(number_of_conn);
    }
    int no_of_conn = atoi(number_of_conn);
    BTIF_TRACE_DEBUG("%s, number_of_conn set in property = %d", __func__, no_of_conn);
    if (no_of_conn < def_no_of_conn || no_of_conn > MAX_ALLOWED_SINK_CONNECTIONS) {
        return def_no_of_conn;
    }
    return no_of_conn;
}

tBTA_AV_HNDL btif_av_get_hndl_by_addr(RawAddress peer_address) {
  tBTA_AV_HNDL hndl = (tBTA_AV_HNDL)INVALID_INDEX;
  int idx = btif_av_idx_by_bdaddr(&peer_address);
  if (idx == btif_max_av_clients) {
    BTIF_TRACE_ERROR("%s: Invalid handle",__func__);
  } else {
    hndl = btif_av_cb[idx].bta_handle;
  }
  return hndl;
}

void btif_av_set_offload_status() {
  if (btif_av_is_split_a2dp_enabled()) {
    BTIF_TRACE_IMP("restart with hardware session");
  } else {
    BTIF_TRACE_IMP("restart with software session");
  }
  reconfig_a2dp = FALSE;
}

void btif_av_set_reconfig_flag(tBTA_AV_HNDL bta_handle) {
  for (int i = 0; i < btif_max_av_clients; i++) {
    if (btif_av_cb[i].bta_handle == bta_handle &&
      (btif_sm_get_state(btif_av_cb[i].sm_handle) == BTIF_AV_STATE_STARTED) &&
      !btif_av_cb[i].remote_started) {
      BTIF_TRACE_IMP("%s:Setting reconfig index for index %d",__func__, i);
      btif_av_cb[i].reconfig_pending = true;
      btif_av_cb[i].flags |= BTIF_AV_FLAG_PENDING_START;
    }
  }
}

void btif_av_set_suspend_rsp_track_timer(int index) {
  int *arg = NULL;
  arg = (int *) osi_malloc(sizeof(int));
  *arg = index;
  BTIF_TRACE_DEBUG("%s: index: %d", __func__, index);
  if (alarm_is_scheduled(btif_av_cb[index].suspend_rsp_track_timer)) {
    alarm_cancel(btif_av_cb[index].suspend_rsp_track_timer);
    BTIF_TRACE_DEBUG("%s: Deleting previously queued timer if any.", __func__);
  }
  alarm_set_on_mloop(btif_av_cb[index].suspend_rsp_track_timer,
                     BTIF_SUSPEND_RSP_FROM_REMOTE_TOUT,
                     btif_av_set_suspend_rsp_track_timer_tout, (void *)arg);
}

void btif_av_set_suspend_rsp_track_timer_tout(void* data) {
  int *arg = (int *)data;
  if (!arg) {
    BTIF_TRACE_ERROR("%s: index is null, return", __func__);
    return;
  }

  int index = *arg;
  RawAddress addr = btif_av_get_addr_by_index(index);
  BTIF_TRACE_DEBUG("%s:fake suspend resp on index: %d, addr: %s",
                                     __func__, index, addr.ToString().c_str());
  btif_av_cb[index].fake_suspend_rsp = true;
  bta_av_fake_suspend_rsp(addr);
  if (arg) {
    osi_free(arg);
  }
}

void btif_av_clear_suspend_rsp_track_timer(int index) {
   BTIF_TRACE_DEBUG("%s: index: %d", __func__, index);
   if (index < btif_max_av_clients && index >= 0) {
     btif_av_cb[index].fake_suspend_rsp = false;
     if (btif_av_cb[index].suspend_rsp_track_timer != NULL)
       alarm_free(btif_av_cb[index].suspend_rsp_track_timer);
     btif_av_cb[index].suspend_rsp_track_timer = NULL;
   }
 }

void btif_av_clear_pending_start_flag() {
  int i = btif_av_get_current_playing_dev_idx();
  BTIF_TRACE_DEBUG("%s: current playing idx: %d",__func__,i);
  if (i == btif_max_av_clients) return;
  if (btif_av_cb[i].reconfig_pending &&
    (btif_av_cb[i].flags &= BTIF_AV_FLAG_PENDING_START) != 0) {
    BTIF_TRACE_DEBUG("%s:clear pending start",__func__);
    btif_av_cb[i].flags &= ~BTIF_AV_FLAG_PENDING_START;
  }
}
