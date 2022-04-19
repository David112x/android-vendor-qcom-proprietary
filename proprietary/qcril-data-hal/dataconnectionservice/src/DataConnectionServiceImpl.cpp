/*
 * Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <cutils/properties.h>

#include "framework/Log.h"
#include "hardware_legacy/power.h"

#include "DataConnectionServiceImpl.h"
#include "GetBearerAllocationRequestMessage.h"
#include "GetAllBearerAllocationsRequestMessage.h"
#include "RegisterBearerAllocationUpdateRequestMessage.h"

using android::hardware::Return;
using android::sp;
using rildata::GetBearerAllocationRequestMessage;
using rildata::GetAllBearerAllocationsRequestMessage;
using rildata::RegisterBearerAllocationUpdateRequestMessage;
using rildata::AllocatedBearerResult_t;
using rildata::ResponseError_t;
using std::placeholders::_1;
using std::placeholders::_3;

/**
 * DataConnectionServiceImpl::DataConnectionServiceImpl()
 *
 * @brief
 * Constructor for IDataConnection HAL implementation. Registers the HAL service per
 * slot id
 */
DataConnectionServiceImpl::DataConnectionServiceImpl(uint8_t setInstanceId):
        instanceId(setInstanceId), nextCookie(0), registeredResponses { },
        registeredIndications { }, wakelockTimerId(TimeKeeper::no_timer),
        wakelockAcquired(false) {
    Log::getInstance().d("DataConnectionServiceImpl()");
}

/**
 * DataConnectionServiceImpl::~DataConnectionServiceImpl()
 *
 * @brief
 * Destructor for IDataConnection HAL implementation. Release shared pointer references
 */
DataConnectionServiceImpl::~DataConnectionServiceImpl() {
    clientDeathRecipient = nullptr;
}

void DataConnectionServiceImpl::init() {
    std::string serviceName = "slot";
    android::status_t status = -1;
    status = registerAsService(serviceName + std::to_string((int)instanceId + 1));
    if(status != android::OK) {
        Log::getInstance().d("DataConnection HAL registration failed ");
    }
    clientDeathRecipient = new ClientDeathRecipient(*this);
}

/**
 * DataConnectionServiceImpl::generateCookie()
 *
 * @brief
 * Generates a unique cookie to used to track each client Response/Indication callback
 *
 * @return uint64_t Unique cookie
 */
uint64_t DataConnectionServiceImpl::generateCookie() {
    uint64_t newCookie;
    cookieMutex.lock();
    newCookie = nextCookie++;
    cookieMutex.unlock();
    return newCookie;
}

/**
 * DataConnectionServiceImpl::registerResponseCb()
 *
 * @brief
 * Stores the client's response callback to the map to be invoked once DataModule completes the
 * request
 *
 * @return uint64_t The cookie used to identify the callback in the Response map
 */
uint64_t DataConnectionServiceImpl::registerResponseCb(const sp<IDataConnectionResponse>& cb) {
    uint64_t cookie = generateCookie();
    registeredResponsesMutex.lock();
    Log::getInstance().d("+cookie "+ std::to_string(cookie));
    registeredResponses.insert({cookie, cb});
    registeredResponsesMutex.unlock();
    cb->linkToDeath(clientDeathRecipient, cookie);
    return cookie;
}

/**
 * DataConnectionServiceImpl::registerIndicationCb()
 *
 * @brief
 * Stores the client's indication callback to the map to be invoked every time a QMI indication
 * is received for DataConnection HAL
 *
 * @return uint64_t The cookie used to identify the callback in the Indications map
 */
uint64_t DataConnectionServiceImpl::registerIndicationCb(const sp<IDataConnectionIndication>& cb) {
    uint64_t cookie = generateCookie();
    registeredIndicationsMutex.lock();
    if (registeredIndications.empty()) {
        auto requestMsg = std::make_shared<RegisterBearerAllocationUpdateRequestMessage>(true);
        requestMsg->dispatch();
    }
    Log::getInstance().d("+cookie "+ std::to_string(cookie));
    registeredIndications.insert({cookie, cb});
    registeredIndicationsMutex.unlock();
    cb->linkToDeath(clientDeathRecipient, cookie);
    return cookie;
}

