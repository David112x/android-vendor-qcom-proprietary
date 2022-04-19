/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef _CMDOFDMATONEPLAN_H_
#define _CMDOFDMATONEPLAN_H_

#if defined(__cplusplus) || defined(__cplusplus__)
extern "C" {
#endif

#if defined(WIN32) || defined(WIN64)
#pragma pack (push, 1)
#endif //WIN32 || WIN64

#define TLV2_OFDMA_TONE_PLAN_SIZE    1054
typedef struct ofdmatoneplan_parms {
    A_UINT8	phyId;
    A_UINT8	ofdmaTonePlan[TLV2_OFDMA_TONE_PLAN_SIZE];
    A_UINT8	pad[1];
} __ATTRIB_PACK CMD_OFDMATONEPLAN_PARMS;

typedef struct ofdmatoneplanrsp_parms {
    A_UINT8	status;
    A_UINT8	numRuSupported;
    A_UINT8	pad[2];
} __ATTRIB_PACK CMD_OFDMATONEPLANRSP_PARMS;

typedef void (*OFDMATONEPLAN_OP_FUNC)(void *pParms);
typedef void (*OFDMATONEPLANRSP_OP_FUNC)(void *pParms);

// Exposed functions

void* initOFDMATONEPLANOpParms(A_UINT8 *pParmsCommon, PARM_OFFSET_TBL *pParmsOffset, PARM_DICT *pParmDict);
A_BOOL OFDMATONEPLANOp(void *pParms);

void* initOFDMATONEPLANRSPOpParms(A_UINT8 *pParmsCommon, PARM_OFFSET_TBL *pParmsOffset, PARM_DICT *pParmDict);
A_BOOL OFDMATONEPLANRSPOp(void *pParms);

#if defined(WIN32) || defined(WIN64)
#pragma pack(pop)
#endif //WIN32 || WIN64


#if defined(__cplusplus) || defined(__cplusplus__)
}
#endif

#endif //_CMDOFDMATONEPLAN_H_
