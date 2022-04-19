/*
 * Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

#define LOG_TAG "ImsCmService_V2_0_VTS"
#include <VtsHalHidlTargetTestBase.h>
#include <VtsHalHidlTargetCallbackBase.h>
#include <log/log.h>
#include <com/qualcomm/qti/imscmservice/2.0/IImsCmService.h>

using ::android::sp;
using ::com::qualcomm::qti::imscmservice::V2_0::IImsCMConnection;
using ::com::qualcomm::qti::imscmservice::V2_0::IImsCMConnectionListener;
using ::com::qualcomm::qti::imscmservice::V2_0::IImsCmService;
using ::com::qualcomm::qti::imscmservice::V2_0::IImsCmServiceListener;
using ::com::qualcomm::qti::imscmservice::V2_0::ConfigType;
using ::com::qualcomm::qti::imscmservice::V2_0::ServiceStatus;
using ::com::qualcomm::qti::imscmservice::V2_0::StatusCode;
using ::com::qualcomm::qti::imscmservice::V2_0::configData;
using ::com::qualcomm::qti::imscmservice::V2_0::userConfig;
using ::com::qualcomm::qti::imscmservice::V2_0::userConfigKeys;
using ::com::qualcomm::qti::imscmservice::V2_0::deviceConfig;
using ::com::qualcomm::qti::imscmservice::V2_0::deviceConfigKeys;
using ::com::qualcomm::qti::imscmservice::V2_0::IMSCM_SERVICE_HANDLE;
using ::com::qualcomm::qti::imscmservice::V2_0::ipTypeEnum;
using ::com::qualcomm::qti::imscmservice::V2_0::connectionEventData;
using ::com::qualcomm::qti::imscmservice::V2_0::incomingMessage;
using ::com::qualcomm::qti::imscmservice::V2_0::incomingMessageKeys;
using ::com::qualcomm::qti::imscmservice::V2_0::outgoingMessage;
using ::com::qualcomm::qti::imscmservice::V2_0::outgoingMessageKeys;
using ::com::qualcomm::qti::imscmservice::V2_0::messageType;
using ::com::qualcomm::qti::imscmservice::V2_0::sipProtocolType;
using ::com::qualcomm::qti::imscmservice::V2_0::connectionEvent;
/*using ::com::qualcomm::qti::imscmservice::V2_0:: */

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;

//CmService Listener callback function names
constexpr char kCallbackonServiceReady[] = "onServiceReady";
constexpr char kCallbackonStatusChange[] = "onStatusChange";
constexpr char kCallbackonConfigurationChange[] = "onConfigurationChange";
constexpr char kCallbackonCommandStatus[] = "onCommandStatus";

//CmConnection Listener Callback function names
constexpr char kCallbackonEventReceived[] = "onEventReceived";
constexpr char kCallbackhandleIncomingMessage[] = "handleIncomingMessage";
constexpr char kCallbackonConnCommandStatus[] = "onCommandStatus";


//CM Service Object listener definitions
class mockImsCmServiceListenerArgs
{
public:
  IMSCM_SERVICE_HANDLE handle;
  uint32_t userdata;
  ServiceStatus status;
  configData config;
  StatusCode statusCode;
};
class mockImsCmServiceListener : public IImsCmServiceListener,
                                 public ::testing::VtsHalHidlTargetCallbackBase<mockImsCmServiceListenerArgs>
{
public:
  mockImsCmServiceListener() {};
  virtual ~mockImsCmServiceListener() {};


  Return<void> onServiceReady(uint64_t connectionManager, uint32_t userdata, uint32_t eStatus)
  {
    ALOGI("%s - :onServiceReady: received","ImsCmServiceListener_ut");
    mockImsCmServiceListenerArgs args;
    args.handle = connectionManager;
    args.userdata = userdata;
    args.status = (ServiceStatus)eStatus;
    NotifyFromCallback(kCallbackonServiceReady, args);
    return Void();
  }
  Return<void> onStatusChange(uint32_t eStatus)
  {
    mockImsCmServiceListenerArgs args;
    args.status = (ServiceStatus)eStatus;
    NotifyFromCallback(kCallbackonStatusChange, args);
    return Void();
  }
  Return<void> onConfigurationChange(const configData& config, uint32_t userdata)
  {
    mockImsCmServiceListenerArgs args;
    args.config = config;
    args.userdata = userdata;
    NotifyFromCallback(kCallbackonConfigurationChange, args);
    return Void();
  }
  Return<void> onCommandStatus(uint32_t userdata, uint32_t status)
  {
    mockImsCmServiceListenerArgs args;
    args.userdata = userdata;
    args.statusCode = (StatusCode)status;
    NotifyFromCallback(kCallbackonCommandStatus, args);
    return Void();
  }
};
// End of CM Service Listener Definitions

