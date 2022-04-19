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
/*
 * Driver interaction with Linux nl80211/cfg80211
 * Copyright (c) 2002-2015, Jouni Malinen <j@w1.fi>
 * Copyright (c) 2003-2004, Instant802 Networks, Inc.
 * Copyright (c) 2005-2006, Devicescape Software, Inc.
 * Copyright (c) 2007, Johannes Berg <johannes@sipsolutions.net>
 * Copyright (c) 2009-2010, Atheros Communications
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include "nl_services.h"
#include "nl80211_copy.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <poll.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h> /* for gettimeofday */
#include <time.h>
#include <unistd.h>
#include <getopt.h>

#ifndef SOL_NETLINK
    #define SOL_NETLINK    270
#endif

#define OUI_QCA 0x001374
#define NL_MSG_SIZE_MAX (8 * 1024)

static const uint32_t WMI_UT_EVENT_ID = 0x1900;
static struct driver_event_report s_driverEventReport;

enum qca_nl80211_vendor_subcmds_ {
    QCA_NL80211_VENDOR_SUBCMD_UNSPEC = 0,
};

enum qca_wlan_vendor_attr {
    QCA_WLAN_VENDOR_ATTR_TYPE = 7,
    QCA_WLAN_VENDOR_ATTR_BUF = 8,
    QCA_WLAN_VENDOR_ATTR_MAX,
};

/* This should be removed and DriverCommandType enum class should be used instead
   when netlink calls are compiled as C++ */
enum wil_nl_60g_cmd_type {
    DRIVER_CMD_FW_WMI = 0,
    DRIVER_CMD_GENERIC_COMMAND = 1,
    DRIVER_CMD_GET_DRIVER_STATISTICS = 2,
    DRIVER_CMD_REGISTER = 3
};

/* enumeration of generic commands supported by the Driver */
enum wil_nl_60g_generic_cmd {
    NL_60G_GEN_FORCE_WMI_SEND = 0,
    NL_60G_GEN_RADAR_ALLOC_BUFFER = 1,
    NL_60G_GEN_FW_RESET = 2,
    NL_60G_GEN_GET_DRIVER_CAPA = 3,
    NL_60G_GEN_GET_FW_STATE = 4,
    NL_60G_GEN_AUTO_RADAR_RX_CONFIG = 5
};
	
/* structure with global state, passed to callback handlers */
typedef struct nl_state_t {
    /* callbacks handle for synchronous NL commands */
    struct nl_cb *cb;
    /* nl socket handle for synchronous NL commands */
    struct nl_sock *nl;
    /* nl socket handler for events */
    struct nl_sock *nl_event;
    /* family id for nl80211 events */
    int nl80211_id;
    /* interface index of wigig driver */
    int ifindex;
    /* event answer buffer to be filled */
    struct driver_event_report *driver_event_report_buf;
    /* sent command response */
    uint32_t command_response;
    /* true if driver has ability to publish WMI events and receive wmi CMD */
    bool has_wmi_pub;

    /* additional members required for wmi commuter */
    bool session_started;
    uint8_t* wmi_resp;
    uint32_t* wmi_resp_len_ptr;
    uint32_t expected_event_id;
    int exitSockets[2];

    char operation_status[512];
} nl_state;

/* Definition of WMI command/event payload. In case of event, it's the content of buffer in driver_event_report. */
struct wil_nl_60g_send_receive_wmi {
    uint32_t cmd_id;    /* command or event id */
    uint16_t reserved;  /* reserved for context id, not in use */
    uint8_t  dev_id;    /* mid, not in use */
    uint16_t buf_len;
    uint8_t  buf[];
} __attribute__((packed));

/**
 * nl callback handler for disabling sequence number checking
 */
static int no_seq_check(struct nl_msg *msg, void *arg)
{
    (void)msg;
    (void)arg;
    return NL_OK;
}

/**
 * nl callback handler called on error
 */
static int error_handler(struct sockaddr_nl *nla, struct nlmsgerr *err, void *arg)
{
    (void)nla;

    int *ret = (int *)arg;
    *ret = err->error;
    /* "error handler with error: %d\n", *ret */
    return NL_SKIP;
}

/**
 * nl callback handler called after all messages in
 * a multi-message reply are delivered. It is used
 * to detect end of synchronous command/reply
 */
static int finish_handler(struct nl_msg *msg, void *arg)
{
    (void)msg;

    int *ret = (int *)arg;
    *ret = 0;
    return NL_SKIP;
}

/**
 * nl callback handler called when ACK is received
 * for a command. It is also used to detect end of
 * synchronous command/reply
 */
static int ack_handler(struct nl_msg *msg, void *arg)
{
    (void)msg;

    int *err = (int *)arg;
    *err = 0;
    return NL_STOP;
}

