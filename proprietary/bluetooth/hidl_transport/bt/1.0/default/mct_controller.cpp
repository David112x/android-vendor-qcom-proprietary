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
#include <unistd.h>
#include <hidl/HidlSupport.h>
#include "mct_controller.h"
#include "hci_mct_transport.h"
#include <cutils/properties.h>
#include <utils/Log.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "vendor.qti.bluetooth@1.0-mct_controller"
#define HCI_MAX_EVENT_SIZE                          260
#define HCI_DBG_PIN_CNN_TEST_CMD_MIN_LEN            0x06
#define HCI_DBG_PIN_CNN_TEST_CMD_OPCODE             0xFC0C
#define HCI_DBG_PIN_CNN_TEST_CMD_SUBCODE            0x0B
#define HCI_DBG_PIN_CNN_TEST_BT_MOD_ID              0x02
#define HCI_DBG_PIN_CNN_TEST_BT_MOD_RES_BITMASK     0x34
#define HCI_DBG_PIN_CNN_TEST_BT_MOD_FM_RES_BITMASK  0x40

#define HCI_DBG_PIN_CNN_TEST_CMD_LEN_OFFSET         0x01
#define HCI_DBG_PIN_CNN_TEST_CMD_OPCODE_OFFSET      0x03
#define HCI_DBG_PIN_CNN_TEST_CMD_STATUS_OFFSET      0x05
#define HCI_DBG_PIN_CNN_TEST_CMD_SUBOPCODE_OFFSET   0x06
#define HCI_DBG_PIN_CNN_TEST_CMD_MOD_CNT_OFFSET     0x07

#define PTR_TO_UINT16(u16, p)  ((u16) = ((uint16_t)(*(p)) + (((uint16_t)(*((p) + 1))) << 8)))

