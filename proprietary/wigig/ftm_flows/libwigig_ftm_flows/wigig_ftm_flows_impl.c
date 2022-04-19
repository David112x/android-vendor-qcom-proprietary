/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <linux/types.h>

#include "wigig_ftm_flows_impl.h"
#include "nl_services.h"

typedef uint8_t u8;
typedef int8_t s8;
typedef int16_t s16;
typedef int64_t s64;
#ifndef __packed
    #define __packed __attribute__ ((packed))
#endif
#include "wmi.h"
#include "wmiUT.h"

#define STRINGER(arg) #arg

static char s_operation_status[512] = { '\0' }; /* internal status buffer for error reporting */
static struct nl_state_t* s_nl_state = NULL;    /* representation of the netlink state, kind of device descriptor */
static const unsigned sc_max_dynamic_response_len = 1024U; /* maximum response size in case of dynamic array */

enum edmg_header_bands {
    DMG_CENTER_BAND         = 0, /* DMG signal at Center band */
    DMG_LOW_BAND            = 1, /* DMG signal at lower band */
    DMG_HIGH_BAND           = 2, /* DMG signal at higher band */
    DMG_DUPLICATE_BAND      = 3, /* DMG signal duplicated on two bands */
    EDMG_CB1_CENTER_BAND    = 4, /* EDMG(CB1) signal at Center band */
    EDMG_CB1_LOW_BAND       = 5, /* EDMG(CB1) signal at lower band */
    EDMG_CB1_HIGH_BAND      = 6, /* EDMG(CB1) signal at higher band */
    EDMG_CB2                = 7  /* EDMG Channel bonding of 2 channels */
};

/* internal helper for error reporting */
static const char* convert_work_mode_to_str(work_mode_impl mode)
{
    switch (mode) {
    case MODE_IMPL_TX:
        return "TX";
    case MODE_IMPL_RX:
        return "RX";
    case MODE_IMPL_STANDBY:
        return "STANDBY";
    case MODE_IMPL_POWER_DOWN:
        return "POWER_DOWN";
    case MODE_IMPL_LOOPBACK:
        return "LOOPBACK";
    default:
        return "Unknown work mode";
    }
}

/* internal helper for error reporting */
static const char* convert_rf_sector_status_to_str(enum wmi_rf_sector_status status)
{
    switch (status) {
    case WMI_RF_SECTOR_STATUS_SUCCESS:
        return "Success";
    case WMI_RF_SECTOR_STATUS_BAD_PARAMETERS_ERROR:
        return "Bad Parameters Error";
    case WMI_RF_SECTOR_STATUS_BUSY_ERROR:
        return "Busy Error";
    case WMI_RF_SECTOR_STATUS_NOT_SUPPORTED_ERROR:
        return "Not Supported Error";
    default:
        return "Unknown status";
    }
}

/* Initialize netlink interface for given WIGIG interface name */
bool initialize_netlink_interface(const char* ifname)
{
    bool res = false;

    if (!ifname) {
        snprintf(s_operation_status, sizeof(s_operation_status), "%s: Invalid arguments", __FUNCTION__);
        return false;
    }

    if (s_nl_state) {
        /* re-set, release previous netlink state */
        nl_release(s_nl_state);
    }

    res = nl_initialize(ifname, &s_nl_state, s_operation_status, sizeof(s_operation_status));
    if ( !(res && s_nl_state) ) {
        /* status was updated */
        return false;
    }

    return true;
}

/* Release current netlink interface */
bool release_netlink_interface()
{
    if (s_nl_state) {
        nl_release(s_nl_state);
    }
    /* otherwise nothing to do */

    return true;
}

static bool ensure_netlink_state(const char* caller_name)
{
    if (!caller_name) { /* shouldn't happen*/
        return false;
    }

    if (!s_nl_state) {
        snprintf(s_operation_status, sizeof(s_operation_status), "%s: Netlink interface is not initialized", caller_name);
        return false;
    }

    return true;
}

static bool ensure_work_mode(const char* caller_name, work_mode_impl expected_mode)
{
    work_mode_impl current_mode = MODE_IMPL_STANDBY;

    if (!caller_name) { /* shouldn't happen*/
        return false;
    }

    /* get current mode and ensure its equal to expected mode */
    if (!BB_GetWorkMode(&current_mode)) {
        /* error was updated */
        return false;
    }

    if (current_mode != expected_mode) {
        snprintf(s_operation_status, sizeof(s_operation_status), "%s work mode detected during %s, expected work mode is %s",
                 convert_work_mode_to_str(current_mode), caller_name, convert_work_mode_to_str(expected_mode));
        return false;
    }

    return true;
}

static bool ensure_operational_mode(bool expected_operational)
{
    uint32_t major = 0U, minor = 0U, sub_minor = 0U, build = 0U, brd_id = 0U;
    bool is_ftm = false;

    if (!BB_GetFwBoardFileVersions(&major, &minor, &sub_minor, &build, &brd_id)) {
        /* error was updated */
        return false;
    }

    /* when first bit of sub-minor field is set, it's WMI ONLY FW */
    is_ftm = (sub_minor & 0x1) == 1;

    if (is_ftm == expected_operational) {
        /* actual is FTM, expected operational
           or actual is Operational, ecpected FTM */
        snprintf(s_operation_status, sizeof(s_operation_status), "functionality is not available in %s mode",
                 is_ftm ? "FTM" : "Operational (Network)");
        return false;
    }

    return true;
}

/* Wrapper for sending of WMI command in netlink commuter layer */
/* response lenght is in/out parameter */
/* is_dynamic_response - true when expected load lenght is unknown and so actual respnse length
    may be less than given response length argument */
/* validate_response_status - true to check that first 32 bits of the response contain value 1 meaning success,
    false to ignore when expected a custom response status */
static bool send_wmi_blocking(enum wmi_command_id commandId, enum wmi_event_id eventId, const char* wmi_name,
                              const void* payload, uint32_t payload_len,
                              void* response, uint32_t* response_len_ptr,
                              bool is_dynamic_response, bool validate_response_status)
{
    uint32_t expected_response_len = 0U;

    if (!(payload || response || response_len_ptr)) { /* shouldn't happen */
        return false;
    }

    if (!s_nl_state) { /* shouldn't happen, check directly without calling ensure_netlink_state */
        return false;
    }

    expected_response_len = *response_len_ptr;

    if (!nl_send_wmi_blocking(s_nl_state, (uint32_t)commandId, (uint32_t)eventId,
                              (const uint8_t*)payload, payload_len, (uint8_t*)response, response_len_ptr)) {
        snprintf(s_operation_status, sizeof(s_operation_status), "failed to send WMI '%s' (%s)", wmi_name, nl_get_last_errror(s_nl_state));
        return false;
    }

    /* check response status: first 32 bits of the response contain WMI success, where 0x1 means success and zero means failure */
    if (*response_len_ptr == 0U) {
        snprintf(s_operation_status, sizeof(s_operation_status), "failed to send WMI '%s' (response length is zero)", wmi_name);
        return false;
    }

    if (validate_response_status && *((uint32_t*)response) == 0U) {
        snprintf(s_operation_status, sizeof(s_operation_status), "failed to send WMI '%s' (response status is false)", wmi_name);
        return false;
    }

    /* ensure that the whole expected response was received */
    /* in case of dynamic payload the actual length may be smaller than expected == maximum_size */
    if (!is_dynamic_response && *response_len_ptr < expected_response_len) { /* shouldn't happen */
        snprintf(s_operation_status, sizeof(s_operation_status), "failed to send WMI '%s' (expected %d Bytes, received %d Bytes)", wmi_name, expected_response_len, *response_len_ptr);
        return false;
    }

    return true;
}

