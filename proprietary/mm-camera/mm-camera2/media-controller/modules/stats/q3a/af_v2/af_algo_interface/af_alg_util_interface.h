/***************************************************************************
Copyright (c) 2016 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
***************************************************************************/
#ifndef __AF_ALG_UTIL_INTF_H__
#define __AF_ALG_UTIL_INTF_H__

#include "af_core.h"

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
    typedef enum {
    AF_SEARCH_TYPE_NONE = 0,
    AF_SEARCH_TYPE_HAF,
    AF_SEARCH_TYPE_SINGLE_HJ,
    AF_SEARCH_TYPE_SINGLE,
    AF_SEARCH_TYPE_FINE,
    AF_SEARCH_TYPE_FULLSWEEP,
  } af_util_search_type_enum_type;

/* -----------------------------------------------------------------------
** af_alg_util_interface API
** ----------------------------------------------------------------------- */
typedef struct _af_alg_util_interface_t af_alg_util_interface_t;

struct _af_alg_util_interface_t {
    int          (*dist_2_pos)       (const af_alg_util_interface_t *p_alg_util, int distmm);
    unsigned int (*pos_2_dist)       (const af_alg_util_interface_t *p_alg_util, unsigned int lp);
    unsigned int (*dac_2_pos)        (const af_alg_util_interface_t *p_alg_util, int dac);
    int          (*pos_2_dac)        (const af_alg_util_interface_t *p_alg_util, unsigned int lp);
    boolean      (*set_lens_info)    (const af_alg_util_interface_t *p_alg_util, af_lens_params *lens_param);
    void         (*set_tuning)       (const af_alg_util_interface_t *p_alg_util, void *haf_tuning);
    int          (*get_lens_sag_compensation)
                                     (const af_alg_util_interface_t *p_alg_util);
};

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
int dist_2_pos(const af_alg_util_interface_t *p_alg_util, int distmm);
unsigned int pos_2_dist(const af_alg_util_interface_t *p_alg_util, unsigned int lp);
unsigned int dac_2_pos(const af_alg_util_interface_t *p_alg_util, int dac);
int pos_2_dac(const af_alg_util_interface_t *p_alg_util, unsigned int lp);
int get_lens_sag_compensation(const af_alg_util_interface_t *p_alg_util);

#endif
