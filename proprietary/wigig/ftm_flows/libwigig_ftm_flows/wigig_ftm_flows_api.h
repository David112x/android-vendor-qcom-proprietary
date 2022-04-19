/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

/*
 * FTM Flow Library provides an API to control 11ad/11ay chip operation on production line environment.
 * This library is not reentrant and is intended to work simultaneously with a single WIGIG interface.
 * Library output is sent by default to the syslog and can be redirected to the standard output upon request.
 * Only errors are reported by default and a verbose logging can be enabled upon request.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* Enumeration of supported channel numbers */
typedef enum {
    WIGIG_CHANNEL_1  = 1,
    WIGIG_CHANNEL_2  = 2,
    WIGIG_CHANNEL_3  = 3,
    WIGIG_CHANNEL_4  = 4,
    WIGIG_CHANNEL_9  = 9,
    WIGIG_CHANNEL_10 = 10,
    WIGIG_CHANNEL_11 = 11
} wigig_channel;

/* Baseband work mode enumeration */
typedef enum {
    MODE_TX         = 1,
    MODE_RX         = 2,
    MODE_STANDBY    = 4,
    MODE_POWER_DOWN = 8,
    MODE_LOOPBACK   = 16
} work_mode;

/* Channel Access mode enumeration */
typedef enum {
    DMG = 0,
    EDMG
} channel_access_mode;

/* Transmission mode enumeration */
typedef enum {
    INACTIVE,
    ACTIVE
} transmission_mode;

/* Supported MCS enumeration */
typedef enum {
    MCS_0 = 0,
    MCS_1,
    MCS_2,
    MCS_3,
    MCS_4,
    MCS_5,
    MCS_6,
    MCS_7,
    MCS_8,
    MCS_9,
    MCS_10,
    MCS_11,
    MCS_12,
    MCS_13,
    MCS_14,
    MCS_15
} mcs_index;

/* Lineup table type enumeration */
typedef enum {
    RX_G0 = 0,
    RX_G1 = 1,
    TX    = 2
} lineup_table_type;

/* Enumeration of supported destinations for logging */
typedef enum {
    SYSLOG, /* default */
    STDOUT
} log_destination_type;

/* Enumeration for log verbosity */
typedef enum {
    LOG_ERROR_ONLY, /*default */
    LOG_ALL
} log_verbosity_type;

/* Enumeration used to enable/disable MCS enforcement in the Operational Mode */
typedef enum {
    AUTO_MCS,
    FORCE_MCS
} mcs_enforcement_mode;

/*
 * @brief Perform initialization required to communicate with given WIGIG interface
 * @param[in] ifname the interface name to be used during FTM flows operation
 * @remarks Only single connection may be active simultaneously.
 *          When called more than once, the active connection is released.
 *          Side effect - enables LO for all XIF ports.
 * @returns 0 on success or a negative error code.
 */
int wigig_ftm_flows_initialize_connection(const char* ifname);

/*
 * @brief Release connection with current WIGIG interface
 */
void wigig_ftm_flows_release_connection();

/*
 * @breif Configure FTM Flows output destination - standard output or syslog (default)
 * @remark syslog content can be retrieved by "logcat | grep _process_name_" command
 */
void wigig_ftm_flows_configure_output_sink(log_destination_type log_destination);

/*
 * @breif Configure FTM Flows log verbosity - all levels of verbosity or errors only (default)
 */
void wigig_ftm_flows_configure_log_verbosity(log_verbosity_type log_verbosity);

/*
 * @brief Return currently set WIGIG Channel
 * @returns 0 on success or a negative error code.
 */
int wigig_ftm_flows_get_channel(wigig_channel* channel_number_ptr);

/*
 * @brief Set WIGIG Channel
 * @remarks Baseband Work Mode will be changed to Standby.
 *          Should be used only in FTM Mode, otherwise will fail.
 * @returns 0 on success or a negative error code.
 */
int wigig_ftm_flows_set_channel(wigig_channel channel_number);

/*
 * @brief Get currently set Baseband Work Mode
 * @remark Work Mode may be one of the following: TX, RX, STANDBY, POWER_DOWN and LOOPBACK.
 * @returns 0 on success or a negative error code.
 */
