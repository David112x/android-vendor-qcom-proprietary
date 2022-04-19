/**********************************
 ============================
Copyright (c)  2004-2012,2017-2018  Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================
**********************************/
/* qplog.h
* \brief Macros and functions for log messages.
*
* Defines IMS VT utility functions
* defines log level.
*/

#if !defined(__QPLOG_H)
#define __QPLOG_H

#include <cutils/log.h>
#include <cutils/properties.h>
#include <semaphore.h>
#include <stdlib.h>
#include "msg.h"
#undef LOG_TAG
#define LOG_TAG "VTLib"
#define LOGE ALOGE
#define LOGD ALOGD
#define LOGW ALOGW
#define LOGI ALOGI
//! IDR frame retransmission logic is enabled if IDR interval is more than
//! 6sec.
#define IDR_RETRANS_INTERVAL (6)

//! In AVPF mode, do not use periodic IDR frames.
//! Only on demand IDR frames will be generated
#define AVPF_FRAME_INTERVAL (-1)
#define AVP_FRAME_INTERVAL 1
#define ASCII_ZERO_VALUE 48

extern uint16 logPriorityValue;
extern uint16 adbLogPropertyValue;
#define VT_ADB_LOGS        (1)

extern boolean bInit_Success;
extern pthread_mutex_t log_write_mutex;

/* To maintain the image share state */
static const int IMAGE_SEND_ACTIVE = 1;
static const int IMAGE_SEND_INACTIVE = 0;

#define UNIQUE_FILE_NO 0

// Macro to be used on the unused variables to fix the warnings
#define QP_UNUSED(v) if((v))\
                     {\
                     }

#define LOG_IMS_DPL "LOG_IMS_DPL"
#define QP_EVT_BASE         0

/** QPLP_NONE, Lowest priority of log event. */
#define QPLP_NONE           0
/** QPLP_TRACE, Log priority for traces. */
#define QPLP_TRACE          1
/** QPLP_DEBUG, Log priority for DEBUG statements. */
#define QPLP_DEBUG          2
/** QPLP_INFO, Log priority for INFO to user. */
#define QPLP_INFO           QPLP_TRACE
/** QPLP_WARNING, Log priority for warning. */
#define QPLP_WARNING        QPLP_TRACE
/** QPLP_EVENT, Log priority for Events. */
#define QPLP_EVENT          4
/** QPLP_FATAL, Log priority for fatal errors. */
#define QPLP_FATAL          8
/** QPLP_CRITICAL, Log priority for critical messages. */
#define QPLP_CRITICAL       16
/** QPLP_CONTROLLOGS, Log priority for CRITICAL + file redirection */
#define QPLP_CONTROLLOGS    16
/** QPLP_MINIMAL, Log priority for minimal logging messages. */
#define QPLP_MINIMAL        32
/** QPLP_ERROR, Log priority for errors. */
#define QPLP_ERROR          64
/** QPLP_LOGPACKET, Log priority for log packet messages. */
#define QPLP_LOGPACKET      128

