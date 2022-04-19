/*============================================================================
  @file Magnetic.cpp

  @brief
  Magnetic class implementation.

  Copyright (c) 2014-2015, 2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
============================================================================*/

#include "Magnetic.h"
#include <cutils/properties.h>

/*============================================================================
  Magnetic Constructor
============================================================================*/
Magnetic::Magnetic(int handle)
    :SMGRSensor(handle)
{
    trigger_mode = SENSOR_MODE_CONT;
    (handle == HANDLE_MAGNETIC_FIELD_WAKE_UP)?(bWakeUp = true):(bWakeUp = false);

    // Set flags
    flags = SENSOR_FLAG_CONTINUOUS_MODE;
#ifdef SNS_DIRECT_REPORT_SUPPORT
    if (Utility::isDirectChannelTypeSupported()) {
        // Add the Direct Channel Gralloc flag
        flags |= SENSOR_FLAG_DIRECT_CHANNEL_GRALLOC;
        // PEND: This should happen when processing the SMGR info in setSensorInfo to dynamically configure the flags
        // Add the supported Direct Channel rates
        flags |= (SENSOR_DIRECT_RATE_NORMAL << SENSOR_FLAG_SHIFT_DIRECT_REPORT);
    }
#endif
}

/*============================================================================
  Magnetic Destructor
============================================================================*/
Magnetic::~Magnetic()
{

}

uint64_t Magnetic::getSuid()
{
  return SNS_SMGR_SUID_MAG_1_V01;
}

/*===========================================================================
  FUNCTION:  setSensorInfo
    Fill the sensor information from the sns_smgr_sensor_datatype_info_s_v01 type
    Parameters
    @datatype : sensor information got from the sensor1 callback
===========================================================================*/
void Magnetic::setSensorInfo(sns_smgr_sensor_datatype_info_s_v01* sensor_datatype)
{
    HAL_LOG_DEBUG("%s: MAG DTy: %d", __FUNCTION__, sensor_datatype->DataType);
    setType(SENSOR_TYPE_MAGNETIC_FIELD);

    if (bWakeUp == true) {
        strlcat(name," -Wakeup",SNS_MAX_SENSOR_NAME_SIZE);
        flags |= SENSOR_FLAG_WAKE_UP;
    }

    setResolution((float)((float)sensor_datatype->Resolution *
                        UNIT_CONVERT_MAGNETIC_FIELD));
    setMaxRange((float)((float)sensor_datatype->MaxRange *
                        UNIT_CONVERT_MAGNETIC_FIELD));
    setMinFreq(SNS_SMGR_REPORT_RATE_MIN_HZ_V01);
    return;
}