/* helper for translation of errors, known to be sent by the host driver, to status */
static void nl_report_errno_to_status(nl_state *state, int err)
{
    if (!state) { /* shouldn't happen */
        return;
    }

    switch (err) {
    case (-EINVAL):
        snprintf(state->operation_status, sizeof(state->operation_status), "command not supported by the host driver");
        break;
    case (-EOPNOTSUPP):
        snprintf(state->operation_status, sizeof(state->operation_status), "command not supported by the DUT FW");
        break;
    case (-EAGAIN):
        snprintf(state->operation_status, sizeof(state->operation_status), "command blocked, system may be in Sys Assert");
        break;
    default:
        snprintf(state->operation_status, sizeof(state->operation_status), "failed to send command, error is %d", err);
        break;
    }
}

/**
 * handler for resolving multicast group (family) id
 * used in nl_get_multicast_id below
 */
struct family_data {
    const char *group;
    int id;
};

static int family_handler(struct nl_msg *msg, void *arg)
{
    struct family_data *res = (struct family_data *)arg;
    struct nlattr *tb[CTRL_ATTR_MAX + 1];
    struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
    struct nlattr *mcgrp;
    int tmp;

    nla_parse(tb, CTRL_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
          genlmsg_attrlen(gnlh, 0), NULL);
    if (!tb[CTRL_ATTR_MCAST_GROUPS])
        return NL_SKIP;

    nla_for_each_nested(mcgrp, tb[CTRL_ATTR_MCAST_GROUPS], tmp) {
        struct nlattr *tb2[CTRL_ATTR_MCAST_GRP_MAX + 1];

        nla_parse(tb2, CTRL_ATTR_MCAST_GRP_MAX, nla_data(mcgrp),
              nla_len(mcgrp), NULL);
        if (!tb2[CTRL_ATTR_MCAST_GRP_NAME] ||
            !tb2[CTRL_ATTR_MCAST_GRP_ID] ||
            strncmp(nla_data(tb2[CTRL_ATTR_MCAST_GRP_NAME]),
                res->group,
                nla_len(tb2[CTRL_ATTR_MCAST_GRP_NAME])) != 0)
            continue;
        res->id = nla_get_u32(tb2[CTRL_ATTR_MCAST_GRP_ID]);
        break;
    };

    return NL_SKIP;
}

/**
 * handler for NL80211_CMD_GET_WIPHY results
 */
static int wiphy_info_handler(struct nl_msg *msg, void* arg)
{
    nl_state *state = (nl_state *)arg;
    struct nlattr *tb[NL80211_ATTR_MAX + 1];
    struct nlattr *attr;
    struct genlmsghdr *gnlh = (struct genlmsghdr *)nlmsg_data(nlmsg_hdr(msg));
    struct nl80211_vendor_cmd_info *cmd;
    int tmp;

    nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
          genlmsg_attrlen(gnlh,0) , NULL);

    if (tb[NL80211_ATTR_VENDOR_DATA]) {
        nla_for_each_nested(attr, tb[NL80211_ATTR_VENDOR_DATA], tmp) {
            if (nla_len(attr) != sizeof(*cmd)) {
                /* "unexpected vendor cmd info\n" */
                continue;
            }
            cmd = nla_data(attr);
            if (cmd->vendor_id == OUI_QCA &&
                cmd->subcmd ==
                QCA_NL80211_VENDOR_SUBCMD_UNSPEC) {
                state->has_wmi_pub = true;
                break;
            }
        }
    }
    return NL_SKIP;
}

/**
* handler for getting command result value
* Note: The only supported result is 32 bits field
*/
static int command_info_handler(struct nl_msg *msg, void* arg)
{
    nl_state *state = (nl_state *)arg;
    struct nlattr *tb[NL80211_ATTR_MAX + 1];
    struct nlattr *attr;
    struct genlmsghdr *gnlh = (struct genlmsghdr *)nlmsg_data(nlmsg_hdr(msg));
    uint32_t* response;
    int tmp;

    state->command_response = 0x0; // initialize with failure value

    nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
        genlmsg_attrlen(gnlh, 0), NULL);

    if (tb[NL80211_ATTR_VENDOR_DATA]) {
        nla_for_each_nested(attr, tb[NL80211_ATTR_VENDOR_DATA], tmp) {
            if (nla_len(attr) != sizeof(uint32_t)) {
                /* "unexpected response\n" */
                continue;
            }
            // otherwise we get the response
            response = (uint32_t*)(nla_data(attr));
            state->command_response = *response;
            break;
        }
    }

    return NL_SKIP;
}

/**
 * send NL command and receive reply synchronously
 */
