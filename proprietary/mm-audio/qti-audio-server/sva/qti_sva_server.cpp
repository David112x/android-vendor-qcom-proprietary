/* Copyright (c) 2017 - 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#define LOG_TAG "qti_sva_server"
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
#include <qti_sva_server.h>
#include <../qti_audio_client.h>
#include <../qas_client_death_notifier_common.h>

extern "C"
{
#include <mm-audio/qsthw/qsthw.h>
}

namespace svahal {
using namespace android;

callback_data cb;

static void recognition_event_cb(
                        struct sound_trigger_recognition_event *event,
                        void *cookie) {
    callback_data *cb = (callback_data *)cookie;
    sp<IRecognitionCallback> cb_binder = cb->cb_binder;

    if (cb_binder == NULL) {
        ALOGE("%s Invalid binder handle", __func__);
        return;
    }
    ALOGD("%s: event(%p) for stream (%p)",
        __func__, event, cookie);
    cb_binder->recognition_cb(event,
                       cb->cookie1, cb->cookie2);
    return;
}

int32_t qti_sva_server::onTransact(uint32_t code,
                                   const Parcel& data,
                                   Parcel* reply,
                                   uint32_t flags) {
    ALOGV("qti_sva_server::onTransact(%i) %i", code, flags);
    data.checkInterface(this);
    int rc = -EINVAL;

    switch(code) {
        case REGISTER_CLIENT: {
            sp<IBinder> binder = data.readStrongBinder();
            return qti_audio_server_register_client(binder);
        }
        break;
        case UNREGISTER_CLIENT: {
            sp<IBinder> binder = data.readStrongBinder();
            return qti_audio_server_unregister_client(binder);
        }
        break;
        case LOAD_SVA_HAL: {
            const char *hal_id = data.readCString();
            ALOGV("LOAD_HAL: %s", hal_id);
            qsthw_module_handle_t *hal_handle =
                    (qsthw_module_handle_t *)qsthw_load_module(hal_id);
            reply->write(&hal_handle, sizeof(qsthw_module_handle_t *));
            qti_audio_server_add_hal_handle((void *)hal_handle, HAL_HANDLE_TYPE_STHAL);
            ALOGV("hal handle (%p)", hal_handle);
        }
        break;
        case UNLOAD_SVA_HAL: {
            qsthw_module_handle_t *handle = NULL;
            data.read(&handle, sizeof(qsthw_module_handle_t *));
            ALOGD("%d: UNLOAD_HAL(%p)", __LINE__, handle);
            rc = qsthw_unload_module(handle);
            reply->writeInt32(rc);
            qti_audio_server_remove_hal_handle((void *)handle);
        }
        break;
        case GET_VERSION: {
            ALOGV("%d: GET_VERSION", __LINE__);
            return qsthw_get_version();
        }
        break;
        case LOAD_SOUND_MODEL: {
            ALOGV("%d: LOAD_SOUND_MODEL", __LINE__);
            const qsthw_module_handle_t *mod_handle = NULL;
            int sm_size;
            uint32_t blob_size;
            struct sound_trigger_sound_model *sound_model = NULL;
            sound_model_handle_t handle = 0;
            data.read(&mod_handle, sizeof(qsthw_module_handle_t *));

            android::Parcel::ReadableBlob blob;
            data.readUint32(&blob_size);
            status_t status = data.readBlob(blob_size, &blob);
            if (status) {
                ALOGE("%s: Error in readblob, status %d", __func__, status);
                return -EINVAL;
            }

            ALOGV("%d: LOAD_SOUND_MODEL, blob_size %d", __LINE__, blob_size);
            sound_model = (struct sound_trigger_sound_model *)calloc(1, blob_size);
            if (sound_model == NULL) {
                ALOGE("%s: Could not allocate memory for sound model",
                        __func__);
                blob.release();
                return -ENOMEM;
            }
            sm_size = sizeof(struct sound_trigger_phrase_sound_model);
            memscpy(sound_model, sm_size, blob.data(), sm_size);

            if ((sound_model->data_size >= blob_size) ||
                (((uint64_t)soumd_model->data_offset +
                (uint64_t)sound_model->data_size) >= blob_size)) {
                ALOGE("%s: sound model size exceeds allocated blob size", __func__);
                return -EINVAL;
            }
            memscpy((((char *)sound_model) + sound_model->data_offset),
                        sound_model->data_size,
                        (((char *)blob.data()) + sound_model->data_offset),
                        sound_model->data_size);
            blob.release();
            rc = qsthw_load_sound_model(mod_handle, sound_model, NULL,
                                       NULL, &handle);
            reply->write(&handle, sizeof(sound_model_handle_t));
            ALOGD("LOAD_SOUND_MODEL: %s: server side stream handle(%p)",
                        __func__, handle);
            if (sound_model != NULL)
                free(sound_model);
            qti_audio_server_add_snd_model_handle(handle);
            return rc;
        }
        break;
        case UNLOAD_SOUND_MODEL: {
            qsthw_module_handle_t *mod_handle = NULL;
            sound_model_handle_t handle = 0;
            data.read(&mod_handle, sizeof(qsthw_module_handle_t *));
            data.read(&handle, sizeof(sound_model_handle_t));
            ALOGV("%d: UNLOAD_SOUND_MODEL, %d", __LINE__, handle);
            rc = qsthw_unload_sound_model(mod_handle, handle);
            reply->writeInt32(rc);
            qti_audio_server_remove_snd_model_handle(handle);
        }
        break;
        case GET_PROPERTIES: {
            ALOGV("%d: GET_PROPERTIES", __LINE__);
            qsthw_module_handle_t *mod_handle = NULL;
            struct sound_trigger_properties properties;
            data.read(&mod_handle, sizeof(qsthw_module_handle_t *));

            memset(&properties, 0x0, sizeof(sound_trigger_properties));
            rc = qsthw_get_properties(mod_handle, &properties);
            if (rc != 0) {
                ALOGE("%d: error in GET_PROPERTIES", __LINE__);
                return -EINVAL;
            }

            uint32_t param_size = sizeof(sound_trigger_properties);
            reply->writeUint32(param_size);
            android::Parcel::WritableBlob blob;
            status_t status = reply->writeBlob(param_size, false, &blob);
            if (status) {
                ALOGE("%s: Error in writeblob, status %d", __func__, status);
                return -EINVAL;
            }
            memset(blob.data(), 0x0, param_size);
            memscpy(blob.data(), param_size, &properties, param_size);
            blob.release();
            return rc;
        }
        break;
        case START_RECOGNITION: {
             const qsthw_module_handle_t *mod_handle = NULL;
             uint32_t blob_size;
             struct sound_trigger_recognition_config *config = NULL;
             int size;
             n_sound_model *n_sm =
                    (n_sound_model *)calloc(1, sizeof(n_sound_model));

            if (0 == n_sm) {
                ALOGE("%s: failed: unable to allocate mem for stream data",
                        __func__);
                reply->writeUint32((uint32_t)NULL);
                return -EINVAL;
            }
            ALOGV("START_RECOGNITION: Allocated Server side stream \
                        mem %p size of snd model %zd",
                        n_sm, sizeof(n_sound_model));
            sound_model_handle_t sound_model_handle = 0;
            data.read(&mod_handle, sizeof(qsthw_module_handle_t *));
            data.read(&sound_model_handle, sizeof(sound_model_handle_t));

            android::Parcel::ReadableBlob blob;
            data.readUint32(&blob_size);
            status_t status = data.readBlob(blob_size, &blob);
            if (status) {
                ALOGE("%s: Error in readblob, status %d", __func__, status);
                return -EINVAL;
            }

            config = (struct sound_trigger_recognition_config *)calloc(1, blob_size);
            if (config == NULL) {
                ALOGE("%s: Could not allocate memory for sound model",
                        __func__);
                blob.release();
                return -ENOMEM;
            }
            n_sm->mod_handle = (qsthw_module_handle_t *)mod_handle;
            n_sm->sm_handle = sound_model_handle;
            size = sizeof(sound_trigger_recognition_config);
            memscpy(config, size, blob.data(), size);
            memscpy(((char *)config) + config->data_offset,
                        (blob_size - size),
                        ((char *)blob.data()) + size, (blob_size - size));
            blob.release();

            sp<IBinder> binder = data.readStrongBinder();
            cb.cb_binder = interface_cast<IRecognitionCallback>(binder);
            data.read(&cb.cookie1, sizeof(qsthw_recognition_callback_t *));
            data.read(&cb.cookie2, sizeof(qsthw_recognition_callback_t *));
            rc = qsthw_start_recognition(mod_handle, sound_model_handle,
                        config, recognition_event_cb,
                        static_cast <void *> (&cb));
            reply->write(&n_sm, sizeof(n_sound_model *));
            if (config != NULL)
                free(config);
            return rc;
        }
        break;
        case STOP_RECOGNITION: {
            qsthw_module_handle_t *mod_handle = NULL;
            sound_model_handle_t sound_model_handle = 0;
            data.read(&mod_handle, sizeof(qsthw_module_handle_t *));
            data.read(&sound_model_handle, sizeof(sound_model_handle_t));
            ALOGV("%d: STOP_RECOGNITION, %d", __LINE__, sound_model_handle);
            rc = qsthw_stop_recognition(mod_handle, sound_model_handle);
            qti_audio_server_remove_shmem_handle(sound_model_handle);
            reply->writeInt32(rc);
        }
        break;
        case GET_PARAM_DATA: {
            ALOGV("%d: GET_PARAM_DATA", __LINE__);
            qsthw_module_handle_t *mod_handle = NULL;
            sound_model_handle_t sound_model_handle = 0;
            qsthw_get_param_payload_t payload;
            size_t payload_size = 0;
            size_t param_data_size = 0;

            data.read(&mod_handle, sizeof(qsthw_module_handle_t *));
            data.read(&sound_model_handle, sizeof(sound_model_handle_t));
            const char *param = data.readCString();
            data.read(&payload_size, sizeof(size_t));

            memset(&payload, 0x0, sizeof(qsthw_get_param_payload_t));
            rc = qsthw_get_param_data(mod_handle, sound_model_handle,
                        param, &payload, payload_size, &param_data_size);
            if (rc != 0) {
                ALOGE("error in GET_PARAM_DATA");
                return -EINVAL;
            }
            reply->write(&param_data_size, sizeof(size_t));
            android::Parcel::WritableBlob blob;
            status_t status = reply->writeBlob(param_data_size, false, &blob);
            if (status) {
                ALOGE("%s: Error in writeblob, status %d", __func__, status);
                return -EINVAL;
            }
            memset(blob.data(), 0x0, param_data_size);
            memscpy(blob.data(), param_data_size, &payload, param_data_size);
            blob.release();
            return rc;
        }
        break;
        case SET_PARAMS: {
            ALOGV("%s SET_PARAMS", __func__);
            qsthw_module_handle_t *mod_handle = NULL;
            sound_model_handle_t sound_model_handle = 0;
            data.read(&mod_handle, sizeof(qsthw_module_handle_t *));
            data.read(&sound_model_handle, sizeof(sound_model_handle_t));
            const char *kv_pairs = data.readCString();
            rc = qsthw_set_parameters(mod_handle, sound_model_handle, kv_pairs);
            reply->writeInt32(rc);
        }
        break;
        case GET_BUFF_SIZE: {
            qsthw_module_handle_t *mod_handle = NULL;
            sound_model_handle_t sound_model_handle = 0;
            data.read(&mod_handle, sizeof(qsthw_module_handle_t *));
            data.read(&sound_model_handle, sizeof(sound_model_handle_t));
            size_t size = qsthw_get_buffer_size(mod_handle, sound_model_handle);
            ALOGV("GET_BUFF_SIZE: size %zu", size);
            reply->writeUint32((uint32_t)size);
        }
        break;
        case PREPARE_FOR_READ:
        {
            n_sound_model *n_stream = NULL;
            data.read(&n_stream, sizeof(n_sound_model *));
            ALOGV("%s PREPARE_FOR_READ (%p)", __func__, n_stream);
            n_stream->sh_mem_handle =
                    interface_cast<IMemory>(data.readStrongBinder());
            n_stream->shmem_data =
                    (sva_sh_mem_data *)n_stream->sh_mem_handle->pointer();

            sva_sh_mem_data *ptr = n_stream->shmem_data;
            n_stream->shmem_data_buff = (void*)++ptr;
            ALOGV("shmem_data buff header(%p) buff(%p) buff size %zd",
                        &n_stream->shmem_data->buf,
                        n_stream->shmem_data_buff,
                        sizeof(sva_sh_mem_data));
            rc = prepare_for_buf_read((n_sound_model *)n_stream);
            qti_audio_server_add_shmem_handle(n_stream->sm_handle, (void *)n_stream);
            reply->writeInt32(rc);
        }
        break;
        case STOP_BUFFERING: {
            ALOGV("%s STOP_BUFFERING", __func__);
            qsthw_module_handle_t *mod_handle = NULL;
            sound_model_handle_t sound_model_handle = 0;
            data.read(&mod_handle, sizeof(qsthw_module_handle_t *));
            data.read(&sound_model_handle, sizeof(sound_model_handle_t));
            rc = qsthw_stop_buffering(mod_handle, sound_model_handle);
            reply->writeInt32(rc);
        }
        break;
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
    return 0;
}

qsthw_module_handle_t* qti_sva_server::qsthw_load_module(
                        const char *hw_module_id) {
    ALOGV("%d:%s",__LINE__, __func__);
    return qsthw_load_module_l(hw_module_id);
}

int qti_sva_server::qsthw_unload_module(
                        const qsthw_module_handle_t *handle) {
    ALOGV("%d:%s",__LINE__, __func__);
    return qsthw_unload_module_l(handle);
}


int qti_sva_server::qsthw_get_version() {
    ALOGV("%d:%s",__LINE__, __func__);
    return qsthw_get_version_l();
}

int qti_sva_server::qsthw_get_properties(
                        const qsthw_module_handle_t *mod_handle,
                        struct sound_trigger_properties *properties) {
    ALOGV("%d:%s",__LINE__, __func__);
    return qsthw_get_properties_l(mod_handle, properties);
}

int qti_sva_server::qsthw_load_sound_model(
                        const qsthw_module_handle_t *mod_handle,
                        sound_trigger_sound_model *sound_model,
                        qsthw_sound_model_callback_t callback __unused,
                        void *cookie __unused,
                        sound_model_handle_t *handle) {
    ALOGD("%d:%s",__LINE__, __func__);
    return qsthw_load_sound_model_l(mod_handle, sound_model, NULL,
                                    NULL, handle);
}

int qti_sva_server::qsthw_unload_sound_model(
                        const qsthw_module_handle_t *mod_handle,
                        sound_model_handle_t handle) {
    ALOGV("%d:%s",__LINE__, __func__);
    return qsthw_unload_sound_model_l(mod_handle, handle);
}

int qti_sva_server::qsthw_start_recognition(
                        const qsthw_module_handle_t *mod_handle,
                        sound_model_handle_t sound_model_handle,
                        const struct sound_trigger_recognition_config *config,
                        qsthw_recognition_callback_t callback,
                        void *cookie) {
    ALOGV("%d:%s",__LINE__, __func__);
    return qsthw_start_recognition_l(mod_handle, sound_model_handle,
                                    config, callback, cookie);
}

int qti_sva_server::qsthw_stop_recognition(
                        const qsthw_module_handle_t *mod_handle,
                        sound_model_handle_t sound_model_handle) {
    ALOGV("%d:%s",__LINE__, __func__);
    return qsthw_stop_recognition_l(mod_handle, sound_model_handle);
}

int qti_sva_server::qsthw_get_param_data(
                        const qsthw_module_handle_t *mod_handle,
                        sound_model_handle_t sound_model_handle,
                        const char *param,
                        void *payload,
                        size_t payload_size,
                        size_t *param_data_size) {
    ALOGV("%d:%s",__LINE__, __func__);
   return qsthw_get_param_data_l(mod_handle, sound_model_handle,
                            param, payload, payload_size, param_data_size);
}

int qti_sva_server::qsthw_set_parameters(
                        const qsthw_module_handle_t *mod_handle,
                        sound_model_handle_t sound_model_handle,
                        const char *kv_pairs)
{
    ALOGV("%d:%s",__LINE__, __func__);
    return qsthw_set_parameters_l(mod_handle, sound_model_handle, kv_pairs);
}

size_t qti_sva_server::qsthw_get_buffer_size(
                        const qsthw_module_handle_t *mod_handle,
                        sound_model_handle_t sound_model_handle) {
    ALOGV("%d:%s",__LINE__, __func__);
    return qsthw_get_buffer_size_l(mod_handle, sound_model_handle);
}

int qti_sva_server::qsthw_read_buffer(
                        const qsthw_module_handle_t *mod_handle __unused,
                        sound_model_handle_t sound_model_handle __unused,
                        unsigned char *buf __unused,
                        size_t bytes __unused) {
    return -ENOSYS;
}

int qti_sva_server::qsthw_stop_buffering(
                        const qsthw_module_handle_t *mod_handle,
                        sound_model_handle_t sound_model_handle) {
    ALOGV("%d:%s",__LINE__, __func__);
    return qsthw_stop_buffering_l(mod_handle, sound_model_handle);
}

static void * read_stream_thread_loop(void *context) {
    int rc = 0;
    n_sound_model *n_stream = (n_sound_model *)context;
    sva_sh_mem_data *shmem_data = n_stream->shmem_data;
    while (1) {
        pthread_cond_wait(&shmem_data->c_cond, &shmem_data->c_mutex);
        switch(shmem_data->cmd) {
            case READ_BUFF:
            {
                shmem_data->buf = (unsigned char *)n_stream->shmem_data_buff;
                ALOGV("%d: %s data %p, bytes %zd", __LINE__, __func__,
                    n_stream->shmem_data_buff, shmem_data->bytes);

                rc = qsthw_read_buffer_l(n_stream->mod_handle, n_stream->sm_handle,
                                        shmem_data->buf, shmem_data->bytes);
                //wakeup calling thread.
                ALOGV("%d: %s: fire status signal(%p:%d)",
                    __LINE__, __func__, n_stream, n_stream->sm_handle);
                shmem_data->status = 0;
                pthread_cond_signal(&shmem_data->c_cond);
                break;
            }
            case STOP:
            {
                shmem_data->status = 0;
                pthread_cond_signal(&shmem_data->c_cond);
                pthread_mutex_unlock(&shmem_data->c_mutex);
                n_stream->sh_mem_handle = NULL;
                ALOGD("%s: exiting", __func__);
                /* Call pthread detach to free thread resources */
                pthread_detach(pthread_self());
                pthread_exit(0);
                break;
            }
        }
    }
}

int qti_sva_server::prepare_for_buf_read(void *n_sm) {
    int rc = 0;
    n_sound_model *n_stream = (n_sound_model *)n_sm;
    rc = pthread_mutex_lock(&n_stream->shmem_data->c_mutex);
    rc = pthread_create(&n_stream->thread,
                   (const pthread_attr_t *)NULL,
                   read_stream_thread_loop,
                   n_stream);
    pthread_detach(n_stream->thread);
    ALOGD("create read Stream Thread ret %d", rc);
    if (rc)
        pthread_mutex_unlock(&n_stream->shmem_data->c_mutex);
    return rc;
}
}; // namespace svahal

