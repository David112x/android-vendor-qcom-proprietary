/******************************************************************************
#  Copyright (c) 2017, 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#  ---------------------------------------------------------------------------
#******************************************************************************/

#include "framework/ModuleLooper.h"
#include "framework/QcrilInitMessage.h"
#include "modules/qmi/ModemEndPointFactory.h"
#include "modules/uim_service/UimGetRilPropertySyncMsg.h"
#include "GstkTimerRequestMsg.h"
#include "GstkModule.h"
#include "qcril_gstk.h"
#include "qcril_scws.h"
#include <string>

#define TAG "GstkModule"
#define MAX_QMI_CAT_ASYNC_MSG 10

/* Android property to disable for certain test scenarios */
#define QCRIL_GSTK_PROPERTY_DISABLED               "persist.vendor.radio.qcril_gstk.disable"
#define QCRIL_GSTK_PROP_DISABLED_VALUE             "1"

static load_module<GstkModule> gstk_module;


/*=========================================================================

  Function:  GstkModule::GstkModule

===========================================================================*/
GstkModule::GstkModule():gstk_req_list("GstkModule")
{
  mName = "GstkModule";
  mReady = false;

  mLooper = std::unique_ptr<ModuleLooper>(new ModuleLooper);

  QCRIL_LOG_INFO( "%s - Constructor\n", __FUNCTION__);

  using std::placeholders::_1;
  mMessageHandler = {
       HANDLER(QcrilInitMessage, GstkModule::GstkProcessRilInit),
       HANDLER(GstkSendEnvelopeRequestMsg, GstkModule::GstkHandleEnvelopeRequest),
       HANDLER(GstkSendTerminalResponseRequestMsg, GstkModule::GstkHandleTerminalResponseRequest),
       HANDLER(GstkSendSetupCallResponseRequestMsg, GstkModule::GstkHandleSetupCallResponseRequest),
       HANDLER(GstkReportSTKIsRunningMsg, GstkModule::GstkProcessStkRunningMsg),
       HANDLER(GstkQmiCatResponseMsg, GstkModule::GstkProcessQmiCatCallback),
       HANDLER(GstkQmiCatIndicationMsg, GstkModule::GstkProcessQmiCatInd),
       HANDLER(GstkTimerRequestMsg, GstkModule::GstkHandleTimerRequest),
       HANDLER(GstkTimerExpiryMsg, GstkModule::GstkHandleTimerExpiry),
       HANDLER(UimCardStatusIndMsg, GstkModule::GstkProcessCardStatusMsg),
                    };
} /* GstkModule::GstkModule */


/*=========================================================================

  Function:  GstkModule::~GstkModule

===========================================================================*/
GstkModule::~GstkModule()
{
  QCRIL_LOG_INFO( "%s - Destructor \n", __FUNCTION__);
  mCatModemEndPoint = NULL;
  mReady = false;
} /* GstkModule::~GstkModule */


/*=========================================================================

  Function:  GstkModule::init

===========================================================================*/
void GstkModule::init()
{
  QCRIL_LOG_INFO( "init\n");

  Module::init();

  /* Reset qcril_gstk */
  qcril_gstk_reset_state(FALSE);

  mCatModemEndPoint = ModemEndPointFactory<CatModemEndPoint>::getInstance().buildEndPoint();
} /* GstkModule::init() */


/*=========================================================================

  Function:  GstkModule::GstkHandleEnvelopeRequest

===========================================================================*/
void GstkModule::GstkHandleEnvelopeRequest(std::shared_ptr<GstkSendEnvelopeRequestMsg> req_ptr)
{
  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if (req_ptr == nullptr)
  {
    return;
  }

  if (mReady == false)
  {
    auto rsp_ptr = std::make_shared<RIL_GSTK_EnvelopeResponse>();

    if (rsp_ptr != NULL)
    {
      rsp_ptr->err = RIL_GSTK_E_RADIO_NOT_AVAILABLE;
    }

    req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
    return;
  }

  qcril_gstk_qmi_request_stk_send_envelope_command(req_ptr);
} /* GstkModule::GstkHandleEnvelopeRequest */