static int nl_cmd_send_and_recv(
                nl_state *state,
                struct nl_msg *msg,
                int (*valid_handler)(struct nl_msg *, void *),
                void *valid_data)
{
    struct nl_cb *cb_clone = nl_cb_clone(state->cb);
    int err;

    if (!cb_clone) {
        snprintf(state->operation_status, sizeof(state->operation_status), "failed to allocate cb_clone");
        return -ENOMEM;
    }

    err = nl_send_auto_complete(state->nl, msg);
    if (err < 0) {
        snprintf(state->operation_status, sizeof(state->operation_status), "failed to send message, error %d", err);
        nl_cb_put(cb_clone);
        return err;
    }

    err = 1;
    nl_cb_err(cb_clone, NL_CB_CUSTOM, error_handler, &err);
    nl_cb_set(cb_clone, NL_CB_FINISH, NL_CB_CUSTOM, finish_handler, &err);
    nl_cb_set(cb_clone, NL_CB_ACK, NL_CB_CUSTOM, ack_handler, &err);
    if (valid_handler) {
        nl_cb_set(cb_clone, NL_CB_VALID, NL_CB_CUSTOM, valid_handler, valid_data);
    }

    while (err > 0) {
        int res = nl_recvmsgs(state->nl, cb_clone);
        if (res < 0) { /* shouldn't happen */
            snprintf(state->operation_status, sizeof(state->operation_status), "nl_recvmsgs failed, error %d", res);
            /* do not exit the loop since similar code in supplicant does not */
        }
    }

    if (err < 0) { /* updated by above callbacks */
        nl_report_errno_to_status(state, err);
    }

    nl_cb_put(cb_clone);
    return err;
}

/**
 * get a multicast group id for registering
 * (such as for vendor events)
 */
static int nl_get_multicast_id(nl_state *state,
                   const char *family, const char *group)
{
    struct nl_msg *msg;
    int ret;
    struct family_data res = { group, -ENOENT };

    msg = nlmsg_alloc();
    if (!msg)
        return -ENOMEM;
    if (!genlmsg_put(msg, 0, 0, genl_ctrl_resolve(state->nl, "nlctrl"),
             0, 0, CTRL_CMD_GETFAMILY, 0) ||
        nla_put_string(msg, CTRL_ATTR_FAMILY_NAME, family)) {
        nlmsg_free(msg);
        return -ENOBUFS;
    }

    ret = nl_cmd_send_and_recv(state, msg, family_handler, &res);
    if (ret == 0)
        ret = res.id;

    return ret;
}

/**
 * add a multicast group to an NL socket so it
 * can receive events for that group
 */
int nl_socket_add_membership(struct nl_sock *sk, int group_id)
{
    int err;
    int s_fd = nl_socket_get_fd(sk);

    if (s_fd == -1 || group_id < 0)
        return -EINVAL;

    err = setsockopt(s_fd, SOL_NETLINK, NETLINK_ADD_MEMBERSHIP,
        &group_id, sizeof(group_id));

    if (err < 0)
        return err;

    return 0;
}

/* handle wmi event, reset session_started flag only when the required wmi event was recieved */
static void nl_wmi_event_handler(nl_state *state)
{
    struct driver_event_report *evt = state->driver_event_report_buf;
    struct wil_nl_60g_send_receive_wmi *wmi = NULL;
    uint32_t payload_start_offset = 0;

    /* driver_event_report_buf contains the message content, was copied as part of nl_event_handler */
    if (evt->buf_len < sizeof(struct wil_nl_60g_send_receive_wmi)) {
        /*"NL WMI event too short %d\n", len */
        return;
    }

    wmi = (struct wil_nl_60g_send_receive_wmi *)&evt->buf;

    if (wmi->cmd_id != state->expected_event_id) {
        /* continue waiting for the required event, do not reset session_started */
        return;
    }

    /* in case of UT WMI ignore module adn subtype ids stored in the first payload dword */
    if (wmi->cmd_id == WMI_UT_EVENT_ID) {
        payload_start_offset = 2 * sizeof(uint16_t);
    }

    if (state->wmi_resp) {
        if (*state->wmi_resp_len_ptr > (wmi->buf_len - payload_start_offset)) {
            /* case 1: it'a a dynamic array, maximum was allocated - reduce to the actual length */
            /* case 2: recieved less than expected, will be marked as failure */
            *state->wmi_resp_len_ptr = wmi->buf_len - payload_start_offset;
        }
        /* if received more than expected, ignore it, there is no room to store it, will be marked as failure */

        memcpy(state->wmi_resp, wmi->buf + payload_start_offset, *state->wmi_resp_len_ptr);
    }

    state->session_started = false;
}

/**
 * handle for vendor events
 */
