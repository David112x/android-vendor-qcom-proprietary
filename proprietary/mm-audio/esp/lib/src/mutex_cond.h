/*============================================================================
* Copyright (c) 2017 Qualcomm Technologies, Inc.                             *
* All Rights Reserved.                                                       *
* Confidential and Proprietary - Qualcomm Technologies, Inc.                 *
* ===========================================================================*/
#ifndef _MUTEX_COND_H_
#define _MUTEX_COND_H_

#if defined(__unix__) || defined(__arm__)
#include <unistd.h>
#endif

#if LINUX_ENABLED
#include <linux/time.h>
#endif

#if defined(_POSIX_VERSION) || defined(__arm__)
#include <pthread.h>

#define DECLARE_COND_VAR(c)   pthread_cond_t c;
#define DECLARE_MUTEX(m)      pthread_mutex_t m;
#define DECLARE_TIMEOUT(t)    struct timespec t;

#define MUTEX_INIT(m)         pthread_mutex_init(m, NULL)
#define COND_VAR_INIT(c)      pthread_cond_init(c, NULL)

#define MUTEX_DESTROY(m)      pthread_mutex_destroy(m)
#define COND_VAR_DESTROY(c)   pthread_cond_destroy(c)

#define MUTEX_LOCK(m)         pthread_mutex_lock(m)
#define MUTEX_UNLOCK(m)       pthread_mutex_unlock(m)

#define COND_VAR_BROADCAST(c) pthread_cond_broadcast(c)
#define COND_VAR_SIGNAL(c)    pthread_cond_signal(c)

#define SET_TIMEOUT(t, ms)     set_timeout(t, ms)
#define COND_VAR_WAIT(c, m, t) pthread_cond_timedwait(c, m, t)

int set_timeout(struct timespec *ts, int timeout_ms);

#else /* !defined(_POSIX_VERSION) && !defined(__arm__) */

#define DECLARE_COND_VAR(c)
#define DECLARE_MUTEX(m)
#define DECLARE_TIMEOUT(t)

#define MUTEX_INIT(m)         (0)
#define COND_VAR_INIT(c)      (0)

#define MUTEX_DESTROY(m)      (0)
#define COND_VAR_DESTROY(c)   (0)

#define MUTEX_LOCK(m)         (0)
#define MUTEX_UNLOCK(m)       (0)

#define COND_VAR_BROADCAST(c) (0)
#define COND_VAR_SIGNAL(c)    (0)

#define SET_TIMEOUT(t, ms)     (0)
#define COND_VAR_WAIT(c, m, t) (-1)

#endif /* _POSIX_VERSION */

#endif /* _MUTEX_COND_H_ */
