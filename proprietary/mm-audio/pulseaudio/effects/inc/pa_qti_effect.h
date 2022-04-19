/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a Contribution.
 * Apache license notifications and license are retained
 * for attribution purposes only.
 *
 * Copyright (c) 2017, The Linux Foundation. All rights reserved.
 * Not a Contribution.
 *
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PA_QTI_EFFECT_H
#define PA_QTI_EFFECT_H

#define PA_QTI_MODULE_ID_PRIMARY "audio.primary"
#define PA_QTI_EFFECT_STRING_LEN_MAX 64

#define MAX_SUPPORTED_EFFECTS 5

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint32_t timeLow;
    uint16_t timeMid;
    uint16_t timeHiAndVersion;
    uint16_t clockSeq;
    uint8_t node[6];
} pa_qti_effect_uuid_t;

typedef struct {
    pa_qti_effect_uuid_t type; /* UUID of to the OpenSL ES interface implemented by this effect */
    pa_qti_effect_uuid_t uuid; /* UUID for this particular implementation */
    uint32_t api_version;      /* Version of the effect control API implemented */
    uint32_t flags;            /* effect engine capabilities/requirements flags */
    uint16_t cpuLoad;          /* CPU load indication */
    uint16_t memoryUsage;      /* Data Memory usage */
    char name[PA_QTI_EFFECT_STRING_LEN_MAX];        /* human readable effect name */
    char implementor[PA_QTI_EFFECT_STRING_LEN_MAX]; /* human readable effect implementor name */
} pa_qti_effect_descriptor_t;

typedef void* pa_qti_effect_handle_t;

/*
Standardized command codes for command() function
*/
enum pa_qti_effect_command_e {
   PA_QTI_EFFECT_CMD_INIT,                 /* initialize effect engine */
   PA_QTI_EFFECT_CMD_SET_CONFIG,           /* configure effect engine (see effect_config_t) */
   PA_QTI_EFFECT_CMD_RESET,                /* reset effect engine */
   PA_QTI_EFFECT_CMD_ENABLE,               /* enable effect process */
   PA_QTI_EFFECT_CMD_DISABLE,              /* disable effect process */
   PA_QTI_EFFECT_CMD_SET_PARAM,            /* set parameter immediately (see effect_param_t) */
   PA_QTI_EFFECT_CMD_SET_PARAM_DEFERRED,   /* set parameter deferred */
   PA_QTI_EFFECT_CMD_SET_PARAM_COMMIT,     /* commit previous set parameter deferred */
   PA_QTI_EFFECT_CMD_GET_PARAM,            /* get parameter */
   PA_QTI_EFFECT_CMD_SET_DEVICE,           /* set audio device (see audio.h, audio_devices_t) */
   PA_QTI_EFFECT_CMD_SET_VOLUME,           /* set volume */
   PA_QTI_EFFECT_CMD_SET_AUDIO_MODE,       /* set the audio mode (normal, ring, ...) */
   PA_QTI_EFFECT_CMD_SET_CONFIG_REVERSE,   /* configure effect engine reverse stream */
                                           /*   (see effect_config_t) */
   PA_QTI_EFFECT_CMD_SET_INPUT_DEVICE,     /* set capture device (see audio.h, audio_devices_t) */
   PA_QTI_EFFECT_CMD_GET_CONFIG,           /* read effect engine configuration */
   PA_QTI_EFFECT_CMD_GET_CONFIG_REVERSE,   /* read configure effect engine reverse stream */
                                           /* configuration */
   PA_QTI_EFFECT_CMD_GET_FEATURE_SUPPORTED_CONFIGS, /* get all supported configurations for a feature. */
   PA_QTI_EFFECT_CMD_GET_FEATURE_CONFIG,   /* get current feature configuration */
   PA_QTI_EFFECT_CMD_SET_FEATURE_CONFIG,   /* set current feature configuration */
   PA_QTI_EFFECT_CMD_SET_AUDIO_SOURCE,     /* set the audio source (see audio.h, audio_source_t) */
   PA_QTI_EFFECT_CMD_OFFLOAD,              /* set if effect thread is an offload one, */
                                           /* send the ioHandle of the effect thread */
   PA_QTI_EFFECT_CMD_FIRST_PROPRIETARY = 0x10000 /* first proprietary command code */
};

