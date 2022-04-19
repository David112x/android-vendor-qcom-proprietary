/*
 * Copyright (c) 2010 - 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
/* Not a Contribution.
 * Apache license notifications and license are retained
 * for attribution purposes only.
 */
/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//#define LOG_NDEBUG 0
#define LOG_TAG "MMParserExtractor"
#include "MMParserExtractor.h"
#include <inttypes.h>
#include <cstring>
#include <media/stagefright/MediaBufferBase.h>
#include <media/stagefright/MediaBufferGroup.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/foundation/avc_utils.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/ABitReader.h>
#include <media/stagefright/foundation/AudioPresentationInfo.h>
#include <media/stagefright/foundation/ColorUtils.h>
#include <media/stagefright/foundation/MediaDefs.h>
#include <media/stagefright/ExtendedMediaDefs.h>
#include <QCMetaData.h>
#include <byteswap.h>
#include <utils/Unicode.h>
#include "parserdatadef.h"
#include "MMDebugMsg.h"


//! Number of sync samples required for thumbnail generation
#define NUM_SYNC_FRAMES (5)
//#define DUMP_TO_FILE
/* Max buffer size in byte stream mode */
#define MAX_PARSER_IN_BUF_SIZE (256*1024)
/* Maximum size that can be allocated for MediaBuffer */
#define MAX_MEDIA_BUFFER_SIZE (96 * 1024 * 1024)
/* Initial buffers to allocate by MediaBufferGroup */
#define INIT_MEDIA_BUFFERS (2)
/* Maximum buffers that can be allocated by MediaBufferGroup */
#define MAX_MEDIA_BUFFERS (8)

//! Flag to indicate whether local playback or streaming use case.
//! This value is used to check whether to use infinite or timed wait for the
//! response from FileSource thread.
static bool gbLocalPlayback = false;