/* Wrapper for sending of UT WMI command in netlink commuter layer */
/* response lenght is in/out parameter */
static bool send_wmi_ut_blocking(enum wmi_ut_module moduleId, int subtypeId, const char* ut_name,
                                 const void* payload, uint32_t payload_len,
                                 void* response, uint32_t* response_len_ptr)
{
    uint32_t expected_response_len = 0U;

    if ( !(payload || response || response_len_ptr) ) { /* shouldn't happen */
        return false;
    }

    if (!s_nl_state) { /* shouldn't happen, check directly without calling ensure_netlink_state */
        return false;
    }

    expected_response_len = *response_len_ptr;

    if (!nl_send_wmi_ut_blocking(s_nl_state, (uint16_t)moduleId, (uint16_t)subtypeId,
                                 (const uint8_t*)payload, payload_len, (uint8_t*)response, response_len_ptr)) {
        snprintf(s_operation_status, sizeof(s_operation_status), "failed to send WMI UT '%s' (%s)", ut_name, nl_get_last_errror(s_nl_state));
        return false;
    }

    /* check response status: first 32 bits of the response contain UT success, where 0x1 means success and zero means failure */

    if (*response_len_ptr == 0U) {
        snprintf(s_operation_status, sizeof(s_operation_status), "failed to send WMI UT '%s' (response length is zero)", ut_name);
        return false;
    }

    if (*((uint32_t*)response) == 0U) {
        snprintf(s_operation_status, sizeof(s_operation_status), "failed to send WMI UT '%s' (response status is false)", ut_name);
        return false;
    }

    /* ensure that the whole expected response was received */
    /* actual length may be bigger than expected in case of dynamic payload */
    if (*response_len_ptr < expected_response_len) { /* shouldn't happen */
        snprintf(s_operation_status, sizeof(s_operation_status), "failed to send WMI UT '%s' (expected %d Bytes, received %d Bytes)", ut_name, expected_response_len, *response_len_ptr);
        return false;
    }

    return true;
}

/* Internal helper. Assumes netlink interface is initialized */
static bool get_bb_status_1(struct wmi_hw_sysapi_get_bb_status_block_1_event* status_ptr)
{
    struct wmi_hw_sysapi_get_bb_status_block_1_cmd params;
    uint32_t status_len = sizeof(struct wmi_hw_sysapi_get_bb_status_block_1_event);

    if (!status_ptr) { /* shouldn't happen */
        return false;
    }

    if (!send_wmi_ut_blocking(ut_module_system_api, (int)hw_sysapi_get_bb_status_block_1, STRINGER(hw_sysapi_get_bb_status_block_1),
                              &params, sizeof(params), status_ptr, &status_len)) {
        return false;
    }

    return true;
}

/* Internal helper. Assumes netlink interface is initialized */
static bool get_bb_status_2(struct wmi_hw_sysapi_get_bb_status_block_2_event* status_ptr)
{
    struct wmi_hw_sysapi_get_bb_status_block_2_cmd params;
    uint32_t status_len = sizeof(struct wmi_hw_sysapi_get_bb_status_block_2_event);

    if (!status_ptr) { /* shouldn't happen */
        return false;
    }

    if (!send_wmi_ut_blocking(ut_module_system_api, (int)hw_sysapi_get_bb_status_block_2, STRINGER(hw_sysapi_get_bb_status_block_2),
                              &params, sizeof(params), status_ptr, &status_len)) {
        return false;
    }

    return true;
}

/* Internal helper. Assumes netlink interface is initialized */
static bool get_rf_status(uint8_t rfc_id, struct wmi_hw_sysapi_get_rf_status_event* status_ptr)
{
    struct wmi_hw_sysapi_get_rf_status_cmd params = { rfc_id };
    uint32_t status_len = sizeof(struct wmi_hw_sysapi_get_rf_status_event);

    if (!status_ptr) { /* shouldn't happen */
        return false;
    }

    if (!send_wmi_ut_blocking(ut_module_system_api, (int)hw_sysapi_get_rf_status, STRINGER(hw_sysapi_get_rf_status),
                              &params, sizeof(params), status_ptr, &status_len)) {
        return false;
    }

    return true;
}

static bool get_statistics(struct wmi_hwd_phy_rx_get_statistics_event* statistics_ptr)
{
    struct wmi_hwd_phy_rx_get_statistics_cmd params;
    uint32_t statistics_len = sizeof(struct wmi_hwd_phy_rx_get_statistics_event);

    if (!statistics_ptr) { /* shouldn't happen */
        return false;
    }

    if (!send_wmi_ut_blocking(ut_module_drivers, (int)hwd_phy_rx_get_statistics, STRINGER(hwd_phy_rx_get_statistics),
        &params, sizeof(params), statistics_ptr, &statistics_len))
    {
        return false;
    }

    return true;
}

static bool configure_packet_transmission(const struct wmi_hwd_phy_tx_self_transmit_ext_cmd* params_ptr)
{
    struct wmi_hwd_phy_tx_self_transmit_ext_event response = { 0 };
    uint32_t response_len = sizeof(response);
    uint32_t params_len = sizeof(struct wmi_hwd_phy_tx_self_transmit_ext_cmd);

    if (!params_ptr) { /* shouldn't happen */
        return false;
    }

    return send_wmi_ut_blocking(ut_module_drivers, (int)hwd_phy_tx_self_transmit_ext, STRINGER(hwd_phy_tx_self_transmit_ext),
                                params_ptr, params_len, &response, &response_len);
}

/* Enables LO for all XIF ports */
bool BB_EnableLO()
{
    struct wmi_hwm_analog_lo_mux_enable_cmd params = { 0xFF }; /* all XIF ports */
    struct wmi_hwm_analog_lo_mux_enable_event response = { 0 };
    uint32_t response_len = sizeof(response);

    if (!s_nl_state) { /* shouldn't happen, check directly without calling ensure_netlink_state */
        return false;
    }

    if (!send_wmi_ut_blocking(ut_module_modes, (int)hwm_analog_lo_mux_enable, STRINGER(hwm_analog_lo_mux_enable),
        &params, sizeof(params), &response, &response_len)) {
        return false;
    }

    return true;
}

 /* Get currently set WIGIG Channel */
