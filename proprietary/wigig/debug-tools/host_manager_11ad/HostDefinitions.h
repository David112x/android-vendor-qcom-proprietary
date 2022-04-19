/*
* Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
/*
* Copyright (c) 2017-2018, The Linux Foundation. All rights reserved.
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

#pragma once

#include <string>
#include <cstdint>
#include <vector>

// FW Defines
#define MAX_RF_NUMBER 8
#define MAX_MCS_RANGE_SPARROW 12
#define MAX_MCS_RANGE_TALYN 16

#define FW_ASSERT_REG 0x91F020
#define UCODE_ASSERT_REG 0x91F028
#define FW_ASSOCIATION_REG 0x880A44
#define FW_ASSOCIATED_VALUE 0x6
#define MCS_REG 0x880A60
#define DEVICE_TYPE_REG 0x880A8C
#define FW_MODE_REG 0x880A34
#define BOOT_LOADER_VERSION_REG 0x880A0C
#define CHANNEL_REG_PRE_TALYN_MA 0x883020
#define CHANNEL_REG_POST_TALYN_MA 0x88941C
#define BOARDFILE_REG 0x880014

#define UCODE_RX_ON_REG 0x94059E
#define BF_SEQ_REG 0x941374
#define BF_TRIG_REG 0x941380
#define NAV_REG 0x940490
#define TX_GP_REG 0x880A58
#define RX_GP_REG 0x880A5C

#define RF_STATE_REG 0x889488

#define BASEBAND_TEMPERATURE_REG 0x91c808
#define RF_TEMPERATURE_REG 0x91c80c

// *************************************************************************************************
/*
 * The reply data may be generated in several ways, the data is expected to be obtained according to this type
 */
enum REPLY_TYPE
{
    REPLY_TYPE_NONE,
    REPLY_TYPE_BUFFER,
    REPLY_TYPE_FILE,
    REPLY_TYPE_WAIT_BINARY,
    REPLY_TYPE_BINARY
};

// *************************************************************************************************
/*
 * A response to the client, through the servers
 */
struct ResponseMessage
{
    ResponseMessage() :
        type(REPLY_TYPE_NONE),
        length(0U),
        binaryMessage(nullptr),
        inputBufSize(0U)
    {}

    std::string message;
    REPLY_TYPE type;
    size_t length;
    uint8_t* binaryMessage;
    std::vector<std::string> internalParsedMessage;
    unsigned inputBufSize;
};

// *************************************************************************************************
/*
 * ConnectionStatus indicates whether to close a connection with a client or not.
 */
enum ConnectionStatus
{
    CLOSE_CONNECTION,
    KEEP_CONNECTION_ALIVE
};

// *************************************************************************************************

struct HostIps
{
    HostIps() :
        m_ip(""),
        m_broadcastIp("")
    {}

    std::string m_ip; // host's IP address
    std::string m_broadcastIp; // host's broadcast IP address (derivated by subnet mask)
};

// *************************************************************************************************

enum ServerType
{
    stTcp,
    stUdp
};
