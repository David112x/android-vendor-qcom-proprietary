/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
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

#include "cnss_interop_issues_ap.h"
#include "cnss_netlink_common.h"

#define IOT_AP_NUM 20
#define IOT_AP_MAC_LEN 6

#define IOTAP_PS_FILE_LEN (IOT_AP_MAC_LEN * IOT_AP_NUM)
#define IOTAP_PS_DATA_FILE "/data/vendor/wifi/iotap_ps.bin"

static struct wlan_iot_ap_ps iotap_ps_data[IOT_AP_NUM];
static struct wlan_iot_ap_ps iotap_ps_flag[2] = {
	/* invalid mac */
	{0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	/* invalid mac, driver uses it to trigger app to set ap list */
	{0xff, 0xff, 0xff, 0xff, 0xff, 0xff}
};

static int read_iot_ap_file(char *filename, int filelen, char *data)
{
	int len;
	int len_read = 0;
	FILE *fp = NULL;

	if (access(filename, F_OK) == -1) {
		wsvc_printf_err("interop issues ap: %s: No such file %s",
				__func__, filename);
		return -EPERM;
	}

	fp = fopen(filename, "rb");
	if (!fp) {
		wsvc_printf_err("interop issues ap: %s: Failed to open file %s",
				__func__, filename);
		return -ENOENT;
	}
	fseek(fp, 0, SEEK_END);
	len = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	if (len > filelen) {
		wsvc_printf_err("interop issues ap: %s: Unexpected file len %d",
				__func__, len);
		fclose(fp);
		return -EFBIG;
	}

	len_read = fread(data, 1, len, fp);
	if (len_read != len) {
		wsvc_printf_err("interop issues ap: %s: invalid read %d: %d",
				__func__, len_read, len);
		fclose(fp);
		return -EIO;
	}
	fclose(fp);

	wsvc_printf_dbg("interop issues ap: %s: read file %s", __func__,
			filename);
	return len_read;
}

static int
save_iot_ap_file(const char *filename, int filelen, char *data)
{
	int len_written;
	FILE *fp;

	fp = fopen(filename, "wb");
	if (!fp) {
		wsvc_printf_err("interop issues ap: %s: Failed to open file %s",
				__func__, filename);
		return -ENOENT;
	}

	len_written = fwrite(data, 1, filelen, fp);
	if (len_written != filelen) {
		wsvc_printf_err("interop issues ap: %s: invalid write %d: %d",
				__func__, len_written, filelen);
		fclose(fp);
		return -EIO;
	}
	fclose(fp);

	wsvc_printf_dbg("interop issues ap: %s: Store file: %s ", __func__,
			filename);
	return len_written;
}

static int read_iot_ap_ps_from_file(void)
{
	int len, i;
	struct wlan_iot_ap_ps *iotap;

	len = read_iot_ap_file(IOTAP_PS_DATA_FILE, IOTAP_PS_FILE_LEN,
			       (char *)iotap_ps_data);
	if (len < IOTAP_PS_FILE_LEN) {
		wsvc_printf_err("interop issues ap: %s, read file error",
				__func__);
		return -EIO;
	}
	iotap = iotap_ps_data;
	for (i = 0; i < IOT_AP_NUM; i++) {
		wsvc_printf_dbg("R interop issues ap%d:  %x:%x:%x:%x:%x:%x",
				i + 1, iotap[i].bssid[0], iotap[i].bssid[1],
				iotap[i].bssid[2], iotap[i].bssid[3],
				iotap[i].bssid[4], iotap[i].bssid[5]);
	}

	return 0;
}

static int save_iot_ap_ps_to_file(void)
{
	int len, i;
	struct wlan_iot_ap_ps *iotap;

	len = save_iot_ap_file(IOTAP_PS_DATA_FILE, IOTAP_PS_FILE_LEN,
			       (char *)iotap_ps_data);
	if (len < 0) {
		wsvc_printf_err("interop issues ap: %s, save file error",
				__func__);
		return -EIO;
	}
	iotap = iotap_ps_data;
	for (i = 0; i < IOT_AP_NUM; i++) {
		wsvc_printf_dbg("W interop issues ap%d:  %x:%x:%x:%x:%x:%x",
				i + 1, iotap[i].bssid[0], iotap[i].bssid[1],
				iotap[i].bssid[2], iotap[i].bssid[3],
				iotap[i].bssid[4], iotap[i].bssid[5]);
	}

	return len;
}

static int
cnss_iot_ap_ps_update_list(struct wlan_iot_ap_ps *data)
{
	int i;

	if (!memcmp(data->bssid, iotap_ps_flag[1].bssid, IOT_AP_MAC_LEN)) {
		wsvc_printf_info("interop issues ap: driver trigger update");
		cnss_iot_ap_set_to_driver();
		return 0;
	}

	for (i = 0; i < IOT_AP_NUM; i++) {
		if (!memcmp(data->bssid, iotap_ps_data[i].bssid,
			    IOT_AP_MAC_LEN)) {
			wsvc_printf_dbg("interop issues ap: mac has added");
			return 0;
		} else if (!memcmp(iotap_ps_data[i].bssid,
				   iotap_ps_flag[0].bssid, IOT_AP_MAC_LEN)) {
			break;
		}
	}

	if (i < IOT_AP_NUM) {
		memcpy(iotap_ps_data[i].bssid, data->bssid, IOT_AP_MAC_LEN);
	} else {
		for (i = 1; i < IOT_AP_NUM; i++)
			memcpy(iotap_ps_data[i - 1].bssid,
			       iotap_ps_data[i].bssid, IOT_AP_MAC_LEN);
		memcpy(iotap_ps_data[i - 1].bssid, data->bssid,
		       IOT_AP_MAC_LEN);
	}

	wsvc_printf_dbg("interop issues ap: %s: the mac is added", __func__);
	save_iot_ap_ps_to_file();
	return 0;
}

int cnss_iot_event_handler(char *data, int len)
{
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_INTEROP_ISSUES_AP_MAX + 1];
	struct wlan_iot_ap_ps iotap;

