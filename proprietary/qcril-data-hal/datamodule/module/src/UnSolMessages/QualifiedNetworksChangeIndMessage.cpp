/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <UnSolMessages/QualifiedNetworksChangeIndMessage.h>

using namespace rildata;

QualifiedNetworksChangeIndMessage::~QualifiedNetworksChangeIndMessage()
{
}

std::shared_ptr<UnSolicitedMessage> QualifiedNetworksChangeIndMessage::clone()
{
  return std::make_shared<QualifiedNetworksChangeIndMessage>( mQualifiedNetworks);
}

std::vector<QualifiedNetwork_t> QualifiedNetworksChangeIndMessage::getQualifiedNetworks()
{
  return mQualifiedNetworks;
}

string QualifiedNetworksChangeIndMessage::dump()
{
  return mName;
}

