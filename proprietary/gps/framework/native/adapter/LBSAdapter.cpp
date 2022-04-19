/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2013-2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/
#define LOG_NDEBUG 0
#define LOG_TAG "LocSvc_LBSAdapter"

#include <LBSAdapter.h>

using namespace loc_core;
using namespace lbs_core;



LBSAdapter* LBSAdapter::mMe = NULL;

LBSAdapter* LBSAdapter::get(const LOC_API_ADAPTER_EVENT_MASK_T mask)
{
    if (NULL == mMe) {
        mMe = new LBSAdapter(mask);
    } else if (mask) {
        mMe->enableMask(mask);
    }
    return mMe;
}

int LBSAdapter::timeInfoInject(long curTimeMillis, int rawOffset, int dstOffset) {
    struct TimeInfoInjectMsg : public LocMsg {
        LBSApiBase *mLBSApi;
        long mCurTimeMs;
        int mOffsetRaw;
        int mOffsetDst;
        inline TimeInfoInjectMsg(LBSApiBase *lbsApi, long curTimeMillis, int rawOffset,
                                 int dstOffset) :
            mLBSApi(lbsApi), mCurTimeMs(curTimeMillis), mOffsetRaw(rawOffset),
            mOffsetDst(dstOffset) {}
        inline virtual void proc() const {
            LOC_LOGD("%s:%d]: Injecting timezone info", __func__, __LINE__);
            mLBSApi->timeInfoInject(mCurTimeMs, mOffsetRaw, mOffsetDst);
        }
    };
    LOC_LOGD("%s:%d]: Injecting timezone info curTime:%ld, rawOffset:%d, dstOffset:%d ",
             __func__, __LINE__, curTimeMillis, rawOffset, dstOffset);
    sendMsg(new TimeInfoInjectMsg(mLBSApi, curTimeMillis, rawOffset, dstOffset));
    return true;
}

int LBSAdapter::cinfoInject(int cid, int lac, int mnc,
                                int mcc, bool roaming)
{
    struct CinfoInjectMsg : public LocMsg {
        LBSApiBase* mLBSApi;
        int mCid, mLac, mMnc, mMcc;
        bool mRoaming;
        inline CinfoInjectMsg(LBSApiBase* lbsApi,
                              int cid, int lac,
                              int mnc, int mcc, bool roaming) :
            LocMsg(), mLBSApi(lbsApi), mCid(cid), mLac(lac),
            mMnc(mnc), mMcc(mcc), mRoaming(roaming) {}
        inline virtual void proc() const {
            mLBSApi->cinfoInject(mCid, mLac, mMnc, mMcc, mRoaming);
        }
    };
    sendMsg(new CinfoInjectMsg(mLBSApi, cid, lac, mnc, mcc, roaming));
    return true;
}

int LBSAdapter::oosInform()
{
    struct OosInformMsg : public LocMsg {
        LBSApiBase* mLBSApi;
        inline OosInformMsg(LBSApiBase* lbsApi) :
            LocMsg(), mLBSApi(lbsApi) {}
        inline virtual void proc() const {
            mLBSApi->oosInform();
        }
    };
    sendMsg(new OosInformMsg(mLBSApi));
    return true;
}

int LBSAdapter::niSuplInit(const char* supl_init, int length)
{
    struct NisuplInitMsg : public LocMsg {
        LBSApiBase* mLBSApi;
        int mLen;
        char* mSuplInit;
        inline NisuplInitMsg(LBSApiBase* lbsApi, const char* supl_init,
                             int len) :
            LocMsg(), mLBSApi(lbsApi), mLen(len),
            mSuplInit(new char[mLen]) {

            if (mSuplInit == nullptr) {
                LOC_LOGE("new allocation failed, fatal error.");
                mLen = 0;
                return;
            }
            memcpy(mSuplInit, supl_init, mLen);
        }
        inline virtual ~NisuplInitMsg() { delete[] mSuplInit; }
        inline virtual void proc() const {
            mLBSApi->niSuplInit(mSuplInit, mLen);
        }
    };
    sendMsg(new NisuplInitMsg(mLBSApi, supl_init, length));
    return true;
}