#ifdef __cplusplus
extern "C" {
#endif

/** QpLogEvent, This structure is passed to qpLogEvent to process a log. */
struct QpLogEvent {
  /** priority of this log event*/
  int priority;
  /** filename in which this log is generated */
  char* fileName;
  /** line number where this log is generated */
  int lineNum;
  /** format string in printf format */
  char* fmt;
  /** pointer to list of arguments */
  va_list ap;
};

typedef struct __attribute__((packed))
{
  /*Log packet version; range is 0 to 255*/
  unsigned char   version;
  uint64_t sys_time_video_frame_input;
  uint64_t sys_time_video_frame_output;
  uint32_t ulFrameSize;
  unsigned char  ucFrameNalType;
}
QP_VIDEO_ENCODER_INFO;

typedef struct __attribute__((packed))
{
  /*Log packet version; range is 0 to 255*/
  unsigned char   version;
  uint64_t  source_time_video_frame;
  uint64_t  system_time_decode_video_frame;
  uint64_t  system_time_render_video_frame;
  uint64_t  source_rtp_timestamp;
  uint32_t  ulFrameSize;
  uint8_t   ucFrameNalType;
}
QP_VIDEO_PLAYOUT_INFO;

#define str_cat1(str1,str2) #str1 " " #str2
#define str_cat(str1,str2) str_cat1(str1,str2)

/* logPriorityValue --> This is property controlled by user. Depending on how much level of logging is needed.
    This value will be set.
    priority --> The level of a particular log message. It will be compared against user set priority and deicded whether to print or ignore.
    adbLogPropertyValue --> This property decides whether adb logging enabled or not.
    bInit_Success --> This flag is to true only if diag initialization is successful. Then only we will get QXDM logs.
Call Diag API only if diag initialization is successful (bInit_Success) */

#define IMS_COMMON1(priority,f) \
        if(priority & logPriorityValue) {\
          if (bInit_Success) {\
            pthread_mutex_lock(&log_write_mutex);\
            MSG_2(MSG_SSID_IMS, MSG_LEGACY_HIGH,str_cat([SIMSVT] %d %d ,f), \
              getpid(),gettid());\
            pthread_mutex_unlock(&log_write_mutex);}\
          if (adbLogPropertyValue)\
            __android_log_print(ANDROID_LOG_ERROR, "SIMSVT", f); }
#define IMS_COMMON2(priority,f,a1) \
        if(priority & logPriorityValue) {\
          if (bInit_Success) {\
            pthread_mutex_lock(&log_write_mutex);\
            MSG_3(MSG_SSID_IMS, MSG_LEGACY_HIGH,str_cat([SIMSVT] %d %d ,f), \
              getpid(),gettid(),(intptr_t)a1);\
            pthread_mutex_unlock(&log_write_mutex);}\
          if (adbLogPropertyValue)\
            __android_log_print(ANDROID_LOG_ERROR, "SIMSVT", f,a1); }
#define IMS_COMMON3(priority,f,a1,a2) \
        if(priority & logPriorityValue) {\
          if (bInit_Success) {\
            pthread_mutex_lock(&log_write_mutex);\
            MSG_4(MSG_SSID_IMS, MSG_LEGACY_HIGH,str_cat([SIMSVT] %d %d ,f), \
              getpid(),gettid(),(intptr_t)a1,(intptr_t)a2);\
            pthread_mutex_unlock(&log_write_mutex);}\
          if (adbLogPropertyValue)\
            __android_log_print(ANDROID_LOG_ERROR, "SIMSVT", f,a1,a2); }
#define IMS_COMMON4(priority,f,a1,a2,a3) \
        if(priority & logPriorityValue) {\
          if (bInit_Success) {\
            pthread_mutex_lock(&log_write_mutex);\
            MSG_5(MSG_SSID_IMS, MSG_LEGACY_HIGH,str_cat([SIMSVT] %d %d ,f), \
              getpid(),gettid(),(intptr_t)a1,(intptr_t)a2,(intptr_t)a3);\
            pthread_mutex_unlock(&log_write_mutex);}\
          if (adbLogPropertyValue)\
            __android_log_print(ANDROID_LOG_ERROR, "SIMSVT", f,a1,a2,a3); }
#define IMS_COMMON5(priority,f,a1,a2,a3,a4) \
        if(priority & logPriorityValue) {\
          if (bInit_Success) {\
            pthread_mutex_lock(&log_write_mutex);\
            MSG_6(MSG_SSID_IMS, MSG_LEGACY_HIGH,str_cat([SIMSVT] %d %d ,f), \
              getpid(),gettid(),a1,a2,a3,a4);\
            pthread_mutex_unlock(&log_write_mutex);}\
          if (adbLogPropertyValue)\
            __android_log_print(ANDROID_LOG_ERROR, "SIMSVT", f,a1,a2,a3,a4);}
#define IMS_COMMON6(priority,f,a1,a2,a3,a4,a5) \
        if(priority & logPriorityValue) {\
          if (bInit_Success) {\
            pthread_mutex_lock(&log_write_mutex);\
            MSG_7(MSG_SSID_IMS, MSG_LEGACY_HIGH,str_cat([SIMSVT] %d %d ,f), \
              getpid(),gettid(),a1,a2,a3,a4,a5);\
            pthread_mutex_unlock(&log_write_mutex);}\
          if (adbLogPropertyValue)\
            __android_log_print(ANDROID_LOG_ERROR, "SIMSVT", f,a1,a2,a3,a4,a5); }


#define IMS_COMMON1_STR(priority,f) \
        if(priority & logPriorityValue) {\
          if (bInit_Success) {\
            pthread_mutex_lock(&log_write_mutex);\
            MSG_SPRINTF_2(MSG_SSID_IMS, MSG_LEGACY_HIGH,str_cat([SIMSVT] %d %d ,f), getpid(),gettid());\
            pthread_mutex_unlock(&log_write_mutex);}\
            if (adbLogPropertyValue && QPLP_MINIMAL != priority)\
              __android_log_print(ANDROID_LOG_ERROR, "SIMSVT", f); }

#define IMS_COMMON2_STR(priority,f,a1) \
        if(priority & logPriorityValue) {\
          if (bInit_Success) {\
            pthread_mutex_lock(&log_write_mutex);\
            MSG_SPRINTF_3(MSG_SSID_IMS, MSG_LEGACY_HIGH,str_cat([SIMSVT] %d %d ,f), getpid(),gettid(),a1);\
            pthread_mutex_unlock(&log_write_mutex);}\
          if (adbLogPropertyValue && QPLP_MINIMAL != priority)\
            __android_log_print(ANDROID_LOG_ERROR, "SIMSVT", f,a1); }
#define IMS_COMMON3_STR(priority,f,a1,a2) \
        if(priority & logPriorityValue) {\
          if (bInit_Success) {\
            pthread_mutex_lock(&log_write_mutex);\
            MSG_SPRINTF_4(MSG_SSID_IMS, MSG_LEGACY_HIGH,str_cat([SIMSVT] %d %d ,f), getpid(),gettid(),a1,a2);\
            pthread_mutex_unlock(&log_write_mutex);}\
          if (adbLogPropertyValue && QPLP_MINIMAL != priority)\
            __android_log_print(ANDROID_LOG_ERROR, "SIMSVT", f,a1,a2); }
#define IMS_COMMON4_STR(priority,f,a1,a2,a3) \
        if(priority & logPriorityValue) {\
          if (bInit_Success) {\
            pthread_mutex_lock(&log_write_mutex);\
            MSG_SPRINTF_5(MSG_SSID_IMS, MSG_LEGACY_HIGH,str_cat([SIMSVT] %d %d ,f), \
              getpid(),gettid(),a1,a2,a3);\
            pthread_mutex_unlock(&log_write_mutex);}\
          if (adbLogPropertyValue && QPLP_MINIMAL != priority)\
            __android_log_print(ANDROID_LOG_ERROR, "SIMSVT", f,a1,a2,a3); }
#define IMS_COMMON5_STR(priority,f,a1,a2,a3,a4) \
        if(priority & logPriorityValue) {\
          if (bInit_Success) {\
            pthread_mutex_lock(&log_write_mutex);\
            MSG_SPRINTF_6(MSG_SSID_IMS, MSG_LEGACY_HIGH,str_cat([SIMSVT] %d %d ,f), \
              getpid(),gettid(),a1,a2,a3,a4);\
            pthread_mutex_unlock(&log_write_mutex);}\
          if (adbLogPropertyValue && QPLP_MINIMAL != priority)\
            __android_log_print(ANDROID_LOG_ERROR, "SIMSVT", f,a1,a2,a3,a4); }

#define IMS_COMMON6_STR(priority,f,a1,a2,a3,a4,a5) \
        if(priority & logPriorityValue) {\
          if (bInit_Success) {\
            pthread_mutex_lock(&log_write_mutex);\
            MSG_SPRINTF_7(MSG_SSID_IMS, MSG_LEGACY_HIGH,str_cat([SIMSVT] %d %d ,f), \
              getpid(),gettid(),a1,a2,a3,a4,a5);\
            pthread_mutex_unlock(&log_write_mutex);}\
          if (adbLogPropertyValue && QPLP_MINIMAL != priority)\
            __android_log_print(ANDROID_LOG_ERROR, "SIMSVT", f,a1,a2,a3,a4,a5); }

#define LOG_1X_BASE_C (0x1000)
#define LOG_IMS_RTP_PKT_LOSS_C                   ((0x569) + LOG_1X_BASE_C)
#define LOG_IMS_VDJ_ENQUEUE_C                    ((0x5CB) + LOG_1X_BASE_C)
#define LOG_IMS_VDJ_DEQUEUE_C                    ((0x5CC) + LOG_1X_BASE_C)
#define LOG_IMS_CAMERA_C                         ((0x751) + LOG_1X_BASE_C)
#define LOG_IMS_VIDEO_ENCODER_C                  ((0x752) + LOG_1X_BASE_C)
#define LOG_VIDEO_PLAYOUT_C                      ((0x753) + LOG_1X_BASE_C)
#define LOG_VIDEO_CODEC_INIT_C                   (0x9A2)
#define LOG_VIDEO_CODEC_SET_C                    (0x9A3)


#define QP_IMPORT_C
#define QP_EXPORT_C
QP_IMPORT_C int sem_timedwait_videoplay();
QP_IMPORT_C void sem_post_videoplay();
QP_IMPORT_C void sem_init_videoplay();

QP_IMPORT_C void sem_destroy_videoplay();
QP_IMPORT_C uint64_t getCurrentSystemTime(struct timespec* ptr);
QP_IMPORT_C void qpLogEvent(struct QpLogEvent*ev, ...);
QP_IMPORT_C void qpLogInit(int priority);
QP_IMPORT_C void qpLogDeInit(void);
QP_IMPORT_C void qpLogEventSimple(int priority, char* fileName, int lineNum,
                                  const char* fmt, ...);
QP_IMPORT_C void recorder_mutex_lock();
QP_IMPORT_C void recorder_mutex_unlock();
QP_IMPORT_C int recorder_mutex_timedlock(uint64_t ts);
int mutex_timedlock(pthread_mutex_t* pMutex, uint64_t ts);
QP_IMPORT_C uint8_t skipFrame(uint64_t currenttstamp, uint64_t previouststamp,
                              uint32_t framerate);

QP_IMPORT_C void qpLogPacket(int priority, uint16_t logcode, void *buf,
                             uint32_t length);

QP_IMPORT_C void setupEGL();
QP_IMPORT_C void destroyEGL();
QP_IMPORT_C int  rotateFrame(uint32_t srcwidth, uint32_t srcheight,
                             uint32_t dstheight,
                             uint32_t dstwidth, int rotation, void* srcbuffer, void* dstbuffer);
QP_IMPORT_C void setupShader();
QP_IMPORT_C int getImageSendStatus();
QP_IMPORT_C void setImageSendStatus(int state);
QP_IMPORT_C int videoPlaybackDecider(int dropsetvalue, int concealmbvalue,
                                     int height, int width);

#ifdef __cplusplus
}
#endif

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#define QPLP_UNINITIALIZED  -1

