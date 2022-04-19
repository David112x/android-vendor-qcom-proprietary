
/******************************************************************************

                  N E T M G R _ N E T D _C L I E N T . CPP

******************************************************************************/

/******************************************************************************

  @file    netmgr_netd_client.cpp
  @brief   Network manager netd client

  DESCRIPTION
  Implementation of netd client module

******************************************************************************/
/*===========================================================================

  Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

#if (!defined(NETMGR_OFFTARGET) && defined(FEATURE_DS_LINUX_ANDROID))
extern "C" {
#include "netmgr_kif.h"

#ifdef NETMGR_QOS_ENABLED
#include "netmgr_tc.h"
#endif
}

#ifdef __cplusplus
#include "netmgr_netd_client.h"
#include <android/system/net/netd/1.1/INetd.h>

using ::android::hardware::Return;
using ::android::sp;
using ::android::wp;
using ::android::hardware::hidl_death_recipient;
using ::android::hidl::manager::V1_0::IServiceNotification;
using ::android::hardware::hidl_string;
using ::android::hardware::Void;

using android::system::net::netd::V1_1::INetd;

class NetmgrNetdClient
{
public:
  static NetmgrNetdClient *getInstance();

  NetmgrNetdClient();

  static NetmgrNetdClient *instancePtr;
  struct NetdProxy : public hidl_death_recipient,
                           public IServiceNotification
  {
public:
    NetdProxy(NetmgrNetdClient &);
    void serviceDied(uint64_t cookie, const wp<IBase>& who) override;
    Return<void> onRegistration(const hidl_string &fqName,
                                const hidl_string &name,
                                bool preexisting) override;
    virtual ~NetdProxy();
    NetmgrNetdClient &com;
    sp<INetd> mService;
    bool registerServerNotification();
    bool registerLinkToDeath();
    bool unregisterLinkToDeath();
    std::tuple<uint64_t, uint32_t> registerNetwork();
    bool unregisterNetwork(uint64_t networkHandle);
    bool addRouteToOemNetwork(uint64_t networkHandle,
                              const hidl_string& ifname,
                              const hidl_string& destination,
                              const hidl_string& nexthop);
    bool removeRouteFromOemNetwork(uint64_t networkHandle,
                                   const hidl_string& ifname,
                                   const hidl_string& destination,
                                   const hidl_string& nexthop);
    bool addInterfaceToOemNetwork(uint64_t networkHandle,
                                  const hidl_string& ifname);
    bool removeInterfaceFromOemNetwork(uint64_t networkHandle,
                                       const hidl_string& ifname);
    bool setIpForwardEnable(bool enable);
    bool setForwardingBetweenInterfaces(const hidl_string& inputIfName,
                                        const hidl_string& outputIfName,
                                        bool enable);
    pthread_t getServiceThread;
    bool startGetServiceThread();
    static void *getServiceImpl(void *);
  };

  sp<NetdProxy> mApiService;

  pthread_mutex_t initMutex;
  pthread_mutex_t connectMutex;
  pthread_cond_t  connectCondition;
  pthread_cond_t  initCondition;

  bool isServiceInited;

  void lockInitMutex();
  void unlockInitMutex();
  void lockConnectMutex();
  void unlockConnectMutex();
  void waitForConnectCondition();
  void signalConnectCondition();
  void waitForInitCondition();
  void signalInitCondition();

  uint64_t getNetworkHandle(std::string networkType);
  void setNetworkHandle(std::string networkType, uint64_t networkHandle);
private:
  std::map<std::string, uint64_t> networkHandleMap;
};

#define NETD_SERVICE ""

#else
  typedef struct NetmgrNetdClient NetmgrNetdClient;
#endif /* __cplusplus*/

#ifdef __cplusplus
#define NETMGR_SUCCESS (0)
#define NETMGR_FAILURE (-1)

  /* C++ treats anything between "" as string literals which is interpreted as const char*.
   * The current logging macros do not have support for handling const char* so we created
   * a new function to handle this. If required we can move it into ds_util module */
#define NETMGR_MAX_LOG_MSG_SIZE      512
/*---------------------------------------------------------------------------
   Function for formating log messages
---------------------------------------------------------------------------*/
  LOCAL void
  netmgr_format_msg (char* buf_ptr, const char* fmt,...)
  {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf_ptr, NETMGR_MAX_LOG_MSG_SIZE, fmt, ap);
    va_end(ap);
  }
#if defined(FEATURE_DATA_LOG_QXDM) && defined(FEATURE_DATA_LOG_ADB)

  #define DS_LOG_TAG "QC-NETMGR-LIB"
  #undef  LOG_TAG
  #define LOG_TAG DS_LOG_TAG

  #define NETMGR_LOG_ERR(...)                                         \
  do                                                                  \
  {                                                                   \
    char buf[NETMGR_MAX_LOG_MSG_SIZE];                                \
    netmgr_format_msg(buf, __VA_ARGS__);                              \
    MSG_SPRINTF_1(MSG_SSID_LINUX_DATA, MSG_LEGACY_ERROR, "%s", buf);  \
    LOGE(__VA_ARGS__);                                                \
  }while (0)

  #define NETMGR_LOG_LOW(...)                                         \
  do{                                                                 \
    char buf[NETMGR_MAX_LOG_MSG_SIZE];                                \
    netmgr_format_msg(buf, __VA_ARGS__);                              \
    MSG_SPRINTF_1(MSG_SSID_LINUX_DATA, MSG_LEGACY_LOW, "%s", buf);    \
    LOGI(__VA_ARGS__);                                                \
  }while (0)

  #define NETMGR_LOG_MED(...)                                         \
  do{                                                                 \
    char buf[NETMGR_MAX_LOG_MSG_SIZE];                                \
    netmgr_format_msg(buf, __VA_ARGS__);                              \
    MSG_SPRINTF_1(MSG_SSID_LINUX_DATA, MSG_LEGACY_MED, "%s", buf);    \
    LOGI(__VA_ARGS__);                                                \
  }while (0)

  #define NETMGR_LOG_HIGH(...)                                        \
  do{                                                                 \
    char buf[NETMGR_MAX_LOG_MSG_SIZE];                                \
    netmgr_format_msg(buf, __VA_ARGS__);                              \
    MSG_SPRINTF_1(MSG_SSID_LINUX_DATA, MSG_LEGACY_HIGH, "%s", buf);   \
    LOGD(__VA_ARGS__);                                                \
  }while (0)