static int nl_event_handler(struct nl_msg *msg, void *arg)
{
    struct nlattr *tb[NL80211_ATTR_MAX + 1];
    struct nlattr *tb2[QCA_WLAN_VENDOR_ATTR_MAX + 1];
    struct genlmsghdr *gnlh = (struct genlmsghdr *)
        nlmsg_data(nlmsg_hdr(msg));
    uint32_t cmd;
    struct driver_event_report* evt;
    nl_state *state = (nl_state *)arg;

    if (!(arg && state->driver_event_report_buf)) {
        return NL_SKIP; /* should not happen */
    }

    nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
        genlmsg_attrlen(gnlh, 0), NULL);

    if (!tb[NL80211_ATTR_VENDOR_ID] ||
        !tb[NL80211_ATTR_VENDOR_SUBCMD] ||
        !tb[NL80211_ATTR_VENDOR_DATA]) {
        return NL_SKIP;
    }

    if (nla_get_u32(tb[NL80211_ATTR_VENDOR_ID]) != OUI_QCA) {
        return NL_SKIP;
    }

    if (nla_parse_nested(tb2, QCA_WLAN_VENDOR_ATTR_MAX,
        tb[NL80211_ATTR_VENDOR_DATA], NULL)) {
        /* "failed to parse vendor command\n" */
        return NL_SKIP;
    }

    cmd = nla_get_u32(tb[NL80211_ATTR_VENDOR_SUBCMD]);
    switch (cmd) {
    case QCA_NL80211_VENDOR_SUBCMD_UNSPEC:
        if (!tb2[QCA_WLAN_VENDOR_ATTR_BUF]) {
            break;
        }

        const uint32_t len = nla_len(tb2[QCA_WLAN_VENDOR_ATTR_BUF]);

        if (len > sizeof(struct driver_event_report)) {
            /* "event respond length is bigger than allocated %d [bytes]\n", sizeof(struct driver_event_report) */
            return NL_SKIP;
        }

        evt = state->driver_event_report_buf;
        memcpy(evt, nla_data(tb2[QCA_WLAN_VENDOR_ATTR_BUF]), len);

        switch (state->driver_event_report_buf->evt_type) {
        case DRIVER_EVENT_FW_EVENT:
            nl_wmi_event_handler(state);
            break;
        case DRIVER_EVENT_DRIVER_GENERIC_EVENT:
            /* ignore for now */
            /* do not reset session_started to continue waiting for relevant WMI event */
            break;
        default:
            /* "unhandled evt_type %d\n", evt->evt_type */
            break;
        }

        break;
    default:
        /* "\nunknown event %d\n", cmd */
        break;
    }

    return NL_SKIP;
}

/**
 * destroy the structures for NL communication
 */
static void destroy_nl_globals(nl_state *state)
{
    if (state->nl) {
        nl_socket_free(state->nl);
        state->nl = NULL;
    }
    if (state->nl_event) {
        nl_socket_free(state->nl_event);
        state->nl_event = NULL;
    }
    if (state->cb) {
        nl_cb_put(state->cb);
        state->cb = NULL;
    }
    state->nl80211_id = 0;
}

/**
 * initialize structures for NL communication
 * in case of failure it is the caller responsibility to call destroy_nl_globals
 */
static bool init_nl_globals(nl_state *state)
{
    int group_id = -ENOMEM;

    /* specify NL_CB_DEBUG instead of NL_CB_DEFAULT to get detailed traces of NL messages */
    state->cb = nl_cb_alloc(NL_CB_DEFAULT);
    if (state->cb == NULL) {
        /* "failed to allocate nl callback\n" */
        return false;
    }

    if (nl_cb_set(state->cb, NL_CB_SEQ_CHECK, NL_CB_CUSTOM, no_seq_check, NULL) < 0) {
        return false;
    }

    state->nl = nl_socket_alloc_cb(state->cb);
    if (state->nl == NULL) {
        /* "failed to allocate nl handle\n" */
        return false;
    }

    if (genl_connect(state->nl) < 0) {
        /* "failed to connect to nl handle\n" */
        return false;
    }

    state->nl80211_id = genl_ctrl_resolve(state->nl, "nl80211");
    if (state->nl80211_id < 0) {
        /* "failed to get nl80211 family, error %d\n", state->nl80211_id */
        return false;
    }

    state->nl_event = nl_socket_alloc_cb(state->cb);
    if (state->nl_event == NULL) {
        /* "failed to allocate nl handle for events\n" */
        return false;
    }

    if (genl_connect(state->nl_event) < 0) {
        /* "failed to connect to nl_event handle\n" */
        return false;
    }

    /* register for receiving vendor events */
    group_id = nl_get_multicast_id(state, "nl80211", "vendor");
    if (group_id < 0) {
        /* "could not get vendor multicast id, err %d\n", rc */
        return false;
    }

    if (nl_socket_add_membership(state->nl_event, group_id) < 0) {
        /* "could not register for vendor events, err %d\n", rc */
        return false;
    }

    if (nl_socket_set_nonblocking(state->nl_event) < 0) {
        return false;
    }

    // provide the state to be passed as last argument to the callback
    // it will contain the buffer address
    if (nl_cb_set(state->cb, NL_CB_VALID, NL_CB_CUSTOM, nl_event_handler, state) < 0) {
        return false;
    }

    return true;
}

