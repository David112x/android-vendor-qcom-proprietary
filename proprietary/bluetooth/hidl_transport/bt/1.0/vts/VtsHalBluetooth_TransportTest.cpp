/*
 *Copyright (c) 2019 Qualcomm Technologies, Inc.
 *All Rights Reserved.
 *Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 *  Not a contribution.
 *
 * Copyright (C) 2016 The Android Open Source Project
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
#define LOG_TAG "bluetooth_hidl_hal_test"
#include <csignal>
#include <ctime>
#include <android-base/logging.h>

#include <android/hardware/bluetooth/1.0/IBluetoothHci.h>
#include <android/hardware/bluetooth/1.0/IBluetoothHciCallbacks.h>
#include <android/hardware/bluetooth/1.0/types.h>
#include <hardware/bluetooth.h>
#include <utils/Log.h>
#include <wake_lock.h>

#include <VtsHalHidlTargetCallbackBase.h>
#include <VtsHalHidlTargetTestBase.h>
#include <VtsHalHidlTargetTestEnvBase.h>

#include <mutex>
#include <chrono>
#include <queue>
#include <thread>
#include <cutils/properties.h>
#include <hwbinder/IPCThreadState.h>

using android::hardware::IPCThreadState;
using ::android::sp;
using ::android::hardware::hidl_death_recipient;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hardware::bluetooth::V1_0::IBluetoothHci;
using ::android::hardware::bluetooth::V1_0::IBluetoothHciCallbacks;
using ::android::hardware::bluetooth::V1_0::Status;

#define HCI_MINIMUM_HCI_VERSION 5  // Bluetooth Core Specification 3.0 + HS
#define HCI_MINIMUM_LMP_VERSION 5  // Bluetooth Core Specification 3.0 + HS
#define NUM_HCI_COMMANDS_BANDWIDTH 1000
#define NUM_SCO_PACKETS_BANDWIDTH 1000
#define NUM_ACL_PACKETS_BANDWIDTH 1000
#define WAIT_FOR_INIT_TIMEOUT std::chrono::milliseconds(2900)
#define WAIT_FOR_SSR (5000)
#define WAIT_FOR_WDOG_SSR (8000)
#define WAIT_FOR_HCI_EVENT_TIMEOUT std::chrono::milliseconds(2000)
#define WAIT_FOR_SCO_DATA_TIMEOUT std::chrono::milliseconds(1000)
#define WAIT_FOR_ACL_DATA_TIMEOUT std::chrono::milliseconds(1000)
#define WAIT_FOR_RX_THREAD_STUCK_TIMEOUT std::chrono::milliseconds(3000)
#define INTERFACE_CLOSE_DELAY_MS (200)
#define HEX( x ) std::setw(2) << std::setfill('0') << std::hex << (int)( x )
#define MAX_CMD_LOG (10)
#define MAX_DATA_LOG (10)
#define MAX_EVENT_LOG (10)

#define COMMAND_HCI_SHOULD_BE_UNKNOWN \
  { 0xff, 0x3B, 0x08, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 }
#define COMMAND_HCI_READ_LOCAL_VERSION_INFORMATION \
  { 0x01, 0x10, 0x00 }
#define COMMAND_HCI_READ_BUFFER_SIZE \
  { 0x05, 0x10, 0x00 }
#define COMMAND_HCI_WRITE_LOOPBACK_MODE_LOCAL \
  { 0x02, 0x18, 0x01, 0x01 }
#define COMMAND_HCI_RESET \
  { 0x03, 0x0c, 0x00 }
#define COMMAND_HCI_WRITE_LOCAL_NAME \
  { 0x13, 0x0c, 0xf8 }
#define HCI_STATUS_SUCCESS 0x00
#define HCI_STATUS_UNKNOWN_HCI_COMMAND 0x01
#define COMMAND_HCI_GET_DEBUG_INFO \
  { 0x5B, 0xFD, 0x00 }
#define SSR_SW_ERROR_FATAL \
  { 0x0C, 0xFC, 0x01, 0x26 }
#define SSR_SW_EXCEPTION_DIV_BY_0 \
  { 0x0C, 0xFC, 0x01, 0x27 }
#define SSR_SW_EXCEPTION_NULL_PTR \
  { 0x0C, 0xFC, 0x01, 0x28 }
#define SSR_WATCHDOG_BITS \
  { 0x0C, 0xFC, 0x01, 0x29 }


#define EVENT_CONNECTION_COMPLETE 0x03
#define EVENT_COMMAND_COMPLETE 0x0e
#define EVENT_COMMAND_STATUS 0x0f
#define EVENT_NUMBER_OF_COMPLETED_PACKETS 0x13
#define EVENT_LOOPBACK_COMMAND 0x19
#define VENDOR_SPECIFIC_EVENT 0xFF
#define VENDOR_SPECIFIC_EVENT_CODE_BYTE1 0xFC
#define VENDOR_SPECIFIC_EVENT_CODE_BYTE2 0x00

#define EVENT_CODE_BYTE 0
#define EVENT_LENGTH_BYTE 1
#define EVENT_FIRST_PAYLOAD_BYTE 2
#define EVENT_COMMAND_STATUS_STATUS_BYTE 2
#define EVENT_COMMAND_STATUS_ALLOWED_PACKETS_BYTE 3
#define EVENT_COMMAND_STATUS_OPCODE_LSBYTE 4  // Bytes 4 and 5
#define EVENT_COMMAND_COMPLETE_ALLOWED_PACKETS_BYTE 2
#define EVENT_COMMAND_COMPLETE_OPCODE_LSBYTE 3  // Bytes 3 and 4
#define EVENT_COMMAND_COMPLETE_STATUS_BYTE 5
#define EVENT_COMMAND_COMPLETE_FIRST_PARAM_BYTE 6
#define EVENT_LOCAL_HCI_VERSION_BYTE EVENT_COMMAND_COMPLETE_FIRST_PARAM_BYTE
#define EVENT_LOCAL_LMP_VERSION_BYTE EVENT_LOCAL_HCI_VERSION_BYTE + 3

#define EVENT_CONNECTION_COMPLETE_PARAM_LENGTH 11
#define EVENT_CONNECTION_COMPLETE_TYPE 11
#define EVENT_CONNECTION_COMPLETE_TYPE_SCO 0
#define EVENT_CONNECTION_COMPLETE_TYPE_ACL 1
#define EVENT_CONNECTION_COMPLETE_HANDLE_LSBYTE 3
#define EVENT_COMMAND_STATUS_LENGTH 4

#define EVENT_NUMBER_OF_COMPLETED_PACKETS_NUM_HANDLES 2

#define ACL_BROADCAST_FLAG_OFFSET 6
#define ACL_BROADCAST_FLAG_POINT_TO_POINT 0x0
#define ACL_BROADCAST_POINT_TO_POINT \
  (ACL_BROADCAST_FLAG_POINT_TO_POINT << ACL_BROADCAST_FLAG_OFFSET)

#define ACL_PACKET_BOUNDARY_FLAG_OFFSET 4
#define ACL_PACKET_BOUNDARY_FLAG_FIRST_AUTO_FLUSHABLE 0x2
#define ACL_PACKET_BOUNDARY_FIRST_AUTO_FLUSHABLE \
  (ACL_PACKET_BOUNDARY_FLAG_FIRST_AUTO_FLUSHABLE \
   << ACL_PACKET_BOUNDARY_FLAG_OFFSET)

// To be removed in VTS release builds
#define ACL_HANDLE_QCA_DEBUG_MESSAGE 0xedc

constexpr char kCallbackNameAclEventReceived[] = "aclDataReceived";
constexpr char kCallbackNameHciEventReceived[] = "hciEventReceived";
constexpr char kCallbackNameInitializationComplete[] = "initializationComplete";
constexpr char kCallbackNameScoEventReceived[] = "scoDataReceived";

static const std::string kMaxAppSleepTimer = "--maxAppSleepTimer=";
static const std::string kMinAppSleepTimer = "--minAppSleepTimer=";
static const std::string kMaxIterations = "--maxIterations=";
static const std::string kMaxMsgsPerIteration = "--maxMsgsPerIteration=";
static const std::string kLogEvents = "--logEvents";
static const std::string kConsoleLog = "--consoleLog";
static const std::string kLogCommands = "--logCommands";
static const std::string kLogData = "--logData";
static uint32_t maxAppSleepTimer = 1000; // in ms
static uint32_t minAppSleepTimer = 0; // in ms
static uint32_t maxIterations = 1;
static uint32_t maxMsgsPerIteration = 1;
static bool     logEvents = false;
static bool     consoleLog = false;
static bool     logCommands = false;
static bool     logData = false;
bool timerExpired = false;

enum SleepTimerState {
  TIMER_NOT_CREATED = 0x00,
  TIMER_CREATED,
  TIMER_ACTIVE
};

class ThroughputLogger {
 public:
  ThroughputLogger(std::string task)
      : task_(task), start_time_(std::chrono::steady_clock::now()) {}

  ~ThroughputLogger() {
    if (total_bytes_ == 0) return;
    std::chrono::duration<double> duration =
        std::chrono::steady_clock::now() - start_time_;
    double s = duration.count();
    if (s == 0) return;
    double rate_kb = (static_cast<double>(total_bytes_) / s) / 1024;
    ALOGI("%s %.1f KB/s (%zu bytes in %.3fs)", task_.c_str(), rate_kb,
          total_bytes_, s);
  }

  void setTotalBytes(size_t total_bytes) { total_bytes_ = total_bytes; }

 private:
  size_t total_bytes_;
  std::string task_;
  std::chrono::steady_clock::time_point start_time_;
};

// Test environment for Bluetooth HIDL HAL.
class BluetoothHidlEnvironment : public ::testing::VtsHalHidlTargetTestEnvBase {
 public:
  // get the test environment singleton
  static BluetoothHidlEnvironment* Instance() {
    static BluetoothHidlEnvironment* instance = new BluetoothHidlEnvironment;
    return instance;
  }

  virtual void registerTestServices() override {
    registerTestService<IBluetoothHci>();
  }

 private:
  BluetoothHidlEnvironment() {}
};

#ifdef ARCH_ARM_32
bool IsLazyHalSupported()
{
  static bool isPropertyRead = false;
  static bool isLazyHalEnabled = false;

  ALOGD("%s isPropertyRead: %d isLazyHalEnabled: %d", __func__, isPropertyRead,
         isLazyHalEnabled);

  if (!isPropertyRead) {
    char LazyService[PROPERTY_VALUE_MAX]= {'\0'};
    int len = property_get("ro.vendor.bluetooth.lazyhal", LazyService, "false");
    if (len) {
      isPropertyRead = true;
      isLazyHalEnabled = (!strcmp(LazyService, "true") ? true : false);
      ALOGD("%s isLazyHalEnabled: %d", __func__, isLazyHalEnabled);
    } else {
      ALOGE("%s: Failed to read property", __func__);
    }
  }

  return isLazyHalEnabled;
}
#endif

// The main test class for Bluetooth HIDL HAL.
class BluetoothTransportTest : public ::testing::VtsHalHidlTargetTestBase {
 public:
  virtual void SetUp() override {
    Wakelock::Init();
    Wakelock::Acquire();
    // currently test passthrough mode only
    bluetooth =
        ::testing::VtsHalHidlTargetTestBase::getService<IBluetoothHci>();
    ASSERT_NE(bluetooth, nullptr);
    ALOGI("%s: getService() for bluetooth is %s", __func__,
          bluetooth->isRemote() ? "remote" : "local");

    bluetooth_hci_death_recipient = new BluetoothHciDeathRecipient(*this);
    ASSERT_NE(bluetooth_hci_death_recipient, nullptr);
    ASSERT_TRUE(
        bluetooth->linkToDeath(bluetooth_hci_death_recipient, 0).isOk());

    bluetooth_cb = new BluetoothHciCallbacks(*this);
    ASSERT_NE(bluetooth_cb, nullptr);

    max_acl_data_packet_length = 0;
    max_sco_data_packet_length = 0;
    max_acl_data_packets = 0;
    max_sco_data_packets = 0;
    simulateRxThreadStuck = false;
    serviceDiedOk = false;

    initialized = false;
    event_cb_count = 0;
    acl_cb_count = 0;
    sco_cb_count = 0;

    ASSERT_FALSE(initialized);
    // Should not be checked in production code
    ASSERT_TRUE(bluetooth->initialize(bluetooth_cb).isOk());

    bluetooth_cb->SetWaitTimeout(kCallbackNameInitializationComplete,
                                 WAIT_FOR_INIT_TIMEOUT);
    bluetooth_cb->SetWaitTimeout(kCallbackNameHciEventReceived,
                                 WAIT_FOR_HCI_EVENT_TIMEOUT);
    bluetooth_cb->SetWaitTimeout(kCallbackNameAclEventReceived,
                                 WAIT_FOR_ACL_DATA_TIMEOUT);
    bluetooth_cb->SetWaitTimeout(kCallbackNameScoEventReceived,
                                 WAIT_FOR_SCO_DATA_TIMEOUT);

    EXPECT_TRUE(bluetooth_cb->WaitForCallback(kCallbackNameInitializationComplete).no_timeout);

    Wakelock::Release();
    ASSERT_TRUE(initialized);
  }

  virtual void TearDown() override {
    // Should not be checked in production code
    Wakelock::Acquire();
    ASSERT_TRUE(bluetooth->close().isOk());

#ifdef ARCH_ARM_32
    if (IsLazyHalSupported()) {
      ALOGD("%s: decrementing HIDL usage counter", __func__);
      IPCThreadState::self()->flushCommands();
    }
#endif

    bluetooth = nullptr;
    std::this_thread::sleep_for(std::chrono::milliseconds(INTERFACE_CLOSE_DELAY_MS));
    ALOGI("TearDown: SleepTimer Expired");
    handle_no_ops();
    EXPECT_EQ(static_cast<size_t>(0), event_queue.size());
    EXPECT_EQ(static_cast<size_t>(0), sco_queue.size());
    EXPECT_EQ(static_cast<size_t>(0), acl_queue.size());
    Wakelock::Release();
    Wakelock::CleanUp();
    ALOGI("TearDown");
  }

  void setBufferSizes();
  void sendHCICommand(hidl_vec<uint8_t> cmd);

  // Functions called from within tests in loopback mode
  void sendAndCheckHCI(int num_packets);
  void sendAndCheckSCO(int num_packets, size_t size, uint16_t handle);
  void sendAndCheckACL(int num_packets, size_t size, uint16_t handle);

  // Helper functions to try to get a handle on verbosity
  void enterLoopbackMode(std::vector<uint16_t>& sco_handles,
                         std::vector<uint16_t>& acl_handles);
  void handle_no_ops();
  void wait_for_event(bool timeout_is_error);
  void wait_for_command_complete_event(hidl_vec<uint8_t> cmd);
  int wait_for_completed_packets_event(uint16_t handle);

  void StartSleepTimer(int sleepTime);
  bool StopSleepTimer();
  class BluetoothHciDeathRecipient : public hidl_death_recipient {
    BluetoothTransportTest& parent_;
   public:
    BluetoothHciDeathRecipient(BluetoothTransportTest& parent) : parent_(parent){};
    void serviceDied(
        uint64_t /*cookie*/,
        const android::wp<::android::hidl::base::V1_0::IBase>& /*who*/)
        override {
      if (!parent_.serviceDiedOk) {
		    std::cout << "Service Died: " << std::endl;
        ALOGE("%s: Service Died", __func__);
        FAIL();
      } else {
		    std::cout << "Service Died: As expected" << std::endl;
      }
    }
  };

  // A simple test implementation of BluetoothHciCallbacks.
  class BluetoothHciCallbacks
      : public ::testing::VtsHalHidlTargetCallbackBase<BluetoothTransportTest>,
        public IBluetoothHciCallbacks {
    BluetoothTransportTest& parent_;

   public:
    BluetoothHciCallbacks(BluetoothTransportTest& parent) : parent_(parent){};

    virtual ~BluetoothHciCallbacks() = default;

    Return<void> initializationComplete(Status status) override {
      std::cout << __func__ << "- Status: " << (int32_t)status << std::endl;
      parent_.initialized = (status == Status::SUCCESS);
      NotifyFromCallback(kCallbackNameInitializationComplete);
      ALOGI("%s (status = %d)", __func__, static_cast<int>(status));
      return Void();
    };

    Return<void> hciEventReceived(
        const ::android::hardware::hidl_vec<uint8_t>& event) override {
      // Ignore enhanced controller logs
      if ((event[0] == VENDOR_SPECIFIC_EVENT) &&
          (event[2] == VENDOR_SPECIFIC_EVENT_CODE_BYTE1) &&
          (event[3] == VENDOR_SPECIFIC_EVENT_CODE_BYTE2)) {
        std::cout << "Ignoring Enhanced Controller Logs" << std::endl;
        return Void();
      }
      if (parent_.simulateRxThreadStuck) {
        // Don't process events for some time
        std::cout << "Simulating Rx thread wait" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_FOR_RX_THREAD_STUCK_TIMEOUT));
        ALOGI("hciEventReceived: RxThreadStuck SleepTimer Expired");
        return Void();
      }

      parent_.event_cb_count++;
      parent_.event_queue.push(event);
      NotifyFromCallback(kCallbackNameHciEventReceived);
      ALOGI("Event received #:%d (length = %d)", parent_.event_cb_count, static_cast<int>(event.size()));
      if (logEvents) {
        std::ostringstream os;
        std::unique_lock<std::mutex> guard(parent_.log_mutex);
        os << "Event received:" << parent_.event_cb_count << " Length: 0x" << event.size() << "  ";
        for(int i = 0; (i < MAX_EVENT_LOG) && (i < event.size()); i++)
          os << " 0x" << HEX(event[i]);
        os << std::endl;
        if (consoleLog)
          std::cout << os.str();
        ALOGI("%s: %s", __func__, os.str().c_str());
      }
      return Void();
    };

    Return<void> aclDataReceived(
        const ::android::hardware::hidl_vec<uint8_t>& data) override {
      parent_.acl_cb_count++;
      parent_.acl_queue.push(data);
      NotifyFromCallback(kCallbackNameAclEventReceived);
      ALOGI("ACL Data received #:%d (length = %d)", parent_.acl_cb_count, static_cast<int>(data.size()));
      std::cout << "AclData received #:" << parent_.acl_cb_count << " Length: 0x" << data.size() << std::endl;
      if (logData) {
        std::unique_lock<std::mutex> guard(parent_.log_mutex);
        for(int i = 0; (i < MAX_DATA_LOG) && (i < data.size()); i++)
          std::cout << " 0x" << HEX(data[i]);
        std::cout << std::endl;
      }
      return Void();
    };

    Return<void> scoDataReceived(
        const ::android::hardware::hidl_vec<uint8_t>& data) override {
      parent_.sco_cb_count++;
      parent_.sco_queue.push(data);
      NotifyFromCallback(kCallbackNameScoEventReceived);
      ALOGI("SCO Data received (length = %d)", static_cast<int>(data.size()));
      if (logData) {
        std::unique_lock<std::mutex> guard(parent_.log_mutex);
        std::cout << "ScoData received #:" << parent_.sco_cb_count << " Length: 0x " << data.size() << std::endl;
        for(int i = 0; (i < MAX_DATA_LOG) && (i < data.size()); i++)
          std::cout << " 0x" << HEX(data[i]);
        std::cout << std::endl;
      }
      return Void();
    };
  };

  sp<IBluetoothHci> bluetooth;
  sp<BluetoothHciCallbacks> bluetooth_cb;
  sp<BluetoothHciDeathRecipient> bluetooth_hci_death_recipient;
  std::queue<hidl_vec<uint8_t>> event_queue;
  std::queue<hidl_vec<uint8_t>> acl_queue;
  std::queue<hidl_vec<uint8_t>> sco_queue;

  bool initialized;

  int event_cb_count;
  int sco_cb_count;
  int acl_cb_count;

  int max_acl_data_packet_length;
  int max_sco_data_packet_length;
  int max_acl_data_packets;
  int max_sco_data_packets;
  static bool simulateRxThreadStuck;
  static bool serviceDiedOk;
  static std::mutex log_mutex;
  static int sleep_timer_state_;
  timer_t sleep_timer_;
  static std::condition_variable cv;
  static std::mutex cv_m;
  static void SleepTimerExpiry(union sigval sig);
};

