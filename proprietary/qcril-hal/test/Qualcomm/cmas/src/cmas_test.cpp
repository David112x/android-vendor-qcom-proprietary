/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <vector>
#include <gtest/gtest.h>

#include "gsm_cmas_cellbroadcast.h"
#include "umts_cmas_cellbroadcast.h"
#include "multipart_gsm_cmas_cellbroadcast.h"

typedef struct {
    uint32_t data_len;
    uint8_t data[255];
} transfer_route_mt_message_type;

typedef enum {
    WMS_BC_RAT_INFO_ENUM_MIN_ENUM_VAL_V01 =
        -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
    WMS_BC_RAT_GSM_RADIO_ACCESS_V01 = 0x00,   /**<  Broadcast SMS received over GSM \n  */
    WMS_BC_RAT_WCDMA_RADIO_ACCESS_V01 = 0x01, /**<  Broadcast SMS received over WCDMA \n  */
    WMS_BC_RAT_LTE_RADIO_ACCESS_V01 = 0x02,   /**<  Broadcast SMS received over LTE \n  */
    WMS_BC_RAT_CDMA_RADIO_ACCESS_V01 = 0x03,  /**<  Broadcast SMS received over CDMA \n  */
    WMS_BC_RAT_NR5G_RADIO_ACCESS_V01 = 0x04,  /**<  Broadcast SMS received over NR5G \n  */
    WMS_BC_RAT_INFO_ENUM_MAX_ENUM_VAL_V01 =
        2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
} wms_bc_rat_info_enum_type;

typedef struct {
    uint8_t bc_sms_rat_info_valid;
    wms_bc_rat_info_enum_type bc_sms_rat_info;
    transfer_route_mt_message_type transfer_route_mt_message;
} event_report_ind_type;

uint32_t get_umts_cmas_msg(event_report_ind_type *data) {
    event_report_ind_type event_report_ind = {
        true,
        WMS_BC_RAT_WCDMA_RADIO_ACCESS_V01,
        {90, {0x01, 0xa8, 0x07, 0x11, 0x14, 0x11, 0x01, 0x30, 0x00, 0x00, 0x30, 0x00, 0x78,
              0x00, 0x41, 0x00, 0x38, 0x00, 0x30, 0x00, 0x37, 0x00, 0x0a, 0x70, 0x7d, 0x5b,
              0xb3, 0x90, 0x7f, 0x96, 0xe3, 0x60, 0xc5, 0x58, 0x31, 0x30, 0x67, 0x30, 0x59,
              0x00, 0x0d, 0x00, 0x0d, 0x00, 0x0d, 0x00, 0x0d, 0x00, 0x0d, 0x00, 0x0d, 0x00,
              0x0d, 0x00, 0x0d, 0x00, 0x0d, 0x00, 0x0d, 0x00, 0x0d, 0x00, 0x0d, 0x00, 0x0d,
              0x00, 0x0d, 0x00, 0x0d, 0x00, 0x0d, 0x00, 0x0d, 0x00, 0x0d, 0x00, 0x0d, 0x00,
              0x0d, 0x00, 0x0d, 0x00, 0x0d, 0x00, 0x0d, 0x00, 0x0d, 0x00, 0x0d, 0x20}}};

    *data = event_report_ind;
    return data->transfer_route_mt_message.data_len;
}

uint32_t get_gsm_cmas_msg(event_report_ind_type *data) {
    event_report_ind_type event_report_ind = {
        true,
        WMS_BC_RAT_GSM_RADIO_ACCESS_V01,
        {88,
         {0x0,  0x16, 0x11, 0x14, 0x1,  0x13, 0x54, 0x74, 0x7a, 0xe,  0x4a, 0xbb, 0xe7, 0x74, 0x79,
          0xbd, 0x5d, 0x76, 0xd3, 0x41, 0x70, 0xf9, 0xdb, 0x9e, 0x26, 0x97, 0xe7, 0x20, 0x73, 0xdd,
          0x3d, 0xa6, 0xa7, 0xdf, 0xee, 0x30, 0x1b, 0x44, 0x2f, 0xcf, 0xe9, 0x69, 0xf7, 0x19, 0xf4,
          0x37, 0x83, 0xc6, 0x65, 0x36, 0x1b, 0x24, 0x96, 0xbf, 0xc3, 0xe4, 0x71, 0x78, 0x4e, 0x7,
          0x4d, 0x9b, 0x53, 0x90, 0x38, 0xf,  0x9a, 0x97, 0xdd, 0xe4, 0xb4, 0xfb, 0xc,  0xaa, 0xc3,
          0x41, 0xf4, 0x37, 0x88, 0x8e, 0x96, 0x97, 0xcb, 0x20, 0xb1, 0xfc, 0x1d, 0x6}}};

    *data = event_report_ind;
    return data->transfer_route_mt_message.data_len;
}

