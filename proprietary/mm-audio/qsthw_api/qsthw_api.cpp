/* qsthw_api.cpp
 *
 * Copyright (c) 2016, 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

#define LOG_TAG "qsthw_api"
/* #define LOG_NDEBUG 0 */
#define LOG_NDDEBUG 0

/* #define VERY_VERBOSE_LOGGING */
#ifdef VERY_VERBOSE_LOGGING
#define ALOGVV ALOGV
#else
#define ALOGVV(a...) do { } while(0)
#endif

#include <utils/Errors.h>
#include <dlfcn.h>
#include <errno.h>
#include <stdlib.h>
#include <cutils/log.h>
#include <cutils/list.h>
#include <tinyalsa/asoundlib.h>
#include <hardware/sound_trigger.h>
#include <assert.h>
#include <cutils/properties.h>

#include "qsthw_api.h"
#include "qsthw.h"
#include "sound_trigger_prop_intf.h"

#if QTI_AUDIO_SERVER_ENABLED
#include <qti_sva_server.h>
#include <qti_audio_server_client.h>

using namespace svahal;
extern struct listnode sm_list;
extern pthread_mutex_t sm_list_lock;

/* Flag to indicate if QAS is enabled or not */
bool g_binder_en = false;
/* QTI audio server handle */
sp<Iqti_sva_server> g_qss = NULL;
/* Handle for client context*/
void* g_ctxt = NULL;
/* Death notification handle */
sp<death_notifier> g_death_notifier = NULL;
/* Client callback handle */
qsthw_error_callback g_qsthw_err_cb = NULL;
/* Flag to indicate qas status */
bool g_qss_died = false;
/* Count how many times hal is loaded */
static unsigned int g_qss_load_count;
/* Store HAL handle */
qsthw_module_handle_t *g_qss_handle = NULL;

void death_notifier::binderDied(const wp<IBinder>& who)
{
    struct listnode *node;
    p_sound_model *handle;

    if (g_qsthw_err_cb) {
        ALOGD("%s %d", __func__, __LINE__);
        g_qsthw_err_cb(g_ctxt);
    }
    g_qss_died = true;

    pthread_mutex_lock(&sm_list_lock);
    list_for_each(node, &sm_list) {
        handle = node_to_item(node, p_sound_model, list);
         if (handle != NULL) {
            sva_sh_mem_data *shmem_data = handle->shmem_data;
            if (shmem_data != NULL) {
                ALOGD("%s: %d: signal to unblock any wait conditions", __func__, __LINE__);
                pthread_cond_signal(&shmem_data->c_cond);
                shmem_data->status = 0;
            }
        }
    }
    pthread_mutex_unlock(&sm_list_lock);
}

void qsthw_register_qas_death_notify_cb(qsthw_error_callback cb,
                        void* context)
{
    ALOGD("%s %d", __func__, __LINE__);
    g_qsthw_err_cb = cb;
    g_ctxt = context;
}

death_notifier::death_notifier()
{
    ALOGV("%s %d", __func__, __LINE__);
    sp<ProcessState> proc(ProcessState::self());
    proc->startThreadPool();
}

sp<Iqti_sva_server> get_qti_sva_server()
{
    sp<IServiceManager> sm;
    sp<IBinder> binder;
    int retry_cnt = 5;

    if (g_qss == 0) {
        sm = defaultServiceManager();
        if (sm != NULL) {
            do {
                binder = sm->getService(String16(QTI_SVA_SERVER));
                if (binder != 0)
                    break;
                else
                    ALOGE("%d:%s: get qas service failed",__LINE__, __func__);

                 ALOGW("qti_sva_server not published, waiting...");
                usleep(500000);
            } while (--retry_cnt);
        } else {
            ALOGE("%d:%s: defaultServiceManager failed",__LINE__, __func__);
        }
        if (binder == NULL)
            return NULL;

        if (g_death_notifier == NULL) {
            g_death_notifier = new death_notifier();
            if (g_death_notifier == NULL) {
                ALOGE("%d: %s() unable to allocate death notifier", __LINE__, __func__);
                return NULL;
            }
        }
        binder->linkToDeath(g_death_notifier);
        g_qss = interface_cast<Iqti_sva_server>(binder);
        assert(g_qss != 0);
    }
    return g_qss;
}