/*
==================================================================================================
 command: PA_QTI_EFFECT_CMD_INIT
--------------------------------------------------------------------------------------------------
 description:
  Initialize effect engine: All configurations return to default
--------------------------------------------------------------------------------------------------
 command format:
  size: 0
  data: N/A
--------------------------------------------------------------------------------------------------
 reply format:
  size: sizeof(int)
  data: status
==================================================================================================
 command: PA_QTI_EFFECT_CMD_SET_CONFIG
--------------------------------------------------------------------------------------------------
 description:
 Apply new audio parameters configurations for input and output buffers
--------------------------------------------------------------------------------------------------
 command format:
  size: sizeof(effect_config_t)
  data: effect_config_t
--------------------------------------------------------------------------------------------------
 reply format:
  size: sizeof(int)
  data: status
==================================================================================================
 command: PA_QTI_EFFECT_CMD_RESET
--------------------------------------------------------------------------------------------------
 description:
  Reset the effect engine. Keep configuration but resets state and buffer content
--------------------------------------------------------------------------------------------------
 command format:
  size: 0
  data: N/A
--------------------------------------------------------------------------------------------------
 reply format:
  size: 0
  data: N/A
==================================================================================================
 command: PA_QTI_EFFECT_CMD_ENABLE
--------------------------------------------------------------------------------------------------
 description:
  Enable the process. Called by the framework before the first call to process()
--------------------------------------------------------------------------------------------------
 command format:
  size: 0
  data: N/A
--------------------------------------------------------------------------------------------------
 reply format:
  size: sizeof(int)
  data: status
==================================================================================================
 command: PA_QTI_EFFECT_CMD_DISABLE
--------------------------------------------------------------------------------------------------
 description:
  Disable the process. Called by the framework after the last call to process()
--------------------------------------------------------------------------------------------------
 command format:
  size: 0
  data: N/A
--------------------------------------------------------------------------------------------------
 reply format:
  size: sizeof(int)
  data: status
==================================================================================================
 command: PA_QTI_EFFECT_CMD_SET_PARAM
--------------------------------------------------------------------------------------------------
 description:
  Set a parameter and apply it immediately
--------------------------------------------------------------------------------------------------
 command format:
  size: sizeof(effect_param_t) + size of param and value
  data: effect_param_t + param + value. See effect_param_t definition below for value offset
--------------------------------------------------------------------------------------------------
 reply format:
  size: sizeof(int)
  data: status
==================================================================================================
 command: PA_QTI_EFFECT_CMD_SET_PARAM_DEFERRED
--------------------------------------------------------------------------------------------------
 description:
  Set a parameter but apply it only when receiving PA_QTI_EFFECT_CMD_SET_PARAM_COMMIT command
--------------------------------------------------------------------------------------------------
 command format:
  size: sizeof(effect_param_t) + size of param and value
  data: effect_param_t + param + value. See effect_param_t definition below for value offset
--------------------------------------------------------------------------------------------------
 reply format:
  size: 0
  data: N/A
==================================================================================================
 command: PA_QTI_EFFECT_CMD_SET_PARAM_COMMIT
--------------------------------------------------------------------------------------------------
 description:
  Apply all previously received PA_QTI_EFFECT_CMD_SET_PARAM_DEFERRED commands
--------------------------------------------------------------------------------------------------
 command format:
  size: 0
  data: N/A
--------------------------------------------------------------------------------------------------
 reply format:
  size: sizeof(int)
  data: status
==================================================================================================
 command: PA_QTI_EFFECT_CMD_GET_PARAM
--------------------------------------------------------------------------------------------------
 description:
  Get a parameter value
--------------------------------------------------------------------------------------------------
 command format:
  size: sizeof(effect_param_t) + size of param
  data: effect_param_t + param
--------------------------------------------------------------------------------------------------
 reply format:
  size: sizeof(effect_param_t) + size of param and value
  data: effect_param_t + param + value. See effect_param_t definition below for value offset
==================================================================================================
 command: PA_QTI_EFFECT_CMD_SET_DEVICE
--------------------------------------------------------------------------------------------------
 description:
  Set the rendering device the audio output path is connected to. See audio.h, audio_devices_t
  for device values.
  The effect implementation must set PA_QTI_EFFECT_FLAG_DEVICE_IND flag in its descriptor to
  receive this command when the device changes
--------------------------------------------------------------------------------------------------
 command format:
  size: sizeof(uint32_t)
  data: uint32_t
--------------------------------------------------------------------------------------------------
 reply format:
  size: 0
  data: N/A
==================================================================================================
 command: PA_QTI_EFFECT_CMD_SET_VOLUME
--------------------------------------------------------------------------------------------------
 description:
  Set and get volume. Used by audio framework to delegate volume control to effect engine.
  The effect implementation must set PA_QTI_EFFECT_FLAG_VOLUME_IND or PA_QTI_EFFECT_FLAG_VOLUME_CTRL
  flag in its descriptor to receive this command before every call to process() function
  If PA_QTI_EFFECT_FLAG_VOLUME_CTRL flag is set in the effect descriptor, the effect engine must
  return the volume that should be applied before the effect is processed. The overall volume
  (the volume actually applied by the effect engine multiplied by the returned value) should
  match the value indicated in the command.
--------------------------------------------------------------------------------------------------
 command format:
  size: n * sizeof(uint32_t)
  data: volume for each channel defined in effect_config_t for output buffer expressed in
      8.24 fixed point format
--------------------------------------------------------------------------------------------------
 reply format:
  size: n * sizeof(uint32_t) / 0
  data: - if PA_QTI_EFFECT_FLAG_VOLUME_CTRL is set in effect descriptor:
              volume for each channel defined in effect_config_t for output buffer expressed in
              8.24 fixed point format
        - if PA_QTI_EFFECT_FLAG_VOLUME_CTRL is not set in effect descriptor:
              N/A
  It is legal to receive a null pointer as pReplyData in which case the effect framework has
  delegated volume control to another effect
==================================================================================================
 command: PA_QTI_EFFECT_CMD_SET_AUDIO_MODE
--------------------------------------------------------------------------------------------------
 description:
  Set the audio mode. The effect implementation must set PA_QTI_EFFECT_FLAG_AUDIO_MODE_IND flag
  in its descriptor to receive this command when the audio mode changes.
-------------------------------------------------------------------------------------------------
 command format:
  size: sizeof(uint32_t)
  data: audio_mode_t
--------------------------------------------------------------------------------------------------
 reply format:
  size: 0
  data: N/A
==================================================================================================
 command: PA_QTI_EFFECT_CMD_SET_CONFIG_REVERSE
--------------------------------------------------------------------------------------------------
 description:
  Apply new audio parameters configurations for input and output buffers of reverse stream.
  An example of reverse stream is the echo reference supplied to an Acoustic Echo Canceler.
--------------------------------------------------------------------------------------------------
 command format:
  size: sizeof(effect_config_t)
  data: effect_config_t
--------------------------------------------------------------------------------------------------
 reply format:
  size: sizeof(int)
  data: status
==================================================================================================
 command: PA_QTI_EFFECT_CMD_SET_INPUT_DEVICE
--------------------------------------------------------------------------------------------------
 description:
  Set the capture device the audio input path is connected to. See audio.h, audio_devices_t
  for device values.
  The effect implementation must set PA_QTI_EFFECT_FLAG_DEVICE_IND flag in its descriptor to
  receive this command when the device changes
--------------------------------------------------------------------------------------------------
 command format:
  size: sizeof(uint32_t)
  data: uint32_t
--------------------------------------------------------------------------------------------------
 reply format:
  size: 0
  data: N/A
==================================================================================================
 command: PA_QTI_EFFECT_CMD_GET_CONFIG
--------------------------------------------------------------------------------------------------
 description:
  Read audio parameters configurations for input and output buffers
--------------------------------------------------------------------------------------------------
 command format:
  size: 0
  data: N/A
--------------------------------------------------------------------------------------------------
 reply format:
  size: sizeof(effect_config_t)
  data: effect_config_t
==================================================================================================
 command: PA_QTI_EFFECT_CMD_GET_CONFIG_REVERSE
--------------------------------------------------------------------------------------------------
 description:
  Read audio parameters configurations for input and output buffers of reverse stream
--------------------------------------------------------------------------------------------------
 command format:
  size: 0
  data: N/A
--------------------------------------------------------------------------------------------------
 reply format:
  size: sizeof(effect_config_t)
  data: effect_config_t
==================================================================================================
 command: PA_QTI_EFFECT_CMD_GET_FEATURE_SUPPORTED_CONFIGS
--------------------------------------------------------------------------------------------------
 description:
 Queries for supported configurations for a particular feature (e.g. get the supported
 combinations of main and auxiliary channels for a noise suppressor).
 The command parameter is the feature identifier (See effect_feature_e for a list of defined
 features) followed by the maximum number of configuration descriptor to return.
 The reply is composed of:
  - status (uint32_t):
          - 0 if feature is supported
          - -ENOSYS if the feature is not supported,
          - -ENOMEM if the feature is supported but the total number of supported configurations
          exceeds the maximum number indicated by the caller.
  - total number of supported configurations (uint32_t)
  - an array of configuration descriptors.
 The actual number of descriptors returned must not exceed the maximum number indicated by
 the caller.
--------------------------------------------------------------------------------------------------
 command format:
  size: 2 x sizeof(uint32_t)
  data: effect_feature_e + maximum number of configurations to return
--------------------------------------------------------------------------------------------------
 reply format:
  size: 2 x sizeof(uint32_t) + n x sizeof (<config descriptor>)
  data: status + total number of configurations supported + array of n config descriptors
==================================================================================================
 command: PA_QTI_EFFECT_CMD_GET_FEATURE_CONFIG
--------------------------------------------------------------------------------------------------
 description:
  Retrieves current configuration for a given feature.
 The reply status is:
      - 0 if feature is supported
      - -ENOSYS if the feature is not supported,
--------------------------------------------------------------------------------------------------
 command format:
  size: sizeof(uint32_t)
  data: effect_feature_e
--------------------------------------------------------------------------------------------------
 reply format:
  size: sizeof(uint32_t) + sizeof (<config descriptor>)
  data: status + config descriptor
==================================================================================================
 command: PA_QTI_EFFECT_CMD_SET_FEATURE_CONFIG
--------------------------------------------------------------------------------------------------
 description:
  Sets current configuration for a given feature.
 The reply status is:
      - 0 if feature is supported
      - -ENOSYS if the feature is not supported,
      - -EINVAL if the configuration is invalid
--------------------------------------------------------------------------------------------------
 command format:
  size: sizeof(uint32_t) + sizeof (<config descriptor>)
  data: effect_feature_e + config descriptor
--------------------------------------------------------------------------------------------------
 reply format:
  size: sizeof(uint32_t)
  data: status
==================================================================================================
 command: PA_QTI_EFFECT_CMD_SET_AUDIO_SOURCE
--------------------------------------------------------------------------------------------------
 description:
  Set the audio source the capture path is configured for (Camcorder, voice recognition...).
  See audio.h, audio_source_t for values.
--------------------------------------------------------------------------------------------------
 command format:
  size: sizeof(uint32_t)
  data: uint32_t
--------------------------------------------------------------------------------------------------
 reply format:
  size: 0
  data: N/A
==================================================================================================
 command: PA_QTI_EFFECT_CMD_OFFLOAD
--------------------------------------------------------------------------------------------------
 description:
  1.indicate if the playback thread the effect is attached to is offloaded or not
  2.update the io handle of the playback thread the effect is attached to
--------------------------------------------------------------------------------------------------
 command format:
  size: sizeof(effect_offload_param_t)
  data: effect_offload_param_t
--------------------------------------------------------------------------------------------------
 reply format:
  size: sizeof(uint32_t)
  data: uint32_t
--------------------------------------------------------------------------------------------------
 command: PA_QTI_EFFECT_CMD_FIRST_PROPRIETARY
--------------------------------------------------------------------------------------------------
 description:
  All proprietary effect commands must use command codes above this value. The size and format of
  command and response fields is free in this case
==================================================================================================
*/