namespace android {

#define BITSTREAM_LOG_LOC "/data/misc/media"

typedef enum {
    AOT_AAC_LC           = 2,   /**< Low Complexity object                     */
    AOT_SBR              = 5,
    AOT_ER_AAC_LC        = 17,   /**< Error Resilient(ER) AAC Low Complexity    */
    AOT_ER_AAC_SCAL      = 20,   /**< Error Resilient(ER) AAC Scalable object   */
    AOT_ER_BSAC          = 22,   /**< Error Resilient(ER) BSAC object           */
    AOT_ER_AAC_LD        = 23,   /**< Error Resilient(ER) AAC LowDelay object   */
    AOT_PS               = 29,   /**< PS, Parametric Stereo (includes SBR)      */
    AOT_ESCAPE           = 31,   /**< Signal AOT uses more than 5 bits          */
} AUDIO_OBJECT_TYPE;

FileSourceWrapper* FileSourceWrapper::New(iStreamPort& aStreamPort,
                                          FileSourceFileFormat eFileFormat) {
    FileSourceWrapper* self = new FileSourceWrapper;
    if (self) {
        status_t err = self->Init(aStreamPort, eFileFormat);
        if (err != OK) {
            delete self;
            self = NULL;
            LOGE("FileSourceWrapper::New Init returned %d", err);
        }
    }
    return self;
}

status_t FileSourceWrapper::Init(iStreamPort& aStreamPort,
                                 FileSourceFileFormat eFileFormat) {
    uint32 ulCount = 0;
    FileSourceStatus status = FILE_SOURCE_FAIL;
    //! Create synchronous FileSource
    m_pFileSource = new FileSource(FileSourceWrapper::cbFileSourceStatus,
                                   (void*)this, false);
    if(!m_pFileSource) {
        return ERROR_IO;
    }

    do {
        status = m_pFileSource->OpenFile(&aStreamPort, eFileFormat, TRUE);
        if(FILE_SOURCE_SUCCESS != status) {
            LOGE(" m_pFileSource->OpenFile error %d\n", status);
            return ERROR_IO;
        }
        mWaitSeamaphore.Wait();
        if(FILE_SOURCE_DATA_NOTAVAILABLE == mStatus) {
            LOGE(" m_pFileSource->OpenFile returned data UnderRun\n");
            usleep(100000); //! Sleep for 100ms
            ulCount++;
        }
        if(ulCount > 5) {
            LOGE(" m_pFileSource->OpenFile returned data UnderRun for 5 times\n");
            break;
        }
    } while(FILE_SOURCE_DATA_NOTAVAILABLE == mStatus);

    if(FILE_SOURCE_SUCCESS !=mStatus ) {
        LOGV(" m_pFileSource->OpenFile async error %d\n", mStatus);
        return ERROR_IO;
    }

    FileSourceFileFormat fileformat = FILE_SOURCE_UNKNOWN;
    status = m_pFileSource->GetFileFormat(fileformat);
    if(FILE_SOURCE_SUCCESS != status ) {
        LOGE(" m_pFileSource->GetFileFormat error %d\n", status);
        return ERROR_IO;
    }

    iDrmEncrypted = m_pFileSource->IsDRMProtection();

    if(iDrmEncrypted && fileformat != FILE_SOURCE_AVI) {
       LOGE("Encrypted clip but in unsupported container");
       return UNKNOWN_ERROR;
    }

    return OK;
}

FileSourceWrapper::FileSourceWrapper()
:   m_pFileSource(NULL),
    iDrmEncrypted(false),
    mStatus(FILE_SOURCE_FAIL) {
    LOGV("FileSourceWrapper::FileSourceWrapper");
}

FileSourceWrapper::~FileSourceWrapper() {
    LOGV("FileSourceWrapper::~FileSourceWrapper");
    m_pFileSource->CloseFile();
    delete m_pFileSource;
}

uint32 FileSourceWrapper::GetWholeTracksIDList(FileSourceTrackIdInfoType *trackIdInfo ) {
    Mutex::Autolock autoLock(mMutex);
    return m_pFileSource->GetWholeTracksIDList(trackIdInfo);
}

FileSourceStatus FileSourceWrapper::GetMimeType(uint32 id, FileSourceMjMediaType& majorType, FileSourceMnMediaType& minorType) {
    Mutex::Autolock autoLock(mMutex);
    return m_pFileSource->GetMimeType(id, majorType, minorType);
}

FileSourceStatus FileSourceWrapper::GetMediaTrackInfo(uint32 id,MediaTrackInfo* info) {
    Mutex::Autolock autoLock(mMutex);
    return m_pFileSource->GetMediaTrackInfo(id, info);
}

uint64 FileSourceWrapper::GetTrackMediaDuration(uint32 id) {
    Mutex::Autolock autoLock(mMutex);
    return m_pFileSource->GetTrackMediaDuration(id);
}

uint64 FileSourceWrapper::GetClipDuration() {
    Mutex::Autolock autoLock(mMutex);
    return m_pFileSource->GetClipDuration();
}

FileSourceStatus FileSourceWrapper::GetFormatBlock(uint32 id, uint8* buf,
                                                   uint32 *pbufSize, bool bRawCodec,
                                                   FS_FORMAT_DATA_TYPE type) {
    Mutex::Autolock autoLock(mMutex);
    return m_pFileSource->GetFormatBlock(id, buf, pbufSize, bRawCodec, type);
}

FileSourceStatus FileSourceWrapper::GetFileFormat(FileSourceFileFormat& fileFormat)
{
   return m_pFileSource->GetFileFormat(fileFormat);
}

bool FileSourceWrapper::GetWMACodecData(uint32 id,WmaCodecData* pCodecData) {
    Mutex::Autolock autoLock(mMutex);
    return m_pFileSource->GetWMACodecData(id, pCodecData);
}

uint8 FileSourceWrapper::IsSeekDenied() {
    Mutex::Autolock autoLock(mMutex);
    return m_pFileSource->IsSeekDenied();
}

FileSourceStatus FileSourceWrapper::SeekAbsolutePosition( int64 trackid ,
                                         const int64 tAbsoluteTime,
                                         bool bSeekToSync,
                                         int64 nCurrPlayTime,
                                         FS_SEEK_MODE eSeekMode) {
    Mutex::Autolock autoLock(mMutex);
    FileSourceStatus err = m_pFileSource->SeekAbsolutePosition(trackid, tAbsoluteTime,
                            bSeekToSync, nCurrPlayTime,eSeekMode);
    if(err!=FILE_SOURCE_SUCCESS) {
        return err;
    }
    mWaitSeamaphore.Wait();
    return mStatus;
}

FileSourceStatus FileSourceWrapper::SeekAbsolutePosition(const int64 tAbsoluteTime,
                                         bool bSeekToSync,
                                         int64 nCurrPlayTime,
                                         FS_SEEK_MODE eSeekMode) {
    Mutex::Autolock autoLock(mMutex);
    FileSourceStatus err = m_pFileSource->SeekAbsolutePosition(tAbsoluteTime,
                            bSeekToSync, nCurrPlayTime,eSeekMode);
    if(err!=FILE_SOURCE_SUCCESS) {
        return err;
    }
    mWaitSeamaphore.Wait();
    return mStatus;
}

FileSourceStatus FileSourceWrapper::SeekRelativeSyncPoint( int currentPlaybacktime, const int numSync) {
    Mutex::Autolock autoLock(mMutex);
    FileSourceStatus err = m_pFileSource->SeekRelativeSyncPoint(currentPlaybacktime, numSync);
    if(err!=FILE_SOURCE_SUCCESS) {
        return err;
    }
    mWaitSeamaphore.Wait();
    return mStatus;
}

int32 FileSourceWrapper::GetTrackMaxFrameBufferSize(uint32 id) {
    Mutex::Autolock autoLock(mMutex);
    return m_pFileSource->GetTrackMaxFrameBufferSize(id);
}

FileSourceMediaStatus FileSourceWrapper::GetNextMediaSample(uint32 id,
                    uint8 *buf, uint32 *size, FileSourceSampleInfo& pSampleInfo) {
    Mutex::Autolock autoLock(mMutex);
    return m_pFileSource->GetNextMediaSample(id, buf, size, pSampleInfo);
}

FileSourceStatus FileSourceWrapper::GetClipMetaData(wchar_t* pMetaData,
                    uint32* size, FileSourceMetaDataType pSampleInfo,
                    FS_TEXT_ENCODING_TYPE *pEncode) {
    Mutex::Autolock autoLock(mMutex);
    return m_pFileSource->GetClipMetaData(pMetaData, size, pSampleInfo, pEncode);
}

void FileSourceWrapper::cbFileSourceStatus(FileSourceCallBackStatus status, void* pCbData)
{
    LOGV("FileSourceWrapper::cbFileSourceStatus , pCbData = 0x%p, status = %d ===>\n", pCbData, status);
    FileSourceWrapper* pThis = reinterpret_cast<FileSourceWrapper*>(pCbData);

    //File Source event
    switch (status)
    {
    case FILE_SOURCE_OPEN_COMPLETE:
        LOGV("FileSourceWrapper::cbFileSourceStatus FILE_SOURCE_OPEN_COMPLETE\n");
        pThis->mStatus = FILE_SOURCE_SUCCESS;
        break;
    case FILE_SOURCE_OPEN_FAIL:
        LOGV("FileSourceWrapper::cbFileSourceStatus FILE_SOURCE_OPEN_FAIL\n");
        pThis->mStatus = FILE_SOURCE_FAIL;
        break;
    case FILE_SOURCE_OPEN_DATA_UNDERRUN:
        LOGV("FileSourceWrapper::cbFileSourceStatus FILE_SOURCE_OPEN_DATA_UNDERRUN\n");
        pThis->mStatus = FILE_SOURCE_DATA_NOTAVAILABLE;
        break;
    case FILE_SOURCE_SEEK_COMPLETE:
        LOGV("FileSourceWrapper::cbFileSourceStatus FILE_SOURCE_SEEK_COMPLETE\n");
        pThis->mStatus = FILE_SOURCE_SUCCESS;
        break;
    case FILE_SOURCE_SEEK_FAIL:
        LOGV("FileSourceWrapper::cbFileSourceStatus FILE_SOURCE_SEEK_FAIL\n");
        pThis->mStatus = FILE_SOURCE_FAIL;
        break;
    default:
        LOGV("FileSourceWrapper::cbFileSourceStatus **NOT SUPPORTED CALLBACK**");
        pThis->mStatus = FILE_SOURCE_FAIL;
        break;
    }
    /* Post the event to the filter thread */
    pThis->mWaitSeamaphore.Signal();
}

bool FileSourceWrapper::IsDrmProtection() {
    Mutex::Autolock autoLock(mMutex);
    return iDrmEncrypted;
}

status_t FileSourceWrapper::SetConfiguration(uint32 id,FileSourceConfigItem* pItem, FileSourceConfigItemEnum ienumData)
{
   return m_pFileSource->SetConfiguration(id,pItem,ienumData);
}

status_t FileSourceWrapper::GetConfiguration(uint32 id,FileSourceConfigItem* pItem, FileSourceConfigItemEnum ienumData)
{
   return m_pFileSource->GetConfiguration(id,pItem,ienumData);
}

bool FileSourceWrapper::GetWavCodecData(uint32 id,WavFormatData* pCodecData)
{
   return m_pFileSource->GetWavCodecData(id, pCodecData);
}

FileSourceStatus FileSourceWrapper::GetStreamParameter(uint32 id, uint32 paramIndex, void* ptr)
{
   return m_pFileSource->GetStreamParameter(id, paramIndex, ptr);
}

status_t FileSourceWrapper::SetStreamParameter(uint32 id, uint32 paramIndex, void* ptr)
{
   return m_pFileSource->SetStreamParameter(id, paramIndex, ptr);
}

bool FileSourceWrapper::GetAACCodecData(uint32 id,AacCodecData* pCodecData)
{
   return m_pFileSource->GetAACCodecData(id,pCodecData);
}

////////////////////////////////////////////////////////////////////////////////////

Semaphore::Semaphore()
:   mCount(1) {}

void Semaphore::Signal() {
    mMutex.lock();
    mCount++;
    if(mCount<=1) { //Implies someone was waiting.
        mCondition.signal();
    }
    mMutex.unlock();
}

void Semaphore::Wait() {
//! Maximum time that Parser can wait for response from FileSource Thread
//! This value is kept as 15sec. This needs to be provided in nanosec units.
//! This timed wait is used only in case of local playback mode.
//! In streaming playback, infinite loop will be used to handle low bandwidth
//! use cases.
    mMutex.lock();
    mCount--;
    if(mCount<=0) {
        if(true == gbLocalPlayback) {
            const nsecs_t sllWaitTime = 15000000000LL;
            mCondition.waitRelative(mMutex, sllWaitTime);
        }
        else
            mCondition.wait(mMutex);
    }
    mMutex.unlock();
}

////////////////////////////////////////////////////////////////////////////////////
const char *MediaType2MIME(FileSourceMnMediaType minorType) {
    const char* mimeType = NULL;
    switch (minorType) {
        case FILE_SOURCE_MN_TYPE_MP3:
            LOGV("MediaType2MIME FILE_SOURCE_MN_TYPE_MP3");
            mimeType = MEDIA_MIMETYPE_AUDIO_MPEG;
            break;
        case FILE_SOURCE_MN_TYPE_AC3:
            LOGV("MediaType2MIME FILE_SOURCE_MN_TYPE_AC3");
            mimeType = MEDIA_MIMETYPE_AUDIO_AC3;
            break;
        case FILE_SOURCE_MN_TYPE_EAC3:
            LOGV("MediaType2MIME FILE_SOURCE_MN_TYPE_EAC3");
            mimeType = MEDIA_MIMETYPE_AUDIO_EAC3;
            break;
        case FILE_SOURCE_MN_TYPE_EAC3_JOC:
            LOGV("MediaType2MIME FILE_SOURCE_MN_TYPE_EAC3_JOC");
            mimeType = "audio/eac3-joc";
            break;
        case FILE_SOURCE_MN_TYPE_AAC:
        case FILE_SOURCE_MN_TYPE_AAC_ADTS:
            LOGV("MediaType2MIME FILE_SOURCE_MN_TYPE_AAC minor type %d", minorType);
            mimeType = MEDIA_MIMETYPE_AUDIO_AAC;
            break;
        case FILE_SOURCE_MN_TYPE_QCELP:
        case FILE_SOURCE_MN_TYPE_QCP:
            LOGV("MediaType2MIME FILE_SOURCE_MN_TYPE_QCELP");
            mimeType = MEDIA_MIMETYPE_AUDIO_QCELP;
            break;
        case FILE_SOURCE_MN_TYPE_EVRC:
            LOGV("MediaType2MIME FILE_SOURCE_MN_TYPE_EVRC");
            mimeType = MEDIA_MIMETYPE_AUDIO_EVRC;
            break;
        case FILE_SOURCE_MN_TYPE_WMA:
            LOGV("MediaType2MIME FILE_SOURCE_MN_TYPE_WMA");
            mimeType = MEDIA_MIMETYPE_AUDIO_WMA;
            break;
        case FILE_SOURCE_MN_TYPE_WMA_PRO:
            LOGV("MediaType2MIME FILE_SOURCE_MN_TYPE_WMA_PRO");
            mimeType = MEDIA_MIMETYPE_AUDIO_WMA;
            break;
        case FILE_SOURCE_MN_TYPE_WMA_LOSSLESS:
            LOGV("MediaType2MIME FILE_SOURCE_MN_TYPE_WMA_LOSSLESS");
            mimeType = MEDIA_MIMETYPE_AUDIO_WMA;
            break;
        case FILE_SOURCE_MN_TYPE_GSM_AMR:
            LOGV("MediaType2MIME FILE_SOURCE_MN_TYPE_GSM_AMR");
            mimeType = MEDIA_MIMETYPE_AUDIO_AMR_NB;
            break;
        case FILE_SOURCE_MN_TYPE_AMR_WB:
            LOGV("MediaType2MIME FILE_SOURCE_MN_TYPE_AMR_WB");
            mimeType = MEDIA_MIMETYPE_AUDIO_AMR_WB;
            break;
        case FILE_SOURCE_MN_TYPE_AMR_WB_PLUS:
            LOGV("MediaType2MIME MEDIA_MIMETYPE_AUDIO_AMR_WB_PLUS");
            mimeType = MEDIA_MIMETYPE_AUDIO_AMR_WB_PLUS;
            break;
        case FILE_SOURCE_MN_TYPE_DTS:
            LOGV("MediaType2MIME FILE_SOURCE_MN_TYPE_DTS");
            mimeType = MEDIA_MIMETYPE_AUDIO_DTS;
            break;
        case FILE_SOURCE_MN_TYPE_DSD:
            LOGV("MediaType2MIME FILE_SOURCE_MN_TYPE_DSD");
            mimeType = MEDIA_MIMETYPE_AUDIO_DSD;
            break;
        case FILE_SOURCE_MN_TYPE_AIFF_PCM:
        case FILE_SOURCE_MN_TYPE_PCM:
            LOGV("MediaType2MIME FILE_SOURCE_MN_TYPE_PCM");
            mimeType = MEDIA_MIMETYPE_AUDIO_RAW;
            break;
        case FILE_SOURCE_MN_TYPE_G711_ALAW:
            LOGV("MediaType2MIME FILE_SOURCE_MN_TYPE_G711_ALAW");
            mimeType = MEDIA_MIMETYPE_AUDIO_G711_ALAW;
            break;
        case FILE_SOURCE_MN_TYPE_G711_MULAW:
            LOGV("MediaType2MIME FILE_SOURCE_MN_TYPE_G711_MULAW");
            mimeType = MEDIA_MIMETYPE_AUDIO_G711_MLAW;
            break;
        case FILE_SOURCE_MN_TYPE_GSM_FR:
            LOGV("MediaType2MIME FILE_SOURCE_MN_TYPE_GSM_FR");
            mimeType = MEDIA_MIMETYPE_AUDIO_MSGSM;
            break;
        case FILE_SOURCE_MN_TYPE_ALAC:
            LOGV("MediaType2MIME FILE_SOURCE_MN_TYPE_ALAC");
            mimeType = MEDIA_MIMETYPE_AUDIO_ALAC;
            break;
        case FILE_SOURCE_MN_TYPE_APE:
            LOGV("MediaType2MIME FILE_SOURCE_MN_TYPE_APE");
            mimeType = MEDIA_MIMETYPE_AUDIO_APE;
            break;
        case FILE_SOURCE_MN_TYPE_SOWT:
            LOGV("MediaType2MIME FILE_SOURCE_MN_TYPE_SOWT");
            mimeType = MEDIA_MIMETYPE_AUDIO_RAW;
            break;
        case FILE_SOURCE_MN_TYPE_FLAC:
            LOGV("MediaType2MIME FILE_SOURCE_MN_TYPE_FLAC");
            mimeType = MEDIA_MIMETYPE_AUDIO_FLAC;
            break;
        case FILE_SOURCE_MN_TYPE_OPUS:
            LOGV("MediaType2MIME FILE_SOURCE_MN_TYPE_OPUS");
            mimeType = MEDIA_MIMETYPE_AUDIO_OPUS;
            break;
        case FILE_SOURCE_MN_TYPE_VORBIS:
            LOGV("MediaType2MIME FILE_SOURCE_MN_TYPE_VORBIS");
            mimeType = MEDIA_MIMETYPE_AUDIO_VORBIS;
            break;
        case FILE_SOURCE_MN_TYPE_MHAS:
            LOGV("MediaType2MIME FILE_SOURCE_MN_TYPE_MHAS");
            mimeType = MEDIA_MIMETYPE_AUDIO_MHAS;
            break;
        case FILE_SOURCE_MN_TYPE_AC4:
            LOGV("MediaType2MIME FILE_SOURCE_MN_TYPE_AC4");
            mimeType = MEDIA_MIMETYPE_AUDIO_AC4;
            break;
        case FILE_SOURCE_MN_TYPE_WMV3:
        case FILE_SOURCE_MN_TYPE_VC1:
            LOGV("MediaType2MIME FILE_SOURCE_MN_TYPE_WMV*");
            mimeType = MEDIA_MIMETYPE_VIDEO_WMV;
            break;
        case FILE_SOURCE_MN_TYPE_MPEG4:
        case FILE_SOURCE_MN_TYPE_STILL_IMAGE:
            LOGV("MediaType2MIME FILE_SOURCE_MN_TYPE_MPEG4");
            mimeType = MEDIA_MIMETYPE_VIDEO_MPEG4;
            break;
        case FILE_SOURCE_MN_TYPE_H263:
            LOGV("MediaType2MIME FILE_SOURCE_MN_TYPE_H263");
            mimeType = MEDIA_MIMETYPE_VIDEO_H263;
            break;
        case FILE_SOURCE_MN_TYPE_H264:
            LOGV("MediaType2MIME FILE_SOURCE_MN_TYPE_H264");
            mimeType = MEDIA_MIMETYPE_VIDEO_AVC;
            break;
       case FILE_SOURCE_MN_TYPE_VP8F:
            LOGV("MediaType2MIME FILE_SOURCE_MN_TYPE_VP8F");
            mimeType = MEDIA_MIMETYPE_VIDEO_VPX;
            break;
       case FILE_SOURCE_MN_TYPE_VP9:
            LOGV("MediaType2MIME FILE_SOURCE_MN_TYPE_VP9");
            mimeType = MEDIA_MIMETYPE_VIDEO_VP9;
            break;
        case FILE_SOURCE_MN_TYPE_HEVC:
            LOGV("MediaType2MIME FILE_SOURCE_MN_TYPE_HEVC");
            mimeType = MEDIA_MIMETYPE_VIDEO_HEVC;
            break;
        case FILE_SOURCE_MN_TYPE_MPEG2:
            LOGV("MediaType2MIME FILE_SOURCE_MN_TYPE_MPEG2");
            mimeType = MEDIA_MIMETYPE_VIDEO_MPEG2;
            break;
        case FILE_SOURCE_MN_TYPE_MPEG1:
            LOGV("MediaType2MIME FILE_SOURCE_MN_TYPE_MPEG1");
            // returning mpeg2 as frameworks does not has support
            // for mpeg1, this enables playback of mpeg1 videos
            mimeType = MEDIA_MIMETYPE_VIDEO_MPEG2;
            break;
        case FILE_SOURCE_MN_TYPE_TIMED_TEXT:
            LOGV("MediaType2MIME FILE_SOURCE_MN_TYPE_TIMED_TEXT");
            mimeType = MEDIA_MIMETYPE_TEXT_3GPP;
            break;
        case FILE_SOURCE_MN_TYPE_META_TEXT:
            LOGV("MediaType2MIME FILE_SOURCE_MN_TYPE_META_TEXT");
            mimeType = "text/plain";
            break;
        case FILE_SOURCE_MN_TYPE_HEIF:
            LOGV("MediaType2MIME FILE_SOURCE_MN_TYPE_HEIF");
            mimeType = MEDIA_MIMETYPE_IMAGE_ANDROID_HEIC;
            break;
        case FILE_SOURCE_MN_TYPE_AV1:
            LOGV("MediaType2MIME FILE_SOURCE_MN_TYPE_AV1");
            mimeType = MEDIA_MIMETYPE_VIDEO_AV1;
            break;
        case FILE_SOURCE_MN_TYPE_MP2:
            LOGV("MediaType2MIME FILE_SOURCE_MN_TYPE_MP2");
            mimeType = MEDIA_MIMETYPE_AUDIO_MPEG_LAYER_II;
            break;
        case FILE_SOURCE_MN_TYPE_AAC_ADIF:
        case FILE_SOURCE_MN_TYPE_AAC_LOAS:
        case FILE_SOURCE_MN_TYPE_WMV1:
        case FILE_SOURCE_MN_TYPE_WMV2:
        default:
            LOGE("MediaType2MIME minor type %d not supported, return NULL", minorType);
            mimeType = NULL;
            break;
    }
    return mimeType;
}

const char *GetFileMimeType(FileSourceFileFormat fileFormat, uint8 tracksPresent) {
    const char* mimeType = NULL;
    switch(fileFormat) {
        case FILE_SOURCE_AC3:
            mimeType = MEDIA_MIMETYPE_AUDIO_AC3;
            break;
        case FILE_SOURCE_AAC:
            mimeType = "audio/aac";
            break;
        case FILE_SOURCE_AMR_NB:
            mimeType = "audio/amr";
            break;
        case FILE_SOURCE_AMR_WB:
            mimeType = MEDIA_MIMETYPE_AUDIO_AMR_WB;
            break;
        case FILE_SOURCE_AIFF:
            mimeType = MEDIA_MIMETYPE_AUDIO_AIFF;
            break;
        case FILE_SOURCE_APE:
            mimeType = MEDIA_MIMETYPE_AUDIO_APE;
            break;
        case FILE_SOURCE_DSF:
        case FILE_SOURCE_DSDIFF:
            mimeType = MEDIA_MIMETYPE_AUDIO_DSD;
            break;
        case FILE_SOURCE_WAV:
            mimeType = MEDIA_MIMETYPE_CONTAINER_WAV;
            break;
        case FILE_SOURCE_FLV:
            mimeType = "video/flv";
            break;
        case FILE_SOURCE_ASF:
            if(tracksPresent & VIDEO_TRACK) {
                mimeType = MEDIA_MIMETYPE_VIDEO_WMV;
            } else {
                mimeType = MEDIA_MIMETYPE_AUDIO_WMA;
            }
            break;
        case FILE_SOURCE_FLAC:
            mimeType = MEDIA_MIMETYPE_AUDIO_FLAC;
            break;
        case FILE_SOURCE_OGG:
            mimeType = MEDIA_MIMETYPE_CONTAINER_OGG;
            break;
        case FILE_SOURCE_AVI:
            mimeType = MEDIA_MIMETYPE_CONTAINER_AVI;
            break;
        case FILE_SOURCE_EVRCB:
        case FILE_SOURCE_EVRC_WB:
            mimeType = "audio/evrc";
            break;
        case FILE_SOURCE_QCP:
            mimeType = MEDIA_MIMETYPE_AUDIO_QCELP;
            break;
        case FILE_SOURCE_WEBM:
            mimeType = "video/webm";
            break;
        case FILE_SOURCE_MKV:
            mimeType = MEDIA_MIMETYPE_CONTAINER_MATROSKA;
            break;
        case FILE_SOURCE_3G2:
        case FILE_SOURCE_MPEG4:
            if(tracksPresent & VIDEO_TRACK) {
                mimeType = MEDIA_MIMETYPE_CONTAINER_MPEG4;
            } else if(tracksPresent & AUDIO_TRACK) {
                mimeType = "audio/mp4";
            } else if(tracksPresent & IMAGE_TRACK) {
                mimeType = MEDIA_MIMETYPE_CONTAINER_HEIF;
            }
            break;
        case FILE_SOURCE_MP3:
            mimeType = MEDIA_MIMETYPE_AUDIO_MPEG;
            break;
        case FILE_SOURCE_MHAS:
            mimeType = MEDIA_MIMETYPE_AUDIO_MHAS;
            break;
        case FILE_SOURCE_DTS:
            mimeType = "audio/dts";
            break;
        case FILE_SOURCE_MP2TS:
            mimeType = MEDIA_MIMETYPE_CONTAINER_MPEG2TS;
            break;
        case FILE_SOURCE_MP2PS:
            mimeType = MEDIA_MIMETYPE_CONTAINER_MPEG2PS;
            break;
        default:
            break;
    }
    return mimeType;
}

MMParserExtractor::MMParserExtractor(DataSourceHelper *source, FileSourceFileFormat format)
    : mDataSource(source),
      mHaveMetadata(NO_INIT),
      mTracks(0),
      mIsAacformatAdif(false),
      mFirstTrack(NULL),
      mLastTrack(NULL),
      m_pFileSourceWrapper(NULL),
      m_pSourcePort(NULL),
      m_eFileFormat(format) {
    LOGI("Created(%p)",this);
    m_pSourcePort = new SourcePort(source);
    mFileMetaData = AMediaFormat_new();
    video::iStreamPort::DataSourceType eSourceType;
    m_pSourcePort->GetSourceType(&eSourceType);
    m_bIsStreaming = (video::iStreamPort::DS_STREAMING_SOURCE == eSourceType);
    if(video::iStreamPort::DS_STREAMING_SOURCE != eSourceType)
        gbLocalPlayback = true;
    else
        gbLocalPlayback = false;

    char property_value[PROPERTY_VALUE_MAX] = {0};
    memset(&m_sDebug, 0, sizeof(DebugInfo));
    property_get("vendor.qcom_parser.debug.meta", property_value, "0");
    m_sDebug.bDumpMetaToLog = atoi(property_value);
    property_value[0] = '\0';

    property_get("vendor.qcom_parser.debug.out", property_value, "0");
    m_sDebug.bDumpToFile = atoi(property_value);
    property_value[0] = '\0';
    if (m_sDebug.bDumpToFile) {
        snprintf(m_sDebug.pucFileName, PROPERTY_VALUE_MAX, "%s", BITSTREAM_LOG_LOC);
        property_get("vendor.qcom_parser.debug.loc", property_value, "");
        if (*property_value)
            strlcpy(m_sDebug.pucFileName, property_value, PROPERTY_VALUE_MAX);
        snprintf(m_sDebug.pucFileName, PROPERTY_VALUE_MAX + 36, "%s/bitstream_%p",
                 m_sDebug.pucFileName, this);
        property_value[0] = '\0';
    }
}

MMParserExtractor::~MMParserExtractor() {
    LOGV("MMParserExtractor::~MMParserExtractor ====> \n");
    Track *track = mFirstTrack;
    while (track) {
        Track *next = track->next;
        AMediaFormat_delete(track->meta);
        delete track;
        track = next;
    }
    mFirstTrack = mLastTrack = NULL;

    if (m_pFileSourceWrapper) {
        delete m_pFileSourceWrapper;
    }
    if (m_pSourcePort) {
        delete m_pSourcePort;
    }
    if (mDataSource) {
        delete mDataSource;
    }
    AMediaFormat_delete(mFileMetaData);

    m_pFileSourceWrapper = NULL;
    m_pSourcePort = NULL;
    mDataSource = NULL;
}

size_t MMParserExtractor::countTracks() {
    status_t err;
    if ((err = readMetaData()) != OK) {
        LOGE("MMParserExtractor::countTracks no tracks");
        return 0;
    }

    LOGV("MMParserExtractor::countTracks %u tracks", mTracks);
    return mTracks;
}

media_status_t MMParserExtractor::getTrackMetaData(AMediaFormat *meta,
                                              size_t index, uint32_t flags) {
    status_t err;
    LOGV("MMParserExtractor::getTrackMetaData track %zu", index);
    if ((err = readMetaData()) != OK) {
        return AMEDIA_ERROR_UNKNOWN;
    }

    Track *track = mFirstTrack;
    while (index > 0) {
        if (track == NULL)
            break;
        track = track->next;
        --index;
    }

    if (track == NULL)
        return AMEDIA_ERROR_UNKNOWN;

    if (flags & kIncludeExtensiveMetaData && !m_bIsStreaming &&
        !track->isExpensiveMetaPresent) {
        findThumbnail(track);
        track->isExpensiveMetaPresent = true;
    }
    AMediaFormat_copy(meta, track->meta);
    return AMEDIA_OK;
}

media_status_t MMParserExtractor::getMetaData(AMediaFormat *meta) {
    status_t err;
    LOGV("MMParserExtractor::getMetaData");
    if ((err = readMetaData()) != OK)  {
       return AMEDIA_ERROR_UNKNOWN;
    }
    AMediaFormat_copy(meta, mFileMetaData);
    return AMEDIA_OK;
}

void MMParserExtractor::setAlbumMetaElement(FileSourceMetaDataType element, const char *key) {
    uint32 size = 0;

    m_pFileSourceWrapper->GetClipMetaData(NULL, &size, element);
    if(size == 0)
        return;

    wchar_t* buffer = new wchar_t[size];
    if (!buffer)
        return;

    FS_TEXT_ENCODING_TYPE eEncode = FS_ENCODING_TYPE_UNKNOWN;
    FileSourceStatus status = m_pFileSourceWrapper->GetClipMetaData(buffer, &size,
                                                                    element, &eEncode);
    if (status == FILE_SOURCE_SUCCESS && size > 0) {
        if (FS_TEXT_ENC_UTF8 == eEncode || FS_TEXT_ENC_ISO8859_1 == eEncode) {
            //! If metadata is in UTF8 or ISO-8859 string format, set it directly
            AMediaFormat_setString(mFileMetaData, key, (const char*)buffer);
        } else if (FS_TEXT_ENC_UTF16 == eEncode || FS_TEXT_ENC_UTF16_BE == eEncode) {
            //! If metadata is in UTF16 string format with BOM,
            //! then check the string is in BE or LE format first.
            const char16_t *pMetaData = (const char16_t *) (buffer);
            uint32 ulMetaSize = size / 2;
            char16_t *pucTemp = NULL;
            char16_t *pFrameData = (char16_t *) buffer;
            // endianness marker doesn't match host endianness, convert
            if (0xfffe == *pMetaData || FS_TEXT_ENC_UTF16_BE == eEncode) {
                pucTemp = new char16_t[ulMetaSize];
                if (pucTemp != NULL) {
                    for (int i = 0; i < ulMetaSize; i++) {
                        pucTemp[i] = bswap_16(pMetaData[i]);
                    }
                    pFrameData = pucTemp;
                }
            }

            // If the string starts with an endianness marker, skip it
            if (0xfeff == *pFrameData) {
                pFrameData++;
                ulMetaSize--;
            }

            bool bEightBitData = false;
            if (FS_TEXT_ENC_UTF16 == eEncode) {
                // check if the resulting data consists entirely of 8-bit values
                bEightBitData = true;
                for (int i = 0; i < ulMetaSize; i++) {
                    if (pFrameData[i] > 0xff) {
                        bEightBitData = false;
                        break;
                    }
                }
                if (bEightBitData) {
                    // convert to 8 bit format, let the client figure out the real encoding
                    char *pFrameDatain8Bit = new char[ulMetaSize + 1];
                    if (pFrameDatain8Bit != NULL) {
                        for (int i = 0; i <= ulMetaSize; i++) {
                            pFrameDatain8Bit[i] = pFrameData[i];
                        }
                        AMediaFormat_setString(mFileMetaData, key, pFrameDatain8Bit);
                        delete [] pFrameDatain8Bit;
                    }
                }
            }

            if (!bEightBitData) {
                String8 temp;
                temp.setTo((const char16_t*)pFrameData, ulMetaSize);
                AMediaFormat_setString(mFileMetaData, key, temp);
            }

            if(pucTemp)
                delete[] pucTemp;
        } else if(FS_TEXT_ENC_UTF32 == eEncode) {
            int len = size / sizeof(wchar_t);
            const ssize_t bytes = utf32_to_utf8_length((char32_t *)buffer, len);
            if (len > 0) {
                char *tempBuffer = new char[bytes + 1];
                if(buffer != NULL) {
                    utf32_to_utf8((char32_t *)buffer, len, tempBuffer, bytes + 1);
                    AMediaFormat_setString(mFileMetaData, key, tempBuffer);
                    delete[] tempBuffer;
                }
            }
        }
    }
    delete[] buffer;
}

void MMParserExtractor::setAlbumArtRelatedMeta() {

    //------- Set ALBUM -------------
    setAlbumMetaElement(FILE_SOURCE_MD_ALBUM, AMEDIAFORMAT_KEY_ALBUM);

    //------- Set ARTIST -----------
    setAlbumMetaElement(FILE_SOURCE_MD_ARTIST, AMEDIAFORMAT_KEY_ARTIST);

    //------- Set Title -----------
    setAlbumMetaElement(FILE_SOURCE_MD_TITLE, AMEDIAFORMAT_KEY_TITLE);

    //------- Set COMPOSER -----------
    setAlbumMetaElement(FILE_SOURCE_MD_COMPOSER, AMEDIAFORMAT_KEY_COMPOSER);

    //------- Set TRACK -----------
    setAlbumMetaElement(FILE_SOURCE_MD_TRACK_NUM, AMEDIAFORMAT_KEY_CDTRACKNUMBER);

    //------- Set YEAR -----------
    setAlbumMetaElement(FILE_SOURCE_MD_REC_YEAR, AMEDIAFORMAT_KEY_YEAR);

    //------- Set GENRE -----------
    setAlbumMetaElement(FILE_SOURCE_MD_GENRE, AMEDIAFORMAT_KEY_GENRE);

    //------- Set AUTHOR -----------
    setAlbumMetaElement(FILE_SOURCE_MD_AUTHOR, AMEDIAFORMAT_KEY_AUTHOR);

    //------- Set ALBUMARTIST -----------
    setAlbumMetaElement(FILE_SOURCE_MD_ALBUM_ARTIST, AMEDIAFORMAT_KEY_ALBUMARTIST);

    //------- Set DISCNUMBER -----------
    setAlbumMetaElement(FILE_SOURCE_MD_DISK_NUM, AMEDIAFORMAT_KEY_DISCNUMBER);

    //------- Set COMPILATION -----------
    setAlbumMetaElement(FILE_SOURCE_MD_COMPILATION, AMEDIAFORMAT_KEY_COMPILATION);

    //------- Set WRITER -----------
    setAlbumMetaElement(FILE_SOURCE_MD_WRITER, AMEDIAFORMAT_KEY_LYRICIST);

    //------- Set CREATION DATE -----------
    setAlbumMetaElement(FILE_SOURCE_MD_CREATION_DATE, AMEDIAFORMAT_KEY_DATE);

    //------- Set Location Info -----------
    setAlbumMetaElement(FILE_SOURCE_MD_GEOTAG, AMEDIAFORMAT_KEY_LOCATION);

    //------- Set Manufacturer Info -----------
    setAlbumMetaElement(FILE_SOURCE_MD_MANUFACTURER, "media-manufacturer");
}

void MMParserExtractor::setAlbumArt() {
    //------- Set Album Art ----------------------
    uint32 size = 0;
    wchar_t* buffer = NULL;

    m_pFileSourceWrapper->GetClipMetaData(NULL, &size, FILE_SOURCE_MD_ALBUM_ART);
    if(size <= sizeof(FS_ALBUM_ART_METADATA))
      return;

    LOGV("setAlbumArt size = %u", size);

    buffer = new wchar_t[size];
    if(buffer != NULL) {
        FileSourceStatus status = m_pFileSourceWrapper->GetClipMetaData(buffer, &size,
                                                                        FILE_SOURCE_MD_ALBUM_ART);
        if(status != FILE_SOURCE_SUCCESS) {
            LOGE("setAlbumArt failed to get album art data");
            delete[] buffer;
            return;
        }

        FS_ALBUM_ART_METADATA *meta = (FS_ALBUM_ART_METADATA*)buffer;
        AMediaFormat_setBuffer(mFileMetaData, AMEDIAFORMAT_KEY_ALBUMART,
                               meta->pucPicData, meta->ulPicDataLen);
        delete[] buffer;
    }
}

bool FileSourceWrapper::IsCodecSupportAvailable(
          FileSourceFileFormat  eFileFormat,
          FileSourceMnMediaType eCodecType,
          MediaTrackInfo mediaTrackInfo) {

    int  nFlags = 0;
    bool bCodecAllowed = true;
    char ucParserPropertyValue[PROPERTY_VALUE_MAX] = {0};
    FileSourceAudioInfo sAudioInfo = mediaTrackInfo.audioTrackInfo;

    property_get("vendor.mm.enable.qcom_parser", ucParserPropertyValue, "0");
    nFlags = atoi(ucParserPropertyValue);
    switch (eFileFormat) {
        case FILE_SOURCE_APE:
            if (FILE_SOURCE_MN_TYPE_APE == eCodecType &&
                8 == sAudioInfo.nBitsPerSample) {
                bCodecAllowed = false;
                LOGE("MM-Parser: AAL: APE- %d bit support is disabled",
                     sAudioInfo.nBitsPerSample);
            }
            break;
        case FILE_SOURCE_FLAC:
            if (FILE_SOURCE_MN_TYPE_FLAC == eCodecType &&
                8 == sAudioInfo.nBitsPerSample) {
                bCodecAllowed = false;
                LOGE("MM-Parser: AAL: FLAC- %d bit support is disabled",
                     sAudioInfo.nBitsPerSample);
            }
            break;

        default:
            break;
    }

    if (!bCodecAllowed)
        LOGE("Codec support disabled for minor type %d", eCodecType);

    return bCodecAllowed;
}

status_t MMParserExtractor::updateAudioTrackInfoFromCSD(Track* track, const char* key) {
    const uint8_t* csd;
    size_t csd_size;
    if (!AMediaFormat_getBuffer(track->meta, key, (void **)&csd, &csd_size)) {
        return ERROR_MALFORMED;
    }

    LOGV("%s Enter:", __func__);
    int32_t sampleRate = 0;
    int32_t numChannels = 0;
    uint32_t objectType = 0;
    if (csd_size == 0) {
       return OK;
    }

    if (csd_size < 2) {
        LOGE("Codec specific data was malformed");
        return ERROR_MALFORMED;
    }

    static uint32_t kSamplingRate[] = {
        96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050,
        16000, 12000, 11025, 8000, 7350
    };

    ABitReader br(csd, csd_size);
    objectType = br.getBits(5);

    if (objectType == 31) {  //AAC-ELD => additional 6 bits
        objectType = 32 + br.getBits(6);
    }

    if (mLastTrack == NULL) {
        return ERROR_MALFORMED;
    }

    AMediaFormat_setInt32(mLastTrack->meta, AMEDIAFORMAT_KEY_AAC_PROFILE, objectType);
    uint32_t freqIndex = br.getBits(4);
    if (freqIndex == 15) {
        if (br.numBitsLeft() < 28) {
            LOGE("Codec specific data was malformed");
            return ERROR_MALFORMED;
        }
        sampleRate = br.getBits(24);
        numChannels = br.getBits(4);
    } else {
        if (br.numBitsLeft() < 4) {
            LOGE("Codec specific data was malformed");
            return ERROR_MALFORMED;
        }
        numChannels = br.getBits(4);
        if (freqIndex == 13 || freqIndex == 14) {
            LOGE("Codec specific data was malformed");
            return ERROR_MALFORMED;
        }
        sampleRate = kSamplingRate[freqIndex];
    }
    if (objectType == AOT_SBR || objectType == AOT_PS) {//SBR specific config per 14496-3 table 1.13
        if (br.numBitsLeft() < 4) return ERROR_MALFORMED;
        uint32_t extFreqIndex = br.getBits(4);
        int32_t extSampleRate __unused;
        if (extFreqIndex == 15) {
            if (csd_size < 8) {
                return ERROR_MALFORMED;
            }
            if (br.numBitsLeft() < 24) return ERROR_MALFORMED;
            extSampleRate = br.getBits(24);
        } else {
            if (extFreqIndex == 13 || extFreqIndex == 14) {
                return ERROR_MALFORMED;
            }
            extSampleRate = kSamplingRate[extFreqIndex];
        }
    }
    switch (numChannels) {
         case 0:
         case 1: //FC
         case 2: //FL FR
         case 3: //FC, FL, FR
         case 4: //FC, FL, FR, RC
         case 5: //FC, FL, FR, SL SR
         case 6: //FC, FL, FR, SL, SR, LFE
            break;
         case 11: //FC, FL, FR, SL, SR, RC, LFE
            numChannels = 7;
            break;
         case 7:
         case 12:
         case 14:
            numChannels = 8;
            break;
         default:
            return ERROR_UNSUPPORTED;
    }//switch(numChannels)
    if (objectType == AOT_SBR || objectType == AOT_PS) {
        if (br.numBitsLeft() < 5) return ERROR_MALFORMED;
        objectType = br.getBits(5);

        if (objectType == AOT_ESCAPE) {
            if (br.numBitsLeft() < 6) return ERROR_MALFORMED;
            objectType = 32 + br.getBits(6);
        }
    }
    if (objectType == AOT_AAC_LC || objectType == AOT_ER_AAC_LC ||
            objectType == AOT_ER_AAC_LD || objectType == AOT_ER_AAC_SCAL ||
            objectType == AOT_ER_BSAC) {
        if (br.numBitsLeft() < 2) return ERROR_MALFORMED;

        const int32_t frameLengthFlag __unused = br.getBits(1);
        const int32_t dependsOnCoreCoder = br.getBits(1);

        if (dependsOnCoreCoder ) {
            if (br.numBitsLeft() < 14) return ERROR_MALFORMED;
            const int32_t coreCoderDelay __unused = br.getBits(14);
        }

        int32_t extensionFlag = -1;
        if (br.numBitsLeft() > 0) {
            extensionFlag = br.getBits(1);
        }else {
           switch (objectType) {
           case AOT_AAC_LC:
              extensionFlag =0;
              break;
           case AOT_ER_AAC_LC:
           case AOT_ER_AAC_SCAL:
           case AOT_ER_BSAC:
           case AOT_ER_AAC_LD:
              extensionFlag = 1;
              break;
           default:
              return ERROR_MALFORMED;
              break;
           }
        }
        if (numChannels == 0 && (br.numBitsLeft() > 0)) {
            int32_t channelsEffectiveNum = 0;
            int32_t channelsNum = 0;
            if (br.numBitsLeft() < 32) {
                return ERROR_MALFORMED;
            }
            const int32_t ElementInstanceTag __unused = br.getBits(4);
            const int32_t Profile __unused = br.getBits(2);
            const int32_t SamplingFrequencyIndex __unused = br.getBits(4);
            const int32_t NumFrontChannelElements = br.getBits(4);
            const int32_t NumSideChannelElements = br.getBits(4);
            const int32_t NumBackChannelElements = br.getBits(4);
            const int32_t NumLfeChannelElements = br.getBits(2);
            const int32_t NumAssocDataElements __unused = br.getBits(3);
            const int32_t NumValidCcElements __unused = br.getBits(4);

            const int32_t MonoMixdownPresent = br.getBits(1);

            if (MonoMixdownPresent != 0) {
                if (br.numBitsLeft() < 4) return ERROR_MALFORMED;
                const int32_t MonoMixdownElementNumber __unused = br.getBits(4);
            }

            if (br.numBitsLeft() < 1) return ERROR_MALFORMED;
            const int32_t StereoMixdownPresent = br.getBits(1);
            if (StereoMixdownPresent != 0) {
                if (br.numBitsLeft() < 4) return ERROR_MALFORMED;
                const int32_t StereoMixdownElementNumber __unused = br.getBits(4);
            }

            if (br.numBitsLeft() < 1) return ERROR_MALFORMED;
            const int32_t MatrixMixdownIndexPresent = br.getBits(1);
            if (MatrixMixdownIndexPresent != 0) {
                if (br.numBitsLeft() < 3) return ERROR_MALFORMED;
                const int32_t MatrixMixdownIndex __unused = br.getBits(2);
                const int32_t PseudoSurroundEnable __unused = br.getBits(1);
            }

            int i;
            for (i=0; i < NumFrontChannelElements; i++) {
                if (br.numBitsLeft() < 5) return ERROR_MALFORMED;
                const int32_t FrontElementIsCpe = br.getBits(1);
                const int32_t FrontElementTagSelect __unused = br.getBits(4);
                channelsNum += FrontElementIsCpe ? 2 : 1;
            }

            for (i=0; i < NumSideChannelElements; i++) {
                if (br.numBitsLeft() < 5) return ERROR_MALFORMED;
                const int32_t SideElementIsCpe = br.getBits(1);
                const int32_t SideElementTagSelect __unused = br.getBits(4);
                channelsNum += SideElementIsCpe ? 2 : 1;
            }

            for (i=0; i < NumBackChannelElements; i++) {
                if (br.numBitsLeft() < 5) return ERROR_MALFORMED;
                const int32_t BackElementIsCpe = br.getBits(1);
                const int32_t BackElementTagSelect __unused = br.getBits(4);
                channelsNum += BackElementIsCpe ? 2 : 1;
            }
            channelsEffectiveNum = channelsNum;

            for (i=0; i < NumLfeChannelElements; i++) {
                if (br.numBitsLeft() < 4) return ERROR_MALFORMED;
                const int32_t LfeElementTagSelect __unused = br.getBits(4);
                channelsNum += 1;
            }
            LOGV("mpeg4 audio channelsNum = %d", channelsNum);
            LOGV("mpeg4 audio channelsEffectiveNum = %d", channelsEffectiveNum);
            numChannels = channelsNum;
        }
    }
    if (numChannels ==0 ) {
       return ERROR_UNSUPPORTED;
    }
    if (mLastTrack == NULL) {
       return ERROR_MALFORMED;
    }
    int32_t prevSampleRate;
    AMediaFormat_getInt32(mLastTrack->meta, AMEDIAFORMAT_KEY_SAMPLE_RATE, &prevSampleRate);
    if (prevSampleRate != sampleRate) {
       LOGE("mpeg4 audio sample rate different from previous setting"
            "was: %d, now: %d",prevSampleRate,sampleRate);
    }
    AMediaFormat_setInt32(mLastTrack->meta, AMEDIAFORMAT_KEY_SAMPLE_RATE, sampleRate);
    LOGV("sampleRate: %d",sampleRate);

    int32_t prevChannelCount;
    AMediaFormat_getInt32(mLastTrack->meta, AMEDIAFORMAT_KEY_CHANNEL_COUNT, &prevChannelCount);
    if (prevChannelCount != numChannels) {
       LOGE("mpeg4 audio channel count different from previous setting"
            "was: %d, now: %d",prevChannelCount,numChannels);
    }
    AMediaFormat_setInt32(mLastTrack->meta, AMEDIAFORMAT_KEY_CHANNEL_COUNT, numChannels);
    LOGV("numChannels: %d",numChannels);

    return OK;
}


static inline bool isValidInt32ColourValue(long long value) {
    return value != MAX_INT64 && value >= MIN_INT32 && value <= MAX_INT32;
}

static inline bool isValidUint16ColourValue(long long value) {
    return value != MAX_INT64 && value >= 0 && value <= MAX_UINT16;
}

static inline bool isValidPrimary(double primary[2]) {
    return primary[0] >= 0 && primary[0] <= 1 &&
           primary[1] >= 0 && primary[1] <= 1;
}

void MMParserExtractor::FillColorInformation(Track *track)
{
    status_t configStatus;
    FS_VIDEO_PARAM_COLOR_INFOTYPE sColor_params = {};
    configStatus = m_pFileSourceWrapper->GetStreamParameter(track->trackId,
                                  FS_IndexParamVideoColorInfo, &sColor_params);
    if(configStatus != FILE_SOURCE_SUCCESS)
        return;

    LOGV("FillColorInformation llPrimaries %llu, llTransferCharacteristics %llu, " \
         "llMatrixCoefficients %llu, llRange %llu",
         sColor_params.llPrimaries,sColor_params.llTransferCharacteristics,
         sColor_params.llMatrixCoefficients,sColor_params.llRange);

    int32_t primaries = 2;
    int32_t coeffs = 2;
    int32_t isotransfer = 2;
    bool fullRange = false;
    bool rangeSpecified = false;
    if (isValidInt32ColourValue(sColor_params.llPrimaries))
        primaries = sColor_params.llPrimaries;
    if (isValidInt32ColourValue(sColor_params.llTransferCharacteristics))
        isotransfer = sColor_params.llTransferCharacteristics;
    if (isValidInt32ColourValue(sColor_params.llMatrixCoefficients))
        coeffs = sColor_params.llMatrixCoefficients;
    if (FILE_SOURCE_MKV == m_eFileFormat &&
        sColor_params.llRange != 0x7fffffffffffffffLL &&
        sColor_params.llRange != 0) {
        fullRange = sColor_params.llRange == 2;
        rangeSpecified = true;
    }

    int32_t range = 0;
    int32_t standard = 0;
    int32_t transfer = 0;
    ColorUtils::convertIsoColorAspectsToPlatformAspects(
                primaries, isotransfer, coeffs, fullRange,
                &range, &standard, &transfer);
    if (range != 0) {
        AMediaFormat_setInt32(track->meta, AMEDIAFORMAT_KEY_COLOR_RANGE, range);
    }
    if (standard != 0) {
        AMediaFormat_setInt32(track->meta, AMEDIAFORMAT_KEY_COLOR_STANDARD, standard);
    }
    if (transfer != 0) {
        AMediaFormat_setInt32(track->meta, AMEDIAFORMAT_KEY_COLOR_TRANSFER, transfer);
    }

    //! Set HDR static info
    FS_VIDEO_PARAM_MASTER_METADATA_INFOTYPE sMaster_meta = {};
    configStatus = m_pFileSourceWrapper->GetStreamParameter(track->trackId,
                            FS_IndexParamVideoMasterMetaDataInfo, &sMaster_meta);
    if(configStatus != FILE_SOURCE_SUCCESS)
        return;

    LOGV("FillColorInformation R_X %lf, R_Y %lf, G_X %lf, G_Y %lf, B_X %lf, B_Y %lf, " \
         "llMaxCLL %lld, llMaxFALL %lld",
         sMaster_meta.fPrimChromaticityR_XY[0],sMaster_meta.fPrimChromaticityR_XY[1],
         sMaster_meta.fPrimChromaticityG_XY[0],sMaster_meta.fPrimChromaticityG_XY[1],
         sMaster_meta.fPrimChromaticityB_XY[0],sMaster_meta.fPrimChromaticityB_XY[1],
         sColor_params.llMaxCLL,sColor_params.llMaxFALL);
    HDRStaticInfo info;
    memset(&info, 0, sizeof(info));
    if (isValidUint16ColourValue(sColor_params.llMaxCLL))
        info.sType1.mMaxContentLightLevel = sColor_params.llMaxCLL;
    if (isValidUint16ColourValue(sColor_params.llMaxFALL))
        info.sType1.mMaxFrameAverageLightLevel = sColor_params.llMaxFALL;
    if (isValidPrimary(sMaster_meta.fPrimChromaticityR_XY) &&
        isValidPrimary(sMaster_meta.fPrimChromaticityG_XY) &&
        isValidPrimary(sMaster_meta.fPrimChromaticityB_XY)) {
        info.sType1.mR.x = (uint16_t)(50000 * sMaster_meta.fPrimChromaticityR_XY[0] + 0.5);
        info.sType1.mR.y = (uint16_t)(50000 * sMaster_meta.fPrimChromaticityR_XY[1] + 0.5);
        info.sType1.mG.x = (uint16_t)(50000 * sMaster_meta.fPrimChromaticityG_XY[0] + 0.5);
        info.sType1.mG.y = (uint16_t)(50000 * sMaster_meta.fPrimChromaticityG_XY[1] + 0.5);
        info.sType1.mB.x = (uint16_t)(50000 * sMaster_meta.fPrimChromaticityB_XY[0] + 0.5);
        info.sType1.mB.y = (uint16_t)(50000 * sMaster_meta.fPrimChromaticityB_XY[1] + 0.5);
    }
    if (isValidPrimary(sMaster_meta.fPrimChromaticityWhile_XY)) {
        info.sType1.mW.x = (uint16_t)(50000 * sMaster_meta.fPrimChromaticityWhile_XY[0] + 0.5);
        info.sType1.mW.y = (uint16_t)(50000 * sMaster_meta.fPrimChromaticityWhile_XY[1] + 0.5);
    }
    if ((sMaster_meta.fLuminance_Max >= 0.5) &&
        (sMaster_meta.fLuminance_Max < 65535.5)) {
        info.sType1.mMaxDisplayLuminance = (uint16_t)(sMaster_meta.fLuminance_Max + 0.5);
    }
    if ((sMaster_meta.fLuminance_Min >= 0.00005) &&
        (sMaster_meta.fLuminance_Min < 6.55355)) {
        info.sType1.mMinDisplayLuminance = (uint16_t)(10000 * sMaster_meta.fLuminance_Min + 0.5);
    }
    ColorUtils::setHDRStaticInfoIntoAMediaFormat(info, track->meta);
}

AudioEncoding getPcmEncoding(uint32 bitWidth) {
    AudioEncoding e = kAudioEncodingPcm16bit;
    switch(bitWidth) {
        case 8:
            e = kAudioEncodingPcm8bit;
            break;
        case 16:
            e = kAudioEncodingPcm16bit;
            break;
        case 24:
            e = kAudioEncodingPcm24bitPacked;
            break;
        case 32:
            e = kAudioEncodingPcm32bit;
            break;
        default:
            break;
    }
    return e;
}

//  Parse metadata in this function
status_t MMParserExtractor::readMetaData() {
    LOGV("MMParserExtractor::readMetaData");
    //! Check if metadata is already parsed or not
    if (NO_INIT != mHaveMetadata) {
        return mHaveMetadata;
    }

    //! Init with error, Update to "OK" if parsing is successful
    mHaveMetadata = UNKNOWN_ERROR;

    m_pFileSourceWrapper = FileSourceWrapper::New(*m_pSourcePort, m_eFileFormat);
    if(!m_pFileSourceWrapper){
        LOGE("readMetaData - FileSourceWrapper::New returned NULL");
        return ERROR_IO;
    }

    FileSourceFileFormat fileFormat = FILE_SOURCE_UNKNOWN;
    FileSourceStatus status = m_pFileSourceWrapper->GetFileFormat(fileFormat);

    FileSourceTrackIdInfoType trackList[FILE_SOURCE_MAX_NUM_TRACKS];
    uint32_t numTracks = m_pFileSourceWrapper->GetWholeTracksIDList(trackList);
    LOGV("readMetaData - Num tracks = %u", numTracks);
    //save the number of tracks
    uint8 trackPresent = 0;
    for (uint32_t i = 0; i < numTracks; i++){
        FileSourceTrackIdInfoType trackInfo = trackList[i];
        MediaTrackInfo mediaTrackInfo;

        LOGV("readMetaData - track %u", i);

        FileSourceMjMediaType majorType = FILE_SOURCE_MJ_TYPE_UNKNOWN;
        FileSourceMnMediaType minorType = FILE_SOURCE_MN_TYPE_UNKNOWN;
        status = m_pFileSourceWrapper->GetMimeType(trackInfo.id, majorType, minorType);
        if (status != FILE_SOURCE_SUCCESS) {
            LOGV("readMetaData failed to get mime type %d", status);
            continue;
        }

        LOGV("readMetaData - id[%u] majorType[%d] minorType[%d]",
             trackInfo.id, majorType, minorType);

        // get mediatrackinfo
        status = m_pFileSourceWrapper->GetMediaTrackInfo(trackInfo.id, &mediaTrackInfo);
        if (status != FILE_SOURCE_SUCCESS) {
            LOGV("readMetaData failed to get media track info %d", status);
            continue;
        }

        //! Some codec types have constraints with respect to bps, sampling freq etc
        bool bIsMIMETypeAllowed =
             m_pFileSourceWrapper->IsCodecSupportAvailable(fileFormat, minorType,
                                                           mediaTrackInfo);
        if (!bIsMIMETypeAllowed)
            continue;

        // get mime type
        const char* pMimeType = MediaType2MIME(minorType);
        if (NULL == pMimeType)
            continue;

        LOGV("readMetaData - mime type %s", pMimeType);

        // Create Track object and set metadata
        Track *track = new Track;
        track->next = NULL;
        if (mLastTrack) {
            mLastTrack->next = track;
        } else {
            mFirstTrack = track;
        }
        mLastTrack = track;

        track->timescale = 0;
        track->trackId = trackInfo.id;
        track->isExpensiveMetaPresent = false;
        track->meta = AMediaFormat_new();
        AMediaFormat_setString(track->meta, AMEDIAFORMAT_KEY_MIME, pMimeType);

        mTracks++;

        switch (majorType) {
            case FILE_SOURCE_MJ_TYPE_AUDIO:
                trackPresent |= AUDIO_TRACK;
                setAudioTrackMeta(track, mediaTrackInfo.audioTrackInfo);
                break;
            case FILE_SOURCE_MJ_TYPE_VIDEO:
                trackPresent |= VIDEO_TRACK;
                setVideoTrackMeta(track, mediaTrackInfo.videoTrackInfo);
                break;
            case FILE_SOURCE_MJ_TYPE_TEXT:
                trackPresent |= TEXT_TRACK;
                setTextTrackMeta(track, mediaTrackInfo.textTrackInfo);
                break;
            case FILE_SOURCE_MJ_TYPE_META:
                trackPresent |= META_TRACK;
                setMetaTrackMeta(track, mediaTrackInfo.sMetaTrackInfo);
                break;
            case FILE_SOURCE_MJ_TYPE_IMAGE:
                trackPresent |= IMAGE_TRACK;
                setImageTrackMeta(track, mediaTrackInfo.sImageTrackInfo);
                break;
            default:
                break;
        }

    }

    if (numTracks > 0 && mTracks == 0) {
        LOGE("readMetaData - No valid tracks!!");
        return BAD_TYPE;
    }

    const char* pFileType = GetFileMimeType(fileFormat, trackPresent);
    if(NULL != pFileType) {
        AMediaFormat_setString(mFileMetaData, AMEDIAFORMAT_KEY_MIME, pFileType);
        LOGV("readMetaData - GetFileMimeType:: %s",(char*)pFileType);
        Track* currentTrack = mFirstTrack;
        while (currentTrack != NULL) {
            AMediaFormat_setString(currentTrack->meta, "file-format", pFileType);
            currentTrack = currentTrack->next;
        }
    }

    setAlbumArtRelatedMeta();
    //! Check for AlbumArt info for all audio only clips
    if(trackPresent & AUDIO_TRACK) {
        setAlbumArt();
    }

    dumpMetadataToLog();

    mHaveMetadata = OK;
    return OK;
}

bool MMParserExtractor::setCodecSpecificData(Track* track, const char* key,
                                             FS_FORMAT_DATA_TYPE type, bool raw) {
    uint32 dataSize = 0;
    FileSourceStatus e = m_pFileSourceWrapper->GetFormatBlock(track->trackId, NULL,
                                                              &dataSize, raw, type);
    if (e != FILE_SOURCE_SUCCESS || !dataSize)
        return false;

    uint8* pBuffer = new uint8_t[dataSize];
    if (!pBuffer)
        return false;

    e = m_pFileSourceWrapper->GetFormatBlock(track->trackId, pBuffer,
                                             &dataSize, raw, type);
    if (e == FILE_SOURCE_SUCCESS) {
        AMediaFormat_setBuffer(track->meta, key, pBuffer, dataSize);
    }
    delete[] pBuffer;

    return (e == FILE_SOURCE_SUCCESS);
}

void MMParserExtractor::checkAndSetArbitraryMode(Track* track, FileSourceMnMediaType minorType) {
    FileSourceFileFormat fileFormat = m_eFileFormat;
    bool arbitraryMode = false;
    //Set decoder arbitrary mode based on file format & codec type
    if (fileFormat == FILE_SOURCE_AVI ||
        fileFormat == FILE_SOURCE_MKV){
        //MPEG4 can contain B frames, use arbitrary mode
        if (minorType == FILE_SOURCE_MN_TYPE_MPEG4) {
            arbitraryMode = true;
        }
    }

    if (arbitraryMode)
        AMediaFormat_setInt32(track->meta, "use-arbitrary-mode", 1);
}

void MMParserExtractor::checkAndSetByteMode(Track* track, FileSourceMnMediaType minorType) {
    //! Disable byte mode by default. Audio to come up with possible solution
    FileSourceConfigItem pitem;
    status_t configStatus = m_pFileSourceWrapper->SetConfiguration(track->trackId, &pitem,
                                              FILE_SOURCE_MEDIA_OUTPUT_SINGLE_AUDIO_FRAME);
    if (configStatus != FILE_SOURCE_SUCCESS ) {
        LOGV("Unable to set single frame mode config");
    }

    if (FILE_SOURCE_MN_TYPE_DSD == minorType) {
        configStatus = m_pFileSourceWrapper->SetConfiguration(track->trackId, &pitem,
                                               FILE_SOURCE_MEDIA_OUTPUT_BYTES_STREAM);
        if (configStatus != FILE_SOURCE_SUCCESS) {
            LOGV("Setting DSD config failed %d", configStatus );
        }
    }
}

void MMParserExtractor::setAudioTrackMeta(Track* track, FileSourceAudioInfo& info) {
    size_t max_size = m_pFileSourceWrapper->GetTrackMaxFrameBufferSize(track->trackId);
    AMediaFormat *meta = track->meta;
    AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_MAX_INPUT_SIZE, max_size);
    // audio
    AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_CHANNEL_COUNT, info.numChannels);
    AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_SAMPLE_RATE, info.samplingRate);
    AMediaFormat_setString(meta, AMEDIAFORMAT_KEY_LANGUAGE, (const char*)info.ucLanguageCode);
    if (info.bitRate)
        AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_BIT_RATE, info.bitRate);
    if (info.maxBitRate)
        AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_MAX_BIT_RATE, info.maxBitRate);
    if (info.nEncoderDelay)
        AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_ENCODER_DELAY, info.nEncoderDelay);
    if (info.nPaddingDelay)
        AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_ENCODER_PADDING, info.nPaddingDelay);
    if (info.nBitsPerSample)
        AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_BITS_PER_SAMPLE, info.nBitsPerSample);

    track->timescale = info.timeScale;
    uint64 duration = info.duration;
    if (m_eFileFormat == FILE_SOURCE_3G2) {
        uint64 clipDuration = m_pFileSourceWrapper->GetClipDuration();
        if (clipDuration > 0)
            duration = clipDuration;
    }

    //! Set duration
    uint32 timescale  = info.timeScale;
    int64 kKeyDurationValue = duration;
    if (timescale && timescale != MICROSEC_TIMESCALE_UNIT)
        kKeyDurationValue = duration * (MICROSEC_TIMESCALE_UNIT / timescale);
    AMediaFormat_setInt64(meta, AMEDIAFORMAT_KEY_DURATION, kKeyDurationValue);

    FileSourceMnMediaType minorType = info.audioCodec;
    if (minorType == FILE_SOURCE_MN_TYPE_AAC ||
        minorType == FILE_SOURCE_MN_TYPE_AAC_ADTS ||
        minorType == FILE_SOURCE_MN_TYPE_AAC_ADIF ||
        minorType == FILE_SOURCE_MN_TYPE_AAC_LOAS) {

        const char* csd_key = AMEDIAFORMAT_KEY_CSD_0;
        if (setCodecSpecificData(track, AMEDIAFORMAT_KEY_ESDS, FS_FORMAT_DATA_ESDS))
            csd_key = "aac-codec-specific-data";

        if (setCodecSpecificData(track, csd_key)) {
            if (updateAudioTrackInfoFromCSD(track, csd_key) != OK)
                LOGE("updateAudioTrackInfoFromCSD failed!");
        }

        AacCodecData pCodecData;
        bool codecDataStatus = m_pFileSourceWrapper->GetAACCodecData(track->trackId,&pCodecData);

        if (codecDataStatus && pCodecData.ucAACProfile == 4) {
            LOGV("setting LTP profile to meta data");
            AMediaFormat_setInt32(meta, "aac-format-ltp", true);
        }

        FileSourceConfigItem pitem;
        status_t configStatus;
        if (minorType == FILE_SOURCE_MN_TYPE_AAC_ADIF ) {
            AMediaFormat_setInt32(meta, "aac-format-adif", true);
            mIsAacformatAdif = true;
            configStatus = m_pFileSourceWrapper->SetConfiguration(track->trackId, &pitem,
                                                    FILE_SOURCE_MEDIA_STRIP_AUDIO_HEADER);
            AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_MAX_INPUT_SIZE, 1536*2);
        } else {
            AMediaFormat_setInt32(meta, "aac-format-adif", false);
            if (codecDataStatus) {
                LOGV("set profile info to meta data");
                AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_AAC_PROFILE ,pCodecData.ucAACProfile);
                // Set strip audio header config only if audio object type is
                // FILE_SOURCE_AAC_PROFILE_LC or FILE_SOURCE_AAC_PROFILE_LTP
                if (pCodecData.ucAACProfile == 2 || pCodecData.ucAACProfile == 4)
                    configStatus = m_pFileSourceWrapper->SetConfiguration(track->trackId,
                                                &pitem, FILE_SOURCE_MEDIA_STRIP_AUDIO_HEADER);
            }
            if(m_eFileFormat == FILE_SOURCE_AAC && minorType == FILE_SOURCE_MN_TYPE_AAC_ADTS) {
                AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_IS_ADTS, 1);
                configStatus = m_pFileSourceWrapper->SetConfiguration(track->trackId,
                                                &pitem, FILE_SOURCE_MEDIA_RETAIN_AUDIO_HEADER);
            }
        }
    } else if (minorType == FILE_SOURCE_MN_TYPE_WMA ||
             minorType == FILE_SOURCE_MN_TYPE_WMA_PRO ||
             minorType == FILE_SOURCE_MN_TYPE_WMA_LOSSLESS) {
        WmaCodecData codecData;
        if (m_pFileSourceWrapper->GetWMACodecData(track->trackId, &codecData)) {
            AMediaFormat_setInt32(meta, "wma-encode-opt", codecData.nEncodeOpt);
            AMediaFormat_setInt32(meta, "wma-block-align", codecData.nBlockAlign);
            if (max_size < codecData.nASFPacketSize) {
                AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_MAX_INPUT_SIZE, codecData.nASFPacketSize);
            }
            AMediaFormat_setInt32(meta, "wma-bits-per-sample", codecData.nBitsPerSample);
            AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_BITS_PER_SAMPLE, codecData.nBitsPerSample);
            AMediaFormat_setInt32(meta, "wma-channel-mask", codecData.nChannelMask);
            AMediaFormat_setInt32(meta, "wma-format-tag", codecData.nFormatTag);
            if (minorType == FILE_SOURCE_MN_TYPE_WMA_PRO ||
                minorType == FILE_SOURCE_MN_TYPE_WMA_LOSSLESS) {
                codecData.nVirtualPktSize = (((codecData.nVirtualPktSize & 0xff) << 8) |
                                             ((codecData.nVirtualPktSize & 0xff00) >> 8));
                AMediaFormat_setInt32(meta, "wma-vir-pkt-size", codecData.nVirtualPktSize);
                AMediaFormat_setInt32(meta, "wma-adv-enc-opt1", codecData.nAdvEncodeOpt);
                AMediaFormat_setInt32(meta, "wma-adv-enc-opt2", codecData.nAdvEncodeOpt2);
            }
        }

        if (minorType == FILE_SOURCE_MN_TYPE_WMA_PRO) {
            AMediaFormat_setInt32(meta, "wma-version", kTypeWMAPro);
        } else if (minorType == FILE_SOURCE_MN_TYPE_WMA_LOSSLESS) {
            AMediaFormat_setInt32(meta, "wma-version", kTypeWMALossLess);
        } else if (minorType == FILE_SOURCE_MN_TYPE_WMA) {
            AMediaFormat_setInt32(meta, "wma-version", kTypeWMA);
        }
    } else if (FILE_SOURCE_MN_TYPE_DTS == minorType) {
        FS_AUDIO_PARAM_DTSTYPE dts_params;
        status_t configStatus = m_pFileSourceWrapper->GetStreamParameter(track->trackId,
                                                                         FS_IndexParamAudioDTS,
                                                                         &dts_params);
        if (configStatus == FILE_SOURCE_SUCCESS) {
            AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_CHANNEL_COUNT, dts_params.usNumChannels);
            AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_SAMPLE_RATE, dts_params.ulSamplingRate);
            if(dts_params.ulBitRate) {
                AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_BIT_RATE, dts_params.ulBitRate);
            }
        }
    } else if (FILE_SOURCE_MN_TYPE_G711_ALAW  == minorType ||
               FILE_SOURCE_MN_TYPE_PCM        == minorType ||
               FILE_SOURCE_MN_TYPE_AIFF_PCM   == minorType ||
               FILE_SOURCE_MN_TYPE_G711_MULAW == minorType ||
               FILE_SOURCE_MN_TYPE_G721       == minorType ||
               FILE_SOURCE_MN_TYPE_G723       == minorType ||
               FILE_SOURCE_MN_TYPE_GSM_FR     == minorType) {
        FileSourceConfigItem pitem;
        status_t configStatus;
        if (24 == info.nBitsPerSample && FILE_SOURCE_MN_TYPE_AIFF_PCM == minorType) {
            AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_BITS_PER_SAMPLE, 24);
            AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_PCM_ENCODING, getPcmEncoding((uint32)24));
            configStatus = m_pFileSourceWrapper->SetConfiguration(track->trackId, &pitem,
                                              FILE_SOURCE_MEDIA_DISABLE_PCM_SAMPLE_UPGRADE);
        } else if (8 == info.nBitsPerSample && (FILE_SOURCE_MN_TYPE_PCM == minorType ||
                                                FILE_SOURCE_MN_TYPE_AIFF_PCM == minorType)) {
            AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_BITS_PER_SAMPLE, 16);
            AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_PCM_ENCODING, kAudioEncodingPcm16bit);
            configStatus = m_pFileSourceWrapper->SetConfiguration(track->trackId, &pitem,
                                              FILE_SOURCE_MEDIA_ENABLE_PCM_SAMPLE_UPGRADE);
            size_t max_size = m_pFileSourceWrapper->GetTrackMaxFrameBufferSize(track->trackId);
            AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_MAX_INPUT_SIZE, max_size);
        } else if (24 == info.nBitsPerSample && FILE_SOURCE_MN_TYPE_PCM == minorType) {
            AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_BITS_PER_SAMPLE, 24);
            AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_PCM_ENCODING, getPcmEncoding((uint32)24));
        } else if (FILE_SOURCE_MN_TYPE_G711_ALAW == minorType ||
                   FILE_SOURCE_MN_TYPE_G711_MULAW == minorType) {
            AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_PCM_ENCODING, kAudioEncodingPcm16bit);
            AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_MAX_INPUT_SIZE, 16384);
        } else {
            AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_PCM_ENCODING, getPcmEncoding(info.nBitsPerSample));
        }

        WavFormatData wav_params;
        if (m_pFileSourceWrapper->GetWavCodecData(track->trackId, &wav_params)) {
            AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_CHANNEL_MASK, wav_params.channel_mask);
        }
    } else if(FILE_SOURCE_MN_TYPE_VORBIS == minorType) {
        bool needFetchSample = true; //TODO: Move to set CSD data directly instead of fetch samples
        if (setCodecSpecificData(track, AMEDIAFORMAT_KEY_CSD_0, FS_FORMAT_DATA_VORBIS_INFO) &&
            setCodecSpecificData(track, AMEDIAFORMAT_KEY_CSD_1, FS_FORMAT_DATA_VORBIS_BOOKS)) {
            setCodecSpecificData(track, "vorbis-data", FS_FORMAT_DATA_VORBIS_DATA);
            needFetchSample = false;
        }
        uint32 nFormatBlockSize = 0;
        //Get the size of the format block.
        FileSourceStatus status = m_pFileSourceWrapper->GetFormatBlock(track->trackId, NULL,
                                                                       &nFormatBlockSize);
        LOGV(" ogg getFormatBlock size = %u \n", nFormatBlockSize);

        if(needFetchSample && (status==FILE_SOURCE_SUCCESS) && nFormatBlockSize!=0) {
            uint8_t *buffer = new uint8_t[nFormatBlockSize];
            uint32 ulIndex = 1;
            while(buffer!=NULL && ulIndex < 4) {
                uint32 ulBufSize = nFormatBlockSize;
                FileSourceSampleInfo sampleInfo;
                memset(&sampleInfo, 0, sizeof(FileSourceSampleInfo));
                if(FILE_SOURCE_DATA_OK == m_pFileSourceWrapper->GetNextMediaSample(track->trackId,
                                                                 buffer, &ulBufSize, sampleInfo)) {
                    if(1 == ulIndex) {
                        AMediaFormat_setBuffer(meta, AMEDIAFORMAT_KEY_CSD_0, buffer, ulBufSize);
                        LOGV(" kKeyVorbisInfo set, size %u \n", ulBufSize);
                        if(ulBufSize > 12)
                        {
                            info.numChannels = buffer[11];
                            LOGV(" set vorbis channel from IdentificationHDR %d",  buffer[11]);
                            AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_CHANNEL_COUNT, info.numChannels);
                        }
                    } else if (2 == ulIndex) {
                        AMediaFormat_setBuffer(meta, "vorbis-data", buffer, ulBufSize);
                        LOGV(" kKeyVorbisData set, size %u \n", ulBufSize);
                    } else if (3 == ulIndex) {
                        AMediaFormat_setBuffer(meta, AMEDIAFORMAT_KEY_CSD_1, buffer, ulBufSize);
                        LOGV(" kKeyVorbisBooks set, size %u \n", ulBufSize);
                    }
                } else {
                    LOGE(" getFormatBlock failed \n");
                }
                ulIndex++;
            }
            delete[] buffer;
        }

        uint8 pBuf[12];
        uint32 ulSize = 12;
        FileSourceStatus eRet = m_pFileSourceWrapper->GetClipMetaData
                                              ((wchar_t*)pBuf, &ulSize,
                                               FILE_SOURCE_MD_ANDROID_LOOP);
        if(FILE_SOURCE_SUCCESS == eRet) {
            LOGV("android loop data is present, %s", pBuf);
            if(!strncmp("true", (char*)pBuf, 4)) {
                AMediaFormat_setInt32(mFileMetaData, AMEDIAFORMAT_KEY_LOOP, true);
                LOGV("kkeyauto loop set to true");
            }
        }
    } else if(FILE_SOURCE_MN_TYPE_FLAC == minorType) {
        FlacFormatData flacParams;
        status_t configStatus = m_pFileSourceWrapper->GetStreamParameter(track->trackId,
                                                                         FS_IndexParamAudioFlac,
                                                                         &flacParams);
        if (FILE_SOURCE_SUCCESS == configStatus) {
            AMediaFormat_setInt32(meta, "min-block-size", flacParams.nMinBlockSize);
            AMediaFormat_setInt32(meta, "max-block-size", flacParams.nMaxBlockSize);
            AMediaFormat_setInt32(meta, "min-frame-size", flacParams.nMinFrameSize);
            AMediaFormat_setInt32(meta, "max-frame-size", flacParams.nMaxFrameSize);
        }
        setCodecSpecificData(track, AMEDIAFORMAT_KEY_CSD_0);
    } else if(FILE_SOURCE_MN_TYPE_OPUS == minorType) {
        FS_AUDIO_PARAM_OPUSTYPE sOpus_params = {};
        status_t configStatus = m_pFileSourceWrapper->GetStreamParameter(track->trackId,
                                                                         FS_IndexParamAudioOPUS,
                                                                         &sOpus_params);
        if (configStatus == FILE_SOURCE_SUCCESS ) {
            AMediaFormat_setBuffer(meta, AMEDIAFORMAT_KEY_CSD_1, &sOpus_params.ullCodecDelay, \
                                   sizeof(sOpus_params.ullCodecDelay));
            AMediaFormat_setBuffer(meta, AMEDIAFORMAT_KEY_CSD_2, &sOpus_params.ullSeekPreRoll, \
                                   sizeof(sOpus_params.ullSeekPreRoll));
        }
        setCodecSpecificData(track, AMEDIAFORMAT_KEY_CSD_0, FS_FORMAT_DATA_DEFAULT, true);
    } else if (FILE_SOURCE_MN_TYPE_APE == minorType ||
               FILE_SOURCE_MN_TYPE_ALAC == minorType) {
        setCodecSpecificData(track, "raw-codec-specific-data");
    } else if (FILE_SOURCE_MN_TYPE_MHAS == minorType) {
        setCodecSpecificData(track, AMEDIAFORMAT_KEY_CSD_0, FS_FORMAT_DATA_MHAS_CONFIG);
        setCodecSpecificData(track, AMEDIAFORMAT_KEY_CSD_1, FS_FORMAT_DATA_MHAS_SCENEINFO);
    }
    //! Call Seek to ZERO API. Parser will set
    //! data pointers to start if it's not in proper state
    checkAndSetByteMode(track, minorType);
    m_pFileSourceWrapper->SeekAbsolutePosition(0);
}

