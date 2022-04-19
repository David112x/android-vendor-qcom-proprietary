/******************************************************************************
 * Copyright (C) 2017, The Linux Foundation. All rights reserved.
 * Not a Contribution.
 ******************************************************************************/
/******************************************************************************
 *
 *  Copyright (C) 2016 The Android Open Source Project
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

#define LOG_TAG "bt_btif_a2dp"

#include <stdbool.h>

#include "audio_a2dp_hw/include/audio_a2dp_hw.h"
#include "bt_common.h"
#include "bta_av_api.h"
#include "btif_a2dp.h"
#include "btif_a2dp_control.h"
#include "btif_a2dp_sink.h"
#include "btif_a2dp_source.h"
#include "btif_av.h"
#include "btif_util.h"
#include "osi/include/log.h"
#include "btif_a2dp_audio_interface.h"
#include "btif_hf.h"

#if (OFF_TARGET_TEST_ENABLED == FALSE)
#include "audio_hal_interface/a2dp_encoding.h"
#endif
#include "btif_bat.h"

#if (OFF_TARGET_TEST_ENABLED == TRUE)
#include "service/a2dp_hal_sim/audio_a2dp_hal_stub.h"
using ::bluetooth::audio::a2dp::SessionType;
#endif

#define BTIF_A2DP_START_BLOCK_SCO_CONNECTED 0x0D

extern bool btif_a2dp_audio_if_init;
extern tBTIF_A2DP_SOURCE_VSC btif_a2dp_src_vsc;
extern tBTIF_A2DP_SOURCE_CB btif_a2dp_source_cb;
extern void btif_av_reset_reconfig_flag();
static char a2dp_hal_imp[PROPERTY_VALUE_MAX] = "false";
extern bool btif_av_current_device_is_tws();

void btif_a2dp_on_idle() {
  APPL_TRACE_EVENT("## ON A2DP IDLE ## peer_sep = %d", btif_av_get_peer_sep());
  if (btif_av_get_peer_sep() == AVDT_TSEP_SNK) {
    btif_a2dp_source_on_idle();
  } else if (btif_av_get_peer_sep() == AVDT_TSEP_SRC) {
    btif_a2dp_sink_on_idle();
  }
}

bool btif_a2dp_on_started(tBTA_AV_START* p_av_start, bool pending_start,
                          tBTA_AV_HNDL hdl) {
  bool ack = false;
  tA2DP_CTRL_CMD pending_cmd = A2DP_CTRL_CMD_NONE;
  if (btif_a2dp_source_is_hal_v2_enabled()) {
    pending_cmd = bluetooth::audio::a2dp::get_pending_command();
  } else {
    pending_cmd = btif_a2dp_control_get_pending_command();
  }

  APPL_TRACE_WARNING("## ON A2DP STARTED ##");

  if (p_av_start == NULL) {
    /* ack back a local start request */
    if (btif_av_is_split_a2dp_enabled()) {
      if (bluetooth::headset::btif_hf_is_call_vr_idle())
        btif_dispatch_sm_event(BTIF_AV_OFFLOAD_START_REQ_EVT, (char *)&hdl, 1);
      else {
        APPL_TRACE_ERROR("call in progress, do not start offload");
        if (btif_a2dp_source_is_hal_v2_enabled()) {
          bluetooth::audio::a2dp::ack_stream_started(A2DP_CTRL_ACK_INCALL_FAILURE);
        } else {
          btif_a2dp_audio_on_started(A2DP_CTRL_ACK_INCALL_FAILURE);
        }
      }
      return true;
    } else {
      if (pending_cmd == A2DP_CTRL_CMD_START) {
        if (btif_a2dp_source_is_hal_v2_enabled()) {
          bluetooth::audio::a2dp::ack_stream_started(A2DP_CTRL_ACK_SUCCESS);
          if (btif_av_get_peer_sep() == AVDT_TSEP_SNK) {
            /* Start the media task to encode the audio */
             if(bluetooth::audio::a2dp::get_session_type() ==
                  SessionType::A2DP_SOFTWARE_ENCODING_DATAPATH) {
               APPL_TRACE_IMP("%s: start audio as it is SW session", __func__);
               btif_av_reset_reconfig_flag();
               btif_a2dp_source_start_audio_req();
             }
          }
        } else {
          btif_a2dp_command_ack(A2DP_CTRL_ACK_SUCCESS);
        }
      }
      return true;
    }
  }

  APPL_TRACE_WARNING(
      "%s: pending_start = %d status = %d suspending = %d initiator = %d",
      __func__, pending_start, p_av_start->status, p_av_start->suspending,
      p_av_start->initiator);

  if (p_av_start->status == BTA_AV_SUCCESS) {
    if (!p_av_start->suspending) {
      if (p_av_start->initiator) {
        if (pending_start) {
          if (btif_av_is_split_a2dp_enabled()) {
            btif_dispatch_sm_event(BTIF_AV_OFFLOAD_START_REQ_EVT, (char *)&hdl, 1);
          } else {
            if (pending_cmd == A2DP_CTRL_CMD_START) {
              if (btif_a2dp_source_is_hal_v2_enabled()) {
                bluetooth::audio::a2dp::ack_stream_started(A2DP_CTRL_ACK_SUCCESS);
                if (btif_av_get_peer_sep() == AVDT_TSEP_SNK) {
                  /* Start the media task to encode the audio */
                  if(bluetooth::audio::a2dp::get_session_type() ==
                       SessionType::A2DP_SOFTWARE_ENCODING_DATAPATH) {
                    APPL_TRACE_IMP("start audio as it is SW session");
                    btif_a2dp_source_start_audio_req();
                  }
                }
              } else {
                btif_a2dp_command_ack(A2DP_CTRL_ACK_SUCCESS);
              }
            }
          }
          ack = true;
        }
      } else {
        /* We were remotely started, make sure codec
         * is setup before datapath is started.
         */
         if (!btif_a2dp_source_is_hal_v2_enabled()){
             btif_a2dp_source_setup_codec(hdl);
         }
         if (btif_av_is_split_a2dp_enabled()) {
           APPL_TRACE_IMP("Do not Initiate VSC exchange on remote start");
         }
         ack = true;
      }

      /* media task is autostarted upon a2dp audiopath connection */
    }
  } else if (pending_start) {
    APPL_TRACE_WARNING("%s: A2DP start request failed: status = %d", __func__,
                       p_av_start->status);
    if (property_get("persist.vendor.bt.a2dp.hal.implementation", a2dp_hal_imp, "false") &&
        !strcmp(a2dp_hal_imp, "true")) {
      int index = (hdl & BTA_AV_HNDL_MSK) - 1;
      RawAddress addr = btif_av_get_addr_by_index(index);
      if (btif_a2dp_source_is_hal_v2_enabled()) {
        bluetooth::audio::a2dp::ack_stream_started(A2DP_CTRL_ACK_FAILURE);
      } else if (btif_av_is_split_a2dp_enabled()) {
        btif_a2dp_audio_on_started(A2DP_CTRL_ACK_FAILURE);
        btif_a2dp_audio_reset_pending_cmds();
      } else {
        btif_a2dp_command_ack(A2DP_CTRL_ACK_FAILURE);
        btif_a2dp_pending_cmds_reset();
      }
      if (!addr.IsEmpty()) {
        btif_dispatch_sm_event(BTIF_AV_DISCONNECT_REQ_EVT, (void *)addr.address,
            sizeof(RawAddress));
        BTIF_TRACE_DEBUG("%s:Trigger disconnect for peer device on Start fail by Remote", __func__);
      }
    } else {
      if (btif_av_is_split_a2dp_enabled()) {
        if (btif_a2dp_source_is_hal_v2_enabled()) {
          bluetooth::audio::a2dp::ack_stream_started(A2DP_CTRL_ACK_LONG_WAIT_ERR);
        } else {
          btif_a2dp_audio_on_started(A2DP_CTRL_ACK_FAILURE);
        }
      } else {
        if (pending_cmd == A2DP_CTRL_CMD_START) {
          if (btif_a2dp_source_is_hal_v2_enabled()) {
            bluetooth::audio::a2dp::ack_stream_started(A2DP_CTRL_ACK_LONG_WAIT_ERR);
          } else {
            btif_a2dp_command_ack(A2DP_CTRL_ACK_FAILURE);
          }
        }
      }
    }
    ack = true;
  }
