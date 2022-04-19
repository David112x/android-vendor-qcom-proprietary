/*!
 * @file vpp_ip_frc_me.h
 *
 * @cr
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * @services: Interface to Motion Estimation from Venus Encoder
 */

#ifndef _VPP_IP_FRC_ME_H_
#define _VPP_IP_FRC_ME_H_

#ifdef __cplusplus
extern "C" {
#endif

enum
{
    eMeIpBufFlag_IpEos,
    eMeIpBufFlag_InternalBypass,
};


#define VPP_BUF_FLAG_ME_INTERN_EOS      (1 << (eVppBufFlag_IpStart + eMeIpBufFlag_IpEos))
#define VPP_BUF_FLAG_ME_INTERN_BYPASS   (1 << (eVppBufFlag_IpStart + eMeIpBufFlag_InternalBypass))



/***************************************************************************
 * Function Prototypes
 ***************************************************************************/

void *vpVppIpFrcMe_Init(t_StVppCtx *pstCtx, uint32_t u32Flags, t_StVppCallback cbs);
void vVppIpFrcMe_Term(void *ctx);
uint32_t u32VppIpFrcMe_Open(void *ctx);
uint32_t u32VppIpFrcMe_Close(void *ctx);
uint32_t u32VppIpFrcMe_SetParam(void *ctx, enum vpp_port port, struct vpp_port_param param);
uint32_t u32VppIpFrcMe_SetCtrl(void *ctx, struct hqv_control ctrl);
uint32_t u32VppIpFrcMe_GetBufferRequirements(void *ctx, t_StVppIpBufReq *pstInputBufReq, t_StVppIpBufReq *pstOutputBufReq);
uint32_t u32VppIpFrcMe_GetBufferNeed(void *ctx, uint32_t *pu32In, uint32_t *pu32Out);
uint32_t u32VppIpFrcMe_QueueBuf(void *ctx, enum vpp_port ePort, t_StVppBuf *pBuf);
uint32_t u32VppIpFrcMe_Flush(void *ctx, enum vpp_port ePort);
uint32_t u32VppIpFrcMe_Drain(void *ctx);

#ifdef __cplusplus
 }
#endif

#endif /* _VPP_IP_FRC_ME_H_ */