int qsthw_get_version()
{
    ALOGV("%d:%s",__LINE__, __func__);
    if (g_binder_en) {
        if (!g_qss_died) {
            sp<Iqti_sva_server> qss = get_qti_sva_server();
            if (qss == 0) {
               ALOGE("%d:%s: invalid HAL handle",__LINE__, __func__);
               return -ENODEV;
            }
            return qss->qsthw_get_version();
        } else {
            return -ENODEV;
        }
    } else {
        return qsthw_get_version_l();
    }
}

qsthw_module_handle_t *qsthw_load_module(const char *hw_module_id)
{
    ALOGV("%d:%s",__LINE__, __func__);
    g_binder_en = property_get_bool("persist.vendor.audio.qas.enabled", false);
    ALOGD("%d:%s: g_binder_enabled %d",__LINE__, __func__, g_binder_en);
    if (g_binder_en) {
        sp<Iqti_sva_server> qss = get_qti_sva_server();
        if (qss == 0) {
           ALOGE("%d:%s: invalid HAL handle",__LINE__, __func__);
           return (void*)(-ENODEV);
        }
        g_qss_handle = qss->qsthw_load_module(hw_module_id);
        if (g_qss_handle == NULL) {
            ALOGE("%s: HAL loading failed", __func__);
        } else if (g_qss_load_count == 0) {
            g_qss_load_count++;
            g_qss_died = false;
            pthread_mutex_init(&sm_list_lock, (const pthread_mutexattr_t *) NULL);
            list_init(&sm_list);
            ALOGV("%s %d: sm_list %p", __func__, __LINE__, sm_list);
        } else {
            g_qss_load_count++;
            ALOGD("%s: returning existing instance of hal", __func__);
        }
    } else {
        g_qss_handle = qsthw_load_module_l(hw_module_id);
    }
    return g_qss_handle;
}

int qsthw_unload_module(const qsthw_module_handle_t *mod_handle)
{
    int rc = -EINVAL;
    ALOGV("%d:%s",__LINE__, __func__);
    if (g_binder_en) {
        if (!g_qss_died && ((g_qss_load_count > 0) && (--g_qss_load_count == 0))) {
            sp<Iqti_sva_server> qss = get_qti_sva_server();
            if (qss == 0) {
               ALOGE("%d:%s: invalid HAL handle",__LINE__, __func__);
               return -ENODEV;
            }
            pthread_mutex_destroy(&sm_list_lock);
            rc = qss->qsthw_unload_module(mod_handle);
            if (g_death_notifier != NULL) {
                IInterface::asBinder(qss)->unlinkToDeath(g_death_notifier);
                g_death_notifier.clear();
            }
            g_qss = NULL;
            return rc;
        } else {
            return -ENODEV;
        }
    } else {
        return qsthw_unload_module_l(mod_handle);
    }
}

int qsthw_get_properties(const qsthw_module_handle_t *mod_handle,
                        struct sound_trigger_properties *properties)
{
    ALOGV("%d:%s",__LINE__, __func__);
    if (g_binder_en) {
        if (!g_qss_died) {
            sp<Iqti_sva_server> qss = get_qti_sva_server();
            if (qss == 0) {
               ALOGE("%d:%s: invalid HAL handle",__LINE__, __func__);
               return -ENODEV;
            }
            return qss->qsthw_get_properties(mod_handle, properties);
        } else {
            return -ENODEV;
        }
    } else {
        return qsthw_get_properties_l(mod_handle, properties);
    }
}