#if (TWS_STATE_ENABLED)
  if (btif_av_current_device_is_tws() &&
    p_av_start->status != BTA_AV_SUCCESS && p_av_start->role == 0xFF) {
    if (btif_av_is_split_a2dp_enabled())
      btif_a2dp_audio_on_started(A2DP_CTRL_ACK_FAILURE);
  }
#endif
  return ack;
}

void btif_a2dp_on_stopped(tBTA_AV_SUSPEND* p_av_suspend) {
  APPL_TRACE_WARNING("## ON A2DP STOPPED ##");

  if (btif_av_get_peer_sep() == AVDT_TSEP_SRC) {
    btif_a2dp_sink_on_stopped(p_av_suspend);
    return;
  }
  if (!btif_av_is_split_a2dp_enabled()) {
      btif_a2dp_source_on_stopped(p_av_suspend);
  } else { //TODO send command to btif_a2dp_audio_interface
    if (btif_a2dp_audio_if_init || btif_a2dp_source_is_hal_v2_supported()) {
      if (p_av_suspend != NULL) {
        if (btif_a2dp_source_is_hal_v2_enabled()) {
          if(p_av_suspend->status != BTA_AV_SUCCESS) {
            if(p_av_suspend->initiator) {
              bluetooth::audio::a2dp::ack_stream_suspended(A2DP_CTRL_ACK_FAILURE);
            }
          } else {
            bluetooth::audio::a2dp::ack_stream_suspended(A2DP_CTRL_ACK_SUCCESS);
          }
        } else {
          btif_a2dp_audio_on_stopped(p_av_suspend->status);
        }
        if (property_get("persist.vendor.bt.a2dp.hal.implementation", a2dp_hal_imp, "false") &&
            !strcmp(a2dp_hal_imp, "true") && (p_av_suspend->status != BTA_AV_SUCCESS)) {
          int index = ((p_av_suspend->hndl) & BTA_AV_HNDL_MSK) - 1;
          RawAddress addr = btif_av_get_addr_by_index(index);
          if (btif_a2dp_source_is_hal_v2_enabled()) {
            bluetooth::audio::a2dp::reset_pending_command();
          } else {
            btif_a2dp_audio_reset_pending_cmds();
          }
          if (!addr.IsEmpty()) {
            btif_dispatch_sm_event(BTIF_AV_DISCONNECT_REQ_EVT, (void *)addr.address,
                sizeof(RawAddress));
            BTIF_TRACE_DEBUG("%s: Disconnect for peer device on Start fail by Remote", __func__);
          }
        }
      }
    }
    else
        APPL_TRACE_EVENT("btif_a2dp_on_stopped, audio interface not up");
  }

  if (btif_a2dp_source_is_hal_v2_supported() &&
      (btif_a2dp_source_cb.tx_flush == true)) {
    btif_a2dp_source_cb.tx_flush = false;
    BTIF_TRACE_DEBUG("%s: clear tx_flush on ack of stop.",__func__);
  }
}

