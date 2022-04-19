/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/UnSolicitedMessage.h>
#include <framework/add_message_id.h>
#include <interfaces/ims/ims.h>

/*
 * Unsol message to notify change in sub config
 *
 */
class QcRilUnsolImsGeoLocationInfo : public UnSolicitedMessage,
                                     public add_message_id<QcRilUnsolImsGeoLocationInfo> {
 private:
  std::optional<double> mLatitude;
  std::optional<double> mLongitude;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilUnsolImsGeoLocationInfo";
  ~QcRilUnsolImsGeoLocationInfo() { }

  QcRilUnsolImsGeoLocationInfo()
      : UnSolicitedMessage(get_class_message_id()) {}

  std::shared_ptr<UnSolicitedMessage> clone() {
    std::shared_ptr<QcRilUnsolImsGeoLocationInfo> msg =
        std::make_shared<QcRilUnsolImsGeoLocationInfo>();
    return msg;
  }

  bool hasLatitude() { return mLatitude ? true : false; }
  double getLatitude() { return *mLatitude; }
  void setLatitude(double val) { mLatitude = val; }
  inline string dumpLatitude() {
    std::ostringstream strs;
    if (mLatitude) {
      strs << *mLatitude;
    } else {
      strs << "<invalid>";
    }
    return " mLatitude = " + strs.str();
  }
  bool hasLongitude() { return mLongitude ? true : false; }
  double getLongitude() { return *mLongitude; }
  void setLongitude(double val) { mLongitude = val; }
  inline string dumpLongitude() {
    std::ostringstream strs;
    if (mLongitude) {
      strs << *mLongitude;
    } else {
      strs << "<invalid>";
    }
    return " mLongitude = " + strs.str();
  }

  virtual string dump() {
    return std::string(QcRilUnsolImsGeoLocationInfo::MESSAGE_NAME) +
      "{" + dumpLatitude() + dumpLongitude() + "}";
  }
};
