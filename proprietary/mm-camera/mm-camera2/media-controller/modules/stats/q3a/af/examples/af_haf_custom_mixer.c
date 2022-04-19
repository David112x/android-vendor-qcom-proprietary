/* af_haf_custom_mixer.c
 *
 * Copyright (c) 2015-2016 Qualcomm Technologies, Inc. 
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#include "af_alg_interface.h"
#include "af_alg_util_interface.h"
#include <math.h>
/* =======================================================

DEFINITIONS AND DECLARATIONS

This section contains definitions for constants, macros, types, variables
and other items needed by this interface.

========================================================= */
/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */

#define AF_CUSTOMMIXER_DEBUG

#ifdef AF_CUSTOMMIXER_DEBUG
  #undef CDBG_HAF
  #define CDBG_HAF HAF_MSG_LOW
#else
  #undef CDBG_HAF
  #define CDBG_HAF(fmt, args...)  do{}while(0)
#endif


#define LOW_LIGHT_THRESHOLD (183)
/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
  /** _af_alg_cmixer_internal_t:
  *     Custom Mixer algo internal struct
  *
  **/
  typedef struct _af_alg_cmixer_internal_t {
    af_alg_interface_t                  af_alg;
    af_alg_util_interface_t            *p_alg_util;
    af_alg_interface_t                 *p_pdaf_alg;
    af_alg_interface_t                 *p_tof_alg;
    boolean                             is_lowLight;
  } af_alg_cmixer_internal_t;

/* -----------------------------------------------------------------------
** Forward Declarations
 ** ----------------------------------------------------------------------- */

/* Factory create functions */
af_alg_interface_t* af_alg_haf_mixer_create();
af_alg_interface_t* af_alg_TOF_create(
    af_alg_util_interface_t *p_alg_util_instance);
af_alg_interface_t* af_alg_PDAF_create(
    af_alg_util_interface_t *p_alg_util_instance);
af_alg_util_interface_t* af_alg_util_create(void);

af_alg_interface_t* af_factory_create_by_type(af_haf_algo_enum_type eAlg_type);

af_alg_interface_t* af_alg_CMIXER_create(
    af_alg_util_interface_t *p_alg_util_instance);
static void af_alg_interface_init(af_alg_cmixer_internal_t *af_alg);
static boolean af_cmixer_set_parameters(af_alg_interface_t *p_alg,
    af_haf_set_parameter_t *param);
static boolean af_cmixer_set_inputs(af_alg_interface_t *p_alg,
    af_haf_set_parameter_t *param);
static boolean af_cmixer_set_tuning(af_alg_interface_t *p_alg,
    af_haf_tune_params_t *haf_tuning);
static void af_cmixer_set_mode(af_alg_interface_t *p_alg, af_mode_type mode);
static void af_cmixer_set_roi(af_alg_interface_t *p_alg, af_roi_info_t *roi);
static void af_cmixer_set_range(af_alg_interface_t *p_alg, int near, int far);
static boolean af_cmixer_process_monitor(af_alg_interface_t *p_alg,
    af_haf_monitor_data_t *output, int cur_pos);
static boolean af_cmixer_process_search(af_alg_interface_t *p_alg,
    af_haf_focus_search_data_t *output, int cur_pos);
static void af_cmixer_destroy(af_alg_interface_t *p_alg);

/* -----------------------------------------------------------------------
** Function Defintions
** ----------------------------------------------------------------------- */

/** af_cmixer_rebase_reference:
*   Notification of focus search end & reset any reference
*   parameters if required.
*
*  Return: TRUE on success else FALSE
**/
boolean af_cmixer_rebase_reference(af_alg_interface_t *p_alg)
{
  boolean result = FALSE;
  HAF_MSG_INFO("Enter !");

  /* NULL pointer check */
  if (NULL == p_alg) {
    return FALSE;
  }

  /* Typecast from af_alg_interface_t to af_alg_PDAF_internal_t */
  af_alg_cmixer_internal_t *pMixer = (af_alg_cmixer_internal_t *)p_alg;

  if(pMixer->p_tof_alg->process_search){
    result = pMixer->p_tof_alg->rebase_reference(pMixer->p_tof_alg);
  }

  if(pMixer->p_pdaf_alg->process_search){
    result = pMixer->p_pdaf_alg->rebase_reference(pMixer->p_pdaf_alg);
  }

  return result;
}