int LBSAdapter::chargerStatusInject(int status)
{
    struct ChargerSatusMsg : public LocMsg {
        LBSApiBase* mLBSApi;
        int mStatus;
        inline ChargerSatusMsg(LBSApiBase* lbsApi, int status) :
            LocMsg(), mLBSApi(lbsApi), mStatus(status) {}
        inline virtual void proc() const {
            mLBSApi->chargerStatusInject(mStatus);
        }
    };
    sendMsg(new ChargerSatusMsg(mLBSApi, status));
    return true;
}

int LBSAdapter::injectWifiApInfo(WifiApInfo &wifiApInfo)
{
    struct WifiApDataMsg : public LocMsg {
        LBSApiBase* mLBSApi;
        WifiApInfo mWifiApData;
        inline WifiApDataMsg(LBSApiBase* lbsApi, WifiApInfo &wifiApInfo) :
            LocMsg(), mLBSApi(lbsApi), mWifiApData(wifiApInfo) {}
        inline virtual void proc() const {
            mLBSApi->injectWifiApInfo(mWifiApData);
        }
    };
    sendMsg(new WifiApDataMsg(mLBSApi, wifiApInfo));
    return true;
}

int LBSAdapter::injectBtClassicDevScanData(BtDeviceInfo &btDevInfo)
{
    struct BtClassicDevScanDataMsg : public LocMsg {
        LBSApiBase* mLBSApi;
        BtDeviceInfo mBtDevInfo;
        inline BtClassicDevScanDataMsg(LBSApiBase* lbsApi, BtDeviceInfo &btDevInfo) :
            LocMsg(), mLBSApi(lbsApi), mBtDevInfo(btDevInfo) {}
        inline virtual void proc() const {
            mLBSApi->injectBtClassicDevScanData(mBtDevInfo);
        }
    };
    sendMsg(new BtClassicDevScanDataMsg(mLBSApi, btDevInfo));
    return true;
}

int LBSAdapter::injectBtLeDevScanData(BtDeviceInfo &btDevInfo)
{
    struct BtLeDevScanDataMsg : public LocMsg {
        LBSApiBase* mLBSApi;
        BtDeviceInfo mBtDevInfo;
        inline BtLeDevScanDataMsg(LBSApiBase* lbsApi, BtDeviceInfo &btDevInfo) :
            LocMsg(), mLBSApi(lbsApi), mBtDevInfo(btDevInfo) {}
        inline virtual void proc() const {
            mLBSApi->injectBtLeDevScanData(mBtDevInfo);
        }
    };
    sendMsg(new BtLeDevScanDataMsg(mLBSApi, btDevInfo));
    return true;
}

int LBSAdapter::setWifiWaitTimeoutValue(int timeout)
{
    struct WifiSetTimeoutMsg : public LocMsg {
        LBSApiBase* mLBSApi;
        int mTimeout;
        inline WifiSetTimeoutMsg(LBSApiBase* lbsApi, int timeout) :
            LocMsg(), mLBSApi(lbsApi), mTimeout(timeout) {}
        inline virtual void proc() const {
            mLBSApi->setWifiWaitTimeoutValue(mTimeout);
        }
    };
    sendMsg(new WifiSetTimeoutMsg(mLBSApi, timeout));
    return true;
}

int LBSAdapter::getZppFixRequest(enum zpp_fix_type type)
{
    if (type == ZPP_FIX_BEST_AVAILABLE) {
        mLocApi->getBestAvailableZppFix();
    }
    else {
        mLocApi->getWwanZppFix();
    }

    return true;
}

bool LBSAdapter::reportWwanZppFix(LocGpsLocation &zppLoc)
{
    sendMsg( new WwanFixMsg(&zppLoc));
    return true;
}

bool LBSAdapter::reportZppBestAvailableFix(LocGpsLocation &zppLoc,
            GpsLocationExtended &location_extended, LocPosTechMask tech_mask) {
    sendMsg( new ZppFixMsg(zppLoc, location_extended, tech_mask));
    return true;
}

