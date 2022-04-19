/* Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef QAS_CLIENT_DEATH_NOTIFIER_H
#define QAS_CLIENT_DEATH_NOTIFIER_H

#include <binder/IBinder.h>
#include <binder/ProcessState.h>
#include <utils/SortedVector.h>
#include <utils/Vector.h>
#include <cutils/list.h>
#include <mm-audio/qahw/inc/qahw.h>
#include <qsthw.h>
#include "qti_audio_client.h"

using namespace android;

typedef enum {
    IO_HANDLE_TYPE_RECORD,
    IO_HANDLE_TYPE_PLAYBACK
} io_handle_type_t;

typedef enum {
    HAL_HANDLE_TYPE_AHAL,
    HAL_HANDLE_TYPE_STHAL
} hal_handle_type_t;

class qas_client_death_notifier : public IBinder::DeathRecipient
{
    public:
        qas_client_death_notifier(void);
        // DeathRecipient
        virtual void binderDied(const android::wp<IBinder>& who);
};

typedef struct {
    struct listnode list;
    void *io_handle;
    bool rx;
    int sm_handle;
} qas_client_session_data;

typedef struct {
    struct listnode list;
    pid_t pid;
    sp<IQASClient> binder;
    void *hal_handle;
    int hal_type;
    sp<qas_client_death_notifier> client_death_notifier;
    struct listnode qas_client_sess_list;
} qas_client_info;

int qti_audio_server_register_client(sp<IBinder> binder);
int qti_audio_server_unregister_client(sp<IBinder> binder);
void qti_audio_server_add_hal_handle(void *handle,
    hal_handle_type_t hal_type);
void qti_audio_server_remove_hal_handle(void *handle);
void qti_audio_server_add_io_stream_handle(void *io_handle,
                                    io_handle_type_t rx);
void qti_audio_server_remove_io_stream_handle(void *io_handle,
    io_handle_type_t rx);
void qti_audio_server_add_snd_model_handle(sound_model_handle_t sm_handle);
void qti_audio_server_remove_snd_model_handle(sound_model_handle_t sm_handle);
void qti_audio_server_add_shmem_handle(sound_model_handle_t sm_handle,
                                        void *stream);
void qti_audio_server_remove_shmem_handle(sound_model_handle_t sm_handle);
void qti_audio_server_clean_dead_audio_client(qas_client_info *handle);
void qti_audio_server_clean_dead_sva_client(qas_client_info *handle);
qas_client_info *qti_audio_server_get_client_handle_from_list(pid_t pid);

#endif //QAS_CLIENT_DEATH_NOTIFIER_H
