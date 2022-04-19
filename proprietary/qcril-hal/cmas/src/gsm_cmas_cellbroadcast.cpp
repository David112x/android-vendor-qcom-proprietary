/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "gsm_cmas_cellbroadcast.h"
#include "android-base/logging.h"
#include <iostream>

/*
 * Common ctor.
 * Input buffer must be at least 6 Octet long.
 * Decode the buffer using Gsm Cmas header format.
 * Refer GsmCmasCellbroadcast.h for header information.
 */
GsmCmasCellbroadcast ::GsmCmasCellbroadcast(BufferType buffer) : CmasCellbroadcast(buffer) {
    if (std::size(buffer) > HEADER_LENGTH) {
        geographical_scope = (cmas_buffer[0] & 0xc0) >> 6;
        serial_number = ((cmas_buffer[0] & 0xff) << 8) | (cmas_buffer[1] & 0xff);
        message_id = ((cmas_buffer[2] & 0xff) << 8) | (cmas_buffer[3] & 0xff);
        dcs = cmas_buffer[4] & 0xff;
        page_index = (cmas_buffer[5] & 0xf0) >> 4;
        page_count = cmas_buffer[5] & 0x0f;
        cb_data_begin = cmas_buffer.begin() + HEADER_LENGTH;
        header_and_cb_data_length = std::size(buffer);
    }
}

/*
 * Accessor method to get geographical scope.
 */
uint GsmCmasCellbroadcast ::get_geographical_scope() const {
    return this->geographical_scope;
}

/*
 * Accessor method to get serial number.
 */
uint GsmCmasCellbroadcast ::get_serial_number() const {
    return this->serial_number;
}

/*
 * Accessor method to get message id.
 */
uint GsmCmasCellbroadcast ::get_message_identifier() const {
    return this->message_id;
}

/*
 * Accessor method to get DCS.
 */
uint GsmCmasCellbroadcast ::get_dcs() const {
    return this->dcs;
}

/*
 * Accessor method to get page index.
 */
uint GsmCmasCellbroadcast ::get_page_index() const {
    return this->page_index;
}

/*
 * Accessor method to get total page count as part of current multipart broadcast
 */
uint GsmCmasCellbroadcast ::get_page_count() const {
    return this->page_count;
}

/*
 * Accessor method to get cb-data aka page content.
 */
BufferType GsmCmasCellbroadcast ::get_cb_data() const {
    if (std::size(cmas_buffer) <= HEADER_LENGTH) {
        LOG(ERROR) << "Invalid cmas buffer.";
        return {};
    }
    auto cb_cbegin = cmas_buffer.cbegin() + HEADER_LENGTH;
    auto cb_cend = cmas_buffer.cbegin() + header_and_cb_data_length;
    if (cb_cend != cmas_buffer.end()) {
        LOG(INFO) << "Missing cb_data";
        return {};
    }
    /* C++17 guaranteed copy-elision.
     */
    return BufferType (cb_cbegin, cb_cend);
}

/*
 * Accessor method to get wac.
 */
BufferType GsmCmasCellbroadcast ::get_warning_area_coordinate() const {
    if (std::size(cmas_buffer) <= HEADER_LENGTH) {
        LOG(ERROR) << "Invalid cmas buffer.";
        return {};
    }
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
 * Accessor method to get buffer in GSM header format.
 */
BufferType GsmCmasCellbroadcast ::get_raw_buffer() const {
    return cmas_buffer;
}

/*
 * Util method to dump key informations for debug purpose.
 */
std::string GsmCmasCellbroadcast ::dump() const {
    return "Gsm_Cmas: " + CmasCellbroadcast::dump();
}

