/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

/* Duplicate of mode enumeration from the API level, needed for internall usage */
typedef enum {
    MODE_IMPL_TX = 1,
    MODE_IMPL_RX = 2,
    MODE_IMPL_STANDBY = 4,
    MODE_IMPL_POWER_DOWN = 8,
    MODE_IMPL_LOOPBACK = 16
} work_mode_impl;

/* Lineup table type enumeration from the API level, needed for internall usage */
typedef enum {
    RX_G0_IMPL  = 0,
    RX_G1_IMPL  = 1,
    TX_IMPL     = 2
} lineup_table_type_impl;

/*
 * @brief Initialize netlink interface for given WIGIG interface name
 */
bool initialize_netlink_interface(const char* ifname);

/*
 * @brief Release current netlink interface
 */
bool release_netlink_interface();

/*
 * @brief Enables LO for all XIF ports
 */
bool BB_EnableLO();

/*
 * @brief Get currently set WIGIG Channel
 */
bool BB_GetChannel(uint32_t* channel_number);

/*
 * @brief Set WIGIG Channel
 * @remarks Should be used only in FTM Mode, otherwise will fail.
 */
bool BB_SetChannel(uint32_t channel_number);

/*
 * @brief Get currently set Baseband Work Mode
 */
bool BB_GetWorkMode(work_mode_impl* mode_impl);

/*
 * @brief Configure Baseband to a specific Work Mode
 * @remarks Supported modes are: TX, RX and STANDBY.
 *          XIF mask is ignored when work mode other than RX is requested.
 *          XIF mask of zeros means all XIFs are non-conductive.
 *          XIF mask of ones (0xFF) means all XIFs are conductive.
 *          Should be used only in FTM Mode, otherwise will fail.
 */
bool BB_SetWorkMode(work_mode_impl requested_mode, uint8_t xif_mask);

/*
 * @brief Return measured PER [%]
 * @remarks Only works when current work mode is RX
 *          Should be used only in FTM Mode, otherwise will fail.
 */
bool BB_GetPer(uint32_t number_packets_sent, double* per_ptr);

/*
 * @brief Clear PER Rx statistics
 * @remarks Only works when current work mode is RX
 *          Should be used only in FTM Mode, otherwise will fail.
 */
bool BB_ClearRxStatistics();

/*
 * @brief Return RF gain lineup index for the current mode
 * @remarks RF gain lineup index can be retrieved only when the current work mode is TX or RX
 */
bool RF_GetGainLineupIndex(uint8_t rfc_id, uint8_t gain_index, uint32_t* lineup_index_ptr, lineup_table_type_impl* table_type_ptr);

/* 
 * @brief Set RF gain lineup index for the given mode
 * @remarks Should be used only in FTM Mode, otherwise will fail.
 */
bool RF_SetGainLineupIndex(uint8_t rfc_id, lineup_table_type_impl table_type, uint8_t gain_index, uint32_t lineup_index);

/*
 * @brief Get RF gain maximum lineup index for the given table type
 * @remarks Minimum index values is always zero
 *          Should be used only in FTM Mode, otherwise will fail.
 */
bool RF_GetGainLineupIndexMaxRange(lineup_table_type_impl table_type, uint32_t* max_lineup_index_ptr);

/*
 * @brief Return current RF sector index
 * @remarks Should be used only in FTM Mode, otherwise will fail.
 */
bool RF_GetSectorIndex(uint8_t rfc_id, uint8_t* sector_index_ptr);

/*
 * @brief Set RF sector index for the current mode and gain index
 * @remarks RF sector index can be changed only when current work mode is TX or RX
 *          Should be used only in FTM Mode, otherwise will fail.
 */
bool RF_SetSectorIndex(uint8_t rfc_id, uint8_t sector_index);

/*
 * @brief Return masks of attached and enabled RF modules
 */
bool RF_GetAttachedEnabledMask(uint8_t* attached_rf_mask_ptr, uint8_t* enabled_rf_mask_ptr);

/*
 * @brief Return current MCS, payload length, silence duration and current transmission state
 * @remarks Only works when current work mode is TX
 *          Should be used only in FTM Mode, otherwise will fail.
 */
bool BB_GetPacketConfiguration(uint32_t* phy_tx_silence_duration_clk_cycles_ptr, uint8_t* mcs_index_ptr,
                               uint32_t* phy_tx_plcp_length_ptr, bool* is_transmission_active_ptr);

/*
 * @brief Start transmission with given parameters
 * @remarks current_channel_number parameter may be passed as -1 when unknown.
 *          This case it will be queried as part of the flow.
 *          Only works when current work mode is TX.
 *          Should be used only in FTM Mode, otherwise will fail.
 */
bool BB_StartPacket(uint32_t phy_tx_silence_duration_clk_cycles, uint8_t mcs_index,
                    uint32_t phy_tx_plcp_length, int current_channel_number);

/*
 * @brief Stop transmission
 * @remarks Only works when current work mode is TX
 *          Should be used only in FTM Mode, otherwise will fail.
 */
bool BB_StopPacket();

/*
 * @brief Return FW and Board file information
 */
bool BB_GetFwBoardFileVersions(uint32_t* fw_version_major_ptr, uint32_t*  fw_version_minor_ptr,
                               uint32_t* fw_version_sub_minor_ptr, uint32_t* fw_version_build_ptr,
                               uint32_t* burned_board_file_ptr); 

/*
 * @brief Send ECHO command with the given value. This is a basic command for connectivity testing.
 * @remarks Returned output value should be identical to the send one.
 */
bool BB_SendEchoCommand(uint32_t echo_val, uint32_t* echo_val_out_ptr);

/*
 * @brief Get description of the last error
 */
const char* get_last_impl_error();

/*
 * @brief Clear last error description
 */
void clear_last_impl_error();

/***************************************************************************/
/*     Following APIs are intended for Operational (Network) mode only     */
/***************************************************************************/

/*
 * @brief Force MCS value
 * @param[in] force_mcs_enable 1 to enforce requested MCS value, 0 for auto MCS.
 * @remarks Should be used only in Operational Mode, otherwise will fail.
 *          Current channel number parameter may be passed as -1 when unknown.
 *          Channel Access Mode is set according to the current WIGIG channel.
 *          Assumed point to point setup, a.k.a. single STA mode.
 */
bool BB_OperationalForceMcs(uint8_t force_mcs_enable, uint8_t mcs_index, int current_channel_number);

/*
 * @brief Force TX sector index for corresponding RF
 * @param[in] tx_sector_index sector index to enforce in range [0, 127] or 0xFFFF to cancel enforcement.
 * @remarks Should be used only in Operational Mode, otherwise will fail.
 */
bool BB_OperationalForceTxSector(uint16_t tx_sector_index);

/*
 * @brief Return BF MCS, measured PER avarage [%], TX and RX goodput [Mbps] for the given time interval
 * @remark Should be used only in Operational Mode, otherwise will fail.
 *         The call is blocking and returns after the requested time interval.
 */
bool BB_OperationalGetStatistics(uint32_t interval_msec,
                                 uint8_t* bf_mcs_ptr, uint8_t* per_average_ptr,
                                 uint32_t* tx_goodput_ptr, uint32_t* rx_goodput_ptr,
                                 uint8_t* tx_sector_index_ptr);
