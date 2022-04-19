/*========================================================================

*//** @file mmsw_platform.h

@par DESCRIPTION:
      Platform layer for multi-media software architecture (MMSW)

@par EXTERNALIZED FUNCTIONS:
      See below.

    Copyright (c) 2015 Qualcomm Technologies, Inc.  All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.

*//*====================================================================== */

#ifndef MMSW_PLATFORM_H
#define MMSW_PLATFORM_H

#ifdef __cplusplus
extern "C" {
#endif

/*========================================================================
 Includes
 ========================================================================*/

#include <stdarg.h> // For vargs
#include <stdlib.h> // For malloc/free
#include "mmsw_framedescriptor.h"

/*========================================================================
 Pre-processor Definitions
 ========================================================================*/

// For logging
#define MMSWLOG_VERBOSE 1
#define MMSWLOG_SEVERE  2

// **** Uncomment to enable the verbose log
#define MMSW_LOG_ENABLE

#ifdef MMSW_LOG_ENABLE
#define MMSWLOG(level, ...) mmsw_platform_log(0, __FILE__, __LINE__, level, __VA_ARGS__)
#else
#define MMSWLOG(level, ...)
#endif
#define MMSWLOG_FORCE(level, ...) mmsw_platform_log(1, __FILE__, __LINE__, level, __VA_ARGS__)

// Memory allocation
#define mmsw_malloc malloc
#define mmsw_free   free
#define MMSW_UNUSED(x) x=x;

/*========================================================================
 Data Types
 ========================================================================*/

// Semaphore
typedef void* mmsw_thread_safe_queue_t;
typedef void* mmsw_sem_t;

// Thread
typedef void(*mmsw_threadfunc_t)(void(*));
typedef void* mmsw_thread_handle_t;

// Timestamp
typedef void* mmsw_timestamp_t;

/*========================================================================
 Function Prototypes
 ========================================================================*/

// Logging
void mmsw_enable_runtime_log(void);
void mmsw_disable_runtime_log(void);

// Semaphore - General purpose semaphore, can be used for an event, mutex, etc.
mmsw_sem_t mmsw_sem_create(int numReleased);
void mmsw_sem_delete(mmsw_sem_t sem);
void mmsw_sem_acquire(mmsw_sem_t sem);
void mmsw_sem_release(mmsw_sem_t sem, int numReleased);
int mmsw_sem_getvalue(mmsw_sem_t sem);

// Thread
mmsw_thread_handle_t mmsw_thread_create(mmsw_threadfunc_t foo, void* data, unsigned int* threadId);
void mmsw_thread_destroy(mmsw_thread_handle_t threadHandle);

// Timer
unsigned int mmsw_get_elapsed_milliseconds(void);
unsigned int mmsw_get_elapsed_microseconds(void);
mmsw_timestamp_t mmsw_get_current_timestamp(void);
float mmsw_get_millisecond_difference(mmsw_timestamp_t start, mmsw_timestamp_t end);

// String equivalent
void* mmsw_memcpy(void* destination, const void* source, unsigned int num);
int mmsw_strcmp(const char* str1, const char* str2);
int mmsw_snprintf(char *str, size_t size, const char *format, ...);
int mmsw_printf(const char *format, ...);
void* mmsw_memset(void* ptr, int value, unsigned int num);

// Frame allocation - platform specific
unsigned int mmsw_frame_allocate(mmsw_frame_desc_t* frame);
unsigned int mmsw_frame_free(mmsw_frame_desc_t* frame);
void mmsw_frame_cache_invalidate(mmsw_frame_desc_t* frame);
void mmsw_frame_cache_flush(mmsw_frame_desc_t* frame);
unsigned int mmsw_frame_get_size(mmsw_frame_desc_t* frame);

// Screen dimensions
void mmsw_queryScreenDims(unsigned int *width, unsigned int *height, int hdmi);

void mmsw_platform_log(int force, const char* fileName, const int lineNum, int level, const char* format, ...);

#ifdef __cplusplus
}
#endif

#endif // MMSW_PLATFORM_H