void DataConnectionServiceImpl::acquireWakelock(TimeKeeper::millisec timeout) {
    std::lock_guard<std::mutex> lock(wakelockMutex);
    // if already acquired, refresh the timer
    if (wakelockAcquired) {
        Log::getInstance().d("DataConnectionServiceImpl::refreshing wakelock");
        bool timerCleared = TimeKeeper::getInstance().clear_timer(wakelockTimerId);
        if (!timerCleared) {
            Log::getInstance().d("Failed to clear wakelock timer");
        }
    } else {
        Log::getInstance().d("DataConnectionServiceImpl::acquiring wakelock");
        if (acquire_wake_lock(PARTIAL_WAKE_LOCK, BEARER_ALLOCATION_WAKELOCK) < 0) {
            Log::getInstance().d("Failed to acquire wakelock");
        }
    }
    wakelockTimerId = TimeKeeper::getInstance().set_timer(
        std::bind(&DataConnectionServiceImpl::releaseWakelockCb, this, _1),
        nullptr,
        timeout);
}

void DataConnectionServiceImpl::releaseWakelockCb(void *) {
    Log::getInstance().d("DataConnectionServiceImpl::release wakelock");
    std::lock_guard<std::mutex> lock(wakelockMutex);
    if (wakelockAcquired)
    {
        if (release_wake_lock(BEARER_ALLOCATION_WAKELOCK) < 0)
        {
            Log::getInstance().d("DataConnectionServiceImpl::wakelock not acquired");
        }
        wakelockAcquired = false;
    }
}

/**
 * ClientDeathRecipient::serviceDied()
 *
 * @brief
 * Removes client from the callback maps once the client dies. Notify DataModule when there are
 * no remaining clients registered for indications.
 */
void DataConnectionServiceImpl::ClientDeathRecipient::serviceDied(uint64_t cookie, const wp<IBase>& who) {
    Log::getInstance().d("DataConnectionServiceImpl::client died -cookie" + std::to_string(cookie));
    mService.registeredResponsesMutex.lock();
    mService.registeredResponses.erase(cookie);
    mService.registeredResponsesMutex.unlock();
    mService.registeredIndicationsMutex.lock();
    mService.registeredIndications.erase(cookie);
    if (mService.registeredIndications.empty()) {
        auto requestMsg = std::make_shared<RegisterBearerAllocationUpdateRequestMessage>(false);
        requestMsg->dispatch();
    }
    mService.registeredIndicationsMutex.unlock();
    (void)who;
}

/**
 * DataConnectionServiceImpl::notifyBearerAllocationUpdate()
 *
 * @brief
 * Notifies all registered clients of a bearer allocation update and sends the
 * updated bearer allocation list.
 */
void DataConnectionServiceImpl::notifyBearerAllocationUpdate(std::shared_ptr<AllocatedBearerResult_t> result) {
    Log::getInstance().d("DataConnectionServiceImpl::notifyBearerAllocationUpdate");
    if (result == nullptr) {
        Log::getInstance().d("bearer allocation update with null");
        return;
    }
    std::vector<AllocatedBearers> bearersList;
    for (auto connection : result->connections) {
        AllocatedBearers bearers;
        transpose(connection, bearers);
        bearersList.push_back(bearers);
    }

    // invoke indication callback on all registered clients
    acquireWakelock(BEARER_ALLOCATION_TIMEOUT);
    std::lock_guard<std::mutex> lock(registeredIndicationsMutex);
    for (auto it = registeredIndications.begin(); it != registeredIndications.end(); ++it) {
        const sp<IDataConnectionIndication> clientCb = it->second;
        if (clientCb != nullptr) {
            clientCb->onBearerAllocationUpdate(bearersList);
        }
    }
}

/**
 * DataConnectionServiceImpl::getBearerAllocation()
 *
 * @brief
 * Implementation for IDataConnection's getBearerAllocation API. This is a
 * public api that can be used to query bearer allocation info for a
 * specified call id.
 */