#elif defined(FEATURE_DATA_LOG_QXDM)
  #define NETMGR_LOG_ERR(...)                                         \
  {                                                                   \
    char buf[NETMGR_MAX_LOG_MSG_SIZE];                                \
    netmgr_format_msg(buf, __VA_ARGS__);                              \
    MSG_SPRINTF_1(MSG_SSID_LINUX_DATA, MSG_LEGACY_ERROR, "%s", buf);  \
  }

  #define NETMGR_LOG_LOW(...)                                         \
  {                                                                   \
    char buf[NETMGR_MAX_LOG_MSG_SIZE];                                \
    netmgr_format_msg(buf, __VA_ARGS__);                              \
    MSG_SPRINTF_1(MSG_SSID_LINUX_DATA, MSG_LEGACY_LOW, "%s", buf);    \
  }

  #define NETMGR_LOG_MED(...)                                         \
  {                                                                   \
    char buf[NETMGR_MAX_LOG_MSG_SIZE];                                \
    netmgr_format_msg(buf, __VA_ARGS__);                              \
    MSG_SPRINTF_1(MSG_SSID_LINUX_DATA, MSG_LEGACY_MED, "%s", buf);    \
  }

  #define NETMGR_LOG_HIGH(...)                                        \
  {                                                                   \
    char buf[NETMGR_MAX_LOG_MSG_SIZE];                                \
    netmgr_format_msg(buf, __VA_ARGS__);                              \
    MSG_SPRINTF_1(MSG_SSID_LINUX_DATA, MSG_LEGACY_HIGH, "%s", buf);   \
  }
#elif defined(FEATURE_DATA_LOG_ADB)
  #define DS_LOG_TAG "QC-NETMGR-LIB"
  #undef  LOG_TAG
  #define LOG_TAG DS_LOG_TAG

  #define NETMGR_LOG_ERR(...)  LOGE(__VA_ARGS__)

  #define NETMGR_LOG_LOW(...)  LOGI(__VA_ARGS__)

  #define NETMGR_LOG_MED(...)  LOGI(__VA_ARGS__)

  #define NETMGR_LOG_HIGH(...) LOGD(__VA_ARGS__)
#endif

#else
  #define NETMGR_LOG_ERR  netmgr_log_err
  #define NETMGR_LOG_LOW  netmgr_log_low
  #define NETMGR_LOG_MED  netmgr_log_med
  #define NETMGR_LOG_HIGH netmgr_log_high
#endif

#ifdef __cplusplus
NetmgrNetdClient* NetmgrNetdClient::instancePtr = NULL;

#define CHECK_ERROR(halStatus, halStatusCode, ret)                      \
  if (!halStatus)                                                       \
  {                                                                     \
    NETMGR_LOG_ERR("%s(): HAL invocation failed!",                      \
                   __func__);                                           \
    ret = false;                                                        \
  }                                                                     \
  else                                                                  \
  {                                                                     \
    switch (halStatusCode)                                              \
    {                                                                   \
    case INetd::StatusCode::OK:                                         \
      ret = true;                                                       \
      break; /* OK */                                                   \
                                                                        \
    case INetd::StatusCode::INVALID_ARGUMENTS:                          \
      NETMGR_LOG_ERR("%s(): failed \"INVALID_ARGUMENTS\"!", __func__);  \
      ret = false;                                                      \
      break; /* INVALID_ARGUMENTS */                                    \
                                                                        \
    case INetd::StatusCode::ALREADY_EXISTS:                             \
      NETMGR_LOG_ERR("%s(): failed \"ALREADY_EXISTS\"!", __func__);     \
      ret = false;                                                      \
      break; /* ALREADY_EXISTS */                                       \
                                                                        \
    case INetd::StatusCode::NO_NETWORK:                                 \
      NETMGR_LOG_ERR("%s(): failed \"NO_NETWORK\"!", __func__);         \
      ret = false;                                                      \
      break; /* NO_NETWORK */                                           \
                                                                        \
    case INetd::StatusCode::PERMISSION_DENIED:                          \
      NETMGR_LOG_ERR("%s(): failed \"PERMISSION_DENIED\"!", __func__);  \
      ret = false;                                                      \
      break; /* PERMISSION_DENIED */                                    \
                                                                        \
    default:                                                            \
      NETMGR_LOG_ERR("%s(): failed \"UNKNOWN_ERROR\"!", __func__);      \
      ret = false;                                                      \
      break; /* UNKNOWN_ERROR */                                        \
    }                                                                   \
  }

