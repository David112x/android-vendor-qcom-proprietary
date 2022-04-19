/******************************************************************************

  Copyright (c) 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

#******************************************************************************/
#ifdef FEATURE_DATA_LQE
#include "OTTModemEndPoint.h"

using std::to_string;

constexpr const char *OTTModemEndPoint::NAME;

void OTTModemEndPoint::requestSetup(string clientToken,
                                      GenericCallback<string>* callback)
{
  auto shared_setupMsg = std::make_shared<QmiSetupRequest>
        (clientToken, 0, nullptr, callback);
  mModule->dispatch(shared_setupMsg);
}
#endif /* FEATURE_DATA_LQE */
