/*-------------------------------------------------------------------
Copyright (c) 2014-2016 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
--------------------------------------------------------------------*/

#ifndef _VIDEOCOMDEF_H
#define _VIDEOCOMDEF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BOOL uint32
#define TRUE 1
#define FALSE 0

#define MAX_TAG_LEN 100
#define MAX_STR_LEN 300
#define MAX_FRAME_NUMBER 99999

#define FCLOSE(file)  if(file){ fclose(file); (file) = NULL; }
#define FREE(ptr)     if(ptr){  free(ptr); (ptr) = NULL; }

#define SPRINTF(fmt,...) snprintf(fmt,sizeof(fmt),##__VA_ARGS__)
#define SNPRINTF(fmt,size,...) snprintf(fmt,size,##__VA_ARGS__)
#define STRCPY(dest,src) strlcpy(dest,src,sizeof(dest))
#define STRLCPY(dest,src,size) ((dest)?strlcpy(dest,src,size):0)
#define STRCAT(dest,src) strlcat(dest,src,sizeof(dest))
#define STRLCAT(dest,src,size) strlcat(dest,src,size)
#define STRCMP(str1,str2) ((str1 && str2)? strcmp(str1,str2) : 1)

typedef unsigned char uint8;
typedef unsigned int uint32;
typedef char* string;

typedef enum{
    VidcStatusSuccess                   = 0,
    VidcStatusEOSExit                   = 1,
    VidcStatusAllocError                = 2,
    VidcStatusBadParamError             = 3,
    VidcStatusXmlCfgError               = 4,
    VidcStatusFileReadError             = 5,
    VidcStatusBufferOverFlowError       = 6,
    VidcStatusFileZeroLenError          = 7,
    VidcStatusDisplyReorderError        = 8,
    VidcStatusMaxSeekError              = 9,
    VidcStatusOutPutThreadError         = 10,
    VidcStatusCaptureThreadError        = 11,
    VidcStatusOperationError            = 12,
    VidcStatusNoIDRFound                = 13,
    VidcStatusDisplyReorderDupCRCError  = 14,
    VidcStatusMBIMismatchError          = 15,
    VidcStatusUnknownError = 0xFFFFF
}VidcStatus;

typedef struct VideoResolution
{
    uint32 nWidth;
    uint32 nHeight;
    uint32 nCropWidth;
    uint32 nCropHeight;
    uint32 nScanlines;
    uint32 nStride;
    uint32 nStartX; // left start x coordinate
    uint32 nStartY; // top start y coordinate
    uint32 nColorFmt;
}VideoResolution;

#ifdef __cplusplus
}
#endif

#endif //_VIDEOCOMDEF_H
