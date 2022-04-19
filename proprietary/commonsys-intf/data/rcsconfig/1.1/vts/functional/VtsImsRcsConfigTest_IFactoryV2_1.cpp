/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

#define LOG_TAG "RcsConfig_V1_1_VTS"
#include <VtsHalHidlTargetTestBase.h>
#include <VtsHalHidlTargetCallbackBase.h>
#include <log/log.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <zlib.h>
#include <vendor/qti/data/factory/2.1/IFactory.h>
#include <vendor/qti/ims/rcsconfig/1.1/IRcsConfig.h>

#define GZIP_WINDOWSIZE 15
#define GZIP_CFACTOR 9
#define GZIP_BSIZE 8096
#define GZIP_OUTPUTBUFFSIZE 32768

using ::android::sp;
using ::vendor::qti::data::factory::V2_1::IFactory;
//using ::vendor::qti::data::factory::V2_0::IFactory::StatusCode;
using ::vendor::qti::ims::rcsconfig::V1_1::IRcsConfig;
using ::vendor::qti::ims::rcsconfig::V1_0::IRcsConfigListener;
using ::vendor::qti::ims::rcsconfig::V1_0::Container;
using ::vendor::qti::ims::rcsconfig::V1_0::ISetConfigCb;
using ::vendor::qti::ims::rcsconfig::V1_1::IGetSettingsCb;
using ::vendor::qti::ims::rcsconfig::V1_1::ISettingsChangeCb;
using ::vendor::qti::ims::rcsconfig::V1_0::RequestStatus;
using ::vendor::qti::ims::rcsconfig::V1_1::SettingsData;
using ::vendor::qti::ims::rcsconfig::V1_1::SettingsId;
using ::vendor::qti::ims::rcsconfig::V1_1::SettingsValues;
using ::vendor::qti::ims::rcsconfig::V1_1::PresenceConfigKeys;
using ::vendor::qti::ims::rcsconfig::V1_1::StandaloneMessagingConfigKeys;
using ::vendor::qti::ims::rcsconfig::V1_1::UserAgentStringKeys;
using ::vendor::qti::ims::rcsconfig::V1_1::ImsServiceEnableConfigKeys;
using vendor::qti::ims::rcsconfig::V1_1::KeyValuePairTypeInt;
using vendor::qti::ims::rcsconfig::V1_1::KeyValuePairTypeBool;
using vendor::qti::ims::rcsconfig::V1_1::KeyValuePairTypeString;

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;

//ISetConfigCb Listener callback function names
constexpr char kCallbacksetConfigResponse[] = "setConfigResponse";
constexpr char kCallbackonReconfigNeeded[] = "onReconfigNeeded";
constexpr char kCallbackGetSettingsResponse[] = "onGetSettingsResponse";
constexpr char kCallbackonUpdateRcsSettingsResponse[] = "onGetUpdatedSettings";

std::map<std::string, std::string> g_configValues;
const int slot_id = 1;
constexpr char isCompressed_key[] = "isCompressed";
constexpr char xmlFileName_key[] = "xmlFile";
constexpr char PROPERTIES_FILE[] = "/data/properties.txt";


void loadProperties()
{
  std::string line;
  std::ifstream fileStream;
  fileStream.open(PROPERTIES_FILE, std::ifstream::in);
  while (std::getline(fileStream, line))
  {
    std::istringstream is_line(line);
    std::string key;
    if (std::getline(is_line, key, '='))
    {
      std::string value = "";
      if (key[0] == '#')
        continue;

      if (std::getline(is_line, value))
      {
        //std::cout << "key [" << key << "] value[" << value << "]\n";
        g_configValues[key] = value;
      }
    }
  }
  fileStream.close();
}

std::string compressFileData(bool compressData)
{
  std::ifstream file(g_configValues[xmlFileName_key].c_str(),
                    (std::ifstream::binary | std::ifstream::ate));
  // get pointer to associated buffer object
  std::filebuf* pbuf = file.rdbuf();

  // get file size using buffer's members
  std::size_t size = pbuf->pubseekoff (0,file.end,file.in);
  ALOGI("original file size %d", size);
  // rewind buffer pointer to the beginning to read config file
  pbuf->pubseekoff(0, file.beg, file.in);

  char* buffer = new char[size];
  int numCopied = pbuf->sgetn (buffer,size);
  ALOGI("Copied %d characters", numCopied);
  //std::cout.write (buffer,size);
  file.close();
  if(compressData == false)
  {
    ALOGI("not compressing");
    return buffer;
  }

  //logic for string compression

  z_stream c_stream = {0};
  inflateInit2(&c_stream, GZIP_WINDOWSIZE + 16);

  if(deflateInit2(&c_stream, Z_BEST_COMPRESSION, Z_DEFLATED,
                  GZIP_WINDOWSIZE + 16,
                  GZIP_CFACTOR,
                  Z_DEFAULT_STRATEGY) != Z_OK)
  {
    std::cout << "failed while compressing,deflateInit2 failed.";
    return buffer;
  }

  c_stream.next_in = (Bytef*)buffer;
  c_stream.avail_in = size;

  char outputBuffer[GZIP_OUTPUTBUFFSIZE];
  std::string resOutputString;
  int err;

  do
  {
    c_stream.next_out = reinterpret_cast<Bytef*>(outputBuffer);
    c_stream.avail_out = sizeof(GZIP_OUTPUTBUFFSIZE);
    err = deflate(&c_stream, Z_FINISH);
    if (resOutputString.size() < c_stream.total_out)
    {
      resOutputString.append(outputBuffer,c_stream.total_out - resOutputString.size());
    }
  } while (err == Z_OK);

  deflateEnd(&c_stream);

  if (err != Z_STREAM_END)
  {
    std::cout << "error in compression: (" << err << ") " << c_stream.msg;
  }
  //std::cout << "\n\n gzip: -[" << resOutputString << "]\n";
  return resOutputString;
}

