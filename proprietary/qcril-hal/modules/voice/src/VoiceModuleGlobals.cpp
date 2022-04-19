/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#include "modules/android/ril_message_factory.h"
#include "modules/voice/VoiceModule.h"
#include "ims/ImsVoiceModule.h"

#if 0
static load_module<VoiceModule> voiceModule;
#else
static load_module<ImsVoiceModule> voiceModule;
#endif

VoiceModule *getVoiceModule() {
    return &(voiceModule.get_module());
}
