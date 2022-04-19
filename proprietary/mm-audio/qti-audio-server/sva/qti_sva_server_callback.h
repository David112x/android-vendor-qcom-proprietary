/* Copyright (c) 2017 - 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <stdlib.h>
#include <utils/RefBase.h>
#include <utils/Log.h>
#include <binder/TextOutput.h>
#include <binder/IInterface.h>
#include <binder/IBinder.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <binder/IPCThreadState.h>
#include <binder/IPCThreadState.h>
#include <qsthw.h>
#include <qsthw_defs.h>

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif

#ifndef memscpy
#define memscpy(dst, dst_size, src, bytes_to_copy) (void) memcpy(dst, src, MIN(dst_size, bytes_to_copy))
#endif

namespace svahal {
using namespace android;

enum {
    ON_RECOGNITION_EVENT = IBinder::FIRST_CALL_TRANSACTION,
    ON_SOUNDMODEL_EVENT,
    ON_SERVICE_STATE_CHANGE
};

class IRecognitionCallback : public ::android::IInterface {
public:
    DECLARE_META_INTERFACE(RecognitionCallback);
    virtual int recognition_cb(
                        void *event,
                        void *cookie1, void *cookie2) = 0;
};

class QTIRecognitionCallback : public ::android::BnInterface<IRecognitionCallback> {
public:
    QTIRecognitionCallback(){};
    ~QTIRecognitionCallback(){};
private:
    int32_t onTransact(uint32_t code,
                        const Parcel& data,
                        Parcel* reply,
                        uint32_t flags) override;
    int recognition_cb(void *event,
                        void *cookie1, void *cookie2) override;
};
}; // namespace svahal

