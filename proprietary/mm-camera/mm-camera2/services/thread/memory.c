/*========================================================================
Copyright (c) 2015 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
*//*====================================================================== */


#include "common.h"
#include "memory.h"
#include <string.h>
#include <stdlib.h>

/*========================================================================*/
/* INTERNAL FUNCTIONS */
/*========================================================================*/



/*========================================================================*/
/* INTERNAL INTERFACE IMPLEMENTATION */
/*========================================================================*/

/**
 * Function: create_memory_manager
 *
 * Description: TBD
 *
 * Arguments:
 *   @arg1: TBD
 *   @arg2: TBD
 *
 * Return values: TBD
 *
 * Notes: none
 **/
th_err_t create_memory_manager(th_services_t *th_serv)
{
  th_err_t err = TH_SUCCESS;

  //memory manager is not implemented in Phase 1
  (void)th_serv;

  TH_LOG_LOW(" create_memory_manager successful");
  return err;
}


/**
 * Function: destroy_memory_manager
 *
 * Description: TBD
 *
 * Arguments:
 *   @arg1: TBD
 *   @arg2: TBD
 *
 * Return values: TBD
 *
 * Notes: none
 **/
th_err_t destroy_memory_manager(th_services_t *th_serv)
{
  th_err_t err = TH_SUCCESS;

  //memory manager is not implemented in Phase 1
  (void)th_serv;

  TH_LOG_LOW(" destroy_memory_manager successful");
  return err;
}


/**
 * Function: get_memory_ptr
 *
 * Description: TBD
 *
 * Arguments:
 *   @arg1: TBD
 *   @arg2: TBD
 *
 * Return values: TBD
 *
 * Notes: none
 **/
void *get_memory_ptr(th_mem_manager_t *th_mem_mgr)
{

  //memory manager is not implemented in Phase 1
  (void)th_mem_mgr;

  return NULL;
}


/**
 * Function: set_memory_ptr
 *
 * Description: TBD
 *
 * Arguments:
 *   @arg1: TBD
 *   @arg2: TBD
 *
 * Return values: TBD
 *
 * Notes: none
 **/
void set_memory_ptr(
  th_mem_manager_t *th_mem_mgr,
  void *mem_ptr
)
{

  //memory manager is not implemented in Phase 1
  (void)th_mem_mgr;
  (void)mem_ptr;

  return;
}


/**
 * Function: get_memory
 *
 * Description: TBD
 *
 * Arguments:
 *   @arg1: TBD
 *   @arg2: TBD
 *
 * Return values: TBD
 *
 * Notes: none
 **/
void *get_memory(
  th_mem_manager_t *th_mem_mgr,
  void *mem_ptr,
  int size
)
{
  (void)th_mem_mgr;

  if (!size) {
    TH_LOG_ERR(" Invalid size for memory allocation request");
    return NULL;
  }

  if (IS_NULL(mem_ptr)) {//local memory manager is not ready
    return malloc(size);
  } else {
    //to be implemented
  }

  return NULL;
}

