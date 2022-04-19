/*
 Copyright (c) 2015 Qualcomm Technologies, Inc.
 All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.
 */
/**
 @file
 @brief Vehicle Network provider implementation file.

 This file defines VNW message processing classes.
 */

#include <stdio.h>
#include <loc_cfg.h>
#include <slim_os_api.h>
#include <slim_os_log_api.h>
#include "VehicleNetworkProvider.h"
#include "VehicleNetworkMessageProcessor.h"
#include "BitStreamReader.h"
#include <math.h>

#undef SLIM_LOG_LOCAL_MODULE
#define SLIM_LOG_LOCAL_MODULE SLIM_LOG_MOD_VNW
namespace slim
{

/* -------------------------------------------------------------------------- */
VehicleNetworkMessageProcessor::VehicleNetworkMessageProcessor(
    VehicleNetworkProvider* pzParent) :
    m_pzParent(pzParent)
{
}

VehicleNetworkMessageProcessor::~VehicleNetworkMessageProcessor()
{
}

VehicleNetworkMessageProcessor* VehicleNetworkMessageProcessor::CreateProcessor
(
  VehicleNetworkProvider* pzParent,
  SensorConfig* pzCfg,
  slimServiceEnumT eService
)
{
  if(0 == pzParent || 0 == pzCfg)
  {
    return 0;
  }

  uint32_t iVersion = pzCfg->GetVersion();

  SLIM_LOGD("Creating processor for configuration version: %d, service: %d",
      iVersion, (int)eService);

  if (eSLIM_SERVICE_VEHICLE_ACCEL == eService &&
      VehicleNetworkConfiguration::eACCEL_CONFIG_DEFAULT == iVersion)
  {
    return new VehicleNetworkDefaultAccelMP(pzParent,
        static_cast<AccelConfig*>(pzCfg));
  }

  if (eSLIM_SERVICE_VEHICLE_GYRO == eService &&
      VehicleNetworkConfiguration::eGYRO_CONFIG_DEFAULT == iVersion)
  {
    return new VehicleNetworkDefaultGyroMP(pzParent,
        static_cast<GyroConfig*>(pzCfg));
  }

  if (eSLIM_SERVICE_VEHICLE_ODOMETRY == eService &&
      VehicleNetworkConfiguration::eODOMETRY_CONFIG_DEFAULT == iVersion)
  {
    return new VehicleNetworkDefaultOdometryMP(pzParent,
        static_cast<OdometryConfig*>(pzCfg));
  }

  if (eSLIM_SERVICE_VEHICLE_SPEED == eService &&
      VehicleNetworkConfiguration::eSPEED_CONFIG_DEFAULT == iVersion)
  {
    return new VehicleNetworkDefaultSpeedMP(pzParent,
        static_cast<SpeedConfig*>(pzCfg));
  }

  if (eSLIM_SERVICE_VEHICLE_DWS == eService &&
      VehicleNetworkConfiguration::eDWS_CONFIG_DEFAULT == iVersion)
  {
    return new VehicleNetworkDefaultDwsMP(pzParent,
        static_cast<DwsConfig*>(pzCfg));
  }

  if (eSLIM_SERVICE_VEHICLE_GEAR == eService &&
      VehicleNetworkConfiguration::eGEAR_CONFIG_DEFAULT == iVersion)
  {
    return new VehicleNetworkDefaultGearMP(pzParent,
        static_cast<GearConfig*>(pzCfg));
  }

  if (eSLIM_SERVICE_VEHICLE_ACCEL == eService &&
      VehicleNetworkConfiguration::eACCEL_CONFIG_CONF1 == iVersion)
  {
    return new VehicleNetworkCONF1AccelMP(pzParent,
        static_cast<AccelCONF1Config*>(pzCfg));
  }

  if (eSLIM_SERVICE_VEHICLE_GYRO == eService &&
      VehicleNetworkConfiguration::eGYRO_CONFIG_CONF1 == iVersion)
  {
    return new VehicleNetworkCONF1GyroMP(pzParent,
        static_cast<GyroCONF1Config*>(pzCfg));
  }

  if (eSLIM_SERVICE_VEHICLE_ODOMETRY == eService &&
      VehicleNetworkConfiguration::eODOMETRY_CONFIG_CONF1 == iVersion)
  {
    return new VehicleNetworkCONF1OdometryMP(pzParent,
        static_cast<OdometryCONF1Config*>(pzCfg));
  }

  if (eSLIM_SERVICE_VEHICLE_SPEED == eService &&
      VehicleNetworkConfiguration::eSPEED_CONFIG_CONF1 == iVersion)
  {
    return new VehicleNetworkCONF1SpeedMP(pzParent,
        static_cast<SpeedCONF1Config*>(pzCfg));
  }

  if (eSLIM_SERVICE_VEHICLE_DWS == eService &&
      VehicleNetworkConfiguration::eDWS_CONFIG_CONF1 == iVersion)
  {
    return new VehicleNetworkCONF1DwsMP(pzParent,
        static_cast<DwsCONF1Config*>(pzCfg));
  }

  if (eSLIM_SERVICE_VEHICLE_GEAR == eService &&
      VehicleNetworkConfiguration::eGEAR_CONFIG_CONF1 == iVersion)
  {
    return new VehicleNetworkCONF1GearMP(pzParent,
        static_cast<GearCONF1Config*>(pzCfg));
  }

  return 0;
}

void VehicleNetworkMessageProcessor::OnNewCanFrameEvent(CanFrameEvent* pzEvent)
{
  /* Just drop it by default */
  (void) pzEvent;
}

void VehicleNetworkMessageProcessor::OnNewTimerExpiryEvent(
    os_TimerExpiryType* pzEvent)
{
  /* Just drop it by default */
  (void) pzEvent;
}

void VehicleNetworkMessageProcessor::OnNewIpcMessage(slim_IpcMsgT* pzMsg)
{
  /* Just drop it by default */
  (void) pzMsg;
}

uint64_t VehicleNetworkMessageProcessor::VnTimeToApTime(uint64_t tVnTime)
{
  if(0 == m_pzParent)
  {
    return tVnTime;
  }
  double dR = m_pzParent->m_pzVehicleNetworkCfg->GetCanApClockRatio();
  return (uint64_t)((double)tVnTime / (0 == dR ? 1.0 : dR) );
}

void VehicleNetworkMessageProcessor::UpdateCanControllerTimeDelta
(
    uint64_t tMCTime
)
{
  if(0 == m_pzParent)
  {
    return;
  }

  m_pzParent->m_tLastBusControllerSyncTime = slim_TimeTickGetMilliseconds();
  m_pzParent->m_tLastBusControllerSyncTimestamp = VnTimeToApTime(tMCTime);
  SLIM_LOGD("Time delta update: %"PRIu64", %"PRIu64", %"PRIu64,
      m_pzParent->m_tLastBusControllerSyncTime,
      m_pzParent->m_tLastBusControllerSyncTimestamp,
      tMCTime);
}

/**
@brief Checks if Vehicle Sensor data once batch size/time is reached.

This method Checks if Vehicle Sensor data once batch size/time is reached.

@param[in] eService     Vehicle Service Type
@param[in] iSamplesLen  Numbe of samples received
@param[in] tFrameTime   Time corresponding to current CAN frame
@param[in] iBatchType   Batch Type (Time based / Count based)
@param[in] iBatchValue  Batch value to be compared with
                        either number of samples or time in milli seconds
*/
bool VehicleNetworkMessageProcessor::IsBatchCompleted
(
  slimServiceEnumT eService,
  int iSamplesLen,
  uint64_t tFrameTime,
  int iBatchType,
  int iBatchValue
)
{
  uint64_t tCurrentTimestamp = 0;
  bool batchComplete = false;
  tCurrentTimestamp = slim_TimeTickGetMilliseconds();
  if (SLIM_VNW_BATCH_TYPE_TIME == iBatchType)
  {/* Time Based Batching configured */

    if (tCurrentTimestamp >= m_tLastCANPktTime+iBatchValue ||
       iSamplesLen >= SLIM_VEHICLE_MOTION_DATA_MAX_SAMPLE_SETS)
    {
      /* Batch time completed, send data to client */

      SLIM_LOGD("IsBatchCompleted eService = %d iBatchType = %d  iBatchValue = %d \
           m_tLastCANPktTime=%" PRIu64 " ms. tCurrentTimestamp = %" PRIu64 " ms. \
           samples_len = %d. FrameTime =%" PRIu64 " ms. \n",eService, iBatchType, iBatchValue, \
           m_tLastCANPktTime, tCurrentTimestamp, iSamplesLen, tFrameTime);
      m_tLastCANPktTime = tFrameTime;
      batchComplete = true;
    }
  }
  else
  {/* Count based Batching configured */
    if (iSamplesLen >= iBatchValue ||
       iSamplesLen >= SLIM_VEHICLE_MOTION_DATA_MAX_SAMPLE_SETS)
    {
      /* Batch count reached, send data to client */
      SLIM_LOGD("routeIndication eService = %d iBatchType = %d  iBatchValue = %d \
           m_tLastCANPktTime=%" PRIu64 " ms. tCurrentTimestamp = %" PRIu64 " ms. \
           sensorData.samples_len = %d. FrameTime =%" PRIu64 " ms. \n",eService, iBatchType, iBatchValue, \
           m_tLastCANPktTime, tCurrentTimestamp, iSamplesLen, tFrameTime);
      m_tLastCANPktTime = tFrameTime;
      batchComplete = true;
    }
  }
  return batchComplete;
}


/* -------------------------------------------------------------------------- */
VehicleNetworkCONF1AccelMP::VehicleNetworkCONF1AccelMP(
    VehicleNetworkProvider* pzParent, AccelCONF1Config* pzCfg) :
    VehicleNetworkMessageProcessor(pzParent), m_pzCfg(pzCfg)
{
  mZData.timeSource = eSLIM_TIME_SOURCE_UNSPECIFIED;
  mZData.flags = 0;
  mZData.provider = eSLIM_SERVICE_PROVIDER_NATIVE;
  mZData.sensorType = eSLIM_VEHICLE_SENSOR_TYPE_ACCEL;
  mZData.axesValidity = 0;
  mZData.axesValidity |= SLIM_MASK_VEHICLE_SENSOR_X_AXIS;
  mZData.axesValidity |= SLIM_MASK_VEHICLE_SENSOR_Y_AXIS;
  mZData.axesValidity |= SLIM_MASK_VEHICLE_SENSOR_Z_AXIS;
  mZData.samples_len = 0;
  m_tLastCANPktTime = 0;
}

void VehicleNetworkCONF1AccelMP::OnNewCanFrameEvent(CanFrameEvent* pzEvent)
{
  CwFrame* pzFrame = pzEvent->GetFrame();
  if (0 == pzFrame)
  {
    return;
  }

  if (0 < m_pzCfg->m_iFrameId
      && pzFrame->getId() == (uint32_t) m_pzCfg->m_iFrameId)
  {
    if (0
        != SLIM_MASK_IS_SET(m_pzParent->m_qEnabledServices,
            eSLIM_SERVICE_VEHICLE_ACCEL))
    {
      SLIM_LOGD("Accel CONF1 MP, event:%p", pzEvent);

      /* Update difference between AP and CAN bus mictocontroller clock.
       * TODO: This code has to be moved to processing of DR_SYNC CAN frame */
      UpdateCanControllerTimeDelta(pzFrame->getTimestamp());

      BitDivider bitStream(pzFrame->getDataPtr(), pzFrame->getDataLen());
      float x_Value  = 0;
      if( 0 <= m_pzCfg->m_iLenX )
      {
          bitStream.SetPos(m_pzCfg->m_iOffsetX);
          if (bitStream.GetStatus() != 0)
            return;
          x_Value = (float) (BitDivider::ExpandSign(
              bitStream.Get(m_pzCfg->m_iLenX), m_pzCfg->m_iLenX)
              * m_pzCfg->m_fScalingX);
          if (bitStream.GetStatus() != 0)
            return;
      }
      else
      {
          int changedLen = -1 * m_pzCfg->m_iLenX;
          int valRead=0, valSwap =0;

          bitStream.SetPos(m_pzCfg->m_iOffsetX);
          if (bitStream.GetStatus() != 0)
            return;

          valRead = bitStream.Get(changedLen);
          valSwap = ( (valRead & 0xFF) << (changedLen/2) )
                    | ( (valRead >>(changedLen/2)) & 0xFF);

          x_Value = (float) (BitDivider::ExpandSign(
                    valSwap, changedLen) * m_pzCfg->m_fScalingX);

          SLIM_LOGD("valRead 0x%X valSwap 0x%X xVal %f", valRead, valSwap, x_Value );

          if (bitStream.GetStatus() != 0)
            return;
      }

      float y_Value = 0;
      if( 0 <= m_pzCfg->m_iLenY )
      {
          bitStream.SetPos(m_pzCfg->m_iOffsetY);
          if (bitStream.GetStatus() != 0)
            return;
          y_Value = (float) (BitDivider::ExpandSign(
              bitStream.Get(m_pzCfg->m_iLenY), m_pzCfg->m_iLenY)
              * m_pzCfg->m_fScalingY);
          if (bitStream.GetStatus() != 0)
            return;
      }
      else
      {
          int changedLen = -1 * m_pzCfg->m_iLenY;
          int valRead=0, valSwap =0;

          bitStream.SetPos(m_pzCfg->m_iOffsetY);
          if (bitStream.GetStatus() != 0)
            return;

          valRead = bitStream.Get(changedLen);
          valSwap = ( (valRead & 0xFF) << (changedLen/2) )
                | ( (valRead >>(changedLen/2)) & 0xFF);

          y_Value = (float) (BitDivider::ExpandSign(
                    valSwap, changedLen) * m_pzCfg->m_fScalingY);

          SLIM_LOGD("valRead 0x%X valSwap 0x%X xVal %f", valRead, valSwap, y_Value );

          if (bitStream.GetStatus() != 0)
            return;
      }

      float z_Value = 0;
      if( 0 <= m_pzCfg->m_iLenZ )
      {
          bitStream.SetPos(m_pzCfg->m_iOffsetZ);
          if (bitStream.GetStatus() != 0)
            return;
          z_Value = (float) (BitDivider::ExpandSign(
              bitStream.Get(m_pzCfg->m_iLenZ), m_pzCfg->m_iLenZ)
              * m_pzCfg->m_fScalingZ);
          if (bitStream.GetStatus() != 0)
            return;
      }
      else
      {
          int changedLen = -1 * m_pzCfg->m_iLenZ;
          int valRead=0, valSwap =0;

          bitStream.SetPos(m_pzCfg->m_iOffsetZ);
          if (bitStream.GetStatus() != 0)
            return;

          valRead = bitStream.Get(changedLen);
          valSwap = ( (valRead & 0xFF) << (changedLen/2) )
                | ( (valRead >>(changedLen/2)) & 0xFF);

          z_Value = (float) (BitDivider::ExpandSign(
                    valSwap, changedLen) * m_pzCfg->m_fScalingZ);

          SLIM_LOGD("valRead 0x%X valSwap 0x%X xVal %f", valRead, valSwap, z_Value );

          if (bitStream.GetStatus() != 0)
             return;
      }

      SLIM_LOGV("accel samples_len %d, mIsBuffFrame %d FrameTime %d",
                 mZData.samples_len,
                 pzEvent->mIsBuffFrame,
                 pzFrame->getTimestamp());
      /* First process buffer data in batches */
      if (pzEvent->mIsBuffFrame) {
        if (mZData.samples_len == 0) {
          mZData.timeBase = VnTimeToApTime(pzFrame->getTimestamp());
          mZData.samples[0].sampleTimeOffset = 0;
        } else {
          mZData.samples[mZData.samples_len].sampleTimeOffset =
              SLIM_CONVERT_MSEC_TO_USEC(VnTimeToApTime(pzFrame->getTimestamp()) - mZData.timeBase);
        }
        mZData.samples[mZData.samples_len].sample[0] = x_Value;
        mZData.samples[mZData.samples_len].sample[1] = y_Value;
        mZData.samples[mZData.samples_len].sample[2] = z_Value;
        mZData.samples_len += 1;
        if (mZData.samples_len == SLIM_VEHICLE_SENSOR_MAX_SAMPLE_SETS) {
          m_pzParent->routeIndication(eSLIM_SERVICE_VEHICLE_ACCEL, mZData);
          /* sleep for 2 ms to allow every batch of data sent to application */
          usleep(TIME_BW_BUFFERED_DATA_BATCHES);
          mZData.samples_len = 0;
        }
      } else {
        /* Process live data*/
        if (0 == mZData.samples_len)
        {
          mZData.timeBase = VnTimeToApTime(pzFrame->getTimestamp());
          mZData.samples[0].sampleTimeOffset = 0;
        }
        else
        {
          mZData.samples[mZData.samples_len].sampleTimeOffset =
                 SLIM_CONVERT_MSEC_TO_USEC( VnTimeToApTime(pzFrame->getTimestamp()) - mZData.timeBase);
        }
        mZData.samples[mZData.samples_len].sample[0] = x_Value;
        mZData.samples[mZData.samples_len].sample[1] = y_Value;
        mZData.samples[mZData.samples_len].sample[2] = z_Value;
        mZData.samples_len++;
        if (true == IsBatchCompleted (eSLIM_SERVICE_VEHICLE_ACCEL,
                                               mZData.samples_len,
                                               pzFrame->getTimestamp(),
                                               m_pzCfg->m_iBatchType,
                                               m_pzCfg->m_iBatchSize))
        {
          m_pzParent->routeIndication(eSLIM_SERVICE_VEHICLE_ACCEL, mZData);
          /* Re initialize sample length and Last CAN packet time*/
          mZData.samples_len = 0;
          mZData.samples[0].sampleTimeOffset = 0;
        }
      }
    }
  }
}

void VehicleNetworkCONF1AccelMP::ToString(char* pStr, size_t sLen)
{
  snprintf(pStr, sLen, "[VehicleNetworkCONF1AccelMP]");
}

/* -------------------------------------------------------------------------- */
VehicleNetworkDefaultAccelMP::VehicleNetworkDefaultAccelMP(
    VehicleNetworkProvider* pzParent, AccelConfig* pzCfg) :
    VehicleNetworkMessageProcessor(pzParent), m_pzCfg(pzCfg)
{
}

void VehicleNetworkDefaultAccelMP::OnNewTimerExpiryEvent(
    os_TimerExpiryType* pzEvent)
{
  VehicleNetworkMessageProcessor::OnNewTimerExpiryEvent(pzEvent);
  if (0
      != SLIM_MASK_IS_SET(m_pzParent->m_qEnabledServices,
          eSLIM_SERVICE_VEHICLE_ACCEL))
  {
    slimVehicleSensorDataStructT zData;
    memset(&zData, 0, sizeof(zData));
    zData.timeBase = slim_TimeTickGetMilliseconds();
    zData.timeSource = eSLIM_TIME_SOURCE_COMMON;
    zData.flags = 0;
    zData.provider = eSLIM_SERVICE_PROVIDER_NATIVE;
    zData.sensorType = eSLIM_VEHICLE_SENSOR_TYPE_ACCEL;
    zData.axesValidity = 0;
    zData.axesValidity |= SLIM_MASK_VEHICLE_SENSOR_X_AXIS;
    zData.axesValidity |= SLIM_MASK_VEHICLE_SENSOR_Y_AXIS;
    zData.axesValidity |= SLIM_MASK_VEHICLE_SENSOR_Z_AXIS;
    zData.samples_len = 1;
    zData.samples[0].sampleTimeOffset = 10;
    zData.samples[0].sample[0] = .5f;
    zData.samples[0].sample[1] = .3f;
    zData.samples[0].sample[2] = -.2f;
    m_pzParent->routeIndication(eSLIM_SERVICE_VEHICLE_ACCEL, zData);
  }
}

void VehicleNetworkDefaultAccelMP::ToString(char* pStr, size_t sLen)
{
  snprintf(pStr, sLen, "[VehicleNetworkDefaultAccelMP]");
}

/* -------------------------------------------------------------------------- */
VehicleNetworkDefaultGyroMP::VehicleNetworkDefaultGyroMP(
    VehicleNetworkProvider* pzParent, GyroConfig* pzCfg) :
    VehicleNetworkMessageProcessor(pzParent), m_pzCfg(pzCfg)
{
}

void VehicleNetworkDefaultGyroMP::OnNewTimerExpiryEvent(
    os_TimerExpiryType* pzEvent)
{
  VehicleNetworkMessageProcessor::OnNewTimerExpiryEvent(pzEvent);
  if (0
      != SLIM_MASK_IS_SET(m_pzParent->m_qEnabledServices,
          eSLIM_SERVICE_VEHICLE_GYRO))
  {
    slimVehicleSensorDataStructT zData;
    memset(&zData, 0, sizeof(zData));
    zData.timeBase = slim_TimeTickGetMilliseconds();
    zData.timeSource = eSLIM_TIME_SOURCE_COMMON;
    zData.flags = 0;
    zData.provider = eSLIM_SERVICE_PROVIDER_NATIVE;
    zData.sensorType = eSLIM_VEHICLE_SENSOR_TYPE_GYRO;
    zData.axesValidity = 0;
    zData.axesValidity |= SLIM_MASK_VEHICLE_SENSOR_X_AXIS;
    zData.axesValidity |= SLIM_MASK_VEHICLE_SENSOR_Y_AXIS;
    zData.axesValidity |= SLIM_MASK_VEHICLE_SENSOR_Z_AXIS;
    zData.samples_len = 1;
    zData.samples[0].sampleTimeOffset = 10;
    zData.samples[0].sample[0] = .3f;
    zData.samples[0].sample[1] = .5f;
    zData.samples[0].sample[2] = -.1f;
    m_pzParent->routeIndication(eSLIM_SERVICE_VEHICLE_GYRO, zData);
  }
}

void VehicleNetworkDefaultGyroMP::ToString(char* pStr, size_t sLen)
{
  //TODO: output full configuration
  snprintf(pStr, sLen, "[VehicleNetworkDefaultGyroMP]");
}

/* -------------------------------------------------------------------------- */
VehicleNetworkCONF1GyroMP::VehicleNetworkCONF1GyroMP(
    VehicleNetworkProvider* pzParent, GyroCONF1Config* pzCfg) :
    VehicleNetworkMessageProcessor(pzParent), m_pzCfg(pzCfg)
{
  mZData.timeSource = eSLIM_TIME_SOURCE_UNSPECIFIED;
  mZData.flags = 0;
  mZData.provider = eSLIM_SERVICE_PROVIDER_NATIVE;
  mZData.sensorType = eSLIM_VEHICLE_SENSOR_TYPE_GYRO;
  mZData.axesValidity = 0;
  mZData.axesValidity |= SLIM_MASK_VEHICLE_SENSOR_X_AXIS;
  mZData.axesValidity |= SLIM_MASK_VEHICLE_SENSOR_Y_AXIS;
  mZData.axesValidity |= SLIM_MASK_VEHICLE_SENSOR_Z_AXIS;
  mZData.samples_len = 0;
  m_tLastCANPktTime = 0;
}

void BitModify:: ChangeBit(long long *pInput, long long off, long long bit, int NBits)
{
  if (NULL == pInput) {
    return;
  }
  long long k = bit << (off * NBits);
  long long m = 0xFF;
  m = m << (off * NBits);
  *pInput &= ~m;
  *pInput = *pInput | k;
}

void BitModify::ReverseBits(long long *pInput, long long LBitPos, long long RBitPos, int NBits)
{
  if (NULL == pInput) {
    return;
  }
  long long LoopCntL,LoopCntR;
  int RBit ,LBit;
  for ( LoopCntL= LBitPos,LoopCntR = RBitPos;  LoopCntL < LoopCntR;  LoopCntL++, LoopCntR--)
  {

    RBit = ((*pInput) >> (LoopCntR * NBits))&0xFF;
    LBit = ((*pInput) >> (LoopCntL * NBits))&0xFF;
    ChangeBit(pInput, LoopCntL, RBit, NBits);
    ChangeBit(pInput, LoopCntR, LBit, NBits);
  }
}

void VehicleNetworkCONF1GyroMP::CheckForError(float InputVal, GyroCONF1Config * m_pzCfg,boolean * pErrorFound)
{
  if (NULL == m_pzCfg || NULL ==pErrorFound) {
  return;
  }
  for (int loopCnt=0; loopCnt<m_pzCfg->m_iNumAbsError; loopCnt++)
  {
    if(InputVal == m_pzCfg->m_pfAbsErrors[loopCnt]) {
      *pErrorFound=true;
      SLIM_LOGD("G-into A error no. %d , val = %f, abs error - %f",loopCnt,InputVal,m_pzCfg->m_pfAbsErrors[loopCnt]);
      return;
    }
  }
  if ( InputVal > m_pzCfg->m_fGyroMax || InputVal < m_pzCfg->m_fGyroMin ) {
    *pErrorFound = true;
    SLIM_LOGD("G-into Range error  val = %f,min  value - %f, max value -%f",InputVal,m_pzCfg->m_fGyroMin,m_pzCfg->m_fGyroMax);
    return;
  }

}
void VehicleNetworkCONF1GyroMP::OnNewCanFrameEvent(CanFrameEvent* pzEvent)
{
  if (NULL == pzEvent) {
    return;
  }
  CwFrame* pzFrame = pzEvent->GetFrame();
  int LoopCnt;
  if (0 == pzFrame){
    return;
  }
  float g_Value[3]={0.0F};

  boolean ErrorFound=false;

  long long BWErrorCount=0;


  SLIM_LOGD("G-Into the processing fun %d %d %d",SLIM_MASK_IS_SET(m_pzParent->m_qEnabledServices,
        eSLIM_SERVICE_VEHICLE_GYRO),m_pzCfg->m_iFrameId,pzFrame->getId());

  if (0 < m_pzCfg->m_iFrameId
      && pzFrame->getId() == (uint32_t) m_pzCfg->m_iFrameId)
  {
    if (0
        != SLIM_MASK_IS_SET(m_pzParent->m_qEnabledServices,
          eSLIM_SERVICE_VEHICLE_GYRO))
    {
      SLIM_LOGD("Gyro CONF1 MP, event:%p", pzEvent);

      /* Update difference between AP and CAN bus mictocontroller clock.
       * TODO: This code has to be moved to processing of DR_SYNC CAN frame */
      UpdateCanControllerTimeDelta(pzFrame->getTimestamp());

      BitDivider bitStream(pzFrame->getDataPtr(), pzFrame->getDataLen());

      bitStream.SetPos(m_pzCfg->m_iSignStartBit);
      long long SignBitWise = bitStream.Get(m_pzCfg->m_iSignLength);

      for ( LoopCnt=0; LoopCnt < m_pzCfg->m_iNumBitWiseError; LoopCnt++)
      {
        bitStream.SetPos(m_pzCfg->m_pzBitWiseErrors[LoopCnt].m_iBitWiseErrorStartBit);
        long long temp=bitStream.Get(m_pzCfg->m_pzBitWiseErrors[LoopCnt].m_iBitWiseErrorLength);
        if( temp != m_pzCfg->m_pzBitWiseErrors[LoopCnt].m_iBitWiseErrorValue)
        {
          SLIM_LOGD("G-into D Error value no. %d ,D error- %d, temp = %d",LoopCnt,m_pzCfg->m_pzBitWiseErrors[LoopCnt].m_iBitWiseErrorValue,temp);
          return;
        }
      }

      for ( LoopCnt=0; LoopCnt < msGYRO_VEL_AX ; LoopCnt++)
      {
        long long valRead=0, valSwap =0,length;
        if( 0 <= m_pzCfg->m_zGyroConfig[LoopCnt].m_iLen )
        {
          bitStream.SetPos(m_pzCfg->m_zGyroConfig[LoopCnt].m_iOffset);
          if (bitStream.GetStatus() != 0)
            return;

          length = m_pzCfg->m_zGyroConfig[LoopCnt].m_iLen;
          if (m_pzCfg->m_iByteOrder)
          {
            length = ceil((float)m_pzCfg->m_zGyroConfig[LoopCnt].m_iLen/8)*8;
          }

          valRead = bitStream.Get(length);
          if (bitStream.GetStatus() != 0 &&  m_pzCfg->m_zGyroConfig[LoopCnt].m_iLen !=0 )
            return;
        }
        else
        {
          length = -1 * m_pzCfg->m_zGyroConfig[LoopCnt].m_iLen;


          if (m_pzCfg->m_iByteOrder)
          {
            length = -1 * ceil((float)m_pzCfg->m_zGyroConfig[LoopCnt].m_iLen/8)*8;
          }
          bitStream.SetPos(m_pzCfg->m_zGyroConfig[LoopCnt].m_iOffset);
          if (bitStream.GetStatus() != 0)
            return;

          valRead = bitStream.Get(length);
          valSwap = ( (valRead & 0xFF) << (length / 2) )
            | ( (valRead >> (length / 2)) & 0xFF);


          SLIM_LOGD("valRead 0x%X valSwap 0x%X %dVal %f", valRead, valSwap,LoopCnt, g_Value[LoopCnt] );
          valRead = valSwap;

          if (bitStream.GetStatus() != 0 &&  m_pzCfg->m_zGyroConfig[LoopCnt].m_iLen != 0 )
            return;
        }

        int NBits = 8; //Chunk of 8.

        if (m_pzCfg->m_iByteOrder)
        {
          long long  mask = 0, remainderBits = m_pzCfg->m_zGyroConfig[LoopCnt].m_iLen%8;
          if ( remainderBits != 0)
          {
            mask = pow(2,8-remainderBits)-1;
          }
          mask <<= remainderBits;
          valRead &= ~mask;


          SLIM_LOGD("Entering reverse byte order valRead 0x%X Length- %d", valRead,m_pzCfg->m_zGyroConfig[LoopCnt].m_iLen);
          BitModify::ReverseBits(&valRead,0,ceil((float)m_pzCfg->m_zGyroConfig[LoopCnt].m_iLen/NBits)-1,NBits);
          SLIM_LOGD("Exiting reverse byte order valRead 0x%X ", valRead);
        }

        if((m_pzCfg->m_iSign && m_pzCfg->m_iSignLength==0) || (SignBitWise != m_pzCfg->m_iSignVal && m_pzCfg->m_iSignLength > 0))
        {
          g_Value[LoopCnt] = (float)(valRead * m_pzCfg->m_zGyroConfig[LoopCnt].m_fScaling);
        }
        else
        {
          g_Value[LoopCnt] = (float) (BitDivider::ExpandSign(
                valRead,length )
              * m_pzCfg->m_zGyroConfig[LoopCnt].m_fScaling);
        }


        g_Value[LoopCnt] += m_pzCfg->m_fAdditiveOffset;

        CheckForError(g_Value[LoopCnt],m_pzCfg,&ErrorFound);
        if (ErrorFound == true)
        {
          return;
        }

        g_Value[LoopCnt] *= m_pzCfg->m_fScalingOffset;  //QC SW interface's scale

        if (m_pzCfg->m_iUnit)                       // if deg/sec convert it.
        {
          g_Value[LoopCnt]*=(M_PI/180);
        }
        SLIM_LOGD("G-into value no. %d = %f",LoopCnt,g_Value[LoopCnt]);
      }

      SLIM_LOGV("gyro samples_len %d, mIsBuffFrame %d FrameTime %d",
          mZData.samples_len,
          pzEvent->mIsBuffFrame,
          pzFrame->getTimestamp());
      /* First process buffer data in batches */
      if (pzEvent->mIsBuffFrame) {
        if (mZData.samples_len == 0) {
          mZData.timeBase = VnTimeToApTime(pzFrame->getTimestamp());
          mZData.samples[0].sampleTimeOffset = 0;
        } else {
          mZData.samples[mZData.samples_len].sampleTimeOffset =
            SLIM_CONVERT_MSEC_TO_USEC(VnTimeToApTime(pzFrame->getTimestamp()) - mZData.timeBase);
        }
        memscpy(mZData.samples[0].sample,sizeof(mZData.samples[0].sample),g_Value,sizeof(g_Value));
        mZData.samples_len += 1;
        if (mZData.samples_len == SLIM_VEHICLE_SENSOR_MAX_SAMPLE_SETS) {
          m_pzParent->routeIndication(eSLIM_SERVICE_VEHICLE_GYRO, mZData);
          /* sleep for 2 ms to allow every batch of data sent to application */
          usleep(TIME_BW_BUFFERED_DATA_BATCHES);
          mZData.samples_len = 0;
        }
      } else {
        /* Process live data*/
        if (0 == mZData.samples_len)
        {
          mZData.timeBase = VnTimeToApTime(pzFrame->getTimestamp());
          mZData.samples[0].sampleTimeOffset = 0;
        }
        else
        {
          mZData.samples[mZData.samples_len].sampleTimeOffset =
                 SLIM_CONVERT_MSEC_TO_USEC( VnTimeToApTime(pzFrame->getTimestamp()) - mZData.timeBase );
        }

        memscpy(mZData.samples[mZData.samples_len].sample,sizeof(mZData.samples[mZData.samples_len].sample),g_Value,sizeof(g_Value));
        mZData.samples_len++;
        if (true == IsBatchCompleted (eSLIM_SERVICE_VEHICLE_GYRO,
                                               mZData.samples_len,
                                               pzFrame->getTimestamp(),
                                               m_pzCfg->m_iBatchType,
                                               m_pzCfg->m_iBatchSize))
        {
          m_pzParent->routeIndication(eSLIM_SERVICE_VEHICLE_GYRO, mZData);
          /* Re initialize sample length and Last CAN packet time*/
          mZData.samples_len = 0;
          mZData.samples[0].sampleTimeOffset = 0;
        }
      }
    }
  }
}

void VehicleNetworkCONF1GyroMP::ToString(char* pStr, size_t sLen)
{
  snprintf(pStr, sLen, "[VehicleNetworkCONF1GyroMP]");
}

/* -------------------------------------------------------------------------- */
VehicleNetworkDefaultOdometryMP::VehicleNetworkDefaultOdometryMP(
    VehicleNetworkProvider* pzParent, OdometryConfig* pzCfg) :
    VehicleNetworkMessageProcessor(pzParent), m_pzCfg(pzCfg)
{
}

void VehicleNetworkDefaultOdometryMP::OnNewTimerExpiryEvent(
    os_TimerExpiryType* pzEvent)
{
  VehicleNetworkMessageProcessor::OnNewTimerExpiryEvent(pzEvent);
  if (0
      != SLIM_MASK_IS_SET(m_pzParent->m_qEnabledServices,
          eSLIM_SERVICE_VEHICLE_ODOMETRY))
  {
    slimVehicleOdometryDataStructT zData;
    memset(&zData, 0, sizeof(zData));
    zData.timeBase = slim_TimeTickGetMilliseconds();
    zData.timeSource = eSLIM_TIME_SOURCE_COMMON;
    zData.provider = eSLIM_SERVICE_PROVIDER_NATIVE;
    zData.wheelFlags = 0;
    zData.wheelFlags |= SLIM_MASK_VEHICLE_ODOMETRY_LEFT_AND_RIGHT_AVERAGE;
    zData.wheelFlags |= SLIM_MASK_VEHICLE_ODOMETRY_LEFT;
    zData.wheelFlags |= SLIM_MASK_VEHICLE_ODOMETRY_RIGHT;
    zData.samples_len = 1;
    zData.samples[0].sampleTimeOffset = 10;
    zData.samples[0].distanceTravelled[0] = 1;
    zData.samples[0].distanceTravelled[1] = 2;
    zData.samples[0].distanceTravelled[2] = 3;
    m_pzParent->routeIndication(zData);
  }
}

void VehicleNetworkDefaultOdometryMP::ToString(char* pStr, size_t sLen)
{
  snprintf(pStr, sLen, "[VehicleNetworkDefaultOdometryMP]");
}

/* -------------------------------------------------------------------------- */
VehicleNetworkCONF1OdometryMP::VehicleNetworkCONF1OdometryMP(
    VehicleNetworkProvider* pzParent, OdometryCONF1Config* pzCfg) :
    VehicleNetworkMessageProcessor(pzParent), m_pzCfg(pzCfg)
{
  mZData.timeSource = eSLIM_TIME_SOURCE_UNSPECIFIED;
  mZData.provider = eSLIM_SERVICE_PROVIDER_NATIVE;
  mZData.distanceTravelledBase = 0;
  mZData.odometryFlags = 0;
  mZData.wheelFlags = 0;
  mZData.samples_len = 0;
  m_tLastCANPktTime = 0;
}

void VehicleNetworkCONF1OdometryMP::OnNewCanFrameEvent(CanFrameEvent* pzEvent)
{
  CwFrame* pzFrame = pzEvent->GetFrame();
  if (0 == pzFrame)
  {
    return;
  }

  if (0 < m_pzCfg->m_iFrameId
      && pzFrame->getId() == (uint32_t) m_pzCfg->m_iFrameId)
  {
    if (0
        != SLIM_MASK_IS_SET(m_pzParent->m_qEnabledServices,
            eSLIM_SERVICE_VEHICLE_ODOMETRY))
    {
      /* Update difference between AP and CAN bus mictocontroller clock.
       * TODO: This code has to be moved to processing of DR_SYNC CAN frame */
      UpdateCanControllerTimeDelta(pzFrame->getTimestamp());

      BitDivider bitStream(pzFrame->getDataPtr(), pzFrame->getDataLen());
      bitStream.SetPos(m_pzCfg->m_iReverseBitOffset);
      if (bitStream.GetStatus() != 0)
        return;
      uint32_t reverse = bitStream.Get(1);
      if (bitStream.GetStatus() != 0)
        return;
      bitStream.SetPos(m_pzCfg->m_iSourceOffset);
      if (bitStream.GetStatus() != 0)
        return;
      uint32_t src = bitStream.Get(m_pzCfg->m_iSourceLen);
      if (bitStream.GetStatus() != 0)
        return;
      bitStream.SetPos(m_pzCfg->m_iDistanceOffset);
      if (bitStream.GetStatus() != 0)
        return;
      uint32_t distance = bitStream.Get(m_pzCfg->m_iDistanceLen);
      if (bitStream.GetStatus() != 0)
        return;

      SLIM_LOGV("odo samples_len %d, mIsBuffFrame %d FrameTime %d",
                 mZData.samples_len,
                 pzEvent->mIsBuffFrame,
                 pzFrame->getTimestamp());
      /* First process buffer data in batches */
      if (pzEvent->mIsBuffFrame) {
        if (mZData.samples_len == 0) {
          mZData.timeBase = VnTimeToApTime(pzFrame->getTimestamp());
          mZData.samples[0].sampleTimeOffset = 0;
        } else {
          mZData.samples[mZData.samples_len].sampleTimeOffset =
              SLIM_CONVERT_MSEC_TO_USEC(VnTimeToApTime(pzFrame->getTimestamp()) - mZData.timeBase);
        }

        if (reverse != 0) {
          mZData.odometryFlags |= SLIM_MASK_VEHICLE_ODOMETRY_REVERSE_MOVEMENT;
        }
        if (src & OdometryCONF1Config::eODO_CFG1_SRC_LEFT) {
          mZData.wheelFlags |= SLIM_MASK_VEHICLE_ODOMETRY_LEFT;
          mZData.samples[mZData.samples_len].distanceTravelled[1] = distance;
        } else if (src & OdometryCONF1Config::eODO_CFG1_SRC_RIGHT) {
          mZData.wheelFlags |= SLIM_MASK_VEHICLE_ODOMETRY_RIGHT;
          mZData.samples[mZData.samples_len].distanceTravelled[2] = distance;
        } else if (src & OdometryCONF1Config::eODO_CFG1_SRC_LR_AVR) {
          mZData.wheelFlags |= SLIM_MASK_VEHICLE_ODOMETRY_LEFT_AND_RIGHT_AVERAGE;
          mZData.samples[mZData.samples_len].distanceTravelled[0] = distance;
        }
        if (mZData.wheelFlags != 0)
          mZData.samples_len += 1;

        if (mZData.samples_len == SLIM_VEHICLE_ODOMETRY_MAX_SAMPLE_SETS) {
          m_pzParent->routeIndication(mZData);
          /* sleep for 2 ms to allow every batch of data sent to application */
          usleep(TIME_BW_BUFFERED_DATA_BATCHES);
          mZData.samples_len = 0;
        }
      } else {
        /* Process live data*/
        if (0 == mZData.samples_len) {
          mZData.odometryFlags = 0;
          mZData.wheelFlags = 0;
          mZData.timeBase = VnTimeToApTime(pzFrame->getTimestamp());
          mZData.samples[0].sampleTimeOffset = 0;
        }
        if (reverse != 0) {
          mZData.odometryFlags |= SLIM_MASK_VEHICLE_ODOMETRY_REVERSE_MOVEMENT;
        }
        if (src & OdometryCONF1Config::eODO_CFG1_SRC_LEFT) {
          mZData.wheelFlags |= SLIM_MASK_VEHICLE_ODOMETRY_LEFT;
          mZData.samples[mZData.samples_len].distanceTravelled[1] = distance;
        } else if (src & OdometryCONF1Config::eODO_CFG1_SRC_RIGHT) {
          mZData.wheelFlags |= SLIM_MASK_VEHICLE_ODOMETRY_RIGHT;
          mZData.samples[mZData.samples_len].distanceTravelled[2] = distance;
        } else if (src & OdometryCONF1Config::eODO_CFG1_SRC_LR_AVR) {
          mZData.wheelFlags |= SLIM_MASK_VEHICLE_ODOMETRY_LEFT_AND_RIGHT_AVERAGE;
          mZData.samples[mZData.samples_len].distanceTravelled[0] = distance;
        }

        if (0 != mZData.wheelFlags) {
          mZData.samples_len++;
          mZData.samples[0].sampleTimeOffset =
                 SLIM_CONVERT_MSEC_TO_USEC( VnTimeToApTime(pzFrame->getTimestamp()) - mZData.timeBase);
        }
        if (true == IsBatchCompleted (eSLIM_SERVICE_VEHICLE_ODOMETRY,
                                               mZData.samples_len,
                                               pzFrame->getTimestamp(),
                                               m_pzCfg->m_iBatchType,
                                               m_pzCfg->m_iBatchSize))
       {
          m_pzParent->routeIndication(mZData);
          mZData.samples_len = 0;
          mZData.samples[0].sampleTimeOffset = 0;
       }
      }
    }
  }
}

void VehicleNetworkCONF1OdometryMP::ToString(char* pStr, size_t sLen)
{
  snprintf(pStr, sLen, "[VehicleNetworkCONF1OdometryMP]");
}


/* -------------------------------------------------------------------------- */
VehicleNetworkDefaultSpeedMP::VehicleNetworkDefaultSpeedMP(
    VehicleNetworkProvider* pzParent, SpeedConfig* pzCfg) :
    VehicleNetworkMessageProcessor(pzParent), m_pzCfg(pzCfg)
{
}

void VehicleNetworkDefaultSpeedMP::OnNewTimerExpiryEvent(
    os_TimerExpiryType* pzEvent)
{
  VehicleNetworkMessageProcessor::OnNewTimerExpiryEvent(pzEvent);
  if (0
      != SLIM_MASK_IS_SET(m_pzParent->m_qEnabledServices,
          eSLIM_SERVICE_VEHICLE_SPEED))
  {
    slimVehicleMotionDataStructT zData;
    memset(&zData, 0, sizeof(zData));
    zData.timeBase = slim_TimeTickGetMilliseconds();
    zData.timeSource = eSLIM_TIME_SOURCE_COMMON;
    zData.provider = eSLIM_SERVICE_PROVIDER_NATIVE;
    zData.sensorType = eSLIM_VEHICLE_MOTION_SENSOR_TYPE_SPEED;
    zData.samples_len = 1;
    zData.samples[0].speed.sampleTimeOffset = 10;
    zData.samples[0].speed.data = 100;

    m_pzParent->routeIndication(eSLIM_SERVICE_VEHICLE_SPEED,zData);
  }
}

void VehicleNetworkDefaultSpeedMP::ToString(char* pStr, size_t sLen)
{
  snprintf(pStr, sLen, "[VehicleNetworkDefaultSpeedMP]");
}

/* -------------------------------------------------------------------------- */
VehicleNetworkCONF1SpeedMP::VehicleNetworkCONF1SpeedMP(
    VehicleNetworkProvider* pzParent, SpeedCONF1Config* pzCfg) :
    VehicleNetworkMessageProcessor(pzParent), m_pzCfg(pzCfg)
{
  mZData.timeSource = eSLIM_TIME_SOURCE_UNSPECIFIED;
  mZData.provider = eSLIM_SERVICE_PROVIDER_NATIVE;
  mZData.sensorType = eSLIM_VEHICLE_MOTION_SENSOR_TYPE_SPEED;
  mZData.samples_len = 0;
  m_tLastCANPktTime = 0;
}

void VehicleNetworkCONF1SpeedMP::OnNewCanFrameEvent(CanFrameEvent* pzEvent)
{
  CwFrame* pzFrame = pzEvent->GetFrame();
  uint8 speedIndex=0;
  float conversionFactor2mps=1.0;
  if (0 == pzFrame)
  {
    return;
  }

  if (0 < m_pzCfg->m_iFrameId
      && pzFrame->getId() == (uint32_t) m_pzCfg->m_iFrameId)
  {
    if (0
        != SLIM_MASK_IS_SET(m_pzParent->m_qEnabledServices,
            eSLIM_SERVICE_VEHICLE_SPEED))
    {
      /* Update difference between AP and CAN bus mictocontroller clock.
       * TODO: This code has to be moved to processing of DR_SYNC CAN frame */
      UpdateCanControllerTimeDelta(pzFrame->getTimestamp());

      BitDivider bitStream(pzFrame->getDataPtr(), pzFrame->getDataLen());

      if(m_pzCfg->m_unitLength)
      {
        bitStream.SetPos(m_pzCfg->m_unitOffset);
        if (bitStream.GetStatus() != 0)
          return;
        uint32_t tempUnitValue = bitStream.Get(m_pzCfg->m_unitLength);
        for(int j=0; j<m_pzCfg->m_numberofSpeedConfig;j++)
        {
          if (m_pzCfg->m_speedConfig[j].unitValue == (uint8)tempUnitValue)
          {
            speedIndex = j;
         break;
          }
        }
      }

      bitStream.SetPos(m_pzCfg->m_speedConfig[speedIndex].speedOffsetLowBits);
      if (bitStream.GetStatus() != 0)
        return;
      uint32_t speedLowBits  = bitStream.Get(m_pzCfg->m_speedConfig[speedIndex].speedLengthLowBits);

      bitStream.SetPos(m_pzCfg->m_speedConfig[speedIndex].speedOffsetUpBits);
      if (bitStream.GetStatus() != 0)
        return;
      uint32_t speedUpBits  = bitStream.Get(m_pzCfg->m_speedConfig[speedIndex].speedLengthUpBits);


      uint32_t speedBytes = (speedUpBits << m_pzCfg->m_speedConfig[speedIndex].speedLengthLowBits) | speedLowBits;

      float speedPhysicalUnit =  (speedBytes - m_pzCfg->m_speedConfig[speedIndex].offset) * m_pzCfg->m_speedConfig[speedIndex].scale;

      switch(m_pzCfg->m_speedConfig[speedIndex].speedUnit)
      {
         case SpeedCONF1Config::eSPEED_UNIT_MPH:
        conversionFactor2mps = 0.44704;
        break;
         case SpeedCONF1Config::eSPEED_UNIT_KMPH:
        conversionFactor2mps = 0.2777778;
        break;
         default:
        conversionFactor2mps = 1.0;
        break;
      }

      float speedmps = speedPhysicalUnit * conversionFactor2mps;
      SLIM_LOGV("speed samples_len %d, mIsBuffFrame %d FrameTime %d",
                 mZData.samples_len,
                 pzEvent->mIsBuffFrame,
                 pzFrame->getTimestamp());
      /* First process buffer data in batches */
      if (pzEvent->mIsBuffFrame) {
        if (mZData.samples_len == 0) {
          mZData.timeBase = VnTimeToApTime(pzFrame->getTimestamp());
          mZData.samples[0].speed.sampleTimeOffset = 0;
        } else {
          mZData.samples[mZData.samples_len].speed.sampleTimeOffset =
              SLIM_CONVERT_MSEC_TO_USEC( VnTimeToApTime(pzFrame->getTimestamp()) - mZData.timeBase );
        }
        mZData.samples[mZData.samples_len].speed.data = speedmps;
        mZData.samples_len += 1;
        if (mZData.samples_len == SLIM_VEHICLE_MOTION_DATA_MAX_SAMPLE_SETS) {
          m_pzParent->routeIndication(eSLIM_SERVICE_VEHICLE_SPEED, mZData);
          /* sleep for 2 ms to allow every batch of data sent to application */
          usleep(TIME_BW_BUFFERED_DATA_BATCHES);
          mZData.samples_len = 0;
        }
      } else {
        /* Process live data*/
        if (0 == mZData.samples_len)
        {
          mZData.timeBase = VnTimeToApTime(pzFrame->getTimestamp());
          mZData.samples[0].speed.sampleTimeOffset = 0;
        }
        else
        {
          mZData.samples[mZData.samples_len].speed.sampleTimeOffset =
                 SLIM_CONVERT_MSEC_TO_USEC( VnTimeToApTime(pzFrame->getTimestamp()) - mZData.timeBase );
        }
        mZData.samples[mZData.samples_len].speed.data = speedmps;
        mZData.samples_len++;
        if (true == IsBatchCompleted (eSLIM_SERVICE_VEHICLE_SPEED,
                                               mZData.samples_len,
                                               pzFrame->getTimestamp(),
                                               m_pzCfg->m_iBatchType,
                                               m_pzCfg->m_iBatchSize))
        {
          m_pzParent->routeIndication(eSLIM_SERVICE_VEHICLE_SPEED, mZData);
          /* Re initialize sample length and Last CAN packet time*/
          mZData.samples_len = 0;
          mZData.samples[0].speed.sampleTimeOffset = 0;
        }
      }
    }
  }
}

void VehicleNetworkCONF1SpeedMP::ToString(char* pStr, size_t sLen)
{
  snprintf(pStr, sLen, "[VehicleNetworkCONF1SpeedMP]");
}

/* -------------------------------------------------------------------------- */
VehicleNetworkDefaultDwsMP::VehicleNetworkDefaultDwsMP(
    VehicleNetworkProvider* pzParent, DwsConfig* pzCfg) :
    VehicleNetworkMessageProcessor(pzParent), m_pzCfg(pzCfg)
{
}

void VehicleNetworkDefaultDwsMP::OnNewTimerExpiryEvent(
    os_TimerExpiryType* pzEvent)
{
  VehicleNetworkMessageProcessor::OnNewTimerExpiryEvent(pzEvent);
  if (0
      != SLIM_MASK_IS_SET(m_pzParent->m_qEnabledServices,
          eSLIM_SERVICE_VEHICLE_DWS))
  {
    slimVehicleMotionDataStructT zData;
    memset(&zData, 0, sizeof(zData));
    zData.timeBase = slim_TimeTickGetMilliseconds();
    zData.timeSource = eSLIM_TIME_SOURCE_COMMON;
    zData.provider = eSLIM_SERVICE_PROVIDER_NATIVE;
    zData.sensorType = eSLIM_VEHICLE_MOTION_SENSOR_TYPE_DWS;
    zData.samples_len = 1;
    zData.samples[0].dws.sampleTimeOffset = 10;
    zData.samples[0].dws.flWheel= 100;
    zData.samples[0].dws.frWheel= 100;
    zData.samples[0].dws.rlWheel= 100;
    zData.samples[0].dws.rrWheel= 100;

    m_pzParent->routeIndication(eSLIM_SERVICE_VEHICLE_DWS,zData);
  }
}

void VehicleNetworkDefaultDwsMP::ToString(char* pStr, size_t sLen)
{
  snprintf(pStr, sLen, "[VehicleNetworkDefaultDwsMP]");
}

/* -------------------------------------------------------------------------- */
VehicleNetworkCONF1DwsMP::VehicleNetworkCONF1DwsMP(
    VehicleNetworkProvider* pzParent, DwsCONF1Config* pzCfg) :
    VehicleNetworkMessageProcessor(pzParent), m_pzCfg(pzCfg)
{
  mZData.timeSource = eSLIM_TIME_SOURCE_UNSPECIFIED;
  mZData.provider = eSLIM_SERVICE_PROVIDER_NATIVE;
  mZData.sensorType = eSLIM_VEHICLE_MOTION_SENSOR_TYPE_DWS;
  mZData.samples_len = 0;
  m_tLastCANPktTime = 0;
}

void VehicleNetworkCONF1DwsMP::OnNewCanFrameEvent(CanFrameEvent* pzEvent)
{
  CwFrame* pzFrame = pzEvent->GetFrame();
  uint8 speedIndex=0;
  float conversionFactor2mps=1.0;
  if (0 == pzFrame)
  {
    return;
  }

  if (0 < m_pzCfg->m_iFrameId
      && pzFrame->getId() == (uint32_t) m_pzCfg->m_iFrameId)
  {
    if (0
        != SLIM_MASK_IS_SET(m_pzParent->m_qEnabledServices,
            eSLIM_SERVICE_VEHICLE_DWS))
    {
      /* Update difference between AP and CAN bus mictocontroller clock.
       * TODO: This code has to be moved to processing of DR_SYNC CAN frame */
      UpdateCanControllerTimeDelta(pzFrame->getTimestamp());

      BitDivider bitStream(pzFrame->getDataPtr(), pzFrame->getDataLen());

      if(m_pzCfg->m_unitLength)
      {
        bitStream.SetPos(m_pzCfg->m_unitOffset);
        if (bitStream.GetStatus() != 0)
          return;
        uint32_t tempUnitValue = bitStream.Get(m_pzCfg->m_unitLength);
        for(int j=0; j<m_pzCfg->m_numberofSpeedConfig;j++)
        {
          if (m_pzCfg->m_dwsConfig[j].unitValue == (uint8)tempUnitValue)
          {
            speedIndex = j;
         break;
          }
        }
      }

      bitStream.SetPos(m_pzCfg->m_dwsConfig[speedIndex].speedOffsetLowBitsFL);
      if (bitStream.GetStatus() != 0)
        return;
      uint32_t speedLowBitsFL  = bitStream.Get(m_pzCfg->m_dwsConfig[speedIndex].speedLengthLowBitsFL);

      bitStream.SetPos(m_pzCfg->m_dwsConfig[speedIndex].speedOffsetUpBitsFL);
      if (bitStream.GetStatus() != 0)
        return;
      uint32_t speedUpBitsFL  = bitStream.Get(m_pzCfg->m_dwsConfig[speedIndex].speedLengthUpBitsFL);

      uint32_t speedBytesFL = (speedUpBitsFL << m_pzCfg->m_dwsConfig[speedIndex].speedLengthLowBitsFL) | speedLowBitsFL;
      float speedPhysicalUnitFL =  (speedBytesFL - m_pzCfg->m_dwsConfig[speedIndex].offset) * m_pzCfg->m_dwsConfig[speedIndex].scale;


      bitStream.SetPos(m_pzCfg->m_dwsConfig[speedIndex].speedOffsetLowBitsFR);
      if (bitStream.GetStatus() != 0)
        return;
      uint32_t speedLowBitsFR  = bitStream.Get(m_pzCfg->m_dwsConfig[speedIndex].speedLengthLowBitsFR);

      bitStream.SetPos(m_pzCfg->m_dwsConfig[speedIndex].speedOffsetUpBitsFR);
      if (bitStream.GetStatus() != 0)
        return;
      uint32_t speedUpBitsFR  = bitStream.Get(m_pzCfg->m_dwsConfig[speedIndex].speedLengthUpBitsFR);

      uint32_t speedBytesFR = (speedUpBitsFR << m_pzCfg->m_dwsConfig[speedIndex].speedLengthLowBitsFR) | speedLowBitsFR;
      float speedPhysicalUnitFR =  (speedBytesFR - m_pzCfg->m_dwsConfig[speedIndex].offset) * m_pzCfg->m_dwsConfig[speedIndex].scale;

      bitStream.SetPos(m_pzCfg->m_dwsConfig[speedIndex].speedOffsetLowBitsRL);
      if (bitStream.GetStatus() != 0)
        return;
      uint32_t speedLowBitsRL  = bitStream.Get(m_pzCfg->m_dwsConfig[speedIndex].speedLengthLowBitsRL);

      bitStream.SetPos(m_pzCfg->m_dwsConfig[speedIndex].speedOffsetUpBitsRL);
      if (bitStream.GetStatus() != 0)
        return;
      uint32_t speedUpBitsRL  = bitStream.Get(m_pzCfg->m_dwsConfig[speedIndex].speedLengthUpBitsRL);

      uint32_t speedBytesRL = (speedUpBitsRL << m_pzCfg->m_dwsConfig[speedIndex].speedLengthLowBitsRL) | speedLowBitsRL;
      float speedPhysicalUnitRL =  (speedBytesRL - m_pzCfg->m_dwsConfig[speedIndex].offset) * m_pzCfg->m_dwsConfig[speedIndex].scale;

      bitStream.SetPos(m_pzCfg->m_dwsConfig[speedIndex].speedOffsetLowBitsRR);
      if (bitStream.GetStatus() != 0)
        return;
      uint32_t speedLowBitsRR  = bitStream.Get(m_pzCfg->m_dwsConfig[speedIndex].speedLengthLowBitsRR);

      bitStream.SetPos(m_pzCfg->m_dwsConfig[speedIndex].speedOffsetUpBitsRR);
      if (bitStream.GetStatus() != 0)
        return;
      uint32_t speedUpBitsRR  = bitStream.Get(m_pzCfg->m_dwsConfig[speedIndex].speedLengthUpBitsRR);

      uint32_t speedBytesRR = (speedUpBitsRR << m_pzCfg->m_dwsConfig[speedIndex].speedLengthLowBitsRR) | speedLowBitsRR;
      float speedPhysicalUnitRR =  (speedBytesRR - m_pzCfg->m_dwsConfig[speedIndex].offset) * m_pzCfg->m_dwsConfig[speedIndex].scale;

      switch(m_pzCfg->m_dwsConfig[speedIndex].speedUnit)
      {
         case DwsCONF1Config::eSPEED_UNIT_MPH:
        conversionFactor2mps = 0.44704;
        break;
         case DwsCONF1Config::eSPEED_UNIT_KMPH:
        conversionFactor2mps = 0.2777778;
        break;
         default:
        conversionFactor2mps = 1.0;
        break;
      }

      float speedmpsFL = speedPhysicalUnitFL * conversionFactor2mps;
      float speedmpsFR = speedPhysicalUnitFR * conversionFactor2mps;
      float speedmpsRL = speedPhysicalUnitRL * conversionFactor2mps;
      float speedmpsRR = speedPhysicalUnitRR * conversionFactor2mps;

      SLIM_LOGV("dws samples_len %d, mIsBuffFrame %d FrameTime %d",
                 mZData.samples_len,
                 pzEvent->mIsBuffFrame,
                 pzFrame->getTimestamp());
      /* First process buffer data in batches */
      if (pzEvent->mIsBuffFrame) {
        if (mZData.samples_len == 0) {
          mZData.timeBase = VnTimeToApTime(pzFrame->getTimestamp());
          mZData.samples[0].dws.sampleTimeOffset = 0;
        } else {
          mZData.samples[mZData.samples_len].dws.sampleTimeOffset =
              SLIM_CONVERT_MSEC_TO_USEC(VnTimeToApTime(pzFrame->getTimestamp()) - mZData.timeBase);
        }
        mZData.samples[mZData.samples_len].dws.flWheel = speedmpsFL;
        mZData.samples[mZData.samples_len].dws.frWheel = speedmpsFR;
        mZData.samples[mZData.samples_len].dws.rlWheel = speedmpsRL;
        mZData.samples[mZData.samples_len].dws.rrWheel = speedmpsRR;
        mZData.samples_len += 1;
        if (mZData.samples_len == SLIM_VEHICLE_MOTION_DATA_MAX_SAMPLE_SETS) {
          m_pzParent->routeIndication(eSLIM_SERVICE_VEHICLE_DWS, mZData);
          /* sleep for 2 ms to allow every batch of data sent to application */
          usleep(TIME_BW_BUFFERED_DATA_BATCHES);
          mZData.samples_len = 0;
        }
      } else {
        /* Process live data*/
        if (0 == mZData.samples_len)
        {
          mZData.timeBase = VnTimeToApTime(pzFrame->getTimestamp());
          mZData.samples[0].dws.sampleTimeOffset = 0;
        }
        else
        {
          mZData.samples[mZData.samples_len].dws.sampleTimeOffset =
                 SLIM_CONVERT_MSEC_TO_USEC( VnTimeToApTime(pzFrame->getTimestamp()) - mZData.timeBase );
        }
        mZData.samples[mZData.samples_len].dws.flWheel = speedmpsFL;
        mZData.samples[mZData.samples_len].dws.frWheel = speedmpsFR;
        mZData.samples[mZData.samples_len].dws.rlWheel = speedmpsRL;
        mZData.samples[mZData.samples_len].dws.rrWheel = speedmpsRR;
        mZData.samples_len++;
        if (true == IsBatchCompleted (eSLIM_SERVICE_VEHICLE_DWS,
                                               mZData.samples_len,
                                               pzFrame->getTimestamp(),
                                               m_pzCfg->m_iBatchType,
                                               m_pzCfg->m_iBatchSize))
        {
          m_pzParent->routeIndication(eSLIM_SERVICE_VEHICLE_DWS, mZData);
          /* Re initialize sample length and Last CAN packet time*/
          mZData.samples_len = 0;
          mZData.samples[0].dws.sampleTimeOffset = 0;
        }
      }
    }
  }
}

void VehicleNetworkCONF1DwsMP::ToString(char* pStr, size_t sLen)
{
  snprintf(pStr, sLen, "[VehicleNetworkCONF1DwsMP]");
}


/* -------------------------------------------------------------------------- */
VehicleNetworkDefaultGearMP::VehicleNetworkDefaultGearMP(
    VehicleNetworkProvider* pzParent, GearConfig* pzCfg) :
    VehicleNetworkMessageProcessor(pzParent), m_pzCfg(pzCfg)
{
}

void VehicleNetworkDefaultGearMP::OnNewTimerExpiryEvent(
    os_TimerExpiryType* pzEvent)
{
  VehicleNetworkMessageProcessor::OnNewTimerExpiryEvent(pzEvent);
  if (0
      != SLIM_MASK_IS_SET(m_pzParent->m_qEnabledServices,
          eSLIM_SERVICE_VEHICLE_GEAR))
  {
    slimVehicleMotionDataStructT zData;
    memset(&zData, 0, sizeof(zData));
    zData.timeBase = slim_TimeTickGetMilliseconds();
    zData.timeSource = eSLIM_TIME_SOURCE_COMMON;
    zData.provider = eSLIM_SERVICE_PROVIDER_NATIVE;
    zData.sensorType = eSLIM_VEHICLE_MOTION_SENSOR_TYPE_GEAR;
    zData.samples_len = 1;
    zData.samples[0].gear.sampleTimeOffset = 10;
    zData.samples[0].gear.data = 100;

    m_pzParent->routeIndication(eSLIM_SERVICE_VEHICLE_GEAR,zData);
  }
}

void VehicleNetworkDefaultGearMP::ToString(char* pStr, size_t sLen)
{
  snprintf(pStr, sLen, "[VehicleNetworkDefaultGearMP]");
}

/* -------------------------------------------------------------------------- */
VehicleNetworkCONF1GearMP::VehicleNetworkCONF1GearMP(
    VehicleNetworkProvider* pzParent, GearCONF1Config* pzCfg) :
    VehicleNetworkMessageProcessor(pzParent), m_pzCfg(pzCfg)
{
  mZData.timeSource = eSLIM_TIME_SOURCE_UNSPECIFIED;
  mZData.provider = eSLIM_SERVICE_PROVIDER_NATIVE;
  mZData.sensorType = eSLIM_VEHICLE_MOTION_SENSOR_TYPE_GEAR;
  mZData.samples_len = 0;
  m_tLastCANPktTime = 0;
}

void VehicleNetworkCONF1GearMP::OnNewCanFrameEvent(CanFrameEvent* pzEvent)
{
  CwFrame* pzFrame = pzEvent->GetFrame();
  int8 geardata =-1;
  if (0 == pzFrame)
  {
    return;
  }

  if (0 < m_pzCfg->m_iFrameId
      && pzFrame->getId() == (uint32_t) m_pzCfg->m_iFrameId)
  {
    if (0
        != SLIM_MASK_IS_SET(m_pzParent->m_qEnabledServices,
            eSLIM_SERVICE_VEHICLE_GEAR))
    {
      /* Update difference between AP and CAN bus mictocontroller clock.
       * TODO: This code has to be moved to processing of DR_SYNC CAN frame */
      UpdateCanControllerTimeDelta(pzFrame->getTimestamp());

      BitDivider bitStream(pzFrame->getDataPtr(), pzFrame->getDataLen());

      if (m_pzCfg->m_gearLength == 0)
        return;


      bitStream.SetPos(m_pzCfg->m_gearOffset);
      if (bitStream.GetStatus() != 0)
        return;
      int8_t canGearValue  = (int8_t)bitStream.Get(m_pzCfg->m_gearLength);


      /* 0 - Parked, 1 - Reverse, 2 - Neutral, 3 - Forward */
      if (canGearValue == m_pzCfg->m_parkValue)
      {
         geardata = 0;
      }
      else if (canGearValue == m_pzCfg->m_reverseValue)
      {
         geardata = 1;
      }
      else if (canGearValue == m_pzCfg->m_neutralValue)
      {
         geardata = 2;
      }
      else
      {
         for(int j=0;j<m_pzCfg->m_numberDriveGear;j++)
         {
            if (canGearValue == m_pzCfg->m_driveValue[j])
            {
               geardata = 3;
               break;
            }
         }
      }

      if (geardata == -1)
          return;

      SLIM_LOGV("gear samples_len %d, mIsBuffFrame %d FrameTime %d",
                 mZData.samples_len,
                 pzEvent->mIsBuffFrame,
                 pzFrame->getTimestamp());
      /* First process buffer data in batches */
      if (pzEvent->mIsBuffFrame) {
        if (mZData.samples_len == 0) {
          mZData.timeBase = VnTimeToApTime(pzFrame->getTimestamp());
          mZData.samples[0].gear.data = geardata;
          mZData.samples[0].gear.sampleTimeOffset = 0;
        } else {
          mZData.samples[mZData.samples_len].gear.data = geardata;
          mZData.samples[mZData.samples_len].gear.sampleTimeOffset =
              SLIM_CONVERT_MSEC_TO_USEC( VnTimeToApTime(pzFrame->getTimestamp()) - mZData.timeBase );
        }
        mZData.samples_len += 1;
        if (mZData.samples_len == SLIM_VEHICLE_MOTION_DATA_MAX_SAMPLE_SETS) {
          m_pzParent->routeIndication(eSLIM_SERVICE_VEHICLE_GEAR, mZData);
          /* sleep for 2 ms to allow every batch of data sent to application */
          usleep(TIME_BW_BUFFERED_DATA_BATCHES);
          mZData.samples_len = 0;
        }
      } else {
        /* Process live data*/
        if (0 == mZData.samples_len)
        {
          mZData.timeBase = VnTimeToApTime(pzFrame->getTimestamp());
          mZData.samples[0].gear.sampleTimeOffset = 0;
        }
        else
        {
          mZData.samples[mZData.samples_len].gear.sampleTimeOffset =
                 SLIM_CONVERT_MSEC_TO_USEC( VnTimeToApTime(pzFrame->getTimestamp()) - mZData.timeBase );
        }

        mZData.samples[mZData.samples_len].gear.data = geardata;
        mZData.samples_len++;
        if (true == IsBatchCompleted (eSLIM_SERVICE_VEHICLE_GEAR,
                                               mZData.samples_len,
                                               pzFrame->getTimestamp(),
                                               m_pzCfg->m_iBatchType,
                                               m_pzCfg->m_iBatchSize))
        {
          m_pzParent->routeIndication(eSLIM_SERVICE_VEHICLE_GEAR, mZData);
          /* Re initialize sample length and Last CAN packet time*/
          mZData.samples_len = 0;
          mZData.samples[0].gear.sampleTimeOffset = 0;
        }
      }
    }
  }
}

void VehicleNetworkCONF1GearMP::ToString(char* pStr, size_t sLen)
{
  snprintf(pStr, sLen, "[VehicleNetworkCONF1GearMP]");
}



}