void btif_a2dp_on_suspended(tBTA_AV_SUSPEND* p_av_suspend) {
  APPL_TRACE_EVENT("## ON A2DP SUSPENDED ##");
  if (!btif_av_is_split_a2dp_enabled()) {
    if (btif_av_get_peer_sep() == AVDT_TSEP_SRC) {
      btif_a2dp_sink_on_suspended(p_av_suspend);
    } else {
      btif_a2dp_source_on_suspended(p_av_suspend);
    }
  } else {
    if (p_av_suspend != NULL) {
      if (btif_a2dp_source_is_hal_v2_enabled()) {
        if(p_av_suspend->status != BTA_AV_SUCCESS) {
          if(p_av_suspend->initiator) {
            bluetooth::audio::a2dp::ack_stream_suspended(A2DP_CTRL_ACK_FAILURE);
          }
        } else {
          bluetooth::audio::a2dp::ack_stream_suspended(A2DP_CTRL_ACK_SUCCESS);
        }
      } else {
        btif_a2dp_audio_on_suspended(p_av_suspend->status);
      }
      if (property_get("persist.vendor.bt.a2dp.hal.implementation", a2dp_hal_imp, "false") &&
          !strcmp(a2dp_hal_imp, "true") && (p_av_suspend->status != BTA_AV_SUCCESS)) {
        int index = ((p_av_suspend->hndl) & BTA_AV_HNDL_MSK) - 1;
        RawAddress addr = btif_av_get_addr_by_index(index);
        if (btif_a2dp_source_is_hal_v2_enabled()) {
          bluetooth::audio::a2dp::reset_pending_command();
        } else {
          btif_a2dp_audio_reset_pending_cmds();
        }

        if (!addr.IsEmpty()) {
          btif_dispatch_sm_event(BTIF_AV_DISCONNECT_REQ_EVT, (void *)addr.address,
              sizeof(RawAddress));
          BTIF_TRACE_DEBUG("%s: Disconnect for peer device on Start fail by Remote", __func__);
        }
      }
    }
  }

  if (btif_a2dp_source_is_hal_v2_supported() &&
      (btif_a2dp_source_cb.tx_flush == true)) {
    btif_a2dp_source_cb.tx_flush = false;
    BTIF_TRACE_DEBUG("%s: clear tx_flush on ack of suspend.",__func__);
  }
}

