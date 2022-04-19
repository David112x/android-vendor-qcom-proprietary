/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <stdlib.h>
#include <mutex>
#include <queue>
#include <vector>

#include "BluetoothSar.h"

#define LOG_TAG "vendor.qti.hardware.bluetooth_sar@1.0-BluetoothSar"

namespace vendor {
namespace qti {
namespace hardware {
namespace bluetooth_sar {
namespace V1_0 {
namespace implementation {

#define BTSAR_TIMER_VALUE_MS 2000

#define HCI_GRP_VENDOR_SPECIFIC (0x3F << 10) /* 0xFC00 */
#define HCI_VS_LINK_POWER_CTRL_REQ_OPCODE (0x00DA | HCI_GRP_VENDOR_SPECIFIC)
#define HCI_CONTROLLER_DEBUG_INFO_OCF (0x015B | HCI_GRP_VENDOR_SPECIFIC)

static std::mutex btsar_timer_mutex;
static std::mutex command_queue_mutex;
static std::queue<std::vector<uint8_t>> cmd_queue;

BluetoothSar* instance = NULL;

BluetoothSar::BluetoothSar():
  is_timer_created(false) {
  instance = this;
}

BluetoothSar::~BluetoothSar() {
  std::lock_guard<std::mutex> guard(btsar_timer_mutex);
  BTSarTimerCleanup();
  instance = NULL;
}

std::vector<uint8_t> buildBTSarVSCommand(int8_t cap) {
  ALOGD("%s:",__func__);
  if(cap < 0) {
    ALOGE("%s: cap= %d, Tx output power is negative",__func__, cap);
    cap = 0;
  }
  else if(cap > 80) {
    ALOGE("%s: cap= %d, Tx output power is out of range",__func__, cap);
    cap = 80;
  }
  uint16_t opcode = (HCI_GRP_VENDOR_SPECIFIC | HCI_VS_LINK_POWER_CTRL_REQ_OPCODE);
  std::vector<uint8_t> data;
  data.push_back((uint8_t)opcode);
  data.push_back((uint8_t)(opcode >> 8));
  data.push_back(2);//length of remaning  data
  data.push_back(6);//sub opcode
  data.push_back(cap);

  return data;
}

// Methods from ::vendor::qti::hardware::bluetooth_sar::V1_0::IBluetoothSar follow.
Return<void> BluetoothSar::setBluetoothTxPowerCap(int8_t cap) {
  ALOGI("BluetoothSar:setBluetoothTxPowerCap");
  std::vector<uint8_t> cmd_data = buildBTSarVSCommand(cap);
  sendDataToController(cmd_data);
  return Void();
}

void BluetoothSar::sendDataToController(std::vector<uint8_t> data) {
  size_t send_data_status;
  DataHandler *data_handler = DataHandler::Get();
  ALOGD("%s:",__func__);
  if (data_handler != nullptr) {
    std::lock_guard<std::mutex> cmd_queue_lock(command_queue_mutex);

    /* Command queue would have a maximum of 2 commands at any time. If a BTSAR client
     * sends another command when queue size is 2, then the queue will be popped and
     * the recent command would be pushed into the queue.
     */
    if(!cmd_queue.empty() && (cmd_queue.size() > 1)) {
      cmd_queue.pop();
    }
    /* Command received from the BT SAR client is always queued for flow control.
    *  This will make sure that the next command recieved from BT SAR client would not
    *  be sent to lower layers when the queue size is > 1(ie when the previous command
    *  has not yet received the HCI event). The command will be sent to the lower layers
    *  only when the queue size is 1.
    */
    cmd_queue.push(data);
    if (!cmd_queue.empty() && (cmd_queue.size() == 1)) {
      ALOGD("%s: sending VS cmd, starting timer",__func__);
      {
        std::lock_guard<std::mutex> guard(btsar_timer_mutex);
        BTSarTimerStart();
      }
      send_data_status = data_handler->SendBTSarData(&data[0], data.size(),
        [this](HciPacketType type, const hidl_vec<uint8_t> *packet) {
          switch (type) {
            case HCI_PACKET_TYPE_EVENT:
            {
              ALOGD("BluetoothSAR:HCI event received");
              {
                std::lock_guard<std::mutex> guard(btsar_timer_mutex);
                BTSarTimerStop();
              }
              bool is_cmd_pending = false;
              std::vector<uint8_t> cmd;
              {
                std::lock_guard<std::mutex> cmd_queue_lock(command_queue_mutex);
                ALOGD("BT SAR::cmd_queue size:%d", cmd_queue.size());
                /*Pop the cmd queue to remove the command which has recieved its
                * corresponding event*/
                if (!cmd_queue.empty())
                  cmd_queue.pop();
                //If the queue is not empty,send the next cmd to the lower layers.
                if (!cmd_queue.empty()) {
                  cmd = cmd_queue.front();
                  cmd_queue.pop();
                  is_cmd_pending = true;
                }
              }
              if (is_cmd_pending)
                sendDataToController(cmd);
                break;
              }
              default:
              {
                ALOGE("%s Unexpected event type %d", __func__, type);
                break;
              }
            }
        });

      if (!send_data_status) {
        ALOGE("BluetoothSAR is down");
        cmd_queue.pop();
        std::lock_guard<std::mutex> guard(btsar_timer_mutex);
        BTSarTimerStop();
      }
      else {
        ALOGD("%s: Command is sent",__func__);
      }
    }
  }
  else {
    ALOGD("%s: Data handler is null",__func__);
    cmd_queue = std::queue<std::vector<uint8_t>>();
  }
}

void BluetoothSar::sendDebugInfo() {
  ALOGD("%s:",__func__);
  uint16_t opcode = (HCI_GRP_VENDOR_SPECIFIC | HCI_CONTROLLER_DEBUG_INFO_OCF);
  std::vector<uint8_t> data;
  data.push_back((uint8_t)opcode);
  data.push_back((uint8_t)(opcode >> 8));
  data.push_back(0);

  DataHandler *data_handler = DataHandler::Get();
  if (data_handler != nullptr) {
    data_handler->SendData(TYPE_BT, HCI_PACKET_TYPE_COMMAND, &data[0], data.size());
  }
}

void BluetoothSar::BTSarTimerExpired() {
  {
    std::lock_guard<std::mutex> guard(btsar_timer_mutex);
    ALOGD("%s:",__func__);
    if (instance) {
      instance->sendDebugInfo();
    }
  }
  std::lock_guard<std::mutex> cmd_queue_lock(command_queue_mutex);
  cmd_queue = std::queue<std::vector<uint8_t>>();
}

void BluetoothSar::BTSarTimerStart() {
    int status;
    struct itimerspec ts;
    struct sigevent se;
    uint32_t timeout_ms;
    ALOGD("%s: is_timer_created=%d",__func__, is_timer_created);
    if (is_timer_created == false) {
      se.sigev_notify_function = (void (*)(union sigval))BTSarTimerExpired;
      se.sigev_notify = SIGEV_THREAD;
      se.sigev_value.sival_ptr = &btsar_timer;
      se.sigev_notify_attributes = NULL;
      instance = this;

      status = timer_create(CLOCK_MONOTONIC, &se, &btsar_timer);
      if (status == 0)
      {
        ALOGD("%s: BT SAR timer created",__func__);
        is_timer_created = true;
      } else {
        ALOGE("%s: Error creating BT SAR timer %d\n", __func__, status);
      }
    }
    if (is_timer_created == true) {
      timeout_ms = BTSAR_TIMER_VALUE_MS;
      ts.it_value.tv_sec = timeout_ms/1000;
      ts.it_value.tv_nsec = 1000000*(timeout_ms%1000);
      ts.it_interval.tv_sec = 0;
      ts.it_interval.tv_nsec = 0;

      status = timer_settime(btsar_timer, 0, &ts, 0);
      if (status < 0)
        ALOGE("%s:Failed to set BT SAR timer: %d",__func__, status);
    }
}

void BluetoothSar::BTSarTimerStop() {
  int status;
  struct itimerspec ts;
  ALOGD("%s: is_timer_created= %d",__func__, is_timer_created);

  if (is_timer_created == true) {
    ts.it_value.tv_sec = 0;
    ts.it_value.tv_nsec = 0;
    ts.it_interval.tv_sec = 0;
    ts.it_interval.tv_nsec = 0;
    status = timer_settime(btsar_timer, 0, &ts, 0);
    if (status == -1)
      ALOGE("%s:Failed to stop BT SAR timer",__func__);
  }
}

void BluetoothSar::BTSarTimerCleanup() {
  if (is_timer_created == true) {
    BTSarTimerStop();
    timer_delete(btsar_timer);
    is_timer_created = false;
    btsar_timer = NULL;
  }
}

// Methods from ::android::hidl::base::V1_0::IBase follow.

IBluetoothSar* HIDL_FETCH_IBluetoothSar(const char* /* name */) {
  ALOGE("HIDL_FETCH_IBluetoothSar");
  return new BluetoothSar();
}
//
}  // namespace implementation
}  // namespace V1_0
}  // namespace bluetooth_sar
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
