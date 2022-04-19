/*!
 * @file vpp_uc.c
 *
 * @cr
 * Copyright (c) 2015-2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.

 * @services
 */

#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

#include "vpp.h"
#define VPP_LOG_TAG     VPP_LOG_MODULE_UC_TAG
#define VPP_LOG_MODULE  VPP_LOG_MODULE_UC
#include "vpp_dbg.h"

#include "vpp_reg.h"
#include "vpp_core.h"
#include "vpp_utils.h"

#include "vpp_uc.h"

/************************************************************************
 * Local definitions
 ***********************************************************************/

#ifndef VPP_TEST_BUILD
#define VPP_GET_UC_REGISTRY(pucnt)      ppstVppUsecase_GetRegistry(pucnt)
#define VPP_GET_UC_AUTO()               pstVppUsecase_GetAuto()
#else
#define VPP_GET_UC_REGISTRY(pucnt)      ppstVppUsecase_GetTestRegistry(pucnt)
#define VPP_GET_UC_AUTO()               pstVppUsecase_GetTestAuto()
#endif

#define VPP_UC_BYPASS_FACTOR_IN_ADD     0
#define VPP_UC_BYPASS_FACTOR_IN_MUL     0
#define VPP_UC_BYPASS_FACTOR_OUT_ADD    0
#define VPP_UC_BYPASS_FACTOR_OUT_MUL    0

/************************************************************************
 * Local static variables
 ***********************************************************************/
#ifdef VPP_TEST_BUILD
static struct {
    uint32_t bValid;
    uint32_t u32RegSz;
    t_StVppUsecase **ppstUcReg;
    t_StVppUsecase *pstUcAuto;
} stVppUcTestCb;
#endif

static t_StVppConcCb  concurrency_main = {
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .pHead = NULL,
};

/************************************************************************
 * Forward Declarations
 ************************************************************************/

/************************************************************************
 * Test Functions
 ***********************************************************************/
#ifdef VPP_TEST_BUILD
void vVppUsecase_InitTestRegistry(uint32_t u32RegSz,
                                  t_StVppUsecase **ppstUcReg,
                                  t_StVppUsecase *pstUcAuto)
{
    stVppUcTestCb.bValid = VPP_TRUE;
    stVppUcTestCb.u32RegSz = u32RegSz;
    stVppUcTestCb.ppstUcReg = ppstUcReg;
    stVppUcTestCb.pstUcAuto = pstUcAuto;
}

void vVppUsecase_TermTestRegistry()
{
    memset(&stVppUcTestCb, 0, sizeof(stVppUcTestCb));
}

static t_StVppUsecase ** ppstVppUsecase_GetTestRegistry(uint32_t *o_pu32Cnt)
{
    if (stVppUcTestCb.bValid)
    {
        *o_pu32Cnt = stVppUcTestCb.u32RegSz;
        return stVppUcTestCb.ppstUcReg;
    }

    return ppstVppUsecase_GetRegistry(o_pu32Cnt);
}

static t_StVppUsecase * pstVppUsecase_GetTestAuto()
{
    if (stVppUcTestCb.bValid)
    {
        return stVppUcTestCb.pstUcAuto;
    }

    return pstVppUsecase_GetAuto();
}
#endif

/************************************************************************
 * Local Functions
 ***********************************************************************/

/*!
* @brief       Checks if HQV Ctrl Enabled
*
* @description Checks if a specific HQV control type has been enabled in the control structure
*
* @in          pstCtrl The HQV Control structure
* @in          ctrl The control type
*
* @return      TRUE if enabled, else FALSE.
*/
static uint32_t isCtrlEnabled(t_StVppHqvCtrl *pstCtrl, enum hqv_control_type ctrl)
{
    if (ctrl == HQV_CONTROL_DI &&
        pstCtrl->di.mode != HQV_DI_MODE_OFF)
        return VPP_TRUE;

    if (ctrl == HQV_CONTROL_TNR &&
        pstCtrl->tnr.mode != HQV_MODE_OFF &&
        pstCtrl->tnr.mode < HQV_MODE_MAX)
        return VPP_TRUE;

    if (ctrl == HQV_CONTROL_CNR &&
        pstCtrl->cnr.mode != HQV_MODE_OFF &&
        pstCtrl->cnr.mode < HQV_MODE_MAX)
        return VPP_TRUE;

    if (ctrl == HQV_CONTROL_AIE &&
        pstCtrl->aie.mode != HQV_MODE_OFF &&
        pstCtrl->aie.mode < HQV_MODE_MAX)
        return VPP_TRUE;

    if (ctrl == HQV_CONTROL_CADE &&
        pstCtrl->cade.mode != HQV_MODE_OFF &&
        pstCtrl->cade.mode < HQV_MODE_MAX)
        return VPP_TRUE;

    if (ctrl == HQV_CONTROL_FRC &&
        pstCtrl->frc.mode != HQV_FRC_MODE_OFF &&
        pstCtrl->frc.mode < HQV_FRC_MODE_MAX)
        return VPP_TRUE;

    return VPP_FALSE;
}


/*!
* @brief       Number of enabled controls
*
* @description Checks count of enabled controls in the control structure
*
* @in          pstCtrl The HQV Control structure
*
* @return      The enabled control count
*/
static uint32_t u32GetEnabledControlCount(t_StVppHqvCtrl *pstCtrl)
{
    uint32_t u32Cnt = 0;

    if (pstCtrl->mode == HQV_MODE_OFF)
        return 0;

    if (isCtrlEnabled(pstCtrl, HQV_CONTROL_DI))
        u32Cnt++;

    if (isCtrlEnabled(pstCtrl, HQV_CONTROL_TNR))
        u32Cnt++;

    if (isCtrlEnabled(pstCtrl, HQV_CONTROL_CNR))
        u32Cnt++;

    if (isCtrlEnabled(pstCtrl, HQV_CONTROL_AIE))
        u32Cnt++;

    if (isCtrlEnabled(pstCtrl, HQV_CONTROL_CADE))
        u32Cnt++;

    if (isCtrlEnabled(pstCtrl, HQV_CONTROL_FRC))
        u32Cnt++;

    return u32Cnt;
}

