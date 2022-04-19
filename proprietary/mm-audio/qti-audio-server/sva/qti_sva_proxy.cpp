/* Copyright (c) 2017 - 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#define LOG_TAG "qti_sva_proxy"
#define ATRACE_TAG (ATRACE_TAG_QAS)
/* #define LOG_NDEBUG 0 */
#define LOG_NDDEBUG 0

#include <stdlib.h>
#include <utils/RefBase.h>
#include <utils/Log.h>
#include <binder/TextOutput.h>
#include <binder/IInterface.h>
#include <binder/IBinder.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <binder/IPCThreadState.h>
#include <binder/MemoryDealer.h>
#include <pthread.h>
#include <qti_sva_server.h>
#include "../qti_audio_client.h"

struct listnode sm_list;
pthread_mutex_t sm_list_lock;
sp<IQASClient> g_sva_binder;

namespace svahal {
using namespace android;

class qti_sva_proxy: public ::android:: BpInterface<Iqti_sva_server> {

public:
qti_sva_proxy(const sp<IBinder>& impl) : BpInterface<Iqti_sva_server>(impl) {
    Parcel data, reply;
    ALOGD("qti_sva_proxy::qti_sva_proxy()");
    sp<IBinder> binder = new QTIClient();
    android::ProcessState::self()->startThreadPool();
    g_sva_binder = interface_cast<IQASClient>(binder);
    data.writeInterfaceToken(Iqti_sva_server::getInterfaceDescriptor());
    data.writeStrongBinder(IInterface::asBinder(g_sva_binder));
    status_t status = remote()->transact(REGISTER_CLIENT, data, &reply);
    if (status)
        ALOGD("qti_sva_proxy: Client registration failed, %d", status);
}

~qti_sva_proxy() {
    Parcel data, reply;
    ALOGD("~qti_sva_proxy::qti_sva_proxy()");
    data.writeInterfaceToken(Iqti_sva_server::getInterfaceDescriptor());
    data.writeStrongBinder(IInterface::asBinder(g_sva_binder));
    status_t status = remote()->transact(UNREGISTER_CLIENT, data, &reply);
    if (status)
        ALOGD("qti_sva_proxy: Unregistering client failed, %d", status);
}

qsthw_module_handle_t *qsthw_load_module(const char *hw_module_id) {
    ALOGV("%s: %d: sva hal string(%s)", __func__, __LINE__, hw_module_id);
    Parcel data, reply;
    qsthw_module_handle_t *handle = NULL;
    data.writeInterfaceToken(Iqti_sva_server::getInterfaceDescriptor());
    data.writeCString(hw_module_id);
    status_t status = remote()->transact(LOAD_SVA_HAL, data, &reply);
    if (status)
        return NULL;
    reply.read(&handle, sizeof(qsthw_module_handle_t *));
    return handle;
}

int qsthw_unload_module(const qsthw_module_handle_t *hw_module) {
    ALOGV("%s %d hal handle(%p)", __func__, __LINE__, hw_module);
    Parcel data, reply;
    data.writeInterfaceToken(Iqti_sva_server::getInterfaceDescriptor());
    data.write(&hw_module, sizeof(qsthw_module_handle_t *));
    status_t status = remote()->transact(UNLOAD_SVA_HAL, data, &reply);
    if (status)
        return status;
    return reply.readInt32();
}

int qsthw_get_version() {
    Parcel data, reply;
    status_t status = remote()->transact(GET_VERSION, data, &reply);
    if (status)
        return status;
    return reply.readInt32();
}

int qsthw_get_properties(const qsthw_module_handle_t *mod_handle,
                         struct sound_trigger_properties *properties) {
    ALOGV("%d:%s",__LINE__, __func__);
    Parcel data, reply;
    data.writeInterfaceToken(Iqti_sva_server::getInterfaceDescriptor());
    data.write(&mod_handle, sizeof(qsthw_module_handle_t *));
    int rc = remote()->transact(GET_PROPERTIES, data, &reply);
    if (rc)
        return rc;

