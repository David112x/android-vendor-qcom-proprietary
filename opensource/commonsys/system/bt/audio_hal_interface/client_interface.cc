/*
 * Copyright 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "BTAudioClientIf"

#include "client_interface.h"

#include <vendor/qti/hardware/bluetooth_audio/2.0/IBluetoothAudioPort.h>
#include <vendor/qti/hardware/bluetooth_audio/2.0/IBluetoothAudioProvidersFactory.h>
#include <base/logging.h>
#include <hidl/MQDescriptor.h>
#include <future>

#include "osi/include/log.h"

namespace bluetooth {
namespace audio {

using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
//using ::android::hardware::audio::common::V5_0::SourceMetadata;
using vendor::qti::hardware::bluetooth_audio::V2_0::IBluetoothAudioPort;
using vendor::qti::hardware::bluetooth_audio::V2_0::
    IBluetoothAudioProvidersFactory;
using DataMQ = ::android::hardware::MessageQueue<
    uint8_t, ::android::hardware::kSynchronizedReadWrite>;

static constexpr int kDefaultDataReadTimeoutMs = 3;      // 3 ms
static constexpr int kDefaultDataReadPollIntervalMs = 1;  // non-blocking poll

std::ostream& operator<<(std::ostream& os, const BluetoothAudioCtrlAck& ack) {
  switch (ack) {
    case BluetoothAudioCtrlAck::SUCCESS_FINISHED:
      return os << "SUCCESS_FINISHED";
    case BluetoothAudioCtrlAck::PENDING:
      return os << "PENDING";
    case BluetoothAudioCtrlAck::FAILURE_UNSUPPORTED:
      return os << "FAILURE_UNSUPPORTED";
    case BluetoothAudioCtrlAck::FAILURE_BUSY:
      return os << "FAILURE_BUSY";
    case BluetoothAudioCtrlAck::FAILURE_DISCONNECTING:
      return os << "FAILURE_DISCONNECTING";
    case BluetoothAudioCtrlAck::FAILURE:
      return os << "FAILURE";
    case BluetoothAudioCtrlAck::FAILURE_LONG_WAIT:
      return os << "FAILURE_LONG_WAIT";
    default:
      return os << "UNDEFINED " << static_cast<int8_t>(ack);
  }
}

// Internal class within BluetoothAudioClientInterfaceace to implement
// IBluetoothAudioPort (control interface used by Bluetooth Audio HAL)
class BluetoothAudioPortImpl : public IBluetoothAudioPort {
 public:
  BluetoothAudioPortImpl(IBluetoothTransportInstance* sink,
                         BluetoothAudioClientInterface* clientif)
      : sink_(sink), clientif_(clientif) {};

  Return<void> startStream() {
    android::sp<IBluetoothAudioProvider> provider =
                        clientif_->GetProvider();
    BluetoothAudioCtrlAck ack = sink_->StartRequest();
    if (provider && ack != BluetoothAudioCtrlAck::PENDING) {
      auto hidl_retval =
          provider->streamStarted(BluetoothAudioCtrlAckToHalStatus(ack));
      if (!hidl_retval.isOk()) {
        LOG(ERROR) << __func__ << ": BluetoothAudioHal Failure";
      }
    }
    return Void();
  }

  Return<void> suspendStream() {
    android::sp<IBluetoothAudioProvider> provider =
                    clientif_->GetProvider();
    BluetoothAudioCtrlAck ack = sink_->SuspendRequest();
    if (provider && ack != BluetoothAudioCtrlAck::PENDING) {
      auto hidl_retval =
          provider->streamSuspended(BluetoothAudioCtrlAckToHalStatus(ack));
      if (!hidl_retval.isOk()) {
        LOG(ERROR) << __func__ << ": BluetoothAudioHal Failure";
      }
    }
    return Void();
  }

  Return<void> stopStream() {
    sink_->StopRequest();
    return Void();
  }

  Return<void> getPresentationPosition(getPresentationPosition_cb _hidl_cb) {
    uint64_t remote_delay_report_ns;
    uint64_t total_bytes_read;
    timespec data_position;
    //std::unique_lock<std::mutex> guard(*clientif_->GetExternalMutex());
    bool retval = sink_->GetPresentationPosition(
        &remote_delay_report_ns, &total_bytes_read, &data_position);

    TimeSpec transmittedOctetsTimeStamp;
    if (retval) {
      transmittedOctetsTimeStamp = timespec_convert_to_hal(data_position);
    } else {
      remote_delay_report_ns = 0;
      total_bytes_read = 0;
      transmittedOctetsTimeStamp = {};
    }
    VLOG(2) << __func__ << ": result=" << retval
            << ", delay=" << remote_delay_report_ns
            << ", data=" << total_bytes_read
            << " byte(s), timestamp=" << toString(transmittedOctetsTimeStamp);
    _hidl_cb((retval ? BluetoothAudioStatus::SUCCESS
                     : BluetoothAudioStatus::FAILURE),
             remote_delay_report_ns, total_bytes_read,
             transmittedOctetsTimeStamp);
    return Void();
  }

  Return<void> updateAptxMode(uint16_t aptxMode) {
    return Void();
  }

 private:
  IBluetoothTransportInstance* sink_;
  BluetoothAudioClientInterface* clientif_;
  TimeSpec timespec_convert_to_hal(const timespec& ts) {
    return {.tvSec = static_cast<uint64_t>(ts.tv_sec),
            .tvNSec = static_cast<uint64_t>(ts.tv_nsec)};
  }
};

class BluetoothAudioDeathRecipient
    : public ::android::hardware::hidl_death_recipient {
 public:
  BluetoothAudioDeathRecipient(
      BluetoothAudioClientInterface* clientif,
       thread_t* message_loop)
      : bluetooth_audio_clientif_(clientif), message_loop_(message_loop) {
        LOG(WARNING) << __func__ << ": worker thread  " << message_loop_;
      }
  void serviceDied(
      uint64_t /*cookie*/,
      const ::android::wp<::android::hidl::base::V1_0::IBase>& /*who*/) {
    LOG(WARNING) << __func__ << ": restarting connection with new Audio Hal";
    if (bluetooth_audio_clientif_ != nullptr && message_loop_ != nullptr) {
      // restart the session on the correct thread
      LOG(WARNING) << __func__ << ": posting restart request to worker thread  "
                   << message_loop_;
      thread_post(message_loop_, RenewAudioProviderAndSession, bluetooth_audio_clientif_);
      LOG(WARNING) << __func__ << ": done with posting restart request";
    } else {
      LOG(ERROR) << __func__ << ": BluetoothAudioClientInterface corrupted";
    }
  }

  void updateDeathHandlerThread(thread_t* message_loop) {
    LOG(WARNING) << __func__ << ": new thread id is " << message_loop;
    message_loop_ = message_loop;
  }

 private:
  BluetoothAudioClientInterface* bluetooth_audio_clientif_;
  thread_t* message_loop_;
  static void RenewAudioProviderAndSession(void * clientif) {
    //TODO
    BluetoothAudioClientInterface *bluetooth_audio_clientif =
                reinterpret_cast<BluetoothAudioClientInterface*> (clientif);
    if (bluetooth_audio_clientif != nullptr) {
      bluetooth_audio_clientif->RenewAudioProviderAndSession();
    }
  }
};