//listener args container definition
class mockSetConfigListenerArgs
{
public:
  RequestStatus status;
};

class mockGetSettingsListenerArgs
{
public:
  RequestStatus status;
  SettingsData data;
};


class mockRcsConfigListener : public IRcsConfigListener,
                              public ::testing::VtsHalHidlTargetCallbackBase<mockSetConfigListenerArgs>
{
public:
  mockRcsConfigListener() {};
  ~mockRcsConfigListener() {};

  Return<void> onReconfigNeeded()
  {
    ALOGI("%s - :onReconfigNeeded: received","mockRcsConfigListener");
    NotifyFromCallback(kCallbackonReconfigNeeded);
    return Void();
  }
};

class mockSetConfigListener : public ISetConfigCb,
                              public ::testing::VtsHalHidlTargetCallbackBase<mockSetConfigListenerArgs>
{
public:
  mockSetConfigListener() {};
  virtual ~mockSetConfigListener() {};


  Return<void> setConfigResponse(RequestStatus status)
  {
    ALOGI("%s - :onServiceReady: received","ImsCmServiceListener_ut");
    mockSetConfigListenerArgs args;
    args.status = status;
    NotifyFromCallback(kCallbacksetConfigResponse, args);
    return Void();
  }
};

class mockGetSettingsListener : public IGetSettingsCb,
                              public ::testing::VtsHalHidlTargetCallbackBase<mockGetSettingsListenerArgs>
{
public:
  mockGetSettingsListener() {};
  virtual ~mockGetSettingsListener() {};


  Return<void> onGetSettingsResponse(RequestStatus status,const SettingsData& data) {
    ALOGI("%s - :onServiceReady: received","ImsCmServiceListener_ut");
    mockGetSettingsListenerArgs args;
    args.data = data;
    args.status = status;
    NotifyFromCallback(kCallbackGetSettingsResponse, args);
    return Void();
  }
};

class mockUpdateRcsSettingsListener : public ISettingsChangeCb,
                                      public ::testing::VtsHalHidlTargetCallbackBase<mockGetSettingsListenerArgs>
{
public:
  mockUpdateRcsSettingsListener() {};
  virtual ~mockUpdateRcsSettingsListener() {};

  Return<void> onGetUpdatedSettings(const SettingsData& cbdata)
  {
    ALOGI("%s - :onGetUpdatedSettings: received","ImsCmServiceListener_ut");
    mockGetSettingsListenerArgs args;
    args.data = cbdata;
    args.status=RequestStatus::OK;
    NotifyFromCallback(kCallbackonUpdateRcsSettingsResponse, args);
    return Void();
  }
};

class RcsConfigTestBase : public ::testing::VtsHalHidlTargetTestBase
{
public:
  virtual void SetUp() override
  {
    ALOGI("Setup function");
    factory = IFactory::getService();
    if(factory == nullptr)
    {
        ALOGI("Old factory version supported");
        factory_2_0 = ::vendor::qti::data::factory::V2_0::IFactory::getService();
    }
    IFactory::StatusCode status = IFactory::StatusCode::NOT_SUPPORTED;
  mRcsConfigListener = new mockRcsConfigListener();
    if(factory != nullptr)
    {
      factory->createRcsConfig_1_1(slot_id, mRcsConfigListener,
              [&](IFactory::StatusCode _status, sp<IRcsConfig> config) {
                    status = _status;
                    pService = config;
              /* end Lamda Func*/ } );

    }
    else if(factory_2_0 != nullptr)
    {
      factory_2_0->createRcsConfig(slot_id, mRcsConfigListener,
              [&](IFactory::StatusCode _status, sp<::vendor::qti::ims::rcsconfig::V1_0::IRcsConfig> config) {
                    status = _status;
                    pService_1_0 = config;
              /* end Lamda Func*/ } );
    }
    mListener = new mockSetConfigListener();
    mGetListener = new mockGetSettingsListener();
    mUpdateSettingListener = new mockUpdateRcsSettingsListener();
  };
    sp<IFactory> factory = nullptr;
    sp<::vendor::qti::data::factory::V2_0::IFactory> factory_2_0 = nullptr;
    sp<IRcsConfig> pService = nullptr;
    sp<::vendor::qti::ims::rcsconfig::V1_0::IRcsConfig> pService_1_0 = nullptr;
  mockSetConfigListener* mListener;
    mockGetSettingsListener* mGetListener;
  mockRcsConfigListener* mRcsConfigListener;
  mockUpdateRcsSettingsListener* mUpdateSettingListener;
};

