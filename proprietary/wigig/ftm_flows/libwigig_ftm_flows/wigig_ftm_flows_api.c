/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <stdbool.h>
#include <stdio.h>
#include <sys/time.h> /* for gettimeofday */
#include <time.h> /* for localtime */
#include <syslog.h>
#include <errno.h>

#include "wigig_ftm_flows_api.h"
#include "wigig_ftm_flows_impl.h"
#include "VersionInfo.h"

/* by default the output is sent to syslog, can be controlled with wigig_ftm_flows_configure_output_sink */
static bool g_use_syslog_sink = true;

/* by default no logs except errors are reported */
static bool g_verbosity_all = false;

/* internal status buffer for error reporting */
static char s_api_operation_status[512] = { '\0' };

/* thresholds */
static const uint8_t sc_max_gain_index = 15;
static const uint8_t sc_max_rfc_id = 7;
static const uint8_t sc_max_rf_sector_index = 127;

/* return local time formatted up to milleseconds */
static const char* get_time_msec()
{
    static char s_time_str[64];

    struct timeval tv = { 0 };
    struct tm* plocaltime = NULL;
    uint32_t bytes_placed = 0;

    /* get number of seconds and microseconds since the Epoch */
    gettimeofday(&tv, NULL);
    /* format date and time up to a second */
    plocaltime = localtime(&tv.tv_sec);
    bytes_placed = strftime(s_time_str, sizeof(s_time_str), "%Y-%m-%d %H:%M:%S", plocaltime);
    /* add milliseconds */
    snprintf(s_time_str + bytes_placed, sizeof(s_time_str) - bytes_placed, ".%03ld", tv.tv_usec / 1000);
    return s_time_str;
}

static const char* priority_to_str(int priority)
{
    /* some of the possible values are: LOG_ERR, LOG_WARNING, LOG_NOTICE, LOG_INFO, LOG_DEBUG */
    switch (priority) {
    case (LOG_ERR):
        return "ERR";
    case (LOG_WARNING):
        return "WRN";
    case (LOG_INFO):
        return "INF";
    case (LOG_DEBUG):
        return "DBG";
    default:
        return "UNKNOWN";
    }
}

static const char* channel_access_mode_to_str(channel_access_mode mode)
{
    switch (mode) {
    case (DMG):
        return "DMG";
    case (EDMG):
        return "EDMG";
    default:
        return "Unknown";
    }
}

/* internal helper for error reporting */
static const char* convert_work_mode_to_str(work_mode mode)
{
    switch (mode) {
    case MODE_TX:
        return "TX";
    case MODE_RX:
        return "RX";
    case MODE_STANDBY:
        return "STANDBY";
    case MODE_POWER_DOWN:
        return "POWER_DOWN";
    case MODE_LOOPBACK:
        return "LOOPBACK";
    default:
        return "Unknown work mode";
    }
}

static const char* convert_table_type_to_str(lineup_table_type table_type)
{
    switch (table_type) {
    case RX_G0:
        return "RX_G0";
    case RX_G1:
        return "RX_G1";
    case TX:
        return "TX";
    default:
        return "[unknown table type]";
    }
}