/*===========================================================================
  FUNCTION  NetmgrNetdClient::getInstance
===========================================================================*/
NetmgrNetdClient* NetmgrNetdClient::getInstance(void)
{
  if (instancePtr == NULL)
  {
    instancePtr = new NetmgrNetdClient();

    /* Netmgr will be able to connect to netd only after onRegistration
       callback is received. Till such time any communication with netd
       will fail. To ensure netmgr connects to netd only after
       onRegistration callback is processed we will block using a
       condition variable. Once onRegistration is completed we will
       broadcast the initCondition variable which will unblock the init */
    instancePtr->lockInitMutex();
    while (instancePtr->isServiceInited == false)
    {
      instancePtr->waitForInitCondition();
    }
    instancePtr->unlockInitMutex();
  }

  return instancePtr;
}

/*===========================================================================
  FUNCTION  NetmgrNetdClient::getNetworkHandle
===========================================================================*/
uint64_t NetmgrNetdClient::getNetworkHandle(std::string networkType)
{
  std::map<std::string, uint64_t>::iterator it;

  it = networkHandleMap.find(networkType);

  if (it == networkHandleMap.end())
  {
    return 0;
  }

  return networkHandleMap.find(networkType)->second;
}

/*===========================================================================
  FUNCTION  NetmgrNetdClient::setNetworkHandle
===========================================================================*/
void NetmgrNetdClient::setNetworkHandle
(
  std::string networkType,
  uint64_t networkHandle
)
{
  std::map<std::string, uint64_t>::iterator it;

  it = networkHandleMap.find(networkType);

  if (it != networkHandleMap.end())
  {
    networkHandleMap.erase(it);
  }

  networkHandleMap[networkType] = networkHandle;
}

/*===========================================================================
  FUNCTION  NetdProxy::NetdProxy
===========================================================================*/
NetmgrNetdClient::NetdProxy::NetdProxy
(
  NetmgrNetdClient &_com
):com(_com)
{
  memset(&getServiceThread,0,sizeof(pthread_t));
  registerServerNotification();
}

/*===========================================================================
  FUNCTION  NetdProxy::~NetdProxy
===========================================================================*/
NetmgrNetdClient::NetdProxy::~NetdProxy()
{
    unregisterLinkToDeath();
}

/*===========================================================================
  FUNCTION  NetdProxy::registerServerNotification
===========================================================================*/
bool NetmgrNetdClient::NetdProxy::registerServerNotification()
{
  sp<NetdProxy> self = this;

    Return<bool> retVal = INetd::registerForNotifications(NETD_SERVICE, self);
    if (!retVal.isOk())
    {
      NETMGR_LOG_ERR("%s(): Failed to register for Netd HAL service", __func__);
      return false;
    }
    else
    {
      NETMGR_LOG_HIGH("%s(): Successfully registered for Netd HAL service",
                      __func__);
      return true;
    }
}

/*===========================================================================
  FUNCTION  NetdProxy::registerLinkToDeath
===========================================================================*/
bool NetmgrNetdClient::NetdProxy::registerLinkToDeath()
{
  sp<NetdProxy> self = this;

  Return<bool> retVal = mService->linkToDeath(self, 0);

  if (!retVal.isOk())
  {
    NETMGR_LOG_ERR("%s(): Failure registerLinkToDeath!", __func__);
    return false;
  }
  else
  {
    NETMGR_LOG_LOW("%s(): Success registerLinkToDeath!", __func__);
    return true;
  }
}

/*===========================================================================
  FUNCTION  NetdProxy::unregisterLinkToDeath
===========================================================================*/
bool NetmgrNetdClient::NetdProxy::unregisterLinkToDeath()
{
    sp<NetdProxy> self = this;
    Return<bool> retVal = mService->unlinkToDeath(self);
    if (!retVal.isOk())
    {
        NETMGR_LOG_ERR("%s(): Failure unregisterLinkToDeath!", __func__);
        return false;
    }
    else
    {
        NETMGR_LOG_LOW("%s(): Success unregisterLinkToDeath!", __func__);
        return true;
    }
}

/*===========================================================================
  FUNCTION  NetdProxy::registerNetwork
===========================================================================*/
std::tuple<uint64_t, uint32_t> NetmgrNetdClient::NetdProxy::registerNetwork()
{
    bool rc = false;
    uint32_t _packetMark = 0;
    uint32_t net_id = 0;
    uint64_t _networkHandle = 0;
    INetd::StatusCode status;
    Return<void> retVal;

    this->com.lockConnectMutex();

    if (mService == NULL)
    {
      NETMGR_LOG_ERR("%s(): mSerivce is NULL", __func__);
      this->com.unlockConnectMutex();
      return std::make_tuple(0, 0);
    }

    NETMGR_LOG_LOW("%s(): Attempting createOemNetwork", __func__);
    retVal = mService->createOemNetwork([&](uint64_t networkHandle,
                                            uint32_t packetMark,
                                            const auto& errMsg)
    {
      _packetMark = packetMark;
      _networkHandle = networkHandle;
      status = errMsg;
    });

    CHECK_ERROR(retVal.isOk(), status, rc);

    this->com.unlockConnectMutex();

    if (rc)
    {
      NETMGR_LOG_LOW("%s(): command completed!", __func__);
      net_id = _packetMark & 0xFFFF;
      NETMGR_LOG_MED("%s(): createOemNetwork succeeded [packet mark : 0x%x]"
                     " [net id : %u] [network handle : 0x%" PRIx64 "]",
                     __func__, _packetMark, net_id, _networkHandle);
      return std::make_tuple(_networkHandle, _packetMark);
    }
    else
    {
      NETMGR_LOG_ERR("%s(): Failure in createOemNetwork!", __func__);
      return std::make_tuple(0, 0);
    }

}

