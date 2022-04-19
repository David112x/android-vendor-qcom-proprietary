/*
 * This file contains the implementation of LPMA (Low Power Mic Access) feature
 * functionality which provides WDSP buffers data access to non-SVA clients.
 * SLPI (Sensors Low Power Island) is one such non-SVA client.
 *
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#define LOG_TAG "sound_trigger_hw_lpma"
/* #define LOG_NDEBUG 0 */

#include <stdlib.h>
#include <dlfcn.h>
#include <errno.h>
#include <cutils/log.h>
#include <linux/wcd-spi-ac-params.h>
#include "gcs_api.h"
#include "sound_trigger_hw.h"
#include "sound_trigger_platform.h"
#include "st_common_defs.h"
#include "st_hw_session_gcs.h"
#include "st_hw_extn.h"
#include "st_graphite_api.h"

/*
 * Notes:
 * 1. Open ac (access control) driver only if usecase is started.
 *    Keeping it opened from bootup is unncessary overhead of ac driver,
 *    if usecase never gets started.
 * 2. During audio concurrency or device switch either the graph is closed
 *    or device backend is disconnected, which means no data flow in WDSP.
 *    Inform SLPI of concurrency disabled/enabled so that it can stop/start
 *    reading the buffers. For SSR cases driver already knows to inform the
 *    SLPI.
 */

#define SPI_AC_DEV_NODE "/dev/wcd-spi-ac-client"
#define GCS_LIB "libgcs.so"

enum lpma_state {
    LPMA_NOINIT,
    LPMA_IDLE,
    LPMA_SETUP,
    LPMA_ACTIVE
};

enum conc_event_type {
    CLIENT_STARTED = 0x01,
    TRANSIT_TO_APE = 0x02,
    WDSP_OFFLINE = 0x04,
    SLPI_OFFLINE = 0x80
};

/* gcs functions loaded from dynamic library */
static int32_t(*gcs_open_fn)(uint32_t UID,
    uint32_t DID, uint32_t *graph_handle);
static int32_t(*gcs_enable_fn)(uint32_t graph_handle,
    void *non_persist_ucal,
    uint32_t size_ucal);
static int32_t(*gcs_disable_fn)(uint32_t graph_handle);
static int32_t(*gcs_close_fn)(uint32_t graph_handle);
static int32_t(*gcs_enable_device_fn)(uint32_t graph_handle,
    uint32_t UID, int8_t *payload, uint32_t payload_size);
static int32_t(*gcs_disable_device_fn)(uint32_t graph_handle);
static int32_t (*gcs_set_config_fn)(uint32_t graph_handle, void *payload,
    uint32_t payload_size);
static int32_t (*gcs_get_config_fn)(uint32_t graph_handle, void *payload,
    uint32_t payload_size, struct gcs_config_rsp *response);

typedef struct {
    struct sound_trigger_device *stdev;
    struct st_lpma_config *lpma_cfg;
    enum lpma_state state;
    enum conc_event_type conc_event;
    void *gcs_lib_handle;
    uint32_t graph_handle;
    struct wcd_spi_ac_write_cmd *ac_cmd;
    int32_t ac_fd;
    uint32_t circbuf_addr[MAX_LPMA_BB_IDS * MAX_BRDIGE_BUF_PORTS];
    st_device_t st_device;
    char *st_device_name;
    pthread_mutex_t lock;
} sthw_extn_lpma_data_t;

static sthw_extn_lpma_data_t lpma_data = {
    .stdev = NULL,
    .lpma_cfg = NULL,
    .state = LPMA_NOINIT,
    .conc_event = 0,
    .gcs_lib_handle = NULL,
    .graph_handle = 0,
    .ac_cmd = NULL,
    .ac_fd = -1,
    .circbuf_addr = {0},
    .st_device = ST_DEVICE_NONE,
    .st_device_name = NULL,
    .lock = PTHREAD_MUTEX_INITIALIZER,
};

