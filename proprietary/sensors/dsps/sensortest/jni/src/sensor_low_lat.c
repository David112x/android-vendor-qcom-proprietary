/*============================================================================
@file sensor_low_lat.cpp

@brief
Java Native code to use the Sensors (SNS) Low Latency stream.

Copyright (c) 2017 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
============================================================================*/

// Prevent AEEStdDef.h (fastRPC header) from redefining uint64 and int64 types
#define _UINT64_DEFINED
#define _INT64_DEFINED

#include <jni.h>
#include <pthread.h>
#include "rpcmem.h"
#include "log.h"
#include "log_codes.h"
#include "sns_low_lat_buffer.h"
#include "fixed_point.h"
#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <stdlib.h>
#include <utils/Log.h>
#include <common_log.h>
#include <stdbool.h>
#include "sns_low_lat_stream.h"
#include "sensor_low_lat.h"
//#include "sns_em.h"

#define LOG_TAG "sensor_low_lat"

#define SENSOR_LOW_LAT_ERR_MSG_MAX_SIZE 120

#define SENSOR_LOW_LAT_DEFAULT_OFFSET 0

#define SENSOR_LOW_LAT_SLEEP_US 5000

#ifndef LOG_SNS_HDRLL_DATA_CLIENT_C
  #define LOG_SNS_HDRLL_DATA_CLIENT_C 0x19B9
#endif /* LOG_SNS_HDRLL_DATA_CLIENT_C */

#define SNS_LOG_HDRLL_DATA_NUM_AXES 3
#define SAMPLE_PERIOD_US      1250 // 800 hz

#define SNS_TIMESTAMP_DSPS  0
#define SNS_TIMESTAMP_APPS  1
#define SNS_TIMESTAMP_MODEM 2

#define USEC_PER_SEC 1000000L
#define NSEC_PER_SEC 1000000000L

#define SEC_TO_USEC(sec)   ((sec)*USEC_PER_SEC)
#define NSEC_TO_USEC(nsec) ((nsec)/1000L)

#define USEC_TO_NSEC(usec) ((usec)*1000L)
#define USEC_TO_SEC(usec)  ((usec)/USEC_PER_SEC)

typedef int32_t q16_t;

#define SNS_LOG_HDRLL_DATA_CLIENT_VERSION     1
typedef PACK(struct) {
   log_hdr_type log_type;       /**< Type, as defined in log_codes.h         */
   uint32_t     version;        /**< Version of the log packet               */
   uint32_t     sensor_id_1;    /**< The sensor ID for the first set of data */
   uint64_t     timestamp_1;    /**< Timestamp for the first set of data (in SSC ticks) */
   q16_t        data_1[3];      /**< First set of data in Q16 */
   uint32_t     sensor_id_2;    /**< The sensor ID for the second set of data */
   uint64_t     timestamp_2;    /**< Timestamp for the second set of data (in SSC ticks) */
   q16_t        data_2[3];      /**< second set of data in Q16 */
   uint8_t      stream_id;      /**< Stream that this data belongs to */
   uint8_t      log_timestamp_type; /**< Source of the timestamp */
   uint64_t     log_timestamp;  /**< Timestamp that this log packet was generated */
} sns_log_hdrll_data_client;


// PEND: This class will allow for better abstraction to deal with concurrent JNI clients
///*============================================================================
// * Class sensor_low_lat
// *============================================================================*/
//class sensor_low_lat
//{
//};

/*============================================================================
 Variable Definitions
 ===========================================================================*/


/*============================================================================
 Static Variable Definitions
 ===========================================================================*/

// PEND: These static variables should, ideally, be encompassed in a C++ object
static uint8* output_buf = 0;
static uint32 circ_buf_iter = 0;
static pthread_t worker_thread;
static boolean active = false;


/*============================================================================
 Static Method Definitions
 ===========================================================================*/
