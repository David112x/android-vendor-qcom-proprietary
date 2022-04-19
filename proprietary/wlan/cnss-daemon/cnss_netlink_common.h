/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef __CNSS_NETLINK_COMMON_H_
#define __CNSS_NETLINK_COMMON_H_

/* better to alloc this random */
#define NLCMNID 88
#define EVENT_CB_MAX 10

typedef int (*nl80211_event_cb)(char *data, int len);

struct global_nl80211_cb {
	int valid;
	int cmd;
	nl80211_event_cb event_cb;
};

struct handler_args {
	const char *group;
	int id;
};

struct global_netlink_common {
	struct nl_cb *cb;
	struct nl_cb *event_cb;
	struct nl_sock *sockfd;
	struct nl_sock *event_sockfd;
	int nl80211_fam_id;
	int err;
};

#ifdef ENABLE_NETLINK_COMMON
int cnss_nl80211_init(void);
int cnss_nl80211_deinit(void);
int cnss_nl80211_register_handler(nl80211_event_cb event_cb, int cmd);
int cnss_nl80211_unregister_handler(int cmd);
int cnss_nl80211_send_cmd_to_driver(struct nl_msg *nlmsg);
int cnss_nl80211_get_famid(void);
int cnss_nl80211_get_socket(void);
int cnss_nl80211_handle_event(void);
#else
static inline int cnss_nl80211_init(void) { return 0; }
static inline int cnss_nl80211_deinit(void) { return 0; }

static inline int
cnss_nl80211_register_handler(nl80211_event_cb event_cb, int cmd) { return 0; }

static inline int cnss_nl80211_unregister_handler(int cmd) { return 0; }

static inline int
cnss_nl80211_send_cmd_to_driver(struct nl_msg *nlmsg) { return 0; }

static inline int cnss_nl80211_get_famid(void) { return 0; }
static inline int cnss_nl80211_get_socket(void) { return 0; }
static inline int cnss_nl80211_handle_event(void) { return 0; }
#endif
#endif
