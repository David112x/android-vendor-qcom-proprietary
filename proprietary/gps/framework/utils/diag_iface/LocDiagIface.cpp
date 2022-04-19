/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>
#include <log.h>
#include <log_codes.h>
#include <loc_pla.h>
#include <log_util.h>
#include <MsgTask.h>
#include "msg_q.h"
#include <diag_lsm.h>
#include "LocDiagIface.h"
#include <loc_cfg.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "EP_DIAG_LOG"

LocDiagIface *LocDiagIface::mInstance = nullptr;

LocDiagIface* LocDiagIface::getInstance()
{
    if (nullptr == mInstance) {
        mInstance = new LocDiagIface();
    }
    return mInstance;
}

LocDiagIface::LocDiagIface(): mQ((void*)msg_q_init2())
{
    mDiagBuffActive = false;
    mIsDiagBufferingEnabled = 0;
    mTimeOut = DIAG_MSG_BUFFER_WAIT_TIMEOUT;
    mLastDiagInitFailedBoottime = 0;
    mLastDiagInitCount = 0;
    mIsDiagInitialized = Diag_LSM_Init(NULL);
    if (false == mIsDiagInitialized) {
        mLastDiagInitFailedBoottime = uptimeMillis();
        mLastDiagInitCount++;
        LOC_LOGE("%s] Failed to initialize DIAG!", __func__);
    }
    static loc_param_s_type buffer_conf_param_table[] =
    {
        {"BUFFER_DIAG_LOGGING", &mIsDiagBufferingEnabled, NULL, 'n'}
    };
    UTIL_READ_CONF(LOC_PATH_GPS_CONF, buffer_conf_param_table);
    LOC_LOGV("%s] BUFFER_DIAG_LOGGING: %d", __func__, mIsDiagBufferingEnabled);
}

void LocDiagIface::destroy()
{
    LOC_LOGD("%s]", __func__);
    if (true == mIsDiagInitialized) {
        Diag_LSM_DeInit();
    }
    mDiagBuffActive = false;
    msg_q_flush((void*)mQ);
    msg_q_destroy((void**)&mQ);
    if (nullptr != mInstance) {
        delete mInstance;
        mInstance = nullptr;
    }
}

LocDiagIface::~LocDiagIface()
{
    LOC_LOGD("%s]", __func__);
}

void LocDiagIface::flushBuffer()
{
    DiagCommitMsg *pCommitMsg = nullptr;
    void *msg = nullptr;
    msq_q_err_type result = eMSG_Q_SUCCESS;
    LOC_LOGV("%s --> \n", __func__);
    do
    {
        result = msg_q_rmv(mQ, (void **)&msg);
        if (eMSG_Q_SUCCESS != result) {
            LOC_LOGE("%s fail to receive msg! result: %d\n", __func__, result);
            break;
        }
        pCommitMsg = (DiagCommitMsg *)msg;
        if (nullptr != pCommitMsg) {
            void *pMsg;
            LOC_LOGV("%s Diag ID: 0x%X buffer source:%d \n",
                    __func__, pCommitMsg->mDiagId, pCommitMsg->mBufferSrc);
            if ((BUFFER_FROM_MALLOC == pCommitMsg->mBufferSrc) &&
                    (pCommitMsg->mSize > sizeof(log_hdr_type))) {
                /*Allocate diag buffer and copy payload from malloc */
                pMsg = log_alloc(pCommitMsg->mDiagId, pCommitMsg->mSize);
                if (pMsg != nullptr) {
                    /**log_alloc will populate the log header, Copy remaining payload */
                    memcpy((void *) ((char *)pMsg + sizeof(log_hdr_type)),
                           ((char *)pCommitMsg->mCommitPtr + sizeof(log_hdr_type)),
                           pCommitMsg->mSize - sizeof(log_hdr_type));
                    log_commit(pMsg);
                } else {
                    LOC_LOGW("[%s] log_alloc return nullptr \n", __func__);
                }
            }
            delete pCommitMsg;
            pCommitMsg = nullptr;
        }
    } while(1);
}

/*!
 * @function: logCommit()
 * @brief Function for push diag data to queue and process queued diag messages
 * Here two types of Diag pointers are pushed to queue.
 * 1) malloc is done for initial messages when diag service is not up.
 * 2) Once Diag service is up log_alloc pointers are sent to the message queue
 */
void LocDiagIface::logCommit(void *pData, diagBuffSrc bufferSrc,
        unsigned int diagId, size_t size)
{
    LOC_LOGV("[%s] diagId: 0x%X bufferSrc: %d",
             __func__, diagId, bufferSrc);
    if (BUFFER_FROM_DIAG == bufferSrc) {
        if (true == mDiagBuffActive) {
            flushBuffer();
            mDiagBuffActive = false;
        }
        log_commit(pData);
    } else if (BUFFER_FROM_MALLOC == bufferSrc) {
        /**Post the message to queue*/
        if (mQ) {
            msg_q_snd(mQ, new DiagCommitMsg(pData, diagId, bufferSrc, size), nullptr);
        } else {
            LOC_LOGE("[%s] Error msg thread is not created", __func__);
        }
    }
}

/*!
 * @function: LocGetDiagBuffer()
 * @brief  Allocate Buffer from Diag log_alloc or malloc based on Diag service availablity
 */
void* LocDiagIface::logAlloc(unsigned int diagId, size_t size, diagBuffSrc *bufferSrc)
{
    void *pData = nullptr;
    *bufferSrc = BUFFER_INVALID;
    int64_t upTime = uptimeMillis();

    // we allow at least five retry and until the buffered time out
    // each retry will be at least 1000 milli-seconds apart
    if ((false == mIsDiagInitialized) &&
        (upTime - mLastDiagInitFailedBoottime) > 1000 &&
        ((upTime < mTimeOut) || (mLastDiagInitCount < 5))) {
        mIsDiagInitialized = Diag_LSM_Init(NULL);
        if (mIsDiagInitialized == false) {
            mLastDiagInitFailedBoottime = uptimeMillis();
            mLastDiagInitCount++;
        }
    }

    if (true == mIsDiagInitialized) {
        pData = log_alloc(diagId, size);
    }
    /**If pData is not nullptr then diag service is up/message mask is enabled
    * so stop msg buffering */
    if (pData != nullptr) {
        *bufferSrc = BUFFER_FROM_DIAG;
    } else if ((pData == nullptr) && (1 == mIsDiagBufferingEnabled) && (upTime < mTimeOut)) {
    /**malloc is done only when buffering is active i.e. first 30 seconds/Diag service is
    * failing to alloc memory*/
        LOC_LOGE("[%s] Allocating memory using malloc", __func__);
        pData =  malloc(size);
        *bufferSrc = BUFFER_FROM_MALLOC;
        mDiagBuffActive = true;
    }
    return(pData);
}

extern "C" {
    LocDiagIface* getLocDiagIface() {
        return LocDiagIface::getInstance();
    }
}