bool BluetoothTransportTest::simulateRxThreadStuck = false;
bool BluetoothTransportTest::serviceDiedOk = false;
std::mutex BluetoothTransportTest::log_mutex;
int BluetoothTransportTest::sleep_timer_state_ = TIMER_NOT_CREATED;
std::condition_variable BluetoothTransportTest::cv;
std::mutex BluetoothTransportTest::cv_m;

// Discard NO-OPs from the event queue.
void BluetoothTransportTest::handle_no_ops() {
  ALOGI("%s", __func__);
  while (event_queue.size() > 0) {
    hidl_vec<uint8_t> event = event_queue.front();
    EXPECT_GE(event.size(),
              static_cast<size_t>(EVENT_COMMAND_COMPLETE_STATUS_BYTE));
    bool event_is_no_op =
        (event[EVENT_CODE_BYTE] == EVENT_COMMAND_COMPLETE) &&
        (event[EVENT_COMMAND_COMPLETE_OPCODE_LSBYTE] == 0x00) &&
        (event[EVENT_COMMAND_COMPLETE_OPCODE_LSBYTE + 1] == 0x00);
    event_is_no_op |= (event[EVENT_CODE_BYTE] == EVENT_COMMAND_STATUS) &&
                      (event[EVENT_COMMAND_STATUS_OPCODE_LSBYTE] == 0x00) &&
                      (event[EVENT_COMMAND_STATUS_OPCODE_LSBYTE + 1] == 0x00);
    if (event_is_no_op) {
      event_queue.pop();
    } else {
      break;
    }
  }
  // To be removed in VTS release builds
  while (acl_queue.size() > 0) {
    hidl_vec<uint8_t> acl_packet = acl_queue.front();
    uint16_t connection_handle = acl_packet[1] & 0xF;
    connection_handle <<= 8;
    connection_handle |= acl_packet[0];
    bool packet_is_no_op = connection_handle == ACL_HANDLE_QCA_DEBUG_MESSAGE;
    if (packet_is_no_op) {
      acl_queue.pop();
    } else {
      break;
    }
  }
}

