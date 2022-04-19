#ifndef __TH_MEMORY_H__
#define __TH_MEMORY_H__

/*========================================================================

*//** @file memory.h

FILE SERVICES:
      Internal Memory Management API for Thread Services Library.

      Detailed description

Copyright (c) 2015 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

*//*====================================================================== */

/*========================================================================
                      Edit History

when       who    what, where, why
--------   ---    -------------------------------------------------------
11/10/14   rg     Initial Version.

========================================================================== */

/*========================================================================*/
/* INCLUDE FILES */
/*==  =====================================================================*/
/* std headers */

/* local headers */
#include "common.h"

/*========================================================================*/
/* DEFINITIONS AND DECLARATIONS */
/*========================================================================*/
#if defined( __cplusplus )
extern "C"
{
#endif /* end of macro __cplusplus */


th_err_t create_memory_manager(th_services_t *th_serv);

th_err_t destroy_memory_manager(th_services_t *th_serv);

void *get_memory_ptr(th_mem_manager_t *th_mem_mgr);

void set_memory_ptr(th_mem_manager_t *th_mem_mgr, void *mem_ptr);

void *get_memory(th_mem_manager_t *th_mem_mgr, void *mem_ptr, int size);


#if defined( __cplusplus )
}
#endif /* end of macro __cplusplus */

#endif /* end of macro __TH_MEMORY_H__ */
