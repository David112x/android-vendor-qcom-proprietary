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
#include <qsthw.h>

namespace svahal {
using namespace android;

typedef enum {
    LOAD_SVA_HAL = ::android::IBinder::FIRST_CALL_TRANSACTION,
    UNLOAD_SVA_HAL,
    GET_VERSION,
    LOAD_SOUND_MODEL,
    UNLOAD_SOUND_MODEL,
    PREPARE_FOR_READ,
    GET_PROPERTIES,
    START_RECOGNITION,
    STOP_RECOGNITION,
    GET_PARAM_DATA,
    SET_PARAMS,
    GET_BUFF_SIZE,
    READ_BUFF,
    STOP_BUFFERING,
    STOP,
    REGISTER_CLIENT,
    UNREGISTER_CLIENT
} stream_command;

typedef struct {
    pthread_cond_t  c_cond;
    pthread_mutex_t c_mutex;
    stream_command cmd;
    int status;
    unsigned char *buf;
    int bytes;
} sva_sh_mem_data;

class Iqti_sva_server: public ::android::IInterface {
public:
    DECLARE_META_INTERFACE(qti_sva_server);
    virtual qsthw_module_handle_t* qsthw_load_module(
                        const char *hw_module_id) = 0;
    virtual int qsthw_unload_module(
                        const qsthw_module_handle_t *hw_module) = 0;
    virtual int qsthw_get_version() = 0;
    virtual int qsthw_get_properties(
                        const qsthw_module_handle_t *mod_handle,
                        struct sound_trigger_properties *properties) = 0;
    virtual int qsthw_load_sound_model(
                        const qsthw_module_handle_t *mod_handle,
                        sound_trigger_sound_model *sound_model,
                        qsthw_sound_model_callback_t callback,
                        void *cookie,
                        sound_model_handle_t *handle) = 0;
    virtual int qsthw_unload_sound_model(
                        const qsthw_module_handle_t *mod_handle,
                        sound_model_handle_t handle) = 0;
    virtual int qsthw_start_recognition(
                        const qsthw_module_handle_t *mod_handle,
                        sound_model_handle_t sound_model_handle,
                        const struct sound_trigger_recognition_config *config,
                        qsthw_recognition_callback_t callback,
                        void *cookie) = 0;
    virtual int qsthw_stop_recognition(
                        const qsthw_module_handle_t *mod_handle,
                        sound_model_handle_t sound_model_handle) = 0;
    virtual int qsthw_get_param_data(
                        const qsthw_module_handle_t *mod_handle,
                        sound_model_handle_t sound_model_handle,
                        const char *param,
                        void *payload,
                        size_t payload_size,
                        size_t *param_data_size) = 0;
    virtual int qsthw_set_parameters(
                        const qsthw_module_handle_t *mod_handle,
                        sound_model_handle_t sound_model_handle,
                        const char *kv_pairs) = 0;
    virtual size_t qsthw_get_buffer_size(
                        const qsthw_module_handle_t *mod_handle,
                        sound_model_handle_t sound_model_handle) = 0;
    virtual int qsthw_read_buffer(
                        const qsthw_module_handle_t *mod_handle,
                        sound_model_handle_t sound_model_handle,
                        unsigned char *buf,
                        size_t bytes) = 0;
    virtual int qsthw_stop_buffering(
                        const qsthw_module_handle_t *mod_handle,
                        sound_model_handle_t sound_model_handle) = 0;
    virtual int prepare_for_buf_read(void *p_stream) = 0;
};
}; // namespace svahal

