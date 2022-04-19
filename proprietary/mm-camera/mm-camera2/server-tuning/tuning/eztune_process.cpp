/***************************************************************************
 * Copyright (c) 2014-2015,2016-2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 ***************************************************************************/

//=============================================================================
/**
 * This file contains eztune process class implementation
 */
//=============================================================================

#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>
#include <poll.h>
#ifdef USE_FASTCV_OPT
#include <fastcv/fastcv.h>
#endif
#include <dlfcn.h>

#include "eztune_process.h"

namespace eztune {
const char kImgTransInfoMajorVersion = 1;
const char kImgTransInfoMinorVersion = 0;
const uint16_t kImgTransInfoHeaderSize = 6;
const char kImgTransInfoTargetType = 1; //Linux
const char kImgTransInfoCapabilities = 0x1 | 0x2 | 0x4 | 0x10; //Preview | JPEG | RAW | 3A Streaming
const uint16_t kPreviewInfoChunkSize = 7168;
const uint16_t kPreviewMaxChunkSize = 10240;

const uint8_t kChunkStatusSuccess = 0;
const uint8_t kChunkStatusNewSize = 2;

char kMagicStr[] = "Qualcomm Camera Debug";
const char kAEString[] = "QCAEC";
const char kAWBString[] = "QCAWB";
const char kAFString[] = "QCAF";
const char kASDString[] = "QCASD";
const char kStatsString[] = "3a stats";
const char kBHistStatsString[] = "QCHST";
const char k3ATuningString[] = "QCTUN";

uint32_t kMaxExifSize = AEC_DEBUG_DATA_SIZE + AWB_DEBUG_DATA_SIZE +
                         AF_DEBUG_DATA_SIZE + ASD_DEBUG_DATA_SIZE +
                         STATS_BUFFER_DEBUG_DATA_SIZE + BHIST_STATS_DEBUG_DATA_SIZE +
                         TUNING_INFO_DEBUG_DATA_SIZE +
                         32 +
                         sizeof(kMagicStr) + sizeof(kAEString) + sizeof(kAWBString) +
                         sizeof(kAFString) + sizeof(kASDString) + sizeof(kStatsString) +
                         sizeof(kBHistStatsString) + sizeof(k3ATuningString);

typedef enum {
    EZTUNE_ORIGIN_TOP_LEFT = 1,
    EZTUNE_ORIGIN_BOTTOM_LEFT,
    EZTUNE_ORIGIN_INVALID
} EztuneImgOriginType;

enum class TuneCmd : uint16_t {
    TUNESERVER_GET_PREVIEW_INFO = 1,
    TUNESERVER_CHANGE_CHUNK_SIZE = 2,
    TUNESERVER_GETPREVIEW_FRAME = 3,
    TUNESERVER_GETSNAPSHOT_FRAME = 4,
    TUNESERVER_GETRAW_FRAME = 5,
    TUNESERVER_3ALIVE_START = 7,
    TUNESERVER_3ALIVE_STOP = 8,
    TUNESERVER_GET_LIST = 1014,
    TUNESERVER_GET_PARMS = 1015,
    TUNESERVER_SET_PARMS = 1016,
    TUNESERVER_MISC_CMDS = 1021,
};

typedef struct {
    uint8_t status;
    uint16_t width;
    uint16_t height;
    uint8_t format;
    uint8_t origin;
    uint32_t frame_size;
} PreviewFrameHeader;

typedef struct  {
    uint8_t major_ver;
    uint8_t minor_ver;
} PreviewInfoVersion;

typedef struct  {
    uint16_t header_size;
    uint8_t target_type;
    uint8_t capabilities;
    uint32_t chunk_size;
} PreviewInfoHeader;

//
//struct holding fastcv dynamic loading information
//p_fcv_lib_ptr - pointer to dynamically loaded lib
// and dynamically loaded functiona pointers
//
static struct {
    void *p_fcv_lib_ptr;
#ifdef USE_FASTCV_OPT
    int (*pFcvSetOperationMode)( fcvOperationMode mode );
#else
    int (*pFcvSetOperationMode)( int mode );
#endif
    void (*pFcvScaleDownMNu8)(
        const uint8_t* __restrict src,
        uint32_t                  srcWidth,
        uint32_t                  srcHeight,
        uint32_t                  srcStride,
        uint8_t* __restrict       dst,
        uint32_t                  dstWidth,
        uint32_t                  dstHeight,
        uint32_t                  dstStride );
    void (*pFcvScaleDownMNInterleaveu8)(
        const uint8_t* __restrict src,
        uint32_t                  srcWidth,
        uint32_t                  srcHeight,
        uint32_t                  srcStride,
        uint8_t* __restrict       dst,
        uint32_t                  dstWidth,
        uint32_t                  dstHeight,
        uint32_t                  dstStride );
} gFastCVDLInfo;

//Define below macro to enable preview frame dumps in a file for debug
//#define DEBUG_FILE_DUMP
#ifdef DEBUG_FILE_DUMP
#define DUMP_FILE_NAME QCAMERA_DUMP_FRM_LOCATION"process_img.yuv"
static int32_t file_fd = -1;

//only opens file one time, if file exits will not open
void debug_open_dump_file()
{
    if (access(DUMP_FILE_NAME, F_OK) != -1) {
        file_fd = -1;
    } else {
        file_fd = open(DUMP_FILE_NAME, O_RDWR | O_CREAT, 0777);
    }
        if (file_fd < 0)
            MMCAM_LOGI("File open error: %s",  strerror(errno));
    }
}

void debug_close_dump_file()
{
    if (file_fd >= 0)
        close(file_fd);
}

void debug_write_dump_file(char *ptr, uint32_t size)
{
    if (file_fd >= 0)
        write(file_fd, ptr,  size);
}
#else //DEBUG_FILE_DUMP
void debug_open_dump_file() {}
void debug_close_dump_file() {}
void debug_write_dump_file(char*, uint32_t) {}
#endif //DEBUG_FILE_DUMP

//Define below macro to enable eztune params dump in a file for debug
//#define DEBUG_EZTUNE_PARAMS
#ifdef DEBUG_EZTUNE_PARAMS

#define DUMP_FILE_NAME  QCAMERA_DUMP_FRM_LOCATION"eztune_params.txt"
static FILE *params_file_fp = 0;

//only opens file one time, if file exits will not open
void debug_open_paramsdump_file()
{
    params_file_fp = fopen(DUMP_FILE_NAME, "w+");
    if (!params_file_fp)
        MMCAM_LOGI("File open error: %s",  strerror(errno));
}

void debug_close_paramsdump_file()
{
    if (params_file_fp)
        fclose(params_file_fp);
}

#define debug_write_paramsdump_file(...) (\
{ \
    if (params_file_fp) \
        fprintf(params_file_fp, __VA_ARGS__); \
} \
)

#else //DEBUG_EZTUNE_PARAMS

void debug_open_paramsdump_file() {}
void debug_close_paramsdump_file() {}
void debug_write_paramsdump_file(...) {}
#endif //DEBUG_EZTUNE_PARAMS

static boolean eztune_queue_item_free(void *data, void*)
{
    if(data) {
        free(data);
        data = NULL;
    }
     return TRUE;
}