void MMParserExtractor::setMetaTrackMeta(Track* track, FileSourceMetaInfo& info) {
    FileSourceMnMediaType minorType = info.metaCodec;
    size_t max_size = m_pFileSourceWrapper->GetTrackMaxFrameBufferSize(track->trackId);
    AMediaFormat_setInt32(track->meta, AMEDIAFORMAT_KEY_MAX_INPUT_SIZE, max_size);
    if (minorType == FILE_SOURCE_MN_TYPE_META_TEXT) {
        uint32 dataSize = 0;
        FileSourceStatus e = m_pFileSourceWrapper->GetFormatBlock(track->trackId, NULL,
                                                                  &dataSize);
        if (e == FILE_SOURCE_SUCCESS && dataSize) {
            uint8* buffer = new uint8_t[dataSize];
            if (buffer) {
                e = m_pFileSourceWrapper->GetFormatBlock(track->trackId, buffer,
                                                         &dataSize);
                if (e == FILE_SOURCE_SUCCESS) {
                    AMediaFormat_setString(track->meta, AMEDIAFORMAT_KEY_MIME,
                                           (const char *)buffer);
                    track->timescale = info.ulTimeScale;
                    int64 kKeyDurationValue = info.ullDuration;
                    if (track->timescale && track->timescale != MICROSEC_TIMESCALE_UNIT) {
                        kKeyDurationValue = kKeyDurationValue * \
                                            (MICROSEC_TIMESCALE_UNIT / track->timescale);
                    }
                    AMediaFormat_setInt64(track->meta, AMEDIAFORMAT_KEY_DURATION,
                                          kKeyDurationValue);
                    LOGV("setMetaTrackMeta - mime = %s timescale = %u duration = %llu",
                         (const char *)buffer, track->timescale, kKeyDurationValue);
                }
                delete[] buffer;
            }
        }
    }
}