/*===========================================================================
  FUNCTION  NetdProxy::unregisterNetwork
===========================================================================*/
bool NetmgrNetdClient::NetdProxy::unregisterNetwork(uint64_t networkHandle)
{
    bool rc = false;

    this->com.lockConnectMutex();

    if (mService == NULL)
    {
      NETMGR_LOG_ERR("%s(): mService is NULL", __func__);
      this->com.unlockConnectMutex();
      return rc;
    }

    NETMGR_LOG_LOW("%s(): Attempting destroyOemNetwork", __func__);
    auto retVal = mService->destroyOemNetwork(networkHandle);

    this->com.unlockConnectMutex();

    CHECK_ERROR(retVal.isOk(), retVal, rc);

    return rc;
}

/*===========================================================================
  FUNCTION  NetdProxy::getServiceImpl
===========================================================================*/
void *NetmgrNetdClient::NetdProxy::getServiceImpl(void *arg)
{
    NetmgrNetdClient::NetdProxy *self = (NetmgrNetdClient::NetdProxy*)(arg);

    self->com.lockConnectMutex();
    self->mService = INetd::getService();
    if (self->mService == NULL)
    {
      NETMGR_LOG_ERR("%s(): service is null", __func__);
    }
    else
    {
      NETMGR_LOG_LOW("%s(): INetd discovered", __func__);
      self->registerLinkToDeath();
    }

    self->com.signalConnectCondition();
    self->com.unlockConnectMutex();

    pthread_exit(0);
}

/*===========================================================================
  FUNCTION  NetdProxy::startGetServiceThread
===========================================================================*/
bool NetmgrNetdClient::NetdProxy::startGetServiceThread()
{
    pthread_attr_t tattr;
    void *args = reinterpret_cast<void *> (this);
    if (pthread_attr_init(&tattr))
    {
        NETMGR_LOG_ERR("Thread attribute object init failed");
        return false;
    }
    else if (pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED))
    {
        NETMGR_LOG_ERR("Could not set detached on the receiving thread");
        pthread_attr_destroy(&tattr);
        return false;
    }
    else if (pthread_create(&getServiceThread, &tattr, getServiceImpl, args))
    {
        NETMGR_LOG_ERR("Could not create monitoring thread");
        pthread_attr_destroy(&tattr);
        return false;
    }
    else
    {
        pthread_attr_destroy(&tattr);
        return true;
    }
}

/*===========================================================================
  FUNCTION  NetmgrNetdClient::NetmgrNetdClient
===========================================================================*/
/*!
@brief
  Initializes NetdProxy object
*/
/*=========================================================================*/
NetmgrNetdClient::NetmgrNetdClient()
{
  initMutex = PTHREAD_MUTEX_INITIALIZER;
  connectMutex = PTHREAD_MUTEX_INITIALIZER;
  connectCondition = PTHREAD_COND_INITIALIZER;
  initCondition = PTHREAD_COND_INITIALIZER;

  isServiceInited = false;

  mApiService = new NetdProxy(*this);
}

/*===========================================================================
  FUNCTION  lockInitMutex
===========================================================================*/
void NetmgrNetdClient::lockInitMutex()
{
  if (pthread_mutex_lock(&initMutex) != 0)
  {
    NETMGR_LOG_ERR("Could not acquire lock in initMutex");
  }
}

/*===========================================================================
  FUNCTION  unlockInitMutex
===========================================================================*/
void NetmgrNetdClient::unlockInitMutex()
{
  if (pthread_mutex_unlock(&initMutex) != 0)
  {
    NETMGR_LOG_ERR("Could not unlock initMutex");
  }
}

/*===========================================================================
  FUNCTION  waitForInitCondition
===========================================================================*/
void NetmgrNetdClient::waitForInitCondition()
{
  if (pthread_cond_wait(&initCondition, &initMutex) != 0)
  {
    NETMGR_LOG_ERR("failed to wait on initCondition");
  }
}

/*===========================================================================
  FUNCTION  signalInitCondition
===========================================================================*/
void NetmgrNetdClient::signalInitCondition()
{
  if (pthread_cond_signal(&initCondition) != 0)
  {
    NETMGR_LOG_ERR("failed to signal initCondition");
  }
}

/*===========================================================================
  FUNCTION  lockConnectMutex
===========================================================================*/
void NetmgrNetdClient::lockConnectMutex()
{
  if (pthread_mutex_lock(&connectMutex) != 0)
  {
    NETMGR_LOG_ERR("Could not acquire lock in connectMutex");
  }
}

/*===========================================================================
  FUNCTION  unlockConnectMutex
===========================================================================*/
void NetmgrNetdClient::unlockConnectMutex()
{
  if (pthread_mutex_unlock(&connectMutex) != 0)
  {
    NETMGR_LOG_ERR("Could not unlock connectMutex");
  }
}

/*===========================================================================
  FUNCTION  waitForConnectCondition
===========================================================================*/
void NetmgrNetdClient::waitForConnectCondition()
{
  if (pthread_cond_wait(&connectCondition, &connectMutex) != 0)
  {
    NETMGR_LOG_ERR("failed to signal connectCondition");
  }
}

