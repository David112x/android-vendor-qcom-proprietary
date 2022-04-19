/*-------------------------------------------------------------------
Copyright (c) 2013-2017 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
--------------------------------------------------------------------*/

#include <stdlib.h>
#include <math.h>
#include <media/hardware/HardwareAPI.h>
#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

#include "OMX_Component.h"
#include "OMX_IndexExt.h"
#include "OMX_VideoExt.h"
#include "OMX_QCOMExtns.h"
#include "QComOMXMetadata.h"
#include "venc/inc/omx_video_common.h"
#include "vtest_Debug.h"
#include "vtest_Signal.h"
#include "vtest_SignalQueue.h"
#include "vtest_Thread.h"
#include "vtest_Sleeper.h"
#include "vtest_Encoder.h"

#undef LOG_TAG
#define LOG_TAG "VTEST_ENCODER"

#define Log2(number, power)  { OMX_U32 temp = number; power = 0; while( (0 == (temp & 0x1)) &&  power < 16) { temp >>=0x1; power++; } }
#define FractionToQ16(q,num,den) { OMX_U32 power; Log2(den,power); q = num << (16 - power); }
#define ALIGN(x, to_align) ((((unsigned long) x) + (to_align - 1)) & ~(to_align - 1))

#ifdef MASTER_SIDE_CP
#define SECURE_ALIGN SZ_4K
#else
#define SECURE_ALIGN SZ_1M
#endif

using android::encoder_media_buffer_type;
using android::StoreMetaDataInBuffersParams;