/*!
* @brief       Checks if input color format is supported
*
* @description Checks if IP block supports the color format set in the config block
*
* @in          pstParam The VPP Param structure
* @in          pstIp The VPP IP block
*
* @return      TRUE if supported, FALSE if not
*/
static uint32_t u32GetColorFormatSupported(t_StVppParam *pstParam, const t_StVppIpBlock *pstIp)
{
    uint32_t u32Supported = VPP_FALSE;
    uint32_t u32InputColorFormat, i;

    u32InputColorFormat = pstParam->input_port.fmt;

    for (i = 0; i < (pstIp->stInputPort.u32FmtCnt); i++)
    {
        if (pstIp->stInputPort.peColorFmts[i] == u32InputColorFormat)
        {
            u32Supported = VPP_TRUE;
            break;
        }
    }

    return u32Supported;
}

/*!
* @brief       Checks if video size is supported
*
* @description Checks if IP block supports the given video size
*
* @in          pstParam The VPP Param structure
* @in          pstIp The VPP IP block
*
* @return      TRUE if supported, FALSE if not
*/
static uint32_t u32GetSizeSupported(t_StVppParam *pstParam, const t_StVppIpBlock *pstIp)
{
    uint32_t u32Width, u32Height;

    u32Width = pstParam->input_port.width;
    u32Height = pstParam->input_port.height;

    if ((u32Width >= pstIp->capabilities.min_res.u32Width) &&
        (u32Width <= pstIp->capabilities.max_res.u32Width) &&
        (u32Height >= pstIp->capabilities.min_res.u32Height) &&
        (u32Height <= pstIp->capabilities.max_res.u32Height) &&
        (u32Width <= RES_UHD_MAX_W) && (u32Height <= RES_UHD_MAX_H))
        return VPP_TRUE;

    LOGD("%dx%d NOT SUPPORTED", u32Width, u32Height);
    return VPP_FALSE;
}


/*!
* @brief       Checks if port configuration quirks are supported
*
* @description Checks if IP block supports the flags set in the config block
*
* @in          pstParam The VPP Param structure
* @in          pstIp The VPP IP block
*
* @return      TRUE if supported, FALSE if not
*/
static uint32_t u32GetQuirksSupported(t_StVppConcSession *pstConfigureSession,
                                      t_StVppParam *pstParam, const t_StVppIpBlock *pstIp)
{
    uint32_t i;
    t_EVppIp tIp;
    uint32_t u32OtherSessionFlags = 0;
    uint32_t u32OtherSessionOpen = 0;
    uint32_t u32Quirks;
    t_StVppConcSession *pstCurrentSession;

    tIp = pstIp->eIp;
    if (tIp >= VPP_IP_MAX)
    {
        return VPP_FALSE;
    }

    u32Quirks = pstIp->capabilities.u32Quirks;

    pstCurrentSession = concurrency_main.pHead;

    while (pstCurrentSession != NULL)
    {
        if ((pstCurrentSession->pstUcCtx != NULL) && (pstCurrentSession != pstConfigureSession) &&
           (pstCurrentSession->bInUse))
        {
            u32OtherSessionOpen = 1;
            // Accumulate all flags for other sessions using this IP
            u32OtherSessionFlags |= pstCurrentSession->u32Flags;
        }
        pstCurrentSession = pstCurrentSession->pNext;
    }

    // Only need to check quirks if other sessions are using the same IP
    if (u32OtherSessionOpen)
    {
        if (u32Quirks & VPP_QUIRK_NO_CONCURRENT_CP_NCP_PID)
        {
            // XOR secure session flag to make sure they are the same
            if (((u32OtherSessionFlags & VPP_SESSION_SECURE) ^
                 (pstParam->u32SessionFlags & VPP_SESSION_SECURE)))
            {
               //Secure and insecure sessions
               LOGD("Quirks check failed for IP[%d]! No concurrent CP with non CP!", tIp);
               return VPP_FALSE;
            }

        }
    }
    return VPP_TRUE;
}



/*!
* @brief       Checks use case credit requirements
*
* @description Checks credits required per frame for a given use case at a given resolution
*
* @in          pstUc The use case
* @in          t_resolution The chosen resolution
* @out         opu32Credits The structure containing the credits for each IP
*
* @return      OK if no errors, ERR otherwise
*/
static uint32_t u32GetUcCreditsPerFrame(t_StVppUsecase *pstUc, t_EVppRes tResolution,
                                        uint32_t *o_pu32Credits)
{
    uint32_t i;

    if (tResolution >= VPP_RESOLUTION_MAX)
    {
        LOGE("%s(): invalid resolution: %u", __func__, tResolution);
        return VPP_ERR_PARAM;
    }

    for (i = 0; i < VPP_IP_BLOCK_MAX; i++)
    {
        if (pstUc->credits[i].ip == NULL)
            continue;
        else
        {
            if (pstUc->credits[i].au32Credit[tResolution] == 0)
            {
                LOGE("ERROR, CREDITS NOT DEFINED FOR IP BLOCK IN UC\n");
                return VPP_ERR;
            }
            o_pu32Credits[i] = pstUc->credits[i].au32Credit[tResolution];
        }
    }
    return VPP_OK;
}

/*!
* @brief       Checks use case concurrency
*
* @description Checks concurrency requirements for a given use case
*
* @in          pstUc The use case
* @out         opu32MaxSessions The structure containing the max concurrency for each IP
*
* @return      OK if no errors, ERR otherwise
*/
static uint32_t u32GetUcMaxSessions(t_StVppUsecase *pstUc, uint32_t *o_pu32MaxSessions)
{

    uint32_t i;
    t_EVppIp tIp;

    for (i = 0; i < VPP_IP_BLOCK_MAX; i++)
    {
        if (pstUc->composition[i].ip == NULL)
            continue;
        else
        {
            tIp = pstUc->composition[i].ip->eIp;
            if (pstUc->composition[i].u32MaxConcurrentStreams == 0)
            {
                LOGE("ERROR, MAX STREAMS NOT DEFINED FOR IP BLOCK IN UC\n");
                return VPP_ERR;
            }
            if (tIp >= VPP_IP_BLOCK_MAX)
            {
                LOGE("ERROR, INVALID IP BLOCK %d DEFINED IN UC\n", tIp);
                return VPP_ERR;
            }
            o_pu32MaxSessions[tIp] = pstUc->composition[i].u32MaxConcurrentStreams;
        }
    }
    return VPP_OK;
}