// Receive an event, discarding NO-OPs.
void BluetoothTransportTest::wait_for_event(bool timeout_is_error = true) {
  ALOGI("%s", __func__);
  hidl_vec<uint8_t> event;
  do {
    Wakelock::Acquire();
    bool no_timeout =
        bluetooth_cb->WaitForCallback(kCallbackNameHciEventReceived).no_timeout;
    Wakelock::Release();
    EXPECT_TRUE(no_timeout || !timeout_is_error);
    if (no_timeout && timeout_is_error) {
      EXPECT_LT(static_cast<size_t>(0), event_queue.size());
    }
    if (event_queue.size() == 0) {
      // WaitForCallback timed out.
      return;
    }
    handle_no_ops();
  } while (event_queue.size() == 0);
}

// Wait until a COMMAND_COMPLETE is received.
void BluetoothTransportTest::wait_for_command_complete_event(hidl_vec<uint8_t> cmd) {
  ALOGI("%s", __func__);
  wait_for_event();
  hidl_vec<uint8_t> event = event_queue.front();
  event_queue.pop();

  EXPECT_GT(event.size(),
            static_cast<size_t>(EVENT_COMMAND_COMPLETE_STATUS_BYTE));
  EXPECT_EQ(EVENT_COMMAND_COMPLETE, event[EVENT_CODE_BYTE]);
  EXPECT_EQ(cmd[0], event[EVENT_COMMAND_COMPLETE_OPCODE_LSBYTE]);
  EXPECT_EQ(cmd[1], event[EVENT_COMMAND_COMPLETE_OPCODE_LSBYTE + 1]);
  EXPECT_EQ(HCI_STATUS_SUCCESS, event[EVENT_COMMAND_COMPLETE_STATUS_BYTE]);
}

