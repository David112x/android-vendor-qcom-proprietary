/*
 * Copyright (c) 2016-2017, 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a Contribution.
 * Apache license notifications and license are retained
 * for attribution purposes only.
 *
 *
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <android-base/logging.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <sys/socket.h>
#include <unistd.h>
#include <poll.h>
#include <sys/stat.h>
#include <pthread.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include "WigigSuppConnector.h"

#define EVENT_BUF_SIZE 2048

static const char SUPPLICANT_NAME[]     = "vendor.wigig_supplicant";
static const char SUPP_PROP_NAME[]      = "init.svc.vendor.wigig_supplicant";
static const char SUPP_CONFIG_TEMPLATE[]= "/vendor/etc/wifi/wigig_supplicant.conf";
static const char SUPP_CONFIG_FILE[]    = "/data/vendor/wifi/wigig_supplicant.conf";
static const char SUPP_CLI_SOCK_PATH[]  = "/data/vendor/wifi/wigig_sockets";
static const char SUPP_P2P_CONFIG_TEMPLATE[]= "/vendor/etc/wifi/wigig_p2p_supplicant.conf";
static const char SUPP_P2P_CONFIG_FILE[]    = "/data/vendor/wifi/wigig_p2p_supplicant.conf";
static const char SENSING_DAEMON_NAME[]     = "vendor.sensingdaemon";
static const char SENSING_DAEMON_PROP_NAME[] = "init.svc.vendor.sensingdaemon";


static const char IFNAME[]              = "IFNAME=";
#define IFNAMELEN                       (sizeof(IFNAME) - 1)
static const char WPA_EVENT_IGNORE[]    = "CTRL-EVENT-IGNORE ";

static const char SUPP_ENTROPY_FILE[]   = "/data/vendor/wifi/wigig_entropy.bin";
static unsigned char dummy_key[21]      = { 0x02, 0x11, 0xbe, 0x33, 0x43, 0x35,
                                            0x68, 0x47, 0x84, 0x99, 0xa9, 0x2b,
                                            0x1c, 0xd3, 0xee, 0xff, 0xf1, 0xe2,
                                            0xf3, 0xf4, 0xf5 };
#define WIGIG_DEFAULT_INTERFACE         "wigig0"

// for WigigHostapdConnector
static const char HOSTAPD_NAME[]         = "vendor.wigig_hostapd";
static const char HOSTAPD_PROP_NAME[]    = "init.svc.vendor.wigig_hostapd";
static const char HOSTAPD_CONF_FILE[]    = "/data/vendor/wifi/hostapd/wigig_hostapd.conf";
static const char HOSTAPD_CLI_SOCK_PATH[]  = "/data/vendor/wifi/wigig_hostapd";

#define WIGIG_AP_CHANNEL_DEFAULT        2
#define WIGIG_AP_MAX_NUM_STA_DEFAULT  8
#define PSK_STR_LEN                     (2*SHA256_DIGEST_LENGTH+1)

//============================
// wil6210 load/unload support
//============================
#ifndef WIGIG_DRIVER_MODULE_NAME
#define WIGIG_DRIVER_MODULE_NAME    "wil6210"
#endif
static const char DRIVER_MODULE_TAG[] = WIGIG_DRIVER_MODULE_NAME " ";
static const char MODULE_FILE[] = "/proc/modules";
static const char DRIVER_PROP_NAME[] = "vendor.wigig.driver";
static const char DRIVER_PROBED_PROP_NAME[] = "vendor.wigig.driver.probed";

static int is_wigig_driver_probed()
{
    char driver_probed[PROPERTY_VALUE_MAX];

    /*
     * the init process will set a property when it probed the driver.
     * This does not mean it was loaded successfully, only that it was
     * probed and _should_ be loaded.
     */
    if (!property_get(DRIVER_PROBED_PROP_NAME, driver_probed, NULL) ||
        strcmp(driver_probed, "1") != 0) {
        return 0; /* driver not loaded */
    }

    return 1;
}

static int is_wigig_driver_loaded()
{
    FILE *proc;
    char line[sizeof(DRIVER_MODULE_TAG) + 10];

    if ((proc = fopen(MODULE_FILE, "r")) == NULL) {
        LOG(WARNING) << "Could not open " << MODULE_FILE;
        return 0;
    }
    while ((fgets(line, sizeof(line), proc)) != NULL) {
        if (strncmp(line, DRIVER_MODULE_TAG, strlen(DRIVER_MODULE_TAG)) == 0) {
            fclose(proc);
            return 1;
        }
    }
    fclose(proc);
    return 0;
}