bool BB_GetChannel(uint32_t* channel_number_ptr)
{
    struct wmi_hw_sysapi_get_bb_status_block_1_event bb_status = { 0 };

    if (!ensure_netlink_state(__FUNCTION__)) {
        return false;
    }

    if (!channel_number_ptr) {
        snprintf(s_operation_status, sizeof(s_operation_status), "%s: Invalid arguments", __FUNCTION__);
        return false;
    }
    
    if (!get_bb_status_1(&bb_status)) {
        /* error was updated */
        return false;
    }

    /* convert the returned channel index to channel number, will be converted to enum by upper level */
    *channel_number_ptr = bb_status.channel + 1U;
    
    return true;
}

/* Set WIGIG Channel
 * This flow exits the current mode (enters standby mode) and sets the BB/RF channel
 *
 * Should be used only in FTM Mode, otherwise will fail.
 */
bool BB_SetChannel(uint32_t channel_number)
{
    struct wmi_hwm_analog_channel_switch_cmd params = { channel_number };
    struct wmi_hwm_analog_channel_switch_event response = { 0 };
    uint32_t response_len = sizeof(response);

    if (!ensure_netlink_state(__FUNCTION__)) {
        return false;
    }

    /* ensure we're not in Operational mode */
    if (!ensure_operational_mode(false /* expected_operational */)) {
        return false;
    }

    /* enter Standby mode, also ensures that netlink interface is initialized */
    if (!BB_SetWorkMode(MODE_IMPL_STANDBY, 0 /* dummy, not in use for Standby */)) {
        /* error was updated */
        return false;
    }

    /* Note: does not perform internal calibration after setting channel as the original flow, moved to set-work-mode flow */

    return send_wmi_ut_blocking(ut_module_modes, (int)hwm_analog_channel_switch, STRINGER(hwm_analog_channel_switch),
                                &params, sizeof(params), &response, &response_len);
}

/* Get currently set Baseband Work Mode */
bool BB_GetWorkMode(work_mode_impl* p_mode_impl)
{
    struct wmi_hw_sysapi_get_bb_status_block_1_event bb_status = { 0 };

    if (!ensure_netlink_state(__FUNCTION__)) {
        return false;
    }

    if (!p_mode_impl) {
        snprintf(s_operation_status, sizeof(s_operation_status), "%s: Invalid arguments", __FUNCTION__);
        return false;
    }

    if (!get_bb_status_1(&bb_status)) {
        /* error was updated */
        return false;
    }

    *p_mode_impl = (work_mode_impl)bb_status.work_mode;
    return true;
}

/* Internal helper to enter STANDBY mode before entering another work mode */
/* Does nothing if current mode is STANDBY or POWER_DOWN */
static bool enter_standby_from_work_mode(work_mode_impl current_mode)
{
    struct wmi_hw_sysapi_enter_tx_mode_ext_cmd params_tx = { 0, 0, 0, 0, 0, 0, 0 };
    struct wmi_hw_sysapi_enter_tx_mode_ext_event response_tx = { 0 };
    uint32_t response_tx_len = sizeof(response_tx);

    struct wmi_hw_sysapi_enter_rx_mode_ext_cmd params_rx = { 0, 0, 0, 0, 0, 0, 0 };
    struct wmi_hw_sysapi_enter_rx_mode_ext_event response_rx = { 0 };
    uint32_t response_rx_len = sizeof(response_rx);

    struct wmi_hwm_analog_if_loopback_ext_cmd params_loopback = { 0, 0, 0, 0, 0 };
    struct wmi_hwm_analog_if_loopback_ext_event response_loopback = { 0 };
    uint32_t response_loopback_len = sizeof(response_loopback);

    switch (current_mode) {
    case MODE_IMPL_TX:
        if (!send_wmi_ut_blocking(ut_module_system_api, (int)hw_sysapi_enter_tx_mode_ext, STRINGER(hw_sysapi_enter_tx_mode_ext),
            &params_tx, sizeof(params_tx), &response_tx, &response_tx_len)) {
            return false;
        }
        break;
    case MODE_IMPL_RX:
        if (!send_wmi_ut_blocking(ut_module_system_api, (int)hw_sysapi_enter_rx_mode_ext, STRINGER(hw_sysapi_enter_rx_mode_ext),
            &params_rx, sizeof(params_rx), &response_rx, &response_rx_len)) {
            return false;
        }
        break;
    case MODE_IMPL_STANDBY:
    case MODE_IMPL_POWER_DOWN:
        /* we're done */
        break;
    case MODE_IMPL_LOOPBACK:
        if (!send_wmi_ut_blocking(ut_module_modes, (int)hwm_analog_if_loopback_ext, STRINGER(hwm_analog_if_loopback_ext),
            &params_loopback, sizeof(params_loopback), &response_loopback, &response_loopback_len)) {
            return false;
        }
        break;
    default:
        snprintf(s_operation_status, sizeof(s_operation_status), "failed to enter STANDBY mode from unknown work mode %s", convert_work_mode_to_str(current_mode));
        return false;
    }

    return true;
}

/* Internal helper to enter the requested work mode
  * Notes:
  * Assumes the current mode is STANDBY and calibration performed (when required)
  * Supported modes are TX, RX and STANDBY
  * XIF is ignored when requested mode is not RX
  */
