/* =========================================================================
Copyright (c) 2015-2017 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

Copyright (c) 2009, The Linux Foundation. All rights reserved.
--------------------------------------------------------------------------*/
/*============================================================================
                            O p e n M A X   w r a p p e r s
                O p e n  M A X   C o m p o n e n t  I n t e r f a c e

*//** @file vpp_omx_component.cpp
  This module contains the abstract interface for the OpenMAX components.

*//*========================================================================*/

//////////////////////////////////////////////////////////////////////////////
//                             Include Files
//////////////////////////////////////////////////////////////////////////////
#include <dlfcn.h>
#include <inttypes.h>
#include <sys/prctl.h>
#include <gralloc_priv.h>
#include <media/hardware/HardwareAPI.h>
#include <media/msm_media_info.h>
#include "vpp_omx_component.h"
#include "omx_core_cmp.h"
#include "OMX_QCOMExtns.h"
#include <cutils/ashmem.h>
#include "OMX_IndexExt.h"

#define _ANDROID_
#include "vidc_debug.h"
#undef _ANDROID_

#define QC_OMX_DECODER_LIBRARY "libOmxVdec.so"
#define DISPLAY_BUF_COUNT 4
#define ARG_TOUCH(x) (void)x
#define MIN_BUFFERS_IN_TURBO 6

#define INT32_PTR(v) (int32_t *)(&(v))
// define the key string for vpp extension parameters
// Hqv mode
#define MODE "mode"
// Global Demo
#define PROCESS_PERCENT "process-percent"
#define PROCESS_SIDE "process-side"
// CADE
#define CADE_MODE MODE
#define CADE_LEVEL "cade-level"
#define CADE_CONTRAST "contrast"
#define CADE_SATURATION "saturation"
// DI
#define DI_MODE MODE
// CNR
#define CNR_MODE MODE
#define CNR_LEVEL "level"
// AIE
#define AIE_MODE MODE
#define HUE_MODE "hue-mode"
#define LTM_LEVEL "ltm-level"
// FRC
#define FRC_MODE MODE

using namespace std;

#define OMX_VPP_PROP_DEFAULT_CONFIG     "vendor.media.vpp.default.config"
#define OMX_VPP_PROP_DEBUG_LVL          "vendor.vpp.debug.level"
#define OMX_VPP_PROP_DEBUG_EXTRADATA    "vendor.vpp.debug.extradata"

// static variable
OMX_CALLBACKTYPE VppOmxComponent::sOmxCallbacks = {
    &OmxEventHandler, &OmxEmptyBufferDone, &OmxFillBufferDone
};

// For runtime log level changes
int debug_level;

// factory function executed by the core to create instances
void *get_omx_component_factory_fn(void)
{
    return (new VppOmxComponent);
}

void* omx_message_thread(void *input);
void post_message(VppOmxComponent *omx, unsigned char id);

template<class T>
static void InitOMXParams(T &params) {
    memset(&params, 0, sizeof(T));
    params.nSize = sizeof(T);
    params.nVersion.s.nVersionMajor = 1;
    params.nVersion.s.nVersionMinor = 0;
    params.nVersion.s.nRevision = 0;
    params.nVersion.s.nStep = 0;
}

static inline OMX_ERRORTYPE setPerfLevel(
                OMX_PTR handle, QOMX_VIDEO_PERF_LEVEL perfLevel) {
    OMX_QCOM_VIDEO_CONFIG_PERF_LEVEL perf;
    InitOMXParams(perf);
    perf.ePerfLevel = perfLevel;
    return qc_omx_component_set_config(handle,
            (OMX_INDEXTYPE) OMX_QcomIndexConfigPerfLevel, (void *)&perf);
}

static const vector<vector<string>> EnumStringList = {
    {"HQV_MODE_OFF", "HQV_MODE_AUTO", "HQV_MODE_MANUAL"},
    {"DI_MODE_OFF", "DI_MODE_VIDEO_1F", "DI_MODE_VIDEO_3F", "DI_MODE_VIDEO_AUTO"},
    {"HQV_HUE_MODE_OFF", "HQV_HUE_MODE_ON"},
    {"FRC_MODE_OFF", "FRC_MODE_LOW", "FRC_MODE_MED", "FRC_MODE_HIGH"},
    {"PROCESS_LEFT", "PROCESS_RIGHT", "PROCESS_TOP", "PROCESS_BOTTOM"},
};

enum MapIndex {
    MAP_INDEX_MODE,
    MAP_INDEX_CADE_MODE = MAP_INDEX_MODE,
    MAP_INDEX_CNR_MODE = MAP_INDEX_MODE,
    MAP_INDEX_AIE_MODE = MAP_INDEX_MODE,
    MAP_INDEX_DI_MODE,
    MAP_INDEX_HUE_MODE,
    MAP_INDEX_FRC_MODE,
    MAP_INDEX_PROCESS_MODE,
    MAP_INDEX_MAX,
};

template <class T>
static const char* getStringFromEnum(T value, enum MapIndex index) {
    if (index < MAP_INDEX_MAX) {
        const vector<string> &cVector = EnumStringList[index];
        if ((size_t)value < cVector.size()) {
            DEBUG_PRINT_LOW("get string %s from %u", cVector[value].c_str(), value);
            return cVector[value].c_str();
        }
    }
    DEBUG_PRINT_LOW("failed to get string from %u", value);
    return NULL;
}

template <class T>
static bool getEnumFromString(char *str, T &value, enum MapIndex index) {
    if (index < MAP_INDEX_MAX) {
        const vector<string> &cVector = EnumStringList[index];
        for (size_t i = 0; i < cVector.size(); i++) {
            if (!strncasecmp(str, cVector[i].c_str(), OMX_MAX_STRINGNAME_SIZE)) {
                value = (T)i;
                DEBUG_PRINT_LOW("get enum %u from %s", value, str);
                return true;
            }
        }
    }
    DEBUG_PRINT_LOW("failed to get enum value from %s", str);
    return false;
}

VppOmxComponent::VppOmxComponent()
    : mState(OMX_StateInvalid),
      mAppData(NULL),
      mCreateOmxFn(NULL),
      mFlushing(false),
      mReconfigPending(false),
      mInReconfig(false),
      mWaitForEOS(false),
      mEosReached(false),
      mVppFlushStatus(0),
      mVppContext(NULL),
      mVppFlags(0),
      mVppBypassMode(false),
      mVppEnable(true),
      mVppExtradataMode(NO_EXTRADATA),
      mClientExtradataFd(-1),
      mVppColorFormat(OMX_COLOR_FormatUnused),
      mBuffersInTurbo(0),
      mSumFrameIntervals(0),
      mPreviousTimestamp(0),
      mDecoderExtensionNum(0),
      mIsVppActive(false),
      mIsDecoderSetup(false),
      mVppHqvCtrlValue()
{
    memset(&m_cmp, 0, sizeof(m_cmp));
    memset(&mOmxCallbacks,0,sizeof(mOmxCallbacks));
    sem_init(&mFlushEventLock,0,0);
    sem_init(&mReconfigEventLock,0,0);
    sem_init(&mEosEventLock,0,0);

    //Initializing the vppLibray specific structures
    memset(&mVppRequirements, 0, sizeof(mVppRequirements));
    memset(&mVppPortParam, 0, sizeof(mVppPortParam));
    sVppCallbacks = {(void*)this, &VppEmptyBufferDone, &VppOutputBufferDone, &VppEvent};

    //Set HQV_CONTROL as AUTO
    memset(&mVppHqvControl, 0, sizeof(mVppHqvControl));
    mVppHqvControl.mode = HQV_MODE_AUTO;

    char property_value[PROPERTY_VALUE_MAX] = {0};
    property_get(OMX_VPP_PROP_DEBUG_LVL, property_value, "1");
    debug_level = atoi(property_value);
    property_value[0] = '\0';
    property_get(OMX_VPP_PROP_DEBUG_EXTRADATA, property_value, "0");
    mDebugExtradata = atoi(property_value);
    DEBUG_PRINT_HIGH(OMX_VPP_PROP_DEBUG_EXTRADATA " value is %d",mDebugExtradata);

    memset(&mDecRcvdParamData, 0, sizeof(mDecRcvdParamData));

    VendorExtensionStore *extStore = const_cast<VendorExtensionStore *>(&mOmxVppVendorExtensionStore);
    initOmxVppExtensions(*extStore);
    mOmxVppVendorExtensionStore.dumpExtensions("OmxVppExt");

    DEBUG_PRINT_INFO("Constructed VppOmxComponent");
}

VppOmxComponent::~VppOmxComponent()
{
    DEBUG_PRINT_INFO("VppOmxComponent::~VppOmxComponent");
    sem_destroy(&mFlushEventLock);
    sem_destroy(&mReconfigEventLock);
    sem_destroy(&mEosEventLock);
}

OMX_ERRORTYPE VppOmxComponent::loadOmxDecoder()
{
    OMX_ERRORTYPE eRet = OMX_ErrorNone;
    DEBUG_PRINT_LOW("loadOmxDecoder %s", QC_OMX_DECODER_LIBRARY);
    mOmxDecoderLib = dlopen(QC_OMX_DECODER_LIBRARY, RTLD_NOW);

    if (mOmxDecoderLib == NULL) {
        // check for errors
        const char* pErr = dlerror();
        if (!pErr) {
            // No error reported, but no handle to the library
            DEBUG_PRINT_ERROR("loadOmxDecoder: Error opening "
                 "library (%s) but no error reported\n", QC_OMX_DECODER_LIBRARY);
        } else {
            // Error reported
            DEBUG_PRINT_ERROR("loadOmxDecoder: Error opening library (%s): %s\n",
                                QC_OMX_DECODER_LIBRARY, pErr);
        }
        eRet = OMX_ErrorUndefined;
        return eRet;
    }
    mCreateOmxFn = (create_qc_omx_component)dlsym(mOmxDecoderLib,
                                             "get_omx_component_factory_fn");
    if (mCreateOmxFn == NULL) {
        DEBUG_PRINT_ERROR("Unable to load sym get_omx_component_factory_fn");
        dlclose(mOmxDecoderLib);
        mOmxDecoderLib = NULL;
        eRet = OMX_ErrorUndefined;
    }
    return eRet;
}

bool VppOmxComponent::allocate_client_extradata_buffer() {
    OMX_ERRORTYPE eRet = OMX_ErrorNone;
    /*Allocate Internal Client ExtraData Buffer*/
    QOMX_VIDEO_CLIENT_EXTRADATATYPE clientExtraDataRequest;
    InitOMXParams(clientExtraDataRequest);
    clientExtraDataRequest.nPortIndex = 1;

    eRet = qc_omx_component_get_parameter(mOmxHandle, (OMX_INDEXTYPE)OMX_QTIIndexParamVideoClientExtradata,
            &clientExtraDataRequest);
    if (eRet != OK) {
        DEBUG_PRINT_ERROR("get Extradata info failed");
        return 0;
    }
    if (clientExtraDataRequest.nExtradataAllocSize == 0) {
        DEBUG_PRINT_ERROR("Extradata size is zero");
        return 0;
    }

    int32_t fd = ashmem_create_region("OMX_VPP-Extradata", clientExtraDataRequest.nExtradataAllocSize);
    if(fd < 0) {
        DEBUG_PRINT_ERROR("Extradata buffer allocate failed!");
        return 0;
    }

    clientExtraDataRequest.nFd = fd;
    eRet = qc_omx_component_set_parameter(mOmxHandle, (OMX_INDEXTYPE)OMX_QTIIndexParamVideoClientExtradata,
                    &clientExtraDataRequest);
    if (eRet != OK) {
        DEBUG_PRINT_ERROR("Failed set extradata param");
        close(fd);
        return 0;
    }
    if (!mBufferManager.SetClientExtraDataBuffer(clientExtraDataRequest.nFd,
                        clientExtraDataRequest.nExtradataAllocSize, clientExtraDataRequest.nExtradataSize)) {
        DEBUG_PRINT_ERROR("Failed SetClientExtraDataBuffer");
        return 0;
    }
    mClientExtradataFd = fd;
    return 1;
}

