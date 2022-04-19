/* af_haf_TOF_decorator.c
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc. 
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#include "af_alg_interface.h"
#include "af_alg_util_interface.h"
#include <math.h>
/* =======================================================================

DEFINITIONS AND DECLARATIONS

This section contains definitions for constants, macros, types, variables
and other items needed by this interface.

========================================================================== */
/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */

#define AF_CUSTOMTOFDECORATOR_DEBUG

#ifdef AF_CUSTOMTOFDECORATOR_DEBUG
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
typedef struct _af_alg_ctof_internal_t {
  af_alg_interface_t                  af_alg;
  af_alg_util_interface_t            *p_alg_util;
  af_alg_interface_t                 *p_tof_alg;
  boolean                             is_lowLight;
} af_alg_ctof_internal_t;

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */

af_alg_interface_t* af_factory_create_by_type(af_haf_algo_enum_type eAlg_type, const af_alg_util_interface_t *p_alg_util);
static af_alg_interface_t* af_alg_CTOF_create(af_alg_util_interface_t *p_alg_util_instance);
static void af_alg_interface_init(af_alg_ctof_internal_t *af_alg);
static boolean af_ctof_set_parameters(af_alg_interface_t *p_alg, af_haf_set_parameter_t *param);
static boolean af_ctof_set_inputs (af_alg_interface_t *p_alg, af_haf_set_parameter_t *param);
static boolean af_ctof_set_tuning(af_alg_interface_t *p_alg, af_haf_tune_params_t *haf_tuning);
static void af_ctof_set_mode(af_alg_interface_t *p_alg, af_mode_type mode);
static void af_ctof_set_roi(af_alg_interface_t *p_alg, af_roi_info_t *roi);
static void af_ctof_set_range(af_alg_interface_t *p_alg, int near, int far);
static boolean af_ctof_process_monitor(af_alg_interface_t *p_alg, af_haf_monitor_data_t *output, int cur_pos);
static boolean af_ctof_process_search(af_alg_interface_t *p_alg, af_haf_focus_search_data_t *output, int cur_pos);
static void af_ctof_destroy(af_alg_interface_t *p_alg);

/* -----------------------------------------------------------------------
** Function Defintions
** ----------------------------------------------------------------------- */

/** af_ctof_rebase_monitor: Notification of focus search end &
*   reset any reference parameters if required.
*
*  Return: void
**/
boolean af_ctof_rebase_monitor(af_alg_interface_t *p_alg_intf)
{
  boolean result = FALSE;
  /* NULL pointer check */
  if (NULL == p_alg_intf) {
    return FALSE;
  }

  /* Typecast from af_alg_interface_t to af_alg_ctof_internal_t */
  af_alg_ctof_internal_t* p_alg = (af_alg_ctof_internal_t *)p_alg_intf;
  if(p_alg->p_tof_alg->rebase_monitor){
    result = p_alg->p_tof_alg->rebase_monitor(p_alg->p_tof_alg);
  }
  return result;
}

/** af_alg_CTOF_create: Create instance of TOF Decorator. Mallocate  internal structure
*                        specifically and initializes each parameter.
*
*  @ p_alg_util_instance: AF algorithm util instance pointer
*
*  Return: af_alg_interface_t
**/
af_alg_interface_t* af_alg_CTOF_create(af_alg_util_interface_t *p_alg_util_instance)
{
  CDBG_HAF("Enter");

  /* Malloc for Custom TOF Decorator internal structure */
  af_alg_ctof_internal_t* p_alg = (void *)malloc(sizeof(af_alg_ctof_internal_t));
  if (NULL == p_alg) {
    /* Malloc failed */
    return NULL;
  }
  af_alg_interface_init(p_alg);
  /* Populate Custom TOF Decorator specific function pointers */
  p_alg->af_alg.set_parameters          = af_ctof_set_parameters;
  p_alg->af_alg.set_inputs              = af_ctof_set_inputs;
  p_alg->af_alg.set_tuning              = af_ctof_set_tuning;
  p_alg->af_alg.set_mode                = af_ctof_set_mode;
  p_alg->af_alg.set_roi                 = af_ctof_set_roi;
  p_alg->af_alg.set_range               = af_ctof_set_range;
  p_alg->af_alg.process_monitor         = af_ctof_process_monitor;
  p_alg->af_alg.process_search          = af_ctof_process_search;
  p_alg->af_alg.destroy                 = af_ctof_destroy;
  p_alg->af_alg.rebase_monitor        = af_ctof_rebase_monitor;

  /* Copy the ext func util instance pass during create time */
  p_alg->p_alg_util                     = p_alg_util_instance;
  p_alg->p_tof_alg                      =
    af_factory_create_by_type(AF_HAF_ALGO_TOF, p_alg_util_instance);

  /* Typecast to af_alg_interface_t defined in af_alg_interface.h */
  return (af_alg_interface_t*)p_alg;
}