static int wigig_unload_driver()
{
    int count = 20; /* wait 10 seconds at most */

    if (!is_wigig_driver_probed()) {
        return 0;
    }
    usleep(200000); /* allow to finish interface down */

    /* set a property to signal init process to remove the driver */
    property_set(DRIVER_PROP_NAME, "off");
    sched_yield();

    while (count-- > 0) {
        if (!is_wigig_driver_probed())
            break;
        usleep(500000);
    }

    return !is_wigig_driver_loaded() ? 0 : -1;
}

static int wigig_load_driver(bool ipa_offload, bool sensing_over_spi)
{
    int count = 20; /* wait 10 seconds at most */
    char propValue[PROPERTY_VALUE_MAX] = "on";

    if (is_wigig_driver_probed()) {
        return 0;
    }

    LOG(INFO) << "loading driver, ipa_offload " << ipa_offload << " sensing_over_spi " << sensing_over_spi;

    if (ipa_offload) {
        strlcat(propValue, ",ipa", sizeof(propValue));
    }
    if (sensing_over_spi) {
        strlcat(propValue, ",sensing", sizeof(propValue));
    }

    /* set a property to signal init process to probe the driver */
    property_set(DRIVER_PROP_NAME, propValue);
    sched_yield();

    while (count-- > 0) {
        if (is_wigig_driver_probed())
            break;
        usleep(500000);
    }

    return is_wigig_driver_loaded() ? 0 : -1;
}

static int start_running(const char *name, const char *prop_name)
{
    char status[PROPERTY_VALUE_MAX] = {'\0'};
    int count = 200; /* wait at most 20 seconds for completion */
    const prop_info *pi;
    unsigned serial = 0;

    /*
     * Get a reference to the status property, so we can distinguish
     * the case where it goes stopped => running => stopped (i.e.,
     * it start up, but fails right away) from the case in which
     * it starts in the stopped state and never manages to start
     * running at all.
     */
    pi = __system_property_find(prop_name);
    if (pi != NULL)
        serial = __system_property_serial(pi);

    property_set("ctl.start", name);
    sched_yield();

    while (count-- > 0) {
        if (pi == NULL)
            pi = __system_property_find(prop_name);
        if (pi != NULL) {
            /*
            * property serial updated means that init process is scheduled
            * after we sched_yield, further property status checking is based on this */
            if (__system_property_serial(pi) != serial) {
                __system_property_read(pi, NULL, status);
                if (strcmp(status, "running") == 0) {
                    // give some extra time to complete initialization procedure
                    usleep(300000);
                    return 0;
                } else if (strcmp(status, "stopped") == 0)
                    return -1;
            }
        }
        usleep(100000);
    }

    return -1;
}

static int stop_running(const char *name, const char *prop_name)
{
    char status[PROPERTY_VALUE_MAX] = {'\0'};
    int count = 50; /* wait at most 5 seconds for completion */

    property_set("ctl.stop", name);
    sched_yield();

    while (count-- > 0) {
        if (property_get(prop_name, status, NULL)) {
            if (strcmp(status, "stopped") == 0)
                return 0;
        }
        usleep(100000);
    }

    return -1;
}

static int start_sensing_daemon()
{
    char sensing_status[PROPERTY_VALUE_MAX] = {'\0'};

    /* Check whether sensing daemon is already running */
    if (property_get(SENSING_DAEMON_PROP_NAME, sensing_status, NULL)
            && strcmp(sensing_status, "running") == 0)
        return 0;

    return start_running(SENSING_DAEMON_NAME, SENSING_DAEMON_PROP_NAME);
}

static int stop_sensing_daemon()
{
    char sensing_status[PROPERTY_VALUE_MAX] = {'\0'};

    /* Check whether sensing daemon is already stopped */
    if (property_get(SENSING_DAEMON_PROP_NAME, sensing_status, NULL)
            && strcmp(sensing_status, "stopped") == 0)
        return 0;

    return stop_running(SENSING_DAEMON_NAME, SENSING_DAEMON_PROP_NAME);
}

//=====================
// WigigSuppConnector
//=====================

WigigSuppConnector::WigigSuppConnector()
:ctrl_conn(NULL), monitor_conn(NULL), is_monitoring(false), mCallback(NULL),
driver_load_only(false), ipa_offload(false), sensing_over_spi(false), sensing_daemon_only(false)
{
    exit_sockets[0] = exit_sockets[1] = -1;
    memset(&mConfig, 0, sizeof(mConfig));
}

WigigSuppConnector::~WigigSuppConnector()
{
    stop_monitoring();
    close_sockets();
    LOG(DEBUG) << "WigigSuppConnector destroyed";
}