void MMParserExtractor::setTextTrackMeta(Track* track, FileSourceTextInfo& info) {
    //! Set maximum input buffer size
    size_t max_size = m_pFileSourceWrapper->GetTrackMaxFrameBufferSize(track->trackId);
    AMediaFormat_setInt32(track->meta, AMEDIAFORMAT_KEY_MAX_INPUT_SIZE, max_size);

    // Update Duration and timescale values
    track->timescale = info.timeScale;

    int64 duration = info.duration;
    if (m_eFileFormat == FILE_SOURCE_3G2) {
        int64 clipDuration = m_pFileSourceWrapper->GetClipDuration();
        if (clipDuration > duration)
            duration = clipDuration;
    }

    int64 kKeyDurationValue = duration;
    if(track->timescale && track->timescale != MICROSEC_TIMESCALE_UNIT) {
        kKeyDurationValue = (duration * MICROSEC_TIMESCALE_UNIT) /
                            track->timescale;
    }
    AMediaFormat_setInt64(track->meta, AMEDIAFORMAT_KEY_DURATION, kKeyDurationValue);
    AMediaFormat_setString(track->meta, AMEDIAFORMAT_KEY_LANGUAGE, (const char*)info.ucLanguageCode);

    setCodecSpecificData(track, AMEDIAFORMAT_KEY_TEXT_FORMAT_DATA);
}