int qsthw_load_sound_model(const qsthw_module_handle_t *mod_handle,
                        struct sound_trigger_sound_model *sound_model,
                        qsthw_sound_model_callback_t callback,
                        void *cookie,
                        sound_model_handle_t *handle)
{
    ALOGV("%d:%s",__LINE__, __func__);
    if (g_binder_en) {
        if (!g_qss_died) {
            sp<Iqti_sva_server> qss = get_qti_sva_server();
            if (qss == 0) {
               ALOGE("%d:%s: invalid HAL handle",__LINE__, __func__);
               return -ENODEV;
            }
            return qss->qsthw_load_sound_model(mod_handle, sound_model,
                                                callback, cookie, handle);
        } else {
            return -ENODEV;
        }
    } else {
        return qsthw_load_sound_model_l(mod_handle, sound_model, callback,
                                        cookie, handle);
    }
}

int qsthw_unload_sound_model(const qsthw_module_handle_t *mod_handle,
                        sound_model_handle_t handle)
{
    ALOGV("%d:%s",__LINE__, __func__);
    if (g_binder_en) {
        if (!g_qss_died) {
            sp<Iqti_sva_server> qss = get_qti_sva_server();
            if (qss == 0) {
               ALOGE("%d:%s: invalid HAL handle",__LINE__, __func__);
               return -ENODEV;
            }
            return qss->qsthw_unload_sound_model(mod_handle, handle);
        } else {
            return -ENODEV;
        }
    } else {
        return qsthw_unload_sound_model_l(mod_handle, handle);
    }
}

int qsthw_start_recognition(const qsthw_module_handle_t *mod_handle,
                        sound_model_handle_t sound_model_handle,
                        const struct sound_trigger_recognition_config *config,
                        qsthw_recognition_callback_t callback,
                        void *cookie)
{
    ALOGV("%d:%s",__LINE__, __func__);
    if (g_binder_en) {
        if (!g_qss_died) {
            sp<Iqti_sva_server> qss = get_qti_sva_server();
            if (qss == 0) {
               ALOGE("%d:%s: invalid HAL handle",__LINE__, __func__);
               return -ENODEV;
            }
            return qss->qsthw_start_recognition(mod_handle, sound_model_handle,
                                                config, callback, cookie);
        } else {
            return -ENODEV;
        }
    } else {
        return qsthw_start_recognition_l(mod_handle, sound_model_handle,
                                        config, callback, cookie);
    }
}

int qsthw_stop_recognition(const qsthw_module_handle_t *mod_handle,
                        sound_model_handle_t sound_model_handle)
{
    ALOGV("%d:%s",__LINE__, __func__);
    if (g_binder_en) {
        if (!g_qss_died) {
            sp<Iqti_sva_server> qss = get_qti_sva_server();
            if (qss == 0) {
               ALOGE("%d:%s: invalid HAL handle",__LINE__, __func__);
               return -ENODEV;
            }
            return qss->qsthw_stop_recognition(mod_handle, sound_model_handle);
        } else {
            p_sound_model *p_sm = NULL;
            struct listnode *node = NULL;
            struct listnode *tempnode = NULL;
            p_sound_model *handle = NULL;
            pthread_mutex_lock(&sm_list_lock);
            list_for_each_safe(node, tempnode, &sm_list) {
                handle = node_to_item(node, p_sound_model, list);
                if (handle != NULL && handle->sm_handle == sound_model_handle) {
                    p_sm = handle;
                    ALOGD("%s: Free up memory p_sm %p", __func__, p_sm);
                    p_sm->sh_mem_dealer.clear();
                    p_sm->sh_mem_handle.clear();
                }
                list_remove(node);
                free(node_to_item(node, p_sound_model, list));
            }
            pthread_mutex_unlock(&sm_list_lock);
            return -ENODEV;
        }
    } else {
        return qsthw_stop_recognition_l(mod_handle, sound_model_handle);
    }
}