ProcessLayer::ProcessLayer(eztune_server_t mode, void *camera_adapter_handle)
    : m_response(eztune::kMaxProtocolBufferSize)
{
    m_proc_thread_created = false;
    m_chunk_size = kPreviewInfoChunkSize;
    m_scaled_buffer = NULL;
    m_exif_buffer = NULL;
    //size of a VGA YCbCr 420 (NV12/21) buffer
    m_scaled_preview_size = kEztuneScaledSize;
    m_exif_size = 0;

    pthread_mutex_init(&m_cond_mutex, 0);
    pthread_mutex_init(&m_lock, 0);
    pthread_cond_init(&m_data_cond, 0);

    //rest of the initialization
    m_connected = false;
    m_items_added = 0;
    m_pending_set_param_apply = false;
    m_3Alive_active = false;
    m_params_dump_fp = NULL;
    m_mode = mode;
    m_notify_pipe_fds[0] = -1;
    m_notify_pipe_fds[1] = -1;

    m_cam_adapter = new eztune::CamAdapter(camera_adapter_handle, mode);

    m_pending_cmds = (mct_queue_t *)malloc(sizeof(mct_queue_t));
    m_resume_pending_cmds = (mct_queue_t *)malloc(sizeof(mct_queue_t));
    if(!m_pending_cmds) {
        MMCAM_LOGE("%s:failed to create m_pending_cmds queue",__func__);
        return;
    }
    if(!m_resume_pending_cmds) {
        MMCAM_LOGE("%s:failed to create m_resume_pending_cmds queue",__func__);
        return;
    }
    mct_queue_init(m_pending_cmds);
    mct_queue_init(m_resume_pending_cmds);
}

ProcessLayer::~ProcessLayer()
{
    pthread_cond_destroy(&m_data_cond);
    pthread_mutex_destroy(&m_cond_mutex);
    pthread_mutex_destroy(&m_lock);

    delete m_cam_adapter;

    mct_queue_free_all(m_pending_cmds, eztune_queue_item_free);
    m_pending_cmds = NULL;
    mct_queue_free_all(m_resume_pending_cmds, eztune_queue_item_free);
    m_resume_pending_cmds = NULL;
}

size_t ProcessLayer::SettingsFileSetItem(eztune_set_val_t *item, FILE* fp)
{
    size_t num;
    char enabled[255];
    property_get(EZTUNE_PROP_STICKY_SETTINGS, enabled, "0");

    // If sticky settings is disabled to not add params to the file.
    if(fp == NULL || (strncmp(enabled, "1", 1) != 0)) {
        return 0;
    }

    fseek (fp, 0, SEEK_END);
    num = fwrite(item, sizeof(eztune_set_val_t), 1, fp);
    fflush(fp);

    if(num < 1) {
        MMCAM_LOGE("eztune: couldn't write command to file, error: %s", strerror(errno));
    }
    return num;
}

size_t ProcessLayer::SettingsFileGetItem(eztune_set_val_t *item, bool start_over, FILE* fp)
{
    size_t ret_num;
    if(fp == NULL) {
        return 0;
    }

    // move read pointer to begining of file
    if(start_over) {
        fseek(fp, 0, SEEK_SET);
    }

    memset(item, 0, sizeof(eztune_set_val_t));
    ret_num = fread(item, sizeof(eztune_set_val_t), 1, fp);
    if(ret_num > 0) {
        MMCAM_LOGV("eztune: SettingsFileGetItem entry %d index %d value %s",
            item->item_num, item->table_index, item->value_string);
    }

    MMCAM_ASSERT(ret_num <= 0 || (item->item_num >= 0 && item->item_num < EZT_PARMS_MAX),
        "Invalid file state get data");
    return ret_num;
}

FILE* ProcessLayer::SettingsFileOpen(const char *fname, bool force_create)
{
    FILE *fp;
    char enabled[255];

    property_get(EZTUNE_PROP_STICKY_SETTINGS, enabled, "0");
    if(strncmp(enabled, "1", 1) != 0) {
        return NULL;
    }

    // force file creation if requested
    if(force_create) {
        fp = fopen(fname, "ab+");
        if (fp == NULL) {
            MMCAM_LOGE("eztune: file ab+ open fail, returning NULL");
            return NULL;
        }
        fclose(fp);
    }

    // open the file with random location read/write enabled
    return fopen(fname, "rb+");
}

void ProcessLayer::SettingsFileClose(FILE** fp)
{
    MMCAM_LOGV("eztune: SettingsFileClose %p", (fp != NULL ? *fp : NULL));
    if ((fp != NULL) && (*fp != NULL)) {
        fclose(*fp);
        *fp = NULL;
    }
 }

void ProcessLayer::ProcessAndGenerateResponse(uint16_t cmd, size_t payload_size, string &payload, size_t &response_size, string &response)
{
    switch(cmd) {
    case (uint16_t)TuneCmd::TUNESERVER_GET_LIST:
        MMCAM_LOGV("%s, cmd TUNESERVER_GET_LIST", __func__);
        this->ProcessGetListCmd(payload_size, payload, response_size, response);
        break;
    case (uint16_t)TuneCmd::TUNESERVER_GET_PARMS:
        MMCAM_LOGV("%s, cmd TUNESERVER_GET_PARMS", __func__);
        this->ProcessGetParamCmd(payload_size, payload, response_size, response);
        break;
    case (uint16_t)TuneCmd::TUNESERVER_SET_PARMS:
        MMCAM_LOGV("%s, cmd TUNESERVER_SET_PARMS", __func__);
        this->ProcessSetParamCmd(payload_size, payload, response_size, response);
        break;
    case (uint16_t)TuneCmd::TUNESERVER_MISC_CMDS:
        MMCAM_LOGV("%s, cmd TUNESERVER_MISC_CMDS", __func__);
        this->ProcessMiscCmd(payload_size, payload, response_size, response);
        break;
    case (uint16_t)TuneCmd::TUNESERVER_GET_PREVIEW_INFO:
        MMCAM_LOGV("%s, cmd TUNESERVER_GET_PREVIEW_INFO", __func__);
        this->ProcessImgTransInfo(payload_size, payload, response_size, response);
        break;
    case (uint16_t)TuneCmd::TUNESERVER_CHANGE_CHUNK_SIZE:
        MMCAM_LOGV("%s, cmd TUNESERVER_CHANGE_CHUNK_SIZE", __func__);
        this->ProcessChangeChunkSize(payload_size, payload, response_size, response);
        break;
    case (uint16_t)TuneCmd::TUNESERVER_GETPREVIEW_FRAME:
        MMCAM_LOGV("%s, cmd TUNESERVER_GETPREVIEW_FRAME", __func__);
        this->ProcessGetPreviewFrame(payload_size, payload, response_size, response);
        break;
    case (uint16_t)TuneCmd::TUNESERVER_GETSNAPSHOT_FRAME:
        MMCAM_LOGV("%s, cmd TUNESERVER_GETPREVIEW_FRAME", __func__);
        this->ProcessGetSnapshotFrame(payload_size, payload, response_size, response);
        break;
    case (uint16_t)TuneCmd::TUNESERVER_GETRAW_FRAME:
        MMCAM_LOGV("%s, cmd TUNESERVER_GETRAW_FRAME", __func__);
        this->ProcessGetRawFrame(payload_size, payload, response_size, response);
        break;
    case (uint16_t)TuneCmd::TUNESERVER_3ALIVE_START:
        MMCAM_LOGV("%s, cmd TUNESERVER_3ALIVE_START", __func__);
        this->ProcessStart3Alive(payload_size, payload, response_size, response);
        break;
    case (uint16_t)TuneCmd::TUNESERVER_3ALIVE_STOP:
        MMCAM_LOGV("%s, cmd TUNESERVER_3ALIVE_STOP", __func__);
        this->ProcessStop3Alive(payload_size, payload, response_size, response);
        break;
    default:
        MMCAM_LOGV("%s, error invalid command: %d", __func__, cmd);
        response_size = 0;
        goto error;
    }

error:
    return;
}

void* ProcessLayer::ResumeTuningSessionWrapper(void *ptr)
{
    ProcessLayer *obj = static_cast<ProcessLayer *>(ptr);
    obj->ResumeTuningSession();
    MMCAM_LOGV("ResumeTuningSession thread started");
    // Call pthread_detach() to let the thread related resouces
    // to be released to the system as soon as the pthread_exit()
    // is executed. In this case, when the function returns.
    pthread_detach(pthread_self());
    return  NULL;
}

