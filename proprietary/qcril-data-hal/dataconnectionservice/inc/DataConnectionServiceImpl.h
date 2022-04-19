/*
 * Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "framework/TimeKeeper.h"
#include <vendor/qti/hardware/data/connection/1.1/IDataConnection.h>
#include "MessageCommon.h"

using android::hardware::Return;
using android::hardware::hidl_death_recipient;
using android::hardware::hidl_vec;
using android::hardware::hidl_bitfield;
using ::android::hardware::hidl_string;
using ::android::hardware::Void;
using android::sp;
using android::wp;
using rildata::RatType_t;
using rildata::AllocatedBearer_t;
using rildata::AllocatedBearerResult_t;
using rildata::ResponseError_t;

using vendor::qti::hardware::data::connection::V1_1::IDataConnection;
using vendor::qti::hardware::data::connection::V1_0::IDataConnectionIndication;
using vendor::qti::hardware::data::connection::V1_0::IDataConnectionResponse;
using vendor::qti::hardware::data::connection::V1_0::StatusCode;
using vendor::qti::hardware::data::connection::V1_0::ErrorReason;
using vendor::qti::hardware::data::connection::V1_0::RatType;
using vendor::qti::hardware::data::connection::V1_0::AllocatedBearers;
using vendor::qti::hardware::data::connection::V1_0::BearerInfo;

#ifndef PROPERTY_VALUE_MAX
  #define PROPERTY_VALUE_MAX 256
#endif

class DataConnectionServiceImpl : public IDataConnection {
public:
    DataConnectionServiceImpl(uint8_t setInstanceId);
    ~DataConnectionServiceImpl();
    void init();
    Return<StatusCode> getBearerAllocation(int32_t cid, const sp<IDataConnectionResponse>& cb) override;
    Return<StatusCode> getAllBearerAllocations(const sp<IDataConnectionResponse>& cb) override;
    Return<StatusCode> registerForAllBearerAllocationUpdates(const sp<IDataConnectionIndication>& cb) override;
    void notifyBearerAllocationUpdate(std::shared_ptr<AllocatedBearerResult_t> bearers);
    Return<void> getConfig(const hidl_string& key,const hidl_string& defaultValue, getConfig_cb _hidl_cb) override;

    struct ClientDeathRecipient : hidl_death_recipient {
    public:
        ClientDeathRecipient(DataConnectionServiceImpl& setService) :
            mService(setService) {};
        ~ClientDeathRecipient() = default;
        virtual void serviceDied(uint64_t cookie, const wp<IBase>& who) override;
    private:
        DataConnectionServiceImpl& mService;
    };
private:
    static void transpose(const AllocatedBearer_t& srcType, AllocatedBearers& dstType);
    static void transpose(const ResponseError_t& srcType, ErrorReason& dstType);
    static void transpose(const RatType_t& srcType, hidl_bitfield<RatType>& dstType);

    static constexpr TimeKeeper::millisec BEARER_ALLOCATION_TIMEOUT = 500;
    static constexpr const char *BEARER_ALLOCATION_WAKELOCK = "qcril_bearer_allocation_result_wakelock";

    void sendBearerAllocationResult(uint64_t cookie,
                                    std::shared_ptr<AllocatedBearerResult_t> result);
    void sendAllBearerAllocationsResult(uint64_t cookie,
                                        std::shared_ptr<AllocatedBearerResult_t> result);
    uint64_t generateCookie();
    uint64_t registerResponseCb(const sp<IDataConnectionResponse>& cb);
    uint64_t registerIndicationCb(const sp<IDataConnectionIndication>& cb);
    void releaseWakelockCb(void *);
    void acquireWakelock(TimeKeeper::millisec timeout);

    std::mutex cookieMutex;
    std::mutex registeredResponsesMutex;
    std::mutex registeredIndicationsMutex;
    std::mutex wakelockMutex;

    uint8_t instanceId;
    uint64_t nextCookie;
    std::map<uint64_t, const sp<IDataConnectionResponse>> registeredResponses;
    std::map<uint64_t, const sp<IDataConnectionIndication>> registeredIndications;
    TimeKeeper::timer_id wakelockTimerId;
    bool wakelockAcquired;

    sp<ClientDeathRecipient> clientDeathRecipient;
};
