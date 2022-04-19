/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a Contribution.
 */

#include <hardware/audio.h>
#pragma once

struct bluetooth_audio_device {
    struct audio_hw_device device;
    audio_stream_out *output;
};

