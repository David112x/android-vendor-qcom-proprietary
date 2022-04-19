/* Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#define LOG_TAG "qti_audio_client"
#define ATRACE_TAG (ATRACE_TAG_QAS)
/* #define LOG_NDEBUG 0 */
#define LOG_NDDEBUG 0

#include <stdlib.h>
#include <utils/RefBase.h>
#include <utils/Log.h>
#include <binder/TextOutput.h>
#include <binder/IInterface.h>
#include <binder/IBinder.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <binder/IPCThreadState.h>
#include <binder/MemoryDealer.h>
#include <pthread.h>
#include "qti_audio_client.h"

using namespace android;

const android::String16 IQASClient::descriptor("IQASClient");
const android::String16& IQASClient::getInterfaceDescriptor() const {
    return IQASClient::descriptor;
}

class BpClient: public ::android:: BpInterface<IQASClient> {
public:
    BpClient(const sp<IBinder>& impl) :
        BpInterface<IQASClient>(impl) {
        ALOGD("BpClient::BpClient()");
    }
};

android::sp<IQASClient> IQASClient::asInterface
    (const android::sp<android::IBinder>& obj) {
    ALOGV("IQASClient::asInterface()");
    android::sp<IQASClient> intr;
    if (obj != NULL) {
        intr = static_cast<IQASClient*>(obj->queryLocalInterface
                        (IQASClient::descriptor).get());
        ALOGD("IQASClient::asInterface() interface %s",
            ((intr == 0)?"zero":"non zero"));
        if (intr == NULL)
            intr = new BpClient(obj);
    }
    return intr;
}

IQASClient::IQASClient() {
    ALOGV("IQASClient::IQASClient()");
}

IQASClient::~IQASClient() {
    ALOGV("IQASClient::~IQASClient()");
}

int32_t QTIClient::onTransact(uint32_t code,
                                   const Parcel& data,
                                   Parcel* reply __unused,
                                   uint32_t flags) {
    int status = 0;
    ALOGV("QTIClient::onTransact(%i) %i", code, flags);
    data.checkInterface(this);
    switch(code) {
        default:
            return BBinder::onTransact(code, data, reply, flags);
            break;
    }
    return status;
}