static void sensor_low_lat_log_samples(sns_low_lat_sample* sample_1, sns_low_lat_sample* sample_2)
{
  int a;
  struct timespec time;
  sns_log_hdrll_data_client* log_pkt;

  // Don't bother logging anything if neither sample is present
  if ((NULL == sample_1) && (NULL == sample_2))
  {
    return;
  }

  log_pkt = (sns_log_hdrll_data_client*) log_alloc(LOG_SNS_HDRLL_DATA_CLIENT_C, sizeof(*log_pkt));

  if (NULL != log_pkt)
  {
    log_pkt->version = SNS_LOG_HDRLL_DATA_CLIENT_VERSION;

    // Log the first sample, if it's available
    if (NULL != sample_1)
    {
      if (SNS_LOW_LAT_ACCEL == sample_1->sensor_type)
      {
        log_pkt->sensor_id_1 = 1;
      }
      else
      {
        log_pkt->sensor_id_1 = 3;
      }
      // Convert the timestamp from QTimer ticks to SSC ticks
      log_pkt->timestamp_1 = (sample_1->timestamp << 4) / 9375;
      for (a = 0; a < SNS_LOG_HDRLL_DATA_NUM_AXES; a++)
      {
        log_pkt->data_1[a] = FX_FLTTOFIX_Q16(sample_1->sample[a]);
      }
    }

    // Log the second sample in the expected pair, if it's available
    if (NULL != sample_2)
    {
      if (SNS_LOW_LAT_ACCEL == sample_2->sensor_type)
      {
        log_pkt->sensor_id_2 = 1;
      }
      else
      {
        log_pkt->sensor_id_2 = 3;
      }
      // Convert the timestamp from QTimer ticks to SSC ticks
      log_pkt->timestamp_2 = (sample_2->timestamp << 4) / 9375;
      for (a = 0; a < SNS_LOG_HDRLL_DATA_NUM_AXES; a++)
      {
        log_pkt->data_2[a] = FX_FLTTOFIX_Q16(sample_2->sample[a]);
      }
    }

    // Currently the client does not know the stream ID so we can't publish it.
    //log_pkt->stream_id = stream_id;
    log_pkt->log_timestamp_type = SNS_TIMESTAMP_APPS;
    clock_gettime( CLOCK_REALTIME, &time );
    log_pkt->log_timestamp = SEC_TO_USEC((uint64_t)time.tv_sec) + NSEC_TO_USEC((uint64_t)time.tv_nsec);

    log_commit(log_pkt);
  }
  else
  {
    LOGW("%s: Unable to allocate log packet. Enabled %d", __FUNCTION__, log_status(LOG_SNS_HDRLL_DATA_CLIENT_C));
  }
}

static void sensor_low_lat_polling(void* arg)
{
  int error_code;
  uint32 most_recent_idx;
  uint32 iter_stop_idx;
  sns_low_lat_sample* sample_1 = NULL;
  sns_low_lat_sample* sample_2 = NULL;

  sns_low_lat_circ_buf* circ_buf = (sns_low_lat_circ_buf*) output_buf;

  // Constantly poll the stream for new samples
  while (active)
  {
    error_code = sns_low_lat_stream_poll((int)arg);

    // Log the data if the poll was successful
    if (0 == error_code)
    {
      most_recent_idx = circ_buf->header.info.most_recent_index;

      iter_stop_idx = most_recent_idx + 1;
      if (iter_stop_idx >= circ_buf->header.info.max_samples)
      {
        iter_stop_idx = 0;
      }

      // Iterate through the circular buffer starting with where we last left off
      do
      {
        // Retrieve the first sample
        sample_1 = &circ_buf->samples[circ_buf_iter].data;
        // Increment the iterator for the next sample
        circ_buf_iter++;
        if (circ_buf_iter >= circ_buf->header.info.max_samples)
        {
          circ_buf_iter = 0;
        }

        // Break out of this loop if the iter hits the stopping point
        if (circ_buf_iter == iter_stop_idx)
        {
          break;
        }

        // Retrieve the second sample
        sample_2 = &circ_buf->samples[circ_buf_iter].data;
        // Increment the iterator for the next sample
        circ_buf_iter++;
        if (circ_buf_iter >= circ_buf->header.info.max_samples)
        {
          circ_buf_iter = 0;
        }

        // Log the samples and nullify the pointers so they can be reused safely
        sensor_low_lat_log_samples(sample_1, sample_2);
        sample_1 = NULL;
        sample_2 = NULL;
      } while (circ_buf_iter != iter_stop_idx);

      // Log any unlogged samples
        sensor_low_lat_log_samples(sample_1, sample_2);
        sample_1 = NULL;
        sample_2 = NULL;
    }
    else
    {
      LOGE("%s: Error polling the low lat stream %d", __FUNCTION__, error_code);
    }
  }

  pthread_exit(NULL);
}

/*============================================================================
 JNI Method Definitions
 ===========================================================================*/

/**
 * Initializes the stream state with the framework.
 *
 * @param bufferSize The desired size of the circular buffer for the output samples.
 * @throws
 */