//Connection Listener object definitions
class mockImsCmConnectionListenerArgs
{
public:
  connectionEventData event;
  incomingMessage message;
  StatusCode statusCode;
  uint32_t userdata;
};

class mockConnectionListener: public IImsCMConnectionListener,
                              public ::testing::VtsHalHidlTargetCallbackBase<mockImsCmConnectionListenerArgs>
{
public:
  mockConnectionListener() { };
  virtual ~mockConnectionListener() { };

  Return<void> onEventReceived(const connectionEventData& event)
  {
    mockImsCmConnectionListenerArgs args;
    args.event = event;
    NotifyFromCallback(kCallbackonEventReceived, args);
    return Void();
  };
  Return<void> handleIncomingMessage(const incomingMessage& message)
  {
    mockImsCmConnectionListenerArgs args;
    args.message = message;
    NotifyFromCallback(kCallbackhandleIncomingMessage, args);
    return Void();
  };
  Return<void> onCommandStatus(uint32_t status, uint32_t userdata)
  {
    mockImsCmConnectionListenerArgs args;
    args.statusCode = (StatusCode)status;
    args.userdata = userdata;
    NotifyFromCallback(kCallbackonConnCommandStatus, args);
    return Void();
  };
};

class IImsCmServiceTestBase : public ::testing::VtsHalHidlTargetTestBase
{
public:
  virtual void SetUp() override
  {
    pService = IImsCmService::getService("qti.ims.connectionmanagerservice");
    mListener = new mockImsCmServiceListener();
    //UPDATE this data as per the ICCID onthe device.
    iccId = "89010010000002792662";
    featureTag = "+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.session\"";
  };
  sp<IImsCmService> pService;
  mockImsCmServiceListener* mListener;
  string iccId;
  string featureTag;
};

TEST_F(IImsCmServiceTestBase, ServiceAvailableTest) {
  ASSERT_NE(nullptr, pService.get()) << "Could not get Service";
}

TEST_F(IImsCmServiceTestBase, InitializeServiceUnsupportedIccIdTest) {
  string iccId3 = "12345678";
  uint32_t userData = 100;
  uint32_t status = (uint32_t)StatusCode::FAILURE;
  uint64_t listenerId = 0L;
  pService->InitializeService(iccId3, mListener, userData,
            /* lamda func */ [&](uint32_t _status, uint64_t _listenerId) {
                                     status = _status;
                                     listenerId = (uint64_t)_listenerId;
            /* end Lamda Func*/ } );
  EXPECT_NE(StatusCode::SUCCESS, (StatusCode)status);
  //EXPECT_EQ(0L, listenerId);
}

TEST_F(IImsCmServiceTestBase, InitializeServiceTest) {
  uint32_t userData = 100;
  uint32_t status = (uint32_t)StatusCode::FAILURE;
  uint64_t listenerId = 0L;
  //Initialize CmService
  pService->InitializeService(iccId, mListener, userData,
            /* lamda func */ [&](uint32_t _status, uint64_t _listenerId) {
                                     status = _status;
                                     listenerId = (uint64_t)_listenerId;
            /* end Lamda Func*/ } );
  EXPECT_EQ(StatusCode::SUCCESS, (StatusCode)status);
  EXPECT_NE(0L, listenerId);
  // wait until OnServiceReady is received
  auto res = mListener->WaitForCallback(kCallbackonServiceReady);
  EXPECT_TRUE(res.no_timeout);
  //service handle should not be zero
  EXPECT_NE(0, res.args->handle);
  EXPECT_EQ(userData, res.args->userdata);

  // close the Service after the test
  status = pService->closeService(res.args->handle);
  EXPECT_EQ(StatusCode::SUCCESS, (StatusCode)status);
}

