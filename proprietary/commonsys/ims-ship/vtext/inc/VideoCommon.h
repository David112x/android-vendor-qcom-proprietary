/**
============================
Copyright (c)  2016-2018  Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================
*/
#ifndef __VIDEOCOMMON_H
#define __VIDEOCOMMON_H

#define VIDEO_FRAME_RES_VGA              640 * 480
#define VIDEO_FRAME_RES_720P             720 * 1280

typedef enum eVIDEO_BUFFER_MODE {
  VIDEO_BUFFER_META_MODE = 0,
  VIDEO_BUFFER_NON_META_MODE,
} QPE_VIDEO_BUFFER_MODE;

typedef enum eCameraHAL {
  CAMERA_HAL1 = 1,
  CAMERA_HAL3 = 3,
} CAMERA_HAL_VERSION;

typedef enum {
  CODEC_IOMX = 0,
  CODEC_MEDIACODEC = 1,
} CODEC_TYPE_T;

/** enum eQPE_VIDEO_MSG, Specifies the various messages passed from video to
    the callback function */
typedef enum eVIDEO_MSG {
  /** Error Indication.*/
  VIDEO_MSG_ERROR = 0,
  /** Recorded data available.*/
  VIDEO_MSG_RECORDED_DATA = 1,
  /** Frame Played.*/
  VIDEO_MSG_DATA_PLAYED = 2,
  /** Resume Playing.*/
  VIDEO_MSG_RESUME_PLAYING = 3,
  /** Recorder Started. */
  VIDEO_MSG_RECORDER_STARTED = 4,
  /** Recorder Stopped. */
  VIDEO_MSG_RECORDER_STOPPED = 5,
  /** Player Started */
  VIDEO_MSG_PLAYER_STARTED = 6,
  /** Player Stopped */
  VIDEO_MSG_PLAYER_STOPPED = 7,
  /** Device is Initialised */
  VIDEO_MSG_DEV_INITIALISED = 8,
  /** Device is uninitialised */
  VIDEO_MSG_DEV_UNINITIALISED = 9,
  /** Resource for Saving locally is acquired */
  VIDEO_MSG_SAVING_RES_ACQUIRED = 10,
  /** Resource for Saving locally is released */
  VIDEO_MSG_SAVING_RES_RELEASED = 11,
  /** Saving started */
  VIDEO_MSG_SAVING_STARTED = 12,
  /** Saving stopped by user */
  VIDEO_MSG_SAVING_STOPPED_BY_USER = 13,
  /** Saving stopped due to storage area full */
  VIDEO_MSG_SAVING_STOPPED_STORAGE_FULL = 14,
  /** Saving stopped due to duration specified reached */
  VIDEO_MSG_SAVING_STOPPED_DURATION_REACHED = 15,
  /** Saving Error */
  VIDEO_MSG_SAVING_ERROR = 16,
  /** Decoded Far video frame */
  VIDEO_MSG_FAR_FRAME = 17,
  /** Decoded Near(Preview) video frame */
  VIDEO_MSG_NEAR_FRAME = 18,
  /** Video Codec changed */
  VIDEO_MSG_CODEC_CHANGED = 19,
  /** Player Reconfiguring ****/
  VIDEO_MSG_PLAYER_RECONFIGURE = 20,
  /**remove the video components */
  VIDEO_MSG_REMOVE_VIDEO_COMPONENTS = 21,
  /**Internal video state change event */
  VIDEO_MSG_INTERNAL_VIDEO_STATE_CHANGE = 22,
  /** CONFIGURATION Error */
  VIDEO_MSG_CONFIGURATION_ERROR = 23,
  /** communication Error */
  VIDEO_MSG_VIDEO_COMMUNICATION_ERROR = 24,
  /** LTR support info */
  VIDEO_MSG_LTR_SUPPRT_INFO,
  /**Internal video event */
  VIDEO_MSG_INTERNAL_EVENT_TRIGGER_RECORDER_HAL1 = 0xFD,
  /**Internal video event */
  VIDEO_MSG_INTERNAL_EVENT_TRIGGER_PLAYER = 0xFE,
  /**Internal video event */
  VIDEO_MSG_INTERNAL_EVENT_TRIGGER_RECORDER = 0xFF,
} QPE_VIDEO_MSG;

/* Structure specifying RTP data to be played.
   This will be part of MEDIA_PACKET_INFO enum. */