TEST_F(RcsConfigTestBase, ServiceAvailableTest) {
  IFactory::StatusCode status = IFactory::StatusCode::NOT_SUPPORTED;
  sp<IFactory> factory = nullptr;
  sp<::vendor::qti::data::factory::V2_0::IFactory> factory_2_0 = nullptr;
  sp<IRcsConfig> service = nullptr;
  sp<::vendor::qti::ims::rcsconfig::V1_0::IRcsConfig> service_1_0 = nullptr;

  factory = IFactory::getService();
  if(factory == nullptr)
  {
      ALOGI("Old factory version supported");
      factory_2_0 = ::vendor::qti::data::factory::V2_0::IFactory::getService();
  }

  if(factory != nullptr)
  {
    factory->createRcsConfig_1_1(slot_id, mRcsConfigListener,
              [&](IFactory::StatusCode _status, sp<IRcsConfig> config) {
                    status = _status;
                    service = config;
              /* end Lamda Func*/ } );
      if(service)
      ASSERT_NE(nullptr, service.get()) << "Could not get Service";
  }
  else if(factory_2_0 != nullptr)
  {
    factory_2_0->createRcsConfig(slot_id, mRcsConfigListener,
              [&](IFactory::StatusCode _status, sp<::vendor::qti::ims::rcsconfig::V1_0::IRcsConfig> config) {
                    status = _status;
                    service_1_0 = config;
              /* end Lamda Func*/ } );
      ASSERT_NE(nullptr, service_1_0.get()) << "Could not get Service";
  }
  else
  {
    ALOGI("No service object received");
    //ASSERT_NE(nullptr, service.get()) << "Could not get Service";
  }
  ALOGI("Things worked as expected");
}



TEST_F(RcsConfigTestBase, SetConfigTest) {
  Container data;
  string buffer;
  bool compressData = false;
  if(g_configValues[isCompressed_key] == "1")
  {
    cout << "Compressing the xml data";
    compressData = true;
  }
  buffer = compressFileData(compressData);
  data.isCompressed = compressData;
  data.config.resize(buffer.size());
  ALOGI("send_buffer size %d", buffer.size());
  data.config.setToExternal(((uint8_t*)buffer.data()), buffer.size());
  auto result = pService->setConfig(data, mListener);
  // need to check what result is of type.
  //EXPECT_TRUE(result, )
  auto res = mListener->WaitForCallback(kCallbacksetConfigResponse);
  EXPECT_TRUE(res.no_timeout);
  EXPECT_EQ(RequestStatus::OK, res.args->status);
}


TEST_F(RcsConfigTestBase, GetSettingsDataPresenceTest) {

  if(pService != nullptr)
  {
    auto result = pService->getSettingsValue(SettingsId::PRESENCE_CONFIG, mGetListener);
    auto res = mGetListener->WaitForCallback(kCallbackGetSettingsResponse);
    EXPECT_TRUE(res.no_timeout);
    EXPECT_EQ(RequestStatus::OK, res.args->status);
    EXPECT_EQ(SettingsId::PRESENCE_CONFIG, res.args->data.settingsId);
  }
  else
  {
     ALOGE("This functionality is not supported");
  }

}

TEST_F(RcsConfigTestBase, GetSettingsDataStandaloneTest) {

  if(pService != nullptr)
  {
    auto result = pService->getSettingsValue(SettingsId::STANDALONE_MESSAGING_CONFIG, mGetListener);
    auto res = mGetListener->WaitForCallback(kCallbackGetSettingsResponse);
    EXPECT_TRUE(res.no_timeout);
    EXPECT_EQ(RequestStatus::OK, res.args->status);
    EXPECT_EQ(SettingsId::STANDALONE_MESSAGING_CONFIG, res.args->data.settingsId);
  }
  else
  {
     ALOGE("This functionality is not supported");
  }

}

TEST_F(RcsConfigTestBase, GetSettingsDataUserAgentTest) {

  if(pService != nullptr)
  {
    auto result = pService->getSettingsValue(SettingsId::USER_AGENT_STRING, mGetListener);
    auto res = mGetListener->WaitForCallback(kCallbackGetSettingsResponse);
    EXPECT_TRUE(res.no_timeout);
    EXPECT_EQ(RequestStatus::OK, res.args->status);
    EXPECT_EQ(SettingsId::USER_AGENT_STRING, res.args->data.settingsId);
  }
  else
  {
     ALOGE("This functionality is not supported");
  }

}

TEST_F(RcsConfigTestBase, GetSettingsDataImsServiceTest) {

  if(pService != nullptr)
  {
    auto result = pService->getSettingsValue(SettingsId::IMS_SERVICE_ENABLE_CONFIG, mGetListener);
    auto res = mGetListener->WaitForCallback(kCallbackGetSettingsResponse);
    EXPECT_TRUE(res.no_timeout);
    EXPECT_EQ(RequestStatus::OK, res.args->status);
    EXPECT_EQ(SettingsId::IMS_SERVICE_ENABLE_CONFIG, res.args->data.settingsId);
  }
  else
  {
     ALOGE("This functionality is not supported");
  }

}

TEST_F(RcsConfigTestBase, RegisterForImsServiceEnableIndications) {
  ALOGE("in vts register for indications");

  if(pService != nullptr)
  {
    ALOGE("calling RegisterForIndications");
    uint32_t result = pService->registerForSettingsChange(SettingsId::IMS_SERVICE_ENABLE_CONFIG, mUpdateSettingListener);
   // auto res = mUpdateSettingListener->WaitForCallback(kCallbackonUpdateRcsSettingsResponse);
   // EXPECT_TRUE(res.no_timeout);
    EXPECT_EQ(RequestStatus::OK,(RequestStatus)result);
  }
  else
  {
     ALOGE("This functionality is not supported");
  }

}