static int lpma_get_bridge_bufs_addr()
{
    struct graphite_cal_header  req_cfg[MAX_LPMA_BB_IDS];
    struct gcs_bridge_buf_params rsp_buf[MAX_LPMA_BB_IDS];
    struct gcs_config_rsp gcs_rsp;
    int status = 0, size = 0;
    unsigned int i = 0;

    /*
     * Request bridge buffers address info. gcs_bridge_buf_params payload is
     * already 4 byte aligned by definition.
     */
    for (i = 0; i < lpma_data.lpma_cfg->num_bb_ids; i++) {
        req_cfg[i].module_id = lpma_data.lpma_cfg->bb_params[i].module_id;
        req_cfg[i].instance_id = lpma_data.lpma_cfg->bb_params[i].instance_id;
        req_cfg[i].param_id = lpma_data.lpma_cfg->bb_params[i].param_id;
        req_cfg[i].reserved =  0;
        req_cfg[i].size = sizeof(struct gcs_bridge_buf_payload);
    }
    size = lpma_data.lpma_cfg->num_bb_ids * sizeof(struct graphite_cal_header);

    gcs_rsp.rsp_buf = rsp_buf;
    gcs_rsp.rsp_size_requested = lpma_data.lpma_cfg->num_bb_ids *
                                 sizeof(struct gcs_bridge_buf_params);
    ALOGD("%s: gcs_get_config req_pld_size %d, rsp_size_requested %d",
          __func__, size, gcs_rsp.rsp_size_requested) ;
    status = gcs_get_config_fn(lpma_data.graph_handle, &req_cfg, size, &gcs_rsp);
    if (status) {
        ALOGE("%s: gcs_get_config failed status %d", __func__, status);
        return status;
    }
    if (gcs_rsp.rsp_size_returned != gcs_rsp.rsp_size_requested) {
        ALOGE("%s: gcs_get_config returned[%d] != requested[%d]", __func__,
              gcs_rsp.rsp_size_returned, gcs_rsp.rsp_size_requested);
        return -EINVAL;
    }
    /* Extract and store the buffer addresses from payload received from gcs */
    for (i = 0; i < lpma_data.lpma_cfg->num_bb_ids; i++) {
        memcpy (&lpma_data.circbuf_addr[i * MAX_BRDIGE_BUF_PORTS],
                rsp_buf[i].payload.addr_circbuf_info,
                sizeof(rsp_buf[i].payload.addr_circbuf_info));
    }
    return 0;
}

static int lpma_send_bridge_bufs_addr()
{
    int size = 0;

    size = lpma_data.lpma_cfg->num_bb_ids *
           MAX_BRDIGE_BUF_PORTS * sizeof(uint32_t);
    memcpy(lpma_data.ac_cmd->payload, lpma_data.circbuf_addr, size);

    lpma_data.ac_cmd->cmd_type = WCD_SPI_AC_CMD_BUF_DATA;
    size += sizeof(struct wcd_spi_ac_write_cmd);

    ALOGD("%s: write size %d", __func__, size);
    if (write(lpma_data.ac_fd, lpma_data.ac_cmd, size) != size) {
        ALOGE("%s: ac_cmd_circ_buf_data failed, %s, size %d", __func__,
              strerror(errno), size);
        return -errno;
    }
    return 0;
}

static int lpma_set_device(bool enable)
{
    char st_device_name[DEVICE_NAME_MAX_SIZE] = { 0 };
    int ref_cnt_idx = 0, ref_cnt = 0, status = 0;
    st_device_t st_device;
    audio_devices_t capture_device;

    if (enable) {
        capture_device = platform_stdev_get_capture_device(
                            lpma_data.stdev->platform);
        st_device = platform_stdev_get_device(lpma_data.stdev->platform,
                        NULL, capture_device, ST_EXEC_MODE_CPE);

        if (platform_stdev_get_device_name(lpma_data.stdev->platform,
                ST_EXEC_MODE_CPE, st_device, st_device_name) < 0) {
            ALOGE("%s: Invalid sound trigger device returned", __func__);
            return -EINVAL;
        }
        pthread_mutex_lock(&lpma_data.stdev->ref_cnt_lock);
        ref_cnt_idx = (ST_EXEC_MODE_CPE * ST_DEVICE_MAX) + st_device;
        ref_cnt = ++(lpma_data.stdev->dev_ref_cnt[ref_cnt_idx]);
        if (1 == ref_cnt) {
            status = platform_stdev_send_calibration(lpma_data.stdev->platform,
                capture_device, ST_EXEC_MODE_CPE, NULL,
                ACDB_LSM_APP_TYPE_NO_TOPOLOGY, false, ST_DEVICE_CAL);

            if (!status) {
                ALOGD("%s: enable device (%x) = %s", __func__, st_device,
                      st_device_name);
                audio_route_apply_and_update_path(lpma_data.stdev->audio_route,
                                                  st_device_name);
                lpma_data.stdev->capture_device = capture_device;
            } else {
                ALOGE("%s: failed to send calibration %d", __func__, status);
                --(lpma_data.stdev->dev_ref_cnt[ref_cnt_idx]);
            }
        }
        pthread_mutex_unlock(&lpma_data.stdev->ref_cnt_lock);
        lpma_data.st_device = st_device;
        lpma_data.st_device_name = strdup(st_device_name);
    } else {
        if (!lpma_data.st_device_name) {
            ALOGE("%s: Invalid sound trigger device name", __func__);
            return -EINVAL;
        }

        ref_cnt_idx = (ST_EXEC_MODE_CPE * ST_DEVICE_MAX) + lpma_data.st_device;
        pthread_mutex_lock(&lpma_data.stdev->ref_cnt_lock);
        ref_cnt = lpma_data.stdev->dev_ref_cnt[ref_cnt_idx];
        if (0 < ref_cnt) {
            ref_cnt = --(lpma_data.stdev->dev_ref_cnt[ref_cnt_idx]);
        } else {
            ALOGV("%s: ref_cnt = %d", __func__, ref_cnt);
            pthread_mutex_unlock(&lpma_data.stdev->ref_cnt_lock);
            return status;
        }

        if (0 == ref_cnt) {
            ALOGD("%s: disable device (%x) = %s", __func__, lpma_data.st_device,
                  lpma_data.st_device_name);
            audio_route_reset_and_update_path(lpma_data.stdev->audio_route,
                                              lpma_data.st_device_name);
        }
        pthread_mutex_unlock(&lpma_data.stdev->ref_cnt_lock);
        free(lpma_data.st_device_name);
    }
    return status;
}

