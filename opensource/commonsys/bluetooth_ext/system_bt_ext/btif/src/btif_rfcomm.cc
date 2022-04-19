/*
 *Copyright (c) 2014, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the followin conditions are met:
 *        * Redistributions of source code must retain the above copyright
 *          notice, this list of conditions and the followin disclaimer.
 *        * Redistributions in binary form must reproduce the above copyriht
 *            notice, this list of conditions and the followin disclaimer in the
 *            documentation and/or other materials provided with the distribution.
 *        * Neither the name of The Linux Foundation nor
 *            the names of its contributors may be used to endorse or promote
 *            products derived from this software without specific prior written
 *            permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT ARE DISCLAIMED.    IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <hardware/bluetooth.h>
#include "port_api.h"
#include "btm_api.h"
#include "btu.h"
#include "btm_api.h"
#include "bt_testapp.h"
#define LOG_NDDEBUG 0
#define LOG_TAG "bluedroid"
#include "btif_api.h"
#include "bt_utils.h"

/************************************************************************************
**  Constants & Macros
************************************************************************************/
#define RFC_BUFFER_SIZE  20000

/************************************************************************************
**  Externs
************************************************************************************/
static void bt_rfc_mmt_cback (uint32_t code, uint16_t handle); //rfc
static void bt_rfc_mmt_cback_msc_data (uint32_t code, uint16_t handle);
static void bt_rfc_mmt_server_cback (uint32_t code, uint16_t handle);//rfc
static int bt_rfc_data_cback (uint16_t port_handle, void *data, uint16_t len);//rfc
static void bt_rfc_port_cback (uint32_t code, uint16_t handle);//rfc
void rdut_rfcomm (uint8_t server);
void rdut_rfcomm_test_interface (tRFC *input);
static uint16_t rfc_handle = 0;
char buffer[RFC_BUFFER_SIZE];

/************************************************************************************
**  Functions
************************************************************************************/
static const btrfcomm_interface_t btrfcInterface = {
    sizeof(btrfcomm_interface_t),
    NULL,
    rdut_rfcomm,
    rdut_rfcomm_test_interface,
    NULL,
    NULL,
};

const btrfcomm_interface_t *btif_rfcomm_get_interface(void)
{
    BTIF_TRACE_EVENT("%s", __FUNCTION__);
    ALOGI("\n%s\n", __FUNCTION__);
    return &btrfcInterface;
}

static void bt_rfc_mmt_cback (uint32_t code, uint16_t handle)
{
    uint16_t   length = 0;
    int      count  = 100;//Number of frames to be send

    ALOGI("dut_rfc_mmt_cback %d, %x", code, handle);
    memset(buffer , 0x01 ,10000); //RFC data
    if (code == PORT_SUCCESS)
    {
        PORT_WriteData (handle, buffer, 10000, &length);
        --count;
        ALOGI("rfc mmt length: %d", length);
    }
}

static void bt_rfc_mmt_cback_msc_data (uint32_t code, uint16_t handle)
{
    ALOGI("dut_rfc_mmt_cback_msc_data %d, %x", code, handle);
}

static void bt_rfc_send_data ( uint16_t handle)
{
    uint16_t   length = 0;

    ALOGI("bt_rfc_send_data %d", handle);
    memset(buffer , 0x01 ,10000); //RFC data
    PORT_WriteData (handle, buffer, 10000, &length);
}

static void bt_rfc_mmt_server_cback (uint32_t code, uint16_t handle)
{
    ALOGI("dut_rfc_mmt_Server_cback %d, %x", code, handle);
}


static int bt_rfc_data_cback (uint16_t port_handle, void *data, uint16_t len)
{
    ALOGI("dut_rfc_data_cback"); // Called from PORT_DataInd
    return 0;
}

static void bt_rfc_port_cback (uint32_t code, uint16_t handle)
{
    ALOGI("bt_rfc_port_cback");
}