void WigigSuppConnector::init()
{
    strlcpy(mConfig.supp_entropy_file, SUPP_ENTROPY_FILE,
        sizeof(mConfig.supp_entropy_file));
    strlcpy(mConfig.supp_prop_name, SUPP_PROP_NAME,
        sizeof(mConfig.supp_prop_name));
    strlcpy(mConfig.supp_cli_sock_path, SUPP_CLI_SOCK_PATH,
        sizeof(mConfig.supp_cli_sock_path));
    strlcpy(mConfig.supplicant_name, SUPPLICANT_NAME,
        sizeof(mConfig.supplicant_name));
    strlcpy(mConfig.primary_iface, WIGIG_DEFAULT_INTERFACE,
        sizeof(mConfig.primary_iface));
    snprintf(mConfig.supp_socket_path, sizeof(mConfig.supp_socket_path),
        "@android:wigig/wpa_%s", mConfig.primary_iface);
    mConfig.receive_events = true;
}

int WigigSuppConnector::startSupplicant()
{
    int ret;

    if (sensing_daemon_only) {
        LOG(INFO) << "skip startSupplicant, only start sensing daemon";
        ret = start_sensing_daemon();
        if (ret) {
            LOG(ERROR) << "start_sensing_daemon failed: " << ret;
        }
        return ret;
    }

    ret = wigig_load_driver(ipa_offload, sensing_over_spi);
    if (ret) {
        LOG(ERROR) << "failed to load driver: " << ret;
        return ret;
    }

    if (driver_load_only) {
        LOG(INFO) << "skip startSupplicant, only loading driver";
        return 0;
    }

    ret = start_supplicant_internal();
    if (ret) {
        LOG(ERROR) << "start_supplicant_internal failed: " << ret;
        return ret;
    }

    ret = connect_to_supplicant();
    if (ret) {
        LOG(ERROR) << "connect_to_supplicant failed: " << ret;
        return ret;
    }

    if (mConfig.receive_events) {
        ret = start_monitoring();
        if (ret) {
            LOG(ERROR) << "start_monitoring failed: " << ret;
            return ret;
        }
    } else {
        LOG(INFO) << "receive_events is not set, skip monitoring";
    }

    LOG(INFO) << "startSupplicant success";
    return 0;
}

int WigigSuppConnector::killSupplicant()
{
    int ret;

    if (sensing_daemon_only) {
        LOG(INFO) << "skip killSupplicant, only kill sensing daemon";
        ret = stop_sensing_daemon();
        if (ret) {
            LOG(ERROR) << "stop_sensing_daemon failed: " << ret;
        }
        return ret;
    }

    if (!driver_load_only) {
        ret = stop_monitoring();
        if (ret) {
            LOG(ERROR) << "stop_monitoring failed: " << ret;
            /* continue */
        }

        close_sockets();

        ret = stop_supplicant_internal();
        if (ret) {
            LOG(ERROR) << "stop_supplicant_internal failed: " << ret;
            return ret;
        }
    }

    ret = wigig_unload_driver();
    if (ret) {
        LOG(ERROR) << "failed to unload driver: " << ret;
        return ret;
    }

    return 0;
}

int WigigSuppConnector::sendCommand(const char *cmd, char *reply, size_t *replylen)
{
    if (!replylen || *replylen == 0) {
        LOG(ERROR) << "invalid reply length";
        return -1;
    }

    (*replylen)--; // make sure we have room to add NUL termination
    int ret = send_command_internal(cmd, reply, replylen);
    if (ret) {
        LOG(ERROR) << "failed to send command: " << ret;
        return ret;
    }
    // strip off trailing newline
    if (*replylen > 0 && reply[*replylen - 1] == '\n')
        (*replylen)--;
    reply[*replylen] = '\0';
    return ret;
}

int WigigSuppConnector::registerCallback(ISuppConnectorCallback *callback)
{
    mCallback = callback;
    return 0;
}

int WigigSuppConnector::setup_config_file()
{
    if ((ensure_config_file_exists(SUPP_CONFIG_FILE, SUPP_CONFIG_TEMPLATE) < 0) ||
        (ensure_config_file_exists(SUPP_P2P_CONFIG_FILE, SUPP_P2P_CONFIG_TEMPLATE) < 0)) {
        return -1;
    }

    return 0;
}

int WigigSuppConnector::setConfigOption(const char *name, const char *value)
{
    if (!strcmp(name, "driver_load_only")) {
        driver_load_only = atoi(value) != 0;
    } else if (!strcmp(name, "wigig_interface")) {
        strlcpy(mConfig.primary_iface, value, sizeof(mConfig.primary_iface));
    } else if (!strcmp(name, "ipa_offload")) {
        ipa_offload = atoi(value) != 0;
    } else if (!strcmp(name, "sensing_over_spi")) {
        sensing_over_spi = atoi(value) != 0;
    } else if (!strcmp(name, "sensing_daemon_only")) {
        sensing_daemon_only = atoi(value) != 0;
    } else {
        LOG(ERROR) << "Unsupported config option: " << name;
        return -1;
    }

    return 0;
}