/* helper adding priority and timestamp prefix */
#define log_printf(priority, format, ...) \
    fprintf(stdout, "{%s} [%s] ", get_time_msec(), priority_to_str(priority)); \
    fprintf(stdout, format, ##__VA_ARGS__); \

/* does not use setlogmask since standard output should be supported as well */
/* do not assume order of priorities... */
#define log_sink(priority, format, ...) \
{ \
    if (g_verbosity_all || priority == LOG_ERR) { \
        if (g_use_syslog_sink) { \
            syslog(priority, format, ##__VA_ARGS__); \
        } \
        else { \
            log_printf(priority, format, ##__VA_ARGS__); \
        } \
    } \
} \

#define _LOG_ERROR(format, ...) \
    log_sink(LOG_ERR, format, ##__VA_ARGS__)
#define _LOG_WARNING(format, ...) \
    log_sink(LOG_WARNING, format, ##__VA_ARGS__)
#define _LOG_INFO(format, ...) \
    log_sink(LOG_INFO, format, ##__VA_ARGS__)
#define _LOG_DEBUG(format, ...) \
    log_sink(LOG_DEBUG, format, ##__VA_ARGS__)

/* Perform initialization required to communicate with given WIGIG interface */
int wigig_ftm_flows_initialize_connection(const char* ifname)
{
    _LOG_INFO("'%s' flow initiated...\n", __FUNCTION__);
    _LOG_DEBUG("'%s' flow parameters: ifname is '%s'.\n", __FUNCTION__, ifname);
    clear_last_impl_error(); /* should be first, allows to distinguish API or IMPL error happened */

    if (!ifname) {
        snprintf(s_api_operation_status, sizeof(s_api_operation_status), "Failed to initialize connection. Interface name is required.");
        _LOG_ERROR("%s\n", s_api_operation_status);
        return -EINVAL;
    }

    if (!initialize_netlink_interface(ifname)) {
        _LOG_ERROR("Failed to initialize netlink interface. Error: %s\n", get_last_impl_error());
        return -ENODEV;
    }

    /* enable LO for all XIF ports as part of the initialization */
    if (!BB_EnableLO()) {
        _LOG_ERROR("Failed to enable LO for all XIF ports. Error: %s\n", get_last_impl_error());
        return -1;
    }

    _LOG_INFO("'%s' flow completed.\n", __FUNCTION__);
    return 0;
}

/* Release connection with current WIGIG interface */
void wigig_ftm_flows_release_connection()
{
    _LOG_INFO("'%s' flow initiated...\n", __FUNCTION__);
    clear_last_impl_error(); /* should be first, allows to distinguish API or IMPL error happened */
    release_netlink_interface(); /* cannot fail */
    _LOG_INFO("'%s' flow completed.\n", __FUNCTION__);
}

/* Configure FTM Flows output destination - standard output or syslog (default) */
void wigig_ftm_flows_configure_output_sink(log_destination_type log_destination)
{
    bool prev_sink_is_syslog = g_use_syslog_sink;

    _LOG_INFO("'%s' flow initiated...\n", __FUNCTION__);
    g_use_syslog_sink = log_destination == SYSLOG;

    if (g_use_syslog_sink != prev_sink_is_syslog) {
        _LOG_INFO("'%s' flow initialization reported to %s.\n",
                  __FUNCTION__, g_use_syslog_sink ? "stdout" : "syslog");
    }
    _LOG_INFO("'%s' flow completed.\n", __FUNCTION__);
}

/* Configure FTM Flows log verbosity - all levels of verbosity or errors only (default) */
void wigig_ftm_flows_configure_log_verbosity(log_verbosity_type log_verbosity)
{
    /* initialization or completion may not be reported because of verbosity level change */
    _LOG_INFO("'%s' flow initiated...\n", __FUNCTION__);
    g_verbosity_all = log_verbosity == LOG_ALL;
    _LOG_INFO("'%s' flow completed.\n", __FUNCTION__);
}

/* Return the currently set WIGIG Channel */
int wigig_ftm_flows_get_channel(wigig_channel* channel_number_ptr)
{
    _LOG_INFO("'%s' flow initiated...\n", __FUNCTION__);
    clear_last_impl_error(); /* should be first, allows to distinguish API or IMPL error happened */

    if (!channel_number_ptr) {
        snprintf(s_api_operation_status, sizeof(s_api_operation_status), "Failed to get channel. Channel output argument is required.");
        _LOG_ERROR("%s\n", s_api_operation_status);
        return -EINVAL;
    }

    uint32_t channel_number = 0U;
    if (!BB_GetChannel(&channel_number)) {
        _LOG_ERROR("Failed to get channel. Error: %s\n", get_last_impl_error());
        return -1;
    }

    *channel_number_ptr = (wigig_channel)channel_number;

    _LOG_DEBUG("'%s' flow results: channel number is %d.\n", __FUNCTION__, channel_number);
    _LOG_INFO("'%s' flow completed.\n", __FUNCTION__);
    return 0;
}

/* Set WIGIG Channel */
/* Note: Baseband Work Mode will be changed to Standby */
int wigig_ftm_flows_set_channel(wigig_channel channel_number)
{
    _LOG_INFO("'%s' flow initiated...\n", __FUNCTION__);
    _LOG_DEBUG("'%s' flow parameters: channel_number is %d.\n", __FUNCTION__, channel_number);
    clear_last_impl_error(); /* should be first, allows to distinguish API or IMPL error happened */

    if (!BB_SetChannel((uint32_t)channel_number)) {
        _LOG_ERROR("Failed to set channel %d. Error: %s\n", (int)channel_number, get_last_impl_error());
        return -1;
    }

    _LOG_INFO("'%s' flow completed.\n", __FUNCTION__);
    return 0;
}

 /* Get currently set Baseband Work Mode */
int wigig_ftm_flows_get_work_mode(work_mode* mode_ptr)
{
    work_mode_impl mode_impl = MODE_IMPL_STANDBY;

    _LOG_INFO("'%s' flow initiated...\n", __FUNCTION__);
    clear_last_impl_error(); /* should be first, allows to distinguish API or IMPL error happened */

    if (!mode_ptr) {
        snprintf(s_api_operation_status, sizeof(s_api_operation_status), "Failed to get work mode. Mode output argument is required.");
        _LOG_ERROR("%s\n", s_api_operation_status);
        return -EINVAL;
    }

    if (!BB_GetWorkMode(&mode_impl)) {
        _LOG_ERROR("Failed to get work mode. Error: %s\n", get_last_impl_error());
        return -1;
    }

    *mode_ptr = (work_mode)mode_impl; /* same values */

    _LOG_DEBUG("'%s' flow results: work mode is %s.\n", __FUNCTION__, convert_work_mode_to_str(*mode_ptr));
    _LOG_INFO("'%s' flow completed.\n", __FUNCTION__);
    return 0;
}

/* Configure Baseband to a specific Work Mode.
    Supported modes are: TX, RX, and STANDBY.
    XIF is ignored when not in RX mode. XIF mask of zeros means close all XIFs */
int wigig_ftm_flows_set_work_mode(work_mode requested_mode, uint8_t xif_mask)
{
    _LOG_INFO("'%s' flow initiated...\n", __FUNCTION__);
    _LOG_DEBUG("'%s' flow parameters: requested_mode is %s, xif_mask is 0x%08x.\n",
               __FUNCTION__, convert_work_mode_to_str(requested_mode), xif_mask);
    clear_last_impl_error(); /* should be first, allows to distinguish API or IMPL error happened */

    if ( !(requested_mode == MODE_TX || requested_mode == MODE_RX || requested_mode == MODE_STANDBY) ) {
        snprintf(s_api_operation_status, sizeof(s_api_operation_status),
                 "Failed to set work mode. Requested unsupported mode %s. Expected modes are TX, RX or STANDBY.", convert_work_mode_to_str(requested_mode));
        _LOG_ERROR("%s\n", s_api_operation_status);
        return -EINVAL;
    }
    
    if (!BB_SetWorkMode((work_mode_impl)requested_mode /* same values */, xif_mask)) {
        _LOG_ERROR("Failed to set work mode %s. Error: %s\n", convert_work_mode_to_str(requested_mode), get_last_impl_error());
        return -1;
    }

    _LOG_INFO("'%s' flow completed.\n", __FUNCTION__);
    return 0;
}

/* Return measured PER [%] */
/* Only works when current work mode is RX */
int wigig_ftm_flows_get_per(uint32_t number_packets_sent, double* per_ptr)
{
    _LOG_INFO("'%s' flow initiated...\n", __FUNCTION__);
    _LOG_DEBUG("'%s' flow parameters: number_packets_sent is %d.\n", __FUNCTION__, number_packets_sent);
    clear_last_impl_error(); /* should be first, allows to distinguish API or IMPL error happened */

    if (number_packets_sent == 0U) {
        snprintf(s_api_operation_status, sizeof(s_api_operation_status), "Failed to get PER. Number of packets cannot be zero.");
        _LOG_ERROR("%s\n", s_api_operation_status);
        return -EINVAL;
    }

    if (!per_ptr) {
        snprintf(s_api_operation_status, sizeof(s_api_operation_status), "Failed to get PER. PER output argument is required.");
        _LOG_ERROR("%s\n", s_api_operation_status);
        return -EINVAL;
    }

    /* curren work mode validity (RX) is tested as part of the flow */
    if (!BB_GetPer(number_packets_sent, per_ptr)) {
        _LOG_ERROR("Failed to get PER. Error: %s\n", get_last_impl_error());
        return -1;
    }

    _LOG_DEBUG("'%s' flow results: PER is %.3f [%%].\n", __FUNCTION__, *per_ptr);
    _LOG_INFO("'%s' flow completed.\n", __FUNCTION__);
    return 0;
}

/* Clear PER RX statistics */
/* Only works when current work mode is RX */
int wigig_ftm_flows_clear_per()
{
    _LOG_INFO("'%s' flow initiated...\n", __FUNCTION__);
    clear_last_impl_error(); /* should be first, allows to distinguish API or IMPL error happened */

    /* curren work mode validity (RX) is tested as part of the flow */
    if (!BB_ClearRxStatistics()) {
        _LOG_ERROR("Failed to clear PER. Error: %s\n", get_last_impl_error());
        return -1;
    }
    
    _LOG_INFO("'%s' flow completed.\n", __FUNCTION__);
    return 0;
}

/* Return RF gain lineup index for the current mode */
int wigig_ftm_flows_get_rf_gain_lineup_index(uint8_t rfc_id, uint8_t gain_index, uint32_t* lineup_index_ptr, lineup_table_type* table_type_ptr)
{
    lineup_table_type_impl table_type_impl = RX_G0_IMPL;

    _LOG_INFO("'%s' flow initiated...\n", __FUNCTION__);
    _LOG_DEBUG("'%s' flow parameters: rfc_id is %d, gain index is %d.\n", __FUNCTION__, rfc_id, gain_index);
    clear_last_impl_error(); /* should be first, allows to distinguish API or IMPL error happened */

    /* test rfc_id in range [0, sc_max_rfc_id] */
    if (rfc_id > sc_max_rfc_id) {
        snprintf(s_api_operation_status, sizeof(s_api_operation_status),
                 "Failed to get RF gain lineup index. RFC id should be in range [0, %d].", sc_max_rfc_id);
        _LOG_ERROR("%s\n", s_api_operation_status);
        return -EINVAL;
    }

    /* test gain_index in range [0, sc_max_gain_index] */
    if (gain_index > sc_max_gain_index) {
        snprintf(s_api_operation_status, sizeof(s_api_operation_status),
            "Failed to get RF gain lineup index. Gain index should be in range [0, %d].", sc_max_gain_index);
        _LOG_ERROR("%s\n", s_api_operation_status);
        return -EINVAL;
    }

    if ( !(lineup_index_ptr && table_type_ptr) ) {
        snprintf(s_api_operation_status, sizeof(s_api_operation_status),
                 "Failed to get RF gain lineup index. Lineup and table type output arguments are required.");
        _LOG_ERROR("%s\n", s_api_operation_status);
        return -EINVAL;
    }

    /* current work mode validity is tested as part of the flow */
    if (!RF_GetGainLineupIndex(rfc_id, gain_index, lineup_index_ptr, &table_type_impl)) {
        _LOG_ERROR("Failed to get RF gain lineup index. Error: %s\n", get_last_impl_error());
        return -1;
    }

    *table_type_ptr = (lineup_table_type)table_type_impl; /* same values */

    _LOG_DEBUG("'%s' flow results: table type is %s, lineup index is %d.\n",
               __FUNCTION__, convert_table_type_to_str(*table_type_ptr), *lineup_index_ptr);
    _LOG_INFO("'%s' flow completed.\n", __FUNCTION__);
    return 0;
}

/* Set RF gain lineup index for the given mode */
int wigig_ftm_flows_set_rf_gain_lineup_index(uint8_t rfc_id, lineup_table_type table_type, uint8_t gain_index, uint32_t lineup_index)
{
    int res = 0;
    uint32_t max_lineup_index = 0U;

    _LOG_INFO("'%s' flow initiated...\n", __FUNCTION__);
    _LOG_DEBUG("'%s' flow parameters: rfc_id is %d, table type is %s, gain index is %d, lineup index is %d.\n",
               __FUNCTION__, rfc_id, convert_table_type_to_str(table_type), gain_index, lineup_index);
    clear_last_impl_error(); /* should be first, allows to distinguish API or IMPL error happened */

    /* test rfc_id in range [0, sc_max_rfc_id] */
    if (rfc_id > sc_max_rfc_id) {
        snprintf(s_api_operation_status, sizeof(s_api_operation_status),
                 "Failed to set RF gain lineup index. RFC id should be in range [0, %d].", sc_max_rfc_id);
        _LOG_ERROR("%s\n", s_api_operation_status);
        return -EINVAL;
    }

    /* test gain_index in range [0, sc_max_gain_index] */
    if (gain_index > sc_max_gain_index) {
        snprintf(s_api_operation_status, sizeof(s_api_operation_status),
                 "Failed to set RF gain lineup index. Gain index should be in range [0, %d].", sc_max_gain_index);
        _LOG_ERROR("%s\n", s_api_operation_status);
        return -EINVAL;
    }

    /* test lineup_index in range [0, get_maximum_lineup_index()] */
    res = wigig_ftm_flows_get_rf_gain_maximum_lineup_index(table_type, &max_lineup_index);
    if (res != 0) {
        /* error reported */
        return res;
    }

    if (lineup_index > max_lineup_index) {
        snprintf(s_api_operation_status, sizeof(s_api_operation_status),
                 "Failed to set RF gain lineup index %d. Lineup index should be in range [0, %d].", lineup_index, max_lineup_index);
        _LOG_ERROR("%s\n", s_api_operation_status);
        return -EINVAL;
    }

    if (!RF_SetGainLineupIndex(rfc_id, (lineup_table_type_impl)table_type /* same values */, gain_index, lineup_index)) {
        _LOG_ERROR("Failed to set RF gain lineup index. Error: %s\n", get_last_impl_error());
        return -1;
    }

    _LOG_INFO("'%s' flow completed.\n", __FUNCTION__);
    return 0;
}

/* Get RF gain maximum lineup index for the given table type */
int wigig_ftm_flows_get_rf_gain_maximum_lineup_index(lineup_table_type table_type, uint32_t* max_lineup_index_ptr)
{
    _LOG_INFO("'%s' flow initiated...\n", __FUNCTION__);
    _LOG_DEBUG("'%s' flow parameters: table type is %s.\n", __FUNCTION__, convert_table_type_to_str(table_type));
    clear_last_impl_error(); /* should be first, allows to distinguish API or IMPL error happened */

    if (!max_lineup_index_ptr) {
        snprintf(s_api_operation_status, sizeof(s_api_operation_status),
                 "Failed to get RF gain maximum lineup index. Maximum lineup index output argument is required.");
        _LOG_ERROR("%s\n", s_api_operation_status);
        return -EINVAL;
    }

    if (!RF_GetGainLineupIndexMaxRange((lineup_table_type_impl)table_type /* same values */, max_lineup_index_ptr)) {
        _LOG_ERROR("Failed to get RF gain maximum lineup index. Error: %s\n", get_last_impl_error());
        return -1;
    }

    _LOG_DEBUG("'%s' flow results: max lineup index is %d.\n", __FUNCTION__, *max_lineup_index_ptr);
    _LOG_INFO("'%s' flow completed.\n", __FUNCTION__);
    return 0;
}

/* Return current RF sector index */
int wigig_ftm_flows_get_rf_sector_index(uint8_t rfc_id, uint8_t* sector_index_ptr)
{
    _LOG_INFO("'%s' flow initiated...\n", __FUNCTION__);
    _LOG_DEBUG("'%s' flow parameters: rfc_id is %d.\n", __FUNCTION__, rfc_id);
    clear_last_impl_error(); /* should be first, allows to distinguish API or IMPL error happened */

    /* test rfc_id in range [0, sc_max_rfc_id] */
    if (rfc_id > sc_max_rfc_id) {
        snprintf(s_api_operation_status, sizeof(s_api_operation_status),
                 "Failed to get RF sector index. RFC id should be in range [0, %d].", sc_max_rfc_id);
        _LOG_ERROR("%s\n", s_api_operation_status);
        return -EINVAL;
    }

    if (!sector_index_ptr) {
        snprintf(s_api_operation_status, sizeof(s_api_operation_status),
                 "Failed to get RF sector index. Sector index output argument is required.");
        _LOG_ERROR("%s\n", s_api_operation_status);
        return -EINVAL;
    }

    if (!RF_GetSectorIndex(rfc_id, sector_index_ptr)) {
        _LOG_ERROR("Failed to get RF sector. Error: %s\n", get_last_impl_error());
        return -1;
    }

    _LOG_DEBUG("'%s' flow results: sector index is %d.\n", __FUNCTION__, *sector_index_ptr);
    _LOG_INFO("'%s' flow completed.\n", __FUNCTION__);
    return 0;
}

/* Set RF sector index for the current mode and gain index */
int wigig_ftm_flows_set_rf_sector_index(uint8_t rfc_id, uint8_t sector_index)
{
    _LOG_INFO("'%s' flow initiated...\n", __FUNCTION__);
    _LOG_DEBUG("'%s' flow parameters: rfc_id is %d, sector index is %d.\n", __FUNCTION__, rfc_id, sector_index);
    clear_last_impl_error(); /* should be first, allows to distinguish API or IMPL error happened */

    /* test rfc_id in range [0, sc_max_rfc_id] */
    if (rfc_id > sc_max_rfc_id) {
        snprintf(s_api_operation_status, sizeof(s_api_operation_status),
                 "Failed to set RF sector index. RFC id should be in range [0, %d].", sc_max_rfc_id);
        _LOG_ERROR("%s\n", s_api_operation_status);
        return -EINVAL;
    }

    /* test sector in range [0, sc_max_rf_sector_index] */
    if (sector_index > sc_max_rf_sector_index) {
        snprintf(s_api_operation_status, sizeof(s_api_operation_status),
                 "Failed to set RF sector index. RF sector index should be in range [0, %d].", sc_max_rf_sector_index);
        _LOG_ERROR("%s\n", s_api_operation_status);
        return -EINVAL;
    }

    /* current work mode validity is tested as part of the flow */
    if (!RF_SetSectorIndex(rfc_id, sector_index)) {
        _LOG_ERROR("Failed to set RF sector index. Error: %s\n", get_last_impl_error());
        return -1;
    }

    _LOG_INFO("'%s' flow completed.\n", __FUNCTION__);
    return 0;
}

/* Return masks of attached and enabled RF modules */
int wigig_ftm_flows_get_attached_enabled_rf_mask(uint8_t* attached_rf_mask_ptr, uint8_t* enabled_rf_mask_ptr)
{
    _LOG_INFO("'%s' flow initiated...\n", __FUNCTION__);
    clear_last_impl_error(); /* should be first, allows to distinguish API or IMPL error happened */

    if ( !(attached_rf_mask_ptr && enabled_rf_mask_ptr) ) {
        snprintf(s_api_operation_status, sizeof(s_api_operation_status),
                 "Failed to get mask of attached and enabled RF modules. All output arguments are required.");
        _LOG_ERROR("%s\n", s_api_operation_status);
        return -EINVAL;
    }

    if (!RF_GetAttachedEnabledMask(attached_rf_mask_ptr, enabled_rf_mask_ptr)) {
        _LOG_ERROR("Failed to get mask of attached and enabled RF modules. Error: %s\n", get_last_impl_error());
        return -1;
    }

    _LOG_DEBUG("'%s' flow results: attached rf_mask is 0x%08x, enabled rf_mask is 0x%08x.\n", __FUNCTION__, *attached_rf_mask_ptr, *enabled_rf_mask_ptr);
    _LOG_INFO("'%s' flow completed.\n", __FUNCTION__);
    return 0;
}

/* Return current MCS, payload length, silence duration and current transmission state */
/* Only works when current work mode is TX */
int wigig_ftm_flows_get_packet_configuration(uint32_t* phy_tx_silence_duration_clk_cycles_ptr,
                                             channel_access_mode* channel_access_mode_ptr, mcs_index* mcs_index_ptr,
                                             uint32_t* phy_tx_plcp_length_ptr, transmission_mode* transmission_mode_ptr)
{
    uint8_t mcs_value = 0;
    uint32_t current_channel_number = 0U;
    bool is_transmission_active = false;

    _LOG_INFO("'%s' flow initiated...\n", __FUNCTION__);
    clear_last_impl_error(); /* should be first, allows to distinguish API or IMPL error happened */

    if ( !(phy_tx_silence_duration_clk_cycles_ptr && channel_access_mode_ptr && mcs_index_ptr
           && phy_tx_plcp_length_ptr && transmission_mode_ptr) ) {
        snprintf(s_api_operation_status, sizeof(s_api_operation_status),
                 "Failed to get packet configuration. All output arguments are required.");
        _LOG_ERROR("%s\n", s_api_operation_status);
        return -EINVAL;
    }

    /* curren work mode validity (TX) is tested as part of the flow */
    /* mcs_value range expected is [0, 15] */
    if (!BB_GetPacketConfiguration(phy_tx_silence_duration_clk_cycles_ptr, &mcs_value, phy_tx_plcp_length_ptr, &is_transmission_active)) {
        _LOG_ERROR("Failed to get packet configuration. Error: %s\n", get_last_impl_error());
        return -1;
    }

    /* convert MCS value returned from status to mcs_index based on the current channel */
    if (!BB_GetChannel(&current_channel_number)) {
        _LOG_ERROR("Failed to get packet configuration. Cannot get current channel required for MCS conversion. Error: %s\n", get_last_impl_error());
        return -1;
    }

    /* Note: There is no validation that current configuration of channel and MCS is valid */
    *channel_access_mode_ptr = current_channel_number < (int)WIGIG_CHANNEL_9 ? DMG : EDMG;
    *mcs_index_ptr = (mcs_index)mcs_value;
    *transmission_mode_ptr = is_transmission_active ? ACTIVE : INACTIVE;

    /* sanity check of channel access mode and WIGIG channel constellation */
    /* DMG with MCS bigger than 12 or EDMG with MCS bigger than 15 */
    if ( (current_channel_number < (int)WIGIG_CHANNEL_9 && mcs_value > (int)MCS_12)
          || (current_channel_number >= (int)WIGIG_CHANNEL_9 && mcs_value > (int)MCS_15) ) {
        _LOG_WARNING("%s: Found inconsistent MCS (MCS_%d) and channel (%d) configuration.\n",
                     __FUNCTION__, mcs_value, current_channel_number);
        /* do not fail the flow */
    }

    _LOG_DEBUG("'%s' flow results: PHY TX silence duration is %d [clock cycles], PHY TX PLCP length is %d, MCS is %s MCS_%d, transmission is %s.\n",
               __FUNCTION__, *phy_tx_silence_duration_clk_cycles_ptr, *phy_tx_plcp_length_ptr,
               *channel_access_mode_ptr == DMG ? "DMG" : "EDMG", mcs_value,
               is_transmission_active ? "active" : "not active");
    _LOG_INFO("'%s' flow completed.\n", __FUNCTION__);
    return 0;
}

/* Start transmission with given parameters */
/* Only works when current work mode is TX */
int wigig_ftm_flows_start_packet(uint32_t phy_tx_silence_duration_clk_cycles,
                                 channel_access_mode channel_access_mode, mcs_index mcs_index,
                                 uint32_t phy_tx_plcp_length)
{
    uint32_t current_channel_number = 0U;

    _LOG_INFO("'%s' flow initiated...\n", __FUNCTION__);
    _LOG_DEBUG("'%s' flow parameters: PHY TX silence duration is %d [clock cycles], PHY TX PLCP length is %d, MCS is %s MCS_%d.\n",
               __FUNCTION__, phy_tx_silence_duration_clk_cycles, phy_tx_plcp_length,
               channel_access_mode == DMG ? "DMG" : "EDMG", mcs_index);
    clear_last_impl_error(); /* should be first, allows to distinguish API or IMPL error happened */

    /* check validity of requested channel access mode accordingly to the current channel number */
    if (!BB_GetChannel(&current_channel_number)) {
        _LOG_ERROR("Failed to get channel. Error: %s\n", get_last_impl_error());
        return -1;
    }

    /* Note: Do not fail the flow when requested configuration of channel and MCS is invalid */
    if ((channel_access_mode == EDMG && current_channel_number < (int)WIGIG_CHANNEL_9)
         || (channel_access_mode == DMG && current_channel_number >= (int)WIGIG_CHANNEL_9)) {
        /* EDMG MCS requested when channel is not in [9-11] or DMG MCS requested when channel is in [9-11] */
        _LOG_WARNING("%s: Requested invalid channel access mode %s (with MCS_%d) for WIGIG channel %d\n",
                     __FUNCTION__, channel_access_mode_to_str(channel_access_mode), mcs_index, current_channel_number);
        /* do not fail the flow */
    }

    /* Note: Do not fail the flow when requested configuration of channel access mode and MCS index is invalid */
    if ( (channel_access_mode == DMG && mcs_index > MCS_12) 
          || (channel_access_mode == EDMG && mcs_index > MCS_15) ) {
        _LOG_WARNING("%s: Requested invalid MCS_%d for %s channel access mode.\n",
                     __FUNCTION__, mcs_index, channel_access_mode_to_str(channel_access_mode));
        /* do not fail the flow */
    }

    /* curren work mode validity (TX) is tested as part of the flow */
    if (!BB_StartPacket(phy_tx_silence_duration_clk_cycles, (uint8_t)mcs_index, phy_tx_plcp_length, (int)current_channel_number)) {
        _LOG_ERROR("Failed to start transmission. Error: %s\n", get_last_impl_error());
        return -1;
    }

    _LOG_INFO("'%s' flow completed.\n", __FUNCTION__);
    return 0;
}

/* Stop transmission */
/* Only works when current work mode is TX */
int wigig_ftm_flows_stop_packet()
{
    _LOG_INFO("'%s' flow initiated...\n", __FUNCTION__);
    clear_last_impl_error(); /* should be first, allows to distinguish API or IMPL error happened */

    /* curren work mode validity (TX) is tested as part of the flow */
    if (!BB_StopPacket()) {
        _LOG_ERROR("Failed to stop transmission. Error: %s\n", get_last_impl_error());
        return -1;
    }

    _LOG_INFO("'%s' flow completed.\n", __FUNCTION__);
    return 0;
}

/* Return FW and Board file information */
int wigig_ftm_flows_get_fw_brd_versions(uint32_t* fw_version_major_ptr, uint32_t*  fw_version_minor_ptr,
                                                              uint32_t* fw_version_sub_minor_ptr, uint32_t* fw_version_build_ptr,
                                                              uint32_t* burned_board_file_ptr)
{
    _LOG_INFO("'%s' flow initiated...\n", __FUNCTION__);
    clear_last_impl_error(); /* should be first, allows to distinguish API or IMPL error happened */

    if ( !(fw_version_major_ptr && fw_version_minor_ptr 
            && fw_version_sub_minor_ptr && fw_version_build_ptr && burned_board_file_ptr) ) {
        snprintf(s_api_operation_status, sizeof(s_api_operation_status),
                 "Failed to get FW and Board file information. All output arguments are required.");
        _LOG_ERROR("%s\n", s_api_operation_status);
        return -EINVAL;
    }

    if (!BB_GetFwBoardFileVersions(fw_version_major_ptr, fw_version_minor_ptr,
                                   fw_version_sub_minor_ptr, fw_version_build_ptr, burned_board_file_ptr)) {
        _LOG_ERROR("Failed to get FW and Board file information. Error: %s\n", get_last_impl_error());
        return -1;
    }

    _LOG_DEBUG("'%s' flow results: FW version is [%d.%d.%d.%d], Board file Id is 0x%08x.\n",
               __FUNCTION__, *fw_version_major_ptr, *fw_version_minor_ptr,
               *fw_version_sub_minor_ptr, *fw_version_build_ptr, *burned_board_file_ptr);
    _LOG_INFO("'%s' flow completed.\n", __FUNCTION__);
    return 0;
}

/* Send ECHO command with the given value. This is a basic command for connectivity testing. */
int wigig_flows_send_echo(uint32_t echo_val, uint32_t* echo_val_out_ptr)
{
    _LOG_INFO("'%s' flow initiated...\n", __FUNCTION__);
    _LOG_DEBUG("'%s' flow parameters: echo value is %d.\n", __FUNCTION__, echo_val);
    clear_last_impl_error(); /* should be first, allows to distinguish API or IMPL error happened */

    if (!echo_val_out_ptr) {
        snprintf(s_api_operation_status, sizeof(s_api_operation_status),
            "Failed to get operational statistics. Echo value output arguments are required.");
        _LOG_ERROR("%s\n", s_api_operation_status);
        return -EINVAL;
    }

    if (!BB_SendEchoCommand(echo_val, echo_val_out_ptr)) {
        _LOG_ERROR("Failed to send ECHO command. Error: %s\n", get_last_impl_error());
        return -1;
    }

    _LOG_DEBUG("'%s' flow results: Echo output value is %d.\n", __FUNCTION__, *echo_val_out_ptr);
    _LOG_INFO("'%s' flow completed.\n", __FUNCTION__);
    return 0;
}

/* Return FTM Flow library version in format Major.Minor.Maintenance.Build */
const char* wigig_ftm_flows_get_version()
{
    return get_ftm_flows_version();
}

const char* wigig_ftm_flows_get_last_error()
{
    const char* impl_operation_status = get_last_impl_error();
    if (impl_operation_status[0] != '\0') {
        return impl_operation_status;
    }
    // otherwise it's API level error

    return s_api_operation_status;
}

/***************************************************************************/
/*     Following APIs are intended for Operational (Network) mode only     */
/***************************************************************************/

/* Force MCS value */
int wigig_operational_flows_force_mcs(mcs_enforcement_mode mcs_enforcement, mcs_index mcs_index)
{
    uint32_t current_channel_number = 0U;

    _LOG_INFO("'%s' flow initiated...\n", __FUNCTION__);
    _LOG_DEBUG("'%s' flow parameters: MCS enforcemetn mode is '%s', MCS index is MCS_%d.\n",
               __FUNCTION__, mcs_enforcement == FORCE_MCS ? "force" : "auto", (int)mcs_index);
    clear_last_impl_error(); /* should be first, allows to distinguish API or IMPL error happened */

    /* check validity of requested MCS index according to the current channel number */
    if (!BB_GetChannel(&current_channel_number)) {
        _LOG_ERROR("Failed to get channel. Error: %s\n", get_last_impl_error());
        return -1;
    }

    /* Note: Do not fail the flow when requested configuration of MCS index and WIGIG channel is invalid */
    if ((current_channel_number < (int)WIGIG_CHANNEL_9 && mcs_index > MCS_12)
        || (current_channel_number >= (int)WIGIG_CHANNEL_9 && mcs_index > MCS_15)) {
        _LOG_WARNING("%s: Requested invalid MCS_%d when current WIGGI channel is %d.\n",
                     __FUNCTION__, mcs_index, current_channel_number);
        /* do not fail the flow */
    }

    /* ensure we're not in FTM mode is the implementation level */
    if (!BB_OperationalForceMcs(mcs_enforcement == FORCE_MCS ? 1U : 0U, mcs_index, (int)current_channel_number)) {
        _LOG_ERROR("Failed to force MCS. Error: %s\n", get_last_impl_error());
        return -1;
    }

    _LOG_INFO("'%s' flow completed.\n", __FUNCTION__);
    return 0;
}

/* Force TX sector index for corresponding RF */
int wigig_operational_flows_force_tx_sector_index(uint16_t tx_sector_index)
{
    static const uint16_t sc_auto_sector_value = 0xFFFF;

    _LOG_INFO("'%s' flow initiated...\n", __FUNCTION__);
    _LOG_DEBUG("'%s' flow parameters: TX sector index %d.\n", __FUNCTION__, tx_sector_index);
    clear_last_impl_error(); /* should be first, allows to distinguish API or IMPL error happened */

    /* test sector in range [0, sc_max_rf_sector_index] */
    if (tx_sector_index > sc_max_rf_sector_index && tx_sector_index != sc_auto_sector_value) {
        snprintf(s_api_operation_status, sizeof(s_api_operation_status),
                 "Failed to force TX sector index. RF sector index should be in range [0, %d] or equal to %d.",
                 sc_max_rf_sector_index, sc_auto_sector_value);
        _LOG_ERROR("%s\n", s_api_operation_status);
        return -EINVAL;
    }

    if (!BB_OperationalForceTxSector(tx_sector_index)) {
        _LOG_ERROR("Failed to force TX sector index. Error: %s\n", get_last_impl_error());
        return -1;
    }

    _LOG_INFO("'%s' flow completed.\n", __FUNCTION__);
    return 0;
}

/* Return BF MCS, measured PER avarage [%], TX and RX goodput [Mbps] for the given time interval */
int wigig_operational_flows_get_statistics(uint32_t interval_msec,
                                           uint8_t* bf_mcs_ptr, uint8_t* per_average_ptr,
                                           uint32_t* tx_goodput_ptr, uint32_t* rx_goodput_ptr,
                                           uint8_t* tx_sector_index_ptr)
{
    static const uint32_t sc_min_interval_msec = 100U;
    static const uint32_t sc_max_interval_msec = 4500U; /* FW allows up to 10 sec., but we have WMI timeout of 5 sec. */

    _LOG_INFO("'%s' flow initiated...\n", __FUNCTION__);
    _LOG_DEBUG("'%s' flow parameters: time interval is %d [msec].\n", __FUNCTION__, interval_msec);
    clear_last_impl_error(); /* should be first, allows to distinguish API or IMPL error happened */

    if ( !(bf_mcs_ptr && per_average_ptr && tx_goodput_ptr
           && rx_goodput_ptr && tx_sector_index_ptr) ) {
        snprintf(s_api_operation_status, sizeof(s_api_operation_status),
                 "Failed to get operational statistics. All output arguments are required.");
        _LOG_ERROR("%s\n", s_api_operation_status);
        return -EINVAL;
    }

    /* test time interval in range [sc_min_interval_msec, sc_max_interval_msec] */
    if (interval_msec < sc_min_interval_msec  || interval_msec > sc_max_interval_msec) {
        snprintf(s_api_operation_status, sizeof(s_api_operation_status),
                 "Failed to get operational statistics. Time interval should be in range [%d, %d].",
                 sc_min_interval_msec, sc_max_interval_msec);
        _LOG_ERROR("%s\n", s_api_operation_status);
        return -EINVAL;
    }

    if (!BB_OperationalGetStatistics(interval_msec, bf_mcs_ptr, per_average_ptr,
                                     tx_goodput_ptr, rx_goodput_ptr, tx_sector_index_ptr)) {
        _LOG_ERROR("Failed to get operational statistics. Error: %s\n", get_last_impl_error());
        return -1;
    }

    _LOG_DEBUG("'%s' flow results: BF MCS is %d, average PER is %d [%%], TX goodput is %d [Mbps], RX goodput is %d [Mbps], TX sector index is %d.\n",
               __FUNCTION__, *bf_mcs_ptr, *per_average_ptr, *tx_goodput_ptr, *rx_goodput_ptr, *tx_sector_index_ptr);
    _LOG_INFO("'%s' flow completed.\n", __FUNCTION__);
    return 0;
}