static int lpma_setup()
{
    int status = 0, acdb_id = 0;
    st_device_t st_device;
    audio_devices_t capture_device;

    ALOGV("%s: enter ", __func__);
    if (lpma_data.state != LPMA_IDLE)
        return 0;

    status = st_hw_gcs_load_wdsp(true);
    if (status) {
        ALOGE("%s: wdsp image load failed %d", __func__, status);
        return status;
    }

    lpma_data.ac_cmd = calloc(1, sizeof(struct wcd_spi_ac_write_cmd) +
                                  sizeof(lpma_data.circbuf_addr));
    if (!lpma_data.ac_cmd) {
        ALOGE("%s: ac_cmd allocation failed", __func__);
        status = -ENOMEM;
        goto cleanup;
    }

    capture_device = platform_stdev_get_capture_device(
                        lpma_data.stdev->platform);
    st_device = platform_stdev_get_device(lpma_data.stdev->platform,
                    NULL, capture_device, ST_EXEC_MODE_CPE);

    acdb_id = platform_stdev_get_acdb_id(st_device, ST_EXEC_MODE_CPE);
    if (0 > acdb_id) {
        ALOGE("%s: Could not get ACDB ID for device %d", __func__,
              st_device);
        status = -EINVAL;
        goto cleanup;
    }

    ALOGD("%s: gcs_open with uid %d, did %d", __func__,
          lpma_data.lpma_cfg->uid, acdb_id);
    status = gcs_open_fn(lpma_data.lpma_cfg->uid, acdb_id,
                         &lpma_data.graph_handle);
    if (status) {
        ALOGE("%s: gcs_open failed status %d", __func__, status);
        goto cleanup;
    }
    lpma_data.state = LPMA_SETUP;
    ALOGV("%s: exit ", __func__);
    return 0;

cleanup:
    if (lpma_data.ac_cmd) {
        free(lpma_data.ac_cmd);
        lpma_data.ac_cmd = NULL;
    }
    st_hw_gcs_load_wdsp(false);
    ALOGV("%s: exit status %d", __func__, status);
    return status;
}

static int lpma_teardown()
{
    int status = 0;

    ALOGV("%s: enter", __func__);
    if (lpma_data.state != LPMA_SETUP)
        return 0;

    ALOGD("%s: gcs_close with handle %d", __func__, lpma_data.graph_handle);
    status = gcs_close_fn(lpma_data.graph_handle);
    if (status)
        ALOGE("%s: gcs_close failed status %d", __func__, status);

    if (lpma_data.ac_cmd) {
        free(lpma_data.ac_cmd);
        lpma_data.ac_cmd = NULL;
    }

    status = st_hw_gcs_load_wdsp(false);
    if (status)
        ALOGE("%s: wdsp image unload failed %d", __func__, status);

    lpma_data.state = LPMA_IDLE;
    ALOGV("%s: exit status %d", __func__, status);
    return status;
}