int wigig_ftm_flows_get_work_mode(work_mode* mode_ptr);

/*
 * @brief Configure Baseband to a specific Work Mode
 * @remarks This flow configures BB to a specific work_mode and performs internal calibration
 *          Supported modes are: TX, RX, and STANDBY.
 *          XIF mask is ignored when work mode other than RX is requested.
 *          XIF mask of zeros means all XIFs are non-conductive.
 *          XIF mask of ones (0xFF) means all XIFs are conductive.
 *          Should be used only in FTM Mode, otherwise will fail.
 * @returns 0 on success or a negative error code.
 */
int wigig_ftm_flows_set_work_mode(work_mode requested_mode, uint8_t xif_mask);

/*
 * @brief Return measured PER [%]
 * @remark Only works when current work mode is RX.
 *         Should be used only in FTM Mode, otherwise will fail.
 * @returns 0 on success or a negative error code.
 */
int wigig_ftm_flows_get_per(uint32_t number_packets_sent, double* per_ptr);

/*
 * @brief Clear PER RX statistics
 * @remark Only works when current work mode is RX.
 *         Should be used only in FTM Mode, otherwise will fail.
 * @returns 0 on success or a negative error code.
 */
int wigig_ftm_flows_clear_per();

/*
 * @brief Return RF gain lineup index for the current mode
 * @remarks RF gain lineup index can be retrieved only when the current work mode is TX or RX
 *          RFC id can be in range [0, 7]
 *          Gain index can be in range [0, 15]
 *          Only initialized gains can be retrieved (first 4 gain index are initialized by the FW)
 *          Flow will fail when the current mode is not TX or RX
 * @returns 0 on success or a negative error code.
 */
int wigig_ftm_flows_get_rf_gain_lineup_index(uint8_t rfc_id, uint8_t gain_index, uint32_t* lineup_index_ptr, lineup_table_type* table_type_ptr);

/*
 * @brief Set RF gain lineup index for the given mode
 * @remarks RFC id can be in range [0, 7].
 *          Gain index can be in range [0, 15].
 *          Lineup index can be in range [0, value returned by call to get_rf_gain_maximum_lineup_index].
 *          Should be used only in FTM Mode, otherwise will fail.
 * @returns 0 on success or a negative error code.
 */
int wigig_ftm_flows_set_rf_gain_lineup_index(uint8_t rfc_id, lineup_table_type table_type, uint8_t gain_index, uint32_t lineup_index);

/*
 * @brief Get RF gain maximum lineup index for the given table type
 * @remarks Minimum index values is always zero.
 *          Maximum possible index value is 255.
 *          Should be used only in FTM Mode, otherwise will fail.
 * @returns 0 on success or a negative error code.
 */
int wigig_ftm_flows_get_rf_gain_maximum_lineup_index(lineup_table_type table_type, uint32_t* max_lineup_index_ptr);

/*
 * @brief Return current RF sector index
 * @remarks RFC id can be in range [0, 7].
 *          RF sector index range is [0, 127].
 *          Should be used only in FTM Mode, otherwise will fail.
 * @returns 0 on success or a negative error code.
 */
int wigig_ftm_flows_get_rf_sector_index(uint8_t rfc_id, uint8_t* sector_index_ptr);

/*
 * @brief Set RF sector index for the current mode and gain index
 * @remarks RF sector can be changed only when the current work mode is TX or RX
 *          RFC id can be in range [0, 7].
 *          RF sector index range is [0, 127].
 *          Should be used only in FTM Mode, otherwise will fail.
 * @returns 0 on success or a negative error code.
 */
int  wigig_ftm_flows_set_rf_sector_index(uint8_t rfc_id, uint8_t sector_index);


/*
 * @brief Return masks of attached and enabled RF modules
 * @returns 0 on success or a negative error code.
 */
int wigig_ftm_flows_get_attached_enabled_rf_mask(uint8_t* attached_rf_mask_ptr, uint8_t* enabled_rf_mask_ptr);

/*
 * @brief Return current MCS (channel access mode and mcs index), payload length, silence duration and current transmission state
 * @remark Only works when current work mode is TX.
 *         Should be used only in FTM Mode, otherwise will fail.
 * @returns 0 on success or a negative error code.
 */