int WigigSuppConnector::ensure_config_file_exists(const char *config_file, const char *template_file)
{
    char buf[2048];
    int srcfd, destfd;
    int nread;
    int ret;

    ret = access(config_file, R_OK|W_OK);
    if ((ret == 0) || (errno == EACCES)) {
        if ((ret != 0) &&
            (chmod(config_file, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP) != 0)) {
            LOG(ERROR) << "Cannot set RW to " << config_file << ": " << strerror(errno);
            return -1;
        }
        return 0;
    }
    if (errno != ENOENT) {
        LOG(ERROR) << "Cannot access " << config_file << ": " << strerror(errno);
        return -1;
    }

    srcfd = TEMP_FAILURE_RETRY(open(template_file, O_RDONLY));
    if (srcfd < 0) {
        LOG(ERROR) << "Cannot open " << template_file << ": " << strerror(errno);
        return -1;
    }

    destfd = TEMP_FAILURE_RETRY(open(config_file, O_CREAT|O_RDWR, 0660));
    if (destfd < 0) {
        close(srcfd);
        LOG(ERROR) << "Cannot create " << config_file << ": " << strerror(errno);
        return -1;
    }

    while ((nread = TEMP_FAILURE_RETRY(read(srcfd, buf, sizeof(buf)))) != 0) {
        if (nread < 0) {
            LOG(ERROR) << "Error reading " << template_file << ": " << strerror(errno);
            close(srcfd);
            close(destfd);
            unlink(config_file);
            return -1;
        }
        TEMP_FAILURE_RETRY(write(destfd, buf, nread));
    }

    close(destfd);
    close(srcfd);

    /* chmod is needed because open() didn't set permisions properly */
    if (chmod(config_file, 0660) < 0) {
        LOG(ERROR) << "Error changing permissions of " << config_file << " to 0660: "
              << strerror(errno);
        unlink(config_file);
        return -1;
    }

#if 0
    if (chown(config_file, AID_SYSTEM, AID_WIFI) < 0) {
        ALOGE("Error changing group ownership of %s to %d: %s",
              config_file, AID_WIFI, strerror(errno));
        unlink(config_file);
        return -1;
    }
#endif

    return 0;
}

int WigigSuppConnector::ensure_entropy_file_exists()
{
    int ret;
    int destfd;

    if (mConfig.supp_entropy_file[0] == '\0') {
        LOG(DEBUG) << "skipping creation of entropy file";
        return 0;
    }

    ret = access(mConfig.supp_entropy_file, R_OK | W_OK);
    if ((ret == 0) || (errno == EACCES)) {
        if ((ret != 0) &&
            (chmod(mConfig.supp_entropy_file, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP) != 0)) {
            LOG(ERROR) << "Cannot set RW to " << mConfig.supp_entropy_file << ": " << strerror(errno);
            return -1;
        }
        return 0;
    }
    destfd = TEMP_FAILURE_RETRY(open(mConfig.supp_entropy_file, O_CREAT|O_RDWR, 0660));
    if (destfd < 0) {
        LOG(ERROR) << "Cannot create " << mConfig.supp_entropy_file << ": " << strerror(errno);
        return -1;
    }

    if (TEMP_FAILURE_RETRY(write(destfd, dummy_key, sizeof(dummy_key))) != sizeof(dummy_key)) {
        LOG(ERROR) << "Error writing " << mConfig.supp_entropy_file << ": " << strerror(errno);
        close(destfd);
        return -1;
    }
    close(destfd);

    /* chmod is needed because open() didn't set permisions properly */
    if (chmod(mConfig.supp_entropy_file, 0660) < 0) {
        LOG(ERROR) << "Error changing permissions of " << mConfig.supp_entropy_file << " to 0660: "
              << strerror(errno);
        unlink(mConfig.supp_entropy_file);
        return -1;
    }

#if 0
    if (chown(mConfig.supp_entropy_file, AID_SYSTEM, AID_WIFI) < 0) {
        ALOGE("Error changing group ownership of %s to %d: %s",
              mConfig.supp_entropy_file, AID_WIFI, strerror(errno));
        unlink(mConfig.supp_entropy_file);
        return -1;
    }
#endif

    return 0;
}

void WigigSuppConnector::wpa_cleanup_internal(const char *cli_path)
{
    DIR *dir;
    struct dirent entry;
    struct dirent *result;
    size_t dirnamelen;
    size_t maxcopy;
    char pathname[PATH_MAX];
    char *namep;

    if ((dir = opendir(cli_path)) == NULL)
        return;

    dirnamelen = (size_t) snprintf(pathname, sizeof(pathname), "%s/",
                                   cli_path);
    if (dirnamelen >= sizeof(pathname)) {
        closedir(dir);
        return;
    }
    namep = pathname + dirnamelen;
    maxcopy = PATH_MAX - dirnamelen;
    while (readdir_r(dir, &entry, &result) == 0 && result != NULL) {
        if (strlcpy(namep, entry.d_name, maxcopy) < maxcopy)
            unlink(pathname);
    }
    closedir(dir);
}

