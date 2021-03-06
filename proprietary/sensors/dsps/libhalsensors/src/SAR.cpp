/*============================================================================
  @file SAR.cpp

  @brief
  SAR class implementation.

  Copyright (c) 2014-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include "SAR.h"
#include "TimeSyncService.h"

/*============================================================================
  SAR Constructor
============================================================================*/
SAR::SAR(int handle)
    :SMGRSensor(handle)
{
    trigger_mode = SENSOR_MODE_EVENT;
}

/*============================================================================
  SAR Destructor
============================================================================*/
SAR::~SAR()
{

}
/*===========================================================================
  FUNCTION:  setSensorInfo
    Fill the sensor information from the sns_smgr_sensor_datatype_info_s_v01 type
    Parameters
    @datatype : sensor information got from the sensor1 callback
    @info : sensor information to be reported to the framework
===========================================================================*/
void SAR::setSensorInfo(sns_smgr_sensor_datatype_info_s_v01* sensor_datatype)
{
    HAL_LOG_DEBUG("%s: SAR DTy: %d", __FUNCTION__, sensor_datatype->DataType);
    setType(SENSOR_TYPE_SAR);
    setFlags(SENSOR_FLAG_ON_CHANGE_MODE);
    setResolution((float)((float)sensor_datatype->Resolution *
                UNIT_CONVERT_Q16));
    setMaxRange((float)((float)sensor_datatype->MaxRange *
                UNIT_CONVERT_Q16));
    return;
}

/*===========================================================================
  FUNCTION:  processReportInd
    process the sensor data indicator from the sensor1 type to sensor event type
    Parameters
    @smgr_data : the sensor1 data message from the sensor1 callback
    @sensor_data : the sensor event message that will be send to framework
===========================================================================*/
void SAR::processReportInd(sns_smgr_periodic_report_ind_msg_v01* smgr_ind,
            sns_smgr_data_item_s_v01* smgr_data, sensors_event_t &sensor_data)
{
    UNREFERENCED_PARAMETER(smgr_ind);
    sensor_data.type = SENSOR_TYPE_SAR;
    sensor_data.sensor = HANDLE_SAR;
    sensor_data.data[0] = (float)(smgr_data->ItemData[0]) * UNIT_CONVERT_Q16;
    HAL_LOG_VERBOSE("%s: x: P: %f", __FUNCTION__, sensor_data.data[0]);

    last_event.timestamp = sensor_data.timestamp;
    sensor_data.version = sizeof(sensors_event_t);
    sensor_data.timestamp = time_service->timestampCalc((uint64_t)smgr_data->TimeStamp, sensor_data.sensor);
    last_event = sensor_data;
}

/*===========================================================================
  FUNCTION:  prepareAddMsg
    SAR::SMGRPrepareAddMsg will call this function and this func will
    fill the item that needed for this type of sensor.
    Parameters
    @buff_req : the sensor1 message buffer
===========================================================================*/
void SAR::prepareAddMsg(sns_smgr_buffering_req_msg_v01 **buff_req)
{
    (*buff_req)->Item[0].SensorId = SNS_SMGR_ID_SAR_V01;
}