uint32_t get_umts_rat_gsm_cmas_msg(event_report_ind_type *data) {
    event_report_ind_type event_report_ind = {
        true,
        WMS_BC_RAT_WCDMA_RADIO_ACCESS_V01,
        {88,
         {0x0,  0x16, 0x11, 0x14, 0x1,  0x13, 0x54, 0x74, 0x7a, 0xe,  0x4a, 0xbb, 0xe7, 0x74, 0x79,
          0xbd, 0x5d, 0x76, 0xd3, 0x41, 0x70, 0xf9, 0xdb, 0x9e, 0x26, 0x97, 0xe7, 0x20, 0x73, 0xdd,
          0x3d, 0xa6, 0xa7, 0xdf, 0xee, 0x30, 0x1b, 0x44, 0x2f, 0xcf, 0xe9, 0x69, 0xf7, 0x19, 0xf4,
          0x37, 0x83, 0xc6, 0x65, 0x36, 0x1b, 0x24, 0x96, 0xbf, 0xc3, 0xe4, 0x71, 0x78, 0x4e, 0x7,
          0x4d, 0x9b, 0x53, 0x90, 0x38, 0xf,  0x9a, 0x97, 0xdd, 0xe4, 0xb4, 0xfb, 0xc,  0xaa, 0xc3,
          0x41, 0xf4, 0x37, 0x88, 0x8e, 0x96, 0x97, 0xcb, 0x20, 0xb1, 0xfc, 0x1d, 0x6}}};

    *data = event_report_ind;
    return data->transfer_route_mt_message.data_len;
}

auto get_gsm_buffer() {
    event_report_ind_type event_report_ind{};
    uint32_t data_len = get_gsm_cmas_msg(&event_report_ind);

    auto array_begin = event_report_ind.transfer_route_mt_message.data;
    auto array_end = array_begin + data_len;
    std::vector<uint8_t> qmi_buffer(array_begin, array_end);
    return qmi_buffer;
}

auto get_umts_buffer() {
    event_report_ind_type event_report_ind{};
    uint32_t data_len = get_umts_cmas_msg(&event_report_ind);

    auto array_begin = event_report_ind.transfer_route_mt_message.data;
    auto array_end = array_begin + data_len;
    std::vector<uint8_t> qmi_buffer(array_begin, array_end);
    return qmi_buffer;
}

TEST(GsmCmasCellbroadcast, VerifyThatGeogrphicalScopeCanBeParsed) {
    auto qmi_buffer = get_gsm_buffer();
    auto gsm_cmas_cellbroadcast = GsmCmasCellbroadcast(qmi_buffer);

    EXPECT_TRUE(gsm_cmas_cellbroadcast.get_geographical_scope() >= 0);
}

TEST(GsmCmasCellbroadcast, VerifyThatSerialNumberCanBeParsed) {
    auto qmi_buffer = get_gsm_buffer();
    auto gsm_cmas_cellbroadcast = GsmCmasCellbroadcast(qmi_buffer);

    EXPECT_TRUE(gsm_cmas_cellbroadcast.get_serial_number() > 0);
}

TEST(GsmCmasCellbroadcast, VerifyThatMessageIdCanBeParsed) {
    auto qmi_buffer = get_gsm_buffer();
    auto gsm_cmas_cellbroadcast = GsmCmasCellbroadcast(qmi_buffer);

    EXPECT_TRUE(gsm_cmas_cellbroadcast.get_message_identifier() > 0);
}

TEST(GsmCmasCellbroadcast, VerifyThatDcsCanCanBeParsed) {
    auto qmi_buffer = get_gsm_buffer();
    auto gsm_cmas_cellbroadcast = GsmCmasCellbroadcast(qmi_buffer);

    EXPECT_TRUE(gsm_cmas_cellbroadcast.get_dcs() > 0);
}

TEST(GsmCmasCellbroadcast, VerifyThatPageIndexCanBeParsed) {
    auto qmi_buffer = get_gsm_buffer();
    auto gsm_cmas_cellbroadcast = GsmCmasCellbroadcast(qmi_buffer);

    EXPECT_TRUE(gsm_cmas_cellbroadcast.get_page_index() > 0);
}