int WigigSuppConnector::start_supplicant_internal()
{
    char supp_status[PROPERTY_VALUE_MAX] = {'\0'};

    /* Check whether already running */
    if (property_get(mConfig.supp_prop_name, supp_status, NULL)
            && strcmp(supp_status, "running") == 0)
        return 0;

    /* Before starting the daemon, make sure its config file exists */
    if (setup_config_file() < 0) {
        LOG(ERROR) << "Wigig will not be enabled";
        return -1;
    }

    if (ensure_entropy_file_exists() < 0)
        LOG(ERROR) << "Wigig entropy file was not created";

    /* Clear out any stale socket files that might be left over. */
    wpa_cleanup_internal(mConfig.supp_cli_sock_path);

    /* Reset sockets used for exiting from hung state */
    exit_sockets[0] = exit_sockets[1] = -1;

    return start_running(mConfig.supplicant_name, mConfig.supp_prop_name);
}

int WigigSuppConnector::stop_supplicant_internal()
{
    char supp_status[PROPERTY_VALUE_MAX] = {'\0'};

    /* Check whether supplicant already stopped */
    if (property_get(mConfig.supp_prop_name, supp_status, NULL)
            && strcmp(supp_status, "stopped") == 0)
        return 0;

    return stop_running(mConfig.supplicant_name, mConfig.supp_prop_name);
}

int WigigSuppConnector::connect_on_socket_path(const char *path)
{
    char supp_status[PROPERTY_VALUE_MAX] = {'\0'};

    /* Make sure supplicant is running */
    if (!property_get(mConfig.supp_prop_name, supp_status, NULL)
            || strcmp(supp_status, "running")) {
        LOG(ERROR) << "Supplicant not running, cannot connect";
        return -1;
    }

    ctrl_conn = wpa_ctrl_open2(path, mConfig.supp_cli_sock_path);
    LOG(INFO) << "Trying to connect to path: " << path;
    if (ctrl_conn == NULL) {
        LOG(ERROR) << "Unable to open connection to supplicant on " << path
              << ": " << strerror(errno);
        return -1;
    }

    LOG(INFO) << "Opened connection to supplicant on " << path;

    if (mConfig.receive_events) {
        monitor_conn = wpa_ctrl_open2(path, mConfig.supp_cli_sock_path);
        if (monitor_conn == NULL) {
        wpa_ctrl_close(ctrl_conn);
            ctrl_conn = NULL;
            return -1;
        }
        if (wpa_ctrl_attach(monitor_conn) != 0) {
            wpa_ctrl_close(monitor_conn);
            wpa_ctrl_close(ctrl_conn);
            ctrl_conn = monitor_conn = NULL;
            return -1;
        }
    }

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, exit_sockets) == -1) {
        if (monitor_conn != NULL)
            wpa_ctrl_close(monitor_conn);
        wpa_ctrl_close(ctrl_conn);
        ctrl_conn = monitor_conn = NULL;
        return -1;
    }

    return 0;
}

/* Establishes the control and monitor socket connections on the interface */
int WigigSuppConnector::connect_to_supplicant()
{
    int retries = 5, ret;

    while (true) {
        ret = connect_on_socket_path(mConfig.supp_socket_path);

        if (!ret)
            break;

        retries--;
        if (retries <= 0) {
            break;
        } else {
            usleep(1000000);
            LOG(INFO) << "connect_to_supplicant: retrying, " << retries << " times left";
        }
    }

    return ret;
}

int WigigSuppConnector::send_command_internal(const char *cmd, char *reply, size_t *reply_len)
{
    int ret;

    if (ctrl_conn == NULL) {
        LOG(VERBOSE) << "Not connected to wpa_supplicant - " << cmd << " command dropped";
        return -1;
    }
    // NOTE: reply_len does not include null-termination
    ret = wpa_ctrl_request(ctrl_conn, cmd, strlen(cmd), reply, reply_len, NULL);
    if (ret == -2) {
        LOG(DEBUG) << "'" << cmd << "' command timed out.";
        /* unblocks the monitor receive socket for termination */
        TEMP_FAILURE_RETRY(write(exit_sockets[0], "T", 1));
        return -2;
    }
    else if (ret < 0 || strncmp(reply, "FAIL", 4) == 0)
        return -1;

    if (strncmp(cmd, "PING", 4) == 0)
        reply[*reply_len] = '\0';

    return 0;
}

