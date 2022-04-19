/******************************************************************************
 *
 *  Copyright (C) 2017 Google, Inc.
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

#define LOG_TAG "bt_hci"

#include "hci_layer.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <base/location.h>
#include <base/logging.h>
#include "buffer_allocator.h"
#include "osi/include/log.h"
#include <cutils/properties.h>

#include <android/hardware/bluetooth/1.0/IBluetoothHci.h>
#include <android/hardware/bluetooth/1.0/IBluetoothHciCallbacks.h>
#include <android/hardware/bluetooth/1.0/types.h>
#include <hwbinder/ProcessState.h>
#include <hwbinder/IPCThreadState.h>

#define LOG_PATH "/data/misc/bluetooth/logs/firmware_events.log"
#define LAST_LOG_PATH "/data/misc/bluetooth/logs/firmware_events.log.last"

using android::hardware::IPCThreadState;
using android::hardware::bluetooth::V1_0::IBluetoothHci;
using android::hardware::bluetooth::V1_0::IBluetoothHciCallbacks;
using android::hardware::bluetooth::V1_0::HciPacket;
using android::hardware::bluetooth::V1_0::Status;
using android::hardware::ProcessState;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hardware::hidl_vec;

extern void initialization_complete();
extern void hci_event_received(const base::Location& from_here,
                               BT_HDR* packet);
extern void acl_event_received(BT_HDR* packet);
extern void sco_data_received(BT_HDR* packet);

static std::mutex bthci_mutex;

android::sp<IBluetoothHci> btHci;


class BluetoothHciCallbacks : public IBluetoothHciCallbacks {
 public:
  BluetoothHciCallbacks() {
    buffer_allocator = buffer_allocator_get_interface();
  }

  BT_HDR* WrapPacketAndCopy(uint16_t event, const hidl_vec<uint8_t>& data) {
    size_t packet_size = data.size() + BT_HDR_SIZE;
    BT_HDR* packet =
        reinterpret_cast<BT_HDR*>(buffer_allocator->alloc(packet_size));
    packet->offset = 0;
    packet->len = data.size();
    packet->layer_specific = 0;
    packet->event = event;
    // TODO(eisenbach): Avoid copy here; if BT_HDR->data can be ensured to
    // be the only way the data is accessed, a pointer could be passed here...
    memcpy(packet->data, data.data(), data.size());
    return packet;
  }

  Return<void> initializationComplete(Status status) {

    if(status == Status::SUCCESS) {
      initialization_complete();
    } else {
      LOG_ERROR(LOG_TAG, "%s: HCI Init failed ", __func__);
    }
    return Void();
  }

  Return<void> hciEventReceived(const hidl_vec<uint8_t>& event) {
    BT_HDR* packet = WrapPacketAndCopy(MSG_HC_TO_STACK_HCI_EVT, event);
    hci_event_received(FROM_HERE, packet);
    return Void();
  }

  Return<void> aclDataReceived(const hidl_vec<uint8_t>& data) {
    BT_HDR* packet = WrapPacketAndCopy(MSG_HC_TO_STACK_HCI_ACL, data);
    acl_event_received(packet);
    return Void();
  }

  Return<void> scoDataReceived(const hidl_vec<uint8_t>& data) {
    BT_HDR* packet = WrapPacketAndCopy(MSG_HC_TO_STACK_HCI_SCO, data);
    sco_data_received(packet);
    return Void();
  }

  const allocator_t* buffer_allocator;
};

#ifdef ARCH_ARM_32
bool IsLazyHalSupported()
{
  char device[PROPERTY_VALUE_MAX] = {'\0'};
  bool isLazyHalEnabled = false;

  /* MSM8937 target supports both lazy and non lazy hal
   * which is differentiated by a property.
   */
  LOG_DEBUG(LOG_TAG, "%s: reading lazy hal properties", __func__);
  property_get("ro.board.platform", device, "");
  if (!strcmp(device, "bengal"))
    isLazyHalEnabled = true;
  else if (!strcmp(device, "msm8937"))
    isLazyHalEnabled = property_get_bool("ro.vendor.bt.enablelazyhal", false);

  LOG_DEBUG(LOG_TAG, "%s: isLazyHalEnabled: %d", __func__, isLazyHalEnabled);
  return isLazyHalEnabled;
}
#endif

