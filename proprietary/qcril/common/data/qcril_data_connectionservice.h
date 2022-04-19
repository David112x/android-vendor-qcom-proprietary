/******************************************************************************
@file    qcril_data_connectionservice.h
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

#include <vendor/qti/hardware/data/connection/1.1/IDataConnection.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

using ::android::hardware::Return;
using ::android::sp;
using ::android::wp;
using ::android::hardware::hidl_string;
using ::android::hardware::Void;

using vendor::qti::hardware::data::connection::V1_1::IDataConnection;
using vendor::qti::hardware::data::connection::V1_0::IDataConnectionIndication;
using vendor::qti::hardware::data::connection::V1_0::IDataConnectionResponse;
using vendor::qti::hardware::data::connection::V1_0::StatusCode;
using vendor::qti::hardware::data::connection::V1_0::ErrorReason;
using vendor::qti::hardware::data::connection::V1_0::RatType;
using vendor::qti::hardware::data::connection::V1_0::AllocatedBearers;
using vendor::qti::hardware::data::connection::V1_0::BearerInfo;

class DataConnectionServiceImpl : public IDataConnection {

public:

  DataConnectionServiceImpl() {}

  Return<StatusCode>
      getBearerAllocation(int32_t, const sp<IDataConnectionResponse>&) override
  {
      return StatusCode::NOT_SUPPORTED;
  }

  Return<StatusCode>
      getAllBearerAllocations(const sp<IDataConnectionResponse>&) override
  {
      return StatusCode::NOT_SUPPORTED;
  }

  Return<StatusCode> registerForAllBearerAllocationUpdates
  (
      const sp<IDataConnectionIndication>&
  ) override
  {
      return StatusCode::NOT_SUPPORTED;
  }

  Return<void> getConfig
  (
      const hidl_string& key,
      const hidl_string& defaultValue,
      getConfig_cb _hidl_cb
  ) override;

  void setInstanceId(int instanceId);

private:

  int mInstanceId;

  void serviceDied( uint64_t, const ::android::wp<::android::hidl::base::V1_0::IBase> &);
};