void rdut_rfcomm (uint8_t server)
{
    uint16_t   handle;
    int status = -1;
    RawAddress remote_bd;
    RawAddress::FromString("00:15:83:0A:0E:1F", remote_bd);
    RawAddress any_add = RawAddress::kAny;

    ALOGI("dut_rfc_mode:%d",server);
    if (server == 0)
    {
        BTM_SetSecurityLevel (TRUE, "", 0, 0, 0x03, 3/*BTM_SEC_PROTO_RFCOMM*/, 20);
        status = RFCOMM_CreateConnection(0x0020, 20, FALSE, 256, remote_bd,
                                            &handle, bt_rfc_mmt_cback);
        rfc_handle = handle;
    }
    else if (server == 1)
    {
        BTM_SetSecurityLevel (FALSE, "", 0, 0, 0x03, 3/*BTM_SEC_PROTO_RFCOMM */, 20);
        BTM_SetConnectability (1, 0, 0); //Pae Mode , window , interval
        status = RFCOMM_CreateConnection (0x0020, 20, TRUE, 256, any_add,
                                       &handle, bt_rfc_mmt_server_cback);
        rfc_handle = handle;
    }
    else if (server == 3)
    {
        ALOGI("dut RFCOMM RemoveConnection");
        RFCOMM_RemoveConnection(rfc_handle);
    }
    if (status == PORT_SUCCESS)
    {
        ALOGI("dut_setdata_callback");
        PORT_SetDataCallback (handle, bt_rfc_data_cback);
        PORT_SetEventCallback(handle, bt_rfc_port_cback);
    }
}

void rdut_rfcomm_test_interface (tRFC *input)
{
    RawAddress remote_bd;
    uint16_t handle;
    int status = -1;
    RawAddress any_add = RawAddress::kAny;

    switch (input->param)
    {
        case RFC_TEST_CLIENT:
        {
            remote_bd = input->data.conn.bdadd;
            BTM_SetSecurityLevel (TRUE, "", 0, 0, 0x03, 3/*BTM_SEC_PROTO_RFCOMM */,
                                                                   input->data.conn.scn);
            status = RFCOMM_CreateConnection(0x0020, input->data.conn.scn, FALSE, 256,
                                                 remote_bd,&handle, bt_rfc_mmt_cback);
            rfc_handle = handle;
        }
        break;
        case RFC_TEST_CLIENT_TEST_MSC_DATA:
        {
            remote_bd = input->data.conn.bdadd;
            BTM_SetSecurityLevel (TRUE, "", 0, 0, 0x03, 3/*BTM_SEC_PROTO_RFCOMM */,
                                                                input->data.conn.scn);
            status = RFCOMM_CreateConnection(0x0020, input->data.conn.scn, FALSE, 256,
                                       remote_bd, &handle, bt_rfc_mmt_cback_msc_data);
            rfc_handle = handle;
        }
        break;
        case RFC_TEST_FRAME_ERROR:
        {
             /* Framing Error */
             PORT_SendError (rfc_handle, 0x08);
        }
        break;
        case RFC_TEST_ROLE_SWITCH:
        {
            /* Role Switch */
            BTM_SwitchRole(input->data.role_switch.bdadd.address, input->data.role_switch.role,
                                                                                         NULL);
        }
        break;
        case RFC_TEST_SERVER:
        {
            BTM_SetSecurityLevel (FALSE, "", 0, 0, 0x03, 3/*BTM_SEC_PROTO_RFCOMM */, 20);
            BTM_SetConnectability (1, 0, 0); //Pae Mode , window , interval
            status = RFCOMM_CreateConnection (0x0020, 20, TRUE, 256, any_add,
                                               &handle, bt_rfc_mmt_server_cback);
            rfc_handle = handle;
        }
        break;
        case RFC_TEST_DISCON:
        {
            ALOGI("dut RFCOMM RemoveConnection");
            RFCOMM_RemoveConnection(rfc_handle);
        }
        break;
        case RFC_TEST_WRITE_DATA:
        {
            ALOGI("dut RFC_TEST_WRITE_DATA");
            bt_rfc_send_data(rfc_handle);
        }
        break;
        default :
            ALOGI("dut RFCOMM Unreconised command");
        break;
    }
    if (status == PORT_SUCCESS)
    {
        ALOGI("dut_setdata_callback");
        PORT_SetDataCallback (handle, bt_rfc_data_cback);
        PORT_SetEventCallback(handle, bt_rfc_port_cback);
    }
}