void MMParserExtractor::setVideoTrackMeta(Track* track, FileSourceVideoInfo& info) {
    AMediaFormat *meta = track->meta;
    FileSourceVideoInfo vInfo;
    memcpy(&vInfo, &info, sizeof(vInfo));
    char ucPropValue[PROPERTY_VALUE_MAX] = {(char)0};
    property_get("vendor.qcom_parser.video.ip.buf.size", ucPropValue, "0");
    if (atoi(ucPropValue) > 0) {
        vInfo.ulMaxBufSize = (uint32)(vInfo.frameWidth * vInfo.frameHeight * 1.5);
        m_pFileSourceWrapper->SetStreamParameter(track->trackId,
                                (uint32)FS_IndexParamOtherMediaTrackInfo, &vInfo);
    }

    //! Set maximum input buffer size
    size_t max_size = m_pFileSourceWrapper->GetTrackMaxFrameBufferSize(track->trackId);
    AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_MAX_INPUT_SIZE, max_size);

    //! Set video resolution and display parameters
    AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_WIDTH, info.frameWidth);
    AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_HEIGHT, info.frameHeight);
    if (info.ulDisplayWidth && info.ulDisplayHeight) {
        AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_DISPLAY_WIDTH, info.ulDisplayWidth);
        AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_DISPLAY_HEIGHT, info.ulDisplayHeight);
    }

    if (info.ulFrameCount)
        AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_FRAME_COUNT, info.ulFrameCount);

    //! Set rotation degree if container provide information
    AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_ROTATION, info.ulRotationDegrees);
    AMediaFormat_setString(meta, AMEDIAFORMAT_KEY_LANGUAGE, (const char*)info.ucLanguageCode);
    track->timescale = info.timeScale;
    LOGV("setVideoTrackMeta - frameWidth = %u frameHeight = %u timeScale = %u duration = %llu",
         info.frameWidth, info.frameHeight, info.timeScale, info.duration);

    //! Set captured and caculated frame rate
    AMediaFormat_setFloat(mFileMetaData, AMEDIAFORMAT_KEY_CAPTURE_RATE,info.capturedFrameRate);
    AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_FRAME_RATE, (int32_t)(info.frameRate));
    LOGV("setVideoTrackMeta - captured frame rate is %f and calc frame rate is %f",
         info.capturedFrameRate, info.frameRate);

    uint64 duration = info.duration;
    if (m_eFileFormat == FILE_SOURCE_3G2) { //File is fragmented
        uint64 clipDuration = m_pFileSourceWrapper->GetClipDuration();
        if (clipDuration > 0) {
            duration = clipDuration;
            LOGV("setVideoTrackMeta - updated duration = %llu",clipDuration);
        }
    }

    //! Set duration
    uint32 timescale  = info.timeScale;
    int64 kKeyDurationValue = duration;
    if (timescale && timescale != MICROSEC_TIMESCALE_UNIT)
        kKeyDurationValue = duration * (MICROSEC_TIMESCALE_UNIT / timescale);
    AMediaFormat_setInt64(meta, AMEDIAFORMAT_KEY_DURATION, kKeyDurationValue);
    LOGV("setVideoTrackMeta - kKeyDuration =  %llu", (unsigned long long)kKeyDurationValue);

    //! AVCC and HVCC is available only in MKV and MP4 file formats.
    //! Provide that data as it is to upper layer. Fill color information as well.
    FileSourceMnMediaType minorType = info.videoCodec;
    bool bRawCodecData = false;
    if (FILE_SOURCE_3G2 == m_eFileFormat || FILE_SOURCE_MPEG4 == m_eFileFormat ||
        FILE_SOURCE_MKV == m_eFileFormat) {
        FillColorInformation(track);
        if (FILE_SOURCE_MN_TYPE_H264 == minorType ||
            FILE_SOURCE_MN_TYPE_HEVC == minorType) {
            bRawCodecData = true;
        }
    }

    //! Set CSD based on codec type
    if (!bRawCodecData) {
        if (FILE_SOURCE_MN_TYPE_WMV3 == minorType ||
            FILE_SOURCE_MN_TYPE_VC1 == minorType) {
            setCodecSpecificData(track, "raw-codec-specific-data");
        } else {
            setCodecSpecificData(track, AMEDIAFORMAT_KEY_CSD_0);
        }
    } else if (FILE_SOURCE_MN_TYPE_H264 == minorType) {
        setCodecSpecificData(track, AMEDIAFORMAT_KEY_CSD_AVC, FS_FORMAT_DATA_DEFAULT, bRawCodecData);
    } else if (FILE_SOURCE_MN_TYPE_HEVC == minorType) {
        setCodecSpecificData(track, AMEDIAFORMAT_KEY_CSD_HEVC, FS_FORMAT_DATA_DEFAULT, bRawCodecData);
    } else if (FILE_SOURCE_MN_TYPE_VP9 == minorType) {
        setCodecSpecificData(track, AMEDIAFORMAT_KEY_CSD_0, FS_FORMAT_DATA_DEFAULT, bRawCodecData);
    }

    checkAndSetArbitraryMode(track, minorType);

    //! Set Default Key_Date
    AMediaFormat_setString(mFileMetaData, AMEDIAFORMAT_KEY_DATE, "0");
}

