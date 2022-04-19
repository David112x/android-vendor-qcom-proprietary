/*
 * Copyright (c) 2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a Contribution.
 * Apache license notifications and license are retained
 * for attribution purposes only.
 */
//
// Copyright 2017 The Android Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <hidl/HidlSupport.h>
#include "uart_controller.h"
#include "hci_uart_transport.h"
#include "power_manager.h"
#include <android-base/logging.h>
#include <cutils/properties.h>
#include <utils/Log.h>
#include "state_info.h"
#undef LOG_TAG
#define LOG_TAG "vendor.qti.bluetooth@1.0-uart_controller"

#define PROC_PANIC_PATH     "/proc/sysrq-trigger"
#define BT_MAX_HCI_PKT_SIZE (1024 + 76) /* Max pkt size + safe margin */
#define MAX_INVALID_BYTES 2
#define HW_CRASH_TIMEOUT (1000)
#define CLIENT_INIT_TIMEOUT (1000)

namespace {

using android::hardware::bluetooth::V1_0::implementation::UartController;

ProtocolType GetProtocol(HciPacketType pkt_type)
{
  ProtocolType type = TYPE_BT;

  switch (pkt_type) {
    case HCI_PACKET_TYPE_COMMAND:
    case HCI_PACKET_TYPE_ACL_DATA:
    case HCI_PACKET_TYPE_SCO_DATA:
    case HCI_PACKET_TYPE_EVENT:
      type = TYPE_BT;
      break;
    case HCI_PACKET_TYPE_ANT_CTRL:
    case HCI_PACKET_TYPE_ANT_DATA:
      type = TYPE_ANT;
      break;
    case HCI_PACKET_TYPE_FM_CMD:
    case HCI_PACKET_TYPE_FM_EVENT:
      type = TYPE_FM;
      break;
    default:
      break;
  }
  return type;
};

}