TEST_F(IImsCmServiceTestBase, CreateConnectionTest) {
  uint32_t userData = 100;
  uint32_t status = (uint32_t)StatusCode::FAILURE;
  uint64_t listenerId = 0L;
  sp<IImsCMConnection> connection = nullptr;
  uint64_t connHandle = 0L;
  uint64_t connectionListToken = 0L;
  pService->InitializeService(iccId, mListener, userData,
            /* lamda func */ [&](uint32_t _status, uint64_t _listenerId) {
                                     status = _status;
                                     listenerId = (uint64_t)_listenerId;
            /* end Lamda Func*/ } );
  EXPECT_EQ(StatusCode::SUCCESS, (StatusCode)status);
  EXPECT_NE(0L, listenerId);
  auto res = mListener->WaitForCallback(kCallbackonServiceReady);
  EXPECT_TRUE(res.no_timeout);
  EXPECT_NE(0, res.args->handle);
  EXPECT_EQ(userData, res.args->userdata);

  // Create connection
  mockConnectionListener* pConnListener = new mockConnectionListener();
  pService->createConnection(res.args->handle, pConnListener,
                             featureTag,
      /* lamda func */ [&](const sp<com::qualcomm::qti::imscmservice::V2_0::IImsCMConnection> &_connection,
                           uint64_t connectionHandle, uint64_t listenerToken)
                           {
                              connection = _connection;
                              connHandle = connectionHandle;
                              connectionListToken = listenerToken;
      /* end Lamda Func*/ });
  EXPECT_NE(nullptr, connection.get());
  // wait for OnEventReceived callback on pConnListener
  auto res2 = pConnListener->WaitForCallback(kCallbackonEventReceived);
  EXPECT_TRUE(res2.no_timeout);
  EXPECT_EQ(connectionEvent::SERVICE_CREATED,
            (connectionEvent)res2.args->event.eEvent);
  EXPECT_EQ(0, featureTag.compare(res2.args->event.featureTag));

  if(nullptr != connection.get())
  {
    //close connection before proceeding to next test case
    status = pService->closeConnection(res.args->handle, connHandle);
    EXPECT_EQ(StatusCode::SUCCESS, (StatusCode)status);
  }

  // close service before proceeding to next test case
  status = pService->closeService(res.args->handle);
  EXPECT_EQ(StatusCode::SUCCESS, (StatusCode)status);
}

