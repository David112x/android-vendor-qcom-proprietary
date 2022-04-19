/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include "modules/nas/NasGetSysInfoMessage.h"

NasGetSysInfoMessage::~NasGetSysInfoMessage() {}

string NasGetSysInfoMessage::dump() { return mName; }