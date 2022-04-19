/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 *
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

/*
 * This file contains the definitions of the WMI protocol specified in the
 * Wireless Module Interface (WMI) for the Qualcomm Technologies, Inc
 * 60 GHz wireless solution.
 * It includes definitions of all the commands and events.
 * Commands are messages from the host to the WM.
 * Events are messages from the WM to the host.
 *
 * This is an automatically generated file.
 * Was edited to strip unused commands and update field types to __le32
 */

#pragma once

/* List of UT modules with corresponding id */
enum wmi_ut_module {
    ut_module_flows_operational = 0x9,
    ut_module_modes             = 0xb,
    ut_module_drivers           = 0xc,
    ut_module_system_api        = 0xa,
};

/* Drivers Commands */
enum wmi_ut_module_drivers_cmd {
    hwd_rfc_tx_sectgain_on          = 0x508,
    hwd_rfc_rx_sectgain_on          = 0x507,
    hwd_phy_tx_self_transmit_ext    = 0x43A,
    hwd_phy_rgf_tx_en               = 0x406,
    hwd_phy_self_rx_mode_entry      = 0x402,
    hwd_phy_rgf_rx_en               = 0x410,
    hwd_phy_rx_ina_det_mode         = 0x411,
    hwd_phy_rx_statistics_clear     = 0x421,
    hwd_phy_rx_get_statistics       = 0x422,
};

/* Flows Commands */
enum wmi_ut_module_flows_operational_cmd {
    hwf_calib_all               = 0x700,
};

/* Modes Commands */
enum wmi_ut_module_modes_cmd {
    hwm_analog_channel_switch   = 0x200,
    hwm_analog_lo_mux_enable    = 0x209,
    hwm_analog_if_loopback_ext  = 0x20a,
};

/* System API Commands */
enum wmi_ut_module_system_api_cmd {
    hw_sysapi_force_mcs                 = 0x6,
    hw_sysapi_get_rf_status             = 0x10,
    hw_sysapi_get_bb_status_block_1     = 0x11,
    hw_sysapi_get_bb_status_block_2     = 0x12,
    hw_sysapi_enter_tx_mode_ext         = 0x13,
    hw_sysapi_enter_rx_mode_ext         = 0x14,
    hw_sysapi_rf_gain_lineup_config     = 0x45,
    hw_sysapi_rf_gain_lineup_read       = 0x46,
    hw_sysapi_get_min_max_lineup_values = 0x4e,
};

/* Commands and Events */

/* hw_sysapi_get_bb_status_block_1 */
struct wmi_hw_sysapi_get_bb_status_block_1_cmd {
} __packed;

/* hw_sysapi_get_bb_status_block_1 */
struct wmi_hw_sysapi_get_bb_status_block_1_event {
    __le32 res;
    __le32 fw_version_major;
    __le32 fw_version_minor;
    __le32 fw_version_sub_minor;
    __le32 fw_version_build;
    __le32 bootloader_version;
    __le16 bb_revision;
    __le16 rf_revision;
    __le32 burned_boardfile;
    __le32 fw_type;
    __le32 rf_status;
    __le32 fw_status;
    __le32 channel;
    __le32 work_mode;
    u8 attached_rf_vector;
    u8 enabled_rf_vector;
    u8 xif_gc;
    u8 gc_ctrl;
    u8 stg2_ctrl;
    u8 reserved1[3];
} __packed;

/* hw_sysapi_get_bb_status_block_2 */
struct wmi_hw_sysapi_get_bb_status_block_2_cmd {
} __packed;