static bool enter_work_mode(work_mode_impl requested_mode, uint8_t xif_mask)
{
    /* parameters for entering TX */
    /* enable,rf_sector_id,rf_gain_index,tx_bb_gain_row,mcs,xif_port_mask,reserved */
    struct wmi_hw_sysapi_enter_tx_mode_ext_cmd params_tx = { 1, 0, 0xFE, 0xFF, 0, 0xFF/* instead of xif mask */, 0 };
    struct wmi_hw_sysapi_enter_tx_mode_ext_event response_tx = { 0 };
    uint32_t response_tx_len = sizeof(response_tx);

    /* parameters for entering RX */
    /* enable,no_detection_mode,rf_sector_id,rf_gain_index,rx_bb_gain_row,xif_port_mask,reserved */
    struct wmi_hw_sysapi_enter_rx_mode_ext_cmd params_rx = { 1, 0, 0, 0xFE, 0xFF, xif_mask, 0 };
    struct wmi_hw_sysapi_enter_rx_mode_ext_event response_rx = { 0 };
    uint32_t response_rx_len = sizeof(response_rx);

    struct wmi_hwd_phy_rx_ina_det_mode_cmd params_det_en = { 3 };
    struct wmi_hwd_phy_rx_ina_det_mode_event response_det_en = { 0 };
    uint32_t response_det_en_len = sizeof(response_det_en);

    switch (requested_mode) {
    case MODE_IMPL_TX:
        /* sets bb gain row to 0, doesn't change rf gain index */
        /* note: we does not support setting XIF mask in TX mode as part of FTM Flow */
        if (!send_wmi_ut_blocking(ut_module_system_api, (int)hw_sysapi_enter_tx_mode_ext, STRINGER(hw_sysapi_enter_tx_mode_ext),
            &params_tx, sizeof(params_tx), &response_tx, &response_tx_len)) {
            return false;
        }
        break;
    case MODE_IMPL_RX:
        /* sets bb gain row to 0, doesn't change rf gain index */
        if (!send_wmi_ut_blocking(ut_module_system_api, (int)hw_sysapi_enter_rx_mode_ext, STRINGER(hw_sysapi_enter_rx_mode_ext),
            &params_rx, sizeof(params_rx), &response_rx, &response_rx_len)) {
            return false;
        }

        /* configures BB module for proper PER measurement */
        if (!send_wmi_ut_blocking(ut_module_drivers, (int)hwd_phy_rx_ina_det_mode, STRINGER(hwd_phy_rx_ina_det_mode),
            &params_det_en, sizeof(params_det_en), &response_det_en, &response_det_en_len)) {
            return false;
        }
        break;
    case MODE_IMPL_STANDBY:
        /* we're done */
        break;
    default:
        snprintf(s_operation_status, sizeof(s_operation_status), "failed to set requested unsupported/unknown work mode %s", convert_work_mode_to_str(requested_mode));
        return false;
    }

    return true;
}

/* 
 * This flow configures BB to a specific Work Mode. 
 *  Supported modes are: TX, RX, and STANDBY.
 *  XIF is ignored when requested mode is not RX
 *  XIF mask of zeros means close all XIFs
 *
 * Should be used only in FTM Mode, otherwise will fail.
 */
bool BB_SetWorkMode(work_mode_impl requested_mode, uint8_t xif_mask)
{
    work_mode_impl current_mode = MODE_IMPL_STANDBY;

    /* calibration parameters */
    struct wmi_hwf_calib_all_cmd hwf_calib_params;
    struct wmi_hwf_calib_all_event hwf_calib_response = { 0 };
    uint32_t hwf_calib_response_len = sizeof(hwf_calib_response);

    if ( !(requested_mode == MODE_IMPL_TX
        || requested_mode == MODE_IMPL_RX 
        || requested_mode == MODE_IMPL_STANDBY) ) { /* shouldn't happen */
        snprintf(s_operation_status, sizeof(s_operation_status), "failed to set unsupported work mode %s", convert_work_mode_to_str(requested_mode));
        return false;
    }

    if (!ensure_netlink_state(__FUNCTION__)) {
        return false;
    }

    /* ensure we're not in Operational mode */
    if (!ensure_operational_mode(false /* expected_operational */)) {
        return false;
    }

    /* get work mode */
    if (!BB_GetWorkMode(&current_mode)) {
        /* error was updated */
        return false;
    }

    /* enter STANDBY mode before entering another work mode (if not already in STANDBY) */
    if (!enter_standby_from_work_mode(current_mode)) {
        /* error was updated */
        return false;
    }

    /* perform internal calibration unless requested mode is Standby */
    /* note: moved from set channel flow */
    if (requested_mode != MODE_IMPL_STANDBY) {
        if (!send_wmi_ut_blocking(ut_module_flows_operational, (int)hwf_calib_all, STRINGER(hwf_calib_all),
            &hwf_calib_params, sizeof(hwf_calib_params), &hwf_calib_response, &hwf_calib_response_len)) {
            return false;
        }
    }

    /* enter to the requested mode */
    if (!enter_work_mode(requested_mode, xif_mask)) {
        /* error was updated */
        return false;
    }
    
    return true;
}

/* Return measured PER [%]
 * Only works when current work mode is RX
 *
 * Should be used only in FTM Mode, otherwise will fail.
 */
bool BB_GetPer(uint32_t number_packets_sent, double* per_ptr)
{
    struct wmi_hwd_phy_rx_get_statistics_event statistics = { 0 };
    double per = 0.0;

    if (!ensure_netlink_state(__FUNCTION__)) {
        return false;
    }

    /* ensure we're not in Operational mode */
    if (!ensure_operational_mode(false /* expected_operational */)) {
        return false;
    }

    if (number_packets_sent == 0U) {
        snprintf(s_operation_status, sizeof(s_operation_status), "%s: Invalid argument. Number of packets cannot be zero.", __FUNCTION__);
        return false;
    }

    if (!per_ptr) {
        snprintf(s_operation_status, sizeof(s_operation_status), "%s: Invalid arguments. PER output argument is required.", __FUNCTION__);
        return false;
    }

    /* get current mode and ensure its RX */
    if (!ensure_work_mode(__FUNCTION__, MODE_IMPL_RX)) {
        return false;
    }

    if (!get_statistics(&statistics)) {
        /* error was updated */
        return false;
    }

    per = (1 - (double)statistics.ber_last_zero_counter_sc / number_packets_sent) * 100;
    *per_ptr = round(per * 1000) / 1000; /* set precision of 3 places after decimal point */

    return true;
}

/* Clear PER Rx statistics
 * Only works when current work mode is RX
 *
 * Should be used only in FTM Mode, otherwise will fail.
 */
bool BB_ClearRxStatistics()
{
    struct wmi_hwd_phy_rx_statistics_clear_cmd params;
    struct wmi_hwd_phy_rx_statistics_clear_event response = { 0 };
    uint32_t response_len = sizeof(response);

    if (!ensure_netlink_state(__FUNCTION__)) {
        return false;
    }

    /* ensure we're not in Operational mode */
    if (!ensure_operational_mode(false /* expected_operational */)) {
        return false;
    }

    /* get current mode and ensure its RX */
    if (!ensure_work_mode(__FUNCTION__, MODE_IMPL_RX)) {
        return false;
    }

    if (!send_wmi_ut_blocking(ut_module_drivers, (int)hwd_phy_rx_statistics_clear, STRINGER(hwd_phy_rx_statistics_clear),
                              &params, sizeof(params), &response, &response_len)) {
        return false;
    }
    
    return true;
}

