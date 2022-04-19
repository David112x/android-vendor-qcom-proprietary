/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Copyright (c) 2017, The Linux Foundation. All rights reserved.
 */

#include "pa_qti_effect_test.h"
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>

#define MAX_CMD_STR_SIZE 20
#define NUM_EQ_BANDS 5

const uint16_t pa_qti_equalizer_band_freqs[NUM_EQ_BANDS] = {60, 230, 910, 3600, 14000}; /* frequncies in HZ */

thread_func_t effect_thread_funcs[MAX_SUPPORTED_EFFECTS] = {
    &bassboost_thread_func,
    &virtualizer_thread_func,
    &equalizer_thread_func,
    &reverb_thread_func,
    &asphere_thread_func,
};

cmd_def_t cmds_lookup_table[MAX_SUPPORTED_EFFECTS][TTY_CMD_MAX] = {
    { /* EFFECT_BASSBOOST */
        {"enable",   TTY_ENABLE,               NULL},
        {"disable",  TTY_DISABLE,              NULL},
        {"strength", TTY_BB_SET_STRENGTH,      "input bassboost strength value(0-1000):\n"},
        {"exit",     TTY_EXIT,                 NULL},
        {"invalid",  TTY_INVALID,              NULL},
    },
    { /* EFFECT_VIRTUALIZER */
        {"enable",   TTY_ENABLE,               NULL},
        {"disable",  TTY_DISABLE,              NULL},
        {"strength", TTY_VT_SET_STRENGTH,      "input virtualizer strength value(0-1000):\n"},
        {"exit",     TTY_EXIT,                 NULL},
        {"invalid",  TTY_INVALID,              NULL},
    },
    { /* EFFECT_EQUALIZER */
        {"enable",   TTY_ENABLE,               NULL},
        {"disable",  TTY_DISABLE,              NULL},
        {"preset",   TTY_EQ_SET_PRESET,        "select equalizer presets:\n(0-normal, 1-classical, 2-dance, 3-flat, 4-folk, 5-heavy metal, 6-hiphop, 7-jazz, 8-pop, 9-rock, 10-fx booster)\n"},
        {"custom",   TTY_EQ_SET_CUSTOM,        "customize equalizer settings:\n"},
        {"exit",     TTY_EXIT,                 NULL},
    },
    { /* EFFECT_REVERB */
        {"enable",   TTY_ENABLE,               NULL},
        {"disable",  TTY_DISABLE,              NULL},
        {"preset",   TTY_RB_SET_PRESET,        "select reverb presets:\n(0-none, 1-small room, 2-medium room, 3-large room, 4-medium hall, 5-large hall, 6-plate)\n"},
        {"exit",     TTY_EXIT,                 NULL},
        {"invalid",  TTY_INVALID,              NULL},
    },
    { /* EFFECT_AUDIOSPHERE */
        {"enable",   TTY_ENABLE,               NULL},
        {"disable",  TTY_DISABLE,              NULL},
        {"strength", TTY_ASPHERE_SET_STRENGTH, "input audiosphere strength value(0-1000):\n"},
        {"exit",     TTY_EXIT,                 NULL},
        {"invalid",  TTY_INVALID,              NULL},
    },
};

static int pa_qti_get_key_from_name(int fx_id, const char *name) {
    cmd_def_t *tmp = cmds_lookup_table[fx_id];
    int rc = -EINVAL;
    int i = 0;

    if (name == NULL)
        goto done;

    for (i = 0; i < TTY_CMD_MAX; i++) {
        if (strcmp(tmp[i].cmd_str, name) == 0) {
            rc = tmp[i].cmd_code;
            break;
        }
    }

done:
    return rc;
}

static char *pa_qti_get_prompt_from_name(int fx_id, const char *name) {
    cmd_def_t *tmp = cmds_lookup_table[fx_id];
    char *rc = NULL;
    int i = 0;

    if (name == NULL)
        goto done;

    for (i = 0; i < TTY_CMD_MAX; i++) {
        if (strcmp(tmp[i].cmd_str, name) == 0) {
            rc = tmp[i].cmd_prompt;
            break;
        }
    }

done:
    return rc;
}

static bool pa_qti_is_valid_input(char *inputs) {
    char *input_ptr = inputs;

    if (input_ptr == NULL)
        return false;

    while (*input_ptr == ' ')
        input_ptr++;

    if ((*input_ptr != '\0') && (*input_ptr != '\n'))
        return true;

    return false;
}

void pa_qti_notify_effect_command(thread_data_t *ethread_data,
                                  int cmd,
                                  uint32_t cmd_code,
                                  uint32_t cmd_size,
                                  void *cmd_data) {
    if (ethread_data == NULL) {
        print_log("%s: invalid thread data\n", __func__);
        return;
    }
    print_log("%s\n", __func__);

    /* leave interval to let thread consume the previous cond signal */
    usleep(500000);

    pthread_mutex_lock(&ethread_data->mutex);
    ethread_data->cmd = cmd;
    ethread_data->cmd_code = cmd_code;
    ethread_data->cmd_size = cmd_size;
    ethread_data->cmd_data = cmd_data;
    pthread_cond_signal(&ethread_data->loop_cond);
    pthread_mutex_unlock(&ethread_data->mutex);

    return;
}

