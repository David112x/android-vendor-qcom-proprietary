/*============================================================================
* Copyright (c) 2018 Qualcomm Technologies, Inc.                       *
* All Rights Reserved.                                                       *
* Confidential and Proprietary - Qualcomm Technologies, Inc.                 *
* ===========================================================================*/
#ifndef _ESP_INTERFACE_H_
#define _ESP_INTERFACE_H_

/**
 * @file esp_interface.h
 *
 * This header file declares the interface to the esp algorithm
 */

/** DATA TYPE MACROS**/
#include <stddef.h>
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _INT16_DEFINED
typedef  signed short       int16;       /* Signed 16 bit value */
#define _INT16_DEFINED
#endif

#ifndef _UINT16_DEFINED
typedef  unsigned short      uint16;       /* UnSigned 16 bit value */
#define _UINT16_DEFINED
#endif

#ifndef _UINT8_DEFINED
typedef  unsigned char      uint8;       /* Unsigned 8  bit value */
#define _UINT8_DEFINED
#endif

#ifndef _UINT32_DEFINED
typedef  unsigned int       uint32;      // Unsigned 32 bit value
#define _UINT32_DEFINED
#endif

#ifndef _INT32_DEFINED
typedef  int    int32;      // signed 32 bit value
#define _INT32_DEFINED
#endif

#ifndef _INT64_DEFINED
typedef  long long  int64;      // signed 64 bit value
#define _INT64_DEFINED
#endif

//EC out frame size 20ms at 16Khz
#define ESPIN_FRAME_SIZE 16*20

/* Error codes */
typedef enum
{
   ESP_SUCCESS             =  0,   /* Success */
   ESP_ERR_INVALID_PARAM         = -1,   /* Invalid function parameter, unspecified */
   ESP_ERR_INVALID_FRAMELEN_PARAM = -2,
   ESP_ERR_INVALID_SAMPLE_RATE_PARAM = -3,
   ESP_ERR_INIT_GETSIZE = -4,
   ESP_ERR_INIT = -5,
   ESP_ERR_GETPARAM = -6,
   ESP_ERR_SETPARAM = -7,
   ESP_ERR_NOMEM    = -100,
   ESP_ERR_ENOSYS = ENOSYS,
   ESP_ERR_ENOENT = ENOENT
}EspStatusType;


/* Define GEG/SET Parms */

/*======================================================================

  FUNCTION      esp_get_param

  DESCRIPTION   Get ESP param, copying from ESP library to a block of memory query for the param

  DEPENDENCIES  params_buffer_ptr must not be NULL.
                esp_init must be called prior to calling this function.

  PARAMETERS    handle: [in/out] Pointer to the library instance
                param_id: [in]: PARAMETER ID
                params_buffer_ptr: [in] Pointer to a memory to save the payload for getting FFV parameter
                buffer_size: [in]: memory size
                param_size_ptr: [out] copied parameter memory size

  RETURN VALUE  ESP_SUCCESS if successful,
                ESP_ERR_GETPARAM otherwise.

  SIDE EFFECTS  None

======================================================================*/

EspStatusType esp_get_param(
        void *handle,
        char *params_buffer_ptr,
        int param_id,
        int buffer_size,
        int *param_size_ptr
        );


#define ESP_VER_PARAM  (0x1)
#define ESP_ENERGY_LEVELS_PARAM (0x2)

typedef struct esp_ver_param_t {
   int ver;
}esp_ver_param_t;

typedef struct esp_energy_levels_t {
    int voiceEnergy;
    int ambientNoiseEnergy;
}esp_energy_levels_t;

/**
 * Initialize ESP algorithm.
 *
 * @param[out] handle
 *    Pointer to dynamically allocated ffvfe library
 *
 * @param[in] frame_len_ms
 *    Number of samples per frame for one channel
 *
 * @param[in] sample_rate
 *    Sampling rate in Hz
 *
 * @param[out] totMemSize
 *    total memory size allocated for the library

 * @return 0, or error code
 */
EspStatusType esp_init(void **handle,
                       int frame_len_ms,
                       int sample_rate,
                       int* totMemSize);

/**
 * ESP process.
 *
 * @param[in/out] handle (from surround_rec_init function)
 * @param in_pcm[in] interleaved 16-bit PCM data *
 */
EspStatusType esp_process(void *handle, const int16 *in_pcm);

/**
 * Deinit algorithm, freeing allocated memory.
 *
 * @param[out] handle
 */

EspStatusType esp_deinit(void *handle);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* #ifndef _ESP_INTERFACE_H_ */
