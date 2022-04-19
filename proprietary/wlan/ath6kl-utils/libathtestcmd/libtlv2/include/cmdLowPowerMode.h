/*
 * Copyright (c) 2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef _CMDSETLOWPOWER_H_
#define _CMDSETLOWPOWER_H_

#if defined(__cplusplus) || defined(__cplusplus__)
extern "C" {
#endif

#if defined(WIN32) || defined(WIN64)
#pragma pack (push, 1)
#endif //WIN32 || WIN64

typedef struct setlowpower_parms {
    A_UINT8	lowPowerMode;
    A_UINT8	phyidMask;
    A_UINT32	feature_Mask;
    A_UINT8	pad[3];
} __ATTRIB_PACK CMD_SETLOWPOWER_PARMS;

typedef void (*SETLOWPOWER_OP_FUNC)(void *pParms);

// Exposed functions

void* initSETLOWPOWEROpParms(A_UINT8 *pParmsCommon, PARM_OFFSET_TBL *pParmsOffset, PARM_DICT *pParmDict);
A_BOOL SETLOWPOWEROp(void *pParms);

#if defined(WIN32) || defined(WIN64)
#pragma pack(pop)
#endif //WIN32 || WIN64


#if defined(__cplusplus) || defined(__cplusplus__)
}
#endif

#endif //_CMDSETLOWPOWER_H_