/**
 * allocate an nl_msg for sending a command
 */

static struct nl_msg *allocate_nl_cmd_msg(int family, int ifindex,
                      int flags, uint8_t cmd)
{
    // nlmsg_alloc allocates 4KB buffer by default
    // this may not be enough for a WMI command with lengthy payload, replaced with nlmsg_alloc_size
    struct nl_msg *msg = nlmsg_alloc_size(NL_MSG_SIZE_MAX);

    if (!msg) {
        /* "failed to allocate nl msg\n" */
        return NULL;
    }

    if (!genlmsg_put(msg,
              0, // pid (automatic)
              0, // sequence number (automatic)
              family, // family
              0, // user specific header length
              flags, // flags
              cmd, // command
              0) // protocol version
        ) {
        /* "failed to init msg\n" */
        nlmsg_free(msg);
        return NULL;
    }

    if (nla_put_u32(msg, NL80211_ATTR_IFINDEX, (uint32_t)ifindex) < 0) {
        /* "failed to set interface index\n" */
        nlmsg_free(msg);
        return NULL;
    }

    return msg;
}

/**
 * send NL command and receive reply synchronously, for
 * non-blocking sockets
 */
static int nl_cmd_send_and_recv_nonblock(nl_state *state, struct nl_msg *msg)
{
    struct nl_sock *nl = state->nl;
    static const int polling_timeout_msec = 500; /* timeout is in msec. */
    struct nl_cb *cb_clone = nl_cb_clone(state->cb);
    struct pollfd fds;
    int err;

    if (!cb_clone) {
        snprintf(state->operation_status, sizeof(state->operation_status), "failed to allocate cb_clone");
        return -ENOMEM;
    }

    err = nl_send_auto_complete(nl, msg);
    if (err < 0) {
        snprintf(state->operation_status, sizeof(state->operation_status), "failed to send message, error %d", err);
        nl_cb_put(cb_clone);
        return err;
    }

    err = 1;
    nl_cb_err(cb_clone, NL_CB_CUSTOM, error_handler, &err);
    nl_cb_set(cb_clone, NL_CB_FINISH, NL_CB_CUSTOM, finish_handler, &err);
    nl_cb_set(cb_clone, NL_CB_ACK, NL_CB_CUSTOM, ack_handler, &err);

    memset(&fds, 0, sizeof(fds));
    fds.fd = nl_socket_get_fd(nl);
    fds.events = POLLIN;
    while (err > 0) {
        int res = poll(&fds, 1, polling_timeout_msec);

        if (res == 0) {
            snprintf(state->operation_status, sizeof(state->operation_status), "poll timeout of %d msec. reached", polling_timeout_msec);
            err = -1;
            break;
        }
        else if (res < 0) {
            snprintf(state->operation_status, sizeof(state->operation_status), "poll failed, error %d", res);
            err = -2;
            break;
        }

        if (fds.revents & POLLIN) {
            res = nl_recvmsgs(nl, cb_clone);
            if (res < 0) { /* shouldn't happen */
                snprintf(state->operation_status, sizeof(state->operation_status), "nl_recvmsgs failed, error %d", res);
                err = -3;
                break;
            }
        }
    }

    if (err < 0) { /* updated by above callbacks */
        nl_report_errno_to_status(state, err);
    }

    nl_cb_put(cb_clone);
    return err;
}

/**
* get publish_event capability for driver using the
* NL80211_CMD_GET_WIPHY command
*/
static bool nl_get_publish_event_capability(nl_state *state)
{
    bool res = false;

    struct nl_msg *msg = allocate_nl_cmd_msg(state->nl80211_id,
        state->ifindex,
        NLM_F_DUMP,
        NL80211_CMD_GET_WIPHY);
    if (msg == NULL) {
        snprintf(state->operation_status, sizeof(state->operation_status), "failed to allocate msg for GET_WIPHY");
        return false;
    }

    if (nla_put_flag(msg, NL80211_ATTR_SPLIT_WIPHY_DUMP) < 0) {
        snprintf(state->operation_status, sizeof(state->operation_status), "failed to set params for GET_WIPHY");
        goto out;
    }

    if (nl_cmd_send_and_recv(state, msg, wiphy_info_handler, state) < 0) {
        snprintf(state->operation_status, sizeof(state->operation_status), "failed to send GET_WIPHY command");
        goto out;
    }

    res = state->has_wmi_pub;

out:
    nlmsg_free(msg);
    return res;
}

/*
 * Send command to the Driver
 * Notes:
 * Id represents driver command type (wil_nl_60g_cmd_type enumeration) which is a contract between the Driver and the command initiator.
 * Response is updated only for DRIVER_CMD_GENERIC_COMMAND.
 */