// Send the command to read the controller's buffer sizes.
void BluetoothTransportTest::setBufferSizes() {
  hidl_vec<uint8_t> cmd = COMMAND_HCI_READ_BUFFER_SIZE;
  std::cout << __func__ << " ";
  ALOGI("%s", __func__);
  sendHCICommand(cmd);
  std::cout << std::endl;

  wait_for_event(true);
  if (event_queue.size() == 0) return;

  hidl_vec<uint8_t> event;
  event = event_queue.front();
  event_queue.pop();

  EXPECT_EQ(EVENT_COMMAND_COMPLETE, event[EVENT_CODE_BYTE]);
  EXPECT_EQ(cmd[0], event[EVENT_COMMAND_COMPLETE_OPCODE_LSBYTE]);
  EXPECT_EQ(cmd[1], event[EVENT_COMMAND_COMPLETE_OPCODE_LSBYTE + 1]);
  EXPECT_EQ(HCI_STATUS_SUCCESS, event[EVENT_COMMAND_COMPLETE_STATUS_BYTE]);

  max_acl_data_packet_length =
      event[EVENT_COMMAND_COMPLETE_STATUS_BYTE + 1] +
      (event[EVENT_COMMAND_COMPLETE_STATUS_BYTE + 2] << 8);
  max_sco_data_packet_length = event[EVENT_COMMAND_COMPLETE_STATUS_BYTE + 3];
  max_acl_data_packets = event[EVENT_COMMAND_COMPLETE_STATUS_BYTE + 4] +
                         (event[EVENT_COMMAND_COMPLETE_STATUS_BYTE + 5] << 8);
  max_sco_data_packets = event[EVENT_COMMAND_COMPLETE_STATUS_BYTE + 6] +
                         (event[EVENT_COMMAND_COMPLETE_STATUS_BYTE + 7] << 8);

  ALOGI("%s: ACL max %d num %d SCO max %d num %d", __func__,
        static_cast<int>(max_acl_data_packet_length),
        static_cast<int>(max_acl_data_packets),
        static_cast<int>(max_sco_data_packet_length),
        static_cast<int>(max_sco_data_packets));
}

void BluetoothTransportTest::sendHCICommand(hidl_vec<uint8_t> cmd) {
  if (logCommands) {
    std::unique_lock<std::mutex> guard(log_mutex);
    std::ostringstream os;
    os << " Command Sent. Length: 0x" << cmd.size() << "  ";
    for(int i = 0; (i < MAX_CMD_LOG) && (i < cmd.size()); i++)
      os << " 0x" << HEX(cmd[i]);
    os << std::endl;
    if (consoleLog)
      std::cout << os.str();
    ALOGI("%s: %s", __func__, os.str().c_str());
  }
  bluetooth->sendHciCommand(cmd).isOk();
}

// Send an HCI command (in Loopback mode) and check the response.
void BluetoothTransportTest::sendAndCheckHCI(int num_packets) {
  ThroughputLogger logger = {__func__};
  int command_size = 0;
  for (int n = 0; n < num_packets; n++) {
    // Send an HCI packet
    std::vector<uint8_t> write_name = COMMAND_HCI_WRITE_LOCAL_NAME;
    // With a name
    char new_name[] = "John Jacob Jingleheimer Schmidt ___________________0";
    size_t new_name_length = strlen(new_name);
    for (size_t i = 0; i < new_name_length; i++)
      write_name.push_back(static_cast<uint8_t>(new_name[i]));
    // And the packet number
    size_t i = new_name_length - 1;
    for (int digits = n; digits > 0; digits = digits / 10, i--)
      write_name[i] = static_cast<uint8_t>('0' + digits % 10);
    // And padding
    for (size_t i = 0; i < 248 - new_name_length; i++)
      write_name.push_back(static_cast<uint8_t>(0));

    hidl_vec<uint8_t> cmd = write_name;
    std::cout << __func__ << ":";
    sendHCICommand(cmd);

    // Check the loopback of the HCI packet
    wait_for_event();
    if (event_queue.size() == 0) return;

    hidl_vec<uint8_t> event = event_queue.front();
    event_queue.pop();
    size_t compare_length =
        (cmd.size() > static_cast<size_t>(0xff) ? static_cast<size_t>(0xff)
                                                : cmd.size());
    EXPECT_GT(event.size(), compare_length + EVENT_FIRST_PAYLOAD_BYTE - 1);

    EXPECT_EQ(EVENT_LOOPBACK_COMMAND, event[EVENT_CODE_BYTE]);
    EXPECT_EQ(compare_length, event[EVENT_LENGTH_BYTE]);

    // Don't compare past the end of the event.
    if (compare_length + EVENT_FIRST_PAYLOAD_BYTE > event.size()) {
      compare_length = event.size() - EVENT_FIRST_PAYLOAD_BYTE;
      ALOGE("Only comparing %d bytes", static_cast<int>(compare_length));
    }

    if (n == num_packets - 1) {
      command_size = cmd.size();
    }

    for (size_t i = 0; i < compare_length; i++)
      EXPECT_EQ(cmd[i], event[EVENT_FIRST_PAYLOAD_BYTE + i]);
  }
  logger.setTotalBytes(command_size * num_packets * 2);
}