/*
 * Class:     com_qualcomm_qti_sensors_core_sensortest_SensorLowLat
 * Method:    init
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_qualcomm_qti_sensors_core_sensortest_SensorLowLat_init
  (JNIEnv *env, jobject obj, jint buffer_size)
{
  int       fd;
  int       error_code;
  int       stream_handle = 0;
  //jclass    mClass = (*env)->GetObjectClass(env, obj);
  jclass    mClass = (*env)->FindClass(env, "com/qualcomm/qti/sensors/core/sensortest/SensorLowLat");
  jfieldID  field_id;
  char      err_msg[SENSOR_LOW_LAT_ERR_MSG_MAX_SIZE];

  // Validate input
  if (buffer_size <= 0)
  {
    jclass input_exception;

    LOGE("%s: Invalid buffer size %d", __FUNCTION__, buffer_size);
    input_exception = (*env)->FindClass(env, "java/lang/IllegalArgumentException");
    if (input_exception != NULL)
    {
      snprintf(err_msg, SENSOR_LOW_LAT_ERR_MSG_MAX_SIZE, "Invalid buffer_size %d", buffer_size);
      (*env)->ThrowNew(env, input_exception, err_msg);
    }
    return;
  }
  if (NULL != output_buf)
  {
    // PEND: This prevents concurrent JNI clients. This limitation should be removed there's a need to support concurrent clients.
    jclass state_exception;

    LOGE("%s: A low-lat JNI client already exists. Concurrent JNI clients are not allowed at the moment.", __FUNCTION__);
    state_exception = (*env)->FindClass(env, "java/lang/IllegalStateException");
    if (state_exception != NULL)
    {
      (*env)->ThrowNew(env, state_exception, "there's an already existing low lat stream, the JNI does not currently support concurrent low lat streams");
    }
    return;
  }
    if (NULL == mClass)
  {
    // PEND: This prevents concurrent JNI clients. This limitation should be removed there's a need to support concurrent clients.
    jclass state_exception;

    LOGE("%s: can't find the object's class", __FUNCTION__);
    state_exception = (*env)->FindClass(env, "java/lang/IllegalStateException");
    if (state_exception != NULL)
    {
      (*env)->ThrowNew(env, state_exception, "can't find the object's class");
    }
    return;
  }

  rpcmem_init();

  // Allocate the shared memory buffer and get the FD for that buffer
  output_buf = (uint8*)rpcmem_alloc(RPCMEM_DEFAULT_HEAP, RPCMEM_DEFAULT_FLAGS, buffer_size);
  if (NULL == output_buf)
  {
    jclass null_ptr_exception;

    LOGE("%s: rpcmem_alloc failed.", __FUNCTION__);
    null_ptr_exception = (*env)->FindClass(env, "java/lang/NullPointerException");
    if (null_ptr_exception != NULL)
    {
      (*env)->ThrowNew(env, null_ptr_exception, "rpcmem_alloc failed");
    }
    return;
  }
  fd = rpcmem_to_fd(output_buf);

  error_code = sns_low_lat_stream_init(fd, buffer_size, SENSOR_LOW_LAT_DEFAULT_OFFSET, &stream_handle);
  if (0 != error_code)
  {
    jclass state_exception;
    rpcmem_free(output_buf);
    output_buf = NULL;

    LOGE("%s: sns_low_lat_stream_init failed! error_code %d", __FUNCTION__, error_code);
    state_exception = (*env)->FindClass(env, "java/lang/IllegalStateException");
    if (state_exception != NULL)
    {
      snprintf(err_msg, SENSOR_LOW_LAT_ERR_MSG_MAX_SIZE, "Init failed! error_code %d", error_code);
      (*env)->ThrowNew(env, state_exception, err_msg);
    }
    return;
  }

  // Save the handle in the Java object
  field_id = (*env)->GetStaticFieldID(env, mClass, "handle", "I");
  if (NULL == field_id)
  {
    // PEND: This prevents concurrent JNI clients. This limitation should be removed there's a need to support concurrent clients.
    jclass state_exception;

    LOGE("%s: can't find the field ID for 'handle'", __FUNCTION__);
    state_exception = (*env)->FindClass(env, "java/lang/IllegalStateException");
    if (state_exception != NULL)
    {
      (*env)->ThrowNew(env, state_exception, "can't find the field ID for 'handle'");
    }
    return;
  }
  (*env)->SetStaticIntField(env, mClass, field_id, (jint)stream_handle);
}

/*
 * Class:     com_qualcomm_qti_sensors_core_sensortest_SensorLowLat
 * Method:    start
 * Signature: ([JII)V
 */