/*
 effect_param_s structure describes the format of the pCmdData argument of
 PA_QTI_EFFECT_CMD_SET_PARAM command and pCmdData and pReplyData of PA_QTI_EFFECT_CMD_GET_PARAM
 command. psize and vsize represent the actual size of parameter and value.

 NOTE: the start of value field inside the data field is always on a 32 bit boundary:

  +-----------+
  | status    | sizeof(int)
  +-----------+
  | psize     | sizeof(int)
  +-----------+
  | vsize     | sizeof(int)
  +-----------+
  |           |   |           |
  ~ parameter ~   > psize     |
  |           |   |           >  ((psize - 1)/sizeof(int) + 1) * sizeof(int)
  +-----------+               |
  | padding   |               |
  +-----------+
  |           |   |
  ~ value     ~   > vsize
  |           |   |
  +-----------+
*/
typedef struct {
    int32_t     status;     /* Transaction status (unused for command, used for reply) */
    uint32_t    psize;      /* Parameter size */
    uint32_t    vsize;      /* Value size */
    char        data[];     /* Start of Parameter + Value data */
} pa_qti_effect_param_t;

/* structure used by PA_QTI_EFFECT_CMD_OFFLOAD command */
typedef struct {
    bool isOffload;         /* true if the playback thread the effect is attached to is offloaded */
    int ioHandle;           /* io handle of the playback thread the effect is attached to */
} pa_qti_effect_offload_param_t;

