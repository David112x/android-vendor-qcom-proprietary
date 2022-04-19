/******************************************************************************
#  Copyright (c) 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#  ---------------------------------------------------------------------------
#******************************************************************************/

#include "modules/gstk/CatModemEndPoint.h"
#include "QmiCatSetupRequestSync.h"

#include "CatModemEndPointModule.h"

/*=========================================================================

  Function:  CatModemEndPoint::CatModemEndPoint

===========================================================================*/
CatModemEndPoint::CatModemEndPoint() : ModemEndPoint("Cat")
{
  mModule = new CatModemEndPointModule("CatModemEndPointModule", this);

  if (mModule != nullptr)
  {
    mModule->init();
  }
} /* CatModemEndPoint::CatModemEndPoint */


/*=========================================================================

  Function:  CatModemEndPoint::requestSetupSync

===========================================================================*/
Message::Callback::Status CatModemEndPoint::requestSetupSync
(
  std::shared_ptr<string>& sharedResponse
)
{
  if (getState() != ModemEndPoint::State::OPERATIONAL)
  {
    auto setupMsg = std::make_shared<QmiCatSetupRequestSync>("QMI_CAT-Module");
    return (setupMsg->dispatchSync(sharedResponse));
  }
  else
  {
    sharedResponse = std::make_shared<string>("Service already operational");
    return Message::Callback::Status::SUCCESS;
  }
} /* CatModemEndPoint::requestSetupSync */


/*=========================================================================

  Function:  CatModemEndPoint::cleanUp

===========================================================================*/
void CatModemEndPoint::cleanUp()
{
  if (mModule != NULL)
  {
    ((CatModemEndPointModule*)mModule)->cleanUpQmiSvcClient();    
  }
  setState(ModemEndPoint::State::NON_OPERATIONAL);
} /* CatModemEndPoint::cleanUp */
