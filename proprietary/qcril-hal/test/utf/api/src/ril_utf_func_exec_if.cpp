/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_func_exec_if.cpp
  @brief   Func exec API's for test case dev

  DESCRIPTION

  API to allow for arbitrary function execution mid test case
  ---------------------------------------------------------------------------
******************************************************************************/
#include "ril_utf_func_exec_if.h"
//---------------------------------------------------------------------------------------
//==================================================
// Func exec Interfaces
//==================================================

ril_utf_func_exec_if::ril_utf_func_exec_if()
{
  my_func = NULL;
}

ril_utf_func_exec_if::ril_utf_func_exec_if(const ril_utf_func_exec_if &node)
{
  my_func = node.my_func;
}

/*==================================================
  FUNC_EXEC_NODE
===================================================*/
void ril_utf_func_exec_if::update_default_func_exec_node( void (*func)(void))
{
  my_func = func;
}

funcptr ril_utf_func_exec_if::get_func_exec_node()
{
  return my_func;
}