typedef struct stMediaPacketInfoRx {
  /** RTP sequence number*/
  uint32_t iSeqNum;
  /** Timestamp of the frame at source */
  uint32_t source_time_video_frame;
  /** RTP Timestamp of the frame at souce */
  uint32_t source_rtp_timestamp;
} MEDIA_PACKET_INFO_RX;

/* Structure specifying Encoded data frame.
   This will be part of MEDIA_PACKET_INFO enum. */
typedef struct stMediaPacketInfoTx {
  /** Flag to indicate whether system or Camera Timesource is used */
  int bSysTime;
  /** Frame count */
  uint32_t ulFrameSeqNum;
  /** LTR Frame Id */
  int32_t slLtrId;
  /** LTR Extra Data */
  uint32_t ulLtrExtraData;
} MEDIA_PACKET_INFO_TX;

typedef union MediaPacketInfo {
  /*This structure will be filled by dpl*/
  MEDIA_PACKET_INFO_TX sMediaPktInfoTx;
  /*This structure will be filled by core rtp*/
  MEDIA_PACKET_INFO_RX sMediaPktInfoRx;
} MEDIA_PACKET_INFO;

typedef struct {
  uint8_t index;
  int belongs_to_drop_set;
  uint32_t concealmb;
} FillBufferInfo;


typedef enum {
  eSurfaceStateNotReady,
  eSurfaceStateReady
} eSurfaceState;

typedef enum ePlayerMsgType {
  FILLBUFFER_INFO   = 0,
  RESOLUTION_UPDATE = 1,
  MOVE_TO_LOADED    = 2,
  MOVE_TO_EXECUTING = 3,
  PORT_DISABLE      = 4,
  PORT_ENABLE       = 5,
  CROP              = 6,
  CODEC_CONFIG_REQ  = 7,
} PlayerMsgType;

typedef enum eRecorderMsgType {
  ROTATION_CROP_PARAM_NOGPU  = 1,
  ROTATION_CROP_PARAM_GPU = 2,
  STOP_RECORDING  = 3,
  WAIT_FOR_BUFFERS = 4,
} RecorderMsgType;


/* This is the state machine to handle multiple requests */
typedef enum {
  VIDEO_INIT = 0,
  VIDEO_CODEC_PENDING,
  VIDEO_SURFACE_PENDING,
  VIDEO_SURFACE_RECEIVED,
  VIDEO_CODEC_CONFIGURED,
} eVIDEO_PLAYER_STATE;

typedef struct {
  void *pData;
  uint iLen;
  //QPE_VIDEO_DEV eDev;
} OMX_EVENTDATA;

typedef struct ePlayerResolution {
  uint32_t height;
  uint32_t width;
} PlayerResolution;


typedef struct PlayerMsg {
  PlayerMsgType type;
  union  {
    /* If this flag is set to true, player start command also triggered
       after codec Config request. */
    uint8_t           bPlayerStart;
    FillBufferInfo    fillbuffer;
    PlayerResolution  resolution;
  } data;
} PlayerInternalMsg;

typedef struct RecorderMsg {
  RecorderMsgType type;
  int *rotation;
  int *cameraFacing;
  int *cameraConfigHeight;
  int *cameraConfigWidth;
  int *cameraMount;
  int *rotationOffset;
} RecorderInternalMsg;


#define VIDEOOMX_MAX_NAL_LENGTH 100
#define VIDEOOMX_MAX_VOL_LENGTH 100
//MP4 VOL information
typedef struct {
  uint16_t volLength;
  uint8_t rawVol[VIDEOOMX_MAX_VOL_LENGTH];
} VIDEOOMXMP4VOLInfo;

//H264 NAL information
typedef struct {
  uint32_t nalLength;
  uint8_t rawnal[VIDEOOMX_MAX_NAL_LENGTH];
} VIDEOOMXH264NalInfo;

/* Structure specifying the data regarding encoded/decoded frames. */
typedef struct QpMultimediaFrame {
  /**Cvo Information**/
  uint8_t cvoInfo;
  /** Flag to indicate, whether frame needs to be dropped or not */
  int8_t iDropFrame;
  /** Pointer to the encoded data or the codec frame. */
  uint8_t *pData;
  /** Length of the encoded data or the codec frame. */
  uint32_t iDataLen;
  /** Length of the allocated data buffer. */
  uint32_t iMaxBuffLen;
  /** Frame Timestamp in micro sec */
  uint64_t ullFrameTimeUs;
  /** Last Updated System Time */
  uint64_t ullSysTime;
  /** MEDIA_PACKET_INFO structure. */
  MEDIA_PACKET_INFO sMediaPacketInfo;
} QP_MULTIMEDIA_FRAME;


