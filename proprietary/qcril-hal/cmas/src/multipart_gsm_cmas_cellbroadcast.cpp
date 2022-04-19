/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "multipart_gsm_cmas_cellbroadcast.h"
#include "umts_cmas_cellbroadcast.h"
#include "android-base/logging.h"
#include <iostream>
/*
 * Default ctor.
 */
MultipartGsmCmasCellbroadcast ::MultipartGsmCmasCellbroadcast() : radio_tech{RadioTech::GSM} {}

/*
 * Default dtor.
 */
MultipartGsmCmasCellbroadcast ::~MultipartGsmCmasCellbroadcast() {
    reset();
}

/*
 * Set the RAT on which the multipart GSM cmas cellbroadcast was received
 * by modem.
 */
void MultipartGsmCmasCellbroadcast ::set_radio_tech(RadioTech rat) {
    radio_tech = rat;
}

/*
 * Reset the aggregator cache. This method must be invoked after transcoding
 * is finished. Made public for testing purposes.
 */
void MultipartGsmCmasCellbroadcast ::reset() {
    radio_tech = RadioTech::INVALID;
    gsm_cmas_cellbroadcast_list.clear();
    warning_area_coordinate.clear();
}

/*
 * Append GsmCmasCellbroadcast to existing list provided that this gsm_cmas_cellbroadcast
 * belongs to the existing multipart delivery.
 */
MultipartGsmCmasCellbroadcast &MultipartGsmCmasCellbroadcast ::append(
    const GsmCmasCellbroadcast gsm_cmas_cellbroadcast) {
    gsm_cmas_cellbroadcast_list.push_back(gsm_cmas_cellbroadcast);
    return *this;
}

/*
 * Private method: GSM to GSM transcoder.
 */
bool MultipartGsmCmasCellbroadcast ::translate_to_gsm_format(const CallbackType &callback) const {
    bool status{false};

    for (auto const &it : gsm_cmas_cellbroadcast_list) {
        /* gsm_cmas_cellbroadcast_list contains list of GsmCmasCellbroadcast.
         * We dont need to do any transcoding for cell broadcastst received in GSM RAT.
         * Send each GsmCmasCellbroadcast to Android.
         */
        status = callback(it.get_raw_buffer());
        if (!status) {
            return status;
        }
    }
    return status;
}

/*
 * Private method: GSM to UMTS transcoder.
 */
bool MultipartGsmCmasCellbroadcast ::translate_to_wcdma_format(const CallbackType &callback) const {
    bool status{false};
    auto page_count = gsm_cmas_cellbroadcast_list.size();

    /* gsm_cmas_cellbroadcast_list contains list of GsmCmasCellbroadcast.
     * We need to transcode the received multipart gsm broadcasts to single umts
     * cell broadcast.
     */
    if (page_count > 0) {
        /* All elements of gsm_cmas_cellbroadcast_list contains gsm cell
         * broadcast that belongs to same multipart broadcast. Hence the header
         * from first element is sufficent to construct header for
         * UmtsCmasCellbroadcast.
         */
        auto &it = gsm_cmas_cellbroadcast_list[0];

        /* Instantiate and initialize UmtsCmasCellbroadcast header using first element
         * gsm_cmas_cellbroadcast_list
         */
        UmtsCmasCellbroadcast umts_cmas_cellbroadcast;
        umts_cmas_cellbroadcast.set_geographical_scope(it.get_geographical_scope());
        umts_cmas_cellbroadcast.set_serial_number(it.get_serial_number());
        umts_cmas_cellbroadcast.set_message_identifier(it.get_message_identifier());
        umts_cmas_cellbroadcast.set_dcs(it.get_dcs());

        /* Create a list of all pages by iterating over
         * gsm_cmas_cellbroadcast_list and extracting cb_data from each element.
         */
        std::vector<BufferType> pages;
        for (auto const &gsm_cmas_cellbroadcast : gsm_cmas_cellbroadcast_list) {
            auto cb_data = gsm_cmas_cellbroadcast.get_cb_data();
            pages.push_back(cb_data);
        }

        /* Let umts_cmas_cellbroadcast transcode list of cb_data as per UMTS
         * header format.
         */
        umts_cmas_cellbroadcast.set_pages(pages);

        /* Set WAC in umts_cmas_cellbroadcast.
         */
        umts_cmas_cellbroadcast.set_warning_area_coordinate(this->get_warning_area_coordinate());

        /* Dump for debugging purpose.
         */
        LOG(INFO) << umts_cmas_cellbroadcast.dump();

        /* Umts transcoding is finished, call client callback to dispatch the
         * buffer to android.
         */
        status = callback(umts_cmas_cellbroadcast.get_raw_buffer());
    }

    return status;
}