tA2DP_CTRL_ACK ba_map_status_to_ack(uint8_t result) {
  tA2DP_CTRL_ACK ack = A2DP_CTRL_ACK_FAILURE;
  switch (result) {
     case BA_CTRL_ACK_SUCCESS:
       ack = A2DP_CTRL_ACK_SUCCESS;
       break;
     case BA_CTRL_ACK_FAILURE:
       ack = A2DP_CTRL_ACK_FAILURE;
       break;
     default:
       ack = A2DP_CTRL_ACK_FAILURE;
       break;
   }
   return ack;
}

void btif_ba_audio_on_suspended(uint8_t result) {
  APPL_TRACE_EVENT("%s result %d", __func__, result);
  if (btif_a2dp_source_is_hal_v2_enabled()) {
    bluetooth::audio::a2dp::ack_stream_suspended(ba_map_status_to_ack(result));
  } else {
    btif_a2dp_audio_on_suspended(result);
  }
}

void btif_ba_audio_on_stopped(uint8_t result) {
  APPL_TRACE_EVENT("%s result %d", __func__, result);
  if (btif_a2dp_source_is_hal_v2_enabled()) {
    bluetooth::audio::a2dp::ack_stream_suspended(ba_map_status_to_ack(result));
  } else {
    btif_a2dp_audio_on_stopped(result);
  }
}

void btif_ba_audio_on_started(uint8_t result) {
  APPL_TRACE_EVENT("%s result %d", __func__, result);
  if (btif_a2dp_source_is_hal_v2_enabled()) {
    bluetooth::audio::a2dp::ack_stream_started(ba_map_status_to_ack(result));
  } else {
    btif_a2dp_audio_on_started(result);
  }
}

void btif_a2dp_on_offload_started(tBTA_AV_STATUS status) {
  tA2DP_CTRL_ACK ack;
  APPL_TRACE_EVENT("%s status %d", __func__, status);

  switch (status) {
    case BTA_AV_SUCCESS:
      btif_a2dp_src_vsc.tx_start_initiated = FALSE;
      btif_a2dp_src_vsc.tx_started = TRUE;
      ack = A2DP_CTRL_ACK_SUCCESS;
      break;
    case BTA_AV_FAIL_RESOURCES:
      APPL_TRACE_ERROR("%s FAILED UNSUPPORTED", __func__);
      btif_a2dp_src_vsc.tx_start_initiated = FALSE;
      ack = A2DP_CTRL_ACK_UNSUPPORTED;
      break;
    case BTA_AV_FAIL_UNSUPPORTED:
      APPL_TRACE_ERROR("%s Facking success to MM: status = %d", __func__, status);
      btif_a2dp_src_vsc.tx_start_initiated = FALSE;
      ack = A2DP_CTRL_ACK_SUCCESS;
      status = BTA_AV_SUCCESS;
      break;
    default:
      APPL_TRACE_ERROR("%s FAILED: status = %d", __func__, status);
      btif_a2dp_src_vsc.tx_start_initiated = FALSE;
      ack = A2DP_CTRL_ACK_FAILURE;
      break;
  }
  if (btif_av_is_split_a2dp_enabled()) {
    btif_av_reset_reconfig_flag();
    if (ack != BTA_AV_SUCCESS &&
        btif_av_stream_started_ready()) {
      /* Offload request will return with failure from btif_av sm if
      ** suspend is triggered for remote start. Disconnect only if SoC
      ** returned failure for offload VSC
      */
      APPL_TRACE_ERROR("%s offload start failed", __func__);
      RawAddress bd_addr;
      btif_av_get_active_peer_addr(&bd_addr);
     /* status 13 means that there is a sco connection request during start vs cmd
        in such case, will not disconnect the stack, this trigger connect again*/
      if (status != BTIF_A2DP_START_BLOCK_SCO_CONNECTED)
          btif_dispatch_sm_event(BTIF_AV_DISCONNECT_REQ_EVT, (void *)bd_addr.address,
                             sizeof(RawAddress));
    }
  }
  if (btif_a2dp_source_is_hal_v2_enabled()) {
    bluetooth::audio::a2dp::ack_stream_started(ack);
  } else {
    btif_a2dp_audio_on_started(status);
  }
}

void btif_a2dp_honor_remote_start(struct alarm_t **remote_start_alarm, int index) {
  APPL_TRACE_WARNING("%s",__func__);
  btif_a2dp_source_on_remote_start(remote_start_alarm, index);
}

void btif_debug_a2dp_dump(int fd) {
  btif_a2dp_source_debug_dump(fd);
  btif_a2dp_sink_debug_dump(fd);
}