TEST_F(IImsCmServiceTestBase, TriggerRegistrationTest) {
  uint32_t userData = 100;
  uint32_t status = (uint32_t)StatusCode::FAILURE;
  uint64_t listenerId = 0L;
  sp<IImsCMConnection> connection = nullptr;
  uint64_t connHandle = 0L;
  uint64_t connectionListToken = 0L;
  pService->InitializeService(iccId, mListener, userData,
            /* lamda func */ [&](uint32_t _status, uint64_t _listenerId) {
                                     status = _status;
                                     listenerId = (uint64_t)_listenerId;
            /* end Lamda Func*/ } );
  EXPECT_EQ(StatusCode::SUCCESS, (StatusCode)status);
  auto res = mListener->WaitForCallback(kCallbackonServiceReady);
  EXPECT_TRUE(res.no_timeout);
  EXPECT_NE(0, res.args->handle);
  EXPECT_EQ(userData, res.args->userdata);

  mockConnectionListener* pConnListener = new mockConnectionListener();
  pService->createConnection(res.args->handle, pConnListener,
                             featureTag,
      /* lamda func */ [&](const sp<com::qualcomm::qti::imscmservice::V2_0::IImsCMConnection> &_connection,
                           uint64_t connectionHandle, uint64_t listenerToken)
                           {
                              connection = _connection;
                              connHandle = connectionHandle;
                              connectionListToken = listenerToken;
      /* end Lamda Func*/ });
  EXPECT_NE(nullptr, connection.get());
  auto res2 = pConnListener->WaitForCallback(kCallbackonEventReceived);
  EXPECT_TRUE(res2.no_timeout);
  EXPECT_EQ(connectionEvent::SERVICE_CREATED,
            (connectionEvent)res2.args->event.eEvent);
  EXPECT_EQ(0, featureTag.compare(res2.args->event.featureTag));

  if(nullptr != connection.get())
  {
    //Trigger Registration
    status = pService->triggerRegistration(res.args->handle, userData);
    EXPECT_EQ(StatusCode::SUCCESS, (StatusCode)status);
    //wait for OnEventReceived Callback on pConnnlistener
    auto res3 = pConnListener->WaitForCallback(kCallbackonEventReceived);
    EXPECT_TRUE(res3.no_timeout);
    EXPECT_EQ(connectionEvent::SERVICE_CREATED,
              (connectionEvent)res2.args->event.eEvent);
    EXPECT_EQ(0, featureTag.compare(res2.args->event.featureTag));

    if(res3.args->event.eEvent == (uint32_t)connectionEvent::SERVICE_REGISTERED)
    {
      //Trigger De-Registration
      status = pService->triggerDeRegistration(res.args->handle, userData);
      EXPECT_EQ(StatusCode::SUCCESS, (StatusCode)status);
      //wait for OnEventReceived Callback on pConnnlistener
      auto res4 = pConnListener->WaitForCallback(kCallbackonEventReceived);
      EXPECT_TRUE(res4.no_timeout);
      EXPECT_EQ(connectionEvent::SERVICE_NOTREGISTERED,
                (connectionEvent)res4.args->event.eEvent);
    }
    // Proceed to close connection
    status = pService->closeConnection(res.args->handle, connHandle);
    EXPECT_EQ(StatusCode::SUCCESS, (StatusCode)status);
  }

  status = pService->closeService(res.args->handle);
  EXPECT_EQ(StatusCode::SUCCESS, (StatusCode)status);
}