static void pa_qti_destroy_effect_thread(thread_data_t *ethread_data) {
    int result = 0;

    if (ethread_data == NULL) {
        print_log("%s: invalid thread data\n", __func__);
        return;
    }

    print_log("%s\n", __func__);

    result = pthread_join(ethread_data->effect_thread, NULL);
    if (result < 0) {
        print_log("%s: Fail to join effect thread!\n", __func__);
        return;
    }
    pthread_mutex_destroy(&ethread_data->mutex);
    pthread_cond_destroy(&ethread_data->loop_cond);

    free(ethread_data);
    ethread_data = NULL;
    pa_qti_effect_deinit();
    return;
}

thread_data_t *pa_qti_create_effect_thread(thread_func_t func_ptr,
                                                  uint32_t effect_idx) {
    int result = 0;

    thread_data_t *ethread_data = (thread_data_t *)calloc(1, sizeof(thread_data_t));
    ethread_data->exit = false;
    ethread_data->who_am_i = effect_idx;
    ethread_data->default_value = -1;
    ethread_data->default_flag = true;
    ethread_data->device_index = -1;

    pthread_attr_init(&ethread_data->attr);
    pthread_attr_setdetachstate(&ethread_data->attr, PTHREAD_CREATE_JOINABLE);
    pthread_mutex_init(&ethread_data->mutex, NULL);
    if (pthread_cond_init(&ethread_data->loop_cond, NULL) != 0) {
        print_log("%s: pthread_cond_init fails\n", __func__);
        return NULL;
    }

    /* create effect thread */
    result = pthread_create(&ethread_data->effect_thread, &ethread_data->attr,
                            func_ptr, ethread_data);

    if (result < 0) {
        print_log("%s: Could not create effect thread!\n", __func__);
        return NULL;
    }
    return ethread_data;
}

void *command_thread_func(void* data) {
    int cmd_key = -1;
    char cmd_str[MAX_CMD_STR_SIZE] = {0};
    int strength = -1;
    uint32_t size = sizeof(pa_qti_effect_param_t) + 3 * sizeof(int32_t);
    uint32_t buf32[size];
    pa_qti_effect_param_t *param = (pa_qti_effect_param_t *)buf32;
    uint32_t size_2 = sizeof(pa_qti_effect_param_t) + 3 * sizeof(int32_t);
    uint32_t buf32_2[size_2];
    pa_qti_effect_param_t *param_2 = (pa_qti_effect_param_t *)buf32_2;
    uint32_t preset = 0;
    uint16_t band_idx = 0;
    int level = -1;
    cmd_data_t *thr_ctxt = (cmd_data_t *)data;
    thread_data_t *ethread_data = *(thr_ctxt->fx_data_ptr);
    char *temp_str, *cmd, *strength_value;

    while (!thr_ctxt->exit) {
        if (fgets(cmd_str, sizeof(cmd_str), stdin) == NULL) {
            printf("read error\n");
            break;
        }
        cmd = strtok_r(cmd_str, "\n", &temp_str);

        if (!pa_qti_is_valid_input(cmd) || !ethread_data)
            continue;

        cmd_key = pa_qti_get_key_from_name(ethread_data->who_am_i, cmd);
        switch(cmd_key) {
            case TTY_ENABLE:
                pa_qti_notify_effect_command(ethread_data, EFFECT_CMD, PA_QTI_EFFECT_CMD_ENABLE, 0, NULL);
                break;
            case TTY_DISABLE:
                pa_qti_notify_effect_command(ethread_data, EFFECT_CMD, PA_QTI_EFFECT_CMD_DISABLE, 0, NULL);
                break;
            case TTY_BB_SET_STRENGTH:
            case TTY_VT_SET_STRENGTH:
            case TTY_ASPHERE_SET_STRENGTH:
            {
                printf("%s", pa_qti_get_prompt_from_name(ethread_data->who_am_i, cmd_str));
                if (fgets(cmd_str, sizeof(cmd_str), stdin) == NULL) {
                    printf("unrecognized strength number!\n");
                    break;
                }

                strength_value = strtok_r(cmd_str, "\n", temp_str);
                strength = atoi(strength_value);
                if ((strength < 0) || (strength > 1000)) {
                    printf("invalid strength number!\n");
                    break;
                }

                param->psize = sizeof(int32_t);
                *(int32_t *)param->data = ((cmd_key == TTY_BB_SET_STRENGTH) ? BASSBOOST_PARAM_STRENGTH :
                                        ((cmd_key == TTY_VT_SET_STRENGTH) ? VIRTUALIZER_PARAM_STRENGTH:
                                        ASPHERE_PARAM_STRENGTH));
                param->vsize = sizeof(int32_t);
                memcpy((param->data + param->psize), &strength, param->vsize);

                pa_qti_notify_effect_command(ethread_data, EFFECT_CMD, PA_QTI_EFFECT_CMD_SET_PARAM, size, param);
                break;
            }
            case TTY_EQ_SET_PRESET:
            {
                printf("%s", pa_qti_get_prompt_from_name(ethread_data->who_am_i, cmd_str));
                if (fgets(cmd_str, sizeof(cmd_str), stdin) == NULL) {
                    printf("unrecognized preset!\n");
                    break;
                }

                strength_value = strtok_r(cmd_str, "\n", temp_str);
                preset = atoi(strength_value);
                if ((preset < EQ_PRESET_NORMAL) || (preset > EQ_PRESET_LAST)) {
                    printf("invalid preset!\n");
                    break;
                }

                param->psize = sizeof(int32_t);
                *(int32_t *)param->data = EQ_PARAM_CUR_PRESET;
                param->vsize = sizeof(int32_t);
                memcpy((param->data + param->psize), &preset, param->vsize);
                pa_qti_notify_effect_command(ethread_data, EFFECT_CMD, PA_QTI_EFFECT_CMD_SET_PARAM, size, param);
                break;
            }
            case TTY_EQ_SET_CUSTOM:
            {
                printf("%s", pa_qti_get_prompt_from_name(ethread_data->who_am_i, cmd_str));
                for (band_idx = 0; band_idx < NUM_EQ_BANDS; ++band_idx) {
                    printf("input level for band (%d - %dHz) (range from -15 to +15):\n",
                            band_idx, pa_qti_equalizer_band_freqs[band_idx]);
                    if (fgets(cmd_str, sizeof(cmd_str), stdin) == NULL) {
                        printf("unrecognized band level!\n");
                        break;
                    }
                    strength_value = strtok_r(cmd_str, "\n", temp_str);
                    level = atoi(strength_value) * 100;
                    if ((level < -1500) || (level > 1500)) {
                        printf("equalizer band level out of range!\n");
                        break;
                    }

                    param_2->psize = 2 * sizeof(int32_t);
                    *(int32_t *)param_2->data = EQ_PARAM_BAND_LEVEL;
                    *((int32_t *)param_2->data + 1) = band_idx;
                    param_2->vsize = sizeof(int32_t);
                    memcpy((param_2->data + param_2->psize), &level, param_2->vsize);

                    pa_qti_notify_effect_command(ethread_data, EFFECT_CMD, PA_QTI_EFFECT_CMD_SET_PARAM, size, param_2);
                }
                break;
            }
            case TTY_RB_SET_PRESET:
            {
                printf("%s", pa_qti_get_prompt_from_name(ethread_data->who_am_i, cmd_str));
                if (fgets(cmd_str, sizeof(cmd_str), stdin) == NULL) {
                    printf("unrecognized preset!\n");
                    break;
                }

                strength_value = strtok_r(cmd_str, "\n", temp_str);
                preset = atoi(strength_value);
                if ((preset < REVERB_PRESET_NONE) || (preset > REVERB_PRESET_LAST)) {
                    printf("invalid preset!\n");
                    break;
                }

                param->psize = sizeof(int32_t);
                *(int32_t *)param->data = REVERB_PARAM_PRESET;
                param->vsize = sizeof(int32_t);
                memcpy((param->data + param->psize), &preset, param->vsize);

                pa_qti_notify_effect_command(ethread_data, EFFECT_CMD, PA_QTI_EFFECT_CMD_SET_PARAM, size, param);
                break;
            }
            case TTY_EXIT:
                thr_ctxt->exit = true;
                pa_qti_notify_effect_command(ethread_data, EFFECT_RELEASE, -1, 0, NULL);
                print_log("Waiting for effect thread exit\n");
                pthread_join(ethread_data->effect_thread, NULL);
                break;
            default:
                printf("Unknown command\n");
                break;
        }
    }
    print_log("%s: Command thread exit\n", __func__);
}