namespace vtest {

struct CmdType {
    OMX_EVENTTYPE   eEvent;
    OMX_COMMANDTYPE eCmd;
    OMX_U32         sCmdData;
    OMX_ERRORTYPE   eResult;
};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
Encoder::Encoder(CodecConfigType *pConfig)
    : ISource(),
      m_bInputEOSReached(OMX_FALSE),
      m_pSignalQueue(new SignalQueue(32, sizeof(CmdType))),
      m_bSecureSession(OMX_FALSE),
      m_eCodec(pConfig->eCodec),
      m_hEncoder(NULL),
      m_eState(OMX_StateLoaded),
      m_eStatePending(OMX_StateLoaded),
      m_nInputBufferCount(0),
      m_nOutputBufferCount(0),
      m_nInputBufferSize(0),
      m_nOutputBufferSize(0),
      m_LTRId(0),
      m_nDynamicIndexCount(0),
      m_bPortReconfig(OMX_FALSE),
      m_pFreeBufferQueue(new SignalQueue(32, sizeof(BufferInfo*))),
      m_bMetaMode(OMX_FALSE),
      m_eMetaBufferType(kMetadataBufferTypeNativeHandleSource),
      m_nFrameWidth(0),
      m_nFrameHeight(0),
      m_nColorFormat(0) {

    OMX_ERRORTYPE result = OMX_ErrorNone;
    OMX_STRING role = (OMX_STRING)"";
    static OMX_CALLBACKTYPE callbacks =
    { EventCallback, EmptyDoneCallback, FillDoneCallback };

    snprintf(m_pName, MAX_NAME_LENGTH, "%s", "Encoder");
    VTEST_MSG_HIGH("%s created...", m_pName);

    GetComponentRole(pConfig->bSecureSession, pConfig->eCodec, &role);
    result = OMX_GetHandle(&m_hEncoder, role, this, &callbacks);
    if (result != OMX_ErrorNone || m_hEncoder == NULL) {
        VTEST_MSG_ERROR("Error getting component handle, rv: %d", result);
        if (pConfig->eCodec == OMX_VIDEO_CodingHEVC) {
            VTEST_MSG_ERROR("Trying again with qti.hevc role");
            role = (OMX_STRING)"OMX.qti.video.encoder.hevc";
            result = OMX_GetHandle(&m_hEncoder, role, this, &callbacks);
            if (result != OMX_ErrorNone || m_hEncoder == NULL) {
                VTEST_MSG_ERROR("Error getting component handle, rv: %d", result);
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
Encoder::~Encoder() {

    OMX_ERRORTYPE result = OMX_ErrorNone;
    VTEST_MSG_LOW("start");

    //Move this a to an intermediate function between stop and destructor
    result = SetState(OMX_StateLoaded, OMX_TRUE);
    FAILED(result, "Could not move to OMX_StateLoaded!");

    Mutex::Autolock autoLock(m_pLock);

    if (m_hEncoder != NULL) {
        OMX_FreeHandle(m_hEncoder);
        m_hEncoder = NULL;
    }
    if (m_pSignalQueue != NULL) {
        delete m_pSignalQueue;
        m_pSignalQueue = NULL;
    }
    if (m_pFreeBufferQueue != NULL) {
        delete m_pFreeBufferQueue;
        m_pFreeBufferQueue = NULL;
    }
    VTEST_MSG_LOW("done");
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
PortBufferCapability Encoder::GetBufferRequirements(OMX_U32 ePortIndex) {

    PortBufferCapability sBufCap;

    Mutex::Autolock autoLock(m_pLock);
    memset(&sBufCap, 0, sizeof(PortBufferCapability));

    // make sure we have been configured here
    if (m_nInputBufferCount <= 0 || m_nOutputBufferSize <= 0) {
        VTEST_MSG_ERROR("Error: must call configure to get valid buf reqs");
        return sBufCap;
    }

    if (ePortIndex == PORT_INDEX_IN) {
        if (m_bMetaMode) {
            sBufCap.bAllocateBuffer = OMX_FALSE;
            sBufCap.bUseBuffer = OMX_TRUE;
        } else {
            sBufCap.bAllocateBuffer = OMX_TRUE;
            sBufCap.bUseBuffer = OMX_FALSE;
        }
        sBufCap.pSource = this;
        sBufCap.ePortIndex = ePortIndex;
        sBufCap.nWidth = m_nFrameWidth;
        sBufCap.nHeight = m_nFrameHeight;
        sBufCap.nMinBufferSize = m_nInputBufferSize;
        sBufCap.nMinBufferCount = m_nInputBufferCount;
        sBufCap.nBufferUsage = 0;
        sBufCap.nExtraBufferCount = 0;
    } else if (ePortIndex == PORT_INDEX_OUT) {
        sBufCap.bAllocateBuffer = OMX_TRUE;
        sBufCap.bUseBuffer = OMX_FALSE;
        sBufCap.pSource = this;
        sBufCap.ePortIndex = ePortIndex;
        sBufCap.nWidth = m_sConfig.nOutputFrameWidth;
        sBufCap.nHeight = m_sConfig.nOutputFrameHeight;
        sBufCap.nMinBufferSize = m_nOutputBufferSize;
        sBufCap.nMinBufferCount = m_nOutputBufferCount;
        sBufCap.nBufferUsage = 0;
        sBufCap.nExtraBufferCount = 0;
    } else {
        VTEST_MSG_ERROR("Error: invalid port selection");
    }
    return sBufCap;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Encoder::Start() {

    OMX_ERRORTYPE result = OMX_ErrorNone;

    if (!mRoiQpFile) {
        mRoiQpFile = fopen(m_sConfig.cRoiQpFileName, "r");
    }
    if (!mRoiQpFile)
        VTEST_MSG_ERROR("Unable to open ROI Qp file: %s", m_sConfig.cRoiQpFileName);

    if (m_eState != OMX_StateExecuting) {
        result = SetState(OMX_StateExecuting, OMX_TRUE);
        FAILED1(result, "Could not move to executing state!");
    }
    return ISource::Start();
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Encoder::Stop() {

    OMX_ERRORTYPE result = OMX_ErrorNone;
    {
        Mutex::Autolock autoLock(m_pLock);
        m_bThreadStop = OMX_TRUE;
        if ((m_eState == OMX_StateIdle)
                || (m_eStatePending == OMX_StateIdle)) {
            return result;
        }
    }

    result = SetState(OMX_StateIdle, OMX_FALSE);
    FAILED(result, "Could not move to OMX_StateIdle!");

    /* We should not acquire a lock across sendcommand because of a possible
     * deadlock scenario */
    Flush(OMX_ALL);
    OMX_SendCommand(m_hEncoder, OMX_CommandPortDisable, PORT_INDEX_OUT, 0);
    OMX_SendCommand(m_hEncoder, OMX_CommandPortDisable, PORT_INDEX_IN, 0);
    if (mRoiQpFile) {
        fclose(mRoiQpFile);
        mRoiQpFile = NULL;
    }
    if (mRoiQpBuf) {
        free(mRoiQpBuf);
        mRoiQpBuf = NULL;
    }
    return ISource::Stop();
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Encoder::Configure(CodecConfigType *pConfig,
        BufferManager *pBufManager, ISource *pSource, ISource *pSink) {

    OMX_ERRORTYPE result = OMX_ErrorNone;
    char value[PROPERTY_VALUE_MAX] = { 0 };
    OMX_VIDEO_PARAM_PORTFORMATTYPE videoPortFmt;
    OMX_INIT_STRUCT(&videoPortFmt, OMX_VIDEO_PARAM_PORTFORMATTYPE);

    if (!pConfig) {
       VTEST_MSG_ERROR("%s - invalid input", __FUNCTION__);
       return OMX_ErrorBadParameter;
    }

    if (!m_hEncoder) {
        VTEST_MSG_ERROR("handle is null");
        return OMX_ErrorBadParameter;
    }

    result = ISource::Configure(pConfig, pBufManager, pSource, pSink);
    if (result != OMX_ErrorNone) {
        VTEST_MSG_ERROR("Encoder configure failed");
        return result;
    }

    Mutex::Autolock autoLock(m_pLock);

    VTEST_MSG_HIGH("");

    m_sConfig = *pConfig;
    m_bSecureSession = pConfig->bSecureSession;
    m_bMetaMode = pConfig->bMetaMode;
    m_eMetaBufferType = pConfig->eMetaBufferType;
    m_nFrameWidth = pConfig->nFrameWidth;
    m_nFrameHeight = pConfig->nFrameHeight;
    m_nColorFormat = pConfig->nInputColorFormat;

    if (!CheckColorFormatSupported((OMX_COLOR_FORMATTYPE)m_nColorFormat, &videoPortFmt)) {
        VTEST_MSG_ERROR("Encoder: No Venus specific color format[0x%x] supported\n", m_nColorFormat);
        return OMX_ErrorUnsupportedSetting;
    }
    //////////////////////////////////////////
    // codec specific config
    //////////////////////////////////////////
    if (m_eCodec == OMX_VIDEO_CodingMPEG4) {

        OMX_VIDEO_PARAM_MPEG4TYPE mp4;
        OMX_INIT_STRUCT(&mp4, OMX_VIDEO_PARAM_MPEG4TYPE);
        mp4.nPortIndex = (OMX_U32)PORT_INDEX_OUT;
        result = OMX_GetParameter(m_hEncoder,
                OMX_IndexParamVideoMpeg4, (OMX_PTR)& mp4);
        if (result != OMX_ErrorNone) {
            return result;
        }

        mp4.nTimeIncRes = pConfig->nTimeIncRes;
        mp4.nBFrames = pConfig->nBFrames;
        mp4.nPFrames = pConfig->nPFrames;
        if ((pConfig->eCodecProfile == MPEG4ProfileSimple) ||
                (pConfig->eCodecProfile == MPEG4ProfileAdvancedSimple)) {
            switch (pConfig->eCodecProfile) {
                case MPEG4ProfileSimple:
                    mp4.eProfile = OMX_VIDEO_MPEG4ProfileSimple;
                    break;
                case MPEG4ProfileAdvancedSimple:
                    mp4.eProfile = OMX_VIDEO_MPEG4ProfileAdvancedSimple;
                    break;
                default:
                    mp4.eProfile = OMX_VIDEO_MPEG4ProfileSimple;
                    break;
            }
        } else {
            VTEST_MSG_MEDIUM("Invalid MPEG4 Profile set defaulting to Simple Profile");
            mp4.eProfile = OMX_VIDEO_MPEG4ProfileSimple;
        }

        switch (pConfig->eCodecLevel) {
            case MPEG4Level0:
                mp4.eLevel = OMX_VIDEO_MPEG4Level0;
                break;
            case MPEG4Level0b:
                mp4.eLevel = OMX_VIDEO_MPEG4Level0b;
                break;
            case MPEG4Level1:
                mp4.eLevel = OMX_VIDEO_MPEG4Level1;
                break;
            case MPEG4Level2:
                mp4.eLevel = OMX_VIDEO_MPEG4Level2;
                break;
            case MPEG4Level3:
                mp4.eLevel = OMX_VIDEO_MPEG4Level3;
                break;
            case MPEG4Level3b:
                mp4.eLevel = OMX_VIDEO_MPEG4Level3;
                break;
            case MPEG4Level4:
                mp4.eLevel = OMX_VIDEO_MPEG4Level4;
                break;
            case MPEG4Level4a:
                mp4.eLevel = OMX_VIDEO_MPEG4Level4a;
                break;
            case MPEG4Level5:
                mp4.eLevel = OMX_VIDEO_MPEG4Level5;
                break;
            default:
                VTEST_MSG_ERROR("Invalid MPEG4 level set, resetting to default");
                mp4.eLevel = OMX_VIDEO_MPEG4LevelMax;
                break;
        }

        mp4.nSliceHeaderSpacing = 0;
        mp4.bReversibleVLC = OMX_FALSE;
        mp4.bACPred = OMX_TRUE;
        mp4.bGov = OMX_FALSE;
        mp4.bSVH = pConfig->bEnableShortHeader;
        mp4.nHeaderExtension = 0;

        OMX_INIT_STRUCT_SIZE(&mp4, OMX_VIDEO_PARAM_MPEG4TYPE);
        result = OMX_SetParameter(m_hEncoder,
                OMX_IndexParamVideoMpeg4, (OMX_PTR)&mp4);

        if (result == OMX_ErrorNone) {
            result = SetBitrate(pConfig->eControlRate, pConfig->nBitrate);
            FAILED1(result, "Failed : Setting bitrate")
        }

    } else if (m_eCodec == OMX_VIDEO_CodingH263) {

        OMX_VIDEO_PARAM_H263TYPE h263;
        OMX_INIT_STRUCT(&h263, OMX_VIDEO_PARAM_H263TYPE);
        h263.nPortIndex = (OMX_U32)PORT_INDEX_OUT;
        result = OMX_GetParameter(m_hEncoder,
                OMX_IndexParamVideoH263, (OMX_PTR)&h263);
        if (result != OMX_ErrorNone) {
            return result;
        }

        h263.nPFrames = pConfig->nPFrames;
        h263.nBFrames = 0;
        if (pConfig->eCodecProfile == H263ProfileBaseline) {
            h263.eProfile = OMX_VIDEO_H263ProfileBaseline;
        } else {
            VTEST_MSG_MEDIUM("Invalid H263 Profile set defaulting to Baseline Profile");
            h263.eProfile = (OMX_VIDEO_H263PROFILETYPE)OMX_VIDEO_H263ProfileBaseline;
        }
        h263.eLevel = OMX_VIDEO_H263Level10;
        h263.bPLUSPTYPEAllowed = OMX_FALSE;
        h263.nAllowedPictureTypes = 2;
        h263.bForceRoundingTypeToZero = OMX_TRUE;
        h263.nPictureHeaderRepetition = 0;
        h263.nGOBHeaderInterval = 1;

        OMX_INIT_STRUCT_SIZE(&h263, OMX_VIDEO_PARAM_H263TYPE);
        result = OMX_SetParameter(m_hEncoder,
                OMX_IndexParamVideoH263, (OMX_PTR)&h263);

        if (result == OMX_ErrorNone) {
            result = SetBitrate(pConfig->eControlRate, pConfig->nBitrate);
            FAILED1(result, "Failed : Setting bitrate")
        }

    } else if (m_eCodec == OMX_VIDEO_CodingAVC) {

        OMX_VIDEO_PARAM_AVCTYPE avc;
    OMX_INIT_STRUCT(&avc, OMX_VIDEO_PARAM_AVCTYPE);

        OMX_VIDEO_CONFIG_AVCINTRAPERIOD idr_period;
    OMX_INIT_STRUCT(&idr_period, OMX_VIDEO_CONFIG_AVCINTRAPERIOD);

        avc.nPortIndex = (OMX_U32)PORT_INDEX_OUT;

        result = OMX_GetParameter(m_hEncoder,
                OMX_IndexParamVideoAvc, (OMX_PTR)&avc);
        if (result != OMX_ErrorNone) {
            return result;
        }

        avc.nPFrames = pConfig->nPFrames;
        avc.nBFrames = pConfig->nBFrames;
        switch (pConfig->eCodecProfile) {
            case AVCProfileBaseline:
                avc.eProfile = OMX_VIDEO_AVCProfileBaseline;
                break;
            case AVCProfileHigh:
                avc.eProfile = OMX_VIDEO_AVCProfileHigh;
                break;
            case AVCProfileMain:
                avc.eProfile = OMX_VIDEO_AVCProfileMain;
                break;
            case AVCProfileConstrainedBaseline:
                avc.eProfile = (OMX_VIDEO_AVCPROFILETYPE)(QOMX_VIDEO_AVCProfileConstrainedBaseline);
                break;
            case AVCProfileConstrainedHigh:
                avc.eProfile = (OMX_VIDEO_AVCPROFILETYPE)(QOMX_VIDEO_AVCProfileConstrainedHigh);
                break;

            default:
                VTEST_MSG_ERROR("Invalid H264 Profile set defaulting to Baseline Profile");
                avc.eProfile = OMX_VIDEO_AVCProfileBaseline;
                avc.nBFrames = 0;
                break;
        }

        OMX_BOOL slice_delivery_mode = OMX_FALSE;
        slice_delivery_mode = pConfig->bEnableSliceDeliveryMode;
        if (slice_delivery_mode) {
            avc.nBFrames = 0;
        }

        switch (pConfig->eCodecLevel) {
            case AVCLevel1:
                avc.eLevel = OMX_VIDEO_AVCLevel1;
                break;
            case AVCLevel1b:
                avc.eLevel = OMX_VIDEO_AVCLevel1b;
                break;
            case AVCLevel11:
                avc.eLevel = OMX_VIDEO_AVCLevel11;
                break;
            case AVCLevel12:
                avc.eLevel = OMX_VIDEO_AVCLevel12;
                break;
            case AVCLevel13:
                avc.eLevel = OMX_VIDEO_AVCLevel13;
                break;
            case AVCLevel2:
                avc.eLevel = OMX_VIDEO_AVCLevel2;
                break;
            case AVCLevel21:
                avc.eLevel = OMX_VIDEO_AVCLevel21;
                break;
            case AVCLevel22:
                avc.eLevel = OMX_VIDEO_AVCLevel22;
                break;
            case AVCLevel3:
                avc.eLevel = OMX_VIDEO_AVCLevel3;
                break;
            case AVCLevel31:
                avc.eLevel = OMX_VIDEO_AVCLevel31;
                break;
            case AVCLevel32:
                avc.eLevel = OMX_VIDEO_AVCLevel32;
                break;
            case AVCLevel4:
                avc.eLevel = OMX_VIDEO_AVCLevel4;
                break;
            case AVCLevel41:
                avc.eLevel = OMX_VIDEO_AVCLevel41;
                break;
            case AVCLevel42:
                avc.eLevel = OMX_VIDEO_AVCLevel42;
                break;
            case AVCLevel5:
                avc.eLevel = OMX_VIDEO_AVCLevel5;
                break;
            case AVCLevel51:
                avc.eLevel = OMX_VIDEO_AVCLevel51;
                break;
            case AVCLevel52:
                avc.eLevel = OMX_VIDEO_AVCLevel52;
                break;
            default:
                VTEST_MSG_ERROR("Invalid H264 level set, resetting to default");
                avc.eLevel = OMX_VIDEO_AVCLevelMax;
                break;
        }

        VTEST_MSG_MEDIUM("AVC level = %u", avc.eLevel);

        avc.bUseHadamard = OMX_FALSE;
        avc.nRefFrames = 1;
        avc.nRefIdx10ActiveMinus1 = 1;
        avc.nRefIdx11ActiveMinus1 = 0;
        avc.bEnableUEP = OMX_FALSE;
        avc.bEnableFMO = OMX_FALSE;
        avc.bEnableASO = OMX_FALSE;
        avc.bEnableRS = OMX_FALSE;
        avc.nAllowedPictureTypes = 2;
        avc.bFrameMBsOnly = OMX_FALSE;
        avc.bMBAFF = OMX_FALSE;
        avc.bWeightedPPrediction = OMX_FALSE;
        avc.nWeightedBipredicitonMode = 0;
        avc.bconstIpred = OMX_FALSE;
        avc.bDirect8x8Inference = OMX_FALSE;
        avc.bDirectSpatialTemporal = OMX_FALSE;

        if (avc.eProfile == OMX_VIDEO_AVCProfileBaseline) {
            avc.bEntropyCodingCABAC = OMX_FALSE;
            avc.nCabacInitIdc = 0;
        } else {
            if (pConfig->bCABAC) {
                avc.bEntropyCodingCABAC = OMX_TRUE;
                avc.nCabacInitIdc = 0;
            } else {
                avc.bEntropyCodingCABAC = OMX_FALSE;
                avc.nCabacInitIdc = 0;
            }
        }

        switch (pConfig->nDeblocker) {
            case 0:
                avc.eLoopFilterMode = OMX_VIDEO_AVCLoopFilterDisable;
                break;
            case 1:
                avc.eLoopFilterMode = OMX_VIDEO_AVCLoopFilterEnable;
                break;
            case 2:
                avc.eLoopFilterMode = OMX_VIDEO_AVCLoopFilterDisableSliceBoundary;
                break;
            default:
                avc.eLoopFilterMode = OMX_VIDEO_AVCLoopFilterEnable;
                break;
        }
        OMX_INIT_STRUCT_SIZE(&avc, OMX_VIDEO_PARAM_AVCTYPE);
        result = OMX_SetParameter(m_hEncoder,
                OMX_IndexParamVideoAvc, (OMX_PTR)&avc);

        if (result != OMX_ErrorNone) {
            return result;
        }
        ///////////////////////////////////////
        // set SPS/PPS insertion for IDR frames
        ///////////////////////////////////////
        android::PrependSPSPPSToIDRFramesParams param;
        memset(&param, 0, sizeof(android::PrependSPSPPSToIDRFramesParams));
        param.nSize = sizeof(android::PrependSPSPPSToIDRFramesParams);
        param.bEnable = pConfig->bInsertInbandVideoHeaders;
        VTEST_MSG_MEDIUM("Set SPS/PPS headers: %d", param.bEnable);
        result = OMX_SetParameter(m_hEncoder,
                (OMX_INDEXTYPE)OMX_QcomIndexParamSequenceHeaderWithIDR,
                (OMX_PTR)&param);

        if (result != OMX_ErrorNone) {
            return result;
        }
        ///////////////////////////////////////
        // set AU delimiters for video stream
        ///////////////////////////////////////
        OMX_QCOM_VIDEO_CONFIG_AUD param_aud;
        memset(&param_aud, 0, sizeof(OMX_QCOM_VIDEO_CONFIG_AUD));
        param_aud.nSize = sizeof(OMX_QCOM_VIDEO_CONFIG_AUD);
        param_aud.bEnable = pConfig->bInsertAUDelimiters;
        VTEST_MSG_MEDIUM("Set AU Delimiters: %d", param_aud.bEnable);
        result = OMX_SetParameter(m_hEncoder,
                (OMX_INDEXTYPE)OMX_QcomIndexParamAUDelimiter,
                (OMX_PTR)&param_aud);

        if (result == OMX_ErrorNone && pConfig->nIDRPeriod) {
            result = SetIDRPeriod(pConfig->nIDRPeriod,avc.nPFrames);
        }
        if (result == OMX_ErrorNone && pConfig->nPerfMode) {
            result = SetVencPerfMode(pConfig->nPerfMode);
        }
        if (result == OMX_ErrorNone) {
            result = SetHierPNumLayers();
        }
        if(result == OMX_ErrorNone && pConfig->nSarWidth &&
                pConfig->nSarHeight) {
           result = SetSarInfo(pConfig->nSarWidth, pConfig->nSarHeight);
        }

        if (result == OMX_ErrorNone) {
            result = SetPqDisable();
        }

        if (pConfig->eControlRate != OMX_Video_ControlRateDisable && pConfig->initialQP_RC_ON.enableInitialQP) {

            QOMX_EXTNINDEX_VIDEO_INITIALQP initQP;
            OMX_INIT_STRUCT(&initQP, QOMX_EXTNINDEX_VIDEO_INITIALQP);
            initQP.nPortIndex = (OMX_U32)PORT_INDEX_OUT;
            result = OMX_GetParameter(m_hEncoder,
                    (OMX_INDEXTYPE)QOMX_IndexParamVideoInitialQp, (OMX_PTR)&initQP);
            if (result != OMX_ErrorNone) {
                return result;
            }
            OMX_INIT_STRUCT_SIZE(&initQP, QOMX_EXTNINDEX_VIDEO_INITIALQP);
            initQP.nQpI = pConfig->initialQP_RC_ON.fQP.iQP;
            initQP.nQpP = pConfig->initialQP_RC_ON.fQP.pQP;
            initQP.nQpB = pConfig->initialQP_RC_ON.fQP.bQP;
            initQP.bEnableInitQp = 1;
            VTEST_MSG_MEDIUM("Setting initialQP: 0x%d%d%d (IPB)", initQP.nQpI,initQP.nQpP,initQP.nQpB);
            result = OMX_SetParameter(m_hEncoder,
                    (OMX_INDEXTYPE)QOMX_IndexParamVideoInitialQp, (OMX_PTR)&initQP);

        } else if (pConfig->eControlRate == OMX_Video_ControlRateDisable) {
            OMX_VIDEO_PARAM_QUANTIZATIONTYPE frameQP;
            OMX_INIT_STRUCT(&frameQP, OMX_VIDEO_PARAM_QUANTIZATIONTYPE);
            frameQP.nPortIndex = (OMX_U32)PORT_INDEX_OUT;
            result = OMX_GetParameter(m_hEncoder,
                    OMX_IndexParamVideoQuantization, (OMX_PTR)&frameQP);
            if (result != OMX_ErrorNone) {
                return result;
            }
            OMX_INIT_STRUCT_SIZE(&frameQP, OMX_VIDEO_PARAM_QUANTIZATIONTYPE);
            frameQP.nQpI = pConfig->frameQP_RC_OFF.iQP;
            frameQP.nQpP = pConfig->frameQP_RC_OFF.pQP;
            frameQP.nQpB = pConfig->frameQP_RC_OFF.bQP;
            VTEST_MSG_MEDIUM("Setting frameQP: 0x%d%d%d (IPB)", frameQP.nQpI,frameQP.nQpP,frameQP.nQpB);
            result = OMX_SetParameter(m_hEncoder,
                    OMX_IndexParamVideoQuantization, (OMX_PTR)&frameQP);
        }

        if (result == OMX_ErrorNone) {
            result = SetBitrate(pConfig->eControlRate, pConfig->nBitrate);
            FAILED1(result, "Failed : Setting bitrate")
        }
    } else if (m_eCodec == OMX_VIDEO_CodingVP8) {

        OMX_VIDEO_PARAM_VP8TYPE vp8;
        OMX_INIT_STRUCT(&vp8, OMX_VIDEO_PARAM_VP8TYPE);
        vp8.nPortIndex = (OMX_U32)PORT_INDEX_OUT;
        result = OMX_GetParameter(m_hEncoder,
                (OMX_INDEXTYPE)OMX_IndexParamVideoVp8, (OMX_PTR)&vp8);

        if (result != OMX_ErrorNone) {
            VTEST_MSG_ERROR("Failed to OMX_GetParameter(OMX_IndexParamVideoVp8), error = 0x%x",
                    result);
            return result;
        }

        if (pConfig->eCodecProfile == VP8ProfileMain) {
            vp8.eProfile = OMX_VIDEO_VP8ProfileMain;
        } else {
            VTEST_MSG_ERROR("Invalid VP8 Profile set defaulting to OMX_VIDEO_VP8ProfileMain");
            vp8.eProfile = OMX_VIDEO_VP8ProfileMain;
        }

        if (pConfig->eCodecLevel == VP8Version0 || pConfig->eCodecProfile == VP8ProfileVersion0) {
            vp8.eLevel = OMX_VIDEO_VP8Level_Version0;
        } else if (pConfig->eCodecLevel == VP8Version1 || pConfig->eCodecProfile == VP8ProfileVersion1) {
            vp8.eLevel = OMX_VIDEO_VP8Level_Version1;
        } else if ((!pConfig->eCodecLevel) == DefaultLevel) {
            VTEST_MSG_ERROR("Invalid VP8 Level using default");
        }

        OMX_INIT_STRUCT_SIZE(&vp8, OMX_VIDEO_PARAM_VP8TYPE);
        result = OMX_SetParameter(m_hEncoder,
                (OMX_INDEXTYPE)OMX_IndexParamVideoVp8, (OMX_PTR)&vp8);
        if (result == OMX_ErrorNone && pConfig->nIntraPeriod) {
            result = SetIntraPeriod(pConfig->nIntraPeriod);
        }

        if(result == OMX_ErrorNone) {
           result = SetHierPNumLayers();
        }

        if (result == OMX_ErrorNone) {
            result = SetBitrate(pConfig->eControlRate, pConfig->nBitrate);
            FAILED1(result, "Failed : Setting bitrate")
        }

    } else if ((OMX_U32)m_eCodec == (OMX_U32)QOMX_VIDEO_CodingHevc) {

        OMX_VIDEO_PARAM_HEVCTYPE hevc;
        OMX_INIT_STRUCT(&hevc, OMX_VIDEO_PARAM_HEVCTYPE);
        hevc.nPortIndex = (OMX_U32)PORT_INDEX_OUT;

        result = OMX_GetParameter(m_hEncoder,
                (OMX_INDEXTYPE)OMX_IndexParamVideoHevc, (OMX_PTR)&hevc);
        if (result != OMX_ErrorNone) {
            VTEST_MSG_ERROR("Failed to OMX_GetParameter(OMX_IndexParamVideoHevc), error = 0x%x",
                    result);
            return result;
        }

        if(pConfig->eCodecProfile == HEVCProfileMain) {
            hevc.eProfile = OMX_VIDEO_HEVCProfileMain;
        } else if(pConfig->eCodecProfile == HEVCProfileMain10) {
            hevc.eProfile = OMX_VIDEO_HEVCProfileMain10;
        } else {
            VTEST_MSG_ERROR("Invalid Hevc Profile set defaulting to OMX_VIDEO_HEVCProfileMain");
            hevc.eProfile = OMX_VIDEO_HEVCProfileMain;
        }

        hevc.eLevel = OMX_VIDEO_HEVCMainTierLevel1;

        OMX_INIT_STRUCT_SIZE(&hevc, OMX_VIDEO_PARAM_HEVCTYPE);
        result = OMX_SetParameter(m_hEncoder,
                (OMX_INDEXTYPE)OMX_IndexParamVideoHevc, (OMX_PTR)&hevc);
        if (result == OMX_ErrorNone && pConfig->nIntraPeriod) {
            result = SetIntraPeriod(pConfig->nIntraPeriod);
        }

        if (result == OMX_ErrorNone) {
            result = SetHierPNumLayers();
            FAILED1(result, "Failed : Setting Hier - P for HEVC")
        }

        if (result == OMX_ErrorNone) {
            result = SetBitrate(pConfig->eControlRate, pConfig->nBitrate);
            FAILED1(result, "Failed : Setting bitrate")
        }
    }

    //////////////////////////////////////////
    // error resilience
    //////////////////////////////////////////
    if (result != OMX_ErrorNone) {
        return result;
    }

    OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE errorCorrection;
    OMX_INIT_STRUCT(&errorCorrection, OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE);
    errorCorrection.nPortIndex = (OMX_U32)PORT_INDEX_OUT;
    result = OMX_GetParameter(m_hEncoder,
            (OMX_INDEXTYPE)OMX_IndexParamVideoErrorCorrection,
            (OMX_PTR)&errorCorrection);

    errorCorrection.bEnableRVLC = OMX_FALSE;
    errorCorrection.bEnableDataPartitioning = OMX_FALSE;

    if (result != OMX_ErrorNone) {
        return result;
    }

    if (pConfig->eResyncMarkerType == RESYNC_MARKER_MB) {
        switch (m_eCodec) {
            case OMX_VIDEO_CodingAVC:
                OMX_VIDEO_PARAM_AVCTYPE avcdata;
                OMX_INIT_STRUCT(&avcdata, OMX_VIDEO_PARAM_AVCTYPE);
                avcdata.nPortIndex = (OMX_U32)PORT_INDEX_OUT; // output
                result = OMX_GetParameter(m_hEncoder,
                                          OMX_IndexParamVideoAvc, (OMX_PTR)&avcdata);
                if (result == OMX_ErrorNone) {
                    VTEST_MSG_HIGH("slice mode enabled with slice size = %u",
                                   (unsigned int)pConfig->nResyncMarkerSpacing);
                    avcdata.nSliceHeaderSpacing = pConfig->nResyncMarkerSpacing;
                    OMX_INIT_STRUCT_SIZE(&avcdata, OMX_VIDEO_PARAM_AVCTYPE);
                    result = OMX_SetParameter(m_hEncoder,
                                              OMX_IndexParamVideoAvc, (OMX_PTR) & avcdata);
                    if (result != OMX_ErrorNone) {
                        VTEST_MSG_ERROR("Failed to set slice header spacing");
                    }

                    OMX_BOOL slice_delivery_mode = OMX_FALSE;
                    slice_delivery_mode = pConfig->bEnableSliceDeliveryMode;

                    if (slice_delivery_mode) {
                        VTEST_MSG_HIGH("Slice delivery mode is enabled.");
                        QOMX_EXTNINDEX_PARAMTYPE extnIndex;
                        OMX_INIT_STRUCT(&extnIndex, QOMX_EXTNINDEX_PARAMTYPE);
                        extnIndex.nPortIndex = PORT_INDEX_OUT;
                        extnIndex.bEnable = OMX_TRUE;
                        OMX_INIT_STRUCT_SIZE(&extnIndex, QOMX_EXTNINDEX_PARAMTYPE);
                        result = OMX_SetParameter(m_hEncoder,
                                                  (OMX_INDEXTYPE)OMX_QcomIndexEnableSliceDeliveryMode,
                                                  (OMX_PTR) & extnIndex);
                        if (result != OMX_ErrorNone) {
                            VTEST_MSG_ERROR("Failed to set slice delivery mode");
                        }
                    }
                }
                break;
            case OMX_VIDEO_CodingMPEG4:
                OMX_VIDEO_PARAM_MPEG4TYPE mp4;
                OMX_INIT_STRUCT(&mp4, OMX_VIDEO_PARAM_MPEG4TYPE);
                mp4.nPortIndex = (OMX_U32)PORT_INDEX_OUT;
                result = OMX_GetParameter(m_hEncoder,
                                          OMX_IndexParamVideoMpeg4, (OMX_PTR)&mp4);
                if (result == OMX_ErrorNone) {
                    mp4.nSliceHeaderSpacing = pConfig->nResyncMarkerSpacing;
                    OMX_INIT_STRUCT_SIZE(&mp4, OMX_VIDEO_PARAM_MPEG4TYPE);
                    result = OMX_SetParameter(m_hEncoder,
                                              OMX_IndexParamVideoMpeg4, (OMX_PTR)&mp4);
                    if (result != OMX_ErrorNone) {
                        VTEST_MSG_ERROR("Failed to set slice header spacing");
                    }
                }
                break;
            default:
                break;
        }
    } else if (pConfig->eResyncMarkerType != RESYNC_MARKER_NONE) {
        if (pConfig->eResyncMarkerType == RESYNC_MARKER_BITS) {
            switch (m_eCodec) {
                case OMX_VIDEO_CodingMPEG4:
                    errorCorrection.bEnableHEC = pConfig->nHECInterval == 0 ? OMX_FALSE : OMX_TRUE;
                case OMX_VIDEO_CodingAVC:
                    errorCorrection.bEnableResync = OMX_TRUE;
                    errorCorrection.nResynchMarkerSpacing = pConfig->nResyncMarkerSpacing;
                    break;
                default:
                    break;
            }
        } else if (pConfig->eResyncMarkerType == RESYNC_MARKER_GOB
                   && m_eCodec == OMX_VIDEO_CodingH263) {
            errorCorrection.bEnableResync = OMX_FALSE;
            errorCorrection.nResynchMarkerSpacing = pConfig->nResyncMarkerSpacing;
            errorCorrection.bEnableDataPartitioning = OMX_TRUE;
        }

        VTEST_MSG_ERROR("Setting nResyncMarkerSpacing = %u", errorCorrection.nResynchMarkerSpacing);
        OMX_INIT_STRUCT_SIZE(&errorCorrection, OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE);
        result = OMX_SetParameter(m_hEncoder,
                                  (OMX_INDEXTYPE)OMX_IndexParamVideoErrorCorrection,
                                  (OMX_PTR)&errorCorrection);
        if (result != OMX_ErrorNone) {
            VTEST_MSG_ERROR("Failed to set error correction type.");
        }
    }

    //////////////////////////////////////////
    // Note: ExtraData should be set before OMX_IndexParamPortDefinition
    //       else driver will through error allocating input/output buffers
    //////////////////////////////////////////

    QOMX_INDEXEXTRADATATYPE e; // OMX_QcomIndexParamIndexExtraDataType
    OMX_INIT_STRUCT(&e, QOMX_INDEXEXTRADATATYPE);
    e.nPortIndex = (OMX_U32)PORT_INDEX_OUT;

    for(OMX_U32 i=0; i<pConfig->nExtradataConfigArraySize; i++)
    {
        switch ((int)pConfig->pExtradataProperties[i].ExtradataType)
        {
            case EncMBIStatSSExtraData:
            case EncMBIStatT2TExtraData:
            case EncMBIStatFRCExtraData:
            {
                e.nIndex = (OMX_INDEXTYPE)OMX_ExtraDataVideoEncoderMBInfo;
                e.bEnabled = OMX_TRUE;
                result = OMX_SetParameter(m_hEncoder,
                                         (OMX_INDEXTYPE)OMX_QcomIndexParamIndexExtraDataType,
                                         (OMX_PTR)&e);
                OMX_QOMX_VIDEO_MBI_STATISTICS MBI_ExData;
                OMX_INIT_STRUCT(&MBI_ExData, OMX_QOMX_VIDEO_MBI_STATISTICS);
                MBI_ExData.nPortIndex = (OMX_U32)PORT_INDEX_OUT;
                if(EncMBIStatSSExtraData == pConfig->pExtradataProperties[i].ExtradataType)
                {
                    MBI_ExData.eMBIStatisticsType = QOMX_MBI_STATISTICS_MODE_1;
                }
                else if(EncMBIStatT2TExtraData == pConfig->pExtradataProperties[i].ExtradataType)
                {
                    MBI_ExData.eMBIStatisticsType = QOMX_MBI_STATISTICS_MODE_2;
                }
                else
                {
                    MBI_ExData.eMBIStatisticsType = QOMX_MBI_STATISTICS_MODE_DEFAULT;//no support for mode 3 in component
                }
                /* enable extradata */
                result = OMX_SetParameter(m_hEncoder, (OMX_INDEXTYPE)OMX_QcomIndexParamMBIStatisticsMode,
                                                                                   (OMX_PTR)&MBI_ExData);
                FAILED1(result, "ERROR: Failed to enable Interalce extradata");
                break;
            }
            default:
            {
                VTEST_MSG_HIGH("Unsupported extradata type 0x%x\n",pConfig->pExtradataProperties[i].ExtradataType);
                break;
            }
        }
    }

    /* LTR encoding settings */
    if (result == OMX_ErrorNone && pConfig->nLTRCount) {
        /* Configure LTR count */
        QOMX_VIDEO_PARAM_LTRCOUNT_TYPE  ltrCount;
        OMX_INIT_STRUCT(&ltrCount, QOMX_VIDEO_PARAM_LTRCOUNT_TYPE);
        ltrCount.nPortIndex = (OMX_U32)PORT_INDEX_OUT;
        ltrCount.nCount = pConfig->nLTRCount;

        result = OMX_SetParameter(m_hEncoder,
                (OMX_INDEXTYPE)QOMX_IndexParamVideoLTRCount,
                (OMX_PTR)&ltrCount);
        if (result != OMX_ErrorNone) {
            VTEST_MSG_ERROR("Set LTR Count failed with status %d", result);
            return result;
        }
    }

    //////////////////////////////////////////
    // BLUR
    //////////////////////////////////////////
    OMX_INDEXTYPE index;
    OMX_STRING name;
    name = const_cast<OMX_STRING>(
                  OMX_QTI_INDEX_CONFIG_VIDEO_BLURINFO);
    result = OMX_GetExtensionIndex(m_hEncoder, name, &index);

    if (result == OMX_ErrorNone) {
        OMX_QTI_VIDEO_CONFIG_BLURINFO blur_resol;
        OMX_INIT_STRUCT(&blur_resol, OMX_QTI_VIDEO_CONFIG_BLURINFO);
        blur_resol.nPortIndex = (OMX_U32)PORT_INDEX_IN;

        if (m_sConfig.nBlurWidth == 0 && m_sConfig.nBlurHeight == 0) {
            blur_resol.eTargetResol = BLUR_RESOL_DISABLED;
        } else if (m_sConfig.nBlurWidth == 426 && m_sConfig.nBlurHeight == 240 ) {
            blur_resol.eTargetResol = BLUR_RESOL_240;
        } else if (m_sConfig.nBlurWidth == 854 && m_sConfig.nBlurHeight == 480) {
            blur_resol.eTargetResol = BLUR_RESOL_480;
        } else if (m_sConfig.nBlurWidth == 1280 && m_sConfig.nBlurHeight == 720) {
            blur_resol.eTargetResol = BLUR_RESOL_720;
        } else if (m_sConfig.nBlurWidth == 1920 && m_sConfig.nBlurHeight == 1080) {
            blur_resol.eTargetResol = BLUR_RESOL_1080;
        } else {
            VTEST_MSG_ERROR("Dimensions not supported for blur %dx%d", m_sConfig.nBlurWidth, m_sConfig.nBlurHeight);
        }

        VTEST_MSG_HIGH("Set Blur resolution W x H = %d x %d", m_sConfig.nBlurWidth, m_sConfig.nBlurHeight);
        result = OMX_SetConfig(m_hEncoder,
                              index,
                              (OMX_PTR)&blur_resol);
        FAILED1(result, "Set Blur resolution failed with status %d", result);

        //Read back the blur resolution
        OMX_INIT_STRUCT(&blur_resol, OMX_QTI_VIDEO_CONFIG_BLURINFO);
        blur_resol.nPortIndex = (OMX_U32)PORT_INDEX_IN;
        result = OMX_GetConfig(m_hEncoder,
                               index,
                               (OMX_PTR)&blur_resol);
        FAILED1(result, "Get Blur resolution failed with status %d", result);

        VTEST_MSG_LOW("Get Blur resol index %d", blur_resol.eTargetResol);
    }

    //////////////////////////////////////////
    // input buffer requirements
    //////////////////////////////////////////
    if (result != OMX_ErrorNone) {
        return result;
    }

    result = SetPortParams(PORT_INDEX_IN,
            m_nFrameWidth, m_nFrameHeight,
            pConfig->nInBufferCount, pConfig->nFramerate,
            &m_nInputBufferSize, &m_nInputBufferCount);

    //////////////////////////////////////////
    // output buffer requirements
    //////////////////////////////////////////
    if (result != OMX_ErrorNone) {
        return result;
    }

    result = SetPortParams(PORT_INDEX_OUT,
            pConfig->nOutputFrameWidth, pConfig->nOutputFrameHeight,
            pConfig->nOutBufferCount, pConfig->nFramerate,
            &m_nOutputBufferSize, &m_nOutputBufferCount);

    //////////////////////////////////////////
    // bitrate
    //////////////////////////////////////////

    if (result != OMX_ErrorNone) {
        return result;
    }

    //////////////////////////////////////////
    // quantization parameter range
    //////////////////////////////////////////

    if ((pConfig->nMinQp > 0 && pConfig->nMaxQp > 0) ||
        (pConfig->minQP_Packed > 0 && pConfig->maxQP_Packed > 0)) {
        result = SetQPRange(pConfig->nMinQp, pConfig->nMaxQp,
            pConfig->minQP_Packed, pConfig->maxQP_Packed);
        if (result != OMX_ErrorNone) {
            return result;
        }
    }

    //////////////////////////////////////////
    // frame rate
    //////////////////////////////////////////
    OMX_CONFIG_FRAMERATETYPE framerate;
    OMX_INIT_STRUCT(&framerate, OMX_CONFIG_FRAMERATETYPE);
    framerate.nPortIndex = (OMX_U32)PORT_INDEX_IN;
    result = OMX_GetConfig(m_hEncoder,
            OMX_IndexConfigVideoFramerate, (OMX_PTR)&framerate);
    if (result != OMX_ErrorNone) {
        return result;
    }

    FractionToQ16(framerate.xEncodeFramerate, (int)(pConfig->nFramerate * 2), 2);
    OMX_INIT_STRUCT_SIZE(&framerate, OMX_CONFIG_FRAMERATETYPE);
    result = OMX_SetConfig(m_hEncoder,
            OMX_IndexConfigVideoFramerate, (OMX_PTR)&framerate);
    if (result != OMX_ErrorNone) {
        return result;
    }

    //////////////////////////////////////////
    // intra refresh
    //////////////////////////////////////////
    if (((OMX_U32)pConfig->nIntraRefreshMBCount <= ((m_nFrameWidth * m_nFrameHeight) >> 8))) {
        if (OMX_VIDEO_IntraRefreshRandom != pConfig->nIRMode) {
            VTEST_MSG_LOW("Send SetConfig for IntraRefresh");
            result = SetIntraRefresh((OMX_VIDEO_INTRAREFRESHTYPE)pConfig->nIRMode,pConfig->nIntraRefreshMBCount);
            if (result != OMX_ErrorNone) {
                VTEST_MSG_ERROR("SetConfig for Cyclic IntraRefresh returned error: %d", result);
                return result;
            }
        }
#ifdef SUPPORT_CONFIG_INTRA_REFRESH
        else
        {
            OMX_VIDEO_CONFIG_ANDROID_INTRAREFRESHTYPE ir;
            OMX_U32 mb_size = pConfig->eCodec == OMX_VIDEO_CodingHEVC ? 32 : 16;
            OMX_U32 num_mbs_per_frame = (ALIGN(m_nFrameHeight, mb_size)/mb_size) * (ALIGN(m_nFrameWidth, mb_size)/mb_size);
            OMX_U32 nRefreshPeriod = 0;

            ir.nPortIndex = (OMX_U32)PORT_INDEX_OUT;
            if (pConfig->nIntraRefreshMBCount == 0){
                nRefreshPeriod = ir.nRefreshPeriod = 0;
            }
            else if (pConfig->nIntraRefreshMBCount == ((m_nFrameWidth * m_nFrameHeight) >> 8)) {
                nRefreshPeriod = ir.nRefreshPeriod = 1;
            }
            else {
                nRefreshPeriod = ir.nRefreshPeriod = ceil(num_mbs_per_frame / pConfig->nIntraRefreshMBCount);
            }
            result = OMX_SetConfig(m_hEncoder,
                   (OMX_INDEXTYPE)OMX_IndexConfigAndroidIntraRefresh, (OMX_PTR)&ir);
            if (result != OMX_ErrorNone) {
                VTEST_MSG_ERROR("SetConfig for Android IntraRefresh returned error: %d", result);
                return result;
            }
            VTEST_MSG_LOW("Send SetConfig for Android IntraRefresh....success");

            result = OMX_GetConfig(m_hEncoder,
                   (OMX_INDEXTYPE)OMX_IndexConfigAndroidIntraRefresh, (OMX_PTR)&ir);
            if (result != OMX_ErrorNone) {
                return result;
            }
            if (nRefreshPeriod != ir.nRefreshPeriod) {
                VTEST_MSG_ERROR("Failed to read back correct IntraRefresh Period Set(%d) vs Read(%d)", nRefreshPeriod, ir.nRefreshPeriod);
                return OMX_ErrorInvalidState;
            }
        }
#endif
    } else {
        VTEST_MSG_ERROR(
                "IntraRefresh not set for %u MBs because total MBs is %u",
                (unsigned int)pConfig->nIntraRefreshMBCount, (unsigned int)(m_nFrameWidth * m_nFrameHeight >> 8));
    }

    //////////////////////////////////////////
    // rotation
    //////////////////////////////////////////
    if (pConfig->nRotation) {

        OMX_CONFIG_ROTATIONTYPE framerotate;
        OMX_INIT_STRUCT(&framerotate, OMX_CONFIG_ROTATIONTYPE);
        framerotate.nPortIndex = (OMX_U32)PORT_INDEX_OUT;
        result = OMX_GetConfig(m_hEncoder,
                OMX_IndexConfigCommonRotate, (OMX_PTR)&framerotate);
        if (result != OMX_ErrorNone) {
            return result;
        }

        framerotate.nRotation = pConfig->nRotation;
        result = OMX_SetConfig(m_hEncoder,
                OMX_IndexConfigCommonRotate, (OMX_PTR)&framerotate);
        if (result != OMX_ErrorNone) {
            return result;
        }
    }

    //////////////////////////////////////////
    // deinterlace
    //////////////////////////////////////////
    if (pConfig->bDeinterlace) {

        OMX_VIDEO_CONFIG_DEINTERLACE deinterlace;
        OMX_INIT_STRUCT(&deinterlace, OMX_VIDEO_CONFIG_DEINTERLACE);
        deinterlace.nPortIndex = (OMX_U32)PORT_INDEX_OUT;
        result = OMX_GetConfig(m_hEncoder,
                OMX_IndexConfigCommonDeinterlace, (OMX_PTR)&deinterlace);
        if (result != OMX_ErrorNone) {
            return result;
        }

        deinterlace.nEnable = (OMX_U32)pConfig->bDeinterlace;
        result = OMX_SetConfig(m_hEncoder,
                OMX_IndexConfigCommonDeinterlace, (OMX_PTR)&deinterlace);
        if (result != OMX_ErrorNone) {
            return result;
        }
    }

    //////////////////////////////////////////
    // MBI Statistics mode selection
    //////////////////////////////////////////
    if (pConfig->eMBIStatisticsMode) {

        OMX_QOMX_VIDEO_MBI_STATISTICS mbistatisticsmode;
        OMX_INIT_STRUCT(&mbistatisticsmode, OMX_QOMX_VIDEO_MBI_STATISTICS);
        mbistatisticsmode.nPortIndex = (OMX_U32)PORT_INDEX_OUT;
        result = OMX_GetParameter(m_hEncoder,
                (OMX_INDEXTYPE)OMX_QcomIndexParamMBIStatisticsMode, (OMX_PTR)&mbistatisticsmode);

        FAILED1(result, "Failed : Get param for OMX_QcomIndexParamMBIStatisticsMode");

        mbistatisticsmode.eMBIStatisticsType = (OMX_QOMX_VIDEO_MBISTATISTICSTYPE)pConfig->eMBIStatisticsMode;
        result = OMX_SetParameter(m_hEncoder,
                (OMX_INDEXTYPE)OMX_QcomIndexParamMBIStatisticsMode, (OMX_PTR)&mbistatisticsmode);

        FAILED1(result, "Failed : Set param for OMX_QcomIndexParamMBIStatisticsMode");
    }

    if (pConfig->nMaxHPLayers && pConfig->nHierPNumLayers > pConfig->nMaxHPLayers) {

        QOMX_EXTNINDEX_VIDEO_HIER_P_LAYERS maxHPlayers;
        OMX_INIT_STRUCT(&maxHPlayers, QOMX_EXTNINDEX_VIDEO_HIER_P_LAYERS);
        result = OMX_GetConfig(m_hEncoder,
                (OMX_INDEXTYPE) OMX_QcomIndexConfigNumHierPLayers, (OMX_PTR)&maxHPlayers);
        FAILED1(result, "Failed : Get Config for OMX_QcomIndexConfigMaxHierPLayers");

        maxHPlayers.nNumHierLayers = (OMX_U32)pConfig->nMaxHPLayers;
        result = OMX_SetConfig(m_hEncoder,
                (OMX_INDEXTYPE) OMX_QcomIndexConfigNumHierPLayers, (OMX_PTR)&maxHPlayers);
        FAILED1(result, "Failed : Set Config for OMX_QcomIndexConfigMaxHierPLayers");
    }

    if (pConfig->nRectLeft || pConfig->nRectTop ||
            pConfig->nRectWidth || pConfig->nRectHeight) {

        OMX_CONFIG_RECTTYPE rectangle;
        OMX_INIT_STRUCT(&rectangle, OMX_CONFIG_RECTTYPE);
        result = OMX_GetConfig(m_hEncoder,
                (OMX_INDEXTYPE) OMX_QcomIndexConfigRectType, (OMX_PTR)&rectangle);
        FAILED1(result, "Failed : Get Config for OMX_QcomIndexConfigRectType");

        rectangle.nLeft = pConfig->nRectLeft;
        rectangle.nTop = pConfig->nRectTop;
        rectangle.nWidth = pConfig->nRectWidth;
        rectangle.nHeight = pConfig->nRectHeight;
        OMX_INIT_STRUCT_SIZE(&rectangle, OMX_CONFIG_RECTTYPE);
        result = OMX_SetConfig(m_hEncoder,
                (OMX_INDEXTYPE) OMX_QcomIndexConfigRectType, (OMX_PTR)&rectangle);
        FAILED1(result, "Failed : Set Config for OMX_QcomIndexConfigRectType");
    }

    if (pConfig->nBaseLayerId) {

        OMX_SKYPE_VIDEO_CONFIG_BASELAYERPID baselayerid;
        OMX_INIT_STRUCT(&baselayerid, OMX_SKYPE_VIDEO_CONFIG_BASELAYERPID);
        result = OMX_GetConfig(m_hEncoder,
                (OMX_INDEXTYPE) OMX_QcomIndexConfigBaseLayerId, (OMX_PTR)&baselayerid);
        FAILED1(result, "Failed : Get Config for OMX_QcomIndexConfigBaseLayerId");

        baselayerid.nPID = (OMX_U32)pConfig->nBaseLayerId;
        OMX_INIT_STRUCT_SIZE(&baselayerid, OMX_SKYPE_VIDEO_CONFIG_BASELAYERPID);
        result = OMX_SetConfig(m_hEncoder,
                (OMX_INDEXTYPE) OMX_QcomIndexConfigBaseLayerId, (OMX_PTR)&baselayerid);
        FAILED1(result, "Failed : Set Config for OMX_QcomIndexConfigBaseLayerId");
    }
    //////////////////////////////////////////
    // H264 8x8 transform mode
    //////////////////////////////////////////
    if (pConfig->nTransform8x8 && m_eCodec == OMX_VIDEO_CodingAVC) {

        OMX_CONFIG_BOOLEANTYPE mTransform8x8;
        if (pConfig->nTransform8x8 == 1)
            mTransform8x8.bEnabled = OMX_TRUE;
        else if (pConfig->nTransform8x8 == 0)
            mTransform8x8.bEnabled = OMX_FALSE;
        result = OMX_SetConfig(m_hEncoder,
                (OMX_INDEXTYPE)OMX_QcomIndexConfigH264Transform8x8, (OMX_PTR)&mTransform8x8);

        FAILED1(result, "Failed : Set config for OMX_QcomIndexConfigH264Transform8x8");
        if (strlen(pConfig->cRoiQpFileName) > 0) {
            OMX_INDEXTYPE index;
            OMX_STRING name;
            name = const_cast<OMX_STRING>(
                    OMX_QTI_INDEX_PARAM_VIDEO_ENABLE_ROIINFO);
            result = OMX_GetExtensionIndex(m_hEncoder, name, &index);
            FAILED1(result, "Failed : Get Extension index OMX_QTI_INDEX_PARAM_VIDEO_ENABLE_ROIINFO");
            OMX_QTI_VIDEO_PARAM_ENABLE_ROIINFO bEnRoi;
            bEnRoi.nPortIndex = (OMX_U32)PORT_INDEX_IN;
            bEnRoi.bEnableRoiInfo = OMX_TRUE;
            result = OMX_SetParameter(m_hEncoder, index, (OMX_PTR)&bEnRoi);
            FAILED1(result, "Failed : Set parameter for %d", index);
        }
    }
    // go to Idle, so we can allocate buffers
    result = SetState(OMX_StateIdle, OMX_FALSE);
    FAILED1(result, "Could not move to OMX_StateIdle!");
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Encoder::SetQPRange(OMX_S32 minQP, OMX_S32 maxQP,
        OMX_U32 minQP_packed, OMX_U32 maxQP_packed) {

    OMX_ERRORTYPE result = OMX_ErrorNone;
    OMX_U8 min_qp_limit = 0, max_qp_limit=0;

    // set minimum and maximum allowed QP values for session
    if (m_eCodec == OMX_VIDEO_CodingAVC ||
        m_eCodec == OMX_VIDEO_CodingHEVC) {
        min_qp_limit = MIN_ALLOWED_QP_H264;
        max_qp_limit = MAX_ALLOWED_QP_H264;
    } else if (m_eCodec == OMX_VIDEO_CodingVP8) {
        min_qp_limit = MIN_ALLOWED_QP_VP8;
        max_qp_limit = MAX_ALLOWED_QP_VP8;
    } else if (m_eCodec == OMX_VIDEO_CodingH263||
        m_eCodec == OMX_VIDEO_CodingMPEG4) {
        min_qp_limit = MIN_ALLOWED_QP_MPEG4;
        max_qp_limit = MAX_ALLOWED_QP_MPEG4;
    }

    if (minQP > 0 && maxQP > 0) {
        ///////////////////////////////////////////////////////////
        // Set same QP range for I P and B frame
        ///////////////////////////////////////////////////////////

        //Check if min and max QP values are within the boundaries
        if(minQP > maxQP || minQP < min_qp_limit || maxQP > max_qp_limit) {
            VTEST_MSG_ERROR("QP range setting are out of bound for this codec");
            return OMX_ErrorBadParameter;
        }

        OMX_QCOM_VIDEO_PARAM_QPRANGETYPE qprange;
        OMX_INIT_STRUCT_SIZE(&qprange, OMX_QCOM_VIDEO_PARAM_QPRANGETYPE);
        qprange.nPortIndex = (OMX_U32)PORT_INDEX_OUT;
        result = OMX_GetParameter(m_hEncoder,
                (OMX_INDEXTYPE)OMX_QcomIndexParamVideoQPRange,
                (OMX_PTR)&qprange);
        if (result != OMX_ErrorNone) {
            return result;
        }

        qprange.minQP = minQP;
        qprange.maxQP = maxQP;

        VTEST_MSG_MEDIUM(
                "Calling OMX_QcomIndexParamVideoQPRange with minQP = %u, maxQP = %u",
                (unsigned int)qprange.minQP, (unsigned int)qprange.maxQP);
        qprange.nPortIndex = (OMX_U32)PORT_INDEX_OUT;
        OMX_INIT_STRUCT_SIZE(&qprange, OMX_QCOM_VIDEO_PARAM_QPRANGETYPE);
        result = OMX_SetParameter(m_hEncoder,
                (OMX_INDEXTYPE)OMX_QcomIndexParamVideoQPRange,
                (OMX_PTR)&qprange);

        if (result != OMX_ErrorNone) {
            return result;
        }
    }

    if(minQP_packed > 0 && maxQP_packed > 0) {
        ///////////////////////////////////////////////////////////
        // Set different QP range for I P and B frame
        ///////////////////////////////////////////////////////////
        OMX_U8 IMinQP, PMinQP, BMinQP;
        OMX_U8 IMaxQP, PMaxQP, BMaxQP;
        BMinQP = (minQP_packed >> 16) & 0xFF;
        PMinQP = (minQP_packed >> 8) & 0xFF;
        IMinQP = minQP_packed & 0xFF;

        BMaxQP = (maxQP_packed >> 16) & 0xFF;
        PMaxQP = (maxQP_packed >> 8) & 0xFF;
        IMaxQP = maxQP_packed & 0xFF;
        //Check if min and max QP values are within the boundaries
        if((IMinQP > IMaxQP) || (PMinQP > PMaxQP) ||  (BMinQP > BMaxQP) ||
            (IMaxQP > max_qp_limit) || (PMaxQP > max_qp_limit) || (BMaxQP > max_qp_limit) ||
            (min_qp_limit > IMinQP) || (min_qp_limit > PMinQP) || (min_qp_limit > BMinQP)) {
            VTEST_MSG_ERROR("QP range setting are out of bound for this codec");
            return OMX_ErrorBadParameter;
        }
        OMX_QCOM_VIDEO_PARAM_IPB_QPRANGETYPE qprange;
        OMX_INIT_STRUCT_SIZE(&qprange, OMX_QCOM_VIDEO_PARAM_IPB_QPRANGETYPE);

        qprange.nPortIndex = (OMX_U32)PORT_INDEX_OUT;
        result = OMX_GetParameter(m_hEncoder,
                (OMX_INDEXTYPE)OMX_QcomIndexParamVideoIPBQPRange,(OMX_PTR)&qprange);
        FAILED1(result, "error getting OMX_QcomIndexParamVideoIPBQPRange");
        OMX_INIT_STRUCT_SIZE(&qprange, OMX_QCOM_VIDEO_PARAM_IPB_QPRANGETYPE);
        qprange.minIQP = IMinQP;
        qprange.maxIQP = IMaxQP;
        qprange.minPQP = PMinQP;
        qprange.maxPQP = PMaxQP;
        qprange.minBQP = BMinQP;
        qprange.maxBQP = BMaxQP;

        result = OMX_SetParameter(m_hEncoder,
            (OMX_INDEXTYPE)OMX_QcomIndexParamVideoIPBQPRange,
            (OMX_PTR)&qprange);
        FAILED1(result, "error setting OMX_QcomIndexParamVideoIPBQPRange");
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Encoder::SetBuffer(BufferInfo *pBuffer, ISource *pSource) {

    OMX_ERRORTYPE result = OMX_ErrorNone;
    {
        Mutex::Autolock autoLock(m_pLock);
        /* TODO : give all buffers back to Buffer Manager */
        if (m_bThreadStop) {
            result = m_pFreeBufferQueue->Push(&pBuffer, sizeof(BufferInfo **));
            return OMX_ErrorNoMore;
        }
        if (m_bPortReconfig && (pSource == m_pSink)) {
            result = m_pFreeBufferQueue->Push(&pBuffer, sizeof(BufferInfo **));
            return result;
        }

    }

    result = ISource::SetBuffer(pBuffer, pSource);
    if (result != OMX_ErrorNone) {
        return result;
    }

    {
        Mutex::Autolock autoLock(m_pLock);
        if ((pBuffer->pHeaderIn->nFlags & OMX_BUFFERFLAG_EOS)
                && (pSource == m_pSource)) {
            VTEST_MSG_HIGH("Got input EOS");
            m_bInputEOSReached = OMX_TRUE;
        }
    }

    if (pSource == m_pSource) { //input port

        VTEST_MSG_LOW("%s ==> %s: ETB size=%u", pSource->Name(), Name(),
                (unsigned int)pBuffer->pHeaderIn->nFilledLen);

        // This allows transcode via buffer copy...
        // TODO: this will not work for secure buffers
        // TODO: fix this ugly hack, required as we lack RTTI
        if (strstr(m_pSource->Name(), "Decoder") != NULL) {
            VTEST_MSG_LOW("In pBuffer copy mode %p %p",
                    pBuffer->pHeaderIn->pBuffer, pBuffer->pHeaderOut->pBuffer);
            memcpy(pBuffer->pHeaderIn->pBuffer, pBuffer->pHeaderOut->pBuffer,
                   pBuffer->pHeaderIn->nFilledLen);
        }
        result = DeliverInput(pBuffer->pHeaderIn);
        FAILED1(result, "ETB error, 0x%lx (%p %p)",
                pBuffer->pHandle, pBuffer->pHeaderIn, pBuffer->pHeaderOut);
    } else if (pSource == m_pSink) { //output port

        VTEST_MSG_LOW("%s ==> %s: FTB size=%u", pSource->Name(), Name(),
                (unsigned int)pBuffer->pHeaderOut->nFilledLen);
        result = DeliverOutput(pBuffer->pHeaderOut);
        FAILED1(result, "FTB error, 0x%lx (%p %p)",
                pBuffer->pHandle, pBuffer->pHeaderIn, pBuffer->pHeaderOut);
    } else {
        VTEST_MSG_ERROR("Invalid source : %s", pSource->Name());
        result = OMX_ErrorBadParameter;
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Encoder::AllocateBuffers(BufferInfo **pBuffers,
        OMX_BUFFERHEADERTYPE **pExtraDataBuffers, OMX_U32 nWidth,
        OMX_U32 nHeight, OMX_U32 nBufferCount, OMX_U32 nBufferSize,
        OMX_U32 nExtraDataBufferCount, OMX_U32 nExtraDataBufferSize,
        OMX_U32 ePortIndex, OMX_U32 nBufferUsage) {
    (void)pExtraDataBuffers, (void)nExtraDataBufferCount, (void)nExtraDataBufferSize;
    (void)nWidth, (void)nHeight, (void)nBufferUsage;
    OMX_ERRORTYPE result = OMX_ErrorNone;
    OMX_U32 nOriginalBufferCount = -1;
    OMX_U32 nOriginalBufferSize = -1;
    OMX_PARAM_PORTDEFINITIONTYPE sPortDef;
    OMX_INIT_STRUCT(&sPortDef, OMX_PARAM_PORTDEFINITIONTYPE);

    Mutex::Autolock autoLock(m_pLock);
    VTEST_MSG_HIGH("alloc %s size %u count %u",
            OMX_PORT_NAME(ePortIndex), (unsigned int)nBufferSize, (unsigned int)nBufferCount);

    if (ePortIndex == PORT_INDEX_IN) {
        nOriginalBufferCount = m_nInputBufferCount;
        nOriginalBufferSize = m_nInputBufferSize;
    } else if (ePortIndex == PORT_INDEX_OUT) {
        nOriginalBufferCount = m_nOutputBufferCount;
        nOriginalBufferSize = m_nOutputBufferSize;
    }

    if ((nOriginalBufferCount > nBufferCount) ||
        (nOriginalBufferSize > nBufferSize)) {
        VTEST_MSG_ERROR(
            "Allocate Buffers failure : original count : %u, new count : %u, "
            "original size : %u, new size : %u",
            (unsigned int)nOriginalBufferCount, (unsigned int)nBufferCount, (unsigned int)nOriginalBufferSize, (unsigned int)nBufferSize);
        return OMX_ErrorBadParameter;
    }

    //TODO: this is a workaround, OMX_Comp should allow this during a PortReconfig but does not.
    if (m_eState != OMX_StateExecuting) {
        sPortDef.nPortIndex = ePortIndex;
        OMX_GetParameter(m_hEncoder, OMX_IndexParamPortDefinition, &sPortDef);
        FAILED1(result, "error get OMX_IndexParamPortDefinition");

        sPortDef.nBufferCountActual = nBufferCount;
        sPortDef.nBufferSize = nBufferSize;
        OMX_INIT_STRUCT_SIZE(&sPortDef, OMX_PARAM_PORTDEFINITIONTYPE);
        result = OMX_SetParameter(m_hEncoder, OMX_IndexParamPortDefinition, (OMX_PTR)&sPortDef);
        FAILED1(result, "error set OMX_IndexParamPortDefinition");
    }

    *pBuffers = new BufferInfo[nBufferCount];
    if(*pBuffers==NULL)
    {
        VTEST_MSG_ERROR("BufferInfo creation failed with a buffer count %d", nBufferCount);
        return OMX_ErrorBadParameter;
    }
    memset(*pBuffers, 0, sizeof(BufferInfo) * nBufferCount);

    for (OMX_U32 i = 0; i < nBufferCount; i++) {

        BufferInfo *pBuffer = &((*pBuffers)[i]);
        OMX_BUFFERHEADERTYPE **pHeaderPtr = pBuffer->GetHeaderPtr(ePortIndex);

        result = OMX_AllocateBuffer(m_hEncoder, pHeaderPtr,
                ePortIndex, this, nBufferSize);
        FAILED1(result, "error allocating buffer");

        pBuffer->pHeaderIn = pBuffer->pHeaderOut = *pHeaderPtr;

        // get buffer fd, this allows "use-buffer" and MDP-access
        struct pmem *pMem =
            (ePortIndex == PORT_INDEX_IN ?
             ((struct pmem *)pBuffer->pHeaderIn->pInputPortPrivate) :
             ((struct pmem *)pBuffer->pHeaderOut->pOutputPortPrivate));
        pBuffer->pHandle = (OMX_U32)pMem->fd;

        VTEST_MSG_HIGH("%s alloc_ct=%u sz=%u handle=%p hdr=(%p %p)",
                OMX_PORT_NAME(ePortIndex), (unsigned int)i+1, (unsigned int)nBufferSize,
                (void *)pBuffer->pHandle, pBuffer->pHeaderIn, pBuffer->pHeaderOut);
    }

    if (ePortIndex == PORT_INDEX_IN) {
        m_nInputBufferCount = nBufferCount;
        m_nInputBufferSize = nBufferSize;
    }
    else if (ePortIndex == PORT_INDEX_OUT) {
        m_nOutputBufferCount = nBufferCount;
        m_nOutputBufferSize = nBufferSize;
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Encoder::UseBuffers(BufferInfo **pBuffers,
        OMX_BUFFERHEADERTYPE **ppExtraDataBuffers, OMX_U32 nWidth,
        OMX_U32 nHeight, OMX_U32 nBufferCount, OMX_U32 nBufferSize,
        OMX_U32 nExtraDataBufferCount, OMX_U32 nExtraDataBufferSize,
        OMX_U32 ePortIndex) {
    (void)nWidth, (void)nHeight;
    (void)ppExtraDataBuffers, (void)nExtraDataBufferCount, (void)nExtraDataBufferSize;
    OMX_ERRORTYPE result = OMX_ErrorNone;

    Mutex::Autolock autoLock(m_pLock);

    if (m_bMetaMode) {
        StoreMetaDataInBuffersParams sMetadataMode;
        OMX_INIT_STRUCT(&sMetadataMode, StoreMetaDataInBuffersParams);
        sMetadataMode.nPortIndex = ePortIndex;
        sMetadataMode.bStoreMetaData = OMX_TRUE;
        result = OMX_SetParameter(m_hEncoder,
            (OMX_INDEXTYPE)OMX_QcomIndexParamVideoMetaBufferMode,
            (OMX_PTR)&sMetadataMode);
        FAILED1(result, "Error OMX_QcomIndexParamVideoEncodeMetaBufferMode");
    }

    OMX_PARAM_PORTDEFINITIONTYPE sPortDef;
    OMX_INIT_STRUCT(&sPortDef, OMX_PARAM_PORTDEFINITIONTYPE);
    sPortDef.nPortIndex = ePortIndex;
    result = OMX_GetParameter(m_hEncoder, OMX_IndexParamPortDefinition, &sPortDef);
    FAILED1(result, "error get OMX_IndexParamPortDefinition");

    if (sPortDef.nBufferCountActual > nBufferCount) {
        VTEST_MSG_ERROR("alloc buffer count to small (%u vs %u)",
                        (unsigned int)sPortDef.nBufferCountActual, (unsigned int)nBufferCount);
        return OMX_ErrorBadParameter;
    }

    sPortDef.nBufferCountActual = nBufferCount;
    sPortDef.nBufferSize = nBufferSize;
    OMX_INIT_STRUCT_SIZE(&sPortDef, OMX_PARAM_PORTDEFINITIONTYPE);
    result = OMX_SetParameter(m_hEncoder, OMX_IndexParamPortDefinition, (OMX_PTR)&sPortDef);
    FAILED1(result, "error set OMX_IndexParamPortDefinition");

    // In meta-mode the OMX_comp allocates a small structure, that we fill in
    for (OMX_U32 i = 0; i < nBufferCount; i++) {

        BufferInfo *pBuffer = &((*pBuffers)[i]);
        OMX_BUFFERHEADERTYPE **pHeaderPtr = pBuffer->GetHeaderPtr(ePortIndex);

        if (m_bMetaMode) {
            result = OMX_AllocateBuffer(m_hEncoder, pHeaderPtr,
                    ePortIndex, this, sizeof(MetaBuffer));

            MetaBuffer *pMetaBuffer = (MetaBuffer *)((*pHeaderPtr)->pBuffer);
            pMetaBuffer->meta_handle = (BufferHandle)pBuffer->pHandle;
            pMetaBuffer->buffer_type = m_eMetaBufferType;
        } else {
            result = OMX_AllocateBuffer(m_hEncoder, pHeaderPtr,
                    ePortIndex, this, nBufferSize);
        }

        VTEST_MSG_HIGH("%s use_ct=%u sz=%u handle=%p hdr=(%p %p)",
                OMX_PORT_NAME(ePortIndex), (unsigned int)i+1, (unsigned int)nBufferSize,
                (void *)pBuffer->pHandle, pBuffer->pHeaderIn, pBuffer->pHeaderOut);
    }

    if (ePortIndex == PORT_INDEX_IN) {
        m_nInputBufferCount = nBufferCount;
        m_nInputBufferSize = nBufferSize;
    } else if (ePortIndex == PORT_INDEX_OUT) {
        m_nOutputBufferCount = nBufferCount;
        m_nOutputBufferSize = nBufferSize;
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Encoder::PortReconfig(OMX_U32 ePortIndex,
        OMX_U32 nWidth, OMX_U32 nHeight, const OMX_CONFIG_RECTTYPE& sRect) {

    OMX_ERRORTYPE result = OMX_ErrorNone;
    CmdType cmd;

    VTEST_MSG_LOW("PortReconfig %s", OMX_PORT_NAME(ePortIndex));

    if (ePortIndex != PORT_INDEX_IN) {
        VTEST_MSG_ERROR("Error output port reconfig not currently supported");
        return OMX_ErrorUndefined;
    }

    // Don't do anything on crop change
    if ((m_nFrameWidth == nWidth) && (m_nFrameHeight == nHeight)) {
        VTEST_MSG_MEDIUM("Got Crop change, ignored");
        return OMX_ErrorNone;
    }

    VTEST_MSG_MEDIUM("PortReconfig Calling Flush on both ports");
    Flush(OMX_ALL);

    // disable ports, Encoder work-around requires we enable/disable both ports
    m_bPortReconfig = OMX_TRUE;
    VTEST_MSG_MEDIUM("PortReconfig OMX_CommandPortDisable");
    result = OMX_SendCommand(m_hEncoder, OMX_CommandPortDisable, PORT_INDEX_IN, 0);
    FAILED1(result, "Disable Input Port failed");
    result = OMX_SendCommand(m_hEncoder, OMX_CommandPortDisable, PORT_INDEX_OUT, 0);
    FAILED1(result, "Disable Output Port failed");

    // Wait for OMX_comp/sink to return all buffers
    BufferInfo *pBuffer = NULL;
    OMX_U32 nFreeBufferCount = 0;
    while (m_pFreeBufferQueue->GetSize() > 0) {

        result = m_pFreeBufferQueue->Pop(&pBuffer, sizeof(pBuffer), 0);
        if ((pBuffer == NULL) || (result != OMX_ErrorNone)) {
            VTEST_MSG_ERROR("Error in getting buffer, it might be null");
        } else {
            VTEST_MSG_MEDIUM("PortReconfig free buffer count %u",
                    (unsigned int)nFreeBufferCount);
        }
        nFreeBufferCount++;
    }

    // delete output buffers
    VTEST_MSG_MEDIUM("PortReconfig free OUTPUT buffers");
    m_pBufferManager->FreeBuffers(this, PORT_INDEX_OUT);

    VTEST_MSG_MEDIUM("PortReconfig getting response");
    if (m_pSignalQueue->Pop(&cmd, sizeof(cmd), 0) != OMX_ErrorNone) {
        FAILED1(result, "Error popping signal");
    }
    if (cmd.eResult != OMX_ErrorNone || cmd.eEvent != OMX_EventCmdComplete ||
            cmd.eCmd != OMX_CommandPortDisable) {
        result = OMX_ErrorUndefined;
        FAILED1(result, "expecting complete for command : %d", OMX_CommandPortDisable);
    }
    if (m_pSignalQueue->Pop(&cmd, sizeof(cmd), 0) != OMX_ErrorNone) {
        FAILED1(result, "Error popping signal");
    }
    if (cmd.eResult != OMX_ErrorNone || cmd.eEvent != OMX_EventCmdComplete ||
            cmd.eCmd != OMX_CommandPortDisable) {
        result = OMX_ErrorUndefined;
        FAILED1(result, "expecting complete for command : %d", OMX_CommandPortDisable);
    }

    VTEST_MSG_MEDIUM("PortReconfig got new settings");
    PortBufferCapability req = GetBufferRequirements(PORT_INDEX_OUT);
    OMX_U32 nBufferCount = m_nInputBufferCount;

    // get updated port parameters
    result = SetPortParams(PORT_INDEX_IN,
            nWidth, nHeight, nBufferCount, 0, //FRAME-RATE not used for input port
            &m_nInputBufferSize, &m_nInputBufferCount);
    VTEST_MSG_MEDIUM("PortReconfig Min Buffer Count = %u", (unsigned int)m_nInputBufferCount);
    VTEST_MSG_MEDIUM("PortReconfig Buffer Size = %u", (unsigned int)m_nInputBufferSize);

    VTEST_MSG_MEDIUM("PortReconfig restarting sink");
    result = m_pSink->PortReconfig(PORT_INDEX_IN,
            m_sConfig.nOutputFrameWidth, m_sConfig.nOutputFrameHeight, sRect);
    FAILED1(result, "PortReconfig done failed on sink");

    // enable ports, Encoder work-around requires we enable/disable both ports
    VTEST_MSG_MEDIUM("PortReconfig re-enabling port");
    OMX_SendCommand(m_hEncoder, OMX_CommandPortEnable, PORT_INDEX_IN, 0);
    FAILED1(result, "Enable Input Port failed");
    OMX_SendCommand(m_hEncoder, OMX_CommandPortEnable, PORT_INDEX_OUT, 0);
    FAILED1(result, "Enable Output Port failed");
    m_bPortReconfig = OMX_FALSE;

    // re-allocate output buffers
    VTEST_MSG_MEDIUM("PortReconfig re-allocate OUTPUT buffers");
    result = m_pBufferManager->SetupBufferPool(this, m_pSink);
    FAILED1(result, "Error in realloacting buffer pool");

    /* no need to wait for enable port complete, as input port will only
     * be enabled once BufferPool is setup by the source */

    m_bPortReconfig = OMX_FALSE;
    BufferInfo *pBuffers;
    result = m_pBufferManager->GetBuffers(
            this, PORT_INDEX_OUT, &pBuffers, &nBufferCount);
    FAILED1(result, "Error in Getting Buffers");

    for (OMX_U32 i = 0; i < nBufferCount; i++) {
        VTEST_MSG_MEDIUM("port-reconfig requeue buffer %lu (%p %p)",
                pBuffers[i].pHandle,
                pBuffers[i].pHeaderIn, pBuffers[i].pHeaderOut);
        result = SetBuffer(&pBuffers[i], m_pSink);
        FAILED1(result, "Error setting port buffer %p", pBuffers[i].pHeaderOut);
    }

    VTEST_MSG_MEDIUM("port-reconfig done");
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Encoder::ThreadRun(OMX_PTR pThreadData) {

    (void)pThreadData;
    OMX_ERRORTYPE result = OMX_ErrorNone;
    OMX_TICKS nTimeStamp = 0;
    OMX_U32 i;

    while (!m_bThreadStop) {
        Sleeper::Sleep(100);
    }

    VTEST_MSG_HIGH("thread exiting...");
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Encoder::FreeBuffer(
        BufferInfo *pBuffer, OMX_U32 ePortIndex) {

    OMX_ERRORTYPE result = OMX_ErrorNone;

    OMX_BUFFERHEADERTYPE **pHeaderPtr = pBuffer->GetHeaderPtr(ePortIndex);
    if (*pHeaderPtr == NULL) {
        return result;
    }

    VTEST_MSG_MEDIUM("Freeing buffer %p 0x%lu",
            *pHeaderPtr, pBuffer->pHandle);

    result = OMX_FreeBuffer(m_hEncoder, ePortIndex, *pHeaderPtr);
    FAILED1(result, "Error freeing %s", OMX_PORT_NAME(ePortIndex));

    *pHeaderPtr = NULL;
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Encoder::SetState(
        OMX_STATETYPE eState, OMX_BOOL bSynchronous) {

    OMX_ERRORTYPE result = OMX_ErrorNone;

    /*if we get an error before idle complete (allocation failure) and
     * we try to move to loaded in the destructor*/
    if (eState == m_eState) {
        return result;
    }

    // check for pending state transition
    if (m_eState != m_eStatePending) {
        result = WaitState(m_eStatePending);
        FAILED1(result, "wait for %s failed, waiting for alloc/dealloc?!?!",
            OMX_STATE_NAME(m_eStatePending));
    }

    // check for invalid transition
    if ((eState == OMX_StateLoaded && m_eState != OMX_StateIdle) ||
        (eState == OMX_StateExecuting && m_eState != OMX_StateIdle)) {
        VTEST_MSG_ERROR(
            "Error: invalid state tranisition: state %s to %s",
            OMX_STATE_NAME(eState), OMX_STATE_NAME(m_eState));
        result = OMX_ErrorIncorrectStateTransition;
    }

    VTEST_MSG_MEDIUM("goto state %s...", OMX_STATE_NAME(eState));
    result = OMX_SendCommand(m_hEncoder,
            OMX_CommandStateSet, (OMX_U32)eState, NULL);
    m_eStatePending = eState;

    FAILED1(result, "failed to set state");
    if (bSynchronous == OMX_TRUE) {
        result = WaitState(eState);
        FAILED1(result, "failed to wait in set state in synchronous mode");
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Encoder::WaitState(OMX_STATETYPE eState) {

    OMX_ERRORTYPE result = OMX_ErrorNone;
    CmdType cmd;

    VTEST_MSG_MEDIUM("waiting for state %s...", OMX_STATE_NAME(eState));
    (void)m_pSignalQueue->Pop(&cmd, sizeof(cmd), 0); // infinite wait
    result = cmd.eResult;

    if (cmd.eEvent == OMX_EventCmdComplete) {
        if (cmd.eCmd == OMX_CommandStateSet) {
            if ((OMX_STATETYPE)cmd.sCmdData == eState) {
                m_eState = (OMX_STATETYPE)cmd.sCmdData;
            } else {
                VTEST_MSG_ERROR("wrong state (%d)", (int)cmd.sCmdData);
                result = OMX_ErrorUndefined;
            }
        } else {
            VTEST_MSG_ERROR("expecting state change");
            result = OMX_ErrorUndefined;
        }
    } else {
        VTEST_MSG_ERROR("expecting cmd complete");
        result = OMX_ErrorUndefined;
    }

    if (result == OMX_ErrorNone) {
        VTEST_MSG_MEDIUM("reached state %s...", OMX_STATE_NAME(eState));
    }
    return result;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Encoder::SetIDRPeriod(OMX_S32 nIDRPeriod, OMX_S32 nPFrames) {

    OMX_ERRORTYPE result = OMX_ErrorNone;
    OMX_VIDEO_CONFIG_AVCINTRAPERIOD idr_period;
    OMX_INIT_STRUCT(&idr_period, OMX_VIDEO_CONFIG_AVCINTRAPERIOD);

    idr_period.nPortIndex = (OMX_U32)PORT_INDEX_OUT;
    result = OMX_GetConfig(m_hEncoder,
                           (OMX_INDEXTYPE)OMX_IndexConfigVideoAVCIntraPeriod,
                           (OMX_PTR) & idr_period);
    if (result != OMX_ErrorNone) {
        VTEST_MSG_ERROR("failed to get state");
        return result;
    }

    OMX_INIT_STRUCT(&idr_period, OMX_VIDEO_CONFIG_AVCINTRAPERIOD);
    idr_period.nPFrames = (OMX_U32)nPFrames;
    idr_period.nIDRPeriod = (OMX_U32)nIDRPeriod;
    OMX_INIT_STRUCT_SIZE(&idr_period, OMX_VIDEO_CONFIG_AVCINTRAPERIOD);
    result = OMX_SetConfig(m_hEncoder,
            (OMX_INDEXTYPE)OMX_IndexConfigVideoAVCIntraPeriod,
            (OMX_PTR)&idr_period);
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Encoder::DeliverInput(OMX_BUFFERHEADERTYPE *pBuffer) {

    OMX_ERRORTYPE result = OMX_ErrorNone;

    VTEST_MSG_MEDIUM("DeliverInput for frame#%d", (OMX_U32)m_nInBufferCount);

    while (m_sConfig.pDynamicProperties != NULL &&
           ((OMX_U32)m_nInBufferCount-1) == m_sConfig.pDynamicProperties[m_nDynamicIndexCount].frame_num) {
        VTEST_MSG_MEDIUM("Processing dyn prop for frame#%d", (OMX_U32)m_nInBufferCount);
        ProcessDynamicConfiguration(m_sConfig.pDynamicProperties, m_nDynamicIndexCount);
        m_nDynamicIndexCount++;
    }

    ProcessRoi();
    VTEST_MSG_ERROR("Buffer Timestamp = %lld", pBuffer->nTimeStamp);;
    return OMX_EmptyThisBuffer(m_hEncoder, pBuffer);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Encoder::ReadRoiQp(OMX_QTI_VIDEO_CONFIG_ROIINFO &roiInfo) {
    int frameNum;
    int numberOfMbs = 0;
    if (m_eCodec == OMX_VIDEO_CodingAVC) {
        numberOfMbs = ((m_nFrameWidth + 15) >> 4) * ((m_nFrameHeight + 15) >> 4);
    } else if (m_eCodec == OMX_VIDEO_CodingHEVC) {
        numberOfMbs = ((m_nFrameWidth + 31) >> 5) * ((m_nFrameHeight + 31) >> 5);
    } else {
        return OMX_ErrorUnsupportedSetting;
    }
    int numOfBytes = ((numberOfMbs * 2 + 7) >> 3);
    int rc = 0;
    OMX_ERRORTYPE ret = OMX_ErrorNone;
    if (!mRoiQpFile) {
        return OMX_ErrorUndefined;
    }
    if (!mRoiQpBuf) {
        mRoiQpBuf = (char *)malloc(numOfBytes);
    }
    if (!mRoiQpBuf) {
        VTEST_MSG_ERROR("Failed to allocate QP buffer");
        return OMX_ErrorUndefined;
    }
    size_t posBeforeFramenum = ftell(mRoiQpFile);
    rc = fread(&frameNum, 1, 4, mRoiQpFile);
    if (rc != 4) {
        goto reachedEOF;
    }
    roiInfo.nPortIndex = PORT_INDEX_IN;
    if (frameNum == m_nInBufferCount - 1) {
        rc = fread(&roiInfo.nLowerQpOffset, 1, 4, mRoiQpFile);
        if (rc != 4)
            goto reachedEOF;
        rc = fread(&roiInfo.nUpperQpOffset, 1, 4, mRoiQpFile);
        if (rc != 4)
            goto reachedEOF;
        rc = fread(mRoiQpBuf, 1, numOfBytes, mRoiQpFile);
        if (rc != numOfBytes) {
            VTEST_MSG_ERROR("RC = %d", rc);
            goto reachedEOF;
        }
        roiInfo.pRoiMBInfo = mRoiQpBuf;
        roiInfo.nRoiMBInfoSize = numOfBytes;
        roiInfo.bUseRoiInfo = OMX_TRUE;
        ret = OMX_ErrorNone;
    } else {
        fseek(mRoiQpFile, posBeforeFramenum, SEEK_SET);
        roiInfo.pRoiMBInfo = NULL;
        roiInfo.nRoiMBInfoSize = 0;
        roiInfo.nPortIndex = PORT_INDEX_IN;
        roiInfo.bUseRoiInfo = OMX_FALSE;
        ret = OMX_ErrorBadParameter;
    }
    return ret;
reachedEOF:
    fclose(mRoiQpFile);
    mRoiQpFile = NULL;
    return OMX_ErrorBadParameter;
}

OMX_ERRORTYPE Encoder::ProcessRoi()
{
    OMX_INDEXTYPE index;
    OMX_ERRORTYPE rc = OMX_ErrorNone;
    OMX_STRING name;
    name = const_cast<OMX_STRING>(
                 OMX_QTI_INDEX_CONFIG_VIDEO_ROIINFO);
    rc = OMX_GetExtensionIndex(m_hEncoder, name, &index);
    if (rc != OMX_ErrorNone) {
        VTEST_MSG_ERROR("Failed to get OMX_QTI_INDEX_CONFIG_VIDEO_ROIINFO index");
        return rc;
    }
    OMX_QTI_VIDEO_CONFIG_ROIINFO roiInfo;
    rc = ReadRoiQp(roiInfo);
    if (rc == OMX_ErrorNone) {
        VTEST_MSG_ERROR("Setting ROI config: %d, %d", roiInfo.nUpperQpOffset, roiInfo.nLowerQpOffset);
        rc = OMX_SetConfig(m_hEncoder, index, &roiInfo);
        if (rc != OMX_ErrorNone) {
            VTEST_MSG_ERROR("Failed to get OMX_QTI_INDEX_CONFIG_VIDEO_ROIINFO index");
        }
    }
    return rc;
}

OMX_ERRORTYPE Encoder::ProcessDynamicConfiguration(
        DynamicConfig *dynamic_config, int index) {

    OMX_ERRORTYPE result = OMX_ErrorNone;

    VTEST_MSG_MEDIUM("Processing dynamic config 0x%x frame# %d",
        dynamic_config[index].config_param, dynamic_config[index].frame_num);

    if (dynamic_config[index].config_param == OMX_IndexConfigVideoFramerate) {
        OMX_S32 framerate = (OMX_S32) dynamic_config[index].config_data.f_framerate;

        VTEST_MSG_MEDIUM("Dyn frame rate = %d", framerate);
        result = ChangeFrameRate(framerate);
        if (result) {
            VTEST_MSG_ERROR("Could not update the framerate");
        }
    }
    if (dynamic_config[index].config_param ==
            (OMX_INDEXTYPE)QOMX_IndexConfigVideoIntraperiod) {
        QOMX_VIDEO_INTRAPERIODTYPE intra_period;
        OMX_INIT_STRUCT(&intra_period, QOMX_VIDEO_INTRAPERIODTYPE);
        intra_period.nPortIndex = PORT_INDEX_OUT;
        result = OMX_GetConfig(m_hEncoder,
                dynamic_config[index].config_param,
                (OMX_PTR)&intra_period);
        if (intra_period.nBFrames) {
            OMX_S32 ratio = intra_period.nBFrames / intra_period.nPFrames;
            dynamic_config[index].config_data.intraperiod.nBFrames =
                ratio * dynamic_config[index].config_data.intraperiod.nPFrames;
        }
        VTEST_MSG_HIGH("Old: P/B frames = %u/%u",
                (unsigned int)intra_period.nPFrames, (unsigned int)intra_period.nBFrames);
        VTEST_MSG_HIGH("New: P/B frames = %u/%u",
                (unsigned int)dynamic_config[index].config_data.intraperiod.nPFrames,
                (unsigned int)dynamic_config[index].config_data.intraperiod.nBFrames);
    }

    result = SetDynamicConfig(dynamic_config[index].config_param, &dynamic_config[index].config_data);

    if (result == OMX_ErrorNone) {
        VTEST_MSG_MEDIUM(
            "Set Dynamic Config Successfully. Frame count: %d. Config Index: %x",
            dynamic_config[index].frame_num,
            dynamic_config[index].config_param);
    } else {
        VTEST_MSG_ERROR(
            "Failed to set Dynamic Config. Frame count: %d. Config Index: %x",
            dynamic_config[index].frame_num,
            dynamic_config[index].config_param);
    }
    return  result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Encoder::SetDynamicConfig(OMX_INDEXTYPE config_param, DynamicConfigData* config_data) {

    OMX_ERRORTYPE result = OMX_ErrorNone;

    switch (config_param) {
        case OMX_IndexConfigVideoIntraVOPRefresh:
            result = RequestIntraVOP();
            break;
        case OMX_IndexConfigVideoBitrate:
            OMX_INIT_STRUCT_SIZE(&config_data->bitrate, OMX_VIDEO_CONFIG_BITRATETYPE);
            result = OMX_SetConfig(m_hEncoder, config_param, (OMX_PTR) &config_data->bitrate);
            break;
        default:
            break;
    }

    OMX_QCOM_EXTN_INDEXTYPE config_param_qcom_ext = (OMX_QCOM_EXTN_INDEXTYPE) config_param;

    switch(config_param_qcom_ext) {
        case OMX_QcomIndexConfigVideoLTRUse:
            OMX_INIT_STRUCT_SIZE(&config_data->useltr, OMX_QCOM_VIDEO_CONFIG_LTRUSE_TYPE);
            result = OMX_SetConfig(m_hEncoder,
                     (OMX_INDEXTYPE) config_param_qcom_ext, (OMX_PTR) &config_data->useltr);
            break;
        case OMX_QcomIndexConfigVideoLTRMark:
            OMX_INIT_STRUCT_SIZE(&config_data->markltr, OMX_QCOM_VIDEO_CONFIG_LTRMARK_TYPE);
            result = OMX_SetConfig(m_hEncoder,
                     (OMX_INDEXTYPE) config_param_qcom_ext, (OMX_PTR) &config_data->markltr);
            break;
        case OMX_QcomIndexConfigVideoVencPerfMode:
            break;
        case OMX_QcomIndexConfigPictureTypeDecode:
            break;
        case OMX_QcomIndexConfigQp:
            OMX_SKYPE_VIDEO_CONFIG_QP configQp;
            OMX_INIT_STRUCT(&configQp, OMX_SKYPE_VIDEO_CONFIG_QP);
            configQp.nQP = (OMX_U32)config_data->configqp.nQP;

            result = OMX_SetConfig(m_hEncoder,
                    (OMX_INDEXTYPE) config_param_qcom_ext, (OMX_PTR) &configQp);
            if (result != OMX_ErrorNone) {
                VTEST_MSG_ERROR("Failed: Setting QP with error: %d", result);
            }
            break;
        case OMX_QcomIndexHierarchicalStructure:
            break;
        case OMX_QcomIndexConfigNumHierPLayers:
            QOMX_EXTNINDEX_VIDEO_HIER_P_LAYERS configHierP;
            OMX_INIT_STRUCT(&configHierP, QOMX_EXTNINDEX_VIDEO_HIER_P_LAYERS);
            configHierP.nNumHierLayers = (OMX_U32)config_data->confighierp.nNumLayers;

            result = OMX_SetConfig(m_hEncoder,
                    (OMX_INDEXTYPE) config_param_qcom_ext, (OMX_PTR) &configHierP);
            if (result != OMX_ErrorNone) {
                VTEST_MSG_ERROR("Failed: Setting HierPLayers with error: %d", result);
            }
            break;
        case QOMX_IndexConfigVideoIntraperiod:
            QOMX_VIDEO_INTRAPERIODTYPE intra;
            OMX_INIT_STRUCT(&intra, QOMX_VIDEO_INTRAPERIODTYPE);
            intra.nPFrames = (OMX_U32)config_data->intraperiod.nPFrames;
            intra.nBFrames = (OMX_U32)config_data->intraperiod.nBFrames;
            intra.nPortIndex = (OMX_U32)config_data->intraperiod.nPortIndex;

            result = OMX_SetConfig(m_hEncoder,
                    (OMX_INDEXTYPE) config_param_qcom_ext, (OMX_PTR) &intra);
            if (result != OMX_ErrorNone) {
                VTEST_MSG_ERROR("Failed: Setting Intraperiod with error: %d", result);
            }
            break;
        case OMX_QcomIndexConfigBaseLayerId:
            break;
        default:
            break;
    }

    OMX_INDEXEXTTYPE config_param_ext = (OMX_INDEXEXTTYPE) config_param;

    switch (config_param_ext) {
        case OMX_IndexConfigVideoVp8ReferenceFrame:
            OMX_INIT_STRUCT_SIZE(&config_data->vp8refframe, OMX_VIDEO_VP8REFERENCEFRAMETYPE);
            result = OMX_SetConfig(m_hEncoder,
                     (OMX_INDEXTYPE) config_param_qcom_ext, (OMX_PTR) &config_data->markltr);
            break;
        default:
            VTEST_MSG_ERROR("Dynamic property not yet implemented");
            break;
    }

    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Encoder::DeliverOutput(OMX_BUFFERHEADERTYPE *pBuffer) {

    pBuffer->nFlags = 0;
    return OMX_FillThisBuffer(m_hEncoder, pBuffer);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Encoder::RequestIntraVOP() {

    OMX_ERRORTYPE result = OMX_ErrorNone;
    OMX_CONFIG_INTRAREFRESHVOPTYPE vop;
    OMX_INIT_STRUCT(&vop, OMX_CONFIG_INTRAREFRESHVOPTYPE);

    vop.IntraRefreshVOP = OMX_TRUE;
    vop.nPortIndex = (OMX_U32)PORT_INDEX_OUT;
    OMX_INIT_STRUCT_SIZE(&vop, OMX_CONFIG_INTRAREFRESHVOPTYPE);
    result = OMX_SetConfig(m_hEncoder,
            OMX_IndexConfigVideoIntraVOPRefresh, (OMX_PTR)&vop);
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Encoder::SetIntraRefresh(OMX_VIDEO_INTRAREFRESHTYPE nIRMode, OMX_S32 nIntraRefresh) {

    OMX_ERRORTYPE result = OMX_ErrorNone;
    OMX_VIDEO_PARAM_INTRAREFRESHTYPE intrarefresh;
    OMX_INIT_STRUCT(&intrarefresh, OMX_VIDEO_PARAM_INTRAREFRESHTYPE);

    intrarefresh.nPortIndex = (OMX_U32)PORT_INDEX_OUT;
    result = OMX_GetParameter(m_hEncoder,
            (OMX_INDEXTYPE)OMX_IndexParamVideoIntraRefresh,
            (OMX_PTR)&intrarefresh);

    if (result != OMX_ErrorNone) {
        VTEST_MSG_ERROR("failed to get IntraRefresh");
        return result;
    }
    intrarefresh.eRefreshMode = nIRMode;
    intrarefresh.nCirMBs = nIntraRefresh;
    OMX_INIT_STRUCT_SIZE(&intrarefresh, OMX_VIDEO_PARAM_INTRAREFRESHTYPE);
    result = OMX_SetParameter(m_hEncoder,
            (OMX_INDEXTYPE)OMX_IndexParamVideoIntraRefresh,
            (OMX_PTR)&intrarefresh);
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Encoder::SetIntraPeriod(OMX_S32 nIntraPeriod) {

    OMX_ERRORTYPE result = OMX_ErrorNone;
    QOMX_VIDEO_INTRAPERIODTYPE intra;
    OMX_INIT_STRUCT(&intra, QOMX_VIDEO_INTRAPERIODTYPE);

    intra.nPortIndex = (OMX_U32)PORT_INDEX_OUT;
    result = OMX_GetConfig(m_hEncoder,
            (OMX_INDEXTYPE)QOMX_IndexConfigVideoIntraperiod,
            (OMX_PTR)&intra);

    if (result != OMX_ErrorNone) {
        VTEST_MSG_ERROR("failed to get state");
        return result;
    }
    intra.nPFrames = (OMX_U32)nIntraPeriod - 1;
    result = OMX_SetConfig(m_hEncoder,
            (OMX_INDEXTYPE)QOMX_IndexConfigVideoIntraperiod,
            (OMX_PTR)&intra);

    FAILED1(result, "Failed to SetConfig - QOMX_IndexConfigVideoIntraperiod %d", result);
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Encoder::SetVencPerfMode(OMX_U32 nPerfMode) {

    OMX_ERRORTYPE result = OMX_ErrorNone;
    QOMX_EXTNINDEX_VIDEO_PERFMODE pMode;
    OMX_INIT_STRUCT(&pMode, QOMX_EXTNINDEX_VIDEO_PERFMODE);

    pMode.nPerfMode = nPerfMode;

    result = OMX_SetConfig(m_hEncoder,
                 (OMX_INDEXTYPE) OMX_QcomIndexConfigVideoVencPerfMode,
                 (OMX_PTR)&pMode);
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Encoder::SetHierPNumLayers() {

    OMX_ERRORTYPE result = OMX_ErrorNone;

    if (m_sConfig.preferredOmxIndex.IndexParamAndroidVideoTemporalLayers) {

        OMX_U32 nHierPNumLayers =
            m_sConfig.sHybridHP.nHpLayers ? m_sConfig.sHybridHP.nHpLayers : m_sConfig.nHierPNumLayers;
        OMX_VIDEO_PARAM_ANDROID_TEMPORALLAYERINGTYPE layerParams;
        OMX_U32 cumulativeBitrate = 0;
        OMX_INIT_STRUCT(&layerParams, OMX_VIDEO_PARAM_ANDROID_TEMPORALLAYERINGTYPE);
        layerParams.nSize = sizeof(OMX_VIDEO_PARAM_ANDROID_TEMPORALLAYERINGTYPE);
        layerParams.nPortIndex = PORT_INDEX_OUT;
        result = OMX_GetParameter(m_hEncoder,
                (OMX_INDEXTYPE)OMX_IndexParamAndroidVideoTemporalLayering,
                &layerParams);
        FAILED1(result, "Error: Get OMX_VIDEO_PARAM_ANDROID_TEMPORALLAYERINGTYPE");

        layerParams.nPLayerCountActual = nHierPNumLayers;
        layerParams.nBLayerCountActual = 0;
        layerParams.bBitrateRatiosSpecified =
            m_sConfig.nRcDistribution[0] != 0 ? OMX_TRUE : OMX_FALSE;
        layerParams.ePattern = OMX_VIDEO_AndroidTemporalLayeringPatternAndroid;

        if (layerParams.bBitrateRatiosSpecified == OMX_TRUE) {
            for (unsigned int i = 0; i < nHierPNumLayers; i++) {
                layerParams.nBitrateRatios[i] = m_sConfig.nRcDistribution[i] + cumulativeBitrate;
                cumulativeBitrate = layerParams.nBitrateRatios[i];
                VTEST_MSG_LOW("Layer Bit-rate setting to Component : Layer =  %d Bit Rate = %d",
                        i, layerParams.nBitrateRatios[i]);
            }
        }
        OMX_INIT_STRUCT_SIZE(&layerParams, OMX_VIDEO_PARAM_ANDROID_TEMPORALLAYERINGTYPE);
        result = OMX_SetParameter(m_hEncoder,
                (OMX_INDEXTYPE)OMX_IndexParamAndroidVideoTemporalLayering,
                &layerParams);
        FAILED1(result, "Error: Set OMX_IndexParamAndroidVideoTemporalLayering");

    } else if (m_sConfig.sHybridHP.nHpLayers) {

        result = SetHybridHp();
        FAILED1(result, "Failed : Setting Hybrid HP for H264");

    } else if (m_sConfig.nHierPNumLayers) {

        QOMX_VIDEO_HIERARCHICALLAYERS hierP;
        OMX_INIT_STRUCT(&hierP, QOMX_VIDEO_HIERARCHICALLAYERS);
        hierP.nPortIndex = (OMX_U32) PORT_INDEX_OUT;
        hierP.nNumLayers = m_sConfig.nHierPNumLayers;
        hierP.eHierarchicalCodingType = QOMX_HIERARCHICALCODING_P;
        VTEST_MSG_HIGH("SetParmater nHierPNumLayers = %d", m_sConfig.nHierPNumLayers);
        result = OMX_SetParameter(m_hEncoder,
                (OMX_INDEXTYPE)OMX_QcomIndexHierarchicalStructure,
                (OMX_PTR) &hierP);
        FAILED1(result, "Error: Set OMX_QcomIndexHierarchicalStructure");
    }

    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Encoder::SetBitrate(OMX_VIDEO_CONTROLRATETYPE eControlRate, OMX_S32 nBitrate) {

    OMX_ERRORTYPE result = OMX_ErrorNone;
    OMX_VIDEO_PARAM_BITRATETYPE bitrate;
    OMX_INIT_STRUCT(&bitrate, OMX_VIDEO_PARAM_BITRATETYPE);
    bitrate.nPortIndex = (OMX_U32)PORT_INDEX_OUT;
    result = OMX_GetParameter(m_hEncoder,
            OMX_IndexParamVideoBitrate, (OMX_PTR)&bitrate);
    if (result != OMX_ErrorNone) {
        return result;
    }

    bitrate.eControlRate = eControlRate;
    bitrate.nTargetBitrate = nBitrate;
    OMX_INIT_STRUCT_SIZE(&bitrate, OMX_VIDEO_PARAM_BITRATETYPE);
    result = OMX_SetParameter(m_hEncoder,
            OMX_IndexParamVideoBitrate, (OMX_PTR)&bitrate);

    VTEST_MSG_HIGH("SetParmater Bitrate = %d", nBitrate);

    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Encoder::SetSarInfo(OMX_U32 nSarWidth, OMX_U32 nSarHeight) {

    OMX_ERRORTYPE result = OMX_ErrorNone;
    QOMX_EXTNINDEX_VIDEO_VENC_SAR sar;
    sar.nSize = sizeof(QOMX_EXTNINDEX_VIDEO_VENC_SAR);
    sar.nSARWidth = nSarWidth;
    sar.nSARHeight = nSarHeight;
    VTEST_MSG_HIGH("SetParmater sar: %d x %d", nSarWidth, nSarHeight);
    result = OMX_SetParameter(m_hEncoder,
            (OMX_INDEXTYPE)OMX_QcomIndexParamVencAspectRatio,
            (OMX_PTR) &sar);
    FAILED1(result, "Error: Set OMX_QcomIndexParamVencAspectRatio");
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Encoder::SetPqDisable() {

    OMX_ERRORTYPE result = OMX_ErrorNone;
    QOMX_DISABLETYPE pqDisable;
    OMX_INIT_STRUCT(&pqDisable, QOMX_DISABLETYPE);

    result = OMX_GetParameter(m_hEncoder,
            (OMX_INDEXTYPE)OMX_QTIIndexParamDisablePQ, (OMX_PTR)&pqDisable);
    VTEST_MSG_HIGH("GetParmater PQ disable: %u", pqDisable.bDisable);
    pqDisable.bDisable = m_sConfig.nDisablePQ;
    VTEST_MSG_HIGH("SetParmater PQ disable: %u", pqDisable.bDisable);
    result = OMX_SetParameter(m_hEncoder,
            (OMX_INDEXTYPE)OMX_QTIIndexParamDisablePQ,
            (OMX_PTR) &pqDisable);
    FAILED1(result, "Error: Set OMX_QTIIndexParamDisablePQ");
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Encoder::SetPortParams(OMX_U32 ePortIndex,
        OMX_U32 nWidth, OMX_U32 nHeight, OMX_U32 nBufferCountMin,
        OMX_U32 nFrameRate, OMX_U32 *nBufferSize, OMX_U32 *nBufferCount) {

    OMX_ERRORTYPE result = OMX_ErrorNone;
    OMX_PARAM_PORTDEFINITIONTYPE sPortDef;
    OMX_INIT_STRUCT(&sPortDef, OMX_PARAM_PORTDEFINITIONTYPE);

    if ((nBufferSize == NULL) || (nBufferCount == NULL)) {
        VTEST_MSG_ERROR("%s - invalid input", __FUNCTION__);
        return OMX_ErrorBadParameter;
    }

    sPortDef.nPortIndex = ePortIndex;
    result = OMX_GetParameter(m_hEncoder,
            OMX_IndexParamPortDefinition, (OMX_PTR)&sPortDef);
    FAILED1(result, "Error: GET OMX_IndexParamPortDefinition");

    FractionToQ16(sPortDef.format.video.xFramerate,(int)(nFrameRate * 2), 2);

    // setup frame width/height
    sPortDef.format.video.nFrameWidth = nWidth;
    sPortDef.format.video.nFrameHeight = nHeight;
    if (ePortIndex == PORT_INDEX_IN) {
        sPortDef.format.video.eColorFormat = (OMX_COLOR_FORMATTYPE)m_nColorFormat;
    }

    OMX_INIT_STRUCT_SIZE(&sPortDef, OMX_PARAM_PORTDEFINITIONTYPE);
    result = OMX_SetParameter(m_hEncoder,
            OMX_IndexParamPortDefinition, (OMX_PTR)&sPortDef);
    FAILED1(result, "Error: SET OMX_IndexParamPortDefinition");

    result = OMX_GetParameter(m_hEncoder,
            OMX_IndexParamPortDefinition, (OMX_PTR)&sPortDef);
    FAILED1(result, "Error: GET OMX_IndexParamPortDefinition");

    if ((sPortDef.format.video.nFrameWidth != nWidth) ||
        (sPortDef.format.video.nFrameHeight != nHeight)) {
        VTEST_MSG_ERROR("width %u != %u or height %u != %u\n",
                (unsigned int)sPortDef.format.video.nFrameWidth, (unsigned int)nWidth,
                (unsigned int)sPortDef.format.video.nFrameHeight, (unsigned int)nHeight);
    }

    // setup buffer count
    if (nBufferCountMin < sPortDef.nBufferCountMin) {
        VTEST_MSG_HIGH("nBufferCount %u too small overriding to %u",
            (unsigned int)nBufferCountMin, (unsigned int)sPortDef.nBufferCountMin);
        nBufferCountMin = sPortDef.nBufferCountMin;
    }
    sPortDef.nBufferCountActual = sPortDef.nBufferCountMin = nBufferCountMin;

    OMX_INIT_STRUCT_SIZE(&sPortDef, OMX_PARAM_PORTDEFINITIONTYPE);
    result = OMX_SetParameter(m_hEncoder,
            OMX_IndexParamPortDefinition, (OMX_PTR)&sPortDef);
    FAILED1(result, "Error: GET OMX_IndexParamPortDefinition");

    result = OMX_GetParameter(m_hEncoder,
            OMX_IndexParamPortDefinition, (OMX_PTR)&sPortDef);
    FAILED1(result, "Error: SET OMX_IndexParamPortDefinition");

    if (nBufferCountMin != sPortDef.nBufferCountActual) {
        VTEST_MSG_ERROR("Buffer reqs dont match...%u != %u\n",
                (unsigned int)nBufferCountMin, (unsigned int)sPortDef.nBufferCountActual);
    }

    VTEST_MSG_HIGH("%s cfg, width %u, height %u, bufs %u, size %u",
            OMX_PORT_NAME(ePortIndex),
            (unsigned int)sPortDef.format.video.nFrameWidth, (unsigned int)sPortDef.format.video.nFrameHeight,
            (unsigned int)sPortDef.nBufferCountActual, (unsigned int)sPortDef.nBufferSize);

    *nBufferCount = sPortDef.nBufferCountActual;
    *nBufferSize = sPortDef.nBufferSize;

    /*Encoder always returns un-aligned size in meta mode*/
    if(m_bSecureSession && m_bMetaMode) {
        *nBufferSize = VTEST_ALIGN(*nBufferSize, SECURE_ALIGN);
        VTEST_MSG_HIGH("Aligned size for secure session : %u",
                (unsigned int)*nBufferSize);
    }
    return result;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Encoder::ChangeFrameRate(OMX_S32 nFramerate) {

    OMX_ERRORTYPE result = OMX_ErrorNone;

    VTEST_MSG_MEDIUM("Changing frame rate to %d",nFramerate);

    OMX_CONFIG_FRAMERATETYPE framerate;
    OMX_INIT_STRUCT(&framerate, OMX_CONFIG_FRAMERATETYPE);
    framerate.nPortIndex = (OMX_U32)PORT_INDEX_IN;
    result = OMX_GetConfig(m_hEncoder,
            OMX_IndexConfigVideoFramerate, (OMX_PTR)&framerate);
    if (result != OMX_ErrorNone) {
        return result;
    }

    FractionToQ16(framerate.xEncodeFramerate, (int)(nFramerate * 2), 2);
    OMX_INIT_STRUCT_SIZE(&framerate, OMX_CONFIG_FRAMERATETYPE);
    result = OMX_SetConfig(m_hEncoder,
            OMX_IndexConfigVideoFramerate, (OMX_PTR)&framerate);

    if (result != OMX_ErrorNone) {
        return result;
    }

    VTEST_MSG_MEDIUM("frame rate set to %d",nFramerate);
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Encoder::ChangeQuality(
        OMX_S32 nFramerate, OMX_S32 nBitrate) {

    OMX_ERRORTYPE result = OMX_ErrorNone;

    //////////////////////////////////////////
    // frame rate
    //////////////////////////////////////////

    OMX_CONFIG_FRAMERATETYPE framerate;
    OMX_INIT_STRUCT(&framerate, OMX_CONFIG_FRAMERATETYPE);
    framerate.nPortIndex = (OMX_U32)PORT_INDEX_IN;
    result = OMX_GetConfig(m_hEncoder,
            OMX_IndexConfigVideoFramerate, (OMX_PTR)&framerate);
    if (result != OMX_ErrorNone) {
        return result;
    }

    FractionToQ16(framerate.xEncodeFramerate, (int)(nFramerate * 2), 2);
    OMX_INIT_STRUCT_SIZE(&framerate, OMX_CONFIG_FRAMERATETYPE);
    result = OMX_SetConfig(m_hEncoder,
            OMX_IndexConfigVideoFramerate, (OMX_PTR)&framerate);
    if (result != OMX_ErrorNone) {
        return result;
    }

    //////////////////////////////////////////
    // bitrate
    //////////////////////////////////////////
    OMX_VIDEO_CONFIG_BITRATETYPE bitrate;
    OMX_INIT_STRUCT(&bitrate, OMX_VIDEO_CONFIG_BITRATETYPE);
    bitrate.nPortIndex = (OMX_U32)PORT_INDEX_OUT;
    result = OMX_GetConfig(m_hEncoder,
            OMX_IndexConfigVideoBitrate, (OMX_PTR)&bitrate);
    if (result != OMX_ErrorNone) {
        VTEST_MSG_ERROR("error changing quality");
        return result;
    }
    bitrate.nEncodeBitrate = (OMX_U32)nBitrate;
    OMX_INIT_STRUCT_SIZE(&bitrate, OMX_VIDEO_CONFIG_BITRATETYPE);
    result = OMX_SetConfig(m_hEncoder,
            OMX_IndexConfigVideoBitrate, (OMX_PTR)&bitrate);
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Encoder::EventCallback(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_PTR pAppData, OMX_IN OMX_EVENTTYPE eEvent,
        OMX_IN OMX_U32 nData1, OMX_IN OMX_U32 nData2, OMX_IN OMX_PTR pEventData) {

    (void)hComponent; (void)pEventData;
    OMX_ERRORTYPE result = OMX_ErrorNone;
    Encoder *pThis = (Encoder *)pAppData;

    {
        Mutex::Autolock autoLock(pThis->m_pLock);

        if (eEvent == OMX_EventCmdComplete) {
            if ((OMX_COMMANDTYPE)nData1 == OMX_CommandStateSet) {
                VTEST_MSG_HIGH("Event callback: state is %s",
                        OMX_STATE_NAME((OMX_STATETYPE)nData2));

                CmdType cmd;
                cmd.eEvent = OMX_EventCmdComplete;
                cmd.eCmd = OMX_CommandStateSet;
                cmd.sCmdData = nData2;
                cmd.eResult = result;
                result = pThis->m_pSignalQueue->Push(&cmd, sizeof(cmd));
                FAILED1(result, "push to signal queue failed");

            } else if ((OMX_COMMANDTYPE)nData1 == OMX_CommandFlush) {
                VTEST_MSG_MEDIUM("Event callback: flush complete on port : %s",
                        OMX_PORT_NAME(nData2));

                CmdType cmd;
                cmd.eEvent = OMX_EventCmdComplete;
                cmd.eCmd = OMX_CommandFlush;
                cmd.sCmdData = 0;
                cmd.eResult = result;
                result = pThis->m_pSignalQueue->Push(&cmd, sizeof(cmd));
                FAILED1(result, "push to signal queue failed");

            } else if ((OMX_COMMANDTYPE)nData1 == OMX_CommandPortDisable) {
                VTEST_MSG_MEDIUM("Event callback: %s port disable",
                        OMX_PORT_NAME(nData2));

                /* Only queue port disable during port reconfig,
                 * during stop, it clashes with the event for moving
                 * to loaded state */
                if (pThis->m_bPortReconfig) {
                    CmdType cmd;
                    cmd.eEvent = OMX_EventCmdComplete;
                    cmd.eCmd = OMX_CommandPortDisable;
                    cmd.sCmdData = 0;
                    cmd.eResult = result;
                    result = pThis->m_pSignalQueue->Push(&cmd, sizeof(cmd));
                    FAILED1(result, "push to signal queue failed");
                }

            } else if ((OMX_COMMANDTYPE)nData1 == OMX_CommandPortEnable) {
                VTEST_MSG_MEDIUM("Event callback: %s port enable",
                        OMX_PORT_NAME(nData2));
            } else {
                result = OMX_ErrorUndefined;
                VTEST_MSG_ERROR("Unimplemented command");
            }
        } else if (eEvent == OMX_EventError) {
            /* Already in error */
            if (pThis->m_bThreadStop) {
                return result;
            }

            if (QOMX_ErrorLTRUseFailed == (OMX_S32)nData1) {
                VTEST_MSG_ERROR("LTR use failed error with LTRID %u", (unsigned int)nData2);
            }

            result = OMX_ErrorUndefined;
            VTEST_MSG_ERROR(
                    "Event callback: async error nData1 %u, nData2 %u",
                    (unsigned int)nData1, (unsigned int)nData2);
        } else if (eEvent == OMX_EventBufferFlag) {
            VTEST_MSG_MEDIUM("Event callback: got buffer flag event");
        } else {
            result = OMX_ErrorUndefined;
            VTEST_MSG_ERROR("Unimplemented event");
        }
    }

    FAILED2(result, pThis->SetError(), "Error out");
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Encoder::EmptyDoneCallback(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_PTR pAppData, OMX_IN OMX_BUFFERHEADERTYPE *pHeader) {

    (void)hComponent;
    OMX_ERRORTYPE result = OMX_ErrorNone;
    Encoder *pThis = (Encoder *)pAppData;
    BufferInfo *pBuffer = NULL;
    int bufferindex = -1;
    BufferPool * pBufferPool1 = NULL;

    VTEST_MSG_MEDIUM("EBD size=%u", (unsigned int)pHeader->nFilledLen);
    bufferindex = pThis->m_pBufferManager->GetBuffer(
            pThis, PORT_INDEX_IN, pHeader, &pBuffer, &pBufferPool1);
    if(bufferindex == -1)
        VTEST_MSG_MEDIUM("Error in finding buffer %p", pBuffer);

    Mutex::Autolock autoLock(pThis->m_pLock);

    /* TODO : give all buffers back to Buffer Manager */
    if (pThis->m_bThreadStop || pThis->m_bInputEOSReached) {
        result = pThis->m_pFreeBufferQueue->Push(&pBuffer, sizeof(BufferInfo **));
        return result;
    }
    result = pThis->m_pSource->SetBuffer(pBuffer, pThis);
    if (result != OMX_ErrorNone) {
        /* Don't treat it as fatal, because there is possibility where the
         * eos hasn't reached us and source is not expecting any more buffers */
        VTEST_MSG_HIGH("SetBuffer on source failed pBuffer: %p", pBuffer);
        pThis->m_pFreeBufferQueue->Push(&pBuffer, sizeof(BufferInfo **));
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Encoder::FillDoneCallback(OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_PTR pAppData, OMX_IN OMX_BUFFERHEADERTYPE *pHeader) {

    (void)hComponent;
    OMX_ERRORTYPE result = OMX_ErrorNone;
    Encoder *pThis = (Encoder *)pAppData;
    BufferInfo *pBuffer = NULL;
    int bufferindex = -1;
    BufferPool * pBufferPool1 = NULL;

    VTEST_MSG_MEDIUM("FBD size=%u", (unsigned int)pHeader->nFilledLen);
    bufferindex = pThis->m_pBufferManager->GetBuffer(
        pThis, PORT_INDEX_OUT, pHeader, &pBuffer, &pBufferPool1);
    if(bufferindex == -1)
        VTEST_MSG_MEDIUM("Error in finding buffer %p", pBuffer);

    Mutex::Autolock autoLock(pThis->m_pLock);

    /* TODO : give all buffers back to Buffer Manager */
    if (pThis->m_bPortReconfig || pThis->m_bThreadStop) {
        result = pThis->m_pFreeBufferQueue->Push(&pBuffer, sizeof(BufferInfo**));
        return result;
    }

    result = pThis->m_pSink->SetBuffer(pBuffer, pThis);
    if (pHeader->nFlags & OMX_BUFFERFLAG_EOS) {
        VTEST_MSG_MEDIUM("got output EOS\n");
        pThis->m_bThreadStop = OMX_TRUE;
    }
    if (result != OMX_ErrorNone) {
        VTEST_MSG_ERROR("SetBuffer on sink failed pBuffer: %p", pBuffer);
        pThis->m_bThreadStop = OMX_TRUE;
        pThis->m_pFreeBufferQueue->Push(&pBuffer, sizeof(BufferInfo **));
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Encoder::GetComponentRole(OMX_BOOL bSecureSession,
        OMX_VIDEO_CODINGTYPE eCodec, OMX_STRING *role) {

    switch ((OMX_U32)eCodec) {
        case OMX_VIDEO_CodingAVC:
            *role = (OMX_STRING)"OMX.qcom.video.encoder.avc";
            break;
        case OMX_VIDEO_CodingMPEG4:
            *role = (OMX_STRING)"OMX.qcom.video.encoder.mpeg4";
            break;
        case OMX_VIDEO_CodingH263:
            *role = (OMX_STRING)"OMX.qcom.video.encoder.h263";
            break;
        case OMX_VIDEO_CodingVP8:
            *role = (OMX_STRING)"OMX.qcom.video.encoder.vp8";
            break;
        case QOMX_VIDEO_CodingHevc:
            *role = (OMX_STRING)"OMX.qcom.video.encoder.hevc";
            break;
        default:
            VTEST_MSG_ERROR("Error: Unsupported codec %d", eCodec);
            return OMX_ErrorBadParameter;
    }

    if (bSecureSession) {
        switch ((int)eCodec) {
            case OMX_VIDEO_CodingAVC:
                *role = (OMX_STRING)"OMX.qcom.video.encoder.avc.secure";
                break;
            case QOMX_VIDEO_CodingHevc:
                *role = (OMX_STRING)"OMX.qcom.video.encoder.hevc.secure";
                break;
            default:
                VTEST_MSG_ERROR("Secure session not supported for codec format : %d",
                                eCodec);
                return OMX_ErrorBadParameter;
        }
    }

    VTEST_MSG_MEDIUM("Role : %s, Codec Format : %d\n", *role, eCodec);
    return OMX_ErrorNone;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Encoder::Flush(OMX_U32 nPortIndex)
{
    OMX_ERRORTYPE result = OMX_ErrorNone;
    CmdType cmd;

    result = OMX_SendCommand(m_hEncoder, OMX_CommandFlush, nPortIndex, 0);
    FAILED1(result, "Flush failed");

    if (m_pSignalQueue->Pop(&cmd, sizeof(cmd), 0) != OMX_ErrorNone) {
        FAILED1(result, "Error popping signal");
    }
    if (cmd.eResult != OMX_ErrorNone || cmd.eEvent != OMX_EventCmdComplete ||
        cmd.eCmd != OMX_CommandFlush) {
        result = OMX_ErrorUndefined;
        VTEST_MSG_ERROR("Was expecting complete for flush command");
    }

    /* Wait for flush complete for both ports */
    if (nPortIndex == OMX_ALL) {
        if (m_pSignalQueue->Pop(&cmd, sizeof(cmd), 0) != OMX_ErrorNone) {
            FAILED1(result, "Error popping signal");
        }
        if (cmd.eResult != OMX_ErrorNone || cmd.eEvent != OMX_EventCmdComplete ||
                cmd.eCmd != OMX_CommandFlush) {
            result = OMX_ErrorUndefined;
            VTEST_MSG_ERROR("Was expecting complete for flush command");
        }
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_BOOL Encoder::CheckColorFormatSupported(OMX_COLOR_FORMATTYPE nColorFormat,
                          OMX_VIDEO_PARAM_PORTFORMATTYPE *pVideoPortFmt) {

    OMX_BOOL bSupported = OMX_FALSE;
    OMX_ERRORTYPE result = OMX_ErrorNone;
    OMX_S32 index = 0;

    OMX_INIT_STRUCT(pVideoPortFmt, OMX_VIDEO_PARAM_PORTFORMATTYPE);

    VTEST_MSG_HIGH("Encoder: Format[0x%x] requested", nColorFormat);

    while (result == OMX_ErrorNone) {
        pVideoPortFmt->nPortIndex = PORT_INDEX_IN;
        pVideoPortFmt->nIndex = index;
        result = OMX_GetParameter(m_hEncoder,
                OMX_IndexParamVideoPortFormat, (OMX_PTR)pVideoPortFmt);
        if (result == OMX_ErrorNone && pVideoPortFmt->eColorFormat == nColorFormat) {
            VTEST_MSG_HIGH("Encoder: Format[0x%x] supported by OMX Encoder", nColorFormat);
            bSupported = OMX_TRUE;
            break;
        }
        index++;
    }
    return bSupported;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE Encoder::SetHybridHp() {

    OMX_ERRORTYPE result = OMX_ErrorNone;
    CodecConfigType *pConfig = &m_sConfig;
    QOMX_EXTNINDEX_VIDEO_HYBRID_HP_MODE pHybridHpMode;
    OMX_INIT_STRUCT(&pHybridHpMode, QOMX_EXTNINDEX_VIDEO_HYBRID_HP_MODE);

    pHybridHpMode.nHpLayers = pConfig->sHybridHP.nHpLayers;
    pHybridHpMode.nKeyFrameInterval = pConfig->nIntraPeriod;
    pHybridHpMode.nMinQuantizer = pConfig->nMinQp;
    pHybridHpMode.nMaxQuantizer = pConfig->nMaxQp;

    for (unsigned int i = 0; i < pConfig->sHybridHP.nHpLayers; i++) {
        pHybridHpMode.nTemporalLayerBitrateRatio[i] = m_sConfig.nRcDistribution[i] * (pConfig->nBitrate / 100);
        VTEST_MSG_LOW("Layer Bit-rate setting to Component : Layer =  %d Bit Rate = %d Ratio = %d",
            i, pConfig->nBitrate, pHybridHpMode.nTemporalLayerBitrateRatio[i]);
    }

    result = OMX_SetParameter(m_hEncoder,
                 (OMX_INDEXTYPE)OMX_QcomIndexParamVideoHybridHierpMode,
                 (OMX_PTR)&pHybridHpMode);
    return result;
}

} // namespace vtest