/*!
* @brief       Find an IP's index in a composition
*
* @description Checks the compostion structure of a UC and returns IP index if it exists
*
* @in          pstUc The use case
* @in          tIp The IP to look for
*
* @return      The IP index, NOT_SUPPORTED if use case does not support it
*/
static uint32_t u32GetCompIpIndex(t_StVppUsecase *pstUc, t_EVppIp tIp)
{

    uint32_t i;

    for (i = 0; i < VPP_IP_BLOCK_MAX; i++)
    {
        // The composition is the processing order, so first NULL means no more IP blocks
        if (pstUc->composition[i].ip == NULL)
            return NOT_SUPPORTED;
        else if (pstUc->composition[i].ip->eIp == tIp)
            return i;
    }
    return NOT_SUPPORTED;
}


/************************************************************************
 * Global Functions
 ***********************************************************************/

/*!
* @brief       Initialize use case for a context
*
* @description Initializes needed resources for concurrency management
*
* @in          pstCtx The context being initialized
*
* @return      None
*/
uint32_t u32VppUsecase_Init(t_StVppCtx *pstCtx)
{
    uint32_t u32SessionAvailable = 0;
    uint32_t u32ConcurrencySession;
    uint32_t i;
    t_StVppConcSession *pstCurrentSession, *pstNextSession, *pstNewSession;

    LOGI("%s\n", __func__);

    if (!pstCtx)
        return VPP_ERR_PARAM;

    pthread_mutex_lock(&concurrency_main.mutex);

    if (concurrency_main.pHead != NULL)
    {
        pstNextSession = concurrency_main.pHead;
        while (pstNextSession != NULL)
        {
            pstCurrentSession = pstNextSession;
            if (pstCurrentSession->pstUcCtx == pstCtx)
            {
                // Context already has a session open
                LOGE("CONTEXT %p ALREADY OPEN\n", pstCtx);

                pthread_mutex_unlock(&concurrency_main.mutex);
                return VPP_ERR_INVALID_CFG;
            }
            pstNextSession = pstNextSession->pNext;
        }
    }

    pstNewSession = calloc(1, sizeof(t_StVppConcSession));
    if (!pstNewSession)
    {
        LOGE("ERROR ALLOCATING NEW SESSION\n");

        pthread_mutex_unlock(&concurrency_main.mutex);
        return VPP_ERR_NO_MEM;
    }
    else
    {
        if (concurrency_main.pHead == NULL)
        {
            concurrency_main.pHead = pstNewSession;
            LOGI("Init context %p to head", pstCtx);
        }
        else
        {
            pstCurrentSession->pNext = pstNewSession;
            LOGI("Init context %p", pstCtx);
        }
        pstNewSession->pstUcCtx = pstCtx;
    }

    pthread_mutex_unlock(&concurrency_main.mutex);

    return VPP_OK;
}

/*!
* @brief       Terminate use case
*
* @description Returns used resources for concurrency management
*
* @in          pstCtx The context being terminated
*
* @return      None
*/
void vVppUsecase_Term(t_StVppCtx *pstCtx)
{
    uint32_t i;
    t_StVppConcSession *pstCurrentSession, *pstPreviousSession;

    LOGI("%s\n", __func__);

    if (!pstCtx)
        return;

    pthread_mutex_lock(&concurrency_main.mutex);

    pstCurrentSession = concurrency_main.pHead;
    pstPreviousSession = concurrency_main.pHead;

    while (pstCurrentSession != NULL)
    {
        if (pstCurrentSession->pstUcCtx == pstCtx)
        {
            if (pstCurrentSession == concurrency_main.pHead)
            {
                concurrency_main.pHead = pstCurrentSession->pNext;
            }
            else
            {
                pstPreviousSession->pNext = pstCurrentSession->pNext;
            }
            LOGI("Closing session %p", pstCurrentSession->pstUcCtx);
            free(pstCurrentSession);
            pthread_mutex_unlock(&concurrency_main.mutex);
            return;
        }

        pstPreviousSession = pstCurrentSession;
        pstCurrentSession = pstCurrentSession->pNext;
    }
    LOGE("ERR Closing. Session not open! \n");

    pthread_mutex_unlock(&concurrency_main.mutex);
    return;

}