#ifndef QPLOG_STATIC_THRESHOLD
#define QPLOG_STATIC_THRESHOLD QPLP_NONE
#endif

#define QPLOG_ISENABLED(priority) \
       (priority >= QPLOG_STATIC_THRESHOLD)

#ifndef USE_SIMPLE_LOGGING
#define _QPLOG_ST(priority, fmt) {              \
         struct QpLogEvent _log_ev =                 \
             {priority, __FILE__, __LINE__, fmt};  \
         qpLogEvent(&_log_ev
#define _QPLOG_ED                                  \
                        );                         \
      }
#else //USE_SIMPLE_LOGGING
#define _QPLOG_ST(priority, fmt)          \
      if(priority & logPriorityValue) {qpLogEventSimple(priority, (char*)__FILE__, (int)__LINE__,fmt

#define _QPLOG_ED  );}
#endif

/* Pass three arguments for LOGPACKET
* a1 : logcode
* a2 : log buffer
* a3 : log buffer length
*/
#define LOGPACKET(a1,a2,a3)              qpLogPacket(QPLP_LOGPACKET,a1,a2,a3)

#if (QPLOG_STATIC_THRESHOLD <= QPLP_TRACE)
#define TRACE1(f)                       IMS_COMMON1(QPLP_TRACE,f)
#define TRACE2(f,a1)                    IMS_COMMON2(QPLP_TRACE,f,a1)
#define TRACE3(f,a1,a2)                 IMS_COMMON3(QPLP_TRACE,f,a1,a2)
#define TRACE4(f,a1,a2,a3)              IMS_COMMON4(QPLP_TRACE,f,a1,a2,a3)
#define TRACE5(f,a1,a2,a3,a4)           IMS_COMMON5(QPLP_TRACE,f,a1,a2,a3,a4)
#define TRACE6(f,a1,a2,a3,a4,a5)        IMS_COMMON6(QPLP_TRACE,f,a1,a2,a3,a4,a5)
//#define TRACE7(f,a1,a2,a3,a4,a5,a6)     _QPLOG_ST(QPLP_TRACE, f) ,a1,a2,a3,a4,a5,a6 _QPLOG_ED
#else
#define TRACE1(f)                       {}
#define TRACE2(f,a1)                    {}
#define TRACE3(f,a1,a2)                 {}
#define TRACE4(f,a1,a2,a3)              {}
#define TRACE5(f,a1,a2,a3,a4)           {}
#define TRACE6(f,a1,a2,a3,a4,a5)        {}
//#define TRACE7(f,a1,a2,a3,a4,a5,a6)     {}
#endif

#if QPLOG_STATIC_THRESHOLD <= QPLP_DEBUG
#define DEBUG1_STR(f)                       IMS_COMMON1_STR(QPLP_DEBUG,f)
#define DEBUG2_STR(f,a1)                    IMS_COMMON2_STR(QPLP_DEBUG,f,a1)
#define DEBUG3_STR(f,a1,a2)                 IMS_COMMON3_STR(QPLP_DEBUG,f,a1,a2)
#define DEBUG4_STR(f,a1,a2,a3)              IMS_COMMON4_STR(QPLP_DEBUG,f,a1,a2,a3)
#define DEBUG5_STR(f,a1,a2,a3,a4)           IMS_COMMON5_STR(QPLP_DEBUG,f,a1,a2,a3,a4)
#define DEBUG6_STR(f,a1,a2,a3,a4,a5)        IMS_COMMON6_STR(QPLP_DEBUG,f,a1,a2,a3,a4,a5)
#endif

#if QPLOG_STATIC_THRESHOLD <= QPLP_DEBUG
#define DEBUG1(f)                       IMS_COMMON1(QPLP_DEBUG,f)
#define DEBUG2(f,a1)                    IMS_COMMON2(QPLP_DEBUG,f,a1)
#define DEBUG3(f,a1,a2)                 IMS_COMMON3(QPLP_DEBUG,f,a1,a2)
#define DEBUG4(f,a1,a2,a3)              IMS_COMMON4(QPLP_DEBUG,f,a1,a2,a3)
#define DEBUG5(f,a1,a2,a3,a4)           IMS_COMMON5(QPLP_DEBUG,f,a1,a2,a3,a4)
#define DEBUG6(f,a1,a2,a3,a4,a5)        IMS_COMMON6(QPLP_DEBUG,f,a1,a2,a3,a4,a5)
//#define DEBUG7(f,a1,a2,a3,a4,a5,a6)     _QPLOG_ST(QPLP_DEBUG, f) ,a1,a2,a3,a4,a5,a6 _QPLOG_ED
#else
#define DEBUG1(f)                       {}
#define DEBUG2(f,a1)                    {}
#define DEBUG3(f,a1,a2)                 {}
#define DEBUG4(f,a1,a2,a3)              {}
#define DEBUG5(f,a1,a2,a3,a4)           {}
#define DEBUG6(f,a1,a2,a3,a4,a5)        {}
#define DEBUG7(f,a1,a2,a3,a4,a5,a6)     {}
#endif

/* ERROR logs has to come in both adb and QXDM. So this handling is done separately. To avoid multi-line macro,
    if(1) condition was added. QXDM logging is checked whether diag init is completed or not. */
#if QPLOG_STATIC_THRESHOLD <= QPLP_ERROR
#define ERROR1(f)                 if(1) {\
                                  __android_log_print(ANDROID_LOG_ERROR, "SIMSVT", f);\
                                  if(bInit_Success) {\
                                    pthread_mutex_lock(&log_write_mutex);\
                                    MSG_SPRINTF_2(MSG_SSID_IMS, MSG_LEGACY_ERROR,str_cat([SIMSVT] %d %d ,f), \
                                      getpid(),gettid());\
                                    pthread_mutex_unlock(&log_write_mutex); }  }
#define ERROR2(f,a1)              if(1) {\
                                  __android_log_print(ANDROID_LOG_ERROR, "SIMSVT", f,a1);\
                                  if(bInit_Success) {\
                                    pthread_mutex_lock(&log_write_mutex);\
                                    MSG_SPRINTF_3(MSG_SSID_IMS, MSG_LEGACY_ERROR,str_cat([SIMSVT] %d %d ,f), \
                                      getpid(),gettid(),a1);\
                                    pthread_mutex_unlock(&log_write_mutex); } }
