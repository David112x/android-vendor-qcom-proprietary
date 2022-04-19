/******************************************************************************
#  Copyright (c) 2016-2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
/*!
  @file
  qcril_signal.c

  @brief
  Enables sending inter-module notifications

*/


#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <QtiMutex.h>
#include "qcril_signal.h"
#include "simple_list.h"
#include "ref_counted.h"
#include "qcril_memory_management.h"

static simple_list *signal_list;
static qtimutex::QtiRecursiveMutex signal_list_mutex;

typedef struct qcril_signal_callback_entry
{
    qcril_signal *signal;
    qcril_signal_callback_t cb;
    void *userdata;
} qcril_signal_callback_entry;

struct qcril_signal
{
    REF_COUNTED_DECL;
    char *name;
    simple_list *callback_list;
    qtimutex::QtiSharedMutex callback_list_mutex;
};

qcril_signal *list;

int qcril_signal_init(void)
{
    signal_list = simple_list_new();
    return 0;
}

static void qcril_signal_delete(void *strct, ref_counted *rfcnt)
{
    //qcril_signal *s = (qcril_signal *)strct;

    if (!strct || !rfcnt)
    {
        return;
    }

    /*TODO: Delete list contents and free*/
    // free(strct->name);
}

static qcril_signal *qcril_signal_new(const char *name)
{
    qcril_signal *ret = NULL;

    if (!name)
    {
        return ret;
    }

    ret = qcril_malloc2(ret);
    if (ret)
    {
        // REFACTOR: enable these macros
        // Options
        // 1 - move to shared ptrs
        // 2 - Use extern and retain C code.
        // 3 - use decltype instead of typeof
        REF_COUNTED_INIT(ret, qcril_signal_delete);
        ret->callback_list = simple_list_new();
        ret->name = strdup(name);
    }

    return ret;
}

qcril_signal *qcril_signal_get(const char *name)
{
    qcril_signal *sig = NULL;
    simple_list_iter *it;

    signal_list_mutex.lock();

    for(it = simple_list_first(signal_list);
        it;
        it = simple_list_iter_next(it))
    {
        sig = (qcril_signal *)simple_list_iter_get_value(it);
        if (sig)
        {
            if (!strcmp(name, sig->name))
            {
                break;
            }
        }
    }

    if (!sig)
    {
        sig = qcril_signal_new(name);
        simple_list_append(signal_list, sig);
    }

    REF_COUNTED_INCR(sig);
    simple_list_iter_done(it);
    signal_list_mutex.unlock();

    return sig;
}
int qcril_signal_add_callback
(
    qcril_signal *signal,
    qcril_signal_callback_t cb,
    void *userdata
)
{
    int ret = 0;
    qcril_signal_callback_entry *cbe;

    if (!signal || ! cb)
    {
        return ret;
    }

    cbe = qcril_malloc2(cbe);
    if (cbe)
    {
        cbe->cb = cb;
        cbe->userdata = userdata;
        cbe->signal = REF_COUNTED_INCR(signal);
        simple_list_append(signal->callback_list, cbe);
    }
    return ret;
}

int qcril_signal_trigger(qcril_signal *signal, ...)
{
    int ret = 0;
    qcril_signal_callback_entry *cbe;
    simple_list_iter *it;
    qcril_signal_args args;

    if (!signal)
    {
        return ret;
    }

    memset(&args, 0, sizeof(args));

    for(it = simple_list_first(signal->callback_list);
        it;
        it = simple_list_iter_next(it))
    {
        cbe = (qcril_signal_callback_entry *)simple_list_iter_get_value(it);
        if (cbe && cbe->cb)
        {
            va_start(args.arg_list, signal);
            cbe->cb(cbe->signal, cbe->userdata, &args);
            va_end(args.arg_list);
        }
    }

    return ret;
}
