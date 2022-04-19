/******************************************************************************

                        N E T M G R _ N E T D _C L I E N T . H

******************************************************************************/

/******************************************************************************

  @file    netmgr_netd_client.h
  @brief   Network manager netd client

  DESCRIPTION
  Header file for netd client module

******************************************************************************/
/*===========================================================================

  Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*===========================================================================
                     GLOBAL DEFINITIONS AND DECLARATIONS
===========================================================================*/
struct networkSettings
{
  uint64_t networkHandle;
  uint32_t packetMark;
};

#if (!defined(NETMGR_OFFTARGET) && defined(FEATURE_DS_LINUX_ANDROID))
/*===========================================================================
  FUNCTION  NetmgrNetdClientInit
===========================================================================*/
/*!
@brief
  Initializes netmgr netd client instance
*/
/*=========================================================================*/
int NetmgrNetdClientInit(void);

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
);

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
);

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
);

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
);

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
);

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
);

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
int NetmgrNetdClientSetIpForwardEnable(int enable);

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
);

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
uint64_t NetmgrNetdClientGetNetworkHandle(char *networkType);
#else
static int NetmgrNetdClientInit(void)
{
  return 1;
}

static int NetmgrNetdClientRegisterNetwork(struct networkSettings *ns, char *networkType)
{
  return 1;
}

static uint64_t NetmgrNetdClientGetNetworkHandle(char *networkType)
{
  return 0;
}

#endif /* !defined(NETMGR_OFFTARGET) && defined(FEATURE_DS_LINUX_ANDROID) */

#ifdef __cplusplus
}
#endif /* __cplusplus */
