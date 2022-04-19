/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once

#include <unordered_map>
#include <memory>
#include <list>
#include <functional>
#include <android/hardware/radio/1.0/IRadio.h>
#include <android/hardware/radio/1.0/IRadioResponse.h>
#include <android/hardware/radio/1.0/IRadioIndication.h>
#include <android/hardware/radio/1.0/types.h>

#include <android/hardware/radio/deprecated/1.0/IOemHook.h>
#include <framework/legacy.h>
#include <framework/Log.h>

using namespace android::hardware::radio;
using namespace android::hardware::radio::V1_0;
using namespace android::hardware::radio::deprecated::V1_0;
using android::sp;


struct RadioImpl;
class QcrildServiceVersion
{
    private:
        uint16_t mMajor;
        uint16_t mMinor;
        uint16_t mLocal;
    public:
        QcrildServiceVersion(uint16_t major, uint16_t minor, uint16_t local = 0):
            mMajor(major), mMinor(minor), mLocal(local)
        { }

        bool operator< (const QcrildServiceVersion &other) const {
            QCRIL_LOG_DEBUG("major: %u, minor: %u local: %u", (unsigned)mMajor, (unsigned)mMinor, (unsigned)mLocal);
            QCRIL_LOG_DEBUG("o.major: %u, o.minor: %u o.local: %u", (unsigned)other.mMajor, (unsigned)other.mMinor, (unsigned)other.mLocal);
            return mMajor >= other.mMajor &&
                mMinor >= other.mMinor &&
                mLocal >= other.mLocal;
        }
        bool operator>= (const QcrildServiceVersion &other) const {
            return (!((*this) < (other)));
        }
};

class QcrildServiceFactory
{
    public:
        template <class B>
        using
        FactMethod = sp<B> (QcrildServiceFactory::*)(qcril_instance_id_e_type instance);

    private:
        template <class T>
        sp<RadioImpl> getTRadioImpl(qcril_instance_id_e_type instance) {
            return sp<T>(new T(instance));
        }

        template <class B>
        struct FactoryEntry {
            FactMethod<B> factMethod;
            QcrildServiceVersion version;
        };

        template <class B>
        static inline
        bool qcrildServiceCompare
        (
         const FactoryEntry<B>& first,
         const FactoryEntry<B>& second)
        {
            return first.version < second.version;
        };


        using RadioImplListType = std::list<FactoryEntry<RadioImpl>>;
        RadioImplListType radioImplList;

    public:
        template <class T>
        void registerRadioImpl(T *) {
            QCRIL_LOG_DEBUG("registerRadioImpl");
            FactMethod<RadioImpl> factMethod = &QcrildServiceFactory::getTRadioImpl<T>;
            RadioImplListType localList { FactoryEntry<RadioImpl> {factMethod, T::getVersion()} };
            radioImplList.merge(localList, &qcrildServiceCompare<RadioImpl>);
        }

        sp<RadioImpl> getRadioImpl(qcril_instance_id_e_type instance) {
            QCRIL_LOG_INFO("getRadioImpl");
            sp<RadioImpl> ret;
            if (!radioImplList.empty()) {
                FactMethod<RadioImpl> m = radioImplList.front().factMethod;
                if (m) {
                    ret = (this->*m)(instance);
                }
            }
            return ret;
        }
};

QcrildServiceFactory &getQcrildServiceFactory();
