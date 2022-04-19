#ifndef CNE_WQE_INTERFACE_H
#define CNE_WQE_INTERFACE_H

/*=============================================================================
               Copyright (c) 2014-2016 Qualcomm Technologies, Inc.
               All Rights Reserved.
               Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

#include "CneCom.h"
#include "CneDefs.h"
#include "CneSrmDefs.h"
#include "CneTimer.h"
#include "SwimNetlinkSocket.h"
#include "SocketWrapperClient.h"
#include "CneConfigs.h"
#include "LocalLogBuffer.h"

#define INTERNET_PROFILE "profile-internet"
#define SLM_PROFILE   "profileSlm"

class SwimInterfaceSelection;
class BQSampling;
class IBitrateEstimator;
class IInterfaceSelection;
class IDefaultInterfaceSelector;
class CnePolicyConfig;
class CneDefaultDnsSelector;
class SwimSocketManager;
class ISocketManager;
class IWqeAgent;

class CneWqeInterface
{
  public:
    CneWqeInterface(){}
    virtual ~CneWqeInterface(){}
    virtual IWqeAgent* createWqeAgent(std::string profileType, CneTimer &timer, CneCom &com)=0;
    virtual IWqeAgent* createWqeAgent(std::string profileType, CneTimer &timer, CneCom &com, const CQEClientOverrides &overrides, const std::string &oper)=0;
    virtual int destroyWqeAgent(IWqeAgent *iwqeA)=0;
    virtual void createWqe(CneCom &com, CneTimer &timer, LocalLogBuffer * const logBuf, bool isSwimEnabled)=0;
    virtual IBitrateEstimator* getBitrateEstimator()=0;
    virtual SwimNetlinkSocket *getNetlinkSocket()=0;
    virtual bool isBbdDisabled()=0;
    virtual CneRetType detectBrokenBackhaul(CneIpcReqMsg_t& req, int fd)=0;
    virtual bool isDefaultSelectorStateWlanGood()=0;
    virtual SwimNimsRetCodeType_t Select( int rfd, SwimNimsSockAddr_t& src, SwimNimsSockAddr_t& dst,
                                          int comfd, int client_fd_val, int uid, int pid)=0;
    virtual bool verifySockExists(int uid, int pid, int fd_val, int rfd)=0;
    virtual void updateAppSockFd(int uid, int pid, int fd_val, int rfd)=0;
    virtual void handleHangup(int comfd)=0;
    virtual void handleClose(int uid, int pid, int comfd, int client_fd_val)=0;

    virtual void createSlmAgent(CneCom &com, CneTimer &timer, LocalLogBuffer * const logBuf)=0;
    virtual void destroySlmAgent()=0;

    virtual void startLinkQualityEstimator(CneWlanResourceType &primaryNetwork, SlmRatInfoType &secondaryNetwork)=0;
    virtual void stopLinkQualityEstimator()=0;

    virtual void updateSlmLinkProperties(CneWlanResourceType &primaryNetwork, SlmRatInfoType &secondaryNetwork)=0;
};
#endif