OMX_ERRORTYPE VppOmxComponent::component_init(OMX_IN OMX_STRING cmpName)
{
    OMX_ERRORTYPE eRet = OMX_ErrorNone;
    DEBUG_PRINT_LOW("component_init In %s", (char*)cmpName); //OMX.QCOM.H264.decoder.super
    strlcpy(mOmxRole, (char*)cmpName, ROLE_MAX_SIZE);

    if ((eRet = loadOmxDecoder()) != OMX_ErrorNone) {
        DEBUG_PRINT_ERROR("component_init failed to load omx core lib \n");
        return eRet;
    }

    void* omxCmp = mCreateOmxFn();
    mOmxHandle = qc_omx_create_component_wrapper((OMX_PTR)omxCmp);

    eRet = qc_omx_component_init(mOmxHandle, mOmxRole);
    if (eRet != OMX_ErrorNone) {
        DEBUG_PRINT_ERROR("Component init failed %s %d", mOmxRole, eRet);
        dlclose(mOmxDecoderLib);
        mOmxDecoderLib = NULL;
        return eRet;
    }

    qc_omx_component_set_callbacks(mOmxHandle, &sOmxCallbacks, this);
    mState = OMX_StateLoaded;
    int len = strlen(cmpName);
    if (len > 7 && !strcmp(cmpName + len - 7, ".secure"))
        mVppFlags |= VPP_SESSION_SECURE;

    // send Default vppHqvControls.
    char value[PROPERTY_VALUE_MAX];
    int32_t vppRet = 0;
    property_get(OMX_VPP_PROP_DEFAULT_CONFIG, value, NULL);
    if (!strcmp("1", value) || !strcmp("true", value)) {
        DEBUG_PRINT_INFO("Calling vpp_init. ");
        mVppContext = vpp_init(mVppFlags, sVppCallbacks);
        if(!mVppContext) {
            DEBUG_PRINT_ERROR("Error Initing vpp");
            return eRet;
        }

        DEBUG_PRINT_INFO("Sending default Config. %d %d",
                mVppHqvControl.mode, mVppHqvControl.ctrl_type);
        mVppHqvCtrlValue.mode = HQV_MODE_AUTO;
        vppRet = vpp_set_ctrl(mVppContext, mVppHqvControl, &mVppRequirements);
        if (vppRet != VPP_OK) {
            DEBUG_PRINT_ERROR("%s: vpp_set_ctrl failed : %d", __func__, vppRet);
        } else {
            if(mVppHqvControl.mode == HQV_MODE_MANUAL && mVppHqvControl.ctrl_type == HQV_CONTROL_FRC)
                mBufferManager.mInFrc = true;

            DEBUG_PRINT_INFO("vpp_set_ctrl successful, meta cnt %d", mVppRequirements.metadata.cnt);
            /*Enable extradata recorded in vpp_requirements::metadata for default algorithm*/
            uint32_t cnt = 0;
            QOMX_ENABLETYPE paramEnable;
            InitOMXParams(paramEnable);
            paramEnable.bEnable = OMX_TRUE;
            for (; cnt < mVppRequirements.metadata.cnt; cnt++) {
                DEBUG_PRINT_LOW("Enable Extradata index: 0x%X", mVppRequirements.metadata.meta[cnt]);
                eRet = qc_omx_component_set_parameter(mOmxHandle, (OMX_INDEXTYPE) mVppRequirements.metadata.meta[cnt], &paramEnable);
                if (eRet != OMX_ErrorNone) {
                    DEBUG_PRINT_ERROR("Enable Extradata failed!");
                }
            }
            if (mClientExtradataFd == -1 && mVppRequirements.metadata.cnt) {
                /*Allocate Internal Client ExtraData Buffer*/
                if (allocate_client_extradata_buffer()) {
                    mVppExtradataMode |= EXTRADATA_INTERNAL;
                }
            }
        }
    }
    // Query decoder extension number
    OMX_CONFIG_ANDROID_VENDOR_EXTENSIONTYPE decoderExtConfig;
    for (uint32_t index = 0;; index++) {
        InitOMXParams(decoderExtConfig);
        decoderExtConfig.nIndex = index;
        decoderExtConfig.nParamSizeUsed = 1;
        if (qc_omx_component_get_config(mOmxHandle, (OMX_INDEXTYPE)OMX_IndexConfigAndroidVendorExtension,
                &decoderExtConfig) == OMX_ErrorNoMore) {
            break;
        }
    }
    mDecoderExtensionNum = decoderExtConfig.nIndex;
    DEBUG_PRINT_LOW("decoder ext number is %u", mDecoderExtensionNum);

    //send codec Info to Vpp
    eRet = set_codec_info(cmpName, mVppContext);

    return eRet;
}

OMX_ERRORTYPE  VppOmxComponent::
                  get_component_version(OMX_HANDLETYPE       cmp_handle,
                                       OMX_STRING             cmp_name,
                                       OMX_VERSIONTYPE*    cmp_version,
                                       OMX_VERSIONTYPE*   spec_version,
                                       OMX_UUIDTYPE*          cmp_UUID)
{
    ARG_TOUCH(cmp_handle);
    ARG_TOUCH(cmp_name);

    if(mState == OMX_StateInvalid) {
        DEBUG_PRINT_ERROR("Get Comp Version in Invalid State\n");
        return OMX_ErrorInvalidState;
    }
    return qc_omx_component_get_version(mOmxHandle, mOmxRole,
                                 cmp_version, spec_version, cmp_UUID);
}

OMX_ERRORTYPE  VppOmxComponent::
                 send_command(OMX_HANDLETYPE cmp_handle,
                              OMX_COMMANDTYPE       cmd,
                              OMX_U32            param1,
                              OMX_PTR          cmd_data)
{
    ARG_TOUCH(cmp_handle);

    if(mState == OMX_StateInvalid) {
         DEBUG_PRINT_ERROR("send_command called in invalid state");
         return OMX_ErrorInvalidState;
    }
    DEBUG_PRINT_INFO("send_command In %d param1 %d", cmd, param1);

    if(mVppContext && !mVppBypassMode) {
        if(cmd == OMX_CommandFlush && ((param1 == OMX_ALL) || (param1 == OMX_CORE_OUTPUT_PORT_INDEX))) {
            AutoMutex autoLock(mFlushLock);
            // Handle eos -> flush case.
            AutoMutex autoEosLock(mEosSyncLock);
            if(mWaitForEOS) {
                mWaitForEOS = false;
                sem_post(&mEosEventLock);
            }
            mInReconfig = false;
            mFlushing = true;
            vpp_flush(mVppContext,VPP_PORT_INPUT);
            vpp_flush(mVppContext,VPP_PORT_OUTPUT);
            // VPP flush is issued once flush done is received from decoder
        } else if(cmd == OMX_CommandPortDisable && ((param1 == OMX_ALL) || (param1 == OMX_CORE_OUTPUT_PORT_INDEX))) {
            AutoMutex autoLock(mFlushLock);
            mFlushing = true;
            mInReconfig = false;
            vpp_flush(mVppContext,VPP_PORT_INPUT);
            vpp_flush(mVppContext,VPP_PORT_OUTPUT);
            if(mReconfigPending) mReconfigPending = false;
            // VPP flush is issued once flush done is received from decoder
        } else if(cmd == OMX_CommandStateSet && (param1 == OMX_StateIdle) && mState > OMX_StateIdle) {
            mFlushing = true;
            mInReconfig = false;
        } else if(cmd == OMX_CommandPortEnable && (param1 == OMX_CORE_OUTPUT_PORT_INDEX)) {
            //CLEAR vector..
            mBufferManager.ClearBuffers();
        }
    }
    return qc_omx_component_send_command(mOmxHandle, cmd, param1, cmd_data);
}

OMX_ERRORTYPE  VppOmxComponent::
                 get_parameter(OMX_HANDLETYPE     cmp_handle,
                               OMX_INDEXTYPE     param_index,
                               OMX_PTR            param_data)
{
    ARG_TOUCH(cmp_handle);
    OMX_ERRORTYPE eRet = OMX_ErrorNone;

    DEBUG_PRINT_LOW("get_parameter In");
    if(mState == OMX_StateInvalid) {
        DEBUG_PRINT_ERROR("get_parameter in Invalid State\n");
        return OMX_ErrorInvalidState;
    }

    if (param_index == (OMX_INDEXTYPE) OMX_QcomIndexParamEnableVpp) {
        VALIDATE_OMX_PARAM_DATA(param_data, QOMX_VPP_ENABLE);
        QOMX_VPP_ENABLE* omxVppEnable = (QOMX_VPP_ENABLE*) param_data;
        omxVppEnable->enable_vpp = OMX_TRUE;
        DEBUG_PRINT_LOW("OMX VPP Component is enabled");
        return OMX_ErrorNone;
    }
    eRet = qc_omx_component_get_parameter(mOmxHandle,
                            param_index, param_data);

    if (mVppContext && !mVppBypassMode && (param_index == OMX_IndexParamPortDefinition)) {
        OMX_PARAM_PORTDEFINITIONTYPE* portDef =
                           (OMX_PARAM_PORTDEFINITIONTYPE*)param_data;
        OMX_VIDEO_PORTDEFINITIONTYPE *videoDef = &(portDef->format.video);

        if (!mInReconfig && portDef->nPortIndex == 1 &&
            (videoDef->eColorFormat ==
            (OMX_COLOR_FORMATTYPE)QOMX_COLOR_FORMATYUV420PackedSemiPlanar32mCompressed ||
            videoDef->eColorFormat ==
            (OMX_COLOR_FORMATTYPE)QOMX_COLOR_FORMATYUV420PackedSemiPlanar32m10bitCompressed)) {
            mVppBypassMode = true;
            if (mBuffersInTurbo) {
                auto ret = setPerfLevel(mOmxHandle, OMX_QCOM_PerfLevelNominal);
                if (ret != OMX_ErrorNone) {
                    DEBUG_PRINT_ERROR("Failed to set nominal perfLevel");
                } else {
                    mBuffersInTurbo = 0;
                }
            }
        }

        if (portDef->nPortIndex == 1 && !mVppBypassMode) {
            DEBUG_PRINT_INFO("FRAME WIDTH %d FRAME HEIGHT %d",videoDef->nFrameHeight,videoDef->nFrameWidth);
            mBufferManager.UpdateBufferRequirements(mVppRequirements,portDef);
            DEBUG_PRINT_INFO("get_parameter counts %d %d ",portDef->nBufferCountActual, portDef->nBufferCountMin);
        } else if (portDef->nPortIndex == 0 && mState == OMX_StateLoaded) {
            // Put a limit of 6 for input buffer count
            if (portDef->nBufferCountActual < 6) {
                portDef->nBufferCountActual += 2;
                eRet = qc_omx_component_set_parameter(mOmxHandle, param_index, param_data);
                if (eRet != OMX_ErrorNone) {
                   DEBUG_PRINT_ERROR("SetParam on input port failed. ");
                   return eRet;
                }
                DEBUG_PRINT_INFO("Input BufferCount is %d", portDef->nBufferCountActual);
            }
        }
    }


    // VPP can operate only on VENUS NV12 format
    if (mVppContext && param_index == OMX_IndexParamVideoPortFormat)  {
        OMX_VIDEO_PARAM_PORTFORMATTYPE *portFmt =
               (OMX_VIDEO_PARAM_PORTFORMATTYPE*)param_data;
        if (OMX_CORE_OUTPUT_PORT_INDEX == portFmt->nPortIndex) {

            if (portFmt->eColorFormat == (OMX_COLOR_FORMATTYPE)QOMX_COLOR_FORMATYUV420PackedSemiPlanar32mCompressed) {
                portFmt->eColorFormat = (OMX_COLOR_FORMATTYPE)QOMX_COLOR_FORMATYUV420PackedSemiPlanar32m;
                DEBUG_PRINT_HIGH("VPP switched from NV12 UBWC to NV12");
            }
            if (mVppEnable) {
                mVppColorFormat = portFmt->eColorFormat;
                if (portFmt->eColorFormat == (OMX_COLOR_FORMATTYPE)QOMX_COLOR_FORMATYUV420PackedSemiPlanar32m) {
                    mVppBypassMode = false;
                    DEBUG_PRINT_HIGH("Get_param Normal Mode");
                } else {
                    mVppBypassMode = true;
                    DEBUG_PRINT_HIGH("Get_param Bypass Mode");
                }
            }
        }
    }

    return eRet;
}

