/******************************************************************************
#  Copyright (c) 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once

#include <utility>
#include <vector>
#include <framework/Log.h>
#include <modules/qmi/ModemEndPoint.h>
#include <modules/qmi/NasModemEndPointModule.h>
#include <network_access_service_v01.h>

std::string toString(nas_radio_if_enum_v01 radio_if);
std::string toString(nas_active_band_enum_v01 active_band);

struct PhysChanInfo {
    bool bandwidth_info_list_valid;
    uint32_t bandwidth_info_list_len;
    nas_rf_bandwidth_info_type_v01 bandwidth_info_list[NAS_RF_BAND_INFO_LIST_MAX_V01];
    bool pcell_info_valid;
    nas_lte_cphy_pcell_info_type_v01 pcell_info;
    bool scell_info_list_valid;
    uint32_t scell_info_list_len;
    nas_cphy_scell_info_v01 scell_info_list[NAS_MAX_SCELL_LIST_LEN_V01];
    bool radio_if_valid;
    nas_radio_if_enum_v01 radio_if;
    bool physical_cell_id_valid;
    uint32_t physical_cell_id;

    bool operator==(const PhysChanInfo &other) {
        // Predicates are ordered from the least expensive to evaluate
        // to the most expensive to evaluate on average.
        return (
            bandwidth_info_list_valid == other.bandwidth_info_list_valid &&
            pcell_info_valid == other.pcell_info_valid &&
            scell_info_list_valid == other.scell_info_list_valid &&
            bandwidth_info_list_len == other.bandwidth_info_list_len &&
            scell_info_list_len == other.scell_info_list_len &&
            radio_if_valid == other.radio_if_valid &&
            radio_if == other.radio_if &&
            physical_cell_id_valid == other.physical_cell_id_valid &&
            physical_cell_id == other.physical_cell_id &&
            memcmp(&pcell_info, &other.pcell_info, sizeof(pcell_info)) == 0 &&
            memcmp(&bandwidth_info_list, &other.bandwidth_info_list, bandwidth_info_list_len * sizeof(nas_rf_bandwidth_info_type_v01)) == 0 &&
            memcmp(&scell_info_list, &other.scell_info_list, scell_info_list_len * sizeof(nas_cphy_scell_info_v01)) == 0
        );
    }

    bool operator!=(const PhysChanInfo &other) {
        return !(*this == other);
    }

    bool compareAndAssign(const PhysChanInfo &other) {
        bool isEqual = (*this == other);
        if (!isEqual) {
          *this = other;
        }
        return !isEqual;
    }
};

class QmiNasRfBandInfoMessage: public QmiIndicationMessage<QmiNasRfBandInfoMessage, nas_rf_band_info_ind_msg_v01>,
                               public add_message_id<QmiNasRfBandInfoMessage>
{
    public:
    static const constexpr char *MESSAGE_NAME = "com.qualcomm.qti.qcril.qmi.QmiNasRfBandInfoMessage";
    using payload_t = nas_rf_band_info_ind_msg_v01;

    QmiNasRfBandInfoMessage() = delete;
    ~QmiNasRfBandInfoMessage() {}

    QmiNasRfBandInfoMessage(const nas_rf_band_info_ind_msg_v01 &band_info):
        QmiIndicationMessage(band_info) {
        // For testing only
        //auto &payload = getPayload();
        //payload.nas_rf_bandwidth_info_valid = true;
        //payload.nas_rf_bandwidth_info.bandwidth = NAS_LTE_BW_NRB_100_V01;
    }
    std::string dump() {
        return std::string("QmiNasRfBandInfoMessage[if: ") + toString(getPayload().rf_band_info.radio_if) +
            " active_band: " + toString(getPayload().rf_band_info.active_band) +
            " active_channel: " + std::to_string(getPayload().rf_band_info.active_channel) + "]";

    }
    std::shared_ptr<UnSolicitedMessage> clone() {
        return std::make_shared<QmiNasRfBandInfoMessage>(getPayload());
    }

    virtual bool isSameMessage(std::shared_ptr<Message> msg) override {
        if (msg) {
            return (get_message_id() == msg->get_message_id());
        }
        return false;
    }
};

class QmiNasLteCphyCaIndMessage: public QmiIndicationMessage<QmiNasLteCphyCaIndMessage, nas_lte_cphy_ca_ind_msg_v01>,
                                 public add_message_id<QmiNasLteCphyCaIndMessage>
{
    public:
    static const constexpr char *MESSAGE_NAME = "com.qualcomm.qti.qcril.qmi.QmiNasLteCphyCaIndMessage";
    using payload_t = nas_lte_cphy_ca_ind_msg_v01;

    QmiNasLteCphyCaIndMessage() = delete;
    ~QmiNasLteCphyCaIndMessage() {}

    QmiNasLteCphyCaIndMessage(const nas_lte_cphy_ca_ind_msg_v01 &lte_cphy_ca):
        QmiIndicationMessage(lte_cphy_ca) {
            // For testing only
            // auto &payload = getPayload();
            // payload.scell_info_valid = true;
            // payload.scell_info.cphy_ca_dl_bandwidth = NAS_LTE_CPHY_CA_BW_NRB_6_V01;
        }

    std::string dump() {
        return std::string("QmiNasLteCphyCaIndMessage{") +
            " pci: " + std::to_string(getPayload().cphy_ca.pci) +
            " freq: " + std::to_string(getPayload().cphy_ca.freq) +
            " scell_state: " + std::to_string(getPayload().cphy_ca.scell_state) +
            (getPayload().cphy_ca_dl_bandwidth_valid ? "[CPHY_CA_DL_BANDWIDTH]" : "" ) +
            (getPayload().scell_info_valid ? "[SCELL_INFO]" : "" ) +
            (getPayload().pcell_info_valid ? "[PCELL_INFO]" : "" ) +
            (getPayload().scell_idx_valid ? "[SCELL_IDX]" : "" ) +
            (getPayload().cphy_ca_aggregated_dl_bandwidth_valid ?
                        "[CPHY_CA_AGGREGATED_DL_BANDWIDTH]":"" ) +
            (getPayload().num_scells_configured_valid ? "[NUM_SCELLS_CONFIGURED]" : "" ) +
            (getPayload().unchanged_scell_info_list_valid ? "[UNCHANGED_SCELL_INFO_LIST]" : "" ) +
            " }";

    }
    std::shared_ptr<UnSolicitedMessage> clone() {
        return std::make_shared<QmiNasLteCphyCaIndMessage>(getPayload());
    }

    virtual bool isSameMessage(std::shared_ptr<Message> msg) override {
        if (msg) {
            return (get_message_id() == msg->get_message_id());
        }
        return false;
    }
};

using QmiNasRegisterForIndResponse = QmiResponse<nas_indication_register_resp_msg_v01>;

class QmiNasRegisterForIndRequest: public SolicitedMessage<QmiNasRegisterForIndResponse>,
    public add_message_id<QmiNasRegisterForIndRequest>
{
    public:
        static const constexpr char *MESSAGE_NAME = "com.qualcomm.qti.qcril.qmi.QmiNasRegisterForIndRequest";
        using cb_t = GenericCallback<QmiNasRegisterForIndResponse>::cb_t;
        using IndicationList = std::vector<std::pair<unsigned,bool>>;
        QmiNasRegisterForIndRequest() = delete;
        ~QmiNasRegisterForIndRequest() {}

        QmiNasRegisterForIndRequest(const IndicationList &indication_ids, cb_t callbackfn):
            SolicitedMessage<QmiNasRegisterForIndResponse>(get_class_message_id()),
            mIndicationIds(indication_ids) {
                mName = MESSAGE_NAME;
                GenericCallback<QmiNasRegisterForIndResponse> cb(callbackfn);
                setCallback(&cb);
            }

        QmiNasRegisterForIndRequest(IndicationList &&indication_ids, cb_t callbackfn):
            SolicitedMessage<QmiNasRegisterForIndResponse>(get_class_message_id()),
            mIndicationIds(std::move(indication_ids)) {
                mName = MESSAGE_NAME;
                GenericCallback<QmiNasRegisterForIndResponse> cb(callbackfn);
                setCallback(&cb);
            }

        std::string dump() {
            return mName;
        }

        IndicationList getIndicationIds() { return mIndicationIds; }
    private:
        IndicationList mIndicationIds;
};

class NasModemEndPoint : public ModemEndPoint {
  public:
    static constexpr const char *NAME = "NAS";
    NasModemEndPoint() : ModemEndPoint(NAME) {
      mModule = new NasModemEndPointModule("NasModemEndPointModule", *this);
      mModule->init();
    }
    ~NasModemEndPoint() {
      Log::getInstance().d("[NasModemEndPoint]: destructor");
      delete mModule;
      mModule = nullptr;
    }

    void requestSetup(string clientToken, GenericCallback<string>* cb);
    void registerForRfBandInfoInd(bool enable, std::shared_ptr<MessageContext> ctx, QmiNasRegisterForIndRequest::cb_t cb);
    void registerForIndications(QmiNasRegisterForIndRequest::IndicationList list,
            std::shared_ptr<MessageContext> ctx, QmiNasRegisterForIndRequest::cb_t cb);

    void getLteCarrierAggregationEventInfo(PhysChanInfo &phyChanInfo);
    void getRfBandInfo(PhysChanInfo &phyChanInfo);
    void getPhysicalCellId(PhysChanInfo &phyChanInfo);
    void getPhysChanConfig(PhysChanInfo &phyChanInfo);
};