int qsthw_get_param_data(const qsthw_module_handle_t *mod_handle,
                        sound_model_handle_t sound_model_handle,
                        const char *param,
                        void *payload,
                        size_t payload_size,
                        size_t *param_data_size)
{
    ALOGV("%d:%s",__LINE__, __func__);
    if (g_binder_en) {
        if (!g_qss_died) {
            sp<Iqti_sva_server> qss = get_qti_sva_server();
            if (qss == 0) {
               ALOGE("%d:%s: invalid HAL handle",__LINE__, __func__);
               return -ENODEV;
            }
            return qss->qsthw_get_param_data(mod_handle, sound_model_handle,
                                            param, payload, payload_size,
                                            param_data_size);
        } else {
            return -ENODEV;
        }
    } else {
        return qsthw_get_param_data_l(mod_handle, sound_model_handle,
                                    param, payload, payload_size,
                                    param_data_size);
    }
}

int qsthw_set_parameters(const qsthw_module_handle_t *mod_handle,
                        sound_model_handle_t sound_model_handle,
                        const char *kv_pairs)
{
    ALOGV("%d:%s",__LINE__, __func__);
    if (g_binder_en) {
        if (!g_qss_died) {
            sp<Iqti_sva_server> qss = get_qti_sva_server();
            if (qss == 0) {
               ALOGE("%d:%s: invalid HAL handle",__LINE__, __func__);
               return -ENODEV;
            }
            return qss->qsthw_set_parameters(mod_handle, sound_model_handle,
                                            kv_pairs);
        } else {
            return -ENODEV;
        }
    } else {
        return qsthw_set_parameters_l(mod_handle, sound_model_handle, kv_pairs);
    }
}

size_t qsthw_get_buffer_size(const qsthw_module_handle_t *mod_handle,
                        sound_model_handle_t sound_model_handle)
{
    ALOGV("%d:%s",__LINE__, __func__);
    if (g_binder_en) {
        if (!g_qss_died) {
            sp<Iqti_sva_server> qss = get_qti_sva_server();
            if (qss == 0) {
               ALOGE("%d:%s: invalid HAL handle",__LINE__, __func__);
               return -ENODEV;
            }
            return qss->qsthw_get_buffer_size(mod_handle, sound_model_handle);
        } else {
            return -ENODEV;
        }
    } else {
        return qsthw_get_buffer_size_l(mod_handle, sound_model_handle);
    }
}

int qsthw_read_buffer(const qsthw_module_handle_t *mod_handle,
                        sound_model_handle_t sound_model_handle,
                        unsigned char *buf,
                        size_t bytes)
{
    ALOGV("%d:%s",__LINE__, __func__);
    if (g_binder_en) {
        if (!g_qss_died) {
            sp<Iqti_sva_server> qss = get_qti_sva_server();
            if (qss == 0) {
               ALOGE("%d:%s: invalid HAL handle",__LINE__, __func__);
               return -ENODEV;
            }
            return qss->qsthw_read_buffer(mod_handle, sound_model_handle,
                                            buf, bytes);
        } else {
            return -ENODEV;
        }
    } else {
        return qsthw_read_buffer_l(mod_handle, sound_model_handle, buf, bytes);
    }
}

int qsthw_stop_buffering(const qsthw_module_handle_t *mod_handle,
                        sound_model_handle_t sound_model_handle)
{
    ALOGV("%d:%s",__LINE__, __func__);
    if (g_binder_en) {
        if (!g_qss_died) {
            sp<Iqti_sva_server> qss = get_qti_sva_server();
            if (qss == 0) {
               ALOGE("%d:%s: invalid HAL handle",__LINE__, __func__);
               return -ENODEV;
            }
            return qss->qsthw_stop_buffering(mod_handle, sound_model_handle);
        } else {
            return -ENODEV;
        }
    } else {
        return qsthw_stop_buffering_l(mod_handle, sound_model_handle);
    }
}
#else
void qsthw_register_qas_death_notify_cb(qsthw_error_callback cb __unused,
                        void* context __unused)
{
}

