/* af_haf_example.c
 *
 * Copyright (c) 2015 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
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
/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
/** _af_alg_example_internal_t:
*     example algo internal struct
**/
typedef struct _af_alg_example_internal_t {
    af_alg_interface_t                   af_alg;
    af_stats_ts_data_t                   time_stamp;
    af_stats_ts_data_t                   reference_time_stamp;
    int                                  range_near;
    int                                  range_far;
    boolean                              switch_search_direction;
} af_alg_example_internal_t;

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
/* Create functions */
af_alg_interface_t* af_alg_example_create(af_alg_util_interface_t *p_alg_util_instance);
static void af_example_destroy(af_alg_interface_t *af_alg_obj);
static void af_alg_interface_init(af_alg_example_internal_t *af_alg_obj);

/* af_alg_interface API */
static boolean af_example_process_monitor(af_alg_interface_t * p_alg, af_haf_monitor_data_t* output, int cur_pos);
static boolean af_example_process_search(af_alg_interface_t * p_alg, af_haf_focus_search_data_t* output,
                                                    int cur_pos);

// SetInputs / SetRange:
static boolean af_example_set_inputs(af_alg_interface_t* p_alg, af_haf_set_parameter_t* param);
static void af_example_set_range(af_alg_interface_t *p_alg, int near, int far);

// Handle Custom Set inputs:
static boolean af_example_handle_custom_inputs(af_alg_interface_t* p_alg,void *input);



/* -----------------------------------------------------------------------
** Function Definitions
** ----------------------------------------------------------------------- */
/** af_alg_example_create: Create instance of example algorithm. Mallocate example internal structure
*                (af_alg_example_internal_t) specifically and initializes each parameter.
*
*  @ p_alg_util_instance: AF algorithm util instance pointer
*
*  Return: af_alg_interface_t
**/
af_alg_interface_t* af_alg_example_create(af_alg_util_interface_t *p_alg_util_instance)
{

    if(!p_alg_util_instance) {
        return NULL;
    }

    /* Malloc for example internal structure */
    af_alg_example_internal_t* p_ex_alg_instance = (void *)malloc(sizeof(af_alg_example_internal_t));
    if (!p_ex_alg_instance) {
        /* Malloc failed */
        return NULL;
    }

    /* Alg interface init */
    af_alg_interface_init(p_ex_alg_instance);

    /* Populate valid API function pointers */
    p_ex_alg_instance->af_alg.set_inputs = af_example_set_inputs;
    p_ex_alg_instance->af_alg.set_range = af_example_set_range;
    p_ex_alg_instance->af_alg.process_monitor = af_example_process_monitor;
    p_ex_alg_instance->af_alg.process_search = af_example_process_search;
    p_ex_alg_instance->af_alg.destroy = af_example_destroy;

    /* Typecast to af_alg_interface_t defined in af_alg_interface.h */
    return (af_alg_interface_t*)p_ex_alg_instance;
}

/** af_alg_interface_init: Init interface pointers and internal variables to default values
*
*  Return: void
**/
static void af_alg_interface_init(af_alg_example_internal_t *af_alg_obj)
{
    if (NULL == af_alg_obj) {
        return;
    }

    /* First set all parameters within internal type to 0 */
    memset(af_alg_obj, 0, sizeof(af_alg_example_internal_t));
    return;
}

/** af_example_destroy: Destroy function to destroy the af example instance. Must be called
*                        specifically.
*
*  Return: void
**/
static void af_example_destroy(af_alg_interface_t *af_alg_obj)
{
    af_alg_example_internal_t *obj =
        (af_alg_example_internal_t *)af_alg_obj;

    if (af_alg_obj) {
        free(af_alg_obj);
    }
}

/** af_example_handle_custom_inputs: For external caller to set custom inputs
*
*  @p_alg : af_alg_interface pointer.
*  @input : Custom input
*
*  Return: boolean
**/
static boolean af_example_handle_custom_inputs(af_alg_interface_t* p_alg,void *input)
{
  boolean result = TRUE;

  /* NULL pointer check */
  if (NULL == p_alg || NULL == input) {
      result = FALSE;
      return result;
  }

  /* Typecast from af_alg_interface_t to af_alg_example_internal_t */
  af_alg_example_internal_t * example_alg = (af_alg_example_internal_t *)p_alg;
  int val = *((int*)input);
  AF_MSG_LOW("AF_HAF_SET_CUSTOM_PARAM == %d",val);
  free(input);
  return result;
}