/*===========================================================================
  FUNCTION  signalConnectCondition
===========================================================================*/
void NetmgrNetdClient::signalConnectCondition()
{
  if (pthread_cond_signal(&connectCondition) != 0)
  {
    NETMGR_LOG_ERR("failed to signal connectCondition");
  }
}

/*===========================================================================
  FUNCTION  NetdProxy::onRegistration
===========================================================================*/
/*!
@brief
  Callback invoked when netd service comes up
*/
/*=========================================================================*/
Return<void> NetmgrNetdClient::NetdProxy::onRegistration
(
   const hidl_string &fqName,
   const hidl_string &name,
   bool preexisting
)
{
  #pragma unused (fqName)
  #pragma unused (name)

  this->com.lockConnectMutex();

  NETMGR_LOG_MED("%s(): Starting Netd getService thread", __func__);
  startGetServiceThread();

  /* The connectCondition condition variable is used to indicate that the
     internal service thread is up and running. The service thread will
     register for the necessary notifications with netd
     ex. registerLinkToDeath */
  this->com.waitForConnectCondition();

  /* After unblocking from connectCondition we can broadcast the initCondition
     condition variable. This will unblock any thread blocked in getInstance
     function. In a netd restart scenario no one is expected to be blocked on
     getInstance so the broadcast operation will not have any side-effect */
  this->com.lockInitMutex();
  this->com.isServiceInited = true;
  this->com.signalInitCondition();
  this->com.unlockInitMutex();

  this->com.unlockConnectMutex();

  if (preexisting)
  {
    NETMGR_LOG_MED("%s(): Service exists, not a restart", __func__);
  }
  else
  {
    NETMGR_LOG_HIGH("%s(): netd restart detected", __func__);
    netmgr_kif_handle_netd_event();
#ifdef NETMGR_QOS_ENABLED
    (void)netmgr_tc_reorder_qos_post_routing_chain_ref();
#endif
  }

  return Void();
}

/*===========================================================================
  FUNCTION  NetdProxy::serviceDied
===========================================================================*/
/*!
@brief
  Callback invoked when netd service dies or stops
*/
/*=========================================================================*/
void NetmgrNetdClient::NetdProxy::serviceDied
(
  uint64_t cookie,
  const wp<IBase>& who
)
{
  #pragma unused (cookie)
  #pragma unused (who)

  NETMGR_LOG_ERR("%s(): Netd service died", __func__);

  this->com.lockConnectMutex();
  this->com.isServiceInited = false;
  mService = NULL;
  this->com.unlockConnectMutex();
}

/*===========================================================================
  FUNCTION  NetdProxy::addRouteToOemNetwork
===========================================================================*/
/*!
@brief
  Add route to a specified OEM network

@param networkHandle - Handle to the OEM network obtained
                       using registration API
@param ifname        - Interface name
@param destination   - The destination prefix of the route
@param nexthop       - IP address of the gateway for the route or
                       NULL for a directly-connected route
*/
/*=========================================================================*/
bool NetmgrNetdClient::NetdProxy::addRouteToOemNetwork
(
   uint64_t networkHandle,
   const hidl_string& ifname,
   const hidl_string& destination,
   const hidl_string& nexthop
)
{
    bool rc = false;

    this->com.lockConnectMutex();

    if (mService == NULL)
    {
      NETMGR_LOG_ERR("%s(): mService is NULL", __func__);
      this->com.unlockConnectMutex();
      return rc;
    }

    NETMGR_LOG_LOW("%s(): Attempting addRouteToOemNetwork", __func__);
    auto retVal = mService->addRouteToOemNetwork(networkHandle,
                                                 ifname,
                                                 destination,
                                                 nexthop);

    this->com.unlockConnectMutex();

    CHECK_ERROR(retVal.isOk(), retVal, rc);

    return rc;
}

/*===========================================================================
  FUNCTION  NetdProxy::removeRouteFromOemNetwork
===========================================================================*/
/*!
@brief
  Remove route from a specified OEM network

@param networkHandle - Handle to the OEM network obtained
                       using registration API
@param ifname        - Interface name
@param destination   - The destination prefix of the route
@param nexthop       - IP address of the gateway for the route or
                       empty string for a directly-connected route
*/
/*=========================================================================*/
bool NetmgrNetdClient::NetdProxy::removeRouteFromOemNetwork
(
   uint64_t networkHandle,
   const hidl_string& ifname,
   const hidl_string& destination,
   const hidl_string& nexthop
)
{
    bool rc = false;

    this->com.lockConnectMutex();

    if (mService == NULL)
    {
      NETMGR_LOG_ERR("%s(): mService is NULL", __func__);
      this->com.unlockConnectMutex();
      return rc;
    }

    NETMGR_LOG_LOW("%s(): Attempting removeRouteFromOemNetwork", __func__);
    auto retVal = mService->removeRouteFromOemNetwork(networkHandle,
                                                      ifname,
                                                      destination,
                                                      nexthop);

    this->com.unlockConnectMutex();

    CHECK_ERROR(retVal.isOk(), retVal, rc);

    return rc;
}