/* hw_sysapi_get_bb_status_block_2 */
struct wmi_hw_sysapi_get_bb_status_block_2_event {
    __le32 res;
    __le32 m_tx_params;
    u8 tx_digial_atten_i;
    u8 tx_digial_atten_q;
    u8 reserved1[2];
    __le32 lo_leak_correction;
    u8 tx_gain_row;
    u8 tx_gain_row_force_enable;
    u8 tx_gain_row_force_value;
    u8 reserved2;
    __le32 tx_swap_iq;
    u8 debug_packets_active;
    u8 reserved3;
    __le16 num_debug_packets;
    u8 debug_packets_mcs;
    u8 reserved4[3];
    __le32 debug_packets_length;
    __le32 silence_between_packets;
    u8 tone_transmitted;
    u8 tone_mode;
    u8 tone_max_value;
    u8 reserved5;
    __le16 tone_frequency_a;
    __le16 tone_frequency_b;
    __le32 m_rx_params;
    u8 zero_db_row;
    u8 agc_start_row;
    u8 agc_final_row;
    u8 rx_gain_row_force_enable;
    u8 rx_gain_row_force_value;
    u8 reserved6[3];
    __le32 digital_post_channel_corrections;
    __le32 digital_iq_correction_w1;
    __le32 digital_iq_correction_w2;
    u8 tx_atten_manual_enable;
    u8 tx_atten_cafe_enable;
} __packed;

/* hwd_phy_rx_get_statistics */
struct wmi_hwd_phy_rx_get_statistics_cmd {
} __packed;

/* hwd_phy_rx_get_statistics */
struct wmi_hwd_phy_rx_get_statistics_event {
    __le32 res;
    __le32 ina_sync_counter_dp;
    __le32 sfd_sync_counter_sc;
    __le32 sfd_timeout_counter_dp;
    __le32 crc_ok_counter_sc;
    __le32 crc_error_counter_sc;
    __le32 ber_last_zero_counter_sc;
    __le32 ber_last_non_zero_counter_sc;
    __le32 ina_sync_counter_cp;
    __le32 sfd_sync_counter_cp;
    __le32 sfd_timeout_counter_cp;
    __le32 crc_ok_counter_cp;
    __le32 crc_error_counter_cp;
    __le32 ber_last_zero_counter_cp;
    __le32 ber_last_non_zero_counter_cp;
} __packed;

/* hwd_phy_tx_self_transmit_ext */
/*
struct wmi_hwd_phy_tx_self_transmit_ext_cmd {
    __le16 nFrames;
    __le32 phy_tx_silence_duration;
    u8 phy_tx_plcp_mcs;
    __le32 phy_tx_plcp_length;
    __le32 phy_tx_edmg_header_a0;
    __le32 phy_tx_mode;
} __packed;
*/
struct wmi_hwd_phy_tx_self_transmit_ext_cmd {
    __le32 nFrames;
    __le32 phy_tx_silence_duration;
    __le32 phy_tx_plcp_mcs;
    __le32 phy_tx_plcp_length;
    __le32 phy_tx_edmg_header_a0;
    __le32 phy_tx_mode;
} __packed;

/* hwd_phy_tx_self_transmit_ext */
struct wmi_hwd_phy_tx_self_transmit_ext_event {
    __le32 res;
} __packed;

/* hwm_analog_channel_switch */
struct wmi_hwm_analog_channel_switch_cmd {
    __le32 fs_tune_channel_e;
} __packed;

/* hwm_analog_channel_switch */
struct wmi_hwm_analog_channel_switch_event {
    __le32 res;
} __packed;

/* hw_sysapi_enter_tx_mode_ext */
/*
struct wmi_hw_sysapi_enter_tx_mode_ext_cmd {
    __le32 enable;
    u8 rf_sector_id;
    u8 rf_gain_index;
    u8 tx_bb_gain_row_num;
    u8 mcs;
    u8 xif_port_bitmask;
    __le32 reserved;
} __packed;
*/
struct wmi_hw_sysapi_enter_tx_mode_ext_cmd {
    __le32 enable;
    __le32 rf_sector_id;
    __le32 rf_gain_index;
    __le32 tx_bb_gain_row_num;
    __le32 mcs;
    __le32 xif_port_bitmask;
    __le32 reserved;
} __packed;

/* hw_sysapi_enter_tx_mode_ext */
struct wmi_hw_sysapi_enter_tx_mode_ext_event {
    __le32 res;
} __packed;