TEST_F(RcsConfigTestBase, RegisterForPresenceIndications) {
  ALOGE("in vts RegisterForPresenceIndications");

  if(pService != nullptr)
  {
    ALOGE("calling RegisterForPresenceIndications");
    uint32_t result = pService->registerForSettingsChange(SettingsId::PRESENCE_CONFIG, mUpdateSettingListener);
    EXPECT_EQ(RequestStatus::OK,(RequestStatus)result);
   // auto res = mUpdateSettingListener->WaitForCallback(kCallbackonUpdateRcsSettingsResponse);
   // EXPECT_TRUE(res.no_timeout);
  }
  else
  {
     ALOGE("This functionality is not supported");
  }

}

TEST_F(RcsConfigTestBase, RegisterForUserAgentIndications) {
  ALOGE("in vts RegisterForUserAgentIndications");

  if(pService != nullptr)
  {
    ALOGE("calling RegisterForUserAgentIndications");
    uint32_t result = pService->registerForSettingsChange(SettingsId::USER_AGENT_STRING, mUpdateSettingListener);
    EXPECT_EQ(RequestStatus::OK,(RequestStatus)result);
    //auto res = mUpdateSettingListener->WaitForCallback(kCallbackonUpdateRcsSettingsResponse);
    //EXPECT_TRUE(res.no_timeout);
  }
  else
  {
     ALOGE("This functionality is not supported");
  }

}

TEST_F(RcsConfigTestBase, RegisterForRcsSmConfigIndications) {
  ALOGE("in vts RegisterForRcsSmConfigIndications");

  if(pService != nullptr)
  {
    ALOGE("calling RegisterForRcsSmConfigIndications");
    uint32_t result = pService->registerForSettingsChange(SettingsId::STANDALONE_MESSAGING_CONFIG, mUpdateSettingListener);
    EXPECT_EQ(RequestStatus::OK,(RequestStatus)result);
    //auto res = mUpdateSettingListener->WaitForCallback(kCallbackonUpdateRcsSettingsResponse);
    //EXPECT_TRUE(res.no_timeout);
  }
  else
  {
     ALOGE("This functionality is not supported");
  }

}

TEST_F(RcsConfigTestBase, SetSettingsDataPresenceConfigTest) {
  ALOGE("in vts SetSettingsDataPresenceConfigTest rcs settings");
  SettingsData data;
  SettingsValues settingsValues;

  vector<KeyValuePairTypeInt> intDataVec;
  vector<KeyValuePairTypeBool> boolDataVec;
  vector<KeyValuePairTypeString> stringDataVec;

  intDataVec.push_back(KeyValuePairTypeInt());
  intDataVec[0].key = (uint32_t)PresenceConfigKeys::PUBLISH_TIMER_KEY;
  intDataVec[0].value = 100;

  intDataVec.push_back(KeyValuePairTypeInt());
  intDataVec[1].key = (uint32_t)PresenceConfigKeys::AVAILABILITY_CACHE_EXPIRY_KEY;
  intDataVec[1].value = 50;

  intDataVec.push_back(KeyValuePairTypeInt());
  intDataVec[2].key = (uint32_t)PresenceConfigKeys::PUBLISH_EXTENDED_TIMER_KEY;
  intDataVec[2].value = 50;

  intDataVec.push_back(KeyValuePairTypeInt());
  intDataVec[3].key = (uint32_t)PresenceConfigKeys::PUBLISH_SRC_THROTTLE_TIMER_KEY;
  intDataVec[3].value = 50;

  intDataVec.push_back(KeyValuePairTypeInt());
  intDataVec[4].key = (uint32_t)PresenceConfigKeys::PUBLISH_ERROR_RECOVERY_TIMER_KEY;
  intDataVec[4].value = 50;

  intDataVec.push_back(KeyValuePairTypeInt());
  intDataVec[5].key = (uint32_t)PresenceConfigKeys::LIST_SUBSCRIPTION_EXPIRY_KEY;
  intDataVec[5].value = 50;

  intDataVec.push_back(KeyValuePairTypeInt());
  intDataVec[6].key = (uint32_t)PresenceConfigKeys::CAPABILITES_CACHE_EXPIRY_KEY;
  intDataVec[6].value = 50;

  intDataVec.push_back(KeyValuePairTypeInt());
  intDataVec[7].key = (uint32_t)PresenceConfigKeys::CAPABILITY_POLL_INTERVAL_KEY;
  intDataVec[7].value = 50;

  intDataVec.push_back(KeyValuePairTypeInt());
  intDataVec[8].key = (uint32_t)PresenceConfigKeys::MAX_ENTIES_IN_LIST_SUBSCRIBE_KEY;
  intDataVec[8].value = 50;

  boolDataVec.push_back(KeyValuePairTypeBool());
  boolDataVec[0].key = (uint32_t)PresenceConfigKeys::CAPABILITY_DISCOVERY_ENABLED_KEY;
  boolDataVec[0].value = false;

  boolDataVec.push_back(KeyValuePairTypeBool());
  boolDataVec[1].key = (uint32_t)PresenceConfigKeys::GZIP_ENABLED_KEY;
  boolDataVec[1].value = true;

  stringDataVec.push_back(KeyValuePairTypeString());
  stringDataVec[0].key = (uint32_t)PresenceConfigKeys::USER_AGENT_KEY;
  stringDataVec[0].value = "Presence_config";

  settingsValues.intData = intDataVec;
  settingsValues.boolData = boolDataVec;
  settingsValues.stringData = stringDataVec;
  data.settingsValues = settingsValues;
  data.settingsId = SettingsId::PRESENCE_CONFIG;

  if(pService != nullptr)
  {
    ALOGE("calling set rcs settings");
    auto result = pService->setSettingsValue(data, mListener);
    auto res = mListener->WaitForCallback(kCallbacksetConfigResponse);
    EXPECT_TRUE(res.no_timeout);
    EXPECT_EQ(RequestStatus::OK, res.args->status);

  }
  else
  {
     ALOGE("This functionality is not supported");

  }

}

