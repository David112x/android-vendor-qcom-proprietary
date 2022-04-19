/*
 * Copyright (c) 2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "tlv2Inc.h"
#include "cmdLowPowerMode.h"

void* initSETLOWPOWEROpParms(A_UINT8 *pParmsCommon, PARM_OFFSET_TBL *pParmsOffset, PARM_DICT *pParmDict)
{
    int i; 	//for initializing array parameter
    CMD_SETLOWPOWER_PARMS  *pSETLOWPOWERParms = (CMD_SETLOWPOWER_PARMS *)pParmsCommon;

    if (pParmsCommon == NULL) return (NULL);

    i = 0;	//assign a number to avoid warning in case i is not used

    // Populate the parm structure with initial values
    pSETLOWPOWERParms->lowPowerMode = pParmDict[PARM_LOWPOWERMODE].v.valU8;
    pSETLOWPOWERParms->phyidMask = pParmDict[PARM_PHYIDMASK].v.valU8;
    pSETLOWPOWERParms->feature_Mask = pParmDict[PARM_PHYIDMASK].v.valU32;

    // Make up ParmOffsetTbl
    resetParmOffsetFields();
    fillParmOffsetTbl((A_UINT32)PARM_LOWPOWERMODE, (A_UINT32)(((A_UINT8 *)&(pSETLOWPOWERParms->lowPowerMode)) - (A_UINT8 *)pSETLOWPOWERParms), pParmsOffset);
    fillParmOffsetTbl((A_UINT32)PARM_PHYIDMASK, (A_UINT32)(((A_UINT8 *)&(pSETLOWPOWERParms->phyidMask)) - (A_UINT8 *)pSETLOWPOWERParms), pParmsOffset);
    fillParmOffsetTbl((A_UINT32)PARM_LOWPOWERFEATUREMASK, (A_UINT32)(((A_UINT8 *)&(pSETLOWPOWERParms->feature_Mask)) - (A_UINT8 *)pSETLOWPOWERParms), pParmsOffset);
    return((void*) pSETLOWPOWERParms);
}

static SETLOWPOWER_OP_FUNC SETLOWPOWEROpFunc = NULL;

TLV2_API void registerSETLOWPOWERHandler(SETLOWPOWER_OP_FUNC fp)
{
    SETLOWPOWEROpFunc = fp;
}

A_BOOL SETLOWPOWEROp(void *pParms)
{
    CMD_SETLOWPOWER_PARMS *pSETLOWPOWERParms = (CMD_SETLOWPOWER_PARMS *)pParms;

#if 0 //for debugging, comment out this line, and uncomment the line below
//#ifdef _DEBUG
    int i; 	//for initializing array parameter
    i = 0;	//assign a number to avoid warning in case i is not used

    A_PRINTF("SETSETLOWPOWEROp: lowPowerMode %u\n", pSETLOWPOWERParms->lowPowerMode);
    A_PRINTF("SETSETLOWPOWEROp: phyidMask %u\n", pSETLOWPOWERParms->phyidMask);
    A_PRINTF("SETSETLOWPOWEROp: feature_Mask %u\n", pSETLOWPOWERParms->feature_Mask);
#endif //_DEBUG

    if (NULL != SETLOWPOWEROpFunc) {
        (*SETLOWPOWEROpFunc)(pSETLOWPOWERParms);
    }
    return(TRUE);
}

