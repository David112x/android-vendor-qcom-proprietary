/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "umts_cmas_cellbroadcast.h"
#include "android-base/logging.h"
#include <iostream>

/*
 * Default ctor.
 */
UmtsCmasCellbroadcast ::UmtsCmasCellbroadcast() {
    /* Message type for UMTS cell broadcast is always 0x01
     */
    cmas_buffer[0] = 0x01;
}

/*
 * Common ctor.
 * Input buffer must be at least 6 Octet long.
 * Decode the buffer using Umts Cmas header format.
 * Refer UmtsCmasCellbroadcast.h for header information.
 *
 * This ctor is used primarily for unit test purpose.
 */
UmtsCmasCellbroadcast ::UmtsCmasCellbroadcast(BufferType buffer) : CmasCellbroadcast(buffer) {
    geographical_scope = (cmas_buffer[3] & 0xc0) >> 6;
    serial_number = ((cmas_buffer[3] & 0xff) << 8) | (cmas_buffer[4] & 0xff);
    message_id = ((cmas_buffer[1] & 0xff) << 8) | (cmas_buffer[2] & 0xff);
    dcs = cmas_buffer[5] & 0xff;
    page_index = 0;
    cb_data_begin = cmas_buffer.begin() + HEADER_LENGTH;
    page_count = *cb_data_begin;
    header_and_cb_data_length = HEADER_LENGTH + 1 + page_count + page_count * CB_DATA_LENGTH;
    if (buffer.size() > header_and_cb_data_length) {
        wac_begin = cb_data_begin + 1 + page_count + page_count * CB_DATA_LENGTH;
    } else {
        wac_begin = cmas_buffer.end();
    }
}

/*
 * Accessor method to get geographical scope.
 */
uint UmtsCmasCellbroadcast ::get_geographical_scope() const {
    return this->geographical_scope;
}

/*
 * Method to set geographical scope.
 * Geographical scode is at 6-7 bits of 4th Octet.
 */
void UmtsCmasCellbroadcast ::set_geographical_scope(const int geographical_scope) {
    uint8_t bit_mask = 0xc0 >> 6;
    cmas_buffer[3] |= ((geographical_scope & bit_mask) << 6);
    this->geographical_scope = geographical_scope;
}

/*
 * Accessor method to get serial number.
 */
uint UmtsCmasCellbroadcast ::get_serial_number() const {
    return this->serial_number;
}

/*
 * Method to set serial number.
 * Serial number is present at 0-4 bits of Octet 4 and Octect 5
 * in Big-endian format.
 */
void UmtsCmasCellbroadcast ::set_serial_number(const int serial_num) {
    // Write big-endian
    uint16_t bit_mask = 0xff << 8;
    cmas_buffer[3] |= ((serial_num & bit_mask) >> 8);
    bit_mask = 0xff;
    cmas_buffer[4] |= (serial_num & bit_mask);
    this->serial_number = serial_num;
}

/*
 * Accessor method to get message id.
 */
uint UmtsCmasCellbroadcast ::get_message_identifier() const {
    return this->message_id;
}

/*
 * Method to set message id.
 * Message id is present in Octet 2-3 in Big-endian format.
 */
void UmtsCmasCellbroadcast ::set_message_identifier(const int msg_id) {
    // Write big-endian
    uint16_t bit_mask = 0xff << 8;
    cmas_buffer[1] |= ((msg_id & bit_mask) >> 8);
    bit_mask = 0xff;
    cmas_buffer[2] |= (msg_id & bit_mask);
    this->message_id = msg_id;
}

/*
 * Accessor method to get Data Coding Scheme.
 */
uint UmtsCmasCellbroadcast ::get_dcs() const {
    return this->dcs;
}

/*
 * Method to set Data Coding Scheme.
 * DCS is present at Octet 6.
 */
void UmtsCmasCellbroadcast ::set_dcs(int dcs) {
    cmas_buffer[5] = dcs & 0xff;
    this->dcs = dcs;
}

/*
 * Accessor method to get page index.
 */
uint UmtsCmasCellbroadcast ::get_page_index() const {
    return this->page_index;
}

/*
 * Accessor method to get page count.
 * Page is hardcoded to the number of CB-Data pages.
 * Page parameters(page-index and page-count) are not present in the buffer.
 */