/* Return RF gain lineup index for the current mode (TX / RX) */
bool RF_GetGainLineupIndex(uint8_t rfc_id, uint8_t gain_index, uint32_t* lineup_index_ptr, lineup_table_type_impl* table_type_ptr)
{
    struct wmi_hw_sysapi_rf_gain_lineup_read_cmd params = { rfc_id, 0, gain_index }; /* is_tx to be updated */
    struct wmi_hw_sysapi_rf_gain_lineup_read_event response = { 0 };
    uint32_t response_len = sizeof(response);
    work_mode_impl current_mode = MODE_IMPL_STANDBY;

    if (!ensure_netlink_state(__FUNCTION__)) {
        return false;
    }

    if ( !(lineup_index_ptr && table_type_ptr) ) { /* shouldn't happen */
        snprintf(s_operation_status, sizeof(s_operation_status), "%s: Invalid arguments", __FUNCTION__);
        return false;
    }

    /* get current mode and ensure its TX or RX */
    if (!BB_GetWorkMode(&current_mode)) {
        /* error was updated */
        return false;
    }

    if ( !(current_mode == MODE_IMPL_TX || current_mode == MODE_IMPL_RX) ) {
        snprintf(s_operation_status, sizeof(s_operation_status), "Cannot get RF gain lineup index when current work mode (%s) is not TX or RX", convert_work_mode_to_str(current_mode));
        return false;
    }

    /* set mode field in the parameters */
    params.is_tx = current_mode == MODE_IMPL_TX ? 1U : 0U;

    if (!send_wmi_ut_blocking(ut_module_system_api, (int)hw_sysapi_rf_gain_lineup_read, STRINGER(hw_sysapi_rf_gain_lineup_read),
                              &params, sizeof(params), &response, &response_len)) {
        return false;
    }

    *lineup_index_ptr = response.lineup_index;
    *table_type_ptr = (lineup_table_type_impl)response.lineup_table_type;

    return true;
}

/* Set RF gain lineup index for the given mode
 *
 * Should be used only in FTM Mode, otherwise will fail.
 */
bool RF_SetGainLineupIndex(uint8_t rfc_id, lineup_table_type_impl table_type, uint8_t gain_index, uint32_t lineup_index)
{
    struct wmi_hw_sysapi_rf_gain_lineup_config_cmd config_params = { rfc_id, (uint8_t)table_type,  gain_index, (uint16_t)lineup_index };
    struct wmi_hw_sysapi_rf_gain_lineup_config_event config_response = { 0 };
    uint32_t config_response_len = sizeof(config_response);

    struct wmi_hwd_rfc_rx_sectgain_on_cmd rx_gain_params = { rfc_id, 0, gain_index }; /* RF sector to be updated */
    struct wmi_hwd_rfc_rx_sectgain_on_event rx_gain_response = { 0 };
    uint32_t rx_gain_response_len = sizeof(rx_gain_response);

    struct wmi_hwd_rfc_tx_sectgain_on_cmd tx_gain_params = { rfc_id, 0, gain_index }; /* RF sector to be updated */
    struct wmi_hwd_rfc_tx_sectgain_on_event tx_gain_response = { 0 };
    uint32_t tx_gain_response_len = sizeof(tx_gain_response);

    uint32_t max_lineup_index = 0U;
    uint8_t current_sector_index = 0U;
    
    if (!ensure_netlink_state(__FUNCTION__)) {
        return false;
    }

    /* ensure we're not in Operational mode */
    if (!ensure_operational_mode(false /* expected_operational */)) {
        return false;
    }

    /* get max lineup for given mode */
    if (!RF_GetGainLineupIndexMaxRange(table_type, &max_lineup_index)) {
        /* error was updated */
        return false;
    }

    /* check lineup validity */
    if (lineup_index > max_lineup_index) { /* shouldn't happen */
        snprintf(s_operation_status, sizeof(s_operation_status), "%s: Failed to set RF gain lineup index %d. Lineup index should be in range [0,%d].", __FUNCTION__, lineup_index, max_lineup_index);
        return false;
    }

    /* read current sector index */
    if (!RF_GetSectorIndex(rfc_id, &current_sector_index)) {
        /* error was updated */
        return false;
    }
    
    if (!send_wmi_ut_blocking(ut_module_system_api, (int)hw_sysapi_rf_gain_lineup_config, STRINGER(hw_sysapi_rf_gain_lineup_config),
        &config_params, sizeof(config_params), &config_response, &config_response_len)) {
        return false;
    }

    switch (table_type)
    {
    case RX_G0_IMPL:
    case RX_G1_IMPL:
        /* update sector parameter, send hwd_rfc_rx_sectgain_on*/
        rx_gain_params.rf_sector = current_sector_index;
        if (!send_wmi_ut_blocking(ut_module_drivers, (int)hwd_rfc_rx_sectgain_on, STRINGER(hwd_rfc_rx_sectgain_on),
            &rx_gain_params, sizeof(rx_gain_params), &rx_gain_response, &rx_gain_response_len)) {
            return false;
        }
        break;
    case TX_IMPL:
        /* update sector parameter, send hwd_rfc_tx_sectgain_on*/
        tx_gain_params.rf_sector = current_sector_index;
        if (!send_wmi_ut_blocking(ut_module_drivers, (int)hwd_rfc_tx_sectgain_on, STRINGER(hwd_rfc_tx_sectgain_on),
            &tx_gain_params, sizeof(tx_gain_params), &tx_gain_response, &tx_gain_response_len)) {
            return false;
        }
        break;
    default:
        snprintf(s_operation_status, sizeof(s_operation_status), "Failed to set RF gain lineup index. Received unknown table type %d", (int)table_type);
        return false;
    }

    return true;
}

/* Get RF gain maximum lineup index for the given table type
 *
 * Should be used only in FTM Mode, otherwise will fail.
 */
bool RF_GetGainLineupIndexMaxRange(lineup_table_type_impl table_type, uint32_t* max_lineup_index_ptr)
{
    struct wmi_hw_sysapi_get_min_max_lineup_values_cmd params;
    struct wmi_hw_sysapi_get_min_max_lineup_values_event response = { 0 };
    uint32_t response_len = sizeof(response);

    if (!ensure_netlink_state(__FUNCTION__)) {
        return false;
    }

     /* ensure we're not in Operational mode */
    if (!ensure_operational_mode(false /* expected_operational */)) {
        return false;
    }

    if (!max_lineup_index_ptr) { /* shouldn't happen */
        snprintf(s_operation_status, sizeof(s_operation_status), "%s: Invalid arguments", __FUNCTION__);
        return false;
    }

    if (!send_wmi_ut_blocking(ut_module_system_api, (int)hw_sysapi_get_min_max_lineup_values, STRINGER(hw_sysapi_get_min_max_lineup_values),
        &params, sizeof(params), &response, &response_len)) {
        return false;
    }
    
    switch (table_type) {
    case(RX_G0_IMPL):
        *max_lineup_index_ptr = response.max_rx_g0_lineup;
        break;
    case(RX_G1_IMPL):
        *max_lineup_index_ptr = response.max_rx_g1_lineup;
        break;
    case(TX_IMPL):
        *max_lineup_index_ptr = response.max_tx_lineup;
        break;
    default:
        snprintf(s_operation_status, sizeof(s_operation_status), "Failed to get RF gain maximal lineup index. Received unknown table type %d", (int)table_type);
        return false;
    }

    return true;
}

