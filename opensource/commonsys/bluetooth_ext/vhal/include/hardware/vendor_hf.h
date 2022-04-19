/*
 * Copyright (c) 2019, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
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

#ifndef ANDROID_INCLUDE_BT_HF_VENDOR_H
#define ANDROID_INCLUDE_BT_HF_VENDOR_H

#include <hardware/bluetooth.h>

__BEGIN_DECLS

#define BT_PROFILE_VENDOR_HF_ID "vendorhfservice"

bool get_swb_codec_status();

void btif_handle_vendor_hf_events(uint16_t event, uint16_t swb_config, RawAddress *bd_addr);

/* SWB callback events */
typedef void (* bt_swb_update_callback)(uint16_t swb_codec_config, RawAddress *bd_addr);

typedef void (* bt_twsp_battery_status_callback)(char* at_string, RawAddress *bd_addr);

/** BT-Vendor hf callback structure. */
typedef struct {
    /** set to sizeof(BtVendorhfCallbacks) */
    size_t      size;
    bt_swb_update_callback  swb_codec_cb;
    bt_twsp_battery_status_callback twsp_batt_status_cb;
} btvendor_hf_callbacks_t;


/** Represents the standard BT-Vendor hf interface.
 */
typedef struct {
    /** set to sizeof(BtVendorHfInterface) */
    size_t  size;

    /**
     * Register the BtVendorhf callbacks
     */
    bt_status_t (*init)( btvendor_hf_callbacks_t* callbacks );

    /** Closes the interface. */
    void  (*cleanup)( void );

    int   (*enable_swb) (bool enable);
} btvendor_hf_interface_t;

__END_DECLS
#endif /* ANDROID_INCLUDE_BT_VENDOR_H */

