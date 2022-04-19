/* Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef AUDIO_CLIENT_H
#define AUDIO_CLIENT_H

#include <stdlib.h>
#include <utils/RefBase.h>
#include <utils/Log.h>
#include <binder/TextOutput.h>
#include <binder/IInterface.h>
#include <binder/IBinder.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <binder/IPCThreadState.h>

using namespace android;

class IQASClient : public ::android::IInterface {
public:
    DECLARE_META_INTERFACE(QASClient);
};

class QTIClient : public ::android::BnInterface<IQASClient> {
public:
    QTIClient(){ ALOGD("QTIClient::QTIClient()"); };
    ~QTIClient(){ ALOGD("~QTIClient::QTIClient()"); };
private:
    int32_t onTransact(uint32_t code,
                        const Parcel& data,
                        Parcel* reply,
                        uint32_t flags) override;
};

#endif //AUDIO_CLIENT_H
