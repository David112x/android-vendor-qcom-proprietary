/* Copyright (c) 2017 - 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#define LOG_TAG "qti_sva_server_callback"
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
#include "qti_sva_server_callback.h"

namespace svahal {
using namespace android;

const android::String16 IRecognitionCallback::descriptor("IRecognitionCallback");
const android::String16& IRecognitionCallback::getInterfaceDescriptor() const {
    return IRecognitionCallback::descriptor;
}

class BpRecognitionCallback: public ::android:: BpInterface<IRecognitionCallback> {
public:
    BpRecognitionCallback(const sp<IBinder>& impl) :
        BpInterface<IRecognitionCallback>(impl) {
        ALOGD("BpRecognitionCallback::BpRecognitionCallback()");
    }

    int recognition_cb(void *event,
                        void *cookie1, void *cookie2) {
        ALOGV("%s %d", __func__,__LINE__);
        Parcel data, reply;
        data.writeInterfaceToken(IRecognitionCallback::getInterfaceDescriptor());
        struct qsthw_phrase_recognition_event *rec_event =
            (struct qsthw_phrase_recognition_event *)event;
        uint32_t size = sizeof(struct qsthw_phrase_recognition_event) +
                        rec_event->phrase_event.common.data_size;
        data.writeUint32(size);
        android::Parcel::WritableBlob blob;
        data.writeBlob(size, false, &blob);
        memset(blob.data(), 0x0, size);
        ALOGD("size %d, offset %d, data_size %d", size,
                    rec_event->phrase_event.common.data_offset,
                    rec_event->phrase_event.common.data_size);
        memscpy(blob.data(), (size - rec_event->phrase_event.common.data_size),
            rec_event, (size - rec_event->phrase_event.common.data_size));
        memscpy(((char *)blob.data()) + rec_event->phrase_event.common.data_offset,
                    rec_event->phrase_event.common.data_size,
                    ((char *)rec_event) + rec_event->phrase_event.common.data_offset,
                    rec_event->phrase_event.common.data_size);
        blob.release();
        data.write(&cookie1, sizeof(qsthw_recognition_callback_t *));
        data.write(&cookie2, sizeof(qsthw_recognition_callback_t *));
        return remote()->transact(ON_RECOGNITION_EVENT, data, &reply);
    }
};

android::sp<IRecognitionCallback> IRecognitionCallback::asInterface
    (const android::sp<android::IBinder>& obj) {
    ALOGD("IRecognitionCallback::asInterface()");
    android::sp<IRecognitionCallback> intr;
    if (obj != NULL) {
        intr = static_cast<IRecognitionCallback*>(obj->queryLocalInterface
                        (IRecognitionCallback::descriptor).get());
        ALOGD("IRecognitionCallback::asInterface() interface %s",
            ((intr == 0)?"zero":"non zero"));
        if (intr == NULL)
            intr = new BpRecognitionCallback(obj);
    }
    return intr;
}

IRecognitionCallback::IRecognitionCallback()
    { ALOGD("IRecognitionCallback::IRecognitionCallback()"); }
IRecognitionCallback::~IRecognitionCallback()
    { ALOGD("IRecognitionCallback::~IRecognitionCallback()"); }

int32_t QTIRecognitionCallback::onTransact(uint32_t code,
                                   const Parcel& data,
                                   Parcel* reply __unused,
                                   uint32_t flags) {
    int status = 0;
    ALOGV("QTIRecognitionCallback::onTransact(%i) %i", code, flags);
    data.checkInterface(this);
    switch(code) {
        case ON_RECOGNITION_EVENT: {
            void *cookie1 = NULL;
            void *cookie2 = NULL;
            uint32_t blob_size;
            struct qsthw_phrase_recognition_event *rec_event = NULL;
            android::Parcel::ReadableBlob blob;
            data.readUint32(&blob_size);
            data.readBlob(blob_size, &blob);
            rec_event = (struct qsthw_phrase_recognition_event *)calloc(1, blob_size);
            if (rec_event == NULL) {
                ALOGE("%s: Could not allocate memory for sound model", __func__);
                return -ENOMEM;
            }
            int size = sizeof(qsthw_phrase_recognition_event);
            memscpy(rec_event, size, blob.data(), size);
            memscpy(((char *)rec_event) + rec_event->phrase_event.common.data_offset,
                        (blob_size - size), ((char *)blob.data())
                        + rec_event->phrase_event.common.data_offset, (blob_size - size));
            blob.release();
            data.read(&cookie1, sizeof(qsthw_recognition_callback_t *));
            data.read(&cookie2, sizeof(qsthw_recognition_callback_t *));
            status = recognition_cb((void *)rec_event, cookie1, cookie2);
            free(rec_event);
            break;
        }
        default:
            ALOGE("%s: Invalid cmd received", __func__);
            status = -EINVAL;
            break;
    }
    return status;
}

int QTIRecognitionCallback::recognition_cb(
                        void *event,
                        void *cookie1, void *cookie2) {
    qsthw_recognition_callback_t rec_callback =
                    (qsthw_recognition_callback_t)cookie1;
    ALOGD("Calling Client Registered Callback(%p)", rec_callback);
    rec_callback((struct sound_trigger_recognition_event *)event, cookie2);
    return 0;
}

}; // namespace svahal