void MMParserExtractor::setImageTrackMeta(Track* track, FileSourceImageInfo& info) {
    AMediaFormat *meta = track->meta;
    if (info.eImageCodec == FILE_SOURCE_MN_TYPE_HEIF) {
        AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_TRACK_ID, track->trackId);
        AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_WIDTH, info.ulImageWidth);
        AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_HEIGHT, info.ulImageHeight);
        AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_MAX_INPUT_SIZE, info.ulMaxBufSize);
        if (info.ucRotation)
            AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_ROTATION, info.ucRotation);
        if (info.bIsPrimary) {
            AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_IS_DEFAULT, 1);
            if (info.ulExifSize) {
                AMediaFormat_setInt64(mFileMetaData, AMEDIAFORMAT_KEY_EXIF_OFFSET,
                                      (int64_t)info.ullExifOffset);
                AMediaFormat_setInt64(mFileMetaData, AMEDIAFORMAT_KEY_EXIF_SIZE,
                                      (int64_t)info.ulExifSize);
            }
        }
        if (info.ulThumbnailWidth && info.ulThumbnailHeight) {
            AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_THUMBNAIL_WIDTH, info.ulThumbnailWidth);
            AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_THUMBNAIL_HEIGHT, info.ulThumbnailHeight);
            //TODO: Differentiate from image hevc data
            setCodecSpecificData(track, AMEDIAFORMAT_KEY_CSD_HEVC,
                                 FS_FORMAT_DATA_HEIF_THUMBNAIL_HVCC);
        }
        if (info.bIsGrid) {
            AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_GRID_ROWS, info.ucGridRows);
            AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_GRID_COLUMNS, info.ucGridColumns);
            AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_TILE_WIDTH, info.ulTileWidth);
            AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_TILE_HEIGHT, info.ulTileHeight);
        }
        setCodecSpecificData(track, AMEDIAFORMAT_KEY_CSD_HEVC, FS_FORMAT_DATA_HEIF_HVCC);
        setCodecSpecificData(track, AMEDIAFORMAT_KEY_ICC_PROFILE, FS_FORMAT_DATA_HEIF_ICC);
    }
}

