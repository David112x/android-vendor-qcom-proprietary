/*
Copyright (c) 2015 Qualcomm Technologies, Inc.
All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
*/
/**
@file
@brief Vehicle Network configuration interface file.

This file defines VNW configuration object implementation.
*/
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <new>
#include <loc_cfg.h>
#include <inttypes.h>
#include <slim_os_log_api.h>
#include "VehicleNetworkConfiguration.h"
#include <loc_pla.h>
#include <algorithm>
/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/
#define VN_PROC_CLOCK_RATIO_STR_ID   "VN_PROC_CLOCK_RATIO"
#define VN_ACCEL_CFG_STR_ID          "VN_ACCEL_CFG"
#define VN_GYRO_CFG_STR_ID           "VN_GYRO_CFG"
#define VN_ODOMETRY_CFG_STR_ID       "VN_ODOMETRY_CFG"
#define VN_SPEED_CFG_STR_ID          "VN_SPEED_CFG"
#define VN_DWS_CFG_STR_ID            "VN_DWS_CFG"
#define VN_GEAR_CFG_STR_ID           "VN_GEAR_CFG"

#define VN_CFG_BATCH_TYPE_STR_ID                 "VN_CFG_BATCH_TYPE"
#define VN_ACCEL_CFG_BATCH_VAL_STR_ID            "VN_ACCEL_CFG_BATCH_VALUE"
#define VN_GYRO_CFG_BATCH_VAL_STR_ID             "VN_GYRO_CFG_BATCH_VALUE"
#define VN_ODOMETRY_CFG_BATCH_VAL_STR_ID         "VN_ODOMETRY_CFG_BATCH_VALUE"
#define VN_SPEED_CFG_BATCH_VAL_STR_ID            "VN_SPEED_CFG_BATCH_VALUE"
#define VN_DWS_CFG_BATCH_VAL_STR_ID              "VN_DWS_CFG_BATCH_VALUE"
#define VN_GEAR_CFG_BATCH_VAL_STR_ID             "VN_GEAR_CFG_BATCH_VALUE"

#undef SLIM_LOG_LOCAL_MODULE
#define SLIM_LOG_LOCAL_MODULE SLIM_LOG_MOD_VNW

