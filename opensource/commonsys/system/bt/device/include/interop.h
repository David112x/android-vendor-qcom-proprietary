/******************************************************************************
 *
 *  Copyright (C) 2015 Google, Inc.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/

#pragma once

#include <stdbool.h>

#include "raw_address.h"

static const char INTEROP_MODULE[] = "interop_module";

// NOTE:
// Only add values at the end of this enum and before END_OF_INTEROP_LIST
// do NOT delete values as they may be used in dynamic device configuration.
typedef enum {

  BEGINING_OF_INTEROP_LIST = 0,
  // Disable secure connections
  // This is for pre BT 4.1/2 devices that do not handle secure mode
  // very well.
  INTEROP_DISABLE_LE_SECURE_CONNECTIONS = BEGINING_OF_INTEROP_LIST,

  // Some devices have proven problematic during the pairing process, often
  // requiring multiple retries to complete pairing. To avoid degrading the user
  // experience for those devices, automatically re-try pairing if page
  // timeouts are received during pairing.
  INTEROP_AUTO_RETRY_PAIRING,

  // Devices requiring this workaround do not handle Bluetooth Absolute Volume
  // control correctly, leading to undesirable (potentially harmful) volume
  // levels or general lack of controlability.
  INTEROP_DISABLE_ABSOLUTE_VOLUME,

  // Disable automatic pairing with headsets/car-kits
  // Some car kits do not react kindly to a failed pairing attempt and
  // do not allow immediate re-pairing. Blacklist these so that the initial
  // pairing attempt makes it to the user instead.
  INTEROP_DISABLE_AUTO_PAIRING,

  // Use a fixed pin for specific keyboards
  // Keyboards should use a variable pin at all times. However, some keyboards
  // require a fixed pin of all 0000. This workaround enables auto pairing for
  // those keyboards.
  INTEROP_KEYBOARD_REQUIRES_FIXED_PIN,

  // Some headsets have audio jitter issues because of increased
  // re-transmissions as the 3 Mbps packets have a lower link margin, and are
  // more prone to interference. We can disable 3DH packets (use only 2DH
  // packets) for the ACL link to improve sensitivity when streaming A2DP audio
  // to the headset. Air sniffer logs show reduced re-transmissions after
  // switching to 2DH packets.

  // Disable 3Mbps packets and use only 2Mbps packets for ACL links when
  // streaming audio.
  INTEROP_2MBPS_LINK_ONLY,

  // Some HID devices have proven problematic behaviour if SDP is initiated
  // again while HID connection is in progress or if more than 1 SDP connection
  // is  created with those HID devices rsulting in issues of connection
  // failure with such devices. To avoid degrading the user experience with
  // those devices, SDP is not attempted as part of pairing process.
  INTEROP_DISABLE_SDP_AFTER_PAIRING,

  // HID Keyboards that claim support for multitouch functionality have issue
  // with  normal functioning of keyboard because of issues in USB HID kernel
  // driver. To avoid degrading the user experience with those devices,
  // digitizer record is removed from the report descriptor.
  INTEROP_REMOVE_HID_DIG_DESCRIPTOR,

  // Some HID devices have problematic behaviour where when hid link is in Sniff
  // and DUT is in Slave role for SCO link ( not eSCO) any solution cannot maintain
  // the link as  SCO scheduling over a short period will overlap with Sniff link due to
  // slave drift.
  // To avoid degrading the user experience with those devices, sniff is disabled from
  // link policy when sco is active, and enabled when sco is disabled.
  INTEROP_DISABLE_SNIFF_DURING_SCO,

  // Do not use supervision timeout value received from preferred connection
  // parameters, use 3s instead. Use with HID only.
  INTEROP_HID_PREF_CONN_SUP_TIMEOUT_3S,

  // Do not send service changed indications (GATT client).
  // This should be removed after the characteristic is implmeented b/62088395.
  INTEROP_GATTC_NO_SERVICE_CHANGED_IND,

  //Few carkits take long time to start sending AT commands
  //Increase AG_CONN TIMEOUT so that AG connection go through
  INTEROP_INCREASE_AG_CONN_TIMEOUT,

  // Some HOGP devices do not respond well when we switch from default LE conn parameters
  // to preferred conn params immediately post connection. Disable automatic switching to
  // preferred conn params for such devices and allow them to explicity ask for it.
  INTEROP_DISABLE_LE_CONN_PREFERRED_PARAMS,

  // Few remote devices do not understand AVRCP version greater than 1.3. For these
  // devices, we would like to blacklist them and advertise AVRCP version as 1.3
  INTEROP_ADV_AVRCP_VER_1_3,

  // certain remote A2DP sinks have issue playing back Music in AAC format.
  // disable AAC for those headsets so that it switch to SBC
  INTEROP_DISABLE_AAC_CODEC,

  // Enable AAC only for whitelist of devices
  INTEROP_ENABLE_AAC_CODEC,

  // Disable role switch for headsets/car-kits
  // Some car kits initiate a role switch but won't initiate encryption
  // after role switch complete
  INTEROP_DISABLE_ROLE_SWITCH_POLICY,

  INTEROP_HFP_1_7_BLACKLIST,

  // Some Carkits are not initiating AVRCP Browse Channel on
  // seeing DUT's AVRCP version as v1.6. Hence fallback DUT's
  // AVRCP version to v1.4 for those Carkits
  INTEROP_STORE_REMOTE_AVRCP_VERSION_1_4,

  // Devices requiring this workaround do not handle Bluetooth PBAP 1.2 version correctly,
  // leading them to go in bad state. So for better interoperability respond with PBAP 1.1
  // as supported version.
  INTEROP_ADV_PBAP_VER_1_1,

  // Devices requiring this workaround do not handle SSR max latency values as mentioned,
  // in their SDP HID Record properly and lead to connection timeout or lags. To prevent
  // such scenarios, device requiring this workaorund need to use specific ssr max latency
  // values.
  INTEROP_UPDATE_HID_SSR_MAX_LAT,

  // Some HID pointing devices have proven problematic behaviour if pairing is initiated with
  // them, resulting in no response for authentication request and ultimately resulting
  // in connection failure.
  // To avoid degrading the user experience with those devices, authentication request
  // is not requested explictly.
  INTEROP_DISABLE_AUTH_FOR_HID_POINTING,

  // Do not use AVDTP RECONFIGURE when reconfiguring A2DP streams.
  // Some A2DP Sink devices report SUCCESS to the AVDTP RECONFIGURE command,
  // but fail to play the reconfigured audio stream.
  INTEROP_DISABLE_AVDTP_RECONFIGURE,

  // Create dynamic blacklist to disable role switch.
  // Some car kits indicate that role switch is supported, but then reject
  // role switch attempts. After rejecting several role switch attempts,
  // such car kits will go into bad state.
  INTEROP_DYNAMIC_ROLE_SWITCH,

  //Few carkit hfp version is hfp1.5 but it support hfp indicator, violate spec
  //remove hfp indicator for such device
  INTEROP_DISABLE_HF_INDICATOR,

  // Disable role switch for headsets/car-kits.
  // Some car kits allow role switch but when the Phone initiates role switch,
  // the Remote device will go into bad state that will lead to LMP time out.
  INTEROP_DISABLE_ROLE_SWITCH,
  
  // Some remotes are very strict in receiving the call active
  // indicator and SCO connection request order for MT call.
  // If CIEV1,1 and SCO connection request are sent back to back
  // to SOC, it may send SCO connection request first then CIEV1,1
  // which may lead to remotes not rendering SCO audio.
  INTEROP_DELAY_SCO_FOR_MT_CALL,

  // Some remotes are taking too long to respond for codec negotiation.
  // Disable codec negotiation for such remotes and directly initiate
  // SCO Connection.
  INTEROP_DISABLE_CODEC_NEGOTIATION,

  // Some Carkits being AVRCP v1.3 upon receiving Play Application Setting Command Response
  // and notification, doesn't send Passthrough commands back to DUT in Streaming State
  INTEROP_DISABLE_PLAYER_APPLICATION_SETTING_CMDS,

  // Disable profile connection for headsets/car-kits
  // Some car kits going bad state when DUT initiate profile connection in collision scenerio
  // Hence don't initaite profile level connections and wait for incoming connetcion
  INTEROP_DISABLE_CONNECTION_AFTER_COLLISION,

  // Some LE devices have proven problematic behaviour if LE connection update is initiated with
  // them, resulting in no response after initiating LE connection update and ultimately resulting
  // in connection timeout.
  // To avoid degrading the user experience with those devices, LE connection update
  // is not requested explictly for those devices.
  INTEROP_DISABLE_LE_CONN_UPDATES,

  // Devices requiring this workaround do not handle Bluetooth PBAP 1.1 version correctly,
  // leading them to go in bad state. So for better interoperability respond with PBAP 1.2
  // as supported version.
  INTEROP_ADV_PBAP_VER_1_2,

  //Few remote sends avrcp browsing request just after host sends avrcp browsing request leading
  //collision which results in browsing channel open failure. For such devices, collision timer
  //is used avoid collision.
  INTEROP_AVRCP_BROWSE_OPEN_CHANNEL_COLLISION,

  //Some remote devices don't support sniff mode when the SCO is connected.
  //For such devices, disable sniff mode after SCO is connected and make
  //the link as active.
  INTEROP_DISABLE_SNIFF_LINK_DURING_SCO,

  // Enable power level 10 adaptive control in FW side for remote devices
  // in this whitelist item.
  INTEROP_ENABLE_PL10_ADAPTIVE_CONTROL,

  //For some remote devicea, disable sniff mode during the call
  INTEROP_DISABLE_SNIFF_DURING_CALL,

  // Set a very low initial sniff subrating for HID devices that do not
  // set their own sniff interval.
  INTEROP_HID_HOST_LIMIT_SNIFF_INTERVAL,

  // Disable LPA enhanced power control
  INTEROP_DISABLE_LPA_ENHANCED_POWER_CONTROL,

  END_OF_INTEROP_LIST
} interop_feature_t;

// Check if a given |addr| matches a known interoperability workaround as
// identified by the |interop_feature_t| enum. This API is used for simple
// address based lookups where more information is not available. No
// look-ups or random address resolution are performed on |addr|.
bool interop_match_addr(const interop_feature_t feature,
                        const RawAddress* addr);

// Check if a given remote device |name| matches a known workaround.
// Name comparisons are case sensitive and do not allow for partial matches.
// If |name| is "TEST" and a workaround exists for "TESTING", then this
// function will return false. But, if |name| is "TESTING" and a workaround
// exists for "TEST", this function will return true. |name| cannot be null and
// must be null terminated.
bool interop_match_name(const interop_feature_t feature, const char* name);

// This api will lookup remote name with |addr| by btif_storage api internally.
// Then if either interop_match_addr or interop_match_name is matched, this function
// will return true.
bool interop_match_addr_or_name(const interop_feature_t feature, const RawAddress* addr);

// Check if a given |manufacturer| matches a known interoperability workaround
// as identified by the |interop_feature_t| enum. This API is used for
// manufacturer based lookups where more information is not available.
bool interop_match_manufacturer(const interop_feature_t feature,
                                uint16_t manufacturer);

// Check if a given |vendor_id, |product_id| matches a known
// interoperability workaround as identified by the |interop_feature_t|
// enum. This API is used for simple name based lookups where more information
// is not available.
bool interop_match_vendor_product_ids(const interop_feature_t feature,
                                      uint16_t vendor_id, uint16_t product_id);

// Add a dynamic interop database entry for a device matching the first |length|
// bytes of |addr|, implementing the workaround identified by |feature|.
// |addr| may not be null.
// |length| must be greater than 0 and less than RawAddress::kLength.
// As |interop_feature_t| is not exposed in the public API, feature must be a
// valid integer representing an option in the enum.
void interop_database_add(const uint16_t feature, const RawAddress* addr,
                          size_t length);

// Clear the dynamic portion of the interoperability workaround database.
void interop_database_clear(void);

//check if device version is matching with the interop database
bool interop_database_match_version(const interop_feature_t feature, uint16_t version);
// Check if a given |addr| matches a known interoperability workaround as identified
// by the |interop_feature_t| enum. This API is used for simple address
// based lookups where more information is not available. No look-ups or random
// address resolution are performed on |addr|. If address is matched,
// max latency for SSR stored for particular remote device is returned.
bool interop_match_addr_get_max_lat(const interop_feature_t feature,
        const RawAddress* addr, uint16_t *max_lat);