void ProcessLayer::ResumeTuningSession()
{
    bool rv;
    eztune_set_val_t item;
    FILE* fp = NULL;

    // only need to load the settings for the control channel
    if(m_mode != EZTUNE_SERVER_CONTROL) {
        return;
    }

    //request buffer-diverts from PPROC container
    if(m_connected) {
      m_cam_adapter->ConfigureBufferDivert(TRUE);
    }

    //If there is an intermediate file, apply all commands in the file.
    //Tuning user is responsible for deleting this file to start fresh
    if (!MCT_QUEUE_IS_EMPTY(m_resume_pending_cmds))
        mct_queue_flush(m_resume_pending_cmds, eztune_queue_item_free);
    fp = SettingsFileOpen(kDumpBinaryFile, false);
    rv = SettingsFileGetItem(&item, true, fp);
    while (rv > 0) {
        eztune_set_val_t *elem = (eztune_set_val_t *)malloc(sizeof(eztune_set_val_t));
        MMCAM_ASSERT(elem != NULL, "eztune: allocation failure");

        memcpy(elem, &item, sizeof(eztune_set_val_t));
        mct_queue_push_tail(m_resume_pending_cmds, elem);
        rv = SettingsFileGetItem(&item, false, fp);
    }
    SettingsFileClose(&fp);

    MMCAM_LOGI("EZTune: ResumeTuningSession applying %d items", m_resume_pending_cmds->length);
    if (m_resume_pending_cmds->length > 0) {
        m_cam_adapter->SetupTuningTables(false);
        ApplyItems(m_resume_pending_cmds );
    } else if(m_connected) {
        // override chromatix settings when Chromatix is connected
        m_cam_adapter->SetupTuningTables(false);
        m_cam_adapter->UpdateCamChromatixData();
        m_cam_adapter->UpdateCamAFTuneData();
    }
}

bool ProcessLayer::Init(int pipe_fd)
{
    m_connected = true;
    if (m_mode == EZTUNE_SERVER_CONTROL) {
        ResumeTuningSession();

        CamAdapter::TuningSetVfe(VFE_MODULE_ALL, SET_STATUS, 1);
        CamAdapter::TuningSetPproc(PP_MODULE_ALL, SET_STATUS, 1);
        CamAdapter::TuningSet3A(EZ_STATUS, 1);

        debug_open_paramsdump_file();
        m_params_dump_fp = SettingsFileOpen(kDumpBinaryFile, true);
    } else {
        //Check if there is any stale process thread and release it.
        // Stale process thread can be possible when DeInit is called
        // before the thread's starting routine is scheduled
        StopProcessThread();
        //store the eztune thread pipe_fd here
        m_intf_pipe_fd = pipe_fd;

        //create pipe for communication with adapter
        //only in case of image port
        int val = pipe(m_notify_pipe_fds);
        MMCAM_ASSERT(val == 0, "Pipe creation failed: %s", strerror(errno));

        //create process worker thread
        val = pthread_create(&m_proc_thread, NULL, ProcessLayer::ProcThreadWrapper, static_cast<void *>(this));
        MMCAM_ASSERT(val == 0, "Process thread creation failed: %s", strerror(errno));
        pthread_setname_np(m_proc_thread, "CAM_eztune");
    }

    MMCAM_LOGI("EZTune: Init done - %s",
        (m_mode == EZTUNE_SERVER_CONTROL ? "SERVER" : "PREVIEW"));
    return true;
}

bool ProcessLayer::DeInit()
{
    // This typically happens when the client closes the connection
    // first and then the camera app is closed. Its safe to ignore this
    //Deinit request
    if(m_connected == false) {
        MMCAM_LOGE("Process layer not initialized. just return");
        return true;
    }

    if (m_mode == EZTUNE_SERVER_CONTROL) {
        SettingsFileClose(&m_params_dump_fp);
        debug_close_paramsdump_file();

        CamAdapter::TuningSetVfe(VFE_MODULE_ALL, SET_STATUS, 0);
        CamAdapter::TuningSetPproc(PP_MODULE_ALL, SET_STATUS, 0);
        CamAdapter::TuningSet3A(EZ_STATUS, 0);

        // do not bother stop buffer-diverts from PPROC
        // This avoids needing to track is any down-stream modules
        // are already subscribed to buffer-diverts.  Since EZtune
        // is only used for tuning we can keep-it-simple and just
        // leave buffer diverts on when we need them.
        //m_cam_adapter->ConfigureBufferDivert(FALSE);
    } else {
        StopProcessThread();
    }

    m_connected = false;

    MMCAM_LOGI("EZTune: DeInit done - %s",
        (m_mode == EZTUNE_SERVER_CONTROL ? "SERVER" : "PREVIEW"));
    return true;
}

void ProcessLayer::DataWaitNotify()
{
    pthread_mutex_lock(&m_cond_mutex);
    pthread_cond_signal(&m_data_cond);
    pthread_mutex_unlock(&m_cond_mutex);
}

bool ProcessLayer::DataWait()
{
    struct timespec   ts;
    struct timeval    tp;
    gettimeofday(&tp, NULL);
    ts.tv_sec  = tp.tv_sec;
    ts.tv_nsec = tp.tv_usec * 1000 + WAIT_TIME_MILLISECONDS * 1000000;

    pthread_mutex_lock(&m_cond_mutex);
    int rc = pthread_cond_timedwait(&m_data_cond, &m_cond_mutex, &ts);
    pthread_mutex_unlock(&m_cond_mutex);

    if (rc == ETIMEDOUT)
        return false;
    else
        return true;
}

void ProcessLayer::SendEventToInterface(uint32_t event_id, void *data, uint32_t size)
{
    InterfaceThreadMessage intf_thrd_msg;

    intf_thrd_msg.type = event_id;
    intf_thrd_msg.payload_ptr = data;
    intf_thrd_msg.payload_size = size;
    int val = write(m_intf_pipe_fd, &intf_thrd_msg, sizeof(intf_thrd_msg));
    MMCAM_ASSERT(val != -1, "Write to pipe failed, write returned: %s", strerror(errno));
}

void ProcessLayer::StopProcessThread()
{
    ProcessThreadMessage process_thread_message;
    int val = 0;

    MMCAM_LOGV("m_proc_thread_created = %d", m_proc_thread_created);
    // if the C-L or any Client app never connected, process thread
    //will not be created, and it will cause a crash if we post the message
    if (m_proc_thread_created == false) {
        goto end;
    }

    process_thread_message.event_id = EZTUNE_STOP_NOTIFY;
    process_thread_message.data = NULL;
    process_thread_message.size = 0;
    val = write(m_notify_pipe_fds[1], &process_thread_message, sizeof(process_thread_message));
    MMCAM_ASSERT(val != -1, "Write to pipe failed, write returned: %s", strerror(errno));

    val = pthread_join(m_proc_thread, NULL);
    MMCAM_ASSERT(val == 0, "pthread_join returned error: %s", strerror(errno));

    if (m_notify_pipe_fds[0] >= 0) {
        close(m_notify_pipe_fds[0]);
        m_notify_pipe_fds[0] = -1;
    }
    if (m_notify_pipe_fds[1] >= 0) {
        close(m_notify_pipe_fds[1]);
        m_notify_pipe_fds[1] = -1;
    }
end:
    return;
}


bool ProcessLayer::Get3ALiveStatus()
{
    bool livestatus = false;
    pthread_mutex_lock(&m_lock);
    livestatus = m_3Alive_active;
    pthread_mutex_unlock(&m_lock);
    return livestatus;
}

void ProcessLayer::NotifyBack()
{
    if (Get3ALiveStatus() == true) {
        //get ready for the next preview buffer
        m_cam_adapter->TriggerPreviewBufferCopy(true);
        //get ready for next metadata
        m_cam_adapter->SetMetadataPending(true);
    }
}