#define DIR_INP 0
#define DIR_OUT 1
#define MAX_BUFFERS 50
/**
 * \def QP_TRUE
 * macro for TRUE value
 *
 * macro for TRUE value
 */
#define QP_TRUE  1L
/**
 * \def QP_FALSE
 * macro for FALSE value
 *
 * macro for FALSE value
 */
#define QP_FALSE 0L

typedef void *QP_VIDEO_DESC;

/** Callback to send various indications to Client of Player and Recorder */
typedef void (*QP_VIDEO_CALLBACK)(QPE_VIDEO_MSG tVideoMsg, void *pFrameData,
                                  uint32_t eDevType, int iStatus,
                                  void *pUserData);

/** enum eHEVC_PROFILE_TYPE, Specifies the profile type of H264 codec */

typedef enum eH265_PROFILE_TYPE {
  VIDEO_H265_PROFILE_MAIN = 0x00,
  VIDEO_H265_PROFILE_MAIN10 = 0x01
} QPE_VIDEO_H265_PROFILE_TYPE;

/** enum eAVC_PROFILE_LEVEL, Specifies the profile level of H264 codec */
typedef enum eH265_PROFILE_LEVEL {
  VIDEO_H265_LEVEL1 = 0x00,
  VIDEO_H265_LEVEL2 = 0x01,
  VIDEO_H265_LEVEL21 = 0x02,
  VIDEO_H265_LEVEL3 = 0x03,
  VIDEO_H265_LEVEL31 = 0x04,
  VIDEO_H265_LEVEL4 = 0x05,
  VIDEO_H265_LEVEL41 = 0x06,
  VIDEO_H265_LEVEL5 = 0x07,
  VIDEO_H265_LEVEL51 = 0x08,
  VIDEO_H265_LEVEL52 = 0x09,
  VIDEO_H265_LEVEL6 = 0x0A,
  VIDEO_H265_LEVEL61 = 0x0B,
  VIDEO_H265_LEVEL62 = 0x0C,
} QPE_VIDEO_H265_PROFILE_LEVEL;

/** enum eAVC_PROFILE_TYPE, Specifies the profile type of H264 codec */
typedef enum eAVC_PROFILE_TYPE {
  VIDEO_AVC_PROFILE_BASELINE = 0x00,
  VIDEO_AVC_PROFILE_MAIN = 0x01,
  VIDEO_AVC_PROFILE_EXTENDED = 0x02,
  VIDEO_AVC_PROFILE_HIGH = 0x03,
} QPE_VIDEO_AVC_PROFILE_TYPE;

/** enum eAVC_PROFILE_LEVEL, Specifies the profile level of H264 codec */
typedef enum eAVC_PROFILE_LEVEL {
  VIDEO_AVC_LEVEL1 = 0x00,
  VIDEO_AVC_LEVEL1B = 0x01,
  VIDEO_AVC_LEVEL11 = 0x02,
  VIDEO_AVC_LEVEL12 = 0x03,
  VIDEO_AVC_LEVEL13 = 0x04,
  VIDEO_AVC_LEVEL2 = 0x05,
  VIDEO_AVC_LEVEL21 = 0x06,
  VIDEO_AVC_LEVEL22 = 0x07,
  VIDEO_AVC_LEVEL3 = 0x08,
  VIDEO_AVC_LEVEL31 = 0x09,
  VIDEO_AVC_LEVEL32 = 0x0A,
  VIDEO_AVC_LEVEL4 = 0x0B,
  VIDEO_AVC_LEVEL41 = 0x0C,
  VIDEO_AVC_LEVEL42 = 0x0D,
  VIDEO_AVC_LEVEL5 = 0x0E,
  VIDEO_AVC_LEVEL51 = 0x0F,
} QPE_VIDEO_AVC_PROFILE_LEVEL;


/** enum eH263_PROFILE_TYPE, Specifies the profile type of H263 codec */
typedef enum eH263_PROFILE_TYPE {
  VIDEO_H263_PROFILE_BASELINE = 0x00,
  VIDEO_H263_PROFILE_H320_CODING = 0x01,
  VIDEO_H263_PROFILE_BACKWARD_COMPATIBLE = 0x02,
  VIDEO_H263_PROFILE_ISWV2 = 0x03,
  VIDEO_H263_PROFILE_ISWV3 = 0x04,
  VIDEO_H263_PROFILE_HIGH_COMPRESSION = 0x05,
  VIDEO_H263_PROFILE_INTERNET = 0x06,
  VIDEO_H263_PROFILE_INTERLACE = 0x07,
  VIDEO_H263_PROFILE_HIGH_LATENCY = 0x08,
} QPE_VIDEO_H263_PROFILE_TYPE;