/*!
* @brief       Finds a use case
*
* @description Finds a use case for a given set of HQV controls
*
* @in          pstCtrl The HQV control structure
* @in          pstParam The VPP parameter structure
*
* @return      The use case if found, NULL otherwise
*/
t_StVppUsecase *pstVppUsecase_Find(t_StVppHqvCtrl *pstCtrl, t_StVppParam *pstParam)
{
    t_StVppUsecase **ppstUcReg;
    t_StVppUsecase *pstUc;
    uint32_t u32RegCnt, u32EnableCnt, u32Resolution, u32CompIpIndex;
    uint32_t i, j, k;

    LOGI("%s\n", __func__);

    if (!pstCtrl || pstCtrl->mode >= HQV_MODE_MAX || pstCtrl->mode == HQV_MODE_OFF)
        return NULL;

    ppstUcReg = VPP_GET_UC_REGISTRY(&u32RegCnt);
    if (!ppstUcReg || !u32RegCnt)
        return NULL;

    if (pstCtrl->mode == HQV_MODE_AUTO)
    {
        pstUc = VPP_GET_UC_AUTO();
        if (pstParam)
        {
            u32Resolution = u32VppUtils_GetVppResolution(&pstParam->input_port);

            if (u32Resolution >= VPP_RESOLUTION_MAX)
                return NULL;

            for (i = 0; i < VPP_IP_BLOCK_MAX; i++)
            {
                if (pstUc->credits[i].ip == NULL)
                    continue;
                u32CompIpIndex = u32GetCompIpIndex(pstUc, i);
                if ((u32CompIpIndex != NOT_SUPPORTED) &&
                   ((!u32GetColorFormatSupported(pstParam, pstUc->composition[u32CompIpIndex].ip)) ||
                   (!u32GetSizeSupported(pstParam, pstUc->composition[u32CompIpIndex].ip))))
                    return NULL;
                if (pstUc->credits[i].au32Credit[u32Resolution] == NOT_SUPPORTED)
                    return pstUc->apstFallback[u32Resolution];
            }
            return pstUc;
        }
        else
        {
            return pstUc;
        }
    }
    else if (pstCtrl->mode == HQV_MODE_MANUAL)
    {
        u32EnableCnt = u32GetEnabledControlCount(pstCtrl);
        if (!u32EnableCnt)
            return NULL;

        // Iterate over all the usecases in the registry
        for (i = 0; i < u32RegCnt; i++)
        {
            pstUc = ppstUcReg[i];

            if (pstUc->algos.u32CtrlCnt != u32EnableCnt)
                continue;

            for (j = 0; j < pstUc->algos.u32CtrlCnt; j++)
            {
                enum hqv_control_type ctrl;
                ctrl = pstUc->algos.ctrls[j];
                if (!isCtrlEnabled(pstCtrl, ctrl))
                    break;
            }

            // Found the match
            if (j == pstUc->algos.u32CtrlCnt)
            {
                if (pstParam)
                {
                    u32Resolution = u32VppUtils_GetVppResolution(&pstParam->input_port);

                    if (u32Resolution >= VPP_RESOLUTION_MAX)
                        return NULL;

                    for (k = 0; k < VPP_IP_BLOCK_MAX; k++)
                    {
                        if (pstUc->credits[k].ip == NULL)
                            continue;
                        u32CompIpIndex = u32GetCompIpIndex(pstUc, k);
                        if ((u32CompIpIndex != NOT_SUPPORTED) &&
                           ((!u32GetColorFormatSupported(pstParam,
                                                         pstUc->composition[u32CompIpIndex].ip)) ||
                           (!u32GetSizeSupported(pstParam,
                                                 pstUc->composition[u32CompIpIndex].ip))))
                            return NULL;
                        if (pstUc->credits[k].au32Credit[u32Resolution] == NOT_SUPPORTED)
                            return pstUc->apstFallback[u32Resolution];
                    }
                    return pstUc;
                }
                else
                {
                    return pstUc;
                }
            }
        }
    }

    return NULL;
}