namespace slim
{

const char* const VehicleNetworkConfiguration::VN_CFG_FILE_NAME =
        LOC_PATH_SAP_CONF;

// See header file for function documentation
VehicleNetworkConfiguration::VehicleNetworkConfiguration()
  : m_pzAccelConfig(0)
  , m_pzGyroConfig(0)
  , m_pzOdometryConfig(0)
  , m_pzSpeedConfig(0)
  , m_pzDwsConfig(0)
  , m_pzGearConfig(0)
  , m_dCanApClockRatio(0.0)
{
}

// See header file for function documentation
VehicleNetworkConfiguration::~VehicleNetworkConfiguration()
{
  delete m_pzAccelConfig;
  delete m_pzGyroConfig;
  delete m_pzOdometryConfig;
  delete m_pzSpeedConfig;
  delete m_pzDwsConfig;
  delete m_pzGearConfig;
}

// See header file for function documentation
int32_t VehicleNetworkConfiguration::GetCfgVersion(const char* pCfgStr) const
{
  /* Configuration string version */
  return strchr(pCfgStr, ':') ? (int32_t)atoi(pCfgStr) : -1;
}

// See header file for function documentation
SensorConfig* VehicleNetworkConfiguration::GetAccelConfig() const
{
  return m_pzAccelConfig;
}

// See header file for function documentation
SensorConfig* VehicleNetworkConfiguration::GetGyroConfig() const
{
  return m_pzGyroConfig;
}

// See header file for function documentation
SensorConfig* VehicleNetworkConfiguration::GetOdometryConfig() const
{
  return m_pzOdometryConfig;
}

// See header file for function documentation
SensorConfig* VehicleNetworkConfiguration::GetSpeedConfig() const
{
  return m_pzSpeedConfig;
}

// See header file for function documentation
SensorConfig* VehicleNetworkConfiguration::GetDwsConfig() const
{
  return m_pzDwsConfig;
}

// See header file for function documentation
SensorConfig* VehicleNetworkConfiguration::GetGearConfig() const
{
  return m_pzGearConfig;
}


// See header file for function documentation
double VehicleNetworkConfiguration::GetCanApClockRatio() const
{
  return m_dCanApClockRatio;
}

// See header file for function documentation
SensorConfig* VehicleNetworkConfiguration::LoadAccelerometerConfig
(const char* pStr)
{
  SensorConfig* pzAC = 0;
  int v = GetCfgVersion(pStr);
  SLIM_LOGD("Accel configuration version v= %d", v);
  if (v < 0) return 0;
  switch (v)
  {
  case eACCEL_CONFIG_DEFAULT:
    {
      pzAC = new (std::nothrow) AccelConfig();
    }
    break;
  case eACCEL_CONFIG_CONF1:
    {
      pzAC = new (std::nothrow) AccelCONF1Config();
    }
    break;
  /* Other configuration versions to be added here */
  default:
    break;
  }

  if (0 != pzAC)
  {
    pzAC->Load(pStr);
  }
  return pzAC;
}

// See header file for function documentation
SensorConfig* VehicleNetworkConfiguration::LoadGyroConfig(const char* pStr)
{
  SensorConfig* pzGC = 0;
  int v = GetCfgVersion(pStr);
  if (v < 0) return 0;
  switch (v)
  {
  case eGYRO_CONFIG_DEFAULT:
    {
      pzGC = new (std::nothrow) GyroConfig();
    }
    break;
  case eGYRO_CONFIG_CONF1:
    {
      pzGC= new (std::nothrow) GyroCONF1Config();
    }
    break;
  /* Other configuration versions to be added here */
  default:
    break;
  }

  if (0 != pzGC)
  {
    pzGC->Load(pStr);
  }
  return pzGC;
}

// See header file for function documentation
SensorConfig* VehicleNetworkConfiguration::LoadOdometryConfig(const char* pStr)
{
  SensorConfig* pzOC = 0;
  int v = GetCfgVersion(pStr);
  if (v < 0) return 0;
  switch (v)
  {
  case eODOMETRY_CONFIG_DEFAULT: // default implementation
    {
      pzOC = new (std::nothrow) OdometryConfig();
    }
    break;
  case eODOMETRY_CONFIG_CONF1:
    {
      pzOC= new (std::nothrow) OdometryCONF1Config();
    }
    break;
  /* Other configuration versions to be added here */
  default:
    break;
  }

  if (0 != pzOC)
  {
    pzOC->Load(pStr);
  }

  return pzOC;
}

// See header file for function documentation
SensorConfig* VehicleNetworkConfiguration::LoadSpeedConfig
(const char* pStr)
{
  SensorConfig* pzSC = 0;
  int v = GetCfgVersion(pStr);
  SLIM_LOGD("Speed configuration version v= %d", v);
  if (v < 0) return 0;
  switch (v)
  {
  case eSPEED_CONFIG_DEFAULT:
    {
      pzSC = new (std::nothrow) SpeedConfig();
    }
    break;
  case eSPEED_CONFIG_CONF1:
    {
      pzSC = new (std::nothrow) SpeedCONF1Config();
    }
    break;
  /* Other configuration versions to be added here */
  default:
    break;
  }

  if (0 != pzSC)
  {
    pzSC->Load(pStr);
  }
  return pzSC;
}


// See header file for function documentation
SensorConfig* VehicleNetworkConfiguration::LoadDwsConfig
(const char* pStr)
{
  SensorConfig* pzDC = 0;
  int v = GetCfgVersion(pStr);
  SLIM_LOGD("DWS configuration version v= %d", v);
  if (v < 0) return 0;
  switch (v)
  {
  case eDWS_CONFIG_DEFAULT:
    {
      pzDC = new (std::nothrow) DwsConfig();
    }
    break;
  case eDWS_CONFIG_CONF1:
    {
      pzDC = new (std::nothrow) DwsCONF1Config();
    }
    break;
  /* Other configuration versions to be added here */
  default:
    break;
  }

  if (0 != pzDC)
  {
    pzDC->Load(pStr);
  }
  return pzDC;
}

// See header file for function documentation
SensorConfig* VehicleNetworkConfiguration::LoadGearConfig
(const char* pStr)
{
  SensorConfig* pzGC = 0;
  int v = GetCfgVersion(pStr);
  SLIM_LOGD("Gear configuration version v= %d", v);
  if (v < 0) return 0;
  switch (v)
  {
  case eGEAR_CONFIG_DEFAULT:
    {
      pzGC = new (std::nothrow) GearConfig();
    }
    break;
  case eGEAR_CONFIG_CONF1:
    {
      pzGC = new (std::nothrow) GearCONF1Config();
    }
    break;
  /* Other configuration versions to be added here */
  default:
    break;
  }

  if (0 != pzGC)
  {
    pzGC->Load(pStr);
  }
  return pzGC;
}

// See header file for function documentation
void VehicleNetworkConfiguration::Load()
{
  /* As this is read from loc cfg routine, need to conform as to
   what is supported by loc cfg module */
  char pzAccelCfgStr[LOC_MAX_PARAM_STRING] = {0};
  char pzGyroCfgStr[LOC_MAX_PARAM_STRING] = {0};
  char pzOdometryCfgStr[LOC_MAX_PARAM_STRING] = {0};
  char pzSpeedCfgStr[LOC_MAX_PARAM_STRING] = {0};
  char pzDwsCfgStr[LOC_MAX_PARAM_STRING] = {0};
  char pzGearCfgStr[LOC_MAX_PARAM_STRING] = {0};
  char pzClockRatioCfgStr[LOC_MAX_PARAM_STRING] = {0};

  char pzCfgBatchTypeStr[LOC_MAX_PARAM_STRING] = {0};
  char pzAccelCfgBatchValStr[LOC_MAX_PARAM_STRING] = {0};
  char pzGyroCfgBatchValStr[LOC_MAX_PARAM_STRING] = {0};
  char pzOdometryCfgBatchValStr[LOC_MAX_PARAM_STRING] = {0};
  char pzSpeedCfgBatchValStr[LOC_MAX_PARAM_STRING] = {0};
  char pzDwsCfgBatchValStr[LOC_MAX_PARAM_STRING] = {0};
  char pzGearCfgBatchValStr[LOC_MAX_PARAM_STRING] = {0};
  uint8_t uCfgBatchTypeStrSet = 0;
  uint8_t uAccelCfgBatchValStrSet = 0;
  uint8_t uGyroCfgBatchValStrSet = 0;
  uint8_t uOdometryCfgBatchValStrSet = 0;
  uint8_t uSpeedCfgBatchValStrSet = 0;
  uint8_t uDwsCfgBatchValStrSet = 0;
  uint8_t uGearCfgBatchValStrSet = 0;

  uint8_t uAccelCfgStrSet = 0;
  uint8_t uGyroCfgStrSet = 0;
  uint8_t uOdometryCfgStrSet = 0;
  uint8_t uSpeedCfgStrSet = 0;
  uint8_t uDwsCfgStrSet = 0;
  uint8_t uGearCfgStrSet = 0;
  uint8_t uClockRatioCfgStrSet = 0;

  loc_param_s_type z_vnParameterTable[] =
  {
  { VN_PROC_CLOCK_RATIO_STR_ID, pzClockRatioCfgStr, &uClockRatioCfgStrSet, 's'},
  { VN_ACCEL_CFG_STR_ID, pzAccelCfgStr, &uAccelCfgStrSet, 's' },
  { VN_GYRO_CFG_STR_ID, pzGyroCfgStr, &uGyroCfgStrSet, 's' },
  { VN_ODOMETRY_CFG_STR_ID, pzOdometryCfgStr, &uOdometryCfgStrSet, 's'},
  { VN_SPEED_CFG_STR_ID, pzSpeedCfgStr, &uSpeedCfgStrSet, 's' },
  { VN_DWS_CFG_STR_ID, pzDwsCfgStr, &uDwsCfgStrSet, 's' },
  { VN_GEAR_CFG_STR_ID, pzGearCfgStr, &uGearCfgStrSet, 's'},
  { VN_CFG_BATCH_TYPE_STR_ID, pzCfgBatchTypeStr, &uCfgBatchTypeStrSet, 's'},
  { VN_ACCEL_CFG_BATCH_VAL_STR_ID, pzAccelCfgBatchValStr, &uAccelCfgBatchValStrSet, 's'},
  { VN_GYRO_CFG_BATCH_VAL_STR_ID, pzGyroCfgBatchValStr, &uGyroCfgBatchValStrSet, 's'},
  { VN_ODOMETRY_CFG_BATCH_VAL_STR_ID, pzOdometryCfgBatchValStr, &uOdometryCfgBatchValStrSet, 's'},
  { VN_SPEED_CFG_BATCH_VAL_STR_ID, pzSpeedCfgBatchValStr, &uSpeedCfgBatchValStrSet, 's'},
  { VN_DWS_CFG_BATCH_VAL_STR_ID, pzDwsCfgBatchValStr, &uDwsCfgBatchValStrSet, 's'},
  { VN_GEAR_CFG_BATCH_VAL_STR_ID, pzGearCfgBatchValStr, &uGearCfgBatchValStrSet, 's'}
  };

  UTIL_READ_CONF(VN_CFG_FILE_NAME, z_vnParameterTable);

  if (0 != uClockRatioCfgStrSet)
  {
    char* rptr = NULL;
    m_dCanApClockRatio = strtod(pzClockRatioCfgStr, &rptr);
    if ((ERANGE == errno) ||
        (pzClockRatioCfgStr == rptr))
    {
      m_dCanApClockRatio = 0.0;
    }
  }

 /* Load parsing configuration for CAN packets*/
  m_pzAccelConfig = (0 != uAccelCfgStrSet) ?
      VehicleNetworkConfiguration::LoadAccelerometerConfig(pzAccelCfgStr) : 0;
  m_pzGyroConfig = (0 != uGyroCfgStrSet) ?
      VehicleNetworkConfiguration::LoadGyroConfig(pzGyroCfgStr) : 0;
  m_pzOdometryConfig = (0 != uOdometryCfgStrSet) ?
      VehicleNetworkConfiguration::LoadOdometryConfig(pzOdometryCfgStr) : 0;

  m_pzSpeedConfig = (0 != uSpeedCfgStrSet) ?
      VehicleNetworkConfiguration::LoadSpeedConfig(pzSpeedCfgStr) : 0;
  m_pzDwsConfig = (0 != uDwsCfgStrSet) ?
      VehicleNetworkConfiguration::LoadDwsConfig(pzDwsCfgStr) : 0;
  m_pzGearConfig = (0 != uGearCfgStrSet) ?
      VehicleNetworkConfiguration::LoadGearConfig(pzGearCfgStr) : 0;

  /* Load Batching configuration for CAN packets */
  if (NULL != m_pzAccelConfig && 0 != uAccelCfgBatchValStrSet)
  {
    if(eACCEL_CONFIG_CONF1 == VehicleNetworkConfiguration::GetCfgVersion(pzAccelCfgStr))
    {
      m_pzAccelConfig->loadBatchConfig(pzCfgBatchTypeStr, pzAccelCfgBatchValStr);
    }
  }
  if (NULL != m_pzGyroConfig && 0 != uGyroCfgBatchValStrSet)
  {
    if(eGYRO_CONFIG_CONF1 == VehicleNetworkConfiguration::GetCfgVersion(pzGyroCfgStr))
    {
       m_pzGyroConfig->loadBatchConfig(pzCfgBatchTypeStr, pzGyroCfgBatchValStr);
    }
  }
  if (NULL != m_pzOdometryConfig && 0 != uOdometryCfgBatchValStrSet)
  {
    if(eODOMETRY_CONFIG_CONF1 == VehicleNetworkConfiguration::GetCfgVersion(pzOdometryCfgStr))
    {
      m_pzOdometryConfig->loadBatchConfig(pzCfgBatchTypeStr, pzOdometryCfgBatchValStr);
    }
  }
  if (NULL != m_pzSpeedConfig && 0 != uSpeedCfgBatchValStrSet)
  {
    if(eSPEED_CONFIG_CONF1 == VehicleNetworkConfiguration::GetCfgVersion(pzSpeedCfgStr))
    {
      m_pzSpeedConfig->loadBatchConfig(pzCfgBatchTypeStr, pzSpeedCfgBatchValStr);
    }
  }
  if (NULL != m_pzDwsConfig && 0 != uDwsCfgBatchValStrSet)
  {
    if(eDWS_CONFIG_CONF1 == VehicleNetworkConfiguration::GetCfgVersion(pzDwsCfgStr))
    {
      m_pzDwsConfig->loadBatchConfig(pzCfgBatchTypeStr, pzDwsCfgBatchValStr);
    }
  }
  if (NULL != m_pzGearConfig && 0 != uGearCfgBatchValStrSet)
  {
    if(eGEAR_CONFIG_CONF1 == VehicleNetworkConfiguration::GetCfgVersion(pzGearCfgStr))
    {
      m_pzGearConfig->loadBatchConfig(pzCfgBatchTypeStr, pzGearCfgBatchValStr);
    }
  }
}

// See header file for function documentation
void VehicleNetworkConfiguration::ToString(char* pStr, size_t sLen) const
{
  if (0 == pStr || 0 == sLen)
  {
    return;
  }

  snprintf(pStr, sLen, "[file:'%s', m_dCanApClockRatio:%f, "
                       "m_pzAccelConfig=%X, "
                       "m_pzGyroConfig=%X, "
                       "m_pzOdometryConfig=%X, "
                       "m_pzSpeedConfig=%X, "
                       "m_pzDwsConfig=%X, "
                       "m_pzGearConfig=%X]",
           VN_CFG_FILE_NAME,
           m_dCanApClockRatio,
           (uintptr_t)m_pzAccelConfig,
           (uintptr_t)m_pzGyroConfig,
           (uintptr_t)m_pzOdometryConfig,
           (uintptr_t)m_pzSpeedConfig,
           (uintptr_t)m_pzDwsConfig,
           (uintptr_t)m_pzGearConfig);
}


/* -------------------------------------------------------------------------- */
// See header file for function documentation
SensorConfig::SensorConfig(uint32_t iCfgVersion)
  : m_pCfgStr(0)
  , m_qCfgVersion(iCfgVersion)
{
}

// See header file for function documentation
SensorConfig::~SensorConfig()
{
  delete m_pCfgStr;
}

// See header file for function documentation
void SensorConfig::Load(const char* pStr)
{
  if (0 == pStr || 0 == *pStr)
  {
    return;
  }

  /* Save a copy of the configuration string */
  size_t len = strlen(pStr);
  len = len < LOC_MAX_PARAM_STRING ? (len + 1) : LOC_MAX_PARAM_STRING;
  m_pCfgStr = new (std::nothrow) char[len];
  if (0 != m_pCfgStr)
  {
    strlcpy(m_pCfgStr, pStr, len);
  }
}

// See header file for function documentation
uint32_t SensorConfig::GetVersion() const
{
  return m_qCfgVersion;
}

// See header file for function documentation
bool SensorConfig::CfgValid() const
{
  return (0 != m_pCfgStr) &&
         (VehicleNetworkConfiguration::eCFG_VERSION_MAX >= m_qCfgVersion);
}

// See header file for function documentation
void SensorConfig::ToString(char* pStr, size_t sLen) const
{
  if (0 == pStr || 0 == sLen)
  {
    return;
  }

  snprintf(pStr, sLen, "[str:'%s', v:%d]",
           (0 != m_pCfgStr) ? m_pCfgStr : "null",
           (int)m_qCfgVersion);
}

// See header file for function documentation
void SensorConfig::loadBatchConfig(const char* pStrBatchType, const char* pStrBatchValue)
{
    /** This is dummy virtual function
        must be implemented by derived class
        if batching configuration need to be parsed
     */
    return;
}

// See header file for function documentation
void SensorConfig::GetNextFloat(float* pfX, char** pFBegin, float fDefault)
{
  if (0 == pfX)
  {
    return;
  }

  if (0 == pFBegin || 0 == *pFBegin || 0 == **pFBegin)
  {
    *pfX = fDefault;
    return;
  }

  char* pDelim = 0;
  *pfX = strtod(*pFBegin, &pDelim);
  *pFBegin = pDelim;
}

// See header file for function documentation
void SensorConfig::GetNextInt(int* piX, char** pFBegin, int iDefault)
{
  if (0 == piX)
  {
    return;
  }

  if (0 == pFBegin || 0 == *pFBegin || 0 == **pFBegin)
  {
    *piX = iDefault;
    return;
  }

  char* pDelim = 0;
  *piX = strtol(*pFBegin, &pDelim, 10);
  *pFBegin = pDelim;
}

/* -------------------------------------------------------------------------- */
// See header file for function documentation
AccelConfig::AccelConfig()
  : SensorConfig(VehicleNetworkConfiguration::eACCEL_CONFIG_DEFAULT)
  , m_fPeriod(0)
{
}

// See header file for function documentation
void AccelConfig::Load(const char* pStr)
{
  if (0 == pStr || 0 == *pStr)
  {
    return;
  }

  SensorConfig::Load(pStr);

  if (0 == m_pCfgStr)
  {
    return;
  }
  m_fPeriod = 0;
  char* pFieldbegin = strchr(m_pCfgStr, ':');
  if (!pFieldbegin) return;
  pFieldbegin++;
  GetNextFloat(&m_fPeriod, &pFieldbegin, -1);

}

// See header file for function documentation
bool AccelConfig::CfgValid() const
{
  return SensorConfig::CfgValid() &&
         VehicleNetworkConfiguration::eACCEL_CONFIG_DEFAULT == m_qCfgVersion &&
         1.0 <= m_fPeriod && 10.0 >= m_fPeriod;
}

// See header file for function documentation
void AccelConfig::ToString(char* pStr, size_t sLen) const
{
  if (0 == pStr || 0 == sLen)
  {
    return;
  }

  char pzTStr[1024];
  SensorConfig::ToString(pzTStr, sizeof(pzTStr));
  snprintf(pStr, sLen, "[%s, period:%f]", pzTStr, m_fPeriod);
}

/* -------------------------------------------------------------------------- */
// See header file for function documentation
GyroConfig::GyroConfig()
  : SensorConfig(VehicleNetworkConfiguration::eGYRO_CONFIG_DEFAULT)
  , m_fPeriod(0)
{
}

// See header file for function documentation
void GyroConfig::Load(const char* pStr)
{
  if (0 == pStr || 0 == *pStr)
  {
    return;
  }

  SensorConfig::Load(pStr);

  if (0 == m_pCfgStr)
  {
    return;
  }

  m_fPeriod = 0;
  char* pFieldbegin = strchr(m_pCfgStr, ':');
  if (!pFieldbegin) return;
  pFieldbegin++;
  GetNextFloat(&m_fPeriod, &pFieldbegin, -1);
}

// See header file for function documentation
bool GyroConfig::CfgValid() const
{
  return SensorConfig::CfgValid() &&
         VehicleNetworkConfiguration::eGYRO_CONFIG_DEFAULT == m_qCfgVersion &&
         1.0 <= m_fPeriod && 10.0 >= m_fPeriod;
}

// See header file for function documentation
void GyroConfig::ToString(char* pStr, size_t sLen) const
{
  if (0 == pStr || 0 == sLen)
  {
    return;
  }

  char pzTStr[1024];
  SensorConfig::ToString(pzTStr, sizeof(pzTStr));
  snprintf(pStr, sLen, "[%s, period:%f]", pzTStr, m_fPeriod);
}

/* -------------------------------------------------------------------------- */
// See header file for function documentation
OdometryConfig::OdometryConfig()
  : SensorConfig(VehicleNetworkConfiguration::eODOMETRY_CONFIG_DEFAULT)
  , m_fAverageSpeed(0)
  , m_fPeriod(0)
{
}

// See header file for function documentation
void OdometryConfig::Load(const char* pStr)
{
  if (0 == pStr || 0 == *pStr)
  {
    return;
  }

  SensorConfig::Load(pStr);

  if (0 == m_pCfgStr)
  {
    return;
  }

  m_fPeriod = 0;
  m_fAverageSpeed = 0;
  char* pFieldbegin = strchr(m_pCfgStr, ':');
  if (!pFieldbegin) return;
  pFieldbegin++;
  GetNextFloat(&m_fPeriod, &pFieldbegin, -1);
  if (',' == *pFieldbegin)
  {
    pFieldbegin++;
    GetNextFloat(&m_fAverageSpeed, &pFieldbegin, -1);
  }
}

// See header file for function documentation
bool OdometryConfig::CfgValid() const
{
  return SensorConfig::CfgValid() &&
       VehicleNetworkConfiguration::eODOMETRY_CONFIG_DEFAULT == m_qCfgVersion &&
       1.0 <= m_fPeriod && 10.0 >= m_fPeriod &&
       1.0 <= m_fAverageSpeed && 25.0 >= m_fAverageSpeed;
}

// See header file for function documentation
void OdometryConfig::ToString(char* pStr, size_t sLen) const
{
  if (0 == pStr || 0 == sLen)
  {
    return;
  }

  char pzTStr[1024];
  SensorConfig::ToString(pzTStr, sizeof(pzTStr));
  snprintf(pStr, sLen, "[%s, period:%f, avrSpeed:%f]",
           pzTStr, m_fPeriod, m_fAverageSpeed);
}


/* -------------------------------------------------------------------------- */
// See header file for function documentation
SpeedConfig::SpeedConfig()
  : SensorConfig(VehicleNetworkConfiguration::eSPEED_CONFIG_DEFAULT)
  , m_fPeriod(0)
{
}

// See header file for function documentation
void SpeedConfig::Load(const char* pStr)
{
  if (0 == pStr || 0 == *pStr)
  {
    return;
  }

  SensorConfig::Load(pStr);

  if (0 == m_pCfgStr)
  {
    return;
  }
  m_fPeriod = 0;
  char* pFieldbegin = strchr(m_pCfgStr, ':');
  if (!pFieldbegin) return;
  pFieldbegin++;
  GetNextFloat(&m_fPeriod, &pFieldbegin, -1);

}

// See header file for function documentation
bool SpeedConfig::CfgValid() const
{
  return SensorConfig::CfgValid() &&
         VehicleNetworkConfiguration::eSPEED_CONFIG_DEFAULT == m_qCfgVersion &&
         1.0 <= m_fPeriod && 10.0 >= m_fPeriod;
}

// See header file for function documentation
void SpeedConfig::ToString(char* pStr, size_t sLen) const
{
  if (0 == pStr || 0 == sLen)
  {
    return;
  }

  char pzTStr[1024];
  SensorConfig::ToString(pzTStr, sizeof(pzTStr));
  snprintf(pStr, sLen, "[%s, period:%f]", pzTStr, m_fPeriod);
}


/* -------------------------------------------------------------------------- */
// See header file for function documentation
DwsConfig::DwsConfig()
  : SensorConfig(VehicleNetworkConfiguration::eDWS_CONFIG_DEFAULT)
  , m_fPeriod(0)
{
}

// See header file for function documentation
void DwsConfig::Load(const char* pStr)
{
  if (0 == pStr || 0 == *pStr)
  {
    return;
  }

  SensorConfig::Load(pStr);

  if (0 == m_pCfgStr)
  {
    return;
  }
  m_fPeriod = 0;
  char* pFieldbegin = strchr(m_pCfgStr, ':');
  if (!pFieldbegin) return;
  pFieldbegin++;
  GetNextFloat(&m_fPeriod, &pFieldbegin, -1);

}

// See header file for function documentation
bool DwsConfig::CfgValid() const
{
  return SensorConfig::CfgValid() &&
         VehicleNetworkConfiguration::eDWS_CONFIG_DEFAULT == m_qCfgVersion &&
         1.0 <= m_fPeriod && 10.0 >= m_fPeriod;
}

// See header file for function documentation
void DwsConfig::ToString(char* pStr, size_t sLen) const
{
  if (0 == pStr || 0 == sLen)
  {
    return;
  }

  char pzTStr[1024];
  SensorConfig::ToString(pzTStr, sizeof(pzTStr));
  snprintf(pStr, sLen, "[%s, period:%f]", pzTStr, m_fPeriod);
}

/* -------------------------------------------------------------------------- */
// See header file for function documentation
GearConfig::GearConfig()
  : SensorConfig(VehicleNetworkConfiguration::eGEAR_CONFIG_DEFAULT)
  , m_fPeriod(0)
{
}

// See header file for function documentation
void GearConfig::Load(const char* pStr)
{
  if (0 == pStr || 0 == *pStr)
  {
    return;
  }

  SensorConfig::Load(pStr);

  if (0 == m_pCfgStr)
  {
    return;
  }
  m_fPeriod = 0;
  char* pFieldbegin = strchr(m_pCfgStr, ':');
  if (!pFieldbegin) return;
  pFieldbegin++;
  GetNextFloat(&m_fPeriod, &pFieldbegin, -1);

}

// See header file for function documentation
bool GearConfig::CfgValid() const
{
  return SensorConfig::CfgValid() &&
         VehicleNetworkConfiguration::eGEAR_CONFIG_DEFAULT == m_qCfgVersion &&
         1.0 <= m_fPeriod && 10.0 >= m_fPeriod;
}

// See header file for function documentation
void GearConfig::ToString(char* pStr, size_t sLen) const
{
  if (0 == pStr || 0 == sLen)
  {
    return;
  }

  char pzTStr[1024];
  SensorConfig::ToString(pzTStr, sizeof(pzTStr));
  snprintf(pStr, sLen, "[%s, period:%f]", pzTStr, m_fPeriod);
}

/* -------------------------------------------------------------------------- */
// See header file for function documentation
AccelCONF1Config::AccelCONF1Config()
  : SensorConfig(VehicleNetworkConfiguration::eACCEL_CONFIG_CONF1)
  , m_iFrameId(-1)
  , m_iOffsetX(0)
  , m_iLenX(0)
  , m_fScalingX(0)
  , m_iOffsetY(0)
  , m_iLenY(0)
  , m_fScalingY(0)
  , m_iOffsetZ(0)
  , m_iLenZ(0)
  , m_fScalingZ(0)
{
  m_iBatchType = SLIM_VNW_BATCH_TYPE_DEFAULT;
  m_iBatchSize = SLIM_VNW_BATCH_VAL_DEFAULT;
}

// See header file for function documentation
void AccelCONF1Config::Load(const char* pStr)
{
  if (0 == pStr || 0 == *pStr)
  {
    return;
  }

  SensorConfig::Load(pStr);

  if (0 == m_pCfgStr)
  {
    return;
  }

  char* pFieldbegin = strchr(m_pCfgStr, ':');
  if (!pFieldbegin)
  {
    return;
  }

  pFieldbegin++;
  GetNextInt(&m_iFrameId, &pFieldbegin, -1);

  if (',' == *pFieldbegin)
  {
    pFieldbegin++;
    GetNextInt(&m_iOffsetX, &pFieldbegin, -1);
  }

  if (',' == *pFieldbegin)
  {
    pFieldbegin++;
    GetNextInt(&m_iLenX, &pFieldbegin, -1);
  }

  if (',' == *pFieldbegin)
  {
    pFieldbegin++;
    GetNextFloat(&m_fScalingX, &pFieldbegin, -1);
  }

  if (',' == *pFieldbegin)
  {
    pFieldbegin++;
    GetNextInt(&m_iOffsetY, &pFieldbegin, -1);
  }

  if (',' == *pFieldbegin)
  {
    pFieldbegin++;
    GetNextInt(&m_iLenY, &pFieldbegin, -1);
  }

  if (',' == *pFieldbegin)
  {
    pFieldbegin++;
    GetNextFloat(&m_fScalingY, &pFieldbegin, -1);
  }

  if (',' == *pFieldbegin)
  {
    pFieldbegin++;
    GetNextInt(&m_iOffsetZ, &pFieldbegin, -1);
  }

  if (',' == *pFieldbegin)
  {
    pFieldbegin++;
    GetNextInt(&m_iLenZ, &pFieldbegin, -1);
  }

  if (',' == *pFieldbegin)
  {
    pFieldbegin++;
    GetNextFloat(&m_fScalingZ, &pFieldbegin, -1);
  }
}

// See header file for function documentation
bool AccelCONF1Config::CfgValid() const
{
  return SensorConfig::CfgValid() &&
         VehicleNetworkConfiguration::eACCEL_CONFIG_CONF1 == m_qCfgVersion &&
         0 <= m_iOffsetX && 0 <= m_iOffsetY && 0 <= m_iOffsetZ;
  // Negative length will be used to support byte swap
}

// See header file for function documentation
void AccelCONF1Config::ToString(char* pStr, size_t sLen) const
{
  if (0 == pStr || 0 == sLen)
  {
    return;
  }

  char pzTStr[1024];
  SensorConfig::ToString(pzTStr, sizeof(pzTStr));
  snprintf(pStr, sLen, "[%s, %d, %d, %d, %.6E, %d, %d, %.6E, %d, %d, %.6E]",
           pzTStr, m_iFrameId, m_iOffsetX, m_iLenX, m_fScalingX, m_iOffsetY,
           m_iLenY, m_fScalingY, m_iOffsetZ, m_iLenZ, m_fScalingZ);
}

/**
@brief Loads Baching configuration deatils for CAN frames.

This method Loads Baching configuration deatils for CAN frames.

@param[in] pStrBatchType   configuration string for Batch type
@param[in] pStrBatchValue  configuration string for Batch Values
*/
void AccelCONF1Config::loadBatchConfig(const char* pStrBatchType, const char* pStrBatchValue)
{
  /* Batch type is default initilized to Time based batching */
  if ( 0 != pStrBatchType )
  {
    SensorConfig::Load(pStrBatchType);

    if (0 == m_pCfgStr)
    {
      SLIM_LOGW("loadBatchConfig parse Fail Type:%s Value:%s", pStrBatchType, pStrBatchValue);
      return;
    }
    m_iBatchType = 1;
    GetNextInt(&m_iBatchType, &m_pCfgStr, SLIM_VNW_BATCH_TYPE_DEFAULT);
  }

  /* Batch type is default initilized to Time based batching with batch period of 100ms*/
  if ( 0 != pStrBatchValue )
   {
    SensorConfig::Load(pStrBatchValue);

    if (0 == m_pCfgStr)
    {
      SLIM_LOGW("loadBatchConfig parse Fail Type:%s Value:%s", pStrBatchType, pStrBatchValue);
      return;
    }
    m_iBatchSize = 0;
    GetNextInt(&m_iBatchSize, &m_pCfgStr, SLIM_VNW_BATCH_VAL_DEFAULT);
    SLIM_LOGD("loadBatchConfig type:%d    val:%d", m_iBatchType, m_iBatchSize);
  }
}

void GyroCONF1Config:: GetGVecMetaByAxis(GyroValRead* pz_ValConfig, char** pFBegin, int iDefault)
{
  if (NULL == pFBegin || NULL == *pFBegin) {
    return;
  }
  if (',' == **pFBegin) {
    (*pFBegin)++;
    GetNextInt(&pz_ValConfig->m_iOffset, pFBegin, iDefault);
  }
  if (',' == **pFBegin) {
    (*pFBegin)++;
    GetNextInt(&pz_ValConfig->m_iLen, pFBegin, iDefault);
  }
  if (',' == **pFBegin) {
    (*pFBegin)++;
    GetNextFloat(&pz_ValConfig->m_fScaling, pFBegin, iDefault);
  }

}


void GyroCONF1Config:: GetNextError(BitErrorStructType* pz_ErrValConfig,
                                    char** pFBegin,
                                    int iDefault)
{
  if (NULL == pFBegin || NULL == *pFBegin) {
    return;
  }
  if (',' == **pFBegin) {
    (*pFBegin)++;
    GetNextInt(&pz_ErrValConfig->m_iBitWiseErrorStartBit, pFBegin, iDefault);
  }
  if (',' == **pFBegin) {
    (*pFBegin)++;
    GetNextInt(&pz_ErrValConfig->m_iBitWiseErrorLength, pFBegin, iDefault);
  }
  if (',' == **pFBegin) {
    (*pFBegin)++;
    GetNextInt(&pz_ErrValConfig->m_iBitWiseErrorValue, pFBegin, iDefault);
  }
}


/* -------------------------------------------------------------------------- */
// See header file for function documentation
GyroCONF1Config::GyroCONF1Config()
  : SensorConfig(VehicleNetworkConfiguration::eGYRO_CONFIG_CONF1)
  , m_iFrameId(-1)
  ,m_iByteOrder(0)
  ,m_iSign(0)
  ,m_iSignStartBit(0)
  ,m_iSignLength(0)
  ,m_iSignVal(0)
  ,m_fGyroMin(0)
  ,m_fGyroMax(INT_MAX)
  ,m_fAdditiveOffset(0)
  ,m_fScalingOffset(1.0F)
  ,m_iUnit(0)
  ,m_iNumAbsError(0)
  ,m_iNumBitWiseError(0)
  ,m_pfAbsErrors(NULL)
  ,m_pzBitWiseErrors(NULL)
  ,m_zGyroConfig{0}
{
  m_iBatchType = SLIM_VNW_BATCH_TYPE_DEFAULT;
  m_iBatchSize = SLIM_VNW_BATCH_VAL_DEFAULT;
}

GyroCONF1Config::~GyroCONF1Config()
{
  if (NULL != m_pfAbsErrors){
    delete[] m_pfAbsErrors;
  }
  if (NULL != m_pzBitWiseErrors){
    delete[] m_pzBitWiseErrors;
  }
}

// See header file for function documentation
void GyroCONF1Config::Load(const char* pStr)
{
  if (0 == pStr || 0 == *pStr)
  {
    return;
  }

  SensorConfig::Load(pStr);

  if (0 == m_pCfgStr)
  {
    return;
  }

  char* pFieldbegin = strchr(m_pCfgStr, ':');
  if (0 == pFieldbegin)
  {
    return;
  }

  pFieldbegin++;
  GetNextInt(&m_iFrameId, &pFieldbegin, -1);

  if (',' == *pFieldbegin)
  {
    pFieldbegin++;
    GetNextInt(&m_iByteOrder, &pFieldbegin, -1);
  }

  if (',' == *pFieldbegin)
  {
    pFieldbegin++;
    GetNextInt(&m_iSign, &pFieldbegin, -1);
  }

  if (',' == *pFieldbegin)
  {
    pFieldbegin++;
    GetNextInt(&m_iSignStartBit, &pFieldbegin, -1);
  }

  if (',' == *pFieldbegin)
  {
    pFieldbegin++;
    GetNextInt(&m_iSignLength, &pFieldbegin, -1);
  }

  if (',' == *pFieldbegin)
  {
    pFieldbegin++;
    GetNextInt(&m_iSignVal, &pFieldbegin, -1);
  }

  if (',' == *pFieldbegin)
  {
    pFieldbegin++;
    GetNextInt(&m_iUnit, &pFieldbegin, -1);
  }

  if (',' == *pFieldbegin)
  {
    pFieldbegin++;
    GetNextFloat(&m_fGyroMin, &pFieldbegin, -1);
  }

  if (',' == *pFieldbegin)
  {
    pFieldbegin++;
    GetNextFloat(&m_fGyroMax, &pFieldbegin, -1);
  }

  if (',' == *pFieldbegin)
  {
    pFieldbegin++;
    GetNextFloat(&m_fAdditiveOffset, &pFieldbegin, -1);
  }

  if (',' == *pFieldbegin)
  {
    pFieldbegin++;
    GetNextFloat(&m_fScalingOffset, &pFieldbegin, -1);
  }

  if (',' == *pFieldbegin)
  {
    pFieldbegin++;
    GetNextInt(&m_iNumAbsError, &pFieldbegin, -1);
  }

  /*abs error assumed to be Integers*/
  if (m_iNumAbsError > 0)
  {
    m_pfAbsErrors = new float[std::min(m_iNumAbsError,5)];
  }
  int LoopCnt;
  float TempFloat;
  for ( LoopCnt=0;LoopCnt < m_iNumAbsError; LoopCnt++)
  {
    if (',' == *pFieldbegin)
    {
      pFieldbegin++;
    }
    else
    {
      break;
    }

    if (m_pfAbsErrors != NULL && LoopCnt < 5)
    {
      GetNextFloat(&m_pfAbsErrors[LoopCnt], &pFieldbegin, -1);
    }
    else if (LoopCnt >= 5)
    {
      GetNextFloat(&TempFloat, &pFieldbegin, -1);
    }
  }


  if (',' == *pFieldbegin)
  {
    pFieldbegin++;
    GetNextInt(&m_iNumBitWiseError, &pFieldbegin, -1);
  }

  if ( m_iNumBitWiseError > 0 )
  {
    m_pzBitWiseErrors = new BitErrorStructType[std::min(m_iNumBitWiseError,5)];
  }
  for ( LoopCnt = 0; LoopCnt < m_iNumBitWiseError; LoopCnt++)
  {
    if ( m_pzBitWiseErrors != NULL && LoopCnt < 5)
    {
      GetNextError(&m_pzBitWiseErrors[LoopCnt], &pFieldbegin, -1);
    }
    else if(LoopCnt >= 5)
    {
      BitErrorStructType TempBitError;
      GetNextError(&TempBitError, &pFieldbegin, -1);
    }
  }

  GetGVecMetaByAxis(&m_zGyroConfig[0], &pFieldbegin, -1);

  GetGVecMetaByAxis(&m_zGyroConfig[1], &pFieldbegin, -1);

  GetGVecMetaByAxis(&m_zGyroConfig[2], &pFieldbegin, -1);
  SLIM_LOGD("Length of G-Config %d,%s", strlen(pStr), pStr);

}

// See header file for function documentation
bool GyroCONF1Config::CfgValid() const
{
  return SensorConfig::CfgValid() &&
         VehicleNetworkConfiguration::eGYRO_CONFIG_CONF1 == m_qCfgVersion &&
         0 <= m_zGyroConfig[0].m_iOffset && 0 <= m_zGyroConfig[1].m_iOffset && 0 <= m_zGyroConfig[2].m_iOffset;
  // Negative length will be used to support byte swap
}

// See header file for function documentation
void GyroCONF1Config::ToString(char* pStr, size_t sLen) const
{
  if (0 == pStr || 0 == sLen)
  {
    return;
  }

  char pzTStr[1024];
  SensorConfig::ToString(pzTStr, sizeof(pzTStr));
  snprintf(pStr, sLen, "[%s,G-Config %d %d %d %d %d %d %f %f %f , %d, %d, %.6E, %d, %d, %.6E, %d, %d, %.6E] AErr %d,",
      pzTStr, m_iFrameId,m_iByteOrder,m_iSign,m_iSignStartBit,m_iSignLength,m_iSignVal,m_fGyroMin,m_fGyroMax,m_fAdditiveOffset,
      m_zGyroConfig[0].m_iOffset, m_zGyroConfig[0].m_iLen,m_zGyroConfig[0].m_fScaling, m_zGyroConfig[1].m_iOffset,
      m_zGyroConfig[1].m_iLen, m_zGyroConfig[1].m_fScaling, m_zGyroConfig[2].m_iOffset, m_zGyroConfig[2].m_iLen, m_zGyroConfig[2].m_fScaling,
      m_iNumAbsError);

  int iStartFrom = strlen(pStr), LoopCnt;
  for (LoopCnt=0; LoopCnt < m_iNumAbsError; LoopCnt++)
  {
    iStartFrom += snprintf(&pStr[iStartFrom], sLen, "%f," , m_pfAbsErrors[LoopCnt]);
  }
  iStartFrom += snprintf(&pStr[iStartFrom], sLen, "DErr %d,", m_iNumBitWiseError);
  for (LoopCnt=0; LoopCnt < m_iNumBitWiseError; LoopCnt++)
  {
    iStartFrom += snprintf(&pStr[iStartFrom], sLen, "(%d,%d,%d),", m_pzBitWiseErrors[LoopCnt].m_iBitWiseErrorStartBit,
        m_pzBitWiseErrors[LoopCnt].m_iBitWiseErrorLength,
        m_pzBitWiseErrors[LoopCnt].m_iBitWiseErrorValue);
  }
}

/**
@brief Loads Baching configuration deatils for CAN frames.

This method Loads Baching configuration deatils for CAN frames.

@param[in] pStrBatchType   configuration string for Batch type
@param[in] pStrBatchValue  configuration string for Batch Values
*/
void GyroCONF1Config::loadBatchConfig(const char* pStrBatchType, const char* pStrBatchValue)
{
  /* Batch type is default initilized to Time based batching */
  if ( 0 != pStrBatchType )
  {
    SensorConfig::Load(pStrBatchType);

    if (0 == m_pCfgStr)
    {
      SLIM_LOGW("loadBatchConfig parse Fail Type:%s Value:%s", pStrBatchType, pStrBatchValue);
      return;
    }
    m_iBatchType = 1;
    GetNextInt(&m_iBatchType, &m_pCfgStr, SLIM_VNW_BATCH_TYPE_DEFAULT);
  }

  /* Batch type is default initilized to Time based batching with batch period of 100ms*/
  if ( 0 != pStrBatchValue )
   {
    SensorConfig::Load(pStrBatchValue);

    if (0 == m_pCfgStr)
    {
      SLIM_LOGW("loadBatchConfig parse Fail Type:%s Value:%s", pStrBatchType, pStrBatchValue);
      return;
    }
    m_iBatchSize = 0;
    GetNextInt(&m_iBatchSize, &m_pCfgStr, SLIM_VNW_BATCH_VAL_DEFAULT);
    SLIM_LOGD("loadBatchConfig type:%d    val:%d", m_iBatchType, m_iBatchSize);
  }
}
/* -------------------------------------------------------------------------- */
// See header file for function documentation
OdometryCONF1Config::OdometryCONF1Config()
  : SensorConfig(VehicleNetworkConfiguration::eODOMETRY_CONFIG_CONF1)
  , m_iFrameId(-1)
  , m_iReverseBitOffset(0)
  , m_iSourceOffset(0)
  , m_iSourceLen(0)
  , m_iDistanceOffset(0)
  , m_iDistanceLen(0)
{
  m_iBatchType = SLIM_VNW_BATCH_TYPE_DEFAULT;
  m_iBatchSize = SLIM_VNW_BATCH_VAL_DEFAULT;
}

// See header file for function documentation
void OdometryCONF1Config::Load(const char* pStr)
{
  if (0 == pStr || 0 == *pStr)
  {
    return;
  }

  SensorConfig::Load(pStr);

  if (0 == m_pCfgStr)
  {
    return;
  }

  char* pFieldbegin = strchr(m_pCfgStr, ':');
  if (0 == pFieldbegin)
  {
    return;
  }

  pFieldbegin++;
  GetNextInt(&m_iFrameId, &pFieldbegin, -1);

  if (',' == *pFieldbegin)
  {
    pFieldbegin++;
    GetNextInt(&m_iReverseBitOffset, &pFieldbegin, -1);
  }

  if (',' == *pFieldbegin)
  {
    pFieldbegin++;
    GetNextInt(&m_iSourceOffset, &pFieldbegin, -1);
  }
  if (',' == *pFieldbegin)
  {
    pFieldbegin++;
    GetNextInt(&m_iSourceLen, &pFieldbegin, -1);
  }
  if (',' == *pFieldbegin)
  {
    pFieldbegin++;
    GetNextInt(&m_iDistanceOffset, &pFieldbegin, -1);
  }
  if (',' == *pFieldbegin)
  {
    pFieldbegin++;
    GetNextInt(&m_iDistanceLen, &pFieldbegin, -1);
  }
}

// See header file for function documentation
bool OdometryCONF1Config::CfgValid() const
{
  return SensorConfig::CfgValid() &&
      VehicleNetworkConfiguration::eODOMETRY_CONFIG_CONF1 == m_qCfgVersion &&
      0 <= m_iReverseBitOffset && 0 <= m_iSourceOffset && 0 < m_iSourceLen &&
      0 <= m_iDistanceOffset && 0 < m_iDistanceLen;
}

// See header file for function documentation
void OdometryCONF1Config::ToString(char* pStr, size_t sLen) const
{
  if (0 == pStr || 0 == sLen)
  {
    return;
  }

  char pzTStr[1024];
  SensorConfig::ToString(pzTStr, sizeof(pzTStr));
  snprintf(pStr, sLen, "[%s, %d, %d, %d, %d, %d, %d]",
           pzTStr, m_iFrameId, m_iReverseBitOffset, m_iSourceOffset,
           m_iSourceLen, m_iDistanceOffset, m_iDistanceLen);
}

/**
@brief Loads Baching configuration deatils for CAN frames.

This method Loads Baching configuration deatils for CAN frames.

@param[in] pStrBatchType   configuration string for Batch type
@param[in] pStrBatchValue  configuration string for Batch Values
*/
void OdometryCONF1Config::loadBatchConfig(const char* pStrBatchType, const char* pStrBatchValue)
{
  /* Batch type is default initilized to Time based batching */
  if ( 0 != pStrBatchType )
  {
    SensorConfig::Load(pStrBatchType);

    if (0 == m_pCfgStr)
    {
      SLIM_LOGW("loadBatchConfig parse Fail Type:%s Value:%s", pStrBatchType, pStrBatchValue);
      return;
    }
    m_iBatchType = 1;
    GetNextInt(&m_iBatchType, &m_pCfgStr, SLIM_VNW_BATCH_TYPE_DEFAULT);
  }

  /* Batch type is default initilized to Time based batching with batch period of 100ms*/
  if ( 0 != pStrBatchValue )
   {
    SensorConfig::Load(pStrBatchValue);

    if (0 == m_pCfgStr)
    {
      SLIM_LOGW("loadBatchConfig parse Fail Type:%s Value:%s", pStrBatchType, pStrBatchValue);
      return;
    }
    m_iBatchSize = 0;
    GetNextInt(&m_iBatchSize, &m_pCfgStr, SLIM_VNW_BATCH_VAL_DEFAULT);
    SLIM_LOGD("loadBatchConfig type:%d    val:%d", m_iBatchType, m_iBatchSize);
  }
}

/* -------------------------------------------------------------------------- */
// See header file for function documentation
SpeedCONF1Config::SpeedCONF1Config()
  : SensorConfig(VehicleNetworkConfiguration::eSPEED_CONFIG_CONF1)
{
  m_iFrameId = -1;
  m_numberofSpeedConfig = 0;
  m_unitOffset = 0;
  m_unitLength = 0;
  memset(m_speedConfig,0,sizeof(m_speedConfig));
  m_iBatchType = SLIM_VNW_BATCH_TYPE_DEFAULT;
  m_iBatchSize = SLIM_VNW_BATCH_VAL_DEFAULT;
}

// See header file for function documentation
void SpeedCONF1Config::Load(const char* pStr)
{
  int tempInt;
  float tempFloat;

  if (0 == pStr || 0 == *pStr)
  {
    return;
  }

  SensorConfig::Load(pStr);

  if (0 == m_pCfgStr)
  {
    return;
  }

  m_numberofSpeedConfig = 0;
  m_unitOffset = 0;
  m_unitLength = 0;
  memset(m_speedConfig,0,sizeof(m_speedConfig));

  char* pFieldbegin = strchr(m_pCfgStr, ':');
  if (0 == pFieldbegin)
  {
    return;
  }

  pFieldbegin++;
  GetNextInt(&m_iFrameId, &pFieldbegin, -1);

  if (',' == *pFieldbegin)
  {
    pFieldbegin++;
    GetNextInt(&tempInt, &pFieldbegin, 0);
    m_unitOffset = static_cast<uint8_t>(tempInt);
  }

  if (',' == *pFieldbegin)
  {
    pFieldbegin++;
    GetNextInt(&tempInt, &pFieldbegin, 0);
    m_unitLength= static_cast<uint8_t>(tempInt);
  }

  if (',' == *pFieldbegin)
  {
    pFieldbegin++;
    GetNextInt(&tempInt, &pFieldbegin, 0);
    m_numberofSpeedConfig = static_cast<uint8_t>(tempInt);
  }


  for(int i=0;i<m_numberofSpeedConfig;i++)
  {

    if (',' == *pFieldbegin)
    {
      pFieldbegin++;
      GetNextInt(&tempInt, &pFieldbegin, 0);
      m_speedConfig[i].speedUnit = static_cast<uint8_t>(tempInt);
    }


    if (',' == *pFieldbegin)
    {
      pFieldbegin++;
      GetNextInt(&tempInt, &pFieldbegin, 0);
      m_speedConfig[i].unitValue= static_cast<uint8_t>(tempInt);
    }

    if (',' == *pFieldbegin)
    {
      pFieldbegin++;
      GetNextFloat(&tempFloat, &pFieldbegin, -1);
      m_speedConfig[i].scale= tempFloat;
    }

    if (',' == *pFieldbegin)
    {
      pFieldbegin++;
      GetNextFloat(&tempFloat, &pFieldbegin, -1);
      m_speedConfig[i].offset= tempFloat;
    }

    if (',' == *pFieldbegin)
    {
      pFieldbegin++;
      GetNextInt(&tempInt, &pFieldbegin, 0);
      m_speedConfig[i].speedOffsetLowBits = static_cast<uint8_t>(tempInt);
    }

    if (',' == *pFieldbegin)
    {
      pFieldbegin++;
      GetNextInt(&tempInt, &pFieldbegin, 0);
      m_speedConfig[i].speedLengthLowBits = static_cast<uint8_t>(tempInt);
    }

    if (',' == *pFieldbegin)
    {
      pFieldbegin++;
      GetNextInt(&tempInt, &pFieldbegin, 0);
      m_speedConfig[i].speedOffsetUpBits = static_cast<uint8_t>(tempInt);
    }

    if (',' == *pFieldbegin)
    {
      pFieldbegin++;
      GetNextInt(&tempInt, &pFieldbegin, 0);
      m_speedConfig[i].speedLengthUpBits = static_cast<uint8_t>(tempInt);
    }
  }
}

// See header file for function documentation
bool SpeedCONF1Config::CfgValid() const
{
  return SensorConfig::CfgValid() &&
         VehicleNetworkConfiguration::eSPEED_CONFIG_CONF1 == m_qCfgVersion &&
         0 < m_numberofSpeedConfig && m_numberofSpeedConfig<=3;
}

// See header file for function documentation
void SpeedCONF1Config::ToString(char* pStr, size_t sLen) const
{
  if (0 == pStr || 0 == sLen)
  {
    return;
  }

  char pzTStr[1024];
  SensorConfig::ToString(pzTStr, sizeof(pzTStr));
  snprintf(pStr, sLen, "[%s, %d, %hhu, %hhu, %hhu,"
             "%hhu, %hhu,  %.6E,  %.6E, %hhu, %hhu, %hhu, %hhu,"
             "%hhu, %hhu,  %.6E,  %.6E, %hhu, %hhu, %hhu, %hhu,"
             "%hhu, %hhu,  %.6E,  %.6E, %hhu, %hhu, %hhu, %hhu]",
              pzTStr, m_iFrameId, m_unitOffset, m_unitLength, m_numberofSpeedConfig,
              m_speedConfig[0].speedUnit,m_speedConfig[0].unitValue,
              m_speedConfig[0].scale, m_speedConfig[0].offset, m_speedConfig[0].speedOffsetLowBits, m_speedConfig[0].speedLengthLowBits,
              m_speedConfig[0].speedOffsetUpBits, m_speedConfig[0].speedLengthUpBits,
              m_speedConfig[1].speedUnit, m_speedConfig[1].unitValue,
              m_speedConfig[1].scale, m_speedConfig[1].offset, m_speedConfig[1].speedOffsetLowBits, m_speedConfig[1].speedLengthLowBits,
              m_speedConfig[1].speedOffsetUpBits, m_speedConfig[1].speedLengthUpBits,
              m_speedConfig[2].speedUnit, m_speedConfig[2].unitValue,
              m_speedConfig[2].scale, m_speedConfig[2].offset, m_speedConfig[2].speedOffsetLowBits, m_speedConfig[2].speedLengthLowBits,
              m_speedConfig[2].speedOffsetUpBits, m_speedConfig[2].speedLengthUpBits);
}

/**
@brief Loads Baching configuration deatils for CAN frames.

This method Loads Baching configuration deatils for CAN frames.

@param[in] pStrBatchType   configuration string for Batch type
@param[in] pStrBatchValue  configuration string for Batch Values
*/
void SpeedCONF1Config::loadBatchConfig(const char* pStrBatchType, const char* pStrBatchValue)
{
  /* Batch type is default initilized to Time based batching */
  if ( 0 != pStrBatchType )
  {
    SensorConfig::Load(pStrBatchType);

    if (0 == m_pCfgStr)
    {
      SLIM_LOGW("loadBatchConfig parse Fail Type:%s Value:%s", pStrBatchType, pStrBatchValue);
      return;
    }
    m_iBatchType = 1;
    GetNextInt(&m_iBatchType, &m_pCfgStr, SLIM_VNW_BATCH_TYPE_DEFAULT);
  }

  /* Batch type is default initilized to Time based batching with batch period of 100ms*/
  if ( 0 != pStrBatchValue )
   {
    SensorConfig::Load(pStrBatchValue);

    if (0 == m_pCfgStr)
    {
      SLIM_LOGW("loadBatchConfig parse Fail Type:%s Value:%s", pStrBatchType, pStrBatchValue);
      return;
    }
    m_iBatchSize = 0;
    GetNextInt(&m_iBatchSize, &m_pCfgStr, SLIM_VNW_BATCH_VAL_DEFAULT);
    SLIM_LOGD("loadBatchConfig type:%d    val:%d", m_iBatchType, m_iBatchSize);
  }
}
/* -------------------------------------------------------------------------- */
// See header file for function documentation
DwsCONF1Config::DwsCONF1Config()
  : SensorConfig(VehicleNetworkConfiguration::eDWS_CONFIG_CONF1)
{
  m_iFrameId = -1;
  m_numberofSpeedConfig = 0;
  memset(m_dwsConfig,0,sizeof(m_dwsConfig));
  m_iBatchType = SLIM_VNW_BATCH_TYPE_DEFAULT;
  m_iBatchSize = SLIM_VNW_BATCH_VAL_DEFAULT;
}

// See header file for function documentation
void DwsCONF1Config::Load(const char* pStr)
{
  int tempInt;
  float tempFloat;

  if (0 == pStr || 0 == *pStr)
  {
    return;
  }

  SensorConfig::Load(pStr);

  if (0 == m_pCfgStr)
  {
    return;
  }

  m_numberofSpeedConfig = 0;
  memset(m_dwsConfig,0,sizeof(m_dwsConfig));

  char* pFieldbegin = strchr(m_pCfgStr, ':');
  if (0 == pFieldbegin)
  {
    return;
  }

  pFieldbegin++;
  GetNextInt(&m_iFrameId, &pFieldbegin, -1);

  if (',' == *pFieldbegin)
  {
    pFieldbegin++;
    GetNextInt(&tempInt, &pFieldbegin, 0);
    m_unitOffset = static_cast<uint8_t>(tempInt);
  }

  if (',' == *pFieldbegin)
  {
    pFieldbegin++;
    GetNextInt(&tempInt, &pFieldbegin, 0);
    m_unitLength = static_cast<uint8_t>(tempInt);
  }

  if (',' == *pFieldbegin)
  {
    pFieldbegin++;
    GetNextInt(&tempInt, &pFieldbegin, 0);
    m_numberofSpeedConfig = static_cast<uint8_t>(tempInt);
  }


  for(int i=0;i<m_numberofSpeedConfig;i++)
  {

    if (',' == *pFieldbegin)
    {
      pFieldbegin++;
      GetNextInt(&tempInt, &pFieldbegin, 0);
     m_dwsConfig[i].speedUnit = static_cast<uint8_t>(tempInt);
    }

    if (',' == *pFieldbegin)
    {
      pFieldbegin++;
      GetNextInt(&tempInt, &pFieldbegin, 0);
      m_dwsConfig[i].unitValue= static_cast<uint8_t>(tempInt);
    }

    if (',' == *pFieldbegin)
    {
      pFieldbegin++;
      GetNextFloat(&tempFloat, &pFieldbegin, -1);
      m_dwsConfig[i].scale= tempFloat;
    }

    if (',' == *pFieldbegin)
    {
      pFieldbegin++;
      GetNextFloat(&tempFloat, &pFieldbegin, -1);
      m_dwsConfig[i].offset= tempFloat;
    }

    if (',' == *pFieldbegin)
    {
      pFieldbegin++;
      GetNextInt(&tempInt, &pFieldbegin, 0);
      m_dwsConfig[i].speedOffsetLowBitsFL = static_cast<uint8_t>(tempInt);
    }

    if (',' == *pFieldbegin)
    {
      pFieldbegin++;
      GetNextInt(&tempInt, &pFieldbegin, 0);
      m_dwsConfig[i].speedLengthLowBitsFL = static_cast<uint8_t>(tempInt);
    }

    if (',' == *pFieldbegin)
    {
      pFieldbegin++;
      GetNextInt(&tempInt, &pFieldbegin, 0);
      m_dwsConfig[i].speedOffsetUpBitsFL = static_cast<uint8_t>(tempInt);
    }

    if (',' == *pFieldbegin)
    {
      pFieldbegin++;
      GetNextInt(&tempInt, &pFieldbegin, 0);
      m_dwsConfig[i].speedLengthUpBitsFL = static_cast<uint8_t>(tempInt);
    }

     if (',' == *pFieldbegin)
    {
      pFieldbegin++;
      GetNextInt(&tempInt, &pFieldbegin, 0);
      m_dwsConfig[i].speedOffsetLowBitsFR = static_cast<uint8_t>(tempInt);
    }

    if (',' == *pFieldbegin)
    {
      pFieldbegin++;
      GetNextInt(&tempInt, &pFieldbegin, 0);
      m_dwsConfig[i].speedLengthLowBitsFR = static_cast<uint8_t>(tempInt);
    }

    if (',' == *pFieldbegin)
    {
      pFieldbegin++;
      GetNextInt(&tempInt, &pFieldbegin, 0);
      m_dwsConfig[i].speedOffsetUpBitsFR= static_cast<uint8_t>(tempInt);
    }

    if (',' == *pFieldbegin)
    {
      pFieldbegin++;
      GetNextInt(&tempInt, &pFieldbegin, 0);
      m_dwsConfig[i].speedLengthUpBitsFR = static_cast<uint8_t>(tempInt);
    }

    if (',' == *pFieldbegin)
    {
      pFieldbegin++;
      GetNextInt(&tempInt, &pFieldbegin, 0);
      m_dwsConfig[i].speedOffsetLowBitsRL = static_cast<uint8_t>(tempInt);
    }

    if (',' == *pFieldbegin)
    {
      pFieldbegin++;
      GetNextInt(&tempInt, &pFieldbegin, 0);
      m_dwsConfig[i].speedLengthLowBitsRL = static_cast<uint8_t>(tempInt);
    }

    if (',' == *pFieldbegin)
    {
      pFieldbegin++;
      GetNextInt(&tempInt, &pFieldbegin, 0);
      m_dwsConfig[i].speedOffsetUpBitsRL = static_cast<uint8_t>(tempInt);
    }

    if (',' == *pFieldbegin)
    {
      pFieldbegin++;
      GetNextInt(&tempInt, &pFieldbegin, 0);
      m_dwsConfig[i].speedLengthUpBitsRL = static_cast<uint8_t>(tempInt);
    }

     if (',' == *pFieldbegin)
    {
      pFieldbegin++;
      GetNextInt(&tempInt, &pFieldbegin, 0);
      m_dwsConfig[i].speedOffsetLowBitsRR = static_cast<uint8_t>(tempInt);
    }

    if (',' == *pFieldbegin)
    {
      pFieldbegin++;
      GetNextInt(&tempInt, &pFieldbegin, 0);
      m_dwsConfig[i].speedLengthLowBitsRR = static_cast<uint8_t>(tempInt);
    }

    if (',' == *pFieldbegin)
    {
      pFieldbegin++;
      GetNextInt(&tempInt, &pFieldbegin, 0);
      m_dwsConfig[i].speedOffsetUpBitsRR = static_cast<uint8_t>(tempInt);
    }

    if (',' == *pFieldbegin)
    {
      pFieldbegin++;
      GetNextInt(&tempInt, &pFieldbegin, 0);
      m_dwsConfig[i].speedLengthUpBitsRR = static_cast<uint8_t>(tempInt);
    }
  }
}

// See header file for function documentation
bool DwsCONF1Config::CfgValid() const
{
  return SensorConfig::CfgValid() &&
         VehicleNetworkConfiguration::eDWS_CONFIG_CONF1 == m_qCfgVersion &&
         0 < m_numberofSpeedConfig && m_numberofSpeedConfig<=3;
}

// See header file for function documentation
void DwsCONF1Config::ToString(char* pStr, size_t sLen) const
{
  if (0 == pStr || 0 == sLen)
  {
    return;
  }

  char pzTStr[1024];
  SensorConfig::ToString(pzTStr, sizeof(pzTStr));
  snprintf(pStr, sLen, "[%s, %d, %hhu, %hhu, %hhu,"
                   "%hhu, %hhu,  %.6E,  %.6E, %hhu, %hhu, %hhu, %hhu,%hhu, %hhu, %hhu, %hhu,%hhu, %hhu, %hhu, %hhu,%hhu, %hhu, %hhu, %hhu,"
                   "%hhu, %hhu,  %.6E,  %.6E, %hhu, %hhu, %hhu, %hhu,%hhu, %hhu, %hhu, %hhu,%hhu, %hhu, %hhu, %hhu,%hhu, %hhu, %hhu, %hhu,"
                   "%hhu, %hhu,  %.6E,  %.6E, %hhu, %hhu, %hhu, %hhu,%hhu, %hhu, %hhu, %hhu,%hhu, %hhu, %hhu, %hhu,%hhu, %hhu, %hhu, %hhu ]",
              pzTStr, m_iFrameId, m_unitOffset, m_unitLength, m_numberofSpeedConfig,
              m_dwsConfig[0].speedUnit, m_dwsConfig[0].unitValue,
              m_dwsConfig[0].scale, m_dwsConfig[0].offset, m_dwsConfig[0].speedOffsetLowBitsFL, m_dwsConfig[0].speedLengthLowBitsFL,
              m_dwsConfig[0].speedOffsetUpBitsFL, m_dwsConfig[0].speedLengthUpBitsFL,m_dwsConfig[0].speedOffsetLowBitsFR, m_dwsConfig[0].speedLengthLowBitsFR,
              m_dwsConfig[0].speedOffsetUpBitsFR, m_dwsConfig[0].speedLengthUpBitsFR,m_dwsConfig[0].speedOffsetLowBitsRL, m_dwsConfig[0].speedLengthLowBitsRL,
              m_dwsConfig[0].speedOffsetUpBitsRL, m_dwsConfig[0].speedLengthUpBitsRL,m_dwsConfig[0].speedOffsetLowBitsRR, m_dwsConfig[0].speedLengthLowBitsRR,
              m_dwsConfig[0].speedOffsetUpBitsRR, m_dwsConfig[0].speedLengthUpBitsRR,
              m_dwsConfig[1].speedUnit, m_dwsConfig[1].unitValue,
              m_dwsConfig[1].scale, m_dwsConfig[1].offset, m_dwsConfig[1].speedOffsetLowBitsFL, m_dwsConfig[1].speedLengthLowBitsFL,
              m_dwsConfig[1].speedOffsetUpBitsFL, m_dwsConfig[1].speedLengthUpBitsFL,m_dwsConfig[1].speedOffsetLowBitsFR, m_dwsConfig[1].speedLengthLowBitsFR,
              m_dwsConfig[1].speedOffsetUpBitsFR, m_dwsConfig[1].speedLengthUpBitsFR,m_dwsConfig[1].speedOffsetLowBitsRL, m_dwsConfig[1].speedLengthLowBitsRL,
              m_dwsConfig[1].speedOffsetUpBitsRL, m_dwsConfig[1].speedLengthUpBitsRL,m_dwsConfig[1].speedOffsetLowBitsRR, m_dwsConfig[1].speedLengthLowBitsRR,
              m_dwsConfig[1].speedOffsetUpBitsRR, m_dwsConfig[1].speedLengthUpBitsRR,
              m_dwsConfig[2].speedUnit, m_dwsConfig[2].unitValue,
              m_dwsConfig[2].scale, m_dwsConfig[2].offset, m_dwsConfig[2].speedOffsetLowBitsFL, m_dwsConfig[2].speedLengthLowBitsFL,
              m_dwsConfig[2].speedOffsetUpBitsFL, m_dwsConfig[2].speedLengthUpBitsFL,m_dwsConfig[2].speedOffsetLowBitsFR, m_dwsConfig[2].speedLengthLowBitsFR,
              m_dwsConfig[2].speedOffsetUpBitsFR, m_dwsConfig[2].speedLengthUpBitsFR,m_dwsConfig[2].speedOffsetLowBitsRL, m_dwsConfig[2].speedLengthLowBitsRL,
              m_dwsConfig[2].speedOffsetUpBitsRL, m_dwsConfig[2].speedLengthUpBitsRL,m_dwsConfig[2].speedOffsetLowBitsRR, m_dwsConfig[2].speedLengthLowBitsRR,
              m_dwsConfig[2].speedOffsetUpBitsRR, m_dwsConfig[2].speedLengthUpBitsRR);
}


/**
@brief Loads Baching configuration deatils for CAN frames.

This method Loads Baching configuration deatils for CAN frames.

@param[in] pStrBatchType   configuration string for Batch type
@param[in] pStrBatchValue  configuration string for Batch Values
*/
void DwsCONF1Config::loadBatchConfig(const char* pStrBatchType, const char* pStrBatchValue)
{
  /* Batch type is default initilized to Time based batching */
  if ( 0 != pStrBatchType )
  {
    SensorConfig::Load(pStrBatchType);

    if (0 == m_pCfgStr)
    {
      SLIM_LOGW("loadBatchConfig parse Fail Type:%s Value:%s", pStrBatchType, pStrBatchValue);
      return;
    }
    m_iBatchType = 1;
    GetNextInt(&m_iBatchType, &m_pCfgStr, SLIM_VNW_BATCH_TYPE_DEFAULT);
  }

  /* Batch type is default initilized to Time based batching with batch period of 100ms*/
  if ( 0 != pStrBatchValue )
   {
    SensorConfig::Load(pStrBatchValue);

    if (0 == m_pCfgStr)
    {
      SLIM_LOGW("loadBatchConfig parse Fail Type:%s Value:%s", pStrBatchType, pStrBatchValue);
      return;
    }
    m_iBatchSize = 0;
    GetNextInt(&m_iBatchSize, &m_pCfgStr, SLIM_VNW_BATCH_VAL_DEFAULT);
    SLIM_LOGD("loadBatchConfig type:%d    val:%d", m_iBatchType, m_iBatchSize);
  }
}
/* -------------------------------------------------------------------------- */
// See header file for function documentation
GearCONF1Config::GearCONF1Config()
  : SensorConfig(VehicleNetworkConfiguration::eGEAR_CONFIG_CONF1)
{
  m_iFrameId = -1;
  m_gearOffset = 0;
  m_gearLength = 0;
  m_parkValue = -1;
  m_reverseValue = -1;
  m_neutralValue = -1;
  m_numberDriveGear = 0;
  memset(m_driveValue,-1,sizeof(m_driveValue));
  m_iBatchType = SLIM_VNW_BATCH_TYPE_DEFAULT;
  m_iBatchSize = SLIM_VNW_BATCH_VAL_DEFAULT;
}

// See header file for function documentation
void GearCONF1Config::Load(const char* pStr)
{
  int tempInt;
  float tempFloat;

  if (0 == pStr || 0 == *pStr)
  {
    return;
  }

  SensorConfig::Load(pStr);

  if (0 == m_pCfgStr)
  {
    return;
  }

  m_gearOffset = 0;
  m_gearLength = 0;
  m_parkValue = -1;
  m_reverseValue = -1;
  m_neutralValue = -1;
  m_numberDriveGear = 0;
  memset(m_driveValue,-1,sizeof(m_driveValue));

  char* pFieldbegin = strchr(m_pCfgStr, ':');
  if (0 == pFieldbegin)
  {
    return;
  }

  pFieldbegin++;
  GetNextInt(&m_iFrameId, &pFieldbegin, -1);

  if (',' == *pFieldbegin)
  {
    pFieldbegin++;
    GetNextInt(&tempInt, &pFieldbegin, 0);
    m_gearOffset = static_cast<uint8_t>(tempInt);
  }

  if (',' == *pFieldbegin)
  {
    pFieldbegin++;
    GetNextInt(&tempInt, &pFieldbegin, 0);
    m_gearLength = static_cast<uint8_t>(tempInt);
  }

  if (',' == *pFieldbegin)
  {
    pFieldbegin++;
    GetNextInt(&tempInt, &pFieldbegin, -1);
    m_parkValue = static_cast<int8_t>(tempInt);
  }

  if (',' == *pFieldbegin)
  {
    pFieldbegin++;
    GetNextInt(&tempInt, &pFieldbegin, -1);
    m_reverseValue = static_cast<int8_t>(tempInt);
  }

   if (',' == *pFieldbegin)
  {
    pFieldbegin++;
    GetNextInt(&tempInt, &pFieldbegin, -1);
    m_neutralValue = static_cast<int8_t>(tempInt);
  }

  if (',' == *pFieldbegin)
  {
    pFieldbegin++;
    GetNextInt(&tempInt, &pFieldbegin, 0);
    m_numberDriveGear = static_cast<uint8_t>(tempInt);
  }


  for(int i=0;i<m_numberDriveGear;i++)
  {
     if (',' == *pFieldbegin)
    {
      pFieldbegin++;
      GetNextInt(&tempInt, &pFieldbegin, -1);
      m_driveValue[i] = static_cast<int8_t>(tempInt);
    }
  }
}

// See header file for function documentation
bool GearCONF1Config::CfgValid() const
{
  return SensorConfig::CfgValid() &&
         VehicleNetworkConfiguration::eGEAR_CONFIG_CONF1 == m_qCfgVersion &&
         0 < m_numberDriveGear && m_numberDriveGear<=10;
}

// See header file for function documentation
void GearCONF1Config::ToString(char* pStr, size_t sLen) const
{
  if (0 == pStr || 0 == sLen)
  {
    return;
  }

  char pzTStr[1024];
  SensorConfig::ToString(pzTStr, sizeof(pzTStr));
  snprintf(pStr, sLen, "[%s, %d, %hhu, %hhu, %hhd, %hhd, %hhd, %hhu,"
             "%hhd, %hhd, %hhd, %hhd, %hhd, %hhd, %hhd, %hhd, %hhd, %hhd]",
              pzTStr, m_iFrameId, m_gearOffset, m_gearLength,m_parkValue,m_reverseValue,m_neutralValue,m_numberDriveGear,
              m_driveValue[0],m_driveValue[1],m_driveValue[2],m_driveValue[3],m_driveValue[4],m_driveValue[5],m_driveValue[6],
              m_driveValue[7],m_driveValue[8],m_driveValue[9]);
}

/**
@brief Loads Baching configuration deatils for CAN frames.

This method Loads Baching configuration deatils for CAN frames.

@param[in] pStrBatchType   configuration string for Batch type
@param[in] pStrBatchValue  configuration string for Batch Values
*/
void GearCONF1Config::loadBatchConfig(const char* pStrBatchType, const char* pStrBatchValue)
{
  /* Batch type is default initilized to Time based batching */
  if ( 0 != pStrBatchType )
  {
    SensorConfig::Load(pStrBatchType);

    if (0 == m_pCfgStr)
    {
      SLIM_LOGW("loadBatchConfig parse Fail Type:%s Value:%s", pStrBatchType, pStrBatchValue);
      return;
    }
    m_iBatchType = 1;
    GetNextInt(&m_iBatchType, &m_pCfgStr, SLIM_VNW_BATCH_TYPE_DEFAULT);
  }

  /* Batch type is default initilized to Time based batching with batch period of 100ms*/
  if ( 0 != pStrBatchValue )
   {
    SensorConfig::Load(pStrBatchValue);

    if (0 == m_pCfgStr)
    {
      SLIM_LOGW("loadBatchConfig parse Fail Type:%s Value:%s", pStrBatchType, pStrBatchValue);
      return;
    }
    m_iBatchSize = 0;
    GetNextInt(&m_iBatchSize, &m_pCfgStr, SLIM_VNW_BATCH_VAL_DEFAULT);
    SLIM_LOGD("loadBatchConfig type:%d    val:%d", m_iBatchType, m_iBatchSize);
  }
}
}

