/*
 * Copyright (c) 2018, The Linux Foundation. All rights reserved.
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

#ifndef ANDROID_INCLUDE_BT_VENDOR_SOCKET_H
#define ANDROID_INCLUDE_BT_VENDOR_SOCKET_H

#include <hardware/bluetooth.h>
#include <hardware/bt_sock.h>

__BEGIN_DECLS

#define BT_PROFILE_VENDOR_SOCKET_ID "vendorSocket"


typedef void (*  btvendor_socket_cleanup_callback)(bool status);

/** BT-Vendor callback structure. */
typedef struct {
    /** set to sizeof(btvendor_socket_callbacks_t) */
    size_t      size;
    btvendor_socket_cleanup_callback  socket_cleanup_cb;
} btvendor_socket_callbacks_t;

typedef enum {
    BTSOCK_OPT_GET_MODEM_BITS = 1,
    BTSOCK_OPT_SET_MODEM_BITS = 2,
    BTSOCK_OPT_CLR_MODEM_BITS = 3,
} btsock_option_type_t;

/** Represents the standard BT-Vendor interface.
 */
typedef struct {

    /** set to sizeof(btvendor_socket_interface_t) */
    size_t  size;

    /**
     * Register the BtVendor callbacks
     */
    bt_status_t (*init)( btvendor_socket_callbacks_t* callbacks );


    /** Closes the interface. */
    void  (*cleanup)( void );

    /*
    set socket option of rfcomm channel socket.
    */
    bt_status_t (*set_sock_opt)(btsock_type_t type, int channel, btsock_option_type_t option_name,
      void *option_value, int option_len);

    /*
    get socket option of rfcomm channel socket.
    */
    bt_status_t (*get_sock_opt)(btsock_type_t type, int channel, btsock_option_type_t option_name,
      void *option_value, int *option_len);
} btvendor_socket_interface_t;

__END_DECLS

#endif /* ANDROID_INCLUDE_BT_VENDOR_SOCKET_H */