/*!
* @brief       Configure a use case
*
* @description Configure, acquire resources, and update concurrency for a use case
*
* @in          pstCtx The context being configured
* @in          pstUc The use case being configured
* @in          pstCtrl The HQV control structure
* @in          pstParam The VPP parameter structure
*
* @return      OK if use case configured, ERR otherwise
*/
uint32_t u32VppUsecase_Configure(t_StVppCtx *pstCtx, t_StVppUsecase *pstUc,
                                 t_StVppHqvCtrl *pstCtrl, t_StVppParam *pstParam)
{
    uint32_t u32Resolution, u32Fps;
    uint32_t au32UsedCredit[VPP_IP_BLOCK_MAX];
    uint32_t au32RequiredCredit[VPP_IP_BLOCK_MAX];
    uint32_t au32UcMaxSessions[VPP_IP_BLOCK_MAX];
    uint32_t au32ConcMaxSessions[VPP_IP_BLOCK_MAX];
    uint32_t au32UcReqIPSessions[VPP_IP_BLOCK_MAX];
    uint32_t au32UsedIPSessions[VPP_IP_BLOCK_MAX];
    uint32_t u32CompIpIndex;
    uint32_t i,j;
    t_StVppConcSession *pstCurrentSession;
    t_StVppConcSession *pstConfigureSession = NULL;

    LOGI("%s\n", __func__);

    if (!pstCtx || !pstUc || !pstCtrl || !pstParam)
        return VPP_ERR_PARAM;

    memset(au32UsedCredit, 0, sizeof(uint32_t)*VPP_IP_BLOCK_MAX);
    memset(au32UcReqIPSessions, 0, sizeof(uint32_t)*VPP_IP_BLOCK_MAX);
    memset(au32UsedIPSessions, 0, sizeof(uint32_t)*VPP_IP_BLOCK_MAX);
    memset(au32UcMaxSessions, 0, sizeof(uint32_t)*VPP_IP_BLOCK_MAX);
    memset(au32RequiredCredit, 0, sizeof(uint32_t)*VPP_IP_BLOCK_MAX);

    // Get max concurrency for each IP in this UC, 0 means IP is unused
    if (u32GetUcMaxSessions(pstUc, au32UcMaxSessions))
        return VPP_ERR;

    for (i = 0; i < VPP_IP_BLOCK_MAX; i++)
    {
        // Set max supported to UC for now. Need to check other sessions later
        au32ConcMaxSessions[i] = au32UcMaxSessions[i];
        if (au32UcMaxSessions[i] != 0)
            au32UcReqIPSessions[i] = 1;
    }
    u32Resolution = u32VppUtils_GetVppResolution(&pstParam->input_port);

    pthread_mutex_lock(&concurrency_main.mutex);
    pstCurrentSession = concurrency_main.pHead;

    while (pstCurrentSession != NULL)
    {
        if (pstCurrentSession->pstUcCtx == pstCtx)
        {
            // Context already has a session open
            LOGD("Found context to configure \n");
            pstConfigureSession = pstCurrentSession;
        }
        else
        {
            if (pstCurrentSession->bInUse)
            {
                // Session is in use by another context
                // Accumulate used credits and max sessions
                for (j = 0; j < VPP_IP_BLOCK_MAX; j++)
                {
                    au32UsedCredit[j] += pstCurrentSession->au32Credit[j];

                    if (pstCurrentSession->au32MaxConcSession[j] != 0)
                    {
                        // This IP block is being used by this session
                        au32UsedIPSessions[j]++;

                        if (au32ConcMaxSessions[j] == 0)
                        {
                            // No other checked sessions are using this IP
                            au32ConcMaxSessions[j] = pstCurrentSession->au32MaxConcSession[j];
                        }
                        else if ((pstCurrentSession->au32MaxConcSession[j]) < au32ConcMaxSessions[j])
                        {
                            // Other sessions are using this IP.  Take the lower IP concurrency
                            au32ConcMaxSessions[j] = pstCurrentSession->au32MaxConcSession[j];
                        }
                    }
                }
            }
        }
        pstCurrentSession = pstCurrentSession->pNext;
    }

    if (pstConfigureSession == NULL)
    {
        LOGE("Context %p did not init. No session to configure!! \n", pstCtx);

        pthread_mutex_unlock(&concurrency_main.mutex);
        return VPP_ERR_INVALID_CFG;
    }

    // Check credits, concurrent sessions
    if (u32GetUcCreditsPerFrame(pstUc, u32Resolution, au32RequiredCredit))
    {
        pstConfigureSession->bInUse = 0;
        pthread_mutex_unlock(&concurrency_main.mutex);
        return VPP_ERR;
    }

    for (i = 0; i < VPP_IP_BLOCK_MAX; i++)
    {
        if (pstUc->credits[i].ip == NULL)
        {
            LOGI("IP[%d] not used", i);
            continue;
        }
        u32CompIpIndex = u32GetCompIpIndex(pstUc, i);
        if (u32CompIpIndex != NOT_SUPPORTED)
        {
            if (!u32GetSizeSupported(pstParam, pstUc->composition[u32CompIpIndex].ip))
            {
                LOGD("RESOLUTION %dx%d NOT SUPPORTED FOR IP %d \n", pstParam->input_port.width,
                     pstParam->input_port.height, i);
                pstConfigureSession->bInUse = 0;

                pthread_mutex_unlock(&concurrency_main.mutex);
                return VPP_ERR_INVALID_CFG;
            }
            if (pstParam->u32SessionFlags & VPP_SESSION_SECURE)
            {
                if (!pstUc->composition[u32CompIpIndex].ip->capabilities.bCp)
                {
                    LOGD("SECURE SESSION NOT SUPPORTED FOR IP %d \n", i);
                    pstConfigureSession->bInUse = 0;

                    pthread_mutex_unlock(&concurrency_main.mutex);
                    return VPP_ERR_INVALID_CFG;
                }
            }
            LOGI("IP[%d]: UC needs %d sessions. UC max: %d, Other sessions max: %d, In use: %d",
                 i, au32UcReqIPSessions[i], au32UcMaxSessions[i], au32ConcMaxSessions[i],
                 au32UsedIPSessions[i]);
            if (au32UcReqIPSessions[i])
            {
                au32UsedIPSessions[i]++;
                if ((au32UsedIPSessions[i] > au32ConcMaxSessions[i]) ||
                   (au32UsedIPSessions[i] >
                   pstUc->composition[u32CompIpIndex].ip->capabilities.u32MaxSessions))
                {
                    LOGD("ALREADY MAX SESSION FOR IP %d \n", i);
                    pstConfigureSession->bInUse = 0;

                    pthread_mutex_unlock(&concurrency_main.mutex);
                    return VPP_ERR_RESOURCES;
                }
            }

            // Check quirks
            if (!u32GetQuirksSupported(pstConfigureSession, pstParam,
               pstUc->composition[u32CompIpIndex].ip))
            {
                pstConfigureSession->bInUse = 0;

                pthread_mutex_unlock(&concurrency_main.mutex);
                return VPP_ERR_INVALID_CFG;
            }
        }

        if (au32RequiredCredit[i] == NOT_SUPPORTED)
        {
            LOGD("CONFIGURED RESOLUTION NOT SUPPORTED FOR IP %d \n", i);
            pstConfigureSession->bInUse = 0;

            pthread_mutex_unlock(&concurrency_main.mutex);
            return VPP_ERR_INVALID_CFG;
        }
         // Use default FPS for use case if not set yet
        if (pstConfigureSession->u32Fps == 0)
        {
            if (!pstUc->u32DefaultInFps)
            {
                LOGE("Default FPS not defined for UC!\n");
                pstConfigureSession->u32Fps = VPP_DEFAULT_FRAME_RATE;
            }
            else
            {
                pstConfigureSession->u32Fps = pstUc->u32DefaultInFps;
            }
        }

        u32Fps = pstConfigureSession->u32Fps;
        au32RequiredCredit[i] *= u32Fps;
        LOGD("UC requires %d credits for %d FPS in IP[%d] \n",
               au32RequiredCredit[i], u32Fps, i);

        if (pstUc->credits[i].ip->capabilities.u32MaxCredits < (au32UsedCredit[i] +
            au32RequiredCredit[i]))
        {
            LOGD("NOT ENOUGH CREDITS FOR IP %d \n", i);
            pstConfigureSession->bInUse = 0;

            pthread_mutex_unlock(&concurrency_main.mutex);
            return VPP_ERR_RESOURCES;
        }
    }

    // Store use case/update credits
    for (i = 0; i < VPP_IP_BLOCK_MAX; i++)
    {
        pstConfigureSession->au32Credit[i] = au32RequiredCredit[i];
        pstConfigureSession->au32MaxConcSession[i] = au32UcMaxSessions[i];
        LOGD("CONFIGURED SESSION, Total used credits[%d]: %d\n", i,
             au32UsedCredit[i] + au32RequiredCredit[i]);
    }

    pstConfigureSession->u32Flags = pstParam->u32SessionFlags;
    pstConfigureSession->pstUcCtx = pstCtx;
    pstConfigureSession->tResolution = u32Resolution;
    pstConfigureSession->pstUc = pstUc;
    pstConfigureSession->bInUse = 1;

    pthread_mutex_unlock(&concurrency_main.mutex);
    return VPP_OK;

}