bool nl_send_driver_command(nl_state *state, uint32_t id, uint32_t bufLen, const void* pBuffer, uint32_t* pResponse)
{
    bool res = false;
    struct nlattr *vendor_data;
    int send_res;

    struct nl_msg *msg = allocate_nl_cmd_msg(state->nl80211_id,
                        state->ifindex,
                        0,
                        NL80211_CMD_VENDOR);
    if (msg == NULL) {
        snprintf(state->operation_status, sizeof(state->operation_status), "failed to allocate msg for VENDOR command");
        goto out;
    }

    if (nla_put_u32(msg, NL80211_ATTR_VENDOR_ID, OUI_QCA) < 0 ||
        nla_put_u32(msg, NL80211_ATTR_VENDOR_SUBCMD, QCA_NL80211_VENDOR_SUBCMD_UNSPEC) < 0) {
        snprintf(state->operation_status, sizeof(state->operation_status), "unable to set parameters for QCA_NL80211_VENDOR_SUBCMD_UNSPEC");
        goto out;
    }

    vendor_data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
    if (!vendor_data) {
        snprintf(state->operation_status, sizeof(state->operation_status), "fail nla_nest_start for vendor_data");
        goto out;
    }

    if (nla_put_u32(msg, QCA_WLAN_VENDOR_ATTR_TYPE, id)) {
        snprintf(state->operation_status, sizeof(state->operation_status), "unable to set wmi_send QCA_WLAN_VENDOR_ATTR_TYPE");
        goto out;
    }

    if (nla_put(msg, QCA_WLAN_VENDOR_ATTR_BUF, (int)bufLen, pBuffer) < 0) {
        snprintf(state->operation_status, sizeof(state->operation_status), "unable to set command payload");
        goto out;
    }

    nla_nest_end(msg, vendor_data); /* always returns zero */

    if (pResponse && id == DRIVER_CMD_GENERIC_COMMAND)
    {   // response required, blocking send-receive
        send_res = nl_cmd_send_and_recv(state, msg, command_info_handler, state);
        if (send_res < 0) {
            /* status updated */
            goto out;
        }

        *pResponse = state->command_response;
    }
    else
    {   // no response expected, non blocking send-receive
        send_res = nl_cmd_send_and_recv_nonblock(state, msg);
        if (send_res < 0) {
            /* status updated */
            goto out;
        }
    }

    res = true;

out:
    nlmsg_free(msg);
    return res;
}

bool nl_get_driver_event(nl_state *state, int cancelationFd, struct driver_event_report* pMessageBuf, int timeoutMsec)
{
    /* 'cancelationFd' is a file descriptor for one of the sockets from the cancellation sockets pair */
    /* sockets pair serves as a pipe - a value written to one of its sockets, is also written to the second one */
    struct pollfd fds[2];
    memset(fds, 0, sizeof(fds));
    fds[0].fd = nl_socket_get_fd(state->nl_event);
    fds[0].events |= POLLIN;
    fds[1].fd = cancelationFd;
    fds[1].events |= POLLIN;

    int res = poll(fds, 2, timeoutMsec); /* negative value means infinite timeout */

    if (res == 0) {
        snprintf(state->operation_status, sizeof(state->operation_status), "timeout waiting for event");
        return false;
    }
    if (res < 0) {
        snprintf(state->operation_status, sizeof(state->operation_status), "poll error %d when waiting for event", res);
        return false;
    }

    if (fds[0].revents & POLLIN) {
        state->driver_event_report_buf = pMessageBuf; /* store report pointer to be used inside the callback*/
        res = nl_recvmsgs(state->nl_event, state->cb);
        if (res < 0) {
            snprintf(state->operation_status, sizeof(state->operation_status), "nl_recvmsgs failed, error %d", res);
            return false;
        }
    }
    else
    {
        snprintf(state->operation_status, sizeof(state->operation_status), "no event received");
        return false;
    }

    return true;
}

