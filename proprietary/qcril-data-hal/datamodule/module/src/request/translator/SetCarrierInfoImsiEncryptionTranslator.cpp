/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#include <memory>
//#include <framework/Log.h>
#include <translators/android/legacy_translator.h>
#include <telephony/ril.h>
#include "legacy/qcril_event_facade.h"
#include "request/RilRequestSetCarrierInfoImsiEncryptionMessage.h"
// ***** IMPORTANT *******
// NOTE: version.h always be included when using
// QCRIL_RIL_VERSION
// ***********************
#include "modules/android/version.h"

class SetCarrierInfoImsiEncryptionTranslator : public legacy_translator<RIL_REQUEST_SET_CARRIER_INFO_IMSI_ENCRYPTION> {
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
        return std::make_shared<rildata::RilRequestSetCarrierInfoImsiEncryptionMessage>(params);

    }
};

static SetCarrierInfoImsiEncryptionTranslator setcarrierinfoimsiencryption_translator;

QCRIL_ANDROID_DECLARE_REQUEST_WITH_CONV(RIL_REQUEST_SET_CARRIER_INFO_IMSI_ENCRYPTION, RIL_CarrierInfoForImsiEncryption *, void, setcarrierinfoimsiencryption_translator);
QCRIL_ANDROID_DEFINE_REQUEST_WITH_CONV(RIL_REQUEST_SET_CARRIER_INFO_IMSI_ENCRYPTION);