int LBSAdapter::wifiAttachmentStatusInject(
    WifiSupplicantInfo &wifiSupplicantInfo)
{
    struct WifiAttachmentStatusMsg : public LocMsg {
        LBSApiBase* mLBSApi;
        WifiSupplicantInfo mWifiSupplicantInfo;
        inline WifiAttachmentStatusMsg(LBSApiBase* lbsApi,
                                       WifiSupplicantInfo &wifiSupplicantInfo) :
            LocMsg(), mLBSApi(lbsApi),
            mWifiSupplicantInfo(wifiSupplicantInfo) {}
        inline virtual void proc() const {
            mLBSApi->wifiAttachmentStatusInject(mWifiSupplicantInfo);
        }
    };
    sendMsg(new WifiAttachmentStatusMsg(mLBSApi, wifiSupplicantInfo));
    return true;
}

int LBSAdapter::wifiEnabledStatusInject(int status)
{
    struct WifiEnabledStatusMsg : public LocMsg {
        LBSApiBase* mLBSApi;
        int mStatus;
        inline WifiEnabledStatusMsg(LBSApiBase* lbsApi, int status) :
            LocMsg(), mLBSApi(lbsApi), mStatus(status) {}
        inline virtual void proc() const {
            mLBSApi->wifiEnabledStatusInject(mStatus);
        }
    };
    sendMsg(new WifiEnabledStatusMsg(mLBSApi, status));
    return true;
}

bool LBSAdapter::requestTimeZoneInfo()
{
    sendMsg(new TimeZoneInfoRequest());
    LOC_LOGD("%s:%d]: Sending message for TimeZoneInfoRequest", __func__, __LINE__);
    return true;
}

void LBSAdapter::registerFdclCommand(FdclCallbacks& fdclCallbacks, void* fdclClientData)
{
    struct RegisterFdclMsg : public LocMsg {
        LBSAdapter* mLBSAdapter;
        FdclCallbacks mFdclCallbacks;
        void* mClientData;
        inline RegisterFdclMsg(LBSAdapter* lbsAdapter, FdclCallbacks& fdclCallbacks,
                               void* fdclClientData) :
            LocMsg(), mLBSAdapter(lbsAdapter), mFdclCallbacks(fdclCallbacks),
            mClientData(fdclClientData) {}
        inline virtual void proc() const {
            if (nullptr == mLBSAdapter) {
                LOC_LOGD("%s]: Null LBS adapter pointer", __func__);
                return;
            }
            mLBSAdapter->mFdclCallbacks = mFdclCallbacks;
            mLBSAdapter->mFdclClientData = mClientData;
            if (mLBSAdapter != nullptr && mLBSAdapter->mFdclCallbacks.envNotifyCb != nullptr) {
                mLBSAdapter->mFdclCallbacks.envNotifyCb(mLBSAdapter->mFdclClientData);
                LOC_LOGD("%s]: Calling notify callback (VM attach)", __func__);
            }else{
                LOC_LOGD("%s]: Not calling notify callback (VM detached)", __func__);
            }
        }
    };

    sendMsg(new RegisterFdclMsg(this, fdclCallbacks, fdclClientData));
}

void LBSAdapter::unregisterFdclCommand()
{
    struct UnregisterFdclMsg : public LocMsg {
        LBSAdapter* mLBSAdapter;
        inline UnregisterFdclMsg(LBSAdapter* lbsAdapter) :
            LocMsg(), mLBSAdapter(lbsAdapter) {}
        inline virtual void proc() const {
            if (mLBSAdapter != nullptr) {
                mLBSAdapter->mFdclCallbacks = {nullptr, nullptr, nullptr, nullptr};
                mLBSAdapter->mFdclClientData = nullptr;
            }
        }
    };

    sendMsg(new UnregisterFdclMsg(this));
}