/* internal helper for sending WMI/UT command and waiting for response */
/* \remark may block up to the predefined timeout of 5sec. */
static bool send_wmi_command_and_get_response_blocking(nl_state_ptr pState, uint32_t eventId,
                                                       struct wil_nl_60g_send_receive_wmi* cmd, uint32_t sz,
                                                       uint8_t* pResponse, uint32_t* pResponseLen)
{
    static const uint32_t sc_timeoutMsec = 5000; /* maximum timeout when waiting for wmi event */

    bool res = false;
    uint32_t elapsedMsec = 0U;
    struct timeval before, after;

    if (!nl_send_driver_command(pState, (uint32_t)DRIVER_CMD_FW_WMI, sz, cmd, NULL /*no response*/)) {
        /* status updated */
        return false;
    }

    /* wait for WMI events and session is complete. Typically this loop should run in a separate thread */
    pState->session_started = true;
    pState->wmi_resp = pResponse;
    pState->wmi_resp_len_ptr = pResponseLen;
    pState->expected_event_id = eventId;

    /* wait for required event, session_started reset happens in nl_wmi_event_handler */
    gettimeofday(&before, NULL);

    while (pState->session_started && elapsedMsec < sc_timeoutMsec) {
        /* wait for requested event till the defined timeout reached */
        res = nl_get_driver_event(pState, pState->exitSockets[1] /* cancellation socket fd */, &s_driverEventReport, sc_timeoutMsec - elapsedMsec);

        if (!pState->session_started) {
            /* requested event received */
            return true;
        }

        /* in case of timeout, elapsed time will exceed predefined timeout value */
        /* otherwise, got irrelevant event or failure, will continue waiting */
        gettimeofday(&after, NULL);
        elapsedMsec = (after.tv_sec - before.tv_sec) * 1000.0; /* sec to msec */
        elapsedMsec += (after.tv_usec - before.tv_usec) / 1000.0; /* add usec to msec */
    }

    if (pState->session_started && res == true) {
        /* got irrelevant event and ran out of time */
        snprintf(pState->operation_status, sizeof(pState->operation_status), "timeout when waiting for WMI/UT");
        return false;
    }
    /* otherwise status is updated */

    return false;
}

/* Blocking call to send WMI command and wait for the response (or 5 sec. timeout)
 * Response length can be potentially decreased in the actual response is shorter than expected
 */
bool nl_send_wmi_blocking(nl_state_ptr pState, uint32_t commandId, uint32_t eventId,
                          const uint8_t* pPayload, uint32_t payloadLen,
                          uint8_t* pResponse, uint32_t* pResponseLen)
{
    struct wil_nl_60g_send_receive_wmi* cmd = NULL;
    const uint32_t sz = sizeof(struct wil_nl_60g_send_receive_wmi) + payloadLen;
    bool res = false;

    if (!pPayload) {
        snprintf(pState->operation_status, sizeof(pState->operation_status), "failed to send WMI command 0x%x: payload input argument is required", commandId);
        return false;
    }

    /* it's OK to pass NULL when response is not required */
    /* test the length pointer given when response required */
    if (pResponse && !pResponseLen) {
        snprintf(pState->operation_status, sizeof(pState->operation_status), "failed to send WMI command 0x%x: response length output argument is required", commandId);
        return false;
    }

    /* allocate wmi command struct and copy given payload */
    cmd = malloc(sz);
    if (!cmd) {
        snprintf(pState->operation_status, sizeof(pState->operation_status), "cannot allocate WMI command structure for command 0x%x", commandId);
        return false;
    }

    cmd->cmd_id = commandId;
    cmd->reserved = 0;
    cmd->dev_id = 0;
    cmd->buf_len = payloadLen;
    if (payloadLen > 0U) { /* payload is zero when command has no parameters */
        memcpy(cmd->buf, pPayload, payloadLen);
    }

    res = send_wmi_command_and_get_response_blocking(pState, eventId, cmd, sz, pResponse, pResponseLen);
    free(cmd);

    /* status updated in case of failure */
    return res;
}

/* Blocking call to send UT WMI command and wait for the response (or 5 sec. timeout)
 * Module Id is comming from wmi_ut_module enumeration
 * Response length can be potentially decreased in the actual response is shorter than expected
 */
bool nl_send_wmi_ut_blocking(nl_state_ptr pState, uint16_t moduleId, uint16_t subtypeId,
                             const uint8_t* pPayload, uint32_t payloadLen,
                             uint8_t* pResponse, uint32_t* pResponseLen)
{
    static const uint32_t WMI_UT_CMD_ID = 0x900;

    struct wil_nl_60g_send_receive_wmi* cmd = NULL;
    const uint32_t totalPayloadLen = payloadLen + sizeof(moduleId) + sizeof(subtypeId);
    const uint32_t sz = sizeof(struct wil_nl_60g_send_receive_wmi) + totalPayloadLen;
    bool res = false;

    if (!pPayload) {
        snprintf(pState->operation_status, sizeof(pState->operation_status), "failed to send UT command for module %d, subtype %d - payload input argument is required", moduleId, moduleId);
        return false;
    }

    /* it's OK to pass NULL when response is not required */
    /* test the length pointer given when response required */
    if (pResponse && !pResponseLen) {
        snprintf(pState->operation_status, sizeof(pState->operation_status), "failed to send UT command for module %d, subtype %d - response length output argument is required", moduleId, moduleId);
        return false;
    }

    /* allocate wmi command struct and copy given payload */
    /* note: module id and subtype id are stored in the first dword of the payload */
    cmd = malloc(sz);
    if (!cmd) {
        snprintf(pState->operation_status, sizeof(pState->operation_status), "cannot allocate UT command structure for module %d, subtype %d", moduleId, moduleId);
        return false;
    }

    cmd->cmd_id = WMI_UT_CMD_ID; /* always same command id in case of UT */
    cmd->reserved = 0;
    cmd->dev_id = 0;
    cmd->buf_len = totalPayloadLen;
    memcpy(cmd->buf, &moduleId, sizeof(moduleId));
    memcpy(cmd->buf + sizeof(moduleId), &subtypeId, sizeof(subtypeId));
    if (payloadLen > 0U) { /* payload is zero when command has no parameters */
        memcpy(cmd->buf + sizeof(moduleId) + sizeof(subtypeId), pPayload, payloadLen);
    }

    res = send_wmi_command_and_get_response_blocking(pState, WMI_UT_EVENT_ID, cmd, sz, pResponse, pResponseLen);   
    free(cmd);
    
    /* status updated in case of failure */
    return res;
}