/** enum eH263_PROFILE_LEVEL, Specifies the profile level of H263 codec */
typedef enum eH263_PROFILE_LEVEL {
  VIDEO_H263_LEVEL10 = 0x00,
  VIDEO_H263_LEVEL20 = 0x01,
  VIDEO_H263_LEVEL30 = 0x02,
  VIDEO_H263_LEVEL40 = 0x03,
  VIDEO_H263_LEVEL50 = 0x04,
  VIDEO_H263_LEVEL60 = 0x05,
  VIDEO_H263_LEVEL70 = 0x06,
  VIDEO_H263_LEVEL45 = 0x07,
} QPE_VIDEO_H263_PROFILE_LEVEL;

typedef enum eVIDEO_PKTN_MODE {
  /** Single NAL unit mode */
  VIDEO_CODEC_SINGLE_NALU = 0,
  /** Non-interleaved mode */
  VIDEO_CODEC_NON_INTERLEAVED = 1,
  /** Interleaved mode */
  VIDEO_CODEC_INTERLEAVED = 2
} QPE_VIDEO_PKTN_MODE;

/** enum eVIDEO_CODEC, Specifies the various video codecs available. */
typedef enum eVIDEO_CODEC {
  /** XVID MPEG4 Codec.*/
  VIDEO_CODEC_MPEG4_XVID,
  /** ISO MPEG4 Codec.*/
  VIDEO_CODEC_MPEG4_ISO,
  /** H.263 Codec.*/
  VIDEO_CODEC_H263,
  /** H.264 Codec.*/
  VIDEO_CODEC_H264,
  /** H.265 Codec */
  VIDEO_CODEC_H265
} QPE_VIDEO_CODEC;

typedef enum eVIDEO_DEV {
  /** PLAYER */
  VIDEO_PLAYER,
  /** RECORDER */
  VIDEO_RECORDER
} QPE_VIDEO_DEV;

/* Client can provide bitrate config mode. These values will map to different
   ENUMs in Video RC algorithm. Comment on each Enum will indicate that.
   For VT usecases, VBR_CFR or CBR_VFR are recommended. By default CBR_VFR is
   choosen for VT usecase. */
typedef enum {
  BITRATE_UNKNOWN = 0,
  /*V4L2_CID_MPEG_VIDC_VIDEO_RATE_CONTROL_VBR_CFR = 2*/
  BITRATE_MODE_VBR_CFR = 1,
  /*V4L2_CID_MPEG_VIDC_VIDEO_RATE_CONTROL_CBR_CFR = 4*/
  BITRATE_MODE_CBR_CFR = 2,
  /*V4L2_CID_MPEG_VIDC_VIDEO_RATE_CONTROL_VBR_VFR = 1*/
  BITRATE_MODE_VBR_VFR = 3,
  /*V4L2_CID_MPEG_VIDC_VIDEO_RATE_CONTROL_CBR_VFR = 3*/
  BITRATE_MODE_CBR_VFR = 4,
  /*V4L2_CID_MPEG_VIDC_VIDEO_RATE_CONTROL_CBR_VFR = 4*/
  BITRATE_MODE_CBR_PLUS_VFR = 5,
} BIT_RATE_MODE;

/** struct QpVideoConfig, Structure specifying the various mandatory video
    configuration parameters */