/* hw_sysapi_enter_rx_mode_ext */
/*
struct wmi_hw_sysapi_enter_rx_mode_ext_cmd {
    __le32 enable;
    __le32 no_detection_mode;
    u8 rf_sector_id;
    u8 rf_gain_index;
    u8 rx_bb_gain_row_num;
    u8 xif_port_bitmask;
    __le32 reserved;
} __packed;
*/
struct wmi_hw_sysapi_enter_rx_mode_ext_cmd {
    __le32 enable;
    __le32 no_detection_mode;
    __le32 rf_sector_id;
    __le32 rf_gain_index;
    __le32 rx_bb_gain_row_num;
    __le32 xif_port_bitmask;
    __le32 reserved;
} __packed;

/* hw_sysapi_enter_rx_mode_ext */
struct wmi_hw_sysapi_enter_rx_mode_ext_event {
    __le32 res;
} __packed;

/* hwm_analog_lo_mux_enable */
/*
struct wmi_hwm_analog_lo_mux_enable_cmd {
    u8 xif;
} __packed;
*/
struct wmi_hwm_analog_lo_mux_enable_cmd {
    __le32 xif;
} __packed;

/* hwm_analog_lo_mux_enable */
struct wmi_hwm_analog_lo_mux_enable_event {
    __le32 res;
} __packed;

/* hwm_analog_if_loopback_ext */
/*
struct wmi_hwm_analog_if_loopback_ext_cmd {
    __le32 enable;
    __le32 abif_common_if_switch_cfg_e;
    u8 tx_gain_index;
    u8 rx_gain_index;
    u8 xif_port;
} __packed;
*/
struct wmi_hwm_analog_if_loopback_ext_cmd {
    __le32 enable;
    __le32 abif_common_if_switch_cfg_e;
    __le32 tx_gain_index;
    __le32 rx_gain_index;
    __le32 xif_port;
} __packed;

/* hwm_analog_if_loopback_ext */
struct wmi_hwm_analog_if_loopback_ext_event {
    __le32 res;
} __packed;

/* hwf_calib_all */
struct wmi_hwf_calib_all_cmd {
} __packed;

/* hwf_calib_all */
struct wmi_hwf_calib_all_event {
    __le32 res;
} __packed;

/* hwd_phy_self_rx_mode_entry */
struct wmi_hwd_phy_self_rx_mode_entry_cmd {
} __packed;

/* hwd_phy_self_rx_mode_entry */
struct wmi_hwd_phy_self_rx_mode_entry_event {
    __le32 res;
} __packed;

/* hwd_phy_rgf_rx_en */
struct wmi_hwd_phy_rgf_rx_en_cmd {
    __le32 enable;
} __packed;

/* hwd_phy_rgf_rx_en */
struct wmi_hwd_phy_rgf_rx_en_event {
    __le32 res;
} __packed;

/* hwd_phy_rgf_tx_en */
struct wmi_hwd_phy_rgf_tx_en_cmd {
    __le32 enable;
} __packed;

/* hwd_phy_rgf_tx_en */
struct wmi_hwd_phy_rgf_tx_en_event {
    __le32 res;
} __packed;

/* hwd_phy_rx_statistics_clear */
struct wmi_hwd_phy_rx_statistics_clear_cmd {
} __packed;

/* hwd_phy_rx_statistics_clear */
struct wmi_hwd_phy_rx_statistics_clear_event {
    __le32 res;
} __packed;

/* hw_sysapi_rf_gain_lineup_read */
/*
struct wmi_hw_sysapi_rf_gain_lineup_read_cmd {
    u8 rfc_id;
    __le32 is_tx;
    u8 hw_table_index;
} __packed;
*/
struct wmi_hw_sysapi_rf_gain_lineup_read_cmd {
    __le32 rfc_id;
    __le32 is_tx;
    __le32 hw_table_index;
} __packed;

/* hw_sysapi_rf_gain_lineup_read */
struct wmi_hw_sysapi_rf_gain_lineup_read_event {
    __le32 res;
    __le32 lineup_index;
    u8 lineup_table_type;
} __packed;