/* Return current RF sector index
 *
 * Should be used only in FTM Mode, otherwise will fail.
 */
bool RF_GetSectorIndex(uint8_t rfc_id, uint8_t* sector_index_ptr)
{
    struct wmi_hw_sysapi_get_rf_status_event rf_status = { 0 };

    if (!ensure_netlink_state(__FUNCTION__)) {
        return false;
    }

    /* ensure we're not in Operational mode */
    if (!ensure_operational_mode(false /* expected_operational */)) {
        return false;
    }

    if (!sector_index_ptr) { /* shouldn't happen */
        snprintf(s_operation_status, sizeof(s_operation_status), "%s: Invalid arguments", __FUNCTION__);
        return false;
    }

    if (!get_rf_status(rfc_id, &rf_status)) {
        /* error was updated */
        return false;
    }

    /* sector index is stored as the first (LS) Byte of the RF status */
    *sector_index_ptr = (uint8_t)(rf_status.rf_current_status & 0x00FF);

    return true;
}

/* Set RF sector index for the current mode and gain index
 *
 * Should be used only in FTM Mode, otherwise will fail.
 */
bool RF_SetSectorIndex(uint8_t rfc_id, uint8_t sector_index)
{
    struct wmi_hwd_rfc_tx_sectgain_on_cmd tx_gain_params = { rfc_id, sector_index, 0 }; /* gain index to be replaced */
    struct wmi_hwd_rfc_tx_sectgain_on_event tx_gain_response = { 0 };
    uint32_t tx_gain_response_len = sizeof(tx_gain_response);
    struct wmi_hwd_rfc_rx_sectgain_on_cmd rx_gain_params = { rfc_id, sector_index, 0 }; /* gain index to be replaced */
    struct wmi_hwd_rfc_rx_sectgain_on_event rx_gain_response = { 0 };
    uint32_t rx_gain_response_len = sizeof(rx_gain_response);
    work_mode_impl current_mode = MODE_IMPL_STANDBY;
    struct wmi_hw_sysapi_get_rf_status_event rf_status = { 0 };
    uint8_t gain_index = 0;

    if (!ensure_netlink_state(__FUNCTION__)) {
        return false;
    }

    /* ensure we're not in Operational mode */
    if (!ensure_operational_mode(false /* expected_operational */)) {
        return false;
    }

    /* get current mode */
    if (!BB_GetWorkMode(&current_mode)) {
        /* error was updated */
        return false;
    }

    if (!get_rf_status(rfc_id, &rf_status)) {
        /* error was updated */
        return false;
    }

    // mask the status to extract gain index
    gain_index = (uint8_t)((rf_status.rf_current_status & 0x0F00) >> 8);

    switch (current_mode)
    {
    case MODE_IMPL_TX:
        tx_gain_params.rf_gain_idx = gain_index;
        if (!send_wmi_ut_blocking(ut_module_drivers, (int)hwd_rfc_tx_sectgain_on, STRINGER(hwd_rfc_tx_sectgain_on),
            &tx_gain_params, sizeof(tx_gain_params), &tx_gain_response, &tx_gain_response_len)) {
            return false;
        }
        break;
    case MODE_IMPL_RX:
        rx_gain_params.rf_gain_idx = gain_index;
        if (!send_wmi_ut_blocking(ut_module_drivers, (int)hwd_rfc_rx_sectgain_on, STRINGER(hwd_rfc_rx_sectgain_on),
            &rx_gain_params, sizeof(rx_gain_params), &rx_gain_response, &rx_gain_response_len)) {
            return false;
        }
        break;
    default:
        snprintf(s_operation_status, sizeof(s_operation_status), "Failed to set RF sector index. Current mode is %s. Expected TX or RX mode.", convert_work_mode_to_str(current_mode));
        return false;
    }

    return true;
}

/* Return masks of attached and enabled RF modules */
bool RF_GetAttachedEnabledMask(uint8_t* attached_rf_mask_ptr, uint8_t* enabled_rf_mask_ptr)
{
    struct wmi_hw_sysapi_get_bb_status_block_1_event bb_status = { 0 };

    if (!ensure_netlink_state(__FUNCTION__)) {
        return false;
    }

    if ( !(attached_rf_mask_ptr && enabled_rf_mask_ptr) ) { /* shouldn't happen */
        snprintf(s_operation_status, sizeof(s_operation_status), "%s: Invalid arguments", __FUNCTION__);
        return false;
    }

    if (!get_bb_status_1(&bb_status)) {
        /* error was updated */
        return false;
    }

    *attached_rf_mask_ptr = bb_status.attached_rf_vector;
    *enabled_rf_mask_ptr = bb_status.enabled_rf_vector;

    return true;
}

/* Return current MCS, payload length, silence duration and current transmission state
 * Only works when current work mode is TX
 *
 * Should be used only in FTM Mode, otherwise will fail.
 */
bool BB_GetPacketConfiguration(uint32_t* phy_tx_silence_duration_clk_cycles_ptr, uint8_t* mcs_index_ptr,
                               uint32_t* phy_tx_plcp_length_ptr, bool* is_transmission_active_ptr)
{
    struct wmi_hw_sysapi_get_bb_status_block_2_event bb_status = { 0 };

    if (!ensure_netlink_state(__FUNCTION__)) {
        return false;
    }

    /* ensure we're not in Operational mode */
    if (!ensure_operational_mode(false /* expected_operational */)) {
        return false;
    }

    if ( !(phy_tx_silence_duration_clk_cycles_ptr && mcs_index_ptr
           && phy_tx_plcp_length_ptr && is_transmission_active_ptr) ) { /* shouldn't happen */
        snprintf(s_operation_status, sizeof(s_operation_status), "%s: Invalid arguments", __FUNCTION__);
        return false;
    }

    /* get current mode and ensure its TX */
    if (!ensure_work_mode(__FUNCTION__, MODE_IMPL_TX)) {
        return false;
    }

    if (!get_bb_status_2(&bb_status)) {
        /* error was updated */
        return false;
    }

    *phy_tx_silence_duration_clk_cycles_ptr = bb_status.silence_between_packets;
    *mcs_index_ptr = bb_status.debug_packets_mcs;
    *phy_tx_plcp_length_ptr = bb_status.debug_packets_length;
    *is_transmission_active_ptr = bb_status.debug_packets_active == 0xFF; /* only 0xFF (255) means active */

    return true;
}

/* Start transmission with given parameters
 * Only works when current work mode is TX 
 *
 * Should be used only in FTM Mode, otherwise will fail.
 */
