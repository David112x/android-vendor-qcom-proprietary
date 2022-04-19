/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <errno.h>
#include <hidl/HidlSupport.h>
#include <hidl/HidlTransportSupport.h>
#include <hidl/LegacySupport.h>
#include <iostream>
#include <iterator>
#include <jni.h>
#include <map>
#include <mutex>
#include <stdlib.h>
#include <stringl.h>
#include <unistd.h>
#include <utils/Log.h>
#include "SystemEvent.h"
#include "SystemResource.h"

using ::android::hidl::base::V1_0::IBase;
using namespace std;
using namespace android;
using android::sp;
using android::hardware::Return;
using android::hardware::Void;
using ::vendor::qti::hardware::systemhelper::V1_0::ISystemEventCallback;
using ::vendor::qti::hardware::systemhelper::V1_0::SystemEventType;

#define LOG_TAG "SYS_EVENT"

namespace vendor {
namespace qti {
namespace hardware {
namespace systemhelper {
namespace V1_0 {
namespace implementation {

#define PHONE_MASK                          \
    (SystemEventType::PHONE_STATE_IDLE |    \
     SystemEventType::PHONE_STATE_RINGING | \
     SystemEventType::PHONE_STATE_OFF_HOOK)

#define SCREEN_MASK                      \
    (SystemEventType::ACTION_SCREEN_ON | \
     SystemEventType::ACTION_SCREEN_OFF)

#define USER_ACTION_MASK                    \
    (SystemEventType::ACTION_USER_PRESENT | \
     SystemEventType::ACTION_SCREEN_OFF)

#define UPDATE_EVENT_STATUS(status, e)    \
    do {                                  \
        uint64_t evt = (uint64_t) e;      \
        auto itr = maskMap.find(evt);     \
        if (itr != maskMap.end()) {       \
            (status) &= (~(itr->second)); \
            (status) |= (evt);            \
        }                                 \
    } while (0);

//TBD: Fix after enhancing systemhelper interface
#define EVENT_MASK (0xFFFFFFFF)
#define CLIENT_ID(x) ((x) >> 32)

struct eventInfo {
    uint64_t events;
    sp<ISystemEventCallback> cb;
};

map<uint32_t, eventInfo *> eventMap;
std::mutex eventLock;

map<uint64_t, uint64_t> createMap() {
    map<uint64_t, uint64_t> m;

    m.insert(pair<uint64_t, uint64_t>
        ((uint64_t) SystemEventType::PHONE_STATE_IDLE,
         (uint64_t) PHONE_MASK));
    m.insert(pair<uint64_t, uint64_t>
        ((uint64_t) SystemEventType::PHONE_STATE_RINGING,
         (uint64_t) PHONE_MASK));
    m.insert(pair<uint64_t, uint64_t>
        ((uint64_t) SystemEventType::PHONE_STATE_OFF_HOOK,
         (uint64_t) PHONE_MASK));
    m.insert(pair<uint64_t, uint64_t>
        ((uint64_t) SystemEventType::ACTION_SCREEN_ON,
         (uint64_t) SCREEN_MASK));
    m.insert(pair<uint64_t, uint64_t>
        ((uint64_t) SystemEventType::ACTION_SCREEN_OFF,
         (uint64_t) (SCREEN_MASK | USER_ACTION_MASK)));
    m.insert(pair<uint64_t, uint64_t>
        ((uint64_t) SystemEventType::ACTION_USER_PRESENT,
         (uint64_t) USER_ACTION_MASK));
    m.insert(pair<uint64_t, uint64_t>
        ((uint64_t) SystemEventType::ACTION_SHUTDOWN,
         (uint64_t) (SystemEventType) EVENT_MASK));

    return m;
};

map<uint64_t, uint64_t> maskMap = createMap();
uint64_t eventStatus = 0;

// Methods from ::vendor::qti::hardware::systemhelper::V1_0::ISystemEvent follow.
Return<int32_t> SystemEvent::registerEvent(uint64_t eventIds, const sp<ISystemEventCallback>& cb) {
    int32_t ret = 0;
    uint64_t events = (eventIds & EVENT_MASK);
    uint32_t client = CLIENT_ID(eventIds);

    if (cb == NULL) {
        ALOGE("%s: received NULL callback input", __func__);
        return -EINVAL;
    }
    if (events == 0 ||
        events > (uint64_t)SystemEventType::SYSTEM_EVENT_MAX) {
        ALOGE("%s: received invalid event id %ld", __func__, events);
        return -EINVAL;
    }

    std::lock_guard<std::mutex> lock(eventLock);
    auto itr = eventMap.find(client);
    if (itr == eventMap.end()) {
        // element not found , so insert new one
        eventInfo * info = new eventInfo();
        if (!info) {
            ret = -ENOMEM;
            goto EXIT;
        }
        info->events = events;
        info->cb = cb;
        eventMap.insert(pair<uint64_t, eventInfo*>(client, info));
    } else {
        if ((itr->second)->events & events) {
            // any of the requested event already present
            // either all events in one request shall be serviced or none
            ret = -EINVAL;
            ALOGE("%s: duplicate register call, event ids: %ld",
                  __func__, events);
        } else {
            (itr->second)->events |= events;
        }
    }

    // Send an event notification with current status
    if (!ret) {
        for (int64_t event = 1; event < events; event *= 2) {
            if (event & eventStatus) {
                cb->onEvent(event);
            }
        }
    }

EXIT:
    return ret;
}

Return<int32_t> SystemEvent::deRegisterEvent(uint64_t eventIds, const sp<ISystemEventCallback>& cb) {
    int32_t ret = 0;
    uint64_t events = (eventIds & EVENT_MASK);
    uint32_t client = CLIENT_ID(eventIds);

    if (cb == NULL) {
        ALOGE("%s: received NULL callback input", __func__);
        return -EINVAL;
    }
    if (events == 0 || events > (uint64_t)SystemEventType::SYSTEM_EVENT_MAX) {
        ALOGE("%s: received invalid event id %ld", __func__, eventIds);
        return -EINVAL;
    }

    std::lock_guard<std::mutex> lock(eventLock);
    auto itr = eventMap.find(client);
    if (itr != eventMap.end()) {
        if (((itr->second)->events & events) == events) {
            (itr->second)->events ^= events;
        } else {
            ALOGE("%s: invalid event id %ld", __func__, events);
            ret = -EINVAL;
        }
        if ((itr->second)->events == 0) {
            eventInfo * info = itr->second;
            // deregistered for all events
            eventMap.erase(client);
            delete info;
        }
    } else {
        ALOGE("%s: client requesting deregisteration does not exist (malicious client)", __func__);
        ret = -EINVAL;
    }

    return ret;
}

extern "C" jint
Java_com_qualcomm_qti_services_systemhelper_SysHelperService_sendEventNotification(
    JNIEnv * /*env*/, jclass /*cls*/, jint id) {
    int event = (int) id;

    ALOGD("send event %d notification to client", event);
    std::lock_guard<std::mutex> lock(eventLock);
    UPDATE_EVENT_STATUS(eventStatus, event);
    for (auto itr = eventMap.begin(); itr != eventMap.end(); ) {
        if ((itr->second)->cb != nullptr) {
            if (event == ((itr->second)->events & event)) {
                auto ret = ((itr->second)->cb)->onEvent(event);
                if (!ret.isOk() && ret.isDeadObject()) {
                    eventInfo * info = itr->second;
                    itr = eventMap.erase(itr);
                    delete info;
                    ALOGE("%s: client requested for event notification does not exist", __func__);
                    continue;
                }
            }
        }
        ++itr;
    }
    return id;
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace systemhelper
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