TEST_F(RcsConfigTestBase, SetSettingsDataStandaloneMessagingTest) {
  ALOGE("in vts SetSettingsDataStandaloneMessagingTest rcs settings");
  SettingsData data;
  SettingsValues settingsValues;

  vector<KeyValuePairTypeInt> intDataVec;
  vector<KeyValuePairTypeString> stringDataVec;
  //intDataVec.resize(2);
  settingsValues.stringData.resize(12);

  intDataVec.push_back(KeyValuePairTypeInt());
  intDataVec[0].key = (uint32_t)StandaloneMessagingConfigKeys::DEFAULT_SMS_APP_KEY;
  intDataVec[0].value = 100;

  intDataVec.push_back(KeyValuePairTypeInt());
  intDataVec[1].key = (uint32_t)StandaloneMessagingConfigKeys::DEFAULT_VVM_APP_KEY;
  intDataVec[1].value = 50;

  //stringDataVec.push_back(KeyValuePairTypeString());
  settingsValues.stringData[0].key = (uint32_t)StandaloneMessagingConfigKeys::AUTO_CONFIG_USER_AGENT_KEY;
  settingsValues.stringData[0].value = "AUTO_CONFIG_USER_AGENT_KEY";

 // stringDataVec.push_back(KeyValuePairTypeString());
  settingsValues.stringData[1].key = (uint32_t)StandaloneMessagingConfigKeys::XDM_CLIENT_USER_AGENT_KEY;
  settingsValues.stringData[1].value = "XDM_CLIENT_USER_AGENT_KEY";

 // stringDataVec.push_back(KeyValuePairTypeString());
  settingsValues.stringData[2].key = (uint32_t)StandaloneMessagingConfigKeys::CLIENT_VENDOR_KEY;
  settingsValues.stringData[2].value = "CLIENT_VENDOR";

 // stringDataVec.push_back(KeyValuePairTypeString());
  settingsValues.stringData[3].key = (uint32_t)StandaloneMessagingConfigKeys::CLIENT_VERSION_KEY;
  settingsValues.stringData[3].value = "CLIENT_VERSION";

//stringDataVec.push_back(KeyValuePairTypeString());
  settingsValues.stringData[4].key = (uint32_t)StandaloneMessagingConfigKeys::TERMINAL_VENDOR_KEY;
  settingsValues.stringData[4].value = "TERMINAL_VENDOR";

//stringDataVec.push_back(KeyValuePairTypeString());
  settingsValues.stringData[5].key = (uint32_t)StandaloneMessagingConfigKeys::TERMINAL_MODEL_KEY;
  settingsValues.stringData[5].value = "TERMINAL_MODEL";

//  stringDataVec.push_back(KeyValuePairTypeString());
  settingsValues.stringData[6].key = (uint32_t)StandaloneMessagingConfigKeys::TERMINAL_SW_VERSION_KEY;
  settingsValues.stringData[6].value = "TERMINAL_SW";

//  stringDataVec.push_back(KeyValuePairTypeString());
  settingsValues.stringData[7].key = (uint32_t)StandaloneMessagingConfigKeys::RCS_VERSION_KEY;
  settingsValues.stringData[7].value = "RCS_VERSION";

//  stringDataVec.push_back(KeyValuePairTypeString());
  settingsValues.stringData[8].key = (uint32_t)StandaloneMessagingConfigKeys::PROVISIONING_VERSION_KEY;
  settingsValues.stringData[8].value = "PROVISIONING";

 // stringDataVec.push_back(KeyValuePairTypeString());
  settingsValues.stringData[9].key = (uint32_t)StandaloneMessagingConfigKeys::FRIENDLY_DEVICE_NAME_KEY;
  settingsValues.stringData[9].value = "FRIENDLY_DEV";

//  stringDataVec.push_back(KeyValuePairTypeString());
  settingsValues.stringData[10].key = (uint32_t)StandaloneMessagingConfigKeys::RCS_PROFILE_KEY;
  settingsValues.stringData[10].value = "RCS_PROFILE";

//  stringDataVec.push_back(KeyValuePairTypeString());
  settingsValues.stringData[11].key = (uint32_t)StandaloneMessagingConfigKeys::BOT_VERSION_KEY;
  settingsValues.stringData[11].value = "BOT_VERSION_KEY";

  settingsValues.intData = intDataVec;
  //settingsValues.stringData = stringDataVec;
  data.settingsValues = settingsValues;
  data.settingsId = SettingsId::STANDALONE_MESSAGING_CONFIG;

  if(pService != nullptr)
  {
    ALOGE("calling set rcs settings");
  auto result = pService->setSettingsValue(data, mListener);
    //sleep(5);
  auto res = mListener->WaitForCallback(kCallbacksetConfigResponse);
  EXPECT_TRUE(res.no_timeout);
  EXPECT_EQ(RequestStatus::OK, res.args->status);
  }
  else
  {
     ALOGE("This functionality is not supported");
  }

}