#define ERROR3(f,a1,a2)           if(1) {\
                                  __android_log_print(ANDROID_LOG_ERROR, "SIMSVT", f,a1,a2);\
                                  if(bInit_Success) {\
                                    pthread_mutex_lock(&log_write_mutex);\
                                    MSG_SPRINTF_4(MSG_SSID_IMS, MSG_LEGACY_ERROR,str_cat([SIMSVT] %d %d ,f), \
                                      getpid(),gettid(),a1,a2);\
                                    pthread_mutex_unlock(&log_write_mutex); } }
#define ERROR4(f,a1,a2,a3)        if(1) {\
                                  __android_log_print(ANDROID_LOG_ERROR, "SIMSVT", f,a1,a2,a3);\
                                  if(bInit_Success) {\
                                    pthread_mutex_lock(&log_write_mutex);\
                                    MSG_SPRINTF_5(MSG_SSID_IMS, MSG_LEGACY_ERROR,str_cat([SIMSVT] %d %d ,f), \
                                      getpid(),gettid(),a1,a2,a3);\
                                    pthread_mutex_unlock(&log_write_mutex); } }
#define ERROR5(f,a1,a2,a3,a4)     if(1) {\
                                  __android_log_print(ANDROID_LOG_ERROR, "SIMSVT", f,a1,a2,a3,a4);\
                                  if(bInit_Success) {\
                                    pthread_mutex_lock(&log_write_mutex);\
                                    MSG_SPRINTF_6(MSG_SSID_IMS, MSG_LEGACY_ERROR,str_cat([SIMSVT] %d %d ,f), \
                                      getpid(),gettid(),a1,a2,a3,a4);\
                                    pthread_mutex_unlock(&log_write_mutex); } }