BluetoothAudioClientInterface::BluetoothAudioClientInterface(
    IBluetoothTransportInstance* sink,
    thread_t* message_loop, std::mutex* mutex)
    : sink_(sink),
      provider_(nullptr),
      stack_if_(nullptr),
      session_started_(false),
      mDataMQ(nullptr),
      external_mutex_(mutex),
      death_recipient_(new BluetoothAudioDeathRecipient(this, message_loop)) {}

std::vector<AudioCapabilities>
BluetoothAudioClientInterface::GetAudioCapabilities() const {
  return capabilities_;
}

android::sp<IBluetoothAudioProvider> BluetoothAudioClientInterface::GetProvider() {
  return provider_;
}

std::mutex* BluetoothAudioClientInterface::GetExternalMutex() {
  return external_mutex_;
}

void BluetoothAudioClientInterface::fetch_audio_provider() {
  if (provider_ != nullptr) {
    LOG(WARNING) << __func__ << ": reflash";
  }

  android::sp<IBluetoothAudioProvidersFactory> providersFactory =
      IBluetoothAudioProvidersFactory::getService();
  LOG(INFO) << "IBluetoothAudioProvidersFactory::getService() returned "
            << providersFactory.get()
            << (providersFactory->isRemote() ? " (remote)" : " (local)");
  CHECK(providersFactory != nullptr);

  std::promise<void> getProviderCapabilities_promise;
  auto getProviderCapabilities_future =
      getProviderCapabilities_promise.get_future();
  auto getProviderCapabilities_cb =
      [& capabilities = this->capabilities_, &getProviderCapabilities_promise](
          const hidl_vec<AudioCapabilities>& audioCapabilities) {
        capabilities.clear();
        for (auto capability : audioCapabilities) {
          capabilities.push_back(capability);
        }
        getProviderCapabilities_promise.set_value();
      };
  auto hidl_retval = providersFactory->getProviderCapabilities(
      sink_->GetSessionType(), getProviderCapabilities_cb);
  getProviderCapabilities_future.get();
  if (!hidl_retval.isOk()) {
    LOG(ERROR) << __func__
               << ": BluetoothAudioHal::getProviderCapabilities Failure";
    return;
  } else if (capabilities_.empty()) {
    LOG(WARNING) << __func__
                 << ": SessionType=" << toString(sink_->GetSessionType())
                 << " Not supported by BluetoothAudioHal";
    return;
  }
  LOG(INFO) << __func__ << ": BluetoothAudioHal SessionType="
            << toString(sink_->GetSessionType()) << " has "
            << capabilities_.size() << " AudioCapabilities";

  std::promise<void> openProvider_promise;
  auto openProvider_future = openProvider_promise.get_future();
  auto openProvider_cb =
      [& provider_ = this->provider_, &openProvider_promise](
          BluetoothAudioStatus status,
          const android::sp<IBluetoothAudioProvider>& provider) {
        LOG(INFO) << "openProvider_cb(" << toString(status) << ")";
        if (status == BluetoothAudioStatus::SUCCESS) {
          provider_ = provider;
        }
        ALOGE_IF(!provider_, "Failed to open BluetoothAudio provider");
        openProvider_promise.set_value();
      };
  hidl_retval =
      providersFactory->openProvider(sink_->GetSessionType(), openProvider_cb);
  openProvider_future.get();
  if (!hidl_retval.isOk()) {
    LOG(ERROR) << __func__ << ": BluetoothAudioHal::openProvider Failure";
  }
  CHECK(provider_ != nullptr);

  LOG(INFO) << "IBluetoothAudioProvidersFactory::openProvider() returned "
            << provider_.get()
            << (provider_->isRemote() ? " (remote)" : " (local)");
}