/** af_alg_CMIXER_create: Create instance of Custom Mixer. Mallocate  internal structure
*                        specifically and initializes each parameter.
*
*  @ p_alg_util_instance: AF algorithm util instance pointer
*
*  Return: af_alg_interface_t
**/
af_alg_interface_t* af_alg_CMIXER_create(af_alg_util_interface_t *p_alg_util_instance)
{
  CDBG_HAF("Enter !");
  /* Malloc for PDAF internal structure */

  af_alg_cmixer_internal_t* p_CMIXER_instance = (void *)malloc(sizeof(af_alg_cmixer_internal_t));
  if (NULL == p_CMIXER_instance) {
    /* Malloc failed */
    return NULL;
  }

  af_alg_interface_init(p_CMIXER_instance);

  /* Populate PDAF specific function pointers */
  p_CMIXER_instance->af_alg.set_parameters      = af_cmixer_set_parameters;
  p_CMIXER_instance->af_alg.set_inputs          = af_cmixer_set_inputs;
  p_CMIXER_instance->af_alg.set_tuning          = af_cmixer_set_tuning;
  p_CMIXER_instance->af_alg.set_mode            = af_cmixer_set_mode;
  p_CMIXER_instance->af_alg.set_roi             = af_cmixer_set_roi;
  p_CMIXER_instance->af_alg.set_range           = af_cmixer_set_range;
  p_CMIXER_instance->af_alg.process_monitor     = af_cmixer_process_monitor;
  p_CMIXER_instance->af_alg.process_search      = af_cmixer_process_search;
  p_CMIXER_instance->af_alg.destroy             = af_cmixer_destroy;
  p_CMIXER_instance->af_alg.rebase_reference    = af_cmixer_rebase_reference;

  /* Copy the ext func util instance pass during create time */
  p_CMIXER_instance->p_alg_util                 = p_alg_util_instance;
  p_CMIXER_instance->p_tof_alg                  =
    af_factory_create_by_type(AF_HAF_ALGO_TOF);
  p_CMIXER_instance->p_pdaf_alg                 =
    af_factory_create_by_type(AF_HAF_ALGO_PDAF);

  /* Typecast to af_alg_interface_t defined in af_alg_interface.h */
  return (af_alg_interface_t*)p_CMIXER_instance;
}


/** af_alg_interface_init: Init interface pointers and internal variables to default values
*
*  Return: void
**/
static void af_alg_interface_init(af_alg_cmixer_internal_t *af_alg)
{
  if (NULL == af_alg) {
    return;
  }

  /* First set all parameters within internal type to 0 */
  memset(af_alg, 0, sizeof(af_alg_cmixer_internal_t));
  /*Initialize the default value*/
  af_alg->is_lowLight                = TRUE;
  return;
}


/** af_cmixer_set_parameters:
*set parameter for caller to set Algo configuration .
*
*@p_alg : af_alg_interface pointer.
*@param : configuration to be set.
*Return: boolean
**/
boolean af_cmixer_set_parameters(af_alg_interface_t *p_alg, af_haf_set_parameter_t *param)
{
  CDBG_HAF("Enter !");
  boolean result = TRUE;

  /* NULL pointer check */
  if (NULL == p_alg || NULL == param) {
    result = FALSE;
    return result;
  }

  /* Typecast from af_alg_interface_t to af_alg_PDAF_internal_t */
  af_alg_cmixer_internal_t *pMixer = (af_alg_cmixer_internal_t *)p_alg;

  /* Any algorithm implementation may route unsupported param to the Set_params method */
  switch (param->type)
  {
  /* As per the design, SetInput/Invalidate interface is added to pass
  * events with big payloads. For other cases, setparam should be used */
    case AF_HAF_SET_PARAM_UPDATE_AEC_INFO:
    {
      if(param->u.aec_info.lux_idx < LOW_LIGHT_THRESHOLD){
        pMixer->is_lowLight = FALSE;
      } else {
        pMixer->is_lowLight = TRUE;
      }
      break;
    }
    default:
      break;
  }
  result = pMixer->p_tof_alg->set_parameters(pMixer->p_tof_alg, param);
  result &= pMixer->p_pdaf_alg->set_parameters(pMixer->p_pdaf_alg, param);

  return result;
}