/*=========================================================================

  Function:  GstkModule::GstkHandleTerminalResponseRequest

===========================================================================*/
void GstkModule::GstkHandleTerminalResponseRequest(std::shared_ptr<GstkSendTerminalResponseRequestMsg> req_ptr)
{
  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if (req_ptr == nullptr)
  {
    return;
  }

  if (mReady == false)
  {
    auto rsp_ptr = std::make_shared<RIL_GSTK_Errno>(RIL_GSTK_E_RADIO_NOT_AVAILABLE);

    req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
    return;
  }

  qcril_gstk_qmi_request_stk_send_terminal_response(req_ptr);
} /* GstkModule::GstkHandleTerminalResponseRequest */


/*=========================================================================

  Function:  GstkModule::GstkHandleSetupCallResponseRequest

===========================================================================*/
void GstkModule::GstkHandleSetupCallResponseRequest(std::shared_ptr<GstkSendSetupCallResponseRequestMsg> req_ptr)
{
  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if (req_ptr == nullptr)
  {
    return;
  }

  if (mReady == false)
  {
    auto rsp_ptr = std::make_shared<RIL_GSTK_Errno>(RIL_GSTK_E_RADIO_NOT_AVAILABLE);

    req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
    return;
  }

  qcril_gstk_qmi_request_stk_handle_call_setup_requested_from_sim(req_ptr);
} /* GstkModule::GstkHandleSetupCallResponseRequest */


/*=========================================================================

  Function:  GstkModule::GstkProcessStkRunningMsg

===========================================================================*/
void GstkModule::GstkProcessStkRunningMsg
(
  std::shared_ptr<GstkReportSTKIsRunningMsg> msg_ptr
)
{
  QCRIL_LOG_INFO( "%s \n", __FUNCTION__);

  qcril_gstk_qmi_request_stk_service_is_running(msg_ptr, mReady);
} /* GstkModule::GstkProcessStkRunningMsg */


/*=========================================================================

  Function:  GstkModule::GstkProcessCardStatusMsg

===========================================================================*/
void GstkModule::GstkProcessCardStatusMsg
(
  std::shared_ptr<UimCardStatusIndMsg> msg_ptr
)
{
  qcril_uim_card_info_type  status = {};
  QCRIL_LOG_INFO( "%s \n", __FUNCTION__);

  if (msg_ptr == nullptr)
  {
    return;
  }

  status = msg_ptr->get_card_status();

  QCRIL_LOG_DEBUG( "Card_status on slot : %d\n", status.slot);
  if (status.slot == qmi_ril_get_process_instance_id())
  {
    switch(status.status)
    {
      case QCRIL_CARD_STATUS_UNKNOWN:
      case QCRIL_CARD_STATUS_NOT_APPLICABLE:
      case QCRIL_CARD_STATUS_DOWN:
      case QCRIL_CARD_STATUS_NOT_ACCESSIBLE:
      case QCRIL_CARD_STATUS_ABSENT:
        qcril_gstk_reset_state(FALSE);
        qcril_scws_card_error(static_cast<qcril_scws_slot_enum_type>(status.slot));
        break;

      case QCRIL_CARD_STATUS_REFRESH:
        qcril_gstk_reset_state(TRUE);
        qcril_scws_card_error(static_cast<qcril_scws_slot_enum_type>(status.slot));
        break;

      default:
        break;
    }
  }
} /* GstkModule::GstkProcessCardStatusMsg */


/*=========================================================================

  Function:  GstkModule::GstkProcessRilInit

===========================================================================*/
void GstkModule::GstkProcessRilInit
(
  std::shared_ptr<Message> msg_ptr
)
{
  auto prop_ptr = std::make_shared<UimGetRilPropertySyncMsg>
                             (QCRIL_GSTK_PROPERTY_DISABLED);
  std::shared_ptr<std::string> sharedResponse = NULL;
  std::shared_ptr<std::string> respPtr        = NULL;

  (void) msg_ptr;

  /* If QCRIL_GSTK needs to be disabled, just return at this point */
  if (prop_ptr != NULL &&
      (prop_ptr->dispatchSync(respPtr) == Message::Callback::Status::SUCCESS)&&
      respPtr != NULL &&
      strcmp(respPtr->c_str(), QCRIL_GSTK_PROP_DISABLED_VALUE) == 0)
  {
    QCRIL_LOG_INFO("QCRIL GSTK is being disabled");
    return;
  }

  if (mCatModemEndPoint != NULL)
  {
    mCatModemEndPoint->requestSetupSync(sharedResponse);
  }
} /* GstkModule::GstkProcessRilInit */


