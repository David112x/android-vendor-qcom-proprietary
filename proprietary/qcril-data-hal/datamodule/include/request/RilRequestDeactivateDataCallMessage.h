/**
* Copyright (c) 2017,2020 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**/

#pragma once
#include <framework/legacy.h>
#include "framework/SolicitedMessage.h"
#include "framework/GenericCallback.h"
#include "framework/add_message_id.h"
#include <modules/android/RilRequestMessage.h>

namespace rildata {

typedef struct
{
    RIL_Errno ril_err_no;
}DeactivateDataCallResp;

class RilRequestDeactivateDataCallMessage : public SolicitedMessage<generic_callback_payload>,
                                            public add_message_id<RilRequestDeactivateDataCallMessage> {
    private:
    legacy_request_payload params;

    static constexpr int DEACTIVATE_DATA_REQUEST_TIMEOUT_DEFAULT { 90 };
    //Conversion of time from seconds to milliseconds or vice-versa
    static constexpr int SEC_TO_MILLISEC_CONVERTER { 1000 };

    static constexpr int MESSAGE_EXPIRY_TIMEOUT {
         (DEACTIVATE_DATA_REQUEST_TIMEOUT_DEFAULT)
           * SEC_TO_MILLISEC_CONVERTER };

    static inline void deepCopy(qcril_request_params_type &p) {
      int n = p.datalen / sizeof(char *);
      for (int i = 0; i < n; i++)
      {
        char *str = ((char **)p.data)[i];
        if (str != NULL)
        {
          int size = strlen(str) + 1;
          ((char **)p.data)[i] = new char[size];
          strlcpy(((char **)p.data)[i], str, size);
        }
      }
    }

    public:
    static constexpr const char* MESSAGE_NAME = "RIL_REQUEST_DEACTIVATE_DATA_CALL";
    RilRequestDeactivateDataCallMessage() = delete;
    ~RilRequestDeactivateDataCallMessage();

    inline RilRequestDeactivateDataCallMessage( const qcril_request_params_type &p )
             :SolicitedMessage<generic_callback_payload>(get_class_message_id(),
                                                         MESSAGE_EXPIRY_TIMEOUT), params(p) {
        mName = MESSAGE_NAME;
        deepCopy(params.get_params());
   }
   qcril_request_params_type &get_params() {
     return params.get_params();
   }
   string dump();
};

}//namespace