void *bassboost_thread_func(void *data) {
    thread_data_t *thr_ctxt = (thread_data_t *)data;
    pa_qti_effect_handle_t effect_handle = NULL;
    pa_qti_effect_descriptor_t descriptor = {0};
    pa_qti_effect_descriptor_t module_descriptors[MAX_SUPPORTED_EFFECTS] = {0};
    pa_qti_effect_uuid_t sink_uuids[MAX_SUPPORTED_EFFECTS] = {0};
    pa_qti_effect_uuid_t port_uuids[MAX_SUPPORTED_EFFECTS] = {0};
    uint32_t sink_effects = UINT32_MAX;
    uint32_t port_effects = UINT32_MAX;
    uint32_t module_effects = UINT32_MAX;
    int rc = -1;
    int version = 0;
    int reply_data = -1;
    int reply_size = sizeof(int);
    uint32_t array_size = sizeof(pa_qti_effect_param_t) + 2 * sizeof(int32_t);
    uint32_t buf32[array_size];
    pa_qti_effect_param_t *values = NULL;

    print_log("%s\n", __func__);

    pthread_mutex_lock(&thr_ctxt->mutex);
    while (!thr_ctxt->exit) {
        pthread_cond_wait(&thr_ctxt->loop_cond, &thr_ctxt->mutex);
        switch (thr_ctxt->cmd) {
            case GET_MODULE_SUPPORTED_EFFECTS:
                rc = pa_qti_module_get_supported_effects(&module_effects, module_descriptors);
                if (rc != 0)
                    print_log("%s: pa_qti_module_get_supported_effects failed.\n", __func__);
                break;
            case GET_SINK_SUPPORTED_EFFECTS:
                pa_qti_sink_get_supported_effects(thr_ctxt->sink_index, &sink_effects, sink_uuids);
                if (sink_effects == UINT32_MAX)
                    print_log("%s: pa_qti_sink_get_supported_effects failed.\n", __func__);
                break;
            case GET_PORT_SUPPORTED_EFFECTS:
                pa_qti_port_get_supported_effects(thr_ctxt->port_name, &port_effects, &thr_ctxt->device_index, port_uuids);
                if (port_effects == UINT32_MAX)
                    print_log("%s: pa_qti_port_get_supported_effects failed.\n", __func__);
                break;
            case EFFECT_CREATE:
                if (thr_ctxt->effect_type == 0) {
                    effect_handle = pa_qti_sink_effect_create(SL_IID_BASSBOOST_UUID, thr_ctxt->sink_index);
                } else if(thr_ctxt->effect_type == 1) {
                    effect_handle = pa_qti_port_effect_create(SL_IID_BASSBOOST_UUID, thr_ctxt->port_name);
                } else if(thr_ctxt->effect_type == 2) {
                    effect_handle = pa_qti_loopback_effect_create(SL_IID_BASSBOOST_UUID, thr_ctxt->patch_id);
                } else {
                    print_log("%s: Invalid effect type\n", __func__);
                    break;
                }
                if (effect_handle == NULL)
                    print_log("%s: pa_qti_effect_create failed\n", __func__);
                break;
            case EFFECT_GET_DESC:
                rc = pa_qti_effect_get_descriptor(effect_handle, SL_IID_BASSBOOST_UUID, &descriptor);
                if (rc != 0)
                    print_log("%s: pa_qti_effect_get_descriptor returned %d\n", __func__, rc);
                break;
            case EFFECT_CMD:
                thr_ctxt->reply_size = reply_size;
                thr_ctxt->reply_data = (void *)&reply_data;
                rc = pa_qti_effect_command(effect_handle, thr_ctxt->cmd_code,
                                         thr_ctxt->cmd_size, thr_ctxt->cmd_data,
                                         thr_ctxt->reply_size, thr_ctxt->reply_data);
                if (rc != 0)
                    print_log("%s: effect_command() returns %d\n", __func__, rc);

                if (thr_ctxt->default_flag && (thr_ctxt->cmd_code == PA_QTI_EFFECT_CMD_ENABLE)) {
                    if (thr_ctxt->default_value == -1)
                        thr_ctxt->default_value = 600;

                    values = (pa_qti_effect_param_t *)buf32;
                    values->psize = sizeof(int32_t);
                    values->vsize = sizeof(int32_t);
                    *(int32_t *)values->data = BASSBOOST_PARAM_STRENGTH;
                    memcpy((values->data + values->psize), &thr_ctxt->default_value, values->vsize);
                    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                                               array_size, (void *)values,
                                               thr_ctxt->reply_size, thr_ctxt->reply_data);
                    if (rc != 0)
                        print_log("%s: effect_command() returns %d\n", __func__, rc);
                    else
                        thr_ctxt->default_flag = false;
                }
                break;
            case EFFECT_RELEASE:
                if (thr_ctxt->effect_type == 0)
                    rc = pa_qti_sink_effect_release(effect_handle);
                else if(thr_ctxt->effect_type == 1)
                    rc = pa_qti_port_effect_release(effect_handle);
                else if(thr_ctxt->effect_type == 2)
                    rc = pa_qti_loopback_effect_release(effect_handle);
                if (rc != 0) {
                    print_log("%s: pa_qti_effect_release returned %d\n", __func__, rc);
                }
                thr_ctxt->exit = true;
                break;
            case EFFECT_GET_VERSION:
                version = pa_qti_effect_get_version();
                print_log("%s: pa_qti_effect_get_version returned %d\n", __func__, version);
                break;
            default:
                print_log("%s: Unrecognized command\n", __func__);
        }
    }
    pthread_mutex_unlock(&thr_ctxt->mutex);
    print_log("%s: Exiting effect thread\n", __func__);
    pa_qti_destroy_effect_thread(thr_ctxt);
    return NULL;
}

