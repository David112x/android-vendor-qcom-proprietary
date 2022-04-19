/*
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

#define LOG_TAG "btsar_hidl_hal_test"
#include <android-base/logging.h>
#include <utils/Log.h>

#include <vendor/qti/hardware/bluetooth_sar/1.0/IBluetoothSar.h>
#include <android/hardware/bluetooth/1.0/IBluetoothHci.h>
#include <android/hardware/bluetooth/1.0/IBluetoothHciCallbacks.h>
#include <android/hardware/bluetooth/1.0/types.h>
#include <hardware/bluetooth.h>

#include <VtsHalHidlTargetCallbackBase.h>
#include <VtsHalHidlTargetTestBase.h>
#include <VtsHalHidlTargetTestEnvBase.h>
#include <queue>
#include <unistd.h>
#include <cutils/properties.h>
#include <hwbinder/IPCThreadState.h>

using android::hardware::IPCThreadState;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;
using ::android::hardware::bluetooth::V1_0::Status;

using ::android::hardware::bluetooth::V1_0::IBluetoothHci;
using ::android::hardware::bluetooth::V1_0::IBluetoothHciCallbacks;
using vendor::qti::hardware::bluetooth_sar::V1_0::IBluetoothSar;
 /*
 * Set "persist.vendor.service.bdroid.ssrlvl" property to 3 before executing
 * VTS test cases.
 */


 /*
 * Compilation flag to identify whether to execute test cases with BT turned ON
 * or BT turned OFF from Settings.
 *
 * If the below flag is enabled, run the test cases with BT turned off from Settings.
 *
 * Else if the below line containing the flag is commented out, run the test cases
 * with BT turned ON from Settings
 */
#define ENABLE_VTS_BT_REGISTRATION  TRUE

#define COMMAND_HCI_DEBUG_INFO  { 0x5B, 0xFD, 0x00 }

#define WAIT_FOR_INIT_TIMEOUT std::chrono::milliseconds(5000)
#define WAIT_FOR_HCI_EVENT_TIMEOUT std::chrono::milliseconds(2000)
#define WAIT_FOR_SCO_DATA_TIMEOUT std::chrono::milliseconds(1000)
#define WAIT_FOR_ACL_DATA_TIMEOUT std::chrono::milliseconds(1000)

#define EVENT_CONNECTION_COMPLETE 0x03
#define EVENT_COMMAND_COMPLETE 0x0e
#define EVENT_COMMAND_STATUS 0x0f
#define EVENT_NUMBER_OF_COMPLETED_PACKETS 0x13
#define EVENT_LOOPBACK_COMMAND 0x19

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

constexpr char kCallbackNameAclEventReceived[] = "aclDataReceived";
constexpr char kCallbackNameHciEventReceived[] = "hciEventReceived";
constexpr char kCallbackNameInitializationComplete[] = "initializationComplete";
constexpr char kCallbackNameScoEventReceived[] = "scoDataReceived";

std::string power_level;

#ifdef ARCH_ARM_32
bool IsLazyHalSupported();
#endif

// Test environment for Bluetooth HIDL HAL.
class BluetoothHidlEnvironment : public ::testing::VtsHalHidlTargetTestEnvBase {
 public:
  // get the test environment singleton
  static BluetoothHidlEnvironment* Instance() {
    static BluetoothHidlEnvironment* instance = new BluetoothHidlEnvironment;
    return instance;
  }

  virtual void registerTestServices() override {
    registerTestService<IBluetoothSar>();
  }

 private:
  BluetoothHidlEnvironment() {}
};

// The main test class for Bluetooth HIDL HAL.
class BluetoothHidlTest : public ::testing::VtsHalHidlTargetTestBase {
 public:
  virtual void SetUp() override {
    // currently test passthrough mode only
    bluetooth_sar =
        ::testing::VtsHalHidlTargetTestBase::getService<IBluetoothSar>(BluetoothHidlEnvironment::Instance()->getServiceName<IBluetoothSar>());
    ASSERT_NE(bluetooth_sar, nullptr);
    ALOGI("%s: getService() for bluetooth SAR ", __func__);
  }

  virtual void TearDown() override {
#ifdef ARCH_ARM_32
    if (IsLazyHalSupported()) {
      ALOGD("%s: decrementing HIDL usage counter", __func__);
      IPCThreadState::self()->flushCommands();
    }
#endif
    bluetooth_sar = nullptr;
  }