OMX_ERRORTYPE  VppOmxComponent::
                 set_parameter(OMX_HANDLETYPE     cmp_handle,
                               OMX_INDEXTYPE     param_index,
                               OMX_PTR            param_data)
{
    ARG_TOUCH(cmp_handle);
    OMX_ERRORTYPE eRet = OMX_ErrorNone;

    DEBUG_PRINT_LOW("set_parameter In");
    if(mState == OMX_StateInvalid) {
        DEBUG_PRINT_ERROR("set_parameter in Invalid State\n");
        return OMX_ErrorInvalidState;
    }

    /* Other parameters first need to go to the decoder and then to VPP*/

    if (eRet == OMX_ErrorNone &&
        ((param_index == (OMX_INDEXTYPE) OMX_QTIIndexParamVideoPreferAdaptivePlayback) ||
               (param_index == (OMX_INDEXTYPE) OMX_QcomIndexParamVideoAdaptivePlaybackMode))) {
        mBufferManager.SetBufferMode(((QOMX_ENABLETYPE *)param_data)->bEnable);
        if (mBufferManager.GetBufferMode()) {
            DEBUG_PRINT_HIGH("Prefer Adaptive Playback is set also in VPP");
        }
    }

    if (param_index == OMX_IndexParamVideoPortFormat
            || param_index == OMX_IndexParamPortDefinition) {
        DEBUG_PRINT_INFO("setup decoder");
        mIsDecoderSetup = true;
    }

    if (!mVppContext)
    {
        DEBUG_PRINT_ERROR("set parameter return, mVppContext %p",(void*)mVppContext);
        return qc_omx_component_set_parameter(mOmxHandle, param_index, param_data);
    }

    if (param_index == OMX_IndexParamVideoPortFormat)  {
        OMX_VIDEO_PARAM_PORTFORMATTYPE *portFmt =
               (OMX_VIDEO_PARAM_PORTFORMATTYPE*)param_data;
        if (OMX_CORE_OUTPUT_PORT_INDEX == portFmt->nPortIndex) {
            if (mVppEnable) {
                mVppColorFormat = portFmt->eColorFormat;
                if (portFmt->eColorFormat == (OMX_COLOR_FORMATTYPE)QOMX_COLOR_FORMATYUV420PackedSemiPlanar32m) {
                    mVppBypassMode = false;
                    DEBUG_PRINT_HIGH("Set_param Normal Mode");
                } else {
                    mVppBypassMode = true;
                    DEBUG_PRINT_HIGH("Set_param Bypass Mode, YUV format not supported by vpp");
                }
            }
        }
    }

    if (param_index == OMX_IndexParamPortDefinition) {
        OMX_PARAM_PORTDEFINITIONTYPE* portDef =
                       (OMX_PARAM_PORTDEFINITIONTYPE*)param_data;
        OMX_VIDEO_PORTDEFINITIONTYPE *videoDef = &(portDef->format.video);

        if (!mInReconfig && portDef->nPortIndex == 1 &&
            (videoDef->eColorFormat ==
            (OMX_COLOR_FORMATTYPE)QOMX_COLOR_FORMATYUV420PackedSemiPlanar32mCompressed ||
            videoDef->eColorFormat ==
            (OMX_COLOR_FORMATTYPE)QOMX_COLOR_FORMATYUV420PackedSemiPlanar32m10bitCompressed)) {
            mVppBypassMode = true;
            if (mBuffersInTurbo) {
                auto ret = setPerfLevel(mOmxHandle, OMX_QCOM_PerfLevelNominal);
                if (ret != OMX_ErrorNone) {
                    DEBUG_PRINT_ERROR("Failed to set nominal perfLevel");
                } else {
                    mBuffersInTurbo = 0;
                }
            }
        }

        if (!mVppBypassMode && (mState == OMX_StateLoaded) && (portDef->nPortIndex == 1)) {
            DEBUG_PRINT_LOW("set_parameter for output port, ActualCount %d ", portDef->nBufferCountActual);
            mDecRcvdParamData = mVppPortParam;
            if (mDecRcvdParamData.height != videoDef->nFrameHeight ||
                mDecRcvdParamData.width != videoDef->nFrameWidth) {
                mDecRcvdParamData.height = videoDef->nFrameHeight;
                mDecRcvdParamData.width = videoDef->nFrameWidth;
                mDecRcvdParamData.stride = VENUS_Y_STRIDE(COLOR_FMT_NV12, videoDef->nFrameWidth);
                mDecRcvdParamData.scanlines = VENUS_Y_SCANLINES(COLOR_FMT_NV12, videoDef->nFrameHeight);
                mDecRcvdParamData.fmt = VPP_COLOR_FORMAT_NV12_VENUS;
                DEBUG_PRINT_LOW("Set vpp height %d & width %d ", videoDef->nFrameHeight, videoDef->nFrameWidth);
                int32_t vpp_err1 = vpp_set_parameter(mVppContext, VPP_PORT_OUTPUT, mDecRcvdParamData);
                DEBUG_PRINT_INFO("vpp_set_parameter output err %d", vpp_err1);
                int32_t vpp_err2 = vpp_set_parameter(mVppContext, VPP_PORT_INPUT, mDecRcvdParamData);
                DEBUG_PRINT_INFO("vpp_set_parameter input err %d", vpp_err2);
                mIsVppActive = true;
                if (vpp_err1 == VPP_ERR_STATE || vpp_err2 == VPP_ERR_STATE) {
                    mReconfigPending = true;
                    if (!vpp_reconfigure(mVppContext, mDecRcvdParamData, mDecRcvdParamData)) {
                        sem_wait(&mReconfigEventLock);
                    } else {
                        DEBUG_PRINT_ERROR("Reconfigure failed ");
                        mBufferManager.ResetBufferRequirements(portDef);
                        mReconfigPending = false;
                    }
                } else if (vpp_err1 || vpp_err2) {
                    //terminate vpp, as it will never be able to come out of bypass
                    //in this session.
                    mBufferManager.ResetBufferRequirements(portDef);
                    vpp_term(mVppContext);
                    mVppContext = NULL;
                } else {
                    mVppPortParam = mDecRcvdParamData;
                }
            }
        }
    } else if (param_index ==
                (OMX_INDEXTYPE) OMX_QcomIndexParamVideoSyncFrameDecodingMode) {
        // Vpp doesn't involve in thumbnail generation
        DEBUG_PRINT_LOW("Thumbnail generation not supported ");
        vpp_term(mVppContext);
        mVppContext = NULL;
    } else if (param_index == (OMX_INDEXTYPE)OMX_QTIIndexParamVideoClientExtradata) {
        DEBUG_PRINT_LOW("set_parameter in omx_vpp: OMX_QTIIndexParamVideoClientExtradata");
        QOMX_VIDEO_CLIENT_EXTRADATATYPE *pParam =
            (QOMX_VIDEO_CLIENT_EXTRADATATYPE *)param_data;
        if (mBufferManager.SetClientExtraDataBuffer(dup(pParam->nFd),
                    pParam->nExtradataAllocSize, pParam->nExtradataSize)) {
            mClientExtradataFd = pParam->nFd;
            mVppExtradataMode |= EXTRADATA_EXTERNAL;
        } else {
            /* If vpp can't register Client Extradata buffer successfully, means vpp won't
               propagate Extradata. Return error to Client to let Client release the buffer.
          */
            DEBUG_PRINT_ERROR("set OMX_QTIIndexParamVideoClientExtradata: Failed.");
            eRet = OMX_ErrorBadParameter;
            mVppExtradataMode = NO_EXTRADATA;
            mClientExtradataFd = -1;
        }
    }

    eRet = qc_omx_component_set_parameter(mOmxHandle, param_index, param_data);
    return eRet;
}

OMX_ERRORTYPE  VppOmxComponent::
                 get_config(OMX_HANDLETYPE      cmp_handle,
                            OMX_INDEXTYPE     config_index,
                            OMX_PTR            config_data)
{
    ARG_TOUCH(cmp_handle);
    OMX_ERRORTYPE eRet = OMX_ErrorNone;

    DEBUG_PRINT_LOW("get_config In");
    if(mState == OMX_StateInvalid) {
        DEBUG_PRINT_ERROR("get_config in Invalid State\n");
        return OMX_ErrorInvalidState;
    }

    if (config_index == (OMX_INDEXTYPE)OMX_IndexConfigAndroidVendorExtension) {
        VALIDATE_OMX_PARAM_DATA(config_data, OMX_CONFIG_ANDROID_VENDOR_EXTENSIONTYPE);
        OMX_CONFIG_ANDROID_VENDOR_EXTENSIONTYPE *ext =
                reinterpret_cast<OMX_CONFIG_ANDROID_VENDOR_EXTENSIONTYPE *>(config_data);
        VALIDATE_OMX_VENDOR_EXTENSION_PARAM_DATA(ext);
        if (ext->nIndex >= mDecoderExtensionNum) {
            DEBUG_PRINT_INFO("OmxVppExt: get Vpp VendorExtension");
            eRet = getOmxVppExtension(ext);
            return eRet;
        }
    }
    return qc_omx_component_get_config(mOmxHandle,
            config_index, config_data);
}

void VppOmxComponent::printHQVControl(hqv_control &hqvControl)
{
    DEBUG_PRINT_LOW("Setting Hqv Control: VPP mode:%d",hqvControl.mode);
    DEBUG_PRINT_LOW("VPP Ctrl type:%d",hqvControl.ctrl_type);

    switch (hqvControl.ctrl_type)
    {
        case static_cast<enum hqv_control_type>(VPP_HQV_CONTROL_CADE):
            DEBUG_PRINT_LOW("VPP CADE mode:%d",hqvControl.cade.mode);
            DEBUG_PRINT_LOW("VPP CADE cade_level:%d",hqvControl.cade.cade_level);
            DEBUG_PRINT_LOW("VPP CADE contrast:%d",hqvControl.cade.contrast);
            DEBUG_PRINT_LOW("VPP CADE saturation:%d",hqvControl.cade.saturation);
            break;
        case static_cast<enum hqv_control_type>(VPP_HQV_CONTROL_DI):
            DEBUG_PRINT_LOW("VPP DI mode:%d",hqvControl.di.mode);
            break;
        case static_cast<enum hqv_control_type>(VPP_HQV_CONTROL_CNR):
            DEBUG_PRINT_LOW("VPP CNR mode:%d",hqvControl.cnr.mode);
            DEBUG_PRINT_LOW("VPP CNR level:%u",hqvControl.cnr.level);
            break;
        case static_cast<enum hqv_control_type>(VPP_HQV_CONTROL_AIE):
            DEBUG_PRINT_LOW("VPP AIE mode:%d",hqvControl.aie.mode);
            DEBUG_PRINT_LOW("VPP AIE hue_mode:%d",hqvControl.aie.hue_mode);
            DEBUG_PRINT_LOW("VPP AIE cade_level:%u",hqvControl.aie.cade_level);
            DEBUG_PRINT_LOW("VPP AIE ltm_level:%u",hqvControl.aie.ltm_level);
            break;
        case static_cast<enum hqv_control_type>(VPP_HQV_CONTROL_FRC):
            DEBUG_PRINT_LOW("VPP FRC mode:%d",hqvControl.frc.mode);
            break;
        case static_cast<enum hqv_control_type>(VPP_HQV_CONTROL_GLOBAL_DEMO):
            DEBUG_PRINT_LOW("VPP DEMO percent:%u",hqvControl.demo.process_percent);
            DEBUG_PRINT_LOW("VPP DEMO direction:%d",hqvControl.demo.process_direction);
            break;
        default:
            DEBUG_PRINT_LOW("VPP algorithm not supported");
            break;
    }
}

OMX_ERRORTYPE  VppOmxComponent::
                 set_config(OMX_HANDLETYPE      cmp_handle,
                            OMX_INDEXTYPE     config_index,
                            OMX_PTR            config_data)
{
    ARG_TOUCH(cmp_handle);

    DEBUG_PRINT_LOW("set_config In");
    if (mState == OMX_StateInvalid) {
        DEBUG_PRINT_ERROR("set_config in Invalid State\n");
        return OMX_ErrorInvalidState;
    }

    if (config_index == (OMX_INDEXTYPE)OMX_IndexConfigAndroidVendorExtension) {
        VALIDATE_OMX_PARAM_DATA(config_data, OMX_CONFIG_ANDROID_VENDOR_EXTENSIONTYPE);
        OMX_CONFIG_ANDROID_VENDOR_EXTENSIONTYPE *ext =
                reinterpret_cast<OMX_CONFIG_ANDROID_VENDOR_EXTENSIONTYPE *>(config_data);
        VALIDATE_OMX_VENDOR_EXTENSION_PARAM_DATA(ext);
        if (!strncmp((char *)ext->cName, "qti-ext-vpp", 11)) {
            if (mState == OMX_StateLoaded) {
                // In Loaded State, both of two cases should be ignored
                // 1. if vpp is not active and decoder is already setup:
                //      ExtendedACodec is not enabled and this call is triggered from ACodec
                // 2. if vpp is already active (vpp_set_paramter is called):
                //      ExtendedACodec already trigger it first, then ACodec would trigger it again
                // In Executing State, pre-Check in setOmxVppExtension would block the bad cases.
                if (!mIsVppActive && mIsDecoderSetup) {
                    DEBUG_PRINT_INFO("OmxVppExt: vpp ext is not supported(avenhancement is disabled)");
                    return OMX_ErrorNone;
                }
                if (mIsVppActive) {
                    DEBUG_PRINT_INFO("OmxVppExt: vpp is already active");
                    return OMX_ErrorNone;
                }
            }
            DEBUG_PRINT_INFO("OmxVppExt: set Vpp VendorExtension");
            return setOmxVppExtension(ext);
        } else
            return qc_omx_component_set_config(mOmxHandle, config_index, config_data);
    }

    if (config_index == (OMX_INDEXTYPE) OMX_IndexConfigOperatingRate) {
        VALIDATE_OMX_PARAM_DATA(config_data, OMX_PARAM_U32TYPE);
        OMX_PARAM_U32TYPE *rate = (OMX_PARAM_U32TYPE *)config_data;
        struct video_property prop;

        prop.property_type = VID_PROP_OPERATING_RATE;
        prop.operating_rate.u32OperatingRate = rate->nU32 >> 16;
        if (mVppContext) {
            auto eRet = vpp_set_vid_prop(mVppContext, prop);
            if (eRet != VPP_OK) {
               DEBUG_PRINT_ERROR("vpp_set_vid_prop failed return %d, operating rate not set", eRet);
            }
        }
    }
    return qc_omx_component_set_config(mOmxHandle,
                            config_index, config_data);
}