typedef struct QpVideoConfig {
  /**device to be uased */
  QPE_VIDEO_DEV eVideoDevice;
  /** Codec that needs to be used.*/
  QPE_VIDEO_CODEC eCodec;
  /** Width of the video capture */
  int32_t iWidth;
  /** Height of the video capture */
  int32_t iHeight;
  /** Output BitRate expected from the encoder in kbps */
  int32_t iBitRate;
  /** Frame Rate of the capture in frames/sec */
  int32_t iFrameRate;
  /** Buffer containing VOL header */
  char *pVolHeader;
  /** Length of the VOL header */
  uint16_t iVolHeaderLen;
  /** Buffer containing NAL header */
  char *pNALHeader;
  /** Length of the NAL header */
  uint16_t iNALHeaderLen;
  /** Sampling rate */
  uint32_t iClockRate;
  /** H264 Profile type */
  QPE_VIDEO_AVC_PROFILE_TYPE eH264ProfileType;
  /** H264 Profile level */
  QPE_VIDEO_AVC_PROFILE_LEVEL eH264ProfileLevel;
  /** H263 Profile type */
  QPE_VIDEO_H263_PROFILE_TYPE eH263ProfileType;
  /** H263 Profile level */
  QPE_VIDEO_H263_PROFILE_LEVEL eH263ProfileLevel;

  /** Packetization Mode type */
  QPE_VIDEO_PKTN_MODE ePktnMode;

  /** H265 Profile type */
  QPE_VIDEO_H265_PROFILE_TYPE eH265ProfileType;
  /** H265 Profile level */
  QPE_VIDEO_H265_PROFILE_LEVEL eH265ProfileLevel;

  /** RTP MTU Size */
  uint16_t iRtpMTUSize;
  /* iFrameInterval based on AVP or AVPF*/
  int32_t  iFrameInterval;
  /** Width of the video capture */
  int32_t iOut_Width;
  /** Height of the video capture */
  int32_t iOut_Height;
  /** internal codec configure  */
  int iInternalCodecConfig;
  /**rotation parameter */
  uint16_t rotation;
  /**GPU Rotation width*/
  uint32_t cameraConfigWidth;
  /**GPU Rotation height*/
  uint32_t cameraConfigHeight;
  uint8_t iErrorState;
  uint16_t iEnableFileStreaming;
  uint16_t iDisplayMode;
  int iCropSet;
  /**Camera HAL Version */
  CAMERA_HAL_VERSION eHALVersion;
  int iPortReconfigSet;
  /**mode parameter*/
  QPE_VIDEO_BUFFER_MODE iInputVideoBufferMode;
  /** LTR feature enabled or not. */
  uint8_t iLTRSupportEnabled;
  /** rate control Mode */
  BIT_RATE_MODE eBitRateMode;
  /** Session Id */
  uint16_t iSessionId;
  uint8_t  iNotifyUI;
  uint8_t bGPURotationSupport;
} QP_VIDEO_CONFIG;

typedef enum eVideoState {
  eVideoStateNotReady, //fresh initialized of codec creation
  eVideoStateAcquiring,
  eVideoStateIdle, //idle means configured state and surface is configured for Recorder
  eVideoStateStarting,
  eVideoStateActive, //executing or started state
  eVideoStateStopping,
  eVideoStateReleasing,
  eVideoStateReConfiguring,
  eVideoStateReConfigureDone,
  eVideoStatePortDisabling,
  eVideoStatePortDisabled,
  eVideoStatePaused,
  eVideoStateCropping,
  eVideoStateSurfaceReleased,
  eVideoStateSurfaceReleasing
} QP_VIDEO_STATE;

typedef enum eVIDEO_ERROR {
  /** No Error OR Success */
  VIDEO_ERROR_OK = 0,
  /** Unknown Error */
  VIDEO_ERROR_UNKNOWN = -1,
  /** Recorder could not be started */
  VIDEO_ERROR_RECORDER_DOWN = 1,
  /** Player could not be started */
  VIDEO_ERROR_PLAYER_DOWN = 2,
  /** Buffer overflow in Recorder or Player */
  VIDEO_ERROR_BUFFER_OVERFLOW = 3,
  /** Buffer underflow in Recorder or Player */
  VIDEO_ERROR_BUFFER_UNDERFLOW = 4,
  /** Recorder busy means recorder has already been started */
  VIDEO_ERROR_RECORDER_BUSY = 5,
  /** Player busy means player has already been started */
  VIDEO_ERROR_PLAYER_BUSY = 6
} QPE_VIDEO_ERROR;


typedef enum {
  CVO_ZERO_ROTATION = 0x00,
  CVO_90_ROTATION = 0x01,
  CVO_180_ROTATION = 0x02,
  CVO_270_ROTATION = 0x03,

  CVO_FLIP_DIABLED = 0x00,
  CVO_FLIP_ENABLED = 0x04,
  CVO_BACK_CAMERA  = 0x08,
  CVO_INVALID_TYPE = 0xFF
} CVO_ROTATION_TYPE;

typedef enum {
  SUCCESS = 0,
  FAILURE = 1
} CALLBACK_STATUS;

#endif
