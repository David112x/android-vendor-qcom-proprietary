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


/************************************************************************************
 *
 *  Filename:      btif_vendor_socket.cc
 *
 *  Description:   Vendor socket Bluetooth Interface
 *
 *
 ***********************************************************************************/

#include <hardware/vendor_socket.h>
#include <stdlib.h>
#include <string.h>

#define LOG_TAG "bt_btif_vendor_socket"

#include <cutils/properties.h>
#include "btif_common.h"
#include "btif_sock_rfc.h"
#include "btif_util.h"

btvendor_socket_callbacks_t *bt_vendor_socket_callbacks = NULL;

/*******************************************************************************
** VENDOR INTERFACE FUNCTIONS
*******************************************************************************/

/*******************************************************************************
**
** Function         init
**
** Description     initializes the vendor socket interface
**
** Returns         bt_status_t
**
*******************************************************************************/
static bt_status_t init( btvendor_socket_callbacks_t* callbacks)
{
    bt_vendor_socket_callbacks = callbacks;
    return BT_STATUS_SUCCESS;
}

static void cleanup(void)
{
    LOG_INFO(LOG_TAG,"cleanup");
    if (bt_vendor_socket_callbacks)
        bt_vendor_socket_callbacks = NULL;
}

static bt_status_t btsock_get_sockopt(btsock_type_t type, int channel,
                                      btsock_option_type_t option_name,
                                      void *option_value, int *option_len)
{
    if((channel <= 0) || (option_value == NULL) || (option_len == NULL))
    {
        BTIF_TRACE_ERROR("invalid parameters, channel:%d, option_value:%p, option_len:%p", channel,
                                                                        option_value, option_len);
        return BT_STATUS_PARM_INVALID;
    }

    bt_status_t status = BT_STATUS_FAIL;
    switch(type)
    {
        case BTSOCK_RFCOMM:
            status = btsock_rfc_get_sockopt(channel, option_name, option_value, option_len);
            break;
        case BTSOCK_L2CAP:
            BTIF_TRACE_ERROR("bt l2cap socket type not supported, type:%d", type);
            status = BT_STATUS_UNSUPPORTED;
            break;
        case BTSOCK_SCO:
            BTIF_TRACE_ERROR("bt sco socket not supported, type:%d", type);
            status = BT_STATUS_UNSUPPORTED;
            break;
        default:
            BTIF_TRACE_ERROR("unknown bt socket type:%d", type);
            status = BT_STATUS_UNSUPPORTED;
            break;
    }
    return status;
}

static bt_status_t btsock_set_sockopt(btsock_type_t type, int channel,
                                      btsock_option_type_t option_name,
                                      void *option_value, int option_len)
{
    if((channel <= 0) || (option_value == NULL))
    {
        BTIF_TRACE_ERROR("invalid parameters, channel:%d, option_value:%p", channel, option_value);
        return BT_STATUS_PARM_INVALID;
    }

    bt_status_t status = BT_STATUS_FAIL;
    switch(type)
    {
        case BTSOCK_RFCOMM:
            status = btsock_rfc_set_sockopt(channel, option_name, option_value, option_len);
            break;
        case BTSOCK_L2CAP:
            BTIF_TRACE_ERROR("bt l2cap socket type not supported, type:%d", type);
            status = BT_STATUS_UNSUPPORTED;
            break;
        case BTSOCK_SCO:
            BTIF_TRACE_ERROR("bt sco socket not supported, type:%d", type);
            status = BT_STATUS_UNSUPPORTED;
            break;
        default:
            BTIF_TRACE_ERROR("unknown bt socket type:%d", type);
            status = BT_STATUS_UNSUPPORTED;
            break;
    }
    return status;
}

static const btvendor_socket_interface_t btvendorSocketInterface = {
    sizeof(btvendorSocketInterface),
    init,
    cleanup,
    btsock_set_sockopt,
    btsock_get_sockopt,
};

/*******************************************************************************
** LOCAL FUNCTIONS
*******************************************************************************/

/*******************************************************************************
**
** Function         btif_vendor_socket_get_interface
**
** Description      Get the vendor socket interface
**
** Returns          btvendor_interface_t
**
*******************************************************************************/
const btvendor_socket_interface_t *btif_vendor_socket_get_interface()
{
    BTIF_TRACE_EVENT("%s", __FUNCTION__);
    return &btvendorSocketInterface;
}
