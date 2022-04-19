/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "cmas_cellbroadcast.h"
#include <sstream>

/*
 * Default ctor.
 * Set initial size of cmas_buffer to 88 octet and initialize to 0.
 */
CmasCellbroadcast ::CmasCellbroadcast()
    : cmas_buffer(HEADER_LENGTH, 0),
      geographical_scope{0},
      serial_number{0},
      message_id{0},
      dcs{0},
      page_index{0},
      page_count{0},
      header_and_cb_data_length{0} {}

/*
 * Common ctor.
 * copy input buffer to cmas_buffer.
 */
CmasCellbroadcast ::CmasCellbroadcast(std::vector<uint8_t> buffer)
    : cmas_buffer{buffer},
      geographical_scope{0},
      serial_number{0},
      message_id{0},
      dcs{0},
      page_index{0},
      page_count{0},
      header_and_cb_data_length{0} {}

/*
 * Dump utility for debugging purpose.
 */
std::string CmasCellbroadcast ::dump() const {
    static constexpr const char *hex_upper = "0123456789ABCDEF";
    std::ostringstream os;

    for (const auto &val : cmas_buffer) {
        os << hex_upper[val >> 4];
        os << hex_upper[val & 0x0f];
    }
    return  os.str();
}
