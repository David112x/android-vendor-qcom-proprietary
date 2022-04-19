/*
 * Copyright (c) 2017, The Linux Foundation. All rights reserved.
 * "Not a contribution"
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the
 * disclaimer below) provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.

    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
 * GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
 * HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * Copyright (C) 2016 The Android Open Source Project
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

//
// Interface to the A2DP aptX-TWS Encoder
//

#ifndef A2DP_VENDOR_APTX_TWS_ENCODER_H
#define A2DP_VENDOR_APTX_TWS_ENCODER_H

#include "a2dp_codec_api.h"
#include "osi/include/time.h"

// Loads the A2DP aptX-HD encoder.
// Return true on success, otherwise false.
bool A2DP_VendorLoadEncoderAptxTWS(void);

// Unloads the A2DP aptX-HD encoder.
void A2DP_VendorUnloadEncoderAptxTWS(void);

// Initialize the A2DP aptX-TWS encoder.
// |p_peer_params| contains the A2DP peer information.
// The current A2DP codec config is in |a2dp_codec_config|.
// |read_callback| is the callback for reading the input audio data.
// |enqueue_callback| is the callback for enqueueing the encoded audio data.
void a2dp_vendor_aptx_tws_encoder_init(
    const tA2DP_ENCODER_INIT_PEER_PARAMS* p_peer_params,
    A2dpCodecConfig* a2dp_codec_config,
    a2dp_source_read_callback_t read_callback,
    a2dp_source_enqueue_callback_t enqueue_callback);

// Cleanup the A2DP aptX-HD encoder.
void a2dp_vendor_aptx_tws_encoder_cleanup(void);

// Reset the feeding for the A2DP aptX-HD encoder.
void a2dp_vendor_aptx_tws_feeding_reset(void);

// Flush the feeding for the A2DP aptX-HD encoder.
void a2dp_vendor_aptx_tws_feeding_flush(void);

// Get the A2DP aptX-HD encoder interval (in milliseconds).
period_ms_t a2dp_vendor_aptx_tws_get_encoder_interval_ms(void);

// Prepare and send A2DP aptX-HD encoded frames.
// |timestamp_us| is the current timestamp (in microseconds).
void a2dp_vendor_aptx_tws_send_frames(uint64_t timestamp_us);

#endif  // A2DP_VENDOR_APTX_TWS_ENCODER_H
