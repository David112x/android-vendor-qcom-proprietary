/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

#include "IPerfManager.h"
#define MAX_SIZE_ALLOWED 64

namespace android {

class BpPerfManager : public BpInterface<IPerfManager> {
public:
    BpPerfManager(const sp<IBinder>& binder)
        : BpInterface<IPerfManager>(binder)
    {
    }

    virtual int perfLockAcquire(int duration, int len, int* boostsList)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IPerfManager::getInterfaceDescriptor());
        data.writeInt32(duration);
        data.writeInt32Array(len, boostsList);
        remote()->transact(PERF_LOCK_ACQUIRE, data, &reply);
        reply.readExceptionCode();
        return reply.readInt32();
    }

    virtual int perfHint(int hint, String16& userDataStr, int userData1, int userData2, int tid)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IPerfManager::getInterfaceDescriptor());
        data.writeInt32(hint);
        data.writeString16(userDataStr);
        data.writeInt32(userData1);
        data.writeInt32(userData2);
        data.writeInt32(tid);
        remote()->transact(PERF_HINT, data, &reply);
        reply.readExceptionCode();
        return reply.readInt32();
    }

    virtual int perfLockRelease()
    {
        Parcel data, reply;
        data.writeInterfaceToken(IPerfManager::getInterfaceDescriptor());
        remote()->transact(PERF_LOCK_RELEASE, data, &reply);
        reply.readExceptionCode();
        return reply.readInt32();
    }

    virtual int perfLockReleaseHandler(int _handle)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IPerfManager::getInterfaceDescriptor());
        data.writeInt32(_handle);
        remote()->transact(PERF_LOCK_RELEASE_HANDLER, data, &reply);
        reply.readExceptionCode();
        return reply.readInt32();
    }

    virtual int perfUXEngine_events(int opcode, int pid, String16& pkg_name, int lat)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IPerfManager::getInterfaceDescriptor());
        data.writeInt32(opcode);
        data.writeInt32(pid);
        data.writeString16(pkg_name);
        data.writeInt32(lat);
        remote()->transact(PERF_UX_ENGINE_EVENTS, data, &reply);
        reply.readExceptionCode();
        return reply.readInt32();
    }

    virtual int setClientBinder(const sp<IBinder>& client)
    {
        Parcel data, reply;
        data.writeInterfaceToken(IPerfManager::getInterfaceDescriptor());
        data.writeStrongBinder(client);
        remote()->transact(SET_CLIENT_BINDER, data, &reply);
        reply.readExceptionCode();
        return reply.readInt32();
    }
};

IMPLEMENT_META_INTERFACE(PerfManager, "com.qualcomm.qti.IPerfManager");

status_t BnPerfManager::onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{

    switch (code) {
    case PERF_LOCK_ACQUIRE: {
        CHECK_INTERFACE(IPerfManager, data, reply);
        int duration = data.readInt32();
        int len = data.readInt32();
        if (len <= 0 || len > MAX_SIZE_ALLOWED)
            return BAD_VALUE;
        int boostList[MAX_SIZE_ALLOWED];
        for (int i = 0; i < len; i++) {
            boostList[i] = data.readInt32();
        }
        int handle = perfLockAcquire(duration, len, boostList);
        reply->writeNoException();
        reply->writeInt32(handle);
        return NO_ERROR;
    } break;
    case PERF_HINT: {
        CHECK_INTERFACE(IPerfManager, data, reply);
        int hint = data.readInt32();
        String16 userDataStr = data.readString16();
        int userData1 = data.readInt32();
        int userData2 = data.readInt32();
        int tid = data.readInt32();
        int handle = perfHint(hint, userDataStr, userData1, userData2, tid);
        reply->writeNoException();
        reply->writeInt32(handle);
        return NO_ERROR;
    } break;
    case PERF_LOCK_RELEASE: {
        CHECK_INTERFACE(IPerfManager, data, reply);
        int result = perfLockRelease();
        reply->writeNoException();
        reply->writeInt32(result);
        return NO_ERROR;
    } break;
    case PERF_LOCK_RELEASE_HANDLER: {
        CHECK_INTERFACE(IPerfManager, data, reply);
        int handle = data.readInt32();
        int result = perfLockReleaseHandler(handle);
        reply->writeNoException();
        reply->writeInt32(result);
        return NO_ERROR;
    } break;
    case PERF_UX_ENGINE_EVENTS: {
        CHECK_INTERFACE(IPerfManager, data, reply);
        int opcode = data.readInt32();
        int pid = data.readInt32();
        String16 pkg_name = data.readString16();
        int lat = data.readInt32();
        int result = perfUXEngine_events(opcode, pid, pkg_name, lat);
        reply->writeNoException();
        reply->writeInt32(result);
        return NO_ERROR;
    } break;
    case SET_CLIENT_BINDER: {
        CHECK_INTERFACE(IPerfManager, data, reply);
        sp<IBinder> clientBinder = data.readStrongBinder();
        int result = setClientBinder(clientBinder);
        reply->writeNoException();
        reply->writeInt32(result);
        return NO_ERROR;
    } break;
    default:
      return BBinder::onTransact(code, data, reply, flags);
    }
}
}; //namespace android
