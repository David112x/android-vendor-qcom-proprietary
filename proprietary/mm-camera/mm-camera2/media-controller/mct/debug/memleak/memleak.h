/* memleak.h
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef MEMLEAK_H
#define MEMLEAK_H

#include <pthread.h>

#define PRINT_LEAK_MEMORY 1
#define MEMLEAK_EXIT 2

extern pthread_mutex_t server_memleak_mut;
extern pthread_cond_t server_memleak_con;
extern char server_memleak_event;
#ifdef __cplusplus
extern "C" {
#endif
void * server_memleak_thread (void * param __unused);
void enable_memleak_trace(int signum __unused);
int mmcamera_stacktrace(uintptr_t *addrs,size_t max_entries);
struct map_info_holder *lib_map_create(pid_t pid);
void print_backtrace(struct map_info_holder *p_map_info, uintptr_t* frames, int frame_count);
void lib_map_destroy(struct map_info_holder *map_hold);
#ifdef __cplusplus
}
#endif

#endif
