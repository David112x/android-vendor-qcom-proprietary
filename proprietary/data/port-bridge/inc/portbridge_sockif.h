/**
  @file
  portbridge_sockif.h

  @brief
  This file provides an API to interact with port-bridge using UNIX socket
  for AT command.

 */

/*==========================================================================

Copyright (c) 2019 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PORTBRIDGE_SOCKIF_H
#define PORTBRIDGE_SOCKIF_H


/*---------------------------------------------------------------------------
 *                            DECLARATIONS
 *---------------------------------------------------------------------------*/
#include<stdint.h>
#define PORTBRIDGE_SOCK_PATH  "/dev/socket/port-bridge/port_bridge_connect_socket"
                               /*File to link to the server socket*/
#define MAX_MSG_SIZE (16 * 1024)
/*___________________________________________________________________________
 *
 * Type Representing Enumeration of control and data messages.
 *___________________________________________________________________________*/

typedef enum
{
  PORT_BRIDGE_SUCCESS = 0,
  PORT_BRIDGE_FAILURE = -1
}portbridge_result_msg;

typedef enum
{
  PORTBRIDGE_SOCK_INVALID_EV = -1,               /* Internal Value*/
  PORTBRIDGE_SOCK_DATA_IND = 0,                  /* Data(AT commands/PPP/IP data)
                                                    from and to Server/client*/
  PORTBRIDGE_SOCK_SIGNAL_IND,                    /* RS-232 Signals from/to
                                                    client and Server */
  PORTBRIDGE_SOCK_SOCKET_MODE_ENABLE_REQ,        /* Enable port bridge socket
                                                    mode request(client) */
  PORTBRIDGE_SOCK_SOCKET_MODE_ENABLE_RES,        /* Enable port bridge socket
                                                    mode response ( server)*/
  PORTBRIDGE_SOCK_SOCKET_MODE_DISABLE_REQ,       /* Disable port bridge socket
                                                    mode request ( client)*/
  PORTBRIDGE_SOCK_SOCKET_MODE_DISABLE_RES,       /* Disable port bridge socket
                                                    mode response  (server)*/
  PORTBRIDGE_SOCK_MAX_EV = 0xFF                  /* Internal Value*/
}portbridge_sock_msg_t;

typedef struct
{
  unsigned int data_len;                  /*Length of data being send*/
  char data[MAX_MSG_SIZE];                /*Buffer containing Data from and to
                                            client and server*/
}portbridge_data_buff;

typedef union
{
  portbridge_result_msg result;           /*Result of Response messages
                                            (Success(0) /Failure(1)*/
  int signals;                            /*RS-232 Signal value passed
                                             from BT server to Port-Bridge*/
  portbridge_data_buff data_buffer;
}portbridge_data_msg_type;

/*___________________________________________________________________________
 *
 * Common Structure used for Control messages, signal and data transfer.
 * __________________________________________________________________________*/
typedef struct
{
  portbridge_sock_msg_t     msg_id;
  portbridge_data_msg_type  data_msg;
}__attribute__((packed)) portbridge_sock_intf_data_t;

#endif /* PORTBRIDGE_SOCKIF_H */
#ifdef __cplusplus
}
#endif