#define ERROR6(f,a1,a2,a3,a4,a5)  if(1) {\
                                  __android_log_print(ANDROID_LOG_ERROR, "SIMSVT", f,a1,a2,a3,a4,a5);\
                                  if(bInit_Success) {\
                                    pthread_mutex_lock(&log_write_mutex);\
                                    MSG_SPRINTF_7(MSG_SSID_IMS, MSG_LEGACY_ERROR,str_cat([SIMSVT] %d %d ,f), \
                                      getpid(),gettid(),a1,a2,a3,a4,a5); \
                                    pthread_mutex_unlock(&log_write_mutex); } }
//#define ERROR7(f,a1,a2,a3,a4,a5,a6)
#else
#define ERROR1(f)                       {}
#define ERROR2(f,a1)                    {}
#define ERROR3(f,a1,a2)                 {}
#define ERROR4(f,a1,a2,a3)              {}
#define ERROR5(f,a1,a2,a3,a4)           {}
#define ERROR6(f,a1,a2,a3,a4,a5)        {}
//#define ERROR7(f,a1,a2,a3,a4,a5,a6)     {}
#endif

#if QPLOG_STATIC_THRESHOLD <= QPLP_EVENT
#ifdef QPLOG_REDUCE_SIZE
#define EVENT1(f)                       _QPLOG_ST(QPLP_EVENT, "") _QPLOG_ED
#else
#define EVENT1(f)                       IMS_COMMON1(QPLP_EVENT,f)
#endif
#define EVENT2(f,a1)                    IMS_COMMON2(QPLP_EVENT,f,a1)
#define EVENT3(f,a1,a2)                 IMS_COMMON3(QPLP_EVENT,f,a1,a2)
#define EVENT4(f,a1,a2,a3)              IMS_COMMON4(QPLP_EVENT,f,a1,a2,a3)
#define EVENT5(f,a1,a2,a3,a4)           IMS_COMMON5(QPLP_EVENT,f,a1,a2,a3,a4)
#define EVENT6(f,a1,a2,a3,a4,a5)        IMS_COMMON6(QPLP_EVENT,f,a1,a2,a3,a4,a5)
//#define EVENT7(f,a1,a2,a3,a4,a5,a6)
#else
#define EVENT1(f)                       {}
#define EVENT2(f,a1)                    {}
#define EVENT3(f,a1,a2)                 {}
#define EVENT4(f,a1,a2,a3)              {}
#define EVENT5(f,a1,a2,a3,a4)           {}
#define EVENT6(f,a1,a2,a3,a4,a5)        {}
#define EVENT7(f,a1,a2,a3,a4,a5,a6)     {}
#endif
#if QPLOG_STATIC_THRESHOLD <= QPLP_CRITICAL
#define CRITICAL1_STR(f)                       IMS_COMMON1_STR(QPLP_CRITICAL,f)
#define CRITICAL2_STR(f,a1)                    IMS_COMMON2_STR(QPLP_CRITICAL,f,a1)
#define CRITICAL3_STR(f,a1,a2)                 IMS_COMMON3_STR(QPLP_CRITICAL,f,a1,a2)
#define CRITICAL4_STR(f,a1,a2,a3)              IMS_COMMON4_STR(QPLP_CRITICAL,f,a1,a2,a3)
#define CRITICAL5_STR(f,a1,a2,a3,a4)           IMS_COMMON5_STR(QPLP_CRITICAL,f,a1,a2,a3,a4)
#define CRITICAL6_STR(f,a1,a2,a3,a4,a5)        IMS_COMMON6_STR(QPLP_CRITICAL,f,a1,a2,a3,a4,a5)
//#define CRITICAL7_STR(f,a1,a2,a3,a4,a5,a6)
#endif

