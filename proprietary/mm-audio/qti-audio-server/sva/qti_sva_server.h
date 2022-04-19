/* Copyright (c) 2017 - 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

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
#include <cutils/list.h>
#include <pthread.h>

#include "qti_sva_server_interface.h"
#include "qti_sva_server_callback.h"

#define QTI_SVA_SERVER "qti_sva_server"

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif

#ifndef memscpy
#define memscpy(dst, dst_size, src, bytes_to_copy) (void) \
                    memcpy(dst, src, MIN(dst_size, bytes_to_copy))
#endif

namespace svahal {
using namespace android;

typedef struct {
    sp<IRecognitionCallback> cb_binder;
    void * cookie1;
    void * cookie2;
} callback_data;

typedef struct {
    qsthw_module_handle_t *mod_handle;
    sound_model_handle_t sm_handle;
    sp<IMemory> sh_mem_handle;
    sva_sh_mem_data *shmem_data;
    void *shmem_data_buff;
    pthread_t thread;
} n_sound_model;

typedef struct {
    struct listnode list;
    bool prepare_done;
    qsthw_module_handle_t *mod_handle;
    sound_model_handle_t sm_handle;
    n_sound_model *n_snd_model;
    sp<MemoryDealer> sh_mem_dealer;
    sp<IMemory> sh_mem_handle;
    sva_sh_mem_data *shmem_data;
    void *shmem_data_buff;
} p_sound_model;

class qti_sva_server : public ::android::BnInterface<Iqti_sva_server> {
public:
    qti_sva_server() { ALOGD("qti_sva_server::qti_sva_server()"); };
    ~qti_sva_server() { ALOGD("~qti_sva_server::qti_sva_server()"); };
private:
    int32_t onTransact(uint32_t code, const ::android::Parcel& data,
                 ::android::Parcel* reply, uint32_t flags = 0) override;
    qsthw_module_handle_t* qsthw_load_module(
                        const char *hw_module_id) override;
    int qsthw_unload_module(
                        const qsthw_module_handle_t *hw_module) override;
    int qsthw_get_version() override;
    int qsthw_get_properties(const qsthw_module_handle_t *mod_handle,
                         struct sound_trigger_properties *properties) override;
    int qsthw_load_sound_model(const qsthw_module_handle_t *mod_handle,
                        sound_trigger_sound_model *sound_model,
                        qsthw_sound_model_callback_t callback,
                        void *cookie,
                        sound_model_handle_t *handle) override;
    int qsthw_unload_sound_model(const qsthw_module_handle_t *mod_handle,
                        sound_model_handle_t handle) override;
    int qsthw_start_recognition(const qsthw_module_handle_t *mod_handle,
                        sound_model_handle_t sound_model_handle,
                        const struct sound_trigger_recognition_config *config,
                        qsthw_recognition_callback_t callback,
                        void *cookie) override;
    int qsthw_stop_recognition(const qsthw_module_handle_t *mod_handle,
                        sound_model_handle_t sound_model_handle) override;
    int qsthw_get_param_data(const qsthw_module_handle_t *mod_handle,
                        sound_model_handle_t sound_model_handle,
                        const char *param,
                        void *payload,
                        size_t payload_size,
                        size_t *param_data_size) override;
    int qsthw_set_parameters(const qsthw_module_handle_t *mod_handle,
                        sound_model_handle_t sound_model_handle,
                        const char *kv_pairs) override;
    size_t qsthw_get_buffer_size(const qsthw_module_handle_t *mod_handle,
                        sound_model_handle_t sound_model_handle) override;
    int qsthw_read_buffer(const qsthw_module_handle_t *mod_handle,
                        sound_model_handle_t sound_model_handle,
                        unsigned char *buf,
                        size_t bytes) override;
    int qsthw_stop_buffering(const qsthw_module_handle_t *mod_handle,
                        sound_model_handle_t sound_model_handle) override;
    virtual int prepare_for_buf_read(void *p_stream) override;
};
}; // namespace svahal