TEST_F(RcsConfigTestBase, SetSettingsDataUserAgentTestEmptyValTest) {
  ALOGE("in vts SetSettingsDataUserAgentTestEmptyValTest rcs settings");
  SettingsData data;
  SettingsValues settingsValues;

  data.settingsValues = settingsValues;
  data.settingsId = SettingsId::USER_AGENT_STRING;

  if(pService != nullptr)
  {
    ALOGE("calling set rcs settings");
    auto result = pService->setSettingsValue(data, mListener);
    auto res = mListener->WaitForCallback(kCallbacksetConfigResponse);
    EXPECT_TRUE(res.no_timeout);
    EXPECT_EQ(RequestStatus::FAIL, res.args->status);
  }
  else
  {
     ALOGE("This functionality is not supported");
  }

}

TEST_F(RcsConfigTestBase, SetSettingsDataUserAgentTest) {
  ALOGE("in vts SetSettingsDataUserAgentTest rcs settings");
  SettingsData data;
  SettingsValues settingsValues;

  vector<KeyValuePairTypeString> stringDataVec;

  stringDataVec.push_back(KeyValuePairTypeString());
  stringDataVec[0].key = (uint32_t)UserAgentStringKeys::IMS_USER_AGENT_KEY;
  stringDataVec[0].value = "user";

  data.settingsValues = settingsValues;
  settingsValues.stringData = stringDataVec;
  data.settingsValues = settingsValues;
  data.settingsId = SettingsId::USER_AGENT_STRING;

  if(pService != nullptr)
  {
    ALOGE("calling set rcs settings");
  auto result = pService->setSettingsValue(data, mListener);
  auto res = mListener->WaitForCallback(kCallbacksetConfigResponse);
  EXPECT_TRUE(res.no_timeout);
  EXPECT_EQ(RequestStatus::OK, res.args->status);
  }
  else
  {
     ALOGE("This functionality is not supported");
  }

}