/*
 * Initiate the transcoding of aggregated gsm cell broadcast to Umts cell
 * broadcast if RAT is non-gsm else dispatch the gsm buffers as it is to
 * Android.
 */
bool MultipartGsmCmasCellbroadcast ::process(const CallbackType &callback) const {
    bool status{false};

    /* Delegate the processing to GSM or UMTS handler based on the RAT on
     * which current multipart broadcast was received.
     */
    if (radio_tech == RadioTech::GSM) {
        status = translate_to_gsm_format(callback);
    } else {
        status = translate_to_wcdma_format(callback);
    }

    return status;
}

/*
 * Set the Warning Area Coordinate, if available.
 */
void MultipartGsmCmasCellbroadcast ::set_warning_area_coordinate(const std::vector<uint8_t> &wac) {
    this->warning_area_coordinate = std::move(wac);
}

/*
 * Accessor method to get wac
 */
const std::vector<uint8_t> &MultipartGsmCmasCellbroadcast ::get_warning_area_coordinate() const {
    return this->warning_area_coordinate;
}

/*
 * Generic dump method for debugging purposes.
 */
void MultipartGsmCmasCellbroadcast ::dump() const {}

/*
 * Free method.
 * This is the only method exposed by this library.
 * Client can invoke this method multiple times upon every arrival of
 * qmi_event_report_ind. Thhe entire business logic of detecting the multipart
 * broadcast and identifying last page arrival is being implemented in this method.
 *
 * This method is not thread-safe.
 */
void handle_event_report_ind_cmas_buffer(MultipartGsmCmasCellbroadcast::RadioTech rat,
                                         MultipartGsmCmasCellbroadcast::CallbackType callback,
                                         std::vector<uint8_t> qmi_buffer,
                                         std::vector<uint8_t> wac) {

    /* Ensure that we have some payload beyond header in the buffer received
     * from qmi.
     */
    if (std::size(qmi_buffer) <= CmasCellbroadcast::HEADER_LENGTH) {
        LOG(ERROR) << "Incorrect qmi buffer received.";
        return;
    }

    /* The qmi buffer as part of qmi_event_report_ind is always in GSM header format.
     * Decode the buffer using GsmCmasCellbroadcast.
     * */
    auto cmas_cellbroadcast = GsmCmasCellbroadcast(std::move(qmi_buffer));
    LOG(INFO) << cmas_cellbroadcast.dump();

    /* Local static variable to aggregate the multipart broadcast.
     */
    static auto multipart_gsm_cmas_cellbroadcast = MultipartGsmCmasCellbroadcast();

    /* Set the RAT on which this cell broadcast was received.
     * It is important field to decide how to further transcode this entire
     * multipart broadcast.
     */
    multipart_gsm_cmas_cellbroadcast.set_radio_tech(rat);

    /* Set the WAC if provided by client. WAC is optional from network.
     */
    if (!wac.empty()) {
        multipart_gsm_cmas_cellbroadcast.set_warning_area_coordinate(wac);
    }

    /* All key fields are set, append the GSM broadcast object.
     */
    multipart_gsm_cmas_cellbroadcast.append(cmas_cellbroadcast);

    /* If we just now received the last page then trigger the transcoding.
     */
    if (cmas_cellbroadcast.get_page_index() == cmas_cellbroadcast.get_page_count()) {
        bool status = false;
        status = multipart_gsm_cmas_cellbroadcast.process(callback);
        if (!status) LOG(ERROR) << "Failed to process multipart cmas broadcast";

        multipart_gsm_cmas_cellbroadcast.reset();
    }
}
