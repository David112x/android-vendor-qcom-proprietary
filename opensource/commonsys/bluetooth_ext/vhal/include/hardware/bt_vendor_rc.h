/*
 * Copyright (c) 2018, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *           * Redistributions of source code must retain the above copyright
 *             notice, this list of conditions and the following disclaimer.
 *           * Redistributions in binary form must reproduce the above
 *           * copyright notice, this list of conditions and the following
 *             disclaimer in the documentation and/or other materials provided
 *             with the distribution.
 *           * Neither the name of The Linux Foundation nor the names of its
 *             contributors may be used to endorse or promote products derived
 *             from this software without specific prior written permission.
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

#ifndef ANDROID_INCLUDE_BT_VENDOR_RC_H
#define ANDROID_INCLUDE_BT_VENDOR_RC_H

#include <hardware/bluetooth.h>

__BEGIN_DECLS

#define BT_PROFILE_AV_RC_VENDOR_CTRL_ID "vendor_avrcp_ctrl"

typedef enum {
 BTRC_FEAT_COVER_ART = 0x08,    /* cover art support */
} btrc_vendor_remote_features_t;

typedef void (* btrc_vendor_ctrl_getrcfeatures_callback) (RawAddress *bd_addr, int features,
                                                       uint16_t cover_art_psm);

typedef void (* btrc_vendor_ctrl_get_media_element_attr_rsp_callback)(RawAddress *bd_addr,
                                                                  uint8_t num_attr,
                                                                  btrc_element_attr_val_t *p_attrs);

/** BT-RC Controller callback structure. */
typedef struct {
    /** set to sizeof(BtRcCallbacks) */
    size_t      size;
    btrc_vendor_ctrl_getrcfeatures_callback               getrcfeatures_cb;
    btrc_vendor_ctrl_get_media_element_attr_rsp_callback   media_element_attr_rsp_cb;
} btrc_vendor_ctrl_callbacks_t;

/** Represents the standard BT-RC AVRCP Controller interface. */

typedef struct {
    /** set to sizeof(BtRcInterface) */
    size_t          size;
    /**
     * Register the BtRc callbacks
     */
    bt_status_t (*init_vendor)( btrc_vendor_ctrl_callbacks_t* callbacks);

    /** Get Media Element interface. */
    bt_status_t (*get_media_element_attributes_vendor) (RawAddress *bd_addr, uint8_t num_attrib,
                    uint32_t *p_attr_ids);

    /** Closes the interface. */
    void  (*cleanup_vendor)( void );

    } btrc_vendor_ctrl_interface_t;

__END_DECLS

#endif /* ANDROID_INCLUDE_BT_VENDOR_RC_H */