void hci_initialize() {
  LOG_INFO(LOG_TAG, "%s", __func__);

  btHci = IBluetoothHci::getService();
  // If android.hardware.bluetooth* is not found, Bluetooth can not continue.
  CHECK(btHci != nullptr);
  LOG_INFO(LOG_TAG, "%s: IBluetoothHci::getService() returned %p (%s)",
           __func__, btHci.get(), (btHci->isRemote() ? "remote" : "local"));

  // Block allows allocation of a variable that might be bypassed by goto.
  {
    android::sp<IBluetoothHciCallbacks> callbacks = new BluetoothHciCallbacks();
    auto hidl_daemon_status = btHci->initialize(callbacks);
    if(!hidl_daemon_status.isOk()) {
      LOG_ERROR(LOG_TAG, "%s: HIDL daemon is dead", __func__);
      btHci = nullptr;
    }
  }
}

void hci_close() {
  LOG_INFO(LOG_TAG, "%s", __func__);

  std::lock_guard<std::mutex> lock(bthci_mutex);
  if (btHci != nullptr) {
    auto hidl_daemon_status = btHci->close();
    if(!hidl_daemon_status.isOk()) {
      LOG_ERROR(LOG_TAG, "%s: HIDL daemon is dead", __func__);
    } else {
#ifdef ARCH_ARM_32
       if (IsLazyHalSupported()) {
         LOG_DEBUG(LOG_TAG, "%s: decrementing HIDL usage counter", __func__);
         IPCThreadState::self()->flushCommands();
      }
#endif
    }

    btHci = nullptr;
  }
}

hci_transmit_status_t hci_transmit(BT_HDR* packet) {
  HciPacket data;
  hci_transmit_status_t status = HCI_TRANSMIT_SUCCESS;
  std::lock_guard<std::mutex> lock(bthci_mutex);

  if(btHci == nullptr) {
    LOG_INFO(LOG_TAG, "%s: Link with Bluetooth HIDL service is closed", __func__);
    return HCI_TRANSMIT_DAEMON_CLOSED;
  }

  data.setToExternal(packet->data + packet->offset, packet->len);

  uint16_t event = packet->event & MSG_EVT_MASK;
  switch (event & MSG_EVT_MASK) {
    case MSG_STACK_TO_HC_HCI_CMD:
    {
      auto hidl_daemon_status = btHci->sendHciCommand(data);
      if(!hidl_daemon_status.isOk()) {
        LOG_ERROR(LOG_TAG, "%s: send Command failed, HIDL daemon is dead", __func__);
        status = HCI_TRANSMIT_DAEMON_DIED;
      }
      break;
    }
    case MSG_STACK_TO_HC_HCI_ACL:
    {
      auto hidl_daemon_status = btHci->sendAclData(data);
      if(!hidl_daemon_status.isOk()) {
        LOG_ERROR(LOG_TAG, "%s: send acl packet failed, HIDL daemon is dead", __func__);
        status = HCI_TRANSMIT_DAEMON_DIED;
      }
      break;
    }
    case MSG_STACK_TO_HC_HCI_SCO:
    {
      auto hidl_daemon_status = btHci->sendScoData(data);
      if(!hidl_daemon_status.isOk()) {
        LOG_ERROR(LOG_TAG, "%s: send sco data failed, HIDL daemon is dead", __func__);
        status = HCI_TRANSMIT_DAEMON_DIED;
      }
      break;
    }
    default:
      status = HCI_TRANSMIT_INVALID_PKT;
      LOG_ERROR(LOG_TAG, "Unknown packet type (%d)", event);
      break;
  }
  return status;
}

int hci_open_firmware_log_file() {
  if (rename(LOG_PATH, LAST_LOG_PATH) == -1 && errno != ENOENT) {
    LOG_ERROR(LOG_TAG, "%s unable to rename '%s' to '%s': %s", __func__,
              LOG_PATH, LAST_LOG_PATH, strerror(errno));
  }

  mode_t prevmask = umask(0);
  int logfile_fd = open(LOG_PATH, O_WRONLY | O_CREAT | O_TRUNC,
                        S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
  umask(prevmask);
  if (logfile_fd == INVALID_FD) {
    LOG_ERROR(LOG_TAG, "%s unable to open '%s': %s", __func__, LOG_PATH,
              strerror(errno));
  }

  return logfile_fd;
}

void hci_close_firmware_log_file(int fd) {
  if (fd != INVALID_FD) close(fd);
}

void hci_log_firmware_debug_packet(int fd, BT_HDR* packet) {
  TEMP_FAILURE_RETRY(write(fd, packet->data, packet->len));
}
