/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_msim_if.h
  @brief   MSIM API's for test case dev

  DESCRIPTION

  API's for modifying MSIM messages used in RIL UTF tests.
  ---------------------------------------------------------------------------
******************************************************************************/
#ifndef RIL_UTF_FUNC_EXEC_IF_H_
#define RIL_UTF_FUNC_EXEC_IF_H_

#include "ril_utf_if.h"

typedef void (*funcptr)();

//RIL interfaces extracted from ril_utf_if.h
typedef class ril_utf_func_exec_if : public ril_utf_node_base_t {
private:
public:
//==================================================
// Func exec Interfaces
//==================================================
  ril_utf_func_exec_if();
  ril_utf_func_exec_if(const ril_utf_func_exec_if &node);

/*==================================================
  FUNC_EXEC_NODE
===================================================*/
void update_default_func_exec_node( void (*func)(void));
funcptr get_func_exec_node();

} FuncExecnode;

#endif /* RIL_UTF_FUNC_EXEC_IF_H_*/
