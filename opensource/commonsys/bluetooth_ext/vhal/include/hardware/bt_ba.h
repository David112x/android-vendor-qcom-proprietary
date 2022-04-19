/*
 * Copyright (c) 2017, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of The Linux Foundation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef ANDROID_INCLUDE_BT_BA_H
#define ANDROID_INCLUDE_BT_BA_H

#include <vector>

#include <hardware/bluetooth.h>

__BEGIN_DECLS

/* BA possible states */
typedef enum {
    BA_STATE_IDLE = 0,// BA is not enabled.
    BA_STATE_PENDING,// Transition between enable/disable
    BA_STATE_PAUSED,// enabled but not streaming
    BA_STATE_STREAMING,// enabled and streaming
    BA_STATE_AUDIO_PENDING// enabled, but transition between paused and streaming
} ba_state_t;

#define BT_PROFILE_BAT_ID "ba_transmitter"

/** Callback for updating apps for A2dp multicast state.
 */
typedef void (* ba_state_update_callback)(ba_state_t state);
typedef void (* ba_enc_key_update_callback) (uint8_t size, uint8_t *p_enc_key);
typedef void (* ba_div_update_callback) (uint8_t size, uint8_t *p_div);
typedef void (* ba_stream_id_update_callback) (uint8_t stream_id);

/** BA callback structure.  */
typedef struct {
    /** set to sizeof(ba_transmitter_callbacks_t) */
    size_t      size;
    ba_state_update_callback  state_cb;
    ba_enc_key_update_callback enc_update_key_cb;
    ba_div_update_callback div_update_cb;
    ba_stream_id_update_callback  stream_id_update_cb;
} ba_transmitter_callbacks_t;

/** Represents the standard BA interface.
 */
typedef struct {

    /** set to sizeof(ba_transmitter_interface_t) */
    size_t          size;
    /**
     * Register the BA Transmitter callbacks.
     */
    bt_status_t (*init)(ba_transmitter_callbacks_t* callbacks);

    /** set BA state */
    bt_status_t (*set_state)( uint8_t state );

    /** refresh enc key **/
    bt_status_t (*refresh_enc_key)();

    /** set vol level **/
    bt_status_t (*set_vol)(uint8_t vol, uint8_t max_vol);

    /** Closes the interface. */
    void  (*cleanup)( void );

} ba_transmitter_interface_t;

__END_DECLS

#endif /* ANDROID_INCLUDE_BT_AV_H */