/*=========================================================================

  Function:  GstkModule::GstkHandleTimerRequest

===========================================================================*/
void GstkModule::GstkHandleTimerRequest
(
  std::shared_ptr<Message> msg_ptr
)
{
  /* Dummy function, and don't send response, Dispatcher will call the callback
     on timer expiry */
  (void) msg_ptr;
} /* GstkModule::GstkHandleTimerRequest */


/*=========================================================================

  Function:  GstkModule::GstkHandleTimerExpiry

===========================================================================*/
void GstkModule::GstkHandleTimerExpiry
(
  std::shared_ptr<GstkTimerExpiryMsg> msg_ptr
)
{
  GstkTimerExpiryCallback  cb = NULL;

  if (msg_ptr == NULL)
  {
    return;
  }

  cb = msg_ptr->get_cb_func();

  if (cb != NULL)
  {
    cb(msg_ptr->get_user_data());
  }
} /* GstkModule::GstkHandleTimerExpiry */


/*=========================================================================

  Function:  GstkModule::GstkProcessQmiCatCallback

===========================================================================*/
void GstkModule::GstkProcessQmiCatCallback
(
  std::shared_ptr<GstkQmiCatResponseMsg> msg_ptr
)
{
  if (mQmiAsyncMsgCount > 0)
  {
    mQmiAsyncMsgCount--;
  }

  /* Received one response, check the queue for pending messages */
  while (!mQmiPendingList.empty())
  {
    std::shared_ptr<int>                  respPtr    = nullptr;
    std::shared_ptr<GstkQmiCatRequestMsg> pendingMsg = mQmiPendingList.front();

    mQmiPendingList.pop();

    if (pendingMsg != nullptr)
    {
      Message::Callback::Status status = pendingMsg->dispatchSync(respPtr);

      QCRIL_LOG_INFO("Processing the message in the QMI Queue :%s", pendingMsg->dump().c_str());

      if (status == Message::Callback::Status::SUCCESS &&
          respPtr != nullptr && *respPtr == 0)
      {
        /* Queuing async request to modem success */
        mQmiAsyncMsgCount++;
        break;
      }
      else
      {
        /* QMI UIM request failed, send error response back */
        std::shared_ptr<qmi_cat_rsp_data_type>  rsp_data = std::make_shared<qmi_cat_rsp_data_type>();
        std::shared_ptr<GstkQmiCatResponseMsg>  resp_ptr = nullptr;

        if (rsp_data != NULL)
        {
          rsp_data->transp_err = QMI_CAT_SERVICE_ERR;
          rsp_data->qmi_err = QMI_CAT_SERVICE_ERR_INTERNAL;
          rsp_data->rsp_id = qcril_gstk_convert_req_type_to_rsp_id(pendingMsg->get_msg_id());
        }

        resp_ptr = std::make_shared<GstkQmiCatResponseMsg>(rsp_data, pendingMsg->get_user_data());
        if (resp_ptr != nullptr)
        {
          resp_ptr->dispatch();
        }
      }
    }
  }

  if (msg_ptr != NULL)
  {
    std::shared_ptr<Message>  req_ptr      = NULL;
    qcril_gstk_user_data      user_data    = {};
    qmi_cat_rsp_data_type   * rsp_data_ptr = msg_ptr->get_message();

    user_data.user_data = msg_ptr->get_user_data();

    req_ptr = gstk_req_list.extract(user_data.token);

    if (req_ptr == NULL)
    {
      return;
    }

    qcril_gstk_process_qmi_cat_cb(rsp_data_ptr, req_ptr);
  }
} /* GstkModule::GstkProcessQmiCatCallback */