JNIEXPORT void JNICALL Java_com_qualcomm_qti_sensors_core_sensortest_SensorLowLat_start
  (JNIEnv *env, jobject obj, jlongArray suids, jint sample_period_us, jint flags)
{
  int       error_code;
  //jclass    mClass = (*env)->GetObjectClass(env, obj);
  jclass    mClass = (*env)->FindClass(env, "com/qualcomm/qti/sensors/core/sensortest/SensorLowLat");
  jfieldID  field_id;
  jsize     suids_len = (*env)->GetArrayLength(env, suids);
  jboolean  sensor_suid_is_copy;
  jlong*    sensor_suid = (*env)->GetLongArrayElements(env, suids, &sensor_suid_is_copy);
  jint      stream_handle = 0;
  char      err_msg[SENSOR_LOW_LAT_ERR_MSG_MAX_SIZE];

  // Validate input
  if (suids_len <= 0)
  {
    jclass input_exception;

    LOGE("%s: invalid suids_len %d", __FUNCTION__, suids_len);
    input_exception = (*env)->FindClass(env, "java/lang/IllegalArgumentException");
    if (input_exception != NULL)
    {
      snprintf(err_msg, SENSOR_LOW_LAT_ERR_MSG_MAX_SIZE, "Invalid suids_len %d", suids_len);
      (*env)->ThrowNew(env, input_exception, err_msg);
    }
    return;
  }
  if (sample_period_us <= 0)
  {
    jclass input_exception;

    LOGE("%s: invalid sample_period_us", __FUNCTION__, sample_period_us);
    input_exception = (*env)->FindClass(env, "java/lang/IllegalArgumentException");
    if (input_exception != NULL)
    {
      snprintf(err_msg, SENSOR_LOW_LAT_ERR_MSG_MAX_SIZE, "Invalid sample_period_us %d", sample_period_us);
      (*env)->ThrowNew(env, input_exception, err_msg);
    }
    return;
  }
  if (NULL == output_buf)
  {
    jclass state_exception;

    LOGE("%s: no available low-lat stream to start!", __FUNCTION__);
    state_exception = (*env)->FindClass(env, "java/lang/IllegalStateException");
    if (state_exception != NULL)
    {
      (*env)->ThrowNew(env, state_exception, "There's no available low lat stream to start, try calling init first");
    }
    return;
  }

  // Get the handle from the Java object
  field_id = (*env)->GetStaticFieldID(env, mClass, "handle", "I");
  // TODO: THROW AN EXCEPTION IF THE "GetFieldID" CALL FAILS
  // if (NULL == field_id)
  stream_handle = (*env)->GetStaticIntField(env, mClass, field_id);

  // Start the stream
  error_code = sns_low_lat_stream_start((int)stream_handle, (uint64*)sensor_suid, (int)suids_len, (unsigned int)sample_period_us, (unsigned int)flags);
  if (0 != error_code)
  {
    jclass state_exception;

    LOGE("%s: sns_low_lat_stream_start failed! error_code %d", __FUNCTION__, error_code);
    state_exception = (*env)->FindClass(env, "java/lang/IllegalStateException");
    if (state_exception != NULL)
    {
      snprintf(err_msg, SENSOR_LOW_LAT_ERR_MSG_MAX_SIZE, "Start failed! error_code %d", error_code);
      (*env)->ThrowNew(env, state_exception, err_msg);
    }
    return;
  }

  // Spawn a worker thread for polling the shared memory
  active = true;
  circ_buf_iter = 0;
  error_code = pthread_create(&worker_thread, NULL, (void *)sensor_low_lat_polling, (void *)(intptr_t)stream_handle);
  if (0 != error_code)
  {
    jclass state_exception;

    active = false;

    LOGE("%s: pthread_create failed! error_code %d", __FUNCTION__, error_code);
    state_exception = (*env)->FindClass(env, "java/lang/IllegalStateException");
    if (state_exception != NULL)
    {
      snprintf(err_msg, SENSOR_LOW_LAT_ERR_MSG_MAX_SIZE, "Unable to create a worker thread! error_code %d", error_code);
      (*env)->ThrowNew(env, state_exception, err_msg);
    }
    return;
  }

  // TODO: MAKE SURE THIS IS CALLED BEFORE ALL OF THE RETURNS SO MEMORY ISN'T LEAKED
  // Release the sensor_suid array so that the Java garbage collector knows
  // it's okay to free the sensor_suid array
  (*env)->ReleaseLongArrayElements(env, suids, sensor_suid, 0);
}

