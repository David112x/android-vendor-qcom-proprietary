/*!
 * @file vpp_ion.h
 *
 * @cr
 * Copyright (c) 2015, 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * @services
 *
 * @description
 */

#ifndef _VPP_ION_H_
#define _VPP_ION_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <linux/msm_ion.h>
#include "vpp_ctx.h"

/************************************************************************
 * Defines
 ***********************************************************************/

/************************************************************************
 * Structures / Enumerations
 ***********************************************************************/

typedef struct {
    int fd_ion_mem;             // fd describing this buffer
    uint32_t len;               // length of the ion allocated buffer
    void *buf;                  // pointer to mmap'd buffer
    uint32_t bSecure;           // content protected buffer or not
} t_StVppIonBuf;

/************************************************************************
 * Function Prototypes
 ***********************************************************************/
uint32_t u32VppIon_Init(t_StVppCtx *pstCtx);
uint32_t u32VppIon_Term(t_StVppCtx *pstCtx);

uint32_t u32VppIon_Alloc(t_StVppCtx *pstCtx, uint32_t u32Len, uint32_t u32Secure,
                         t_StVppIonBuf *buf);

uint32_t u32VppIon_Free(t_StVppCtx *pstCtx, t_StVppIonBuf *buf);

uint32_t u32VppIon_GetHeapId(t_StVppCtx *pstCtx, uint32_t bSecure);
//int32_t dvp_ion_cache_ops(t_StVppIonBuf *buf, unsigned int cmd);

#ifdef __cplusplus
}
#endif

#endif /* _VPP_ION_H_ */