#if QPLOG_STATIC_THRESHOLD <= QPLP_CRITICAL
#define CRITICAL1(f)                IMS_COMMON1(QPLP_CRITICAL,f)
#define CRITICAL2(f,a1)             IMS_COMMON2(QPLP_CRITICAL,f,a1)
#define CRITICAL3(f,a1,a2)          IMS_COMMON3(QPLP_CRITICAL,f,a1,a2)
#define CRITICAL4(f,a1,a2,a3)       IMS_COMMON4(QPLP_CRITICAL,f,a1,a2,a3)
#define CRITICAL5(f,a1,a2,a3,a4)    IMS_COMMON5(QPLP_CRITICAL,f,a1,a2,a3,a4)
#define CRITICAL6(f,a1,a2,a3,a4,a5) IMS_COMMON6(QPLP_CRITICAL,f,a1,a2,a3,a4,a5)
#else
#define CRITICAL1(f)                       {}
#define CRITICAL2(f,a1)                    {}
#define CRITICAL3(f,a1,a2)                 {}
#define CRITICAL4(f,a1,a2,a3)              {}
#define CRITICAL5(f,a1,a2,a3,a4)           {}
#define CRITICAL6(f,a1,a2,a3,a4,a5)        {}
#define CRITICAL7(f,a1,a2,a3,a4,a5,a6)     {}
#endif


