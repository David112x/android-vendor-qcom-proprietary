//*******************************************************************************/
/* Copyright (c) 2018 Qualcomm Technologies, Inc.                        */
/* All Rights Reserved.                                                        */
/* Confidential and Proprietary - Qualcomm Technologies, Inc.                  */
/*******************************************************************************/

#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include "stringl.h"

#include <esp_api.h>

using namespace std;

#ifdef FEATURE_ESP
#include "./VAD_Features/VAD_class.h"
#include "./DA_Metrics/FrameEnergyComputing.h"

#ifdef __cplusplus
extern "C" {
#include "parallel_task_queue.h"
#include "esp_interface_priv.h"
}
#endif

#define WAIT_TIMEOUT 20

#if defined(__arm__)
#if defined(__DEBUG_FILE_OUTPUT__)
FILE *energyCsv = NULL;
#endif
#else
FILE *energyCsv = NULL;
#endif

EspStatusType esp_init(void **handle,
                       int frame_len_ms,
                       int sample_rate,
                       int* totMemSize)
{
    uint32 memSize, total_memsize;
    EspStatusType rc = ESP_SUCCESS;
    esp_lib_t *pLib = NULL;
    VADClass *m_VAD;
    FrameEnergyClass *m_FrameEnergyCompute;

    if (handle == NULL)
    {
        return (ESP_ERR_INVALID_PARAM);
    }
    if ((frame_len_ms != 8) && (frame_len_ms != 15) && (frame_len_ms != 16))
    {
        return (ESP_ERR_INVALID_FRAMELEN_PARAM);
    }
    if (sample_rate != 16000)
    {
        return (ESP_ERR_INVALID_SAMPLE_RATE_PARAM);
    }

    memSize = ALIGN8(sizeof(esp_lib_t));
    total_memsize = memSize;

    pLib = (esp_lib_t *)malloc(memSize);
    if (pLib == NULL) {
        rc = ESP_ERR_NOMEM;
    }

    pLib->frameSize = (sample_rate / 1000) * frame_len_ms;

    //printf("framesize = %d\n", pLib->frameSize);

    // Setup objects based on frameSize
    m_VAD = new VADClass(pLib->frameSize);
    m_FrameEnergyCompute = new FrameEnergyClass(pLib->frameSize);

    memSize = sizeof(VADClass);
    total_memsize += memSize;

    memSize = sizeof(FrameEnergyClass);
    total_memsize += memSize;

    pLib->GVAD = 0;
    pLib->frameNum = 0;
    pLib->currentFrameEnergy = 0;
    pLib->vadObjPtr = m_VAD;
    pLib->frameEnergyComputeObjPtr = m_FrameEnergyCompute;

    memset(pLib->pInCirPCMBuf, 0, sizeof(pLib->pInCirPCMBuf));
    pLib->readIndex = 0;
    pLib->writeIndex = 0;
    pLib->unReadSamples = 0;

    // Initialize read mutex and condition variable
    if (MUTEX_INIT(&pLib->readMtx) != 0) {
        rc = ESP_ERR_NOMEM;
    }
    if (COND_VAR_INIT(&pLib->readCond) != 0) {
        rc = ESP_ERR_NOMEM;
}

#if defined(__arm__) && defined(__MULTI_THREAD__)
    pLib->pTaskQueue = init_parallel_task_queue();
    if (pLib->pTaskQueue == NULL)
    {
        rc = ESP_ERR_NOMEM;
    }
#endif

    *handle = (void *)pLib;

    if (totMemSize != NULL)
    {
        *totMemSize = total_memsize;
    }

#if defined(__arm__)
#if defined(__DEBUG_FILE_OUTPUT__)
    energyCsv = fopen("/data/local/tmp/energyVal.csv", "w+");
    if (energyCsv != NULL)
    {
        fprintf(energyCsv, "Frame Number, VoiceEnergy, AmbientEnergy, VADFlag, CurrentFrameEnergy\n");
    }
#endif
#else
    energyCsv = fopen("energyVal.csv", "w+");
    if (energyCsv != NULL)
    {
        fprintf(energyCsv, "Frame Number, VoiceEnergy, AmbientEnergy, VADFlag, CurrentFrameEnergy\n");
    }
#endif


    return rc;
}

