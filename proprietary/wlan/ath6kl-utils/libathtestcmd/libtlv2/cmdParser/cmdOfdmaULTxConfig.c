/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "tlv2Inc.h"
#include "cmdOfdmaULTxConfig.h"

void* initOFDMAULTXCONFIGOpParms(A_UINT8 *pParmsCommon, PARM_OFFSET_TBL *pParmsOffset, PARM_DICT *pParmDict)
{
    int i, j; 	//for initializing array parameter

    if (pParmsCommon == NULL) return (NULL);

    i = j = 0;	//assign a number to avoid warning in case i and j are not used

    // Make up ParmOffsetTbl
    resetParmOffsetFields();
    return((void*) pParmsCommon);
}

static OFDMAULTXCONFIG_OP_FUNC OFDMAULTXCONFIGOpFunc = NULL;

TLV2_API void registerOFDMAULTXCONFIGHandler(OFDMAULTXCONFIG_OP_FUNC fp)
{
    OFDMAULTXCONFIGOpFunc = fp;
}

A_BOOL OFDMAULTXCONFIGOp(void *pParms)
{
    if (NULL != OFDMAULTXCONFIGOpFunc) {
        (*OFDMAULTXCONFIGOpFunc)(pParms);
    }
    return(TRUE);
}

void* initOFDMAULTXCONFIGRSPOpParms(A_UINT8 *pParmsCommon, PARM_OFFSET_TBL *pParmsOffset, PARM_DICT *pParmDict)
{
    int i, j; 	//for initializing array parameter
    CMD_OFDMAULTXCONFIGRSP_PARMS  *pOFDMAULTXCONFIGRSPParms = (CMD_OFDMAULTXCONFIGRSP_PARMS *)pParmsCommon;

    if (pParmsCommon == NULL) return (NULL);

    i = j = 0;	//assign a number to avoid warning in case i and j are not used

    // Populate the parm structure with initial values
    pOFDMAULTXCONFIGRSPParms->Pre_Fec_Pad = pParmDict[PARM_PREFECPAD].v.valU8;
    pOFDMAULTXCONFIGRSPParms->Ldpc_Extra_Symbol = pParmDict[PARM_LDPCEXTRASYMBOL].v.valU8;
    pOFDMAULTXCONFIGRSPParms->Pkt_Disambiguity = pParmDict[PARM_PEDISAMBIGUITY].v.valU8;
    pOFDMAULTXCONFIGRSPParms->Pkt_Extension = pParmDict[PARM_PACKETEXT].v.valU8;
    pOFDMAULTXCONFIGRSPParms->Num_He_Ltf = pParmDict[PARM_NUMOFHELTF].v.valU32;

    // Make up ParmOffsetTbl
    resetParmOffsetFields();
    fillParmOffsetTbl((A_UINT32)PARM_PREFECPAD, (A_UINT32)(((A_UINT32)&(pOFDMAULTXCONFIGRSPParms->Pre_Fec_Pad)) - (A_UINT32)pOFDMAULTXCONFIGRSPParms), pParmsOffset);
    fillParmOffsetTbl((A_UINT32)PARM_LDPCEXTRASYMBOL, (A_UINT32)(((A_UINT32)&(pOFDMAULTXCONFIGRSPParms->Ldpc_Extra_Symbol)) - (A_UINT32)pOFDMAULTXCONFIGRSPParms), pParmsOffset);
    fillParmOffsetTbl((A_UINT32)PARM_PEDISAMBIGUITY, (A_UINT32)(((A_UINT32)&(pOFDMAULTXCONFIGRSPParms->Pkt_Disambiguity)) - (A_UINT32)pOFDMAULTXCONFIGRSPParms), pParmsOffset);
    fillParmOffsetTbl((A_UINT32)PARM_PACKETEXT, (A_UINT32)(((A_UINT32)&(pOFDMAULTXCONFIGRSPParms->Pkt_Extension)) - (A_UINT32)pOFDMAULTXCONFIGRSPParms), pParmsOffset);
    fillParmOffsetTbl((A_UINT32)PARM_NUMOFHELTF, (A_UINT32)(((A_UINT32)&(pOFDMAULTXCONFIGRSPParms->Num_He_Ltf)) - (A_UINT32)pOFDMAULTXCONFIGRSPParms), pParmsOffset);
    return((void*) pOFDMAULTXCONFIGRSPParms);
}

static OFDMAULTXCONFIGRSP_OP_FUNC OFDMAULTXCONFIGRSPOpFunc = NULL;

TLV2_API void registerOFDMAULTXCONFIGRSPHandler(OFDMAULTXCONFIGRSP_OP_FUNC fp)
{
    OFDMAULTXCONFIGRSPOpFunc = fp;
}

A_BOOL OFDMAULTXCONFIGRSPOp(void *pParms)
{
    CMD_OFDMAULTXCONFIGRSP_PARMS *pOFDMAULTXCONFIGRSPParms = (CMD_OFDMAULTXCONFIGRSP_PARMS *)pParms;

#if 0 //for debugging, comment out this line, and uncomment the line below
//#ifdef _DEBUG
    int i; 	//for initializing array parameter
    i = 0;	//assign a number to avoid warning in case i is not used

    A_PRINTF("OFDMAULTXCONFIGRSPOp: pre_fec %d\n", pOFDMAULTXCONFIGRSPParms->Pre_Fec_Pad);
    A_PRINTF("OFDMAULTXCONFIGRSPOp: ldpc_ext %d\n", pOFDMAULTXCONFIGRSPParms->Ldpc_Extra_Symbol);
    A_PRINTF("OFDMAULTXCONFIGRSPOp: pkt_dis %d\n", pOFDMAULTXCONFIGRSPParms->Pkt_Disambiguity);
    A_PRINTF("OFDMAULTXCONFIGRSPOp: pkt_ext %d\n", pOFDMAULTXCONFIGRSPParms->Pkt_Extension);
    A_PRINTF("OFDMAULTXCONFIGRSPOp: num_he %d\n", pOFDMAULTXCONFIGRSPParms->Num_He_Ltf);
#endif //_DEBUG

    if (NULL != OFDMAULTXCONFIGRSPOpFunc) {
        (*OFDMAULTXCONFIGRSPOpFunc)(pOFDMAULTXCONFIGRSPParms);
    }
    return(TRUE);
}

