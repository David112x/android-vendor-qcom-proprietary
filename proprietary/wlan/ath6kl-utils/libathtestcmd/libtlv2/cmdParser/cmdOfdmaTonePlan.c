/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "tlv2Inc.h"
#include "cmdOfdmaTonePlan.h"

void* initOFDMATONEPLANOpParms(A_UINT8 *pParmsCommon, PARM_OFFSET_TBL *pParmsOffset, PARM_DICT *pParmDict)
{
    int i, j; 	//for initializing array parameter
    CMD_OFDMATONEPLAN_PARMS  *pOFDMATONEPLANParms = (CMD_OFDMATONEPLAN_PARMS *)pParmsCommon;

    if (pParmsCommon == NULL) return (NULL);

    i = j = 0;	//assign a number to avoid warning in case i and j are not used

    // Populate the parm structure with initial values
    pOFDMATONEPLANParms->phyId = pParmDict[PARM_PHYID].v.valU8;
    memset(pOFDMATONEPLANParms->ofdmaTonePlan, 0, sizeof(pOFDMATONEPLANParms->ofdmaTonePlan));

    // Make up ParmOffsetTbl
    resetParmOffsetFields();
    fillParmOffsetTbl((A_UINT32)PARM_PHYID, (A_UINT32)(((A_UINT32)&(pOFDMATONEPLANParms->phyId)) - (A_UINT32)pOFDMATONEPLANParms), pParmsOffset);
    fillParmOffsetTbl((A_UINT32)PARM_OFDMATONEPLAN, (A_UINT32)(((A_UINT32)&(pOFDMATONEPLANParms->ofdmaTonePlan)) - (A_UINT32)pOFDMATONEPLANParms), pParmsOffset);
    return((void*) pOFDMATONEPLANParms);
}

static OFDMATONEPLAN_OP_FUNC OFDMATONEPLANOpFunc = NULL;

TLV2_API void registerOFDMATONEPLANHandler(OFDMATONEPLAN_OP_FUNC fp)
{
    OFDMATONEPLANOpFunc = fp;
}

A_BOOL OFDMATONEPLANOp(void *pParms)
{
    CMD_OFDMATONEPLAN_PARMS *pOFDMATONEPLANParms = (CMD_OFDMATONEPLAN_PARMS *)pParms;

#if 0 //for debugging, comment out this line, and uncomment the line below
//#ifdef _DEBUG
    int i; 	//for initializing array parameter
    i = 0;	//assign a number to avoid warning in case i is not used

    A_PRINTF("OFDMATONEPLANOp: phyId %u\n", pOFDMATONEPLANParms->phyId);
    for (i = 0; i < 8 ; i++) // can be modified to print up to 1054 entries
    {
        A_PRINTF("OFDMATONEPLANOp: ofdmaTonePlan %u\n", pOFDMATONEPLANParms->ofdmaTonePlan[i]);
    }
#endif //_DEBUG

    if (NULL != OFDMATONEPLANOpFunc) {
        (*OFDMATONEPLANOpFunc)(pOFDMATONEPLANParms);
    }
    return(TRUE);
}

void* initOFDMATONEPLANRSPOpParms(A_UINT8 *pParmsCommon, PARM_OFFSET_TBL *pParmsOffset, PARM_DICT *pParmDict)
{
    int i, j; 	//for initializing array parameter
    CMD_OFDMATONEPLANRSP_PARMS  *pOFDMATONEPLANRSPParms = (CMD_OFDMATONEPLANRSP_PARMS *)pParmsCommon;

    if (pParmsCommon == NULL) return (NULL);

    i = j = 0;	//assign a number to avoid warning in case i and j are not used

    // Populate the parm structure with initial values
    pOFDMATONEPLANRSPParms->status = pParmDict[PARM_STATUS].v.valU8;
    pOFDMATONEPLANRSPParms->numRuSupported = pParmDict[PARM_NUMRUSUPPORTED].v.valU8;

    // Make up ParmOffsetTbl
    resetParmOffsetFields();
    fillParmOffsetTbl((A_UINT32)PARM_STATUS, (A_UINT32)(((A_UINT32)&(pOFDMATONEPLANRSPParms->status)) - (A_UINT32)pOFDMATONEPLANRSPParms), pParmsOffset);
    fillParmOffsetTbl((A_UINT32)PARM_NUMRUSUPPORTED, (A_UINT32)(((A_UINT32)&(pOFDMATONEPLANRSPParms->numRuSupported)) - (A_UINT32)pOFDMATONEPLANRSPParms), pParmsOffset);
    return((void*) pOFDMATONEPLANRSPParms);
}

static OFDMATONEPLANRSP_OP_FUNC OFDMATONEPLANRSPOpFunc = NULL;

TLV2_API void registerOFDMATONEPLANRSPHandler(OFDMATONEPLANRSP_OP_FUNC fp)
{
    OFDMATONEPLANRSPOpFunc = fp;
}

A_BOOL OFDMATONEPLANRSPOp(void *pParms)
{
    CMD_OFDMATONEPLANRSP_PARMS *pOFDMATONEPLANRSPParms = (CMD_OFDMATONEPLANRSP_PARMS *)pParms;

#if 0 //for debugging, comment out this line, and uncomment the line below
//#ifdef _DEBUG
    int i; 	//for initializing array parameter
    i = 0;	//assign a number to avoid warning in case i is not used

    A_PRINTF("OFDMATONEPLANRSPOp: status %u\n", pOFDMATONEPLANRSPParms->status);
    A_PRINTF("OFDMATONEPLANRSPOp: numRuSupported %u\n", pOFDMATONEPLANRSPParms->numRuSupported);
#endif //_DEBUG

    if (NULL != OFDMATONEPLANRSPOpFunc) {
        (*OFDMATONEPLANRSPOpFunc)(pOFDMATONEPLANRSPParms);
    }
    return(TRUE);
}
