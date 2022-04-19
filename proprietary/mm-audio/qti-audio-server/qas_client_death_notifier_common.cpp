/* Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#define LOG_TAG "qas_client_death_notifier_common"
#define ATRACE_TAG (ATRACE_TAG_QAS)
/* #define LOG_NDEBUG 0 */
#define LOG_NDDEBUG 0

#include <cutils/list.h>
#include <signal.h>
#include <pthread.h>
#include "qti_audio_server.h"
#include "sva/qti_sva_server.h"
#include "qas_client_death_notifier_common.h"

using namespace android;
using namespace audiohal;
using namespace svahal;

struct listnode g_client_list;
pthread_mutex_t g_client_list_lock;
bool g_client_list_init = false;

qas_client_death_notifier::qas_client_death_notifier(void) {
    ALOGV("%s: %d", __func__, __LINE__);
    sp<ProcessState> proc(ProcessState::self());
    proc->startThreadPool();
}

void qas_client_death_notifier::binderDied(const wp<IBinder>& who) {
    qas_client_info *handle = NULL;
    struct listnode *node = NULL;
    struct listnode *tempnode = NULL;

    ALOGD("%s: Client Died, who %p", __func__, who.unsafe_get());
    pthread_mutex_lock(&g_client_list_lock);
    list_for_each_safe(node, tempnode, &g_client_list) {
        handle = node_to_item(node, qas_client_info, list);
        if (handle != NULL &&
            (IInterface::asBinder(handle->binder).get() == who.unsafe_get())) {
            /* TO DO: Instead of checking for hal type have fn cb pointer */
            if (handle->hal_type == HAL_HANDLE_TYPE_AHAL)
                qti_audio_server_clean_dead_audio_client(handle);
            else
                qti_audio_server_clean_dead_sva_client(handle);
            list_remove(node);
            free(handle);
        }
    }
    pthread_mutex_unlock(&g_client_list_lock);
    ALOGD("%s: exit", __func__);
}

qas_client_info *qti_audio_server_get_client_handle_from_list(pid_t pid) {
    struct listnode *node = NULL;
    qas_client_info *handle = NULL;

    pthread_mutex_lock(&g_client_list_lock);
    list_for_each(node, &g_client_list) {
        handle = node_to_item(node, qas_client_info, list);
        ALOGD("%s: pid %d, handle->pid %d", __func__, pid, handle->pid);
         if (handle != NULL && handle->pid == pid) {
             ALOGD("%s: Found handle %p", __func__, handle);
             pthread_mutex_unlock(&g_client_list_lock);
             return handle;
        }
    }
    pthread_mutex_unlock(&g_client_list_lock);
    return NULL;
}

void qti_audio_server_add_hal_handle(void *handle, hal_handle_type_t hal_type) {
    qas_client_info *client_handle= NULL;

    client_handle = qti_audio_server_get_client_handle_from_list(IPCThreadState::self()->getCallingPid());
    if (client_handle == NULL) {
        ALOGE("%s: Could not find client handle", __func__);
        return;
    }
    pthread_mutex_lock(&g_client_list_lock);
    client_handle->hal_handle = handle;
    client_handle->hal_type = hal_type;
    ALOGD("%s: Added hal handle %p to list for hal type %d", __func__,
        client_handle->hal_handle, client_handle->hal_type);
    list_init(&client_handle->qas_client_sess_list);
    pthread_mutex_unlock(&g_client_list_lock);
}

void qti_audio_server_remove_hal_handle(void *handle) {
    qas_client_info *client_handle= NULL;
    struct listnode *node = NULL;
    struct listnode *tempnode = NULL;
    qas_client_session_data *sess = NULL;
    struct listnode *sess_node = NULL;
    struct listnode *sess_tempnode = NULL;

    client_handle = qti_audio_server_get_client_handle_from_list(IPCThreadState::self()->getCallingPid());
    if (client_handle == NULL) {
        ALOGE("%s: Could not find client handle", __func__);
        return;
    }

    pthread_mutex_lock(&g_client_list_lock);
    if (client_handle->hal_handle == handle) {
        list_for_each_safe(sess_node, sess_tempnode, &client_handle->qas_client_sess_list) {
            sess = node_to_item(sess_node, qas_client_session_data, list);
            if (sess != NULL) {
                ALOGD("%s: removed sess %p", __func__, sess);
                list_remove(sess_node);
                free(sess);
            }
        }
        client_handle->hal_handle = NULL;
    }
    pthread_mutex_unlock(&g_client_list_lock);
}

int qti_audio_server_unregister_client(sp<IBinder> binder) {
    sp<IQASClient>client_binder = interface_cast<IQASClient>(binder);
    qas_client_info *handle = NULL;
    struct listnode *tempnode = NULL;
    struct listnode *node = NULL;

    ALOGV("%s: enter", __func__);
    pthread_mutex_lock(&g_client_list_lock);
    list_for_each_safe(node, tempnode, &g_client_list) {
        handle = node_to_item(node, qas_client_info, list);
        if (handle != NULL && handle->pid ==
            IPCThreadState::self()->getCallingPid()) {
            if (handle->client_death_notifier != NULL) {
                IInterface::asBinder(client_binder)->unlinkToDeath(handle->client_death_notifier);
                handle->client_death_notifier.clear();
                ALOGV("%s: unlink to death %d", __func__, handle->pid);
            }
            list_remove(node);
            free(handle);
        }
    }
    ALOGV("%s: exit", __func__);
    pthread_mutex_unlock(&g_client_list_lock);
    return 0;
}

int qti_audio_server_register_client(sp<IBinder> binder) {
    pid_t pid = IPCThreadState::self()->getCallingPid();
    sp<IQASClient>client_binder = interface_cast<IQASClient>(binder);
    qas_client_info *client_handle = NULL;
    int rc = 0;
    sp<qas_client_death_notifier> client_death_notifier = NULL;

    client_death_notifier = new qas_client_death_notifier();
    if (client_death_notifier == NULL) {
        ALOGE("%s: Unable to allocate death notifier", __func__);
        rc = -EINVAL;
        goto exit;
    }

    ALOGD("%s: client %p, pid %d", __func__, client_death_notifier.get(), pid);
    IInterface::asBinder(client_binder)->linkToDeath(client_death_notifier);
    if (g_client_list_init == false) {
        pthread_mutex_init(&g_client_list_lock, (const pthread_mutexattr_t *) NULL);
        list_init(&g_client_list);
        g_client_list_init = true;
    }

    client_handle = (qas_client_info *)calloc(1, sizeof(qas_client_info));
    if (client_handle == NULL) {
        ALOGE("%s: Cannot allocate memory for client handle", __func__);
        rc = -EINVAL;
        goto exit;
    }

    pthread_mutex_lock(&g_client_list_lock);
    client_handle->pid = pid;
    client_handle->binder = client_binder;
    client_handle->client_death_notifier = client_death_notifier;
    list_add_tail(&g_client_list, &client_handle->list);
    pthread_mutex_unlock(&g_client_list_lock);

exit:
    return rc;
}
