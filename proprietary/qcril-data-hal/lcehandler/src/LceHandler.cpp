/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#include "LceHandler.h"
#include "request/LinkCapIndMessage.h"
#include "qcril_data.h"
using namespace rildata;

bool LceHandler::toggleReporting(ReportFilter_t reporting_state) {
   Log::getInstance().d("[" + mName + "]:" + __func__ );

   if(!qcril_data_is_threshold_based_reporting_supported())
   {
     Log::getInstance().d("[" + mName + "]: Threshold based reporting not supported");
     return true;
   }
   bool retstate = false;

   if (reporting_state == m_reportingState) {
      Log::getInstance().d("[" + mName + "]: Reporting state unchanged, got  = "
       + std::to_string(reporting_state));
      retstate = true;
   } else {
      qcril_data_lce_rpt_state state = (reporting_state == ReportFilter_t::enabled) ?
                                       lce_report_on:lce_report_off;
      int uprpt = qcril_data_lqe_toggle_reporting(state, LQE_DATA_UPLINK_DIRECTION);
      int downrpt = qcril_data_lqe_toggle_reporting(state, LQE_DATA_DOWNLINK_DIRECTION);
      if ((uprpt == LQE_SUCCESS) && (downrpt == LQE_SUCCESS)) {
         m_reportingState = reporting_state;
         retstate = true;
      } else {
         Log::getInstance().d("[" + mName + "]:" +
            " Failed to change reporting state = " + std::to_string(reporting_state) +
            " try fallback to original reporting state  = " + std::to_string(m_reportingState));

         qcril_data_lce_rpt_state fallbackstate = (m_reportingState == ReportFilter_t::enabled) ?
                                                lce_report_on:lce_report_off;

         if (uprpt == LQE_SUCCESS) {
            if (LQE_FAILURE == qcril_data_lqe_toggle_reporting(fallbackstate,
                                                LQE_DATA_UPLINK_DIRECTION)) {
               // Just error out...it's just not our day
               Log::getInstance().d("[" + mName + "]: Failed to fallback uplink reporting state");
            }
         }

         if (downrpt == LQE_SUCCESS) {
            if (LQE_FAILURE == qcril_data_lqe_toggle_reporting(fallbackstate,
                                                LQE_DATA_DOWNLINK_DIRECTION)) {
               // Just error out...it's just not our day
               Log::getInstance().d("[" + mName + "]: Failed to fallback downlink reporting state");
            }
         }
      }
   }

   return retstate;
}

LinkCapCriteriaResult_t LceHandler::setCriteria(LinkCapCriteria_t criteria) {
   Log::getInstance().d("[" + mName + "]:" + __func__ );

   LinkCapCriteriaResult_t result = LinkCapCriteriaResult_t::success;

   if ((criteria.ran != RAN_t::WCDMA) &&
      (criteria.ran != RAN_t::LTE)) {
      Log::getInstance().d("[" + mName + "]:" +
         " RAN " + std::to_string((int)criteria.ran) +
         " not supported by modem for bandwidth reporting!");
      return LinkCapCriteriaResult_t::request_not_supported;
   }
   if (criteria.thresholdsDownlinkKbps.size() >= QMI_OTT_DL_THRESHOLD_LIST_LEN_V01 ||
       criteria.thresholdsUplinkKbps.size() >= QMI_OTT_UL_THRESHOLD_LIST_LEN_V01) {
      Log::getInstance().d("[" + mName + "]:" +
         " threshold list size out of bounds! downlink=" +
         std::to_string(criteria.thresholdsDownlinkKbps.size()) + ", uplink=" +
         std::to_string(criteria.thresholdsUplinkKbps.size()));
      return LinkCapCriteriaResult_t::internal_err;
   }

   std::sort(criteria.thresholdsUplinkKbps.begin(), criteria.thresholdsUplinkKbps.end());
   std::sort(criteria.thresholdsDownlinkKbps.begin(), criteria.thresholdsDownlinkKbps.end());
   qcril_data_lce_rpt_criteria c = convertCriteria(criteria);
   switch (qcril_data_lqe_set_rpt_criteria(&c)) {
      case LQE_FAILURE:
      case LQE_NOT_SUPPORTED:
      result = LinkCapCriteriaResult_t::internal_err;
      break;

      case LQE_INVALID_ARGUMENTS:
      result = LinkCapCriteriaResult_t::invalid_arguments;
      break;

      case LQE_SUCCESS:
      default:
      break;
   }

   return result;
}
