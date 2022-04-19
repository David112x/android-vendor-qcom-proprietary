/*============================================================================
  @file TimeSyncService.h

  @brief
  TimeSyncService class definition.

  Copyright (c) 2014-2017 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef ANDROID_TIMER_H
#define ANDROID_TIMER_H

#include "sns_time_api_v02.h"
extern "C" {
#include "sensor1.h"
#define __bool_true_false_are_defined 1
#include "sensors_hal.h"
}

/*============================================================================
 * Class TimeSyncService
 *=============================================================================*/
class TimeSyncService {
private:
    /* time_service enabled status */
    int               enabled;
    /*timesync_sensor1_cb_t to be used for Daemon restart cases as well*/
    hal_sensor1_cb_t *sensor1_cb;
    /* Offset (in ns) between apps and dsps timestamps */
    int64_t           timestamp_offset_apps_dsps;
    /* Last DSPS timestamp received */
    uint32_t          dsps_ts_last[MAX_NUM_SENSORS];
    /* # of times the DSPS clock has "rolled over" and restarted at 0 */
    uint32_t          dsps_rollover_cnt[MAX_NUM_SENSORS];
    /* Rollover count as received in last time service report */
    uint32_t          dsps_rollover_cnt_rcv[MAX_NUM_SENSORS];
    /* Boottime timestamp of the last predicted rollover event */
    int64_t           boot_ts_last_rollover[MAX_NUM_SENSORS];
    /* The timestamp on the last sample sent to Android (per sensor) */
    int64_t           timestamp_last_sent[MAX_NUM_SENSORS];
    /* we only allow time_service class has one object */
    static TimeSyncService *self;
    /*Sensor type*/
    int sns_type;
    /* QTimer availability */
    static bool       is_qtimer_supported;
/*============================================================================
  FUNCTION Constructor
    Put the constructor to be private.
============================================================================*/
    TimeSyncService();
/*===========================================================================
  FUNCTION:  rolloverCntIs
    Update the rollover count
    parameters
        @rollover_cnt : the rollover count in the ssc side
===========================================================================*/
    void rolloverCntIs(uint32_t rollover_cnt);
/*===========================================================================
  FUNCTION:  tsOffsetIs
    Update local timestamp offset state.
    paramters
        @timestamp_ssc : the timestamp in the SSC side
        @timestamp_ap: the timestamp in the AS sdie
===========================================================================*/
    void tsOffsetIs(uint32_t timestamp_ssc, uint64_t timestamp_ap);

public:
    ~TimeSyncService();
/*============================================================================
  FUNCTION getTimeSyncService
    Get the TimeSyncService static object.
    Return value
        @TimeSyncService* : The TimeSyncService object
============================================================================*/
    static TimeSyncService* getTimeSyncService();
/*===========================================================================
  FUNCTION:  getQtimerTicks
    Retrieve current QTimer value
    Return value
        uint64_t : current QTimer value (in 19.2 Mhz ticks)
                   if QTimer is unavailable, then this will return 0
===========================================================================*/
    static uint64_t getQtimerTicks(void);
/*===========================================================================
  FUNCTION:  qtimerTicksToNano
    Convert a timestamp from QTimer ticks to nanoseconds
    Parameters
        @qtimer_ticks : timestamp in QTimer ticks (19.2 Mhz clock)
    Return value
        uint64_t : timestamp in nanoseconds
===========================================================================*/
    static uint64_t qtimerTicksToNano(uint64_t qtimer_ticks);
/*===========================================================================
  FUNCTION:  getElapsedRealtimeNanoOffset
    Calculate and return the offset between the QTimer and Android's
    elapsedRealtimeNano clocks. offset = qtimerNano - elapsedRealtimeNano
    Return value
        int64_t : offset in nanoseconds
===========================================================================*/
    static int64_t getElapsedRealtimeNanoOffset(void);
/*============================================================================
  FUNCTION getCb
    Get the TimeSyncService sensor1 handle;
    Return value
        @hal_sensor1_cb_t* : Then sensor1 callback data structure
============================================================================*/
    hal_sensor1_cb_t* getCb();
/*===========================================================================
  FUNCTION:  timeServiceStart
    Start the time_service service
    Return value
        1 : successful
        0 : failed
===========================================================================*/
    int timeServiceStart();
/*===========================================================================
  FUNCTION:  timeServiceStop
    Stop the time_service service
    Return value
        1 : successful
        0 : failed
===========================================================================*/
    int timeServiceStop();
/*===========================================================================
  FUNCTION:  getTimeSyncServiceStatus
    Get the time_service status
    Return value
        1 : time_service is started
        0 : time_service is stopped
===========================================================================*/
    int getTimeSyncServiceStatus();
/*===========================================================================
  FUNCTION:  processTimeResp
    Process response message from the time sync service.
    Parameters
        @msg_hdr : sensor1 message header
        @msg_ptr : sensor1 message pointer
===========================================================================*/
    void processTimeResp(sensor1_msg_header_s const *msg_hdr,
        sns_time_timestamp_resp_msg_v02 const *msg_ptr);
/*===========================================================================
  FUNCTION:  processTimeInd
    Process indication message from the time sync service.
    Parameters
        @msg_hdr : sensor1 message header
        @msg_ptr : sensor1 message pointer
===========================================================================*/
    void processTimeInd(sensor1_msg_header_s const *msg_hdr,
        sns_time_timestamp_ind_msg_v02 const *msg_ptr);
/*===========================================================================
  FUNCTION:  processCancelResp
    Process Cancel response to the time_service service.
===========================================================================*/
    void processCancelResp();
/*===========================================================================
  FUNCTION:  timestampCalc
    Converts the DSPS clock ticks from a sensor sample to a LA timestamp (ns
    since epoch).  Adjusts return value based on dsps timestamp rollover
    and makes minor adjustments to ensure sensor samples are sent with
    ascending timestamps.
    Parameters
        @dsps_timestamp : the time stamp in the sensor event
        @sensor_handle : one type of sensor handle
    Return value
        int64_t : the calculated timestamp
===========================================================================*/
    int64_t timestampCalc(uint64_t dsps_timestamp, int sensor_handle);

/*===========================================================================
  FUNCTION:  timeSyncEnable
        reset the variables for each sensor enable
    Parameters
        @sensor_handle : one type of sensor handle
        @snsType : sensor type to store for logging
===========================================================================*/
    void timeSyncEnable(int sensor_handle, int snsType);

/*===========================================================================
  FUNCTION:  isTimeServiceStarted
    Return value
        bool : whether started or not yet
===========================================================================*/
    bool isTimeServiceStarted();
};

/*===========================================================================
  FUNCTION:  time_service_sensor1_cb
    Parameters
        @cb_data : callback data pointer
        @msg_hdr : sensor1 message header
        @msg_type : sensor1 message type
        @msg_ptr : sensor1 message pointer
===========================================================================*/
void time_service_sensor1_cb(intptr_t cb_data, sensor1_msg_header_s *msg_hdr,
        sensor1_msg_type_e msg_type, void *msg_ptr);

#endif
