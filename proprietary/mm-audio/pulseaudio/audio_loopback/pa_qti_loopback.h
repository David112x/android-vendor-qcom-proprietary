/* pa_qti_loopback.h
*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*
*/

#ifndef PA_QTI_LOOPBACK_H
#define PA_QTI_LOOPBACK_H

#include <pulse/format.h>
#include <pulse/sample.h>

#define PA_QTI_LOOPBACK_MODULE_ID_PRIMARY "audio.primary"
#define NUM_CHANNEL_POSITIONS 8

#define PA_QTI_LOOPBACK_PORT_CONFIG_SAMPLE_RATE  0x1
#define PA_QTI_LOOPBACK_PORT_CONFIG_CHANNEL_MASK 0x2
#define PA_QTI_LOOPBACK_PORT_CONFIG_FORMAT       0x4
#define PA_QTI_LOOPBACK_PORT_CONFIG_GAIN         0x8
#define PA_QTI_LOOPBACK_PORT_CONFIG_AUTO         0x10
#define PA_QTI_LOOPBACK_PORT_CONFIG_ALL (PA_QTI_LOOPBACK_PORT_CONFIG_SAMPLE_RATE | \
                                         PA_QTI_LOOPBACK_PORT_CONFIG_CHANNEL_MASK | \
                                         PA_QTI_LOOPBACK_PORT_CONFIG_FORMAT | \
                                         PA_QTI_LOOPBACK_PORT_CONFIG_GAIN)

#define PA_QTI_LOOPBACK_ERROR_EVENT_STRING "qahw loopback error event"
#define PA_QTI_LOOPBACK_FORMAT_UPDATE_EVENT_STRING "qahw loopback format update event"
#define PA_QTI_LOOPBACK_RECREATED_EVENT_STRING "qahw loopback session recreated event"

typedef int pa_qti_loopback_port_handle_t;

typedef unsigned int pa_qti_loopback_channel_mask_t;

typedef enum {
    PA_QTI_LOOPBACK_PORT_ROLE_NONE,
    PA_QTI_LOOPBACK_PORT_ROLE_SOURCE,
    PA_QTI_LOOPBACK_PORT_ROLE_SINK,
} pa_qti_loopback_port_role_t;

struct pa_qti_loopback_port_config {
    pa_qti_loopback_port_handle_t    id;
    pa_qti_loopback_port_role_t      role;
    unsigned int                     config_mask;
    unsigned int                     sample_rate;
    unsigned int                     num_channels;
    pa_encoding_t                    format;
    pa_sample_format_t               bitwidth;
    double                           gain;
    char                             *port_name;
};

typedef enum {
    PA_QTI_LOOPBACK_PATCH_HANDLE_NONE = 0,
} pa_qti_loopback_handle_t;

typedef enum {
    PA_QTI_LOOPBACK_MODE_PCM = 0,
    PA_QTI_LOOPBACK_MODE_COMPRESS = 1,
} pa_qti_loopback_mode_t;

typedef enum {
    PA_QTI_LOOPBACK_ERROR_EVENT,
    PA_QTI_LOOPBACK_FORMAT_UPDATE_EVENT,
    PA_QTI_LOOPBACK_RECREATED_EVENT,
} pa_qti_loopback_event_t;

struct pa_qti_loopback_event_data {
    unsigned int     sample_rate;
    unsigned int     channels;
    const char       *encoding_string;
    const char       *port_name;
    char             **channel_maps;
};

/* Function declarations */
int pa_qti_loopback_init(const char *module_name);

void pa_qti_loopback_deinit(void);

typedef void (*pa_qti_loopback_event_callback_t)(pa_qti_loopback_event_t event,
                                                 struct pa_qti_loopback_event_data event_data);

pa_qti_loopback_handle_t pa_qti_loopback_create(unsigned int num_srcs,
                                                struct pa_qti_loopback_port_config *src_cfg,
                                                unsigned int num_sinks,
                                                struct pa_qti_loopback_port_config *sink_cfg,
                                                pa_qti_loopback_event_callback_t callback);

int pa_qti_loopback_stop(pa_qti_loopback_handle_t id);

int pa_qti_loopback_get_port_config(struct pa_qti_loopback_port_config *cfg,
                                    pa_qti_loopback_port_handle_t id,
                                    pa_qti_loopback_port_role_t role);

int pa_qti_loopback_set_port_config(struct pa_qti_loopback_port_config *cfg);

#endif
