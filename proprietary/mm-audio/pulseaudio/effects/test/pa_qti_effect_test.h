/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Copyright (c) 2017, The Linux Foundation. All rights reserved.
 */

#include "pa_qti_effect.h"
#include "pa_qti_effect_audiosphere.h"
#include "pa_qti_effect_bassboost.h"
#include "pa_qti_effect_equalizer.h"
#include "pa_qti_effect_presetreverb.h"
#include "pa_qti_effect_virtualizer.h"
#include <stdbool.h>
#include <cutils/log.h>
#include <pthread.h>

/* #define TEST_LOG */
#ifdef TEST_LOG
#define print_log ALOGD
#else
#define print_log printf
#endif

#define TTY_CMD_MAX 5

typedef void* (*thread_func_t)(void *);
extern thread_func_t effect_thread_funcs[MAX_SUPPORTED_EFFECTS];

void *bassboost_thread_func(void*);   /* thread main of bassboost effect */
void *virtualizer_thread_func(void*); /* thread main of virtualizer effect */
void *equalizer_thread_func(void*);   /* thread main of equalizer effect */
void *reverb_thread_func(void*);      /* thread main of reverb effect */
void *asphere_thread_func(void*);     /* thread main of audiosphere effect */

void *command_thread_func(void*);

typedef struct {
    pthread_t         effect_thread;
    pthread_attr_t    attr;
    pthread_mutex_t   mutex;
    pthread_cond_t    loop_cond;
    int               effect_type;
    uint32_t          sink_index;
    char              *port_name;
    uint64_t          patch_id;
    uint32_t          device_index;
    int               who_am_i;
    volatile bool     exit;
    int               cmd;
    uint32_t          cmd_code;
    uint32_t          cmd_size;
    void              *cmd_data;
    uint32_t          reply_size;
    void              *reply_data;
    int               default_value;
    bool              default_flag;
} thread_data_t;

typedef struct cmd_data {
    pthread_t         cmd_thread;
    pthread_attr_t    attr;
    volatile bool     exit;
    thread_data_t     **fx_data_ptr;
} cmd_data_t;

typedef struct {
    char *cmd_str;
    uint32_t cmd_code;
    char *cmd_prompt;
} cmd_def_t;

enum {
    EFFECT_TYPE_INVALID = -1,
    EFFECT_TYPE_SINK,
    EFFECT_TYPE_PORT,
    EFFECT_TYPE_LOOPBACK
};

enum {
    GET_MODULE_SUPPORTED_EFFECTS = 1,
    GET_SINK_SUPPORTED_EFFECTS,
    GET_PORT_SUPPORTED_EFFECTS,
    EFFECT_CREATE,
    EFFECT_GET_DESC,
    EFFECT_CMD,
    EFFECT_RELEASE,
    EFFECT_GET_VERSION
};

enum {
    TTY_INVALID = 0,
    TTY_ENABLE,
    TTY_DISABLE,
    TTY_BB_SET_STRENGTH,
    TTY_VT_SET_STRENGTH,
    TTY_EQ_SET_PRESET,
    TTY_EQ_SET_CUSTOM,
    TTY_RB_SET_PRESET,
    TTY_ASPHERE_SET_STRENGTH,
    TTY_EXIT
};

extern thread_data_t *pa_qti_create_effect_thread(thread_func_t, uint32_t);
extern void pa_qti_notify_effect_command(thread_data_t *, int, uint32_t, uint32_t, void *);