TEST_F(RcsConfigTestBase, SetSettingsDataImsServiceEnableTest) {
  ALOGE("in vts set rcs settings.. sleeping for sometime");
  //sleep(15);
  SettingsData data;
  SettingsValues settingsValues;

  //vector<KeyValuePairTypeInt> intDataVec;
  vector<KeyValuePairTypeBool> boolDataVec;
  vector<KeyValuePairTypeString> stringDataVec;
  vector<KeyValuePairTypeInt> intDataVec;

  boolDataVec.push_back(KeyValuePairTypeBool());
  boolDataVec[0].key = (uint32_t)ImsServiceEnableConfigKeys::VIDEOTELEPHONY_ENABLED_KEY;
  boolDataVec[0].value = 0;

  boolDataVec.push_back(KeyValuePairTypeBool());
  boolDataVec[1].key = (uint32_t)ImsServiceEnableConfigKeys::WIFI_CALLING_ENABLED_KEY;
  boolDataVec[1].value = 1;

  boolDataVec.push_back(KeyValuePairTypeBool());
  boolDataVec[2].key = (uint32_t)ImsServiceEnableConfigKeys::VOLTE_ENABLED_KEY;
  boolDataVec[2].value = 0;

  boolDataVec.push_back(KeyValuePairTypeBool());
  boolDataVec[3].key = (uint32_t)ImsServiceEnableConfigKeys::MOBILE_DATA_ENABLED_KEY;
  boolDataVec[3].value = 0;

  boolDataVec.push_back(KeyValuePairTypeBool());
  boolDataVec[4].key = (uint32_t)ImsServiceEnableConfigKeys::WIFI_CALLING_IN_ROAMING_ENABLED_KEY;
  boolDataVec[4].value = 0;

  boolDataVec.push_back(KeyValuePairTypeBool());
  boolDataVec[5].key = (uint32_t)ImsServiceEnableConfigKeys::IMS_SERVICE_ENABLED_KEY;
  boolDataVec[5].value = 0;

  boolDataVec.push_back(KeyValuePairTypeBool());
  boolDataVec[6].key = (uint32_t)ImsServiceEnableConfigKeys::UT_ENABLED_KEY;
  boolDataVec[6].value = 0;

  boolDataVec.push_back(KeyValuePairTypeBool());
  boolDataVec[7].key = (uint32_t)ImsServiceEnableConfigKeys::SMS_ENABLED_KEY;
  boolDataVec[7].value = 1;

  boolDataVec.push_back(KeyValuePairTypeBool());
  boolDataVec[8].key = (uint32_t)ImsServiceEnableConfigKeys::DAN_ENABLED_KEY;
  boolDataVec[8].value = 1;

  boolDataVec.push_back(KeyValuePairTypeBool());
  boolDataVec[9].key = (uint32_t)ImsServiceEnableConfigKeys::USSD_ENABLED_KEY;
  boolDataVec[9].value = 0;

  boolDataVec.push_back(KeyValuePairTypeBool());
  boolDataVec[10].key = (uint32_t)ImsServiceEnableConfigKeys::MWI_ENABLED_KEY;
  boolDataVec[10].value = 0;

  boolDataVec.push_back(KeyValuePairTypeBool());
  boolDataVec[11].key = (uint32_t)ImsServiceEnableConfigKeys::PRESENCE_ENABLED_KEY;
  boolDataVec[11].value = 0;

  boolDataVec.push_back(KeyValuePairTypeBool());
  boolDataVec[12].key = (uint32_t)ImsServiceEnableConfigKeys::AUTOCONFIG_ENABLED_KEY;
  boolDataVec[12].value = 0;

  boolDataVec.push_back(KeyValuePairTypeBool());
  boolDataVec[13].key = (uint32_t)ImsServiceEnableConfigKeys::XDM_CLIENT_ENABLED_KEY;
  boolDataVec[13].value = 0;

  boolDataVec.push_back(KeyValuePairTypeBool());
  boolDataVec[14].key = (uint32_t)ImsServiceEnableConfigKeys::RCS_MESSAGING_ENABLED_KEY;
  boolDataVec[14].value = 1;

  boolDataVec.push_back(KeyValuePairTypeBool());
  boolDataVec[15].key = (uint32_t)ImsServiceEnableConfigKeys::CALL_MODE_PREF_ROAM_ENABLED_KEY;
  boolDataVec[15].value = 0;

  boolDataVec.push_back(KeyValuePairTypeBool());
  boolDataVec[16].key = (uint32_t)ImsServiceEnableConfigKeys::RTT_ENABLED_KEY;
  boolDataVec[16].value = 0;

  boolDataVec.push_back(KeyValuePairTypeBool());
  boolDataVec[17].key = (uint32_t)ImsServiceEnableConfigKeys::CARRIER_CONFIG_ENABLED_KEY;
  boolDataVec[17].value = 1;

  intDataVec.push_back(KeyValuePairTypeInt());
  intDataVec[0].key = (uint32_t)ImsServiceEnableConfigKeys::CALL_MODE_PREFERENCE_KEY;
  intDataVec[0].value = 2;

  intDataVec.push_back(KeyValuePairTypeInt());
  intDataVec[1].key = (uint32_t)ImsServiceEnableConfigKeys::SERVICE_MASK_BY_NETWORK_ENABLED_KEY;
  intDataVec[1].value = 1;


  stringDataVec.push_back(KeyValuePairTypeString());
  stringDataVec[0].key = (uint32_t)ImsServiceEnableConfigKeys::WIFI_PROVISIONING_ID_KEY;
  stringDataVec[0].value = "abcd";

  //settingsValues.intData = intDataVec;
  settingsValues.boolData = boolDataVec;
  settingsValues.stringData = stringDataVec;
  settingsValues.intData = intDataVec;
  data.settingsValues = settingsValues;
  data.settingsId = SettingsId::IMS_SERVICE_ENABLE_CONFIG;

  if(pService != nullptr)
  {
    ALOGE("calling set rcs settings");
    auto result = pService->setSettingsValue(data, mListener);
    auto res = mListener->WaitForCallback(kCallbacksetConfigResponse);
    EXPECT_TRUE(res.no_timeout);
    EXPECT_EQ(RequestStatus::OK, res.args->status);
  }
  else
  {
     ALOGE("This functionality is not supported");
  }

}


TEST_F(RcsConfigTestBase, GetSettingsDataPresenceTest1) {

  if(pService != nullptr)
  {
    auto result = pService->getSettingsValue(SettingsId::PRESENCE_CONFIG, mGetListener);
    auto res = mGetListener->WaitForCallback(kCallbackGetSettingsResponse);
    EXPECT_TRUE(res.no_timeout);
    EXPECT_EQ(RequestStatus::OK, res.args->status);
    EXPECT_EQ(SettingsId::PRESENCE_CONFIG, res.args->data.settingsId);
  }
  else
  {
     ALOGE("This functionality is not supported");
  }

}

TEST_F(RcsConfigTestBase, GetSettingsDataStandaloneTest1) {

  if(pService != nullptr)
  {
    auto result = pService->getSettingsValue(SettingsId::STANDALONE_MESSAGING_CONFIG, mGetListener);
    auto res = mGetListener->WaitForCallback(kCallbackGetSettingsResponse);
    EXPECT_TRUE(res.no_timeout);
    EXPECT_EQ(RequestStatus::OK, res.args->status);
    EXPECT_EQ(SettingsId::STANDALONE_MESSAGING_CONFIG, res.args->data.settingsId);
  }
  else
  {
     ALOGE("This functionality is not supported");
  }

}

TEST_F(RcsConfigTestBase, GetSettingsDataUserAgentTest1) {

  if(pService != nullptr)
  {
    auto result = pService->getSettingsValue(SettingsId::USER_AGENT_STRING, mGetListener);
    auto res = mGetListener->WaitForCallback(kCallbackGetSettingsResponse);
    EXPECT_TRUE(res.no_timeout);
    EXPECT_EQ(RequestStatus::OK, res.args->status);
    EXPECT_EQ(SettingsId::USER_AGENT_STRING, res.args->data.settingsId);
  }
  else
  {
     ALOGE("This functionality is not supported");
  }

}

