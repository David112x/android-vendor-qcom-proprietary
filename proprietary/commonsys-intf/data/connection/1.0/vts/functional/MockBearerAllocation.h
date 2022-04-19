/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <vendor/qti/hardware/data/connection/1.0/IDataConnection.h>
#include <VtsHalHidlTargetCallbackBase.h>
#include <log/log.h>

using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hardware::hidl_vec;

using ::vendor::qti::hardware::data::connection::V1_0::IDataConnectionResponse;
using ::vendor::qti::hardware::data::connection::V1_0::IDataConnectionIndication;
using ::vendor::qti::hardware::data::connection::V1_0::AllocatedBearers;
using ::vendor::qti::hardware::data::connection::V1_0::BearerInfo;
using ::vendor::qti::hardware::data::connection::V1_0::ErrorReason;

class BearerAllocationArg {
public:
    ErrorReason error;
    AllocatedBearers bearers;
    std::vector<AllocatedBearers> bearersList;
};

class MockBearerAllocationCallback : public ::testing::VtsHalHidlTargetCallbackBase<BearerAllocationArg> {
protected:
    void printBearers(const AllocatedBearers& bearers) {
        ALOGI("AllocatedBearers for cid=%d apn=%s", bearers.cid, std::string(bearers.apn).c_str());
        for (BearerInfo bearer : bearers.bearers) {
            ALOGI("Bearer: id=%d ul=%d dl=%d", bearer.bearerId, bearer.uplink, bearer.downlink);
        }
    }
};

class MockBearerAllocationResponse : public IDataConnectionResponse,
                                     public MockBearerAllocationCallback {
public:
    MockBearerAllocationResponse() {}
    virtual ~MockBearerAllocationResponse() {}

    Return<void> onBearerAllocationResponse(ErrorReason error, const AllocatedBearers& bearers) override {
        ALOGI("onBearerAllocationResponse arrived error=%d", error);
        printBearers(bearers);
        BearerAllocationArg args;
        NotifyFromCallback("onBearerAllocationResponse", args);
        return Void();
    }

    Return<void> onAllBearerAllocationsResponse(ErrorReason error, const hidl_vec<AllocatedBearers>& bearersList) override {
        ALOGI("onAllBearerAllocationsResponse arrived error=%d bearerSize=%d", error, (uint32_t)bearersList.size());
        for (AllocatedBearers bearers : bearersList) {
            printBearers(bearers);
        }
        BearerAllocationArg args;
        NotifyFromCallback("onAllBearerAllocationsResponse", args);
        return Void();
    }

};

class MockBearerAllocationIndication : public IDataConnectionIndication,
                                       public MockBearerAllocationCallback {
public:
    MockBearerAllocationIndication() {}
    virtual ~MockBearerAllocationIndication() {}

    Return<void> onBearerAllocationUpdate(const hidl_vec<AllocatedBearers>& bearersList) override {
        ALOGI("onBearerAllocationUpdate bearerSize=%d", (uint32_t)bearersList.size());
        for (AllocatedBearers bearers : bearersList) {
            printBearers(bearers);
        }
        BearerAllocationArg args;
        NotifyFromCallback("onBearerAllocationUpdate", args);
        return Void();
    }
};