/******************************************************************************
@file    qcril_data_connectionservice.cc
@brief   qcril data connection service

DESCRIPTION
Basic implementaton of IDataConnection interface.
Register as data connection HAL service

******************************************************************************/

/*=============================================================================
  Copyright (c) 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

#include <cutils/properties.h>

#include "qcril_data_connectionservice.h"

extern "C" {
  #include "qcril_log.h"
}

using ::android::hardware::Status;

static sp<DataConnectionServiceImpl> dataConnectionService = NULL;


/*===========================================================================

FUNCTION:  DataConnectionServiceImpl::setInstanceId

===========================================================================*/
void DataConnectionServiceImpl::setInstanceId
(
  int instanceId
)
{
  mInstanceId = instanceId;
} /* DataConnectionServiceImpl::setInstanceId */

/*===========================================================================

FUNCTION:  qcril_dataconnectionservice_register_service

===========================================================================*/
extern "C" void qcril_dataconnectionservice_register_service
(
  int instanceId
)
{
  android::status_t ret = -1;
  dataConnectionService = new DataConnectionServiceImpl();
  dataConnectionService->setInstanceId(instanceId);
  if (instanceId == 0) {
    ret = dataConnectionService->registerAsService("slot1");
  } else if (instanceId == 1) {
    ret = dataConnectionService->registerAsService("slot2");
  }
  QCRIL_LOG_INFO("qcril_dataconnectionservice_register_service instanceId=%d status=%d", instanceId, ret);

} /* qcril_dataconnectionservice_register_service */

/*===========================================================================

FUNCTION: getConfig

 * @brief
 * This API queries the vendor property value
 * and returns it to the client

===========================================================================*/
Return<void> DataConnectionServiceImpl::getConfig
(
    const hidl_string& key,
    const hidl_string& defaultValue,
    getConfig_cb _hidl_cb
) {
    QCRIL_LOG_INFO("DataConnectionServiceImpl::getConfig");
    char prop[PROPERTY_VALUE_MAX] = {'\0'};
    property_get(key.c_str(), prop, defaultValue.c_str());
    std::string configValue(prop);
    _hidl_cb(configValue);
    return Void();
}/* DataConnectionServiceImpl::getConfig */

/*===========================================================================

FUNCTION:  DataConnectionServiceImpl::serviceDied

===========================================================================*/
void DataConnectionServiceImpl::serviceDied
(
  uint64_t,
  const ::android::wp<::android::hidl::base::V1_0::IBase> &
)
{
  QCRIL_LOG_INFO("DataConnectionServiceImpl::serviceDied: Client died.");
} /* DataConnectionServiceImpl::serviceDied */