int qsthw_get_version()
{
    ALOGV("%d:%s",__LINE__, __func__);
    return qsthw_get_version_l();
}

qsthw_module_handle_t *qsthw_load_module(const char *hw_module_id)
{
    ALOGV("%d:%s",__LINE__, __func__);
    return qsthw_load_module_l(hw_module_id);
}

int qsthw_unload_module(const qsthw_module_handle_t *mod_handle)
{
    ALOGV("%d:%s",__LINE__, __func__);
    return qsthw_unload_module_l(mod_handle);
}

int qsthw_get_properties(const qsthw_module_handle_t *mod_handle,
                        struct sound_trigger_properties *properties)
{
    ALOGV("%d:%s",__LINE__, __func__);
    return qsthw_get_properties_l(mod_handle, properties);
}

int qsthw_load_sound_model(const qsthw_module_handle_t *mod_handle,
                        struct sound_trigger_sound_model *sound_model,
                        qsthw_sound_model_callback_t callback,
                        void *cookie,
                        sound_model_handle_t *handle)
{
    ALOGV("%d:%s",__LINE__, __func__);
    return qsthw_load_sound_model_l(mod_handle, sound_model, callback,
                                    cookie, handle);
}

int qsthw_unload_sound_model(const qsthw_module_handle_t *mod_handle,
                        sound_model_handle_t handle)
{
    ALOGV("%d:%s",__LINE__, __func__);
    return qsthw_unload_sound_model_l(mod_handle, handle);
}

int qsthw_start_recognition(const qsthw_module_handle_t *mod_handle,
                        sound_model_handle_t sound_model_handle,
                        const struct sound_trigger_recognition_config *config,
                        qsthw_recognition_callback_t callback,
                        void *cookie)
{
    ALOGV("%d:%s",__LINE__, __func__);
    return qsthw_start_recognition_l(mod_handle, sound_model_handle,
                                    config, callback, cookie);
}

int qsthw_stop_recognition(const qsthw_module_handle_t *mod_handle,
                        sound_model_handle_t sound_model_handle)
{
    ALOGV("%d:%s",__LINE__, __func__);
    return qsthw_stop_recognition_l(mod_handle, sound_model_handle);
}

int qsthw_get_param_data(const qsthw_module_handle_t *mod_handle,
                        sound_model_handle_t sound_model_handle,
                        const char *param,
                        void *payload,
                        size_t payload_size,
                        size_t *param_data_size)
{
    ALOGV("%d:%s",__LINE__, __func__);
    return qsthw_get_param_data_l(mod_handle, sound_model_handle,
                                param, payload, payload_size,
                                param_data_size);
}

int qsthw_set_parameters(const qsthw_module_handle_t *mod_handle,
                        sound_model_handle_t sound_model_handle,
                        const char *kv_pairs)
{
    ALOGV("%d:%s",__LINE__, __func__);
    return qsthw_set_parameters_l(mod_handle, sound_model_handle, kv_pairs);
}

size_t qsthw_get_buffer_size(const qsthw_module_handle_t *mod_handle,
                        sound_model_handle_t sound_model_handle)
{
    ALOGV("%d:%s",__LINE__, __func__);
    return qsthw_get_buffer_size_l(mod_handle, sound_model_handle);
}

int qsthw_read_buffer(const qsthw_module_handle_t *mod_handle,
                        sound_model_handle_t sound_model_handle,
                        unsigned char *buf,
                        size_t bytes)
{
    ALOGV("%d:%s",__LINE__, __func__);
    return qsthw_read_buffer_l(mod_handle, sound_model_handle, buf, bytes);
}

int qsthw_stop_buffering(const qsthw_module_handle_t *mod_handle,
                        sound_model_handle_t sound_model_handle)
{
    ALOGV("%d:%s",__LINE__, __func__);
    return qsthw_stop_buffering_l(mod_handle, sound_model_handle);
}

#endif