/** af_cmixer_set_inputs: set input parameter for caller to set  algorithm inputs.
*
*  @p_alg : af_alg_interface pointer.
*  @param : Algorithm configuration to be set.
*
*  Return: void
**/
boolean af_cmixer_set_inputs (af_alg_interface_t *p_alg, af_haf_set_parameter_t *param)
{
  CDBG_HAF("Enter !");
  boolean result = TRUE;

  /* NULL pointer check */
  if (NULL == p_alg || NULL == param) {
    result = FALSE;
    return result;
  }

  /* Typecast from af_alg_interface_t to af_alg_PDAF_internal_t */
  af_alg_cmixer_internal_t *pMixer = (af_alg_cmixer_internal_t *)p_alg;

  if (pMixer->p_tof_alg->set_inputs)
  {
     result = pMixer->p_tof_alg->set_inputs(pMixer->p_tof_alg, param);
  }
  if (pMixer->p_pdaf_alg->set_inputs)
  {
      result &= pMixer->p_pdaf_alg->set_inputs(pMixer->p_pdaf_alg, param);
  }
  return result;
}


/** af_cmixer_set_tuning:
*Set Chromatix tuning data structure pointer internally
*
*@p_alg : af_alg_interface pointer.
*@haf_tuning : af_haf_tune_params_t pointer.
*Return: boolean
**/
boolean af_cmixer_set_tuning(af_alg_interface_t *p_alg, af_haf_tune_params_t *haf_tuning)
{
  CDBG_HAF("Enter !");

  boolean result = TRUE;
  /* NULL pointer check */
  if (NULL == p_alg || NULL == haf_tuning) {
    result = FALSE;
    return result;
  }

  /* Typecast from af_alg_interface_t to af_alg_PDAF_internal_t */
  af_alg_cmixer_internal_t *pMixer = (af_alg_cmixer_internal_t *)p_alg;

  result = pMixer->p_tof_alg->set_tuning(pMixer->p_tof_alg, haf_tuning);
  result &= pMixer->p_pdaf_alg->set_tuning(pMixer->p_pdaf_alg, haf_tuning);

  return result;
}


/** af_cmixer_set_mode:
*Set Focus Mode
*
*@p_alg : af_alg_interface pointer.
*@mode : Focus mode
*Return: void
**/
void af_cmixer_set_mode(af_alg_interface_t *p_alg, af_mode_type mode)
{
  CDBG_HAF("Enter !");
  (void)mode;
  (void)p_alg;
  /*This is not being used now*/
}


/** af_cmixer_set_roi:
*to set AF ROI configuration .
*
*@p_alg : af_alg_interface pointer.
*@roi :  roi configuration to be set.
*Return: void
**/
void af_cmixer_set_roi(af_alg_interface_t *p_alg, af_roi_info_t *roi)
{
  CDBG_HAF("Enter !");
  /* NULL pointer check */
  if (NULL == p_alg || NULL == roi) {
    return;
  }


  /* Typecast from af_alg_interface_t to af_alg_PDAF_internal_t */
  af_alg_cmixer_internal_t *pMixer = (af_alg_cmixer_internal_t *)p_alg;

  if (pMixer->p_tof_alg->set_roi)
      pMixer->p_tof_alg->set_roi(pMixer->p_tof_alg, roi);
  if (pMixer->p_pdaf_alg->set_roi)
    pMixer->p_pdaf_alg->set_roi(pMixer->p_pdaf_alg, roi);
}


/** af_cmixer_set_range:
*to set AF range configuration
*Must be called specifically.
*
*@p_alg_intf : af_alg_interface pointer.
*@near : nearest end of the AF search range
*@far : farthest end of the AF search range
*Return: void
**/
void af_cmixer_set_range(af_alg_interface_t *p_alg, int near, int far)
{
  CDBG_HAF("Enter !");

  /* NULL pointer check */
  if (NULL == p_alg) {
    return;
  }

  /* Typecast from af_alg_interface_t to af_alg_PDAF_internal_t */
  af_alg_cmixer_internal_t *pMixer = (af_alg_cmixer_internal_t *)p_alg;

  if (pMixer->p_tof_alg->set_range)
      pMixer->p_tof_alg->set_range(pMixer->p_tof_alg, near, far);
  if (pMixer->p_pdaf_alg->set_range)
      pMixer->p_pdaf_alg->set_range(pMixer->p_pdaf_alg, near, far);

}