TEST_F(IImsCmServiceTestBase, SendReceiveMessageTest) {
  uint32_t userData = 100;
  uint32_t status = (uint32_t)StatusCode::FAILURE;
  uint64_t listenerId = 0L;
  sp<IImsCMConnection> connection = nullptr;
  uint64_t connHandle = 0L;
  uint64_t connectionListToken = 0L;
  pService->InitializeService(iccId, mListener, userData,
            /* lamda func */ [&](uint32_t _status, uint64_t _listenerId) {
                                     status = _status;
                                     listenerId = (uint64_t)_listenerId;
            /* end Lamda Func*/ } );
  EXPECT_EQ(StatusCode::SUCCESS, (StatusCode)status);
  auto res = mListener->WaitForCallback(kCallbackonServiceReady);
  EXPECT_TRUE(res.no_timeout);
  EXPECT_NE(0, res.args->handle);
  EXPECT_EQ(userData, res.args->userdata);

  mockConnectionListener* pConnListener = new mockConnectionListener();
  pService->createConnection(res.args->handle, pConnListener,
                             featureTag,
      /* lamda func */ [&](const sp<com::qualcomm::qti::imscmservice::V2_0::IImsCMConnection> &_connection,
                           uint64_t connectionHandle, uint64_t listenerToken)
                           {
                              connection = _connection;
                              connHandle = connectionHandle;
                              connectionListToken = listenerToken;
      /* end Lamda Func*/ });
  EXPECT_NE(nullptr, connection.get());
  auto res2 = pConnListener->WaitForCallback(kCallbackonEventReceived);
  EXPECT_TRUE(res2.no_timeout);
  EXPECT_EQ(connectionEvent::SERVICE_CREATED,
            (connectionEvent)res2.args->event.eEvent);
  EXPECT_EQ(0, featureTag.compare(res2.args->event.featureTag));

  if(nullptr != connection.get())
  {
    //Trigger Registration
    status = pService->triggerRegistration(res.args->handle, userData);
    EXPECT_EQ(StatusCode::SUCCESS, (StatusCode)status);
    //wait for OnEventReceived Callback on pConnnlistener
    auto res3 = pConnListener->WaitForCallback(kCallbackonEventReceived);
    EXPECT_TRUE(res3.no_timeout);
    EXPECT_EQ(connectionEvent::SERVICE_CREATED,
              (connectionEvent)res2.args->event.eEvent);
    EXPECT_EQ(0, featureTag.compare(res2.args->event.featureTag));

    if(res3.args->event.eEvent == (uint32_t)connectionEvent::SERVICE_REGISTERED)
    {
      //Send Message
      //Read From URI from the device UE SIM Card
      string fromUri = "sip:111111@w1v6v4.com";
      // configure ICE service with TO URI
      string toUri = "sip:22222@w1v6v4.com";
      //Note: both TO and FROM URI should contain same domain name
      // "w1v6v4.com" is an example domain name

      // Ice or IMS network Server IpAddress,
      //can be V4 or V6 depending on device configuration
      string networkServerIpaddress = "[2002:c023:9c17:401e::3]";

      string messageSIP = "MESSAGE ";
      messageSIP.append(toUri);
      messageSIP.append(" SIP/2.0\r\n\
      From: \"TestUE1\" <");
      messageSIP.append( fromUri);
      messageSIP.append( ">;tag=334455\r\n\
      To: <");
      messageSIP.append(toUri);
      messageSIP.append(">\r\n\
      CSeq: 255229832 MESSAGE\r\n\
      Call-ID: 3486455304_1570329456@");
      messageSIP.append(networkServerIpaddress);
      messageSIP.append("\r\n\
      Via: SIP/2.0/UDP");
      messageSIP.append( networkServerIpaddress);
      messageSIP.append(":5060;\
      branch=z9hG4bK504163233\r\n\
      Max-Forwards: 70\r\n\
      Route: <sip:");
      messageSIP.append(networkServerIpaddress);
      messageSIP.append(":5060;lr>\r\n\
      P-Access-Network-Info: 3GPP-E-UTRAN-FDD; \
      utran-cell-id-3gpp=3114800000011C000\r\n\
      Accept-Contact: *;+g.3gpp.icsi-ref=\"urn:urn-7:\"3gpp-service.ims.icsi.oma.cpm.session\"\r\n\
      Content-Type: text/plain\r\n\
      Expires: 2345\r\n\
      Content-Length: 8\r\n\r\nSM_MESSAGE\r\n\r\n");
      //Send the above constructed Message to ICE/IMS network Server
      outgoingMessage connMessage;
      string callId = "3486455304_1570329456@";
      callId.append(networkServerIpaddress);

      connMessage.data.resize(5);
      {
        connMessage.data[0].key = (uint32_t)outgoingMessageKeys::MessageType;
        connMessage.data[0].value = std::to_string((uint32_t)messageType::TYPE_REQUEST);
      }
      {
        connMessage.data[1].key = (uint32_t)outgoingMessageKeys::OutboundProxy;
        connMessage.data[1].value = networkServerIpaddress;
      }
      {
        connMessage.data[2].key = (uint32_t)outgoingMessageKeys::RemotePort;
        connMessage.data[2].value = std::to_string(5060);
      }
      {
        connMessage.data[3].key = (uint32_t)outgoingMessageKeys::Protocol;
        connMessage.data[3].value = std::to_string((uint32_t)sipProtocolType::UDP);
      }
      {
        connMessage.data[4].key = (uint32_t)outgoingMessageKeys::CallId;
        connMessage.data[4].value = callId;
      }

      {
        //SIP Packet is of Byte Buffer
        char* sipPacket = new char[messageSIP.size()+1];
        memcpy(sipPacket, messageSIP.c_str(), messageSIP.size());
        connMessage.bufferData.resize(1);
        connMessage.bufferData[0].key = (uint32_t)outgoingMessageKeys::Message;
        connMessage.bufferData[0].value.resize(messageSIP.size());
        connMessage.bufferData[0].value.setToExternal((uint8_t*)sipPacket, messageSIP.size(), true);
      }
      connection->sendMessage(connMessage, userData);
      // wait for command stats callback
      auto sendMsgRes = pConnListener->WaitForCallback(kCallbackonConnCommandStatus);
      EXPECT_TRUE(sendMsgRes.no_timeout);
      EXPECT_EQ(userData, sendMsgRes.args->userdata);
      EXPECT_EQ(StatusCode::SUCCESS, sendMsgRes.args->statusCode);

      /* Note: If you have configured ICE server properly
         you should expect Incoming Message from network for
         the above message.
         Enable the below code to test the Incoming Message
      */
      /*
      auto IncomingMsgRes = pConnListener->WaitForCallback(kCallbackhandleIncomingMessage);
      EXPECT_TRUE(IncomingMsgRes.no_timeout);
      IMSCM_CONN_INCOMING_MESSAGE* pIncomingMsg;
      pIncomingMsg = &IncomingMsgRes.args->message;
      EXPECT_NE(0, pIncomingMsg->pMessage.size());
      EXPECT_NE(0, pIncomingMsg->recdAddr.size());

      */

      //Trigger De-Registration
      status = pService->triggerDeRegistration(res.args->handle, userData);
      EXPECT_EQ(StatusCode::SUCCESS, (StatusCode)status);
      //wait for OnEventReceived Callback on pConnnlistener
      auto res4 = pConnListener->WaitForCallback(kCallbackonEventReceived);
      EXPECT_TRUE(res4.no_timeout);
      EXPECT_EQ(connectionEvent::SERVICE_CREATED,
                (connectionEvent)res2.args->event.eEvent);
      EXPECT_EQ(0, featureTag.compare(res2.args->event.featureTag));
    }
    // Proceed to close connection
    status = pService->closeConnection(res.args->handle, connHandle);
    EXPECT_EQ(StatusCode::SUCCESS, (StatusCode)status);
  }

  status = pService->closeService(res.args->handle);
  EXPECT_EQ(StatusCode::SUCCESS, (StatusCode)status);
}