TEST(GsmCmasCellbroadcast, VerifyThatPageCountCanBeParsed) {
    auto qmi_buffer = get_gsm_buffer();
    auto gsm_cmas_cellbroadcast = GsmCmasCellbroadcast(qmi_buffer);

    EXPECT_TRUE(gsm_cmas_cellbroadcast.get_page_count() > 0);
}

TEST(GsmCmasCellbroadcast, VerifyThatCbDataCanBeParsed) {
    auto qmi_buffer = get_gsm_buffer();
    auto gsm_cmas_cellbroadcast = GsmCmasCellbroadcast(qmi_buffer);

    EXPECT_FALSE(gsm_cmas_cellbroadcast.get_cb_data().empty());
}

TEST(GsmCmasCellbroadcast, VerifyThatRawBufferCanBeParsed) {
    auto qmi_buffer = get_gsm_buffer();
    auto gsm_cmas_cellbroadcast = GsmCmasCellbroadcast(qmi_buffer);

    EXPECT_FALSE(gsm_cmas_cellbroadcast.get_raw_buffer().empty());
}

TEST(GsmCmasCellbroadcast, VerifyThatWacIsAbsent) {
    auto qmi_buffer = get_gsm_buffer();
    auto gsm_cmas_cellbroadcast = GsmCmasCellbroadcast(qmi_buffer);

    EXPECT_TRUE(gsm_cmas_cellbroadcast.get_warning_area_coordinate().empty());
}

TEST(GsmCmasCellbroadcast, VerifyThatEmptyBufferIsHandledGracefully) {
    std::vector<uint8_t> qmi_buffer{};
    auto gsm_cmas_cellbroadcast = GsmCmasCellbroadcast(qmi_buffer);

    EXPECT_TRUE(gsm_cmas_cellbroadcast.get_geographical_scope() == 0);
    EXPECT_TRUE(gsm_cmas_cellbroadcast.get_serial_number() == 0);
    EXPECT_TRUE(gsm_cmas_cellbroadcast.get_message_identifier() == 0);
    EXPECT_TRUE(gsm_cmas_cellbroadcast.get_dcs() == 0);
    EXPECT_TRUE(gsm_cmas_cellbroadcast.get_page_index() ==  0);
    EXPECT_TRUE(gsm_cmas_cellbroadcast.get_page_count() == 0);
    EXPECT_TRUE(gsm_cmas_cellbroadcast.get_cb_data().empty());
    EXPECT_TRUE(gsm_cmas_cellbroadcast.get_raw_buffer().empty());
    EXPECT_TRUE(gsm_cmas_cellbroadcast.get_warning_area_coordinate().empty());
}

TEST(GsmCmasCellbroadcast, VerifyThatShortBufferIsHandledGracefully) {
    std::vector<uint8_t> qmi_buffer{0x01, 0x02, 0x03, 0x04};
    auto gsm_cmas_cellbroadcast = GsmCmasCellbroadcast(qmi_buffer);

    EXPECT_TRUE(gsm_cmas_cellbroadcast.get_geographical_scope() == 0);
    EXPECT_TRUE(gsm_cmas_cellbroadcast.get_serial_number() == 0);
    EXPECT_TRUE(gsm_cmas_cellbroadcast.get_message_identifier() == 0);
    EXPECT_TRUE(gsm_cmas_cellbroadcast.get_dcs() == 0);
    EXPECT_TRUE(gsm_cmas_cellbroadcast.get_page_index() ==  0);
    EXPECT_TRUE(gsm_cmas_cellbroadcast.get_page_count() == 0);
    EXPECT_TRUE(gsm_cmas_cellbroadcast.get_cb_data().empty());
    EXPECT_FALSE(gsm_cmas_cellbroadcast.get_raw_buffer().empty());
    EXPECT_TRUE(gsm_cmas_cellbroadcast.get_warning_area_coordinate().empty());
}
TEST(UmtsCmasCellbroadcast, VerifyGetAndSetForSerialNumber) {
    auto qmi_buffer = get_umts_buffer();
    auto expected_umts_cmas_cellbroadcast = UmtsCmasCellbroadcast(qmi_buffer);
    auto expected_serial_num = expected_umts_cmas_cellbroadcast.get_serial_number();

    auto umts_cmas_cellbroadcast = UmtsCmasCellbroadcast();
    umts_cmas_cellbroadcast.set_serial_number(expected_serial_num);
    auto serial_num = umts_cmas_cellbroadcast.get_serial_number();

    EXPECT_EQ(expected_serial_num, serial_num);
}