void *virtualizer_thread_func(void *data) {
    thread_data_t *thr_ctxt = (thread_data_t *)data;
    pa_qti_effect_handle_t effect_handle = NULL;
    pa_qti_effect_descriptor_t descriptor = {0};
    pa_qti_effect_descriptor_t module_descriptors[MAX_SUPPORTED_EFFECTS] = {0};
    pa_qti_effect_uuid_t sink_uuids[MAX_SUPPORTED_EFFECTS] = {0};
    pa_qti_effect_uuid_t port_uuids[MAX_SUPPORTED_EFFECTS] = {0};
    uint32_t sink_effects = UINT32_MAX;
    uint32_t port_effects = UINT32_MAX;
    uint32_t module_effects = UINT32_MAX;
    int rc = -1;
    int version = 0;
    int reply_data = -1;
    int reply_size = sizeof(int);
    uint32_t array_size = sizeof(pa_qti_effect_param_t) + 2 * sizeof(int32_t);
    uint32_t buf32[array_size];
    pa_qti_effect_param_t *values = NULL;

    print_log("%s\n", __func__);

    pthread_mutex_lock(&thr_ctxt->mutex);
    while (!thr_ctxt->exit) {
        pthread_cond_wait(&thr_ctxt->loop_cond, &thr_ctxt->mutex);
        switch (thr_ctxt->cmd) {
            case GET_MODULE_SUPPORTED_EFFECTS:
                rc = pa_qti_module_get_supported_effects(&module_effects, module_descriptors);
                if (rc != 0)
                    print_log("%s: pa_qti_module_get_supported_effects failed.\n", __func__);
                break;
            case GET_SINK_SUPPORTED_EFFECTS:
                pa_qti_sink_get_supported_effects(thr_ctxt->sink_index, &sink_effects, sink_uuids);
                if (sink_effects == UINT32_MAX)
                    print_log("%s: pa_qti_sink_get_supported_effects failed.\n", __func__);
                break;
            case GET_PORT_SUPPORTED_EFFECTS:
                pa_qti_port_get_supported_effects(thr_ctxt->port_name, &port_effects, &thr_ctxt->device_index, port_uuids);
                if (port_effects == UINT32_MAX)
                    print_log("%s: pa_qti_port_get_supported_effects failed.\n", __func__);
                break;
            case EFFECT_CREATE:
                if (thr_ctxt->effect_type == 0) {
                    effect_handle = pa_qti_sink_effect_create(SL_IID_VIRTUALIZER_UUID, thr_ctxt->sink_index);
                } else if(thr_ctxt->effect_type == 1) {
                    effect_handle = pa_qti_port_effect_create(SL_IID_VIRTUALIZER_UUID, thr_ctxt->port_name);
                } else if(thr_ctxt->effect_type == 2) {
                    effect_handle = pa_qti_loopback_effect_create(SL_IID_VIRTUALIZER_UUID, thr_ctxt->patch_id);
                } else {
                    print_log("%s: Invalid effect type\n", __func__);
                    break;
                }
                if (effect_handle == NULL)
                    print_log("%s: pa_qti_effect_create failed\n", __func__);
                break;
            case EFFECT_GET_DESC:
                rc = pa_qti_effect_get_descriptor(effect_handle, SL_IID_VIRTUALIZER_UUID, &descriptor);
                if (rc != 0)
                    print_log("%s: pa_qti_effect_get_descriptor returned %d\n", __func__, rc);
                break;
            case EFFECT_CMD:
                thr_ctxt->reply_size = reply_size;
                thr_ctxt->reply_data = (void *)&reply_data;
                rc = pa_qti_effect_command(effect_handle, thr_ctxt->cmd_code,
                                         thr_ctxt->cmd_size, thr_ctxt->cmd_data,
                                         thr_ctxt->reply_size, thr_ctxt->reply_data);
                if (rc != 0)
                    print_log("%s: effect_command() returns %d\n", __func__, rc);

                if (thr_ctxt->default_flag && (thr_ctxt->cmd_code == PA_QTI_EFFECT_CMD_ENABLE)) {
                    if (thr_ctxt->default_value == -1)
                        thr_ctxt->default_value = 600;

                    values = (pa_qti_effect_param_t *)buf32;
                    values->psize = sizeof(int32_t);
                    values->vsize = sizeof(int32_t);
                    *(int32_t *)values->data = VIRTUALIZER_PARAM_STRENGTH;
                    memcpy((values->data + values->psize), &thr_ctxt->default_value, values->vsize);
                    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                                               array_size, (void *)values,
                                               thr_ctxt->reply_size, thr_ctxt->reply_data);
                    if (rc != 0)
                        print_log("%s: effect_command() returns %d\n", __func__, rc);
                    else
                        thr_ctxt->default_flag = false;
                }
                break;
            case EFFECT_RELEASE:
                if (thr_ctxt->effect_type == 0)
                    rc = pa_qti_sink_effect_release(effect_handle);
                else if(thr_ctxt->effect_type == 1)
                    rc = pa_qti_port_effect_release(effect_handle);
                else if(thr_ctxt->effect_type == 2)
                    rc = pa_qti_loopback_effect_release(effect_handle);
                if (rc != 0)
                    print_log("%s: pa_qti_effect_release returned %d\n", __func__, rc);
                thr_ctxt->exit = true;
                break;
            case EFFECT_GET_VERSION:
                version = pa_qti_effect_get_version();
                print_log("%s: pa_qti_effect_get_version returned %d\n", __func__, version);
                break;
            default:
                print_log("%s: Unrecognized command\n", __func__);
        }
    }
    pthread_mutex_unlock(&thr_ctxt->mutex);
    print_log("%s: Exiting effect thread\n", __func__);
    pa_qti_destroy_effect_thread(thr_ctxt);
    return NULL;
}

