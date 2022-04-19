/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdbool.h>

#include "wigig_ftm_flows_api.h"

#define log_printf(format, ...) \
    printf("[Test App] "); \
    printf(format, ##__VA_ARGS__); \

static void print_usage_set_channel()
{
    printf("\t-f set_channel <channel_number>\n");
}

static void print_usage_set_work_mode()
{
    printf("\t-f set_work_mode <requested_mode in {TX, RX, STANDBY, POWER_DOWN, LOOPBACK}> <xif_mask>\n");
}

static void print_usage_get_per()
{
    printf("\t-f get_per <number_packets_sent>\n");
}

static void print_usage_get_rf_gain_lineup_index()
{
    printf("\t-f get_rf_gain_lineup_index <rfc_id> <gain_index>\n");
}

static void print_usage_set_rf_gain_lineup_index()
{
    printf("\t-f set_rf_gain_lineup_index <rfc_id> <table_type in {G0, G1, TX}> <gain_index> <lineup_index>\n");
}

static void print_usage_get_rf_gain_maximum_lineup_index()
{
    printf("\t-f get_rf_gain_maximum_lineup_index <table_type in {G0, G1, TX}>\n");
}

static void print_usage_get_rf_sector_index()
{
    printf("\t-f get_rf_sector_index <rfc_id>\n");
}

static void print_usage_set_rf_sector_index()
{
    printf("\t-f set_rf_sector_index <rfc_id> <sector_index>\n");
}

static void print_usage_start_packet()
{
    printf("\t-f start_packet <phy_tx_silence_duration_clk_cycles> <mcs_mode in {DMG, EDMG}> <mcs_index in {0, 1, ..., 15}> <phy_tx_plcp_length>\n");
}

static void print_usage_send_echo()
{
    printf("\t-f send_echo <value to echo>\n");
}

static void print_usage_operational_flows_force_mcs()
{
    printf("\t-f operational_flows_force_mcs <mcs_enforcement_mode in {0, 1} (0 for auto, 1 for enforce)> <mcs_index in {0, 1, ..., 15}>\n");
}

static void print_usage_operational_flows_force_tx_sector_index()
{
    printf("\t-f operational_flows_force_tx_sector_index <TX sector index (or 0xFFFF for auto selection)>\n");
}

static void print_usage_operational_flows_get_statistics()
{
    printf("\t-f operational_flows_get_statistics <time_interval_msec in [100, 4500]>\n");
}

static void print_help()
{
    printf("Test application for WIGIG FTM Flow Library\n");
    printf("-------------------------------------------\n");
    printf("USAGE: ftm_flows_test [option] [-d if_name] [-f flow_name flow_parameters]\n");
    printf("OPTIONS:\n");
    printf(" -h, --help : print this message\n");
    printf(" -i, --info: print version information for WIGIG FTM Flows library\n");
    printf(" -v, --verbose: turn verbose logging mode\n");
    printf(" -s, --syslog: use syslog for logging instead of standard output\n");
    printf(" -d, --device if_name: WIGIG interface name to be used during the selected flow\n");
    printf(" -f, --flow flow_name flow_parameters: run the selected flow with given parameters\n");
    
    printf("\t-f get_channel\n");
    print_usage_set_channel();
    printf("\t-f get_work_mode\n");
    print_usage_set_work_mode();
    print_usage_get_per();
    printf("\t-f clear_per\n");
    print_usage_get_rf_gain_lineup_index();
    print_usage_set_rf_gain_lineup_index();
    print_usage_get_rf_gain_maximum_lineup_index();
    print_usage_get_rf_sector_index();
    print_usage_set_rf_sector_index();
    printf("\t-f get_attached_enabled_rf_mask\n");
    printf("\t-f get_packet_configuration\n");
    print_usage_start_packet();
    printf("\t-f stop_packet\n");
    printf("\t-f get_fw_brd_versions\n");
    print_usage_send_echo();
    print_usage_operational_flows_force_mcs();
    print_usage_operational_flows_force_tx_sector_index();
    print_usage_operational_flows_get_statistics();
}

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
        return "[unknown work mode]";
    }
}

