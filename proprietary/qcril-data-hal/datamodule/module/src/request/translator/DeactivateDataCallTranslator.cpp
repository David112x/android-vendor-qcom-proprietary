/******************************************************************************
#  Copyright (c) 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#include <memory>
#include <translators/android/legacy_translator.h>
#include <telephony/ril.h>
#include "request/RilRequestDeactivateDataCallMessage.h"
#include "legacy/qcril_event_facade.h"
// ***** IMPORTANT *******
// NOTE: version.h always be included when using
// QCRIL_RIL_VERSION
// ***********************
#include "modules/android/version.h"

class DeactivateDataCallTranslator : public legacy_translator<RIL_REQUEST_DEACTIVATE_DATA_CALL> {
public:
    std::shared_ptr<SolicitedMessage<generic_callback_payload>> translate_request(const ril_request_type &request)
    {
        qcril_request_params_type params {
            request.instance_id,
            QCRIL_DEFAULT_MODEM_ID, /* Always pass in dfault */
            request.req_id_android,
            request.req_id, /* event_id --> req_id */
            const_cast<void *>(request.payload), /* no deepcopy here, just pass it in to the message for that */
            request.payload_len,
            request.t,
            NULL
        };
        return std::make_shared<rildata::RilRequestDeactivateDataCallMessage>(params);

    }

std::shared_ptr<ril_response_type> translate_response
(
  std::shared_ptr<SolicitedMessage<generic_callback_payload>> msg,
  Message::Callback::Status status,
  std::shared_ptr<generic_callback_payload> payload
)
{
    std::shared_ptr<ril_response_type> ret = std::make_shared<ril_response_type>();
    std::shared_ptr<legacy_response_payload> resp_ptr = std::static_pointer_cast<legacy_response_payload>(payload);
    std::shared_ptr<rildata::RilRequestDeactivateDataCallMessage> req_ptr = std::static_pointer_cast<rildata::RilRequestDeactivateDataCallMessage>(msg);

    QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

    if( ret == nullptr )
    {
      QCRIL_LOG_ERROR(" 'ril_response_type' pointer is NULL. Returning NULL data. \n");
      return ret;
    }

    if (status != Message::Callback::Status::SUCCESS)
    {
        ret->e = RIL_E_INTERNAL_ERR;
    }
    else
    {
        ret->e = RIL_E_SUCCESS;
    }
    ret->response = nullptr;
    ret->response_len = 0;

    if (resp_ptr)
    {
        const qcril_request_resp_params_type &params = resp_ptr->get_params();
        if (status == Message::Callback::Status::SUCCESS)
        {
            ret->e = params.ril_err_no;
        }
        ret->instance_id = params.instance_id;
        ret->req_id      = params.request_id_android;
        ret->t           = params.t;
        ret->response_len = params.resp_len;
        ret->response = params.resp_pkt;
    }
    else if (req_ptr)
    {
        const qcril_request_params_type &params = req_ptr->get_params();

        ret->instance_id = params.instance_id;
        ret->req_id      = params.event_id_android;
        ret->t           = params.t;
    }
    return ret;
}

};

static DeactivateDataCallTranslator deactivatedatacall_translator;

QCRIL_ANDROID_DECLARE_REQUEST_WITH_CONV(RIL_REQUEST_DEACTIVATE_DATA_CALL, char **, void, deactivatedatacall_translator);
QCRIL_ANDROID_DEFINE_REQUEST_WITH_CONV(RIL_REQUEST_DEACTIVATE_DATA_CALL);
