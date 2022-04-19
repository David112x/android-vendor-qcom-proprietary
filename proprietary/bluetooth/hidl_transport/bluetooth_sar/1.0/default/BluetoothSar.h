/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef VENDOR_QTI_HARDWARE_BLUETOOTH_SAR_V1_0_BLUETOOTHSAR_H
#define VENDOR_QTI_HARDWARE_BLUETOOTH_SAR_V1_0_BLUETOOTHSAR_H

#include <vendor/qti/hardware/bluetooth_sar/1.0/IBluetoothSar.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include "controller.h"
#include "data_handler.h"
#include <utils/Log.h>

namespace vendor {
namespace qti {
namespace hardware {
namespace bluetooth_sar {
namespace V1_0 {
namespace implementation {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

using android::hardware::bluetooth::V1_0::implementation::DataHandler;

struct BluetoothSar : public IBluetoothSar {
  BluetoothSar();
  ~BluetoothSar();
  // Methods from ::vendor::qti::hardware::bluetooth_sar::V1_0::IBluetoothSar follow.
  Return<void> setBluetoothTxPowerCap(int8_t cap) override;

private:
  bool is_timer_created;
  timer_t btsar_timer;
  void sendDataToController(std::vector<uint8_t> data);
  void InitializeDataHandler();
  void sendDebugInfo();
  static void BTSarTimerExpired();
  void BTSarTimerStart();
  void BTSarTimerStop();
  void BTSarTimerCleanup();

  // Methods from ::android::hidl::base::V1_0::IBase follow.

};

// FIXME: most likely delete, this is only for passthrough implementations
extern "C" IBluetoothSar* HIDL_FETCH_IBluetoothSar(const char* name);

}  // namespace implementation
}  // namespace V1_0
}  // namespace bluetooth_sar
}  // namespace hardware
}  // namespace qti
}  // namespace vendor

#endif  // VENDOR_QTI_HARDWARE_BLUETOOTH_SAR_V1_0_BLUETOOTHSAR_H