/** af_alg_interface_init: Init interface pointers and internal variables to default values
*
*  Return: void
**/
static void af_alg_interface_init(af_alg_ctof_internal_t *af_alg)
{
  if (NULL == af_alg) {
    return;
  }

  /* First set all parameters within internal type to 0 */
  memset(af_alg, 0, sizeof(af_alg_ctof_internal_t));
  /*Initialize the default value*/
  af_alg->is_lowLight                = FALSE;
  return;
}


/** af_ctof_set_parameters:
*set parameter for caller to set Algo configuration .
*
*@p_alg : af_alg_interface pointer.
*@param : configuration to be set.
*Return: boolean
**/
boolean af_ctof_set_parameters(af_alg_interface_t *p_alg_intf, af_haf_set_parameter_t *param)
{
  boolean result = TRUE;

  /* NULL pointer check */
  if (NULL == p_alg_intf || NULL == param) {
    result = FALSE;
    return result;
  }

  /* Typecast from af_alg_interface_t to af_alg_ctof_internal_t */
  af_alg_ctof_internal_t* p_alg = (af_alg_ctof_internal_t *)p_alg_intf;

  /* Any algorithm implementation may route unsupported param to the Set_params method */
  switch (param->type)
  {
  /* As per the design, SetInput/Invalidate interface is added to pass
  * events with big payloads. For other cases, setparam should be used */
    case AF_HAF_SET_PARAM_UPDATE_AEC_INFO:
    {
      if(param->u.aec_info.lux_idx < LOW_LIGHT_THRESHOLD){
        p_alg->is_lowLight = FALSE;
      } else {
        p_alg->is_lowLight = TRUE;
      }

      break;
    }
    default:
      break;
  }
  if(p_alg->p_tof_alg->set_parameters)
    result = p_alg->p_tof_alg->set_parameters(p_alg->p_tof_alg, param);

  return result;
}


/** af_ctof_set_inputs: set input parameter for caller to set  algorithm inputs.
*
*  @p_alg : af_alg_interface pointer.
*  @param : Algorithm configuration to be set.
*
*  Return: void
**/
boolean af_ctof_set_inputs (af_alg_interface_t *p_alg_intf, af_haf_set_parameter_t *param)
{
  boolean result = TRUE;

  /* NULL pointer check */
  if (NULL == p_alg_intf || NULL == param) {
    result = FALSE;
    return result;
  }

  /* Typecast from af_alg_interface_t to af_alg_ctof_internal_t */
  af_alg_ctof_internal_t* p_alg = (af_alg_ctof_internal_t *)p_alg_intf;
  if(p_alg->p_tof_alg->set_inputs)
    result = p_alg->p_tof_alg->set_inputs(p_alg->p_tof_alg, param);
  return result;
}


/** af_ctof_set_tuning:
*Set Chromatix tuning data structure pointer internally
*
*@p_alg : af_alg_interface pointer.
*@haf_tuning : af_haf_tune_params_t pointer.
*Return: boolean
**/
boolean af_ctof_set_tuning(af_alg_interface_t *p_alg, af_haf_tune_params_t *haf_tuning)
{
  boolean result = TRUE;
  if (NULL == p_alg || NULL == haf_tuning) {
    result = FALSE;
    return result;
  }

  /* Typecast from af_alg_interface_t to af_alg_ctof_internal_t */
  af_alg_ctof_internal_t* p_af_ctof_alg = (af_alg_ctof_internal_t *)p_alg;

  if(p_af_ctof_alg->p_tof_alg->set_tuning)
    result = p_af_ctof_alg->p_tof_alg->set_tuning(p_af_ctof_alg->p_tof_alg, haf_tuning);

  return result;
}