OMX_ERRORTYPE  VppOmxComponent::
                 get_extension_index(OMX_HANDLETYPE  cmp_handle,
                                     OMX_STRING       paramName,
                                     OMX_INDEXTYPE*   indexType)

{
    ARG_TOUCH(cmp_handle);
    OMX_ERRORTYPE eRet = OMX_ErrorNone;

    DEBUG_PRINT_LOW("get_extension_index In");
    if(mState == OMX_StateInvalid) {
        DEBUG_PRINT_ERROR("get_extension_index in Invalid State\n");
        return OMX_ErrorInvalidState;
    }
    eRet = qc_omx_component_get_extension_index(mOmxHandle,
                            paramName, indexType);
    return eRet;
}

OMX_ERRORTYPE  VppOmxComponent::
                  get_state(OMX_HANDLETYPE  cmp_handle,
                           OMX_STATETYPE*       state)

{
    ARG_TOUCH(cmp_handle);

    return qc_omx_component_get_state(mOmxHandle, state);
}

OMX_ERRORTYPE  VppOmxComponent::
                  component_tunnel_request(OMX_HANDLETYPE           cmp_handle,
                                          OMX_U32                        port,
                                          OMX_HANDLETYPE       peer_component,
                                          OMX_U32                   peer_port,
                                          OMX_TUNNELSETUPTYPE*   tunnel_setup)
{
    ARG_TOUCH(cmp_handle);
    ARG_TOUCH(port);
    ARG_TOUCH(peer_port);
    ARG_TOUCH(peer_component);
    ARG_TOUCH(tunnel_setup);
    OMX_ERRORTYPE eRet = OMX_ErrorNotImplemented;
    ALOGW("component_tunnel_request not implemented");
    return eRet;
}

OMX_ERRORTYPE  VppOmxComponent::
                 use_buffer(OMX_HANDLETYPE                cmp_handle,
                            OMX_BUFFERHEADERTYPE**        buffer_hdr,
                            OMX_U32                             port,
                            OMX_PTR                         app_data,
                            OMX_U32                            bytes,
                            OMX_U8*                           buffer)
{
    ARG_TOUCH(cmp_handle);
    ARG_TOUCH(port);
    ARG_TOUCH(buffer_hdr);
    ARG_TOUCH(app_data);
    ARG_TOUCH(bytes);
    ARG_TOUCH(buffer);
    DEBUG_PRINT_LOW("Use Buffer In");
    auto eRet = qc_omx_component_use_buffer(mOmxHandle, buffer_hdr, port, app_data, bytes, buffer);
    if (mVppContext) {
        mBufferManager.RegisterClientBuffer(*buffer_hdr,buffer);
        DEBUG_PRINT_LOW("use_buffer add OmxVppBuffer owner = omx_client, *buffer_hdr %p \
              buffer %p ",(void*)(*buffer_hdr), (void *)buffer);
    }
    return eRet;
}


OMX_ERRORTYPE  VppOmxComponent::
                 allocate_buffer(OMX_HANDLETYPE                cmp_handle,
                                 OMX_BUFFERHEADERTYPE**        buffer_hdr,
                                 OMX_U32                             port,
                                 OMX_PTR                         app_data,
                                 OMX_U32                            bytes)

{
    ARG_TOUCH(cmp_handle);
    ARG_TOUCH(port);
    ARG_TOUCH(buffer_hdr);
    ARG_TOUCH(app_data);
    ARG_TOUCH(bytes);
    OMX_ERRORTYPE eRet = OMX_ErrorNone;
    eRet = qc_omx_component_allocate_buffer(mOmxHandle, buffer_hdr, port, app_data, bytes);
    return eRet;
}

OMX_ERRORTYPE  VppOmxComponent::
                 free_buffer(OMX_HANDLETYPE         cmp_handle,
                             OMX_U32                      port,
                             OMX_BUFFERHEADERTYPE*      buffer)
{
    ARG_TOUCH(cmp_handle);
    ARG_TOUCH(port);
    ARG_TOUCH(buffer);
    auto eRet = OMX_ErrorNone;
    DEBUG_PRINT_INFO("free_buffer called omxHeader %p",(void *)buffer);

    eRet = qc_omx_component_free_buffer(mOmxHandle, port, buffer);
    return eRet;
}

OMX_ERRORTYPE  VppOmxComponent::
                  empty_this_buffer(OMX_HANDLETYPE         cmp_handle,
                                   OMX_BUFFERHEADERTYPE*      buffer)

{
    ARG_TOUCH(cmp_handle);
    ARG_TOUCH(buffer);
    OMX_ERRORTYPE eRet = OMX_ErrorNone;
    DEBUG_PRINT_LOW("Etb to the decoder ");
    eRet = qc_omx_component_empty_this_buffer(mOmxHandle, buffer);
    return eRet;
}

OMX_ERRORTYPE  VppOmxComponent::
                  fill_this_buffer(OMX_HANDLETYPE         cmp_handle,
                                  OMX_BUFFERHEADERTYPE*      buffer)

{
    ARG_TOUCH(cmp_handle);
    OMX_ERRORTYPE eRet = OMX_ErrorNone;
    DEBUG_PRINT_LOW("Ftb from client for buffer: %p",(void*)buffer);
    if (mVppContext != NULL && !mVppBypassMode) {
        mBufferManager.PrintBufferStatistics();
        struct vpp_buffer *vppBuffer = NULL;
        OmxBufferOwner bufDestination = mBufferManager.GetFtbDestination(buffer,&vppBuffer,
                                            mReconfigPending,
                                            mWaitForEOS || mEosReached, mVppExtradataMode);
        if(bufDestination == OmxBufferOwner::OMX_VDEC) {
            AutoMutex autoLock(mDecoderSyncLock);
            eRet = qc_omx_component_fill_this_buffer(mOmxHandle, buffer);
            if(eRet)
                DEBUG_PRINT_ERROR("omx_fill_this_buffer to Decoder returned %d", eRet);
        } else if(bufDestination == OmxBufferOwner::VPP_OUT) {
            AutoMutex autoLock(mVppSyncLock);
            int32_t vpp_err = vpp_queue_buf(mVppContext, VPP_PORT_OUTPUT, vppBuffer);
            if(vpp_err)
                 DEBUG_PRINT_ERROR("vpp_queue_buf output port returned %d", vpp_err);
        } else {
            DEBUG_PRINT_ERROR("Buffer not registered. Giving it to Vdec.");
            AutoMutex autoLock(mDecoderSyncLock);
            eRet = qc_omx_component_fill_this_buffer(mOmxHandle, buffer);
            if(eRet)
                DEBUG_PRINT_ERROR("omx_fill_this_buffer to Decoder returned %d", eRet);
            mVppBypassMode = true;
        }
        return eRet; //Need to take vpp_err also into consideration depending on severity
    }
    eRet = qc_omx_component_fill_this_buffer(mOmxHandle, buffer);
    return eRet;
}

OMX_ERRORTYPE  VppOmxComponent::
                 set_callbacks( OMX_HANDLETYPE        cmp_handle,
                                OMX_CALLBACKTYPE*      callbacks,
                                OMX_PTR                 app_data)
{
    ARG_TOUCH(cmp_handle);
    OMX_ERRORTYPE eRet = OMX_ErrorNone;
    DEBUG_PRINT_LOW("set_callbacks In");

    mOmxCallbacks       = *callbacks;
    mAppData = app_data;
    return eRet;
}

OMX_ERRORTYPE VppOmxComponent::component_deinit(OMX_HANDLETYPE cmp_handle)
{
    ARG_TOUCH(cmp_handle);
    OMX_ERRORTYPE eRet = OMX_ErrorNone;

    DEBUG_PRINT_LOW("component_deinit In");
    qc_omx_component_deinit(mOmxHandle);
    int err = dlclose(mOmxDecoderLib);
    mOmxDecoderLib = NULL;
    if (err) {
        DEBUG_PRINT_ERROR("Error in dlclose of vdec lib\n");
    }
    vpp_term(mVppContext);
    mVppContext = NULL;
    return eRet;
}

OMX_ERRORTYPE  VppOmxComponent::
                 use_EGL_image(OMX_HANDLETYPE                cmp_handle,
                               OMX_BUFFERHEADERTYPE**        buffer_hdr,
                               OMX_U32                             port,
                               OMX_PTR                         app_data,
                               void*                          egl_image)
{
    ARG_TOUCH(cmp_handle);
    return qc_omx_component_use_EGL_image(mOmxHandle, buffer_hdr, port,
                                          app_data, egl_image);

}

OMX_ERRORTYPE  VppOmxComponent::
                component_role_enum( OMX_HANDLETYPE cmp_handle,
                                      OMX_U8*              role,
                                      OMX_U32             index)
{
    ARG_TOUCH(cmp_handle);
    return qc_omx_component_role_enum(mOmxHandle, role, index);
}

