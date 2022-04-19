/**
* Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**/

#pragma once

namespace rildata {

 enum IWLANOperationMode {
   DEFAULT = 0,
   LEGACY,
   AP_ASSISTED
 };
 class InitTracker {
 private:
    bool mAuthServiceReady;
    bool mDsdServiceReady;
    bool mWdsServiceReady;
    bool mOttServiceReady;
    IWLANOperationMode mOperMode;
    bool mIWLANEnabled;
    bool mPartialRetryEnabled;
    bool mModemCapability;
    bool mAfterBootup;
 public:
    InitTracker() : mAuthServiceReady(false), mDsdServiceReady(false),
      mWdsServiceReady(false), mOttServiceReady(false), mOperMode(DEFAULT),
      mIWLANEnabled(false), mPartialRetryEnabled(true), mModemCapability(false),
      mAfterBootup(true) {}
    bool allServicesReady() {
      return mAuthServiceReady && mDsdServiceReady && mWdsServiceReady;
    }
    bool isIWLANEnabled () {
      return mIWLANEnabled;
    }
    void setIWLANEnabled(bool enabled) {
      mIWLANEnabled = enabled;
    }
    bool isPartialRetryEnabled () {
      return mPartialRetryEnabled;
    }
    void setPartialRetryEnabled(bool enabled) {
      mPartialRetryEnabled = enabled;
    }
    bool isModeDetermined() {
      return mOperMode != DEFAULT;
    }
    bool isAPAssistMode() {
      return mOperMode == AP_ASSISTED;
    }
    void setIWLANMode(IWLANOperationMode mode) {
      mOperMode = mode;
    }
    void setModemCapability(bool capability) {
      mModemCapability = capability;
    }
    bool getModemCapability() {
      return mModemCapability;
    }
    bool getDsdServiceReady() {
      return mDsdServiceReady;
    }
    bool getAuthServiceReady() {
      return mAuthServiceReady;
    }
    bool getWdsServiceReady() {
      return mWdsServiceReady;
    }
    bool getOttServiceReady() {
      return mOttServiceReady;
    }
    void setDsdServiceReady(bool ready) {
      //If DSD service state is transitioning from ready to not ready, we know that modem SSR
      //has happened
      if (!ready && mDsdServiceReady)
      {
        mAfterBootup = false;
      }
      mDsdServiceReady = ready;
    }
    void setAuthServiceReady(bool ready) {
      mAuthServiceReady = ready;
    }
    void setWdsServiceReady(bool ready) {
      mWdsServiceReady = ready;
    }
    void setOttServiceReady(bool ready) {
      mOttServiceReady = ready;
    }
    bool servicesReadyAfterBootup() {
      return mAfterBootup;
    }
    void dump(std::string padding, std::ostream& os) {
      os << padding << std::boolalpha << "AuthServiceReady=" << mAuthServiceReady << endl;
      os << padding << std::boolalpha << "DsdServiceReady=" << mDsdServiceReady << endl;
      os << padding << std::boolalpha << "WdsServiceReady=" << mWdsServiceReady << endl;
      os << padding << "OperMode=" << mOperMode << endl;
      os << padding << std::boolalpha << "IWLANEnabled=" << mIWLANEnabled << endl;
      os << padding << std::boolalpha << "ModemCapability=" << mModemCapability << endl;
      os << padding << std::boolalpha << "AfterBootup=" << mAfterBootup << endl;
    }
  };

} //namespace
