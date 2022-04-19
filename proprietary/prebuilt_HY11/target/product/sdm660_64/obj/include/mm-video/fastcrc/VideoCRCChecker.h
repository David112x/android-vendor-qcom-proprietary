/*-------------------------------------------------------------------
Copyright (c) 2014-2016 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
--------------------------------------------------------------------*/

#ifndef _VIDEOCRCCHECKER_H
#define _VIDEOCRCCHECKER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <VideoComDef.h>

typedef struct VideoCRCChecker VideoCRCChecker;
struct VideoCRCChecker{

    FILE* fCRCStream;
    uint32 crc;
    uint32 crcPolynomial;
    uint32 m_CRCTable[256];
    uint32 m_fCRCCounter;
    uint32 m_nTotalCRCCount;
    uint32 m_bIsDivxPacked;
    BOOL  m_bCheckFile; //=1, Check each CRC calculated in entire file
    BOOL m_bIsGenerate; //=1, Generates crc file on output root path with inputclipname.fcrc
    VidcStatus (*Configure)(VideoCRCChecker* pCRC, string refCrcAbsolutepath);
    void (*Destroy)(VideoCRCChecker* pCRC);
    void (*SetCRCCheckFileMode)(VideoCRCChecker* pCRC,BOOL bFileMode);
    uint32 (*GetPendingCRCCount)(VideoCRCChecker* pCRC); //Returns still how many pending CRC's left
    VidcStatus(*CheckCRCofYUVframe)(VideoCRCChecker* pCRC, uint8 * pData, VideoResolution *pRes,
        BOOL bInterleaved, BOOL* pbCRCResult);
    void (*SkipFrameCRCCheck)(VideoCRCChecker* pCRC);
    uint32 (*TotalCRCCount)(VideoCRCChecker* pCRC);
    uint32 prevCrc;

};

VidcStatus VideoCRCCheckerInit(VideoCRCChecker* pCRC, BOOL isGenerate);

#ifdef __cplusplus
}
#endif

#endif //_VIDEOCRCCHECKER_H