// callbacks from component
OMX_ERRORTYPE VppOmxComponent::OmxEventHandler(
                                      OMX_HANDLETYPE pHandle,
                                      OMX_PTR pAppData,
                                      OMX_EVENTTYPE eEvent,
                                      OMX_U32 nData1,
                                      OMX_U32 nData2,
                                      OMX_PTR pEventData)
{
    ARG_TOUCH(pHandle);
    OMX_ERRORTYPE eRet = OMX_ErrorNone;
    auto super_omx = reinterpret_cast<VppOmxComponent*>(pAppData);
    auto omx_comp = (OMX_COMPONENTTYPE*)(super_omx->mOmxHandle);
    struct timespec ts;

    DEBUG_PRINT_LOW("OmxEventHandler In, event %d", eEvent);
    switch ((OMX_U32)eEvent)
    {
        case OMX_EventCmdComplete:
        {
            DEBUG_PRINT_LOW("Command complete %d %d", nData1, nData2);
            if (nData1 == OMX_CommandFlush && (nData2 == OMX_ALL || nData2 == 1)) {
                if(super_omx->mVppContext && !super_omx->mVppBypassMode) {
                    AutoMutex autoLock(super_omx->mFlushLock);
                    DEBUG_PRINT_HIGH("Waiting for flush_done from VPP");
                    sem_wait(&super_omx->mFlushEventLock);
                    super_omx->mFlushing = false;
                    DEBUG_PRINT_HIGH("FLUSH RECEIVED");
                }
            }

            if(nData1 == OMX_CommandPortDisable && nData2 == 1) {
                if(super_omx->mVppContext && !super_omx->mVppBypassMode) {
                    AutoMutex autoLock(super_omx->mFlushLock);
                    DEBUG_PRINT_INFO("Port Disable send flush to VPP library ");
                    sem_wait(&super_omx->mFlushEventLock);
                    super_omx->mFlushing = false;
                }
            }

            if (nData1 == OMX_CommandPortEnable && nData2 == 1) {
                if (super_omx->mVppContext && !super_omx->mVppBypassMode) {
                    eRet = setPerfLevel(super_omx->mOmxHandle, OMX_QCOM_PerfLevelTurbo);
                    if (eRet != OMX_ErrorNone) {
                        DEBUG_PRINT_ERROR("Error setting perf level. ");
                        eRet = OMX_ErrorNone;
                    } else {
                        super_omx->mBuffersInTurbo = MIN_BUFFERS_IN_TURBO;
                        super_omx->mSumFrameIntervals = 0;
                        super_omx->mPreviousTimestamp = -1;
                        super_omx->mFrameRateFinalized = false;
                    }
                }
            }

            if(nData1 ==  OMX_CommandStateSet) {
                auto prevState = super_omx->mState;
                super_omx->mState = (OMX_STATETYPE)nData2;
                if(super_omx->mVppContext && !super_omx->mVppBypassMode && nData2 == OMX_StateIdle) {
                        if(prevState > super_omx->mState) {
                        DEBUG_PRINT_INFO("Flush during executing to idle!");
                        vpp_flush(super_omx->mVppContext,VPP_PORT_INPUT);
                        vpp_flush(super_omx->mVppContext,VPP_PORT_OUTPUT);
                        sem_wait(&super_omx->mFlushEventLock);
                        super_omx->mFlushing = false;
                        } else {
                            eRet = setPerfLevel(super_omx->mOmxHandle, OMX_QCOM_PerfLevelTurbo);
                            if (eRet != OMX_ErrorNone) {
                                DEBUG_PRINT_ERROR("Error setting perf level. ");
                                eRet = OMX_ErrorNone;
                            } else {
                                super_omx->mBuffersInTurbo = MIN_BUFFERS_IN_TURBO;
                                super_omx->mSumFrameIntervals = 0;
                                super_omx->mPreviousTimestamp = -1;
                                super_omx->mFrameRateFinalized = false;
                            }
                        }
                }
            }

            //TODO: Add a wait to serialize VPP flush complete and disable cmd complete
            eRet = super_omx->mOmxCallbacks.EventHandler(&super_omx->m_cmp,
                                  super_omx->mAppData,
                                  OMX_EventCmdComplete,
                                  nData1, nData2, pEventData);
            break;
        }
        case OMX_EventPortFormatDetected:
        {
            DEBUG_PRINT_LOW("OMX_EventPortFormatDetected %d", nData1);
            break;
        }
        case OMX_EventPortSettingsChanged:
        {
            bool handleReconfig = true;

            if (nData2 == OMX_IndexParamPortDefinition) {
                DEBUG_PRINT_LOW("OMX_EventPortSettingsChanged %d", nData1);
                super_omx->mInReconfig =
                    super_omx->mVppContext && !super_omx->mVppBypassMode;
            } else if (nData2 == OMX_IndexConfigCommonOutputCrop) {
                DEBUG_PRINT_LOW("Reconfig - IndexConfigCommonOutputCrop  ");
            } else if ((nData2 == OMX_QTIIndexConfigDescribeColorAspects) ||
                       (nData2 == OMX_QTIIndexConfigDescribeHDRColorInfo)) {
                DEBUG_PRINT_LOW("Reconfig - ColorAspects or HDR Info");
                handleReconfig = false;
            }else {
                DEBUG_PRINT_ERROR("Invalid Port Reconfig Event. ");
                handleReconfig = false;
            }

            // frame_data is sent always irrespective of superOMX or VPP
            // is in bypass mode
            uint32_t *frameData = (uint32_t *)pEventData;

            if(!frameData) {
                DEBUG_PRINT_ERROR("Reconfig height and width absent, frameData is NULL !!");
                return eRet;
            }

            if(super_omx->mVppContext && !super_omx->mVppBypassMode && handleReconfig) {
                uint32_t stride(0), scanlines(0);
                uint32_t dither_type(0), color_space(0), bit_depth(0);

                bit_depth = frameData[4];
                color_space = frameData[5];
                dither_type = frameData[6];
                stride = VENUS_Y_STRIDE(COLOR_FMT_NV12, frameData[3]);
                scanlines = VENUS_Y_SCANLINES(COLOR_FMT_NV12, frameData[2]);
                DEBUG_PRINT_INFO("VT %dx%dx%dx%d", frameData[1], frameData[0],
                                 stride, scanlines);
                super_omx->mDecRcvdParamData = super_omx->mVppPortParam;
                struct vpp_port_param& mVppPortParam = super_omx->mDecRcvdParamData;

                unsigned vdecOutCount, vppInCount, vppOutCount, clientCount;
                {
                    AutoMutex autoLock(super_omx->mFbdSyncLock);
                    super_omx->mBufferManager.GetBufferCounts(vdecOutCount, vppInCount, vppOutCount, clientCount);
                }
                bool needReconfigure = (nData2 == OMX_IndexParamPortDefinition) && vppInCount;
                // Reconfigure VPP only when the height or width has changed, this is
                // required to prevent calling vpp_reconfigure in regular cases when
                // the reconfig dimensions are same as crop dimensions. vpp_reconfigure is
                // expensive as it results in reopening the HVX/GPU session
                // Calling Vpp for Port reconfig for paramPortDefinition, since we can't
                // afford to lose valid fbds held in vpp. PortReconfig has to strictly
                // follow the FBDs, otherwise fbds will be discarded by client.
                if(needReconfigure ||
                    (super_omx->mVppPortParam.height != frameData[0]) ||
                    (super_omx->mVppPortParam.width != frameData[1]) ||
                    (super_omx->mVppPortParam.stride != stride) ||
                    (super_omx->mVppPortParam.scanlines != scanlines)) {
                    mVppPortParam.height = frameData[0];
                    mVppPortParam.width = frameData[1];
                    mVppPortParam.stride = stride;
                    mVppPortParam.scanlines = scanlines;
                    mVppPortParam.fmt =
                        (bit_depth == MSM_VIDC_BIT_DEPTH_10) ?
                             VPP_COLOR_FORMAT_MAX: VPP_COLOR_FORMAT_NV12_VENUS;
                    DEBUG_PRINT_INFO("Setting vpp height %d & width %d stride %d scanlines %d bitdepth %d",
                                     frameData[0],frameData[1], stride, scanlines, bit_depth);

                    // Deferring the event for vpp_reconfigure complete.
                    DEBUG_PRINT_INFO("sending reconfigure to VPP library ");
                    super_omx->mReconfigPending = true;
                    if (!vpp_reconfigure(super_omx->mVppContext, mVppPortParam, mVppPortParam))
                        sem_wait(&super_omx->mReconfigEventLock);
                    else {
                        DEBUG_PRINT_ERROR("Vpp Reconfigure failed!!");
                        super_omx->mReconfigPending = false;
                    }
                }
            }
            eRet = super_omx->mOmxCallbacks.EventHandler(&super_omx->m_cmp,
                                  super_omx->mAppData,
                                  OMX_EventPortSettingsChanged,
                                  nData1, nData2, NULL);
            break;
        }
        case OMX_EventBufferFlag:
        {
            DEBUG_PRINT_INFO("OMX_EventBufferFlag %d", nData1);
            if(nData1 == OMX_CORE_OUTPUT_PORT_INDEX && nData2 == OMX_BUFFERFLAG_EOS) {
                if(super_omx->mVppContext && !super_omx->mVppBypassMode) {
                    super_omx->mEosSyncLock.lock();
                    if(!super_omx->mEosReached && !super_omx->mFlushing) {
                        super_omx->mWaitForEOS = true;
                        super_omx->mEosSyncLock.unlock();
                        DEBUG_PRINT_LOW("Waiting for EOS");
                        sem_wait(&super_omx->mEosEventLock);
                        DEBUG_PRINT_LOW("Got eos / Flush happened");
                    } else {
                        super_omx->mEosSyncLock.unlock();
                    }
                    super_omx->mEosReached = false;
                    super_omx->mWaitForEOS = false;
                }
            }
            eRet = super_omx->mOmxCallbacks.EventHandler(&super_omx->m_cmp,
                                  super_omx->mAppData,
                                  OMX_EventBufferFlag,
                                  nData1, nData2, pEventData);
            break;
        }
        case OMX_EventError:
        {
            DEBUG_PRINT_LOW("OMX_EventError %d", nData1);
            eRet = super_omx->mOmxCallbacks.EventHandler(&super_omx->m_cmp,
                                  super_omx->mAppData,
                                  OMX_EventError,
                                  nData1, nData2, pEventData);
            break;
        }
        default:
            DEBUG_PRINT_ERROR("unknown event %d", eEvent);
    }
    return eRet;
}

OMX_ERRORTYPE VppOmxComponent::OmxEmptyBufferDone(
                                         OMX_HANDLETYPE hComponent,
                                         OMX_PTR pAppData,
                                         OMX_BUFFERHEADERTYPE* pBufHdr)
{
    ARG_TOUCH(hComponent);
    OMX_ERRORTYPE eRet = OMX_ErrorNone;
    DEBUG_PRINT_LOW("Ebd from decoder received %p ",(void *)pBufHdr);
    VppOmxComponent* super_omx = reinterpret_cast<VppOmxComponent*>(pAppData);
    eRet = super_omx->mOmxCallbacks.EmptyBufferDone(&super_omx->m_cmp, super_omx->mAppData, pBufHdr);
    return eRet;
}

OMX_ERRORTYPE VppOmxComponent::OmxFillBufferDone(
                                        OMX_HANDLETYPE hComponent,
                                        OMX_PTR pAppData,
                                        OMX_BUFFERHEADERTYPE* buffer)
{
    ARG_TOUCH(hComponent);
    OMX_ERRORTYPE eRet = OMX_ErrorNone;
    auto super_omx = reinterpret_cast<VppOmxComponent*>(pAppData);
    DEBUG_PRINT_LOW("Fill buffer done from decoder %p",(void *)buffer);
    super_omx->mBufferManager.PrintBufferStatistics();
    if (super_omx->mVppContext && !super_omx->mVppBypassMode) {
        AutoMutex autoLock(super_omx->mFlushLock);
        struct vpp_buffer *vppBuffer = super_omx->mBufferManager.GetBufferForVppIn(
                                            buffer);
        if(!vppBuffer) {
            AutoMutex autoLock(super_omx->mFbdSyncLock);
            DEBUG_PRINT_ERROR("No corresponding vppBuffer, returning to client. ");
            super_omx->mBufferManager.UpdateBufferOwnership(buffer, OmxBufferOwner::OMX_CLIENT);
            eRet = super_omx->mOmxCallbacks.FillBufferDone(&super_omx->m_cmp,
                        super_omx->mAppData, buffer);
            return eRet;
        }

        DEBUG_PRINT_LOW("fd, cookie, allocLen, filledLen, offset, timestamp. %d %p %d %d %d %llu ",
            vppBuffer->pixel.fd, vppBuffer->cookie, vppBuffer->pixel.alloc_len, vppBuffer->pixel.filled_len,
            vppBuffer->pixel.offset, (long long unsigned)vppBuffer->timestamp);

        if (super_omx->mFlushing || super_omx->mReconfigPending || !vppBuffer) {
            AutoMutex autoLock(super_omx->mFbdSyncLock);
            if (super_omx->mFlushing || super_omx->mReconfigPending) {
                DEBUG_PRINT_HIGH("Flush/Reconfig: Buffer returned to client. ");
            } else if (!vppBuffer) {
                DEBUG_PRINT_HIGH("No corresponding vppBuffer, returning to client. ");
            }
            super_omx->mBufferManager.UpdateBufferOwnership(buffer,OmxBufferOwner::OMX_CLIENT);
            eRet = super_omx->mOmxCallbacks.FillBufferDone(&super_omx->m_cmp, super_omx->mAppData, buffer);
            return eRet;
        }

        int32_t vpp_err = vpp_queue_buf(super_omx->mVppContext, VPP_PORT_INPUT, vppBuffer);

        if(vpp_err)
            DEBUG_PRINT_ERROR("vpp_queue_buf input port returned %d", vpp_err);
        return OMX_ErrorNone; //Need to decide on the return value.
    }

    // No VPP
    eRet = super_omx->mOmxCallbacks.FillBufferDone(&super_omx->m_cmp, super_omx->mAppData, buffer);
    return eRet;
}