bool BluetoothAudioClientInterface::UpdateAudioConfig(
    const AudioConfiguration& audio_config) {
  bool is_software_audio_config =
      (sink_->GetSessionType() ==
           SessionType::A2DP_SOFTWARE_ENCODING_DATAPATH ||
       sink_->GetSessionType() ==
           SessionType::HEARING_AID_SOFTWARE_ENCODING_DATAPATH);
  bool is_offload_audio_config =
      (sink_->GetSessionType() == SessionType::A2DP_HARDWARE_OFFLOAD_DATAPATH);
  if (!is_software_audio_config && !is_offload_audio_config) {
    return false;
  }
  sink_->UpdateAudioConfiguration(audio_config);
  return true;
}

int BluetoothAudioClientInterface::StartSession() {
  fetch_audio_provider();
  if (provider_ == nullptr) {
    LOG(ERROR) << __func__ << ": BluetoothAudioHal nullptr";
    session_started_ = false;
    return -EINVAL;
  }
  if (session_started_) {
    LOG(ERROR) << __func__ << ": session started already";
    return -EBUSY;
  }
  if(!stack_if_) {
    stack_if_ = new BluetoothAudioPortImpl(sink_, this);
  }
  std::unique_ptr<DataMQ> tempDataMQ;
  BluetoothAudioStatus session_status;

  std::promise<void> hidl_startSession_promise;
  auto hidl_startSession_future = hidl_startSession_promise.get_future();
  auto hidl_cb = [&session_status, &tempDataMQ, &hidl_startSession_promise](
                     BluetoothAudioStatus status,
                     const DataMQ::Descriptor& dataMQ) {
    LOG(INFO) << "startSession_cb(" << toString(status) << ")";
    session_status = status;
    if (status == BluetoothAudioStatus::SUCCESS && dataMQ.isHandleValid()) {
      tempDataMQ.reset(new DataMQ(dataMQ));
    }
    hidl_startSession_promise.set_value();
  };

  if (!provider_->linkToDeath(death_recipient_, 0).isOk()) {
    LOG(ERROR) << __func__ << ": BluetoothAudioDeathRecipient Failure";
  }

  auto hidl_retval = provider_->startSession(
      stack_if_, sink_->GetAudioConfiguration(), hidl_cb);
  hidl_startSession_future.get();
  if (!hidl_retval.isOk()) {
    LOG(ERROR) << __func__ << ": BluetoothAudioHal Failure";
    return -EPROTO;
  }

  if (tempDataMQ && tempDataMQ->isValid()) {
    mDataMQ = std::move(tempDataMQ);
  } else if (sink_->GetSessionType() ==
                 SessionType::A2DP_HARDWARE_OFFLOAD_DATAPATH &&
             session_status == BluetoothAudioStatus::SUCCESS) {
    sink_->ResetPresentationPosition();
    session_started_ = true;
    return 0;
  }
  if (mDataMQ && mDataMQ->isValid()) {
    sink_->ResetPresentationPosition();
    session_started_ = true;
    return 0;
  } else {
    ALOGE_IF(!mDataMQ, "Failed to obtain audio data path");
    ALOGE_IF(mDataMQ && !mDataMQ->isValid(), "Audio data path is invalid");
    session_started_ = false;
    return -EIO;
  }
}

