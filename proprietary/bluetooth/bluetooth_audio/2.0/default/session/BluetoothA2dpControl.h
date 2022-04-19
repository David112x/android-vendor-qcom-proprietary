/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once

#include <mutex>
#include <unordered_map>

#include <vendor/qti/hardware/bluetooth_audio/2.0/IBluetoothAudioPort.h>
#include <fmq/MessageQueue.h>
#include <hardware/audio.h>
#include <hidl/MQDescriptor.h>

namespace vendor {
namespace qti {
namespace bluetooth_audio {

using ::vendor::qti::hardware::bluetooth_audio::V2_0::SessionType;
using ::vendor::qti::hardware::bluetooth_audio::V2_0::SinkLatency;

class BluetoothA2dpControl {

 private:
  static BluetoothA2dpControl *A2dpControl;
  SinkLatency sinkLatency;
  bool updateLocalLatency;
  uint32_t mtu;
  uint16_t bitrate;
  std::mutex a2dpControlMutex;

  BluetoothA2dpControl() {
    updateLocalLatency = false;
    sinkLatency.remoteDeviceAudioDelay = 0;
    sinkLatency.transmittedOctets = 0;
    mtu = 0;
    bitrate = 0;
  }

 public:

  // This function instantiate A2DP Control
  // This is called when Audio Session is ready
  static void startA2DPControl() {
    if(A2dpControl == NULL) {
      A2dpControl = new BluetoothA2dpControl();
    }
  }

  // This function returns A2DP Control instance if Audio Session is ready
  // @return: A2DP control instance if Audio Session is active else returns null
  static BluetoothA2dpControl *getA2DPControl() {
    return A2dpControl;
  }

  // This function reports the sink latency shared as delay reporting
  // Local values will be updated if data is already fetched once
  // This will return null if Audio Session is not present
  bool getSinkLatency(const SessionType& session_type,
                                      uint64_t* remote_delay_report_ns,
                                      uint64_t* total_bytes_readed,
                                      timespec* data_position);

  // This function updates the latency value based on stack feedback
  // Same value will be updated to Audio when requested
  void updateSinkLatency(uint64_t remoteDeviceAudioDelay);

  // Destructor
  static void freeA2dpControl() {
    delete A2dpControl;
    A2dpControl = NULL;
  }

};

}  // namespace bluetooth_audio
}  // namespace qti
}  // namespace vendor