void LBSAdapter::injectFdclDataCommand(FdclData& fdclData)
{
    struct InjectFdclDataMsg : public LocMsg {
        LBSAdapter* mLBSAdapter;
        LBSApiBase* mLBSApi;
        FdclData mFdclData;
        inline InjectFdclDataMsg(LBSAdapter* lbsAdapter, LBSApiBase* lbsApi,
                                 FdclData& fdclData) :
            LocMsg(), mLBSAdapter(lbsAdapter), mLBSApi(lbsApi), mFdclData(fdclData) {}
        inline virtual void proc() const {
            if (nullptr == mLBSApi || nullptr == mLBSAdapter) {
                return;
            }
            if (!mLBSAdapter->isFeatureSupported(LOC_SUPPORTED_FEATURE_FDCL)) {
                if (mLBSAdapter->mFdclCallbacks.statusCb != nullptr) {
                    mLBSAdapter->mFdclCallbacks.statusCb(false, "UNSUPPORTED",
                                                         mLBSAdapter->mFdclClientData);
                }
            } else {
                mLBSApi->injectFdclData(mFdclData);
            }
        }
    };

    sendMsg(new InjectFdclDataMsg(this, mLBSApi, fdclData));
}

void LBSAdapter::requestFdclDataCommand(int32_t expireInDays)
{
    struct RequestFdclDataMsg : public LocMsg {
        LBSAdapter* mLBSAdapter;
        LBSApiBase* mLBSApi;
        int32_t mExpireInDays;
        inline RequestFdclDataMsg(LBSAdapter* lbsAdapter, LBSApiBase* lbsApi,
                                  int32_t expireInDays) :
            LocMsg(), mLBSAdapter(lbsAdapter), mLBSApi(lbsApi), mExpireInDays(expireInDays) {}
        inline virtual void proc() const {
            if (nullptr == mLBSApi || nullptr == mLBSAdapter) {
              if (nullptr == mLBSApi){
                LOC_LOGD("%s]: Null LBSApi pointer", __func__);
              }
              if (nullptr == mLBSAdapter) {
                LOC_LOGD("%s]: Null LBS adapter pointer", __func__);
              }
              return;
            }
            if (!mLBSAdapter->isFeatureSupported(LOC_SUPPORTED_FEATURE_FDCL)) {
                if (mLBSAdapter->mFdclCallbacks.statusCb != nullptr) {
                    mLBSAdapter->mFdclCallbacks.statusCb(false, "UNSUPPORTED",
                                                         mLBSAdapter->mFdclClientData);
                LOC_LOGD("%s]: Reporting FDCL Feature not supported", __func__);
                }
            } else {
                mLBSApi->requestFdclData(mExpireInDays);
            }
        }
    };

    sendMsg(new RequestFdclDataMsg(this, mLBSApi, expireInDays));
}

bool LBSAdapter::requestFdclDataEvent()
{
    struct requestFdclDataEventMsg : public LocMsg {
        LBSAdapter* mLBSAdapter;
        inline requestFdclDataEventMsg(LBSAdapter* lbsAdapter) :
            LocMsg(), mLBSAdapter(lbsAdapter) {}
        inline virtual void proc() const {
            if (mLBSAdapter != nullptr && mLBSAdapter->mFdclCallbacks.serviceReqCb != nullptr) {
                mLBSAdapter->mFdclCallbacks.serviceReqCb(mLBSAdapter->mFdclClientData);
            }
        }
    };
    sendMsg(new requestFdclDataEventMsg(this));
    return true;
}

bool LBSAdapter::reportFdclStatusEvent(bool success, const char* errorString, uint32_t errorLength)
{
    struct reportFdclStatusEventMsg : public LocMsg {
        LBSAdapter* mLBSAdapter;
        bool mSuccess;
        const char* mErrorString;
        inline reportFdclStatusEventMsg(LBSAdapter* lbsAdapter, bool success,
                                        const char* errorString, uint32_t errorLength) :
            LocMsg(), mLBSAdapter(lbsAdapter), mSuccess(success),
            mErrorString(new char[errorLength+1]) {
            if (mErrorString != nullptr) {
                strlcpy((char*)mErrorString, errorString, errorLength+1);
            }
        }
        inline virtual ~reportFdclStatusEventMsg() {
            delete[] mErrorString;
        }
        inline virtual void proc() const {
            if (mLBSAdapter != nullptr && mLBSAdapter->mFdclCallbacks.statusCb != nullptr) {
                mLBSAdapter->mFdclCallbacks.statusCb(mSuccess, mErrorString,
                                                     mLBSAdapter->mFdclClientData);
            }
        }
    };
    sendMsg(new reportFdclStatusEventMsg(this, success, errorString, errorLength));

    return true;
}

