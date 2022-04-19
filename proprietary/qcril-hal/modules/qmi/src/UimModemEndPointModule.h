/******************************************************************************
#  Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#include "qmi_client.h"
#include "modules/qmi/ModemEndPoint.h"
#include "modules/uim/UimQmiUimRequestMsg.h"
#include "QmiUimSetupRequestSync.h"

/*=========================================================================

  CLASS:  UimModemEndPointModule

===========================================================================*/
class UimModemEndPointModule : public Module {
 public:
  UimModemEndPointModule(string name, ModemEndPoint* owner);
  ~UimModemEndPointModule(); 

  void init();
  void cleanUpQmiSvcClient();

  void onQmiUnsolIndCb(void                         * user_data,
                       qmi_uim_indication_id_type     ind_id,
                       qmi_uim_indication_data_type * ind_data_ptr);

 private:
  qmi_idl_service_object_type               mServiceObject;
  qmi_client_type                           mQmiSvcClient;
  ModemEndPoint                           * mOwner;
  qmi_client_os_params                      mOsParams;
  qmi_client_type                           mQmiNotifyHandle;
  qmi_client_os_params                      mNotifierOsParams;

  void handleQmiClientSetup(std::shared_ptr<QmiUimSetupRequestSync> msg);

  void handleQmiServiceUp(std::shared_ptr<Message> msg);

  void handleQmiServiceDown(std::shared_ptr<Message> msg);

  void handleQcrilUimRequest(std::shared_ptr<UimQmiUimRequestMsg> msg_ptr);

  int processQcrilUimRequest(qmi_client_type                mQmiSvcClient,
                             uint32_t                       request_type,
                             const void                   * param_data_ptr,
                             qmi_uim_user_async_cb_type     callback_function_ptr,
                             void                         * user_data,
                             qmi_uim_rsp_data_type        * rsp_data_ptr);

  static void onQmiNotifyCb(qmi_client_type                 user_handle,
                            qmi_idl_service_object_type     service_obj,
                            qmi_client_notify_event_type    service_event,
                            void                          * notify_cb_data);

  static void UimQmiAsyncCb (qmi_uim_rsp_data_type        * rsp_data,
                             void                         * user_data);
};