void ProcessLayer::ScaleDownImage(void *data)
{
#ifdef USE_FASTCV_OPT
    uint8_t *src_buff = (uint8_t *)data;
    uint32_t width = 0, height = 0;
    int32_t ret = 0;

    //if fastcv is not loaded, load it.
    if (NULL == gFastCVDLInfo.p_fcv_lib_ptr) {
        gFastCVDLInfo.p_fcv_lib_ptr = dlopen("libfastcvopt.so", RTLD_NOW);

        if (NULL != gFastCVDLInfo.p_fcv_lib_ptr) {

            *(void **)&(gFastCVDLInfo.pFcvScaleDownMNInterleaveu8) =
              dlsym(gFastCVDLInfo.p_fcv_lib_ptr, "fcvScaleDownMNInterleaveu8");

            *(void **)&(gFastCVDLInfo.pFcvScaleDownMNu8) =
              dlsym(gFastCVDLInfo.p_fcv_lib_ptr, "fcvScaleDownMNu8");

            *(void **)&(gFastCVDLInfo.pFcvSetOperationMode) =
              dlsym(gFastCVDLInfo.p_fcv_lib_ptr, "fcvSetOperationMode");

            if (gFastCVDLInfo.pFcvScaleDownMNInterleaveu8 == NULL ||
                gFastCVDLInfo.pFcvScaleDownMNu8 == NULL ||
                gFastCVDLInfo.pFcvSetOperationMode == NULL ) {
              MMCAM_LOGV("%s%d] Loading libfastcvopt error",
                __func__, __LINE__);
              dlclose(gFastCVDLInfo.p_fcv_lib_ptr);
              gFastCVDLInfo.p_fcv_lib_ptr = NULL;
            }
        }
    }
    MMCAM_ASSERT(NULL != gFastCVDLInfo.p_fcv_lib_ptr && gFastCVDLInfo.pFcvScaleDownMNu8 != NULL,
        "Downscaler FCV not loaded");

    m_cam_adapter->GetPreviewDimension(width, height);
    MMCAM_ASSERT(width > 0 && height > 0, "Invalid Preview dimensions");

    if (m_scaled_buffer == NULL) {
        //fastcv needs 128 bit aligned buffers
        ret = posix_memalign((void **)&m_scaled_buffer, 128, m_scaled_preview_size);
        MMCAM_ASSERT(ret == 0 && m_scaled_buffer != NULL, "posix_memalign returned failure");
    }

    if (width <= kEztuneScaledWidth && height <= kEztuneScaledHeight) {
        MMCAM_LOGV("Scaled dimensions >= preview, using preview dimension directly for 3A");
        m_scaled_preview_size = (width * height *3) >> 1;
        //we can avoid this memcpy and directly use the preview buffer
        memcpy(m_scaled_buffer, data, m_scaled_preview_size);
        return;
    }

    uint8_t *src_buff_cbcr = src_buff + (width * height);
    uint8_t *dst_buff_cbcr = m_scaled_buffer + kEztuneScaledLumaSize;

    //set fast cv operation mode and scale down Y and CbCr buffers separately
    gFastCVDLInfo.pFcvSetOperationMode(FASTCV_OP_CPU_PERFORMANCE);
    gFastCVDLInfo.pFcvScaleDownMNu8(src_buff, width, height, 0, m_scaled_buffer, kEztuneScaledWidth, kEztuneScaledHeight, 0);
    gFastCVDLInfo.pFcvScaleDownMNInterleaveu8(src_buff_cbcr,  width>>1, height>>1, 0, dst_buff_cbcr, kEztuneScaledWidth>>1, kEztuneScaledHeight>>1, 0);

    MMCAM_LOGV("Scaled down image from (%zu/%zu) to (%zu/%zu)", width, height, kEztuneScaledWidth, kEztuneScaledHeight);
#endif
    return;
}