EspStatusType esp_process(void *handle, const int16 *in_pcm)
{
    esp_lib_t *pLib = NULL;
    int16 len = 0;

    pLib = (esp_lib_t *)handle;

    if ((pLib->writeIndex < 0) || (pLib->writeIndex >= CIRC_BUF_SIZE))
    {
        printf("Invalid write index %d", pLib->writeIndex);
        goto err;
    }

    (void)MUTEX_LOCK(&pLib->readMtx);

    for (;;)
    {
        len = CIRC_BUF_SIZE - ESPIN_FRAME_SIZE;
        if (pLib->unReadSamples <= len)
        {
            break;
        }
        else
        {
            DECLARE_TIMEOUT(timeout);
            (void)SET_TIMEOUT(&timeout, WAIT_TIMEOUT);
            COND_VAR_WAIT(&pLib->readCond, &pLib->readMtx, &timeout);
        }

    }

    memscpy(&pLib->pInCirPCMBuf[pLib->writeIndex], ESPIN_FRAME_SIZE * sizeof(int16), in_pcm, ESPIN_FRAME_SIZE * sizeof(int16));

    pLib->writeIndex += ESPIN_FRAME_SIZE;

    if (pLib->writeIndex >= CIRC_BUF_SIZE)
    {
        pLib->writeIndex -= CIRC_BUF_SIZE;
    }

    pLib->unReadSamples += ESPIN_FRAME_SIZE;

    (void)MUTEX_UNLOCK(&pLib->readMtx);

#if defined(__arm__) && defined(__MULTI_THREAD__)
    (void) pLib->pTaskQueue->run_ec1(pLib->pTaskQueue->handle, esp_process_thread_fn, handle);
#else
    esp_process_thread(handle);
#endif

err:
    return ESP_SUCCESS;
}

#if defined(__arm__)
static void esp_process_thread_fn(void *handle)
{
    esp_process_thread(handle);
}
#endif

void esp_process_thread(void *handle)
{
    esp_lib_t *pLib = NULL;
    uint16 len = 0, remaining_sampls = 0;
    uint16 rem_len = 0;
    uint16 pendingSamples =0;

    pLib = (esp_lib_t *)handle;

    if ((pLib->readIndex < 0) || (pLib->readIndex >= CIRC_BUF_SIZE))
    {
        printf("Invalid Read index %d", pLib->readIndex);
        goto err;
    }

    for (;;)
    {

        (void)MUTEX_LOCK(&pLib->readMtx);

        if (pLib->unReadSamples < pLib->frameSize)
        {
            (void)MUTEX_UNLOCK(&pLib->readMtx);
            break;
        }

        len = ((CIRC_BUF_SIZE - pLib->readIndex) >= pLib->frameSize) ? pLib->frameSize : (CIRC_BUF_SIZE - pLib->readIndex);

        memscpy(pLib->pInbuf, len * sizeof(int16), &pLib->pInCirPCMBuf[pLib->readIndex], len * sizeof(int16));

        rem_len = pLib->frameSize - len;

        if (rem_len != 0)
        {
            memscpy(&pLib->pInbuf[len], rem_len * sizeof(int16), &pLib->pInCirPCMBuf[0], rem_len * sizeof(int16));
        }

        pLib->unReadSamples -= pLib->frameSize;

        pLib->readIndex += pLib->frameSize;

        if (pLib->readIndex >= CIRC_BUF_SIZE)
        {
            pLib->readIndex -= CIRC_BUF_SIZE;
        }

        if (pLib->writeIndex >= pLib->readIndex)
        {
            pendingSamples = pLib->writeIndex - pLib->readIndex;
        }
        else
        {
            pendingSamples = (pLib->writeIndex - pLib->readIndex) + CIRC_BUF_SIZE;
        }

        if (pendingSamples != pLib->unReadSamples)
        {
            fprintf(stderr, "Circular buffer indexing error");
        }

        (void)COND_VAR_BROADCAST(&pLib->readCond);

        (void)MUTEX_UNLOCK(&pLib->readMtx);

        pLib->vadObjPtr->process(pLib->pInbuf, pLib->GVAD, pLib->currentFrameEnergy);
        pLib->frameEnergyComputeObjPtr->process(pLib->GVAD, pLib->currentFrameEnergy);

        pLib->frameNum++;

    //  printf("In esp_process_thread frame no **%d\n", pLib->frameNum);

        (void)MUTEX_LOCK(&pLib->readMtx);
        pLib->enrgyLevels.voiceEnergy = pLib->frameEnergyComputeObjPtr->getVoicedEnergy();
        pLib->enrgyLevels.ambientNoiseEnergy = pLib->frameEnergyComputeObjPtr->getAmbientEnergy();
        (void)MUTEX_UNLOCK(&pLib->readMtx);

#if defined(__arm__)
#if defined(__DEBUG_FILE_OUTPUT__)
        if (energyCsv != NULL)
        {
            fprintf(energyCsv, "%u, %d, %d, %d, %lld\n", pLib->frameNum, pLib->frameEnergyComputeObjPtr->getVoicedEnergy(), pLib->frameEnergyComputeObjPtr->getAmbientEnergy(), pLib->GVAD, pLib->currentFrameEnergy);
        }
#endif
#else
        if (energyCsv != NULL)
        {
            fprintf(energyCsv, "%u, %d, %d, %d, %lld\n", pLib->frameNum, pLib->frameEnergyComputeObjPtr->getVoicedEnergy(), pLib->frameEnergyComputeObjPtr->getAmbientEnergy(), pLib->GVAD, pLib->currentFrameEnergy);
        }
#endif

    }

err:

    return;
}

