/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2019-2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/
#ifndef CDFWAPI_CASE_H
#define CDFWAPI_CASE_H

#include <pthread.h>
#include "GardenFramework.h"
#include "IGardenCase.h"
#include "LocTimer.h"
#include "MsgTask.h"
#include "QDgnssApi.h"
#include "QDgnssRemoteApi.h"


namespace garden {

typedef uint32_t NtripConfigParamsMask;
enum NtripConfigParamsBits {
    NTRIP_CONFIG_PARAMS_HAS_MAX_CONTENT_DATA_BUFFER_SIZE_BIT = (1<<0),
    NTRIP_CONFIG_PARAMS_HAS_MAX_TIMEOUT_FOR_FULL_BUFFER_BIT  = (1<<1),
    NTRIP_CONFIG_PARAMS_HAS_MAX_SOCKET_READ_RETRY_ATTEMPTS_BIT  = (1<<2),
    NTRIP_CONFIG_PARAMS_HAS_WAIT_BETWEEN_RECONNECT_ATTEMPTS_BIT = (1<<3),
    NTRIP_CONFIG_PARAMS_HAS_LOG_CORRECTION_DATA_BIT    = (1<<4)
};

/**
    @brief structure to hold configurations needed for debugging / tuning the NTRIP service.
*/
struct ntripConfigs {
    // Validity bits for values set in the structure
    NtripConfigParamsMask paramsMask;
    // Max content data buffer size in bytes for NTRIP service
    int maxContentDataBufferSizeInBytes;
    // Max timeout for full buffer in sec for NTRIP service
    int maxTimeoutForFullBufferInSec;
    // Max Socket read retry attemps for NTRIP service
    int maxSocketReadRetryAttempts;
    // Wait time between reconnect attemps for NTRIP service
    int waitTimeBetweenReconnectAttemptsInSec;
    // Log correction data
    bool logCorrectionData;
};

typedef std::function<void(uint8_t *correctionData, uint32_t lengthInBytes)>
         CorrectionDataCb;
typedef void (NtripClientStartDataStreamingApi)(bool useNtrip2Version, const string &hostName,
        const string & mountPoint, const string & userNamePwdEncodedInBase64Format,
        uint32_t port, const ntripConfigs &configs, CorrectionDataCb corrDataCb);
typedef void (NtripClientStopDataStreamingApi)();

class CDFWApiCase;
/******************************************************************************
LocTimer override
******************************************************************************/
class InjectDataTimer : public LocTimer {
public:
    InjectDataTimer(CDFWApiCase& apiCase, MsgTask& msgTask) :
        mApiCase(apiCase), mMsgTask(msgTask) {}
    void timeOutCallback() override;

private:
    CDFWApiCase& mApiCase;
    MsgTask& mMsgTask;
};


class QDgnssFileListener : public QDgnssListener {
public:
    virtual void onReceiveMsg(unique_ptr<IQDgnssMsg>& dGnssMsg) override;
    inline QDgnssFileListener(CDFWApiCase& apiCase, MsgTask& msgTask) :
        QDgnssListener(), mApiCase(apiCase), mMsgTask(msgTask) {}

private:
    CDFWApiCase& mApiCase;
    MsgTask& mMsgTask;
};

class QDgnssInternetListener : public QDgnssListener {
public:
    virtual void onReceiveMsg(unique_ptr<IQDgnssMsg>& dGnssMsg) override;
};

class CDFWApiCase:public IGardenCase
{
public:
    CDFWApiCase();
    virtual ~CDFWApiCase();
    virtual GARDEN_RESULT preRun();
    virtual GARDEN_RESULT postRun();
    GARDEN_RESULT menuCase();
    void processQDgnssMsg(const char* data, uint32_t length);

private:
    void injectDataFromInternet();
    void injectDataFromFile();
    void closeFile();
    ntripConfigs mNtripConfigs;
    NtripClientStartDataStreamingApi* mNtripClientStartDataStreamingApi;
    NtripClientStopDataStreamingApi* mNtripClientStopDataStreamingApi;
    shared_ptr<QDgnssSource> mFileSource;
    unique_ptr<QDgnssFileListener> mFileListener;
    unique_ptr<QDgnssSource> mInternetSource;
    unique_ptr<QDgnssInternetListener> mInternetListener;
    bool mNtripClientStarted;
    static MsgTask* mMsgTask;
    InjectDataTimer* mInjectDataTimer;
    FILE *mFrtcm = nullptr;
    bool mIsFileOpen = false;
    uint32_t mTotalReadSize = 0;
    pthread_t  mPthreadId;
    bool mIsThreadAlive;
    char   mHostName[30];
    char   mMountPoint[30];
    char   mUserNamePwdInBase64Format[50];
    const char*  mNtripClientConfFile;
    uint32_t  mPort;

    shared_ptr<QDgnssRemoteSource> mpSource;

friend InjectDataTimer;
};

}
#endif
