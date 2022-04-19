/* af_alg_factory_custom.c
*
* Copyright (c) 2015 Qualcomm Technologies, Inc. All Rights Reserved.
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
af_alg_interface_t* af_alg_example_create(af_alg_util_interface_t *p_alg_util_instance);
af_alg_interface_t* af_alg_CTOF_create(af_alg_util_interface_t *p_alg_util_instance);
af_alg_interface_t* af_alg_CMIXER_create(af_alg_util_interface_t *p_alg_util_instance);

/* -----------------------------------------------------------------------
** Function Declarations
** ----------------------------------------------------------------------- */
af_alg_interface_t* af_factory_create_custom();

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

af_alg_interface_t* af_factory_create_custom()
{
  /* Leave open for custom create function */
  af_alg_interface_t* p_af_alg = NULL;

  /*Uncomment below to initialize Example Algo rather than Default HAF algorithm*/
  #if 0
  af_alg_util_interface_t *p_alg_util = af_alg_util_create();
  memset(p_alg_util,0,sizeof(af_alg_util_interface_t));
  AF_MSG_LOW("Customer Create function being invoked");
  p_af_alg = af_alg_CMIXER_create(p_alg_util);
  #endif

  return p_af_alg;
} /*af_factory_create_custom*/