EspStatusType esp_get_param(
    void *handle,
    char *params_buffer_ptr,
    int param_id,
    int buffer_size,
    int *param_size_ptr
)
{
    EspStatusType rc = ESP_SUCCESS;
    esp_lib_t *pLib = (esp_lib_t *)handle;

    if ((params_buffer_ptr == NULL) ||
        (pLib == NULL))
    {
        rc = ESP_ERR_GETPARAM;
        goto get_parm_err;
    }

    switch (param_id)
    {
    case ESP_VER_PARAM:
    {
        if (buffer_size >= sizeof(esp_ver_param_t))
        {
            esp_ver_param_t *ver_param = (esp_ver_param_t *)params_buffer_ptr;
            ver_param->ver = ESP_RELEASE_VERSION;

            if (param_size_ptr != NULL)
            {
                *param_size_ptr = sizeof(esp_ver_param_t);
            }

        }

        break;
    }
    case ESP_ENERGY_LEVELS_PARAM:
    {
        if (buffer_size >= sizeof(esp_energy_levels_t))
        {
            esp_energy_levels_t *enrgy_buf_ptr = (esp_energy_levels_t *)params_buffer_ptr;

            (void)MUTEX_LOCK(&pLib->readMtx);
            enrgy_buf_ptr->voiceEnergy = pLib->enrgyLevels.voiceEnergy;
            enrgy_buf_ptr->ambientNoiseEnergy = pLib->enrgyLevels.ambientNoiseEnergy;
            (void)MUTEX_UNLOCK(&pLib->readMtx);

            if (param_size_ptr != NULL)
            {
                *param_size_ptr = sizeof(esp_energy_levels_t);
            }

        }
        break;
    }
    default:
    {
        rc = ESP_ERR_INVALID_PARAM;
    }
    } //End of Switch

get_parm_err:
    return rc;
}


EspStatusType esp_deinit(void *handle)
{

    if (handle != NULL)
    {
        esp_lib_t *pLib = (esp_lib_t *)handle;

#if defined(__arm__) && defined(__MULTI_THREAD__)
        deinit_parallel_task_queue(pLib->pTaskQueue);
#endif

        (void)COND_VAR_BROADCAST(&pLib->readCond);
        (void)COND_VAR_DESTROY(&pLib->readCond);
        (void)MUTEX_DESTROY(&pLib->readMtx);

        if (pLib->vadObjPtr != NULL)
        {
            delete (pLib->vadObjPtr);
        }
        if (pLib->frameEnergyComputeObjPtr != NULL)
        {
            delete (pLib->frameEnergyComputeObjPtr);
        }

#if defined(__arm__)
#if defined(__DEBUG_FILE_OUTPUT__)
        if (energyCsv != NULL)
        {
            fclose(energyCsv);
        }
#endif
#else
        if (energyCsv != NULL)
        {
            fclose(energyCsv);
        }
#endif

        free(handle);

        return ESP_SUCCESS;
    }

    return ESP_ERR_INVALID_PARAM;
}
#else // FEATURE_ESP not defined
EspStatusType esp_init(void **handle,
    int frame_len_ms,
    int sample_rate,
    int* totMemSize)
{
    return ESP_ERR_ENOSYS;
}

EspStatusType esp_process(void *handle, const int16 *in_pcm)
{
    return ESP_ERR_ENOSYS;
}

EspStatusType esp_get_param(
    void *handle,
    char *params_buffer_ptr,
    int param_id,
    int buffer_size,
    int *param_size_ptr
)
{
    return ESP_ERR_ENOSYS;
    }

EspStatusType esp_deinit(void *handle)
{
    return ESP_ERR_ENOSYS;
}

#endif //FEATURE_ESP
