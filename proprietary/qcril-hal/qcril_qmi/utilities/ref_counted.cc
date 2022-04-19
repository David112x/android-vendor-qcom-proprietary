/******************************************************************************
#  Copyright (c) 2015, 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <QtiMutex.h>
#include "qcril_memory_management.h"
#include "ref_counted.h"
#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

struct ref_counted_struct
{
    qtimutex::QtiRecursiveMutex ref_cnt_mutex;
    unsigned long ref_cnt;
    ref_counted_delete_cb delete_cb;
};

void ref_counted_default_free(void *strct, ref_counted *rfcnt)
{
    if (!strct || !rfcnt) return;

    free(strct);
}
void ref_counted_init(void *strct, ref_counted **ref_cnt, ref_counted_delete_cb delete_cb)
{
    ref_counted *rfcnt;

    if (!strct || !ref_cnt)
    {
        return;
    }

    rfcnt = qcril_malloc2(rfcnt);
    if(rfcnt)
    {
        rfcnt->ref_cnt = 1;
        rfcnt->delete_cb = delete_cb;
    }

    if (ref_cnt)
    {
        *ref_cnt = rfcnt;
    }
}

void * ref_counted_incr(void *strct, ref_counted *rfcnt)
{
    void *ret = NULL;

    if (!strct || !rfcnt)
    {
        return ret;
    }

    rfcnt->ref_cnt_mutex.lock();
    ++rfcnt->ref_cnt;
    rfcnt->ref_cnt_mutex.unlock();
    return strct;
}

unsigned long ref_counted_decr(void *strct, ref_counted *rfcnt)
{
    ref_counted_delete_cb delete_cb;
    int deleted = 0;
    unsigned long ret = 0;

    if (!strct || !rfcnt)
    {
        return ret;
    }

    rfcnt->ref_cnt_mutex.lock();

    if ( (ret = --rfcnt->ref_cnt) == 0 )
    {
        deleted = 1;
    }

    rfcnt->ref_cnt_mutex.unlock();
    if ( deleted )
    {
        delete_cb = rfcnt->delete_cb;
        if (delete_cb)
        {
            delete_cb(strct, rfcnt);
        }
        free(rfcnt);
    }
    return ret;
}
