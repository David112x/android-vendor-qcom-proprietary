/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "MockBearerAllocation.h"

#include <log/log.h>
#include <vector>
#include <VtsHalHidlTargetTestBase.h>
#include <cutils/properties.h>

using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::sp;

using ::vendor::qti::hardware::data::connection::V1_0::IDataConnection;
using ::vendor::qti::hardware::data::connection::V1_0::StatusCode;

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
    int32_t CID_1 = 0;
};

// get bearer allocation request/response
TEST_F(DataConnectionTest, TestGetBearerAllocation) {
    sp<MockBearerAllocationResponse> mResponse = new MockBearerAllocationResponse();
    StatusCode ret = mConnection->getBearerAllocation(CID_1, mResponse);
    ASSERT_EQ(ret, StatusCode::OK);
    auto res = mResponse->WaitForCallback("onBearerAllocationResponse");
    EXPECT_TRUE(res.no_timeout);
}

// register bearer allocation indication
TEST_F(DataConnectionTest, TestEnableBearerAllocationUpdates) {
    sp<MockBearerAllocationIndication> mIndication = new MockBearerAllocationIndication();
    StatusCode ret = mConnection->registerForAllBearerAllocationUpdates(mIndication);
    ASSERT_EQ(ret, StatusCode::OK);
    auto res = mIndication->WaitForCallback("onBearerAllocationUpdate");
    EXPECT_TRUE(res.no_timeout);
}

// get bearer allocation request/response
TEST_F(DataConnectionTest, TestGetAllBearerAllocations) {
    sp<MockBearerAllocationResponse> mResponse = new MockBearerAllocationResponse();
    StatusCode ret = mConnection->getAllBearerAllocations(mResponse);
    ASSERT_EQ(ret, StatusCode::OK);
    auto res = mResponse->WaitForCallback("onAllBearerAllocationsResponse");
    EXPECT_TRUE(res.no_timeout);
}

// multisim: get IDataConnection HAL instance on slot2
TEST_F(DataConnectionTest, TestGetConnection2) {
    ASSERT_NE(nullptr, mConnection2.get()) << "Could not get DataConnection instance slot2";
}

// multisim: get bearer allocation request/response on slot2
TEST_F(DataConnectionTest, TestGetBearerAllocation2) {
    ASSERT_NE(nullptr, mConnection2.get()) << "Could not get DataConnection instance";
    sp<MockBearerAllocationResponse> mResponse = new MockBearerAllocationResponse();
    StatusCode ret = mConnection2->getBearerAllocation(CID_1, mResponse);
    ASSERT_EQ(ret, StatusCode::OK);
    auto res = mResponse->WaitForCallback("onBearerAllocationResponse");
    EXPECT_TRUE(res.no_timeout);
}

int main(int argc, char** argv) {
    ALOGI("Enter DataConnection VTS test suite");
    ::testing::InitGoogleTest(&argc, argv);
    int status = RUN_ALL_TESTS();
    ALOGI("Test result with status = %d", status);
    return status;
}