/* hw_sysapi_rf_gain_lineup_config */
/*
struct wmi_hw_sysapi_rf_gain_lineup_config_cmd {
    u8 rfc_id;
    u8 lineup_table_type;
    u8 hw_table_index;
    __le16 lineup_index;
} __packed;
*/
struct wmi_hw_sysapi_rf_gain_lineup_config_cmd {
    __le32 rfc_id;
    __le32 lineup_table_type;
    __le32 hw_table_index;
    __le32 lineup_index;
} __packed;

/* hw_sysapi_rf_gain_lineup_config */
struct wmi_hw_sysapi_rf_gain_lineup_config_event {
    __le32 res;
} __packed;

/* hwd_rfc_rx_sectgain_on */
/*
struct wmi_hwd_rfc_rx_sectgain_on_cmd {
    u8 rf_id;
    u8 rf_sector;
    u8 rf_gain_idx;
} __packed;
*/
struct wmi_hwd_rfc_rx_sectgain_on_cmd {
    __le32 rf_id;
    __le32 rf_sector;
    __le32 rf_gain_idx;
} __packed;

/* hwd_rfc_rx_sectgain_on */
struct wmi_hwd_rfc_rx_sectgain_on_event {
    __le32 res;
} __packed;

/* hwd_rfc_tx_sectgain_on */
/*
struct wmi_hwd_rfc_tx_sectgain_on_cmd {
    u8 rf_id;
    u8 rf_sector;
    u8 rf_gain_idx;
} __packed;
*/
struct wmi_hwd_rfc_tx_sectgain_on_cmd {
    __le32 rf_id;
    __le32 rf_sector;
    __le32 rf_gain_idx;
} __packed;

/* hwd_rfc_tx_sectgain_on */
struct wmi_hwd_rfc_tx_sectgain_on_event {
    __le32 res;
} __packed;

/* CMD: hw_sysapi_get_min_max_lineup_values */
struct wmi_hw_sysapi_get_min_max_lineup_values_cmd {
} __packed;

/* EVENT: hw_sysapi_get_min_max_lineup_values */
struct wmi_hw_sysapi_get_min_max_lineup_values_event {
    __le32 res;
    __le32 min_tx_lineup;
    __le32 max_tx_lineup;
    __le32 min_rx_g0_lineup;
    __le32 max_rx_g0_lineup;
    __le32 min_rx_g1_lineup;
    __le32 max_rx_g1_lineup;
} __packed;

/* hw_sysapi_force_mcs */
/*
struct wmi_hw_sysapi_force_mcs_cmd {
    u8 sta_id;
    u8 force_enable;
    u8 mcs;
    u8 use_cb2;
} __packed;
*/
struct wmi_hw_sysapi_force_mcs_cmd {
    __le32 sta_id;
    __le32 force_enable;
    __le32 mcs;
    __le32 use_cb2;
} __packed;

/* hw_sysapi_force_mcs */
struct wmi_hw_sysapi_force_mcs_event {
    __le32 res;
} __packed;

/* hw_sysapi_get_rf_status */
/*
struct wmi_hw_sysapi_get_rf_status_cmd {
    u8 rfc_id;
} __packed;
*/
struct wmi_hw_sysapi_get_rf_status_cmd {
    __le32 rfc_id;
} __packed;

/* hw_sysapi_get_rf_status */
struct wmi_hw_sysapi_get_rf_status_event {
    __le32 res;
    __le32 rfc_id;
    __le32 rf_revision;
    __le32 burned_boardfile;
    __le32 rf_status;
    __le32 rf_current_status;
    __le32 current_sector_1;
    __le32 current_sector_2;
    __le32 current_sector_3;
    __le32 current_sector_4;
    __le32 current_sector_5;
    __le32 current_sector_6;
    __le32 lo_current_status_1;
    __le32 lo_current_status_2;
    __le32 lo_current_status_3;
} __packed;

/* hwd_phy_rx_ina_det_mode */
struct wmi_hwd_phy_rx_ina_det_mode_cmd {
    __le32 ina_det_en_e;
} __packed;

/* hwd_phy_rx_ina_det_mode */
struct wmi_hwd_phy_rx_ina_det_mode_event {
    __le32 res;
} __packed;
