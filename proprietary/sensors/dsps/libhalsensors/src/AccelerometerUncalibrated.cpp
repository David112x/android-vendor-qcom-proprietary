/*============================================================================
  @file AccelerometerUncalibrated.cpp

  @brief
  AccelerometerUncalibrated class implementation.

  Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
============================================================================*/

#include "AccelerometerUncalibrated.h"
#include "SensorsContext.h"

/*============================================================================
  AccelerometerUncalibrated Constructor
============================================================================*/
AccelerometerUncalibrated::AccelerometerUncalibrated(int handle)
    :SMGRSensor(handle)
{
    trigger_mode = SENSOR_MODE_CONT;
    (handle == HANDLE_ACCELERATION_UNCALIBRATED_WAKE_UP)?(bWakeUp = true):(bWakeUp = false);
    flags = SENSOR_FLAG_CONTINUOUS_MODE;
#ifdef SNS_DIRECT_REPORT_SUPPORT
    if (Utility::isDirectChannelTypeSupported()) {
        // Add the Direct Channel Gralloc flag
        flags |= SENSOR_FLAG_DIRECT_CHANNEL_GRALLOC;
        // PEND: This should happen when processing the SMGR info in setSensorInfo to dynamically configure the flags
        // Add the supported Direct Channel rates
        flags |= (SENSOR_DIRECT_RATE_VERY_FAST << SENSOR_FLAG_SHIFT_DIRECT_REPORT);
    }
#endif

}

/*============================================================================
  AccelerometerUncalibrated Destructor
============================================================================*/
AccelerometerUncalibrated::~AccelerometerUncalibrated()
{

}
/*===========================================================================
  FUNCTION:  setSensorInfo
    Fill the sensor information from the sns_smgr_sensor_datatype_info_s_v01 type
    Parameters
    @datatype : sensor information got from the sensor1 callback
===========================================================================*/
void AccelerometerUncalibrated::setSensorInfo(sns_smgr_sensor_datatype_info_s_v01* datatype)
{
    UNREFERENCED_PARAMETER(datatype);
    SensorsContext *context = SensorsContext::getInstance();
    Sensor *mSensor = context->getSensor(HANDLE_ACCELERATION);
    if (mSensor != NULL) {
        setName(mSensor->getName());
        strlcat(name, " Uncalibrated", SNS_MAX_SENSOR_NAME_SIZE);
        setVendor(mSensor->getVendor());
        setVersion(mSensor->getVersion());
        setType(SENSOR_TYPE_ACCELEROMETER_UNCALIBRATED);

        if(bWakeUp == false) {
           flags |= SENSOR_FLAG_CONTINUOUS_MODE;
        } else {
            strlcat(name," -Wakeup",SNS_MAX_SENSOR_NAME_SIZE);
            setFlags(SENSOR_FLAG_CONTINUOUS_MODE|SENSOR_FLAG_WAKE_UP);
        }
        setMaxRange(mSensor->getMaxRange());
        setResolution(mSensor->getResolution());
        setPower(mSensor->getPower());
        setMaxFreq(mSensor->getMaxFreqBkp());
        setMinFreq(mSensor->getMinFreq());
        setAttribOK(mSensor->getAttribOK());
        setMaxBufferedSamples(mSensor->getMaxBufferedSamples());
        setResBufferedSamples(mSensor->getResBufferedSamples());
    }
    else {
        HAL_LOG_ERROR("The mSensor handle %d is NULL!", HANDLE_ACCELERATION);
    }
}

/*===========================================================================
  FUNCTION:  processReportInd
    process the sensor data indicator from the sensor1 type to sensor event type
    Parameters
    @smgr_data : the sensor1 data message from the sensor1 callback
    @sensor_data : the sensor event message that will be send to framework
===========================================================================*/
void AccelerometerUncalibrated::processReportInd(
                        sns_smgr_periodic_report_ind_msg_v01* smgr_ind,
                        sns_smgr_data_item_s_v01* smgr_data,
                        sensors_event_t &sensor_data)
{
    UNREFERENCED_PARAMETER(smgr_ind);
    HAL_LOG_DEBUG("AccelerometerUncalibrated::processReportInd");
    sensor_data.type = SENSOR_TYPE_ACCELEROMETER_UNCALIBRATED;

    if(bWakeUp == false) {
        sensor_data.sensor = HANDLE_ACCELERATION_UNCALIBRATED;
        HAL_LOG_VERBOSE("%s:sensor %s ",__FUNCTION__,
                Utility::SensorTypeToSensorString(getType()));
    } else {
        sensor_data.sensor = HANDLE_ACCELERATION_UNCALIBRATED_WAKE_UP;
        HAL_LOG_VERBOSE("%s:sensor %s (wake_up)",__FUNCTION__,
                Utility::SensorTypeToSensorString(getType()));
    }

       /* Convert from SAE to Android co-ordinates and scale
       x' = y; y' = x; z' = -z;*/
       sensor_data.uncalibrated_accelerometer.x_uncalib =
               (float)(smgr_data->ItemData[1]) * UNIT_CONVERT_ACCELERATION;
       sensor_data.uncalibrated_accelerometer.y_uncalib =
               (float)(smgr_data->ItemData[0]) * UNIT_CONVERT_ACCELERATION;
       sensor_data.uncalibrated_accelerometer.z_uncalib =
               (float)(-smgr_data->ItemData[2]) * UNIT_CONVERT_ACCELERATION;

       sensor_data.uncalibrated_accelerometer.x_bias = (float)(0);
       sensor_data.uncalibrated_accelerometer.y_bias = (float)(0);
       sensor_data.uncalibrated_accelerometer.z_bias = (float)(0);

       HAL_LOG_VERBOSE( "%s: Uncal Accel x:%f y:%f z:%f xb:%f yb:%f zb:%f",
               __FUNCTION__,
               sensor_data.uncalibrated_accelerometer.x_uncalib,
               sensor_data.uncalibrated_accelerometer.y_uncalib,
               sensor_data.uncalibrated_accelerometer.z_uncalib,
               sensor_data.uncalibrated_accelerometer.x_bias,
               sensor_data.uncalibrated_accelerometer.y_bias,
               sensor_data.uncalibrated_accelerometer.z_bias );
}

/*===========================================================================
  FUNCTION:  prepareAddMsg
    SMGRSensor::SMGRPrepareAddMsg will call this function and this func will
    fill the item that needed for this type of sensor.
    Parameters
    @buff_req : the sensor1 message buffer
===========================================================================*/
void AccelerometerUncalibrated::prepareAddMsg(sns_smgr_buffering_req_msg_v01 **buff_req)
{
    HAL_LOG_DEBUG("AccelerometerUncalibrated::prepareAddMsg");
    (*buff_req)->Item[0].SensorId = SNS_SMGR_ID_ACCEL_V01;
}
