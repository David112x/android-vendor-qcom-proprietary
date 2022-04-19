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

#pragma once

#include <hidl/HidlSupport.h>
#include "controller.h"
#include "patch_dl_manager.h"
#include "ibs_handler.h"
#include "health_info_log.h"
#include "hci_internals.h"
#include "state_info.h"

#define RX_THREAD_USAGE_TIMEOUT    (1500)
#define RX_THREAD_SCHEDULING_DELAY (50)

namespace android {
namespace hardware {
namespace bluetooth {
namespace V1_0 {
namespace implementation {

enum CrashTimerState {
  TIMER_NOT_CREATED = 0x00,
  TIMER_CREATED = 0x01,
  TIMER_ACTIVE = 0x02,
  TIMER_OVERFLOW = 0x03
};

typedef struct {
  uint8_t timer_state;
  timer_t timer_id;
  uint32_t timeout_ms;
} ClientInitWaitTimerThread;

typedef struct {
  CrashTimerState timer_state;
  timer_t timer_id;
  uint32_t timeout_ms;
}RxThreadTimer;

// singleton class
class UartController : public Controller {
 public:
  // used to get the instance_ controller class
  // open expects the packet reader callback and when the data is available it will call this callback with
  // protocol and pacekt type, and the actual packet.
  virtual bool Init(PacketReadCallback packet_read_cb) override;
  // send packet is used to send the data. protocol type, actual packet types are expected.
  virtual size_t SendPacket(HciPacketType packet_type, const uint8_t *data, size_t length) override;
  virtual bool Cleanup(void) override;
  virtual features_list* GetAddOnFeatureList() override;

  void OnDataReady(int fd);

  void OnPacketReady(hidl_vec<uint8_t> *data);
  void bt_kernel_panic(void);
  UartController(BluetoothSocType soc_type);
  static void SocCrashWaitTimeout(union sigval sig);
  virtual void WaitforCrashdumpFinish();
  virtual void SignalCrashDumpFinish();
  virtual bool Disconnect();
  virtual void UnlockControllerMutex();
  void SsrCleanup(PrimaryReasonCode reason);
  void StartRxThreadTimer();
  void StopRxThreadTimer();
  void ResetInvalidByteCounter();
  void StopClientInitWaitTimer();
  void StartClientInitWaitTimer();
  void StartSocCrashWaitTimer();
  ~UartController();
  double GetRxThreadSchedTSDiff();

 private:
  void StartSocCrashWaitTimer(int ssrtimeout);
  void ReportSocFailure(bool dumped_uart_log, PrimaryReasonCode reason,
                        bool cleanupSocCrashWaitTimer, bool cleanupIbs);
  bool StopSocCrashWaitTimer();
  void CleanupSocCrashWaitTimer();
  bool command_is_get_dbg_info(const unsigned char* buf, int len);
  bool IsHciPacketValid(HciPacketType type);
  void SendSpecialBuffer();
  void HciTransportCleanup();
  bool IsSecondaryReasonValid();
  static void ClientInitTimeOut(union sigval);
  PrimaryReasonCode GetPrimaryReason();
  void SendCrashPacket();
  static void RxThreadTimeOut(union sigval);
  void SetRxThreadTimerState(CrashTimerState);
  CrashTimerState GetRxThreadTimerState();

 public:
  bool is_spurious_wake;

 private:
  int soc_crash_wait_timer_state_;
  timer_t soc_crash_wait_timer_;
  bool force_special_byte_enabled_;
  bool sibs_enabled_;
  bool soc_crashed;
  BluetoothSocType soc_type_;
  HciPacketType hci_packet_type_{HCI_PACKET_TYPE_UNKNOWN};
  HciPacketizer hci_packetizer_;
  std::condition_variable cv;
  std::mutex cv_m;
  bool is_cmd_timeout;
  bool is_invalid_pkt_from_soc;
  bool is_soc_wakeup_failure;
  HealthInfoLog* health_info;
  int invalid_bytes_counter_;
  std::mutex controller_mutex;
  features_list add_on_features;
  ClientInitWaitTimerThread client_init_thread_;
  BtState *state_info_;
  struct timeval time_wk_lockacq_rel_;
  RxThreadTimer rx_timer_state_machine_;
};

} // namespace implementation
} // namespace V1_0
} // namespace bluetooth
} // namespace hardware
} // namespace android