static int lpma_start(bool conc)
{
    int status = 0, size = 0;
    char ac_dev_node[50];

    ALOGV("%s: enter ", __func__);
    if (lpma_data.state != LPMA_SETUP)
        return 0;

    status = lpma_set_device(true);
    if (status)
        return status;

    ALOGD("%s: gcs_enable with handle %d", __func__, lpma_data.graph_handle);
    status = gcs_enable_fn(lpma_data.graph_handle, NULL, 0);
    if (status) {
        ALOGE("%s: gcs_enable failed status %d", __func__, status);
        goto cleanup1;
    }

    snprintf(ac_dev_node, sizeof(ac_dev_node), "/%s/%s",
             "dev", WCD_SPI_AC_CLIENT_CDEV_NAME);
    lpma_data.ac_fd = open(ac_dev_node, O_RDWR);
    if (lpma_data.ac_fd < 0) {
        ALOGE("%s: %s ", __func__, strerror(errno));
        status = -errno;
        goto cleanup2;
    }

    /* query buffer addresses from WDSP and send to access control driver */
    status = lpma_get_bridge_bufs_addr();
    if (!status)
        status = lpma_send_bridge_bufs_addr();

    if (status)
        goto cleanup2;

    if (conc) {
        /* disable SLPI SPI access */
        lpma_data.ac_cmd->cmd_type = WCD_SPI_AC_CMD_CONC_END;
        size = sizeof(struct wcd_spi_ac_write_cmd);
        if (write(lpma_data.ac_fd, lpma_data.ac_cmd, size) != size) {
            ALOGE("%s: ac_cmd_conc_begin failed %s", __func__,
                  strerror(errno));
            status = -errno;
            goto cleanup2;
        }
    }

    lpma_data.state = LPMA_ACTIVE;
    ALOGV("%s: exit ", __func__);
    return 0;

cleanup2:
    if (lpma_data.ac_fd >= 0)
        close(lpma_data.ac_fd);
    gcs_disable_fn(lpma_data.graph_handle);
cleanup1:
    lpma_set_device(false);
    ALOGV("%s: exit status %d", __func__, status);
    return status;
}

static int lpma_stop(bool conc)
{
    int status = 0, rc = 0, size = 0;

    ALOGV("%s: enter", __func__);
    if (lpma_data.state != LPMA_ACTIVE)
        return 0;

    if (conc) {
        /* disable SLPI SPI access */
        lpma_data.ac_cmd->cmd_type = WCD_SPI_AC_CMD_CONC_BEGIN;
        size = sizeof(struct wcd_spi_ac_write_cmd);
        if (write(lpma_data.ac_fd, lpma_data.ac_cmd, size) != size) {
            ALOGE("%s: ac_cmd_conc_begin failed %s", __func__,
                  strerror(errno));
            return -errno;
        }
    }
    /* Must be closed before DSP graph close, as part of gcs_disable, to
     * disable SPI access to SLPI */
    close(lpma_data.ac_fd);

    ALOGD("%s: gcs_disable with handle %d", __func__, lpma_data.graph_handle);
    status = gcs_disable_fn(lpma_data.graph_handle);
    if (status) {
        ALOGE("%s: gcs_disable failed status %d", __func__, status);
        rc = status;
    }

    status = lpma_set_device(false);
    if (status)
        rc = status;

    lpma_data.state = LPMA_SETUP;
    ALOGV("%s: exit status %d", __func__, rc);
    return rc;
}

static int lpma_handle_audio_concurrency()
{
    int status = 0;

    ALOGV("%s: enter", __func__);
    /* Usecases:
     * UC1: lpma is not yet active -> concurrency is active: ignore
     * UC2: lpma is active -> concurrency is active: stop lpma
     * UC3: lpma is already stopped -> another concurrency: ignore
     */
    if (lpma_data.state == LPMA_IDLE)
        return 0;

    if ((lpma_data.state == LPMA_ACTIVE) &&
        (lpma_data.stdev->tx_concurrency_active == 1))
        status = lpma_stop(true);
    else if ((lpma_data.state != LPMA_ACTIVE) &&
             (lpma_data.stdev->tx_concurrency_active == 0))
        status = lpma_start(true);

    ALOGV("%s: exit status %d", __func__, status);
    return status;
}

