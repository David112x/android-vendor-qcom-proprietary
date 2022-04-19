/*
 * Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

#define LOG_TAG "RcsConfig_V1_0_VTS"
#include <VtsHalHidlTargetTestBase.h>
#include <VtsHalHidlTargetCallbackBase.h>
#include <log/log.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <zlib.h>
#include <vendor/qti/data/factory/2.0/IFactory.h>
#include <vendor/qti/ims/rcsconfig/1.0/IRcsConfig.h>


#define GZIP_WINDOWSIZE 15
#define GZIP_CFACTOR 9
#define GZIP_BSIZE 8096
#define GZIP_OUTPUTBUFFSIZE 32768

using ::android::sp;
using ::vendor::qti::data::factory::V2_0::IFactory;
//using ::vendor::qti::data::factory::V2_0::IFactory::StatusCode;
using ::vendor::qti::ims::rcsconfig::V1_0::IRcsConfig;
using ::vendor::qti::ims::rcsconfig::V1_0::IRcsConfigListener;
using ::vendor::qti::ims::rcsconfig::V1_0::Container;
using ::vendor::qti::ims::rcsconfig::V1_0::ISetConfigCb;
using ::vendor::qti::ims::rcsconfig::V1_0::RequestStatus;

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;

//ISetConfigCb Listener callback function names
constexpr char kCallbacksetConfigResponse[] = "setConfigResponse";
constexpr char kCallbackonConfigNeeded[] = "onReconfigNeeded";


std::map<std::string, std::string> g_configValues;
constexpr char slot_id_key[] = "slot_id";
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

class mockRcsConfigListener : public IRcsConfigListener,
                              public ::testing::VtsHalHidlTargetCallbackBase<mockSetConfigListenerArgs>
{
public:
  mockRcsConfigListener() {};
  ~mockRcsConfigListener() {};

  Return<void> onReconfigNeeded()
  {
    ALOGI("%s - :onReconfigNeeded: received","mockRcsConfigListener");
    NotifyFromCallback(kCallbackonConfigNeeded);
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

class RcsConfigTestBase : public ::testing::VtsHalHidlTargetTestBase
{
public:
  virtual void SetUp() override
  {
    sp<IFactory> factory = IFactory::getService();
    IFactory::StatusCode status = IFactory::StatusCode::NOT_SUPPORTED;
	mRcsConfigListener = new mockRcsConfigListener();
    factory->createRcsConfig(atoi(g_configValues[slot_id_key].c_str()), mRcsConfigListener,
              [&](IFactory::StatusCode _status, sp<IRcsConfig> config) {
                    status = _status;
                    pService = config;
              /* end Lamda Func*/ } );
    mListener = new mockSetConfigListener();
  };
  sp<IRcsConfig> pService;
  mockSetConfigListener* mListener;
  mockRcsConfigListener* mRcsConfigListener;
};

TEST_F(RcsConfigTestBase, ServiceAvailableTest) {
  IFactory::StatusCode status = IFactory::StatusCode::NOT_SUPPORTED;
  sp<IFactory> factory = IFactory::getService();
  sp<IRcsConfig> service = nullptr;
  factory->createRcsConfig(atoi(g_configValues[slot_id_key].c_str()), mRcsConfigListener,
              [&](IFactory::StatusCode _status, sp<IRcsConfig> config) {
                    status = _status;
                    service = config;
              /* end Lamda Func*/ } );
  ASSERT_NE(nullptr, service.get()) << "Could not get Service";
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


int main(int argc, char** argv)
{
  //loading properties
  loadProperties();
  ::testing::InitGoogleTest(&argc, argv);
  int status = RUN_ALL_TESTS();
  ALOGE("Test result with status=%d", status);
  return status;
}