  void bluetooth_init();
  void bluetooth_deinit();
  void handle_no_ops();

  // A simple test implementation of BluetoothHciCallbacks.
  class BluetoothHciCallbacks
      : public ::testing::VtsHalHidlTargetCallbackBase<BluetoothHidlTest>,
        public IBluetoothHciCallbacks {
    BluetoothHidlTest& parent_;

   public:
    BluetoothHciCallbacks(BluetoothHidlTest& parent) : parent_(parent){};

    virtual ~BluetoothHciCallbacks() = default;

    Return<void> initializationComplete(Status status) override {
      parent_.initialized = (status == Status::SUCCESS);
      NotifyFromCallback(kCallbackNameInitializationComplete);
      ALOGV("%s (status = %d)", __func__, static_cast<int>(status));
      return Void();
    };

    Return<void> hciEventReceived(
        const ::android::hardware::hidl_vec<uint8_t>& event) override {
      parent_.event_cb_count++;
      parent_.event_queue.push(event);
      NotifyFromCallback(kCallbackNameHciEventReceived);
      ALOGV("Event received (length = %d)", static_cast<int>(event.size()));
      return Void();
    };

    Return<void> aclDataReceived(
        const ::android::hardware::hidl_vec<uint8_t>& data) override {
      parent_.acl_cb_count++;
      parent_.acl_queue.push(data);
      NotifyFromCallback(kCallbackNameAclEventReceived);
      return Void();
    };

    Return<void> scoDataReceived(
        const ::android::hardware::hidl_vec<uint8_t>& data) override {
      parent_.sco_cb_count++;
      parent_.sco_queue.push(data);
      NotifyFromCallback(kCallbackNameScoEventReceived);
      return Void();
    };
  };

  sp<IBluetoothHci> bluetooth;
  sp<BluetoothHciCallbacks> bluetooth_cb;

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

  sp<IBluetoothSar> bluetooth_sar;
};

void BluetoothHidlTest::bluetooth_init() {
  bluetooth =
      ::testing::VtsHalHidlTargetTestBase::getService<IBluetoothHci>();
  ASSERT_NE(bluetooth, nullptr);
  ALOGI("%s: getService() for bluetooth is %s", __func__,
         bluetooth->isRemote() ? "remote" : "local");

  bluetooth_cb = new BluetoothHciCallbacks(*this);
  ASSERT_NE(bluetooth_cb, nullptr);

  max_acl_data_packet_length = 0;
  max_sco_data_packet_length = 0;
  max_acl_data_packets = 0;
  max_sco_data_packets = 0;

  initialized = false;
  event_cb_count = 0;
  acl_cb_count = 0;
  sco_cb_count = 0;

  ASSERT_EQ(initialized, false);
  bluetooth->initialize(bluetooth_cb);

  bluetooth_cb->SetWaitTimeout(kCallbackNameInitializationComplete,
                               WAIT_FOR_INIT_TIMEOUT);
  bluetooth_cb->SetWaitTimeout(kCallbackNameHciEventReceived,
                               WAIT_FOR_HCI_EVENT_TIMEOUT);
  bluetooth_cb->SetWaitTimeout(kCallbackNameAclEventReceived,
                               WAIT_FOR_ACL_DATA_TIMEOUT);
  bluetooth_cb->SetWaitTimeout(kCallbackNameScoEventReceived,
                               WAIT_FOR_SCO_DATA_TIMEOUT);

  EXPECT_TRUE(bluetooth_cb->WaitForCallback(kCallbackNameInitializationComplete)
              .no_timeout);

  ASSERT_EQ(initialized, true);
}

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

void BluetoothHidlTest::bluetooth_deinit() {
  Return<void> ret;
  ret = bluetooth->close();
  if (!ret.isOk()) {
    ALOGE("%s bluetooth HAL server is dead ", __func__);
    initialized = false;
  } else {
#ifdef ARCH_ARM_32
    if (IsLazyHalSupported()) {
       ALOGD("%s: decrementing HIDL usage counter", __func__);
       IPCThreadState::self()->flushCommands();
    }
#endif
  }

  bluetooth = nullptr;
  handle_no_ops();
  EXPECT_EQ(static_cast<size_t>(0), event_queue.size());
  EXPECT_EQ(static_cast<size_t>(0), sco_queue.size());
  EXPECT_EQ(static_cast<size_t>(0), acl_queue.size());
}