TEST(UmtsCmasCellbroadcast, VerifyThatWarningAgeaCoordinateCanBeSet) {
    std::vector<uint8_t> wac{0x00, 0x01, 0x02, 0x03, 0x04, 0x05};
    auto qmi_buffer = get_umts_buffer();
    auto expected_umts_cmas_cellbroadcast = UmtsCmasCellbroadcast(qmi_buffer);
    expected_umts_cmas_cellbroadcast.set_warning_area_coordinate(wac);
    auto wac1 = expected_umts_cmas_cellbroadcast.get_warning_area_coordinate();
    /* First 2 bytes are wac length */
    auto offset = 2;
    std::vector<uint8_t> wac1_buffer(&wac1[offset], &wac1[wac.size() + offset]);
    EXPECT_EQ(wac.size(), (wac1[1] << 8 | wac1[0]));
    EXPECT_EQ(wac, wac1_buffer);
}

TEST(UmtsCmasCellbroadcast, VerifyGetAndSetForMessageId) {
    auto qmi_buffer = get_umts_buffer();
    auto expected_umts_cmas_cellbroadcast = UmtsCmasCellbroadcast(qmi_buffer);
    auto expected_msg_id = expected_umts_cmas_cellbroadcast.get_message_identifier();

    auto umts_cmas_cellbroadcast = UmtsCmasCellbroadcast();
    umts_cmas_cellbroadcast.set_message_identifier(expected_msg_id);
    auto msg_id = umts_cmas_cellbroadcast.get_message_identifier();

    EXPECT_EQ(expected_msg_id, msg_id);
}

TEST(UmtsCmasCellbroadcast, VerifyGetAndSetForDcs) {
    auto qmi_buffer = get_umts_buffer();
    auto expected_umts_cmas_cellbroadcast = UmtsCmasCellbroadcast(qmi_buffer);
    auto expected_dcs = expected_umts_cmas_cellbroadcast.get_dcs();

    auto umts_cmas_cellbroadcast = UmtsCmasCellbroadcast();
    umts_cmas_cellbroadcast.set_dcs(expected_dcs);
    auto dcs = umts_cmas_cellbroadcast.get_dcs();

    EXPECT_EQ(expected_dcs, dcs);
}