// vpp empty_buffer_done callback
void VppOmxComponent::VppEmptyBufferDone(void *handle, struct vpp_buffer *buf)
{
    VppOmxComponent *sOmxVpp = (VppOmxComponent *)handle;

    DEBUG_PRINT_LOW("VppEmptyBufferDone for buffer [%p]",(void*)buf->cookie);
    auto buffer = (OMX_BUFFERHEADERTYPE *)buf->cookie;

    sOmxVpp->mBufferManager.PrintBufferStatistics();
    if (sOmxVpp->mFlushing || sOmxVpp->mReconfigPending) {
        AutoMutex autoLock(sOmxVpp->mFbdSyncLock);
        bool bDisableDynamicBufMode = sOmxVpp->mBufferManager.GetBufferMode();
        DEBUG_PRINT_LOW("Flush/Reconfig: %s :returning buffer to client",__func__);
        sOmxVpp->mBufferManager.UpdateBufferOwnership(buffer,OmxBufferOwner::OMX_CLIENT);
        buffer->nAllocLen = bDisableDynamicBufMode? buffer->nAllocLen: sizeof(struct VideoDecoderOutputMetaData);
        buffer->nFilledLen = 0; // Flushed buffer.
        buffer->nFlags = 0; //Clear flags
        sOmxVpp->mOmxCallbacks.FillBufferDone(&sOmxVpp->m_cmp, sOmxVpp->mAppData,
             (OMX_BUFFERHEADERTYPE*)buf->cookie);
        return;
    }

    struct vpp_buffer *vppBuffer = NULL;
    auto bufDestination = sOmxVpp->mBufferManager.GetFtbDestination(buffer,
                                &vppBuffer, sOmxVpp->mReconfigPending,
                                sOmxVpp->mEosReached || sOmxVpp->mWaitForEOS, sOmxVpp->mVppExtradataMode);

    if(bufDestination == OmxBufferOwner::OMX_VDEC) {
        AutoMutex autoLock(sOmxVpp->mDecoderSyncLock);
        DEBUG_PRINT_LOW("To Vdec : (buffer Header ,offset, allocLen, nFilledLen, nTimestamp) = (%p,%d,%d,%d,%lld)",
                         (void *)buffer, buffer->nOffset, buffer->nAllocLen, buffer->nFilledLen, buffer->nTimeStamp);

        OMX_ERRORTYPE eRet = qc_omx_component_fill_this_buffer(sOmxVpp->mOmxHandle, buffer);
        if(eRet)
            DEBUG_PRINT_ERROR("omx_fill_this_buffer to Decoder returned %d", eRet);
    } else if(bufDestination == OmxBufferOwner::VPP_OUT) {
        AutoMutex autoLock(sOmxVpp->mVppSyncLock);
        DEBUG_PRINT_LOW("To Vpp : fd, cookie, allocLen, filledLen, offset, timestamp. %d %p %d %d %d %llu ",
        vppBuffer->pixel.fd, vppBuffer->cookie, vppBuffer->pixel.alloc_len, vppBuffer->pixel.filled_len,
        vppBuffer->pixel.offset, (long long unsigned)vppBuffer->timestamp);

        auto vpp_err = vpp_queue_buf(sOmxVpp->mVppContext, VPP_PORT_OUTPUT, vppBuffer);
        if(vpp_err)
                 DEBUG_PRINT_LOW("vpp_queue_buf output port returned %d", vpp_err);
    } else {
        DEBUG_PRINT_ERROR("You shouldn't be here.. ");
    }
}

// vpp output_buffer_done callback
void VppOmxComponent::VppOutputBufferDone(void *handle, struct vpp_buffer *buf)
{
    VppOmxComponent *sOmxVpp = (VppOmxComponent *)handle;

    AutoMutex autoLock(sOmxVpp->mFbdSyncLock);
    DEBUG_PRINT_LOW("Vpp Output buffer done");
    auto buffer = (OMX_BUFFERHEADERTYPE *)buf->cookie;
    bool bDisableDynamicBufMode = sOmxVpp->mBufferManager.GetBufferMode();

    if(sOmxVpp->mBufferManager.HandleBypass(buffer)) {
        DEBUG_PRINT_HIGH("Vpp running in bypass.");
    } else {
        DEBUG_PRINT_LOW("Vpp running in normal/frc mode.");
    }

    sOmxVpp->mBufferManager.PrintBufferStatistics();
    if (sOmxVpp->mDebugExtradata && buf->extradata.pvMapped != NULL && buf->extradata.filled_len) {
        uint8_t *otherExtra = (uint8_t *)buf->extradata.pvMapped + buf->extradata.offset;
        uint32_t len = buf->extradata.filled_len;
        while (len >= sizeof(OMX_OTHER_EXTRADATATYPE)) {
            sOmxVpp->print_debug_extradata((OMX_OTHER_EXTRADATATYPE *)otherExtra);
            if (((OMX_OTHER_EXTRADATATYPE *)otherExtra)->eType == OMX_ExtraDataNone) {
                break;
            }
            otherExtra += ((OMX_OTHER_EXTRADATATYPE *)otherExtra)->nSize;
            len -= ((OMX_OTHER_EXTRADATATYPE *)otherExtra)->nSize;
        };
    }
    sOmxVpp->mBufferManager.UpdateBufferOwnership(buffer,OmxBufferOwner::OMX_CLIENT);

    /* Populate timestamp and filled length from the vpp_buffer*/

    buffer->nTimeStamp = buf->timestamp;
    DEBUG_PRINT_LOW("VPP output port(buffer,filledLen, allocLen, timestamp) = ( %p,%u,%u,%lld ) ", (void*)buffer, buf->pixel.filled_len,
                                                                                      buffer->nAllocLen, buffer->nTimeStamp);
    buffer->nAllocLen = bDisableDynamicBufMode? buffer->nAllocLen:sizeof(struct VideoDecoderOutputMetaData);
    buffer->nFilledLen = buf->pixel.filled_len;
    if(bDisableDynamicBufMode) {
        sOmxVpp->mBufferManager.UpdateBufferGeometry(buffer,&sOmxVpp->mVppPortParam);
    } else {
        buffer->nFilledLen = (buffer->nFilledLen ? sizeof(struct VideoDecoderOutputMetaData) : 0);
    }

    sOmxVpp->mBufferManager.UpdateMetaData(buffer,sOmxVpp->mVppColorFormat);
    sOmxVpp->mBufferManager.ConvertOmxVppFlags(buf->flags, buffer->nFlags, false);

    if(buffer->nFlags & OMX_BUFFERFLAG_EOS) {
        AutoMutex autoLock(sOmxVpp->mEosSyncLock);
        DEBUG_PRINT_INFO("Eos Reached !!");
        sOmxVpp->mEosReached = true;
        if(sOmxVpp->mWaitForEOS) {
            sem_post(&sOmxVpp->mEosEventLock);
            sOmxVpp->mWaitForEOS = false;
        }
    }

    // Bring the clock back to nominal when the turbo window is over.
    if (sOmxVpp->mBuffersInTurbo == 1 && sOmxVpp->mFrameRateFinalized && buffer->nFilledLen) {
        AutoMutex autoLock(sOmxVpp->mDecoderSyncLock);
        OMX_ERRORTYPE eRet = setPerfLevel(sOmxVpp->mOmxHandle, OMX_QCOM_PerfLevelNominal);
        if (eRet != OMX_ErrorNone) {
            DEBUG_PRINT_ERROR("Error setting perf level. ");
            eRet = OMX_ErrorNone;
        }
    }

    if (sOmxVpp->mBuffersInTurbo && !sOmxVpp->mFrameRateFinalized && buffer->nFilledLen) {
        // Calculate instantanious frame rate
        if (sOmxVpp->mPreviousTimestamp == -1) {
            sOmxVpp->mPreviousTimestamp = buf->timestamp;
        } else {
            int64_t interval = buf->timestamp - sOmxVpp->mPreviousTimestamp;
            DEBUG_PRINT_LOW("buffer: %d interval %" PRId64 , sOmxVpp->mBuffersInTurbo, interval);
            sOmxVpp->mSumFrameIntervals += interval;
            sOmxVpp->mPreviousTimestamp = buf->timestamp;
            sOmxVpp->mFrameRateFinalized = (sOmxVpp->mBuffersInTurbo == 1);
            if (sOmxVpp->mFrameRateFinalized) {
                int64_t avgFrameInterval = sOmxVpp->mSumFrameIntervals / (MIN_BUFFERS_IN_TURBO - 1);
                DEBUG_PRINT_LOW("average interval is %" PRId64 , avgFrameInterval);
                sOmxVpp->mBuffersInTurbo = (avgFrameInterval < 25000) ? 250: 2;
            }
        }
    }

    if (sOmxVpp->mBuffersInTurbo && buf->pixel.filled_len)
        sOmxVpp->mBuffersInTurbo--;

    DEBUG_PRINT_LOW("VPP output port(buffer,filledLen, allocLen, timestamp,flags) = (%p,%u,%u,%lld,%u) ",
            (void*)buffer, buffer->nFilledLen,
            buffer->nAllocLen, buffer->nTimeStamp, buffer->nFlags);
    sOmxVpp->mOmxCallbacks.FillBufferDone(&sOmxVpp->m_cmp, sOmxVpp->mAppData,buffer);

}

// vpp event callback
void VppOmxComponent::VppEvent(void *handle, struct vpp_event e)
{
    VppOmxComponent *sOmxVpp = (VppOmxComponent *)handle;
    if (e.type == VPP_EVENT_FLUSH_DONE) {
        if(e.flush_done.port == VPP_PORT_INPUT)
            sOmxVpp->mVppFlushStatus |= (uint8_t)VPP_INPUT;
        else
            sOmxVpp->mVppFlushStatus |= (uint8_t)VPP_OUTPUT;
        DEBUG_PRINT_HIGH("Vpp flush event status %x", sOmxVpp->mVppFlushStatus);

        if (sOmxVpp->mVppFlushStatus == VPP_ALL) {
            sem_post(&sOmxVpp->mFlushEventLock);
            sOmxVpp->mVppFlushStatus = 0;
        }
    } else if (e.type == VPP_EVENT_RECONFIG_DONE) {
        DEBUG_PRINT_HIGH("VPP Reconfig event received ");
        sOmxVpp->mVppRequirements = e.port_reconfig_done.req;
        sOmxVpp->mVppPortParam = sOmxVpp->mDecRcvdParamData;

        if(sOmxVpp->mReconfigPending) {
            sem_post(&sOmxVpp->mReconfigEventLock);
            sOmxVpp->mReconfigPending = false;
      }
    }
}


//Helper function to parse OMX component name , convert to a codec enum and send it to vpp library
OMX_ERRORTYPE VppOmxComponent::set_codec_info(OMX_STRING cmpName, void* vpp_ctxt)
{
    OMX_ERRORTYPE eRet = OMX_ErrorNone;
    struct video_property prop;
    uint32_t vppRet = VPP_OK;

    if (!strncmp(cmpName,"OMX.qcom.video.decoder.avc",\
                OMX_MAX_STRINGNAME_SIZE)) {
        prop.codec.eCodec = VPP_CODEC_TYPE_AVC;
    } else if (!strncmp(cmpName,"OMX.qcom.video.decoder.avc.dsmode",\
                OMX_MAX_STRINGNAME_SIZE)) {
        prop.codec.eCodec = VPP_CODEC_TYPE_AVC;
    } else if (!strncmp(cmpName, "OMX.qcom.video.decoder.avc.secure",\
                OMX_MAX_STRINGNAME_SIZE)) {
        prop.codec.eCodec = VPP_CODEC_TYPE_AVC;
    } else if (!strncmp(cmpName, "OMX.qcom.video.decoder.avc.secure.dsmode",\
                OMX_MAX_STRINGNAME_SIZE)) {
        prop.codec.eCodec = VPP_CODEC_TYPE_AVC;
    } else if (!strncmp(cmpName, "OMX.qcom.video.decoder.divx4",\
                OMX_MAX_STRINGNAME_SIZE)) {
        prop.codec.eCodec = VPP_CODEC_TYPE_DIVX4;
    } else if (!strncmp(cmpName, "OMX.qcom.video.decoder.divx",\
                OMX_MAX_STRINGNAME_SIZE)) {
        prop.codec.eCodec = VPP_CODEC_TYPE_DIVX;
    } else if (!strncmp(cmpName, "OMX.qcom.video.decoder.divx311",\
                OMX_MAX_STRINGNAME_SIZE)) {
        prop.codec.eCodec = VPP_CODEC_TYPE_DIVX311;
    } else if (!strncmp(cmpName, "OMX.qcom.video.decoder.mpeg4",\
                OMX_MAX_STRINGNAME_SIZE)) {
        prop.codec.eCodec = VPP_CODEC_TYPE_MPEG4;
    } else if (!strncmp(cmpName, "OMX.qcom.video.decoder.mpeg4.secure",\
                OMX_MAX_STRINGNAME_SIZE)) {
        prop.codec.eCodec = VPP_CODEC_TYPE_MPEG4;
    } else if (!strncmp(cmpName, "OMX.qcom.video.decoder.mpeg2",\
                OMX_MAX_STRINGNAME_SIZE)) {
        prop.codec.eCodec = VPP_CODEC_TYPE_MPEG2;
    } else if (!strncmp(cmpName, "OMX.qcom.video.decoder.mpeg2.secure",\
                OMX_MAX_STRINGNAME_SIZE)) {
        prop.codec.eCodec = VPP_CODEC_TYPE_MPEG2;
    } else if (!strncmp(cmpName, "OMX.qcom.video.decoder.vc1",\
                OMX_MAX_STRINGNAME_SIZE)) {
        prop.codec.eCodec = VPP_CODEC_TYPE_VC1;
    } else if (!strncmp(cmpName, "OMX.qcom.video.decoder.vc1.secure",\
                OMX_MAX_STRINGNAME_SIZE)) {
        prop.codec.eCodec = VPP_CODEC_TYPE_VC1;
    } else if (!strncmp(cmpName, "OMX.qcom.video.decoder.wmv",\
                OMX_MAX_STRINGNAME_SIZE)) {
        prop.codec.eCodec = VPP_CODEC_TYPE_WMV;
    } else if (!strncmp(cmpName, "OMX.qcom.video.decoder.wmv.secure",\
                OMX_MAX_STRINGNAME_SIZE)) {
        prop.codec.eCodec = VPP_CODEC_TYPE_WMV;
    } else if (!strncmp(cmpName, "OMX.qcom.video.decoder.h263",\
                OMX_MAX_STRINGNAME_SIZE)) {
        prop.codec.eCodec = VPP_CODEC_TYPE_H263;
    } else if (!strncmp(cmpName, "OMX.qcom.video.decoder.hevc",\
                OMX_MAX_STRINGNAME_SIZE)) {
        prop.codec.eCodec = VPP_CODEC_TYPE_HEVC;
    } else if (!strncmp(cmpName, "OMX.qcom.video.decoder.hevc.secure",\
                OMX_MAX_STRINGNAME_SIZE)) {
        prop.codec.eCodec = VPP_CODEC_TYPE_HEVC;
    } else if (!strncmp(cmpName, "OMX.qcom.video.decoder.vp8",\
                OMX_MAX_STRINGNAME_SIZE)) {
        prop.codec.eCodec = VPP_CODEC_TYPE_VP8;
    } else if (!strncmp(cmpName, "OMX.qcom.video.decoder.vp9",\
                OMX_MAX_STRINGNAME_SIZE)) {
        prop.codec.eCodec = VPP_CODEC_TYPE_VP9;
    } else {
        DEBUG_PRINT_ERROR("ERROR:Unknown Component");
        prop.codec.eCodec = VPP_CODEC_TYPE_UNKNOWN;
    }

    if (prop.codec.eCodec != VPP_CODEC_TYPE_UNKNOWN) {
        prop.property_type = VID_PROP_CODEC;
        DEBUG_PRINT_LOW(" Codec Info is %d", prop.codec.eCodec);
        vppRet = vpp_set_vid_prop(vpp_ctxt, prop);
        if (vppRet) {
            DEBUG_PRINT_ERROR(" set codec property to vpp failed");
        }
    }
    return eRet;
}