int wigig_ftm_flows_get_packet_configuration(uint32_t* phy_tx_silence_duration_clk_cycles_ptr,
                                             channel_access_mode* channel_access_mode_ptr, mcs_index* mcs_index_ptr,
                                             uint32_t* phy_tx_plcp_length_ptr, transmission_mode* transmission_mode_ptr);

/*
 * @brief Start transmission with given parameters
 * @remarks Only works when current work mode is TX. 
 *          PHY TX silence clock runs at 330MHz (~3.03 nsec).
 *          Only DMG channel access mode can be used with WIGIG channels 1-4.
 *          Only EDMG channel access mode can be used with WIGIG channels 9-11.
 *          Only MCS 0-12 are allowed for DMG channel access.
 *          Only MCS 0-15 are allowed for EDMG channel access.
 *          Current transmission (if any) is stopped before starting new transmission with given parameters.
 *          Should be used only in FTM Mode, otherwise will fail.
 * @returns 0 on success or a negative error code.
 */
int wigig_ftm_flows_start_packet(uint32_t phy_tx_silence_duration_clk_cycles,
                                 channel_access_mode channel_access_mode, mcs_index mcs_index,
                                 uint32_t phy_tx_plcp_length);

/*
 * @brief Stop transmission
 * @remark Only works when current work mode is TX.
 *         Should be used only in FTM Mode, otherwise will fail.
 * @returns 0 on success or a negative error code.
 */
int wigig_ftm_flows_stop_packet();

/*
 * @brief Return FW and Board file information
 * @returns 0 on success or a negative error code.
 */
int wigig_ftm_flows_get_fw_brd_versions(uint32_t* fw_version_major_ptr, uint32_t*  fw_version_minor_ptr,
                                        uint32_t* fw_version_sub_minor_ptr, uint32_t* fw_version_build_ptr,
                                        uint32_t* burned_board_file_ptr);

/*
 * @brief Send ECHO command with the given value. This is a basic command for connectivity testing.
 * @remarks Returned output value should be identical to the send one.
 */
int wigig_flows_send_echo(uint32_t echo_val, uint32_t* echo_val_out_ptr);

/*
 * @brief Return FTM Flows library version in format Major.Minor.Maintenance.Build
 */
const char* wigig_ftm_flows_get_version();

/*
 * @brief Get description of the last library error
 */
const char* wigig_ftm_flows_get_last_error(); 

/***************************************************************************/
/*     Following APIs are intended for Operational (Network) mode only     */
/***************************************************************************/

/*
 * @brief Force MCS value
 * @param[in] mcs_enforcement FORCE_MCS to enforce requested MCS value, AUTO_MCS to cancel MCS enforcement.
 * @remarks Should be used only in Operational Mode, otherwise will fail.
 *          Channel Access Mode is set according to the current WIGIG channel.
 *          Assumed point to point setup, a.k.a. single STA mode.
 * @returns 0 on success or a negative error code.
 */
int wigig_operational_flows_force_mcs(mcs_enforcement_mode mcs_enforcement, mcs_index mcs_index);

/*
 * @brief Force TX sector index for corresponding RF
 * @param[in] tx_sector_index sector index to enforce in range [0, 127] or 0xFFFF to cancel enforcement.
 * @remarks Should be used only in Operational Mode, otherwise will fail.
 *          RF sector index may be in range [0, 127] or equal to 0xFFFF.
 * @returns 0 on success or a negative error code.
 */
int wigig_operational_flows_force_tx_sector_index(uint16_t tx_sector_index);

/*
 * @brief Return BF MCS, measured PER avarage [%], TX and RX goodput [Mbps] for the given time interval
 * @remark Should be used only in Operational Mode, otherwise will fail.
 *         The call is blocking and returns after the requested time interval.
 *         Time interval can be in range [100, 4500] msec.
 * @returns 0 on success or a negative error code.
 */
int wigig_operational_flows_get_statistics(uint32_t interval_msec,
                                           uint8_t* bf_mcs_ptr, uint8_t* per_average_ptr,
                                           uint32_t* tx_goodput_ptr, uint32_t* rx_goodput_ptr,
                                           uint8_t* tx_sector_index_ptr);

#ifdef __cplusplus
}
#endif