void BluetoothAudioClientInterface::StreamStarted(
    const BluetoothAudioCtrlAck& ack) {
  if (provider_ == nullptr) {
    LOG(ERROR) << __func__ << ": BluetoothAudioHal nullptr";
    return;
  }
  BluetoothAudioStatus status = BluetoothAudioCtrlAckToHalStatus(ack);
  auto hidl_retval = provider_->streamStarted(status);
  if (!hidl_retval.isOk()) {
    LOG(ERROR) << __func__ << ": BluetoothAudioHal Failure";
  }
}

void BluetoothAudioClientInterface::updateSessionParams(
                 const SessionParams& sessionParams) {
  if (provider_ == nullptr) {
    LOG(ERROR) << __func__ << ": BluetoothAudioHal nullptr";
    return;
  }
  auto hidl_retval = provider_->updateSessionParams(sessionParams);
  if (!hidl_retval.isOk()) {
    LOG(ERROR) << __func__ << ": BluetoothAudioHal Failure";
  }
}

void BluetoothAudioClientInterface::UpdateDeathHandlerThread(
                                  thread_t* message_loop) {
  death_recipient_->updateDeathHandlerThread(message_loop);
}

void BluetoothAudioClientInterface::StreamSuspended(
    const BluetoothAudioCtrlAck& ack) {
  if (provider_ == nullptr) {
    LOG(ERROR) << __func__ << ": BluetoothAudioHal nullptr";
    return;
  }
  BluetoothAudioStatus status = BluetoothAudioCtrlAckToHalStatus(ack);
  auto hidl_retval = provider_->streamSuspended(status);
  if (!hidl_retval.isOk()) {
    LOG(ERROR) << __func__ << ": BluetoothAudioHal Failure";
  }
}