static int lpma_enable_device()
{
    int status = 0, acdb_id = 0, size = 0;

    ALOGV("%s: enter", __func__);
    /*
     * Device switch can initiate disable and enable device when lpma is not
     * yet active due to usecase not started or audio concurrency. Ignore it.
     * Note that we don't change state as disable/enable are internal
     * sequential calls.
     */
    if (lpma_data.state != LPMA_ACTIVE)
        return 0;

    status = lpma_set_device(true);
    if (status)
        return status;

    acdb_id = platform_stdev_get_acdb_id(lpma_data.st_device, ST_EXEC_MODE_CPE);
    if (0 > acdb_id) {
        ALOGE("%s: Could not get ACDB ID for device %d", __func__,
              lpma_data.st_device);
        status = -EINVAL;
        goto cleanup1;
    }

    ALOGD("%s: gcs_enable_device with handle %d, acdb_id %d",
          __func__, lpma_data.graph_handle, acdb_id);
    status = gcs_enable_device_fn(lpma_data.graph_handle, acdb_id, NULL, 0);
    if (status) {
        ALOGE("%s: gcs_enable_device failed status %d", __func__, status);
        goto cleanup1;
    }

    /* enable SLPI SPI access */
    lpma_data.ac_cmd->cmd_type = WCD_SPI_AC_CMD_CONC_END;
    size = sizeof(struct wcd_spi_ac_write_cmd);
    if (write(lpma_data.ac_fd, lpma_data.ac_cmd, size) != size) {
        ALOGE("%s: ac_cmd_conc_end failed %s", __func__,
              strerror(errno));
        status = -errno;
        goto cleanup2;
    }
    ALOGV("%s: exit", __func__);
    return 0;

cleanup2:
    gcs_disable_fn(lpma_data.graph_handle);
cleanup1:
    lpma_set_device(false);
    ALOGV("%s: exit status %d", __func__, status);
    return status;
}

static int lpma_disable_device()
{
    int status = 0, rc = 0, size = 0;

    ALOGV("%s: enter", __func__);
    /*
     * Device switch can initiate disable and enable device when lpma is not
     * yet active due to usecase not started or audio concurrency. Ignore it.
     * Note that we don't change state as disable/enable are internal
     * sequential calls.
     */
    if (lpma_data.state != LPMA_ACTIVE)
        return 0;

    /* disable SLPI SPI access */
    lpma_data.ac_cmd->cmd_type = WCD_SPI_AC_CMD_CONC_BEGIN;
    size = sizeof(struct wcd_spi_ac_write_cmd);
    if (write(lpma_data.ac_fd, lpma_data.ac_cmd, size) != size) {
        ALOGE("%s: ac_cmd_conc_begin failed %s", __func__,
              strerror(errno));
        return -errno;
    }

    ALOGD("%s: gcs_disable_device with handle %d", __func__,
          lpma_data.graph_handle);
    rc = gcs_disable_device_fn(lpma_data.graph_handle);
    if (rc) {
        ALOGE("%s: gcs_disable_device failed status %d", __func__, status);
        status = rc;
    }

    rc = lpma_set_device(false);
    if (rc)
        status = rc;

    ALOGV("%s: exit status %d", __func__, status);
    return status;
}