void *equalizer_thread_func(void *data) {
    thread_data_t *thr_ctxt = (thread_data_t *)data;
    pa_qti_effect_handle_t effect_handle = NULL;
    pa_qti_effect_descriptor_t descriptor = {0};
    pa_qti_effect_descriptor_t module_descriptors[MAX_SUPPORTED_EFFECTS] = {0};
    pa_qti_effect_uuid_t sink_uuids[MAX_SUPPORTED_EFFECTS] = {0};
    pa_qti_effect_uuid_t port_uuids[MAX_SUPPORTED_EFFECTS] = {0};
    uint32_t sink_effects = UINT32_MAX;
    uint32_t port_effects = UINT32_MAX;
    uint32_t module_effects = UINT32_MAX;
    int rc = -1;
    int version = 0;
    int reply_data = -1;
    int reply_size = sizeof(int);
    uint32_t array_size = sizeof(pa_qti_effect_param_t) + 2 * sizeof(int32_t);
    uint32_t buf32[array_size];
    pa_qti_effect_param_t *values = NULL;

    print_log("%s\n", __func__);

    pthread_mutex_lock(&thr_ctxt->mutex);
    while (!thr_ctxt->exit) {
        pthread_cond_wait(&thr_ctxt->loop_cond, &thr_ctxt->mutex);
        switch (thr_ctxt->cmd) {
            case GET_MODULE_SUPPORTED_EFFECTS:
                rc = pa_qti_module_get_supported_effects(&module_effects, module_descriptors);
                if (rc != 0)
                    print_log("%s: pa_qti_module_get_supported_effects failed.\n", __func__);
                break;
            case GET_SINK_SUPPORTED_EFFECTS:
                pa_qti_sink_get_supported_effects(thr_ctxt->sink_index, &sink_effects, sink_uuids);
                if (sink_effects == UINT32_MAX)
                    print_log("%s: pa_qti_sink_get_supported_effects failed.\n", __func__);
                break;
            case GET_PORT_SUPPORTED_EFFECTS:
                pa_qti_port_get_supported_effects(thr_ctxt->port_name, &port_effects, &thr_ctxt->device_index, port_uuids);
                if (port_effects == UINT32_MAX)
                    print_log("%s: pa_qti_port_get_supported_effects failed.\n", __func__);
                break;
            case EFFECT_CREATE:
                if (thr_ctxt->effect_type == 0) {
                    effect_handle = pa_qti_sink_effect_create(SL_IID_EQUALIZER_UUID, thr_ctxt->sink_index);
                } else if(thr_ctxt->effect_type == 1) {
                    effect_handle = pa_qti_port_effect_create(SL_IID_EQUALIZER_UUID, thr_ctxt->port_name);
                } else if(thr_ctxt->effect_type == 2) {
                    effect_handle = pa_qti_loopback_effect_create(SL_IID_EQUALIZER_UUID, thr_ctxt->patch_id);
                } else {
                    print_log("%s: Invalid effect type\n", __func__);
                    break;
                }
                if (effect_handle == NULL)
                    print_log("%s: pa_qti_effect_create failed\n", __func__);
                break;
            case EFFECT_GET_DESC:
                rc = pa_qti_effect_get_descriptor(effect_handle, SL_IID_EQUALIZER_UUID, &descriptor);
                if (rc != 0)
                    print_log("%s: pa_qti_effect_get_descriptor returned %d\n", __func__, rc);
                break;
            case EFFECT_CMD:
                thr_ctxt->reply_size = reply_size;
                thr_ctxt->reply_data = (void *)&reply_data;
                rc = pa_qti_effect_command(effect_handle, thr_ctxt->cmd_code,
                                           thr_ctxt->cmd_size, thr_ctxt->cmd_data,
                                           thr_ctxt->reply_size, thr_ctxt->reply_data);
                if (rc != 0)
                    print_log("%s: effect_command() returns %d\n", __func__, rc);

                if (thr_ctxt->default_flag && (thr_ctxt->cmd_code == PA_QTI_EFFECT_CMD_ENABLE)) {
                    if (thr_ctxt->default_value == -1)
                        thr_ctxt->default_value = 2;

                    values = (pa_qti_effect_param_t *)buf32;
                    values->psize = sizeof(int32_t);
                    values->vsize = sizeof(int32_t);
                    *(int32_t *)values->data = EQ_PARAM_CUR_PRESET;
                    memcpy((values->data + values->psize), &thr_ctxt->default_value, values->vsize);
                    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                                               array_size, (void *)values,
                                               thr_ctxt->reply_size, thr_ctxt->reply_data);
                    if (rc != 0)
                        print_log("%s: effect_command() returns %d\n", __func__, rc);
                    else
                        thr_ctxt->default_flag = false;
                }
                break;
            case EFFECT_RELEASE:
                if (thr_ctxt->effect_type == 0)
                    rc = pa_qti_sink_effect_release(effect_handle);
                else if(thr_ctxt->effect_type == 1)
                    rc = pa_qti_port_effect_release(effect_handle);
                else if(thr_ctxt->effect_type == 2)
                    rc = pa_qti_loopback_effect_release(effect_handle);
                if (rc != 0)
                    print_log("%s: pa_qti_effect_release returned %d\n", __func__, rc);
                    thr_ctxt->exit = true;
                break;
            case EFFECT_GET_VERSION:
                version = pa_qti_effect_get_version();
                print_log("%s: pa_qti_effect_get_version returned %d\n", __func__, version);
                break;
            default:
                print_log("%s: Unrecognized command\n", __func__);
        }
    }
    pthread_mutex_unlock(&thr_ctxt->mutex);
    print_log("%s: Exiting effect thread\n", __func__);
    pa_qti_destroy_effect_thread(thr_ctxt);
    return NULL;
}