Return<StatusCode> DataConnectionServiceImpl::getBearerAllocation(
    int32_t cid,
    const sp<IDataConnectionResponse>& cb
) {
    uint64_t cookie = registerResponseCb(cb);
    Log::getInstance().d("DataConnectionServiceImpl::getBearerAllocation()" + to_string(cookie));
    auto requestMsg = std::make_shared<GetBearerAllocationRequestMessage>(cid);
    auto responseFn = std::bind(&DataConnectionServiceImpl::sendBearerAllocationResult, this, cookie, _3);
    GenericCallback<AllocatedBearerResult_t> requestCb(responseFn);
    requestMsg->setCallback(&requestCb);
    requestMsg->dispatch();
    return StatusCode::OK;
}

/**
 * DataConnectionServiceImpl::getAllBearerAllocations()
 *
 * @brief
 * Implementation for IDataConnection's getAllBearerAllocations API. This is a
 * public api that can be used to query bearer allocation info for all
 * calls.
 */
Return<StatusCode> DataConnectionServiceImpl::getAllBearerAllocations(
    const sp<IDataConnectionResponse>& cb
) {
    uint64_t cookie = registerResponseCb(cb);
    Log::getInstance().d("DataConnectionServiceImpl::getAllBearerAllocations()" + to_string(cookie));
    if (cb == nullptr) {
        Log::getInstance().d("getAllBearerAllocations cb is null");
    }
    auto requestMsg = std::make_shared<GetAllBearerAllocationsRequestMessage>();
    auto responseFn = std::bind(&DataConnectionServiceImpl::sendAllBearerAllocationsResult, this, cookie, _3);
    GenericCallback<AllocatedBearerResult_t> requestCb(responseFn);
    requestMsg->setCallback(&requestCb);
    requestMsg->dispatch();
    return StatusCode::OK;
}

/**
 * DataConnectionServiceImpl::sendBearerAllocationResult()
 *
 * @brief
 * This invokes the client's Response callback when querying for bearer
 * allocation. This is passed as a GenericCallback inside the
 * GetBearerAllocationRequestMessage to DataModule. DataModule
 * shall call this once bearer allocation information is retrieved
 */
void DataConnectionServiceImpl::sendBearerAllocationResult(
    uint64_t cookie,
    std::shared_ptr<AllocatedBearerResult_t> result
) {
    registeredResponsesMutex.lock();
    const sp<IDataConnectionResponse> cb = registeredResponses[cookie];
    registeredResponsesMutex.unlock();
    Log::getInstance().d("DataConnectionServiceImpl::sendBearerAllocationResult()" + to_string(cookie));
    if (cb == nullptr) {
        Log::getInstance().d("sendBearerAllocationResult() client callback is null");
        return;
    }
    acquireWakelock(BEARER_ALLOCATION_TIMEOUT);
    AllocatedBearers bearers = {};
    ErrorReason error = ErrorReason::HARDWARE_ERROR;
    if (result == nullptr) {
        Log::getInstance().d("bearer list is null");
        cb->onBearerAllocationResponse(error, bearers);
        return;
    }
    transpose(result->error, error);
    if (result->error != ResponseError_t::NO_ERROR ||
        result->connections.empty()) {
        Log::getInstance().d("getBearerAllocation returned error");
        cb->onBearerAllocationResponse(error, bearers);
        return;
    }

    AllocatedBearer_t bearerResult = result->connections.front();
    transpose(bearerResult, bearers);
    cb->onBearerAllocationResponse(error, bearers);
}

/**
 * DataConnectionServiceImpl::sendAllBearerAllocationsResult()
 *
 * @brief
 * This invokes the client's Response callback when querying for bearer
 * allocation. This is passed as a GenericCallback inside the
 * GetAllBearerAllocationsRequestMessage to DataModule. DataModule
 * shall call this once bearer allocation information is retrieved
 */
