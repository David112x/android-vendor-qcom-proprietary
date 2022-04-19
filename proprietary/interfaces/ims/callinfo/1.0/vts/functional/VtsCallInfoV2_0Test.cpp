/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

#define LOG_TAG "CallInfo_V1_0_VTS"
#include <VtsHalHidlTargetTestBase.h>
#include <VtsHalHidlTargetCallbackBase.h>
#include <log/log.h>
#include <android-base/logging.h>
#include <vendor/qti/ims/callinfo/1.0/IIncomingCallListener.h>
#include <vendor/qti/ims/callinfo/1.0/IService.h>
#include <chrono>

using ::android::sp;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_string;
using ::vendor::qti::ims::callinfo::V1_0::IIncomingCallListener;
using ::vendor::qti::ims::callinfo::V1_0::IService;
//using ::vendor::qti::ims::callinfo::V1_0::IService::StatusCode;

constexpr char cbOnSIPInviteAvailable[] = "onSIPInviteAvailable";

class mockIncomingCallArgs
{
public:
  string iccId;
  hidl_vec<string> sipHeader;
};

class mockIncomingCallListener : public IIncomingCallListener,
                                 public ::testing::VtsHalHidlTargetCallbackBase<mockIncomingCallArgs>
{
public:
  mockIncomingCallListener() {};
  virtual ~mockIncomingCallListener() {};

  Return<void> onSipInviteAvailable(const hidl_string& iccId, const hidl_vec<hidl_string>& sipHeader)
  {
    LOG(INFO) << LOG_TAG << "onSIPInviteAvailable";
    mockIncomingCallArgs args;
    args.iccId = iccId;
    args.sipHeader.resize(sipHeader.size());
    int count = 0;
    for(hidl_string str : sipHeader)
    {
      args.sipHeader[count] = str;
      count++;
    }
    NotifyFromCallback(cbOnSIPInviteAvailable, args);
    return Void();
  };
};

class IncomingCallTestBase : public ::testing::VtsHalHidlTargetTestBase
{
public:
  virtual void SetUp() override
  {
    pService = IService::getService();
    mlistener = new mockIncomingCallListener();
  };

  sp<IService> pService;
  mockIncomingCallListener* mlistener;
};

TEST_F(IncomingCallTestBase, registerServiceTest) {
  IService::StatusCode status = IService::StatusCode::FAIL;
  status = pService->registerForIncomingCallData(mlistener);
  EXPECT_EQ(IService::StatusCode::OK, status);
}

TEST_F(IncomingCallTestBase, IncomingCallTest) {
  IService::StatusCode status = IService::StatusCode::FAIL;
  status = pService->registerForIncomingCallData(mlistener);
  EXPECT_EQ(IService::StatusCode::OK, status);

  /**
   * Note: there should be a Incoming Call when testing this usecase.
   * the test case will wait for 10 secs to receive an incoming call
   */
  auto res = mlistener->WaitForCallback(cbOnSIPInviteAvailable, std::chrono::milliseconds(10000));
  EXPECT_TRUE(res.no_timeout);
  EXPECT_NE("", res.args->iccId);
  EXPECT_NE(0, res.args->sipHeader.size());
}


TEST_F(IncomingCallTestBase, UnregisterServiceTest) {
  IService::StatusCode status = IService::StatusCode::FAIL;
  status = pService->registerForIncomingCallData(mlistener);
  EXPECT_EQ(IService::StatusCode::OK, status);
  delete mlistener;
}