void *reverb_thread_func(void *data) {
    thread_data_t *thr_ctxt = (thread_data_t *)data;
    pa_qti_effect_handle_t effect_handle = NULL;
    pa_qti_effect_descriptor_t descriptor = {0};
    pa_qti_effect_descriptor_t module_descriptors[MAX_SUPPORTED_EFFECTS] = {0};
    pa_qti_effect_uuid_t sink_uuids[MAX_SUPPORTED_EFFECTS] = {0};
    pa_qti_effect_uuid_t port_uuids[MAX_SUPPORTED_EFFECTS] = {0};
    uint32_t sink_effects = UINT32_MAX;
    uint32_t port_effects = UINT32_MAX;
    uint32_t module_effects = UINT32_MAX;
    int rc = -1;
    int version = 0;
    int reply_data = -1;
    int reply_size = sizeof(int);
    uint32_t array_size = sizeof(pa_qti_effect_param_t) + 2 * sizeof(int32_t);
    uint32_t buf32[array_size];
    pa_qti_effect_param_t *values = NULL;

    print_log("%s\n", __func__);

    pthread_mutex_lock(&thr_ctxt->mutex);
    while (!thr_ctxt->exit) {
        pthread_cond_wait(&thr_ctxt->loop_cond, &thr_ctxt->mutex);
        switch (thr_ctxt->cmd) {
            case GET_MODULE_SUPPORTED_EFFECTS:
                rc = pa_qti_module_get_supported_effects(&module_effects, module_descriptors);
                if (rc != 0)
                    print_log("%s: pa_qti_module_get_supported_effects failed.\n", __func__);
                break;
            case GET_SINK_SUPPORTED_EFFECTS:
                pa_qti_sink_get_supported_effects(thr_ctxt->sink_index, &sink_effects, sink_uuids);
                if (sink_effects == UINT32_MAX)
                    print_log("%s: pa_qti_sink_get_supported_effects failed.\n", __func__);
                break;
            case GET_PORT_SUPPORTED_EFFECTS:
                pa_qti_port_get_supported_effects(thr_ctxt->port_name, &port_effects, &thr_ctxt->device_index, port_uuids);
                if (port_effects == UINT32_MAX)
                    print_log("%s: pa_qti_port_get_supported_effects failed.\n", __func__);
                break;
            case EFFECT_CREATE:
                if (thr_ctxt->effect_type == 0) {
                    effect_handle = pa_qti_sink_effect_create(SL_IID_INS_PRESETREVERB_UUID, thr_ctxt->sink_index);
                } else if(thr_ctxt->effect_type == 1) {
                    effect_handle = pa_qti_port_effect_create(SL_IID_INS_PRESETREVERB_UUID, thr_ctxt->port_name);
                } else if(thr_ctxt->effect_type == 2) {
                    effect_handle = pa_qti_loopback_effect_create(SL_IID_INS_PRESETREVERB_UUID, thr_ctxt->patch_id);
                } else {
                    print_log("%s: Invalid effect type\n", __func__);
                    break;
                }
                if (effect_handle == NULL)
                    print_log("%s: pa_qti_effect_create failed\n", __func__);
                break;
            case EFFECT_GET_DESC:
                rc = pa_qti_effect_get_descriptor(effect_handle, SL_IID_INS_PRESETREVERB_UUID, &descriptor);
                if (rc != 0)
                    print_log("%s: pa_qti_effect_get_descriptor returned %d\n", __func__, rc);
                break;
            case EFFECT_CMD:
                thr_ctxt->reply_size = reply_size;
                thr_ctxt->reply_data = (void *)&reply_data;
                rc = pa_qti_effect_command(effect_handle, thr_ctxt->cmd_code,
                                           thr_ctxt->cmd_size, thr_ctxt->cmd_data,
                                           thr_ctxt->reply_size, thr_ctxt->reply_data);
                if (rc != 0)
                    print_log("%s: effect_command() returns %d\n", __func__, rc);

                if (thr_ctxt->default_flag && (thr_ctxt->cmd_code == PA_QTI_EFFECT_CMD_ENABLE)) {
                    if (thr_ctxt->default_value == -1)
                        thr_ctxt->default_value = 2;

                    values = (pa_qti_effect_param_t *)buf32;
                    values->psize = sizeof(int32_t);
                    values->vsize = sizeof(int32_t);
                    *(int32_t *)values->data = REVERB_PARAM_PRESET;
                    memcpy((values->data + values->psize), &thr_ctxt->default_value, values->vsize);
                    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                                               array_size, (void *)values,
                                               thr_ctxt->reply_size, thr_ctxt->reply_data);
                    if (rc != 0)
                        print_log("%s: effect_command() returns %d\n", __func__, rc);
                    else
                         thr_ctxt->default_flag = false;
                }
                break;
            case EFFECT_RELEASE:
                if (thr_ctxt->effect_type == 0)
                    rc = pa_qti_sink_effect_release(effect_handle);
                else if(thr_ctxt->effect_type == 1)
                    rc = pa_qti_port_effect_release(effect_handle);
                else if(thr_ctxt->effect_type == 2)
                    rc = pa_qti_loopback_effect_release(effect_handle);
                if (rc != 0)
                    print_log("%s: pa_qti_effect_release returned %d\n", __func__, rc);
                thr_ctxt->exit = true;
                break;
            case EFFECT_GET_VERSION:
                version = pa_qti_effect_get_version();
                print_log("%s: pa_qti_effect_get_version returned %d\n", __func__, version);
                break;
            default:
                print_log("%s: Unrecognized command\n", __func__);
        }
    }
    pthread_mutex_unlock(&thr_ctxt->mutex);
    print_log("%s: Exiting effect thread\n", __func__);
    pa_qti_destroy_effect_thread(thr_ctxt);
    return NULL;
}