// Send a SCO data packet (in Loopback mode) and check the response.
void BluetoothTransportTest::sendAndCheckSCO(int num_packets, size_t size,
                                        uint16_t handle) {
  ThroughputLogger logger = {__func__};
  for (int n = 0; n < num_packets; n++) {
    // Send a SCO packet
    hidl_vec<uint8_t> sco_packet;
    std::vector<uint8_t> sco_vector;
    sco_vector.push_back(static_cast<uint8_t>(handle & 0xff));
    sco_vector.push_back(static_cast<uint8_t>((handle & 0x0f00) >> 8));
    sco_vector.push_back(static_cast<uint8_t>(size & 0xff));
    sco_vector.push_back(static_cast<uint8_t>((size & 0xff00) >> 8));
    for (size_t i = 0; i < size; i++) {
      sco_vector.push_back(static_cast<uint8_t>(i + n));
    }
    sco_packet = sco_vector;
    if (logData) {
      std::unique_lock<std::mutex> guard(log_mutex);
      std::cout << __func__ << ": Tx: " << sco_vector.size() << std::endl;
      for(int i = 0; i < sco_vector.size(); i++)
        std::cout << " 0x" << HEX(sco_vector[i]);
      std::cout << std::endl;
    }
    bluetooth->sendScoData(sco_vector);

    // Check the loopback of the SCO packet
    EXPECT_TRUE(bluetooth_cb->WaitForCallback(kCallbackNameScoEventReceived)
                    .no_timeout);
    hidl_vec<uint8_t> sco_loopback = sco_queue.front();
    sco_queue.pop();

    EXPECT_EQ(sco_packet.size(), sco_loopback.size());
    size_t successful_bytes = 0;

    for (size_t i = 0; i < sco_packet.size(); i++) {
      if (sco_packet[i] == sco_loopback[i]) {
        successful_bytes = i;
      } else {
        ALOGE("Miscompare at %d (expected %x, got %x)", static_cast<int>(i),
              sco_packet[i], sco_loopback[i]);
        ALOGE("At %d (expected %x, got %x)", static_cast<int>(i + 1),
              sco_packet[i + 1], sco_loopback[i + 1]);
        break;
      }
    }
    EXPECT_EQ(sco_packet.size(), successful_bytes + 1);
  }
  logger.setTotalBytes(num_packets * size * 2);
}

// Send an ACL data packet (in Loopback mode) and check the response.
void BluetoothTransportTest::sendAndCheckACL(int num_packets, size_t size,
                                        uint16_t handle) {
  static int count = 1;
  ThroughputLogger logger = {__func__};
  for (int n = 0; n < num_packets; n++) {
    std::cout << __func__ << ": Data# " << count << std::endl;
    count++;
    // Send an ACL packet
    hidl_vec<uint8_t> acl_packet;
    std::vector<uint8_t> acl_vector;
    acl_vector.push_back(static_cast<uint8_t>(handle & 0xff));
    acl_vector.push_back(static_cast<uint8_t>((handle & 0x0f00) >> 8) |
                         ACL_BROADCAST_POINT_TO_POINT |
                         ACL_PACKET_BOUNDARY_FIRST_AUTO_FLUSHABLE);
    acl_vector.push_back(static_cast<uint8_t>(size & 0xff));
    acl_vector.push_back(static_cast<uint8_t>((size & 0xff00) >> 8));
    for (size_t i = 0; i < size; i++) {
      acl_vector.push_back(static_cast<uint8_t>(i + n));
    }
    acl_packet = acl_vector;
    if (logData) {
      std::unique_lock<std::mutex> guard(log_mutex);
      std::cout << __func__ << ": Tx: " << acl_vector.size() << std::endl;
      for(int i = 0; i < acl_vector.size(); i++)
        std::cout << " 0x" << HEX(acl_vector[i]);
      std::cout << std::endl;
    }
    bluetooth->sendAclData(acl_vector);

    // Check the loopback of the ACL packet
    EXPECT_TRUE(bluetooth_cb->WaitForCallback(kCallbackNameAclEventReceived)
                    .no_timeout);
    hidl_vec<uint8_t> acl_loopback = acl_queue.front();
    acl_queue.pop();

    EXPECT_EQ(acl_packet.size(), acl_loopback.size());
    size_t successful_bytes = 0;

    for (size_t i = 0; i < acl_packet.size(); i++) {
      if (acl_packet[i] == acl_loopback[i]) {
        successful_bytes = i;
      } else {
        ALOGE("Miscompare at %d (expected %x, got %x)", static_cast<int>(i),
              acl_packet[i], acl_loopback[i]);
        ALOGE("At %d (expected %x, got %x)", static_cast<int>(i + 1),
              acl_packet[i + 1], acl_loopback[i + 1]);
        break;
      }
    }
    EXPECT_EQ(acl_packet.size(), successful_bytes + 1);
  }
  logger.setTotalBytes(num_packets * size * 2);
}

// Return the number of completed packets reported by the controller.
int BluetoothTransportTest::wait_for_completed_packets_event(uint16_t handle) {
  int packets_processed = 0;
  wait_for_event(false);
  if (event_queue.size() == 0) {
    ALOGI("%s: WaitForCallback timed out.", __func__);
    return packets_processed;
  }
  while (event_queue.size() > 0) {
    hidl_vec<uint8_t> event = event_queue.front();
    event_queue.pop();

    EXPECT_EQ(EVENT_NUMBER_OF_COMPLETED_PACKETS, event[EVENT_CODE_BYTE]);
    EXPECT_EQ(1, event[EVENT_NUMBER_OF_COMPLETED_PACKETS_NUM_HANDLES]);

    uint16_t event_handle = event[3] + (event[4] << 8);
    EXPECT_EQ(handle, event_handle);

    packets_processed += event[5] + (event[6] << 8);
  }
  return packets_processed;
}

// Send local loopback command and initialize SCO and ACL handles.
void BluetoothTransportTest::enterLoopbackMode(std::vector<uint16_t>& sco_handles,
                                          std::vector<uint16_t>& acl_handles) {
  hidl_vec<uint8_t> cmd = COMMAND_HCI_WRITE_LOOPBACK_MODE_LOCAL;
  std::cout << __func__ << " ";
  ALOGI("%s", __func__);
  sendHCICommand(cmd);
  std::cout << std::endl;

  // Receive connection complete events with data channels
  int connection_event_count = 0;
  bool command_complete_received = false;
  while (true) {
    wait_for_event(false);
    if (event_queue.size() == 0) {
      // Fail if there was no event received or no connections completed.
      EXPECT_TRUE(command_complete_received);
      EXPECT_LT(0, connection_event_count);
      return;
    }
    hidl_vec<uint8_t> event = event_queue.front();
    event_queue.pop();
    EXPECT_GT(event.size(),
              static_cast<size_t>(EVENT_COMMAND_COMPLETE_STATUS_BYTE));
    if (event[EVENT_CODE_BYTE] == EVENT_CONNECTION_COMPLETE) {
      EXPECT_GT(event.size(),
                static_cast<size_t>(EVENT_CONNECTION_COMPLETE_TYPE));
      EXPECT_EQ(event[EVENT_LENGTH_BYTE],
                EVENT_CONNECTION_COMPLETE_PARAM_LENGTH);
      uint8_t connection_type = event[EVENT_CONNECTION_COMPLETE_TYPE];

      EXPECT_TRUE(connection_type == EVENT_CONNECTION_COMPLETE_TYPE_SCO ||
                  connection_type == EVENT_CONNECTION_COMPLETE_TYPE_ACL);

      // Save handles
      uint16_t handle = event[EVENT_CONNECTION_COMPLETE_HANDLE_LSBYTE] |
                        event[EVENT_CONNECTION_COMPLETE_HANDLE_LSBYTE + 1] << 8;
      if (connection_type == EVENT_CONNECTION_COMPLETE_TYPE_SCO)
        sco_handles.push_back(handle);
      else
        acl_handles.push_back(handle);

      ALOGI("Connect complete type = %d handle = %d",
            event[EVENT_CONNECTION_COMPLETE_TYPE], handle);
      connection_event_count++;
    } else {
      EXPECT_EQ(EVENT_COMMAND_COMPLETE, event[EVENT_CODE_BYTE]);
      EXPECT_EQ(cmd[0], event[EVENT_COMMAND_COMPLETE_OPCODE_LSBYTE]);
      EXPECT_EQ(cmd[1], event[EVENT_COMMAND_COMPLETE_OPCODE_LSBYTE + 1]);
      EXPECT_EQ(HCI_STATUS_SUCCESS, event[EVENT_COMMAND_COMPLETE_STATUS_BYTE]);
      command_complete_received = true;
    }
  }
}