bool BB_StartPacket(uint32_t phy_tx_silence_duration_clk_cycles, uint8_t mcs_index, uint32_t phy_tx_plcp_length, int current_channel_number)
{
    /* {__le16 nFrames; __le32 phy_tx_silence_duration; u8 phy_tx_plcp_mcs; __le32 phy_tx_plcp_length; __le32 phy_tx_edmg_header_a0; __le32 phy_tx_mode;} */
    /* edmg_header will stay zero, phy_tx_mode to be updated */
    struct wmi_hwd_phy_tx_self_transmit_ext_cmd config_params = { 0xFFFF, phy_tx_silence_duration_clk_cycles, mcs_index, phy_tx_plcp_length, 0, 0 };

    if (!ensure_netlink_state(__FUNCTION__)) {
        return false;
    }

    /* ensure we're not in Operational mode */
    if (!ensure_operational_mode(false /* expected_operational */)) {
        return false;
    }

    /* get current mode and ensure its TX */
    if (!ensure_work_mode(__FUNCTION__, MODE_IMPL_TX)) {
        return false;
    }

    /* fetch current channel number if required */
    if (current_channel_number < 0) {
        uint32_t p_channel_number = 0U;
        if (!BB_GetChannel(&p_channel_number)) {
            /* error was updated */
            return false;
        }
        current_channel_number = (int)p_channel_number;
    }

    /* stop transmission (and check netlink interface is initialized) */
    if (!BB_StopPacket()) {
        /* error was updated */
        return false;
    }

    /* initialize the PHY TX mode */
    config_params.phy_tx_mode = current_channel_number < 9 ? DMG_CENTER_BAND : EDMG_CB2;

    if (!configure_packet_transmission(&config_params)) {
        /* error was updated */
        return false;
    }

    return true;
}

/* Stop transmission */
/* Only works when current work mode is TX */
bool BB_StopPacket()
{
    /* {__le16 nFrames; __le32 phy_tx_silence_duration; u8 phy_tx_plcp_mcs; __le32 phy_tx_plcp_length; __le32 phy_tx_edmg_header_a0; __le32 phy_tx_mode;} */
    /* phy_tx_silence_durationphy_tx_silence_duration, phy_tx_plcp_mcs and phy_tx_plcp_length will be updated */
    struct wmi_hwd_phy_tx_self_transmit_ext_cmd config_params = { 1, 0, 0, 0, 0, 0 };
    struct wmi_hw_sysapi_get_bb_status_block_2_event bb_status = { 0 };

    if (!ensure_netlink_state(__FUNCTION__)) {
        return false;
    }

    /* ensure we're not in Operational mode */
    if (!ensure_operational_mode(false /* expected_operational */)) {
        return false;
    }

    /* get current mode and ensure its TX */
    if (!ensure_work_mode(__FUNCTION__, MODE_IMPL_TX)) {
        return false;
    }

    if (!get_bb_status_2(&bb_status)) {
        /* error was updated */
        return false;
    }

    /* keep current silence duration, mcs and packet length */
    /* nFrames stays 1, phy_tx_edmg_header_a0 and phy_tx_mode stay zeros */
    config_params.phy_tx_silence_duration = bb_status.silence_between_packets;
    config_params.phy_tx_plcp_mcs = bb_status.debug_packets_mcs;
    config_params.phy_tx_plcp_length = bb_status.debug_packets_length;

    if (!configure_packet_transmission(&config_params)) {
        /* error was updated */
        return false;
    }

    return true;
}

/* Return FW and Board file information */
bool BB_GetFwBoardFileVersions(uint32_t* fw_version_major_ptr, uint32_t*  fw_version_minor_ptr,
                               uint32_t* fw_version_sub_minor_ptr, uint32_t* fw_version_build_ptr,
                               uint32_t* burned_board_file_ptr)
{
    struct wmi_hw_sysapi_get_bb_status_block_1_event bb_status = { 0 };

    if (!ensure_netlink_state(__FUNCTION__)) {
        return false;
    }

    if ( !(fw_version_major_ptr && fw_version_minor_ptr 
            && fw_version_sub_minor_ptr && fw_version_build_ptr && burned_board_file_ptr) ) { /* shouldn't happen */
        snprintf(s_operation_status, sizeof(s_operation_status), "%s: Invalid arguments", __FUNCTION__);
        return false;
    }

    if (!get_bb_status_1(&bb_status)) {
        /* error was updated */
        return false;
    }

    *fw_version_major_ptr = bb_status.fw_version_major;
    *fw_version_minor_ptr = bb_status.fw_version_minor;
    *fw_version_sub_minor_ptr = bb_status.fw_version_sub_minor;
    *fw_version_build_ptr = bb_status.fw_version_build;
    *burned_board_file_ptr = bb_status.burned_boardfile;

    return true;
}

/* Send ECHO command with the given value. This is a basic command for connectivity testing. */
bool BB_SendEchoCommand(uint32_t echo_val, uint32_t* echo_val_out_ptr)
{
    struct wmi_echo_cmd params = { echo_val };
    struct wmi_echo_rsp_event response = { 0 };
    uint32_t response_len = sizeof(response);

    if (!ensure_netlink_state(__FUNCTION__)) {
        return false;
    }

    if (!echo_val_out_ptr) { /* shouldn't happen */
        snprintf(s_operation_status, sizeof(s_operation_status), "%s: Invalid arguments", __FUNCTION__);
        return false;
    }

    if (!send_wmi_blocking(WMI_ECHO_CMDID, WMI_ECHO_RSP_EVENTID, STRINGER(WMI_ECHO_CMDID),
        &params, sizeof(params), &response, &response_len, false /* not dynamic */,
        true /* validate response status */)) {
        return false;
    }

    *echo_val_out_ptr = response.echoed_value;

    return true;
}

/* Get description of the last error */
const char* get_last_impl_error()
{
    return s_operation_status;
}

/* Clear last error description */
void clear_last_impl_error()
{
    s_operation_status[0] = '\0';
}

/***************************************************************************/
/*     Following APIs are intended for Operational (Network) mode only     */
/***************************************************************************/

/* Force MCS value */
bool BB_OperationalForceMcs(uint8_t force_mcs_enable, uint8_t mcs_index, int current_channel_number)
{
    /* sta_id (hard-coded zero assuming single STA mode), force_enable, mcs, use_cb2 (to be replaced) */
    struct wmi_hw_sysapi_force_mcs_cmd params = { 0, force_mcs_enable, mcs_index, 0 };
    struct wmi_hw_sysapi_force_mcs_event response = { 0 };
    uint32_t response_len = sizeof(response);

    /* ensure we're not in FTM mode and so netlink state */
    if (!ensure_operational_mode(true /* expected_operational */)) {
        return false;
    }

    /* fetch current channel number if required */
    if (current_channel_number < 0) {
        uint32_t p_channel_number = 0U;
        if (!BB_GetChannel(&p_channel_number)) {
            /* error was updated */
            return false;
        }
        current_channel_number = (int)p_channel_number;
    }

    /* set channel access mode according to the current channel */
    params.use_cb2 = current_channel_number < 9 ? 0 : 1;

    if (!send_wmi_ut_blocking(ut_module_system_api, (int)hw_sysapi_force_mcs, STRINGER(hw_sysapi_force_mcs),
                              &params, sizeof(params), &response, &response_len)) {
        return false;
    }

    return true;
}

