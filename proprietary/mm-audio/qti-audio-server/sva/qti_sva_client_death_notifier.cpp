/* Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#define LOG_TAG "qti_sva_client_death_notifier"
#define ATRACE_TAG (ATRACE_TAG_QAS)
/* #define LOG_NDEBUG 0 */
#define LOG_NDDEBUG 0

#include <cutils/list.h>
#include <signal.h>
#include <pthread.h>
#include <qti_sva_server.h>
#include <../qas_client_death_notifier_common.h>

using namespace android;
using namespace svahal;
extern struct listnode g_client_list;
extern pthread_mutex_t g_client_list_lock;

void qti_audio_server_clean_dead_sva_client(qas_client_info *handle) {
    qas_client_session_data *sess = NULL;
    struct listnode *sess_node = NULL;
    struct listnode *sess_tempnode = NULL;
    int rc = -1;

    list_for_each_safe(sess_node, sess_tempnode, &handle->qas_client_sess_list) {
        sess = node_to_item(sess_node, qas_client_session_data, list);
        if ((sess != NULL) && (sess->sm_handle > 0)) {
            n_sound_model *stream_handle = (n_sound_model *)sess->io_handle;
            /*
             * To Do: Instead of force pthread cancel issue stop cmd to
             * server to exit the thread gracefully and then destroy
             * mutex and condition variables
             */
            if (stream_handle != NULL && stream_handle->thread) {
                ALOGV("%s: cancel record thread", __func__);
                pthread_mutex_lock(&stream_handle->shmem_data->c_mutex);
                rc = pthread_cancel(stream_handle->thread);
                ALOGV("%s: pthread_cancel returned, rc %d", __func__, rc);
                pthread_mutex_unlock(&stream_handle->shmem_data->c_mutex);
                stream_handle->sh_mem_handle.clear();
                free(stream_handle);
            }
            ALOGD("%s: Unload sound model %d", __func__, sess->sm_handle);
            qsthw_unload_sound_model_l(handle->hal_handle, sess->sm_handle);
            list_remove(sess_node);
            free(sess);
        }
    }

     if (handle->hal_handle != NULL) {
        ALOGD("%s: Unload STHAL", __func__);
        qsthw_unload_module_l(handle->hal_handle);
    }
    if (handle->client_death_notifier != NULL) {
        IInterface::asBinder(handle->binder)->unlinkToDeath(handle->client_death_notifier);
        handle->client_death_notifier.clear();
        ALOGV("%s: unlink to death for client %d", __func__, handle->pid);
    }
    ALOGV("%s: Exit", __func__);
}

void qti_audio_server_add_snd_model_handle(
                sound_model_handle_t sm_handle) {

    qas_client_info *client_handle = NULL;
    qas_client_session_data *sess = NULL;
    struct listnode *sess_node = NULL;
    struct listnode *sess_tempnode = NULL;

    client_handle = qti_audio_server_get_client_handle_from_list(IPCThreadState::self()->getCallingPid());
    if (client_handle == NULL) {
        ALOGE("%s: Could not find client handle", __func__);
        goto exit;
    }
    pthread_mutex_lock(&g_client_list_lock);
    sess = (qas_client_session_data *)calloc(1, sizeof(qas_client_session_data));
    if (sess == NULL) {
        ALOGE("%s: Cannot allocate memory to store sthal handle", __func__);
        goto exit;
    }
    sess->sm_handle = sm_handle;
    list_add_tail(&client_handle->qas_client_sess_list, &sess->list);
    ALOGD("%s: Added sm handle %d", __func__, sess->sm_handle);

exit:
    pthread_mutex_unlock(&g_client_list_lock);
}

void qti_audio_server_remove_snd_model_handle(
                sound_model_handle_t sm_handle) {

    qas_client_info *client_handle = NULL;
    qas_client_session_data *sess = NULL;
    struct listnode *sess_node = NULL;
    struct listnode *sess_tempnode = NULL;

    client_handle = qti_audio_server_get_client_handle_from_list(IPCThreadState::self()->getCallingPid());
    if (client_handle == NULL) {
        ALOGE("%s: Could not find client handle", __func__);
        return;
    }

    pthread_mutex_lock(&g_client_list_lock);
    list_for_each_safe(sess_node, sess_tempnode, &client_handle->qas_client_sess_list) {
        sess = node_to_item(sess_node, qas_client_session_data, list);
        if ((sess != NULL) && (sess->sm_handle == sm_handle)) {
            ALOGD("%s: Removed snd model %d from list", __func__, sm_handle);
            list_remove(sess_node);
            free(sess);
            break;
        }
    }
    pthread_mutex_unlock(&g_client_list_lock);
}


void qti_audio_server_add_shmem_handle(sound_model_handle_t sm_handle,
                void *stream) {
    qas_client_info *client_handle = NULL;
    qas_client_session_data *sess = NULL;
    struct listnode *node = NULL;

    client_handle = qti_audio_server_get_client_handle_from_list(IPCThreadState::self()->getCallingPid());
    if (client_handle == NULL) {
        ALOGE("%s: Could not find client handle", __func__);
        return;
    }
    pthread_mutex_lock(&g_client_list_lock);
    list_for_each(node, &client_handle->qas_client_sess_list) {
        sess = node_to_item(node, qas_client_session_data, list);
         if (sess != NULL && sess->sm_handle == sm_handle) {
            sess->io_handle = stream;
            ALOGD("%s: Added shared mem handle %p to list", __func__,
                sess->io_handle);
            break;
        }
    }
    pthread_mutex_unlock(&g_client_list_lock);
}

void qti_audio_server_remove_shmem_handle(sound_model_handle_t sm_handle) {
    qas_client_info *client_handle = NULL;
    qas_client_session_data *sess = NULL;
    struct listnode *sess_node = NULL;
    struct listnode *sess_tempnode = NULL;
    struct listnode *node = NULL;

    client_handle = qti_audio_server_get_client_handle_from_list(IPCThreadState::self()->getCallingPid());
    if (client_handle == NULL) {
        ALOGE("%s: Could not find client handle", __func__);
        return;
    }
    pthread_mutex_lock(&g_client_list_lock);
    list_for_each_safe(sess_node, sess_tempnode, &client_handle->qas_client_sess_list) {
        sess = node_to_item(node, qas_client_session_data, list);
         if (sess != NULL && sess->sm_handle == sm_handle) {
            sess->io_handle = NULL;
            ALOGD("%s: Removed shared mem handle from list", __func__);
            break;
        }
    }
    pthread_mutex_unlock(&g_client_list_lock);
}
