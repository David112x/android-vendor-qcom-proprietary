/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include "WDSCallModemEndPoint.h"
#include "WDSCallModemEndPointModule.h"
#include "event/CallManagerEventMessage.h"

using std::to_string;

void WDSCallModemEndPoint::requestSetup(string clientToken,
                                      GenericCallback<string>* callback)
{
  auto shared_setupMsg = std::make_shared<QmiSetupRequest>(clientToken, 0, nullptr, callback);
  mModule->dispatch(shared_setupMsg);
}

void WDSCallModemEndPoint::setBindMuxDataPortParams(int eptype, int epid, int muxid)
{
  Log::getInstance().d("["+mModuleName+"]::setBindMuxDataPortParams "+
                       ", eptype = "+std::to_string((int)eptype)+
                       ", epid = "+std::to_string((int)epid)+
                       ", muxid = "+std::to_string((int)muxid));
  mEpType = eptype;
  mEpId = epid;
  mMuxId = muxid;
}

void WDSCallModemEndPoint::getBindMuxDataPortParams(int *eptype, int *epid, int *muxid)
{
  Log::getInstance().d("["+mModuleName+"]::getBindMuxDataPortParams "+
                       ", eptype = "+std::to_string((int)mEpType)+
                       ", epid = "+std::to_string((int)mEpId)+
                       ", muxid = "+std::to_string((int)mMuxId));
  *eptype = mEpType;
  *epid = mEpId;
  *muxid = mMuxId;
}

void WDSCallModemEndPoint::releaseWDSCallEPModuleQMIClient()
{
  Log::getInstance().d("["+mModuleName+"]::releaseWDSCallEPModuleQMIClient");

  WDSCallModemEndPointModule *wdsCallModule = static_cast<WDSCallModemEndPointModule*>(mModule);

  if( wdsCallModule != nullptr )
  {
    Log::getInstance().d("["+mModuleName+"]::releaseWDSCallEPModuleQMIClient . Call to WDSModule");
    wdsCallModule->releaseWDSCallEPModuleQMIClient();
  }
  else
  {
    Log::getInstance().d("["+mModuleName+"]::releaseWDSCallEPModuleQMIClient wdsCallModule is NULL");
  }
}

void WDSCallModemEndPoint::setLinkStateChangeReport(bool report)
{
  Log::getInstance().d("["+mModuleName+"]::setLinkStateChangeReport "+std::to_string((int)report));

  mReportLinkStateChange = report;
  updateLinkStateChangeReport();
}

void WDSCallModemEndPoint::updateLinkStateChangeReport()
{
  Log::getInstance().d("["+mModuleName+"]::updateLinkStateChangeReport");

  qmi_client_error_type res = QMI_NO_ERR;

  if (getState() == ModemEndPoint::State::OPERATIONAL)
  {
    wds_set_event_report_req_msg_v01 evt_rpt_reg_req;
    wds_set_event_report_resp_msg_v01 evt_rpt_reg_resp;
    memset(&evt_rpt_reg_req, 0, sizeof(evt_rpt_reg_req));
    memset(&evt_rpt_reg_resp, 0, sizeof(evt_rpt_reg_resp));

    evt_rpt_reg_req.report_dormancy_status_valid = true;
    evt_rpt_reg_req.report_dormancy_status = mReportLinkStateChange;

    res = sendRawSync(QMI_WDS_SET_EVENT_REPORT_REQ_V01,
          (void*)&evt_rpt_reg_req, sizeof(evt_rpt_reg_req),
          (void*)&evt_rpt_reg_resp, sizeof(evt_rpt_reg_resp),
          DEFAULT_SYNC_TIMEOUT);
    if (res != QMI_NO_ERR) {
      Log::getInstance().d("["+mModuleName+"]: wds dormancy status change report register failed "
                            +std::to_string((int)res));
    }

    // query current data call status since report is enabled from disabled which is checked in callmanager
    if(mReportLinkStateChange) {
      wds_get_dormancy_status_req_msg_v01 dormancy_status_req;
      wds_get_dormancy_status_resp_msg_v01 dormancy_status_resp;
      memset(&dormancy_status_req, 0, sizeof(dormancy_status_req));
      memset(&dormancy_status_resp, 0, sizeof(dormancy_status_resp));

      res = sendRawSync(QMI_WDS_GET_DORMANCY_STATUS_REQ_V01,
            (void*)&dormancy_status_req, sizeof(dormancy_status_req),
            (void*)&dormancy_status_resp, sizeof(dormancy_status_resp),
            DEFAULT_SYNC_TIMEOUT);
      if (res != QMI_NO_ERR) {
        Log::getInstance().d("["+mModuleName+"]: wds get dormancy status "
                              +std::to_string((int)dormancy_status_resp.dormancy_status));
      }
      rildata::CallManagerEvent_t event;
      if(dormancy_status_resp.dormancy_status == WDS_DORMANCY_STATUS_ACTIVE_V01) {
        event.type = rildata::CallManagerEventType_t::_eCallManagerEventLinkActive;
      }
      else {
        event.type = rildata::CallManagerEventType_t::_eCallManagerEventLinkDormant;
      }
      event.cid = getCid();
      auto msg = std::make_shared<rildata::CallManagerEventMessage>(event);
      msg->broadcast();
    }
  } else {
    Log::getInstance().d("["+mModuleName+"] wds is not operational");
  }
}

bool WDSCallModemEndPoint::goDormant() {
  int rc;
  bool ret = true;
  wds_go_dormant_req_msg_v01 req;
  wds_go_dormant_resp_msg_v01 resp;

  memset(&req, 0, sizeof(req));
  memset(&resp, 0, sizeof(resp));

  rc = sendRawSync(QMI_WDS_GO_DORMANT_REQ_V01,
                   (void *)&req, sizeof(req),
                   (void*)&resp, sizeof(resp),
                   DEFAULT_SYNC_TIMEOUT);

  if (rc != QMI_NO_ERR || resp.resp.result != QMI_RESULT_SUCCESS_V01)
  {
    Log::getInstance().d("["+mModuleName+"]: failed with rc=" + std::to_string(rc) + ", qmi_err=" + std::to_string(resp.resp.error));
    ret = false;
  }

  return ret;
}
