/*==========================================================================
Description
  It has implementation for ring buffer.

# Copyright (c) 2017 Qualcomm Technologies, Inc.
# All Rights Reserved.
# Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

#pragma once

#include <fcntl.h>
#include <errno.h>
#include <mutex>
#include "hci_internals.h"
#include "ring_buffer.h"

#define HEALTH_INFO_FILE_NAME    "/data/vendor/bluetooth/%.02d-stat_info.txt"

namespace android {
namespace hardware {
namespace bluetooth {
namespace V1_0 {
namespace implementation {

class HealthInfoLog : public PacketBuff
{
private:
  std::mutex health_info_mutex_;
  static int max_interval_rx_sleep_wake;
  static int max_interval_tx_packet;
  static int count_wake_sent;
  static int count_wake_received;
  static int count_sleep_sent;
  static int count_sleep_received;
  static uint8_t tx_vote_;
  static uint8_t rx_vote_;
  timer_t health_statistics_timer;
  int health_statistics_timer_state;
  static HealthInfoLog* instance;

private:
  int GetStatInfoDumpIndex();

public:
  HealthInfoLog();
  static HealthInfoLog* Get();
  ~HealthInfoLog();
  void CommitStatInfoBuff();
  static void ReportHealthInfo();
  void IncreaseWakeSent() { count_wake_sent++; };
  void IncreaseWakeRcv() { count_wake_received++; };
  void IncreaseSleepSent() { count_sleep_sent++; };
  void IncreaseSleepRcv() { count_sleep_received++; };
  void UpdateIntervalRxSleepWake(int interval);
  void UpdateIntervalTxPackets(int interval);
  void UpdateTxVote(uint8_t vote);
  void UpdateRxVote(uint8_t vote);
  int  HealthStatisticsTimerStart();
  bool HealthStatisticsTimerStop();
  void HealthStatisticsTimerCleanup();
};

}  // namespace implementation
}  // namespace V1_0
}  // namespace bluetooth
}  // namespace hardware
}  // namespace android
