/******************************************************************************
 * #  Copyright (c) 2017 Qualcomm Technologies, Inc.
 * #  All Rights Reserved.
 * #  Confidential and Proprietary - Qualcomm Technologies, Inc.
 * #******************************************************************************/
/******************************************************************************
 *   @file    qcril_memory_management.h
 *   @brief   qcril qmi - memory management
 *
 *   DESCRIPTION
 *     Provides generic memory allocation and the correct typecasting.
 *     Determines the type from the destination address passed as a
 *     reference parameter.
 *
 *******************************************************************************/

#ifndef QCRIL_MEMORY_MANAGEMENT
#define QCRIL_MEMORY_MANAGEMENT

/*===========================================================================
 *
 *                            INCLUDE FILES
 *
 *===========================================================================*/

#include <framework/legacy.h>

//===========================================================================
// templated malloc
//===========================================================================
template<typename T>
inline T* qcril_malloc2(T*& adr, size_t n = 1) {
    adr = static_cast<T*>(qcril_malloc(sizeof(T) * n));
    return adr;
}

#endif //QCRIL_MEMORY_MANAGEMENT