/*!
* @brief       Validate a use case
*
* @description Checks if use case is valid given the current configuration
*
* @in          pstCtrl The HQV control structure
* @in          pstUc The use case being validated
* @in          pstParam The VPP parameter structure
* @out         o_pstUcFallBack The fallback use case (if exists), if given use case is invalid
*
* @return      OK if use case validated, ERR otherwise
*/
uint32_t u32VppUsecase_Validate(t_StVppHqvCtrl *pstCtrl, t_StVppUsecase *pstUc,
                                t_StVppParam *pstParam, t_StVppUsecase **o_pstUcFallBack)
{
    uint32_t u32Resolution, u32CompIpIndex;
    uint32_t i;

    LOGI("%s\n", __func__);

    VPP_UNUSED(pstCtrl);

    if (o_pstUcFallBack != NULL)
        *o_pstUcFallBack = NULL;

    if (!pstUc || !pstParam)
        return VPP_ERR_PARAM;


    u32Resolution = u32VppUtils_GetVppResolution(&pstParam->input_port);

    if (u32Resolution >= VPP_RESOLUTION_MAX)
        return VPP_ERR_INVALID_CFG;

    for (i = 0; i < VPP_IP_BLOCK_MAX; i++)
    {
        if (pstUc->credits[i].ip == NULL)
            continue;

        u32CompIpIndex = u32GetCompIpIndex(pstUc, i);
        if ((u32CompIpIndex != NOT_SUPPORTED) &&
           ((!u32GetColorFormatSupported(pstParam,
                                         pstUc->composition[u32CompIpIndex].ip)) ||
           (!u32GetSizeSupported(pstParam,
                                 pstUc->composition[u32CompIpIndex].ip))))
            return VPP_ERR_INVALID_CFG;
        if (pstUc->credits[i].au32Credit[u32Resolution] == 0)
        {
            LOGE("ERROR IN USE CASE REGISTRY DEFINITION! CREDITS NOT DEFINED");
            return VPP_ERR;
        }
        if (pstUc->credits[i].au32Credit[u32Resolution] == NOT_SUPPORTED)
        {
            if (o_pstUcFallBack != NULL)
                *o_pstUcFallBack = pstUc->apstFallback[u32Resolution];
            return VPP_ERR_INVALID_CFG;
        }
    }
    return VPP_OK;

}


/*!
* @brief       Update session framerate
*
* @description Update the framerate/credit use of an open session
*
* @in          pstCtx The context being reconfigured
* @in          pstUc The use case being reconfigured
* @in          u32Fps The new frame rate
*
* @return      OK if new framerate supported, ERR otherwise
*/
uint32_t u32VppUsecase_UpdateFramerate(t_StVppCtx *pstCtx, t_StVppUsecase *pstUc,
                                       uint32_t u32Fps, t_EVppBufType eBufType)
{
    uint32_t u32ConcurrencySession, u32Resolution, u32MaxFpsIndex, u32MaxFps;
    uint32_t i, j;
    uint32_t au32RequiredCredit[VPP_IP_BLOCK_MAX];
    uint32_t au32UsedCredit[VPP_IP_BLOCK_MAX];
    t_StVppConcSession *pstCurrentSession;
    t_StVppConcSession *pstConfigureSession = NULL;

    LOGI("%s\n", __func__);

    if (!pstCtx || !pstUc || !u32Fps)
        return VPP_ERR_PARAM;

    memset(au32UsedCredit, 0, sizeof(uint32_t)*VPP_IP_BLOCK_MAX);
    memset(au32RequiredCredit, 0, sizeof(uint32_t)*VPP_IP_BLOCK_MAX);

    pthread_mutex_lock(&concurrency_main.mutex);

    pstCurrentSession = concurrency_main.pHead;

    while (pstCurrentSession != NULL)
    {
        if (pstCurrentSession->pstUcCtx == pstCtx)
        {
            // Context already has a session open
            LOGD("Found context to configure \n");
            pstConfigureSession = pstCurrentSession;
        }
        else
        {
            if (pstCurrentSession->bInUse)
            {
                // Session is in use by another context
                // Accumulate used credits
                for (j = 0; j < VPP_IP_BLOCK_MAX; j++)
                {
                    au32UsedCredit[j] += pstCurrentSession->au32Credit[j];
                }
            }
        }
        pstCurrentSession = pstCurrentSession->pNext;
    }

    if (pstConfigureSession == NULL)
    {
        LOGE("Session not open to update FPS! \n");

        pthread_mutex_unlock(&concurrency_main.mutex);
        return VPP_ERR_INVALID_CFG;
    }

    u32Resolution = pstConfigureSession->tResolution;
    if ((u32Resolution >= VPP_RESOLUTION_MAX) ||
        (u32GetUcCreditsPerFrame(pstUc, u32Resolution, au32RequiredCredit) != VPP_OK))
    {
        pstConfigureSession->bInUse = 0;
        pthread_mutex_unlock(&concurrency_main.mutex);
        return VPP_ERR;
    }

    // Check max supported frame rate
    if (eBufType == eVppBufType_Progressive)
        u32MaxFpsIndex = VPP_UC_BUFFER_TYPE_P;
    else
        u32MaxFpsIndex = VPP_UC_BUFFER_TYPE_I;
    u32MaxFps = pstUc->au32MaxInFps[u32MaxFpsIndex][u32Resolution];

    // Skip this check if u32MaxFps is not defined (ie. 0)
    if (u32MaxFps && (u32Fps > u32MaxFps))
    {
        LOGD("Current framerate %d exceeds UC max supported framerate %d", u32Fps, u32MaxFps);
        pstConfigureSession->bInUse = 0;

        pthread_mutex_unlock(&concurrency_main.mutex);
        return VPP_ERR_RESOURCES;
    }

    // Check credits
    for (i = 0; i < VPP_IP_BLOCK_MAX; i++)
    {
        if (pstUc->credits[i].ip == NULL)
            continue;

        au32RequiredCredit[i] *= u32Fps;
        LOGD("New frame rate requires %d credits for %d FPS in IP[%d] \n",
               au32RequiredCredit[i], u32Fps, i);

        if (pstUc->credits[i].ip->capabilities.u32MaxCredits < (au32UsedCredit[i] +
           au32RequiredCredit[i]))
        {
            LOGD("NOT ENOUGH CREDITS FOR IP %d \n", i);
            pstConfigureSession->bInUse = 0;

            pthread_mutex_unlock(&concurrency_main.mutex);
            return VPP_ERR_RESOURCES;
        }
    }

    // Update credits/FPS
    for (i = 0; i < VPP_IP_BLOCK_MAX; i++)
    {
        pstConfigureSession->au32Credit[i] = au32RequiredCredit[i];
        LOGD("Updated Session FPS, Total used credits[%d]: %d\n", i,
             au32UsedCredit[i] + au32RequiredCredit[i]);
    }
    pstConfigureSession->u32Fps = u32Fps;
    pstConfigureSession->bInUse = 1;

    pthread_mutex_unlock(&concurrency_main.mutex);
    return VPP_OK;

}



