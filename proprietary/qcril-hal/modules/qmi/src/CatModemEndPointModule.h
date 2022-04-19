/******************************************************************************
#  Copyright (c) 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#include <map>

#include "qmi_client.h"
#include "card_application_toolkit_v02.h"

#include "modules/qmi/ModemEndPoint.h"
#include "modules/gstk/GstkQmiCatRequestMsg.h"
#include "QmiCatSetupRequestSync.h"

class CatModemEndPointModule : public Module
{
  public:
    CatModemEndPointModule(string name, ModemEndPoint* owner);
    ~CatModemEndPointModule();

    void onQmiUnsolIndCb(qmi_cat_indication_id_type   ind_id,
                         uint32_t                     ind_len,
                         uint8_t                    * ind_data_ptr);

    void init();

    void cleanUpQmiSvcClient();

  private:
    static std::map<uint32_t, void *>                    mEnvList;
    static std::map<uint32_t, void *>                    mTrMsgPtr;
    qmi_idl_service_object_type                          mServiceObject;
    qmi_client_type                                      mQmiSvcClient;
    ModemEndPoint                                      * mOwner;
    qmi_client_os_params                                 mOsParams;
    qmi_client_os_params                                 mNotifyParams;
    qmi_client_type                                      mNotifyHandle;

    void handleQmiClientSetup(std::shared_ptr<QmiCatSetupRequestSync> msg);

    void handleQmiServiceUp(std::shared_ptr<Message> msg);

    void handleQmiServiceDown(std::shared_ptr<Message> msg);

    void handleQcrilGstkRequest(std::shared_ptr<GstkQmiCatRequestMsg> msg_ptr);

    static void onQmiAsyncCb(qmi_client_type                user_handle,
                             unsigned int                   msg_id,
                             void                         * resp_c_struct,
                             unsigned int                   resp_c_struct_len,
                             void                         * resp_cb_data,
                             qmi_client_error_type          transp_err);

    static void onQmiNotifyCb(qmi_client_type                user_handle,
                              qmi_idl_service_object_type    service_obj,
                              qmi_client_notify_event_type   service_event,
                              void                         * notify_cb_data);
};