/*===========================================================================
  FUNCTION  NetdProxy::addInterfaceToOemNetwork
===========================================================================*/
/*!
@brief
  Add interface to a specified OEM network

@param networkHandle - Handle to the OEM network obtained
                       using registration API
@param ifname        - Interface name
*/
/*=========================================================================*/
bool NetmgrNetdClient::NetdProxy::addInterfaceToOemNetwork
(
   uint64_t networkHandle,
   const hidl_string& ifname
)
{
    bool rc = false;

    this->com.lockConnectMutex();

    if (mService == NULL)
    {
      NETMGR_LOG_ERR("%s(): mService is NULL", __func__);
      this->com.unlockConnectMutex();
      return rc;
    }

    NETMGR_LOG_LOW("%s(): Attempting addInterfaceToOemNetwork", __func__);
    auto retVal = mService->addInterfaceToOemNetwork(networkHandle, ifname);

    this->com.unlockConnectMutex();

    CHECK_ERROR(retVal.isOk(), retVal, rc);

    return rc;
}

/*===========================================================================
  FUNCTION  NetdProxy::removeInterfaceFromOemNetwork
===========================================================================*/
/*!
@brief
  Remove interface from a specified OEM network

@param networkHandle - Handle to the OEM network obtained
                       using registration API
@param ifname        - Interface name
*/
/*=========================================================================*/
bool NetmgrNetdClient::NetdProxy::removeInterfaceFromOemNetwork
(
   uint64_t networkHandle,
   const hidl_string& ifname
)
{
    bool rc = false;

    this->com.lockConnectMutex();

    if (mService == NULL)
    {
      NETMGR_LOG_ERR("%s(): mService is NULL", __func__);
      this->com.unlockConnectMutex();
      return rc;
    }

    NETMGR_LOG_LOW("%s(): Attempting removeInterfaceFromOemNetwork", __func__);
    auto retVal = mService->removeInterfaceFromOemNetwork(networkHandle,
                                                          ifname);

    this->com.unlockConnectMutex();

    CHECK_ERROR(retVal.isOk(), retVal, rc);

    return rc;
}

/*===========================================================================
  FUNCTION  NetdProxy::setIpForwardEnable
===========================================================================*/
/*!
@brief
  Enable/disables IP forwarding on the system

@param 1 to enable forwarding
       0 to disable forwarding
*/
/*=========================================================================*/
bool NetmgrNetdClient::NetdProxy::setIpForwardEnable(bool enable)
{
    bool rc = false;

    this->com.lockConnectMutex();

    if (mService == NULL)
    {
      NETMGR_LOG_ERR("%s(): mService is NULL", __func__);
      this->com.unlockConnectMutex();
      return rc;
    }

    NETMGR_LOG_LOW("%s(): Attempting setIpForwardEnable", __func__);
    auto retVal = mService->setIpForwardEnable(enable);

    this->com.unlockConnectMutex();

    CHECK_ERROR(retVal.isOk(), retVal, rc);

    return rc;
}

/*===========================================================================
  FUNCTION  NetdProxy::setForwardingBetweenInterfaces
===========================================================================*/
/*!
@brief
  Enables forwarding between two interfaces, one of which must be in an
  OEM network

@param inputIfName - Input interface
@param inputIfName - Output interface
@param enable      - 1 to enable forwarding, 0 to disable forwarding
*/
/*=========================================================================*/
bool NetmgrNetdClient::NetdProxy::setForwardingBetweenInterfaces
(
   const hidl_string& inputIfName,
   const hidl_string& outputIfName,
   bool               enable
)
{
    bool rc = false;

    this->com.lockConnectMutex();

    if (mService == NULL)
    {
      NETMGR_LOG_ERR("%s(): mService is NULL", __func__);
      this->com.unlockConnectMutex();
      return rc;
    }

    NETMGR_LOG_LOW("%s(): Attempting setForwardingBetweenInterfaces",
                   __func__);
    auto retVal = mService->setForwardingBetweenInterfaces(inputIfName,
                                                           outputIfName,
                                                           enable);

    this->com.unlockConnectMutex();

    CHECK_ERROR(retVal.isOk(), retVal, rc);

    return rc;
}
#endif /* __cplusplus*/

