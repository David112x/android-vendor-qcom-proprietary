/**********************************************************************
*  Copyright (c) 2012,2016 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include "QIBase.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

/**
 * define loglevel
 */
volatile uint32_t g_mmstillloglevel = 0;

/*===========================================================================
 * Function: QIBase
 *
 * Description: QIBase constructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QIBase::QIBase()
{
}

/*===========================================================================
 * Function: ~QIBase
 *
 * Description: QIBase destructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QIBase::~QIBase()
{
}

