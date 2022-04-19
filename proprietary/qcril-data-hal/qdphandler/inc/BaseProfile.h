/******************************************************************************
#  Copyright (c) 2018, 2020 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once

#include <stdlib.h>
#include <string>
#include <list>
#include "qdp.h"

/**
 * @brief      Base profile that defines all the profile parameters.
 *             It provides the definition of a lookup method that any deriving
 *             class must implement.
 */
class BaseProfile
{
public:
  struct params_t
  {
    int32_t profileId;
    uint8_t radioTech;
    std::string apn;
    std::string username;
    std::string password;
    std::string authType;
    /**
     * the MVNO type: possible values are "imsi", "gid", "spn"
     */
    std::string mvnoType;      /* removed in Radio HAL 1.4 */
    std::string mvnoMatchData; /* removed in Radio HAL 1.4 */
    /**
    * one of the PDP_type values such as "IP", "IPV6", "IPV4V6", or "PPP".
    */
    std::string protocol;
    /**
    * one of the PDP_type values such as "IP", "IPV6", "IPV4V6", or "PPP".
    */
    std::string roamingProtocol;
    uint32_t apnTypeBitmask;
    uint32_t bearerBitmask;
    uint8_t modemCognitive;   /* removed in Radio HAL 1.4 */
    uint8_t mtu;
    bool roamingAllowed;
    bool preferred;           /* added in Radio HAL 1.4 */
    bool persistent;          /* added in Radio HAL 1.4 */
    int32_t maxConnsTime;
    int32_t maxConns;
    int32_t waitTime;
    bool enableProfile;

    params_t()
    {
      profileId = 0;
      radioTech = 0;
      apn = "";
      username = "";
      password = "";
      authType = "";
      mvnoType = "";
      mvnoMatchData = "";
      protocol = "";
      roamingProtocol = "";
      apnTypeBitmask = 0;
      bearerBitmask = 0;
      modemCognitive = false;
      mtu = 0;
      roamingAllowed = false;
      preferred = false;
      persistent = false;
      maxConnsTime = 0;
      maxConns = 0;
      waitTime = 0;
      enableProfile = false;
    }
  };

  static const unsigned int INVALID_PROFILE_ID = QDP_INVALID_PROFILE;

  BaseProfile(params_t p): params(p) {}
  BaseProfile() = default;
  virtual ~BaseProfile() = default;
  virtual void dump(std::string padding, std::ostream& os);
  /**
   * @brief      Look up wrapper which will call qdp specfic lookup
                 API
   *
   * @return     profie id if found, INVALID_PROFILE_ID otherwise
   */
  unsigned int lookup() const;
  /**
   * @brief      Look up wrapper which will call qdp specfic lookup
                 via APN class
   *
   * @return     QDP_FAILURE if profile was not found/created.
   *             QDP_SUCCESS otherwise
   */
  virtual unsigned int lookupByApnClass(params_t in_params) const;

  /**
   * @brief      Param Match wrapper which will call qdp specfic
                 paramter match API
   *
   * @return     profile ID if matched, INVALID_PROFILE_ID otherwise
   *
   */
  virtual unsigned int findMatch(
  const std::list<uint16_t>& profile_list) const;

protected:
  params_t params;
  //TO BE ENABLED LATER
  /**
   * @brief      Match home protocol (IP type) strictly
   *
   * @param[in]  other  protocol of the profile on modem
   *
   * @return     true or false
   */
  //virtual bool matchHomeProtocolStrict(std::string other) const;
  /**
   * @brief      Match home protocol (IP type) loosely
   *             i.e. if modem has IPv4v6 and we
   *             have IP or IPv6, then that is considered
   *             a match.
   *
   * @param[in]  other  protocol of the profile on modem
   *
   * @return     true or false
   */
  //virtual bool matchHomeProtocolLoose(std::string other) const;
  /**
   * @brief      Match home protocol (IP type) strictly
   *
   * @param[in]  other  protocol of the profile on modem
   *
   * @return     true or false
   */
  //virtual bool matchRoamProtocolStrict(std::string other) const;
  /**
   * @brief      Match roam protocol (IP type) loosely
   *             i.e. if modem has IPv4v6 and we
   *             have IP or IPv6, then that is considered
   *             a match.
   *
   * @param[in]  other  protocol of the profile on modem
   *
   * @return     true or false
   */
  //virtual bool matchRoamProtocolLoose(std::string other) const;
private:
  const std::string mName = "[BaseProfile]: ";
};
