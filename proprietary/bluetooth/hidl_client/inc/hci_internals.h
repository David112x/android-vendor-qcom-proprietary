/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
*/

#include <stdlib.h>

// HCI UART transport packet types (Volume 4, Part A, 2)

#define BT_PACKET_TYPE_UNKNOWN  0
#define BT_PACKET_TYPE_COMMAND  1
#define BT_PACKET_TYPE_ACL_DATA 2
#define BT_PACKET_TYPE_SCO_DATA 3
#define BT_PACKET_TYPE_EVENT    4
#define ANT_PACKET_TYPE_CTRL    0x0c
#define ANT_PACKET_TYPE_DATA    0x0e
#define FM_PACKET_TYPE_CMD      0x11
#define FM_PACKET_TYPE_EVENT    0x14

// Re-run |fn| system call until the system call doesn't cause EINTR.
#define CLIENT_NO_INTR(fn)  do {} while ((fn) == -1 && errno == EINTR)

#define LOG_TAG "bt_hidl_client"

// 2 bytes for opcode, 1 byte for parameter length (Volume 2, Part E, 5.4.1)
const int BT_COMMAND_PREAMBLE_SIZE = 3;
const int BT_LENGTH_OFFSET_CMD = 2;

// 2 bytes for handle, 2 bytes for data length (Volume 2, Part E, 5.4.2)
const int BT_ACL_PREAMBLE_SIZE = 4;
const int BT_LENGTH_OFFSET_ACL = 2;

// 2 bytes for handle, 1 byte for data length (Volume 2, Part E, 5.4.3)
const int BT_SCO_PREAMBLE_SIZE = 3;
const int BT_LENGTH_OFFSET_SCO = 2;

// 1 byte for event code, 1 byte for parameter length (Volume 2, Part E, 5.4.4)
const int BT_EVENT_PREAMBLE_SIZE = 2;
const int BT_LENGTH_OFFSET_EVT = 1;

const int ANT_COMMAND_PREAMBLE_SIZE = 1;
const int ANT_LENGTH_OFFSET_CMD = 0;

const int FM_COMMAND_PREAMBLE_SIZE = 3;
const int FM_LENGTH_OFFSET_CMD = 2;

const int FM_EVENT_PREAMBLE_SIZE = 2;
const int FM_LENGTH_OFFSET_EVT = 1;

const int PREAMBLE_SIZE_MAX = BT_ACL_PREAMBLE_SIZE;

