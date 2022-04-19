/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a Contribution.
 */

 /*
 * Copyright 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "BTAudioHw_QTI"

#include <android-base/logging.h>
#include <errno.h>
#include <hardware/audio.h>
#include <hardware/hardware.h>
#include <log/log.h>
#include <malloc.h>
#include <string.h>
#include <system/audio.h>

#include "audio_bluetooth_hw.h"
#include "stream_apis.h"
#include "utils.h"

int adev_set_parameters(struct audio_hw_device* dev,
                               const char* kvpairs) {
  LOG(VERBOSE) << __func__ << ": kevpairs=[" << kvpairs << "]";
  struct bluetooth_audio_device* bt_dev = (struct bluetooth_audio_device*)dev;
  struct BluetoothStreamOut* out = reinterpret_cast<BluetoothStreamOut*>(bt_dev->output); ;
  int retval = 0;
  if (out == NULL) return retval;
  return out->stream_out_.common.set_parameters(&bt_dev->output->common, kvpairs);
}

char* adev_get_parameters(const struct audio_hw_device* dev __unused,
                                 const char* keys) {
  LOG(VERBOSE) << __func__ << ": keys=[" << keys << "]";
  return strdup("");
}

int adev_init_check(const struct audio_hw_device* dev __unused) { return 0; }

int adev_set_voice_volume(struct audio_hw_device* dev __unused, float volume) {
  LOG(VERBOSE) << __func__ << ": volume=" << volume;
  return -ENOSYS;
}

int adev_set_master_volume(struct audio_hw_device* dev __unused, float volume) {
  LOG(VERBOSE) << __func__ << ": volume=" << volume;
  return -ENOSYS;
}

int adev_get_master_volume(struct audio_hw_device* dev __unused, float* volume __unused) {
  return -ENOSYS;
}

int adev_set_master_mute(struct audio_hw_device* dev __unused, bool muted) {
  LOG(VERBOSE) << __func__ << ": mute=" << muted;
  return -ENOSYS;
}

int adev_get_master_mute(struct audio_hw_device* dev __unused, bool* muted __unused) {
  return -ENOSYS;
}

int adev_set_mode(struct audio_hw_device* dev __unused, audio_mode_t mode) {
  LOG(VERBOSE) << __func__ << ": mode=" << mode;
  return 0;
}

int adev_set_mic_mute(struct audio_hw_device* dev __unused, bool state) {
  LOG(VERBOSE) << __func__ << ": state=" << state;
  return -ENOSYS;
}

int adev_get_mic_mute(const struct audio_hw_device* dev __unused, bool* state __unused) {
  return -ENOSYS;
}

size_t adev_get_input_buffer_size(const struct audio_hw_device* dev,
                                  const struct audio_config* config) {
  return stream_input_buffer_size(dev, config);
}

int adev_open_output_stream(struct audio_hw_device* dev,
                            audio_io_handle_t handle, audio_devices_t devices,
                            audio_output_flags_t flags,
                            struct audio_config* config,
                            struct audio_stream_out** stream_out,
                            const char* address __unused) {
  return open_output_stream(dev, handle, devices, flags, config, stream_out, address);
}

void adev_close_output_stream(struct audio_hw_device* dev,
                              struct audio_stream_out* stream){
  close_output_stream(dev, stream);
}

int adev_open_input_stream(struct audio_hw_device* dev,
                           audio_io_handle_t handle, audio_devices_t devices,
                           struct audio_config* config,
                           struct audio_stream_in** stream_in,
                           audio_input_flags_t flags __unused,
                           const char* address __unused,
                           audio_source_t source __unused){
  return open_input_stream(dev, handle, devices, config, stream_in, flags, address, source);
}

void adev_close_input_stream(struct audio_hw_device* dev,
                             struct audio_stream_in* in){
  close_input_stream(dev, in);
}

int adev_dump(const audio_hw_device_t* device __unused, int fd __unused) { return 0; }

int adev_close(hw_device_t* device) {
  free(device);
  return 0;
}