/* Effect library interface */

int pa_qti_effect_init(void);

void pa_qti_effect_deinit(void);

/* Returns the descriptors of all the supported effects. */
int pa_qti_module_get_supported_effects(uint32_t *num_effects,
                                        pa_qti_effect_descriptor_t *desc);

/* Returns UUIDs of all the effects supported on the provided sink */
int pa_qti_sink_get_supported_effects(uint32_t sink_index,
                                      uint32_t *sink_effects,
                                      pa_qti_effect_uuid_t *uuid_list);

/* Returns UUIDs of all the effects supported on the provided port */
int pa_qti_port_get_supported_effects(const char *port_name,
                                      uint32_t *port_effects,
                                      uint32_t *device_index,
                                      pa_qti_effect_uuid_t *uuid_list);

/* Creates the effect of the specified implementation uuid on the sink corresponding to the sink id and returns the effect handle. */
pa_qti_effect_handle_t pa_qti_sink_effect_create(const pa_qti_effect_uuid_t uuid,
                                                 uint32_t sink_index);

/* Creates the effect of the specified implementation uuid on the loopback session corresponding to the given patch id and returns the effect handle. */
pa_qti_effect_handle_t pa_qti_loopback_effect_create(const pa_qti_effect_uuid_t uuid,
                                                     uint64_t patch_id);