int sthw_extn_lpma_notify_event(enum sthw_extn_lpma_event_type event)
{
    int status = 0, rc = 0;

    ALOGV("%s: enter", __func__);
    if (lpma_data.state == LPMA_NOINIT)
        return -EINVAL;

    pthread_mutex_lock(&lpma_data.lock);
    switch (event) {
    case LPMA_EVENT_START:
        if (!(lpma_data.conc_event & (WDSP_OFFLINE | TRANSIT_TO_APE))) {
            status  = lpma_setup();
            if (!status && !lpma_data.stdev->tx_concurrency_active)
                status = lpma_start(false);
        }
        if (!status)
            lpma_data.conc_event |= CLIENT_STARTED;
        break;

    case LPMA_EVENT_CPE_STATUS_ONLINE:
    case LPMA_EVENT_TRANSIT_APE_TO_CPE:
        if (lpma_data.conc_event & CLIENT_STARTED) {
            status  = lpma_setup();
            if (!status && !lpma_data.stdev->tx_concurrency_active)
                status = lpma_start(event == LPMA_EVENT_TRANSIT_APE_TO_CPE ?
                                    true : false);
        }
        if (event == LPMA_EVENT_CPE_STATUS_ONLINE)
            lpma_data.conc_event &= ~WDSP_OFFLINE;
        else
            lpma_data.conc_event &= ~TRANSIT_TO_APE;

        break;

    case LPMA_EVENT_STOP:
    case LPMA_EVENT_CPE_STATUS_OFFLINE:
    case LPMA_EVENT_TRANSIT_CPE_TO_APE:
        rc = lpma_stop((event == LPMA_EVENT_TRANSIT_CPE_TO_APE) ? true : false);
        if (rc)
            status = rc;
        rc = lpma_teardown();
        if (rc)
            status = rc;

        if (event == LPMA_EVENT_STOP)
            lpma_data.conc_event &= ~CLIENT_STARTED;
        else if (event == LPMA_EVENT_CPE_STATUS_OFFLINE)
            lpma_data.conc_event |= WDSP_OFFLINE;
        else
            lpma_data.conc_event |= TRANSIT_TO_APE;
        break;

    case LPMA_EVENT_AUDIO_CONCURRENCY:
        status = lpma_handle_audio_concurrency();
        break;

    case LPMA_EVENT_ENABLE_DEVICE:
        status = lpma_disable_device();
        break;

    case LPMA_EVENT_DISABLE_DEVICE:
        status = lpma_enable_device();
        break;

    case LPMA_EVENT_SLPI_STATUS_OFFLINE:
        lpma_data.conc_event |= SLPI_OFFLINE;
        break;

    case LPMA_EVENT_SLPI_STATUS_ONLINE:
        /* resend bridge buffer addresses to access control driver */
        if (lpma_data.state == LPMA_ACTIVE)
            status = lpma_send_bridge_bufs_addr();
        lpma_data.conc_event &= ~SLPI_OFFLINE;
        break;

    default:
        ALOGW("%s: unhandled event %d", __func__, event);
        status = -EINVAL;
    }
    pthread_mutex_unlock(&lpma_data.lock);
    ALOGV("%s: exist status %d", __func__, status);
    return status;
}

int sthw_extn_lpma_init(struct sound_trigger_device *stdev)
{
    int status = 0;

    ALOGV("%s: enter", __func__);
    lpma_data.gcs_lib_handle = dlopen(GCS_LIB, RTLD_NOW);
    if (!lpma_data.gcs_lib_handle) {
        ALOGE("%s: %s", __func__, dlerror());
        return -ENOENT;
    }

    DLSYM(lpma_data.gcs_lib_handle, gcs_open_fn, gcs_open, status);
    if (status)
        goto cleanup;
    DLSYM(lpma_data.gcs_lib_handle, gcs_close_fn, gcs_close, status);
    if (status)
        goto cleanup;
    DLSYM(lpma_data.gcs_lib_handle, gcs_enable_fn, gcs_enable, status);
    if (status)
        goto cleanup;
    DLSYM(lpma_data.gcs_lib_handle, gcs_disable_fn, gcs_disable, status);
    if (status)
        goto cleanup;
    DLSYM(lpma_data.gcs_lib_handle, gcs_enable_device_fn, gcs_enable_device,
          status);
    if (status)
        goto cleanup;
    DLSYM(lpma_data.gcs_lib_handle, gcs_disable_device_fn,
          gcs_disable_device, status);
    if (status)
        goto cleanup;
    DLSYM(lpma_data.gcs_lib_handle, gcs_set_config_fn, gcs_set_config,
          status);
    if (status)
        goto cleanup;
    DLSYM(lpma_data.gcs_lib_handle, gcs_get_config_fn, gcs_get_config,
          status);
    if (status)
        goto cleanup;
    /*
     * No need to gcs_init() as it gets initialized during boot up from
     * platform source
     */

    /* Get lpma graph ids from platform config */
    platform_stdev_get_lpma_config(stdev->platform, &lpma_data.lpma_cfg);
    if (!lpma_data.lpma_cfg) {
        ALOGE("%s: lpma graph platform info not present", __func__);
        goto cleanup;
    }

    lpma_data.stdev = stdev;
    lpma_data.state = LPMA_IDLE;
    lpma_data.conc_event = 0;
    ALOGD("%s: exit", __func__);
    return 0;

cleanup:
    dlclose(lpma_data.gcs_lib_handle);
    lpma_data.gcs_lib_handle = NULL;
    ALOGV("%s: exit status %d", __func__, status);
    return status;
}

void sthw_extn_lpma_deinit()
{
    ALOGV("%s: enter", __func__);
    if (lpma_data.gcs_lib_handle) {
        dlclose(lpma_data.gcs_lib_handle);
        lpma_data.gcs_lib_handle = NULL;
    }
    lpma_data.state = LPMA_NOINIT;
    ALOGD("%s: exit", __func__);
}