void VppOmxComponent::print_debug_extradata(OMX_OTHER_EXTRADATATYPE *extra)
{
    if (!extra)
        return;

    DEBUG_PRINT_HIGH(
            "============== Extra Data ==============\n"
            "           Size: %u\n"
            "        Version: %u\n"
            "      PortIndex: %u\n"
            "           Type: %x\n"
            "       DataSize: %u",
            (unsigned int)extra->nSize, (unsigned int)extra->nVersion.nVersion,
            (unsigned int)extra->nPortIndex, extra->eType, (unsigned int)extra->nDataSize);

    if (extra->eType == (OMX_EXTRADATATYPE)OMX_ExtraDataInterlaceFormat) {
        OMX_STREAMINTERLACEFORMAT *intfmt = (OMX_STREAMINTERLACEFORMAT *)(void *)extra->data;
        DEBUG_PRINT_HIGH(
                "------ Interlace Format ------\n"
                "                Size: %u\n"
                "             Version: %u\n"
                "           PortIndex: %u\n"
                " Is Interlace Format: %d\n"
                "   Interlace Formats: %u\n"
                "=========== End of Interlace ===========",
                (unsigned int)intfmt->nSize, (unsigned int)intfmt->nVersion.nVersion, (unsigned int)intfmt->nPortIndex,
                intfmt->bInterlaceFormat, (unsigned int)intfmt->nInterlaceFormats);
    } else if (extra->eType == (OMX_EXTRADATATYPE)OMX_ExtraDataFrameInfo) {
        OMX_QCOM_EXTRADATA_FRAMEINFO *fminfo = (OMX_QCOM_EXTRADATA_FRAMEINFO *)(void *)extra->data;

        DEBUG_PRINT_HIGH(
                "-------- Frame Format --------\n"
                "             Picture Type: %d\n"
                "           Interlace Type: %d\n"
                " Pan Scan Total Frame Num: %u\n"
                "   Concealed Macro Blocks: %u\n"
                "               frame rate: %u\n"
                "               Time Stamp: %llu\n"
                "           Aspect Ratio X: %u\n"
                "           Aspect Ratio Y: %u",
                fminfo->ePicType,
                fminfo->interlaceType,
                (unsigned int)fminfo->panScan.numWindows,
                (unsigned int)fminfo->nConcealedMacroblocks,
                (unsigned int)fminfo->nFrameRate,
                fminfo->nTimeStamp,
                (unsigned int)fminfo->aspectRatio.aspectRatioX,
                (unsigned int)fminfo->aspectRatio.aspectRatioY);

        for (OMX_U32 i = 0; i < fminfo->panScan.numWindows; i++) {
            DEBUG_PRINT_HIGH(
                    "------------------------------"
                    "     Pan Scan Frame Num: %u\n"
                    "            Rectangle x: %d\n"
                    "            Rectangle y: %d\n"
                    "           Rectangle dx: %d\n"
                    "           Rectangle dy: %d",
                    (unsigned int)i, (unsigned int)fminfo->panScan.window[i].x, (unsigned int)fminfo->panScan.window[i].y,
                    (unsigned int)fminfo->panScan.window[i].dx, (unsigned int)fminfo->panScan.window[i].dy);
        }

        DEBUG_PRINT_HIGH("========= End of Frame Format ==========");
    } else if (extra->eType == (OMX_EXTRADATATYPE)OMX_ExtraDataFramePackingArrangement) {
        OMX_QCOM_FRAME_PACK_ARRANGEMENT *framepack = (OMX_QCOM_FRAME_PACK_ARRANGEMENT *)(void *)extra->data;
        DEBUG_PRINT_HIGH(
                "------------------ Framepack Format ----------\n"
                "                           id: %u \n"
                "                  cancel_flag: %u \n"
                "                         type: %u \n"
                " quincunx_sampling_flagFormat: %u \n"
                "  content_interpretation_type: %u \n"
                "        spatial_flipping_flag: %u \n"
                "          frame0_flipped_flag: %u \n"
                "             field_views_flag: %u \n"
                " current_frame_is_frame0_flag: %u \n"
                "   frame0_self_contained_flag: %u \n"
                "   frame1_self_contained_flag: %u \n"
                "       frame0_grid_position_x: %u \n"
                "       frame0_grid_position_y: %u \n"
                "       frame1_grid_position_x: %u \n"
                "       frame1_grid_position_y: %u \n"
                "                reserved_byte: %u \n"
                "            repetition_period: %u \n"
                "               extension_flag: %u \n"
                "================== End of Framepack ===========",
                (unsigned int)framepack->id,
                (unsigned int)framepack->cancel_flag,
                (unsigned int)framepack->type,
                (unsigned int)framepack->quincunx_sampling_flag,
                (unsigned int)framepack->content_interpretation_type,
                (unsigned int)framepack->spatial_flipping_flag,
                (unsigned int)framepack->frame0_flipped_flag,
                (unsigned int)framepack->field_views_flag,
                (unsigned int)framepack->current_frame_is_frame0_flag,
                (unsigned int)framepack->frame0_self_contained_flag,
                (unsigned int)framepack->frame1_self_contained_flag,
                (unsigned int)framepack->frame0_grid_position_x,
                (unsigned int)framepack->frame0_grid_position_y,
                (unsigned int)framepack->frame1_grid_position_x,
                (unsigned int)framepack->frame1_grid_position_y,
                (unsigned int)framepack->reserved_byte,
                (unsigned int)framepack->repetition_period,
                (unsigned int)framepack->extension_flag);
    } else if (extra->eType == (OMX_EXTRADATATYPE)OMX_ExtraDataQP) {
        OMX_QCOM_EXTRADATA_QP * qp = (OMX_QCOM_EXTRADATA_QP *)(void *)extra->data;
        DEBUG_PRINT_HIGH(
                "---- QP (Frame quantization parameter) ----\n"
                "    Frame QP: %u \n"
                "================ End of QP ================\n",
                (unsigned int)qp->nQP);
    } else if (extra->eType == (OMX_EXTRADATATYPE)OMX_ExtraDataInputBitsInfo) {
        OMX_QCOM_EXTRADATA_BITS_INFO * bits = (OMX_QCOM_EXTRADATA_BITS_INFO *)(void *)extra->data;
        DEBUG_PRINT_HIGH(
                "--------- Input bits information --------\n"
                "    Header bits: %u \n"
                "     Frame bits: %u \n"
                "===== End of Input bits information =====\n",
                (unsigned int)bits->header_bits, (unsigned int)bits->frame_bits);
    } else if (extra->eType == (OMX_EXTRADATATYPE)OMX_ExtraDataMP2UserData) {
        OMX_QCOM_EXTRADATA_USERDATA *userdata = (OMX_QCOM_EXTRADATA_USERDATA *)(void *)extra->data;
        OMX_U8 *data_ptr = (OMX_U8 *)userdata->data;
        OMX_U32 userdata_size = extra->nDataSize - sizeof(userdata->type);
        OMX_U32 i = 0;
        DEBUG_PRINT_HIGH(
                "--------------  Userdata  -------------\n"
                "    Stream userdata type: %u\n"
                "          userdata size: %u\n"
                "    STREAM_USERDATA:",
                (unsigned int)userdata->type, (unsigned int)userdata_size);
                for (i = 0; i < userdata_size; i+=4) {
                    DEBUG_PRINT_HIGH("        %x %x %x %x",
                        data_ptr[i], data_ptr[i+1],
                        data_ptr[i+2], data_ptr[i+3]);
                }
        DEBUG_PRINT_HIGH(
                "=========== End of Userdata ===========");
    } else if (extra->eType == (OMX_EXTRADATATYPE)OMX_ExtraDataVQZipSEI) {
        OMX_QCOM_EXTRADATA_VQZIPSEI *vq = (OMX_QCOM_EXTRADATA_VQZIPSEI *)(void *)extra->data;
        DEBUG_PRINT_HIGH(
                "--------------  VQZip  -------------\n"
                "    Size: %u\n",
                (unsigned int)vq->nSize);
        DEBUG_PRINT_HIGH( "=========== End of VQZip ===========");
    } else if (extra->eType == OMX_ExtraDataNone) {
        DEBUG_PRINT_HIGH("========== End of Terminator ===========");
    } else {
        DEBUG_PRINT_HIGH("======= End of Driver Extradata ========");
    }
}

// Main functions to support vendor extensions
void VppOmxComponent::initOmxVppExtensions(VendorExtensionStore &store) {
    // Init vpp vendor extension
    // use enum hqv_control_type as extension index directly
    DEBUG_PRINT_LOW("OmxVppExt: initOmxVppExtensions");
    ADD_EXTENSION("qti-ext-vpp", HQV_CONTROL_NONE, OMX_DirOutput)
    ADD_PARAM_END(MODE, OMX_AndroidVendorValueString)

    ADD_EXTENSION("qti-ext-vpp-cade", HQV_CONTROL_CADE, OMX_DirOutput)
    ADD_PARAM(CADE_MODE, OMX_AndroidVendorValueString)
    ADD_PARAM(CADE_LEVEL, OMX_AndroidVendorValueInt32)
    ADD_PARAM(CADE_CONTRAST, OMX_AndroidVendorValueInt32)
    ADD_PARAM_END(CADE_SATURATION, OMX_AndroidVendorValueInt32)

    ADD_EXTENSION("qti-ext-vpp-di", HQV_CONTROL_DI, OMX_DirOutput)
    ADD_PARAM_END(DI_MODE, OMX_AndroidVendorValueString)

    ADD_EXTENSION("qti-ext-vpp-cnr", HQV_CONTROL_CNR, OMX_DirOutput)
    ADD_PARAM(CNR_MODE, OMX_AndroidVendorValueString)
    ADD_PARAM_END(CNR_LEVEL, OMX_AndroidVendorValueInt32)

    ADD_EXTENSION("qti-ext-vpp-aie", HQV_CONTROL_AIE, OMX_DirOutput)
    ADD_PARAM(AIE_MODE, OMX_AndroidVendorValueString)
    ADD_PARAM(HUE_MODE, OMX_AndroidVendorValueString)
    ADD_PARAM(CADE_LEVEL, OMX_AndroidVendorValueInt32)
    ADD_PARAM_END(LTM_LEVEL, OMX_AndroidVendorValueInt32)

    ADD_EXTENSION("qti-ext-vpp-frc", HQV_CONTROL_FRC, OMX_DirOutput)
    ADD_PARAM_END(FRC_MODE, OMX_AndroidVendorValueString)

    ADD_EXTENSION("qti-ext-vpp-demo", HQV_CONTROL_GLOBAL_DEMO, OMX_DirOutput)
    ADD_PARAM(PROCESS_PERCENT, OMX_AndroidVendorValueInt32)
    ADD_PARAM_END(PROCESS_SIDE, OMX_AndroidVendorValueString)
}