/* Creates the effect of the specified implementation uuid on the port given and returns the effect handle. */
pa_qti_effect_handle_t pa_qti_port_effect_create(const pa_qti_effect_uuid_t uuid,
                                                 const char *port_name);

/* Returns descriptor of the effect corresponding to the specified uuid. */
int pa_qti_effect_get_descriptor(pa_qti_effect_handle_t effect_handle,
                                 const pa_qti_effect_uuid_t uuid,
                                 pa_qti_effect_descriptor_t *desc);

/* Releases the effect engine corresponding to the given handle. */
int pa_qti_sink_effect_release(pa_qti_effect_handle_t effect_handle);

/* Releases the effect engine corresponding to the given handle. */
int pa_qti_port_effect_release(pa_qti_effect_handle_t effect_handle);

/* Releases the effect engine corresponding to the given handle. */
int pa_qti_loopback_effect_release(pa_qti_effect_handle_t effect_handle);

/* Get version of effect APIs */
int pa_qti_effect_get_version(void);

/* Send a command and receive a response to/from effect engine. */
int pa_qti_effect_command(pa_qti_effect_handle_t effect_handle,
                          uint32_t cmd_code,
                          uint32_t cmd_size,
                          void *cmd_data,
                          uint32_t reply_size,
                          void *reply_data);
#endif