void MMParserExtractor::findThumbnail(Track *track) {
    const char *mime;
    CHECK(AMediaFormat_getString(track->meta, AMEDIAFORMAT_KEY_MIME, &mime));
    if (strncasecmp(mime, "video/", 6))
        return;

    int64_t duration = 0;
    CHECK(AMediaFormat_getInt64(track->meta, AMEDIAFORMAT_KEY_DURATION, &duration));
    duration = duration / MILLISEC_TIMESCALE_UNIT;
    //! Find sync frames within 40% of duration
    //! Calculate delta based on total num of sync frames required
    //! In this case, that value is 5
    uint64 Delta = (uint64)(duration * 10) /
                           (NUM_SYNC_FRAMES * 100);
    LOGV("Delta value is : %llu", Delta);
    //We need to pick a suitable iframe for thumbnail generation
    int64_t thumbnailTimestampUs = 0;
    uint32 maxSyncSampleSize = 0;
    if (!m_pFileSourceWrapper->IsSeekDenied()) {
        uint32 buffLen = m_pFileSourceWrapper->GetTrackMaxFrameBufferSize(track->trackId);
        void *buff = malloc(buffLen);
        FileSourceSampleInfo sampleInfo;
        memset(&sampleInfo, 0, sizeof(FileSourceSampleInfo));
        uint64 lastSyncSampleTime_real = 0;

        for (uint64 lastSyncSampleTime = 0, syncSample = 0;
             syncSample < NUM_SYNC_FRAMES && buff != NULL; ++syncSample) {
            if( ((m_eFileFormat == FILE_SOURCE_MP2TS)||
                  (m_eFileFormat == FILE_SOURCE_MKV)||
                  (m_eFileFormat == FILE_SOURCE_ASF)) && syncSample)
            {
                thumbnailTimestampUs = lastSyncSampleTime_real * 1000;
                break;
            }
            //! Asf, find 1 sync frame at 0 ts
            //! MKV, find 1 sync frame at 5% duration
            if(m_eFileFormat == FILE_SOURCE_ASF){
                lastSyncSampleTime = 0;
            }
            else if(m_eFileFormat == FILE_SOURCE_MKV){
                lastSyncSampleTime = duration/20;
            }
            uint32 bytesRead = 0;
            FileSourceMediaStatus status;

            //seek to next sync sample
            FileSourceStatus stat = m_pFileSourceWrapper->SeekAbsolutePosition
                    (track->trackId, lastSyncSampleTime, true,
                     lastSyncSampleTime);
            if (FILE_SOURCE_SUCCESS != stat){
                LOGE("SeekAbsolutePosition returns err = %d\n", stat);
                break;
            }
            lastSyncSampleTime_real = lastSyncSampleTime;
            bytesRead = buffLen; //GetNextMediaSample will set the actual bytes read
            status = m_pFileSourceWrapper->GetNextMediaSample(track->trackId,
                                                              (uint8 *)buff,
                                                              &bytesRead,
                                                              sampleInfo);
            /* Increase Sync Sample time by 1ms,
               This is required to do seek for next sync frame.
               Sample time is in micro-sec untis, convert to milli-sec*/
            lastSyncSampleTime = (sampleInfo.startTime)/1000 + Delta;

            if (FILE_SOURCE_DATA_OK != status || bytesRead <= 0){
                //Some error in reading, file is probably screwed up
                //lets not bother parsing further
                break;
            } else if (bytesRead >= maxSyncSampleSize
                    && sampleInfo.bStartTsValid) {
                thumbnailTimestampUs = sampleInfo.startTime;
                maxSyncSampleSize = bytesRead;
            }
        }//for(lastSyncSampleTime)
        if(buff){
            free(buff);
        }
    }
    m_pFileSourceWrapper->SeekAbsolutePosition(0); //reposition back to 0
    AMediaFormat_setInt64(track->meta, AMEDIAFORMAT_KEY_THUMBNAIL_TIME, thumbnailTimestampUs);
    LOGV("thumbnail time is %" PRId64 ", max sync sample size %u",
         thumbnailTimestampUs, maxSyncSampleSize);
}

void MMParserExtractor::dumpMetadataToLog() {
    if(m_sDebug.bDumpMetaToLog) {
        LOGI("[METADUMP] Start");
        LOGI("[METADUMP] FileMeta");
        LOGI("%s",AMediaFormat_toString(mFileMetaData));

        int count = 0;
        Track* currentTrack = mFirstTrack;
        while (currentTrack != NULL) {
            LOGI("[METADUMP] Track %d",++count);
            LOGI("%s",AMediaFormat_toString(currentTrack->meta));
            currentTrack = currentTrack->next;
        }
        LOGI("[METADUMP] End");
    }
}
MediaTrackHelper* MMParserExtractor::getTrack(size_t index) {
    LOGV("MMParserExtractor::getTrack : index = %zu", index);
    status_t err;
    if ((err = readMetaData()) != OK) {
        return NULL;
    }

    Track *track = mFirstTrack;
    while (index > 0) {
        if (track == NULL) {
            return NULL;
        }
        track = track->next;
        --index;
    }

    return new MMParserSource(track->meta, this, m_pFileSourceWrapper,
                              track->trackId, track->timescale);
}

uint32_t MMParserExtractor::flags() const {
    LOGV("MMParserExtractor::flags");
    if(m_pFileSourceWrapper==NULL) {
        LOGE("MMParserExtractor::flags problem m_pFileSourceWrapper not yet inited - say we support nothing");
        return 0;
    }
    const uint8 seekDenied = m_pFileSourceWrapper->IsSeekDenied();
    if(seekDenied) {
        LOGE("Note - seek not supported for this clip");
        return CAN_PAUSE;
    }

    if (mIsAacformatAdif == true) {
        return (CAN_SEEK_TO_ZERO | CAN_PAUSE);
    } else {
        return (CAN_SEEK | CAN_SEEK_BACKWARD | CAN_SEEK_FORWARD | CAN_PAUSE);
    }
}

MMParserSource::MMParserSource(
        AMediaFormat *format,
        MMParserExtractor* extractor,
        FileSourceWrapper *fileSourceWrapper,
        uint32_t trackId, int32_t timeScale)
    : mFormat(format),
      mMMParserExtractor(extractor),
      mTimescale(timeScale),
      mTrackId(trackId),
      m_pFileSourceWrapper(fileSourceWrapper),
      mCurrentSampleIndex(0),
      mCurrentTimeMS(0),
      mMajorType(FILE_SOURCE_MJ_TYPE_UNKNOWN),
      mMinorType(FILE_SOURCE_MN_TYPE_UNKNOWN),
      mFileFormat(FILE_SOURCE_UNKNOWN),
      mStarted(false),
      mBuffer(NULL),
      mFile(NULL) {
    const char *mime;
    CHECK(AMediaFormat_getString(mFormat, AMEDIAFORMAT_KEY_MIME, &mime));
    memset(&mExtraData, 0, sizeof(FS_PARAM_EXTRA_DATATYPE));
    LOGV("MMParserSource() id %u mime = %s timescale = %d", trackId, mime, timeScale);
}

MMParserSource::~MMParserSource() {
    if (mStarted) {
        stop();
    }

    if (mExtraData.pucData != NULL) {
        free(mExtraData.pucData);
        mExtraData.pucData = NULL;
    }

    if (mFile != NULL) {
        fclose(mFile);
        mFile = NULL;
    }
}

media_status_t MMParserSource::start() {
    CHECK(!mStarted);

    LOGV("MMParserSource::start track %d", mTrackId);

    m_pFileSourceWrapper->GetMimeType(mTrackId, mMajorType, mMinorType);
    m_pFileSourceWrapper->GetFileFormat(mFileFormat);

    int32_t ulSize = 0;
    CHECK(AMediaFormat_getInt32(mFormat, AMEDIAFORMAT_KEY_MAX_INPUT_SIZE, &ulSize));
    size_t max_size = ulSize;
    LOGV("MMParserSource::start max input size = %zu", max_size);

    if (max_size == 0 || max_size > MAX_MEDIA_BUFFER_SIZE)
      return AMEDIA_ERROR_MALFORMED;

    const char *mime;
    AMediaFormat_getString(mFormat, AMEDIAFORMAT_KEY_MIME, &mime);
    if (!strncasecmp(mime, "video/", 6)) {
        max_size += 10 * 2;
    }

    size_t growthLimit = 0;
    size_t initBuffers = 1;
    if (supportsNonBlockingRead()) {
        growthLimit = MAX_MEDIA_BUFFER_SIZE / max_size;
        if (growthLimit > MAX_MEDIA_BUFFERS)
            growthLimit = MAX_MEDIA_BUFFERS;
        initBuffers = INIT_MEDIA_BUFFERS;
    }
    mBufferGroup->init(initBuffers, max_size, growthLimit);
    LOGV("MMParserSource::start buffers %zu buffer_size %zu growthLimit %zu",
         initBuffers, max_size, growthLimit);

    mStarted = true;
    return AMEDIA_OK;
}

