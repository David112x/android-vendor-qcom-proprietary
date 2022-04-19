/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2015-2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

#ifndef __IZAT_MANAGER_WIPER_H__
#define __IZAT_MANAGER_WIPER_H__

#include <MsgTask.h>
#include <ContextBase.h>
#include "IFreeWifiScanObserver.h"
#include "IDataItemObserver.h"
#include <mq_client/IPCMessagingProxy.h>
#include "LBSAdapter.h"
#include "FreeWifiScanObserver.h"
#include <base_util/vector.h>
#include "IzatContext.h"
#include "lowi_response.h"
#include "DataItemConcreteTypes.h"
#include "LocTimer.h"
#include "IDataItemCore.h"

namespace qc_loc_fw {
class InPostcard;
class LOWIScanMeasurement;
class LOWIResponse;
class LOWIDiscoveryScanResultResponse;
class LOWIAsyncDiscoveryScanResultResponse;
}

namespace izat_manager
{

using namespace std;
using namespace loc_core;
using namespace izat_manager;

typedef map < string, BtAccessPointInfo > BtApInfoMap;


class Wiper :
    public IFreeWifiScanObserver,
    public IDataItemObserver {

public:

    // Singleton methods
    static Wiper * getInstance(const struct s_IzatContext * izatContex);
    static void destroyInstance();

    // set feature registration bits and initialize/update LBSAdapter with event mask
    void setFeatureRegistrationBits();

    // IFreeWifiScanObserver overrides
    virtual void notify(const LOWIResponse * response);

    // IDataItemObserver overrides
    virtual void getName (string & name);
    virtual void notify(const std::list<IDataItemCore *> & dlist);

    static void handleWifiApDataRequest(const WifiApDataRequest &request);
    static void handleSsrInform();
    static void handleTimeZoneInfoRequest();
    static void handleSrnRequest(const GnssSrnDataReq &gnssSrnDataReq);

private:
    typedef enum WiperTimerClient {
        BTLE_SCAN_DATA_REQ_TIMER,
        BTCLASSIC_SCAN_DATA_REQ_TIMER,
        WIPERTIMERCLIENT_MAX
    } WiperTimerClient;

    typedef struct WiperConfigData {
        int    mWifiWaitTimeoutSelect;
        int    mSrnDataScanInjectTime;
    } WiperConfigData;

    class Timer : public LocTimer {
        Wiper* mClient;
        int    mTimerClient;
    public:
        inline Timer(Wiper* client, int clientId) :
            LocTimer(), mClient(client), mTimerClient(clientId) {}
        virtual void timeOutCallback();
    };
    friend class Timer;

    typedef enum WiperFeatureFlag {
        FEATURE_SRN_SCAN_DATA_INJECT = 0x0001,
        FEATURE_FREE_WIFI_SCAN_INJECT_MASK = 0x2000,
        FEATURE_SUPL_WIFI_MASK = 0x4000,
        FEATURE_WIFI_SUPPLICANT_INFO_MASK = 0x8000
    } WiperFeatureFlag;

    // Data members
    static Wiper                  * mInstance;
    const struct s_IzatContext    * mIzatContext;
    LBSAdapter                    * mLbsAdapter;
    FreeWifiScanObserver          * mWifiScanObserver;
    int                             mWiperFlag;
    WiperConfigData                 mWiperConfigData;
    bool                            mIsWifiScanInSession;
    int64                           mWifiScanRequestedUTCTimesInMs;
    WifiScanSet                     mWifiScanSet;
    WifiSupplicantStatusDataItem::WifiSupplicantState    mLatestSupplicantState;
    bool                            mSupportWIFIInject2V2;
    Timer                           mBtLeScanDataRequestTimer;
    Timer                           mBtClassicScanDataRequestTimer;
    BtApInfoMap                     mBtClassicDevicesMap;
    BtApInfoMap                     mBtLeDevicesMap;

    Wiper(const struct s_IzatContext * izatContext,
        unsigned int wiperFlag, WiperConfigData &wiperConfData);
    ~Wiper();

    static void stringify (WifiSupplicantInfo & wsi, string & valueStr);
    static void stringify (WifiApInfo & wai, string & valueStr);
    static void stringify (WifiApSsidInfo & ws, string & valueStr);

    struct wifiScanNotificationMsg : public LocMsg {
        Wiper *mWiperObj;
        qc_loc_fw::vector<LOWIScanMeasurement *> mLowiScanMeasurements;
        WifiScanType mWifiScanType;
        LOWIMacAddress mSelfMacAddr;

        wifiScanNotificationMsg(Wiper *wiperObj,
            qc_loc_fw::vector<LOWIScanMeasurement *> lowiScanMeasurements,
            WifiScanType scanType, LOWIMacAddress self_mac);

        ~wifiScanNotificationMsg();
        virtual void proc() const;
    };

     struct wifiScanNotificationErrorMsg : public LocMsg {
        Wiper *mWiperObj;
        WifiScanResponseType mErrType;
        WifiScanType mWifiScanType;

        wifiScanNotificationErrorMsg(Wiper *wiperObj, WifiScanResponseType errType,
            WifiScanType scanType) : mWiperObj(wiperObj), mErrType(errType), mWifiScanType(scanType) {}

        ~wifiScanNotificationErrorMsg() {}
        virtual void proc() const;
    };

    struct wifiScanRequestMsg : public LocMsg {
        Wiper *mWiperObj;
        const WifiApDataRequest mRequest;

        inline wifiScanRequestMsg(Wiper *wiperObj, const WifiApDataRequest &request) :
            mWiperObj(wiperObj), mRequest(request) {
        }

        inline ~wifiScanRequestMsg() {}
        virtual void proc() const;
    };

    struct handleOsObserverUpdateMsg : public LocMsg {
        Wiper *mWiperObj;
        std::list <IDataItemCore *> mDataItemList;

        handleOsObserverUpdateMsg(Wiper *wiperObj,
            const std::list<IDataItemCore *> & dataItemList);

        ~handleOsObserverUpdateMsg();
        virtual void proc() const;
    };

    struct handleSsrMsg : public LocMsg {
        Wiper *mWiperObj;

        inline handleSsrMsg(Wiper *wiperObj) :
            mWiperObj(wiperObj) {
        }

        inline ~handleSsrMsg() {}
        virtual void proc() const;
    };

    struct wiperSubscribeInit : public LocMsg {
        Wiper *mWiperObj;
        int mWiperFlag;

        wiperSubscribeInit(Wiper *wiperObj, int wiperFlag) :
            mWiperObj(wiperObj), mWiperFlag(wiperFlag) {
        }

        inline ~wiperSubscribeInit() {}
        virtual void proc() const;
    };

    class LBSShutDownAdapter;
    struct lbsShutDownMsg : public LocMsg {
        LBSShutDownAdapter* mAdapter;
        inline lbsShutDownMsg(LBSShutDownAdapter* adapter) :
            LocMsg(), mAdapter(adapter) {}
        inline ~lbsShutDownMsg() {
            ContextBase* ctx = mAdapter->getContext();
            delete mAdapter;
            ((MsgTask*)ctx->getMsgTask())->destroy();
            delete ctx;
        }
        void proc() const;
    };

    class LBSShutDownAdapter : public LBSAdapterBase {
    public:
        inline LBSShutDownAdapter(ContextBase* context) :
                LBSAdapterBase(0, context) {}
        inline virtual void handleEngineUpEvent() {
            sendMsg(new (nothrow) lbsShutDownMsg(this));
        }
    };

    struct handleTimeZoneInfoRequestMsg : public LocMsg {
        Wiper *mWiperObj;

        inline handleTimeZoneInfoRequestMsg(Wiper *wiperObj) :
            mWiperObj(wiperObj) {
        }

        inline ~handleTimeZoneInfoRequestMsg() {}
        virtual void proc() const;
    };

    struct injectBtLeScanData : public LocMsg {
        Wiper* mWiperObj;

        inline injectBtLeScanData(Wiper *pWiperObj) : mWiperObj(pWiperObj) {}
        virtual void proc() const;
        inline ~injectBtLeScanData() {}
    };

    struct injectBtClassicScanData : public LocMsg {
        Wiper* mWiperObj;

        inline injectBtClassicScanData(Wiper *pWiperObj) : mWiperObj(pWiperObj) {}
        virtual void proc() const;
        inline ~injectBtClassicScanData() {}
    };

};


} // namespace izat_manager

#endif // #ifndef __IZAT_MANAGER_WIPER_H__