bool LBSAdapter::reportFdclInfoEvent(FdclInfo& fdclInfo, uint8_t status, UlpLocation& location)
{
    struct reportFdclInfoEventMsg : public LocMsg {
        LBSAdapter* mLBSAdapter;
        FdclInfo mFdclInfo;
        uint8_t mStatus;
        UlpLocation mLocation;
        inline reportFdclInfoEventMsg(LBSAdapter* lbsAdapter, FdclInfo& fdclInfo, uint8_t status,
                UlpLocation& location) : LocMsg(), mLBSAdapter(lbsAdapter), mFdclInfo(fdclInfo),
                mStatus(status), mLocation(location) {}
        inline virtual void proc() const {
            if (mLBSAdapter != nullptr && mLBSAdapter->mFdclCallbacks.bsListAvailCb != nullptr) {

                BSInfo_s* bsInfo = new BSInfo_s[mFdclInfo.bsInfoList.size()];
                if (nullptr == bsInfo) {
                    LOC_LOGE("BSInfo new allocation failed !");
                    return;
                }
                int index = 0;
                for(auto const& item: mFdclInfo.bsInfoList) {
                    bsInfo[index++] = item;
                }
                mLBSAdapter->mFdclCallbacks.bsListAvailCb(bsInfo, mFdclInfo.bsInfoList.size(),
                        mStatus, mLocation, mLBSAdapter->mFdclClientData);
                delete[] bsInfo;
            }
        }
    };
    sendMsg(new reportFdclInfoEventMsg(this, fdclInfo, status, location));

    return true;
}

void LBSAdapter::registerCellCSCommand(CellCSCallbacks& cellCSCallbacks, void* cellCSClientData)
{
    struct RegisterCellCSMsg : public LocMsg {
        LBSAdapter* mLBSAdapter;
        CellCSCallbacks mCellCSCallbacks;
        void* mClientData;
        inline RegisterCellCSMsg(LBSAdapter* lbsAdapter, CellCSCallbacks& cellCSCallbacks,
                               void* cellCSClientData) :
            LocMsg(), mLBSAdapter(lbsAdapter), mCellCSCallbacks(cellCSCallbacks),
            mClientData(cellCSClientData) {}
        inline virtual void proc() const {
            if (nullptr == mLBSAdapter) {
                LOC_LOGD("%s]: Null LBS adapter pointer", __func__);
                return;
            }
            mLBSAdapter->mCellCSCallbacks = mCellCSCallbacks;
            mLBSAdapter->mCellCSClientData = mClientData;
            if (mLBSAdapter != nullptr && mLBSAdapter->mCellCSCallbacks.envNotifyCb != nullptr) {
                mLBSAdapter->mCellCSCallbacks.envNotifyCb(mLBSAdapter->mCellCSClientData);
                LOC_LOGD("%s]: Calling notify callback (VM attach)", __func__);
            }else{
                LOC_LOGD("%s]: Not calling notify callback (VM detached)", __func__);
            }
        }
    };

    sendMsg(new RegisterCellCSMsg(this, cellCSCallbacks, cellCSClientData));
}

void LBSAdapter::unregisterCellCSCommand()
{
    struct UnregisterCellCSMsg : public LocMsg {
        LBSAdapter* mLBSAdapter;
        inline UnregisterCellCSMsg(LBSAdapter* lbsAdapter) :
            LocMsg(), mLBSAdapter(lbsAdapter) {}
        inline virtual void proc() const {
            if (mLBSAdapter != nullptr) {
                mLBSAdapter->mCellCSCallbacks = {nullptr, nullptr, nullptr};
                mLBSAdapter->mCellCSClientData = nullptr;
            }
        }
    };

    sendMsg(new UnregisterCellCSMsg(this));
}

