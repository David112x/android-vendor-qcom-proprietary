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

int adev_set_parameters(struct audio_hw_device* dev,
                               const char* kvpairs);

char* adev_get_parameters(const struct audio_hw_device* dev,
                                 const char* keys);

int adev_init_check(const struct audio_hw_device* dev);

int adev_set_voice_volume(struct audio_hw_device* dev, float volume);

int adev_set_master_volume(struct audio_hw_device* dev, float volume);

int adev_get_master_volume(struct audio_hw_device* dev, float* volume);

int adev_set_master_mute(struct audio_hw_device* dev, bool muted);

int adev_get_master_mute(struct audio_hw_device* dev, bool* muted);

int adev_set_mode(struct audio_hw_device* dev, audio_mode_t mode);

int adev_set_mic_mute(struct audio_hw_device* dev, bool state);

int adev_get_mic_mute(const struct audio_hw_device* dev, bool* state);

int adev_dump(const audio_hw_device_t* device, int fd);

int adev_close(hw_device_t* device);

size_t adev_get_input_buffer_size(const struct audio_hw_device* dev,
                                  const struct audio_config* config);

int adev_open_output_stream(struct audio_hw_device* dev,
                            audio_io_handle_t handle, audio_devices_t devices,
                            audio_output_flags_t flags,
                            struct audio_config* config,
                            struct audio_stream_out** stream_out,
                            const char* address __unused);

void adev_close_output_stream(struct audio_hw_device* dev,
                              struct audio_stream_out* stream);

int adev_open_input_stream(struct audio_hw_device* dev,
                           audio_io_handle_t handle, audio_devices_t devices,
                           struct audio_config* config,
                           struct audio_stream_in** stream_in,
                           audio_input_flags_t flags __unused,
                           const char* address __unused,
                           audio_source_t source __unused);

void adev_close_input_stream(struct audio_hw_device* dev,
                             struct audio_stream_in* in);