/*=========================================================================

  Function:  GstkModule::GstkProcessQmiCatInd

===========================================================================*/
void GstkModule::GstkProcessQmiCatInd
(
  std::shared_ptr<GstkQmiCatIndicationMsg> msg_ptr
)
{
  if (msg_ptr != nullptr)
  {
    QCRIL_LOG_DEBUG(" GstkProcessQmiCatInd : 0x%x", msg_ptr->get_ind_id());
    switch(msg_ptr->get_ind_id())
    {
      case QMI_CAT_SRVC_UP_IND_MSG:
        if (qcril_gstk_init_state() != 0)
        {
          QCRIL_LOG_INFO("Init failed");
          break;
        }
        mReady = true;
        break;
      case QMI_CAT_SRVC_DOWN_IND_MSG:
        mReady = false;
        qcril_gstk_reset_state(FALSE);
        break;
      default:
        qcril_gstk_process_qmi_cat_ind(msg_ptr->get_ind_id(), msg_ptr->get_message());
        break;
    }
  }
} /* GstkModule::GstkProcessQmiCatInd */


/*=========================================================================

  FUNCTION:  GstkModule::GstkSendQmiCatRequest

===========================================================================*/
int GstkModule::GstkSendQmiCatRequest
(
  qcril_gstk_request_type                            msg_id,
  std::shared_ptr<qcril_gstk_request_data_type>      params_ptr,
  std::shared_ptr<Message>                           req_ptr
)
{
  std::shared_ptr<GstkQmiCatRequestMsg>     req_msg_ptr    = nullptr;
  qcril_gstk_user_data                      user_data      = {};
  std::pair<uint16_t, bool>                 token          = {0, false};
  int                                       ret            = 0;

  token = gstk_req_list.insert(req_ptr);

  if (!token.second)
  {
    return -1;
  }

  user_data.token = token.first;

  req_msg_ptr = std::make_shared<GstkQmiCatRequestMsg>(msg_id,
                                                       params_ptr,
                                                       nullptr,
                                                       user_data.user_data);

  if (req_msg_ptr == nullptr)
  {
    return -1;
  }

  /* QMI async message count less than MAX_QMI_CAT_ASYNC_MSG */
  if (mQmiAsyncMsgCount < MAX_QMI_CAT_ASYNC_MSG)
  {
    std::shared_ptr<int>                 respPtr        = nullptr;

    if (req_msg_ptr->dispatchSync(respPtr) == Message::Callback::Status::SUCCESS)
    {
      if (respPtr == nullptr)
      {
        ret = -1;
      }
      else
      {
        ret = *respPtr;
      }

      /* Request is qmi async increase QMI msg count */
      if (ret == 0)
      {
        mQmiAsyncMsgCount++;
      }
    }
    else
    {
      /*In case of dispatchSync failure from other Modules due to SSR etc,
      return -1 to sendResponse back*/

      ret = -1;
    }
  }
  else
  {
    /* Async message and waiting for 10 QMI async responses */
    QCRIL_LOG_INFO("Waiting for response from QMI UIM putting message in the QMI Queue");
    mQmiPendingList.push(req_msg_ptr);
  }

  return ret;
} /* GstkModule::GstkSendQmiCatRequest */


/*=========================================================================

  Function:  GstkModule::qcrilHalGstkModuleCleanup

===========================================================================*/
void GstkModule::qcrilHalGstkModuleCleanup()
{
  if (mCatModemEndPoint != NULL)
  {
    mCatModemEndPoint->cleanUp();
  }
  mReady = false;
} /* GstkModule::qcrilHalGstkModuleCleanup */


/*=========================================================================

  Function:  getGstkModule

===========================================================================*/
GstkModule *getGstkModule()
{
  return &(gstk_module.get_module());
} /* getGstkModule */


#ifdef QMI_RIL_UTF
void qcril_qmi_hal_gstk_module_cleanup()
{
    gstk_module.get_module().qcrilHalGstkModuleCleanup();
}
#endif