/****************
 * Below Test cases are not supposed to work
 *
 * Todo: figure out a way to test below.
 * Todo: Incoming Message testCase.
 * Todo: Method Response Testcase
 * *******************
TEST_F(IImsCmServiceTestBase, GetUserConfigTest) {
  uint32_t userData = 100;
  uint32_t status = (uint32_t)StatusCode::FAILURE;
  uint64_t listenerId = 0L;
  sp<IImsCMConnection> connection = nullptr;
  uint64_t connHandle = 0L;
  uint64_t connectionListToken = 0L;
  pService->InitializeService(iccId, mListener, userData,
            /* lamda func * / [&](uint32_t _status, uint64_t _listenerId) {
                                     status = _status;
                                     listenerId = (uint64_t)_listenerId;
            /* end Lamda Func* / } );
  EXPECT_EQ(StatusCode::SUCCESS, (StatusCode)status);
  auto res = mListener->WaitForCallback(kCallbackonServiceReady);
  EXPECT_TRUE(res.no_timeout);
  EXPECT_NE(0, res.args->handle);
  EXPECT_EQ(userData, res.args->userdata);

  status = pService->getConfiguration(res.args->handle,
                                      ConfigType::USER_CONFIG,
                                      userData);
  EXPECT_EQ(StatusCode::SUCCESS, (StatusCode)status);
  auto res2 = mListener->WaitForCallback(kCallbackonConfigurationChange);
  EXPECT_TRUE(res2.no_timeout);
  EXPECT_EQ(userData, res2.args->userdata);
  userConfig* pUserconfig = &res2.args->config.userConfigData;
  for(auto element : pUserconfig->data)
  {
    //if(element.key == userConfigkeys::UEClientPort )
    {
      EXPECT_NE(0, element.value.size());
    }
  }

  /* EXPECT_NE(0, pUserconfig->iUEServerPort);
  EXPECT_NE(0, pUserconfig->pAssociatedURI.size());
  EXPECT_NE(0, pUserconfig->pUEPublicIPAddress.size());
  EXPECT_NE(0, pUserconfig->iUEPublicPort);
  EXPECT_NE(0, pUserconfig->pSipPublicUserId.size());
  EXPECT_NE(0, pUserconfig->pSipPrivateUserId.size());
  EXPECT_NE(0, pUserconfig->pSipHomeDomain.size());
  EXPECT_NE(0, pUserconfig->pUEPubGruu.size());
  EXPECT_NE(0, pUserconfig->pLocalHostIPAddress.size());
  EXPECT_EQ(QIMSCM_IPTYPE_ENUM::QIMSCM_IPTYPE_V6, pUserconfig->eIpType);
  EXPECT_NE(0, pUserconfig->pIMEIStr.size());
  EXPECT_NE(0, pUserconfig->iUEOldSAClientPort); * /


  status = pService->closeService(res.args->handle);
  EXPECT_EQ(StatusCode::SUCCESS, (StatusCode)status);
}

TEST_F(IImsCmServiceTestBase, GetDeviceConfigTest) {
  uint32_t userData = 100;
  uint32_t status = (uint32_t)StatusCode::FAILURE;
  uint64_t listenerId = 0L;
  sp<IImsCMConnection> connection = nullptr;
  uint64_t connHandle = 0L;
  uint64_t connectionListToken = 0L;
  pService->InitializeService(iccId, mListener, userData,
            /* lamda func * / [&](uint32_t _status, uint64_t _listenerId) {
                                     status = _status;
                                     listenerId = (uint64_t)_listenerId;
            /* end Lamda Func* / } );
  EXPECT_EQ(StatusCode::SUCCESS, (StatusCode)status);
  auto res = mListener->WaitForCallback(kCallbackonServiceReady);
  EXPECT_TRUE(res.no_timeout);
  EXPECT_NE(0, res.args->handle);
  EXPECT_EQ(userData, res.args->userdata);

  status = pService->getConfiguration(res.args->handle,
                                      ConfigType::DEVICE_CONFIG,
                                      userData);
  EXPECT_EQ(StatusCode::SUCCESS, (StatusCode)status);
  auto res2 = mListener->WaitForCallback(kCallbackonConfigurationChange);
  EXPECT_TRUE(res2.no_timeout);
  EXPECT_EQ(userData, res2.args->userdata);
  deviceConfig* pDeviceconfig = &res2.args->config.deviceConfigData;

  for(auto element : pDeviceconfig->data)
  {
    //if(element.key == userConfigkeys::UEClientPort )
    {
      EXPECT_NE(0, element.value.size());
    }
  }
  /*
  EXPECT_FALSE(pDeviceconfig->bUEBehindNAT);
  EXPECT_FALSE(pDeviceconfig->bIpSecEnabled);
  EXPECT_FALSE(pDeviceconfig->bCompactFormEnabled);
  EXPECT_FALSE(pDeviceconfig->bKeepAliveEnableStatus);
  EXPECT_FALSE(pDeviceconfig->bGruuEnabled);
  EXPECT_NE(0, pDeviceconfig->pStrSipOutBoundProxyName.size());
  EXPECT_NE(0, pDeviceconfig->iSipOutBoundProxyPort);
  EXPECT_NE(0, pDeviceconfig->iPCSCFClientPort);
  EXPECT_NE(0, pDeviceconfig->iPCSCFServerPort);
  EXPECT_NE(0, pDeviceconfig->pArrAuthChallenge.size());
  EXPECT_NE(0, pDeviceconfig->pArrNC.size());
  EXPECT_NE(0, pDeviceconfig->pServiceRoute.size());
  EXPECT_NE(0, pDeviceconfig->pSecurityVerify.size());
  EXPECT_NE(0, pDeviceconfig->iPCSCFOldSAClientPort);
  EXPECT_NE(0, pDeviceconfig->iTCPThresholdValue);
  EXPECT_NE(0, pDeviceconfig->sPANI.size());
  EXPECT_NE(0, pDeviceconfig->sPATH.size());
  * /

  status = pService->closeService(res.args->handle);
  EXPECT_EQ(StatusCode::SUCCESS, (StatusCode)status);
}
**********End of TestCases */

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  int status = RUN_ALL_TESTS();
  ALOGE("Test result with status=%d", status);
  return status;
}
