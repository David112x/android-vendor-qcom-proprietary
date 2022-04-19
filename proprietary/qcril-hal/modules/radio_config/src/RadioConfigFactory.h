/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once

#include <unordered_map>
#include <memory>
#include <list>
#include <functional>
#include <android/hardware/radio/1.0/types.h>

#include <framework/legacy.h>
#include <framework/Log.h>

using android::sp;

struct RadioConfigImpl;

class QcrildConfigVersion
{
    private:
        uint16_t mMajor;
        uint16_t mMinor;
        uint16_t mLocal;
    public:
        QcrildConfigVersion(uint16_t major, uint16_t minor, uint16_t local = 0):
            mMajor(major), mMinor(minor), mLocal(local)
        { }

        bool operator< (const QcrildConfigVersion &other) const {
            QCRIL_LOG_DEBUG("major: %u, minor: %u local: %u", (unsigned)mMajor, (unsigned)mMinor, (unsigned)mLocal);
            QCRIL_LOG_DEBUG("o.major: %u, o.minor: %u o.local: %u", (unsigned)other.mMajor, (unsigned)other.mMinor, (unsigned)other.mLocal);
            return mMajor >= other.mMajor &&
                mMinor >= other.mMinor &&
                mLocal >= other.mLocal;
        }
        bool operator>= (const QcrildConfigVersion &other) const {
            return (!((*this) < (other)));
        }
};

class RadioConfigFactory
{
    public:
        template <class B>
        using
        FactMethod = sp<B> (RadioConfigFactory::*)();

    private:
        template <class T>
        sp<RadioConfigImpl> getTRadioConfigImpl() {
            return sp<T>(new T());
        }

        template <class B>
        struct FactoryEntry {
            FactMethod<B> factMethod;
            QcrildConfigVersion version;
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


        using RadioConfigImplListType = std::list<FactoryEntry<RadioConfigImpl>>;
        RadioConfigImplListType radioConfigImplList;

    public:
        template <class T>
        void registerRadioConfigImpl(T *) {
            QCRIL_LOG_DEBUG("registerRadioConfigImpl");
            FactMethod<RadioConfigImpl> factMethod = &RadioConfigFactory::getTRadioConfigImpl<T>;
            RadioConfigImplListType localList { FactoryEntry<RadioConfigImpl> {factMethod, T::getVersion()} };
            radioConfigImplList.merge(localList, &qcrildServiceCompare<RadioConfigImpl>);
        }

        sp<RadioConfigImpl> getRadioConfigImpl() {
            QCRIL_LOG_INFO("getRadioConfigImpl");
            sp<RadioConfigImpl> ret;
            if (!radioConfigImplList.empty()) {
                FactMethod<RadioConfigImpl> m = radioConfigImplList.front().factMethod;
                if (m) {
                    ret = (this->*m)();
                }
            }
            return ret;
        }
};

RadioConfigFactory &getRadioConfigFactory();