/*
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "asd_ext_algo_wrapper.h"

typedef struct asd_wrapper_algo_internal_t {
  void            *asd_default_internal; /* Algo private data */
  void            *asd_oem_internal; /* OEM algo private data */
  asd_ops_t       default_ops;
  asd_ops_t       oem_ops;
} asd_wrapper_algo_internal_t;



/**
 * asd_wrapper_set_parameters
 *
 * @param: Msg to set data
 * @asd_object: Data structure to hold interface with ASD algo
 *
 * Set data to availables algo
 *
 * Return: Void
 **/
boolean asd_wrapper_set_parameters(asd_set_parameter_t *param,
  void *asd_obj)
{
  asd_wrapper_algo_internal_t *asd = (asd_wrapper_algo_internal_t*)asd_obj;

  if (NULL == asd) {
    return FALSE;
  }

  if (asd->asd_default_internal && asd->default_ops.set_parameters) {
    asd->default_ops.set_parameters(param, asd->asd_default_internal);
  }
  if (asd->asd_oem_internal && asd->oem_ops.set_parameters) {
    asd->oem_ops.set_parameters(param, asd->asd_oem_internal);
  }

  return TRUE;
}

/**
 * asd_wraper_get_parameters
 *
 * @param: Msg to request data
 * @asd_object: Data structure to hold interface with ASD algo
 *
 * Request data to availables algo
 *
 * Return: Void
 **/
boolean asd_wraper_get_parameters(asd_get_parameter_t *param, void *asd_obj)
{
  asd_wrapper_algo_internal_t *asd = (asd_wrapper_algo_internal_t*)asd_obj;

  if (NULL == asd) {
    return FALSE;
  }

  if (asd->asd_default_internal && asd->default_ops.get_parameters) {
    asd->default_ops.get_parameters(param, asd->asd_default_internal);
  }
  if (asd->asd_oem_internal && asd->oem_ops.get_parameters) {
    asd->oem_ops.get_parameters(param, asd->asd_oem_internal);
  }

  return TRUE;
}


/**
 * asd_wrapper_process
 *
 * @process_data: Stats data
 * @asd_object: Data structure to hold interface with ASD algo
 * @output: Process output is save here
 *
 * Do the calls to process stats data for the available algos
 *
 * Return: Void
 **/
void asd_wrapper_process(asd_process_data_t *process_data, void *asd_obj,
  asd_output_data_t *output)
{
  asd_wrapper_algo_internal_t *asd = (asd_wrapper_algo_internal_t*)asd_obj;

  if (NULL == asd) {
    return;
  }

  if (asd->asd_default_internal && asd->default_ops.process) {
    asd->default_ops.process(process_data, asd->asd_default_internal,
      output);
  }
  if (asd->asd_oem_internal && asd->oem_ops.process) {
    asd->oem_ops.process(process_data, asd->asd_oem_internal,
      output);
  }

  return;
}

/**
 * asd_wrapper_destroy
 *
 * @asd_wrapper_algo_internal: Data structure to hold interface with ASD algo
 *
 * Feee resources allocated by algo
 *
 * Return: Void
 **/
void asd_wrapper_destroy(void *asd_wrapper_algo_internal)
{
  asd_wrapper_algo_internal_t *asd =
    (asd_wrapper_algo_internal_t*)asd_wrapper_algo_internal;

  if (NULL == asd) {
    return;
  }

  ASD_HIGH("Do destroy");
  if (asd->asd_oem_internal && asd->oem_ops.deinit) {
    ASD_HIGH("OEM deinit");
    asd->oem_ops.deinit(asd->asd_oem_internal);
    asd->asd_oem_internal = NULL;
  }
  if (asd->asd_default_internal && asd->default_ops.deinit) {
    ASD_HIGH("Default deinit");
    asd->default_ops.deinit(asd->asd_default_internal);
    asd->asd_default_internal = NULL;
  }

  free(asd);
  asd = NULL;
  return;
}

/**
 * asd_wrapper_init
 *
 * @asd_object: Data structure to hold interface with ASD algo
 *
 * Algo allocate resources here.
 *
 * Return: Void
 **/
void *asd_wrapper_init(void *handler)
{
  asd_wrapper_iface_handle_t *iface_handle =
    (asd_wrapper_iface_handle_t*)handler;
  asd_wrapper_algo_internal_t *asd = NULL;
  asd = calloc(1, sizeof(asd_wrapper_algo_internal_t));
  if (NULL == asd) {
    return NULL;
  }

  do {
    if (iface_handle->default_ops.init) {
      ASD_HIGH("Default init");
      asd->asd_default_internal = iface_handle->default_ops.init(NULL);
      if (!asd->asd_default_internal) {
        break;
      }
      /* Save ops in internal struct to be use by wrapper */
      asd->default_ops = iface_handle->default_ops;
    }
    if (iface_handle->oem_ops.init) {
      ASD_HIGH("OEM init");
      asd->asd_oem_internal = iface_handle->oem_ops.init(NULL);
      if (!asd->asd_oem_internal) {
        break;
      }
      /* Save ops in internal struct to be use by wrapper */
      asd->oem_ops = iface_handle->oem_ops;
    }
    return asd;
  } while (0);

  /* Handle errors */
  if (asd->asd_oem_internal && iface_handle->oem_ops.deinit) {
    iface_handle->oem_ops.deinit(asd->asd_oem_internal);
  }
  if (asd->asd_default_internal && iface_handle->default_ops.deinit) {
    iface_handle->default_ops.deinit(asd->asd_default_internal);
  }

  if (asd) {
    free(asd);
    asd = NULL;
  }
  return NULL;
}


/**
 * asd_wrapper_unlink_algo
 *
 * @asd_ops: Holder of function pointers to wrapper
 *
 * Reset function pointers to wrapper
 *
 * Return: Void
 **/
void asd_wrapper_unlink_algo(asd_ops_t *asd_ops)
{
  asd_ops->set_parameters = NULL;
  asd_ops->get_parameters = NULL;
  asd_ops->process        = NULL;
  asd_ops->init           = NULL;
  asd_ops->deinit         = NULL;

  return;
}

/**
 * asd_wrapper_link_algo
 *
 * @asd_ops: Holder of function pointers to wrapper
 *
 * Set/init function pointers to wrapper
 *
 * Return: Void
 **/
void asd_wrapper_link_algo(asd_ops_t *asd_ops)
{
  asd_ops->set_parameters = asd_wrapper_set_parameters;
  asd_ops->get_parameters = asd_wraper_get_parameters;
  asd_ops->process        = asd_wrapper_process;
  asd_ops->init           = asd_wrapper_init;
  asd_ops->deinit         = asd_wrapper_destroy;

  return;
}