TEST(MultipartGsmCmasCellbroadcast, VerifyThatMultipartGsmCbIsAggregatedOnUmtsRat) {
    std::vector<uint8_t> gsm_cb_page1{
        0x0,  0x16, 0x11, 0x14, 0x1,  0x13, 0x54, 0x74, 0x7a, 0xe,  0x4a, 0xbb, 0xe7, 0x74, 0x79,
        0xbd, 0x5d, 0x76, 0xd3, 0x41, 0x70, 0xf9, 0xdb, 0x9e, 0x26, 0x97, 0xe7, 0x20, 0x73, 0xdd,
        0x3d, 0xa6, 0xa7, 0xdf, 0xee, 0x30, 0x1b, 0x44, 0x2f, 0xcf, 0xe9, 0x69, 0xf7, 0x19, 0xf4,
        0x37, 0x83, 0xc6, 0x65, 0x36, 0x1b, 0x24, 0x96, 0xbf, 0xc3, 0xe4, 0x71, 0x78, 0x4e, 0x7,
        0x4d, 0x9b, 0x53, 0x90, 0x38, 0xf,  0x9a, 0x97, 0xdd, 0xe4, 0xb4, 0xfb, 0xc,  0xaa, 0xc3,
        0x41, 0xf4, 0x37, 0x88, 0x8e, 0x96, 0x97, 0xcb, 0x20, 0xb1, 0xfc, 0x1d, 0x6};

    std::vector<uint8_t> gsm_cb_page2{
        0x0,  0x16, 0x11, 0x14, 0x1,  0x23, 0xe4, 0x71, 0x78, 0x4e, 0x7,  0xb5, 0xcb, 0xf3, 0x79,
        0xf8, 0x5c, 0x9e, 0x83, 0xe8, 0x6f, 0x10, 0x1d, 0x5d, 0x6,  0x91, 0xcb, 0xf6, 0xf4, 0xb8,
        0xc,  0xaa, 0xbb, 0xc9, 0x65, 0x39, 0x88, 0x5e, 0x9e, 0xd3, 0x5d, 0x20, 0xea, 0xfd, 0xd,
        0x32, 0xa7, 0xf1, 0x65, 0x32, 0x88, 0x5e, 0xc6, 0xd3, 0x41, 0xed, 0xf2, 0x7c, 0x1e, 0x3e,
        0x97, 0xe7, 0xa0, 0xb0, 0x9b, 0xc,  0xaa, 0xcf, 0xcb, 0x72, 0x10, 0xb9, 0x6c, 0x4e, 0xbb,
        0xcb, 0x64, 0x10, 0xbd, 0x8c, 0xa7, 0x83, 0xde, 0x72, 0x90, 0x38, 0xed, 0x6};

    std::vector<uint8_t> gsm_cb_page3{
        0x0,  0x16, 0x11, 0x14, 0x1,  0x33, 0x61, 0x79, 0xfe, 0x55, 0x1d, 0x4e, 0x65, 0xa0, 0x76,
        0x79, 0x3e, 0xf,  0x9f, 0xcb, 0x73, 0x50, 0x58, 0x5e, 0x6,  0x85, 0xed, 0xe1, 0x34, 0x3b,
        0x2c, 0x66, 0x97, 0x41, 0xe6, 0xb7, 0x1c, 0x34, 0x2f, 0xb3, 0xcb, 0x63, 0x7a, 0xfa, 0xed,
        0x76, 0x81, 0xa8, 0xe8, 0x32, 0x68, 0x5e, 0x1e, 0xbf, 0xdd, 0x64, 0x90, 0x39, 0x8d, 0x2f,
        0x93, 0x41, 0xed, 0xf2, 0x7c, 0x1e, 0x3e, 0x97, 0x41, 0x73, 0x78, 0xd8, 0x3d, 0x7,  0xb5,
        0xeb, 0x6c, 0x7a, 0x1a, 0xce, 0x2e, 0x83, 0xe0, 0xe1, 0x73, 0x79, 0xee, 0x2};

    static auto multipart_gsm_cmas_cellbroadcast = MultipartGsmCmasCellbroadcast();
    multipart_gsm_cmas_cellbroadcast.set_radio_tech(
        MultipartGsmCmasCellbroadcast::RadioTech::WCDMA);
    auto page1 = GsmCmasCellbroadcast(gsm_cb_page1);
    auto page2 = GsmCmasCellbroadcast(gsm_cb_page2);
    auto page3 = GsmCmasCellbroadcast(gsm_cb_page3);
    multipart_gsm_cmas_cellbroadcast.append(page1);
    multipart_gsm_cmas_cellbroadcast.append(page2);
    multipart_gsm_cmas_cellbroadcast.append(page3);

    int callback_invocation_count = 0;
    MultipartGsmCmasCellbroadcast::CallbackType callback = [&](const std::vector<uint8_t> buffer) {
        ++callback_invocation_count;
        return true;
    };

    multipart_gsm_cmas_cellbroadcast.process(callback);
    // For UMTS callback must be called only once.
    EXPECT_EQ(1, callback_invocation_count);
    multipart_gsm_cmas_cellbroadcast.reset();
}