int WigigSuppConnector::is_supplicant_connection_active()
{
    char supp_status[PROPERTY_VALUE_MAX] = {'\0'};

    if (property_get(mConfig.supp_prop_name, supp_status, NULL)) {
        if (strcmp(supp_status, "stopped") == 0)
            return -1;
    }

    return 0;
}

int WigigSuppConnector::wait_event_internal(char *reply, size_t *reply_len)
{
    int res;
    int ctrlfd = wpa_ctrl_get_fd(monitor_conn);
    struct pollfd rfds[2];

    memset(rfds, 0, 2 * sizeof(struct pollfd));
    rfds[0].fd = ctrlfd;
    rfds[0].events |= POLLIN;
    rfds[1].fd = exit_sockets[1];
    rfds[1].events |= POLLIN;

    do {
        res = TEMP_FAILURE_RETRY(poll(rfds, 2, 30000));
        if (res < 0) {
            LOG(ERROR) << "Error poll = " << res;
            return res;
        }
        else if (res == 0) {
            /* timed out, check if supplicant is active
             * or not ..
             */
            res = is_supplicant_connection_active();
            if (res < 0)
                return -2;
        }
    } while (res == 0);

    if (rfds[0].revents & POLLIN)
        return wpa_ctrl_recv(monitor_conn, reply, reply_len);

    /* it is not rfds[0], then it must be rfts[1] (i.e. the exit socket)
     * or we timed out. In either case, this call has failed ..
     */
    return -2;
}

int WigigSuppConnector::recv_from_supplicant_internal(char *buf, size_t buflen)
{
    size_t nread = buflen - 1;
    int result;
    char *match, *match2;

    if (monitor_conn == NULL)
        return snprintf(buf, buflen, "IFNAME=%s %s - connection closed",
                        mConfig.primary_iface, WPA_EVENT_TERMINATING);

    result = wait_event_internal(buf, &nread);

    /* Terminate reception on exit socket */
    if (result == -2) {
        is_monitoring = false;
        return snprintf(buf, buflen, "IFNAME=%s %s - connection closed",
                        mConfig.primary_iface, WPA_EVENT_TERMINATING);
    }

    if (result < 0) {
        LOG(DEBUG) << "wigig_ctrl_recv failed: " << strerror(errno);
        is_monitoring = false;
        return snprintf(buf, buflen, "IFNAME=%s %s - recv error",
                        mConfig.primary_iface, WPA_EVENT_TERMINATING);
    }
    buf[nread] = '\0';
    /* Check for EOF on the socket */
    if (result == 0 && nread == 0) {
        /* Fabricate an event to pass up */
        LOG(DEBUG) << "Received EOF on supplicant socket";
        is_monitoring = false;
        return snprintf(buf, buflen, "IFNAME=%s %s - signal 0 received",
                        mConfig.primary_iface, WPA_EVENT_TERMINATING);
    }
    /*
     * Events strings are in the format
     *
     *     IFNAME=iface <N>CTRL-EVENT-XXX
     *        or
     *     <N>CTRL-EVENT-XXX
     *
     * where N is the message level in numerical form (0=VERBOSE, 1=DEBUG,
     * etc.) and XXX is the event name. The level information is not useful
     * to us, so strip it off.
     */

    if (strncmp(buf, IFNAME, IFNAMELEN) == 0) {
        match = strchr(buf, ' ');
        if (match != NULL) {
            if (match[1] == '<') {
                match2 = strchr(match + 2, '>');
                if (match2 != NULL) {
                    nread -= (match2 - match);
                    memmove(match + 1, match2 + 1, nread - (match - buf) + 1);
                }
            }
        }
        else
            return snprintf(buf, buflen, "%s", WPA_EVENT_IGNORE);
    }
    else if (buf[0] == '<') {
        match = strchr(buf, '>');
        if (match != NULL) {
            nread -= (match + 1 - buf);
            memmove(buf, match + 1, nread + 1);
            LOG(VERBOSE) << "supplicant generated event without interface -" << buf;
        }
    }
    else
        /* let the event go as is! */
        LOG(WARNING) << "supplicant generated event without interface and without message level - " << buf;

    return nread;
}

void WigigSuppConnector::close_sockets()
{
    if (ctrl_conn != NULL) {
        wpa_ctrl_close(ctrl_conn);
        ctrl_conn = NULL;
    }

    if (monitor_conn != NULL) {
        wpa_ctrl_close(monitor_conn);
        monitor_conn = NULL;
    }

    if (exit_sockets[0] >= 0) {
        close(exit_sockets[0]);
        exit_sockets[0] = -1;
    }

    if (exit_sockets[1] >= 0) {
        close(exit_sockets[1]);
        exit_sockets[1] = -1;
    }
}