/*
 * Class:     com_qualcomm_qti_sensors_core_sensortest_SensorLowLat
 * Method:    stop
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_qualcomm_qti_sensors_core_sensortest_SensorLowLat_stop
  (JNIEnv *env, jobject obj)
{
  int       error_code;
  //jclass    mClass = (*env)->GetObjectClass(env, obj);
  jclass    mClass = (*env)->FindClass(env, "com/qualcomm/qti/sensors/core/sensortest/SensorLowLat");
  jfieldID  field_id;
  jint      stream_handle = 0;
  char      err_msg[SENSOR_LOW_LAT_ERR_MSG_MAX_SIZE];

  // Validate input
  if (NULL == output_buf)
  {
    jclass state_exception;

    LOGE("%s: no available low-lat stream to stop!", __FUNCTION__);
    state_exception = (*env)->FindClass(env, "java/lang/IllegalStateException");
    if (state_exception != NULL)
    {
      (*env)->ThrowNew(env, state_exception, "There's no available low lat stream to stop, try calling init and start first");
    }
    return;
  }

  // Get the handle from the Java object
  field_id = (*env)->GetStaticFieldID(env, mClass, "handle", "I");
  // TODO: THROW AN EXCEPTION IF THE "GetFieldID" CALL FAILS
  // if (NULL == field_id)
  stream_handle = (*env)->GetStaticIntField(env, mClass, field_id);

  // Signal the worker thread to exit and wait a little bit
  active = false;
  usleep(SENSOR_LOW_LAT_SLEEP_US);

  // Stop the stream
  error_code = sns_low_lat_stream_stop((int)stream_handle);
  if (0 != error_code)
  {
    jclass state_exception;

    LOGE("%s: sns_low_lat_stream_stop failed! error_code %d", __FUNCTION__, error_code);
    state_exception = (*env)->FindClass(env, "java/lang/IllegalStateException");
    if (state_exception != NULL)
    {
      snprintf(err_msg, SENSOR_LOW_LAT_ERR_MSG_MAX_SIZE, "Stop failed! error_code %d", error_code);
      (*env)->ThrowNew(env, state_exception, err_msg);
    }
    return;
  }
}

/*
 * Class:     com_qualcomm_qti_sensors_core_sensortest_SensorLowLat
 * Method:    deinit
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_qualcomm_qti_sensors_core_sensortest_SensorLowLat_deinit
  (JNIEnv *env, jobject obj)
{
  int       error_code;
  //jclass    mClass = (*env)->GetObjectClass(env, obj);
  jclass    mClass = (*env)->FindClass(env, "com/qualcomm/qti/sensors/core/sensortest/SensorLowLat");
  jfieldID  field_id;
  jint      stream_handle = 0;
  char      err_msg[SENSOR_LOW_LAT_ERR_MSG_MAX_SIZE];

  if (active)
  {
    jclass state_exception;

    LOGE("%s: trying to deinit an active stream! error_code %d", __FUNCTION__);
    state_exception = (*env)->FindClass(env, "java/lang/IllegalStateException");
    if (state_exception != NULL)
    {
      (*env)->ThrowNew(env, state_exception, "Unexpected deinit called on an active stream!");
    }
    active = false;
    // No need to return early from this failure, continue freeing the rest of the resources
  }

  // Get the handle from the Java object
  field_id = (*env)->GetStaticFieldID(env, mClass, "handle", "I");
  // TODO: THROW AN EXCEPTION IF THE "GetFieldID" CALL FAILS
  if (NULL == field_id)
  {
    // PEND: This prevents concurrent JNI clients. This limitation should be removed there's a need to support concurrent clients.
    jclass state_exception;

    LOGE("%s: can't find the field ID for 'handle'", __FUNCTION__);
    state_exception = (*env)->FindClass(env, "java/lang/IllegalStateException");
    if (state_exception != NULL)
    {
      (*env)->ThrowNew(env, state_exception, "can't find the field ID for 'handle'");
    }
    return;
  }

  stream_handle = (*env)->GetStaticIntField(env, mClass, field_id);

  error_code = sns_low_lat_stream_deinit((int)stream_handle);
  if (0 != error_code)
  {
    jclass state_exception;

    LOGE("%s: sns_low_lat_stream_deinit failed! error_code %d", __FUNCTION__, error_code);
    state_exception = (*env)->FindClass(env, "java/lang/IllegalStateException");
    if (state_exception != NULL)
    {
      snprintf(err_msg, SENSOR_LOW_LAT_ERR_MSG_MAX_SIZE, "Deinit failed! error_code %d", error_code);
      (*env)->ThrowNew(env, state_exception, err_msg);
    }
    // No need to return early from this failure, continue freeing the rest of the resources
  }
  // Set the handle to NULL in the Java object
  (*env)->SetStaticIntField(env, mClass, field_id, (jint)NULL);

  if (NULL != output_buf)
  {
    rpcmem_free(output_buf);
    output_buf = NULL;
  }
  rpcmem_deinit();
}