TEST(MultipartGsmCmasCellbroadcast, VerifyThatMultipartGsmCbIsNotAggregatedOnGsmRat) {
    std::vector<uint8_t> gsm_cb_page1{
        0x0,  0x16, 0x11, 0x14, 0x1,  0x13, 0x54, 0x74, 0x7a, 0xe,  0x4a, 0xbb, 0xe7, 0x74, 0x79,
        0xbd, 0x5d, 0x76, 0xd3, 0x41, 0x70, 0xf9, 0xdb, 0x9e, 0x26, 0x97, 0xe7, 0x20, 0x73, 0xdd,
        0x3d, 0xa6, 0xa7, 0xdf, 0xee, 0x30, 0x1b, 0x44, 0x2f, 0xcf, 0xe9, 0x69, 0xf7, 0x19, 0xf4,
        0x37, 0x83, 0xc6, 0x65, 0x36, 0x1b, 0x24, 0x96, 0xbf, 0xc3, 0xe4, 0x71, 0x78, 0x4e, 0x7,
        0x4d, 0x9b, 0x53, 0x90, 0x38, 0xf,  0x9a, 0x97, 0xdd, 0xe4, 0xb4, 0xfb, 0xc,  0xaa, 0xc3,
        0x41, 0xf4, 0x37, 0x88, 0x8e, 0x96, 0x97, 0xcb, 0x20, 0xb1, 0xfc, 0x1d, 0x6};

    std::vector<uint8_t> gsm_cb_page2{
        0x0,  0x16, 0x11, 0x14, 0x1,  0x23, 0xe4, 0x71, 0x78, 0x4e, 0x7,  0xb5, 0xcb, 0xf3, 0x79,
        0xf8, 0x5c, 0x9e, 0x83, 0xe8, 0x6f, 0x10, 0x1d, 0x5d, 0x6,  0x91, 0xcb, 0xf6, 0xf4, 0xb8,
        0xc,  0xaa, 0xbb, 0xc9, 0x65, 0x39, 0x88, 0x5e, 0x9e, 0xd3, 0x5d, 0x20, 0xea, 0xfd, 0xd,
        0x32, 0xa7, 0xf1, 0x65, 0x32, 0x88, 0x5e, 0xc6, 0xd3, 0x41, 0xed, 0xf2, 0x7c, 0x1e, 0x3e,
        0x97, 0xe7, 0xa0, 0xb0, 0x9b, 0xc,  0xaa, 0xcf, 0xcb, 0x72, 0x10, 0xb9, 0x6c, 0x4e, 0xbb,
        0xcb, 0x64, 0x10, 0xbd, 0x8c, 0xa7, 0x83, 0xde, 0x72, 0x90, 0x38, 0xed, 0x6};

    std::vector<uint8_t> gsm_cb_page3{
        0x0,  0x16, 0x11, 0x14, 0x1,  0x33, 0x61, 0x79, 0xfe, 0x55, 0x1d, 0x4e, 0x65, 0xa0, 0x76,
        0x79, 0x3e, 0xf,  0x9f, 0xcb, 0x73, 0x50, 0x58, 0x5e, 0x6,  0x85, 0xed, 0xe1, 0x34, 0x3b,
        0x2c, 0x66, 0x97, 0x41, 0xe6, 0xb7, 0x1c, 0x34, 0x2f, 0xb3, 0xcb, 0x63, 0x7a, 0xfa, 0xed,
        0x76, 0x81, 0xa8, 0xe8, 0x32, 0x68, 0x5e, 0x1e, 0xbf, 0xdd, 0x64, 0x90, 0x39, 0x8d, 0x2f,
        0x93, 0x41, 0xed, 0xf2, 0x7c, 0x1e, 0x3e, 0x97, 0x41, 0x73, 0x78, 0xd8, 0x3d, 0x7,  0xb5,
        0xeb, 0x6c, 0x7a, 0x1a, 0xce, 0x2e, 0x83, 0xe0, 0xe1, 0x73, 0x79, 0xee, 0x2};

    std::vector<uint8_t> wac{0x00, 0x01, 0x02, 0x03, 0x04, 0x05};

    static auto multipart_gsm_cmas_cellbroadcast = MultipartGsmCmasCellbroadcast();
    multipart_gsm_cmas_cellbroadcast.set_radio_tech(MultipartGsmCmasCellbroadcast::RadioTech::GSM);
    auto page1 = GsmCmasCellbroadcast(gsm_cb_page1);
    auto page2 = GsmCmasCellbroadcast(gsm_cb_page2);
    auto page3 = GsmCmasCellbroadcast(gsm_cb_page3);
    multipart_gsm_cmas_cellbroadcast.append(page1);
    multipart_gsm_cmas_cellbroadcast.append(page2);
    multipart_gsm_cmas_cellbroadcast.append(page3);
    multipart_gsm_cmas_cellbroadcast.set_warning_area_coordinate(wac);

    int callback_invocation_count = 0;
    MultipartGsmCmasCellbroadcast::CallbackType callback = [&](const std::vector<uint8_t> buffer) {
        switch(callback_invocation_count) {
            case 0:
                EXPECT_EQ(gsm_cb_page1, buffer);
                break;
            case 1:
                EXPECT_EQ(gsm_cb_page2, buffer);
                break;
            case 2:
                EXPECT_EQ(gsm_cb_page3, buffer);
                break;
        }
        ++callback_invocation_count;
        return true;
    };

    multipart_gsm_cmas_cellbroadcast.process(callback);
    // For GSM callback must be called for all pages of this multipart broadcast.
    EXPECT_EQ(3, callback_invocation_count);
    multipart_gsm_cmas_cellbroadcast.reset();
}