media_status_t MMParserSource::stop() {
    CHECK(mStarted);

    LOGV("MMParserSource::stop track %d", mTrackId);

    if (mBuffer != NULL) {
        mBuffer->release();
        mBuffer = NULL;
    }

    mStarted = false;
    mCurrentSampleIndex = 0;

    return AMEDIA_OK;
}

media_status_t MMParserSource::getFormat(AMediaFormat *meta) {
    LOGV("MMParserSource::getFormat");
    AMediaFormat_copy(meta, mFormat);
    return AMEDIA_OK;
}

media_status_t MMParserSource::read(
        MediaBufferHelper **out, const ReadOptions *options) {
    CHECK(mStarted);

    *out = NULL;
    if (supportsNonBlockingRead() && options &&
        options->getNonBlocking() && !mBufferGroup->has_buffers()) {
        return AMEDIA_ERROR_WOULD_BLOCK;
    }

    int64_t seekTimeUs;
    ReadOptions::SeekMode seekMode;
    FS_SEEK_MODE m_eSeekMode = FS_SEEK_DEFAULT;
    FS_SEEK_INFOTYPE seekInfo = {0};
    seekInfo.targetTimeUs = -1;

    if (options && options->getSeekTo(&seekTimeUs, &seekMode)) {
        uint64_t seekTimems = 0;
        LOGV("MMParserSource::read Seek seekTimeUs = %" PRId64 ", seekMode %d", seekTimeUs, seekMode);
        //! Seektime needs to be in milli-sec units, Convert from Us to Ms.
        if (seekTimeUs < 0 && mMinorType == FILE_SOURCE_MN_TYPE_HEIF) {
          //! Extract thumbnail
          seekTimems = MAX_UINT64;
        } else {
            seekTimems = (seekTimeUs / 1000);
            switch(seekMode)
            {
                case ReadOptions::SEEK_PREVIOUS_SYNC:
                    m_eSeekMode = FS_SEEK_PREV;
                    break;
                case ReadOptions::SEEK_NEXT_SYNC:
                    m_eSeekMode = FS_SEEK_NEXT;
                    break;
                case ReadOptions::SEEK_CLOSEST_SYNC:
                    m_eSeekMode = FS_SEEK_CLOSEST;
                    break;
                case ReadOptions::SEEK_CLOSEST:
                    m_eSeekMode = FS_SEEK_CLOSEST_NOSYNC;
                    break;
                case ReadOptions::SEEK_FRAME_INDEX:
                    m_eSeekMode = FS_SEEK_FRAME_INDEX;
                    seekTimems = seekTimeUs;
                    break;
                default:
                    break;
            }
        }
        LOGV("MMParserSource::read Seek seekTimems = %" PRIu64 ", mTrackId = %d", seekTimems,mTrackId);
        FileSourceStatus stat = m_pFileSourceWrapper->SeekAbsolutePosition((int64)mTrackId,
                                        seekTimems, true, mCurrentTimeMS,m_eSeekMode);
        if (mBuffer != NULL) {
            mBuffer->release();
            mBuffer = NULL;
        }

        if (FILE_SOURCE_SUCCESS != stat) {
            LOGE("SeekAbsolutePosition returns err = %d", stat);
            return AMEDIA_ERROR_END_OF_STREAM;
        }

        LOGV("FileSource SeekAbsolutePosition success");

        if (m_eSeekMode == FS_SEEK_CLOSEST_NOSYNC || m_eSeekMode == FS_SEEK_FRAME_INDEX) {
          if (FILE_SOURCE_SUCCESS != m_pFileSourceWrapper->GetStreamParameter((int64)mTrackId,
                                                                   FS_IndexParamOtherSeekInfo,
                                                                   &seekInfo)) {
              seekInfo.targetTimeUs = -1;
          }
        }
        // fall through
    }

    bool newBuffer = false;
    if (mBuffer == NULL) {
        newBuffer = true;
        status_t err = mBufferGroup->acquire_buffer(&mBuffer,
                              options != NULL && options->getNonBlocking());
        if (err != OK) {
            CHECK(mBuffer == NULL);
            return AMEDIA_ERROR_UNKNOWN;
        }
    }

    if (newBuffer && mBuffer) {
        size_t size = mBuffer->size();
        uint8_t *data = (uint8_t *)mBuffer->data();
        FileSourceSampleInfo sInfo = {};
        sp<ABuffer> setBuffSEI;
        int32 nSeiCount = 0;

        FileSourceMediaStatus status = m_pFileSourceWrapper->GetNextMediaSample(mTrackId, data, (uint32*)&size, sInfo);

        if (FILE_SOURCE_DATA_OK != status || 0 == size) {
            if (mBuffer != NULL) {
                mBuffer->release();
                mBuffer = NULL;
            }
            //! Sample Size ZERO is special case to indicate NAL corruption
            if (FILE_SOURCE_DATA_END == status || 0 == size) {    //Grab this as a special case
                LOGV("FileSource::FILE_SOURCE_DATA_END");
                return AMEDIA_ERROR_END_OF_STREAM;
            }
            return AMEDIA_ERROR_IO;
        }

        if (mFileFormat == FILE_SOURCE_MP2TS && mMinorType == FILE_SOURCE_MN_TYPE_H264) {
            uint32 ulIdx = 0;
            uint8_t *tempPtr = data;
            size_t dataSize = size;
            const uint8_t *nalStart;
            size_t nalSize;
            NALPosition max_sei[6];

            while ((ulIdx < size) &&
              getNextNALUnit((const uint8_t **)&(tempPtr),&dataSize,&nalStart,&nalSize,true) == OK) {
                // For SEI naltype is 0x06
                // currently kept support of  max number of sei's per fram is 6
                if ((nalStart[0] & 0x1f) == 0x06 && nSeiCount < 6) {
                    ulIdx = (uint32)(nalStart - data);
                    ulIdx += (uint32)nalSize;
                    max_sei[nSeiCount].nalSize = nalSize;
                    max_sei[nSeiCount].nalOffset = (size_t)(nalStart - data);
                    tempPtr = (data + ulIdx);
                    if (ulIdx < size)
                        dataSize = size - ulIdx;
                    nSeiCount++;
                }
            }//while(ulIdx<size)

            // Allocate and update SEI buffer if SEI NAL found
            if (nSeiCount > 0) {
                setBuffSEI = new ABuffer(nSeiCount * sizeof(NALPosition));
                for (int i = 0; i < nSeiCount; i++) {
                    NALPosition &seiPos = ((NALPosition *)setBuffSEI->data())[i];
                    seiPos.nalOffset = max_sei[i].nalOffset;
                    seiPos.nalSize = max_sei[i].nalSize;
                }//for(nSeiCount)
            }//if(nSeiCount)
        }

        //dumpToFile
        dumpToFile(mBuffer->data(), size);

        mBuffer->set_range(0, size);
        AMediaFormat *meta = mBuffer->meta_data();
        AMediaFormat_clear(meta);

        // set buffer on kKeySEI if SEI NAL found
        if (nSeiCount > 0)
          AMediaFormat_setBuffer(meta, AMEDIAFORMAT_KEY_SEI, setBuffSEI->data(), setBuffSEI->size());

        // here we are sending composition time instead of decode time
        // timestamp is in microsecond units
        uint64 ullTime = sInfo.startTime;
        AMediaFormat_setInt64(meta, AMEDIAFORMAT_KEY_TIME_US, ullTime);

        ++mCurrentSampleIndex;
        //! Timestamp is availale in micro-sec units
        mCurrentTimeMS = (sInfo.startTime / 1000);
        //! Set target time
        if (seekInfo.targetTimeUs >= 0) {
            AMediaFormat_setInt64(meta, AMEDIAFORMAT_KEY_TARGET_TIME, seekInfo.targetTimeUs);
        }
        //! This field is available only for OGG Files
        if (sInfo.nGranule) {
            AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_VALID_SAMPLES, sInfo.nGranule);
            LOGV("Granule value is set as %llu", sInfo.nGranule);
        }
        if (mFileFormat == FILE_SOURCE_MPEG4) {
            if (mMinorType == FILE_SOURCE_MN_TYPE_H264) {
                uint32_t layerId = FindAVCLayerId(
                    (const uint8_t *)mBuffer->data(), mBuffer->range_length());
                AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_TEMPORAL_LAYER_ID, layerId);
            }
        }
        LOGV("mCurrentSampleIndex = %d startTime = %lld Timescale = %d size = %zu",
             mCurrentSampleIndex, sInfo.startTime, mTimescale, size);
        if (sInfo.sync) {
            AMediaFormat_setInt32(meta, AMEDIAFORMAT_KEY_IS_SYNC_FRAME, 1);
        }

        if (sInfo.bIsExtraDataAvailable) {
            setBufferExtraMetaData(meta);
        }
    }

    *out = mBuffer;
    mBuffer = NULL;

    return AMEDIA_OK;
}

bool MMParserSource::supportsNonBlockingRead()
{
    if (mFileFormat == FILE_SOURCE_MPEG4)
      return true;
    return false;
}

void MMParserSource::setBufferExtraMetaData(AMediaFormat * meta)
{
    FS_MEDIA_INDEXTYPE index = FS_IndexParamExtraData;
    if (mFileFormat == FILE_SOURCE_MP2TS &&
        mMinorType == FILE_SOURCE_MN_TYPE_AC4) {
        index = FS_IndexParamExtraDataAudioPreselection;
    } else if ((mFileFormat == FILE_SOURCE_WEBM ||
                mFileFormat == FILE_SOURCE_MKV) &&
               mMinorType == FILE_SOURCE_MN_TYPE_VP9) {
        index = FS_IndexParamExtraDataHDR10PlusInfo;
    }

    if (index == FS_IndexParamExtraData)
        return;

    FileSourceStatus status =
        m_pFileSourceWrapper->GetStreamParameter(mTrackId, index, &mExtraData);
    if (FILE_SOURCE_SUCCESS != status || !mExtraData.ulDataLength)
        return;

    //! Ensure buffer size is enough and fetch again after allocate
    if (!mExtraData.pucData || mExtraData.ulMaxBufferSize < mExtraData.ulDataLength) {
        if (mExtraData.pucData != NULL) {
          free(mExtraData.pucData);
          mExtraData.pucData = NULL;
        }
        mExtraData.pucData = (uint8*)malloc(mExtraData.ulDataLength);
        if (!mExtraData.pucData)
            return;
        mExtraData.ulMaxBufferSize = mExtraData.ulDataLength;
        status = m_pFileSourceWrapper->GetStreamParameter(mTrackId, index, &mExtraData);
        if (FILE_SOURCE_SUCCESS != status)
            return;
    }

    uint8_t* buffer = mExtraData.pucData;
    int32_t size = mExtraData.ulDataLength;
    if (!buffer || !size)
        return;

    int32_t pos = 0;
    if (index == FS_IndexParamExtraDataAudioPreselection) {
        //! Core parsers have already validated data, but cross check here
        //! as we parse it again while fill AudioPresentationV1
        AudioPresentationCollection apList;
        if (size < 2 || buffer[pos++] != 0x19)
            return;

        int32_t numAudioPreSelections = (buffer[pos++] >> 3);
        size -= 2;
        for (int32_t i = 0; i < numAudioPreSelections; i++) {
            if (size < 2)
                return;
            AudioPresentationV1 ap;
            ap.mPresentationId = buffer[pos] >> 3;
            ap.mMasteringIndication =
                static_cast<MasteringIndication>((buffer[pos++] & 0x03));
            ap.mAudioDescriptionAvailable = (buffer[pos] & (1 << 7));
            ap.mSpokenSubtitlesAvailable = (buffer[pos] & (1 << 6));
            ap.mDialogueEnhancementAvailable = (buffer[pos] & (1 << 5));
            bool isLanguagePresent = (buffer[pos] & (1 << 3));
            bool isTextLabelPresent = (buffer[pos] & (1 << 2));
            bool isMultiStreamInfoPresent = (buffer[pos] & (1 << 1));
            bool isFutureExtension = (buffer[pos++] & 1);
            size -= 2;
            if (isLanguagePresent) {
                if (size < 3)
                    return;
                char language[4];
                language[0] = buffer[pos++];
                language[1] = buffer[pos++];
                language[2] = buffer[pos++];
                language[3] = '\0';
                ap.mLanguage = std::string(language);
                size -= 3;
            }
            if (isTextLabelPresent) {
                if (size < 1)
                    return;
                pos++;
                size--;
            }
            if (isMultiStreamInfoPresent) {
                if (size < 1)
                    return;
                int32_t dataLength = ((buffer[pos] >> 5) & 0x3) + 1;
                if (size < dataLength)
                    return;
                size -= dataLength;
                pos += dataLength;
            }
            if (isFutureExtension) {
                if (size < 1)
                    return;
                int32_t dataLength = ((buffer[pos]) & 0x1F) + 1;
                size -= dataLength;
                pos += dataLength;
            }
            apList.push_back(std::move(ap));
        }
        std::ostringstream outStream(std::ios::out);
        serializeAudioPresentations(apList, &outStream);
        AMediaFormat_setBuffer(meta, AMEDIAFORMAT_KEY_AUDIO_PRESENTATION_INFO,
                               outStream.str().data(), outStream.str().size());
    }
    if (index == FS_IndexParamExtraDataHDR10PlusInfo) {
        AMediaFormat_setBuffer(meta, "hdr10-plus-info", buffer, size);
    }
}

void MMParserSource::dumpToFile(void *data, size_t size)
{
    MMParserExtractor* extractor = mMMParserExtractor;
    if(!extractor->m_sDebug.bDumpToFile)
        return;
    if(mFile == NULL) {
        char pucFileName[PROPERTY_VALUE_MAX + 36] = {0};
        snprintf(pucFileName, PROPERTY_VALUE_MAX + 36, "%s_track_%d",
                 extractor->m_sDebug.pucFileName, mTrackId);
        mFile = fopen(pucFileName, "ab");
        if (mFile == NULL) {
            LOGE("ERROR - o/p file %s could NOT be opened", pucFileName);
            return;
        }
    }
    fwrite(data, size, 1, mFile);
}

}  // namespace android