TEST_F(RcsConfigTestBase, GetSettingsDataImsServiceTest1) {

  if(pService != nullptr)
  {
    auto result = pService->getSettingsValue(SettingsId::IMS_SERVICE_ENABLE_CONFIG, mGetListener);
    auto res = mGetListener->WaitForCallback(kCallbackGetSettingsResponse);
  EXPECT_TRUE(res.no_timeout);
    EXPECT_EQ(RequestStatus::OK, res.args->status);
    EXPECT_EQ(SettingsId::IMS_SERVICE_ENABLE_CONFIG, res.args->data.settingsId);
  }
  else
  {
     ALOGE("This functionality is not supported");
  }

}

TEST_F(RcsConfigTestBase, DeRegisterForPresenceIndications) {
  ALOGE("in vts DeRegisterForPresenceIndications");

  if(pService != nullptr)
  {
    ALOGE("calling DeRegisterForPresenceIndications");
    uint32_t result = pService->deregisterForSettingsChange(SettingsId::PRESENCE_CONFIG);
    EXPECT_EQ(RequestStatus::OK,(RequestStatus)result);
  }
  else
  {
     ALOGE("This functionality is not supported");
  }

}

TEST_F(RcsConfigTestBase, SetSettingsDataPresenceConfigTestAfterDeregister) {
  ALOGE("in vts SetSettingsDataPresenceConfigTestAfterDeregister rcs settings");
  SettingsData data;
  SettingsValues settingsValues;

  vector<KeyValuePairTypeInt> intDataVec;
  vector<KeyValuePairTypeBool> boolDataVec;
  vector<KeyValuePairTypeString> stringDataVec;

  intDataVec.push_back(KeyValuePairTypeInt());
  intDataVec[0].key = (uint32_t)PresenceConfigKeys::PUBLISH_TIMER_KEY;
  intDataVec[0].value = 100;

  intDataVec.push_back(KeyValuePairTypeInt());
  intDataVec[1].key = (uint32_t)PresenceConfigKeys::AVAILABILITY_CACHE_EXPIRY_KEY;
  intDataVec[1].value = 50;

  intDataVec.push_back(KeyValuePairTypeInt());
  intDataVec[2].key = (uint32_t)PresenceConfigKeys::PUBLISH_EXTENDED_TIMER_KEY;
  intDataVec[2].value = 50;

  intDataVec.push_back(KeyValuePairTypeInt());
  intDataVec[3].key = (uint32_t)PresenceConfigKeys::PUBLISH_SRC_THROTTLE_TIMER_KEY;
  intDataVec[3].value = 50;

  intDataVec.push_back(KeyValuePairTypeInt());
  intDataVec[4].key = (uint32_t)PresenceConfigKeys::PUBLISH_ERROR_RECOVERY_TIMER_KEY;
  intDataVec[4].value = 50;

  intDataVec.push_back(KeyValuePairTypeInt());
  intDataVec[5].key = (uint32_t)PresenceConfigKeys::LIST_SUBSCRIPTION_EXPIRY_KEY;
  intDataVec[5].value = 50;

  intDataVec.push_back(KeyValuePairTypeInt());
  intDataVec[6].key = (uint32_t)PresenceConfigKeys::CAPABILITES_CACHE_EXPIRY_KEY;
  intDataVec[6].value = 50;

  intDataVec.push_back(KeyValuePairTypeInt());
  intDataVec[7].key = (uint32_t)PresenceConfigKeys::CAPABILITY_POLL_INTERVAL_KEY;
  intDataVec[7].value = 50;

  intDataVec.push_back(KeyValuePairTypeInt());
  intDataVec[8].key = (uint32_t)PresenceConfigKeys::MAX_ENTIES_IN_LIST_SUBSCRIBE_KEY;
  intDataVec[8].value = 50;

  boolDataVec.push_back(KeyValuePairTypeBool());
  boolDataVec[0].key = (uint32_t)PresenceConfigKeys::CAPABILITY_DISCOVERY_ENABLED_KEY;
  boolDataVec[0].value = false;

  boolDataVec.push_back(KeyValuePairTypeBool());
  boolDataVec[1].key = (uint32_t)PresenceConfigKeys::GZIP_ENABLED_KEY;
  boolDataVec[1].value = true;

  stringDataVec.push_back(KeyValuePairTypeString());
  stringDataVec[0].key = (uint32_t)PresenceConfigKeys::USER_AGENT_KEY;
  stringDataVec[0].value = "Presence_config";

  settingsValues.intData = intDataVec;
  settingsValues.boolData = boolDataVec;
  settingsValues.stringData = stringDataVec;
  data.settingsValues = settingsValues;
  data.settingsId = SettingsId::PRESENCE_CONFIG;

  if(pService != nullptr)
  {
    ALOGE("calling set rcs settings");
    auto result = pService->setSettingsValue(data, mListener);
    auto res = mListener->WaitForCallback(kCallbacksetConfigResponse);
    EXPECT_TRUE(res.no_timeout);
    EXPECT_EQ(RequestStatus::OK, res.args->status);

  }
  else
  {
     ALOGE("This functionality is not supported");

  }

}

int main(int argc, char** argv)
{
  //loading properties
  cout << "Please double check you have a carrier config xml file and a properties.txt file";
  cout << " in the same location as this test binary" << endl;
  loadProperties();
  ::testing::InitGoogleTest(&argc, argv);
  int status = RUN_ALL_TESTS();
  ALOGE("Test result with status=%d", status);
  return status;
}