void BluetoothTransportTest::SleepTimerExpiry(union sigval sig) {
  BluetoothTransportTest *btTransportTest = static_cast<BluetoothTransportTest*>(sig.sival_ptr);
  ALOGI("%s", __func__);
  if (btTransportTest) {
    {
      std::unique_lock<std::mutex> lk(BluetoothTransportTest::cv_m);
      timerExpired = true;
    }
    // signal the condition variable
    cv.notify_one();
  }
}

void BluetoothTransportTest::StartSleepTimer(int sleepTime) {
  int status;
  struct itimerspec ts;
  struct sigevent se;
  uint32_t timeout_ms;
  timerExpired = false;

  ALOGV("%s", __func__);

  if (sleep_timer_state_ == TIMER_NOT_CREATED) {
    se.sigev_notify_function = (void (*)(union sigval))SleepTimerExpiry;
    se.sigev_notify = SIGEV_THREAD;
    se.sigev_value.sival_ptr = this;
    se.sigev_notify_attributes = NULL;

    status = timer_create(CLOCK_BOOTTIME_ALARM, &se, &sleep_timer_);
    if (status == 0) {
      ALOGI("%s: Sleep timer created", __func__);
      sleep_timer_state_ = TIMER_CREATED;
    } else {
      ALOGE("%s: Error creating Sleep timer %d\n", __func__, status);
    }
  }

  if (sleep_timer_state_ == TIMER_CREATED || sleep_timer_state_ == TIMER_ACTIVE) {
    timeout_ms = sleepTime;
    ts.it_value.tv_sec = timeout_ms / 1000;
    ts.it_value.tv_nsec = 1000000 * (timeout_ms % 1000);
    ts.it_interval.tv_sec = 0;
    ts.it_interval.tv_nsec = 0;

    status = timer_settime(sleep_timer_, 0, &ts, 0);
    if (status < 0)
      ALOGE("%s:Failed to set Sleep timer: %d", __func__, status);
    else {
      ALOGI("%s: timer started", __func__);
      sleep_timer_state_ = TIMER_ACTIVE;
    }
  }
}

bool BluetoothTransportTest::StopSleepTimer() {
  int status;
  struct itimerspec ts;
  bool was_active = false;

  ALOGI("%s", __func__);

  if (sleep_timer_state_ == TIMER_ACTIVE) {
    ts.it_value.tv_sec = 0;
    ts.it_value.tv_nsec = 0;
    ts.it_interval.tv_sec = 0;
    ts.it_interval.tv_nsec = 0;
    was_active = true;
    status = timer_settime(sleep_timer_, 0, &ts, 0);
    if (status == -1)
      ALOGE("%s:Failed to stop sleep timer", __func__);
    else if (status == 0) {
      ALOGV("%s: Sleep timer Stopped", __func__);
      sleep_timer_state_ = TIMER_CREATED;
    }
  }
  return was_active;
}

// Empty test: Initialize()/Close() are called in SetUp()/TearDown().
TEST_F(BluetoothTransportTest, InitializeAndClose) {
  ALOGI("===========Testcase: InitializationAndClose===============");
  std::cout << "====-=====Testcase: InitializationAndClose============" << std::endl;
}

// Send an HCI Reset with sendHciCommand and wait for a command complete event.
TEST_F(BluetoothTransportTest, HciReset) {
  ALOGI("===================Testcase: HciReset====================");
  std::cout << "====================Testcase: HciReset====================" << std::endl;
  hidl_vec<uint8_t> cmd = COMMAND_HCI_RESET;
  sendHCICommand(cmd);
  wait_for_command_complete_event(cmd);
}

// Read and check the HCI version of the controller.
TEST_F(BluetoothTransportTest, HciVersionTest) {
  ALOGI("====================Testcase: HciVersionTest====================");
  std::cout << "====================Testcase: HciVersionTest====================" << std::endl;
  hidl_vec<uint8_t> cmd = COMMAND_HCI_READ_LOCAL_VERSION_INFORMATION;
  sendHCICommand(cmd);

  wait_for_event();
  if (event_queue.size() == 0) return;

  hidl_vec<uint8_t> event = event_queue.front();
  event_queue.pop();
  EXPECT_GT(event.size(), static_cast<size_t>(EVENT_LOCAL_LMP_VERSION_BYTE));

  EXPECT_EQ(EVENT_COMMAND_COMPLETE, event[EVENT_CODE_BYTE]);
  EXPECT_EQ(cmd[0], event[EVENT_COMMAND_COMPLETE_OPCODE_LSBYTE]);
  EXPECT_EQ(cmd[1], event[EVENT_COMMAND_COMPLETE_OPCODE_LSBYTE + 1]);
  EXPECT_EQ(HCI_STATUS_SUCCESS, event[EVENT_COMMAND_COMPLETE_STATUS_BYTE]);

  EXPECT_LE(HCI_MINIMUM_HCI_VERSION, event[EVENT_LOCAL_HCI_VERSION_BYTE]);
  EXPECT_LE(HCI_MINIMUM_LMP_VERSION, event[EVENT_LOCAL_LMP_VERSION_BYTE]);
}

// Enter loopback mode, but don't send any packets.
TEST_F(BluetoothTransportTest, WriteLoopbackMode) {
  ALOGI("====================Testcase: WriteLoopbackMode====================");
  std::cout << "====================Testcase: WriteLoopbackMode====================" << std::endl;
  std::vector<uint16_t> sco_connection_handles;
  std::vector<uint16_t> acl_connection_handles;
  enterLoopbackMode(sco_connection_handles, acl_connection_handles);
}

TEST_F(BluetoothTransportTest, RandomHostCommands) {
  ALOGI("====================Testcase: RandomHostCommands====================");
  std::cout << "====================Testcase: RandomHostCommands====================" << std::endl;
  int sleepMs = 0;
  int maxMsgs = 0;
  std::chrono::steady_clock::time_point start_time_ = std::chrono::steady_clock::now();
  std::chrono::steady_clock::time_point end_time_;
  std::chrono::duration<double> duration(0.0);
  hidl_vec<uint8_t> cmd = COMMAND_HCI_READ_BUFFER_SIZE;
  int count = 1;

  for(int i = 0; i < maxIterations; i++) {
    ALOGI("RandomHostCommands - Iteration: %d", i + 1);
    sleepMs = rand() % maxAppSleepTimer;
    if (sleepMs <  minAppSleepTimer)
      sleepMs = minAppSleepTimer;
    {
      StartSleepTimer(sleepMs);
      std::unique_lock<std::mutex> lk(BluetoothTransportTest::cv_m);
      BluetoothTransportTest::cv.wait(lk);
      ALOGI("RandomHostCommands: SleepTimer Expired");
    }
    end_time_ = std::chrono::steady_clock::now();
    duration = end_time_ - start_time_;
    start_time_ = end_time_;
    maxMsgs = rand() % maxMsgsPerIteration;
    if (maxMsgs < 1) maxMsgs = 1;
    std::cout << "Iteration:" << (i + 1) << " Duration:"
              << duration.count() << " SleepMs: " << sleepMs << " Msgs: "
              << maxMsgs << std::endl;
    ALOGI("RandomHostCommands: Iteration:%d, sleepMs: %d, Msgs:%d, Duration:%f",
          (i + 1), sleepMs, maxMsgs, duration.count());
    for(int j = 0; j < maxMsgs; j++) {
      std::cout << "HostCommand# " << count << std::endl;
      count++;
      sendHCICommand(cmd);
      wait_for_command_complete_event(cmd);
    }
  }
}