/** af_example_set_inputs: For external caller to set example algorithm inputs.
*
*  @p_alg : af_alg_interface pointer.
*  @param : example configuration to be set.
*
*  Return: boolean
**/
static boolean af_example_set_inputs(af_alg_interface_t* p_alg, af_haf_set_parameter_t* param)
{
    boolean result = TRUE;

    /* NULL pointer check */
    if (NULL == p_alg || NULL == param) {
        result = FALSE;
        return result;
    }

    /* Typecast from af_alg_interface_t to af_alg_example_internal_t */
    af_alg_example_internal_t * example_alg = (af_alg_example_internal_t *)p_alg;

    /* Any algorithm implementation may route unsupported param to the Set_params method */
    switch (param->type)
    {
        /* For example_alg, the only input that's been set is AF_HAF_SET_PARAM_STATS_TS_DATA */
        case AF_HAF_SET_PARAM_STATS_TS_DATA:
        {
            /* Copy the time_stamp input from message to internal input structure */
            example_alg->time_stamp.frame_id = param->u.af_stats_ts.frame_id;
            example_alg->time_stamp.time_stamp.time_stamp_sec = param->u.af_stats_ts.time_stamp.time_stamp_sec;
            example_alg->time_stamp.time_stamp.time_stamp_us = param->u.af_stats_ts.time_stamp.time_stamp_us;
        }
        break;
        case AF_HAF_SET_CUSTOM_PARAM:
        {
            af_example_handle_custom_inputs(p_alg,param->u.customData);
        }
        break;
        /* If Set_Input doesn't realize the ID, pass down to Set_Param */
        default:
        {
            if (p_alg->set_parameters)
            {
                return p_alg->set_parameters(p_alg, param);
            }
        }
    }
    return result;
}

/** af_example_set_range: For external caller to set range to example algorithm.
*
*  @p_alg : af_alg_interface pointer.
*  @near  : example range near configuration.
*  @far    :  example range far configuration.
*  Return: void
**/
static void af_example_set_range(af_alg_interface_t *p_alg, int near, int far)
{
    /* NULL pointer check */
    if (NULL == p_alg) {
        return;
    }

    /* Typecast from af_alg_interface_t to af_alg_example_internal_t */
    af_alg_example_internal_t * example_alg = (af_alg_example_internal_t *)p_alg;

    /* Set range according to input */
    example_alg->range_near = near;
    example_alg->range_far  = far;

    return;
}

/** af_example_process_monitor:
* example process_monitor API. Handle monitor state for example alg.
*
*  Return: boolean
**/
static boolean af_example_process_monitor(af_alg_interface_t* p_alg, af_haf_monitor_data_t* output, int cur_pos)
{
    boolean result = TRUE;
    (void)cur_pos;

    /* NULL pointer check */
    if (NULL == p_alg || NULL == output) {
        result = FALSE;
        return result;
    }

    /* Typecast from af_alg_interface_t to af_alg_example_internal_t */
    af_alg_example_internal_t * example_alg = (af_alg_example_internal_t *)p_alg;
    boolean is_defocused = FALSE;

    /* if current input timestamp is 10 seconds greater than reference timestamp, mark scene defocus */
    uint32_t time_delta = example_alg->time_stamp.time_stamp.time_stamp_sec -
                            example_alg->reference_time_stamp.time_stamp.time_stamp_sec;

    if (time_delta >= 10)
    {
      is_defocused = TRUE;
    }

    /* The following output values can take intermediate values. The values can be defined to have their own meaning. */
    output->focus_level = (is_defocused)?(AF_FOCUS_LEVEL_LOW):(AF_FOCUS_LEVEL_HIGH);
    output->stability   = AF_STABILITY_HIGH;
    output->confidence  = AF_CONFIDENCE_HIGH;

    return result;
}

/** af_example_process_search:
* example process_search API. Handle search state for example.
*
*  Return: boolean
**/
static boolean af_example_process_search(af_alg_interface_t* p_alg, af_haf_focus_search_data_t* output, int cur_pos)
{
    boolean result = TRUE;

    /* NULL pointer check */
    if (NULL == p_alg || NULL == output) {
        result = FALSE;
        return result;
    }

    /* Typecast from af_alg_interface_t to af_alg_example_internal_t */
    af_alg_example_internal_t * example_alg = (af_alg_example_internal_t *)p_alg;

    /* Calculate next destination lens position */
    int delta_step = (example_alg->range_far - example_alg->range_near)/60;
    delta_step = (example_alg->switch_search_direction)?(delta_step):-(delta_step);
    output->next_pos = cur_pos + delta_step;
    output->progress = AF_PROGRESS_50_RUNNING;

    /* perform simple boundary check, and report converge success once we reach near_limit or far_limit */
    if (output->next_pos <= example_alg->range_near || output->next_pos >= example_alg->range_far)
    {
        output->next_pos = (output->next_pos <= example_alg->range_near)?
                               (example_alg->range_near):(example_alg->range_far);
        output->progress = AF_PROGRESS_100_DONE;
        output->confidence = AF_CONFIDENCE_HIGH;
    }

    /* if converge success is reached, then we store new reference time stamp for monitoring defocus */
    if (output->progress == AF_PROGRESS_100_DONE)
    {
        example_alg->reference_time_stamp.time_stamp.time_stamp_sec =
                                      example_alg->time_stamp.time_stamp.time_stamp_sec;
        example_alg->switch_search_direction = (example_alg->switch_search_direction)?(FALSE):(TRUE);
        output->range_near = 0;
        output->range_far = 0;
        output->fine_step_size = 0;
    }
    return result;
}