/** af_ctof_set_mode:
*Set Focus Mode
*
*@p_alg : af_alg_interface pointer.
*@mode : Focus mode
*Return: void
**/
void af_ctof_set_mode(af_alg_interface_t *p_alg, af_mode_type mode)
{
  (void)mode;
  (void)p_alg;
  /*This is not being used now*/
}


/** af_ctof_set_roi:
*to set AF ROI configuration .
*
*@p_alg : af_alg_interface pointer.
*@roi :  roi configuration to be set.
*Return: void
**/
void af_ctof_set_roi(af_alg_interface_t *p_alg_intf, af_roi_info_t *roi)
{

  /* NULL pointer check */
  if (NULL == p_alg_intf || NULL == roi) {
    return;
  }

  /* Typecast from af_alg_interface_t to af_alg_ctof_internal_t */
  af_alg_ctof_internal_t* p_alg = (af_alg_ctof_internal_t *)p_alg_intf;

  if(p_alg->p_tof_alg->set_roi)
    p_alg->p_tof_alg->set_roi(p_alg->p_tof_alg, roi);
}


/** af_ctof_set_range:
*to set AF range configuration
*Must be called specifically.
*
*@p_alg_intf : af_alg_interface pointer.
*@near : nearest end of the AF search range
*@far : farthest end of the AF search range
*Return: void
**/
void af_ctof_set_range(af_alg_interface_t *p_alg_intf, int near, int far)
{
  /* NULL pointer check */
  if (NULL == p_alg_intf) {
    return;
  }

  /* Typecast from af_alg_interface_t to af_alg_ctof_internal_t */
  af_alg_ctof_internal_t* p_alg = (af_alg_ctof_internal_t *)p_alg_intf;

  if(p_alg->p_tof_alg->set_range)
    p_alg->p_tof_alg->set_range(p_alg->p_tof_alg, near, far);

}

/** af_ctof_process_monitor:
* decorator process_monitor API. Handle monitor state
*
*  Return: void
**/
boolean af_ctof_process_monitor(af_alg_interface_t *p_alg_intf, af_haf_monitor_data_t *output, int cur_pos)
{
  boolean result = TRUE;

  /* NULL pointer check */
  if (NULL == p_alg_intf || NULL == output) {
    result = FALSE;
    return result;
  }

  /* Typecast from af_alg_interface_t to af_alg_ctof_internal_t */
  af_alg_ctof_internal_t* p_alg = (af_alg_ctof_internal_t *)p_alg_intf;
  if(p_alg->p_tof_alg->process_monitor){
    result = p_alg->p_tof_alg->process_monitor(p_alg->p_tof_alg,output,cur_pos);
  }
  return result;
}

/** af_ctof_process_search:
* decorator process_search API. Handle search state .
*
*  Return: void
**/
boolean af_ctof_process_search(af_alg_interface_t *p_alg_intf, af_haf_focus_search_data_t *output, int cur_pos)
{
  boolean result = TRUE;

  /* NULL pointer check */
  if (NULL == p_alg_intf || NULL == output) {
    result = FALSE;
    return result;
  }

  /* Typecast from af_alg_interface_t to af_alg_ctof_internal_t */
  af_alg_ctof_internal_t* p_alg = (af_alg_ctof_internal_t *)p_alg_intf;
  if(p_alg->p_tof_alg->process_search){
    result = p_alg->p_tof_alg->process_search(p_alg->p_tof_alg,output,cur_pos);
  }

  if(output&& !p_alg->is_lowLight){
    output->confidence = AF_CONFIDENCE_LOW;
  }
  return result;
}

/** af_ctof_destroy: Destroy function to destroy the af example instance. Must be called
*                        specifically.
*
*  Return: void
**/
void af_ctof_destroy(af_alg_interface_t *p_alg_intf)
{
  if(!p_alg_intf)
    return;
  /* Typecast from af_alg_interface_t to af_alg_ctof_internal_t */
  af_alg_ctof_internal_t* p_alg = (af_alg_ctof_internal_t *)p_alg_intf;
  if(p_alg->p_tof_alg) {
    p_alg->p_tof_alg->destroy(p_alg->p_tof_alg);
    free(p_alg);
  }
}

