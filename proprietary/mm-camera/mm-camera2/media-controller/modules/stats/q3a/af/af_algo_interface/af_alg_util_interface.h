/***************************************************************************
Copyright (c) 2013-2016 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
***************************************************************************/
#ifndef __AF_ALG_UTIL_INTF_H__
#define __AF_ALG_UTIL_INTF_H__


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
/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** af_alg_util_interface API
** ----------------------------------------------------------------------- */
/** _af_alg_util_interface_t:
*     external function pointers for algo to call def
*
*    @dist_2_pos   : Function pointer for Distance (in mm) to Lens pos
*    @calc_dof     : Function pointer for getting DOF range (in pos)
*    @get_lens_sag_comp : Function pointer for getting Lens sag comp*
**/
typedef struct _af_alg_util_interface_t {
    int(*dist_2_pos)(int distmm, float focal_length,
        float sensitivity, int anchor_distmm, int anchor_lens_pos,
    int gravity_comp, boolean comp_on);
    void(*calc_dof)(int distmm, float Fnum, float focal_length,
        float pixel_size, int *dist_near, int *dist_far);
    int(*get_lens_sag_comp)(af_algo_tune_parms_adapter_t *tuning,
      af_input_from_gravity_vector_t gravity_input,
      float actuator_sensitivity);
    int(*pos_2_dist)(int distmm, float focal_length,
        float sensitivity, int anchor_distmm, int anchor_lens_pos,
    int gravity_comp, boolean comp_on);
} af_alg_util_interface_t;

#endif