void ProcessLayer::GetEXIFHeader()
{
    int32_t ret = 0;
    bool ae_debug_params_valid = false;
    bool awb_debug_params_valid = false;
    bool af_debug_params_valid = false;
    bool asd_debug_params_valid = false;
    bool stats_debug_params_valid = false;
    bool bhist_debug_params_valid = false;
    bool q3a_tuning_debug_params_valid = false;

    // Major Revision|Minor Revision|Patch Revision
    // 5.2.2 = 05020002
    uint8_t version[4];
    version[0] = 5;
    version[1] = 2;
    version[2] = 0;
    version[3] = 2;
    // 3A version is hardcoded to 5.0.0.0 for now
    // This will be removed when the backend support is available
    // to read the 3A version runtime.
    uint64_t aaa_version = 0x05;

    m_cam_adapter->MetadataLock();
    metadata_buffer_t *pMetaData = m_cam_adapter->GetMetadata();
    MMCAM_ASSERT(pMetaData != NULL, "metadata not available");

    m_exif_size = 0;

    IF_META_AVAILABLE(cam_ae_exif_debug_t, ae_exif_debug_params,
            CAM_INTF_META_EXIF_DEBUG_AE, pMetaData) {
        ae_debug_params_valid = true;
    }
    IF_META_AVAILABLE(cam_awb_exif_debug_t, awb_exif_debug_params,
            CAM_INTF_META_EXIF_DEBUG_AWB, pMetaData) {
        awb_debug_params_valid = true;
    }
    IF_META_AVAILABLE(cam_af_exif_debug_t, af_exif_debug_params,
            CAM_INTF_META_EXIF_DEBUG_AF, pMetaData) {
        af_debug_params_valid = true;
    }
    IF_META_AVAILABLE(cam_asd_exif_debug_t, asd_exif_debug_params,
            CAM_INTF_META_EXIF_DEBUG_ASD, pMetaData) {
        asd_debug_params_valid = true;
    }
    IF_META_AVAILABLE(cam_stats_buffer_exif_debug_t, stats_exif_debug_params,
            CAM_INTF_META_EXIF_DEBUG_STATS, pMetaData) {
        stats_debug_params_valid = true;
    }
    IF_META_AVAILABLE(cam_bhist_buffer_exif_debug_t, bhist_exif_debug_params,
            CAM_INTF_META_EXIF_DEBUG_BHIST, pMetaData) {
        bhist_debug_params_valid = true;
    }
    IF_META_AVAILABLE(cam_q3a_tuning_info_t, q3a_tuning_exif_debug_params,
            CAM_INTF_META_EXIF_DEBUG_3A_TUNING, pMetaData) {
        q3a_tuning_debug_params_valid = true;
    }

    //measure the overall size of the exif buffer reqd
    if (ae_debug_params_valid) {
        m_exif_size += sizeof(kAEString);
        m_exif_size += 4;
        m_exif_size += ae_exif_debug_params->aec_debug_data_size;
    }

    if (awb_debug_params_valid) {
        m_exif_size += sizeof(kAWBString);
        m_exif_size += 4;
        m_exif_size += awb_exif_debug_params->awb_debug_data_size;
    }

    if (af_debug_params_valid) {
        m_exif_size += sizeof(kAFString);
        m_exif_size += 4;
        m_exif_size += af_exif_debug_params->af_debug_data_size;
    }

    if (asd_debug_params_valid) {
        m_exif_size += sizeof(kASDString);
        m_exif_size += 4;
        m_exif_size += asd_exif_debug_params->asd_debug_data_size;
    }

    if (stats_debug_params_valid) {
        m_exif_size += sizeof(kStatsString);
        m_exif_size += 4;
        m_exif_size += stats_exif_debug_params->bg_stats_buffer_size +
                       stats_exif_debug_params->bg_config_buffer_size;
    }

    if (bhist_debug_params_valid) {
        m_exif_size += sizeof(kBHistStatsString);
        m_exif_size += 4;
        m_exif_size += bhist_exif_debug_params->bhist_stats_buffer_size;
    }

    if (q3a_tuning_debug_params_valid) {
        m_exif_size += sizeof(k3ATuningString);
        m_exif_size += 4;
        m_exif_size += q3a_tuning_exif_debug_params->tuning_info_buffer_size;
    }

    if (m_exif_size) {
        m_exif_size += sizeof(kMagicStr);
        m_exif_size += sizeof(version);
        m_exif_size += sizeof(aaa_version);
    }else {
        m_cam_adapter->MetadataUnlock();
        return;
    }

    //allocate the exif header buffer
    if (m_exif_buffer == NULL) {
        ret = posix_memalign((void **)&m_exif_buffer, 32, kMaxExifSize);
        MMCAM_ASSERT(ret == 0 && m_exif_buffer != NULL, "posix_memalign returned failure");
    }

    uint8_t *tmp_exif_buff = m_exif_buffer;

    //write "Qualcomm Camera Debug"
    memcpy(tmp_exif_buff, kMagicStr, sizeof(kMagicStr));
    tmp_exif_buff += sizeof(kMagicStr);

    //write version number
    memcpy(tmp_exif_buff, &version, sizeof(version));
    tmp_exif_buff += sizeof(version);

    //write 3a version number
    memcpy(tmp_exif_buff, &aaa_version, sizeof(aaa_version));
    tmp_exif_buff += sizeof(aaa_version);


    //write awb data
    if (awb_debug_params_valid) {
        //write awb identifier
        memcpy(tmp_exif_buff, kAWBString, sizeof(kAWBString));
        tmp_exif_buff += sizeof(kAWBString);
        //write awb data size
        memcpy(tmp_exif_buff, &awb_exif_debug_params->awb_debug_data_size,
                          sizeof(awb_exif_debug_params->awb_debug_data_size));
        tmp_exif_buff += sizeof(awb_exif_debug_params->awb_debug_data_size);
        //write awb payload
        memcpy(tmp_exif_buff, &awb_exif_debug_params->awb_private_debug_data[0],
                                      awb_exif_debug_params->awb_debug_data_size);
        tmp_exif_buff += awb_exif_debug_params->awb_debug_data_size;
        MMCAM_LOGV("Written AWB EXIF header");
    }

    //write aec data
    if (ae_debug_params_valid) {
        //write aec identifier
        memcpy(tmp_exif_buff, kAEString, sizeof(kAEString));
        tmp_exif_buff += sizeof(kAEString);
        //write aec data size
        memcpy(tmp_exif_buff, &ae_exif_debug_params->aec_debug_data_size,
                         sizeof(ae_exif_debug_params->aec_debug_data_size));
        tmp_exif_buff += sizeof(ae_exif_debug_params->aec_debug_data_size);
        //write aec payload
        memcpy(tmp_exif_buff, &ae_exif_debug_params->aec_private_debug_data[0],
                                      ae_exif_debug_params->aec_debug_data_size);
        tmp_exif_buff += ae_exif_debug_params->aec_debug_data_size;
        MMCAM_LOGV("Written AEC EXIF header");
    }

    //write af data
    if (af_debug_params_valid) {
        //write af identifier
        memcpy(tmp_exif_buff, kAFString, sizeof(kAFString));
        tmp_exif_buff += sizeof(kAFString);
        //write af data size
        memcpy(tmp_exif_buff, &af_exif_debug_params->af_debug_data_size,
                         sizeof(af_exif_debug_params->af_debug_data_size));
        tmp_exif_buff += sizeof(af_exif_debug_params->af_debug_data_size);
        //write af payload
        memcpy(tmp_exif_buff, &af_exif_debug_params->af_private_debug_data[0],
                                      af_exif_debug_params->af_debug_data_size);
        tmp_exif_buff += af_exif_debug_params->af_debug_data_size;
        MMCAM_LOGV("Written AF EXIF header");
    }

    //write asd data
    if (asd_debug_params_valid) {
        //write asd identifier
        memcpy(tmp_exif_buff, kASDString, sizeof(kASDString));
        tmp_exif_buff += sizeof(kASDString);
        //write asd data size
        memcpy(tmp_exif_buff, &asd_exif_debug_params->asd_debug_data_size,
                         sizeof(asd_exif_debug_params->asd_debug_data_size));
        tmp_exif_buff += sizeof(asd_exif_debug_params->asd_debug_data_size);
        //write asd payload
        memcpy(tmp_exif_buff, &asd_exif_debug_params->asd_private_debug_data[0],
                                     asd_exif_debug_params->asd_debug_data_size);
        tmp_exif_buff += asd_exif_debug_params->asd_debug_data_size;
        MMCAM_LOGV("Written ASD EXIF header");
    }

    //write stats data
    if (stats_debug_params_valid) {
        //write stats identifier
        memcpy(tmp_exif_buff, kStatsString, sizeof(kStatsString));
        tmp_exif_buff += sizeof(kStatsString);
        //write stats data size
        int32_t data_size = (stats_exif_debug_params->bg_stats_buffer_size);
        memcpy(tmp_exif_buff, &data_size, sizeof(data_size));
        tmp_exif_buff += sizeof(data_size);
        //write stats payload
        memcpy(tmp_exif_buff, &stats_exif_debug_params->stats_buffer_private_debug_data[0], data_size);
        tmp_exif_buff += data_size;//this is not really needed
        MMCAM_LOGV("Written Stats EXIF header");
    }

     //write BHIST data
    if (bhist_debug_params_valid) {
        //write stats identifier
        memcpy(tmp_exif_buff, kBHistStatsString, sizeof(kBHistStatsString));
        tmp_exif_buff += sizeof(kBHistStatsString);
        //write bhist stats data size
        int32_t data_size = (bhist_exif_debug_params->bhist_stats_buffer_size);

        memcpy(tmp_exif_buff, &data_size, sizeof(data_size));
        tmp_exif_buff += sizeof(data_size);

        //write bhist stats payload
        memcpy(tmp_exif_buff, &bhist_exif_debug_params->bhist_private_debug_data[0], data_size);
        tmp_exif_buff += data_size;//this is not really needed
        MMCAM_LOGV("Written BHIST EXIF header");
    }

    m_cam_adapter->MetadataUnlock();

    MMCAM_LOGV("Processed EXIF header");

    return;
}

void ProcessLayer::Process3ALiveData(void *data, string &response)
{
    uint16_t marker = 0xFE7E;//start of 3ALive packet
    int8_t status = 0;
    struct timeval curr_time;
    unsigned long mtime, seconds, useconds;

    //start with clear string
    response.clear();

    if(false == Get3ALiveStatus()) {
        MMCAM_LOGE("%s 3A live stopped, Exit", __func__);
        return;
    }

    //pack the marker
    response.append((char *)&marker, sizeof(marker));
    //pack the status
    response.append((char *)&status, sizeof(status));

    //pack the relative timestamp
    gettimeofday(&curr_time, NULL);
    mtime =
     ((curr_time.tv_sec * 1000) + (curr_time.tv_usec / 1000)) -
     ((m_prev_time.tv_sec * 1000) + (m_prev_time.tv_usec / 1000));

    //pack the relative timestamp
    response.append((char *)&mtime, sizeof(mtime));
    MMCAM_LOGV("Relative timestamp: (%ld)", mtime);

    //get EXIF header
    GetEXIFHeader();
    if (m_exif_size == 0) {
        response.clear();
        MMCAM_LOGE("no 3A header found, nothing to send to C-L");
        return;
    }

    //downscale image to VGA
    ScaleDownImage(data);

    MMCAM_LOGV("3A EXIF header size: (%zu), Scaled down image size: (%zu)", m_exif_size, m_scaled_preview_size);

    //pack downscaled image size
    response.append((char *)&m_scaled_preview_size, sizeof(m_scaled_preview_size));
    //pack 3A EXIF header size
    response.append((char *)&m_exif_size, sizeof(m_exif_size));
    //pack downscaled image
    response.append((char *)m_scaled_buffer, m_scaled_preview_size);
    //pack 3A EXIF header
    response.append((char *)m_exif_buffer, m_exif_size);

    MMCAM_LOGV("Appended EXIF header and Scaled down image");

    return;
}

void* ProcessLayer::ProcThreadWrapper(void *ptr)
{
    ProcessLayer *obj = static_cast<ProcessLayer *>(ptr);
    MMCAM_ASSERT(!obj->m_proc_thread_created, "eztune: thread already running");

    obj->ProcessThread();

    MMCAM_LOGV("exiting proc thread");
    return  NULL;
}

