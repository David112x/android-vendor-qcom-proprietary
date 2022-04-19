/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include "sync/GetAttachListMessage.h"

GetAttachListMessage::~GetAttachListMessage() {
}

string GetAttachListMessage::dump() { return mName; }