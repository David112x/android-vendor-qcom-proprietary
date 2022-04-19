/******************************************************************************
#  Copyright (c) 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#ifndef MODULES_ANDROID_RIL_API_H
#define MODULES_ANDROID_RIL_API_H

#include <telephony/ril.h>

void android_ril_on_request_complete(qcril_instance_id_e_type instance_id,
        RIL_Token token_id,
        RIL_Errno e,
        std::shared_ptr<void> response,
        size_t response_len);

void android_ril_on_request_complete(qcril_instance_id_e_type instance_id,
        RIL_Token token_id,
        RIL_Errno e,
        void *response,
        size_t response_len);

void android_ril_on_request_ack(qcril_instance_id_e_type instance_id,
        RIL_Token token);

void android_ril_setup_timed_callback( qcril_instance_id_e_type instance_id,
        RIL_TimedCallback callback,
        void *param,
        const struct timeval *relativeTime );

#endif /* MODULES_ANDROID_RIL_API_H */
