/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/UnSolicitedMessage.h"
#include "framework/add_message_id.h"
#include "framework/Dispatcher.h"

inline void qcril_suppress_unsol_msg(message_id_t id) {
    Dispatcher::getInstance().suppressMessage(id, true);
}

inline void qcril_unsuppress_unsol_msg(message_id_t id) {
    Dispatcher::getInstance().suppressMessage(id, false);
}

