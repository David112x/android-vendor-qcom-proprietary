/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once

#include <list>
#include "BaseProfile.h"
#include "qdp_adapter.h"

/**
 * @brief      Profile class for Attach procedure.
 */
class AttachProfile : public BaseProfile
{
public:
  AttachProfile(params_t p): BaseProfile(p) {}
  AttachProfile() = default;
  virtual ~AttachProfile() = default;
  AttachProfile& operator=( const AttachProfile& ) = default;
  /**
   * @brief      Look up params to see whether it matches any profile.
   *             Most parameters are matched strictly.
   *             IP addresses may have loose matching.
   *
   *
   * @return     profile ID if matched, INVALID_PROFILE_ID otherwise
   */
  unsigned int lookup() const;
  /**
   * @brief      Given a list of profile IDs, determine which matches
   *             one the profile represented by the current instance.
   *             Protocol uses loose matching.
   *             Not all parameters matched.
   *
   * @return     profile ID if matched, INVALID_PROFILE_ID otherwise
   *
   */
  virtual unsigned int findMatch(
  const std::list<uint16_t>& profile_list) const override;

  params_t getParams();
};