    android::Parcel::ReadableBlob blob;
    uint32_t blob_size;
    reply.readUint32(&blob_size);
    status_t status = reply.readBlob(blob_size, &blob);
    if (status) {
        ALOGE("%s: Error in readblob, status %d", __func__, status);
        return -EINVAL;
    }
    memset(properties, 0x0, sizeof(sound_trigger_properties));
    memscpy(properties, sizeof(sound_trigger_properties), blob.data(), blob_size);
    blob.release();
    return reply.readInt32();
}

int qsthw_load_sound_model(const qsthw_module_handle_t *mod_handle,
                           sound_trigger_sound_model *sound_model,
                           qsthw_sound_model_callback_t callback,
                           void *cookie,
                           sound_model_handle_t *handle) {
    ALOGV("%d:%s",__LINE__, __func__);
    Parcel data, reply;
    int rc = 0;
    int sm_size = 0, tot_size = 0;
    android::Parcel::WritableBlob blob;

    data.writeInterfaceToken(Iqti_sva_server::getInterfaceDescriptor());
    data.write(&mod_handle, sizeof(qsthw_module_handle_t *));

    if (sound_model->type == SOUND_MODEL_TYPE_KEYPHRASE)
        sm_size = sizeof(struct sound_trigger_phrase_sound_model);
    else
        sm_size = sizeof(struct sound_trigger_sound_model);

    tot_size = sm_size + sound_model->data_size;
    data.writeUint32(tot_size);
    status_t status = data.writeBlob(tot_size, false, &blob);
    if (status) {
        ALOGE("%s: Error in writeblob, status %d", __func__, status);
        return -EINVAL;
    }
    memset(blob.data(), 0x0, tot_size);
    memscpy(blob.data(), sm_size, sound_model, sm_size);
    memscpy((((char *)blob.data()) + sound_model->data_offset), sound_model->data_size,
              (((char *)sound_model) + sound_model->data_offset),
              sound_model->data_size);
    blob.release();

    /* To Do: Implement sound model callback */
    data.write(&callback, sizeof(qsthw_sound_model_callback_t)); // cookie1
    data.write(&cookie, sizeof(void *)); // cookie2
    sound_model_handle_t sm_handle = 0;
    rc = remote()->transact(LOAD_SOUND_MODEL, data, &reply);
    if (rc)
        return rc;
    reply.read(&sm_handle, sizeof(sound_model_handle_t));
    *handle = sm_handle;
    return reply.readInt32();
}

int qsthw_unload_sound_model(const qsthw_module_handle_t *mod_handle,
                             sound_model_handle_t handle) {
    ALOGV("%d:%s, %d",__LINE__, __func__, handle);
    Parcel data, reply;
    data.writeInterfaceToken(Iqti_sva_server::getInterfaceDescriptor());
    data.write(&mod_handle, sizeof(qsthw_module_handle_t *));
    data.write(&handle, sizeof(sound_model_handle_t));
    status_t status = remote()->transact(UNLOAD_SOUND_MODEL, data, &reply);
    if (status)
        return status;
    return reply.readInt32();
}

int qsthw_start_recognition(const qsthw_module_handle_t *mod_handle,
                            sound_model_handle_t sound_model_handle,
                            const struct sound_trigger_recognition_config *config,
                            qsthw_recognition_callback_t callback,
                            void *cookie) {
    ALOGV("%d:%s",__LINE__, __func__);
    Parcel data, reply;
    p_sound_model *p_sm = NULL;
    android::Parcel::WritableBlob blob;

    data.writeInterfaceToken(Iqti_sva_server::getInterfaceDescriptor());
    data.write(&mod_handle, sizeof(qsthw_module_handle_t *));
    data.write(&sound_model_handle, sizeof(sound_model_handle_t));

    uint32_t size = sizeof(struct sound_trigger_recognition_config)
                           + config->data_size;
    data.writeUint32(size);
    status_t status = data.writeBlob(size, false, &blob);
    if (status) {
        ALOGE("%s: Error in writeblob, status %d", __func__, status);
        return -EINVAL;
    }
    memset(blob.data(), 0x0, size);
    memscpy(blob.data(), (size - config->data_size), config,
                   (size - config->data_size));
    memscpy(((char *)blob.data()) + config->data_offset, config->data_size,
              ((char *)config) + config->data_offset, config->data_size);

    sp<IBinder> binder = new QTIRecognitionCallback();
    android::ProcessState::self()->startThreadPool();
    sp<IRecognitionCallback> cb = interface_cast<IRecognitionCallback>(binder);
    data.writeStrongBinder(IInterface::asBinder(cb));
    data.write(&callback, sizeof(qsthw_recognition_callback_t *));// cookie1
    data.write(&cookie, sizeof(void *));// cookie2
    int rc = remote()->transact(START_RECOGNITION, data, &reply);
    if (rc) {
        blob.release();
        return rc;
    }
    p_sm = (p_sound_model *)get_sm_handle_from_list(sound_model_handle);
    if (p_sm == NULL) {
        p_sm = (p_sound_model *)calloc(1, sizeof(p_sound_model));
        if (p_sm == NULL) {
            ALOGE("%s: Cannot allocate memory for proxy handle", __func__);
            blob.release();
            goto exit;
        }
        pthread_mutex_lock(&sm_list_lock);
        list_add_tail(&sm_list, &p_sm->list);
        pthread_mutex_unlock(&sm_list_lock);
    }
    p_sm->sm_handle = sound_model_handle;
    p_sm->mod_handle = (qsthw_module_handle_t *)mod_handle;
    reply.read(&p_sm->n_snd_model, sizeof(n_sound_model*));
    blob.release();
    return reply.readInt32();
exit:
    if (p_sm != NULL) {
        p_sm->sh_mem_dealer.clear();
        p_sm->sh_mem_handle.clear();
        free(p_sm);
    }
    ALOGE("%s: failed: unable to open output stream", __func__);
    return -EINVAL;
}

int qsthw_stop_recognition(const qsthw_module_handle_t *mod_handle,
                           sound_model_handle_t sound_model_handle) {
    ALOGV("%d:%s",__LINE__, __func__);
    Parcel data, reply;
    data.writeInterfaceToken(Iqti_sva_server::getInterfaceDescriptor());
    data.write(&mod_handle, sizeof(qsthw_module_handle_t *));
    data.write(&sound_model_handle, sizeof(sound_model_handle_t));
    int rc = remote()->transact(STOP_RECOGNITION, data, &reply);
    if (rc)
        return rc;
    remove_sm_handle_from_list(sound_model_handle);
    return reply.readUint32();
}

int qsthw_get_param_data(const qsthw_module_handle_t *mod_handle,
                         sound_model_handle_t sound_model_handle,
                         const char *param,
                         void *payload,
                         size_t payload_size,
                         size_t *param_data_size) {
    ALOGV("%d:%s",__LINE__, __func__);
    Parcel data, reply;
    android::Parcel::ReadableBlob blob;
    uint32_t blob_size;

    data.writeInterfaceToken(Iqti_sva_server::getInterfaceDescriptor());
    data.write(&mod_handle, sizeof(qsthw_module_handle_t *));
    data.write(&sound_model_handle, sizeof(sound_model_handle_t));
    data.writeCString(param);
    data.write(&payload_size, sizeof(size_t));
    int rc = remote()->transact(GET_PARAM_DATA, data, &reply);
    if (rc)
        return rc;
    reply.read(param_data_size, sizeof(size_t));
    ALOGV("payload_size: %d, param_data_size: %d", payload_size, *param_data_size);
    blob_size = (uint32_t)*param_data_size;
    status_t status = reply.readBlob(blob_size, &blob);
    if (status) {
        ALOGE("%s: Error in readblob, status %d", __func__, status);
        return -EINVAL;
    }

    memset(payload, 0x0, *param_data_size);
    memscpy(payload, *param_data_size, blob.data(), blob_size);
    blob.release();
    return reply.readInt32();
}

int qsthw_set_parameters(const qsthw_module_handle_t *mod_handle,
                         sound_model_handle_t sound_model_handle,
                         const char *kv_pairs) {
    ALOGV("%d:%s",__LINE__, __func__);
    Parcel data, reply;
    data.writeInterfaceToken(Iqti_sva_server::getInterfaceDescriptor());
    data.write(&mod_handle, sizeof(qsthw_module_handle_t *));
    data.write(&sound_model_handle, sizeof(sound_model_handle_t));
    data.writeCString(kv_pairs);
    status_t status = remote()->transact(SET_PARAMS, data, &reply);
    if (status)
        return status;
    return reply.readInt32();
}

size_t qsthw_get_buffer_size(const qsthw_module_handle_t *mod_handle,
                             sound_model_handle_t sound_model_handle) {
    ALOGV("%d:%s",__LINE__, __func__);
    Parcel data, reply;
    data.writeInterfaceToken(Iqti_sva_server::getInterfaceDescriptor());
    data.write(&mod_handle, sizeof(qsthw_module_handle_t *));
    data.write(&sound_model_handle, sizeof(sound_model_handle_t));
    status_t status = remote()->transact(GET_BUFF_SIZE, data, &reply);
    if (status)
        return status;
    return (size_t)(reply.readUint32());
}

int qsthw_read_buffer(const qsthw_module_handle_t *mod_handle __unused,
                      sound_model_handle_t sound_model_handle,
                      unsigned char *buf,
                      size_t bytes) {
    Parcel data, reply;
    sva_sh_mem_data *shmem_data = NULL;
    void *shmem_data_buff = NULL;
    char *data_buff = (char *)buf;
    int rc;
    p_sound_model *p_sm = NULL;

    p_sm = (p_sound_model *)get_sm_handle_from_list(sound_model_handle);
    if (p_sm == NULL) {
        ALOGE("%s: No valid handle, exit from read buffer", __func__);
        goto exit;
    }
    if (p_sm ->prepare_done == false) {
        rc = prepare_for_buf_read(p_sm);
        if (rc != 0)
            goto exit;
    }
    shmem_data = p_sm->shmem_data;
    shmem_data_buff = p_sm->shmem_data_buff;
    ALOGV("%s: %d shmem_data_buff %p", __func__, __LINE__, shmem_data_buff);
    shmem_data->cmd = READ_BUFF;
    shmem_data->bytes = bytes;
    // is the server thread waiting for commands
    ALOGV("Taking command mutex %p", &shmem_data->c_mutex);
    rc = pthread_mutex_lock(&shmem_data->c_mutex);

    shmem_data->status = -1;

    //wakeup server stream thread.
    ALOGV("Waking up stream thread(%p:%p)",
        p_sm, p_sm->n_snd_model);
    rc = pthread_cond_signal(&shmem_data->c_cond);
    ALOGV("Woken up stream thread(%p:%p) %d",
        p_sm, p_sm->n_snd_model, rc);
    while (shmem_data->status == -1) {
        ALOGV("Waiting for status signal stream(%p:%p)",
            p_sm, p_sm->n_snd_model);
        rc = pthread_cond_wait(&shmem_data->c_cond, &shmem_data->c_mutex);
        ALOGV("Waiting ended for status signal stream(%p:%p) %d",
            p_sm, p_sm->n_snd_model, rc);
    }
    rc = pthread_mutex_unlock(&shmem_data->c_mutex);
    memscpy(data_buff, bytes, shmem_data_buff, bytes);
    return bytes;
exit:
    ALOGE("%s: read failed", __func__);
    return -EINVAL;
}

int qsthw_stop_buffering(const qsthw_module_handle_t *mod_handle,
                         sound_model_handle_t sound_model_handle) {
    ALOGV("%d:%s",__LINE__, __func__);
    int rc = 0;
    sva_sh_mem_data *shmem_data = NULL;
    Parcel data, reply;
    p_sound_model *p_sm = NULL;
    data.writeInterfaceToken(Iqti_sva_server::getInterfaceDescriptor());
    data.write(&mod_handle, sizeof(qsthw_module_handle_t *));
    data.write(&sound_model_handle, sizeof(sound_model_handle_t));

    p_sm = (p_sound_model *)get_sm_handle_from_list(sound_model_handle);
    if (p_sm == NULL) {
        ALOGE("%s: No valid handle, exit from read buffer", __func__);
        goto exit;
    }
    if (p_sm ->prepare_done == true) {
        shmem_data = p_sm->shmem_data;
        if(shmem_data != NULL) {
           shmem_data->cmd = STOP;
           shmem_data->status = -1;
           pthread_mutex_lock(&shmem_data->c_mutex);
           rc = pthread_cond_signal(&shmem_data->c_cond);
           ALOGV("Woken up stream thread%x %d", p_sm, rc);
           while (shmem_data->status == -1) {
                ALOGV("Waiting for status signal %x", p_sm);
                pthread_cond_wait(&shmem_data->c_cond, &shmem_data->c_mutex);
           }
           pthread_mutex_unlock(&shmem_data->c_mutex);
        }
    }
    rc = remote()->transact(STOP_BUFFERING, data, &reply);
    if (rc)
        return rc;
    remove_sm_handle_from_list(sound_model_handle);
    return reply.readUint32();
exit:
    ALOGE("%s failed", __func__);
    return -EINVAL;
}

private:
int init_ipc_primitives(sva_sh_mem_data *shmem_data) {
    int rc = 0;
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    rc = pthread_mutex_init(&shmem_data->c_mutex, &attr);
    if (rc != 0)
        goto exit;
    pthread_condattr_t cattr;
    pthread_condattr_init(&cattr);
    pthread_condattr_setpshared(&cattr, PTHREAD_PROCESS_SHARED);
    rc = pthread_cond_init(&shmem_data->c_cond, &cattr);
    if (rc != 0)
        goto exit;
    ALOGD("IPC primitives inited");
    return rc;
exit:
    ALOGE("Unable to setup IPC primitives");
    return -EINVAL;
}

int prepare_for_buf_read(void *p_sm) {
    int rc = 0;
    Parcel data, reply;
    sva_sh_mem_data *mem_ptr = NULL;
    data.writeInterfaceToken(Iqti_sva_server::getInterfaceDescriptor());
    p_sound_model *p_stream = (p_sound_model *)p_sm;

    //allocate shared mem
    size_t shmem_data_buff_size = qsthw_get_buffer_size(p_stream->mod_handle,
                                        p_stream->sm_handle);
    int total_sh_mem = sizeof(sva_sh_mem_data) + shmem_data_buff_size;

    ALOGD("%s: %d, %d Total shared mem needed %d", __func__,
            sizeof(sva_sh_mem_data), shmem_data_buff_size, total_sh_mem);
    p_stream->sh_mem_dealer = new MemoryDealer(total_sh_mem, "qas");
    if (p_stream->sh_mem_dealer == NULL)
        goto exit;
    p_stream->sh_mem_handle =
        p_stream->sh_mem_dealer->allocate(total_sh_mem);
    if (0 == p_stream->sh_mem_handle->size())
        goto exit;
    ALOGV("%s: Allocated shared mem %p size %zu", __func__,
                p_stream->sh_mem_handle->pointer(),
                p_stream->sh_mem_handle->size());

    p_stream->shmem_data =static_cast<sva_sh_mem_data*>
            (p_stream->sh_mem_handle->pointer());
    rc = init_ipc_primitives(p_stream->shmem_data);
    if (rc != 0)
        goto exit;

    mem_ptr = p_stream->shmem_data;
    p_stream->shmem_data_buff = (void*)++mem_ptr;
    ALOGV("%s: shmem_data buff header(%p) buff(%p) buff size %zu",
                __func__, &mem_ptr->buf,
                p_stream->shmem_data_buff,
                sizeof(sva_sh_mem_data));
    data.write(&p_stream->n_snd_model, sizeof(n_sound_model *));
    data.writeStrongBinder(IInterface::asBinder
                (p_stream->sh_mem_handle));
    rc = remote()->transact(PREPARE_FOR_READ, data, &reply);
    if (rc)
        return rc;
    rc = reply.readInt32();
    if (rc) {
        ALOGE("%s: PREPARE_FOR_READ failed %d", __func__, rc);
        goto exit;
    }

    p_stream->prepare_done = true;
    ALOGV("%s: done %d", __func__, rc);
    return reply.readInt32();
exit:
    ALOGE("%s: Unable to prepare for read", __func__);
    return -EINVAL;
}

void *get_sm_handle_from_list(
                sound_model_handle_t sound_model_handle) {
    struct listnode *node = NULL;
    p_sound_model *handle = NULL;

    ALOGV("%s", __func__);
    pthread_mutex_lock(&sm_list_lock);
    list_for_each(node, &sm_list) {
        handle = node_to_item(node, p_sound_model, list);
         if (handle != NULL && handle->sm_handle == sound_model_handle) {
             pthread_mutex_unlock(&sm_list_lock);
             ALOGV("%s Found handle %p for sound_model_handle %d", __func__,
                        handle, sound_model_handle);
             return (void *)handle;
        }
    }
    pthread_mutex_unlock(&sm_list_lock);
    return NULL;
}

void remove_sm_handle_from_list(
            sound_model_handle_t sound_model_handle) {
    p_sound_model *p_sm = NULL;
    struct listnode *node = NULL;
    struct listnode *tempnode = NULL;
    p_sound_model *handle = NULL;

    pthread_mutex_lock(&sm_list_lock);
    list_for_each_safe(node, tempnode, &sm_list) {
        handle = node_to_item(node, p_sound_model, list);
        if (handle != NULL && handle->sm_handle == sound_model_handle) {
            p_sm = handle;
            ALOGD("%s: Free up memory p_sm %p for sound_model_handle %d", __func__,
                        p_sm, sound_model_handle);
            p_sm->sh_mem_dealer.clear();
            p_sm->sh_mem_handle.clear();
        }
        list_remove(node);
        free(node_to_item(node, p_sound_model, list));
    }
    pthread_mutex_unlock(&sm_list_lock);
}

};
const android::String16 Iqti_sva_server::descriptor("Iqti_sva_server");
const android::String16& Iqti_sva_server::getInterfaceDescriptor() const {
    return Iqti_sva_server::descriptor;
}

android::sp<Iqti_sva_server> Iqti_sva_server::asInterface
    (const android::sp<android::IBinder>& obj) {
    ALOGD("Iqti_sva_server::asInterface()");
    android::sp<Iqti_sva_server> intr;
    if (obj != NULL) {
        intr = static_cast<Iqti_sva_server*>(obj->queryLocalInterface(Iqti_sva_server::descriptor).get());
        ALOGD("Iqti_sva_server::asInterface() interface %s",
            ((intr == 0)?"zero":"non zero"));

        if (intr == NULL) {
            intr = new qti_sva_proxy(obj);
        }
    }
    return intr;
}

Iqti_sva_server::Iqti_sva_server()
    { ALOGD("Iqti_sva_server::Iqti_sva_server()"); }
Iqti_sva_server::~Iqti_sva_server()
    { ALOGD("Iqti_sva_server::~Iqti_sva_server()"); }

}; // namespace svahal