void ProcessLayer::ProcessThread()
{
    boolean exit_thread = false;
    int32_t num_fds = 1, ready = 0, read_bytes = 0;
    ProcessThreadMessage process_thread_message;
    struct pollfd pollfds;

    m_proc_thread_created = true;
    while (exit_thread == false) {
        pollfds.fd = m_notify_pipe_fds[0];
        pollfds.events = POLLIN|POLLPRI;
        ready = poll(&pollfds, (nfds_t)num_fds, -1);
        if (ready > 0) {
            if (pollfds.revents & (POLLIN|POLLPRI)) {
              read_bytes = read(pollfds.fd, &process_thread_message,
                sizeof(ProcessThreadMessage));
              if ((read_bytes < 0) ||
                  (read_bytes != sizeof(ProcessThreadMessage))) {
                MMCAM_LOGE("failed: read_bytes %d", read_bytes);
                continue;
              }

              switch (process_thread_message.event_id) {
                case EZTUNE_PREVIEW_NOTIFY:
                case EZTUNE_JPEG_SNAPSHOT_NOTIFY:
                case EZTUNE_RAW_SNAPSHOT_NOTIFY:
                  MMCAM_LOGV("%s, Event-id = %d", __func__, process_thread_message.event_id);
                  m_response.clear();
                  PrepareResponse(m_response, process_thread_message.event_id);
                  m_response.append((char *)process_thread_message.data, process_thread_message.size);
                  //send the async event to the eztune interface thread
                  SendEventToInterface(EZTUNE_INTF_ASYNC_RESP, (void *)&m_response, m_response.size());
                  break;

                case EZTUNE_3ALIVE_NOTIFY:
                  //process the 3A live data -
                  Process3ALiveData(process_thread_message.data, m_response);
                  //send the async event to the eztune interface thread
                  if (m_response.size()) { //there was a valid EXIF header, so packed it with preview and send to host
                    SendEventToInterface(EZTUNE_INTF_ASYNC_RESP, (void *)&m_response, m_response.size());
                  }else { //there was no valid EXIF header, nothing to send and just continue for the next set
                      //get ready for the next preview buffer
                      m_cam_adapter->TriggerPreviewBufferCopy(true);
                      //get ready for next metadata
                      m_cam_adapter->SetMetadataPending(true);
                  }
                  break;

                case EZTUNE_STOP_NOTIFY:
                  exit_thread = true;
                  break;

                default:
                  MMCAM_LOGE("invalid event type %d", process_thread_message.event_id);
                  break;
              }
            }
        } else if (ready <= 0) {
            MMCAM_LOGE("failed: exit thread");
            break;
        }
    }

    pthread_mutex_lock(&m_cond_mutex);
    m_proc_thread_created = FALSE;
    pthread_mutex_unlock(&m_cond_mutex);

    return;
}

void ProcessLayer::ProcessImgTransInfo(size_t, string&,
                                       size_t &response_size, string &response)
{
    response.clear();

    PreviewInfoVersion version = {
        .major_ver = kImgTransInfoMajorVersion,
        .minor_ver = kImgTransInfoMinorVersion
    };

    response.append((char *)&version, sizeof(version));

    PreviewInfoHeader header = {
        .header_size = kImgTransInfoHeaderSize,
        .target_type = kImgTransInfoTargetType,
        .capabilities = kImgTransInfoCapabilities,
        .chunk_size = kPreviewInfoChunkSize
    };

    response.append((char *)&header, sizeof(header));

    response_size = response.size();

    MMCAM_LOGV("%s: Size(%zu)", __func__, response_size);

    return;
}

void ProcessLayer::ProcessChangeChunkSize(size_t, string &payload,
                                          size_t &response_size, string &response)
{
    uint32_t new_chunk_size = *(uint32_t *)payload.data();
    uint8_t status;
    uint32_t chunk_size;

    if (new_chunk_size <= kPreviewMaxChunkSize) {
        status = kChunkStatusSuccess;
        chunk_size = new_chunk_size;
    } else {
        status = kChunkStatusNewSize;
        chunk_size = kPreviewMaxChunkSize;
    }
    m_chunk_size = chunk_size;

    response.clear();
    response.append((char *)&status, sizeof(status));
    response.append((char *)&chunk_size, sizeof(chunk_size));
    response_size = response.size();

    MMCAM_LOGV("%s: Size(%zu)", __func__, response_size);

    return;
}

void ProcessLayer::ProcessGetPreviewFrame(size_t, string&,
                                          size_t &response_size, string &response)
{
    int8_t status = 0;
    //start with clear string
    response.clear();

    if (Get3ALiveStatus() == true) {
        //set status as failure
        status = 1;
        response.append((char *)&status, sizeof(status));
        MMCAM_LOGV("3A Live streaming enabled");
    }
    else {
        //set preview buffer pending
        m_cam_adapter->TriggerPreviewBufferCopy(true);
    }

    response_size = response.size();
    MMCAM_LOGV("%s: Size(%zu)", __func__, response_size);

    return;
}

void ProcessLayer::ProcessGetSnapshotFrame(size_t, string&,
                                           size_t &response_size, string &response)
{
    int8_t status = 0;
    //start with clear string
    response.clear();

    if (Get3ALiveStatus() == true) {
        //set status as failure
        status = 1;
        response.append((char *)&status, sizeof(status));
        MMCAM_LOGI("Eztune Snapshot can not be triggered while 3A Live streaming enabled");
    }
    else {
        bool ret = m_cam_adapter->TriggerSnapshot(EZTUNE_BUSMSG_SNAP_JPEG);
        if (ret == false) {
            //Could not trigger snapshot
            //set status as failure
            status = 1;
            response.append((char *)&status, sizeof(status));
            MMCAM_LOGI("Snapshot could not be triggered");
       }
    }
    response_size = response.size();
    MMCAM_LOGV("%s: Size(%zu)", __func__, response_size);

    return;
}

void ProcessLayer::ProcessGetRawFrame(size_t, string&,
                                      size_t &response_size, string &response)
{
    int8_t status = 0;
    //start with clear string
    response.clear();

    bool ret = m_cam_adapter->TriggerSnapshot(EZTUNE_BUSMSG_SNAP_RAW);
    if (ret == false) {
        //Could not trigger snapshot
        //set status as failure
        status = 1;
        response.append((char *)&status, sizeof(status));
        MMCAM_LOGI("Snapshot could not be triggered");
    }

    response_size = response.size();
    MMCAM_LOGV("%s: Size(%zu)", __func__, response_size);

    return;
}

void ProcessLayer::ProcessStart3Alive(size_t payload_size, string &payload, size_t &response_size, string &response)
{
    int8_t status = 0;

    //get rid of compiler warnings
    (void)payload_size;
    (void)payload;

    //start with clear string
    response.clear();

    MMCAM_LOGV("Received 3A Live Start");

    pthread_mutex_lock(&m_lock);
    m_3Alive_active = true;
    pthread_mutex_unlock(&m_lock);

    //set status as success
    status = 0;
    response.append((char *)&status, sizeof(status));

    uint8_t fps;
    uint16_t width;
    uint16_t height;
    uint8_t format;
    uint8_t origin = EZTUNE_ORIGIN_BOTTOM_LEFT;

    m_cam_adapter->Get3AliveInfo(fps,
                                 width,
                                 height,
                                 format);

    //add 3Alive Start response
    response.append((char *)&fps, sizeof(fps));
    response.append((char *)&width, sizeof(width));
    response.append((char *)&height, sizeof(height));
    response.append((char *)&format, sizeof(format));
    response.append((char *)&origin, sizeof(origin));

    //we need to keep checking for new metadata
    m_cam_adapter->SetMetadataPending(true);
    //set preview buffer pending
    m_cam_adapter->TriggerPreviewBufferCopy(true);
    //set the expected scaled buffer size
    m_scaled_preview_size = kEztuneScaledSize;

    gettimeofday(&m_prev_time, NULL);

    response_size = response.size();
    MMCAM_LOGV("%s: Size(%zu)", __func__, response_size);

    return;
}

