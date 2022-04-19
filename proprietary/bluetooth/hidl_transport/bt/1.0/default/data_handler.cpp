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
// Copyright 2016 The Android Open Source Project
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

#include <pthread.h>
#include <chrono>
#include <thread>
#include <unistd.h>
#include <hidl/HidlSupport.h>
#include "data_handler.h"
#include "logger.h"
#include <cutils/properties.h>
#include "bluetooth_address.h"
#include <utils/Log.h>
#include <signal.h>
#include "state_info.h"
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "vendor.qti.bluetooth@1.0-data_handler"

namespace {

using android::hardware::bluetooth::V1_0::implementation::DataHandler;

DataHandler *data_handler = nullptr;

std::mutex init_mutex_;

std::mutex evt_wait_mutex_;
std::condition_variable event_wait_cv;
bool event_wait;
uint16_t awaited_evt;

}

namespace android {
namespace hardware {
namespace bluetooth {
namespace V1_0 {
namespace implementation {

using ::android::hardware::hidl_vec;
using DataReadCallback = std::function<void(HciPacketType, const hidl_vec<uint8_t>*)>;
using InitializeCallback = std::function<void(bool success)>;

bool soc_need_reload_patch = true;
bool DataHandler :: caught_signal = false;
unsigned int ClientStatus;
unsigned int RxthreadStatus;
std::chrono::time_point<std::chrono::steady_clock> InitTime, CloseTime;
double time_diff;
#define MIN_OPCODE_LEN     (5)
#define HIDL_INIT_TIMEOUT  (2900)

void DataHandler::data_service_sighandler(int signum)
{
  // lock is required incase of multiple binder threads
  std::unique_lock<std::mutex> guard(init_mutex_);
  ALOGW("%s: Caught Signal: %d", __func__, signum);

  if (data_handler) {
    data_handler->SetSignalCaught();
    if (data_handler->Close(TYPE_BT))
        goto cleanup;
    if (data_handler->Close(TYPE_FM))
        goto cleanup;
    if (data_handler->Close(TYPE_ANT))
        goto cleanup;
    return;
cleanup:
    ALOGI("%s: deleting data_handler", __func__);
    delete data_handler;
    data_handler = NULL;
    kill(getpid(), SIGKILL);
  }
}

int DataHandler::data_service_setup_sighandler(void)
{
  struct sigaction sig_act;

  ALOGI("%s: Entry", __func__);
  memset(&sig_act, 0, sizeof(sig_act));
  sig_act.sa_handler = data_handler->data_service_sighandler;
  sigemptyset(&sig_act.sa_mask);

  sigaction(SIGTERM, &sig_act, NULL);

  return 0;
}

bool DataHandler::isBTSarEvent(HciPacketType type, const uint8_t* data) {
  uint16_t opcode = ((data[4] << 8) | data[3]);
  if (type == HCI_PACKET_TYPE_EVENT && (opcode == cmd_opcode)) {
    cmd_opcode = 0x00;
    ALOGD("%s: true",__func__);
    return true;
  }
  return false;
}

bool DataHandler::SendBTSarData(const uint8_t *data, size_t length, DataReadCallback event_cb)
{
  ALOGD("DataHandler::SendBTSarData()");
  if (!isProtocolInitialized(TYPE_BT)) {
    ALOGE("BT HAL not registered, hence not sending BTSAR command");
    return false;
  }
  if (event_cb)
    btsar_event_cb = event_cb;
  if (data != nullptr) {
    cmd_opcode = ((data[1] << 8) | data[0]);
    if (SendData(TYPE_BT, HCI_PACKET_TYPE_COMMAND, data, length) > 0) {
      return true;
    } else {
      // sending command failed, reset cmd_opcode again
      cmd_opcode = 0x00;
    }
  }
  return false;
}

bool DataHandler::Init(ProtocolType type, InitializeCallback init_cb,
                       DataReadCallback data_read_cb)
{
  // lock required incase of multiple binder threads
  ALOGW("DataHandler:: Init()");
  std::unique_lock<std::mutex> guard(init_mutex_);
  if (!data_handler) {
    data_handler = new DataHandler();
    data_handler->data_service_setup_sighandler();
  }
  return data_handler->Open(type, init_cb, data_read_cb);
}

void DataHandler::CleanUp(ProtocolType type)
{
  // lock is required incase of multiple binder threads
  std::unique_lock<std::mutex> guard(init_mutex_);
  ALOGW("DataHandler::CleanUp()");
  if (data_handler && data_handler->Close(type)) {
    delete data_handler;
    data_handler = NULL;
  }
}

bool DataHandler::isProtocolInitialized(ProtocolType pType)
{
  std::map<ProtocolType, ProtocolCallbacksType *>::iterator it;
  bool status = false;
  ALOGD("%s:",__func__);

  it = protocol_info_.find(pType);
  if (it != protocol_info_.end()) {
    ProtocolCallbacksType *cb_data = (ProtocolCallbacksType*)it->second;
    if(!cb_data->is_pending_init_cb && init_status_ == INIT_STATUS_SUCCESS) {
      status = true;
    }
    else {
      status = false;
    }
  }
  else {
    status = false;
  }
  ALOGI("%s: status:%d",__func__, status);
  return status;
}

bool DataHandler::isProtocolAdded(ProtocolType pType)
{
  std::map<ProtocolType, ProtocolCallbacksType *>::iterator it;
  bool status = false;
  ALOGD("%s:",__func__);

  it = protocol_info_.find(pType);
  if (it != protocol_info_.end()) {
    status = true;
  }
  else {
    status = false;
  }

  ALOGI("%s: status:%d",__func__, status);
  return status;
}

DataHandler * DataHandler::Get()
{
  return data_handler;
}

DataHandler::DataHandler() {
  soc_type_ = GetSocTypeInt();
  logger_ = Logger::Get();
  cmd_opcode = 0x00;
}

bool DataHandler::IsSocAlwaysOnEnabled()
{
  /* If HAL is configured as LAZY, the library will be unloaded
   * from the primary memory when the client's usage count goes to
   * zero. For the next BT ON fw will be loaded back. Having SoC
   * always on will have unnecessary power lekage.
   */
#ifndef LAZY_SERVICE
  char value[PROPERTY_VALUE_MAX] = {'\0'};
  property_get("persist.vendor.service.bdroid.soc.alwayson", value, "false");
  return (strcmp(value, "true") == 0) &&
            ((soc_type_ == BT_SOC_CHEROKEE) || (soc_type_ == BT_SOC_HASTINGS));
#else
  ALOGD("%s SoC always ON not supported on this platform", __func__);
  return false;
#endif
}

BluetoothSocType DataHandler::GetSocType()
{
  return soc_type_;
}

BluetoothSocType DataHandler::GetSocTypeInt()
{
  int ret = 0;
  char soc[PROPERTY_VALUE_MAX];

  ret = property_get("vendor.qcom.bluetooth.soc", soc, NULL);

  if (ret == 0) {
    ALOGW("vendor.qcom.bluetooth.soc prop  not set, so using pronto.\n ");
    soc_type_ = BT_SOC_DEFAULT;
  } else {
    ALOGW("vendor.qcom.bluetooth.soc set to %s\n", soc);
    if (!strncasecmp(soc, "rome", sizeof("rome"))) {
      soc_type_ = BT_SOC_ROME;
    } else if (!strncasecmp(soc, "cherokee", sizeof("cherokee"))) {
      soc_type_ = BT_SOC_CHEROKEE;
    } else if (!strncasecmp(soc, "ath3k", sizeof("ath3k"))) {
      soc_type_ = BT_SOC_AR3K;
    } else if (!strncasecmp(soc, "napier", sizeof("napier"))) {
      soc_type_ = BT_SOC_NAPIER;
    } else if (!strncasecmp(soc, "hastings", sizeof("hastings"))) {
      soc_type_ = BT_SOC_HASTINGS;
    } else if (!strncasecmp(soc, "genoa", sizeof("genoa"))) {
      soc_type_ = BT_SOC_GENOA;
    } else {
      ALOGI("vendor.qcom.bluetooth.soc not set, so using pronto.\n");
      soc_type_ = BT_SOC_DEFAULT;
    }
  }
  return soc_type_;
}

// this is used to send the actual packet.
size_t DataHandler::SendData(ProtocolType ptype, HciPacketType packet_type,
                             const uint8_t *data, size_t length)
{
  if (CheckSignalCaughtStatus()) {
    ALOGD("%s: Return as SIGTERM Signal is caught", __func__);
    return 0;
  }

  std::map<ProtocolType, ProtocolCallbacksType *>::iterator it;
  {
    it = protocol_info_.find(ptype);
    if (it == protocol_info_.end()) {
      ALOGE("%s: NO entry found for the protocol %d \n", __func__, ptype);
      return 0;
    }
    if (init_status_ != INIT_STATUS_SUCCESS) {
      ALOGE("%s: BT Daemon not initialized, ignore packet", __func__);
      return 0;
    }
  }

  UpdateRingBuffer(packet_type, data, length);
  if (controller_ != nullptr)
     return controller_->SendPacket(packet_type, data, length);

  return 0;
}

#ifdef DUMP_RINGBUF_LOG
void DataHandler::AddHciCommandTag(char* dest_tag_str, struct timeval& time_val, char * opcode)
{
  uint32_t w_index = 0;

  memset(dest_tag_str, 0, RX_TAG_STR_LEN);
  add_time_str(dest_tag_str, &time_val);

  w_index = strlen(dest_tag_str);
  snprintf(dest_tag_str+w_index, strlen(opcode) + 1, "-%s", opcode);
}
#endif

void DataHandler::OnPacketReady(ProtocolType ptype, HciPacketType type,
                                const hidl_vec<uint8_t>*hidl_data)
{
  std::map<ProtocolType, ProtocolCallbacksType *>::iterator it;

  uint16_t len = hidl_data->size();
  const uint8_t* data = hidl_data->data();

  logger_->ProcessRx(type, data, len);

  if (logger_->IsControllerLogPkt(type, data, len)) {
    ALOGV("%s:Received a controller log packet\n", __func__);
    if(!logger_->IsHciFwSnoopEnabled()) {
      delete hidl_data;
      return;
    }
  }

  if (type == HCI_PACKET_TYPE_EVENT && data_handler->isAwaitedEvent(data, len)) {
    ALOGW("%s: Received event for command sent internally: %02x %02x \n",
            __func__, data[3], data[4]);
    delete hidl_data;
    event_wait = true;
    event_wait_cv.notify_all();
    return;
  }

  if (cmd_opcode != 0x00 && ptype == TYPE_BT && len >= MIN_OPCODE_LEN) {
    if (isBTSarEvent(type, data)) {
      if (btsar_event_cb) {
        btsar_event_cb(type, hidl_data);
      }
      delete hidl_data;
      return;
    }
  }
  ProtocolCallbacksType *cb_data = nullptr;
  if (diag_interface_.GetCleanupStatus(ptype)) {
    if (GetRxthreadStatus(ptype)) {
      SetRxthreadStatus(false, ptype);
      ALOGW("Skip sending packet to client as cleanup in process\n");
    }
    delete hidl_data;
    return;
  }
  std::unique_lock<std::mutex> guard(internal_mutex_);
  // update the pending Init cb and other callbacks
  it = protocol_info_.find(ptype);
  if (it != protocol_info_.end()) {
    cb_data = (ProtocolCallbacksType*)it->second;
  }

  // execute callbacks here
  if (cb_data != nullptr && !cb_data->is_pending_init_cb && controller_ != nullptr) {
    if (!diag_interface_.isSsrTriggered())
      controller_->StartRxThreadTimer();
    cb_data->data_read_cb(type, hidl_data);
    controller_->StopRxThreadTimer();
  } else {
    ALOGD("%s: packet discarded and not handled", __func__);
    for (int pktindex = 0; pktindex < len && pktindex < 5; ++pktindex)
      ALOGD("%s: discarded packet[%d] = \t0x%02x ", __func__, pktindex, data[pktindex]);
  }
  delete hidl_data;
}

// signal handler
void DataHandler::usr1_handler(int /* s */)
{
  bool status = TRUE;

  ALOGI("%s: exit\n", __func__);
  Wakelock :: UnlockWakelockMutex();
  pthread_exit(&status);
}

unsigned int  DataHandler :: GetRxthreadStatus(ProtocolType Type)
{
  return (RxthreadStatus & (0x01 << Type));
}

void DataHandler :: SetRxthreadStatus(bool status, ProtocolType Type)
{
  if (status)
    RxthreadStatus = (RxthreadStatus | (0x01 << Type));
  else
    RxthreadStatus = (RxthreadStatus & (~(0x01 << Type)));

}

unsigned int  DataHandler :: GetClientStatus(ProtocolType Type)
{
  return (ClientStatus & (0x01 << Type));
}

void DataHandler :: SetClientStatus(bool status, ProtocolType Type)
{
  if (status)
    ClientStatus = (ClientStatus | (0x01 << Type));
  else
    ClientStatus = (ClientStatus & (~(0x01 << Type)));

  if (logger_)
    logger_->SetClientStatus(status, Type);
}

bool DataHandler::Open(ProtocolType type, InitializeCallback init_cb,
                       DataReadCallback data_read_cb)
{
  char dst_buff[MAX_BUFF_SIZE];
  char init_buff[MAX_BUFF_SIZE];
  struct timeval tv;
  std::map<ProtocolType, ProtocolCallbacksType *>::iterator it;
  std::unique_lock<std::mutex> guard(internal_mutex_);

  /* Don't register new client when SSR in progress. This avoids
   * Crash as we will kill daemon after collecting the dump.
   */
  if (diag_interface_.isSsrTriggered()) {
    ALOGE("<%s: Returning as SSR or cleanup in progress>", __func__);
    return false;
  }

  if (isProtocolAdded(type)) {
    ALOGE("<%s: Returning as protocol already added>", __func__);
    return false;
  }

  ALOGI("Open init_status %d \n", init_status_);
  SetClientStatus(true, type);
  SetRxthreadStatus(true, type);

  gettimeofday(&tv, NULL);
  snprintf(init_buff, sizeof(init_buff), "HCI initialize rcvd from client type = %d", type);
  BtState::Get()->AddLogTag(dst_buff, tv, init_buff);
  BtState::Get()->SetTsHCIInitClose(HCI_INIT, dst_buff);

  BtState::Get()->SetAlwaysOnEnabled(IsSocAlwaysOnEnabled());
  // update the pending Init cb and other callbacks
  it = protocol_info_.find(type);
  if (it == protocol_info_.end()) {
    ProtocolCallbacksType *cb_data  = new (ProtocolCallbacksType);
    cb_data->type = type;
    cb_data->is_pending_init_cb = true;
    cb_data->init_cb = init_cb;
    cb_data->data_read_cb = data_read_cb;
    protocol_info_[type] = cb_data;
  }
  switch (init_status_) {
    case INIT_STATUS_INITIALIZING:
      return true;
      break;
    case INIT_STATUS_SUCCESS:
      it = protocol_info_.find(type);
      if (it != protocol_info_.end()) {
        ProtocolCallbacksType *cb_data = (ProtocolCallbacksType*)it->second;
        cb_data->is_pending_init_cb = false;
        cb_data->init_cb(true);

        if (cb_data->type == TYPE_BT && logger_->IsPreviousItrCrashed()) {
          /* Clients require some time to boot up there stacks after init event
           * recevied. Wait until the timeout happens to post the event.
           */
          controller_->StartClientInitWaitTimer();
       }
      }
      return true;
      break;
    case INIT_STATUS_FAILED:
      init_thread_.join();
      [[fallthrough]];
    case INIT_STATUS_IDLE:
      init_status_ = INIT_STATUS_INITIALIZING;
      if (!IsSocAlwaysOnEnabled()) {
        soc_need_reload_patch = true;
      }
      ALOGI("%s: soc_need_reload_patch = %d",
                  __func__, soc_need_reload_patch);
#ifdef WAKE_LOCK_ENABLED
      Wakelock :: Init();
#endif
      break;
  }

  logger_->ResetCrashReasons();
  init_thread_ = std::thread([this, type]() {
    bool status = false;
    struct sigaction old_sa, sa;
    char cb_status_buf[MAX_BUFF_SIZE] = {'\0'};
    char dst_buff[MAX_BUFF_SIZE];
    struct timeval tv;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = usr1_handler;
    sigaction(SIGUSR1, &sa, &old_sa);
    InitTime = std::chrono::steady_clock::now();

    if (soc_type_ == BT_SOC_SMD) {
      controller_ = static_cast<Controller *> (new MctController(soc_type_));
    } else {
      controller_ = static_cast<Controller *> (new UartController(soc_type_));
    }
    if (controller_) {
      status = controller_->Init([this](ProtocolType ptype, HciPacketType type,
                                        const hidl_vec<uint8_t> *hidl_data)   {
                                   OnPacketReady(ptype, type, hidl_data);
                                 });
      gettimeofday(&tv, NULL);
      snprintf(dst_buff, sizeof(dst_buff), "Controller Init status = %d", status);
      BtState::Get()->AddLogTag(cb_status_buf, tv, dst_buff);
      BtState::Get()->SetTsCtrlrInitStatus(cb_status_buf);
    }

    std::unique_lock<std::mutex> guard(internal_mutex_);
    if (status) {
      if (!soc_need_reload_patch) {
        guard.unlock();
        sendCommandWait(HCI_RESET_CMD, type);
        if (event_wait) {
          gettimeofday(&tv, NULL);
          BtState::Get()->AddLogTag(dst_buff, tv, (char*)"RSP rcvd for first HCI_RESET_CMD");
          BtState::Get()->SetHciResetCmdRsp(dst_buff);
          ALOGD("%s: RSP rcvd for first HCI_RESET_CMD", __func__);
        }
        sendCommandWait(HCI_WRITE_BD_ADDRESS, type);
        guard.lock();
      }
      init_status_ = INIT_STATUS_SUCCESS;
      ALOGD("Firmware download succeded.");
    } else {
      ALOGE("Controller Init failed ");
      init_status_ = INIT_STATUS_FAILED;
      delete controller_;
      controller_ = nullptr;

      logger_->PrepareDumpProcess();
#ifdef DUMP_IPC_LOG
      if(soc_type_ != BT_SOC_SMD)
        logger_->DumpUartIpcLogs();
#endif
#ifdef DUMP_RINGBUF_LOG
      logger_->DumpRingBuffers();
#endif

      logger_->SetPrimaryCrashReason(REASON_PATCH_DLDNG_FAILED_SOCINIT);
      logger_->StoreCrashReason();
      BtState :: Get()->DumpBtState();

      //Reset logger so that we can get correct log file index next time.
      logger_->Cleanup();
    }

    std::map<ProtocolType, ProtocolCallbacksType *>::iterator it;
    for (auto& it: protocol_info_) {
      ProtocolCallbacksType *cb_data = (ProtocolCallbacksType*)it.second;
      cb_data->is_pending_init_cb = false;
      gettimeofday(&tv, NULL);
      snprintf(dst_buff, sizeof(dst_buff), "Init callback status = %d", status);
      BtState::Get()->AddLogTag(cb_status_buf, tv, dst_buff);
      BtState::Get()->SetTsStatusOfCbSent(cb_status_buf);
      cb_data->init_cb(status);

      /* Check and post the previous iteration close reason. */
      if (status && cb_data->type == TYPE_BT && logger_->IsPreviousItrCrashed()) {
        /* Clients require some time to boot up there stacks after init event
         * recevied. Wait until the timeout happens to post the event.
         */
         controller_->StartClientInitWaitTimer();
      }
    }

    // clear the list if the controller open call fails
    if (!status) {
      /* clearing up all callback data as controller init itself failed */
      for (auto& it: protocol_info_) {
        ProtocolCallbacksType *cb_data = (ProtocolCallbacksType*)it.second;
        delete (cb_data);
      }
      protocol_info_.clear();
      BtState::Get()->Cleanup();
#ifdef USER_DEBUG
      if (!data_handler->CheckSignalCaughtStatus()) {
        ALOGE("%s: Aborting daemon to recover as controller init failed", __func__);
        abort();
      } else {
        /* user triggerred reboot, no need to call abort */
        ALOGE("%s: Killing daemon as user triggered forced reboot", __func__);
        kill(getpid(), SIGKILL);
      }
#else
      ALOGE("%s: Killing daemon to recover as controller init failed", __func__);
      kill(getpid(), SIGKILL);
#endif

    }
  });
  return true;
}

bool DataHandler::Close(ProtocolType type)
{
  char dst_buff[MAX_BUFF_SIZE];
  char type_buff[MAX_BUFF_SIZE];
  struct timeval tv;
  std::map<ProtocolType, ProtocolCallbacksType *>::iterator it;

  CloseTime = std::chrono::steady_clock::now();
  if (!controller_) {
#ifdef WAKE_LOCK_ENABLED
    Wakelock :: CleanUp();
#endif
    /* Do cleanup if controller init failed */
    if (init_status_ == INIT_STATUS_FAILED) {
      if (init_thread_.joinable()) {
        ALOGD("%s: joining init thread", __func__);
        init_thread_.join();
        ALOGI("%s: joined init thread", __func__);
      }
      return true;
    } else {
      if (init_thread_.joinable()) {
        ALOGD("%s: sending SIGUSR1 signal", __func__);
        pthread_kill(init_thread_.native_handle(), SIGUSR1);
        ALOGD("%s: joining init thread", __func__);
        init_thread_.join();
        ALOGI("%s: joined Init thread", __func__);
      }

      /*callback cleanup incase when controller constructor
        call is ongoing/stuck and we call close*/
      ALOGD("%s: deleting callback data", __func__);
      it = protocol_info_.find(type);
      if (it != protocol_info_.end()) {
        ProtocolCallbacksType *cb_data = reinterpret_cast<ProtocolCallbacksType *> (it->second);
        delete (cb_data);
        protocol_info_.erase(it);
      }
#ifdef USER_DEBUG
      if (GetInitCloseTSDiff() < HIDL_INIT_TIMEOUT) {
        /* close called before startup time (2.9 sec) is finished  */
        ALOGE("%s: Killing daemon to recover as close called before startup timer expiry",
              __func__);
        kill(getpid(), SIGKILL);
      } else if (data_handler->CheckSignalCaughtStatus()) {
        /* user triggerred reboot, no need to call abort */
        ALOGE("%s: Killing daemon as user triggered forced reboot", __func__);
        kill(getpid(), SIGKILL);
      } else {
        ALOGE("%s: Aborting daemon to recover as controller constructor is stuck", __func__);
        abort();
      }
#else
      ALOGE("%s: Killing daemon to recover as controller constructor is stuck", __func__);
      kill(getpid(), SIGKILL);
#endif
    }
    return false;
  }

  gettimeofday(&tv, NULL);
  snprintf(type_buff, sizeof(type_buff), "HCI Close rcvd from client type = %d", type);
  BtState::Get()->AddLogTag(dst_buff, tv, type_buff);
  BtState::Get()->SetTsHCIInitClose(HCI_CLOSE, dst_buff);
  controller_->WaitforCrashdumpFinish();

  bool status = false;
  bool Cleanup_Status = true;

  ALOGI("DataHandler:: init_status %d", init_status_);

  {
    /* Acquire a lock to overcome race conditions between Tx thread
     * and Rx thread before deleting callbacks.
     */
    std::unique_lock<std::mutex> guard(internal_mutex_);
    ALOGD("%s: Signal close to Diag interface", __func__);
    if (!diag_interface_.SignalHALCleanup(type)) {
      ALOGE("%s: Returning as SSR or cleanup in progress", __func__);
      Cleanup_Status = false;
    }

    it = protocol_info_.find(type);
    if (it != protocol_info_.end()) {
      ProtocolCallbacksType *cb_data =
        reinterpret_cast<ProtocolCallbacksType *> (it->second);
      delete (cb_data);
      protocol_info_.erase(it);
    }
  }

  SetClientStatus(false, type);

  /* Stop moving forward if the HAL Cleanup is in process */
  if (!Cleanup_Status)
    return false;

  if (data_handler && type == TYPE_BT && (protocol_info_.size() ||
      soc_need_reload_patch == false || IsSocAlwaysOnEnabled() ||
      data_handler->soc_type_ <= BT_SOC_ROME) && init_status_ == INIT_STATUS_SUCCESS) {

    data_handler->sendCommandWait(HCI_RESET_CMD, type);
  }

  std::unique_lock<std::mutex> guard(internal_mutex_);
  if (!protocol_info_.size()) {
    protocol_info_.clear();

    /* Unlock internal mutex to unblock Rx thread if they are any pending packets
     * to be posted, So that CC for Pre shutdown command will be recevied.
     */
    guard.unlock();
    if(data_handler && data_handler->soc_type_ >= BT_SOC_CHEROKEE &&
       init_status_ == INIT_STATUS_SUCCESS) {
      data_handler->sendCommandWait(SOC_PRE_SHUTDOWN_CMD, type);
    }

    if (init_status_ == INIT_STATUS_SUCCESS) {
      /* checking SocAlwaysOn property to decide whether to close
       * transport completely or partialy
       */
      if (!IsSocAlwaysOnEnabled()) {
        soc_need_reload_patch = true;
      } else {
        soc_need_reload_patch = false;
      }
#ifdef DUMP_RINGBUF_LOG
      //Dumping the ring buffer logs based on property for debugging purpose
      char value[PROPERTY_VALUE_MAX] = {'\0'};
      property_get("persist.vendor.service.bdroid.dump_ringbuff", value, "false");
      if(strcmp(value, "true") == 0) {
        logger_->PrepareDumpProcess();
        logger_->DumpRingBuffers();
      }
#endif
      /* Cleanup returns false when dump collected and other thread
       * is in process of doing post dump procedure.
       */
      if (!controller_->Cleanup()) {
        ALOGW("Skip controller cleanup as other thread is in process of cleanup: %s", __func__);
        return false;
      }
      ALOGW("controller Cleanup done");
      if (!data_handler->CheckSignalCaughtStatus()) {
        delete controller_;
        controller_ = nullptr;
      }
    }
    guard.lock();

    if (init_status_ > INIT_STATUS_IDLE ) {
      if ( INIT_STATUS_INITIALIZING == init_status_) {
        pthread_kill(init_thread_.native_handle(), SIGUSR1);

        // Sleep for 50 ms to ensure that thread exit happens
        usleep(50 * 1000);
        /* Get the Close and Init call time difference */
        time_diff = GetInitCloseTSDiff();

        // Ensure acquired mutex(s) are released
        logger_->UnlockRingbufferMutex();
        controller_->UnlockControllerMutex();
        logger_->UnlockDiagMutex();

        // Collect dumps only if there is no forced reboot
        if (!data_handler->CheckSignalCaughtStatus() && (time_diff >= HIDL_INIT_TIMEOUT)) {
          logger_->PrepareDumpProcess();

#ifdef DUMP_IPC_LOG
          if (soc_type_ != BT_SOC_SMD)
            logger_->DumpUartIpcLogs();
#endif

#ifdef DUMP_RINGBUF_LOG
          logger_->DumpRingBuffers();
#endif

          logger_->SetPrimaryCrashReason(REASON_PATCH_DLDNG_FAILED_SOCINIT);
          logger_->StoreCrashReason();
          //Dump last BT states of transport driver
          BtState :: Get()->DumpBtState();
        }
        /* complete transport cleanup as we are aborting/killing HIDL daemon */
        soc_need_reload_patch = true;
        // close the transport
        controller_->Disconnect();

#ifdef WAKE_LOCK_ENABLED
        Wakelock :: CleanUp();
#endif

#ifdef USER_DEBUG
        if (time_diff < HIDL_INIT_TIMEOUT) {
          /* close called before startup time (2.9 sec) is finished  */
          ALOGE("%s: Killing daemon to recover as close called before startup timer expiry",
                __func__);
          kill(getpid(), SIGKILL);
        } else if (data_handler->CheckSignalCaughtStatus()) {
          /* user triggerred reboot, no need to call abort */
          ALOGE("%s: Killing daemon as user triggered forced reboot", __func__);
          kill(getpid(), SIGKILL);
        } else {
          ALOGE("%s: Aborting daemon to recover as firmware download is stuck", __func__);
          abort();
        }
#else
        ALOGE("%s: Killing daemon to recover as firmware download is stuck", __func__);
        kill(getpid(), SIGKILL);
#endif
      }

      if (init_thread_.joinable()) {
        ALOGD("%s: joining init thread", __func__);
        init_thread_.join();
        ALOGI("DataHandler:: joined Init thread \n");
      }

      init_status_ = INIT_STATUS_IDLE;
    }
    BtState::Get()->Cleanup();
#ifdef WAKE_LOCK_ENABLED
    Wakelock :: CleanUp();
#endif

    status = true;
  }
  diag_interface_.SignalEndofCleanup(type);
  return status;
}

void DataHandler::UpdateRingBuffer(HciPacketType packet_type, const uint8_t *data, int length)
{
#ifdef DUMP_RINGBUF_LOG
  gettimeofday(&time_hci_cmd_arrived_, NULL);
  snprintf(last_hci_cmd_timestamp_.opcode, OPCODE_LEN, "0x%02x%02x", data[0], data[1]);
  AddHciCommandTag(last_hci_cmd_timestamp_.hcicmd_timestamp,
                   time_hci_cmd_arrived_, last_hci_cmd_timestamp_.opcode);
  logger_->SaveLastStackHciCommand(last_hci_cmd_timestamp_.hcicmd_timestamp);
#endif
  logger_->ProcessTx(packet_type, data, length);
}

void DataHandler::sendCommandWait(HciCommand cmd, ProtocolType type)
{
  char dst_buff[MAX_BUFF_SIZE];
  struct timeval tv;
  std::unique_lock<std::mutex> guard(evt_wait_mutex_);
  event_wait = false;
  switch (cmd) {
    case HCI_RESET_CMD:
    {
      HciPacketType packet_type = HCI_PACKET_TYPE_COMMAND;
      const uint8_t data[] = {0x03, 0x0C, 0x00};
      int length = 3;

      ALOGI("Sending HCI Reset \n");
      awaited_evt = *(uint16_t *)(&data);
      UpdateRingBuffer(packet_type, data, length);
      if (!controller_->SendPacket(packet_type, data, length)) {
        ALOGD("Unable to send HCI Reset \n");
        awaited_evt = 0;
        return;
      }

      if (!logger_->GetCleanupStatus(type)) {
        gettimeofday(&tv, NULL);
        BtState::Get()->AddLogTag(dst_buff, tv, (char *)"Sent first HCI_RESET_CMD");
        BtState::Get()->SetHciResetCmdSent(dst_buff);
        ALOGD("%s: Sent first HCI_RESET_CMD", __func__);
      }
      break;
    }

    case SOC_PRE_SHUTDOWN_CMD:
    {
      HciPacketType packet_type = HCI_PACKET_TYPE_COMMAND;
      const uint8_t data[] = {0x08, 0xFC, 0x00};
      int length = 3;

      ALOGI("Sending Pre-shutdown command \n");
      awaited_evt = *(uint16_t *)(&data);
      UpdateRingBuffer(packet_type, data, length);
      if (!controller_->SendPacket(packet_type, data, length)) {
        ALOGD("Unable to send Pre-shutdown command \n");
        awaited_evt = 0;
        return;
      }
      break;
    }

    case HCI_WRITE_BD_ADDRESS:
    {
      HciPacketType packet_type = HCI_PACKET_TYPE_COMMAND;
      uint8_t data[HCI_WRITE_BD_ADDRESS_LENGTH] = {0x14, 0xFC, 0x06};

      BluetoothAddress::GetLocalAddress(&data[HCI_WRITE_BD_ADDRESS_OFFSET]);
      BluetoothAddress::le2bd(&data[HCI_WRITE_BD_ADDRESS_OFFSET]);

      ALOGI("Sending HCI_WRITE_BD_ADDRESS command \n");
      awaited_evt = *(uint16_t *)(&data);
      UpdateRingBuffer(packet_type, data, HCI_WRITE_BD_ADDRESS_LENGTH);
      if (!controller_->SendPacket(packet_type, data, HCI_WRITE_BD_ADDRESS_LENGTH)) {
        ALOGD("Unable to send HCI_WRITE_BD_ADDRESS \n");
        awaited_evt = 0;
        return;
      }
      break;
    }

    default:
      return;
  }

  event_wait_cv.wait_for(guard, std::chrono::milliseconds(500), []{return event_wait;});
  awaited_evt = 0;
}

inline bool DataHandler::isAwaitedEvent(const uint8_t *buff, uint16_t len)
{
  if (len < MIN_OPCODE_LEN)
    return false;

  if(awaited_evt && (*(uint16_t *)(&buff[3])) == awaited_evt)
    return true;
  else
    return false;
}

void DataHandler::SetSignalCaught()
{
  caught_signal = true;
}

bool DataHandler::CheckSignalCaughtStatus()
{
  return caught_signal;
}

features_list* DataHandler:: GetAddOnFeatureList()
{
  if (isProtocolInitialized(TYPE_BT)) {
    return controller_->GetAddOnFeatureList();
  } else {
    return NULL;
  }
}

void DataHandler::StartSocCrashWaitTimer()
{
  if (controller_)
    controller_->StartSocCrashWaitTimer();
}

inline double DataHandler :: GetInitCloseTSDiff()
{
  return std::chrono::duration_cast<std::chrono::milliseconds>(CloseTime-InitTime).count();
}

} // namespace implementation
} // namespace V1_0
} // namespace bluetooth
} // namespace hardware
} // namespace android
