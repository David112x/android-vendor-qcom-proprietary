/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef _CMAS_H_INCLUDED
#define _CMAS_H_INCLUDED

#include <string>
#include <vector>

using BufferType = std::vector<uint8_t>;

/*
 * CmasCellbroadcast is the abstract base class to provide common behaviour for
 * various concrete classes like GsmCmasCellbroadcast and UmtsCmasCellbroadcast.
 */
class CmasCellbroadcast {

   protected:
    /*
     * This is raw buffer. Encoding is dependent on concrete class */
    std::vector<uint8_t> cmas_buffer;

    /*
     * Geographical scope of the cell broadcast. The scope could be cell wide or
     * plmn wide. Currently this is opaque to qcril. */
    uint geographical_scope;

    /*
     * Serial number contains the code to differentiates msgs with the same
     * message identifier (message codes can be allocated by the operator).
     */
    uint serial_number;

    /*
     * The message identifier identifies the type of the CB message.
     * Numbers in the range of 0-999 can be defined by the operator and
     * correspond to the channel that is selected on many end user devices. For
     * example, the message identifier for channel location is typically 50.
     * The device or the SIM can store several message identifiers and these can
     * correspond to the “services” that the device looks for.
     * For Public Warning Systems (PWS), networks are only allowed to use
     * identifiers between 4352-6399. These are used to identify different types
     * of alerts: for example 4370 is the CMAS Identifier for a presidential
     * alert.
     *
     * Currently this is opaque to qcril.
     */
    uint message_id;

    /*
     * Data Coding Scheme.
     * If the message is not set to immediate display, this parameter tells the
     * mobile handset how to display the message and which alphabet/language to
     * use when interpreting the message. Through the use of an interface on the
     * handset the user is able to ignore messages in an unfamiliar language.
     * However, some warning messages may be transmitted in mandatory languages
     * that are forcibly displayed.
     */
    uint dcs;

    /* A single Cell broadcast may be delivered in multiple parts.
     * page_index indicates the index of current page.
     */
    int page_index;

    /*
     * Total number of pages that constitutes this multipart cell broadcast message.
     */
    int page_count;

    /*
     * This is handy variable to quickly jump past the header and cb_data field
     * in the raw buffer.
     */
    int header_and_cb_data_length;

    /*
     * This is the raw buffer representing the actual text or binary of the cell broadcast.
     * This of it as the actual message that needs to be conveyed to users.
     */
    BufferType cb_data;

    /*
     * Warning Area Coordinate(WAC). This is an envelope for the current
     * emergency. HLOS is reposible to identify if the UE is currently within
     * this envelope or not.
     */
    BufferType wac;

    /*
     * Mutable iterator to the begining of the cb_data buffer in the CB.
     */
    BufferType::iterator cb_data_begin;

    /*
     * Mutable iterator to the begining of the Warning Area Coordinate(WAC).
     * If the WAC is absent then wac_begin points to the one past end of the buffer.
     */
    BufferType::iterator wac_begin;

   public:
    /*
     * Serial-Number = 2 Octet.
     * Message-id = 2 Octet.
     * DCS = 1 Octet.
     * Page-index = 4 bits.
     * Page-count = 4 bits.
     * Page(cb_data) = 82 Octet.
     */
    static constexpr int HEADER_LENGTH = 6;
    static constexpr int CB_DATA_LENGTH = 82;

    /*
     * Default ctor.
     */
    CmasCellbroadcast();

    /*
     * Common ctor.
     * Takes the raw buffer and parse/decodes it.
     * Ctor of concrete class provides the decode functionality.
     * This baseclass ctor is reposible to reserve the buffer size to the
     * minimum of header size.
     */
    CmasCellbroadcast(BufferType buffer);

    /*
     * Default dtor.
     */
    virtual ~CmasCellbroadcast() = default;

    /*
     * Default copy ctor.
     */
    CmasCellbroadcast(const CmasCellbroadcast &) = default;

    /*
     * Default move ctor.
     */
    CmasCellbroadcast(CmasCellbroadcast &&) = default;

    /*
     * Default copy assignment ctor.
     */
    CmasCellbroadcast &operator=(const CmasCellbroadcast &) = default;

    /*
     * Default move assignment ctor.
     */
    CmasCellbroadcast &operator=(CmasCellbroadcast &&) = default;

    /*
     * Decode the input buffer and extract geographical_scope.
     */
    [[nodiscard]] virtual uint get_geographical_scope() const = 0;

    /*
     * Decode the input buffer and extract serial number.
     */
    [[nodiscard]] virtual uint get_serial_number() const = 0;

    /*
     * Decode the input buffer and extract message id.
     */
    [[nodiscard]] virtual uint get_message_identifier() const = 0;

    /*
     * Decode the input buffer and extract dcs.
     */
    [[nodiscard]] virtual uint get_dcs() const = 0;

    /*
     * Decode the input buffer and extract page index.
     */
    [[nodiscard]] virtual uint get_page_index() const = 0;

    /*
     * Decode the input buffer and extract total number of pages.
     */
    [[nodiscard]] virtual uint get_page_count() const = 0;

    /*
     * Decode the input buffer and extract buffer related to cb_data.
     */
    [[nodiscard]] virtual BufferType get_cb_data() const = 0;

    /*
     * Decode the input buffer and extract buffer related to WAC.
     */
    [[nodiscard]] virtual BufferType get_warning_area_coordinate() const = 0;

    /*
     * Get the raw buffer encoded as per the concrete class implementation.
     */
    [[nodiscard]] virtual BufferType get_raw_buffer() const = 0;

    /*
     * Util method to dump key informations for debug purpose.
     */
    [[nodiscard]] virtual std::string dump() const;
};
#endif
