/* af_value_monitor.h

 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __AF_VALUEMONITOR_H__
#define __AF_VALUEMONITOR_H__

/* ----------------------------------------------------------------------*/
/* Header files & define for MATLAB (Should be removed for C on MTP)
 * ----------------------------------------------------------------------*/
#include "q3a_platform.h"
#include "stats_chromatix_wrapper.h"

#define FILTER_HISTORY_SIZE 10
#define VM_NAME_SIZE_MAX 30
/* -----------------------------------------------------------------------
 * Fixed Point Implementation of Value Monitor (Internally)
 * Q16.15 is utilized for the fixed point implementation
 * ----------------------------------------------------------------------*/
#define BIT_SHIFT 15

/**************************************************************************
 * Structures for external input parameters
 * (1) floating point coefficients
 *************************************************************************/
/**************************************************************************
 * Structures for internal usage
 * (1) fixed point coefficients
 * (2) additional parameters
 *************************************************************************/
typedef struct _af_median_fltr_intern_config_t {
        int                         enable;
        int                         num_of_samples;
} af_median_fltr_intern_config_t;

typedef struct _af_iir_fltr_intern_config_t {
        int                         enable;
        int                         num_of_samples;
        int32_t                     coeff_numerator;
        int32_t                     coeff_denominator;
} af_iir_fltr_intern_config_t;

typedef struct _af_mvavg_fltr_intern_config_t {
        int                         enable;
        int                         num_of_samples;
        int32_t                     coeff;
 } af_mvavg_fltr_intern_config_t;

typedef struct _af_abs_mode_internal_config {
        int                        enable;
        int32_t                    baseline;
 } af_abs_mode_intern_config_t;

typedef struct _af_vm_intern_config_t {
        af_vm_detector_type_adapter_enum         detection_type;
        int32_t                                  trigger_threshold;
        int                                      consecutive_count_threshold;
        int                                      frame2armed;
        af_abs_mode_intern_config_t              absolute_mode_cfg;
        /* Current filter configurations */
        af_median_fltr_intern_config_t           median_fltr_cfg;
        af_mvavg_fltr_intern_config_t            mvavg_fltr_cfg;
        af_iir_fltr_intern_config_t              iir_fltr_cfg;
        /* Value Monitor parameters for three sensitivities */
        af_vm_sens_profile_adapter_t             sens_profile_low;
        af_vm_sens_profile_adapter_t             sens_profile_med;
        af_vm_sens_profile_adapter_t             sens_profile_high;
 } af_vm_intern_config_t;

typedef struct _af_median_fltr_intern_prop_t {
        int32_t                       buffer[FILTER_HISTORY_SIZE];
 } af_median_fltr_intern_prop_t;

typedef struct _af_iir_fltr_intern_prop_t {
        int32_t                       buffer_num[FILTER_HISTORY_SIZE];
        int32_t                       buffer_den[FILTER_HISTORY_SIZE];
 } af_iir_fltr_intern_prop_t;

typedef struct _af_mvavg_fltr_intern_prop_t {
        int32_t                       buffer[FILTER_HISTORY_SIZE];
 } af_mvavg_fltr_intern_prop_t;

typedef struct _af_vm_intern_params_t {
        int                               rebasing;
        int                               delay_counter;
        int                               frame_counter;
        int                               sensitivity;
        int32_t                           baseline;
        int32_t                           residual;
        float                             metafilter_output;
        af_median_fltr_intern_prop_t      median_fltr_prop;
        af_iir_fltr_intern_prop_t         iir_fltr_prop;
        af_mvavg_fltr_intern_prop_t       mvavg_fltr_prop;
 } af_vm_intern_params_t;

typedef struct _af_value_monitor_internal_t af_value_monitor_internal_t;
/*************************************************************************/
/** _af_value_monitor_internal_t:
 *     Custom Mixer algo internal struct
 *
 *     @ af_alg : mandatary object for module
 *     @ p_child_algo : ptr to child algo base structure
 *     @ p_alg_util : ptr to utility func by HAF
 *     @ params : internal parameters
 *     @ ctrl : internal control parameters
 *
 **/
struct _af_value_monitor_internal_t {
  boolean (*get_trigger)(af_value_monitor_internal_t *);
  boolean (*set_config)(af_value_monitor_internal_t *, af_vm_config_adapter_t *, int);
  boolean (*set_input)(af_value_monitor_internal_t *, float input);
  boolean (*set_sensitivity)(af_value_monitor_internal_t *, int);
  boolean (*rebase_ref)(af_value_monitor_internal_t *);
  af_vm_intern_config_t af_vm_intern_config;
  af_vm_intern_params_t af_vm_intern_params;
  float input;
  boolean decision;
  boolean FlagDisplayLog;
  char name[VM_NAME_SIZE_MAX];
};

af_value_monitor_internal_t* af_value_monitor_create(const char *name);
void af_value_monitor_destroy(af_value_monitor_internal_t *pMonitor);

#endif /* __AF_VALUEMONITOR_H__ */

