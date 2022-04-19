/*===========================================================================
 *
 *    Copyright (c) 2018 Qualcomm Technologies, Inc.
 *    All Rights Reserved.
 *    Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 *===========================================================================*/

#include "platform/common/CommonPlatform.h"
#include <mutex>
#include <vector>

static CommonPlatform* sPlatform = nullptr;

struct platformList {
    std::vector<CommonPlatform*> list;
    std::mutex listLock;
};

// This is to make sure the object is constructed whenever the function is invoked
platformList& getPlatformList() {
    static platformList sPlatList;
    return sPlatList;
}

void setPlatform(CommonPlatform* platform) {
    auto& platList = getPlatformList();
    std::lock_guard<std::mutex> guard(platList.listLock);
    platList.list.push_back(platform);
}

static CommonPlatform* getPlatform() {
    return sPlatform;
}

extern "C" void platform_register(RIL_RadioFunctions *callbacks) {
    auto& platList = getPlatformList();
    std::lock_guard<std::mutex> guard(platList.listLock);

    for (auto const& item: platList.list) {
        item->Register(callbacks);
    }
}

extern "C" int platform_onRequest(int request, void *data, size_t datalen, RIL_Token t) {
    auto& platList = getPlatformList();
    std::lock_guard<std::mutex> guard(platList.listLock);

    for (auto const& item: platList.list) {
        if (!item->OnRequest(request, data, datalen, t))
            return 0;
    }

    return -1;
}
