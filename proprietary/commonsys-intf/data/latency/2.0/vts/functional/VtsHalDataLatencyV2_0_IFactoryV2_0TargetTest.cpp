/*
 * Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#define LOG_TAG "latency_hidl_test"

#include <log/log.h>
#include <vendor/qti/data/factory/2.0/IFactory.h>
#include <vendor/qti/latency/2.0/ILinkLatencyService.h>
#include <vendor/qti/latency/2.0/IClientToken.h>
#include <VtsHalHidlTargetTestBase.h>

using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;
using ::vendor::qti::data::factory::V2_0::IFactory;
using ::vendor::qti::latency::V2_0::IClientToken;
using ::vendor::qti::latency::V2_0::ILinkLatencyService;
using ::vendor::qti::latency::V2_0::Level;
using ::vendor::qti::latency::V2_0::Radio;
using ::vendor::qti::latency::V2_0::SlotId;
using ::vendor::qti::latency::V2_0::Config;

Config params =
{
    Radio::WWAN,
    SlotId::SLOT_UNSPECIFIED,
    Level::L1,
    Level::L1,
    false
};

class ClientToken : public IClientToken {};

#define L1_CB [this](::vendor::qti::latency::V2_0::StatusCode status, Level eUplink, Level eDownlink) { \
    ASSERT_EQ(::vendor::qti::latency::V2_0::StatusCode::OK, status); \
    ASSERT_EQ(Level::L1, eUplink); \
    ASSERT_EQ(Level::L1, eDownlink); \
}
#define L2_CB [this](::vendor::qti::latency::V2_0::StatusCode status, Level eUplink, Level eDownlink) { \
    ASSERT_EQ(::vendor::qti::latency::V2_0::StatusCode::OK, status); \
    ASSERT_EQ(Level::L2, eUplink); \
    ASSERT_EQ(Level::L2, eDownlink); \
}
#define L3_CB [this](::vendor::qti::latency::V2_0::StatusCode status, Level eUplink, Level eDownlink) { \
    ASSERT_EQ(::vendor::qti::latency::V2_0::StatusCode::OK, status); \
    ASSERT_EQ(Level::L3, eUplink); \
    ASSERT_EQ(Level::L3, eDownlink); \
}
#define L4_CB [this](::vendor::qti::latency::V2_0::StatusCode status, Level eUplink, Level eDownlink) { \
    ASSERT_EQ(::vendor::qti::latency::V2_0::StatusCode::OK, status); \
    ASSERT_EQ(Level::L4, eUplink); \
    ASSERT_EQ(Level::L4, eDownlink); \
}
#define L5_CB [this](::vendor::qti::latency::V2_0::StatusCode status, Level eUplink, Level eDownlink) { \
    ASSERT_EQ(::vendor::qti::latency::V2_0::StatusCode::OK, status); \
    ASSERT_EQ(Level::L5, eUplink); \
    ASSERT_EQ(Level::L5, eDownlink); \
}
#define L6_CB [this](::vendor::qti::latency::V2_0::StatusCode status, Level eUplink, Level eDownlink) { \
    ASSERT_EQ(::vendor::qti::latency::V2_0::StatusCode::OK, status); \
    ASSERT_EQ(Level::L6, eUplink); \
    ASSERT_EQ(Level::L6, eDownlink); \
}

#define INVALID_ARGS_CB [this](::vendor::qti::latency::V2_0::StatusCode status, Level eUplink, Level eDownlink) { \
    ASSERT_EQ(::vendor::qti::latency::V2_0::StatusCode::INVALID_ARGUMENTS, status); \
}

class LatencyHidlTestBase : public ::testing::VtsHalHidlTargetTestBase {
public:
    virtual void SetUp() override {
        setUpFactory();
    }

    void setUpFactory() {
        latencyIFactory = ::testing::VtsHalHidlTargetTestBase::getService<IFactory>();
        ASSERT_NE(nullptr, latencyIFactory.get()) << "Could not get HIDL instance";
    }

    sp<ILinkLatencyService> requestService(const ::vendor::qti::data::factory::V2_0::IFactory::StatusCode
                                                  expected_result) {
        sp<ILinkLatencyService> linkLatencyService;

        auto cb = [&](::vendor::qti::data::factory::V2_0::IFactory::StatusCode status,
                      sp<ILinkLatencyService> service) {
            ASSERT_EQ(expected_result, status);

            linkLatencyService = service;
            ASSERT_NE(nullptr, linkLatencyService.get()) << "Could not get HIDL instance";
        };

        sp<ClientToken> token = new ClientToken();
        const Return<void> ret = latencyIFactory->createILinkLatencyService(token, cb);
        return linkLatencyService;
    }

    sp<IFactory> latencyIFactory;
};

// positive test. Ensure linkLatency can request an ILinkLatencyService and release the same.
TEST_F(LatencyHidlTestBase, TestCreateLinkLatencyService) {
    sp<ILinkLatencyService> service =
        requestService(::vendor::qti::data::factory::V2_0::IFactory::StatusCode::OK);
    ASSERT_NE(nullptr, service.get()) << "Could not get Service instance";
}

class LatencyHidlTestService : public LatencyHidlTestBase {
public:
    virtual void SetUp() override {
        setUpFactory();
        linkLatencyService = requestService(::vendor::qti::data::factory::V2_0::IFactory::StatusCode::OK);
        linkLatencyService2 = requestService(::vendor::qti::data::factory::V2_0::IFactory::StatusCode::OK);
    }

    sp<ILinkLatencyService> linkLatencyService;
    sp<ILinkLatencyService> linkLatencyService2;
};

// positive test. Ensure setLevel updates the effective levels for uplink and dowlink for dds slot.
TEST_F(LatencyHidlTestService, TestSetLevel) {
    params.uplink = Level::L1;
    params.downlink = Level::L1;

    Return<void> ret = linkLatencyService->setLevel(params, L1_CB);
    ASSERT_TRUE(ret.isOk());
}
// positive test. Ensure setLevel updates the effective levels for uplink and dowlink for dds slot.
TEST_F(LatencyHidlTestService, TestSetLevel2) {
    params.uplink = Level::L2;
    params.downlink = Level::L2;

    Return<void> ret = linkLatencyService->setLevel(params, L2_CB);
    ASSERT_TRUE(ret.isOk());
}
// positive test. Ensure setLevel updates the effective levels for uplink and dowlink for slotId 1.
TEST_F(LatencyHidlTestService, TestSetLevel3Slot1) {
    Return<void> ret;
    params.uplink = Level::L3;
    params.downlink = Level::L3;
    params.slotId = SlotId::SLOT_SECOND;

    ret = linkLatencyService->setLevel(params, L3_CB);
    params.slotId = SlotId::SLOT_UNSPECIFIED;
    ASSERT_TRUE(ret.isOk());
    params.uplink = Level::L2;
    params.downlink = Level::L2;
    params.slotId = SlotId::SLOT_SECOND;
    ret = linkLatencyService->setLevel(params, L2_CB);
    params.slotId = SlotId::SLOT_UNSPECIFIED;
    ASSERT_TRUE(ret.isOk());
}

// negative test. Invalid Level returns INVALID_ARGUMENTS
TEST_F(LatencyHidlTestService, TestSetLevelInvalidArgs) {
    params.uplink = (Level) 8;
    params.downlink = Level::L1;

    Return<void> ret = linkLatencyService->setLevel(params, INVALID_ARGS_CB);
    ASSERT_TRUE(ret.isOk());
}

// negative test. Invalid Level returns INVALID_ARGUMENTS
TEST_F(LatencyHidlTestService, TestSetLevelInvalidArgs2) {
    params.uplink = Level::L1;
    params.downlink = (Level) 0;

    Return<void> ret = linkLatencyService->setLevel(params, INVALID_ARGS_CB);
    ASSERT_TRUE(ret.isOk());
}

// negative test. Invalid Level returns INVALID_ARGUMENTS
TEST_F(LatencyHidlTestService, TestSetLevelInvalidArgs3) {
    params.uplink = (Level) 0;
    params.downlink = (Level) 8;

    Return<void> ret = linkLatencyService->setLevel(params, INVALID_ARGS_CB);
    ASSERT_TRUE(ret.isOk());
}

// positive test. Ensure setLevel works on multiple clients
TEST_F(LatencyHidlTestService, TestSetLevel2Clients) {
    Return<void> ret;
    params.uplink = Level::L4;
    params.downlink = Level::L4;

    ret = linkLatencyService->setLevel(params, L4_CB);
    ASSERT_TRUE(ret.isOk());

    ret = linkLatencyService2->setLevel(params, L4_CB);
    ASSERT_TRUE(ret.isOk());
}

// positive test. Ensure setLevel works on multiple clients
TEST_F(LatencyHidlTestService, TestSetLevel2Clients2) {
    Return<void> ret;
    params.uplink = Level::L6;
    params.downlink = Level::L6;

    ret = linkLatencyService->setLevel(params, L6_CB);
    ASSERT_TRUE(ret.isOk());
    params.uplink = Level::L5;
    params.downlink = Level::L5;
    ret = linkLatencyService2->setLevel(params, L6_CB);
    ASSERT_TRUE(ret.isOk());
}



int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    int status = RUN_ALL_TESTS();
    ALOGE("Test result with status=%d", status);
    return status;
}