/*===========================================================================
  FUNCTION:  processReportInd
    process the sensor data indicator from the sensor1 type to sensor event type
    Parameters
    @smgr_data : the sensor1 data message from the sensor1 callback
    @sensor_data : the sensor event message that will be send to framework
===========================================================================*/
void Magnetic::processReportInd(sns_smgr_periodic_report_ind_msg_v01* smgr_ind,
            sns_smgr_data_item_s_v01* smgr_data, sensors_event_t &sensor_data)
{
    uint8_t accuracy = 0x06; // bits 1,2 of Item Flag used for accuracy
    UNREFERENCED_PARAMETER(smgr_ind);
    float cal_mag_x = 0.0;
    float cal_mag_y = 0.0;
    float cal_mag_z = 0.0;
    sensor_data.type = SENSOR_TYPE_MAGNETIC_FIELD;
    char mag_halcal[PROPERTY_VALUE_MAX] = "false";

    if(bWakeUp == false) {
        sensor_data.sensor = HANDLE_MAGNETIC_FIELD;
        HAL_LOG_VERBOSE("%s:sensor %s ",__FUNCTION__,
                    Utility::SensorTypeToSensorString(getType()));
    } else {
        sensor_data.sensor = HANDLE_MAGNETIC_FIELD_WAKE_UP;
        HAL_LOG_VERBOSE("%s:sensor %s (wake_up)",__FUNCTION__,
                        Utility::SensorTypeToSensorString(getType()));
    }

    property_get( HAL_PROP_MAGHALCAL, mag_halcal, "false" );
    if (!strncmp("true", mag_halcal, 4)) {
    /* The noise level of calibrated magnetometer  at  Sensor1 layer is greater than 5 mGauss
        The only options are to implement an averaging filter in HAL or a digital filter in the driver*/
        /* Convert from SAE to Android co-ordinates and scale
        x' = y; y' = x; z' = -z; */
        HAL_LOG_INFO( "applying averaging filter in HAL");

        cal_mag_x = (float)((smgr_data->ItemData[1]) * UNIT_CONVERT_MAGNETIC_FIELD);
        cal_mag_y = (float)((smgr_data->ItemData[0]) * UNIT_CONVERT_MAGNETIC_FIELD);
        cal_mag_z = (float)((-smgr_data->ItemData[2]) * UNIT_CONVERT_MAGNETIC_FIELD);

        if( buff_cal_mag_index < CAL_MAG_FILTER_LENGTH ) {
            cal_mag_sum_x += cal_mag_x - buff_cal_mag[0][buff_cal_mag_index];
            cal_mag_sum_y += cal_mag_y - buff_cal_mag[1][buff_cal_mag_index];
            cal_mag_sum_z += cal_mag_z - buff_cal_mag[2][buff_cal_mag_index];

            buff_cal_mag[0][buff_cal_mag_index] = cal_mag_x;
            buff_cal_mag[1][buff_cal_mag_index] = cal_mag_y;
            buff_cal_mag[2][buff_cal_mag_index] = cal_mag_z;
        }
        if( buff_cal_mag_full ) {
            sensor_data.magnetic.x = (float)(cal_mag_sum_x/CAL_MAG_FILTER_LENGTH);
            sensor_data.magnetic.y = (float)(cal_mag_sum_y/CAL_MAG_FILTER_LENGTH);
            sensor_data.magnetic.z = (float)(cal_mag_sum_z/CAL_MAG_FILTER_LENGTH);
        } else {
            sensor_data.magnetic.x = (float)(cal_mag_sum_x/(buff_cal_mag_index+1));
            sensor_data.magnetic.y = (float)(cal_mag_sum_y/(buff_cal_mag_index+1));
            sensor_data.magnetic.z = (float)(cal_mag_sum_z/(buff_cal_mag_index+1));
        }

        buff_cal_mag_index++;
        if( buff_cal_mag_index == CAL_MAG_FILTER_LENGTH )
        {
            buff_cal_mag_index = 0;
            buff_cal_mag_full = true;
        }
    } else {
        sensor_data.magnetic.x =
                (float)(smgr_data->ItemData[1]) * UNIT_CONVERT_MAGNETIC_FIELD;
        sensor_data.magnetic.y =
                    (float)(smgr_data->ItemData[0]) * UNIT_CONVERT_MAGNETIC_FIELD;
        sensor_data.magnetic.z =
                    (float)(-smgr_data->ItemData[2]) * UNIT_CONVERT_MAGNETIC_FIELD;
    }
    HAL_LOG_VERBOSE( "%s: Calibrated Mag, %f, %f, %f",
                __FUNCTION__,
                sensor_data.magnetic.x,
                sensor_data.magnetic.y,
                sensor_data.magnetic.z);

    accuracy &= smgr_data->ItemFlags;
    accuracy >>= 1;
    ((sensors_vec_t*)(sensor_data.data))->status = accuracy;
}

/*===========================================================================
  FUNCTION:  prepareAddMsg
    SMGRSensor::SMGRPrepareAddMsg will call this function and this func will
    fill the item that needed for this type of sensor.
    Parameters
    @buff_req : the sensor1 message buffer
===========================================================================*/
void Magnetic::prepareAddMsg(sns_smgr_buffering_req_msg_v01 **buff_req)
{
    (*buff_req)->Item[0].SensorId = SNS_SMGR_ID_MAG_V01;
    (*buff_req)->Item[0].Decimation = SNS_SMGR_DECIMATION_FILTER_V01;
    char mag_halcal[PROPERTY_VALUE_MAX] = "false";
    /*  The noise level of calibrated magnetometer  at  Sensor1 layer is greater than 5 mGauss
     The only options are to implement an averaging filter in HAL or a digital filter in the driver*/
     property_get( HAL_PROP_MAGHALCAL, mag_halcal, "false" );
     if (!strncmp("true", mag_halcal, 4)) {
         /* The noise level of calibrated magnetometer  at  Sensor1 layer is greater than 5 mGauss
                The only options are to implement an averaging filter in HAL or a digital filter in the driver*/
         uint8_t i = 0, j = 0;
         HAL_LOG_INFO("applying averaging filter in HAL and resetting initial params");
         // flush cal mag filter buffer, reset index
         buff_cal_mag_index = 0;
         for(i = 0; i < CAL_MAG_AXES; i++)
         {
             for(j = 0; j < CAL_MAG_FILTER_LENGTH; j++)
             { buff_cal_mag[i][j] = 0.0f; }
         }
         buff_cal_mag_full = false;
         cal_mag_sum_x = 0.0f;
         cal_mag_sum_y = 0.0f;
         cal_mag_sum_z = 0.0f;
    }
}