OMX_ERRORTYPE VppOmxComponent::getOmxVppExtension(OMX_CONFIG_ANDROID_VENDOR_EXTENSIONTYPE *ext) {
    uint32_t index = ext->nIndex - mDecoderExtensionNum;
    if (index >= mOmxVppVendorExtensionStore.size())
        return OMX_ErrorNoMore;
    const VendorExtension& vExt = mOmxVppVendorExtensionStore[index];
    DEBUG_PRINT_LOW("OmxVppExt: getConfig: omx index=%u (%s), omx vpp index = %u", ext->nIndex, vExt.name(), index);
    vExt.copyInfoTo(ext);
    if (ext->nParamSizeUsed < vExt.paramCount()) {
        // this happens during initial getConfig to query only extension-name and param-count
        return OMX_ErrorNone;
    }

    bool setStatus = true;
    switch ((enum hqv_control_type)vExt.extensionIndex()) {
        case HQV_CONTROL_NONE:
        {
            const char *str = getStringFromEnum(mVppHqvCtrlValue.mode, MAP_INDEX_MODE);
            setStatus &= (str != NULL)
                    && vExt.setParamString(ext, MODE, str);
            break;
        }
        case HQV_CONTROL_GLOBAL_DEMO:
        {
            setStatus &= vExt.setParamInt32(ext, PROCESS_PERCENT, mVppHqvCtrlValue.demo.process_percent);
            const char *str = getStringFromEnum(mVppHqvCtrlValue.demo.process_direction, MAP_INDEX_PROCESS_MODE);
            setStatus &= (str != NULL)
                    && vExt.setParamString(ext, PROCESS_SIDE, str);
            break;
        }
        case HQV_CONTROL_CADE:
        {
            const char *str = getStringFromEnum(mVppHqvCtrlValue.cade.mode, MAP_INDEX_CADE_MODE);
            setStatus &= (str != NULL)
                    && vExt.setParamString(ext, CADE_MODE, str);
            setStatus &= vExt.setParamInt32(ext, CADE_LEVEL, mVppHqvCtrlValue.cade.cade_level);
            setStatus &= vExt.setParamInt32(ext, CADE_CONTRAST, mVppHqvCtrlValue.cade.contrast);
            setStatus &= vExt.setParamInt32(ext, CADE_SATURATION, mVppHqvCtrlValue.cade.saturation);
            break;
        }
        case HQV_CONTROL_DI:
        {
            const char *str = getStringFromEnum(mVppHqvCtrlValue.di.mode, MAP_INDEX_DI_MODE);
            setStatus &= (str != NULL)
                    && vExt.setParamString(ext, DI_MODE, str);
            break;
        }
        case HQV_CONTROL_CNR:
        {
            const char *str = getStringFromEnum(mVppHqvCtrlValue.cnr.mode, MAP_INDEX_CNR_MODE);
            setStatus &= (str != NULL)
                    && vExt.setParamString(ext, CNR_MODE, str);
            setStatus &= vExt.setParamInt32(ext, CNR_LEVEL, mVppHqvCtrlValue.cnr.level);
            break;
        }
        case HQV_CONTROL_AIE:
        {
            const char *str = getStringFromEnum(mVppHqvCtrlValue.aie.mode, MAP_INDEX_AIE_MODE);
            setStatus &= (str != NULL)
                    && vExt.setParamString(ext, AIE_MODE, str);
            const char *str2 = getStringFromEnum(mVppHqvCtrlValue.aie.hue_mode, MAP_INDEX_HUE_MODE);
            setStatus &= (str2 != NULL)
                    && vExt.setParamString(ext, HUE_MODE, str2);
            setStatus &= vExt.setParamInt32(ext, CADE_LEVEL, mVppHqvCtrlValue.aie.cade_level);
            setStatus &= vExt.setParamInt32(ext, LTM_LEVEL, mVppHqvCtrlValue.aie.ltm_level);
            break;
        }
        case HQV_CONTROL_FRC:
        {
            const char *str = getStringFromEnum(mVppHqvCtrlValue.frc.mode, MAP_INDEX_FRC_MODE);
            setStatus &= (str != NULL)
                    && vExt.setParamString(ext, FRC_MODE, str);
            break;
        }
        default:
        {
            DEBUG_PRINT_ERROR("No vpp extension index was found");
            return OMX_ErrorNotImplemented;
        }
    }
    return setStatus ? OMX_ErrorNone : OMX_ErrorUndefined;
}

OMX_ERRORTYPE VppOmxComponent::setOmxVppExtension(OMX_CONFIG_ANDROID_VENDOR_EXTENSIONTYPE *ext) {
    uint32_t index = 0;
    for (index = 0; index < mOmxVppVendorExtensionStore.size(); index++) {
        if (!strncmp((char *)ext->cName, mOmxVppVendorExtensionStore[index].name(), OMX_MAX_STRINGNAME_SIZE)) {
            DEBUG_PRINT_INFO("OmxVppExt: find extension name: %s index: %u", (char *)ext->cName, index);
            break;
        }
    }
    if (index == mOmxVppVendorExtensionStore.size()) {
        DEBUG_PRINT_ERROR("OmxVppExt: unrecognized vendor extension name (%s)", (char *)ext->cName);
        return OMX_ErrorBadParameter;
    }

    const VendorExtension& vExt = mOmxVppVendorExtensionStore[index];
    DEBUG_PRINT_LOW("OmxVppExt: setConfig: omx index=%u (%s), omx vpp index = %u", ext->nIndex, vExt.name(), index);

    OMX_ERRORTYPE err = OMX_ErrorNone;
    err = vExt.isConfigValid(ext);
    if (err != OMX_ErrorNone) {
        DEBUG_PRINT_ERROR("OmxVppExt: config is not valid");
        return err;
    }

    // mark this as set, regardless of set_config succeeding/failing.
    // App will know by inconsistent values in output-format
    vExt.set();

    uint32_t extensionIndex = vExt.extensionIndex();
    // Deal with "qti-ext-vpp" extension
    if (extensionIndex == HQV_CONTROL_NONE) {
        if (mVppContext != NULL) {
            DEBUG_PRINT_ERROR("vpp is already initialized when set vpp mode");
            return OMX_ErrorBadParameter;
        }
        char str[OMX_MAX_STRINGNAME_SIZE] = {0};
        bool ret = vExt.readParamInt64(ext, MODE, str)
                    && getEnumFromString(str, mVppHqvCtrlValue.mode, MAP_INDEX_MODE);
        if (ret && (mVppHqvCtrlValue.mode == HQV_MODE_MANUAL
                || mVppHqvCtrlValue.mode == HQV_MODE_AUTO)) {
            mVppContext = vpp_init(mVppFlags, sVppCallbacks);
            if (mVppContext != NULL) {
                DEBUG_PRINT_INFO("vpp init successfully");
                set_codec_info(mOmxRole, mVppContext);
                if (mVppHqvCtrlValue.mode == HQV_MODE_AUTO) {
                    struct hqv_control hqvCtrl{};
                    hqvCtrl.mode = mVppHqvCtrlValue.mode;
                    if (vpp_set_ctrl(mVppContext, hqvCtrl, &mVppRequirements)
                            == VPP_OK) {
                        DEBUG_PRINT_LOW("In auto mode: vpp set ctrl return OK");
                        return OMX_ErrorNone;
                    }
                } else {
                    DEBUG_PRINT_LOW("in manual mode: wait for next vpp extension");
                    return OMX_ErrorNone;
                }
            }
        }
        DEBUG_PRINT_ERROR("fail to set vpp extension(qti-ext-vpp)");
        return OMX_ErrorBadParameter;
    }

    // Deal with other vpp extension in manual mode
    if (mVppContext == NULL || mVppHqvCtrlValue.mode != HQV_MODE_MANUAL) {
        DEBUG_PRINT_ERROR("set vpp extension in bad condition");
        return OMX_ErrorBadParameter;
    }
    bool valueSet = false;
    struct hqv_control hqvCtrl{};
    hqvCtrl.ctrl_type = (enum hqv_control_type)extensionIndex;
    hqvCtrl.mode = mVppHqvCtrlValue.mode;
    char str[OMX_MAX_STRINGNAME_SIZE] = {0};
    switch (hqvCtrl.ctrl_type) {
        case HQV_CONTROL_GLOBAL_DEMO:
        {
            valueSet |= vExt.readParamInt32(ext, PROCESS_PERCENT,
                    INT32_PTR(mVppHqvCtrlValue.demo.process_percent));
            valueSet |= vExt.readParamInt64(ext, PROCESS_SIDE, str)
                    && getEnumFromString(str, mVppHqvCtrlValue.demo.process_direction, MAP_INDEX_PROCESS_MODE);
            memcpy(&hqvCtrl.demo, &mVppHqvCtrlValue.demo, sizeof(hqvCtrl.demo));
            break;
        }

        case HQV_CONTROL_CADE:
        {
            valueSet |= vExt.readParamInt64(ext, CADE_MODE, str)
                    && getEnumFromString(str, mVppHqvCtrlValue.cade.mode, MAP_INDEX_CADE_MODE);
            valueSet |= vExt.readParamInt32(ext, CADE_LEVEL, INT32_PTR(mVppHqvCtrlValue.cade.cade_level));
            valueSet |= vExt.readParamInt32(ext, CADE_CONTRAST, INT32_PTR(mVppHqvCtrlValue.cade.contrast));
            valueSet |= vExt.readParamInt32(ext, CADE_SATURATION, INT32_PTR(mVppHqvCtrlValue.cade.saturation));
            memcpy(&hqvCtrl.cade, &mVppHqvCtrlValue.cade, sizeof(hqvCtrl.cade));
            break;
        }
        case HQV_CONTROL_DI:
        {
            valueSet |= vExt.readParamInt64(ext, DI_MODE, str)
                    && getEnumFromString(str, mVppHqvCtrlValue.di.mode, MAP_INDEX_DI_MODE);
            memcpy(&hqvCtrl.di, &mVppHqvCtrlValue.di, sizeof(hqvCtrl.di));
            break;
        }
        case HQV_CONTROL_CNR:
        {
            valueSet |= vExt.readParamInt64(ext, CNR_MODE, str)
                    && getEnumFromString(str, mVppHqvCtrlValue.cnr.mode, MAP_INDEX_CNR_MODE);
            valueSet |= vExt.readParamInt32(ext, CNR_LEVEL, INT32_PTR(mVppHqvCtrlValue.cnr.level));
            memcpy(&hqvCtrl.cnr, &mVppHqvCtrlValue.cnr, sizeof(hqvCtrl.cnr));
            break;
        }
        case HQV_CONTROL_AIE:
        {
            valueSet |= vExt.readParamInt64(ext, AIE_MODE, str)
                    && getEnumFromString(str, mVppHqvCtrlValue.aie.mode, MAP_INDEX_AIE_MODE);
            str[0] = '\0';
            valueSet |= vExt.readParamInt64(ext, HUE_MODE, str)
                    && getEnumFromString(str, mVppHqvCtrlValue.aie.hue_mode, MAP_INDEX_HUE_MODE);
            valueSet |= vExt.readParamInt32(ext, CADE_LEVEL, INT32_PTR(mVppHqvCtrlValue.aie.cade_level));
            valueSet |= vExt.readParamInt32(ext, LTM_LEVEL, INT32_PTR(mVppHqvCtrlValue.aie.ltm_level));
            memcpy(&hqvCtrl.aie, &mVppHqvCtrlValue.aie, sizeof(hqvCtrl.aie));
            break;
        }
        case HQV_CONTROL_FRC:
        {
            valueSet |= vExt.readParamInt64(ext, FRC_MODE, str)
                    && getEnumFromString(str, mVppHqvCtrlValue.frc.mode, MAP_INDEX_FRC_MODE);
            memcpy(&hqvCtrl.frc, &mVppHqvCtrlValue.frc, sizeof(hqvCtrl.frc));
            break;
        }
        default:
        {
            DEBUG_PRINT_ERROR("No vpp extension index was found");
            return OMX_ErrorNotImplemented;
        }
    }
    if (valueSet) {
        printHQVControl(hqvCtrl);
        auto eRet = vpp_set_ctrl(mVppContext, hqvCtrl, &mVppRequirements);
        if (eRet == VPP_OK) {
            if (hqvCtrl.ctrl_type == HQV_CONTROL_FRC)
                mBufferManager.mInFrc = true;
            return OMX_ErrorNone;
        } else
            DEBUG_PRINT_ERROR("OmxVppExt: vpp_set_ctrl failed");
    }
    return OMX_ErrorBadParameter;
}