#if QPLOG_STATIC_THRESHOLD <= QPLP_CONTROLLOGS

#define CONTROLLOG(f,a1,a2,a3)           IMS_COMMON4(QPLP_CONTROLLOGS,f,a1,a2,a3)

#endif


#if QPLOG_STATIC_THRESHOLD <= QPLP_MINIMAL
#define MINIMAL1_STR(f)                       IMS_COMMON1_STR(QPLP_MINIMAL,f)
#define MINIMAL2_STR(f,a1)                    IMS_COMMON2_STR(QPLP_MINIMAL,f,a1)
#define MINIMAL3_STR(f,a1,a2)                 IMS_COMMON3_STR(QPLP_MINIMAL,f,a1,a2)
#define MINIMAL4_STR(f,a1,a2,a3)              IMS_COMMON4_STR(QPLP_MINIMAL,f,a1,a2,a3)
#define MINIMAL5_STR(f,a1,a2,a3,a4)           IMS_COMMON5_STR(QPLP_MINIMAL,f,a1,a2,a3,a4)
#define MINIMAL6_STR(f,a1,a2,a3,a4,a5)        IMS_COMMON6_STR(QPLP_MINIMAL,f,a1,a2,a3,a4,a5)
//#define CRITICAL7_STR(f,a1,a2,a3,a4,a5,a6)
#endif