void ProcessLayer::ProcessStop3Alive(size_t payload_size, string &payload, size_t &response_size, string &response)
{
    uint16_t marker = 0xFE7E;//start of 3ALive packet
    int8_t status = 1;
    uint32_t size = 0;
    uint32_t mtime = 0;

    //get rid of compiler warnings
    (void)payload_size;
    (void)payload;

    MMCAM_LOGV("Received 3A Live Stop");

    pthread_mutex_lock(&m_lock);
    m_3Alive_active = false;
    pthread_mutex_unlock(&m_lock);

    m_scaled_preview_size = 0;
    m_exif_size = 0;

    //start with clear string
    response.clear();

    //pack the marker
    response.append((char *)&marker, sizeof(marker));
    //pack the status = 1 = END
    response.append((char *)&status, sizeof(status));
    //no valid timestamp for stop response
    response.append((char *)&mtime, sizeof(mtime));
    //zero image data size
    response.append((char *)&m_scaled_preview_size, sizeof(m_scaled_preview_size));
    //zero 3A packet size
    response.append((char *)&m_exif_size, sizeof(m_exif_size));

    //we no longer need metadata
    m_cam_adapter->SetMetadataPending(false);
    //we no longer need preview
    m_cam_adapter->TriggerPreviewBufferCopy(false);

    response_size = response.size();
    MMCAM_LOGV("%s: Size(%zu)", __func__, response_size);

    return;
}

void ProcessLayer::ProcessGetListCmd(size_t, string&,
                                     size_t &response_size, string &response)
{
    MMCAM_ASSERT_PRE(m_connected == true, "Process Layer is not yet initialized");

    //start with clear string
    response.clear();

    //set default first 3 bytes for status and number of records.
    //This gets overwritten to actual number of records outside the for loop
    response.append("\x00", 1);
    response.append((const char *)&m_items_added, 2);

    MMCAM_LOGV("Number of items to add: %d, Max buffer size: %zu",
        (EZT_PARMS_MAX - m_items_added), response_size);

    int i;
    for (i = m_items_added; i < EZT_PARMS_MAX; i++) {

        eztune_item_t item = eztune_get_item(i);

        //check if enough space to insert current entry
        if (response.size() >= (response_size - 8 - strlen(item.name) - 1))
            break;

        //insert index
        response.append((const char *)&i, 2);

        //insert offset
        if (item.offset)
            response.append((const char *)&item.entry_count, 2);
        else
            response.append("\x01\x00", 2);

        //insert flag
        if (item.reg_flag == EZT_WRITE_FLAG) {
            response.append("\x00\x00\x00\x00", 4);
        } else if (item.reg_flag == EZT_READ_FLAG || item.reg_flag == EZT_3A_FLAG) {
            response.append("\x01\x00\x00\x00", 4);
        } else if (item.reg_flag == EZT_CHROMATIX_FLAG) {
            response.append("\x40\x00\x00\x00", 4);
        } else if (item.reg_flag == (EZT_CHROMATIX_FLAG | EZT_READ_FLAG)) {
            response.append("\x41\x00\x00\x00", 4);
        } else if (item.reg_flag == EZT_AUTOFOCUS_FLAG) {
            response.append("\x00\x04\x00\x00", 4);
        } else if (item.reg_flag == (EZT_AUTOFOCUS_FLAG | EZT_READ_FLAG)) {
            response.append("\x01\x04\x00\x00", 4);
        }

        //insert name
        response.append(item.name, strlen(item.name) + 1);
    }

    //replace the character 0 if not all params are added
    if (i != EZT_PARMS_MAX)
        response.replace(0, 1, "\x01");

    //replace character 1 and 2 to to indicate number of items added
    uint16_t items_added = i - m_items_added;
    response.replace(1, 2, (const char *)&items_added);

    //update number of items added
    if (i == EZT_PARMS_MAX) {
        m_items_added = 0;
        MMCAM_LOGI("All getlist Items added");
    } else {
        m_items_added = i;
    }

    MMCAM_LOGV("Number of items added: %d, Response buffer size: %zu",
        items_added, response.size());

    //fill remaining data with 0 bytes
    MMCAM_ASSERT(response_size >= response.size(), "response string overflow");
    response.append(response_size - response.size(), 0);

    return;
}

void ProcessLayer::ProcessGetParamCmd(size_t, string &payload,
                                      size_t &response_size, string &response)
{
    MMCAM_ASSERT_PRE(m_connected == true, "Process Layer is not yet initialized");

    const char *input = payload.data();

    uint16_t num_items =  *(uint16_t *)input;
    input = input + sizeof(uint16_t);

    response.clear();

    if (m_pending_set_param_apply == true)
        response.append("\x01", 1);
    else
        response.append("\x00", 1);

    //insert number of items
    response.append((const char *)&num_items, sizeof(uint16_t));

    MMCAM_LOGV("Number of params to get: %d, Max buffer size: %zu", num_items, response_size);

    //get new tuning tables only if non zero items needs to be fetched
    //PS: eztune host tool periodically sends get param with no items
    if (num_items) {
        if (m_cam_adapter->SetupTuningTables(false) == false) {
            MMCAM_LOGW("Get Tuning tables failed perhaps due to timeout");
            //reset number of items added as zero
            uint16_t num_items_added = 0;
            response.replace(1, 2, (const char *)&num_items_added);
            goto end;
        }
    }

    //debug
    debug_write_paramsdump_file("Adding GetParams. Num params: %d\n", num_items);

    int i;
    for (i = 0; i < num_items; ++i) {
        int rc = -1;

        uint16_t item_num = *(uint16_t *)input;
        input = input + sizeof(uint16_t);

        uint16_t table_index = *(uint16_t *)input;
        input = input + sizeof(uint16_t);

        //check if enough space to insert current entry
        if (response.size() >= (response_size - 4 - 64)) {
            break;
        }

        //insert item number and table index
        response.append((const char *)&item_num, sizeof(uint16_t));
        response.append((const char *)&table_index, sizeof(uint16_t));

        //fetch the required item and then extract info
        char temp_string[EZTUNE_FORMAT_MAX];
        eztune_item_t item = eztune_get_item(item_num);
        rc = eztune_get_item_value(&item, table_index, temp_string);

        //debug
        debug_write_paramsdump_file("Item Name: %s, Item no: %d, Table Index: %d, Value: %s\n",
            item.name, item_num, table_index, temp_string);
        MMCAM_LOGV("%s: Item Name: %s, Item no: %d, Table Index: %d, Value: %s\n", __func__,
            item.name, item_num, table_index, temp_string);

        response.append((char *)temp_string, rc + 1);
    }

    MMCAM_LOGV("Number of params obtained: %d, Response buffer size: %zu", i, response.size());

    //debug
    debug_write_paramsdump_file("End of GetParams: Num items written: %d\n\n", i);

    //replace the number of items added with actual elements written
    response.replace(1, 2, (const char *)&i);

end:
    //fill remaining data with 0 bytes
    MMCAM_ASSERT(response_size >= response.size(), "response string overflow");
    response.append(response_size - response.size(), 0);
    return;
}