extern "C" {
/*===========================================================================
  FUNCTION  NetmgrNetdClientInit
===========================================================================*/
/*!
@brief
  Initializes netmgr netd client instance
*/
/*=========================================================================*/
int NetmgrNetdClientInit(void)
{
  NETMGR_LOG_LOW("%s(): Looking for Netd service", __func__);
  NetmgrNetdClient* netd_client = NetmgrNetdClient::getInstance();
  if (netd_client == NULL)
  {
    NETMGR_LOG_ERR("%s(): Failed to allocate netd client", __func__);
    return NETMGR_FAILURE;
  }

  NETMGR_LOG_MED("%s(): Created netd client", __func__);
  return NETMGR_SUCCESS;
}

/*===========================================================================
  FUNCTION  NetmgrNetdClientRegisterNetwork
===========================================================================*/
/*!
@brief
  Registers with netd to create a custom oem network

@param networkType - IWLAN or LINK_LOCAL

@return networkSettings
        networkHandle - Handle to the created OEM network
        packetMark    - Unique packet mark associated with
                        the OEM network
*/
/*=========================================================================*/
int NetmgrNetdClientRegisterNetwork
(
  struct networkSettings *ns,
  char                   *networkType
)
{
  uint64_t networkHandle = 0;
  uint32_t packetMark = 0;

  if ((ns == NULL) || (networkType == NULL))
  {
    NETMGR_LOG_ERR("%s(): Network Settings input buffer is NULL", __func__);
    return NETMGR_FAILURE;
  }

  NETMGR_LOG_LOW("%s(): Looking for Netd service", __func__);
  NetmgrNetdClient* netd_client = NetmgrNetdClient::getInstance();
  if (netd_client == NULL)
  {
    NETMGR_LOG_ERR("%s(): Failed to allocate netd client", __func__);
    return NETMGR_FAILURE;
  }

  NETMGR_LOG_LOW("%s(): register to create new OEM network", __func__);
  std::tie(networkHandle, packetMark) = netd_client->mApiService->registerNetwork();
  NETMGR_LOG_MED("%s(): [packet mark : 0x%x] [network handle : 0x%" PRIx64 "]",
                 __func__, packetMark, networkHandle);

  if ((networkHandle) && (packetMark))
  {
    ns->networkHandle = networkHandle;
    ns->packetMark = packetMark;
    netd_client->setNetworkHandle(networkType, networkHandle);
    return NETMGR_SUCCESS;
  }

  return NETMGR_FAILURE;
}

/*===========================================================================
  FUNCTION  NetmgrNetdClientUnregisterNetwork
===========================================================================*/
/*!
@brief
  Unregisters with netd and destroys the custom oem network

@param networkHandle - Handle to the OEM network obtained
                       using registration API
@param networkType   - IWLAN or LINK_LOCAL

@return NETMGR_SUCCESS or NETMGR_FAILURE
*/
/*=========================================================================*/
int NetmgrNetdClientUnregisterNetwork
(
  uint64_t networkHandle,
  char     *networkType
)
{
  if (networkType == NULL)
  {
    NETMGR_LOG_ERR("%s(): network type is invalid!", __func__);
    return NETMGR_FAILURE;
  }

  NETMGR_LOG_LOW("%s(): Looking for Netd service", __func__);
  NetmgrNetdClient* netd_client = NetmgrNetdClient::getInstance();
  if (netd_client == NULL)
  {
    NETMGR_LOG_ERR("%s(): Failed to allocate netd client", __func__);
    return NETMGR_FAILURE;
  }

  if (netd_client->mApiService->unregisterNetwork(networkHandle))
  {
    NETMGR_LOG_LOW("%s(): completed command", __func__);

    /* Clear the networkHandle associated with the networkType */
    netd_client->setNetworkHandle(networkType, 0);

    return NETMGR_SUCCESS;
  }

  NETMGR_LOG_ERR("%s(): command failed!", __func__);
  return NETMGR_FAILURE;
}

/*===========================================================================
  FUNCTION  NetmgrNetdClientAddRouteToOemNetwork
===========================================================================*/
/*!
@brief
  Add route to a specified OEM network

@param networkHandle - Handle to the OEM network obtained
                       using registration API
@param ifname        - Interface name
@param destination   - The destination prefix of the route
@param nexthop       - IP address of the gateway for the route or
                       NULL for a directly-connected route

@return NETMGR_SUCCESS or NETMGR_FAILURE
*/
/*=========================================================================*/
int NetmgrNetdClientAddRouteToOemNetwork
(
  uint64_t   networkHandle,
  const char *ifname,
  const char *destination,
  const char *nexthop
)
{
  NETMGR_LOG_LOW("%s(): Looking for Netd service", __func__);
  NetmgrNetdClient *netd_client = NetmgrNetdClient::getInstance();
  if (netd_client == NULL)
  {
    NETMGR_LOG_ERR("%s(): Failed to allocate netd client", __func__);
    return NETMGR_FAILURE;
  }

  NETMGR_LOG_MED("%s(): starting command", __func__);
  if (netd_client->mApiService->addRouteToOemNetwork(networkHandle,
                                                     ifname,
                                                     destination,
                                                     nexthop))
  {
    NETMGR_LOG_LOW("%s(): completed command", __func__);
    return NETMGR_SUCCESS;
  }

  return NETMGR_FAILURE;
}

/*===========================================================================
  FUNCTION  NetmgrNetdClientRemoveRouteFromOemNetwork
===========================================================================*/
/*!
@brief
  Remove route from a specified OEM network

@param networkHandle - Handle to the OEM network obtained
                       using registration API
@param ifname        - Interface name
@param destination   - The destination prefix of the route
@param nexthop       - IP address of the gateway for the route or
                       NULL for a directly-connected route

@return NETMGR_SUCCESS or NETMGR_FAILURE
*/
/*=========================================================================*/
int NetmgrNetdClientRemoveRouteFromOemNetwork
(
  uint64_t   networkHandle,
  const char *ifname,
  const char *destination,
  const char *nexthop
)
{
  NETMGR_LOG_LOW("%s(): Looking for Netd service", __func__);
  NetmgrNetdClient *netd_client = NetmgrNetdClient::getInstance();
  if (netd_client == NULL)
  {
    NETMGR_LOG_ERR("%s(): Failed to allocate netd client", __func__);
    return NETMGR_FAILURE;
  }

  NETMGR_LOG_MED("%s(): starting command", __func__);
  if (netd_client->mApiService->removeRouteFromOemNetwork(networkHandle,
                                                          ifname,
                                                          destination,
                                                          nexthop))
  {
    NETMGR_LOG_LOW("%s(): completed command", __func__);
    return NETMGR_SUCCESS;
  }

  return NETMGR_FAILURE;
}

/*===========================================================================
  FUNCTION  NetmgrNetdClientAddInterfaceToOemNetwork
===========================================================================*/
/*!
@brief
  Add interface to a specified OEM network

@param networkHandle - Handle to the OEM network obtained
                       using registration API
@param ifname        - Interface name

@return NETMGR_SUCCESS or NETMGR_FAILURE
*/
/*=========================================================================*/
int NetmgrNetdClientAddInterfaceToOemNetwork
(
  uint64_t   networkHandle,
  const char *ifname
)
{
  NETMGR_LOG_LOW("%s(): Looking for Netd service", __func__);
  NetmgrNetdClient *netd_client = NetmgrNetdClient::getInstance();
  if (netd_client == NULL)
  {
    NETMGR_LOG_ERR("%s(): Failed to allocate netd client", __func__);
    return NETMGR_FAILURE;
  }

  NETMGR_LOG_MED("%s(): starting command", __func__);
  if (netd_client->mApiService->addInterfaceToOemNetwork(networkHandle,
                                                         ifname))
  {
    NETMGR_LOG_LOW("%s(): completed command", __func__);
    return NETMGR_SUCCESS;
  }

  return NETMGR_FAILURE;
}

/*===========================================================================
  FUNCTION  NetmgrNetdClientRemoveInterfaceFromOemNetwork
===========================================================================*/
/*!
@brief
  Remove interface from a specified OEM network

@param networkHandle - Handle to the OEM network obtained
                       using registration API
@param ifname        - Interface name

@return NETMGR_SUCCESS or NETMGR_FAILURE
*/
/*=========================================================================*/
int NetmgrNetdClientRemoveInterfaceFromOemNetwork
(
  uint64_t   networkHandle,
  const char *ifname
)
{
  NETMGR_LOG_LOW("%s(): Looking for Netd service", __func__);
  NetmgrNetdClient *netd_client = NetmgrNetdClient::getInstance();
  if (netd_client == NULL)
  {
    NETMGR_LOG_ERR("%s(): Failed to allocate netd client", __func__);
    return NETMGR_FAILURE;
  }

  NETMGR_LOG_MED("%s(): starting command", __func__);
  if (netd_client->mApiService->removeInterfaceFromOemNetwork(networkHandle,
                                                              ifname))
  {
    NETMGR_LOG_LOW("%s(): completed command", __func__);
    return NETMGR_SUCCESS;
  }

  return NETMGR_FAILURE;
}

/*===========================================================================
  FUNCTION  NetmgrNetdClientSetIpForwardEnable
===========================================================================*/
/*!
@brief
  Enable/disables IP forwarding on the system

@param 1 to enable forwarding
       0 to disable forwarding

@return NETMGR_SUCCESS or NETMGR_FAILURE
*/
/*=========================================================================*/
int NetmgrNetdClientSetIpForwardEnable(int enable)
{
  NETMGR_LOG_LOW("%s(): Looking for Netd service", __func__);
  NetmgrNetdClient *netd_client = NetmgrNetdClient::getInstance();
  if (netd_client == NULL)
  {
    NETMGR_LOG_ERR("%s(): Failed to allocate netd client", __func__);
    return NETMGR_FAILURE;
  }

  NETMGR_LOG_MED("%s(): starting command", __func__);
  if (netd_client->mApiService->setIpForwardEnable(enable))
  {
    NETMGR_LOG_LOW("%s(): completed command", __func__);
    return NETMGR_SUCCESS;
  }

  return NETMGR_FAILURE;
}

/*===========================================================================
  FUNCTION  NetmgrNetdClientSetForwardingBetweenInterfaces
===========================================================================*/
/*!
@brief
  Enables forwarding between two interfaces, one of which must be in an
  OEM network

@param inputIfName - Input interface
@param inputIfName - Output interface
@param enable      - 1 to enable forwarding, 0 to disable forwarding

@return NETMGR_SUCCESS or NETMGR_FAILURE
*/
/*=========================================================================*/
int NetmgrNetdClientSetForwardingBetweenInterfaces
(
  const char *inputIfName,
  const char *outputIfName,
  int        enable
)
{
  NETMGR_LOG_LOW("%s(): Looking for Netd service", __func__);
  NetmgrNetdClient *netd_client = NetmgrNetdClient::getInstance();
  if (netd_client == NULL)
  {
    NETMGR_LOG_ERR("%s(): Failed to allocate netd client", __func__);
    return NETMGR_FAILURE;
  }

  NETMGR_LOG_MED("%s(): starting command", __func__);
  if (netd_client->mApiService->setForwardingBetweenInterfaces(inputIfName,
                                                               outputIfName,
                                                               enable))
  {
    NETMGR_LOG_LOW("%s(): completed command", __func__);
    return NETMGR_SUCCESS;
  }

  return NETMGR_FAILURE;
}

/*===========================================================================
  FUNCTION  NetmgrNetdClientGetNetworkHandle
===========================================================================*/
/*!
@brief
  Returns the network handle associated with the given network type

@param networkType - IWLAN or LINK_LOCAL

@return NETMGR_SUCCESS or NETMGR_FAILURE
*/
/*=========================================================================*/
uint64_t NetmgrNetdClientGetNetworkHandle(char *networkType)
{
  NetmgrNetdClient* netd_client = NetmgrNetdClient::getInstance();
  uint64_t networkHandle;

  if (networkType == NULL)
  {
    return NETMGR_FAILURE;
  }

  if (netd_client == NULL)
  {
    NETMGR_LOG_ERR("%s(): Failed to allocate netd client", __func__);
    return NETMGR_FAILURE;
  }

  networkHandle = netd_client->getNetworkHandle(networkType);
  NETMGR_LOG_MED("%s(): [network handle : %" PRIu64 "] [network type : %s]",
                 __func__, networkHandle,  networkType);

  return networkHandle;
}
} /* extern "C" */

#endif /* !defined(NETMGR_OFFTARGET) && defined(FEATURE_DS_LINUX_ANDROID)  */
