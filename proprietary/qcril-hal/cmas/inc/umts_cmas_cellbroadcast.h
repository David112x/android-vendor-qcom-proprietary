/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef _UMTS_CMAS_H_INCLUDED
#define _UMTS_CMAS_H_INCLUDED

#include "cmas_cellbroadcast.h"

/*
 * UmtsCmasCellbroadcast is the concrete class to provide behavior related to
 * Umts/EUTRA/NR CB. The main responsibility of this class is to decode the raw cellbroadcast
 * buffer and provide accessor and setter methods for various fields of cellbroadcast.
 */
class UmtsCmasCellbroadcast : public CmasCellbroadcast {
   public:
    /*
     * Default ctor.
     */
    UmtsCmasCellbroadcast();

    /*
     * ctor only for unit test purpose.
     * Takes the Umts Cellbroadcast encoded buffer and parses it.
     */
    UmtsCmasCellbroadcast(BufferType buffer);

    /*
     * Default dtor.
     */
    ~UmtsCmasCellbroadcast() override = default;

    /*
     * Default copy ctor.
     */
    UmtsCmasCellbroadcast(const UmtsCmasCellbroadcast &) = default;

    /*
     * Default move ctor.
     */
    UmtsCmasCellbroadcast(UmtsCmasCellbroadcast &&) = default;

    /*
     * Default copy assignment ctor.
     */
    UmtsCmasCellbroadcast &operator=(const UmtsCmasCellbroadcast &) = default;

    /*
     * Default move assignment ctor.
     */
    UmtsCmasCellbroadcast &operator=(UmtsCmasCellbroadcast &&) = default;

    /*
     * Accessor method to get geographical scope.
     * Geographical scode is at 6-7 bits of 4th Octet.
     */
    [[nodiscard]] uint get_geographical_scope() const override;

    /*
     * Method to set geographical scope.
     * Geographical scode is at 6-7 bits of 4th Octet.
     */
    void set_geographical_scope(const int geographical_scope);

    /*
     * Accessor method to get serial number.
     * Serial number is present at 0-4 bits of Octet 4 and Octect 5
     * in Big-endian format.
     */
    [[nodiscard]] uint get_serial_number() const override;

    /*
     * Method to set serial number.
     * Serial number is present at 0-4 bits of Octet 4 and Octect 5
     * in Big-endian format.
     */
    void set_serial_number(const int serial_num);

    /*
     * Accessor method to get message id.
     * Message id is present in Octet 2-3 in Big-endian format.
     */
    [[nodiscard]] uint get_message_identifier() const override;

    /*
     * Method to set message id.
     * Message id is present in Octet 2-3 in Big-endian format.
     */
    void set_message_identifier(const int msg_id);

    /*
     * Accessor method to get Data Coding Scheme.
     * DCS is present at Octet 6.
     */
    [[nodiscard]] uint get_dcs() const override;

    /*
     * Method to set Data Coding Scheme.
     * DCS is present at Octet 6.
     */
    void set_dcs(int dcs);

    /*
     * Accessor method to get page index.
     * For UMTS/EUTRA/NR the page index is always 1.
     */
    [[nodiscard]] uint get_page_index() const override;

    /*
     * Accessor method to get page count.
     * Page is hardcoded to the number of CB-Data pages.
     * Page parameters(page-index and page-count) are not present in the buffer.
     */
    [[nodiscard]] uint get_page_count() const override;

    /*
     * Set method to insert a list of pages(cb-data buffer) after the header.
     * This method is also responsible to update the page-count value.
     */
    void set_pages(const std::vector<BufferType> &pages);

    /*
     * Accessor method to get the cb-data.
     * The format for UMTS/EUTRA/NR is as follow.
     * Num-of-pages(1 Octet), Page-1(82 Octet), Page-1-len(1 Octet), Page-2(82
     * Octect), Page-2-len(1 Octet) ... and so on.
     *
     * Curently this method returns empty vector.
     */
    [[nodiscard]] BufferType get_cb_data() const override;

    /*
     * Accessor method to get buffer related Warning Area Coordinate.
     * WAC constitues "WAC-Len, WAC-Data". It starts after CB-Data.
     */
    [[nodiscard]] BufferType get_warning_area_coordinate() const override;

    /*
     * Method to set buffer related Warning Area Coordinate.
     * WAC constitues "WAC-Len, WAC-Data". It starts after CB-Data.
     */
    void set_warning_area_coordinate(const BufferType &wac);

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
