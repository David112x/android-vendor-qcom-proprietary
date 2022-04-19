/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef _GSM_CMAS_H_INCLUDED
#define _GSM_CMAS_H_INCLUDED

#include "cmas_cellbroadcast.h"

/*
 * GsmCmasCellbroadcast is the concrete class to provide behavior related to GSM
 * CB. The main responsibility of this class is to decode the gsm cellbroadcast
 * buffer and provide accessor method for various fields of gsm cellbroadcast.
 */
class GsmCmasCellbroadcast : public CmasCellbroadcast {
   public:
    /*
     * Common ctor.
     * Takes the GSM Cellbroadcast encoded buffer and parses it.
     */
    GsmCmasCellbroadcast(BufferType buffer);

    /*
     * Default dtor.
     */
    ~GsmCmasCellbroadcast() override = default;

    /*
     * Default copy ctor.
     */
    GsmCmasCellbroadcast(const GsmCmasCellbroadcast &) = default;

    /*
     * Default move ctor.
     */
    GsmCmasCellbroadcast(GsmCmasCellbroadcast &&) = default;

    /*
     * Default copy assignment ctor.
     */
    GsmCmasCellbroadcast &operator=(const GsmCmasCellbroadcast &) = default;

    /*
     * Default move assignment ctor.
     */
    GsmCmasCellbroadcast &operator=(GsmCmasCellbroadcast &&) = default;

    /*
     * Accessor method to get geographical scope.
     * Geographical scode is at 6-7 bits of 1st Octet.
     */
    [[nodiscard]] uint get_geographical_scope() const override;

    /*
     * Accessor method to get serial number.
     * Serial number is present at 0-4 bits of Octet 1 and Octect 2
     * in Big-endian format.
     */
    [[nodiscard]] uint get_serial_number() const override;

    /*
     * Accessor method to get message id.
     * Message id is present in Octet 3-4 in Big-endian format.
     */
    [[nodiscard]] uint get_message_identifier() const override;

    /*
     * Accessor method to get Data Coding Scheme.
     * DCS is present at Octet 5.
     */
    [[nodiscard]] uint get_dcs() const override;

    /*
     * Accessor method to get page index.
     * Page index is present at 4-7 bits of Octet 6.
     */
    [[nodiscard]] uint get_page_index() const override;

    /*
     * Accessor method to get page count.
     * Page index is present at 0-3 bits of Octet 6.
     */
    [[nodiscard]] uint get_page_count() const override;

    /*
     * Accessor method to get cell broadcast data buffer.
     * CB-Data is 82 Octet long and starts from Octet 7.
     */
    [[nodiscard]] BufferType get_cb_data() const override;

    /*
     * Accessor method to get buffer related Warning Area Coordinate.
     * WAC constitues "WAC-Len, WAC-Data". It starts after CB-Data.
     */
    [[nodiscard]] BufferType get_warning_area_coordinate() const override;

    /*
     * Accessor method to return the raw buffer encoded in the GSM cell broadcast
     * format.
     */
    [[nodiscard]] BufferType get_raw_buffer() const override;

    /*
     * Util method to dump key informations for debug purpose.
     */
    [[nodiscard]] std::string dump() const override;
};
#endif