int BluetoothAudioClientInterface::EndSession() {
  if (!session_started_) {
    LOG(INFO) << __func__ << ": sessoin ended already";
    return 0;
  }

  session_started_ = false;
  if (provider_ == nullptr) {
    LOG(ERROR) << __func__ << ": BluetoothAudioHal nullptr";
    return -EINVAL;
  }
  mDataMQ = nullptr;
  auto hidl_retval = provider_->endSession();
  if (!hidl_retval.isOk()) {
    LOG(ERROR) << __func__ << ": BluetoothAudioHal Failure";
    return -EPROTO;
  }
  auto retval = provider_->unlinkToDeath(death_recipient_);
  if (!retval.isOk()) {
    LOG(ERROR) << __func__ << ": BluetoothAudioDeathRecipient Failure";
  }
  return 0;
}

size_t BluetoothAudioClientInterface::ReadAudioData(uint8_t* p_buf,
                                                    uint32_t len) {
  if (provider_ == nullptr) {
    LOG(ERROR) << __func__ << ": BluetoothAudioHal nullptr";
    return 0;
  }
  if (p_buf == nullptr || len == 0) return 0;

  size_t total_read = 0;
  int timeout_ms = kDefaultDataReadTimeoutMs;
  do {
    if (mDataMQ == nullptr || !mDataMQ->isValid()) break;

    size_t avail_to_read = mDataMQ->availableToRead();
    if (avail_to_read) {
      if (avail_to_read > len - total_read) {
        avail_to_read = len - total_read;
      }
      if (mDataMQ->read(p_buf + total_read, avail_to_read) == 0) {
        LOG(WARNING) << __func__ << ": len=" << len
                     << " total_read=" << total_read << " failed";
        break;
      }
      total_read += avail_to_read;
    } else if (timeout_ms >= kDefaultDataReadPollIntervalMs) {
      std::this_thread::sleep_for(
          std::chrono::milliseconds(kDefaultDataReadPollIntervalMs));
      timeout_ms -= kDefaultDataReadPollIntervalMs;
      continue;
    } else {
      LOG(WARNING) << __func__ << ": " << (len - total_read) << "/" << len
                   << " no data " << (kDefaultDataReadTimeoutMs - timeout_ms)
                   << " ms";
      break;
    }
  } while (total_read < len);

  if (timeout_ms <
          (kDefaultDataReadTimeoutMs - kDefaultDataReadPollIntervalMs) &&
      timeout_ms >= kDefaultDataReadPollIntervalMs) {
    VLOG(1) << __func__ << ": underflow " << len << " -> " << total_read
            << " read " << (kDefaultDataReadTimeoutMs - timeout_ms) << " ms";
  } else {
    VLOG(2) << __func__ << ": " << len << " -> " << total_read << " read";
  }

  sink_->LogBytesRead(total_read);
  return total_read;
}

size_t BluetoothAudioClientInterface::WriteAudioData(uint8_t* p_buf,
                                                     uint32_t len) {
  // Not implemented!
  return 0;
}

void BluetoothAudioClientInterface::RenewAudioProviderAndSession() {
  // NOTE: must be invoked on the same thread where this
  // BluetoothAudioClientInterface is running
  std::unique_lock<std::mutex> guard(*external_mutex_);
  provider_ = nullptr;
  usleep(500000); //sleep for 0.5sec for hal server to restart
  session_started_ = false;
  StartSession();
}

}  // namespace audio
}  // namespace bluetooth