/** af_cmixer_process_monitor:
* Custom Mixer's process_monitor API. Handle monitor state as per the light condition
*
*  Return: void
**/
boolean af_cmixer_process_monitor(af_alg_interface_t *p_alg, af_haf_monitor_data_t *output, int cur_pos)
{
  CDBG_HAF("Enter !");

  boolean result = TRUE;
  /* NULL pointer check */
  if (NULL == p_alg || NULL == output) {
    result = FALSE;
    return result;
  }

  /* Typecast from af_alg_interface_t to af_alg_PDAF_internal_t */
  af_alg_cmixer_internal_t *pMixer = (af_alg_cmixer_internal_t *)p_alg;
  if(pMixer->is_lowLight){
    if(pMixer->p_tof_alg->process_monitor) {
      result = pMixer->p_tof_alg->process_monitor(pMixer->p_tof_alg,output,cur_pos);
    }
  } else {
    if(pMixer->p_pdaf_alg->process_monitor){
      result = pMixer->p_pdaf_alg->process_monitor(pMixer->p_pdaf_alg,output,cur_pos);
    }
  }
  return result;
}


/** af_cmixer_process_search:
* Custom Mixer's process_search API. Handle search state as per the light condition
*
*  Return: void
**/
boolean af_cmixer_process_search(af_alg_interface_t *p_alg, af_haf_focus_search_data_t *output, int cur_pos)
{
  CDBG_HAF("Enter !");

  boolean result = TRUE;
  /* NULL pointer check */
  if (NULL == p_alg || NULL == output) {
    result = FALSE;
    return result;
  }

  /* Typecast from af_alg_interface_t to af_alg_PDAF_internal_t */
  af_alg_cmixer_internal_t *pMixer = (af_alg_cmixer_internal_t *)p_alg;

  if(pMixer->is_lowLight){
    if(pMixer->p_tof_alg->process_search){
      result = pMixer->p_tof_alg->process_search(pMixer->p_tof_alg,output,cur_pos);
    }
  } else {
    if(pMixer->p_pdaf_alg->process_search){
      result = pMixer->p_pdaf_alg->process_search(pMixer->p_pdaf_alg,output,cur_pos);
    }
  }
  return result;

}

/** af_cmixer_destroy: Destroy function to destroy the af example instance. Must be called
*                        specifically.
*
*  Return: void
**/
void af_cmixer_destroy(af_alg_interface_t *p_alg)
{
  CDBG_HAF("Enter !");

  if(!p_alg)
    return;
  /* Typecast from af_alg_interface_t to af_alg_PDAF_internal_t */
  af_alg_cmixer_internal_t *pMixer = (af_alg_cmixer_internal_t *)p_alg;
  if(pMixer->p_tof_alg)
    pMixer->p_tof_alg->destroy(pMixer->p_tof_alg);
  if(pMixer->p_pdaf_alg)
    pMixer->p_pdaf_alg->destroy(pMixer->p_pdaf_alg);
  if (pMixer) {
    free(pMixer);
  }
}


/** af_factory_create_by_type: is the only function that will access implementation specific function of each alg.
*                               All other calls to the algorithms must be made from the interface.
*
*  @p_tuning: Tuning data pointer
*  @eAlg_type: Enabled algorithm enum
*
*  Return: af_alg_interface_t
**/
af_alg_interface_t* af_factory_create_by_type(af_haf_algo_enum_type eAlg_type)
{
  af_alg_interface_t *p_alg = NULL;

  /* Create af_alg_ext_util_interface instance and pass to each alg that is going to be created */
  af_alg_util_interface_t *p_alg_util = af_alg_util_create();

  /* AF Alg util NULL pointer check */
  if (NULL == p_alg_util)
      {
    return NULL;
  }

  HAF_MSG_LOW("AF_HAF_ALGO_TYPE = %d!", (int )eAlg_type);
  /* Switch based on enum and call create accordingly */
  switch (eAlg_type) {
  case AF_HAF_ALGO_TOF:
    p_alg = af_alg_TOF_create(p_alg_util);
    break;
  case AF_HAF_ALGO_PDAF:
    p_alg = af_alg_PDAF_create(p_alg_util);
    break;
  default:
    // Output error message
    break;
  }

  /* Free memory if failed to create current algorithm */
  if (!p_alg) {
    AF_FREE(p_alg_util);
    p_alg_util = NULL;
  }

  return p_alg;
} /*af_factory_create_by_type*/
