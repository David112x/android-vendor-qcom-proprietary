/* af_alg_factory_custom.c
*
* Copyright (c) 2016 Qualcomm Technologies, Inc. All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*/

#include "af_alg_interface.h"
#include "af_alg_util_interface.h"
/* =======================================================================

DEFINITIONS AND DECLARATIONS

This section contains definitions for constants, macros, types, variables
and other items needed by this source file.

========================================================================== */

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
af_alg_util_interface_t* af_alg_util_create(void);
af_alg_interface_t* af_alg_example_create(const af_alg_util_interface_t *p_alg_util_instance);

/* -----------------------------------------------------------------------
** Function Declarations
** ----------------------------------------------------------------------- */
af_alg_interface_t* af_factory_create_custom(af_alg_util_interface_t *p_alg_util);

/* -----------------------------------------------------------------------
** Function Definitions
** ----------------------------------------------------------------------- */
/*
*  af_factory_create_custom: is implemented by customer to override default
*        creation code.
*
*
*  Return: af_alg_interface_t
**/

af_alg_interface_t* af_factory_create_custom(af_alg_util_interface_t *p_alg_util)
{
  /* Leave open for custom create function */
  af_alg_interface_t* p_af_alg = NULL;

  /*Make if(1) to initialize Example Algo rather than Default HAF algorithm
  This is for the customers who want to write their own algo and want to
  over-ride the default implementation*/
  if(0) {
    AF_MSG_LOW("Customer Create function being invoked");
    p_af_alg = af_alg_example_create(p_alg_util);
  }

  return p_af_alg;
} /*af_factory_create_custom*/