void WigigSuppConnector::close_supplicant_connection()
{
    char supp_status[PROPERTY_VALUE_MAX] = {'\0'};
    int count = 50; /* wait at most 5 seconds to ensure init has stopped stupplicant */

    close_sockets();

    while (count-- > 0) {
        if (property_get(mConfig.supp_prop_name, supp_status, NULL)) {
            if (strcmp(supp_status, "stopped") == 0)
                return;
        }
        usleep(100000);
    }
}

int WigigSuppConnector::start_monitoring()
{
    if (is_monitoring) {
        LOG(DEBUG) << "already monitoring";
        return 0;
    }

    is_monitoring = true;
    int ret = pthread_create(&monitor_thread, 0, do_monitor, this);
    if (ret) {
        LOG(ERROR) << "failed to create monitor thread: " << ret;
        is_monitoring = false;
        return ret;
    }

    LOG(INFO) << "monitor thread created";
    return 0;
}

int WigigSuppConnector::stop_monitoring()
{
    if (!is_monitoring) {
        LOG(DEBUG) << "not monitoring";
        return 0;
    }

    is_monitoring = false;
    TEMP_FAILURE_RETRY(write(exit_sockets[0], "T", 1));
    pthread_join(monitor_thread, NULL);
    return 0;
}

void *WigigSuppConnector::do_monitor(void *arg)
{
    char buf[EVENT_BUF_SIZE];
    WigigSuppConnector *inst = (WigigSuppConnector *)arg;

    LOG(INFO) << "entered do_monitor thread";

    while (inst->is_monitoring) {
        int nread = inst->recv_from_supplicant_internal(buf, sizeof(buf));
        if (nread > 0) {
            inst->notify_event(buf, nread);
        }
    }

    return 0;
}

void WigigSuppConnector::notify_event(char *buf, size_t buflen)
{
    ISuppConnectorCallback *callback = mCallback;

    if (callback)
        callback->onEvent(buf, buflen);
}

//=====================
// WigigHostapdConnector
//=====================
WigigHostapdConnector::WigigHostapdConnector()
{
    // nothing to do for now
}

WigigHostapdConnector::~WigigHostapdConnector()
{
    // nothing to do for now
}

void WigigHostapdConnector::init()
{
    strlcpy(mConfig.supp_prop_name, HOSTAPD_PROP_NAME,
        sizeof(mConfig.supp_prop_name));
    strlcpy(mConfig.supp_cli_sock_path, HOSTAPD_CLI_SOCK_PATH,
        sizeof(mConfig.supp_cli_sock_path));
    strlcpy(mConfig.supplicant_name, HOSTAPD_NAME,
        sizeof(mConfig.supplicant_name));
    strlcpy(mConfig.primary_iface, WIGIG_DEFAULT_INTERFACE,
            sizeof(mConfig.primary_iface));
    snprintf(mConfig.supp_socket_path, sizeof(mConfig.supp_socket_path),
        "%s/%s", mConfig.supp_cli_sock_path, mConfig.primary_iface);
    ifname[0] = ssid[0] = passphrase[0] = wowlan_triggers[0] = country_code[0] = '\0';
    channel = hidden = secured = max_num_sta = enable_edmg = edmg_channel = 0;
    mConfig.receive_events = true;
}

int WigigHostapdConnector::setConfigOption(const char *name, const char *value)
{
    if (!strcmp(name, "ifname")) {
        strlcpy(ifname, value, sizeof(ifname));
    } else if (!strcmp(name, "ssid")) {
        strlcpy(ssid, value, sizeof(ssid));
    } else if (!strcmp(name, "channel")) {
        // empty string will set channel to 0
        channel = atoi(value);
    } else if (!strcmp(name, "hidden")) {
        hidden = atoi(value);
    } else if (!strcmp(name, "secured")) {
        secured = atoi(value);
    } else if (!strcmp(name, "passphrase")) {
        strlcpy(passphrase, value, sizeof(passphrase));
    } else if (!strcmp(name, "wowlan_triggers")) {
        strlcpy(wowlan_triggers, value, sizeof(wowlan_triggers));
    } else if (!strcmp(name, "max_num_sta")) {
        // empty string will set max_num_sta to 0
        max_num_sta = atoi(value);
    } else if (!strcmp(name, "country_code")) {
        strlcpy(country_code, value, sizeof(country_code));
    } else if (!strcmp(name, "enable_edmg")) {
        enable_edmg = atoi(value);
    } else if (!strcmp(name, "edmg_channel")) {
        edmg_channel = atoi(value);
    } else {
        return WigigSuppConnector::setConfigOption(name, value);
    }

    return 0;
}

