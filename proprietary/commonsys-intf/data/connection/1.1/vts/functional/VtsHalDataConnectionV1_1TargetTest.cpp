/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <log/log.h>
#include <VtsHalHidlTargetTestBase.h>
#include <vendor/qti/hardware/data/connection/1.1/IDataConnection.h>

using ::android::hardware::hidl_string;
using ::android::hardware::Return;
using ::android::sp;

using ::vendor::qti::hardware::data::connection::V1_1::IDataConnection;

class DataConnectionTest : public ::testing::VtsHalHidlTargetTestBase {
public:
    virtual void SetUp() override {
        mConnection = IDataConnection::tryGetService("slot1");
        mConnection2 = IDataConnection::tryGetService("slot2");
        ASSERT_NE(nullptr, mConnection.get()) << "Could not get DataConnection instance slot1";
    }

    virtual void TearDown() override {
    }

    sp<IDataConnection> mConnection;
    sp<IDataConnection> mConnection2;
};

TEST_F(DataConnectionTest, TestGetXlatPropertyValue1) {
    ASSERT_NE(nullptr, mConnection.get()) << "Could not get DataConnection instance";
    hidl_string xlatRequired = "persist.vendor.net.doxlat";
    hidl_string result = "";
    mConnection->getConfig(xlatRequired, "", [&](const hidl_string& result){
            ALOGI("Config Value %s", result.c_str());
    });
    ASSERT_NE("", result.c_str());
}

TEST_F(DataConnectionTest, TestGetXlatPropertyValue2) {
    ASSERT_NE(nullptr, mConnection2.get()) << "Could not get DataConnection instance";
    hidl_string xlatRequired = "persist.vendor.net.doxlat";
    hidl_string result = "";
    mConnection2->getConfig(xlatRequired, "", [&](const hidl_string& result){
            ALOGI("Config Value %s", result.c_str());
    });
    ASSERT_NE("", result.c_str());
}

int main(int argc, char** argv) {
    ALOGI("Enter DataConnection VTS test suite");
    ::testing::InitGoogleTest(&argc, argv);
    int status = RUN_ALL_TESTS();
    ALOGI("Test result with status = %d", status);
    return status;
}