void ProcessLayer::PrepareResponse( string &response, int event_id)
{
    int8_t status = 0;
    response.append((char *)&status, sizeof(status));

    uint16_t width = 0, height = 0, crop_dummy = 0;
    uint8_t format = 0, bit_depth = 0, pack_type = 0;
    uint32_t frame_size = 0;

    switch (event_id) {
      case EZTUNE_PREVIEW_NOTIFY:
          m_cam_adapter->GetPreviewInfo(width,
                                        height,
                                        format,
                                        frame_size);
          break;
      case EZTUNE_JPEG_SNAPSHOT_NOTIFY:
          m_cam_adapter->GetJPEGSnapshotInfo(width,
                                            height,
                                            format,
                                            frame_size);
          break;
      case EZTUNE_RAW_SNAPSHOT_NOTIFY:
          m_cam_adapter->GetRAWSnapshotInfo(width,
                                            height,
                                            format,
                                            frame_size,
                                            bit_depth,
                                            pack_type);
          break;
      default:
          MMCAM_LOGV("%s Invalid Event Notified, Id = %d", __func__, event_id);
          break;
    }
    uint8_t origin = EZTUNE_ORIGIN_BOTTOM_LEFT;

    response.append((char *)&width, sizeof(width));
    response.append((char *)&height, sizeof(height));
    response.append((char *)&format, sizeof(format));
    response.append((char *)&origin, sizeof(origin));
    if (EZTUNE_RAW_SNAPSHOT_NOTIFY == event_id) {
        response.append((char *)&bit_depth, sizeof(bit_depth));
        response.append((char *)&pack_type, sizeof(pack_type));
        response.append((char *)&crop_dummy, sizeof(crop_dummy));
        response.append((char *)&crop_dummy, sizeof(crop_dummy));
        response.append((char *)&crop_dummy, sizeof(crop_dummy));
        response.append((char *)&crop_dummy, sizeof(crop_dummy));
    }
    response.append((char *)&frame_size, sizeof(frame_size));
}

static void debug_item(eztune_set_val_t item)
{
    MMCAM_LOGV("Item num(%d), Item Index(%d), Item string(%s)", item.item_num, item.table_index, item.value_string);

    //debug
    eztune_item_t item_details = eztune_get_item(item.item_num);
    debug_write_paramsdump_file("Item name(%s), Item num(%d), Table Index(%d), Value(%s)\n", item_details.name, item.item_num, item.table_index, item.value_string);
}

void ProcessLayer::ProcessSetParamCmd(size_t, string &payload,
                                      size_t &response_size, string &response)
{
    MMCAM_ASSERT_PRE(m_connected == true, "Process Layer is not yet initialized");

    const char *input = payload.data();

    uint16_t num_items =  *(uint16_t *)input;
    input = input + sizeof(uint16_t);

    MMCAM_ASSERT(num_items < EZT_PARMS_MAX, "Invalid number of items: %u", num_items);

    MMCAM_LOGV("%s, num_items: %d", __func__, num_items);

    //debug
    debug_write_paramsdump_file("Starting SetParams: Num items requested: %d", num_items);

    int i;
    for (i = 0; i < num_items; ++i) {
        eztune_set_val_t *item = (eztune_set_val_t *)malloc(sizeof(eztune_set_val_t));
        MMCAM_ASSERT(item != NULL, "malloc failed");

        item->item_num = *(uint16_t *)input;
        input = input + sizeof(uint16_t);

        item->table_index = *(uint16_t *)input;
        input = input + sizeof(uint16_t);

        MMCAM_ASSERT(strlen(input) < sizeof(eztune_set_val_t::value_string), "Item value too long");
        strlcpy(item->value_string, input, sizeof(eztune_set_val_t::value_string));
        input = input + strlen(input) + 1;

        debug_item(*item);
        MMCAM_LOGV("%s, item->item_num: %d item->table_index: %d value: %s", __func__,
            item->item_num, item->table_index, item->value_string);

        // Maintain state of Eztune incase C-L disconnects.
        // Write settings to a file and read them back during init.
        // User must delete the file if they want a fresh start
        size_t num = SettingsFileSetItem(item, m_params_dump_fp);

        //push to pending cmds vector. This gets applied during mscl command
        mct_queue_push_tail(m_pending_cmds, item);
    }

    //debug
    debug_write_paramsdump_file("Completed SetParams: Num items written: %d", i);

    m_pending_set_param_apply = true;

    response.clear();

    //set response string
    response.append("\x01", 1);

    //fill remaining data with 0 bytes
    MMCAM_ASSERT(response_size >= response.size(), "response string overflow");
    response.append(response_size - response.size(), 0);

    return;
}

void ProcessLayer::ApplyItems(mct_queue_t *pending_cmds_queue)
{
    eztune_item_t item;
    bool update_chromatix = false;
    bool update_aftune = false;

    MMCAM_ASSERT(pending_cmds_queue != NULL, "Null cmd vector pointter");
    debug_write_paramsdump_file("Starting Apply. Number of params %zu\n", pending_cmds_queue->length);

    //get elements from the pending cmds vector and apply change
    while(pending_cmds_queue->length > 0) {
        eztune_set_val_t *elem = (eztune_set_val_t *)mct_queue_pop_head(pending_cmds_queue);
        MMCAM_ASSERT(elem != NULL, "null element in queue");

        item = eztune_get_item(elem->item_num);
        MMCAM_ASSERT((item.id >= 0) && (item.id < EZT_PARMS_MAX),
                     "Out of range index ID: %d", item.id);

        debug_item(*elem);

        eztune_set_item_value(&item, elem);
        MMCAM_LOGV("%s: item->item_num: %d item->table_index: %d value: %s", __func__,
            elem->item_num, elem->table_index, elem->value_string);

        if (item.type == EZT_T_CHROMATIX)
            update_chromatix = true;
        if (item.type == EZT_T_AUTOFOCUS)
            update_aftune = true;
        if(elem != NULL) {
        free(elem);
        elem = NULL;
    }
    }

    if (update_chromatix) {
        m_cam_adapter->UpdateCamChromatixData();
    }
    if (update_aftune) {
        m_cam_adapter->UpdateCamAFTuneData();
    }

    debug_write_paramsdump_file("Completed Apply. Cleared size %zu\n\n", pending_cmds_queue->length);

    return;
}

void ProcessLayer::ProcessMiscCmd(size_t, string &payload,
                                  size_t &response_size, string &response)
{
    MMCAM_ASSERT_PRE(m_connected == true, "Process Layer is not yet initialized");

    const char *data = payload.data();
    uint8_t cmd = *(uint8_t *)data;

    response.clear();

    if (cmd == EZTUNE_MISC_GET_VERSION) {
        MMCAM_LOGI("Get version command");
        //insert version number and return
        response.append(eztune::kEztuneVersion);
        goto end;

    } else if (cmd == EZTUNE_MISC_APPLY_CHANGES) {
        MMCAM_LOGI("%s: Apply changes command, cmd_ct = %d", __func__,
            m_pending_cmds->length);

        if (m_cam_adapter->SetupTuningTables(false) == false) {
            MMCAM_LOGW("Get Tuning tables failed perhaps due to timeout");
            response.clear();
            response_size = 0;
            goto end;
        }

        ApplyItems(m_pending_cmds);

        //Assign response
        //TODO: Instead of setting default to 00 (no error), check error condition
        //and set the string accordingly
        response.append("\x00", 1);

        m_pending_set_param_apply = false;
    }

end:
    //fill remaining data with 0 bytes
    MMCAM_ASSERT(response_size >= response.size(), "response string overflow");
    response.append(response_size - response.size(), 0);

    return;
}

void ProcessLayer::EventNotify(uint32_t type, void *data)
{
    switch(type) {
    case EZTUNE_STREAMON_NOTIFY: {
        pthread_t thread_info;
        int val = pthread_create(&thread_info, NULL,
        ProcessLayer::ResumeTuningSessionWrapper, static_cast<void *>(this));
        MMCAM_ASSERT(val == 0, "Process thread creation failed: %s", strerror(errno));
        pthread_setname_np(thread_info, "CAM_eztune_evnt");
        break;
    }
    case EZTUNE_STREAMOFF_NOTIFY:
    case EZTUNE_SET_CHROMATIX_NOTIFY:
    default:
        break;
    }

    if (m_connected) {
        //pipe_fd and 3Alive needed only for Image instance
        //Contol instance will NOT use it
        m_cam_adapter->EventNotify(type, data, m_notify_pipe_fds[1], Get3ALiveStatus());
    } else {
        MMCAM_LOGV("Event skipped as not eztune not connected");
    }
}

};