/*!
* @brief       Give session requirements
*
* @description Gives VPP requirements given a use case
*
* @in          pstUc The use case to find session requirements for
* @out         o_pstReq The output requirements structure
*
* @return      OK if requirements found, ERR otherwise
*/
uint32_t u32VppUsecase_GetRequirements(t_StVppUsecase *pstUc, struct vpp_requirements *o_pstReq)
{
    uint32_t u32IpCtrlCnt;
    uint32_t i, j, k, l, m;
    uint32_t u32MetaFound;
    t_StVppAlgo *stpVppAlgo;

    LOGI("%s\n", __func__);

    if (!o_pstReq)
        return VPP_ERR_PARAM;

    if (!pstUc)
    {
        for (i = 0; i < VPP_RESOLUTION_MAX; i++)
        {
            o_pstReq->in_factor[i].add = VPP_UC_BYPASS_FACTOR_IN_ADD;
            o_pstReq->in_factor[i].mul = VPP_UC_BYPASS_FACTOR_IN_MUL;
            o_pstReq->out_factor[i].add = VPP_UC_BYPASS_FACTOR_OUT_ADD;
            o_pstReq->out_factor[i].mul = VPP_UC_BYPASS_FACTOR_OUT_MUL;
        }
        o_pstReq->metadata.cnt = 0;
        return VPP_OK;
    }
    memcpy(o_pstReq->in_factor, pstUc->in_factor,
           VPP_RESOLUTION_MAX * sizeof(struct vpp_port_factor));
    memcpy(o_pstReq->out_factor, pstUc->out_factor,
           VPP_RESOLUTION_MAX * sizeof(struct vpp_port_factor));
    o_pstReq->metadata.cnt = 0;

    for (i = 0; i < VPP_IP_BLOCK_MAX; i++)
    {
         if (pstUc->composition[i].ip == NULL)
             continue;
         u32IpCtrlCnt = pstUc->composition[i].ip->algo_cap.u32AlgoCnt;

         for (j = 0; j < pstUc->algos.u32CtrlCnt; j++)
         {
             for (k = 0; k < u32IpCtrlCnt; k++)
             {
                stpVppAlgo = &(pstUc->composition[i].ip->algo_cap.algos[k]);
                if (pstUc->algos.ctrls[j] == stpVppAlgo->ctrl)
                {
                    LOGD("Getting meta req for UC control %d in IP[%d]: Control %d\n", j, i, k);
                    for (l = 0; l < stpVppAlgo->u32MetaCnt; l++)
                    {
                        u32MetaFound = 0;
                        for (m = 0; m < o_pstReq->metadata.cnt; m++)
                        {
                            if (stpVppAlgo->meta[l] ==
                               o_pstReq->metadata.meta[m])
                                u32MetaFound = 1;
                        }
                        if (u32MetaFound == 0)
                        {
                            if (o_pstReq->metadata.cnt < META_MAX_CNT)
                            {
                                LOGD("Adding new meta requirement: %d\n", stpVppAlgo->meta[l]);
                                o_pstReq->metadata.meta[o_pstReq->metadata.cnt] =
                                    stpVppAlgo->meta[l];
                                o_pstReq->metadata.cnt++;
                            }
                            else
                            {
                                LOGE("Can't add meta requirement %d. Count already at max %d",
                                     stpVppAlgo->meta[l], o_pstReq->metadata.cnt);
                            }
                        }
                        else
                            LOGD("Required meta previously added\n");
                    }
                }
             }
         }
    }
    LOGD("Added %d metadata requirements", o_pstReq->metadata.cnt);
    return VPP_OK;
}

/*!
* @brief       Checks if use cases have the same composition
*
* @description Compares two use cases to check if they use the same IP blocks in the same order
*
* @in          pstUc1 The first use case to compare
* @in          pstUc2 The second use case to compare
*
* @return      TRUE if same, FALSE otherwise
*/
uint32_t u32VppUsecase_CompositionEqual(t_StVppUsecase *pstUc1,
                                        t_StVppUsecase *pstUc2)
{
    uint32_t i;

    if (!pstUc1 || !pstUc2)
        return VPP_FALSE;

    if (pstUc1 == pstUc2)
        return VPP_TRUE;

    for (i = 0; i < VPP_IP_BLOCK_MAX; i++)
    {
        if (pstUc1->composition[i].ip != pstUc2->composition[i].ip)
            return VPP_FALSE;
    }

    return VPP_TRUE;
}

/*!
* @brief       Number of IPs used
*
* @description Checks the number of IP submodules required for a use case
*
* @in          pstUc The use case to check
*
* @return      The number of IP submodules
*/
uint32_t u32VppUsecase_SubmodCount(t_StVppUsecase *pstUc)
{
    uint32_t i = 0;
    if (!pstUc)
        return 0;

    for (i = 0; i < VPP_IP_MAX; i++)
    {
        if (!pstUc->composition[i].ip)
            break;
    }

    return i;
}

/*!
* @brief       Checks if CP is supported
*
* @description Checks if any IP in use case registry can support CP
*
* @return      TRUE if CP is supported, FALSE otherwise
*/
uint32_t u32VppIsCpSupported()
{
    t_EVppIp tCheckedIp[VPP_IP_BLOCK_MAX];
    t_EVppIp tIp;
    uint32_t u32IpCount = 0;
    t_StVppUsecase **ppstUcReg;
    t_StVppUsecase *pstUc;
    uint32_t u32RegCnt, u32IpChecked;
    uint32_t i, j, k;

    LOGI("%s\n", __func__);

    memset(&tCheckedIp[0], 0, sizeof(t_EVppIp) * VPP_IP_BLOCK_MAX);

    ppstUcReg = VPP_GET_UC_REGISTRY(&u32RegCnt);
    if (!ppstUcReg || !u32RegCnt)
        return VPP_FALSE;

    // Iterate over all the usecases in the registry
    for (i = 0; i < u32RegCnt; i++)
    {
        pstUc = ppstUcReg[i];

        for (j = 0; j < VPP_IP_BLOCK_MAX; j++)
        {
            if (pstUc->composition[j].ip == NULL)
                continue;
            tIp = pstUc->composition[j].ip->eIp;
            if (pstUc->composition[j].ip->capabilities.bCp)
            {
                LOGD("CP Supported in IP[%d].\n",tIp);
                return VPP_TRUE;
            }
            u32IpChecked = 0;
            for (k = 0; k < u32IpCount; k++)
            {
                if (tCheckedIp[k] == tIp)
                    u32IpChecked = 1;
            }
            if (u32IpChecked == 0)
            {
                LOGD("Checked IP[%d] doesn't support CP.\n", tIp);
                tCheckedIp[u32IpCount] = tIp;
                u32IpCount++;
            }
            if (u32IpCount >= VPP_IP_BLOCK_MAX)
            {
                LOGD("Checked MAX IP blocks. No CP support found.\n");
                return VPP_FALSE;
            }
        }
    }
    LOGD("Checked all usecases. No CP support found.\n");
    return VPP_FALSE;
}