/* Initialize the netlink interface */
bool nl_initialize(const char* interfaceName, nl_state** ppState, char* status, uint32_t status_len)
{
    int ifindex = 0;
    nl_state* pState = NULL;

    if (!ppState) {
        snprintf(status, status_len, "nl_state output argument is required");
        return false;
    }

    ifindex = if_nametoindex(interfaceName);
    if (ifindex == 0) {
        snprintf(status, status_len, "unknown WIGIG interface '%s'", interfaceName);
        return false;
    }

    pState = (nl_state*)malloc(sizeof(nl_state));
    if (!pState) {
        snprintf(status, status_len, "cannot allocate netlink state descriptor, out of memory");
        return false;
    }

    memset(pState, 0, sizeof(*pState)); /* also marks status as empty string */
    pState->ifindex = ifindex;
    
    /* initialize structures for NL communication */
    if (!init_nl_globals(pState)) {
        snprintf(status, status_len, "%s", pState->operation_status);
        nl_release(pState); /*it is the caller responsibility */
        return false;
    }

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, pState->exitSockets) == -1) /* shouldn't happen */
    {
        snprintf(status, status_len, "cannot create socket pair required for communication");
        nl_release(pState);
        return false;
    }

    /* test if the driver supports event publishing through netlink interface */
    if (!nl_get_publish_event_capability(pState)) {
        snprintf(status, status_len, "event publishing is not supported by the host driver");
        nl_release(pState);
        return false;
    }

    /* enable events transport on the driver side, i.e. ask to publish them to the user space 
       note: this is different from the regular flow where registration command is sent explicitly */
    if (!nl_enable_driver_events_transport(pState, true)) {
        snprintf(status, status_len, "%s", pState->operation_status);
        nl_release(pState);
        return false;
    }

    *ppState = pState;
    return true;
}

static void nl_close_socket_pair(nl_state* pState)
{
    if (pState->exitSockets[0] >= 0)
    {
        close(pState->exitSockets[0]);
    }
    if (pState->exitSockets[1] >= 0)
    {
        close(pState->exitSockets[1]);
    }
}

void nl_release(nl_state* pState)
{
    if (!pState) {
        return;
    }

    nl_close_socket_pair(pState);
    destroy_nl_globals(pState);
    free(pState);
}

/* Get bitmask of the driver capabilities */
bool nl_get_capabilities(nl_state_ptr pState, uint32_t* pDriverCapabilities)
{
    if (!pDriverCapabilities)
    {
        snprintf(pState->operation_status, sizeof(pState->operation_status), "driver capabilities output argument is required");
        return false;
    }

    uint32_t buf = (uint32_t)NL_60G_GEN_GET_DRIVER_CAPA; // payload contains only the generic command id
    return nl_send_driver_command(pState, (uint32_t)DRIVER_CMD_GENERIC_COMMAND, sizeof(buf), &buf, pDriverCapabilities);
}

/* Request to Enable/Disable events publishing to the user space */
bool nl_enable_driver_events_transport(nl_state_ptr pState, bool enable)
{
    // send buffer of 4 bytes with 1 to enable and zero to disable
    uint32_t buf = enable ? (uint32_t)1U : (uint32_t)0U;

    return nl_send_driver_command(pState, (uint32_t)DRIVER_CMD_REGISTER, sizeof(buf), &buf, NULL /*no response*/);
}

/* FW Reset through generic driver command */
bool nl_fw_reset(nl_state_ptr pState)
{
    uint32_t buf = (uint32_t)NL_60G_GEN_FW_RESET; // payload contains only the generic command id
    return nl_send_driver_command(pState, (uint32_t)DRIVER_CMD_GENERIC_COMMAND, sizeof(buf), &buf, NULL /*no response*/);
}

/* Get description of the last error for given NL descriptor */
const char* nl_get_last_errror(nl_state_ptr pState)
{
    if (!pState) {
        return "Invalid argument. State handle required to get last error";
    }

    return pState->operation_status;
}

/* Clear last error */
void nl_clear_last_error(nl_state_ptr pState)
{
    if (!pState) { /* shouldn't happen */
        return;
    }

    pState->operation_status[0] = '\0';
}