// Discard NO-OPs from the event queue.
void BluetoothHidlTest::handle_no_ops() {
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
      return;
    }
  }
}

#ifdef ENABLE_VTS_BT_REGISTRATION

TEST_F(BluetoothHidlTest, BTSarTest) {
  bluetooth_init();
  if(bluetooth_sar)
    bluetooth_sar->setBluetoothTxPowerCap(1);

  bluetooth_sar =
    ::testing::VtsHalHidlTargetTestBase::getService<IBluetoothSar>(BluetoothHidlEnvironment::Instance()->getServiceName<IBluetoothSar>());

  if(bluetooth_sar) {
    bluetooth_sar->setBluetoothTxPowerCap(2);
    bluetooth_sar->setBluetoothTxPowerCap(3);
    bluetooth_sar->setBluetoothTxPowerCap(4);
    bluetooth_sar->setBluetoothTxPowerCap(5);
    bluetooth_sar->setBluetoothTxPowerCap(6);
    sleep(2);
    bluetooth_sar->setBluetoothTxPowerCap(3);
    sleep(10);
    ALOGI("VTS waking up after 10 secs");
    bluetooth_sar->setBluetoothTxPowerCap(10);
    bluetooth_sar->setBluetoothTxPowerCap(15);
  }
  sleep(2);
  bluetooth_deinit();
}

TEST_F(BluetoothHidlTest, BTSarTestOutOfRange) {
  bluetooth_init();
  if(bluetooth_sar) {
    bluetooth_sar->setBluetoothTxPowerCap(90);
    bluetooth_sar->setBluetoothTxPowerCap(120);
  }
  sleep(2);
  bluetooth_deinit();
}

TEST_F(BluetoothHidlTest, BTSarTestNegativePowerLevel) {
  bluetooth_init();
  if(bluetooth_sar) {
    bluetooth_sar->setBluetoothTxPowerCap(-5);
  }
  sleep(2);
  bluetooth_deinit();
}

TEST_F(BluetoothHidlTest, BTSarTestAfterBToff) {
  if(bluetooth_sar) {
    bluetooth_sar->setBluetoothTxPowerCap(1);
    bluetooth_sar->setBluetoothTxPowerCap(2);
  }
}

TEST_F(BluetoothHidlTest, BTSarTestAfterDebugInfo) {
  hidl_vec<uint8_t> cmd = COMMAND_HCI_DEBUG_INFO;
  bluetooth_init();
  bluetooth->sendHciCommand(cmd);
  ALOGI("VTS:: sending debug info cmd");
  bluetooth_sar =
          ::testing::VtsHalHidlTargetTestBase::getService<IBluetoothSar>(BluetoothHidlEnvironment::Instance()->getServiceName<IBluetoothSar>());
  if(bluetooth_sar)
    bluetooth_sar->setBluetoothTxPowerCap(1);
  bluetooth_sar =
          ::testing::VtsHalHidlTargetTestBase::getService<IBluetoothSar>(BluetoothHidlEnvironment::Instance()->getServiceName<IBluetoothSar>());
  if(bluetooth_sar)
    bluetooth_sar->setBluetoothTxPowerCap(2);
  bluetooth_deinit();
}

#else //ENABLE_VTS_BT_REGISTRATION

TEST_F(BluetoothHidlTest, BTSarTestWithPowerLevel) {
  int power_level_value = 0;
  if(!power_level.empty()) {
    power_level_value = std::stoi(power_level);
  }
  ALOGI("power_level_value::= %d", power_level_value);
  if(bluetooth_sar && (power_level_value >= 0)) {
    bluetooth_sar->setBluetoothTxPowerCap(power_level_value);
  }
}

#endif //ENABLE_VTS_BT_REGISTRATION

int main(int argc, char** argv) {
  ::testing::AddGlobalTestEnvironment(BluetoothHidlEnvironment::Instance());
  ::testing::InitGoogleTest(&argc, argv);
  BluetoothHidlEnvironment::Instance()->init(&argc, argv);
  if(argc >=2) {
    power_level = argv[1];
  }
  int status = RUN_ALL_TESTS();
  ALOGI("Test result = %d", status);
  return status;
}