/*!
* @brief       Checks if Set HQV Ctrl is supported
*
* @description Checks if the HQV Ctrl is supported for the given use case
*              during active video
*
* @in          pstUC The use case to verify controls for
*
* @return      TRUE if ctrl is supported, FALSE otherwise
*/
uint32_t u32VppUsecase_IsCtrlSupported(t_StVppUsecase *pstUc, struct hqv_control stCtrl)
{
    uint32_t i;

    LOGI("%s\n", __func__);

    if (!pstUc)
        return VPP_FALSE;

    // Custom and global controls are accepted for any use case. IP block will handle them
    if ((stCtrl.ctrl_type == HQV_CONTROL_CUST) || ((stCtrl.ctrl_type >= HQV_CONTROL_GLOBAL_START) &&
        stCtrl.ctrl_type < HQV_CONTROL_MAX))
        return VPP_TRUE;

    for (i = 0; i < pstUc->algos.u32CtrlCnt; i++)
    {
        enum hqv_control_type ctrl;
        ctrl = pstUc->algos.ctrls[i];
        if (ctrl == stCtrl.ctrl_type)
        {
            switch (stCtrl.ctrl_type)
            {
                case HQV_CONTROL_CADE:
                    if (stCtrl.cade.mode != HQV_MODE_OFF)
                        return VPP_TRUE;
                    break;
                case HQV_CONTROL_DI:
                    if (stCtrl.di.mode!= HQV_DI_MODE_OFF)
                        return VPP_TRUE;
                    break;
                case HQV_CONTROL_TNR:
                    if (stCtrl.tnr.mode != HQV_MODE_OFF)
                        return VPP_TRUE;
                    break;
                case HQV_CONTROL_CNR:
                    if (stCtrl.cnr.mode != HQV_MODE_OFF)
                        return VPP_TRUE;
                    break;
                case HQV_CONTROL_AIE:
                    if (stCtrl.aie.mode != HQV_MODE_OFF)
                        return VPP_TRUE;
                    break;
                case HQV_CONTROL_FRC:
                    if (stCtrl.frc.mode != HQV_FRC_MODE_OFF)
                        return VPP_TRUE;
                    break;
                default:
                    break;
            }
        }
    }

    LOGD("Control not supported for given use case.");

    return VPP_FALSE;
}

/*!
* @brief       Returns the maximum resolution that a usecase can support
*
* @description Returns the maximum resolution that is supported by a usecase.
*              Refer to @sa t_EVppRes for valid return values
*
* @in          pstUc The usecase
*
* @return      Returns a value from @sa t_EVppRes or VPP_RESOLUTION_MAX if
*              no resolutions are supported, or invalid usecase is passed.
*/
uint32_t u32VppUsecase_GetMaxResForUc(t_StVppUsecase *pstUc)
{
    uint32_t i, j;
    uint32_t u32Max = VPP_RESOLUTION_MAX;
    t_StVppUsecaseIpCredits *pstCredit;

    if (!pstUc)
        return VPP_RESOLUTION_MAX;

    for (i = 0; i < VPP_IP_BLOCK_MAX; i++)
    {
        pstCredit = &pstUc->credits[i];
        if (!pstCredit->ip)
            continue;

        for (j = VPP_RESOLUTION_MAX; j; j--)
        {
            uint32_t res = j - 1;
            if (pstCredit->au32Credit[res] != NOT_SUPPORTED &&
                (u32Max == VPP_RESOLUTION_MAX || res <= u32Max))
            {
                u32Max = res;
                break;
            }

            if ((j - 1) == 0)
                u32Max = VPP_RESOLUTION_MAX;
        }

        // Implies that NOT_SUPPORTED is defined for all of the resolutions
        // for a given IP in the usecase composition.
        if (u32Max == VPP_RESOLUTION_MAX)
            break;
    }

    LOGI("max res=%u for uc=%s", u32Max, pstUc->name);
    return u32Max;
}

uint32_t u32VppUsecase_IsMappedPxBufRequired(t_StVppUsecase *pstUc)
{
    uint32_t i;
    uint32_t bReqMap = VPP_FALSE;
    const t_StVppIpBlock *pstIp;

    LOG_ENTER();

    // NULL usecase can refer to bypass, in which case, mapping is not required
    if (!pstUc)
        return VPP_FALSE;

    if (pstUc->bRequireMappedPxBuf)
    {
        LOGI("UC %s requires mapped pixel buffer", pstUc->name);
        return VPP_TRUE;
    }

    for (i = 0; i < VPP_IP_BLOCK_MAX; i++)
    {
        pstIp = pstUc->composition[i].ip;
        if (!pstIp)
            continue;

        if (pstIp->capabilities.bRequireMappedPxBuf)
        {
            LOGI("IP (%u/%s) for UC %s requires mapped pixel buffer",
                 i, pstIp->name, pstUc->name);
            bReqMap = VPP_TRUE;
        }
    }

    LOG_EXIT_RET(bReqMap);
}

uint32_t u32VppUsecase_GetDefaultFps(t_StVppUsecase *pstUc)
{
    if (!pstUc)
        return 0;

    if (!pstUc->u32DefaultInFps)
    {
        LOGE("Default FPS not defined for UC!");
        return VPP_DEFAULT_FRAME_RATE;
    }

    return pstUc->u32DefaultInFps;
}
