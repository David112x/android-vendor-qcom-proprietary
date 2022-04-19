/* mct_debug_fdleak.h
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#ifndef FDLEAK_H
#define FDLEAK_H

#include <pthread.h>

#define DUMP_FDLEAK_TRACE 1
#define FDLEAK_EXIT 2

extern pthread_mutex_t mct_debug_fdleak_mut;
extern pthread_cond_t mct_debug_fdleak_con;
extern char mct_debug_fdleak_event;
#ifdef __cplusplus
extern "C" {
#endif
void * mct_debug_fdleak_thread (void * param __unused);
void mct_degug_enable_fdleak_trace();
void mct_degug_dump_fdleak_trace();
#ifdef __cplusplus
}
#endif

#endif
