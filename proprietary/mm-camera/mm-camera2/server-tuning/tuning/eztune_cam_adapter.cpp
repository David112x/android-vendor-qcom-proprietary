/*******************************************************************************
 * Copyright (c) 2014-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *******************************************************************************/

//=============================================================================
/**
 * This file contains eztune network protocol class implementation
 */
//=============================================================================

#include <stdlib.h>

#include "mct_pipeline.h"
#include "mct_stream.h"
#include "mct_list.h"
#include "mmcam_log_utils.h"
#include "eztune_lookup.h"
#include "eztune.h"
#include "eztune_cam_adapter.h"
#include "eztune_module.h"

namespace eztune
{

static cam_intf_parm_type_t kTuningCmdToCamIntfCmd[TUNING_SET_MAX];
CamAdapter* CamAdapter::m_singleton = NULL;

CamAdapter::CamAdapter(void *handle, eztune_server_t mode)
{
    m_singleton = this;

    MMCAM_ASSERT(handle != NULL, "Null Camera client handle");
    m_client_handle = handle;

    //build LUT for tuning cmds to cam intf commands
    kTuningCmdToCamIntfCmd[TUNING_SET_RELOAD_CHROMATIX] = CAM_INTF_PARM_SET_RELOAD_CHROMATIX;
    kTuningCmdToCamIntfCmd[TUNING_SET_RELOAD_AFTUNE] = CAM_INTF_PARM_SET_RELOAD_AFTUNE;
    kTuningCmdToCamIntfCmd[TUNING_SET_AUTOFOCUS_TUNING] = CAM_INTF_PARM_SET_AUTOFOCUSTUNING;
    kTuningCmdToCamIntfCmd[TUNING_SET_VFE_COMMAND] = CAM_INTF_PARM_SET_VFE_COMMAND;
    kTuningCmdToCamIntfCmd[TUNING_SET_POSTPROC_COMMAND] = CAM_INTF_PARM_SET_PP_COMMAND;
    kTuningCmdToCamIntfCmd[TUNING_SET_3A_COMMAND] = CAM_INTF_PARM_EZTUNE_CMD;
    kTuningCmdToCamIntfCmd[TUNING_SET_AEC_LOCK] = CAM_INTF_PARM_AEC_LOCK;
    kTuningCmdToCamIntfCmd[TUNING_SET_AEC_UNLOCK] = CAM_INTF_PARM_AEC_LOCK;
    kTuningCmdToCamIntfCmd[TUNING_SET_AWB_LOCK] = CAM_INTF_PARM_AWB_LOCK;
    kTuningCmdToCamIntfCmd[TUNING_SET_AWB_UNLOCK] = CAM_INTF_PARM_AWB_LOCK;

    // unfortunatly non-literal strings are not interpreted until the pre-processor,
    // so we can check the size at compile time but cannot dump the value in the warning...
    static_assert(MAX_ISP_DATA_SIZE >= sizeof(vfe_diagnostics_t),
        "VFE metadata struct too small, increase HAL MAX_ISP_DATA_SIZE");
    static_assert(MAX_PP_DATA_SIZE >= sizeof(ez_pp_params_t),
        "PP metadata struct too small, increase HAL MAX_PP_DATA_SIZE");

    m_mode = mode;
    m_chromatix = new ::eztune_chromatix_t;
    memset(&m_metadata, 0, sizeof(metadata_buffer_t));
    memset(&m_af_driver, 0, sizeof(actuator_driver_params_t));
    memset(&m_af_tuning, 0, sizeof(ez_af_tuning_params_t));
    if (m_chromatix) {
        memset(m_chromatix, 0, sizeof(eztune_chromatix_t));
    }

    MMCAM_ASSERT(m_chromatix != NULL, "Null tuning pointers");

    if (EZTUNE_SERVER_CONTROL == mode) {
        m_chromatix->chromatixData = (uint8_t*)new ::chromatix_parms_type;
        m_chromatix->snap_chromatixData = (uint8_t*)new ::chromatix_parms_type;
        m_chromatix->common_chromatixData = (uint8_t*)new ::chromatix_VFE_common_type;
        m_chromatix->cpp_chromatixData = (uint8_t*)new ::chromatix_cpp_type;
        m_chromatix->snap_cpp_chromatixData = (uint8_t*)new ::chromatix_cpp_type;
        m_chromatix->postproc_chromatixData = (uint8_t*)new ::chromatix_sw_postproc_type;
        m_chromatix->aaa_chromatixData = (uint8_t*)new ::chromatix_3a_parms_type;

        MMCAM_ASSERT((m_chromatix->chromatixData) && (m_chromatix->snap_chromatixData) &&
                     (m_chromatix->common_chromatixData) && (m_chromatix->cpp_chromatixData) &&
                     (m_chromatix->snap_cpp_chromatixData) && (m_chromatix->postproc_chromatixData) &&
                     (m_chromatix->aaa_chromatixData),
                     "Null tuning pointers");
    }

    m_chromatix_init_done = false;
    m_af_init_done = false;
    m_pending_metadata_request = false;
    m_pending_preview_request = false;
    m_pending_snapshot_request = false;

    pthread_mutex_init(&m_lock, 0);
    pthread_mutex_init(&m_cond_mutex, 0);
    pthread_cond_init(&m_data_cond, 0);

    m_preview_image_width = 0;
    m_preview_image_height = 0;
    m_snapshot_image_width = 0;
    m_snapshot_image_height = 0;
    m_preview_format = CAM_FORMAT_YUV_420_NV21;
    m_snapshot_format = CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GRBG;
    m_preview_buffer = NULL;
    m_snapshot_buffer = NULL;
    m_old_preview_size = 0;
    m_curr_snapshot_size = 0;
}

CamAdapter::~CamAdapter()
{
    pthread_cond_destroy(&m_data_cond);
    pthread_mutex_destroy(&m_cond_mutex);
    pthread_mutex_destroy(&m_lock);

    if (EZTUNE_SERVER_CONTROL == m_mode) {
        delete m_chromatix->chromatixData;
        delete m_chromatix->snap_chromatixData;
        delete m_chromatix->common_chromatixData;
        delete m_chromatix->cpp_chromatixData;
        delete m_chromatix->snap_cpp_chromatixData;
        delete m_chromatix->postproc_chromatixData;
        delete m_chromatix->aaa_chromatixData;
    }

    delete m_chromatix;
    delete[] m_preview_buffer;
    delete[] m_snapshot_buffer;

    m_singleton = NULL;
}

void CamAdapter::DataWaitNotify()
{
    pthread_mutex_lock(&m_cond_mutex);
    pthread_cond_signal(&m_data_cond);
    pthread_mutex_unlock(&m_cond_mutex);
}

bool CamAdapter::DataWait()
{
    struct timespec   ts;
    struct timeval    tp;
    gettimeofday(&tp, NULL);
    ts.tv_sec  = tp.tv_sec;
    ts.tv_nsec = tp.tv_usec * 1000 + WAIT_TIME_MILLISECONDS * 1000000;

    pthread_mutex_lock(&m_cond_mutex);
    int rc = pthread_cond_timedwait(&m_data_cond, &m_cond_mutex, &ts);
    pthread_mutex_unlock(&m_cond_mutex);

    if (rc == ETIMEDOUT) {
        MMCAM_LOGE("error: DataWait timeout %d", WAIT_TIME_MILLISECONDS);
        return false;
    } else {
        return true;
    }
}

bool CamAdapter::SetupTuningTables(bool reset_ptr)
{
    bool rv = true;
    eztune_init_t ezctrl;

    // get chromatix data from sensor
    if(!m_chromatix_init_done || reset_ptr) {
        m_chromatix_init_done = FetchChromatixData();
        MMCAM_LOGI("EZTune: fetch chromatix (success %d)",
            m_chromatix_init_done);
    }
    // get aftune data from sensor
    if(!m_af_init_done || reset_ptr) {
        m_af_init_done = FetchAFTuneData();
        MMCAM_LOGI("EZTune: fetch af (success %d)",
            m_af_init_done);
    }

    // set function ptrs
    ezctrl.tuning_set_vfe = (void*)CamAdapter::TuningSetVfe;
    ezctrl.tuning_set_pp  = (void*)CamAdapter::TuningSetPproc;
    ezctrl.tuning_set_3a  = (void*)CamAdapter::TuningSet3A;
    ezctrl.tuning_set_focus = (void*)CamAdapter::TuningSetFocus;
    ezctrl.tuning_post_bus_msg = (void*)CamAdapter::TuningPostBusMsg;

    // set chromatix ptrs
    ezctrl.chromatixptr = (chromatix_parms_type *)m_chromatix->chromatixData;
    ezctrl.snap_chromatixptr = (chromatix_parms_type *)m_chromatix->snap_chromatixData;
    ezctrl.common_chromatixptr = (chromatix_VFE_common_type *)m_chromatix->common_chromatixData;
    ezctrl.cpp_chromatixptr = (chromatix_cpp_type *)m_chromatix->cpp_chromatixData;
    ezctrl.snap_cpp_chromatixptr = (chromatix_cpp_type *)m_chromatix->snap_cpp_chromatixData;
    ezctrl.swpp_chromatixptr = (chromatix_sw_postproc_type *)m_chromatix->postproc_chromatixData;
    ezctrl.aaa_chromatixptr = (chromatix_3a_parms_type*)m_chromatix->aaa_chromatixData;

    // set chromatix ptrs ZSL ovrrides
    mct_pipeline_t *pipeline = eztune_get_pipeline();
    bool zsl_enabled = (pipeline != NULL && pipeline->is_zsl_mode);
    if(zsl_enabled) {
        ezctrl.snap_chromatixptr = (chromatix_parms_type*)m_chromatix->chromatixData;
        ezctrl.snap_cpp_chromatixptr = (chromatix_cpp_type*)m_chromatix->cpp_chromatixData;
    }

    // set AF tuning ptrs
    ezctrl.af_driver_ptr = &m_af_driver;
    ezctrl.af_tuning_ptr = &m_af_tuning;

    // set metadata ptr
    ezctrl.metadata = &m_metadata;

    // Request fresh metadata.
    // Meta data is only fetched when the flag is toggled
    // Locks are used as the update is on a separate
    Lock();
    MMCAM_LOGV("Requesting metadata");
    m_pending_metadata_request = true;
    UnLock();

    if (!DataWait() == true) {
        MMCAM_LOGE("Error: failed to get metadata");
        rv = false;
    }

    // update lookup pointers
    eztune_lookup_init(&ezctrl);

    return rv;
}

bool CamAdapter::GetPreviewInfo(uint16_t &width, uint16_t &height, uint8_t &format, uint32_t &size)
{
    width = m_preview_image_width;
    height = m_preview_image_height;
    format = (uint8_t)GetPreviewFormat();
    size = m_old_preview_size;

    return true;
}

bool CamAdapter::GetJPEGSnapshotInfo(uint16_t &width, uint16_t &height, uint8_t &format, uint32_t &size)
{
    width = m_snapshot_image_width;
    height = m_snapshot_image_height;
    format = EZTUNE_FORMAT_JPG;
    size = m_curr_snapshot_size;

    return true;
}

uint8_t CamAdapter::GetRAWSnapshotBitWidth()
{
    uint8_t bit_width = 10;
    switch(m_snapshot_format) {
        case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GBRG:
        case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GRBG:
        case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_RGGB:
        case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_BGGR:
        case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_GBRG:
        case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_GRBG:
        case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_RGGB:
        case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_BGGR:
        case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_GBRG:
        case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_GRBG:
        case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_RGGB:
        case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_BGGR:
        case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_GBRG:
        case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_GRBG:
        case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_RGGB:
        case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_BGGR:
            bit_width = 8;
            break;
        case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GBRG:
        case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GRBG:
        case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_RGGB:
        case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_BGGR:
        case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_GBRG:
        case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_GRBG:
        case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_RGGB:
        case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_BGGR:
        case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_GBRG:
        case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_GRBG:
        case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_RGGB:
        case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_BGGR:
        case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_GBRG:
        case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_GRBG:
        case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_RGGB:
        case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_BGGR:
            bit_width = 10;
            break;
        case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GBRG:
        case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GRBG:
        case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_RGGB:
        case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_BGGR:
        case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_GBRG:
        case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_GRBG:
        case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_RGGB:
        case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_BGGR:
        case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_GBRG:
        case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_GRBG:
        case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_RGGB:
        case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_BGGR:
        case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_GBRG:
        case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_GRBG:
        case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_RGGB:
        case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_BGGR:
            bit_width = 12;
            break;
        default:
            break;
    }
    return bit_width;
}

uint8_t CamAdapter::GetRAWSnapshotPackType()
{
    // UNPACKED is the default type
    uint8_t pack_type = 1;
    switch(m_snapshot_format) {
        case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GBRG:
        case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GRBG:
        case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_RGGB:
        case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_BGGR:
        case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GBRG:
        case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GRBG:
        case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_RGGB:
        case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_BGGR:
        case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GBRG:
        case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GRBG:
        case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_RGGB:
        case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_BGGR:
        case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_GBRG:
        case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_GRBG:
        case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_RGGB:
        case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_BGGR:
        case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_GBRG:
        case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_GRBG:
        case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_RGGB:
        case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_BGGR:
        case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_GBRG:
        case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_GRBG:
        case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_RGGB:
        case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_BGGR:
        // MIPI format
            pack_type = 2;
            break;
        default:
            break;
    }
    return pack_type;
}

eztune_raw_format_t CamAdapter::GetRAWSnapshotBayerPattern()
{
    eztune_raw_format_t bayer_pattern = EZTUNE_FORMAT_BAYER_RGGB;
    switch(m_snapshot_format) {
        case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GBRG:
        case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GBRG:
        case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GBRG:
        case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_GBRG:
        case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_GBRG:
        case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_GBRG:
            bayer_pattern = EZTUNE_FORMAT_BAYER_GBRG;
            break;
        case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GRBG:
        case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GRBG:
        case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GRBG:
        case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_GRBG:
        case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_GRBG:
        case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_GRBG:
            bayer_pattern = EZTUNE_FORMAT_BAYER_GRBG;
            break;
        case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_RGGB:
        case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_RGGB:
        case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_RGGB:
        case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_RGGB:
        case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_RGGB:
        case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_RGGB:
            bayer_pattern = EZTUNE_FORMAT_BAYER_RGGB;
            break;
        case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_BGGR:
        case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_BGGR:
        case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_BGGR:
        case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_BGGR:
        case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_BGGR:
        case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_BGGR:
            bayer_pattern = EZTUNE_FORMAT_BAYER_BGGR;
            break;
        default:
            break;
    }
    return bayer_pattern;
}

uint8_t CamAdapter::GetPreviewFormat()
{
    eztune_prev_format_t prev_format = EZTUNE_FORMAT_YCrCb_420;

    switch(m_preview_format) {
        case CAM_FORMAT_YUV_420_NV12:
            prev_format = EZTUNE_FORMAT_YCbCr_420;
            break;
        case CAM_FORMAT_YUV_420_NV21:
            prev_format = EZTUNE_FORMAT_YCrCb_420;
            break;
        case CAM_FORMAT_YUV_422_NV16:
            prev_format = EZTUNE_FORMAT_YCbCr_422;
            break;
        case CAM_FORMAT_YUV_422_NV61:
            prev_format = EZTUNE_FORMAT_YCbCr_422;
            break;
        default:
            MMCAM_LOGE("Unsupported preview format");
            break;
    }
    MMCAM_LOGV("m_preview_format = %d, prev_format = %d", m_preview_format, prev_format);
    return prev_format;
}

bool CamAdapter::GetRAWSnapshotInfo(uint16_t &width, uint16_t &height, uint8_t &format, uint32_t &size,
                          uint8_t& bit_depth, uint8_t& pack_type)
{
    width = m_snapshot_image_width;
    height = m_snapshot_image_height;
    size = m_curr_snapshot_size;
    format = (uint8_t)GetRAWSnapshotBayerPattern();
    bit_depth = GetRAWSnapshotBitWidth();
    pack_type = GetRAWSnapshotPackType();
    return true;
}

bool CamAdapter::Get3AliveInfo(uint8_t &fps, uint16_t &width, uint16_t &height, uint8_t &format)
{
    fps = kEztuneMinFps; //tbd - get the mn fps from backend
    width = kEztuneScaledWidth;
    height = kEztuneScaledHeight;
    format = (uint8_t)GetPreviewFormat();

    return true;
}

void CamAdapter::TriggerPreviewBufferCopy(bool value)
{
    //set flag for preview buffer copy
    Lock();
    m_pending_preview_request = value;
    UnLock();
}

void CamAdapter::TriggerSnapshotBufferCopy(bool value)
{
    //set flag for snapshot buffer copy
    Lock();
    m_pending_snapshot_request = value;
    UnLock();
}

void CamAdapter::SetMetadataPending(bool value)
{
    //set flag for metadata copy
    Lock();
    m_pending_metadata_request = value;
    UnLock();
}

bool CamAdapter::TriggerSnapshot(eztune_bus_msg_t type)
{
    bool rv = false;

    switch (type) {
    case EZTUNE_BUSMSG_SNAP_RAW:
    case EZTUNE_BUSMSG_SNAP_JPEG:
        rv = TuningPostBusMsg(type, 0);
        if(rv)
            TriggerSnapshotBufferCopy(true);
        else
            MMCAM_LOGW("Failed to trigger snapshot");
        break;
    default:
        MMCAM_LOGE("Invalid snapshot request %d", type);
        break;
    }

    return rv;
}

void CamAdapter::GetPreviewDimension(uint32_t &width, uint32_t &height)
{
    width = m_preview_image_width;
    height = m_preview_image_height;

    return;
}

metadata_buffer_t* CamAdapter::GetMetadata()
{
    return &m_metadata;
}

void CamAdapter::MetadataLock()
{
    Lock();
}

void CamAdapter::MetadataUnlock()
{
    UnLock();
}

void CamAdapter::CopyPreviewImage(void *ptr)
{
    mct_stream_info_t *streaminfo = (mct_stream_info_t *)ptr;
    int32_t width = streaminfo->dim.width;
    int32_t  height = streaminfo->dim.height;
    cam_frame_len_offset_t plane_info = streaminfo->buf_planes.plane_info;
    cam_format_t format = streaminfo->fmt;

    uint32_t size = 0;
    for (uint32_t i = 0; i < (uint32_t)plane_info.num_planes; i++) {
        size += plane_info.mp[i].height * plane_info.mp[i].width;
    }
    MMCAM_ASSERT(size != 0, "Zero preview buffer size");

    //delete and allocate new buffer if size differs
    if (m_old_preview_size != size) {
        delete[] m_preview_buffer;
        //mem aligned because of possible fast cv operation on buffer
        bool ret = posix_memalign((void **)&m_preview_buffer, 128, size);
        //additional check for safety
        MMCAM_ASSERT(ret == 0, "posix_memalign returned failure");
        m_old_preview_size = size;
    }

    MMCAM_ASSERT(m_preview_buffer != NULL, "preview buffer null");

    //Actual buffer pointer is passed through the unused img_buffer_list
    uint8_t *buffer = (uint8_t *)streaminfo->img_buffer_list;
    uint8_t *out_buffer = m_preview_buffer;

    MMCAM_LOGV("Buffer (%p), width(%d), height(%d), format(%d), size(%zu)", buffer, width, height, format, m_old_preview_size);
    MMCAM_ASSERT(buffer != 0, "NULL input buffer");

    for (uint32_t i = 0; i < (uint32_t)plane_info.num_planes; i++) {
        uint32_t index = plane_info.mp[i].offset;
        if (i > 0) {
            index += plane_info.mp[i - 1].len;
        }
        for (int j = 0; j < plane_info.mp[i].height; j++) {
            void *data = (void *)(buffer + index);
            memcpy(out_buffer, data, plane_info.mp[i].width);
            index += plane_info.mp[i].stride;
            out_buffer = out_buffer + plane_info.mp[i].width;
        }
    }

    //update preview info
    m_preview_image_width = width;
    m_preview_image_height = height;
    m_preview_format = format;
}

eztune_bus_msg_t CamAdapter::CopySnapshotImage(void *ptr)
{
  cam_int_evt_params_t *params = (cam_int_evt_params_t *)ptr;

    MMCAM_ASSERT(params != NULL, "JPEG params null");
    MMCAM_ASSERT(params->path != NULL, "JPEG file path null");

    int file_fd = open(params->path, O_RDONLY);
    //assert on fd, as open should never fail in this case
    MMCAM_ASSERT(file_fd >= 0, "Snapshot file open failed");

    if ((size_t)m_curr_snapshot_size != params->size) {
        delete[] m_snapshot_buffer;
        m_snapshot_buffer = new uint8_t[params->size];
        m_curr_snapshot_size = params->size;
    }

    MMCAM_ASSERT(m_snapshot_buffer != NULL, "snapshot buffer null");

    ssize_t read_len = read(file_fd, m_snapshot_buffer, params->size);
    MMCAM_ASSERT(read_len == (ssize_t)params->size,
            "Snapshot file read returned less size than expected");

    close(file_fd);

    m_snapshot_image_width = params->dim.width;
    m_snapshot_image_height = params->dim.height;
    m_snapshot_format = params->picture_format;
    return (eztune_bus_msg_t)params->event_type;
}

void CamAdapter::SendProcessEvent(EztuneNotify event_id, void *data, uint32_t size, int pipe_fd)
{
    ProcessThreadMessage process_thrd_msg;

    process_thrd_msg.event_id = (uint32_t)event_id;
    process_thrd_msg.data = data;
    process_thrd_msg.size = size;
    int val = write(pipe_fd, &process_thrd_msg, sizeof(process_thrd_msg));
    MMCAM_ASSERT(val != -1, "Write to pipe failed, write returned: %s", strerror(errno));
}

void CamAdapter::EventNotify(uint32_t type, void *data, int pipe_fd, bool live_started)
{
    bool unlock_needed = false;

    if (TryLock() == 0) {
        unlock_needed = true;

        switch (type) {
        case EZTUNE_METADATA_NOTIFY:
            if (m_pending_metadata_request == true) {
                //make a copy of metadata, clear pending request and notify
                MMCAM_LOGV("Copying new metadata %p", data);
                memcpy((void *)&m_metadata, data, sizeof(metadata_buffer_t));
                m_pending_metadata_request = false;
                DataWaitNotify();
            }
            break;

        case EZTUNE_PREVIEW_NOTIFY:
            if (m_pending_preview_request == true) {
                //make a copy of preview buff, clear pending request and notify
                MMCAM_LOGV("Copying preview buffer");
                CopyPreviewImage(data);
                m_pending_preview_request = false;

                // need unlock before SendProcessEvent to avoid deadlock issue
                unlock_needed = false;
                UnLock();
                if (live_started == false) {
                    SendProcessEvent(EZTUNE_PREVIEW_NOTIFY, m_preview_buffer, m_old_preview_size, pipe_fd);
                } else {
                    SendProcessEvent(EZTUNE_3ALIVE_NOTIFY, m_preview_buffer, m_old_preview_size, pipe_fd);
                }
            }
            break;

        case EZTUNE_SNAPSHOT_NOTIFY:
            if (m_pending_snapshot_request == true) {
                //read the snapshot buff, clear pending request and notify
                //we can also read the snapshot buff in the proc thread, shall we?
                eztune_bus_msg_t img_type = CopySnapshotImage(data);
                m_pending_snapshot_request = false;

                // need unlock before SendProcessEvent to avoid deadlock issue
                unlock_needed = false;
                UnLock();
                if (img_type == EZTUNE_BUSMSG_SNAP_JPEG) {
                    MMCAM_LOGV("Copying snapshot buffer");
                    SendProcessEvent(EZTUNE_JPEG_SNAPSHOT_NOTIFY, m_snapshot_buffer,
                        m_curr_snapshot_size, pipe_fd);
                } else if (img_type == EZTUNE_BUSMSG_SNAP_RAW) {
                    MMCAM_LOGV("Copying snapshot buffer");
                    SendProcessEvent(EZTUNE_RAW_SNAPSHOT_NOTIFY, m_snapshot_buffer,
                        m_curr_snapshot_size, pipe_fd);
                } else {
                    MMCAM_LOGV("Snapshot open failed, no snapshot waiting");
                }
            }
            break;

        case EZTUNE_STREAMON_NOTIFY:
        case EZTUNE_STREAMOFF_NOTIFY:
        default:
            MMCAM_LOGW("Unknown Event: %d, Ignored", type);
            break;
        }

        if(unlock_needed) {
            UnLock();
        }

    } else {
        MMCAM_LOGV("Lock is held. Event skipped");
    }

end:
    return;
}

bool CamAdapter::ConfigureBufferDivert(bool enable)
{
    uint32_t val = (uint32_t)enable;
    return SendEvent(MCT_EVENT_MODULE_REQ_DIVERT, &val);
}

bool CamAdapter::FetchChromatixData()
{
    return SendEvent(MCT_EVENT_MODULE_EZTUNE_GET_CHROMATIX, m_chromatix);
}

bool CamAdapter::FetchAFTuneData()
{
    return SendEvent(MCT_EVENT_MODULE_EZTUNE_GET_AFTUNE, &m_af_driver);
}

bool CamAdapter::UpdateCamChromatixData()
{
    bool bChromatixUpdate = false;
    if(m_chromatix_init_done) {
        bChromatixUpdate = SendEvent(MCT_EVENT_MODULE_EZTUNE_SET_CHROMATIX, m_chromatix);
    } else {
        MMCAM_LOGE("EZTune:Error: update Chromatix data before initial fetch");
    }
    return bChromatixUpdate;
}

bool CamAdapter::UpdateCamAFTuneData()
{
    bool bAfTuneUpdate = false;
    if(m_af_init_done) {
        bAfTuneUpdate = SendEvent(MCT_EVENT_MODULE_EZTUNE_SET_AFTUNE, &m_af_driver);
    } else {
        MMCAM_LOGE("EZTune:Error: update AF data before initial fetch");
    }
    return bAfTuneUpdate;
}

// This is a special command that was used to route EZTune commands
// through the HAL.  This is being phased out.
bool CamAdapter::SendCmd(uint32_t cmd, void *value, bool get_param)
{
    mct_pipeline_t *pipeline = NULL;
    mct_stream_t *stream = NULL;

    MMCAM_LOGV("EZTune: Sending command: %u, Value: %p", cmd, value);

    pipeline = eztune_get_pipeline();
    if(!pipeline) {
        // not an error, server could be closing
        return false;
    }

    stream = eztune_get_stream_by_type(CAM_STREAM_TYPE_PREVIEW);
    if(!stream) {
        // catch non-ZSL snapshot case where no preview stream
        stream = eztune_get_stream_by_type(CAM_STREAM_TYPE_SNAPSHOT);
        MMCAM_LOGV("EZTune: sending messages in SNAPSHOT stream");
    }
    if(!stream) {
        MMCAM_LOGE("EZTune: stream is NULL");
        return false;
    }

    mct_event_t cmd_event;
    mct_event_control_t event_data;
    mct_event_control_parm_t event_parm;

    event_data.type = (get_param ?
        MCT_EVENT_CONTROL_GET_PARM : MCT_EVENT_CONTROL_SET_PARM);
    event_data.control_event_data = &event_parm;

    event_parm.type = (cam_intf_parm_type_t) cmd;
    event_parm.parm_data = value;

    if( !mct_pipeline_pack_event(
        MCT_EVENT_CONTROL_CMD,
        (pack_identity(MCT_PIPELINE_SESSION(pipeline), stream->streamid)),
        MCT_EVENT_DOWNSTREAM,
        &event_data, &cmd_event)){
        MMCAM_LOGE("EZTune: error in pack event");
        return false;
    }
    pipeline->send_event(pipeline, stream->streamid, &cmd_event);

    return true;
}

void CamAdapter::SendCmdWrapper(void *ptr, uint32_t cmd, void *value)
{
    if (ptr) {
        MMCAM_ASSERT(cmd < TUNING_SET_MAX, "Error mapping tuning cmd to intf cmd");
        cam_intf_parm_type_t intf_cmd = kTuningCmdToCamIntfCmd[(tune_set_t)cmd];

        CamAdapter *obj = static_cast<CamAdapter *>(ptr);
        obj->SendCmd((uint32_t)intf_cmd, value, false);
    } else {
        MMCAM_LOGW("Null pointer to obj. Send cmd ignored");
    }
    return ;
}

bool CamAdapter::SendEvent(mct_event_module_type_t type, void *data)
{
    bool rv = FALSE;
    uint32_t identity = 0;
    mct_pipeline_t *pipeline = NULL;
    mct_stream_t *stream = NULL;

    pipeline = eztune_get_pipeline();
    if(!pipeline) {
        // not an error, server could be closing
        return false;
    }

    stream = eztune_get_stream_by_type(CAM_STREAM_TYPE_PREVIEW);
    if(!stream) {
        // catch non-ZSL snapshot case where no preview stream
        stream = eztune_get_stream_by_type(CAM_STREAM_TYPE_SNAPSHOT);
        MMCAM_LOGE("EZTune: sending messages in SNAPSHOT stream");
    }
    if(!stream) {
        MMCAM_LOGE("EZTune: stream is NULL");
        return false;
    }

    identity = pack_identity(MCT_PIPELINE_SESSION(pipeline), stream->streamid);
    rv = eztune_send_event(type, identity, data);

    return rv;
}

void CamAdapter::TuningSetVfe(vfemodule_t module, optype_t optype, int32_t value)
{
    tune_cmd_t module_cmd;
    module_cmd.module = module;
    module_cmd.type = optype;
    module_cmd.value = value;

    SendCmdWrapper(m_singleton, TUNING_SET_VFE_COMMAND, (void *)&module_cmd);
}

void CamAdapter::TuningSetPproc(pp_module_t module, optype_t optype, int32_t value)
{
    tune_cmd_t module_cmd;
    module_cmd.module = module;
    module_cmd.type = optype;
    module_cmd.value = value;

    SendCmdWrapper(m_singleton, TUNING_SET_POSTPROC_COMMAND, (void *)&module_cmd);
}

void CamAdapter::TuningSet3A(aaa_set_optype_t optype, int32_t value)
{
    tune_set_t set_type = TUNING_SET_MAX;
    cam_eztune_cmd_data_t aaa_cmd;

    void *param = &aaa_cmd;

    switch (optype) {
    case EZ_STATUS:
        set_type = TUNING_SET_3A_COMMAND;
        aaa_cmd.cmd = CAM_EZTUNE_CMD_STATUS;
        aaa_cmd.u.running = value;
        break;

    case EZ_AEC_ENABLE:
        set_type = TUNING_SET_3A_COMMAND;
        aaa_cmd.cmd = CAM_EZTUNE_CMD_AEC_ENABLE;
        aaa_cmd.u.aec_enable = value;
        break;
    case EZ_AEC_TESTENABLE:
        break;
    case EZ_AEC_LOCK:
        if (value)
            set_type = TUNING_SET_AEC_LOCK;
        else
            set_type = TUNING_SET_AEC_UNLOCK;
        param = &value;
        break;
    case EZ_AEC_FORCEPREVEXPOSURE:
        set_type = TUNING_SET_3A_COMMAND;
        aaa_cmd.cmd = CAM_EZTUNE_CMD_AEC_FORCE_EXP;
        aaa_cmd.u.ez_force_param.forced = 1;
        aaa_cmd.u.ez_force_param.u.force_exp_value = (float)(value) / Q10;
        break;
    case EZ_AEC_FORCEPREVGAIN:
        set_type = TUNING_SET_3A_COMMAND;
        aaa_cmd.cmd = CAM_EZTUNE_CMD_AEC_FORCE_GAIN;
        aaa_cmd.u.ez_force_param.forced = 1;
        aaa_cmd.u.ez_force_param.u.force_gain_value = (float)(value) / Q10;
        break;
    case EZ_AEC_FORCEPREVLINECOUNT:
        set_type = TUNING_SET_3A_COMMAND;
        aaa_cmd.cmd = CAM_EZTUNE_CMD_AEC_FORCE_LINECOUNT;
        aaa_cmd.u.ez_force_param.forced = 1;
        aaa_cmd.u.ez_force_param.u.force_linecount_value = value;
        break;
    case EZ_AEC_FORCESNAPEXPOSURE:
        set_type = TUNING_SET_3A_COMMAND;
        aaa_cmd.cmd = CAM_EZTUNE_CMD_AEC_FORCE_SNAP_EXP;
        aaa_cmd.u.ez_force_param.forced = 1;
        aaa_cmd.u.ez_force_param.u.force_snap_exp_value = (float)(value) / Q10;;
        break;
    case EZ_AEC_FORCESNAPGAIN:
        set_type = TUNING_SET_3A_COMMAND;
        aaa_cmd.cmd = CAM_EZTUNE_CMD_AEC_FORCE_SNAP_GAIN;
        aaa_cmd.u.ez_force_param.forced = 1;
        aaa_cmd.u.ez_force_param.u.force_snap_gain_value = (float)(value) / Q10;
        break;
    case EZ_AEC_FORCESNAPLINECOUNT:
        set_type = TUNING_SET_3A_COMMAND;
        aaa_cmd.cmd = CAM_EZTUNE_CMD_AEC_FORCE_SNAP_LC;
        aaa_cmd.u.ez_force_param.forced = 1;
        aaa_cmd.u.ez_force_param.u.force_snap_linecount_value = value;
        break;
    case EZ_AWB_MODE:
        set_type = TUNING_SET_3A_COMMAND;
        aaa_cmd.cmd = CAM_EZTUNE_CMD_AWB_MODE;
        aaa_cmd.u.awb_mode = value;
        break;
    case EZ_AWB_ENABLE:
        set_type = TUNING_SET_3A_COMMAND;
        aaa_cmd.cmd = CAM_EZTUNE_CMD_AWB_ENABLE;
        aaa_cmd.u.awb_enable = value;
        break;
    case EZ_AWB_LOCK:
        if (value)
            set_type = TUNING_SET_AWB_LOCK;
        else
            set_type = TUNING_SET_AWB_UNLOCK;
        param = &value;
        break;
   case EZ_AWB_FORCE_DUAL_LED_IDX:
        set_type = TUNING_SET_3A_COMMAND;
        aaa_cmd.cmd = CAM_EZTUNE_CMD_AWB_FORCE_DUAL_LED_IDX;
        aaa_cmd.u.ez_force_dual_led_idx = value;
        break;
    case EZ_AF_ENABLE:
        set_type = TUNING_SET_3A_COMMAND;
        aaa_cmd.cmd = CAM_EZTUNE_CMD_AF_ENABLE;
        aaa_cmd.u.af_enable = value;
        break;
    default:
        break;
    }

    if (set_type != TUNING_SET_MAX) {
        SendCmdWrapper(m_singleton, set_type, param);
    } else {
        MMCAM_LOGW("No matching 3A command: %d", optype);
    }
}

void CamAdapter::TuningSetFocus(void *eztune_t_ptr, aftuning_optype_t optype, int32_t)
{
    eztune_t *ezctrl = (eztune_t*)eztune_t_ptr;
    tune_actuator_t act_tuning;

    memset(&act_tuning, 0, sizeof(tune_actuator_t));
    switch (optype) {
    case EZ_AF_LOADPARAMS:
        act_tuning.ttype = ACTUATOR_TUNE_RELOAD_PARAMS;
        break;
    case EZ_AF_LINEARTEST_ENABLE:
        act_tuning.ttype = ACTUATOR_TUNE_TEST_LINEAR;
        act_tuning.stepsize = ezctrl->af_tuning_ptr->linearstepsize;
        break;
    case EZ_AF_RINGTEST_ENABLE:
        act_tuning.ttype = ACTUATOR_TUNE_TEST_RING;
        act_tuning.stepsize = ezctrl->af_tuning_ptr->ringstepsize;
        break;
    case EZ_AF_MOVFOCUSTEST_ENABLE:
        act_tuning.ttype = ACTUATOR_TUNE_MOVE_FOCUS;
        act_tuning.direction = ezctrl->af_tuning_ptr->movfocdirection;
        act_tuning.num_steps = ezctrl->af_tuning_ptr->movfocsteps;
        break;
    case EZ_AF_DEFFOCUSTEST_ENABLE:
        act_tuning.ttype = ACTUATOR_TUNE_DEF_FOCUS;
        break;
    }

    SendCmdWrapper(m_singleton, TUNING_SET_AUTOFOCUS_TUNING, (void*)&act_tuning);
}

bool CamAdapter::TuningPostBusMsg(eztune_bus_msg_t optype, int32_t msg)
{
    bool rv = FALSE;
    mct_pipeline_t *pipeline = NULL;

    switch(optype) {
        case EZTUNE_BUSMSG_FLASHMODE:
            rv = eztune_post_msg_to_bus(MCT_BUS_MSG_LED_MODE_OVERRIDE, sizeof(cam_flash_mode_t), &msg);
            break;
        case EZTUNE_BUSMSG_SNAP_JPEG:
            rv = eztune_post_msg_to_bus(MCT_BUS_MSG_EZTUNE_JPEG, 0, NULL);
            break;
        case EZTUNE_BUSMSG_SNAP_RAW:
            pipeline = eztune_get_pipeline();
            if(!pipeline || pipeline->is_zsl_mode) {
                rv = FALSE;
                break;
            }
            rv = eztune_post_msg_to_bus(MCT_BUS_MSG_EZTUNE_RAW, 0, NULL);
            break;
        default:
            MMCAM_LOGW("No matching command: %d", optype);
            break;
    }
    return rv;
}

};