int WigigHostapdConnector::setup_config_file()
{
    char buf[512];
    int res = 0;

    FILE *f;
    f = fopen(HOSTAPD_CONF_FILE, "w");
    if (f == NULL) {
        LOG(ERROR) << __func__ << ": unable to open the file" << HOSTAPD_CONF_FILE;
        return -1;
    }

    if (channel <= 0) {
        channel = WIGIG_AP_CHANNEL_DEFAULT;
    }
    if (max_num_sta <= 0) {
        max_num_sta = WIGIG_AP_MAX_NUM_STA_DEFAULT;
    }

    res = snprintf(buf, sizeof(buf),
                   "interface=%s\n"
                   "ctrl_interface=%s\n"
                   "ssid=%s\n"
                   "channel=%d\n"
                   "hw_mode=ad\n"
                   "ignore_broadcast_ssid=%d\n"
                   "wowlan_triggers=%s\n" /* an empty value is allowed; means no triggers are defined */
                   "max_num_sta=%d\n",
                   ifname, mConfig.supp_cli_sock_path, ssid, channel, hidden,
                   wowlan_triggers, max_num_sta);
    if (res < 0) {
        LOG(ERROR) << "fail to create hostapd conf file";
        goto out;
    }

    res = fprintf(f, "%s", buf);
    if (res < 0) {
        LOG(ERROR) << "fail to write to hostapd conf file";
        goto out;
    }

    if (enable_edmg) {
        res = snprintf(buf, sizeof(buf),
                       "enable_edmg=1\n"
                       "edmg_channel=%d\n", edmg_channel);
        if (res < 0) {
            LOG(ERROR) << "fail to create EDMG config";
            goto out;
        }

        res = fprintf(f, "%s", buf);
        if (res < 0) {
            LOG(ERROR) << "fail to write EDMG config to hostapd conf file";
            goto out;
        }
    }

    if (country_code[0] != '\0') {
        res = snprintf(buf, sizeof(buf), "country_code=%s\n", country_code);
        if (res < 0) {
            LOG(ERROR) << "fail to create country_code config";
            goto out;
        }

        res = fprintf(f, "%s", buf);
        if (res < 0) {
            LOG(ERROR) << "fail to write country_code to hostapd conf file";
            goto out;
        }
    }

    if (secured) {
        char psk_str[PSK_STR_LEN];

        if (generate_psk(ssid, passphrase, psk_str, sizeof(psk_str))) {
            LOG(ERROR) << "fail to create PSK string";
            res = -1;
            goto out;
        }

        res = snprintf(buf, sizeof(buf),
                       "wpa=2\n"
                       "rsn_pairwise=GCMP\n"
                       "wpa_psk=%s\n", psk_str);
        if (res < 0) {
            LOG(ERROR) << "fail to create secured config";
            goto out;
        }

        res = fprintf(f, "%s", buf);
        if (res < 0) {
            LOG(ERROR) << "fail to write secured config to hostapd conf file";
            goto out;
        }
    }

    /* chmod is needed because open() didn't set permissions properly */
    if (chmod(HOSTAPD_CONF_FILE, 0660) < 0) {
        LOG(ERROR) << "Error changing permissions of " << HOSTAPD_CONF_FILE <<
                      "to 0660: " << strerror(errno);
        unlink(HOSTAPD_CONF_FILE);
        res = -1;
        goto out;
    }

#if 0
    if (chown(HOSTAPD_CONF_FILE, AID_SYSTEM, AID_WIFI) < 0) {
        LOG(ERROR) << "Error changing group ownership of " << HOSTAPD_CONF_FILE <<
                      " to " << AID_WIFI << ": " << strerror(errno);
            unlink(HOSTAPD_CONF_FILE);
            res = -1;
            goto out;
    }
#endif

out:
    fclose(f);
    return ((res < 0) ? res : 0);
}

int WigigHostapdConnector::generate_psk(const char *ssid, const char *passphrase,
                                        char *psk_str, size_t psk_str_len)
{
    unsigned char psk[SHA256_DIGEST_LENGTH];
    int j;

    if (psk_str_len < PSK_STR_LEN) {
        LOG(ERROR) << "psk_str_len is too short (" << psk_str_len << ")";
        return -1;
    }

    // Use the PKCS#5 PBKDF2 with 4096 iterations
    if (PKCS5_PBKDF2_HMAC_SHA1(passphrase, strlen(passphrase),
                               (const uint8_t *)ssid, strlen(ssid), 4096, SHA256_DIGEST_LENGTH,
                               psk) != 1) {
        LOG(ERROR) << "Cannot generate PSK using PKCS#5 PBKDF2";
        return -1;
    }

    for (j=0; j < SHA256_DIGEST_LENGTH; j++) {
        snprintf(&psk_str[j*2], psk_str_len - j*2, "%02x", psk[j]);
    }

    return 0;
}

