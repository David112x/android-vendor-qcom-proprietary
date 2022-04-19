/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2013-2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/
#ifndef __LBS_PROXY_H__
#define __LBS_PROXY_H__
#include <LBSProxyBase.h>
#include <LocAdapterBase.h>
#include <pthread.h>

using namespace loc_core;

namespace lbs_core {

class LBSProxy : public LBSProxyBase {
    static pthread_mutex_t mLock;
    static LocAdapterBase* mAdapter;
    static unsigned long mCapabilities;
    static const char * COM_QUALCOMM_LOCATION_APK_FILE_PATH;
    static const char * FFOS_LOCATION_EXTENDED_CLIENT;
    static const bool mLocationExtendedClientExists;
    static const bool mNativeXtraClientExists;
public:
    inline LBSProxy() : LBSProxyBase() {}
    inline ~LBSProxy() {}
    virtual LocApiBase* getLocApi(LOC_API_ADAPTER_EVENT_MASK_T exMask,
                                  ContextBase *context) const;
    inline virtual bool hasAgpsExtendedCapabilities() const {
        return mLocationExtendedClientExists;
    }
    inline virtual bool hasNativeXtraClient() const {
        return mNativeXtraClientExists;
    }
#ifdef MODEM_POWER_VOTE
#ifdef PDMAPPER_AVAILABLE
    void parseModemRegistryForGpsService(int* pModemType) const;
#endif /*PDMAPPER_AVAILABLE*/
#endif /*MODEM_POWER_VOTE*/
    inline virtual void modemPowerVote(bool power) const;
    virtual void injectFeatureConfig(ContextBase* context) const;
private:
    inline static bool checkIfLocationExtendedClientExists();
    inline static bool checkIfNativeXtraClientExists();
};

}  // namespace lbs_core
#endif //__LBS_PROXY_H__
