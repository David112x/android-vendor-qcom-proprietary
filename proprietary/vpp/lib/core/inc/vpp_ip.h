/*!
 * @file vpp_ip.h
 *
 * @cr
 * Copyright (c) 2015-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.

 * @services
 */
#ifndef _VPP_IP_H_
#define _VPP_IP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>
#include "vpp_queue.h"

typedef enum {
    eVppIpFlag_WorkerStarted,
    eVppIpFlag_DrainPending,
    eVppIpFlag_EOSPending,
    eVppIpFlag_CorruptBuffers,
    eVppIpFlag_FlushPending,
    /*!
     * Flags between positions 24 and 31 are owned by IP blocks and are only
     * valid during IP block operations. These flags' meanings are only valid
     * while set and unset by the IP block.
     */
    eVppIpFlag_IpStart = 24,
    eVppIpFlag_IpMax = 31,
} t_EVppIpFlags;

#define IP_WORKER_STARTED   (1 << eVppIpFlag_WorkerStarted)
#define IP_DRAIN_PENDING    (1 << eVppIpFlag_DrainPending)
#define IP_EOS_PENDING      (1 << eVppIpFlag_EOSPending)
#define IP_CORRUPT_BUFFERS  (1 << eVppIpFlag_CorruptBuffers)
#define IP_FLUSH_PENDING    (1 << eVppIpFlag_FlushPending)

#define LOG_CMD             vVppIp_LogCmd
#define GET_IP_STR(cmd)     vVppIp_GetStateStr(cmd)

#define VPP_IP_STATE_SET(_cb, state) do { \
    t_EVppIpState ePrev = (_cb)->eState; \
    (_cb)->eState = state; \
    LOGI("state transition: %s to %s", GET_IP_STR(ePrev), GET_IP_STR(state)); \
} while(0)

#define VPP_IP_STATE_EQUAL(cb, state) \
    (cb->eState == state ? VPP_TRUE : VPP_FALSE)

typedef enum {
    VPP_IP_STATE_NULL = 0,
    VPP_IP_STATE_INITED,
    VPP_IP_STATE_ACTIVE,
    VPP_IP_STATE_OPENING,
    VPP_IP_STATE_CLOSING,
    VPP_IP_STATE_MAX,
} t_EVppIpState;

typedef enum {
    VPP_IP_CMD_THREAD_EXIT = 1,
    VPP_IP_CMD_INIT,
    VPP_IP_CMD_TERM,
    VPP_IP_CMD_OPEN,
    VPP_IP_CMD_CLOSE,
    VPP_IP_CMD_FLUSH,
    VPP_IP_CMD_DRAIN,
    VPP_IP_CMD_STOP,
    VPP_IP_CMD_UPDATE_GLOBAL_PARAM,
    VPP_IP_CMD_RECONFIGURE,
    VPP_IP_CMD_MAX,
} t_EVppIpCmd;

typedef struct {
    t_EVppIpCmd eCmd;
    union {
        struct {
            enum vpp_port ePort;
        } flush;
        struct {
            uint32_t u32Id;
            uint32_t u32SubId;
        } stGlobalCustomFlag;
    };
} t_StVppIpCmd;

typedef struct {
    t_StVppCtx *pstCtx;
    t_StVppCallback stCb;

    uint32_t u32Flags;
    uint32_t bSecureSession;

    struct vpp_port_param stInputParam;
    struct vpp_port_param stOutputParam;

    void *pvStatsHdl;
} t_StVppIpBase;

#define VPP_IP_PROF_START(_base, _stat) \
    u32VppStats_Start((_base)->pstCtx, (_base)->pvStatsHdl, _stat)

#define VPP_IP_PROF_STOP(_base, _stat) \
    u32VppStats_Stop((_base)->pstCtx, (_base)->pvStatsHdl, _stat)

#define VPP_IP_PROF_REGISTER(_base, _stats, _cnt) \
    u32VppStats_Register((_base)->pstCtx, _stats, _cnt, &((_base)->pvStatsHdl))

#define VPP_IP_PROF_UNREGISTER(_base) \
    u32VppStats_Unregister((_base)->pstCtx, (_base)->pvStatsHdl)

#define VPP_IP_PROF_RESET_SINGLE(_base, _stat) \
    u32VppStats_ResetStat((_base)->pstCtx, (_base)->pvStatsHdl, _stat)

typedef struct {
    pthread_cond_t cond;
    pthread_mutex_t mutex;
    t_StVppQueue stCmdQ;
    t_StVppIpCmd *pstCmdNode;
    uint32_t u32Cnt;
} t_StVppIpCmdQueue;

/***************************************************************************
 * Function Prototypes
 ***************************************************************************/

uint32_t u32VppIp_SupportsAlgo(const t_StVppIpBlock *pstIp,
                               enum hqv_control_type type);

uint32_t u32VppIp_GetRequiredMetadata(t_StVppIpBlock *pstIp,
                                      enum hqv_control_type ctrl,
                                      uint32_t *o_pu32Idx,
                                      uint32_t **o_peMeta);

const char *vVppIp_GetStateStr(uint32_t u32);
void vVppIp_LogCmd(const char *pc, t_EVppIpCmd eCmd);

uint32_t u32VppIp_CmdQueueInit(t_StVppIpCmdQueue *pstQ, uint32_t u32Cnt);
uint32_t u32VppIp_CmdQueueTerm(t_StVppIpCmdQueue *pstQ);
uint32_t u32VppIp_CmdGet(t_StVppIpCmdQueue *pstQ, t_StVppIpCmd *pstCmd,
                         uint32_t u32Blocking);
uint32_t u32VppIp_CmdPut(t_StVppIpCmdQueue *pstQ, t_StVppIpCmd stCmd);
uint32_t u32VppIp_CmdCnt(t_StVppIpCmdQueue *pstQ);

uint32_t u32VppIp_SetBase(t_StVppCtx *pstCtx, uint32_t u32Flags,
                          t_StVppCallback cbs, t_StVppIpBase *pstBase);

#ifdef __cplusplus
}
#endif

#endif /* _VPP_IP_H_ */