namespace android {
namespace hardware {
namespace bluetooth {
namespace V1_0 {
namespace implementation {

const int HCI_SSR_RETRY_COUNT = 6;

using PacketReadCallback = std::function<void(ProtocolType, HciPacketType, const hidl_vec<uint8_t>*)>;

MctController::MctController(BluetoothSocType soc_type)
  : soc_type_(soc_type),
  hci_event_packetizer_([this](hidl_vec<uint8_t> *data) { OnEventPacketReady(data); }),
  hci_acl_packetizer_([this](hidl_vec<uint8_t> *data) { OnAclPacketReady(data); })
{
  init_done_ = false;
  hci_transport_ = nullptr;
}

int MctController::CheckDigPinConnectivity()
{
  int size = 4, err = 0;
  unsigned char cmd[4] = { 0x0C, 0xFC, 0x01, 0x0B };
  unsigned char rsp[HCI_MAX_EVENT_SIZE];
  ALOGI("%s", __func__);

  err = SendPacket(HCI_PACKET_TYPE_COMMAND, cmd, 4);
  if (err != size) {
    ALOGE("%s: Send failed with ret value: %d", __func__, err);
    err = -1;
    goto error;
  }

  /* Wait for command complete event */
  err = ReadHciEvent(rsp, HCI_MAX_EVENT_SIZE);
  if ( err < 0) {
    ALOGE("%s: Failed to receive event", __func__);
  } else if (rsp[0] != EVT_CMD_COMPLETE) {
    ALOGE("%s: unknwon event 0x%02x received", __func__, rsp[0]);
    err = -1;
  } else {
  /* Check param value (s) */
    uint16_t rspLen = rsp[HCI_DBG_PIN_CNN_TEST_CMD_LEN_OFFSET];
    uint16_t opCode;
    uint8_t subOpCode;
    uint16_t btModRes;
    uint8_t numOfModules;

    if (rspLen >= HCI_DBG_PIN_CNN_TEST_CMD_MIN_LEN &&
        rsp[HCI_DBG_PIN_CNN_TEST_CMD_STATUS_OFFSET] == 0x00) {
        PTR_TO_UINT16(opCode, &rsp[HCI_DBG_PIN_CNN_TEST_CMD_OPCODE_OFFSET]);
        subOpCode = rsp[HCI_DBG_PIN_CNN_TEST_CMD_SUBOPCODE_OFFSET];
        numOfModules = rsp[HCI_DBG_PIN_CNN_TEST_CMD_MOD_CNT_OFFSET];
        ALOGI("%s: rspLen = %d, opCode = 0x%04x, subOpCode = 0x%02x, numOfModules = %d",
            __func__, rspLen, opCode, subOpCode, numOfModules);
        if (opCode == HCI_DBG_PIN_CNN_TEST_CMD_OPCODE &&
            subOpCode == HCI_DBG_PIN_CNN_TEST_CMD_SUBCODE && numOfModules) {
          /* Check if count of modules is within expected length of command */
          if ((numOfModules * 3) + 6 > rspLen) {
              ALOGE("%s: failed due to len mismatch in response", __func__);
              err = -1;
              goto error;
          }
          int count;
          /* Loop in until we find module id as BT (0x02) */
          for (count = 0; count < numOfModules; count ++) {
            if (rsp[HCI_DBG_PIN_CNN_TEST_CMD_MOD_CNT_OFFSET + 1 + (count * 3)] ==
              HCI_DBG_PIN_CNN_TEST_BT_MOD_ID) {
              PTR_TO_UINT16(btModRes, &rsp[HCI_DBG_PIN_CNN_TEST_CMD_MOD_CNT_OFFSET + 2
                + (count * 3)]);
              ALOGI("%s: btModRes = 0x%04x", __func__, btModRes);
              if (btModRes & HCI_DBG_PIN_CNN_TEST_BT_MOD_FM_RES_BITMASK) {
                  ALOGI("%s: FM Pin validation failed, FM not supported!!", __func__);
              }
              if (!(btModRes & HCI_DBG_PIN_CNN_TEST_BT_MOD_RES_BITMASK)) {
                  ALOGI("%s: success", __func__);
                  err = 0;
              } else {
                  ALOGE("%s: failed validation for BT", __func__);
                  err = -1;
              }
              goto error;
            }
          }
          ALOGE("%s: failed due to BT module status not found", __func__);
          err = -1;
        } else {
            ALOGE("%s: failed due to invalid opcode or subcode", __func__);
            err = -1;
        }
    } else {
        ALOGE("%s: failed due to invalid len or status", __func__);
        err = -1;
    }
  }
error:
  return err;
}

bool MctController::Init(PacketReadCallback pkt_read_cb)
{
  HciMctTransport* mct_transport = nullptr;
  NvmTagsManager* nvm_tags_manager = nullptr;

  if (init_done_) {
    ALOGD("already initialized, return");
    return true;
  }

  read_cb_ = pkt_read_cb;

  // initialize the HCI transport
  if (!(mct_transport = new HciMctTransport())) {
    goto error;
  }

  hci_transport_ = static_cast<HciTransport*> (mct_transport);

  mct_transport->Init(soc_type_);

#ifdef WAKE_LOCK_ENABLED
  Wakelock :: Acquire();
#endif

  if (CheckDigPinConnectivity() < 0) {
#ifdef WAKE_LOCK_ENABLED
    Wakelock :: Release();
#endif
    goto error;
  }

  //Download the NVM tags
  nvm_tags_manager = new NvmTagsManager(mct_transport);
  if (nvm_tags_manager->SocInit() < 0) {
#ifdef WAKE_LOCK_ENABLED
    Wakelock :: Release();
#endif
    goto error;
  }

  // set up the fd watcher now
  fd_watcher_.WatchFdForNonBlockingReads(
    mct_transport->GetCtrlFd(),
    [this](int fd) { OnEventDataReady(fd); });

  fd_watcher_.WatchFdForNonBlockingReads(
    mct_transport->GetDataFd(),
    [this](int fd) { OnAclDataReady(fd); });

#ifdef WAKE_LOCK_ENABLED
  Wakelock :: Release();
#endif

  if (nvm_tags_manager) {
    delete nvm_tags_manager;
    nvm_tags_manager = nullptr;
  }

  init_done_ = true;
  ALOGD("Init succeded");
  return init_done_;

 error:
  ALOGE("Init failed");
  if (nvm_tags_manager) {
    delete nvm_tags_manager;
    nvm_tags_manager = nullptr;
  }

  if (hci_transport_) {
    hci_transport_->CleanUp();
    delete hci_transport_;
    hci_transport_ = nullptr;
  }
  return init_done_;
}

size_t MctController::SendPacket(HciPacketType packet_type,
                        const uint8_t *data, size_t length)
{
  if (hci_transport_)
    return hci_transport_->Write(packet_type, data, length);
  return 0;
}

int MctController::ReadHciEvent(unsigned char* buf, int size)
{
  int retval;
  unsigned char hdr[BT_EVT_HDR_SIZE];
  unsigned char packet_len;
  unsigned short tot_len;

  if (size < 0) {
    ALOGE("%s: Invalid size: %d\n", __func__, size);
    return -1;
  }

  retval = hci_transport_->Read(hdr, BT_EVT_HDR_SIZE);
  if (retval < 0) {
    ALOGE("%s:error in reading hdr: %d", __func__, retval);
    return -1;
  }
  packet_len = hdr[BT_EVT_HDR_LEN_OFFSET];

  memcpy(buf, hdr, BT_EVT_HDR_SIZE);
  retval = hci_transport_->Read(buf + BT_EVT_HDR_SIZE, packet_len);
  if (retval < 0) {
    ALOGE("%s:error in reading buf: %d", __func__, retval);
    retval = -1;
    return retval;
  }
  tot_len = packet_len + BT_EVT_HDR_SIZE;
  ALOGV("read scucesssfully payload: tot_len: %d", tot_len);
  return tot_len;
}

bool MctController::Cleanup(void)
{
  if (!init_done_) {
    ALOGD("MctController:: already closed, return");
    return false;
  }

  // stop the fd watcher
  fd_watcher_.StopWatchingFileDescriptors();

  if (hci_transport_) {
    hci_transport_->CleanUp();
    delete hci_transport_;
    hci_transport_ = nullptr;
  }
  init_done_ = false;
  return true;
}

void MctController::OnAclPacketReady(hidl_vec<uint8_t> *data)
{
  if(data == nullptr) {
    ALOGE("Error reading data from smd");
    return;
  }

  if (read_cb_) {
    read_cb_(TYPE_BT, HCI_PACKET_TYPE_ACL_DATA, data);
  }
}

void MctController::OnEventPacketReady(hidl_vec<uint8_t> *data)
{
  if(data == nullptr) {
    ALOGE("Error reading data from smd");
    return;
  }

  if (read_cb_) {
    read_cb_(TYPE_BT, HCI_PACKET_TYPE_EVENT, data);
  }
}

void MctController::OnAclDataReady(int fd)
{
  hci_acl_packetizer_.OnDataReady(fd, HCI_PACKET_TYPE_ACL_DATA);
}

void MctController::OnEventDataReady(int fd)
{
  uint8_t reset = DevInReset(fd);

  if(reset) {
    unsigned char bt_eve_buf[LENGTH_HW_ERROR_EVT] = { 0x10, 0x01, 0x0f };
    hidl_vec<uint8_t> *bt_packet_ = new hidl_vec<uint8_t>;
    bt_packet_->resize(LENGTH_HW_ERROR_EVT);
    ALOGD("%s: Sending hw error event", __func__);
    memcpy(bt_packet_->data(), bt_eve_buf, LENGTH_HW_ERROR_EVT);
    read_cb_(TYPE_BT, HCI_PACKET_TYPE_EVENT, bt_packet_);
   } else {
     hci_event_packetizer_.OnDataReady(fd, HCI_PACKET_TYPE_EVENT);
   }
}

bool MctController::DevInReset(int fd)
{
  volatile int serial_bits;
  bool dev_reset_done = 0;
  uint8_t retry_count = 0;
  ioctl(fd, TIOCMGET, &serial_bits);
  if (serial_bits & TIOCM_OUT2) {
    while(serial_bits & TIOCM_OUT1) {
      ALOGD("%s: device in reset", __func__);
      sleep(2);
      retry_count++;
      ioctl(fd, TIOCMGET, &serial_bits);
      if((serial_bits & TIOCM_OUT1)) {
        dev_reset_done = 0;
      } else {
        dev_reset_done = 1;
        ALOGD("%s: device reset completed", __func__);
      }
      if(retry_count == HCI_SSR_RETRY_COUNT) {
        //treat it as ssr completed to kill the bt
        // process
        ALOGE("%s: Retrials exceeded, treating it reset completed", __func__);
        dev_reset_done = 1;
        break;
      }
    }
  }
  return dev_reset_done;
}

features_list* MctController:: GetAddOnFeatureList(){
  return NULL;
}

} // namespace implementation
} // namespace V1_0
} // namespace bluetooth
} // namespace hardware
} // namespace android