static work_mode convert_str_to_work_mode(const char* mode_str)
{
    if (!mode_str) { /* shouldn't happen */
        log_printf("missing work mode parameter\n");
        exit(1);
    }

    if (!strcmp(mode_str, "TX")) {
        return MODE_TX;
    }
    if (!strcmp(mode_str, "RX")) {
        return MODE_RX;
    }
    if (!strcmp(mode_str, "STANDBY")) {
        return MODE_STANDBY;
    }
    if (!strcmp(mode_str, "POWER_DOWN")) {
        return MODE_POWER_DOWN;
    }
    if (!strcmp(mode_str, "LOOPBACK")) {
        return MODE_LOOPBACK;
    }

    log_printf("Unknown work mode parameter '%s'\n", mode_str);
    exit(1);
}

static const char* convert_channel_access_mode_to_str(channel_access_mode mode)
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

static channel_access_mode convert_str_to_channel_access_mode(const char* channel_access_mode_str)
{
    if (!channel_access_mode_str) { /* shouldn't happen */
        log_printf("missing channel access mode parameter\n");
        exit(1);
    }

    if (!strcmp(channel_access_mode_str, "DMG")) {
        return DMG;
    }
    if (!strcmp(channel_access_mode_str, "EDMG")) {
        return EDMG;
    }

    log_printf("Unknown channel access mode parameter '%s'\n", channel_access_mode_str);
    exit(1);
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

static lineup_table_type convert_str_to_table_type(const char* table_type_str)
{
    if (!table_type_str) {
        log_printf("missing table type parameter\n");
        exit(1);
    }
    if (!strcmp(table_type_str, "G0")) {
        return RX_G0;
    }
    if (!strcmp(table_type_str, "G1")) {
        return RX_G1;
    }
    if (!strcmp(table_type_str, "TX")) {
        return TX;
    }

    log_printf("Unknown table type parameter '%s'\n", table_type_str);
    exit(1);
}

int get_channel(const char* flow_name, int argc, char* argv[])
{
    wigig_channel current_channel_number = 1;

    if (!flow_name) { /* shouldn't happen */
        return -1;
    }

    if (argc > 0) {
        log_printf("No arguments expected for %s flow. Extra arguments are ignored.\n", flow_name);
    }
    
    if (wigig_ftm_flows_get_channel(&current_channel_number) != 0) {
        log_printf("%s flow failed. Error: %s\n", flow_name, wigig_ftm_flows_get_last_error());
        return -1;
    }

    log_printf("Current channel number is %d\n", (int)current_channel_number);
    return 0;
}

int set_channel(const char* flow_name, int argc, char* argv[])
{
    wigig_channel channel_number = 1;

    if (!flow_name) { /* shouldn't happen */
        return -1;
    }

    if (argc != 1) {
        log_printf("Invalid arguments for %s flow. Expected format is:\n", flow_name);
        print_usage_set_channel();
        return -1;
    }

    channel_number = strtoul(argv[0], NULL, 0);

    log_printf("- Activating '%s' flow with follwing parameters: channel_number=%d\n", flow_name, channel_number);

    if (wigig_ftm_flows_set_channel(channel_number) != 0) {
        log_printf("%s flow failed. Error: %s\n", flow_name, wigig_ftm_flows_get_last_error());
        return -1;
    }

    log_printf("%s flow succeeded.\n", flow_name);
    return 0;
}

int get_work_mode(const char* flow_name, int argc, char* argv[])
{
    work_mode current_mode = MODE_STANDBY;

    if (!flow_name) { /* shouldn't happen */
        return -1;
    }

    if (argc > 0) {
        log_printf("No arguments expected for %s flow. Extra arguments are ignored.\n", flow_name);
    }

    if (wigig_ftm_flows_get_work_mode(&current_mode) != 0) {
        log_printf("%s flow failed. Error: %s\n", flow_name, wigig_ftm_flows_get_last_error());
        return -1;
    }

    log_printf("Current work mode is %s\n", convert_work_mode_to_str(current_mode));
    return 0;
}

int set_work_mode(const char* flow_name, int argc, char* argv[])
{
    work_mode requested_mode = MODE_STANDBY;
    uint8_t xif_mask = 0xFF;

    if (!flow_name) { /* shouldn't happen */
        return -1;
    }

    if (argc != 2) {
        log_printf("Invalid arguments for %s flow. Expected format is:\n", flow_name);
        print_usage_set_work_mode();
        return -1;
    }

    requested_mode = convert_str_to_work_mode(argv[0]);
    xif_mask = strtoul(argv[1], NULL, 0);

    log_printf("- Activating '%s' flow with follwing parameters: requested_mode=%s, xif_mask=0x%08x\n", flow_name, convert_work_mode_to_str(requested_mode), xif_mask);

    if (wigig_ftm_flows_set_work_mode(requested_mode, xif_mask) != 0) {
        log_printf("%s flow failed. Error: %s\n", flow_name, wigig_ftm_flows_get_last_error());
        return -1;
    }

    log_printf("%s flow succeeded.\n", flow_name);
    return 0;
}

int get_per(const char* flow_name, int argc, char* argv[])
{
    uint32_t number_packets_sent = 0U;
    double per = 0.0;

    if (!flow_name) { /* shouldn't happen */
        return -1;
    }

    if (argc != 1) {
        log_printf("Invalid arguments for %s flow. Expected format is:\n", flow_name);
        print_usage_get_per();
        return -1;
    }

    number_packets_sent = strtoul(argv[0], NULL, 0);

    log_printf("- Activating '%s' flow with follwing parameters: number_packets_sent=%d\n", flow_name, number_packets_sent);

    if (wigig_ftm_flows_get_per(number_packets_sent, &per) != 0) {
        log_printf("%s flow failed. Error: %s\n", flow_name, wigig_ftm_flows_get_last_error());
        return -1;
    }

    log_printf("PER is %.3f [%%]\n", per);
    return 0;
}

int clear_per(const char* flow_name, int argc, char* argv[])
{
    if (!flow_name) { /* shouldn't happen */
        return -1;
    }

    if (argc > 0) {
        log_printf("No arguments expected for %s flow. Extra arguments are ignored.\n", flow_name);
    }

    if (wigig_ftm_flows_clear_per() != 0) {
        log_printf("%s flow failed. Error: %s\n", flow_name, wigig_ftm_flows_get_last_error());
        return -1;
    }

    log_printf("%s flow succeeded.\n", flow_name);
    return 0;
}

int get_rf_gain_lineup_index(const char* flow_name, int argc, char* argv[])
{
    uint8_t rfc_id = 0U, gain_index = 0U;
    uint32_t lineup_index = 0U;
    lineup_table_type table_type = RX_G0;

    if (!flow_name) { /* shouldn't happen */
        return -1;
    }

    if (argc != 2) {
        log_printf("Invalid arguments for %s flow. Expected format is:\n", flow_name);
        print_usage_get_rf_gain_lineup_index();
        return -1;
    }

    rfc_id = strtoul(argv[0], NULL, 0);
    gain_index = strtoul(argv[1], NULL, 0);

    log_printf("- Activating '%s' flow with follwing parameters: rfc_id=%d, gain_index=%d\n", flow_name, rfc_id, gain_index);

    if (wigig_ftm_flows_get_rf_gain_lineup_index(rfc_id, gain_index, &lineup_index, &table_type) != 0) {
        log_printf("%s flow failed. Error: %s\n", flow_name, wigig_ftm_flows_get_last_error());
        return -1;
    }

    log_printf("Table type %s, lineup index %d\n", convert_table_type_to_str(table_type), lineup_index);
    return 0;
}

int set_rf_gain_lineup_index(const char* flow_name, int argc, char* argv[])
{
    uint8_t rfc_id = 0U;
    lineup_table_type table_type = RX_G0;
    uint8_t gain_index = 0U;
    uint32_t lineup_index = 0U;

    if (!flow_name) { /* shouldn't happen */
        return -1;
    }

    if (argc != 4) {
        log_printf("Invalid arguments for %s flow. Expected format is:\n", flow_name);
        print_usage_set_rf_gain_lineup_index();
        return -1;
    }

    rfc_id = strtoul(argv[0], NULL, 0);
    table_type = convert_str_to_table_type(argv[1]);
    gain_index = strtoul(argv[2], NULL, 0);
    lineup_index = strtoul(argv[3], NULL, 0);

    log_printf("- Activating '%s' flow with follwing parameters: rfc_id=%d, table_type=%s, gain_index=%d, lineup_index=%d\n",
           flow_name, rfc_id, convert_table_type_to_str(table_type), gain_index, lineup_index);

    if (wigig_ftm_flows_set_rf_gain_lineup_index(rfc_id, table_type, gain_index, lineup_index) != 0) {
        log_printf("%s flow failed. Error: %s\n", flow_name, wigig_ftm_flows_get_last_error());
        return -1;
    }

    log_printf("%s flow succeeded.\n", flow_name);
    return 0;
}

int get_rf_gain_maximum_lineup_index(const char* flow_name, int argc, char* argv[])
{
    lineup_table_type table_type = RX_G0;
    uint32_t max_lineup_index = 0U;

    if (!flow_name) { /* shouldn't happen */
        return -1;
    }

    if (argc != 1) {
        log_printf("Invalid arguments for %s flow. Expected format is:\n", flow_name);
        print_usage_get_rf_gain_maximum_lineup_index();
        return -1;
    }

    table_type = convert_str_to_table_type(argv[0]);

    log_printf("- Activating '%s' flow with follwing parameters: table_type=%s\n", flow_name, convert_table_type_to_str(table_type));

    if (wigig_ftm_flows_get_rf_gain_maximum_lineup_index(table_type, &max_lineup_index) != 0) {
        log_printf("%s flow failed. Error: %s\n", flow_name, wigig_ftm_flows_get_last_error());
        return -1;
    }

    log_printf("Max lineup index for %s is %d\n", argv[0], max_lineup_index);
    return 0;
}

int get_rf_sector_index(const char* flow_name, int argc, char* argv[])
{
    uint8_t rfc_id = 0U;
    uint8_t sector_index = 0U;

    if (!flow_name) { /* shouldn't happen */
        return -1;
    }

    if (argc != 1) {
        log_printf("Invalid arguments for %s flow. Expected format is:\n", flow_name);
        print_usage_get_rf_sector_index();
        return -1;
    }

    rfc_id = strtoul(argv[0], NULL, 0);

    log_printf("- Activating '%s' flow with follwing parameters: rfc_id=%d\n", flow_name, rfc_id);

    if (wigig_ftm_flows_get_rf_sector_index(rfc_id, &sector_index) != 0) {
        log_printf("%s flow failed. Error: %s\n", flow_name, wigig_ftm_flows_get_last_error());
        return -1;
    }

    log_printf("RF sector index is %d\n", sector_index);
    return 0;
}

int set_rf_sector_index(const char* flow_name, int argc, char* argv[])
{
    uint8_t rfc_id = 0U, sector_index = 0U;

    if (!flow_name) { /* shouldn't happen */
        return -1;
    }

    if (argc != 2) {
        log_printf("Invalid arguments for %s flow. Expected format is:\n", flow_name);
        print_usage_set_rf_sector_index();
        return -1;
    }

    rfc_id = strtoul(argv[0], NULL, 0);
    sector_index = strtoul(argv[1], NULL, 0);

    log_printf("- Activating '%s' flow with follwing parameters: rfc_id=%d, sector_index=%d\n", flow_name, rfc_id, sector_index);

    if (wigig_ftm_flows_set_rf_sector_index(rfc_id, sector_index) != 0) {
        log_printf("%s flow failed. Error: %s\n", flow_name, wigig_ftm_flows_get_last_error());
        return -1;
    }

    log_printf("%s flow succeeded.\n", flow_name);
    return 0;
}

int get_attached_enabled_rf_mask(const char* flow_name, int argc, char* argv[])
{
    uint8_t attached_rf_mask = 0U, enabled_rf_mask = 0U;

    if (!flow_name) { /* shouldn't happen */
        return -1;
    }

    if (argc > 0) {
        log_printf("No arguments expected for %s flow. Extra arguments are ignored.\n", flow_name);
    }

    if (wigig_ftm_flows_get_attached_enabled_rf_mask(&attached_rf_mask, &enabled_rf_mask) != 0) {
        log_printf("%s flow failed. Error: %s\n", flow_name, wigig_ftm_flows_get_last_error());
        return -1;
    }

    log_printf("Attached RFs: 0x%08x, enabled RFs: 0x%08x\n", attached_rf_mask, enabled_rf_mask);
    return 0;
}

int get_packet_configuration(const char* flow_name, int argc, char* argv[])
{
    uint32_t phy_tx_silence_duration_clk_cycles = 0U, phy_tx_plcp_length = 0U;
    transmission_mode is_transmission_active = INACTIVE;
    channel_access_mode channel_access_mode = DMG;
    mcs_index mcs_index = MCS_0;

    if (!flow_name) { /* shouldn't happen */
        return -1;
    }

    if (argc > 0) {
        log_printf("No arguments expected for %s flow. Extra arguments are ignored.\n", flow_name);
    }

    if (wigig_ftm_flows_get_packet_configuration(&phy_tx_silence_duration_clk_cycles,
        &channel_access_mode, &mcs_index, &phy_tx_plcp_length, &is_transmission_active) != 0) {
        log_printf("%s flow failed. Error: %s\n", flow_name, wigig_ftm_flows_get_last_error());
        return -1;
    }

    log_printf("PHY TX silence duration is %d [clock cycles], PHY TX PLCP length is %d, MCS is %s MCS_%d, transmission is %s\n",
            phy_tx_silence_duration_clk_cycles, phy_tx_plcp_length,
            convert_channel_access_mode_to_str(channel_access_mode), mcs_index,
            is_transmission_active == ACTIVE ? "active" : "not active");
    return 0;
}

int start_packet(const char* flow_name, int argc, char* argv[])
{
    uint32_t phy_tx_silence_duration_clk_cycles = 0U;
    channel_access_mode channel_access_mode = DMG;
    mcs_index mcs_index_val = MCS_0;
    uint32_t phy_tx_plcp_length = 0U;

    if (!flow_name) { /* shouldn't happen */
        return -1;
    }

    if (argc != 4) {
        log_printf("Invalid arguments for %s flow. Expected format is:\n", flow_name);
        print_usage_start_packet();
        return -1;
    }

    phy_tx_silence_duration_clk_cycles = strtoul(argv[0], NULL, 0);
    channel_access_mode = convert_str_to_channel_access_mode(argv[1]);
    mcs_index_val = (mcs_index)(strtoul(argv[2], NULL, 0));
    phy_tx_plcp_length = strtoul(argv[3], NULL, 0);

    log_printf("- Activating '%s' flow with follwing parameters: phy_tx_silence_duration_clk_cycles=%d, MCS=%s MCS_%d, phy_tx_plcp_length=%d\n",
               flow_name, phy_tx_silence_duration_clk_cycles,
               convert_channel_access_mode_to_str(channel_access_mode), mcs_index_val, phy_tx_plcp_length);

    if (wigig_ftm_flows_start_packet(phy_tx_silence_duration_clk_cycles, channel_access_mode, mcs_index_val, phy_tx_plcp_length) != 0) {
        log_printf("%s flow failed. Error: %s\n", flow_name, wigig_ftm_flows_get_last_error());
        return -1;
    }

    log_printf("%s flow succeeded.\n", flow_name);
    return 0;
}

int stop_packet(const char* flow_name, int argc, char* argv[])
{
    if (!flow_name) { /* shouldn't happen */
        return -1;
    }

    if (argc > 0) {
        log_printf("No arguments expected for %s flow. Extra arguments are ignored.\n", flow_name);
    }

    if (wigig_ftm_flows_stop_packet() != 0) {
        log_printf("%s flow failed. Error: %s\n", flow_name, wigig_ftm_flows_get_last_error());
        return -1;
    }

    log_printf("%s flow succeeded.\n", flow_name);
    return 0;
}

int get_fw_brd_versions(const char* flow_name, int argc, char* argv[])
{
    uint32_t fw_version_major = 0U, fw_version_minor = 0U, fw_version_sub_minor = 0U, fw_version_build = 0U;
    uint32_t burned_board_file = 0U;

    if (!flow_name) { /* shouldn't happen */
        return -1;
    }

    if (argc > 0) {
        log_printf("No arguments expected for %s flow. Extra arguments are ignored.\n", flow_name);
    }

    if (wigig_ftm_flows_get_fw_brd_versions(&fw_version_major, &fw_version_minor,
        &fw_version_sub_minor, &fw_version_build, &burned_board_file) != 0) {
        log_printf("%s flow failed. Error: %s\n", flow_name, wigig_ftm_flows_get_last_error());
        return -1;
    }

    log_printf("FW version on DUT is [%d.%d.%d.%d]\n", fw_version_major, fw_version_minor, fw_version_sub_minor, fw_version_build);
    log_printf("Board file Id is 0x%08x\n", burned_board_file);
    return 0;
}

int send_echo(const char* flow_name, int argc, char* argv[])
{
    uint32_t echo_val = 0U, echo_val_out = 0U;

    if (!flow_name) { /* shouldn't happen */
        return -1;
    }

    if (argc != 1) {
        log_printf("Invalid arguments for %s flow. Expected format is:\n", flow_name);
        print_usage_send_echo();
        return -1;
    }

    echo_val = strtoul(argv[0], NULL, 0);

    log_printf("- Activating '%s' flow with follwing parameters: echo value = %d\n", flow_name, echo_val);

    if (wigig_flows_send_echo(echo_val, &echo_val_out) != 0) {
        log_printf("%s flow failed. Error: %s\n", flow_name, wigig_ftm_flows_get_last_error());
        return -1;
    }

    log_printf("Echo value returned is %d\n", echo_val_out);
    return 0;
}

int operational_flows_force_mcs(const char* flow_name, int argc, char* argv[])
{
    mcs_enforcement_mode mcs_enforcement = AUTO_MCS;
    mcs_index mcs_index_val = MCS_0;

    if (!flow_name) { /* shouldn't happen */
        return -1;
    }

    if (argc != 2) {
        log_printf("Invalid arguments for %s flow. Expected format is:\n", flow_name);
        print_usage_operational_flows_force_mcs();
        return -1;
    }

    mcs_enforcement = strtoul(argv[0], NULL, 0) == 1 ? FORCE_MCS : AUTO_MCS;
    mcs_index_val = (mcs_index)(strtoul(argv[1], NULL, 0));

    log_printf("- Activating '%s' flow with follwing parameters: mcs_enforcement='%s', mcs_index=MCS_%d\n",
               flow_name, mcs_enforcement == FORCE_MCS ? "force" : "auto", (int)mcs_index_val);

    if (wigig_operational_flows_force_mcs(mcs_enforcement, mcs_index_val) != 0) {
        log_printf("%s flow failed. Error: %s\n", flow_name, wigig_ftm_flows_get_last_error());
        return -1;
    }

    log_printf("%s flow succeeded.\n", flow_name);
    return 0;
}

int wigig_operational_flows_force_sector_index(const char* flow_name, int argc, char* argv[])
{
    uint16_t tx_sector_index = 0U;

    if (!flow_name) { /* shouldn't happen */
        return -1;
    }

    if (argc != 1) {
        log_printf("Invalid arguments for %s flow. Expected format is:\n", flow_name);
        print_usage_operational_flows_force_tx_sector_index();
        return -1;
    }

    tx_sector_index = strtoul(argv[0], NULL, 0);

    log_printf("- Activating '%s' flow with follwing parameters: TX sector index=%d\n", flow_name, tx_sector_index);

    if (wigig_operational_flows_force_tx_sector_index(tx_sector_index) != 0) {
        log_printf("%s flow failed. Error: %s\n", flow_name, wigig_ftm_flows_get_last_error());
        return -1;
    }

    log_printf("%s flow succeeded.\n", flow_name);
    return 0;
}

int operational_flows_get_statistics(const char* flow_name, int argc, char* argv[])
{
    uint32_t interval_msec = 0U;
    uint8_t bf_mcs = 0U, per_average = 0U, tx_sector_index = 0U;
    uint32_t tx_goodput = 0U, rx_goodput = 0U;

    if (!flow_name) { /* shouldn't happen */
        return -1;
    }

    if (argc != 1) {
        log_printf("Invalid arguments for %s flow. Expected format is:\n", flow_name);
        print_usage_operational_flows_get_statistics();
        return -1;
    }

    interval_msec = strtoul(argv[0], NULL, 0);

    log_printf("- Activating '%s' flow with follwing parameters: interval_msec=%d\n", flow_name, interval_msec);

    if (wigig_operational_flows_get_statistics(interval_msec, &bf_mcs, &per_average,
                                               &tx_goodput, &rx_goodput, &tx_sector_index) != 0) {
        log_printf("%s flow failed. Error: %s\n", flow_name, wigig_ftm_flows_get_last_error());
        return -1;
    }

    log_printf("BF MCS is %d, average PER is %d [%%], TX goodput is %d [Mbps], RX goodput is %d [Mbps], TX sector index is %d\n",
               bf_mcs, per_average, tx_goodput, rx_goodput, tx_sector_index);
    return 0;
}

/* process the given test*/
int process_command(const char* flow_name, int argc, char* argv[])
{
    if (!flow_name) { /* shouldn't happen */
        return -1;
    }

    if (!strcmp(flow_name, "get_channel")) {
        return get_channel(flow_name, argc, argv);
    }
    if (!strcmp(flow_name, "set_channel")) {
        return set_channel(flow_name, argc, argv);
    }
    else if (!strcmp(flow_name, "get_work_mode")) {
        return get_work_mode(flow_name, argc, argv);
    }
    else if (!strcmp(flow_name, "set_work_mode")) {
        return set_work_mode(flow_name, argc, argv);
    }
    else if (!strcmp(flow_name, "get_per")) {
        return get_per(flow_name, argc, argv);
    }
    else if (!strcmp(flow_name, "clear_per")) {
        return clear_per(flow_name, argc, argv);
    }
    else if (!strcmp(flow_name, "get_rf_gain_lineup_index")) {
        return get_rf_gain_lineup_index(flow_name, argc, argv);
    }
    else if (!strcmp(flow_name, "set_rf_gain_lineup_index")) {
        return set_rf_gain_lineup_index(flow_name, argc, argv);
    }
    else if (!strcmp(flow_name, "get_rf_gain_maximum_lineup_index")) {
        return get_rf_gain_maximum_lineup_index(flow_name, argc, argv);
    }
    else if (!strcmp(flow_name, "get_rf_sector_index")) {
        return get_rf_sector_index(flow_name, argc, argv);
    }
    else if (!strcmp(flow_name, "set_rf_sector_index")) {
        return set_rf_sector_index(flow_name, argc, argv);
    }
    else if (!strcmp(flow_name, "get_attached_enabled_rf_mask")) {
        return get_attached_enabled_rf_mask(flow_name, argc, argv);
    }
    else if (!strcmp(flow_name, "get_packet_configuration")) {
        return get_packet_configuration(flow_name, argc, argv);
    }
    else if (!strcmp(flow_name, "start_packet")) {
        return start_packet(flow_name, argc, argv);
    }
    else if (!strcmp(flow_name, "stop_packet")) {
        return stop_packet(flow_name, argc, argv);
    }
    else if (!strcmp(flow_name, "get_fw_brd_versions")) {
        return get_fw_brd_versions(flow_name, argc, argv);
    }
    else if (!strcmp(flow_name, "operational_flows_force_mcs")) {
        return operational_flows_force_mcs(flow_name, argc, argv);
    }
    else if (!strcmp(flow_name, "operational_flows_force_tx_sector_index")) {
        return wigig_operational_flows_force_sector_index(flow_name, argc, argv);
    }
    else if (!strcmp(flow_name, "operational_flows_get_statistics")) {
        return operational_flows_get_statistics(flow_name, argc, argv);
    }
    else if (!strcmp(flow_name, "send_echo")) {
        return send_echo(flow_name, argc, argv);
    }
    // otherwise invalid flow name

    log_printf("Invalid Arguments. Unknows flow name '%s'. See list of supported flows below.\n", flow_name);
    print_help();
    return -1;
}

int main(int argc, char* argv[])
{
    static const struct option options[] = {
        { "help", no_argument, 0, 'h' },
        { "info", no_argument, 0, 'i' },
        { "verbose", no_argument, 0, 'v' },
        { "syslog", no_argument, 0, 's' },
        { "device", required_argument, 0, 'd' },
        { "flow", required_argument, 0, 'f' },
        { 0, 0, 0, 0 }
    };

    int rc = 0;
    int c, optidx = 0;
    bool should_print_help = false;
    bool should_print_info = false;
    bool use_syslog_sink = false;
    bool allow_verbose_logging = false;
    char *flow_name = NULL;
    char *ifname = NULL;

    if (argc == 1) {
        print_help();
        return -1;
    }

    while ((c = getopt_long(argc, argv, "hivsd:f:", options, &optidx)) != EOF) {
        switch (c) {
        case 'h':
            should_print_help = true;
            break;
        case 'i':
            should_print_info = true;
            break;
        case 'v':
            allow_verbose_logging = true;
            break;
        case 's':
            use_syslog_sink = true;
            break;
        case 'd':
            ifname = optarg;
            break;
        case 'f':
            flow_name = optarg;
            break;
        default:
            print_help();
            return -1;
        }
    }

    if (should_print_help) {
        print_help();
        return 0;
    }

    if (should_print_info) {
        log_printf("WIGIG FTM Flows Library Version: %s\n", wigig_ftm_flows_get_version());
        return 0;
    }

    // otherwise it's a flow request
    if ( !(ifname && flow_name) ) {
        log_printf("Invalid Arguments. WIGIG infterface name and flow name should be provided.\n");
        print_help();
        return -1;
    }

    /* jump to first option after the processed ones (if any) */
    argc -= optind;
    argv = argc > 0 ? &argv[optind] : NULL;

    if (allow_verbose_logging) {
        wigig_ftm_flows_configure_log_verbosity(LOG_ALL);
    }

    if (!use_syslog_sink) {
        wigig_ftm_flows_configure_output_sink(STDOUT);
    }

    if (wigig_ftm_flows_initialize_connection(ifname) != 0) {
        log_printf("Failed to initialize netlink interface for %s. Error: %s\n", ifname, wigig_ftm_flows_get_last_error());
        return -1;
    }

    rc = process_command(flow_name, argc, argv);

    wigig_ftm_flows_release_connection();

    return rc;
}