void DataConnectionServiceImpl::sendAllBearerAllocationsResult(
    uint64_t cookie,
    std::shared_ptr<AllocatedBearerResult_t> result
) {
    registeredResponsesMutex.lock();
    const sp<IDataConnectionResponse> cb = registeredResponses[cookie];
    registeredResponsesMutex.unlock();
    Log::getInstance().d("DataConnectionServiceImpl::sendBearerAllocationResult()" + to_string(cookie));
    if (cb == nullptr) {
        Log::getInstance().d("sendAllBearerAllocationsResult() client callback is null");
        return;
    }
    acquireWakelock(BEARER_ALLOCATION_TIMEOUT);
    std::vector<AllocatedBearers> bearersList;
    ErrorReason error = ErrorReason::HARDWARE_ERROR;
    if (result == nullptr) {
        Log::getInstance().d("bearer list is null");
        cb->onAllBearerAllocationsResponse(error, bearersList);
        return;
    }
    transpose(result->error, error);
    if (result->error != ResponseError_t::NO_ERROR) {
        Log::getInstance().d("getAllBearerAllocations returned error");
        cb->onAllBearerAllocationsResponse(error, bearersList);
        return;
    }
    for (auto connection : result->connections) {
        AllocatedBearers bearers = {};
        transpose(connection, bearers);
        bearersList.push_back(bearers);
    }
    cb->onAllBearerAllocationsResponse(error, bearersList);
}

/**
 * DataConnectionServiceImpl::getConfig()
 *
 * @brief
 * This API queries the vendor property value
 * and returns it to the client
 */
Return<void> DataConnectionServiceImpl::getConfig(const hidl_string& key, const hidl_string& defaultValue, getConfig_cb _hidl_cb)
{
    Log::getInstance().d("DataConnectionServiceImpl::getConfig");
    char prop[PROPERTY_VALUE_MAX] = {'\0'};
    property_get(key.c_str(), prop, defaultValue.c_str());
    std::string configValue(prop);
    _hidl_cb(configValue);
    return Void();
}

/**
 * DataConnectionServiceImpl::registerForAllBearerAllocationUpdates()
 *
 * @brief
 * Implementation for IDataConnection's registerForAllBearerAllocationUpdates
 * API. This is a public api that can be used to register for bearer allocation
 * updates for all calls.
 */
Return<StatusCode> DataConnectionServiceImpl::registerForAllBearerAllocationUpdates(
    const sp<IDataConnectionIndication>& cb
)
{
    Log::getInstance().d("DataConnectionServiceImpl::registerForBearerAllocationUpdates");
    registerIndicationCb(cb);
    return StatusCode::OK;
}

void DataConnectionServiceImpl::transpose(
    const AllocatedBearer_t& connection,
    AllocatedBearers& bearers
) {
    bearers.cid = connection.cid;
    bearers.apn = connection.apn;
    std::vector<BearerInfo> dataBearers;
    for (auto connectionBearers : connection.bearers) {
        BearerInfo bearerInfo;
        bearerInfo.bearerId = connectionBearers.bearerId;
        transpose(connectionBearers.uplink, bearerInfo.uplink);
        transpose(connectionBearers.downlink, bearerInfo.downlink);
        dataBearers.push_back(bearerInfo);
    }
    bearers.bearers = dataBearers;
}

void DataConnectionServiceImpl::transpose(
    const RatType_t& commonType,
    hidl_bitfield<RatType>& ratType
) {
    switch (commonType) {
        case RatType_t::RAT_4G:
            ratType = static_cast<uint32_t>(RatType::RAT_4G);
            break;
        case RatType_t::RAT_5G:
            ratType = static_cast<uint32_t>(RatType::RAT_5G);
            break;
        case RatType_t::RAT_SPLITED:
            ratType = static_cast<uint32_t>(RatType::RAT_4G | RatType::RAT_5G);
            break;
        case RatType_t::RAT_UNSPECIFIED:
        default:
            ratType = static_cast<uint32_t>(RatType::UNSPECIFIED);
            break;
    }
}

void DataConnectionServiceImpl::transpose(
    const ResponseError_t& commonError,
    ErrorReason& halError
) {
    switch (commonError) {
        case ResponseError_t::NO_ERROR:
            halError = ErrorReason::NO_ERROR;
            break;
        case ResponseError_t::CALL_NOT_AVAILABLE:
            halError = ErrorReason::CALL_NOT_AVAILABLE;
            break;
        default:
            halError = ErrorReason::HARDWARE_ERROR;
            break;
    }
}
