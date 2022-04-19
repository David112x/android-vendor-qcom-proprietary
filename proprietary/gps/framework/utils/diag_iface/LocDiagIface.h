/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/
#ifndef __LOC_DIAG_IFACE__
#define __LOC_DIAG_IFACE__

#include <stdlib.h>
#include <stdint.h>
#include <loc_pla.h>
#include <log.h>
#include <log_codes.h>
#include <MsgTask.h>

using namespace std;

#define DIAG_MSG_BUFFER_WAIT_TIMEOUT (30*1000)
/**This is to indicate if the buffer is allocated from Diag or malloc.
* This is needed for cases where Diag service is not up
*/
typedef enum
{
   BUFFER_INVALID = 0,
   BUFFER_FROM_MALLOC, /**The diag buffer is allocated from heap using malloc.
                        * Used for diag messages that are generated prio to DIag
                        * susbsystem being initialised*/
   BUFFER_FROM_DIAG    /**The diag buffer is allocated from diag heap using log_alloc.*/
} diagBuffSrc;

struct DiagCommitMsg {
    void  *mCommitPtr;
    unsigned int mDiagId;
    diagBuffSrc mBufferSrc;
    size_t mSize;
    inline DiagCommitMsg(void *pLogCommitPtr, unsigned int msgId,
            diagBuffSrc src, size_t datasize)
    {
        mCommitPtr = pLogCommitPtr;
        mBufferSrc = src;
        mDiagId = msgId;
        mSize = datasize;
    }
    /**Free up malloc pointers in destructor log_alloc pointers are cleared in log_commit*/
    inline ~DiagCommitMsg() {
        if (mBufferSrc == BUFFER_FROM_MALLOC) {
            if (mCommitPtr != nullptr) {
                free(mCommitPtr);
            }
        }
    }
};

class LocDiagIface {
public:
    // singleton instance
    static LocDiagIface* getInstance();
    void destroy();
    void* logAlloc(uint32_t diagId, size_t size, diagBuffSrc *bufferSrc);
    void logCommit(void *pData, diagBuffSrc bufferSrc, uint32_t diagId, size_t size);
private:
    static LocDiagIface *mInstance;
    bool                mDiagBuffActive;
    uint32_t            mTimeOut;
    bool                mIsDiagInitialized;
    void*               mQ;
    int64_t             mLastDiagInitFailedBoottime;
    int                 mLastDiagInitCount;
    uint32_t            mIsDiagBufferingEnabled;

    void flushBuffer();
    LocDiagIface();
    ~LocDiagIface();
};
#endif /**__LOC_DIAG_IFACE__*/
