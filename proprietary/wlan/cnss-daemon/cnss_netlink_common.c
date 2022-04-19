/*
 * Copyright (c) 2011-2012, 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * 2011-2012 Qualcomm Atheros Inc.
 * All rights reserved.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/netlink.h>
#include <linux/nl80211.h>
#include <linux/if_addr.h>
#include <linux/if_ether.h>
#include <netinet/in.h>
#include <net/if.h>

#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include <netlink/netlink.h>
#include <netlink/socket.h>
#include <netlink-private/object-api.h>
#include <netlink-private/types.h>
#include "wlan_msg.h"
#include "debug.h"

#include "cnss_netlink_common.h"

static struct global_netlink_common global_nlcmn;
static struct global_nl80211_cb nl80211_cb[EVENT_CB_MAX];

static int nl80211_no_seq_check(struct nl_msg *msg, void *arg)
{
	UNUSED(msg);
	UNUSED(arg);
	wsvc_printf_dbg("netlink no sequence check received\n");

	return NL_OK;
}

static int nl80211_error_handler(struct sockaddr_nl *nla,
				 struct nlmsgerr *err, void *arg)
{
	int *ret = (int *)arg;

	UNUSED(nla);
	*ret = err->error;
	wsvc_printf_dbg("netlink error handler invoked err: %s\n",
			strerror(err->error));

	return NL_SKIP;
}

static int nl80211_finish_handler(struct nl_msg *msg, void *arg)
{
	int *ret = (int *)arg;

	UNUSED(msg);
	*ret = 0;
	wsvc_printf_dbg("netlink finish handler invoked\n");

	return NL_SKIP;
}

static int nl80211_ack_handler(struct nl_msg *msg, void *arg)
{
	int *err = (int *)arg;

	UNUSED(msg);
	*err = 0;
	wsvc_printf_dbg("netlink ack handler invoked\n");

	return NL_STOP;
}

static int nl80211_response_handler(struct nl_msg *msg, void *arg)
{
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct nlattr *tb[NL80211_ATTR_MAX+1];
	int ifidx = -1;
	int wdev_id = 0;
	int wdev_id_set = 0;
	int vendor_id = 0, subcmd = 0;
	char *data = NULL;
	int len, ret, i;

	UNUSED(arg);
	wsvc_printf_info("nl80211 response handler invoked\n");

	ret = nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
			genlmsg_attrlen(gnlh, 0), NULL);
	if (ret < 0)
		return 0;

	if (tb[NL80211_ATTR_IFINDEX])
		ifidx = nla_get_u32(tb[NL80211_ATTR_IFINDEX]);
	else if (tb[NL80211_ATTR_WDEV]) {
		wdev_id = nla_get_u64(tb[NL80211_ATTR_WDEV]);
		wdev_id_set = 1;
	}

	if (tb[NL80211_ATTR_VENDOR_ID] && tb[NL80211_ATTR_VENDOR_SUBCMD]) {
		vendor_id = nla_get_u32(tb[NL80211_ATTR_VENDOR_ID]);
		subcmd = nla_get_u32(tb[NL80211_ATTR_VENDOR_SUBCMD]);
	}

	wsvc_printf_info("%s: cmd %d, vendorID %d, subcmd %d  received\n",
			 __func__, gnlh->cmd, vendor_id, subcmd);

	if (tb[NL80211_ATTR_VENDOR_DATA]) {
		data = nla_data(tb[NL80211_ATTR_VENDOR_DATA]);
		len = nla_len(tb[NL80211_ATTR_VENDOR_DATA]);

		for (i = 0; i < EVENT_CB_MAX; i++) {
			if (nl80211_cb[i].valid && nl80211_cb[i].cmd ==
			    subcmd) {
				nl80211_cb[i].event_cb(data, len);
				break;
			}
		}
	}
	return 0;
}

static int nl80211_family_handler(struct nl_msg *msg, void *arg)
{
	struct handler_args *grp = arg;
	struct nlattr *tb[CTRL_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct nlattr *mcgrp;
	int rem_mcgrp;

	nla_parse(tb, CTRL_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
		  genlmsg_attrlen(gnlh, 0), NULL);

	if (!tb[CTRL_ATTR_MCAST_GROUPS])
		return NL_SKIP;

	nla_for_each_nested(mcgrp, tb[CTRL_ATTR_MCAST_GROUPS], rem_mcgrp) {
		struct nlattr *tb_mcgrp[CTRL_ATTR_MCAST_GRP_MAX + 1];

		nla_parse(tb_mcgrp, CTRL_ATTR_MCAST_GRP_MAX,
			  nla_data(mcgrp), nla_len(mcgrp), NULL);

		if (!tb_mcgrp[CTRL_ATTR_MCAST_GRP_NAME] ||
		    !tb_mcgrp[CTRL_ATTR_MCAST_GRP_ID])
			continue;
		else
			grp->id = nla_get_u32(tb_mcgrp[CTRL_ATTR_MCAST_GRP_ID]);
		if (strncmp(nla_data(tb_mcgrp[CTRL_ATTR_MCAST_GRP_NAME]),
			    grp->group,
			    nla_len(tb_mcgrp[CTRL_ATTR_MCAST_GRP_NAME])))
			continue;
		grp->id = nla_get_u32(tb_mcgrp[CTRL_ATTR_MCAST_GRP_ID]);
		break;
	}

	return NL_SKIP;
}

static int nl80211_get_multicast_id(struct nl_sock *sock, const char *family,
				    const char *group)
{
	struct nl_msg *msg;
	struct nl_cb *cb;
	int ret, ctrlid;
	struct handler_args grp = {
		.group = group,
		.id = -ENOENT,
	};

	msg = nlmsg_alloc();
	if (!msg)
		return -ENOMEM;

	cb = nl_cb_alloc(NL_CB_DEFAULT);
	if (!cb) {
		ret = -ENOMEM;
		goto out_fail_cb;
	}

	ctrlid = genl_ctrl_resolve(sock, "nlctrl");
#ifdef ANDROID
	genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ, GENL_ID_CTRL, 0, 0,
		    CTRL_CMD_GETFAMILY, 1);
#else
	genlmsg_put(msg, 0, 0, ctrlid, 0, 0, CTRL_CMD_GETFAMILY, 0);
#endif
	ret = -ENOBUFS;
	NLA_PUT_STRING(msg, CTRL_ATTR_FAMILY_NAME, family);

	ret = nl_send_auto_complete(sock, msg);
	if (ret < 0)
		goto out;

	global_nlcmn.err = 1;

	nl_cb_err(cb, NL_CB_CUSTOM, nl80211_error_handler, &global_nlcmn.err);
	nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, nl80211_ack_handler,
		  &global_nlcmn.err);
	nl_cb_set(cb, NL_CB_FINISH, NL_CB_CUSTOM, nl80211_finish_handler,
		  &global_nlcmn.err);
	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, nl80211_family_handler, &grp);

	while (global_nlcmn.err > 0)
		nl_recvmsgs(sock, cb);

	if (global_nlcmn.err == 0)
		ret = grp.id;
nla_put_failure:
out:
	nl_cb_put(cb);
out_fail_cb:
	nlmsg_free(msg);
	return ret;
}

static struct nl_sock *nl80211_handle_alloc()
{
	struct nl_sock *handle;
	int pid = getpid() & 0x3FFFFF;
	static int count = NLCMNID;
	int ret;

	handle = nl_socket_alloc();
	if (handle == NULL) {
		wsvc_printf_err("netlink alloc failed\n");
		return NULL;
	}

	pid += count << 22;
	count++;

	handle->s_flags |= NL_OWN_PORT;
	handle->s_local.nl_pid = pid;

	ret = nl_connect(handle, NETLINK_GENERIC);
	if (ret) {
		nl_socket_free(handle);
		wsvc_printf_err("netlink connect failed\n");
		return NULL;
	}

	return handle;
}

static struct nl_cb *nl80211_set_cb(struct nl_sock *handle, int flag)
{
	struct nl_cb *cb;

	cb = nl_socket_get_cb(handle);
	if (cb == NULL) {
		wsvc_printf_err("netlink cb allocation failed\n");
		return NULL;
	}

	nl_cb_set(cb, NL_CB_SEQ_CHECK, NL_CB_CUSTOM, nl80211_no_seq_check,
		  NULL);
	nl_cb_err(cb, NL_CB_CUSTOM, nl80211_error_handler, &global_nlcmn.err);
	nl_cb_set(cb, NL_CB_FINISH, NL_CB_CUSTOM, nl80211_finish_handler,
		  &global_nlcmn.err);
	nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, nl80211_ack_handler,
		  &global_nlcmn.err);
	if (flag)
		nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM,
			  nl80211_response_handler, NULL);
	else
		nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, NULL, NULL);
	nl_cb_put(cb);

	return cb;
}

int cnss_nl80211_handle_event(void)
{
	int res;

	wsvc_printf_dbg("Event message available\n");
	res = nl_recvmsgs(global_nlcmn.event_sockfd, global_nlcmn.event_cb);
	if (res < 0)
		wsvc_printf_err("Event recvmsgs failed: %d\n", res);

	return res;
}

int cnss_nl80211_get_socket(void)
{
	return global_nlcmn.event_sockfd->s_fd;
}

int cnss_nl80211_get_famid(void)
{
	return global_nlcmn.nl80211_fam_id;
}

int cnss_nl80211_send_cmd_to_driver(struct nl_msg *nlmsg)
{
	struct nl_cb *cb = global_nlcmn.cb;
	int err;

	if (NULL == global_nlcmn.sockfd) {
		wsvc_printf_err("nl command socket is not created\n");
		return -ENODEV;
	}

	err = nl_send_auto_complete(global_nlcmn.sockfd, nlmsg);
	if (err < 0) {
		wsvc_printf_err("nl auto complete err: %d\n", err);
		return -EIO;
	}

	err = nl_recvmsgs(global_nlcmn.sockfd, cb);
	if (err)
		wsvc_printf_err("message send failed: %d\n", err);
	else
		wsvc_printf_dbg("message successfully sent\n");

	return 0;
}

int cnss_nl80211_register_handler(nl80211_event_cb event_cb, int cmd)
{
	int i;

	for (i = 0; i < EVENT_CB_MAX; i++) {
		if (nl80211_cb[i].valid && nl80211_cb[i].cmd == cmd) {
			wsvc_printf_err("event handler registered!");
			return 0;
		}
	}

	for (i = 0; i < EVENT_CB_MAX; i++) {
		if (!nl80211_cb[i].valid)
			break;
	}

	if (i == EVENT_CB_MAX) {
		wsvc_printf_err("can't register handler!");
		return -EINVAL;
	}

	nl80211_cb[i].valid = 1;
	nl80211_cb[i].cmd = cmd;
	nl80211_cb[i].event_cb = event_cb;

	return 0;
}

int cnss_nl80211_unregister_handler(int cmd)
{
	int i;

	for (i = 0; i < EVENT_CB_MAX; i++) {
		if (nl80211_cb[i].valid && nl80211_cb[i].cmd == cmd) {
			wsvc_printf_dbg("unregister cmd %d!", cmd);
			nl80211_cb[i].valid = 0;
			nl80211_cb[i].cmd = 0;
			nl80211_cb[i].event_cb = NULL;
			return 0;
		}
	}
	return -EINVAL;
}

int cnss_nl80211_init(void)
{
	int ret;

	global_nlcmn.sockfd = nl80211_handle_alloc();
	if (!global_nlcmn.sockfd) {
		wsvc_printf_err("%s: create nl_socket failed\n", __func__);
		return -ENODEV;
	}
	global_nlcmn.nl80211_fam_id =
		genl_ctrl_resolve(global_nlcmn.sockfd, "nl80211");


	global_nlcmn.event_sockfd = nl80211_handle_alloc();
	if (!global_nlcmn.event_sockfd) {
		nl_close(global_nlcmn.sockfd);
		nl_socket_free(global_nlcmn.sockfd);
		global_nlcmn.sockfd = NULL;
		wsvc_printf_err("%s: create nl_event_socket failed\n",
				__func__);
		return -ENODEV;
	}

	ret = nl80211_get_multicast_id(global_nlcmn.sockfd, "nl80211",
				       "vendor");
	if (ret >= 0)
		ret = nl_socket_add_membership(global_nlcmn.event_sockfd, ret);
	else
		wsvc_printf_err("%s: can't add for vendor: %d\n", __func__,
				ret);

	global_nlcmn.event_cb =
		    nl80211_set_cb(global_nlcmn.event_sockfd, 1);
	global_nlcmn.cb =
		    nl80211_set_cb(global_nlcmn.sockfd, 0);

	wsvc_printf_dbg("%s: wifi nl socket successfully initialized\n",
			__func__);

	return 0;
}

int cnss_nl80211_deinit(void)
{
	if (global_nlcmn.sockfd) {
		nl_close(global_nlcmn.sockfd);
		nl_socket_free(global_nlcmn.sockfd);
		global_nlcmn.sockfd = NULL;
	}

	if (global_nlcmn.event_sockfd) {
		nl_close(global_nlcmn.event_sockfd);
		nl_socket_free(global_nlcmn.event_sockfd);
		global_nlcmn.event_sockfd = NULL;
	}

	memset(&global_nlcmn, 0, sizeof(global_nlcmn));
	return 0;
}