/* MINIMAL logs are not needed to be printed in adb, so handling of MINIMAL is different .
    Call Diag API only if diag initialization is successful (bInit_Success) */
#if QPLOG_STATIC_THRESHOLD <= QPLP_MINIMAL
#define MINIMAL1(f)                 if(bInit_Success) {\
                                      pthread_mutex_lock(&log_write_mutex);\
                                      MSG_2(MSG_SSID_IMS, MSG_LEGACY_HIGH,str_cat([SIMSVT] %d %d ,f), \
                                        getpid(),gettid());\
                                      pthread_mutex_unlock(&log_write_mutex);}
#define MINIMAL2(f,a1)              if(bInit_Success) {\
                                      pthread_mutex_lock(&log_write_mutex);\
                                      MSG_3(MSG_SSID_IMS, MSG_LEGACY_HIGH,str_cat([SIMSVT] %d %d ,f), \
                                        getpid(),gettid(),(intptr_t)a1);\
                                      pthread_mutex_unlock(&log_write_mutex);}
#define MINIMAL3(f,a1,a2)           if(bInit_Success) {\
                                      pthread_mutex_lock(&log_write_mutex);\
                                      MSG_4(MSG_SSID_IMS, MSG_LEGACY_HIGH,str_cat([SIMSVT] %d %d ,f), \
                                        getpid(),gettid(),(intptr_t)a1,(intptr_t)a2);\
                                      pthread_mutex_unlock(&log_write_mutex);}
#define MINIMAL4(f,a1,a2,a3)        if(bInit_Success) {\
                                      pthread_mutex_lock(&log_write_mutex);\
                                      MSG_5(MSG_SSID_IMS, MSG_LEGACY_HIGH,str_cat([SIMSVT] %d %d ,f), \
                                        getpid(),gettid(),(intptr_t)a1,(intptr_t)a2,(intptr_t)a3);\
                                      pthread_mutex_unlock(&log_write_mutex);}
#define MINIMAL5(f,a1,a2,a3,a4)     if(bInit_Success) {\
                                      pthread_mutex_lock(&log_write_mutex);\
                                      MSG_6(MSG_SSID_IMS, MSG_LEGACY_HIGH,str_cat([SIMSVT] %d %d ,f), \
                                        getpid(),gettid(),a1,a2,a3,a4);\
                                      pthread_mutex_unlock(&log_write_mutex);}
#define MINIMAL6(f,a1,a2,a3,a4,a5)  if(bInit_Success) {\
                                      pthread_mutex_lock(&log_write_mutex);\
                                      MSG_7(MSG_SSID_IMS, MSG_LEGACY_HIGH,str_cat([SIMSVT] %d %d ,f), \
                                        getpid(),gettid(),a1,a2,a3,a4,a5);\
                                      pthread_mutex_unlock(&log_write_mutex);}


#else
#define MINIMAL1(f)                       {}
#define MINIMAL2(f,a1)                    {}
#define MINIMAL3(f,a1,a2)                 {}
#define MINIMAL4(f,a1,a2,a3)              {}
#define MINIMAL5(f,a1,a2,a3,a4)           {}
#define MINIMAL6(f,a1,a2,a3,a4,a5)        {}
#define MINIMAL7(f,a1,a2,a3,a4,a5,a6)     {}
#endif

#endif


/* endif __QPLOG_H */
#endif