TEST_F(BluetoothTransportTest, RandomHostData) {
  ALOGI("====================Testcase: RandomHostData====================");
  std::cout << "====================Testcase: RandomHostData====================" << std::endl;
  setBufferSizes();
  std::vector<uint16_t> acl_connection_handles;
  std::vector<uint16_t> sco_connection_handles;
  enterLoopbackMode(sco_connection_handles, acl_connection_handles);

  int sleepMs = 0;
  int maxMsgs = 0;
  sendAndCheckHCI(1);
  std::chrono::steady_clock::time_point start_time_ = std::chrono::steady_clock::now();
  std::chrono::steady_clock::time_point end_time_;
  std::chrono::duration<double> duration;

  for(int i = 0; i < maxIterations; i++) {
    ALOGI("RandomHostData - Iteration: %d", i + 1);
    sleepMs = rand() % maxAppSleepTimer;
    if (sleepMs <  minAppSleepTimer)
      sleepMs = minAppSleepTimer;
    {
      StartSleepTimer(sleepMs);
      std::unique_lock<std::mutex> lk(BluetoothTransportTest::cv_m);
      BluetoothTransportTest::cv.wait(lk);
      ALOGI("RandomHostData: SleepTimer Expired");
    }
    end_time_ = std::chrono::steady_clock::now();
    duration = end_time_ - start_time_;
    start_time_ = end_time_;
    if (acl_connection_handles.size() > 0) {
      maxMsgs = rand() % maxMsgsPerIteration;
      // ensure atleast one message is sent
      if (maxMsgs == 0) maxMsgs = 1;
      std::cout << "Iteration:" << (i + 1) << " Duration:"
                << duration.count() << " SleepMs: " << sleepMs << " Msgs: "
                << maxMsgs << std::endl;
      ALOGI("RandomHostCommands: Iteration:%d, sleepMs: %d, Msgs:%d, Duration:%f",
          (i + 1), sleepMs, maxMsgs, duration.count());
      EXPECT_LT(0, max_acl_data_packet_length);
      sendAndCheckACL(maxMsgs, max_acl_data_packet_length, acl_connection_handles[0]);
      int acl_packets_sent = maxMsgs;
      int completed_packets =
        wait_for_completed_packets_event(acl_connection_handles[0]);
      if (acl_packets_sent != completed_packets) {
        ALOGI("%s: packets_sent (%d) != completed_packets (%d)", __func__,
            acl_packets_sent, completed_packets);
      }
    }
  }
}

TEST_F(BluetoothTransportTest, BtOnOff) {
  ALOGI("====================Testcase: BtOnOff====================");
  std::cout << "====================Testcase: BtOnOff====================" << std::endl;
  TearDown();
  for(int i = 0; i < maxIterations; i++) {
    ALOGI("%s - Iteration: %d", __func__, i + 1);
    std::cout << "Iteration:" << i << std::endl;
    SetUp();
    TearDown();
  }
  SetUp();
}

TEST_F(BluetoothTransportTest, SsrStackInitiated) {
  ALOGI("====================Testcase: SsrStackInitiated====================");
  std::cout << "====================Testcase: SsrStackInitiated====================" << std::endl;
  for(int i = 0; i < maxIterations; i++) {
    std::cout << "Iteration:" << (i + 1) << std::endl;
    ALOGI("SsrStackInitiated - Iteration: %d", i + 1);
    hidl_vec<uint8_t> cmd = COMMAND_HCI_GET_DEBUG_INFO;
    serviceDiedOk = true;
    sendHCICommand(cmd);
    wait_for_event(false);
    {
      // Acquire Wakelock as system may go to sleep and serviceDied() may not get called
      Wakelock::Acquire();
      StartSleepTimer(WAIT_FOR_SSR);
      std::unique_lock<std::mutex> lk(BluetoothTransportTest::cv_m);
      BluetoothTransportTest::cv.wait(lk);
      ALOGI("SsrStackInitiated: SleepTimer Expired");
      Wakelock::Release();
    }
    serviceDiedOk = false;
    while (event_queue.size())
      event_queue.pop();
    bluetooth->close().isOk();
    SetUp(); // Turn BT On
  }
}

TEST_F(BluetoothTransportTest, RxThreadStuck) {
  ALOGI("====================Testcase: RxThreadStuck====================");
  std::cout << "====================Testcase: RxThreadStuck====================" << std::endl;
  for(int i = 0; i < maxIterations; i++) {
    ALOGI("RxThreaddStuck - Iteration: %d", i + 1);
    std::cout << "Iteration:" << (i + 1) << std::endl;
    hidl_vec<uint8_t> cmd = COMMAND_HCI_READ_BUFFER_SIZE;
    serviceDiedOk = true;
    sendHCICommand(cmd);
    wait_for_event(false);
    {
      StartSleepTimer(WAIT_FOR_SSR);
      std::unique_lock<std::mutex> lk(BluetoothTransportTest::cv_m);
      BluetoothTransportTest::cv.wait(lk);
      ALOGI("RxThreadStuck: SleepTimer Expired");
    }
    serviceDiedOk = false;
    simulateRxThreadStuck = false;
    while (event_queue.size())
      event_queue.pop();
    bluetooth->close().isOk();
    SetUp(); // Turn BT On
  }
}

// Send an unknown HCI command and wait for the error message.
TEST_F(BluetoothTransportTest, HciUnknownCommand) {
  ALOGI("====================Testcase: HciUnknownCommand====================");
  std::cout << "====================Testcase: HciUnknownCommand====================" << std::endl;
  hidl_vec<uint8_t> cmd = COMMAND_HCI_SHOULD_BE_UNKNOWN;
  sendHCICommand(cmd);

  wait_for_event();
  if (event_queue.size() == 0) return;

  hidl_vec<uint8_t> event = event_queue.front();
  event_queue.pop();

  EXPECT_GT(event.size(),
            static_cast<size_t>(EVENT_COMMAND_COMPLETE_STATUS_BYTE));
  if (event[EVENT_CODE_BYTE] == EVENT_COMMAND_COMPLETE) {
    EXPECT_EQ(cmd[0], event[EVENT_COMMAND_COMPLETE_OPCODE_LSBYTE]);
    EXPECT_EQ(cmd[1], event[EVENT_COMMAND_COMPLETE_OPCODE_LSBYTE + 1]);
    EXPECT_EQ(HCI_STATUS_UNKNOWN_HCI_COMMAND,
              event[EVENT_COMMAND_COMPLETE_STATUS_BYTE]);
  } else {
    EXPECT_EQ(EVENT_COMMAND_STATUS, event[EVENT_CODE_BYTE]);
    EXPECT_EQ(cmd[0], event[EVENT_COMMAND_STATUS_OPCODE_LSBYTE]);
    EXPECT_EQ(cmd[1], event[EVENT_COMMAND_STATUS_OPCODE_LSBYTE + 1]);
    EXPECT_EQ(HCI_STATUS_UNKNOWN_HCI_COMMAND,
              event[EVENT_COMMAND_STATUS_STATUS_BYTE]);
  }
}

