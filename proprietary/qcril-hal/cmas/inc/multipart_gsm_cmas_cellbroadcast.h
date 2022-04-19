/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef _MULTIPART_GSM_CMAS__H_INCLUDED
#define _MULTIPART_GSM_CMAS__H_INCLUDED

#include "gsm_cmas_cellbroadcast.h"

/* This is aggregator but it is more than just a list of individual objects.
 * The main responsibility of this class is to transform a list of
 * GsmCmasbroadcast objects to a single UmtsCmasCellbroadcast object.
 * Client is responsible to identify when to initiate the transformation. i.e.
 * client need to identify when is the last page for GsmCmasbroadcast.
 * Client needs to provide a callback(lambda/functor) which would get invoked
 * once the transformation to a single umtsCmasCellbroadcast is complete.
 * The callback would get the raw-buffer of the umtsCmasCellbroadcast. This
 * buffer can be sent as it is to HLOS layer.
 */
class MultipartGsmCmasCellbroadcast {

   public:
    using CallbackType = std::function<bool(const std::vector<uint8_t>)>;

    /*
     * This is internal represetation of qmi RAT type WMS_BC_RAT_INFO_ENUM_TYPE
     */
    enum class RadioTech {INVALID, GSM, WCDMA, LTE, CDMA, NR5G};

    /*
     * Default ctor
     */
    MultipartGsmCmasCellbroadcast();

    /*
     * Default dtor.
     */
    ~MultipartGsmCmasCellbroadcast();

    /*
     * Default copy ctor.
     */
    MultipartGsmCmasCellbroadcast(const MultipartGsmCmasCellbroadcast &) = default;

    /*
     * Default move ctor.
     */
    MultipartGsmCmasCellbroadcast(MultipartGsmCmasCellbroadcast &&) = default;

    /*
     * Default copy assignment ctor.
     */
    MultipartGsmCmasCellbroadcast &operator=(const MultipartGsmCmasCellbroadcast &) = default;

    /*
     * Default move assignment ctor.
     */
    MultipartGsmCmasCellbroadcast &operator=(MultipartGsmCmasCellbroadcast &&) = default;
    /*
     * Set the RAT on which the multipart GSM cmas cellbroadcast was received
     * by modem.
     */
    void set_radio_tech(RadioTech rat);

    /*
     * Set the Warning Area Coordinate, if available.
     */
    void set_warning_area_coordinate(const std::vector<uint8_t> &wac);

    /*
     * Accessor method to get wac
     */
    [[nodiscard]] const std::vector<uint8_t> &get_warning_area_coordinate() const;

    /*
     * Append GsmCmasCellbroadcast to existing list provided that this gsm_cmas_cellbroadcast
     * belongs to the existing multipart delivery.
     */
     MultipartGsmCmasCellbroadcast &append(const GsmCmasCellbroadcast gsm_cmas_cellbroadcast);

    /*
     * Initiate the transcoding of aggregated gsm cell broadcast to Umts cell
     * broadcast if RAT is non-gsm else dispatch the gsm buffers as it is to
     * Android.
     */
    [[nodiscard]] bool process(const CallbackType &callback) const;

    /*
     * Reset the aggregator cache. This method must be invoked after transcoding
     * is finished. Made public for testing purposes.
     */
    void reset();

    /*
     * Generic dump method for debugging purposes.
     */
    void dump() const;

   private:
    /*
     * GSM to GSM transcoder.
     */
    [[nodiscard]] bool translate_to_gsm_format(const CallbackType &callback) const;

    /*
     * GSM to UMTS transcoder.
     */
    [[nodiscard]] bool translate_to_wcdma_format(const CallbackType &callback) const;

    /*
     * Member field to keep track of RAT for current multi part broadcast.
     */
    RadioTech radio_tech;

    /*
     * Container to track all GSM cellbroadcast related to single multipart broadcast.
     */
    std::vector<GsmCmasCellbroadcast> gsm_cmas_cellbroadcast_list;

    /*
     * Member field to record WAC for current multi part broadcast.
     */
    std::vector<uint8_t> warning_area_coordinate;
};

/*
 * A convenient method to encapsulate the logic related to aggregation and transcoding of
 * incoming cellbroadcast buffer from qmi to android buffer.
 */
void handle_event_report_ind_cmas_buffer(MultipartGsmCmasCellbroadcast::RadioTech rat,
                                         MultipartGsmCmasCellbroadcast::CallbackType callback,
                                         std::vector<uint8_t> qmi_buffer,
                                         std::vector<uint8_t> wac = {});
#endif