void *asphere_thread_func(void *data) {
    thread_data_t *thr_ctxt = (thread_data_t *)data;
    pa_qti_effect_handle_t effect_handle = NULL;
    pa_qti_effect_descriptor_t descriptor = {0};
    pa_qti_effect_descriptor_t module_descriptors[MAX_SUPPORTED_EFFECTS] ={0};
    pa_qti_effect_uuid_t sink_uuids[MAX_SUPPORTED_EFFECTS] = {0};
    pa_qti_effect_uuid_t port_uuids[MAX_SUPPORTED_EFFECTS] = {0};
    uint32_t sink_effects = UINT32_MAX;
    uint32_t port_effects = UINT32_MAX;
    uint32_t module_effects = UINT32_MAX;
    int rc = -1;
    int version = 0;
    int reply_data = -1;
    uint32_t reply_size = sizeof(int);
    uint32_t array_size = sizeof(pa_qti_effect_param_t) + 2 * sizeof(int32_t);
    uint32_t buf32[array_size], buf32_2[array_size];
    pa_qti_effect_param_t *values = NULL;
    int enable = -1;

    pthread_mutex_lock(&thr_ctxt->mutex);
    while (!thr_ctxt->exit) {
        pthread_cond_wait(&thr_ctxt->loop_cond, &thr_ctxt->mutex);
        switch (thr_ctxt->cmd) {
            case GET_MODULE_SUPPORTED_EFFECTS:
                rc = pa_qti_module_get_supported_effects(&module_effects, module_descriptors);
                if (rc != 0)
                    print_log("%s: pa_qti_module_get_supported_effects failed.\n", __func__);
                break;
            case GET_SINK_SUPPORTED_EFFECTS:
                pa_qti_sink_get_supported_effects(thr_ctxt->sink_index, &sink_effects, sink_uuids);
                if (sink_effects == UINT32_MAX)
                    print_log("%s: pa_qti_sink_get_supported_effects failed.\n", __func__);
                break;
            case GET_PORT_SUPPORTED_EFFECTS:
                pa_qti_port_get_supported_effects(thr_ctxt->port_name, &port_effects, &thr_ctxt->device_index, port_uuids);
                if (port_effects == UINT32_MAX)
                    print_log("%s: pa_qti_port_get_supported_effects failed.\n", __func__);
                break;
            case EFFECT_CREATE:
                if (thr_ctxt->effect_type == 0) {
                    effect_handle = pa_qti_sink_effect_create(SL_IID_AUDIOSPHERE_UUID, thr_ctxt->sink_index);
                } else if(thr_ctxt->effect_type == 1) {
                    effect_handle = pa_qti_port_effect_create(SL_IID_AUDIOSPHERE_UUID, thr_ctxt->port_name);
                } else if(thr_ctxt->effect_type == 2) {
                    effect_handle = pa_qti_loopback_effect_create(SL_IID_AUDIOSPHERE_UUID, thr_ctxt->patch_id);
                } else {
                    print_log("%s: Invalid effect type\n", __func__);
                    break;
                }
                if (effect_handle == NULL)
                    print_log("%s: pa_qti_effect_create failed\n", __func__);
                break;
            case EFFECT_GET_DESC:
                rc = pa_qti_effect_get_descriptor(effect_handle, SL_IID_AUDIOSPHERE_UUID, &descriptor);
                if (rc != 0)
                    print_log("%s: pa_qti_effect_get_descriptor returned %d\n", __func__, rc);
                break;
            case EFFECT_CMD:
                thr_ctxt->reply_size = reply_size;
                thr_ctxt->reply_data = (void *)&reply_data;
                rc = pa_qti_effect_command(effect_handle, thr_ctxt->cmd_code,
                                         thr_ctxt->cmd_size, thr_ctxt->cmd_data,
                                         thr_ctxt->reply_size, thr_ctxt->reply_data);
                if (rc != 0)
                    print_log("%s: effect_command() returns %d\n", __func__, rc);

                if (thr_ctxt->cmd_code == PA_QTI_EFFECT_CMD_ENABLE || thr_ctxt->cmd_code == PA_QTI_EFFECT_CMD_DISABLE) {
                    enable = ((thr_ctxt->cmd_code == PA_QTI_EFFECT_CMD_ENABLE) ? 1 : 0);

                    values = (pa_qti_effect_param_t *)buf32_2;
                    values->psize = 2 * sizeof(int32_t);
                    values->vsize = sizeof(int32_t);
                    *(int32_t *)values->data = ASPHERE_PARAM_ENABLE;
                    memcpy((values->data + values->psize), &enable, values->vsize);
                    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                                             array_size, (void *)values,
                                             thr_ctxt->reply_size, thr_ctxt->reply_data);
                    if (rc != 0)
                        print_log("%s: effect_command() returns %d\n", __func__, rc);
                }
                if (thr_ctxt->default_flag && (thr_ctxt->cmd_code == PA_QTI_EFFECT_CMD_ENABLE)) {
                    if (thr_ctxt->default_value == -1)
                        thr_ctxt->default_value = 600;

                    values = (pa_qti_effect_param_t *)buf32;
                    values->psize = sizeof(int32_t);
                    values->vsize = sizeof(int32_t);
                    *(int32_t *)values->data = ASPHERE_PARAM_STRENGTH;
                    memcpy((values->data + values->psize), &thr_ctxt->default_value, values->vsize);
                    rc = pa_qti_effect_command(effect_handle, PA_QTI_EFFECT_CMD_SET_PARAM,
                                             array_size, (void *)values,
                                             thr_ctxt->reply_size, thr_ctxt->reply_data);
                    if (rc != 0)
                        print_log("%s: effect_command() returns %d\n", __func__, rc);
                    else
                        thr_ctxt->default_flag = false;
                }
                break;
            case EFFECT_RELEASE:
                if (thr_ctxt->effect_type == 0)
                    rc = pa_qti_sink_effect_release(effect_handle);
                else if(thr_ctxt->effect_type == 1)
                    rc = pa_qti_port_effect_release(effect_handle);
                else if(thr_ctxt->effect_type == 2)
                    rc = pa_qti_loopback_effect_release(effect_handle);
                if (rc != 0)
                    print_log("%s: pa_qti_effect_release returned %d\n", __func__, rc);
                thr_ctxt->exit = true;
                break;
            case EFFECT_GET_VERSION:
                version = pa_qti_effect_get_version();
                print_log("%s: pa_qti_effect_get_version returned %d\n", __func__, version);
                break;
            default:
                print_log("%s: Unrecognized command\n", __func__);
        }
    }
    pthread_mutex_unlock(&thr_ctxt->mutex);
    print_log("%s: Exiting effect thread\n", __func__);
    pa_qti_destroy_effect_thread(thr_ctxt);
    return NULL;
}