TEST_F(BluetoothTransportTest, SsrSwErrFatal) {
  ALOGI("====================Testcase: SsrSwErrFatal====================");
  std::cout << "====================Testcase: SsrSwErrFatal====================" << std::endl;
  for(int i = 0; i < maxIterations; i++) {
    std::cout << "Iteration:" << (i + 1) << std::endl;
    ALOGI("SsrSwErrFatal: Iteration:%d", (i + 1));
    hidl_vec<uint8_t> cmd = SSR_SW_ERROR_FATAL;
    serviceDiedOk = true;
    sendHCICommand(cmd);
    wait_for_event(false);
    {
      // Acquire Wakelock as system may go to sleep and serviceDied() may not get called
      Wakelock::Acquire();
      StartSleepTimer(WAIT_FOR_SSR);
      std::unique_lock<std::mutex> lk(BluetoothTransportTest::cv_m);
      BluetoothTransportTest::cv.wait(lk);
      ALOGI("SsrSwErrFatal: SleepTimer Expired");
      Wakelock::Release();
    }
    serviceDiedOk = false;
    while (event_queue.size())
      event_queue.pop();
    bluetooth->close().isOk();
    SetUp(); // Turn BT On
  }
}

TEST_F(BluetoothTransportTest, SsrSwExceptionDivBy0) {
  ALOGI("====================Testcase: SsrSwExceptionDivBy0====================");
  std::cout << "====================Testcase: SsrSwExcpetionDivBy0====================" << std::endl;
  for(int i = 0; i < maxIterations; i++) {
    std::cout << "Iteration:" << (i + 1) << std::endl;
    ALOGI("SsrSwExceptionDivBy0: Iteration:%d", (i + 1));
    hidl_vec<uint8_t> cmd = SSR_SW_EXCEPTION_DIV_BY_0;
    serviceDiedOk = true;
    sendHCICommand(cmd);
    wait_for_event(false);
    {
      // Acquire Wakelock as system may go to sleep and serviceDied() may not get called
      Wakelock::Acquire();
      StartSleepTimer(WAIT_FOR_SSR);
      std::unique_lock<std::mutex> lk(BluetoothTransportTest::cv_m);
      BluetoothTransportTest::cv.wait(lk);
      ALOGI("SsrSwExceptionDivBy0: SleepTimer Expired");
      Wakelock::Release();
    }
    serviceDiedOk = false;
    while (event_queue.size())
      event_queue.pop();
    bluetooth->close().isOk();
    SetUp(); // Turn BT On
  }
}

TEST_F(BluetoothTransportTest, SsrSwExceptionNullPtr) {
  ALOGI("====================Testcase: SsrSwExceptionNullPtr====================");
  std::cout << "====================Testcase: SsrSwExceptionNullPtr====================" << std::endl;
  for(int i = 0; i < maxIterations; i++) {
    std::cout << "Iteration:" << (i + 1) << std::endl;
    ALOGI("SsrSwExceptionNullPtr: Iteration:%d", (i + 1));
    hidl_vec<uint8_t> cmd = SSR_SW_EXCEPTION_NULL_PTR;
    serviceDiedOk = true;
    sendHCICommand(cmd);
    wait_for_event(false);
    {
      // Acquire Wakelock as system may go to sleep and serviceDied() may not get called
      Wakelock::Acquire();
      StartSleepTimer(WAIT_FOR_SSR);
      std::unique_lock<std::mutex> lk(BluetoothTransportTest::cv_m);
      BluetoothTransportTest::cv.wait(lk);
      ALOGI("SsrSwExceptionNullPtr: SleepTimer Expired");
      Wakelock::Release();
    }
    serviceDiedOk = false;
    while (event_queue.size())
      event_queue.pop();
    bluetooth->close().isOk();
    SetUp(); // Turn BT On
  }
}

TEST_F(BluetoothTransportTest, SsrWatchDogBits) {
  ALOGI("====================Testcase: SsrWatchDogBits====================");
  std::cout << "====================Testcase: SsrWatchDogBits====================" << std::endl;
  for(int i = 0; i < maxIterations; i++) {
    std::cout << "Iteration:" << (i + 1) << std::endl;
    ALOGI("SsrWatchDogBits: Iteration:%d", (i + 1));
    hidl_vec<uint8_t> cmd = SSR_WATCHDOG_BITS;
    serviceDiedOk = true;
    sendHCICommand(cmd);
    wait_for_event(false);
    {
      // Acquire Wakelock as system may go to sleep and serviceDied() may not get called
      Wakelock::Acquire();
      StartSleepTimer(WAIT_FOR_WDOG_SSR);
      std::unique_lock<std::mutex> lk(BluetoothTransportTest::cv_m);
      BluetoothTransportTest::cv.wait(lk);
      ALOGI("SsrWatchDogBits: SleepTimer Expired");
      Wakelock::Release();
    }
    serviceDiedOk = false;
    while (event_queue.size())
      event_queue.pop();
    bluetooth->close().isOk();
    SetUp(); // Turn BT On
  }
}

int main(int argc, char** argv) {
  ::testing::AddGlobalTestEnvironment(BluetoothHidlEnvironment::Instance());
  ::testing::InitGoogleTest(&argc, argv);
  BluetoothHidlEnvironment::Instance()->init(&argc, argv);

  for (int i = 1; i < argc; i++) {
    if (argv[i] == NULL) continue;

    if (string(argv[i]).find(kMaxAppSleepTimer) == 0) {
      string value = string(argv[i]).substr(kMaxAppSleepTimer.length());
      maxAppSleepTimer = std::atoi(value.c_str());
    } else if (string(argv[i]).find(kMinAppSleepTimer) == 0) {
      string value = string(argv[i]).substr(kMinAppSleepTimer.length());
      minAppSleepTimer = std::atoi(value.c_str());
    } else if (string(argv[i]).find(kMaxIterations) == 0) {
      string value = string(argv[i]).substr(kMaxIterations.length());
      maxIterations = std::atoi(value.c_str());
    } else if (string(argv[i]).find(kMaxMsgsPerIteration) == 0) {
      string value = string(argv[i]).substr(kMaxMsgsPerIteration.length());
      maxMsgsPerIteration = std::atoi(value.c_str());
    } else if (string(argv[i]).find(kLogEvents) == 0) {
      logEvents = true;
    } else if (string(argv[i]).find(kConsoleLog) == 0) {
      consoleLog = true;
    } else if (string(argv[i]).find(kLogCommands) == 0) {
      logCommands = true;
    } else if (string(argv[i]).find(kLogData) == 0) {
      logData = true;
    }

    // Shift the remainder of the argv list left by one.
    for (int j = i; j != argc; j++) {
      argv[j] = argv[j + 1];
    }

    // Decrements the argument count.
    argc--;

    // We also need to decrement the iterator as we just removed an element.
    i--;
  }
  std::cout << "Max time:" << maxAppSleepTimer << " Min Time:" << minAppSleepTimer
            << " Iterations:" << maxIterations << " MaxMsgsPerIteration:"
            << maxMsgsPerIteration << " logEvents:" << logEvents
            << " logCommands:" << logCommands << " logData:" << logData
            << " consoleLog:" << consoleLog << std::endl;
  int status = RUN_ALL_TESTS();
  ALOGI("Test result = %d", status);
  return status;
}