uint UmtsCmasCellbroadcast ::get_page_count() const {
    return this->page_count;
}

/*
 * Set method to insert a list of pages(cb-data buffer) after the header.
 * This method is also responsible to update the page-count value.
 */
void UmtsCmasCellbroadcast ::set_pages(const std::vector<BufferType> &pages) {
    /* Set member field to number of pages.
     */
    page_count = pages.size();

    /* Umts cell broadcast can have multiple pages. this is a handy variable to
     * get the offset beyound the CB-Data.
     */
    header_and_cb_data_length = HEADER_LENGTH;

    /* The format of CB-DATA param in UMTS cell broadcast is as follow.
     * 1 Octet for number of pages.
     * Next 82 Octet for page-1.
     * Next 1 Octet for page-1 size.
     * Follow above 83 Octets for remaining pages.
     */
    cmas_buffer.push_back(static_cast<uint8_t>(page_count));
    header_and_cb_data_length += 1;
    for(auto const &page : pages) {
        /* The content of page is 82 Octet or less.
         */
        auto page_size = static_cast<uint8_t>(page.size());
        /* Insert the page content.
         */
        cmas_buffer.insert(cmas_buffer.end(), page.begin(), page.end());
        /* Insert padding to make page 82 Octet wide.
         */
        if (page_size < CB_DATA_LENGTH) {
            BufferType padding(CB_DATA_LENGTH - page_size, 0);
            cmas_buffer.insert(cmas_buffer.end(), padding.begin(), padding.end());
        }
        header_and_cb_data_length += CB_DATA_LENGTH;

        /* Insert the actual page content length (excluding padding)
         */
        cmas_buffer.push_back(page_size);
        header_and_cb_data_length += 1;
    }

    /* Handy member fields to get offset related to CB-data and WAC in the buffer.
     */
    cb_data_begin = cmas_buffer.begin() + HEADER_LENGTH;
    wac_begin = cmas_buffer.begin() + header_and_cb_data_length;
}

/*
 * Accessor method to get the cb-data.
 * The format for UMTS/EUTRA/NR is as follow.
 * Num-of-pages(1 Octet), Page-1(82 Octet), Page-1-len(1 Octet), Page-2(82
 * Octect), Page-2-len(1 Octet) ... and so on.
 *
 * Curently this method returns empty vector.
 */
BufferType UmtsCmasCellbroadcast ::get_cb_data() const {
    return {};
}

/*
 * Accessor method to get buffer related Warning Area Coordinate.
 * WAC constitues "WAC-Len, WAC-Data". It starts after CB-Data.
 */
BufferType UmtsCmasCellbroadcast ::get_warning_area_coordinate() const {
    auto wac_cbegin = cmas_buffer.cbegin() + header_and_cb_data_length;
    auto wac_cend = cmas_buffer.cend();

    if (wac_cbegin != wac_cend) {
        /* C++17 guaranteed copy-elision.
         */
        return BufferType(wac_cbegin, wac_cend);
    }
    return {};
}

/*
 * Method to set buffer related Warning Area Coordinate.
 * WAC constitues "WAC-Len, WAC-Data". It starts after CB-Data.
 */
void UmtsCmasCellbroadcast ::set_warning_area_coordinate(const BufferType &wac) {
    uint16_t bit_mask = 0xff << 8;

    if (wac.empty()) {
        return;
    }
    auto wac_size = wac.size();
    if (wac_begin != cmas_buffer.end()) {
        LOG(INFO) << "Not overwriting WAC.";
        return;
    }
    /* Write wac length in little endian format
     */
    cmas_buffer.push_back(wac_size & 0xff);
    cmas_buffer.push_back((wac_size & bit_mask) >> 8);

    /* copy wac.
     */
    cmas_buffer.insert(cmas_buffer.end(), wac.begin(), wac.end());

    cb_data_begin = cmas_buffer.begin() + HEADER_LENGTH;
    wac_begin = cmas_buffer.begin() + header_and_cb_data_length;
}

/*
 * Accessor method to return the raw buffer encoded in the GSM cell broadcast
 * format.
 */
BufferType UmtsCmasCellbroadcast ::get_raw_buffer() const {
    return cmas_buffer;
}

std::string UmtsCmasCellbroadcast ::dump() const {
    auto temp = "Umts_Cmas: " + CmasCellbroadcast::dump();
    return temp;
}
