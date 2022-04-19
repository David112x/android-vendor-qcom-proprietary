/*********************************************************************
*
* Copyright (c) 2016, The Linux Foundation. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
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
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
* BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
* EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
************************************************************************/

#include <stdio.h>
#include <sys/types.h>

/**
 * Commands
 */
typedef enum  {
    VENDOR_LOGGER_LOGS = 201, // Signifies Packet containing Logger Log
    GENERATE_VND_LOG_SIGNAL, // Signifies command to generate logs
    START_SNOOP_SIGNAL,
    STOP_SNOOP_SIGNAL,
    STOP_LOGGING_SIGNAL,
} CommandTypes;

/*
** Set property: "persist.bt_logger.log_mask" by ORing these features

** Example: To enable Full snoop logging and Dynamic logcat logs capture,
** property value should be (HCI_SNOOP_LOG_FULL|DYNAMIC_LOGCAT_CAPTURE) = 3
*/
typedef enum {
    HCI_SNOOP_LOG_FULL = 1,     // Complete HCI snoop logs with media packets
    DYNAMIC_LOGCAT_CAPTURE = 2, // Level 6 logcat logs over logger socket
    HCI_SNOOP_LOG_LITE = 4,     // Always enabled, HCI snoop logs sans media packets
    HCI_SNOOP_ONLY_HEADER = 8,  //  HCI snoop logs with only ACL Header packets
} LoggingFlags;

void init_vnd_Logger(void);
void clean_vnd_logger(void);

typedef struct {
    /** Set to sizeof(bt_vndor_interface_t) */
    size_t          size;

    /*
     * Functions need to be implemented in Logger libray (libbt-logClient.so).
     */

    /*
     *  Initialize logging by conneting client socket
     *  to Logger process
     */
    int   (*init)(void);

    /**  Sending Logs of Logger process */
    void (*send_log_msg)(const char *tag, const char *fmt_str, va_list ap);
    void (*send_log_data)(const char *tag, const char *fmt_str, ...);

    /**  Sending Logs of Logger process */
    void (*send_event)(char evt);

    /** Closes the socket connection to logger process */
    int  (*cleanup)(void);

} bt_logger_interface_t;

extern uint16_t vendor_logging_level;

#define GENERATE_VND_LOGS() if(logger_interface)logger_interface->send_event(GENERATE_VND_LOG_SIGNAL)
#define START_SNOOP_LOGGING() if(logger_interface)logger_interface->send_event(START_SNOOP_SIGNAL)
#define STOP_SNOOP_LOGGING() if(logger_interface)logger_interface->send_event(STOP_SNOOP_SIGNAL)