void LBSAdapter::requestCellCSDataCommand()
{
    struct RequestCellCSDataMsg : public LocMsg {
        LBSAdapter* mLBSAdapter;
        LBSApiBase* mLBSApi;
        inline RequestCellCSDataMsg(LBSAdapter* lbsAdapter, LBSApiBase* lbsApi) :
            LocMsg(), mLBSAdapter(lbsAdapter), mLBSApi(lbsApi) {}
        inline virtual void proc() const {
            if (nullptr == mLBSApi || nullptr == mLBSAdapter) {
              if (nullptr == mLBSApi){
                LOC_LOGD("%s]: Null LBSApi pointer", __func__);
              }
              if (nullptr == mLBSAdapter) {
                LOC_LOGD("%s]: Null LBS adapter pointer", __func__);
              }
              return;
            }
            if (!mLBSAdapter->isFeatureSupported(LOC_SUPPORTED_FEATURE_FDCL_2)) {
                LOC_LOGD("%s]: Reporting CELLCS Feature not supported", __func__);
            } else {
                mLBSApi->requestCellCSData();
            }
        }
    };

    sendMsg(new RequestCellCSDataMsg(this, mLBSApi));
}

bool LBSAdapter::requestCellCSDataEvent()
{
    struct requestCellCSDataEventMsg : public LocMsg {
        LBSAdapter* mLBSAdapter;
        inline requestCellCSDataEventMsg(LBSAdapter* lbsAdapter) :
            LocMsg(), mLBSAdapter(lbsAdapter) {}
        inline virtual void proc() const {
            if (mLBSAdapter != nullptr && mLBSAdapter->mCellCSCallbacks.serviceReqCb != nullptr) {
                mLBSAdapter->mCellCSCallbacks.serviceReqCb(mLBSAdapter->mCellCSClientData);
            }
        }
    };
    sendMsg(new requestCellCSDataEventMsg(this));
    return true;
}

bool LBSAdapter::reportCellCSInfoEvent(CellCSInfo& cellCSInfo, uint8_t status)
{
    struct reportCellCSInfoEventMsg : public LocMsg {
        LBSAdapter* mLBSAdapter;
        CellCSInfo mCellCSInfo;
        uint8_t mStatus;
        inline reportCellCSInfoEventMsg(LBSAdapter* lbsAdapter, CellCSInfo& cellCSInfo,
                uint8_t status) : LocMsg(), mLBSAdapter(lbsAdapter), mCellCSInfo(cellCSInfo),
                mStatus(status) {}
        inline virtual void proc() const {
            if (mLBSAdapter != nullptr &&
                    mLBSAdapter->mCellCSCallbacks.bsObsLocDataAvailable != nullptr) {

                BSObsData_s* csInfo = new BSObsData_s[mCellCSInfo.bsObsDataList.size()];
                if (NULL != csInfo) {
                    int index = 0;
                    for (auto const& item : mCellCSInfo.bsObsDataList) {
                        csInfo[index++] = item;
                    }
                    mLBSAdapter->mCellCSCallbacks.bsObsLocDataAvailable(csInfo,
                        mCellCSInfo.bsObsDataList.size(), mStatus,
                        mLBSAdapter->mCellCSClientData);
                    delete[] csInfo;
                } else {
                    LOC_LOGe("csInfo is NULL!");
                }
            }
        }
    };
    sendMsg(new reportCellCSInfoEventMsg(this, cellCSInfo, status));

    return true;
}

int LBSAdapter::enableMask(LOC_API_ADAPTER_EVENT_MASK_T mask)
{
    struct enableMaskMsg : public LocMsg {
        LOC_API_ADAPTER_EVENT_MASK_T mMask;
        inline enableMaskMsg(LOC_API_ADAPTER_EVENT_MASK_T mask) :
            LocMsg(), mMask(mask) {}
        inline virtual void proc() const {
            LBSAdapter::mMe->updateEvtMask(mMask, LOC_REGISTRATION_MASK_ENABLED);
        }
    };
    sendMsg(new enableMaskMsg(mask));
    return true;
}


bool LBSAdapter::batteryLevelInject(int status)
{
    struct BatteryLevelMsg : public LocMsg {
        LBSApiBase* mLBSApi;
        int mBatteryPct;
        inline BatteryLevelMsg(LBSApiBase* lbsApi, int batteryPct) :
            LocMsg(), mLBSApi(lbsApi), mBatteryPct(batteryPct) {}
        inline virtual void proc() const {
            mLBSApi->batteryLevelInject(mBatteryPct);
        }
    };
    sendMsg(new BatteryLevelMsg(mLBSApi, status));
    return true;
}