namespace android {
namespace hardware {
namespace bluetooth {
namespace V1_0 {
namespace implementation {

const int LEN_PACKET_TO_SOC = 1100;
extern bool soc_need_reload_patch;
std::mutex rx_thread_timer_mutex_;
std::mutex rx_thread_state_mutex_;
PrimaryReasonCode actual_reason;
std::chrono::time_point<std::chrono::steady_clock> RxTimerStartTs, RxTimerSchedTs;
using PacketReadCallback = std::function<void(ProtocolType, HciPacketType, const hidl_vec<uint8_t>*)>;

UartController::UartController(BluetoothSocType soc_type)
  : soc_crashed(false), soc_type_(soc_type),
  hci_packetizer_([this](hidl_vec<uint8_t> *data) { OnPacketReady(data); })
{
  memset(&add_on_features, 0, sizeof(add_on_features));
  init_done_ = false;
  hci_transport_ = nullptr;
  is_cmd_timeout = false;
  is_spurious_wake = false;
  is_invalid_pkt_from_soc = false;
  is_soc_wakeup_failure = false;
  force_special_byte_enabled_ = false;
  invalid_bytes_counter_ = 0;
  logger_ = Logger::Get();
  state_info_  = BtState::Get();
  client_init_thread_.timer_state = TIMER_NOT_CREATED;
  client_init_thread_.timer_id = NULL;
  client_init_thread_.timeout_ms = 0;
  memset(&rx_timer_state_machine_, 0 , sizeof(RxThreadTimer));
  logger_->RegisterCrashDumpCallback([this](bool crash_dump_in_progress) {
    char value[PROPERTY_VALUE_MAX] = {'\0'};
#ifdef ENABLE_FW_CRASH_DUMP
    const char* default_value = "1"; //crash device after collecting crash dump
    if (soc_type_ == BT_SOC_HASTINGS)
      default_value = "3";          //This is a WAR and will be reverted later
#else
    const char* default_value = "3"; //restart BT host after collecting crash dump
#endif

    property_get("persist.vendor.service.bdroid.ssrlvl", value, default_value);
    int ssr_level = atoi(value);
    ALOGI("%s: ssr_level set to %d\n", __func__, ssr_level);
    if (crash_dump_in_progress) {
      StartSocCrashWaitTimer(SSR_TIMEOUT);
      //After SOC crashes, it will reset IBS to false
      soc_crashed = true;
    } else {
      StopSocCrashWaitTimer();
      SignalCrashDumpFinish();

      if (is_spurious_wake || is_invalid_pkt_from_soc) {
        ALOGI("%s: crashed due to spurious wake or invalid bytes from SoC\n", __func__);
        assert(false);
      }

      if ((ssr_level == 1 || ssr_level == 2) &&
          (is_cmd_timeout == false) &&
          (is_soc_wakeup_failure == false)) {
        logger_->StoreCrashReason();
        BtState :: Get()->DumpBtState();
        /* call kernel panic to report it to crashscope */
        bt_kernel_panic();
      } else {
        // We only dumps logs when dumped_uart_log is false hence reason is none here.
        ReportSocFailure(true, REASON_DEFAULT_NONE, true, true);
      }

      is_spurious_wake = false;
      is_cmd_timeout = false;
      is_invalid_pkt_from_soc = false;
      is_soc_wakeup_failure = false;
    }
  });

  logger_->RegisterHWEventCallback([this]() {
    /* There are chances of receiving HW error with out any dump or
     * before dump. Will start a timer to wait for the dump to be received
     * or else when timer overflows ReportSoCfailure is triggered.
     */
    StartSocCrashWaitTimer(HW_CRASH_TIMEOUT);
  });
  //force special byte to be sent to SOC
#ifdef ENABLE_FW_CRASH_DUMP
  const char* default_send_special_byte = "true";
#else
  const char* default_send_special_byte = "false";
#endif

  char value[PROPERTY_VALUE_MAX] = { 0 };
  if (property_get("vendor.wc_transport.force_special_byte", value,
                    default_send_special_byte)) {
    force_special_byte_enabled_ = (strcmp(value, "false") == 0) ? false : true;
  }

  soc_crash_wait_timer_state_ = TIMER_NOT_CREATED;
}

void UartController::bt_kernel_panic(void)
{
  char panic_set ='c';

  ALOGV("%s: open: %s", __func__, PROC_PANIC_PATH);
  int fd_sysrq = open(PROC_PANIC_PATH, O_WRONLY);
  if(fd_sysrq < 0) {
    ALOGE("%s: open (%s) fail - %s (%d)", __func__,
            PROC_PANIC_PATH, strerror(errno), errno);
    //ignore if panic path can't be opened
  }
  else if (write(fd_sysrq, &panic_set, 1) < 0) {
    ALOGE("%s: write (%s) fail - %s (%d)", __func__, PROC_PANIC_PATH, strerror(errno), errno);
  }

  return;
}

void UartController::WaitforCrashdumpFinish()
{
  if (soc_crash_wait_timer_state_ != TIMER_ACTIVE && Logger::Get()->isSsrTriggered() == false) {
   return;
  }

  if (Logger::Get()->isSsrTriggered() && (soc_crash_wait_timer_state_ != TIMER_ACTIVE)) {
    StartSocCrashWaitTimer(SSR_TIMEOUT);
  }

  ALOGD("%s: Wait for collecting crash dump to finish\n", __func__);
  std::unique_lock<std::mutex> lk(cv_m);
  if (cv.wait_for(lk, std::chrono::seconds(10), [this]{
    return (soc_crash_wait_timer_state_ != TIMER_ACTIVE);})) {
    ALOGD("%s: finished collecting crashdump\n", __func__);
  } else {
    ALOGD("%s: collecting crashdump timed out\n", __func__);
  }
}

void UartController::SignalCrashDumpFinish()
{
  std::lock_guard<std::mutex> lk(cv_m);
  ALOGD("%s: notify the waiting clients \n", __func__);
  cv.notify_all();
}

void UartController::HciTransportCleanup()
{
  std::unique_lock<std::mutex> guard(controller_mutex);
  if (hci_transport_ != nullptr) {
    ALOGD("%s: deleting hci_transport", __func__);
    if (!Logger::Get()->isSsrTriggered())
      (static_cast<HciUartTransport* >(hci_transport_))->Disconnect(soc_need_reload_patch);
    else
      /* incase when ssr is triggered we do complete transport cleanup */
      (static_cast<HciUartTransport* >(hci_transport_))->Disconnect();
    delete hci_transport_;
    hci_transport_ = nullptr;
  } else {
    ALOGD("%s: Unable to delete hci_transport", __func__);
  }
}

bool UartController::Init(PacketReadCallback pkt_read_cb)
{
  HciUartTransport* uart_transport = nullptr;
  PatchDLManager* patch_dl_manager = nullptr;
  char skip_patch_download[PROPERTY_VALUE_MAX];
  features_list *temp_add_on_features = NULL;
  char wklk_buff[MAX_BUFF_SIZE];
  char dst_buff[MAX_BUFF_SIZE];
  struct timeval tv;
  if (init_done_) {
    ALOGD("already initialized, return");
    return true;
  }

  ALOGI("soc need reload patch = %d", soc_need_reload_patch);
  read_cb_ = pkt_read_cb;
  power_manager_.Init(soc_type_);
  if (soc_need_reload_patch) {
    // power off the chip first
    power_manager_.SetPower(false);

  // power on the chip using power manager
    power_manager_.SetPower(true);
  }
#ifdef WAKE_LOCK_ENABLED
  gettimeofday(&time_wk_lockacq_rel_, NULL);
  snprintf(wklk_buff, MAX_BUFF_SIZE, "Wakelock Acquired by UartController %s", __func__);
  state_info_->AddLogTag(dst_buff, time_wk_lockacq_rel_, wklk_buff);
  state_info_->SetWakeLockAcqTimeName(dst_buff);
  Wakelock :: Acquire();
#endif

  if (!(health_info = new (std::nothrow)HealthInfoLog()))
    goto error;
  // initialize the HCI transport
  if (!(uart_transport = new HciUartTransport(health_info))) {
    goto error;
  }

  hci_transport_ = static_cast<HciTransport*> (uart_transport);

  logger_->SetSecondaryCrashReason(SEC_REASON_BTON_UARTINIT_STUCK);
  uart_transport->Init(soc_type_, soc_need_reload_patch);
  patch_dl_manager = new PatchDLManager(soc_type_, uart_transport, &power_manager_);
  if (!patch_dl_manager) {
    goto error;
  }
  sibs_enabled_ = patch_dl_manager->IsSibsEnabled();
  ALOGV("%s: sibs_enabled = %d\n ", __func__, sibs_enabled_);
#ifdef WCNSS_IBS_ENABLED
  if (sibs_enabled_)
    IbsHandler::Init(uart_transport, health_info, this);
#endif

  uart_transport->ClockOperation(USERIAL_OP_CLK_ON);

  //Download the NVM/RAM patch
  if (soc_need_reload_patch) {
    property_get("vendor.wc_transport.skip_patch_dload", skip_patch_download, "false");
    if (strcmp(skip_patch_download, "true") != 0) {
      if (patch_dl_manager->PerformChipInit() < 0) {
      // Always turn UART clock off even if BT fail to boot.
        uart_transport->ClockOperation(USERIAL_OP_CLK_OFF);
        goto error;
      }
      temp_add_on_features = patch_dl_manager->GetAddOnFeatureList();

      if (temp_add_on_features != NULL) {
        ALOGD("add on features read true");
        memcpy(&add_on_features, temp_add_on_features, sizeof(features_list));
      } else {
        ALOGD("add on features read false");
      }
    } else {
      /* Change baud rate 115.2 kbps to 3Mbps*/
      if (patch_dl_manager->SetBaudRateReq() < 0) {
        ALOGE("%s: Baud rate change failed!", __func__);
        goto error;
      }
      ALOGI("%s: Baud rate changed successfully ", __func__);
    }
  }

  // set up the fd watcher now
  if ( fd_watcher_.WatchFdForNonBlockingReads(
    uart_transport->GetCtrlFd(),
    [this](int fd) { OnDataReady(fd); })== 0) {
    gettimeofday(&tv, NULL);
    memset(dst_buff, 0, MAX_BUFF_SIZE);
    state_info_->AddLogTag(dst_buff, tv, (char *)"Read thread was started: SUCCESS");
    state_info_->SetReadThreadStatus(dst_buff);
  } else {
    gettimeofday(&tv, NULL);
    memset(dst_buff, 0, MAX_BUFF_SIZE);
    state_info_->AddLogTag(dst_buff, tv, (char *)"Read thread FAILED to start");
    state_info_->SetReadThreadStatus(dst_buff);
  }

  /* (1) If IBS is enabled (default), UART clock can be voted on/off
         dynamically by IbsHandler. So it's suitable to turn UART
         clock off after patch downloading is completed.
     (2) If IBS is disabled, UART clock should always be turned on.
         Otherwise, BT fail to work after patch downloading. */
#ifdef WCNSS_IBS_ENABLED
  if (sibs_enabled_) {
    uart_transport->ClockOperation(USERIAL_OP_CLK_OFF);
  }
#endif

#ifdef WAKE_LOCK_ENABLED
  gettimeofday(&tv, NULL);
  memset(dst_buff, 0, MAX_BUFF_SIZE);
  state_info_->AddLogTag(dst_buff, tv, (char *)"Wakelock::ReleaseDelay 500ms during UART init");
  state_info_->SetWakeLockRelTimeName(dst_buff);
  Wakelock :: ReleaseDelay(500);
#endif

  if (patch_dl_manager) {
    delete patch_dl_manager;
    patch_dl_manager = nullptr;
  }

  /* calling diag LSM init if DIAG_ENABLED is set*/
  Logger::Get()->Init(hci_transport_);

#ifdef ENABLE_HEALTH_TIMER
  if (health_info->HealthStatisticsTimerStart()== TIMER_ACTIVE) {
    memset(dst_buff, 0, MAX_BUFF_SIZE);
    strlcpy(dst_buff, "HealthStatisticsTimerStart started: SUCCESS", MAX_BUFF_SIZE);
    state_info_->SetHelthStatsTimerStatus(dst_buff);
  } else {
    memset(dst_buff, 0, MAX_BUFF_SIZE);
    strlcpy(dst_buff, "HealthStatisticsTimerStart FAILED to Start", MAX_BUFF_SIZE);
    state_info_->SetHelthStatsTimerStatus(dst_buff);
  }
#endif
  init_done_ = true;

  // Move Bt SoC to retention mode. SW_CTRL will control the voting now
  if (soc_type_ == BT_SOC_HASTINGS)
    power_manager_.SetPower(true, true);

  logger_->SetSecondaryCrashReason(SEC_REASON_DEFAULT);
  ALOGD("Init succeded");
  return init_done_;

 error:
  ALOGE("Init failed");
  if (patch_dl_manager) {
    delete patch_dl_manager;
    patch_dl_manager = nullptr;
  }

#ifdef WCNSS_IBS_ENABLED
  if (sibs_enabled_ && IbsHandler::Get()) {
    IbsHandler::Get()->CleanUp();
  }
#endif

  soc_need_reload_patch = true;
  HciTransportCleanup();
  if (health_info) {
    delete health_info;
    health_info = nullptr;
  }
#ifdef WAKE_LOCK_ENABLED
  gettimeofday(&time_wk_lockacq_rel_, NULL);
  snprintf(wklk_buff, MAX_BUFF_SIZE, "Released by UartController %s", __func__);
  memset(wklk_buff, 0, MAX_BUFF_SIZE);
  memset(dst_buff, 0, MAX_BUFF_SIZE);
  state_info_->AddLogTag(dst_buff, time_wk_lockacq_rel_, wklk_buff);
  state_info_->SetWakeLockRelTimeName(dst_buff);
  Wakelock :: Release();
#endif

  return init_done_;
}

void UartController::StartSocCrashWaitTimer()
{
  StartSocCrashWaitTimer(SSR_TIMEOUT);
}

void UartController::StartSocCrashWaitTimer(int ssrtimeout)
{
  int status;
  struct itimerspec ts;
  struct sigevent se;
  uint32_t timeout_ms;

  ALOGV("%s", __func__);

  if (soc_crash_wait_timer_state_ == TIMER_NOT_CREATED) {
    se.sigev_notify_function = (void (*)(union sigval))SocCrashWaitTimeout;
    se.sigev_notify = SIGEV_THREAD;
    se.sigev_value.sival_ptr = this;
    se.sigev_notify_attributes = NULL;

    status = timer_create(CLOCK_MONOTONIC, &se, &soc_crash_wait_timer_);
    if (status == 0) {
      ALOGV("%s: SoC Crash wait timer created", __func__);
      soc_crash_wait_timer_state_ = TIMER_CREATED;
    } else {
      ALOGE("%s: Error creating timer %d\n", __func__, status);
    }
  }

  if (soc_crash_wait_timer_state_ == TIMER_CREATED || soc_crash_wait_timer_state_ == TIMER_ACTIVE) {
    timeout_ms = ssrtimeout;
    ts.it_value.tv_sec = timeout_ms / 1000;
    ts.it_value.tv_nsec = 1000000 * (timeout_ms % 1000);
    ts.it_interval.tv_sec = 0;
    ts.it_interval.tv_nsec = 0;

    status = timer_settime(soc_crash_wait_timer_, 0, &ts, 0);
    if (status < 0)
      ALOGE("%s:Failed to set soc Crash wait timer: %d", __func__, status);
    else {
      ALOGV("%s: time started", __func__);
      soc_crash_wait_timer_state_ = TIMER_ACTIVE;
    }
  }
}

bool UartController::StopSocCrashWaitTimer()
{
  int status;
  struct itimerspec ts;
  bool was_active = false;

  ALOGV("%s", __func__);

  if (soc_crash_wait_timer_state_ == TIMER_ACTIVE) {
    ts.it_value.tv_sec = 0;
    ts.it_value.tv_nsec = 0;
    ts.it_interval.tv_sec = 0;
    ts.it_interval.tv_nsec = 0;
    was_active = true;
    status = timer_settime(soc_crash_wait_timer_, 0, &ts, 0);
    if (status == -1)
      ALOGE("%s:Failed to stop soc Crash wait timer", __func__);
    else if (status == 0) {
      ALOGV("%s: Soc crash timer Stopped", __func__);
      soc_crash_wait_timer_state_ = TIMER_CREATED;
    }
  }
  return was_active;
}

void UartController::CleanupSocCrashWaitTimer()
{
  if (soc_crash_wait_timer_state_ == TIMER_ACTIVE)
    StopSocCrashWaitTimer();
  if (soc_crash_wait_timer_state_ == TIMER_CREATED) {
    timer_delete(soc_crash_wait_timer_);
    soc_crash_wait_timer_state_ = TIMER_NOT_CREATED;
    soc_crash_wait_timer_ = NULL;
  }
}

void UartController::SsrCleanup(PrimaryReasonCode reason)
{
  if (!Logger::Get()->SetSsrTriggeredFlag()) {
    ALOGE("%s: Returning as SSR or cleanup in progress", __func__);
    return;
  }

  logger_->SetPrimaryCrashReason(reason);

  /*Indicate it by sending special byte */
  if (reason == REASON_SSR_CMD_TIMEDOUT || reason == REASON_SSR_SPURIOUS_WAKEUP ||
      reason == REASON_SSR_INVALID_BYTES_RCVD ||
      reason == REASON_SSR_UNABLE_TO_WAKEUP_SOC ||
      reason == REASON_SSR_RCVD_LARGE_PKT_FROM_SOC) {
    if (force_special_byte_enabled_ && !logger_->IsCrashDumpStarted()) {
      if (reason == REASON_SSR_CMD_TIMEDOUT) {
        is_cmd_timeout = true;
      } else if (reason == REASON_SSR_SPURIOUS_WAKEUP) {
        is_spurious_wake = true;
      } else if (reason == REASON_SSR_INVALID_BYTES_RCVD ||
        reason == REASON_SSR_RCVD_LARGE_PKT_FROM_SOC) {
        is_invalid_pkt_from_soc = true;
      } else if (reason == REASON_SSR_UNABLE_TO_WAKEUP_SOC) {
        is_soc_wakeup_failure = true;
      }

      ALOGD("%s: SSR triggered due to %d sending special buffer", __func__, reason);
      BtState::Get()->SetSSRtrigger(true);
      SendSpecialBuffer();
    } else if (!force_special_byte_enabled_) {
      /*  Kill HIDL daemon for graceful recovery after SSR conditions are
       *  triggerred in USER builds, where vendor.wc_transport.force_special_byte
       * flag is set to false.
       */
      ALOGD("%s: SSR triggered due to %d skip sending special buffer",
            __func__, reason);
      ReportSocFailure(false, reason, true, false);
    }
  } else if (reason == REASON_RX_THREAD_STUCK) {
    ReportSocFailure(false, REASON_RX_THREAD_STUCK, true, true);
  } else {
    Cleanup();
  }
}

bool UartController::IsSecondaryReasonValid()
{
  if (!strcmp(logger_->GetSecondaryCrashReason(), "Default"))
    return false;
  else
    return true;
}

PrimaryReasonCode UartController::GetPrimaryReason()
{
  return logger_->GetPrimaryReason();
}

void UartController::SocCrashWaitTimeout(union sigval sig)
{
  UartController *uart_controller = static_cast<UartController*>(sig.sival_ptr);
  ALOGE("%s: Unable to complete crash dump collection\n", __func__);
  if (uart_controller) {
    PrimaryReasonCode reason = REASON_NONE_SOC_WAIT_TIMEOUT;
    if (uart_controller->IsSecondaryReasonValid()) {
      ALOGE("%s: crash is decoded but dump is disabled\n", __func__);
      reason = uart_controller->GetPrimaryReason();
    } else if (uart_controller->is_spurious_wake) {
      ALOGE("%s: crashed due to spurious wake\n", __func__);
      reason = REASON_SPURIOUS_WAKEUP_SOC_WAIT_TIMEOUT;
    } else if (uart_controller->is_invalid_pkt_from_soc) {
      ALOGE("%s: crashed due to invalid bytes received from SoC\n", __func__);
      reason = REASON_INV_BYTES_SOC_WAIT_TIMEOUT;
    } else if (uart_controller->is_cmd_timeout) {
      ALOGE("%s: crashed due to command timeout\n", __func__);
      reason = REASON_CMD_TIMEDOUT_SOC_WAIT_TIMEOUT;
    } else if (uart_controller->is_soc_wakeup_failure) {
      ALOGE("%s: crashed due to SoC wakeup failure\n", __func__);
      reason = REASON_SOC_WAKEUP_FAILED_SOC_WAIT_TIMEOUT;
    } else if (Logger::Get()->isSsrTriggered()) {
      ALOGE("%s: crash due to diag triggered SSR\n", __func__);
      reason = REASON_SOC_CRASHED_DIAG_SSR_SOC_WAIT_TIMEOUT;
      Logger::Get()->SetPrimaryCrashReason(reason);
    } else {
      /* Ideally we should not reach this part of code, as we should have valid reason
       * from above cases for triggerring SSR and starting CrashTimer
       */
      ALOGE("%s: Crashed due to unknown reason\n", __func__);
    }
    uart_controller->ReportSocFailure(false, reason, false, true);
  }
}

void UartController::SendCrashPacket()
{
  hidl_vec<uint8_t> *bt_packet_ = new hidl_vec<uint8_t>;

  /* Crash reason is posted to client if it is active and they
   * don't trigger close call, if close is triggered observed chances
   * of missing the crash data in stack.
   */
  if (read_cb_ && logger_->GetClientStatus(TYPE_BT) &&
      (logger_->GetCleanupStatus(TYPE_BT) == false) && logger_->FrameCrashEvent(bt_packet_)) {
    ALOGD("%s send crash reasons to the client", __func__);
    read_cb_(GetProtocol(HCI_PACKET_TYPE_EVENT), HCI_PACKET_TYPE_EVENT, bt_packet_);
    /* we can't get the return status of read call, based on client return
     * status we are clearing crash flag. But there could be chances of
     * packet stuck in the binder, we need to see how to address them
     * in future.
     */
    if (logger_->GetClientStatus(TYPE_BT))
      property_set("persist.vendor.bluetooth.crash.reasons", "false");
  } else {
    ALOGE("%s: skip sending crash packet", __func__);
  }
}

void UartController::ClientInitTimeOut(union sigval sig)
{
  UartController *uart_controller = static_cast<UartController*>(sig.sival_ptr);

  ALOGD("%s", __func__);
  if (uart_controller)
    uart_controller->SendCrashPacket();
}

void UartController::StopClientInitWaitTimer()
{
  int status;
  struct itimerspec ts;
  if (client_init_thread_.timer_state == TIMER_ACTIVE) {
    ts.it_value.tv_sec = 0;
    ts.it_value.tv_nsec = 0;
    ts.it_interval.tv_sec = 0;
    ts.it_interval.tv_nsec = 0;
    status = timer_settime(client_init_thread_.timer_id, 0, &ts, 0);
    if (status == -1) {
      ALOGE("%s:Failed to stop set client init thread", __func__);
    } else if (status == 0) {
      ALOGD("%s: Client init timer Stopped", __func__);
      client_init_thread_.timer_state = TIMER_CREATED;
    }
  }
}

void UartController::StartClientInitWaitTimer()
{
  int status;
  struct itimerspec ts;
  struct sigevent se;

  ALOGV("%s", __func__);
  if (client_init_thread_.timer_state == TIMER_NOT_CREATED) {
    se.sigev_notify_function = (void (*)(union sigval))ClientInitTimeOut;
    se.sigev_notify = SIGEV_THREAD;
    se.sigev_value.sival_ptr = this;
    se.sigev_notify_attributes = NULL;
    status = timer_create(CLOCK_MONOTONIC, &se, &client_init_thread_.timer_id);
    if (status == 0)
      client_init_thread_.timer_state = TIMER_CREATED;
  }
  if (client_init_thread_.timer_state == TIMER_CREATED ||
      client_init_thread_.timer_state == TIMER_ACTIVE) {
    client_init_thread_.timeout_ms = CLIENT_INIT_TIMEOUT;
    ts.it_value.tv_sec = client_init_thread_.timeout_ms / 1000;
    ts.it_value.tv_nsec = 1000000 * (client_init_thread_.timeout_ms % 1000);
    ts.it_interval.tv_sec = 0;
    ts.it_interval.tv_nsec = 0;
    status = timer_settime(client_init_thread_.timer_id, 0, &ts, 0);
    if (status == -1)
    {
      ALOGE("%s:failed to set wait timeout for Client Init", __func__);
    } else {
      client_init_thread_.timer_state = TIMER_ACTIVE;
      ALOGD("Started client init timer:%s", __func__);
    }
  }
}

void UartController::ReportSocFailure(bool dumped_uart_log, PrimaryReasonCode reason,
              bool cleanupSocCrashWaitTimer, bool cleanupIbs)
{
  unsigned char bt_eve_buf[LENGTH_HW_ERROR_EVT] = { 0x10, 0x01, 0x0f };
  unsigned char fm_eve_buf[LENGTH_HW_ERROR_EVT] = { 0x1A, 0x01, 0x0f };
  unsigned char ant_eve_buf[LENGTH_HW_ERROR_EVT] = { 0x1C, 0x01, 0x0f };
  static bool execution_count = false;
  bool IsRxSchedDelay = false;

  /* Only once we need to excute ReportSoCFailure, we have chances
   * of calling ReportSoCFailure again by the timer threads due to
   * timeouts.
   */
  if (execution_count) {
    ALOGW("returing as other thread is in process of execution of %s", __func__);
    return;
  }

  actual_reason = reason;
  /* If Rx thread might have unblocked, stop closing BT. */
  if (reason == REASON_RX_THREAD_STUCK) {
    if (GetRxThreadTimerState() != TIMER_OVERFLOW) {
      ALOGI("%s: Rx thread unblocked returning back from cleanup", __func__);
      logger_->ResetCrashReasons();
      logger_->ResetForceSsrTriggeredFlag();
      StopSocCrashWaitTimer();
      return;
    }

    long TsDiff = GetRxThreadSchedTSDiff();
    if (TsDiff > (RX_THREAD_USAGE_TIMEOUT + RX_THREAD_SCHEDULING_DELAY)) {
      IsRxSchedDelay = true;
      ALOGE("%s: Might be LMKD is running in background resulting RxThread Scheduling delay",
            __func__);
    }
  }

  execution_count = true;
  ALOGD("%s: reason %d", __func__, reason);

  logger_->StoreCrashReason();

  if (!dumped_uart_log) {
    logger_->PrepareDumpProcess();
#ifdef DUMP_RINGBUF_LOG
    logger_->DumpRingBuffers();
#endif
#ifdef DUMP_IPC_LOG
    logger_->DumpUartIpcLogs();
#endif

#ifdef ENABLE_HEALTH_TIMER
    health_info->ReportHealthInfo();
#endif
  }

  /* No point in sending the HW error event to stack if crash
   * is due to Rx thread stuck.
   */
  if (read_cb_ && reason != REASON_RX_THREAD_STUCK) {
    hidl_vec<uint8_t> *bt_packet_ = new hidl_vec<uint8_t>;
    hidl_vec<uint8_t> *fm_packet_ = new hidl_vec<uint8_t>;
    hidl_vec<uint8_t> *ant_packet_ = new hidl_vec<uint8_t>;
    bt_packet_->resize(LENGTH_HW_ERROR_EVT);
    fm_packet_->resize(LENGTH_HW_ERROR_EVT);
    ant_packet_->resize(LENGTH_HW_ERROR_EVT);

    ALOGD("%s send H/W error event to clients", __func__);
    memcpy(fm_packet_->data(), fm_eve_buf, LENGTH_HW_ERROR_EVT);
    read_cb_(GetProtocol(HCI_PACKET_TYPE_FM_EVENT), HCI_PACKET_TYPE_FM_EVENT, fm_packet_);

    memcpy(bt_packet_->data(), bt_eve_buf, LENGTH_HW_ERROR_EVT);
    read_cb_(GetProtocol(HCI_PACKET_TYPE_EVENT), HCI_PACKET_TYPE_EVENT, bt_packet_);

    memcpy(ant_packet_->data(), ant_eve_buf, LENGTH_HW_ERROR_EVT);
    read_cb_(GetProtocol(HCI_PACKET_TYPE_ANT_CTRL), HCI_PACKET_TYPE_ANT_CTRL, ant_packet_);

    SendCrashPacket();
  }

  /* Stop Rx thread, as from this point we don't need any data from SoC. */
  fd_watcher_.StopThreadRoutine();

  /* Don't perform Cleanup of SocCrashTimeout if ReportSoC Failure is
   * triggerred from SocCrashWaitTimeout, which would stall
   * cleanup progress. Subsequent abort/kill will take
   * care of freeing IBS resources.
   */
  if (cleanupSocCrashWaitTimer) {
    ALOGD("%s calling CleanupSocCrashWaitTimer", __func__);
    CleanupSocCrashWaitTimer();
  }

#ifdef WCNSS_IBS_ENABLED
  if (sibs_enabled_) {
    /* Don't perform IBS cleanup in below condition(s)
     * as this would kill IBS Timer and will stall further
     * cleanup progress. Subsequent abort/kill will take
     * care of freeing IBS resources.
     */
    if (cleanupIbs) {
      ALOGD("%s Cleaning up IBS", __func__);
      IbsHandler::Get()->CleanUp();
    }
  }
#endif

  HciTransportCleanup();

  // turn off the chip
  if (soc_need_reload_patch) {
    power_manager_.SetPower(false);
    power_manager_.Cleanup();
  }

#ifdef WAKE_LOCK_ENABLED
  Wakelock :: CleanUp();
#endif

#ifdef USER_DEBUG
  bool isDiagSsr =  logger_->isDiagSsrTriggered();
#endif
  BtState::Get()->DumpBtState();
#ifdef USER_DEBUG
  if (isDiagSsr) {
    ALOGE("Killing daemon as DIAG SSR is completed !");
    kill(getpid(), SIGKILL);
  } else if (IsRxSchedDelay) {
    ALOGE("Killing daemon as Rxthread timeout callback scheduling is delayed !");
    kill(getpid(), SIGKILL);
  } else if (reason == REASON_RX_THREAD_STUCK &&
             GetRxThreadTimerState() != TIMER_OVERFLOW) {
    ALOGE("Killing daemon as Rxthread is unblocked !");
    kill(getpid(), SIGKILL);
  } else {
    ALOGE("Aborting daemon as SSR is completed !");
    abort();
  }
#else
  ALOGE("Killing daemon as SSR is completed!");
  kill(getpid(), SIGKILL);
#endif
}

void UartController::SendSpecialBuffer()
{
  unsigned char buf[LEN_PACKET_TO_SOC];
  int retval;

  memset(buf, 0xfb, LEN_PACKET_TO_SOC);

  ALOGV("%s: sending special bytes to crash SOC", __func__);

  HciUartTransport* uart_transport = static_cast<HciUartTransport*> (hci_transport_);
  if (hci_transport_ == nullptr) {
    ALOGE("%s: failed to send special buffer", __func__);
    return;
  }

  retval = uart_transport->UartWrite(buf, LEN_PACKET_TO_SOC);
  if (retval <= 0) {
    ALOGE("%s: Write Err : %d (%s)", __func__, retval, strerror(errno));
    ReportSocFailure(false, REASON_WRITE_FAIL_SPCL_BUFF_CRASH_SOC, true, false);
  } else {
    StartSocCrashWaitTimer(SSR_TIMEOUT);
  }
}

/**
  * this is to filter out Get Controller Debug Info cmd from stack.
  * we should crash SOC and collect ram dump after receiving it.
  */
bool UartController::command_is_get_dbg_info(const unsigned char* buf, int len)
{
    bool result = true;
    int i;
    const unsigned char get_dbg_info[] = {0x5B, 0xFD, 0x00};

    ALOGV("%s: lenth = %d ", __func__, len);
    if ( len != 3) {
        result = false;
    }
    else {
        for (i=0; i<len; i++) {
             if(buf[i] != get_dbg_info[i])
                result = false;
        }
    }

    return result;
}

size_t UartController::SendPacket(HciPacketType packet_type, const uint8_t *data, size_t length)
{
  size_t size = 0;
  int ibs_status = 0;

  if (Logger::Get()->isSsrTriggered() || (soc_crash_wait_timer_state_== TIMER_ACTIVE)) {
    /* For Diag initiated SSR, no timer is started when we send
     * a crash command, for safer side starting a timer.
     */
    if (Logger::Get()->isSsrTriggered() && (soc_crash_wait_timer_state_ != TIMER_ACTIVE)) {
      StartSocCrashWaitTimer(SSR_TIMEOUT);
    }

    /* Cleanup will be halted if rx thread unblocked a later point.
     * Check and send the command to the SoC.
     */
    {
      std::unique_lock<std::mutex> guard(rx_thread_timer_mutex_);
      if (actual_reason == REASON_RX_THREAD_STUCK) {
        if (GetRxThreadTimerState() == TIMER_OVERFLOW) {
           ALOGD("<%s: give up Tx since crash dump has started", __func__);
           return size;
        }
        else {
           actual_reason = REASON_DEFAULT_NONE;
        }
      } else {
        ALOGD("<%s: give up Tx since crash dump has started", __func__);
        return size;
      }
    }
  }

#ifdef WCNSS_IBS_ENABLED
  if (sibs_enabled_ && !soc_crashed) {
    IbsHandler* hIbs = IbsHandler::Get();
    if (!hIbs) {
      ALOGW("<%s: Uart socket has been closed", __func__);
      return size;
    } else {
      ibs_status = hIbs->WakeAssert();
      // perform recovery and dump logs only if SoC not crashed
      if (!(DataHandler::Get()->CheckSignalCaughtStatus()) && ibs_status && !soc_crashed) {
        ALOGE("Initiating crash dump collection as unable wake up SOC");
        SsrCleanup(REASON_SSR_UNABLE_TO_WAKEUP_SOC);
        return size;
      }
    }
  }
#endif

  if ((HCI_PACKET_TYPE_COMMAND == packet_type) &&
      command_is_get_dbg_info(data, length)) {
    ALOGD("<%s: received Get_DBG_INFO command", __func__);
    if (is_soc_wakeup_failure == false)
      SsrCleanup(REASON_SSR_CMD_TIMEDOUT);
    else {
      ALOGE("%s: SSR: Unnable to wakeUp SoC, dropping GET_DBG_INFO Command", __func__);
      return size;
    }
  } else if ((ibs_status == 0 || (DataHandler::Get()->CheckSignalCaughtStatus()))
                && hci_transport_ != nullptr) {
    size = hci_transport_->Write(packet_type, data, length);
  } else {
    ALOGE("%s: Dropping packet! packet type = %d ibs_status = %d", __func__, packet_type, ibs_status);
  }

#ifdef WCNSS_IBS_ENABLED
  if (sibs_enabled_ && (ibs_status == 0) && !soc_crashed) {
    IbsHandler::Get()->TransmitDone(true);
  }
#endif

  return size;
}

void UartController::UnlockControllerMutex()
{
   ALOGE("%s: unlocking controller_mutex", __func__);
   controller_mutex.unlock();
}

bool UartController::Disconnect()
{
  StopClientInitWaitTimer();
  fd_watcher_.StopThreadRoutine();
  HciTransportCleanup();

  return true;
}

bool UartController::Cleanup(void)
{
  if (!init_done_) {
    ALOGD("UartController:: already closed, return");
    return false;
  }

  /* Rechecking whether the SSR is in progress, before voting down the regs */
  if (Logger::Get()->isSsrTriggered() || (soc_crash_wait_timer_state_== TIMER_ACTIVE)) {
    ALOGW("waiting for crashdump to finish in %s", __func__);
    WaitforCrashdumpFinish();
    return false;
  }

  ALOGD("UartController::Cleanup, soc_need_reload_patch=%d", soc_need_reload_patch);

  if (!soc_need_reload_patch) {
#ifdef WCNSS_IBS_ENABLED
    if (sibs_enabled_) {
      bool ret;
      IbsHandler::Get()->DeviceSleep();
      ret = IbsHandler::Get()->waitUntilSleepIndRecvd();
      if (!ret) {
        soc_need_reload_patch = true;
      }
    }
#endif
  }

  StopClientInitWaitTimer();
  // stop the fd watcher
  fd_watcher_.StopWatchingFileDescriptors();

  /* Possiblity we are in between dump proces discard dump and stop the timer. */
  CleanupSocCrashWaitTimer();

#ifdef WCNSS_IBS_ENABLED
  if (sibs_enabled_) {
    IbsHandler::Get()->StopAllTimers();
    IbsHandler::Get()->CleanUp();
  }
#endif

  // clean up the transport
  HciTransportCleanup();

  // turn off the chip
  if (soc_need_reload_patch) {
    power_manager_.SetPower(false);
    power_manager_.Cleanup();
  }

  // Cleaning Logger which ensures close of diag port
  Logger::Get()->Cleanup();
  if (DiagInterface::GetDiagDeInitTimeoutTriggered())
    return false;
  delete health_info;

  init_done_ = false;

  return true;
}

void UartController::ResetInvalidByteCounter()
{
    ALOGV("%s", __func__);
    invalid_bytes_counter_ = 0;
}

void UartController::OnPacketReady(hidl_vec<uint8_t> *data)
{
  if(data == nullptr) {
    ALOGE("Error reading data from uart");
    ReportSocFailure(false, REASON_ERROR_READING_DATA_FROM_UART, true, true);
    return;
  }

  if (data->size() > BT_MAX_HCI_PKT_SIZE) {
    ALOGE("Received large pkt %zu from soc trigger ssr",
           data->size());
    SsrCleanup(REASON_SSR_RCVD_LARGE_PKT_FROM_SOC);
    return;
  }

  if (read_cb_) {
    read_cb_(GetProtocol(hci_packet_type_), hci_packet_type_, data);
  }
  // Get ready for the next type byte.
  hci_packet_type_ = HCI_PACKET_TYPE_UNKNOWN;
}

void UartController::OnDataReady(int fd)
{

  if (hci_packet_type_ == HCI_PACKET_TYPE_UNKNOWN) {
    uint8_t buffer[1] = { 0 };
    size_t bytes_read = TEMP_FAILURE_RETRY(read(fd, buffer, 1));
    CHECK(bytes_read == 1);

    hci_packet_type_ = static_cast<HciPacketType>(buffer[0]);

#ifdef WCNSS_IBS_ENABLED
    if (sibs_enabled_) {
      IbsHandler* hIbs = IbsHandler::Get();
      if (hIbs->IsIbsCmd(hci_packet_type_)) {
        hIbs->ProcessIbsCmd( reinterpret_cast <uint8_t *> (&hci_packet_type_));
        hci_packet_type_ = HCI_PACKET_TYPE_UNKNOWN;
        return;
      }
    }
#endif

    if (!IsHciPacketValid(hci_packet_type_)) {
      ALOGE("%s: Invalid hci packet type byte received 0x%x, invalid_bytes_counter_ = %d",
        __func__, hci_packet_type_, invalid_bytes_counter_);
      hci_packet_type_ = HCI_PACKET_TYPE_UNKNOWN;
      invalid_bytes_counter_++;
      if (!is_cmd_timeout &&
          !is_soc_wakeup_failure &&
          (invalid_bytes_counter_ >= MAX_INVALID_BYTES) &&
          (!soc_crashed)) {
        SsrCleanup(REASON_SSR_INVALID_BYTES_RCVD);
      }
    }
  } else {
    hci_packetizer_.OnDataReady(fd, hci_packet_type_);
#ifdef WCNSS_IBS_ENABLED
    if (sibs_enabled_) {
      IbsHandler* hIbs = IbsHandler::Get();
      if (hIbs) {
        hIbs->ibs_spurious_wake_timer_start();
      }
    }
#endif
  }
}

bool UartController::IsHciPacketValid(HciPacketType type)
{
  bool result = false;

  if (HCI_PACKET_TYPE_EVENT == type || HCI_PACKET_TYPE_ACL_DATA == type ||
      HCI_PACKET_TYPE_SCO_DATA == type || HCI_PACKET_TYPE_COMMAND == type ||
      HCI_PACKET_TYPE_ANT_CTRL == type || HCI_PACKET_TYPE_ANT_DATA == type ||
      HCI_PACKET_TYPE_FM_CMD == type || HCI_PACKET_TYPE_FM_EVENT == type) {
    result = true;
  }

  return result;
}

features_list* UartController:: GetAddOnFeatureList()
{
  if (add_on_features.feat_mask_len > 0) {
    return &add_on_features;
  } else {
    return NULL;
  }
}

UartController :: ~UartController()
{
  ALOGV("%s", __func__);
  if (client_init_thread_.timer_state == TIMER_ACTIVE) {
    StopClientInitWaitTimer();
  }
  if (soc_crash_wait_timer_state_ == TIMER_CREATED) {
    timer_delete(client_init_thread_.timer_id);
    client_init_thread_.timer_state = TIMER_NOT_CREATED;
    client_init_thread_.timer_id = NULL;
  }

  StopRxThreadTimer();
  if (GetRxThreadTimerState() == TIMER_CREATED) {
    timer_delete(rx_timer_state_machine_.timer_id);
    SetRxThreadTimerState(TIMER_NOT_CREATED);
  }
}

void UartController::StartRxThreadTimer()
{
  int status;
  struct itimerspec ts;
  struct sigevent se;

  ALOGV("%s", __func__);
  if (GetRxThreadTimerState() == TIMER_NOT_CREATED) {
    se.sigev_notify_function = (void (*)(union sigval))RxThreadTimeOut;
    se.sigev_notify = SIGEV_THREAD;
    se.sigev_value.sival_ptr = this;
    se.sigev_notify_attributes = NULL;

    status = timer_create(CLOCK_MONOTONIC, &se, &rx_timer_state_machine_.timer_id);
    if (status == 0)
      SetRxThreadTimerState(TIMER_CREATED);
  }

  if ((GetRxThreadTimerState() == TIMER_CREATED) ||
      (GetRxThreadTimerState() == TIMER_OVERFLOW)) {
    rx_timer_state_machine_.timeout_ms = RX_THREAD_USAGE_TIMEOUT;
    ts.it_value.tv_sec = rx_timer_state_machine_.timeout_ms / 1000;
    ts.it_value.tv_nsec = 1000000 * (rx_timer_state_machine_.timeout_ms % 1000);
    ts.it_interval.tv_sec = 0;
    ts.it_interval.tv_nsec = 0;

    status = timer_settime(rx_timer_state_machine_.timer_id, 0, &ts, 0);
    if (status == -1)
      ALOGE("%s:Failed to set RxThread Usage timer", __func__);
    else
      SetRxThreadTimerState(TIMER_ACTIVE);
  }

  RxTimerStartTs = std::chrono::steady_clock::now();
}

void UartController::RxThreadTimeOut(union sigval sig)
{
  {
    std::unique_lock<std::mutex> guard(rx_thread_state_mutex_);
    UartController *uart_controller = static_cast<UartController*>(sig.sival_ptr);
    uart_controller->rx_timer_state_machine_.timer_state = TIMER_OVERFLOW;
    RxTimerSchedTs = std::chrono::steady_clock::now();
    ALOGE("%s:Rx thread stuck detected and callback scheduled in TS:%0.2lf ms",
          __func__, uart_controller->GetRxThreadSchedTSDiff());
  }

  {
    std::unique_lock<std::mutex> guard(rx_thread_timer_mutex_);
    UartController *uart_controller = static_cast<UartController*>(sig.sival_ptr);
    if (uart_controller->GetRxThreadTimerState() != TIMER_OVERFLOW) {
      ALOGI("%s: Rx thread is unblocked resuming back", __func__);
      return;
    }

    uart_controller->SsrCleanup(REASON_RX_THREAD_STUCK);
  }
}

void UartController::StopRxThreadTimer()
{
  int status;
  struct itimerspec ts;

  if (GetRxThreadTimerState() != TIMER_NOT_CREATED) {
    ts.it_value.tv_sec = 0;
    ts.it_value.tv_nsec = 0;
    ts.it_interval.tv_sec = 0;
    ts.it_interval.tv_nsec = 0;
    status = timer_settime(rx_timer_state_machine_.timer_id, 0, &ts, 0);
    if(status == -1) {
      ALOGE("%s:Failed to stop Rx thread timer",__func__);
      return;
    }
    ALOGV("%s: Rx thread timer Stopped",__func__);
    SetRxThreadTimerState(TIMER_CREATED);
  }
}

CrashTimerState UartController::GetRxThreadTimerState()
{
  std::unique_lock<std::mutex> guard(rx_thread_state_mutex_);
  return rx_timer_state_machine_.timer_state;
}

void UartController::SetRxThreadTimerState(CrashTimerState state)
{
  std::unique_lock<std::mutex> guard(rx_thread_state_mutex_);
  rx_timer_state_machine_.timer_state = state;
}

double UartController:: GetRxThreadSchedTSDiff()
{
  return (std::chrono::duration_cast<std::chrono::milliseconds>(RxTimerSchedTs -
          RxTimerStartTs).count());
}

} // namespace implementation
} // namespace V1_0
} // namespace bluetooth
} // namespace hardware
} // namespace android