	if (nla_parse(tb, QCA_WLAN_VENDOR_ATTR_INTEROP_ISSUES_AP_MAX,
		      (struct nlattr *)data, len, NULL))
		return -EINVAL;

	if (tb[QCA_WLAN_VENDOR_ATTR_INTEROP_ISSUES_AP_BSSID]) {
		nla_memcpy(iotap.bssid,
			   tb[QCA_WLAN_VENDOR_ATTR_INTEROP_ISSUES_AP_BSSID],
			   ETH_ALEN);
		wsvc_printf_info("interop issues ap: %x:%x:%x:%x:%x:%x",
				 iotap.bssid[0], iotap.bssid[1],
				 iotap.bssid[2], iotap.bssid[3],
				 iotap.bssid[4], iotap.bssid[5]);
		cnss_iot_ap_ps_update_list(&iotap);
	}

	return 0;
}

int cnss_iot_ap_set_to_driver(void)
{
	struct nl_msg *nlmsg;
	struct nlattr *params;
	struct nlattr *nlbssids, *attr;

	int ifindex = if_nametoindex("wlan0");
	void *msg_head = NULL;
	int i, count = 0;

	for (i = 0; i < IOT_AP_NUM; i++)
		if (memcmp(iotap_ps_data[i].bssid, iotap_ps_flag[0].bssid,
			   IOT_AP_MAC_LEN))
			count++;

	if (!count) {
		wsvc_printf_err("no interop issues ap currently\n");
		return -ENODEV;
	}

	nlmsg = nlmsg_alloc();
	if (NULL == nlmsg) {
		wsvc_printf_err("nl msg alloc failed\n");
		return -ENOMEM;
	}

	msg_head = genlmsg_put(nlmsg, 0, 0, cnss_nl80211_get_famid(),
			       0, 0, NL80211_CMD_VENDOR, 0);
	if (NULL == msg_head) {
		wsvc_printf_err("genlmsg put failed\n");
		goto fail;
	}

	if (nla_put_u32(nlmsg, NL80211_ATTR_IFINDEX, ifindex) ||
	    nla_put_u32(nlmsg, NL80211_ATTR_VENDOR_ID, OUI_QCA) ||
	    nla_put_u32(nlmsg, NL80211_ATTR_VENDOR_SUBCMD,
			QCA_NL80211_VENDOR_SUBCMD_INTEROP_ISSUES_AP)) {
		wsvc_printf_err("nlmsg failed for vendor subcmd\n");
		goto fail;
	}

	params = nla_nest_start(nlmsg, NL80211_ATTR_VENDOR_DATA);
	if (!params) {
		wsvc_printf_err("nlmsg failed for vendor data\n");
		goto fail;
	}

	nlbssids = nla_nest_start(nlmsg,
				  QCA_WLAN_VENDOR_ATTR_INTEROP_ISSUES_AP_LIST);
	if (NULL == nlbssids) {
		wsvc_printf_err("nla nest start failed\n");
		goto fail;
	}

	for (i = 0; i < count; i++) {
		attr = nla_nest_start(nlmsg, i);
		if (NULL == attr)
			goto fail;
		if (nla_put(nlmsg, QCA_WLAN_VENDOR_ATTR_INTEROP_ISSUES_AP_BSSID,
			    ETH_ALEN, iotap_ps_data[i].bssid)) {
			wsvc_printf_err("nla put failed for mac addr\n");
			goto fail;
		}
		nla_nest_end(nlmsg, attr);
	}
	nla_nest_end(nlmsg, nlbssids);
	nla_nest_end(nlmsg, params);

	if (cnss_nl80211_send_cmd_to_driver(nlmsg))
		goto fail;

	return 0;
fail:
	nlmsg_free(nlmsg);
	return -EINVAL;
}

/* init handler */
int cnss_iot_ap_init(void)
{
	int ret;

	read_iot_ap_ps_from_file();
	ret = cnss_nl80211_register_handler(cnss_iot_event_handler,
				QCA_NL80211_VENDOR_SUBCMD_INTEROP_ISSUES_AP);
	if (ret < 0) {
		wsvc_printf_err("unable to register event handler\n");
		return ret;
	}

	wsvc_printf_info("interop issues ap init successfully\n");
	return 0;
}

/* deinit handler */
int cnss_iot_ap_deinit(void)
{
	save_iot_ap_ps_to_file();
	cnss_nl80211_unregister_handler(
				QCA_NL80211_VENDOR_SUBCMD_INTEROP_ISSUES_AP);
	return 0;
}