/* Force TX sector index for corresponding RF */
bool BB_OperationalForceTxSector(uint16_t tx_sector_index)
{
    /* cid, sector_type, sector_idx */
    struct wmi_set_selected_rf_sector_index_cmd params = { 0 };
    struct wmi_set_selected_rf_sector_index_done_event response = { 0 };
    uint32_t response_len = sizeof(response);

    /* ensure we're not in FTM mode and so netlink state */
    if (!ensure_operational_mode(true /* expected_operational */)) {
        return false;
    }

    params.cid = 0; /* single STA mode */
    params.sector_type = WMI_RF_SECTOR_TYPE_TX; /* only TX type is supported */
    params.sector_idx = tx_sector_index;

    /* custom return status, do not validate */
    if (!send_wmi_blocking(WMI_SET_SELECTED_RF_SECTOR_INDEX_CMDID, WMI_SET_SELECTED_RF_SECTOR_INDEX_DONE_EVENTID,
                           STRINGER(WMI_SET_SELECTED_RF_SECTOR_INDEX_CMDID),
                           &params, sizeof(params), &response, &response_len, false /* not dynamic */,
                           false /* to ignore response status */)) {
        return false;
    }

    /* check the custom response status */
    if (response.status != WMI_RF_SECTOR_STATUS_SUCCESS) {
        snprintf(s_operation_status, sizeof(s_operation_status), "Failed to force TX sector index, response status is '%s'.",
                 convert_rf_sector_status_to_str(response.status));
        return false;
    }

    return true;
}

/* Return BF MCS, measured PER avarage [%], TX and RX goodput [Mbps] for the given time interval */
bool BB_OperationalGetStatistics(uint32_t interval_msec, 
                                 uint8_t* bf_mcs_ptr, uint8_t* per_average_ptr,
                                 uint32_t* tx_goodput_ptr, uint32_t* rx_goodput_ptr,
                                 uint8_t* tx_sector_index_ptr)
{    
    bool res = false;
    const uint32_t stats_record_sizeof = sizeof(struct wmi_link_stats_record);
    const uint32_t stats_basic_sizeof = sizeof(struct wmi_link_stats_basic);
    struct wmi_link_stats_record* stats_record_ptr = NULL;
    struct wmi_link_stats_basic* stats_basic_ptr = NULL;

    /* record_type_mask, cid, action, reserved[6], interval_msec */
    struct wmi_link_stats_cmd params = { 0 };
    uint32_t dynamic_response_len = sc_max_dynamic_response_len;
    struct wmi_link_stats_event* response_ptr = malloc(dynamic_response_len); /* dynamic payload, allocate maximum size */

    if (!response_ptr) {
        snprintf(s_operation_status, sizeof(s_operation_status), "%s: Cannot allocate a dynamic response of size %d Bytes",
                 __FUNCTION__, dynamic_response_len);
        return false;
    }

    /* ensure we're not in FTM mode and so netlink state */
    if (!ensure_operational_mode(true /* expected_operational */)) {
        goto out;
    }

    if ( !(bf_mcs_ptr && per_average_ptr && tx_goodput_ptr
           && rx_goodput_ptr && tx_sector_index_ptr)) { /* shouldn't happen */
        snprintf(s_operation_status, sizeof(s_operation_status), "%s: Invalid arguments", __FUNCTION__);
        goto out;
    }

    params.record_type_mask = (uint32_t)WMI_LINK_STATS_TYPE_BASIC;
    params.cid = 0xFF; /* 0xff for all cids */
    params.action = (uint8_t)WMI_LINK_STATS_SNAPSHOT;
    params.interval_msec = interval_msec;

    /* blocking wait for STATS event (and not CONFIG_DONE) */
    if (!send_wmi_blocking(WMI_LINK_STATS_CMDID, WMI_LINK_STATS_EVENTID, STRINGER(WMI_LINK_STATS_CMDID),
                           &params, sizeof(params), response_ptr, &dynamic_response_len, true /* dynamic */,
                           true /* validate response status */)) {
        goto out;
    }

    /* make sure the actual response doesn't exceed preallocated buffer length */
    if (response_ptr->payload_size > dynamic_response_len - sizeof(struct wmi_link_stats_event)) {
        snprintf(s_operation_status, sizeof(s_operation_status), "%s: not enough room for dymamic response, actual payload length is %d Bytes",
                 __FUNCTION__, response_ptr->payload_size);
        goto out;
    }

    /* sanity, expected at least size of header */
    if (response_ptr->payload_size < sizeof(struct wmi_link_stats_record)) { /* shouldn't happen */
        snprintf(s_operation_status, sizeof(s_operation_status), "%s: received %d Bytes of payload when expected at least %d",
                 __FUNCTION__, response_ptr->payload_size, stats_record_sizeof);
        goto out;
    }

    stats_record_ptr = (struct wmi_link_stats_record*)&response_ptr->payload;
    
    /* check it's the same stats type we requested */
    if (stats_record_ptr->record_type_id != (uint8_t)WMI_LINK_STATS_TYPE_BASIC) { /* shouldn't happen */
        snprintf(s_operation_status, sizeof(s_operation_status), "%s: received unexpected record type 0x%x when waiting for WMI_LINK_STATS_TYPE_BASIC (0x1)",
                 __FUNCTION__, stats_record_ptr->record_type_id);
        goto out;
    }

    if (stats_record_ptr->record_size < sizeof(struct wmi_link_stats_basic)) { /* shouldn't happen */
        snprintf(s_operation_status, sizeof(s_operation_status), "%s: received record of %d Bytes when expected at least %d",
                 __FUNCTION__, stats_record_ptr->record_size, stats_basic_sizeof);
        goto out;
    }

    stats_basic_ptr = (struct wmi_link_stats_basic*)&stats_record_ptr->record;

    *bf_mcs_ptr = stats_basic_ptr->bf_mcs;
    *per_average_ptr = stats_basic_ptr->per_average;
    *tx_goodput_ptr = stats_basic_ptr->tx_goodput;
    *rx_goodput_ptr = stats_basic_ptr->rx_goodput;
    *tx_sector_index_ptr = stats_basic_ptr->my_tx_sector;

    res = true;

out:
    free(response_ptr);
    return res;
}
