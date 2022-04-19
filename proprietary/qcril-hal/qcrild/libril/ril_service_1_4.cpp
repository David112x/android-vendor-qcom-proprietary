/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*
* Not a Contribution.
*/
/*
 * Copyright (c) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#define LOG_TAG "RILC"
#define TAG "RILQ_ril_1_4"

#include <memory>
#include <android/hardware/radio/1.4/IRadio.h>
#include <android/hardware/radio/1.4/IRadioResponse.h>
#include <android/hardware/radio/1.4/IRadioIndication.h>
#include <android/hardware/radio/1.4/types.h>

#include <ril_service.h>
#include <ril_service_1_4.h>
#include "RilServiceModule_1_4.h"
#ifndef QMI_RIL_UTF
#include <hidl/HidlTransportSupport.h>
#endif
#include <log/log.h>
#include <utils/SystemClock.h>
#include <inttypes.h>
#include <framework/Log.h>
#include <framework/Module.h>
#include <framework/QcrilInitMessage.h>
#include <framework/UnSolicitedMessage.h>
#include <request/LinkCapIndMessage.h>
#include <modules/nas/NasModule.h>
#include "interfaces/nas/nas_types.h"
#include "SetRadioClientCapUnSolMessage.h"
#include <request/SetupDataCallRequestMessage.h>
#include <request/DeactivateDataCallRequestMessage.h>
#include <request/GetRadioDataCallListRequestMessage.h>
#include <request/SetDataProfileRequestMessage.h>
#include <request/SetInitialAttachApnRequestMessage.h>
#include <request/StartKeepAliveRequestMessage.h>
#include <request/StopKeepAliveRequestMessage.h>

#include "interfaces/nas/RilUnsolEmergencyListIndMessage.h"

#include <interfaces/voice/QcRilRequestCsDialMessage.h>

using rildata::LinkCapIndMessage;
using rildata::LinkCapEstimate_t;

#define INVALID_HEX_CHAR 16

using namespace android::hardware::radio;
using namespace android::hardware::radio::V1_0;
using namespace android::hardware::radio::deprecated::V1_0;
#ifndef QMI_RIL_UTF
using ::android::hardware::configureRpcThreadpool;
using ::android::hardware::joinRpcThreadpool;
#endif
using ::android::hardware::Return;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_array;
using ::android::hardware::Void;
using android::CommandInfo;
using android::RequestInfo;
using android::requestToString;
using android::sp;
using RadioContext = RadioImpl_1_4::RadioContext;

extern bool dispatchStrings(int serial, int slotId, int request, bool allowEmpty, int countStrings, ...);

sp<RadioImpl_1_4> radioImpl_1_4;

unordered_map<DataCallFailCause_t, V1_4::DataCallFailCause> dcFailCauseMap = {
    {DataCallFailCause_t::NONE, V1_4::DataCallFailCause::NONE},
    {DataCallFailCause_t::OPERATOR_BARRED, V1_4::DataCallFailCause::OPERATOR_BARRED},
    {DataCallFailCause_t::NAS_SIGNALLING, V1_4::DataCallFailCause::NAS_SIGNALLING},
    {DataCallFailCause_t::LLC_SNDCP, V1_4::DataCallFailCause::LLC_SNDCP},
    {DataCallFailCause_t::INSUFFICIENT_RESOURCES, V1_4::DataCallFailCause::INSUFFICIENT_RESOURCES},
    {DataCallFailCause_t::MISSING_UKNOWN_APN, V1_4::DataCallFailCause::MISSING_UKNOWN_APN},
    {DataCallFailCause_t::UNKNOWN_PDP_ADDRESS_TYPE, V1_4::DataCallFailCause::UNKNOWN_PDP_ADDRESS_TYPE},
    {DataCallFailCause_t::USER_AUTHENTICATION, V1_4::DataCallFailCause::USER_AUTHENTICATION},
    {DataCallFailCause_t::ACTIVATION_REJECT_GGSN, V1_4::DataCallFailCause::ACTIVATION_REJECT_GGSN},
    {DataCallFailCause_t::ACTIVATION_REJECT_UNSPECIFIED, V1_4::DataCallFailCause::ACTIVATION_REJECT_UNSPECIFIED},
    {DataCallFailCause_t::SERVICE_OPTION_NOT_SUPPORTED, V1_4::DataCallFailCause::SERVICE_OPTION_NOT_SUPPORTED},
    {DataCallFailCause_t::SERVICE_OPTION_NOT_SUBSCRIBED, V1_4::DataCallFailCause::SERVICE_OPTION_NOT_SUBSCRIBED},
    {DataCallFailCause_t::SERVICE_OPTION_OUT_OF_ORDER, V1_4::DataCallFailCause::SERVICE_OPTION_OUT_OF_ORDER},
    {DataCallFailCause_t::NSAPI_IN_USE, V1_4::DataCallFailCause::NSAPI_IN_USE},
    {DataCallFailCause_t::REGULAR_DEACTIVATION, V1_4::DataCallFailCause::REGULAR_DEACTIVATION},
    {DataCallFailCause_t::QOS_NOT_ACCEPTED, V1_4::DataCallFailCause::QOS_NOT_ACCEPTED},
    {DataCallFailCause_t::NETWORK_FAILURE, V1_4::DataCallFailCause::NETWORK_FAILURE},
    {DataCallFailCause_t::UMTS_REACTIVATION_REQ, V1_4::DataCallFailCause::UMTS_REACTIVATION_REQ},
    {DataCallFailCause_t::FEATURE_NOT_SUPP, V1_4::DataCallFailCause::FEATURE_NOT_SUPP},
    {DataCallFailCause_t::TFT_SEMANTIC_ERROR, V1_4::DataCallFailCause::TFT_SEMANTIC_ERROR},
    {DataCallFailCause_t::TFT_SYTAX_ERROR, V1_4::DataCallFailCause::TFT_SYTAX_ERROR},
    {DataCallFailCause_t::UNKNOWN_PDP_CONTEXT, V1_4::DataCallFailCause::UNKNOWN_PDP_CONTEXT},
    {DataCallFailCause_t::FILTER_SEMANTIC_ERROR, V1_4::DataCallFailCause::FILTER_SEMANTIC_ERROR},
    {DataCallFailCause_t::FILTER_SYTAX_ERROR, V1_4::DataCallFailCause::FILTER_SYTAX_ERROR},
    {DataCallFailCause_t::PDP_WITHOUT_ACTIVE_TFT, V1_4::DataCallFailCause::PDP_WITHOUT_ACTIVE_TFT},
    {DataCallFailCause_t::ACTIVATION_REJECTED_BCM_VIOLATION, V1_4::DataCallFailCause::ACTIVATION_REJECTED_BCM_VIOLATION},
    {DataCallFailCause_t::ONLY_IPV4_ALLOWED, V1_4::DataCallFailCause::ONLY_IPV4_ALLOWED},
    {DataCallFailCause_t::ONLY_IPV6_ALLOWED, V1_4::DataCallFailCause::ONLY_IPV6_ALLOWED},
    {DataCallFailCause_t::ONLY_SINGLE_BEARER_ALLOWED, V1_4::DataCallFailCause::ONLY_SINGLE_BEARER_ALLOWED},
    {DataCallFailCause_t::ESM_INFO_NOT_RECEIVED, V1_4::DataCallFailCause::ESM_INFO_NOT_RECEIVED},
    {DataCallFailCause_t::PDN_CONN_DOES_NOT_EXIST, V1_4::DataCallFailCause::PDN_CONN_DOES_NOT_EXIST},
    {DataCallFailCause_t::MULTI_CONN_TO_SAME_PDN_NOT_ALLOWED, V1_4::DataCallFailCause::MULTI_CONN_TO_SAME_PDN_NOT_ALLOWED},
    {DataCallFailCause_t::COLLISION_WITH_NETWORK_INITIATED_REQUEST, V1_4::DataCallFailCause::COLLISION_WITH_NETWORK_INITIATED_REQUEST},
    {DataCallFailCause_t::ONLY_IPV4V6_ALLOWED, V1_4::DataCallFailCause::ONLY_IPV4V6_ALLOWED},
    {DataCallFailCause_t::ONLY_NON_IP_ALLOWED, V1_4::DataCallFailCause::ONLY_NON_IP_ALLOWED},
    {DataCallFailCause_t::UNSUPPORTED_QCI_VALUE, V1_4::DataCallFailCause::UNSUPPORTED_QCI_VALUE},
    {DataCallFailCause_t::BEARER_HANDLING_NOT_SUPPORTED, V1_4::DataCallFailCause::BEARER_HANDLING_NOT_SUPPORTED},
    {DataCallFailCause_t::MAX_ACTIVE_PDP_CONTEXT_REACHED, V1_4::DataCallFailCause::MAX_ACTIVE_PDP_CONTEXT_REACHED},
    {DataCallFailCause_t::UNSUPPORTED_APN_IN_CURRENT_PLMN, V1_4::DataCallFailCause::UNSUPPORTED_APN_IN_CURRENT_PLMN},
    {DataCallFailCause_t::INVALID_TRANSACTION_ID, V1_4::DataCallFailCause::INVALID_TRANSACTION_ID},
    {DataCallFailCause_t::MESSAGE_INCORRECT_SEMANTIC, V1_4::DataCallFailCause::MESSAGE_INCORRECT_SEMANTIC},
    {DataCallFailCause_t::INVALID_MANDATORY_INFO, V1_4::DataCallFailCause::INVALID_MANDATORY_INFO},
    {DataCallFailCause_t::MESSAGE_TYPE_UNSUPPORTED, V1_4::DataCallFailCause::MESSAGE_TYPE_UNSUPPORTED},
    {DataCallFailCause_t::MSG_TYPE_NONCOMPATIBLE_STATE, V1_4::DataCallFailCause::MSG_TYPE_NONCOMPATIBLE_STATE},
    {DataCallFailCause_t::UNKNOWN_INFO_ELEMENT, V1_4::DataCallFailCause::UNKNOWN_INFO_ELEMENT},
    {DataCallFailCause_t::CONDITIONAL_IE_ERROR, V1_4::DataCallFailCause::CONDITIONAL_IE_ERROR},
    {DataCallFailCause_t::MSG_AND_PROTOCOL_STATE_UNCOMPATIBLE, V1_4::DataCallFailCause::MSG_AND_PROTOCOL_STATE_UNCOMPATIBLE},
    {DataCallFailCause_t::PROTOCOL_ERRORS, V1_4::DataCallFailCause::PROTOCOL_ERRORS},
    {DataCallFailCause_t::APN_TYPE_CONFLICT, V1_4::DataCallFailCause::APN_TYPE_CONFLICT},
    {DataCallFailCause_t::INVALID_PCSCF_ADDR, V1_4::DataCallFailCause::INVALID_PCSCF_ADDR},
    {DataCallFailCause_t::INTERNAL_CALL_PREEMPT_BY_HIGH_PRIO_APN, V1_4::DataCallFailCause::INTERNAL_CALL_PREEMPT_BY_HIGH_PRIO_APN},
    {DataCallFailCause_t::EMM_ACCESS_BARRED, V1_4::DataCallFailCause::EMM_ACCESS_BARRED},
    {DataCallFailCause_t::EMERGENCY_IFACE_ONLY, V1_4::DataCallFailCause::EMERGENCY_IFACE_ONLY},
    {DataCallFailCause_t::IFACE_MISMATCH, V1_4::DataCallFailCause::IFACE_MISMATCH},
    {DataCallFailCause_t::COMPANION_IFACE_IN_USE, V1_4::DataCallFailCause::COMPANION_IFACE_IN_USE},
    {DataCallFailCause_t::IP_ADDRESS_MISMATCH, V1_4::DataCallFailCause::IP_ADDRESS_MISMATCH},
    {DataCallFailCause_t::IFACE_AND_POL_FAMILY_MISMATCH, V1_4::DataCallFailCause::IFACE_AND_POL_FAMILY_MISMATCH},
    {DataCallFailCause_t::EMM_ACCESS_BARRED_INFINITE_RETRY, V1_4::DataCallFailCause::EMM_ACCESS_BARRED_INFINITE_RETRY},
    {DataCallFailCause_t::AUTH_FAILURE_ON_EMERGENCY_CALL, V1_4::DataCallFailCause::AUTH_FAILURE_ON_EMERGENCY_CALL},
    {DataCallFailCause_t::INVALID_DNS_ADDR, V1_4::DataCallFailCause::INVALID_DNS_ADDR},
    {DataCallFailCause_t::MIP_FA_REASON_UNSPECIFIED, V1_4::DataCallFailCause::MIP_FA_REASON_UNSPECIFIED},
    {DataCallFailCause_t::MIP_FA_ADMIN_PROHIBITED, V1_4::DataCallFailCause::MIP_FA_ADMIN_PROHIBITED},
    {DataCallFailCause_t::MIP_FA_INSUFFICIENT_RESOURCES, V1_4::DataCallFailCause::MIP_FA_INSUFFICIENT_RESOURCES},
    {DataCallFailCause_t::MIP_FA_MOBILE_NODE_AUTHENTICATION_FAILURE, V1_4::DataCallFailCause::MIP_FA_MOBILE_NODE_AUTHENTICATION_FAILURE},
    {DataCallFailCause_t::MIP_FA_HOME_AGENT_AUTHENTICATION_FAILURE, V1_4::DataCallFailCause::MIP_FA_HOME_AGENT_AUTHENTICATION_FAILURE},
    {DataCallFailCause_t::MIP_FA_REQUESTED_LIFETIME_TOO_LONG, V1_4::DataCallFailCause::MIP_FA_REQUESTED_LIFETIME_TOO_LONG},
    {DataCallFailCause_t::MIP_FA_MALFORMED_REQUEST, V1_4::DataCallFailCause::MIP_FA_MALFORMED_REQUEST},
    {DataCallFailCause_t::MIP_FA_MALFORMED_REPLY, V1_4::DataCallFailCause::MIP_FA_MALFORMED_REPLY},
    {DataCallFailCause_t::MIP_FA_ENCAPSULATION_UNAVAILABLE, V1_4::DataCallFailCause::MIP_FA_ENCAPSULATION_UNAVAILABLE},
    {DataCallFailCause_t::MIP_FA_VJ_HEADER_COMPRESSION_UNAVAILABLE, V1_4::DataCallFailCause::MIP_FA_VJ_HEADER_COMPRESSION_UNAVAILABLE},
    {DataCallFailCause_t::MIP_FA_REVERSE_TUNNEL_UNAVAILABLE, V1_4::DataCallFailCause::MIP_FA_REVERSE_TUNNEL_UNAVAILABLE},
    {DataCallFailCause_t::MIP_FA_REVERSE_TUNNEL_IS_MANDATORY, V1_4::DataCallFailCause::MIP_FA_REVERSE_TUNNEL_IS_MANDATORY},
    {DataCallFailCause_t::MIP_FA_DELIVERY_STYLE_NOT_SUPPORTED, V1_4::DataCallFailCause::MIP_FA_DELIVERY_STYLE_NOT_SUPPORTED},
    {DataCallFailCause_t::MIP_FA_MISSING_NAI, V1_4::DataCallFailCause::MIP_FA_MISSING_NAI},
    {DataCallFailCause_t::MIP_FA_MISSING_HOME_AGENT, V1_4::DataCallFailCause::MIP_FA_MISSING_HOME_AGENT},
    {DataCallFailCause_t::MIP_FA_MISSING_HOME_ADDRESS, V1_4::DataCallFailCause::MIP_FA_MISSING_HOME_ADDRESS},
    {DataCallFailCause_t::MIP_FA_UNKNOWN_CHALLENGE, V1_4::DataCallFailCause::MIP_FA_UNKNOWN_CHALLENGE},
    {DataCallFailCause_t::MIP_FA_MISSING_CHALLENGE, V1_4::DataCallFailCause::MIP_FA_MISSING_CHALLENGE},
    {DataCallFailCause_t::MIP_FA_STALE_CHALLENGE, V1_4::DataCallFailCause::MIP_FA_STALE_CHALLENGE},
    {DataCallFailCause_t::MIP_HA_REASON_UNSPECIFIED, V1_4::DataCallFailCause::MIP_HA_REASON_UNSPECIFIED},
    {DataCallFailCause_t::MIP_HA_ADMIN_PROHIBITED, V1_4::DataCallFailCause::MIP_HA_ADMIN_PROHIBITED},
    {DataCallFailCause_t::MIP_HA_INSUFFICIENT_RESOURCES, V1_4::DataCallFailCause::MIP_HA_INSUFFICIENT_RESOURCES},
    {DataCallFailCause_t::MIP_HA_MOBILE_NODE_AUTHENTICATION_FAILURE, V1_4::DataCallFailCause::MIP_HA_MOBILE_NODE_AUTHENTICATION_FAILURE},
    {DataCallFailCause_t::MIP_HA_FOREIGN_AGENT_AUTHENTICATION_FAILURE, V1_4::DataCallFailCause::MIP_HA_FOREIGN_AGENT_AUTHENTICATION_FAILURE},
    {DataCallFailCause_t::MIP_HA_REGISTRATION_ID_MISMATCH, V1_4::DataCallFailCause::MIP_HA_REGISTRATION_ID_MISMATCH},
    {DataCallFailCause_t::MIP_HA_MALFORMED_REQUEST, V1_4::DataCallFailCause::MIP_HA_MALFORMED_REQUEST},
    {DataCallFailCause_t::MIP_HA_UNKNOWN_HOME_AGENT_ADDRESS, V1_4::DataCallFailCause::MIP_HA_UNKNOWN_HOME_AGENT_ADDRESS},
    {DataCallFailCause_t::MIP_HA_REVERSE_TUNNEL_UNAVAILABLE, V1_4::DataCallFailCause::MIP_HA_REVERSE_TUNNEL_UNAVAILABLE},
    {DataCallFailCause_t::MIP_HA_REVERSE_TUNNEL_IS_MANDATORY, V1_4::DataCallFailCause::MIP_HA_REVERSE_TUNNEL_IS_MANDATORY},
    {DataCallFailCause_t::MIP_HA_ENCAPSULATION_UNAVAILABLE, V1_4::DataCallFailCause::MIP_HA_ENCAPSULATION_UNAVAILABLE},
    {DataCallFailCause_t::CLOSE_IN_PROGRESS, V1_4::DataCallFailCause::CLOSE_IN_PROGRESS},
    {DataCallFailCause_t::NETWORK_INITIATED_TERMINATION, V1_4::DataCallFailCause::NETWORK_INITIATED_TERMINATION},
    {DataCallFailCause_t::MODEM_APP_PREEMPTED, V1_4::DataCallFailCause::MODEM_APP_PREEMPTED},
    {DataCallFailCause_t::PDN_IPV4_CALL_DISALLOWED, V1_4::DataCallFailCause::PDN_IPV4_CALL_DISALLOWED},
    {DataCallFailCause_t::PDN_IPV4_CALL_THROTTLED, V1_4::DataCallFailCause::PDN_IPV4_CALL_THROTTLED},
    {DataCallFailCause_t::PDN_IPV6_CALL_DISALLOWED, V1_4::DataCallFailCause::PDN_IPV6_CALL_DISALLOWED},
    {DataCallFailCause_t::PDN_IPV6_CALL_THROTTLED, V1_4::DataCallFailCause::PDN_IPV6_CALL_THROTTLED},
    {DataCallFailCause_t::MODEM_RESTART, V1_4::DataCallFailCause::MODEM_RESTART},
    {DataCallFailCause_t::PDP_PPP_NOT_SUPPORTED, V1_4::DataCallFailCause::PDP_PPP_NOT_SUPPORTED},
    {DataCallFailCause_t::UNPREFERRED_RAT, V1_4::DataCallFailCause::UNPREFERRED_RAT},
    {DataCallFailCause_t::PHYSICAL_LINK_CLOSE_IN_PROGRESS, V1_4::DataCallFailCause::PHYSICAL_LINK_CLOSE_IN_PROGRESS},
    {DataCallFailCause_t::APN_PENDING_HANDOVER, V1_4::DataCallFailCause::APN_PENDING_HANDOVER},
    {DataCallFailCause_t::PROFILE_BEARER_INCOMPATIBLE, V1_4::DataCallFailCause::PROFILE_BEARER_INCOMPATIBLE},
    {DataCallFailCause_t::SIM_CARD_CHANGED, V1_4::DataCallFailCause::SIM_CARD_CHANGED},
    {DataCallFailCause_t::LOW_POWER_MODE_OR_POWERING_DOWN, V1_4::DataCallFailCause::LOW_POWER_MODE_OR_POWERING_DOWN},
    {DataCallFailCause_t::APN_DISABLED, V1_4::DataCallFailCause::APN_DISABLED},
    {DataCallFailCause_t::MAX_PPP_INACTIVITY_TIMER_EXPIRED, V1_4::DataCallFailCause::MAX_PPP_INACTIVITY_TIMER_EXPIRED},
    {DataCallFailCause_t::IPV6_ADDRESS_TRANSFER_FAILED, V1_4::DataCallFailCause::IPV6_ADDRESS_TRANSFER_FAILED},
    {DataCallFailCause_t::TRAT_SWAP_FAILED, V1_4::DataCallFailCause::TRAT_SWAP_FAILED},
    {DataCallFailCause_t::EHRPD_TO_HRPD_FALLBACK, V1_4::DataCallFailCause::EHRPD_TO_HRPD_FALLBACK},
    {DataCallFailCause_t::MIP_CONFIG_FAILURE, V1_4::DataCallFailCause::MIP_CONFIG_FAILURE},
    {DataCallFailCause_t::PDN_INACTIVITY_TIMER_EXPIRED, V1_4::DataCallFailCause::PDN_INACTIVITY_TIMER_EXPIRED},
    {DataCallFailCause_t::MAX_IPV4_CONNECTIONS, V1_4::DataCallFailCause::MAX_IPV4_CONNECTIONS},
    {DataCallFailCause_t::MAX_IPV6_CONNECTIONS, V1_4::DataCallFailCause::MAX_IPV6_CONNECTIONS},
    {DataCallFailCause_t::APN_MISMATCH, V1_4::DataCallFailCause::APN_MISMATCH},
    {DataCallFailCause_t::IP_VERSION_MISMATCH, V1_4::DataCallFailCause::IP_VERSION_MISMATCH},
    {DataCallFailCause_t::DUN_CALL_DISALLOWED, V1_4::DataCallFailCause::DUN_CALL_DISALLOWED},
    {DataCallFailCause_t::INTERNAL_EPC_NONEPC_TRANSITION, V1_4::DataCallFailCause::INTERNAL_EPC_NONEPC_TRANSITION},
    {DataCallFailCause_t::INTERFACE_IN_USE, V1_4::DataCallFailCause::INTERFACE_IN_USE},
    {DataCallFailCause_t::APN_DISALLOWED_ON_ROAMING, V1_4::DataCallFailCause::APN_DISALLOWED_ON_ROAMING},
    {DataCallFailCause_t::APN_PARAMETERS_CHANGED, V1_4::DataCallFailCause::APN_PARAMETERS_CHANGED},
    {DataCallFailCause_t::NULL_APN_DISALLOWED, V1_4::DataCallFailCause::NULL_APN_DISALLOWED},
    {DataCallFailCause_t::THERMAL_MITIGATION, V1_4::DataCallFailCause::THERMAL_MITIGATION},
    {DataCallFailCause_t::DATA_SETTINGS_DISABLED, V1_4::DataCallFailCause::DATA_SETTINGS_DISABLED},
    {DataCallFailCause_t::DATA_ROAMING_SETTINGS_DISABLED, V1_4::DataCallFailCause::DATA_ROAMING_SETTINGS_DISABLED},
    {DataCallFailCause_t::DDS_SWITCHED, V1_4::DataCallFailCause::DDS_SWITCHED},
    {DataCallFailCause_t::FORBIDDEN_APN_NAME, V1_4::DataCallFailCause::FORBIDDEN_APN_NAME},
    {DataCallFailCause_t::DDS_SWITCH_IN_PROGRESS, V1_4::DataCallFailCause::DDS_SWITCH_IN_PROGRESS},
    {DataCallFailCause_t::CALL_DISALLOWED_IN_ROAMING, V1_4::DataCallFailCause::CALL_DISALLOWED_IN_ROAMING},
    {DataCallFailCause_t::NON_IP_NOT_SUPPORTED, V1_4::DataCallFailCause::NON_IP_NOT_SUPPORTED},
    {DataCallFailCause_t::PDN_NON_IP_CALL_THROTTLED, V1_4::DataCallFailCause::PDN_NON_IP_CALL_THROTTLED},
    {DataCallFailCause_t::PDN_NON_IP_CALL_DISALLOWED, V1_4::DataCallFailCause::PDN_NON_IP_CALL_DISALLOWED},
    {DataCallFailCause_t::CDMA_LOCK, V1_4::DataCallFailCause::CDMA_LOCK},
    {DataCallFailCause_t::CDMA_INTERCEPT, V1_4::DataCallFailCause::CDMA_INTERCEPT},
    {DataCallFailCause_t::CDMA_REORDER, V1_4::DataCallFailCause::CDMA_REORDER},
    {DataCallFailCause_t::CDMA_RELEASE_DUE_TO_SO_REJECTION, V1_4::DataCallFailCause::CDMA_RELEASE_DUE_TO_SO_REJECTION},
    {DataCallFailCause_t::CDMA_INCOMING_CALL, V1_4::DataCallFailCause::CDMA_INCOMING_CALL},
    {DataCallFailCause_t::CDMA_ALERT_STOP, V1_4::DataCallFailCause::CDMA_ALERT_STOP},
    {DataCallFailCause_t::CHANNEL_ACQUISITION_FAILURE, V1_4::DataCallFailCause::CHANNEL_ACQUISITION_FAILURE},
    {DataCallFailCause_t::MAX_ACCESS_PROBE, V1_4::DataCallFailCause::MAX_ACCESS_PROBE},
    {DataCallFailCause_t::CONCURRENT_SERVICE_NOT_SUPPORTED_BY_BASE_STATION, V1_4::DataCallFailCause::CONCURRENT_SERVICE_NOT_SUPPORTED_BY_BASE_STATION},
    {DataCallFailCause_t::NO_RESPONSE_FROM_BASE_STATION, V1_4::DataCallFailCause::NO_RESPONSE_FROM_BASE_STATION},
    {DataCallFailCause_t::REJECTED_BY_BASE_STATION, V1_4::DataCallFailCause::REJECTED_BY_BASE_STATION},
    {DataCallFailCause_t::CONCURRENT_SERVICES_INCOMPATIBLE, V1_4::DataCallFailCause::CONCURRENT_SERVICES_INCOMPATIBLE},
    {DataCallFailCause_t::NO_CDMA_SERVICE, V1_4::DataCallFailCause::NO_CDMA_SERVICE},
    {DataCallFailCause_t::RUIM_NOT_PRESENT, V1_4::DataCallFailCause::RUIM_NOT_PRESENT},
    {DataCallFailCause_t::CDMA_RETRY_ORDER, V1_4::DataCallFailCause::CDMA_RETRY_ORDER},
    {DataCallFailCause_t::ACCESS_BLOCK, V1_4::DataCallFailCause::ACCESS_BLOCK},
    {DataCallFailCause_t::ACCESS_BLOCK_ALL, V1_4::DataCallFailCause::ACCESS_BLOCK_ALL},
    {DataCallFailCause_t::IS707B_MAX_ACCESS_PROBES, V1_4::DataCallFailCause::IS707B_MAX_ACCESS_PROBES},
    {DataCallFailCause_t::THERMAL_EMERGENCY, V1_4::DataCallFailCause::THERMAL_EMERGENCY},
    {DataCallFailCause_t::CONCURRENT_SERVICES_NOT_ALLOWED, V1_4::DataCallFailCause::CONCURRENT_SERVICES_NOT_ALLOWED},
    {DataCallFailCause_t::INCOMING_CALL_REJECTED, V1_4::DataCallFailCause::INCOMING_CALL_REJECTED},
    {DataCallFailCause_t::NO_SERVICE_ON_GATEWAY, V1_4::DataCallFailCause::NO_SERVICE_ON_GATEWAY},
    {DataCallFailCause_t::NO_GPRS_CONTEXT, V1_4::DataCallFailCause::NO_GPRS_CONTEXT},
    {DataCallFailCause_t::ILLEGAL_MS, V1_4::DataCallFailCause::ILLEGAL_MS},
    {DataCallFailCause_t::ILLEGAL_ME, V1_4::DataCallFailCause::ILLEGAL_ME},
    {DataCallFailCause_t::GPRS_SERVICES_AND_NON_GPRS_SERVICES_NOT_ALLOWED, V1_4::DataCallFailCause::GPRS_SERVICES_AND_NON_GPRS_SERVICES_NOT_ALLOWED},
    {DataCallFailCause_t::GPRS_SERVICES_NOT_ALLOWED, V1_4::DataCallFailCause::GPRS_SERVICES_NOT_ALLOWED},
    {DataCallFailCause_t::MS_IDENTITY_CANNOT_BE_DERIVED_BY_THE_NETWORK, V1_4::DataCallFailCause::MS_IDENTITY_CANNOT_BE_DERIVED_BY_THE_NETWORK},
    {DataCallFailCause_t::IMPLICITLY_DETACHED, V1_4::DataCallFailCause::IMPLICITLY_DETACHED},
    {DataCallFailCause_t::PLMN_NOT_ALLOWED, V1_4::DataCallFailCause::PLMN_NOT_ALLOWED},
    {DataCallFailCause_t::LOCATION_AREA_NOT_ALLOWED, V1_4::DataCallFailCause::LOCATION_AREA_NOT_ALLOWED},
    {DataCallFailCause_t::GPRS_SERVICES_NOT_ALLOWED_IN_THIS_PLMN, V1_4::DataCallFailCause::GPRS_SERVICES_NOT_ALLOWED_IN_THIS_PLMN},
    {DataCallFailCause_t::PDP_DUPLICATE, V1_4::DataCallFailCause::PDP_DUPLICATE},
    {DataCallFailCause_t::UE_RAT_CHANGE, V1_4::DataCallFailCause::UE_RAT_CHANGE},
    {DataCallFailCause_t::CONGESTION, V1_4::DataCallFailCause::CONGESTION},
    {DataCallFailCause_t::NO_PDP_CONTEXT_ACTIVATED, V1_4::DataCallFailCause::NO_PDP_CONTEXT_ACTIVATED},
    {DataCallFailCause_t::ACCESS_CLASS_DSAC_REJECTION, V1_4::DataCallFailCause::ACCESS_CLASS_DSAC_REJECTION},
    {DataCallFailCause_t::PDP_ACTIVATE_MAX_RETRY_FAILED, V1_4::DataCallFailCause::PDP_ACTIVATE_MAX_RETRY_FAILED},
    {DataCallFailCause_t::RADIO_ACCESS_BEARER_FAILURE, V1_4::DataCallFailCause::RADIO_ACCESS_BEARER_FAILURE},
    {DataCallFailCause_t::ESM_UNKNOWN_EPS_BEARER_CONTEXT, V1_4::DataCallFailCause::ESM_UNKNOWN_EPS_BEARER_CONTEXT},
    {DataCallFailCause_t::DRB_RELEASED_BY_RRC, V1_4::DataCallFailCause::DRB_RELEASED_BY_RRC},
    {DataCallFailCause_t::CONNECTION_RELEASED, V1_4::DataCallFailCause::CONNECTION_RELEASED},
    {DataCallFailCause_t::EMM_DETACHED, V1_4::DataCallFailCause::EMM_DETACHED},
    {DataCallFailCause_t::EMM_ATTACH_FAILED, V1_4::DataCallFailCause::EMM_ATTACH_FAILED},
    {DataCallFailCause_t::EMM_ATTACH_STARTED, V1_4::DataCallFailCause::EMM_ATTACH_STARTED},
    {DataCallFailCause_t::LTE_NAS_SERVICE_REQUEST_FAILED, V1_4::DataCallFailCause::LTE_NAS_SERVICE_REQUEST_FAILED},
    {DataCallFailCause_t::DUPLICATE_BEARER_ID, V1_4::DataCallFailCause::DUPLICATE_BEARER_ID},
    {DataCallFailCause_t::ESM_COLLISION_SCENARIOS, V1_4::DataCallFailCause::ESM_COLLISION_SCENARIOS},
    {DataCallFailCause_t::ESM_BEARER_DEACTIVATED_TO_SYNC_WITH_NETWORK, V1_4::DataCallFailCause::ESM_BEARER_DEACTIVATED_TO_SYNC_WITH_NETWORK},
    {DataCallFailCause_t::ESM_NW_ACTIVATED_DED_BEARER_WITH_ID_OF_DEF_BEARER, V1_4::DataCallFailCause::ESM_NW_ACTIVATED_DED_BEARER_WITH_ID_OF_DEF_BEARER},
    {DataCallFailCause_t::ESM_BAD_OTA_MESSAGE, V1_4::DataCallFailCause::ESM_BAD_OTA_MESSAGE},
    {DataCallFailCause_t::ESM_DOWNLOAD_SERVER_REJECTED_THE_CALL, V1_4::DataCallFailCause::ESM_DOWNLOAD_SERVER_REJECTED_THE_CALL},
    {DataCallFailCause_t::ESM_CONTEXT_TRANSFERRED_DUE_TO_IRAT, V1_4::DataCallFailCause::ESM_CONTEXT_TRANSFERRED_DUE_TO_IRAT},
    {DataCallFailCause_t::DS_EXPLICIT_DEACTIVATION, V1_4::DataCallFailCause::DS_EXPLICIT_DEACTIVATION},
    {DataCallFailCause_t::ESM_LOCAL_CAUSE_NONE, V1_4::DataCallFailCause::ESM_LOCAL_CAUSE_NONE},
    {DataCallFailCause_t::LTE_THROTTLING_NOT_REQUIRED, V1_4::DataCallFailCause::LTE_THROTTLING_NOT_REQUIRED},
    {DataCallFailCause_t::ACCESS_CONTROL_LIST_CHECK_FAILURE, V1_4::DataCallFailCause::ACCESS_CONTROL_LIST_CHECK_FAILURE},
    {DataCallFailCause_t::SERVICE_NOT_ALLOWED_ON_PLMN, V1_4::DataCallFailCause::SERVICE_NOT_ALLOWED_ON_PLMN},
    {DataCallFailCause_t::EMM_T3417_EXPIRED, V1_4::DataCallFailCause::EMM_T3417_EXPIRED},
    {DataCallFailCause_t::EMM_T3417_EXT_EXPIRED, V1_4::DataCallFailCause::EMM_T3417_EXT_EXPIRED},
    {DataCallFailCause_t::RRC_UPLINK_DATA_TRANSMISSION_FAILURE, V1_4::DataCallFailCause::RRC_UPLINK_DATA_TRANSMISSION_FAILURE},
    {DataCallFailCause_t::RRC_UPLINK_DELIVERY_FAILED_DUE_TO_HANDOVER, V1_4::DataCallFailCause::RRC_UPLINK_DELIVERY_FAILED_DUE_TO_HANDOVER},
    {DataCallFailCause_t::RRC_UPLINK_CONNECTION_RELEASE, V1_4::DataCallFailCause::RRC_UPLINK_CONNECTION_RELEASE},
    {DataCallFailCause_t::RRC_UPLINK_RADIO_LINK_FAILURE, V1_4::DataCallFailCause::RRC_UPLINK_RADIO_LINK_FAILURE},
    {DataCallFailCause_t::RRC_UPLINK_ERROR_REQUEST_FROM_NAS, V1_4::DataCallFailCause::RRC_UPLINK_ERROR_REQUEST_FROM_NAS},
    {DataCallFailCause_t::RRC_CONNECTION_ACCESS_STRATUM_FAILURE, V1_4::DataCallFailCause::RRC_CONNECTION_ACCESS_STRATUM_FAILURE},
    {DataCallFailCause_t::RRC_CONNECTION_ANOTHER_PROCEDURE_IN_PROGRESS, V1_4::DataCallFailCause::RRC_CONNECTION_ANOTHER_PROCEDURE_IN_PROGRESS},
    {DataCallFailCause_t::RRC_CONNECTION_ACCESS_BARRED, V1_4::DataCallFailCause::RRC_CONNECTION_ACCESS_BARRED},
    {DataCallFailCause_t::RRC_CONNECTION_CELL_RESELECTION, V1_4::DataCallFailCause::RRC_CONNECTION_CELL_RESELECTION},
    {DataCallFailCause_t::RRC_CONNECTION_CONFIG_FAILURE, V1_4::DataCallFailCause::RRC_CONNECTION_CONFIG_FAILURE},
    {DataCallFailCause_t::RRC_CONNECTION_TIMER_EXPIRED, V1_4::DataCallFailCause::RRC_CONNECTION_TIMER_EXPIRED},
    {DataCallFailCause_t::RRC_CONNECTION_LINK_FAILURE, V1_4::DataCallFailCause::RRC_CONNECTION_LINK_FAILURE},
    {DataCallFailCause_t::RRC_CONNECTION_CELL_NOT_CAMPED, V1_4::DataCallFailCause::RRC_CONNECTION_CELL_NOT_CAMPED},
    {DataCallFailCause_t::RRC_CONNECTION_SYSTEM_INTERVAL_FAILURE, V1_4::DataCallFailCause::RRC_CONNECTION_SYSTEM_INTERVAL_FAILURE},
    {DataCallFailCause_t::RRC_CONNECTION_REJECT_BY_NETWORK, V1_4::DataCallFailCause::RRC_CONNECTION_REJECT_BY_NETWORK},
    {DataCallFailCause_t::RRC_CONNECTION_NORMAL_RELEASE, V1_4::DataCallFailCause::RRC_CONNECTION_NORMAL_RELEASE},
    {DataCallFailCause_t::RRC_CONNECTION_RADIO_LINK_FAILURE, V1_4::DataCallFailCause::RRC_CONNECTION_RADIO_LINK_FAILURE},
    {DataCallFailCause_t::RRC_CONNECTION_REESTABLISHMENT_FAILURE, V1_4::DataCallFailCause::RRC_CONNECTION_REESTABLISHMENT_FAILURE},
    {DataCallFailCause_t::RRC_CONNECTION_OUT_OF_SERVICE_DURING_CELL_REGISTER, V1_4::DataCallFailCause::RRC_CONNECTION_OUT_OF_SERVICE_DURING_CELL_REGISTER},
    {DataCallFailCause_t::RRC_CONNECTION_ABORT_REQUEST, V1_4::DataCallFailCause::RRC_CONNECTION_ABORT_REQUEST},
    {DataCallFailCause_t::RRC_CONNECTION_SYSTEM_INFORMATION_BLOCK_READ_ERROR, V1_4::DataCallFailCause::RRC_CONNECTION_SYSTEM_INFORMATION_BLOCK_READ_ERROR},
    {DataCallFailCause_t::NETWORK_INITIATED_DETACH_WITH_AUTO_REATTACH, V1_4::DataCallFailCause::NETWORK_INITIATED_DETACH_WITH_AUTO_REATTACH},
    {DataCallFailCause_t::NETWORK_INITIATED_DETACH_NO_AUTO_REATTACH, V1_4::DataCallFailCause::NETWORK_INITIATED_DETACH_NO_AUTO_REATTACH},
    {DataCallFailCause_t::ESM_PROCEDURE_TIME_OUT, V1_4::DataCallFailCause::ESM_PROCEDURE_TIME_OUT},
    {DataCallFailCause_t::INVALID_CONNECTION_ID, V1_4::DataCallFailCause::INVALID_CONNECTION_ID},
    {DataCallFailCause_t::MAXIMIUM_NSAPIS_EXCEEDED, V1_4::DataCallFailCause::MAXIMIUM_NSAPIS_EXCEEDED},
    {DataCallFailCause_t::INVALID_PRIMARY_NSAPI, V1_4::DataCallFailCause::INVALID_PRIMARY_NSAPI},
    {DataCallFailCause_t::CANNOT_ENCODE_OTA_MESSAGE, V1_4::DataCallFailCause::CANNOT_ENCODE_OTA_MESSAGE},
    {DataCallFailCause_t::RADIO_ACCESS_BEARER_SETUP_FAILURE, V1_4::DataCallFailCause::RADIO_ACCESS_BEARER_SETUP_FAILURE},
    {DataCallFailCause_t::PDP_ESTABLISH_TIMEOUT_EXPIRED, V1_4::DataCallFailCause::PDP_ESTABLISH_TIMEOUT_EXPIRED},
    {DataCallFailCause_t::PDP_MODIFY_TIMEOUT_EXPIRED, V1_4::DataCallFailCause::PDP_MODIFY_TIMEOUT_EXPIRED},
    {DataCallFailCause_t::PDP_INACTIVE_TIMEOUT_EXPIRED, V1_4::DataCallFailCause::PDP_INACTIVE_TIMEOUT_EXPIRED},
    {DataCallFailCause_t::PDP_LOWERLAYER_ERROR, V1_4::DataCallFailCause::PDP_LOWERLAYER_ERROR},
    {DataCallFailCause_t::PDP_MODIFY_COLLISION, V1_4::DataCallFailCause::PDP_MODIFY_COLLISION},
    {DataCallFailCause_t::MAXINUM_SIZE_OF_L2_MESSAGE_EXCEEDED, V1_4::DataCallFailCause::MAXINUM_SIZE_OF_L2_MESSAGE_EXCEEDED},
    {DataCallFailCause_t::NAS_REQUEST_REJECTED_BY_NETWORK, V1_4::DataCallFailCause::NAS_REQUEST_REJECTED_BY_NETWORK},
    {DataCallFailCause_t::RRC_CONNECTION_INVALID_REQUEST, V1_4::DataCallFailCause::RRC_CONNECTION_INVALID_REQUEST},
    {DataCallFailCause_t::RRC_CONNECTION_TRACKING_AREA_ID_CHANGED, V1_4::DataCallFailCause::RRC_CONNECTION_TRACKING_AREA_ID_CHANGED},
    {DataCallFailCause_t::RRC_CONNECTION_RF_UNAVAILABLE, V1_4::DataCallFailCause::RRC_CONNECTION_RF_UNAVAILABLE},
    {DataCallFailCause_t::RRC_CONNECTION_ABORTED_DUE_TO_IRAT_CHANGE, V1_4::DataCallFailCause::RRC_CONNECTION_ABORTED_DUE_TO_IRAT_CHANGE},
    {DataCallFailCause_t::RRC_CONNECTION_RELEASED_SECURITY_NOT_ACTIVE, V1_4::DataCallFailCause::RRC_CONNECTION_RELEASED_SECURITY_NOT_ACTIVE},
    {DataCallFailCause_t::RRC_CONNECTION_ABORTED_AFTER_HANDOVER, V1_4::DataCallFailCause::RRC_CONNECTION_ABORTED_AFTER_HANDOVER},
    {DataCallFailCause_t::RRC_CONNECTION_ABORTED_AFTER_IRAT_CELL_CHANGE, V1_4::DataCallFailCause::RRC_CONNECTION_ABORTED_AFTER_IRAT_CELL_CHANGE},
    {DataCallFailCause_t::RRC_CONNECTION_ABORTED_DURING_IRAT_CELL_CHANGE, V1_4::DataCallFailCause::RRC_CONNECTION_ABORTED_DURING_IRAT_CELL_CHANGE},
    {DataCallFailCause_t::IMSI_UNKNOWN_IN_HOME_SUBSCRIBER_SERVER, V1_4::DataCallFailCause::IMSI_UNKNOWN_IN_HOME_SUBSCRIBER_SERVER},
    {DataCallFailCause_t::IMEI_NOT_ACCEPTED, V1_4::DataCallFailCause::IMEI_NOT_ACCEPTED},
    {DataCallFailCause_t::EPS_SERVICES_AND_NON_EPS_SERVICES_NOT_ALLOWED, V1_4::DataCallFailCause::EPS_SERVICES_AND_NON_EPS_SERVICES_NOT_ALLOWED},
    {DataCallFailCause_t::EPS_SERVICES_NOT_ALLOWED_IN_PLMN, V1_4::DataCallFailCause::EPS_SERVICES_NOT_ALLOWED_IN_PLMN},
    {DataCallFailCause_t::MSC_TEMPORARILY_NOT_REACHABLE, V1_4::DataCallFailCause::MSC_TEMPORARILY_NOT_REACHABLE},
    {DataCallFailCause_t::CS_DOMAIN_NOT_AVAILABLE, V1_4::DataCallFailCause::CS_DOMAIN_NOT_AVAILABLE},
    {DataCallFailCause_t::ESM_FAILURE, V1_4::DataCallFailCause::ESM_FAILURE},
    {DataCallFailCause_t::MAC_FAILURE, V1_4::DataCallFailCause::MAC_FAILURE},
    {DataCallFailCause_t::SYNCHRONIZATION_FAILURE, V1_4::DataCallFailCause::SYNCHRONIZATION_FAILURE},
    {DataCallFailCause_t::UE_SECURITY_CAPABILITIES_MISMATCH, V1_4::DataCallFailCause::UE_SECURITY_CAPABILITIES_MISMATCH},
    {DataCallFailCause_t::SECURITY_MODE_REJECTED, V1_4::DataCallFailCause::SECURITY_MODE_REJECTED},
    {DataCallFailCause_t::UNACCEPTABLE_NON_EPS_AUTHENTICATION, V1_4::DataCallFailCause::UNACCEPTABLE_NON_EPS_AUTHENTICATION},
    {DataCallFailCause_t::CS_FALLBACK_CALL_ESTABLISHMENT_NOT_ALLOWED, V1_4::DataCallFailCause::CS_FALLBACK_CALL_ESTABLISHMENT_NOT_ALLOWED},
    {DataCallFailCause_t::NO_EPS_BEARER_CONTEXT_ACTIVATED, V1_4::DataCallFailCause::NO_EPS_BEARER_CONTEXT_ACTIVATED},
    {DataCallFailCause_t::INVALID_EMM_STATE, V1_4::DataCallFailCause::INVALID_EMM_STATE},
    {DataCallFailCause_t::NAS_LAYER_FAILURE, V1_4::DataCallFailCause::NAS_LAYER_FAILURE},
    {DataCallFailCause_t::MULTIPLE_PDP_CALL_NOT_ALLOWED, V1_4::DataCallFailCause::MULTIPLE_PDP_CALL_NOT_ALLOWED},
    {DataCallFailCause_t::EMBMS_NOT_ENABLED, V1_4::DataCallFailCause::EMBMS_NOT_ENABLED},
    {DataCallFailCause_t::IRAT_HANDOVER_FAILED, V1_4::DataCallFailCause::IRAT_HANDOVER_FAILED},
    {DataCallFailCause_t::EMBMS_REGULAR_DEACTIVATION, V1_4::DataCallFailCause::EMBMS_REGULAR_DEACTIVATION},
    {DataCallFailCause_t::TEST_LOOPBACK_REGULAR_DEACTIVATION, V1_4::DataCallFailCause::TEST_LOOPBACK_REGULAR_DEACTIVATION},
    {DataCallFailCause_t::LOWER_LAYER_REGISTRATION_FAILURE, V1_4::DataCallFailCause::LOWER_LAYER_REGISTRATION_FAILURE},
    {DataCallFailCause_t::DATA_PLAN_EXPIRED, V1_4::DataCallFailCause::DATA_PLAN_EXPIRED},
    {DataCallFailCause_t::UMTS_HANDOVER_TO_IWLAN, V1_4::DataCallFailCause::UMTS_HANDOVER_TO_IWLAN},
    {DataCallFailCause_t::EVDO_CONNECTION_DENY_BY_GENERAL_OR_NETWORK_BUSY, V1_4::DataCallFailCause::EVDO_CONNECTION_DENY_BY_GENERAL_OR_NETWORK_BUSY},
    {DataCallFailCause_t::EVDO_CONNECTION_DENY_BY_BILLING_OR_AUTHENTICATION_FAILURE, V1_4::DataCallFailCause::EVDO_CONNECTION_DENY_BY_BILLING_OR_AUTHENTICATION_FAILURE},
    {DataCallFailCause_t::EVDO_HDR_CHANGED, V1_4::DataCallFailCause::EVDO_HDR_CHANGED},
    {DataCallFailCause_t::EVDO_HDR_EXITED, V1_4::DataCallFailCause::EVDO_HDR_EXITED},
    {DataCallFailCause_t::EVDO_HDR_NO_SESSION, V1_4::DataCallFailCause::EVDO_HDR_NO_SESSION},
    {DataCallFailCause_t::EVDO_USING_GPS_FIX_INSTEAD_OF_HDR_CALL, V1_4::DataCallFailCause::EVDO_USING_GPS_FIX_INSTEAD_OF_HDR_CALL},
    {DataCallFailCause_t::EVDO_HDR_CONNECTION_SETUP_TIMEOUT, V1_4::DataCallFailCause::EVDO_HDR_CONNECTION_SETUP_TIMEOUT},
    {DataCallFailCause_t::FAILED_TO_ACQUIRE_COLOCATED_HDR, V1_4::DataCallFailCause::FAILED_TO_ACQUIRE_COLOCATED_HDR},
    {DataCallFailCause_t::OTASP_COMMIT_IN_PROGRESS, V1_4::DataCallFailCause::OTASP_COMMIT_IN_PROGRESS},
    {DataCallFailCause_t::NO_HYBRID_HDR_SERVICE, V1_4::DataCallFailCause::NO_HYBRID_HDR_SERVICE},
    {DataCallFailCause_t::HDR_NO_LOCK_GRANTED, V1_4::DataCallFailCause::HDR_NO_LOCK_GRANTED},
    {DataCallFailCause_t::DBM_OR_SMS_IN_PROGRESS, V1_4::DataCallFailCause::DBM_OR_SMS_IN_PROGRESS},
    {DataCallFailCause_t::HDR_FADE, V1_4::DataCallFailCause::HDR_FADE},
    {DataCallFailCause_t::HDR_ACCESS_FAILURE, V1_4::DataCallFailCause::HDR_ACCESS_FAILURE},
    {DataCallFailCause_t::UNSUPPORTED_1X_PREV, V1_4::DataCallFailCause::UNSUPPORTED_1X_PREV},
    {DataCallFailCause_t::LOCAL_END, V1_4::DataCallFailCause::LOCAL_END},
    {DataCallFailCause_t::NO_SERVICE, V1_4::DataCallFailCause::NO_SERVICE},
    {DataCallFailCause_t::FADE, V1_4::DataCallFailCause::FADE},
    {DataCallFailCause_t::NORMAL_RELEASE, V1_4::DataCallFailCause::NORMAL_RELEASE},
    {DataCallFailCause_t::ACCESS_ATTEMPT_ALREADY_IN_PROGRESS, V1_4::DataCallFailCause::ACCESS_ATTEMPT_ALREADY_IN_PROGRESS},
    {DataCallFailCause_t::REDIRECTION_OR_HANDOFF_IN_PROGRESS, V1_4::DataCallFailCause::REDIRECTION_OR_HANDOFF_IN_PROGRESS},
    {DataCallFailCause_t::EMERGENCY_MODE, V1_4::DataCallFailCause::EMERGENCY_MODE},
    {DataCallFailCause_t::PHONE_IN_USE, V1_4::DataCallFailCause::PHONE_IN_USE},
    {DataCallFailCause_t::INVALID_MODE, V1_4::DataCallFailCause::INVALID_MODE},
    {DataCallFailCause_t::INVALID_SIM_STATE, V1_4::DataCallFailCause::INVALID_SIM_STATE},
    {DataCallFailCause_t::NO_COLLOCATED_HDR, V1_4::DataCallFailCause::NO_COLLOCATED_HDR},
    {DataCallFailCause_t::UE_IS_ENTERING_POWERSAVE_MODE, V1_4::DataCallFailCause::UE_IS_ENTERING_POWERSAVE_MODE},
    {DataCallFailCause_t::DUAL_SWITCH, V1_4::DataCallFailCause::DUAL_SWITCH},
    {DataCallFailCause_t::PPP_TIMEOUT, V1_4::DataCallFailCause::PPP_TIMEOUT},
    {DataCallFailCause_t::PPP_AUTH_FAILURE, V1_4::DataCallFailCause::PPP_AUTH_FAILURE},
    {DataCallFailCause_t::PPP_OPTION_MISMATCH, V1_4::DataCallFailCause::PPP_OPTION_MISMATCH},
    {DataCallFailCause_t::PPP_PAP_FAILURE, V1_4::DataCallFailCause::PPP_PAP_FAILURE},
    {DataCallFailCause_t::PPP_CHAP_FAILURE, V1_4::DataCallFailCause::PPP_CHAP_FAILURE},
    {DataCallFailCause_t::PPP_CLOSE_IN_PROGRESS, V1_4::DataCallFailCause::PPP_CLOSE_IN_PROGRESS},
    {DataCallFailCause_t::LIMITED_TO_IPV4, V1_4::DataCallFailCause::LIMITED_TO_IPV4},
    {DataCallFailCause_t::LIMITED_TO_IPV6, V1_4::DataCallFailCause::LIMITED_TO_IPV6},
    {DataCallFailCause_t::VSNCP_TIMEOUT, V1_4::DataCallFailCause::VSNCP_TIMEOUT},
    {DataCallFailCause_t::VSNCP_GEN_ERROR, V1_4::DataCallFailCause::VSNCP_GEN_ERROR},
    {DataCallFailCause_t::VSNCP_APN_UNATHORIZED, V1_4::DataCallFailCause::VSNCP_APN_UNATHORIZED},
    {DataCallFailCause_t::VSNCP_PDN_LIMIT_EXCEEDED, V1_4::DataCallFailCause::VSNCP_PDN_LIMIT_EXCEEDED},
    {DataCallFailCause_t::VSNCP_NO_PDN_GATEWAY_ADDRESS, V1_4::DataCallFailCause::VSNCP_NO_PDN_GATEWAY_ADDRESS},
    {DataCallFailCause_t::VSNCP_PDN_GATEWAY_UNREACHABLE, V1_4::DataCallFailCause::VSNCP_PDN_GATEWAY_UNREACHABLE},
    {DataCallFailCause_t::VSNCP_PDN_GATEWAY_REJECT, V1_4::DataCallFailCause::VSNCP_PDN_GATEWAY_REJECT},
    {DataCallFailCause_t::VSNCP_INSUFFICIENT_PARAMETERS, V1_4::DataCallFailCause::VSNCP_INSUFFICIENT_PARAMETERS},
    {DataCallFailCause_t::VSNCP_RESOURCE_UNAVAILABLE, V1_4::DataCallFailCause::VSNCP_RESOURCE_UNAVAILABLE},
    {DataCallFailCause_t::VSNCP_ADMINISTRATIVELY_PROHIBITED, V1_4::DataCallFailCause::VSNCP_ADMINISTRATIVELY_PROHIBITED},
    {DataCallFailCause_t::VSNCP_PDN_ID_IN_USE, V1_4::DataCallFailCause::VSNCP_PDN_ID_IN_USE},
    {DataCallFailCause_t::VSNCP_SUBSCRIBER_LIMITATION, V1_4::DataCallFailCause::VSNCP_SUBSCRIBER_LIMITATION},
    {DataCallFailCause_t::VSNCP_PDN_EXISTS_FOR_THIS_APN, V1_4::DataCallFailCause::VSNCP_PDN_EXISTS_FOR_THIS_APN},
    {DataCallFailCause_t::VSNCP_RECONNECT_NOT_ALLOWED, V1_4::DataCallFailCause::VSNCP_RECONNECT_NOT_ALLOWED},
    {DataCallFailCause_t::IPV6_PREFIX_UNAVAILABLE, V1_4::DataCallFailCause::IPV6_PREFIX_UNAVAILABLE},
    {DataCallFailCause_t::HANDOFF_PREFERENCE_CHANGED, V1_4::DataCallFailCause::HANDOFF_PREFERENCE_CHANGED},
    {DataCallFailCause_t::OEM_DCFAILCAUSE_1, V1_4::DataCallFailCause::OEM_DCFAILCAUSE_1},
    {DataCallFailCause_t::OEM_DCFAILCAUSE_2, V1_4::DataCallFailCause::OEM_DCFAILCAUSE_2},
    {DataCallFailCause_t::OEM_DCFAILCAUSE_3, V1_4::DataCallFailCause::OEM_DCFAILCAUSE_3},
    {DataCallFailCause_t::OEM_DCFAILCAUSE_4, V1_4::DataCallFailCause::OEM_DCFAILCAUSE_4},
    {DataCallFailCause_t::OEM_DCFAILCAUSE_5, V1_4::DataCallFailCause::OEM_DCFAILCAUSE_5},
    {DataCallFailCause_t::OEM_DCFAILCAUSE_6, V1_4::DataCallFailCause::OEM_DCFAILCAUSE_6},
    {DataCallFailCause_t::OEM_DCFAILCAUSE_7, V1_4::DataCallFailCause::OEM_DCFAILCAUSE_7},
    {DataCallFailCause_t::OEM_DCFAILCAUSE_8, V1_4::DataCallFailCause::OEM_DCFAILCAUSE_8},
    {DataCallFailCause_t::OEM_DCFAILCAUSE_9, V1_4::DataCallFailCause::OEM_DCFAILCAUSE_9},
    {DataCallFailCause_t::OEM_DCFAILCAUSE_10, V1_4::DataCallFailCause::OEM_DCFAILCAUSE_10},
    {DataCallFailCause_t::OEM_DCFAILCAUSE_11, V1_4::DataCallFailCause::OEM_DCFAILCAUSE_11},
    {DataCallFailCause_t::OEM_DCFAILCAUSE_12, V1_4::DataCallFailCause::OEM_DCFAILCAUSE_12},
    {DataCallFailCause_t::OEM_DCFAILCAUSE_13, V1_4::DataCallFailCause::OEM_DCFAILCAUSE_13},
    {DataCallFailCause_t::OEM_DCFAILCAUSE_14, V1_4::DataCallFailCause::OEM_DCFAILCAUSE_14},
    {DataCallFailCause_t::OEM_DCFAILCAUSE_15, V1_4::DataCallFailCause::OEM_DCFAILCAUSE_15},
    {DataCallFailCause_t::VOICE_REGISTRATION_FAIL, V1_4::DataCallFailCause::VOICE_REGISTRATION_FAIL},
    {DataCallFailCause_t::DATA_REGISTRATION_FAIL, V1_4::DataCallFailCause::DATA_REGISTRATION_FAIL},
    {DataCallFailCause_t::SIGNAL_LOST, V1_4::DataCallFailCause::SIGNAL_LOST},
    {DataCallFailCause_t::PREF_RADIO_TECH_CHANGED, V1_4::DataCallFailCause::PREF_RADIO_TECH_CHANGED},
    {DataCallFailCause_t::RADIO_POWER_OFF, V1_4::DataCallFailCause::RADIO_POWER_OFF},
    {DataCallFailCause_t::TETHERED_CALL_ACTIVE, V1_4::DataCallFailCause::TETHERED_CALL_ACTIVE},
    {DataCallFailCause_t::ERROR_UNSPECIFIED, V1_4::DataCallFailCause::ERROR_UNSPECIFIED}
};

void RadioImpl_1_4::clearCallbacks() {
    RLOGD("RadioImpl_1_4::clearCallbacks");
    mRadioResponseV1_4 = NULL;
    mRadioIndicationV1_4 = NULL;
    RadioImpl_1_3::clearCallbacks();
}

Return<void> RadioImpl_1_4::setResponseFunctions_nolock(
        const ::android::sp<IRadioResponse>& radioResponseParam,
        const ::android::sp<IRadioIndication>& radioIndicationParam) {
    RLOGD("RadioImpl_1_4::setResponseFunctions_nolock");

    mRadioResponseV1_4 = V1_4::IRadioResponse::castFrom(radioResponseParam).withDefault(nullptr);
    mRadioIndicationV1_4 = V1_4::IRadioIndication::castFrom(radioIndicationParam).withDefault(nullptr);

    bool isAPAssistCap = true;

    if (mRadioResponseV1_4 == nullptr || mRadioIndicationV1_4 == nullptr) {
        mRadioResponseV1_4 = nullptr;
        mRadioIndicationV1_4 = nullptr;
        isAPAssistCap = false;
    }

    QCRIL_LOG_DEBUG("radioResponseParamV1_4: %s. radioIndicationV1_4: %s",
            mRadioResponseV1_4 ? "<not null>" : "<null>",
            mRadioIndicationV1_4 ? "<not null>" : "<null>");

    auto msg = std::make_shared<rildata::SetRadioClientCapUnSolMessage>(isAPAssistCap);
    if(msg) {
        msg->broadcast();
    }

    RadioImpl_1_3::setResponseFunctions_nolock(
            radioResponseParam,
            radioIndicationParam);

    return Void();
}

Return<void> RadioImpl_1_4::setResponseFunctions(
        const ::android::sp<IRadioResponse>& radioResponseParam,
        const ::android::sp<IRadioIndication>& radioIndicationParam) {
    QCRIL_LOG_DEBUG("setResponseFunctions_1_4. radioResponseParam: %s. radioIndicationParam: %s",
            radioResponseParam ? "<not null>" : "<null>",
            radioIndicationParam ? "<not null>" : "<null>");
    RLOGD("setResponseFunctions_1_4");

    radioImpl_1_4 = this;

    qtimutex::QtiSharedMutex *radioServiceRwlockPtr = radio::getRadioServiceRwlock(mSlotId);
    radioServiceRwlockPtr->lock();

    RadioImpl_1_4::setResponseFunctions_nolock(radioResponseParam, radioIndicationParam);

    radioServiceRwlockPtr->unlock();

 #ifndef QMI_RIL_UTF
    // client is connected. Send initial indications.
    android::onNewCommandConnect((RIL_SOCKET_ID) mSlotId);
#endif

    return Void();
}

vector<hidl_string> convertAddrStringToHidlStringVector(const string &addr) {
    string tmpString;
    char delimiter = 0x20;
    vector<hidl_string> hidlAddressesVector;
    stringstream ssAddresses(addr);
    while(getline(ssAddresses, tmpString, delimiter)) {
        hidlAddressesVector.push_back(tmpString);
    }
    return hidlAddressesVector;
}

V1_4::PdpProtocolType convertStringToPdpProtocolType(const string &type) {
    V1_4::PdpProtocolType protocolType = V1_4::PdpProtocolType::UNKNOWN;

    if (type.compare("IP") == 0) {
        protocolType = V1_4::PdpProtocolType::IP;
    }
    else if (type.compare("IPV6") == 0) {
        protocolType = V1_4::PdpProtocolType::IPV6;
    }
    else if (type.compare("IPV4V6") == 0) {
        protocolType = V1_4::PdpProtocolType::IPV4V6;
    }
    else if (type.compare("PPP") == 0) {
        protocolType = V1_4::PdpProtocolType::PPP;
    }
    else if (type.compare("NON_IP") == 0) {
        protocolType = V1_4::PdpProtocolType::NON_IP;
    }
    else if (type.compare("UNSTRUCTURED") == 0) {
        protocolType = V1_4::PdpProtocolType::UNSTRUCTURED;
    }
    else {
        protocolType = V1_4::PdpProtocolType::UNKNOWN;
    }

    return protocolType;
}

string convertPdpProtocolTypeToString(const V1_4::PdpProtocolType protocol) {
    string protocolType;
    switch(protocol) {
        case V1_4::PdpProtocolType::IP:
        protocolType = string("IP");
        break;

        case V1_4::PdpProtocolType::IPV6:
        protocolType = string("IPV6");
        break;

        case V1_4::PdpProtocolType::IPV4V6:
        protocolType = string("IPV4V6");
        break;

        case V1_4::PdpProtocolType::PPP:
        protocolType = string("PPP");
        break;

        case V1_4::PdpProtocolType::UNKNOWN:
        case V1_4::PdpProtocolType::NON_IP:
        case V1_4::PdpProtocolType::UNSTRUCTURED:
        default:
        protocolType = string("UNKNOWN");
        break;
    }
    return protocolType;
}

V1_4::DataCallFailCause convertDcFailCauseToHidlDcFailCause(const DataCallFailCause_t &cause) {
    V1_4::DataCallFailCause hidlCause = V1_4::DataCallFailCause::ERROR_UNSPECIFIED;
    if (dcFailCauseMap.find(cause) != dcFailCauseMap.end()) {
        hidlCause = dcFailCauseMap[cause];
    }
    return hidlCause;
}

Return<void> RadioImpl_1_4::getDataCallList(int32_t serial) {
    QCRIL_LOG_DEBUG("RadioImpl_1_4::getDataCallList, serial = %d", serial);
    using namespace rildata;

    if( mRadioResponseV1_4 == nullptr)
    {
      QCRIL_LOG_DEBUG("mRadioResponseV1_4 is null");
      return RadioImpl_1_3::getDataCallList(serial);
    }

    auto msg = std::make_shared<GetRadioDataCallListRequestMessage>();
    GenericCallback<DataCallListResult_t> cb(
        ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<DataCallListResult_t> responseDataPtr) -> void {
            ::android::hardware::hidl_vec<V1_4::SetupDataCallResult> dcResultList;
            RadioResponseInfo responseInfo {.type = RadioResponseType::SOLICITED, .serial = serial, .error = RadioError::NO_MEMORY};
            if (solicitedMsg && responseDataPtr) {
                RadioError e = RadioError::NONE;
                if ((status == Message::Callback::Status::SUCCESS) &&
                   (responseDataPtr->respErr == ResponseError_t::NO_ERROR)) {
                    QCRIL_LOG_DEBUG("getDataCallList cb invoked status %d respErr %d", status, responseDataPtr->respErr);
                    QCRIL_LOG_DEBUG("Call list size = %d", responseDataPtr->call.size());

                    dcResultList.resize(responseDataPtr->call.size());
                    int i=0;
                    for (rildata::DataCallResult_t entry: responseDataPtr->call) {
                        dcResultList[i].cause = (V1_4::DataCallFailCause) entry.cause;
                        dcResultList[i].suggestedRetryTime = entry.suggestedRetryTime;
                        dcResultList[i].cid = entry.cid;
                        QCRIL_LOG_DEBUG("cid %d", entry.cid);
                        dcResultList[i].active = (V1_4::DataConnActiveStatus)entry.active;
                        dcResultList[i].type = convertStringToPdpProtocolType(entry.type);
                        QCRIL_LOG_DEBUG("ifname %s", entry.ifname.c_str());
                        dcResultList[i].ifname = entry.ifname;
                        QCRIL_LOG_DEBUG("addresses %s", entry.addresses.c_str());
                        dcResultList[i].addresses = convertAddrStringToHidlStringVector(entry.addresses);
                        dcResultList[i].dnses = convertAddrStringToHidlStringVector(entry.dnses);
                        dcResultList[i].gateways = convertAddrStringToHidlStringVector(entry.gateways);
                        dcResultList[i].pcscf = convertAddrStringToHidlStringVector(entry.pcscf);
                        dcResultList[i].mtu = entry.mtu;
                        i++;
                    }
                }
                else {
                    switch (responseDataPtr->respErr) {
                        case ResponseError_t::NOT_SUPPORTED: e = RadioError::REQUEST_NOT_SUPPORTED; break;
                        case ResponseError_t::INVALID_ARGUMENT: e = RadioError::INVALID_ARGUMENTS; break;
                        default: e = RadioError::GENERIC_FAILURE; break;
                    }
                }
                responseInfo = {.type = RadioResponseType::SOLICITED, .serial = serial, .error = e};
            }
            RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponseV1_4, getDataCallListResponse_1_4, responseInfo, dcResultList);
        }));
    if (msg) {
        msg->setCallback(&cb);
        msg->dispatch();
    }
    return Void();
}

Return<void> RadioImpl_1_4::deactivateDataCall_1_2(int32_t serial, int32_t cid, V1_2::DataRequestReason reason) {
#if VDBG
    RLOGD("RadioImpl_1_4::deactivateDataCall_1_2: serial %d", serial);
#endif
    using namespace rildata;
    if( mRadioResponseV1_4 == nullptr)
    {
      QCRIL_LOG_DEBUG("mRadioResponseV1_4 is null");
      return RadioImpl_1_3::deactivateDataCall_1_2(serial, cid, reason);
    }

    auto msg =
        std::make_shared<DeactivateDataCallRequestMessage>(
            serial,
            cid,
            (DataRequestReason_t)reason,
            nullptr);
    if (msg) {
        GenericCallback<ResponseError_t> cb([serial](std::shared_ptr<Message>,
                                                     Message::Callback::Status status,
                                                     std::shared_ptr<ResponseError_t> rsp) -> void {
            if (rsp != nullptr) {
                auto indMsg = std::make_shared<DeactivateDataCallRadioResponseIndMessage>(*rsp, serial, status);
                if (indMsg != nullptr) {
                    indMsg->broadcast();
                } else {
                    QCRIL_LOG_DEBUG("deactivate data call cb failed to allocate message status %d respErr %d", status, *rsp);
                }
            } else {
                QCRIL_LOG_ERROR("deactivate data call resp is nullptr");
            }
        });
        msg->setCallback(&cb);
        msg->dispatch();
    }
    return Void();
}

Return<void> RadioImpl_1_4::startKeepalive(int32_t serial, const ::android::hardware::radio::V1_1::KeepaliveRequest& keepalive) {
#if VDBG
    RLOGD("RadioImpl_1_4::startKeepalive: serial %d", serial);
#endif
    using namespace rildata;
    if( mRadioResponseV1_4 == nullptr)
    {
      QCRIL_LOG_DEBUG("mRadioResponseV1_4 is null");
      return RadioImpl_1_3::startKeepalive(serial, keepalive);
    }
    switch((KeepaliveType_t)keepalive.type) {
        case KeepaliveType_t::NATT_IPV4:
            if (keepalive.sourceAddress.size() != 4 ||
                    keepalive.destinationAddress.size() != 4) {
                QCRIL_LOG_DEBUG("Invalid address for keepalive!");
                RadioResponseInfo responseInfo {.serial = serial, .error = RadioError::INVALID_ARGUMENTS};
                ::android::hardware::radio::V1_1::KeepaliveStatus ka_status = {};
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponseV1_4, startKeepaliveResponse, responseInfo, ka_status);
                return Void();
            }
            break;
        case KeepaliveType_t::NATT_IPV6:
            if (keepalive.sourceAddress.size() != 16 ||
                    keepalive.destinationAddress.size() != 16) {
                QCRIL_LOG_DEBUG("Invalid address for keepalive!");
                RadioResponseInfo responseInfo {.serial = serial, .error = RadioError::INVALID_ARGUMENTS};
                ::android::hardware::radio::V1_1::KeepaliveStatus ka_status = {};
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponseV1_4, startKeepaliveResponse, responseInfo, ka_status);
                return Void();
            }
            break;
        default:
            QCRIL_LOG_DEBUG("Unknown packet keepalive type!");
            RadioResponseInfo responseInfo {.serial = serial, .error = RadioError::INVALID_ARGUMENTS};
            ::android::hardware::radio::V1_1::KeepaliveStatus ka_status = {};
            RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponseV1_4, startKeepaliveResponse, responseInfo, ka_status);
            return Void();
    }

    KeepaliveRequest_t req;
    req.type = (KeepaliveType_t)keepalive.type;
    req.sourceAddress = keepalive.sourceAddress;
    req.sourcePort = keepalive.sourcePort;
    req.destinationAddress = keepalive.destinationAddress;
    req.destinationPort = keepalive.destinationPort;
    req.maxKeepaliveIntervalMillis = keepalive.maxKeepaliveIntervalMillis;
    req.cid = keepalive.cid;
    auto msg = std::make_shared<StartKeepAliveRequestMessage>(serial, req);

    QCRIL_LOG_DEBUG("KeepAlive type %d", req.type);
    QCRIL_LOG_DEBUG("KeepAlive sourceAddress %s", ::android::hardware::toString(keepalive.sourceAddress).c_str());
    QCRIL_LOG_DEBUG("KeepAlive sourcePort %d", req.sourcePort);
    QCRIL_LOG_DEBUG("KeepAlive destinationAddress %s", ::android::hardware::toString(keepalive.destinationAddress).c_str());
    QCRIL_LOG_DEBUG("KeepAlive destinationPort %d", req.destinationPort);
    QCRIL_LOG_DEBUG("KeepAlive maxKeepaliveIntervalMillis %d", req.maxKeepaliveIntervalMillis);
    QCRIL_LOG_DEBUG("KeepAlive cid %d", req.cid);

    if (msg) {
        GenericCallback<StartKeepAliveResp_t> cb([this, serial](std::shared_ptr<Message> msg,
                                                         Message::Callback::Status status,
                                                         std::shared_ptr<StartKeepAliveResp_t> rsp) -> void {
            RadioResponseInfo responseInfo {.serial = serial, .error = RadioError::NO_MEMORY};
            if (msg && rsp) {
                RadioError e = RadioError::NONE;
                if ((status != Message::Callback::Status::SUCCESS) ||
                   (rsp->error != ResponseError_t::NO_ERROR)) {
                    switch(rsp->error) {
                        case ResponseError_t::INVALID_ARGUMENT: e = RadioError::INVALID_ARGUMENTS; break;
                        default: e = RadioError::NO_RESOURCES; break;
                    }
                }
                responseInfo = {.serial = serial, .error = e};
                QCRIL_LOG_DEBUG("startKeepAlivecb cb invoked status %d respErr %d", status, *rsp);
            }
            ::android::hardware::radio::V1_1::KeepaliveStatus ka_status = {};
            ka_status.sessionHandle = rsp->handle;
            ka_status.code = static_cast<V1_1::KeepaliveStatusCode>(rsp->status);
            RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponseV1_4, startKeepaliveResponse, responseInfo, ka_status);
        });
        msg->setCallback(&cb);
        msg->dispatch();
    }
    return Void();
}

Return<void> RadioImpl_1_4::stopKeepalive(int32_t serial, int32_t sessionHandle) {
#if VDBG
    RLOGD("RadioImpl_1_4::stopKeepalive: serial %d", serial);
#endif
    using namespace rildata;
    if( mRadioResponseV1_4 == nullptr)
    {
      QCRIL_LOG_DEBUG("mRadioResponseV1_4 is null");
      return RadioImpl_1_3::stopKeepalive(serial, sessionHandle);
    }

    auto msg =
        std::make_shared<StopKeepAliveRequestMessage>(
            serial,
            sessionHandle);

    if (msg) {
        GenericCallback<ResponseError_t> cb([this, serial](std::shared_ptr<Message> msg,
                                                         Message::Callback::Status status,
                                                         std::shared_ptr<ResponseError_t> rsp) -> void {
            RadioResponseInfo responseInfo {.serial = serial, .error = RadioError::NO_MEMORY};
            if (msg && rsp) {
                RadioError e = RadioError::NONE;
                if ((status != Message::Callback::Status::SUCCESS) ||
                   (*rsp != ResponseError_t::NO_ERROR)) {
                        e = RadioError::INVALID_ARGUMENTS;
                }
                responseInfo = {.serial = serial, .error = e};
                QCRIL_LOG_DEBUG("stopKeepAlivecb cb invoked status %d respErr %d", status, *rsp);
            }
            RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponseV1_4, stopKeepaliveResponse, responseInfo);
        });
        msg->setCallback(&cb);
        msg->dispatch();
    }
    return Void();
}

void RadioImpl_1_4::KeepAliveStatusInd(RadioIndicationType indType, const ::android::hardware::radio::V1_1::KeepaliveStatus status) {
  QCRIL_LOG_FUNC_ENTRY();

  auto ind = mRadioIndicationV1_4;
  if (ind) {
    QCRIL_LOG_INFO("KeepAliveStatusIndication");
    Return<void> retStatus = ind->keepaliveStatus(indType, status);
    checkReturnStatus(retStatus);
  } else {
    RLOGE("RadioKeepAliveStatusInd: radioService[%d]->mRadioIndication == NULL", mSlotId);
  }

  QCRIL_LOG_FUNC_RETURN();
}

void RadioImpl_1_4::updatePcoData(RadioIndicationType indType, const ::android::hardware::radio::V1_0::PcoDataInfo pco) {
  QCRIL_LOG_FUNC_ENTRY();

  auto ind = mRadioIndicationV1_4;
  if (ind) {
    QCRIL_LOG_INFO("UpdatePcoData");
    Return<void> retStatus = ind->pcoData(indType, pco);
    checkReturnStatus(retStatus);
  } else {
    RLOGE("UpdatePcoData: radioService[%d]->mRadioIndication == NULL", mSlotId);
  }

  QCRIL_LOG_FUNC_RETURN();
}

void RadioImpl_1_4::carrierInfoForImsiEncryptionRefresh(RadioIndicationType indType) {
  QCRIL_LOG_FUNC_ENTRY();

  auto ind = mRadioIndicationV1_4;
  if (ind) {
    QCRIL_LOG_INFO("CarrierInfoForImsiEncryptionRefresh");
    Return<void> retStatus = ind->carrierInfoForImsiEncryption(indType);
    checkReturnStatus(retStatus);
  } else {
    RLOGE("CarrierInfoForImsiEncryptionRefresh: radioService[%d]->mRadioIndication == NULL", mSlotId);
  }

  QCRIL_LOG_FUNC_RETURN();
}

Return<void> RadioImpl_1_4::setupDataCall_1_4(int32_t serial, V1_4::AccessNetwork accessNetwork, const V1_4::DataProfileInfo& dataProfileInfo,
            bool roamingAllowed, V1_2::DataRequestReason reason, const hidl_vec<hidl_string>& addresses, const hidl_vec<hidl_string>& dnses) {
#if VDBG
    RLOGD("RadioImpl_1_4::setupDataCall_1_4: serial %d", serial);
#endif
    using namespace rildata;

    std::vector<std::string> radioAddresses;
    std::vector<std::string> radioDnses;

    for (size_t i = 0 ; i < addresses.size(); i++ ) {
        radioAddresses.push_back(addresses[i]);
    }
    for (size_t i = 0 ; i < dnses.size(); i++ ) {
        radioDnses.push_back(dnses[i]);
    }

    AccessNetwork_t accnet = AccessNetwork_t::UNKNOWN;
    switch (accessNetwork) {
        case V1_4::AccessNetwork::GERAN:
            accnet = AccessNetwork_t::GERAN;
            break;
        case V1_4::AccessNetwork::UTRAN:
            accnet = AccessNetwork_t::UTRAN;
            break;
        case V1_4::AccessNetwork::EUTRAN:
            accnet = AccessNetwork_t::EUTRAN;
            break;
        case V1_4::AccessNetwork::CDMA2000:
            accnet = AccessNetwork_t::CDMA2000;
            break;
        case V1_4::AccessNetwork::IWLAN:
            accnet = AccessNetwork_t::IWLAN;
            break;
        default:
            Log::getInstance().d("Unknown AccessNetwork ran = "+std::to_string((int)accnet));
            break;
    }

    auto msg =
      std::make_shared<SetupDataCallRequestMessage>(
        serial,
        #ifdef REQUEST_SOURCE_INCLUDED
        RequestSource_t::RADIO,
        #endif
        accnet,
        (DataProfileId_t)dataProfileInfo.profileId,
        dataProfileInfo.apn,
        convertPdpProtocolTypeToString(dataProfileInfo.protocol),
        convertPdpProtocolTypeToString(dataProfileInfo.roamingProtocol),
        (ApnAuthType_t)dataProfileInfo.authType,
        dataProfileInfo.user,
        dataProfileInfo.password,
        (DataProfileInfoType_t)dataProfileInfo.type,
        dataProfileInfo.maxConnsTime,
        dataProfileInfo.maxConns,
        dataProfileInfo.waitTime,
        dataProfileInfo.enabled,
        (ApnTypes_t)dataProfileInfo.supportedApnTypesBitmap,
        (RadioAccessFamily_t)dataProfileInfo.bearerBitmap,
        dataProfileInfo.mtu,
        dataProfileInfo.preferred,
        dataProfileInfo.persistent,
        roamingAllowed,
        (DataRequestReason_t)reason,
        radioAddresses,
        radioDnses,
        nullptr);

      if (msg) {
        GenericCallback<SetupDataCallResponse_t> cb([serial](std::shared_ptr<Message>,
                                                             Message::Callback::Status status,
                                                             std::shared_ptr<SetupDataCallResponse_t> rsp) -> void {
            if (rsp != nullptr) {
                auto indMsg = std::make_shared<SetupDataCallRadioResponseIndMessage>(*rsp, serial, status);
                if (indMsg != nullptr) {
                    indMsg->broadcast();
                } else {
                    QCRIL_LOG_DEBUG("setup data call cb failed to allocate message status %d respErr %d", status, rsp->respErr);
                }
            } else {
                QCRIL_LOG_ERROR("setupDataCall_1_4 resp is nullptr");
            }
         });
        msg->setCallback(&cb);
        msg->dispatch();
    }

    return Void();
}

Return<void> RadioImpl_1_4::setInitialAttachApn_1_4(int32_t serial, const V1_4::DataProfileInfo& dataProfileInfo) {

    #if VDBG
    RLOGD("setInitialAttachApn: serial %d", serial);
#endif
    RequestInfo *pRI = android::addRequestToList(serial, mSlotId,
            RIL_REQUEST_SET_INITIAL_ATTACH_APN);
    if (pRI == NULL) {
        return Void();
    }

    RIL_InitialAttachApn_v15 iaa = {};

    if (!copyHidlStringToRil(&iaa.apn, dataProfileInfo.apn, pRI, true)) {
        return Void();
    }

    if (!copyHidlStringToRil(&iaa.protocol,
           convertPdpProtocolTypeToString(dataProfileInfo.protocol), pRI)) {
        memsetAndFreeStrings(1, iaa.apn);
        return Void();
    }
    if (!copyHidlStringToRil(&iaa.roamingProtocol,
           convertPdpProtocolTypeToString(dataProfileInfo.roamingProtocol), pRI)) {
       memsetAndFreeStrings(2, iaa.apn, iaa.protocol);
       return Void();
    }
    iaa.authtype = (int) dataProfileInfo.authType;
    if (!copyHidlStringToRil(&iaa.username, dataProfileInfo.user, pRI)) {
        memsetAndFreeStrings(3, iaa.apn, iaa.protocol, iaa.roamingProtocol);
        return Void();
    }
    if (!copyHidlStringToRil(&iaa.password, dataProfileInfo.password, pRI)) {
        memsetAndFreeStrings(4, iaa.apn, iaa.protocol, iaa.roamingProtocol, iaa.username);
        return Void();
    }
    iaa.supportedTypesBitmask = dataProfileInfo.supportedApnTypesBitmap;
    iaa.bearerBitmask = dataProfileInfo.bearerBitmap;
    iaa.modemCognitive = 0;
    iaa.mtu = dataProfileInfo.mtu;

    CALL_ONREQUEST(RIL_REQUEST_SET_INITIAL_ATTACH_APN, &iaa, sizeof(iaa), pRI, mSlotId);

    memsetAndFreeStrings(5, iaa.apn, iaa.protocol, iaa.roamingProtocol, iaa.username,
            iaa.password);

    return Void();
}

Return<void> RadioImpl_1_4::setDataProfile_1_4(int32_t serial, const hidl_vec<V1_4::DataProfileInfo>& profiles) {
  vector<rildata::DataProfileInfo_t> p;
  rildata::DataProfileInfo_t t;

  for (size_t i = 0; i < profiles.size(); i++) {
    /* copy all the profiles */
    memset(&t, 0, sizeof(t));
    t.profileId = (rildata::DataProfileId_t)profiles[i].profileId;
    t.apn = profiles[i].apn;
    t.protocol = convertPdpProtocolTypeToString(profiles[i].protocol);
    t.roamingProtocol = convertPdpProtocolTypeToString(profiles[i].roamingProtocol);
    t.authType = (rildata::ApnAuthType_t)profiles[i].authType;
    t.username = profiles[i].user;
    t.password = profiles[i].password;
    t.dataProfileInfoType = (rildata::DataProfileInfoType_t)profiles[i].type,
    t.maxConnsTime = profiles[i].maxConnsTime;
    t.maxConns = profiles[i].maxConns;
    t.waitTime = profiles[i].waitTime;
    t.enableProfile = profiles[i].enabled;
    t.supportedApnTypesBitmap = (rildata::ApnTypes_t)profiles[i].supportedApnTypesBitmap;
    t.bearerBitmap = (rildata::RadioAccessFamily_t)profiles[i].bearerBitmap;
    t.mtu = profiles[i].mtu;
    t.preferred = profiles[i].preferred;
    t.persistent = profiles[i].persistent;
    p.push_back(t);
  }

  auto msg = std::make_shared<rildata::SetDataProfileRequestMessage>(p);
  if(msg) {
    GenericCallback<RIL_Errno> cb(
        ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<RIL_Errno> responseDataPtr) -> void {
            RadioResponseInfo responseInfo {.serial = serial, .type = RadioResponseType::SOLICITED, .error = RadioError::NO_MEMORY};
            if (solicitedMsg && responseDataPtr) {
                Log::getInstance().d(
                    "[SetDataProfile cb invoked: [msg = " +
                    solicitedMsg->dump() + "] executed. client data = " +
                    "set-ril-service-cb-token status = " + std::to_string((int)status));
                if(status == Message::Callback::Status::SUCCESS) {
                    RadioError e = (RadioError)(*responseDataPtr);
                    responseInfo = {.serial = serial, .type = RadioResponseType::SOLICITED, .error = e};
                }
                else {
                    responseInfo = {.serial = serial, .type = RadioResponseType::SOLICITED, .error = RadioError::GENERIC_FAILURE};
                }
            }
            RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponseV1_4, setDataProfileResponse, responseInfo);
        }));
    msg->setCallback(&cb);
    msg->dispatch();
  }
  return Void();
}

uint64_t toRilCategories(
    const hidl_bitfield<V1_4::EmergencyServiceCategory>& categories) {
    uint64_t rilCategories = (uint64_t)qcril::interfaces::RIL_EccCategoryMask::UNKNOWN;

    if (categories & V1_4::EmergencyServiceCategory::POLICE) {
        rilCategories |= (uint32_t)qcril::interfaces::RIL_EccCategoryMask::POLICE;
    }
    if (categories & V1_4::EmergencyServiceCategory::AMBULANCE) {
        rilCategories |= (uint32_t)qcril::interfaces::RIL_EccCategoryMask::AMBULANCE;
    }
    if (categories & V1_4::EmergencyServiceCategory::FIRE_BRIGADE) {
        rilCategories |= (uint32_t)qcril::interfaces::RIL_EccCategoryMask::FIRE_BRIGADE;
    }
    if (categories & V1_4::EmergencyServiceCategory::MARINE_GUARD) {
        rilCategories |= (uint32_t)qcril::interfaces::RIL_EccCategoryMask::MARINE_GUARD;
    }
    if (categories & V1_4::EmergencyServiceCategory::MOUNTAIN_RESCUE) {
        rilCategories |= (uint32_t)qcril::interfaces::RIL_EccCategoryMask::MOUNTAIN_RESCUE;
    }
    if (categories & V1_4::EmergencyServiceCategory::MIEC ) {
        rilCategories |= (uint32_t)qcril::interfaces::RIL_EccCategoryMask::MANUAL_ECALL;
    }
    if (categories & V1_4::EmergencyServiceCategory::AIEC ) {
        rilCategories |= (uint32_t)qcril::interfaces::RIL_EccCategoryMask::AUTO_ECALL;
    }

    return rilCategories;
}

qcril::interfaces::EmergencyCallRouting toRilRouting(
    const V1_4::EmergencyCallRouting& routing) {
      qcril::interfaces::EmergencyCallRouting rilRouting =
        qcril::interfaces::EmergencyCallRouting::UNKNOWN;

      if (routing == V1_4::EmergencyCallRouting::EMERGENCY) {
          rilRouting = qcril::interfaces::EmergencyCallRouting::EMERGENCY;
      } else if (routing == V1_4::EmergencyCallRouting::NORMAL) {
          rilRouting = qcril::interfaces::EmergencyCallRouting::NORMAL;
      }
      return rilRouting;
}

Return<void> RadioImpl_1_4::emergencyDial(int32_t serial, const V1_0::Dial& dialInfo, hidl_bitfield<V1_4::EmergencyServiceCategory> categories,
        const hidl_vec<hidl_string>& urns, V1_4::EmergencyCallRouting routing, bool fromEmergencyDialer, bool isTesting) {
#if VDBG
  RLOGD("RadioImpl_1_4::emergencyDial: serial %d", serial);
#endif
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestCsDialMessage>(getContext(serial));
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    if (!dialInfo.address.empty()) {
      msg->setAddress(dialInfo.address.c_str());
    }
    msg->setClir((int)dialInfo.clir);
    if (dialInfo.uusInfo.size() != 0) {
      auto uusInfo = std::make_shared<qcril::interfaces::UusInfo>();
      if (uusInfo) {
        uusInfo->setType((RIL_UUS_Type) dialInfo.uusInfo[0].uusType);
        uusInfo->setDcs((RIL_UUS_DCS) dialInfo.uusInfo[0].uusDcs);
        if (dialInfo.uusInfo[0].uusData.size() > 0) {
          uusInfo->setData(std::string(dialInfo.uusInfo[0].uusData.c_str(),
                                       dialInfo.uusInfo[0].uusData.size()));
        }
        msg->setUusInfo(uusInfo);
      }
    }

    msg->setIsEmergency(true);
    msg->setCategories(toRilCategories(categories));
    msg->setRouting(toRilRouting(routing));
    msg->setIsForEccTesting(isTesting);
    msg->setIsIntentionEcc(false);
    msg->setIsIntentionEcc(fromEmergencyDialer);

    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                       std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = resp->errorCode;
          }
          RadioResponseInfo respInfo = {};
          populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, errorCode);
          RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponseV1_4, emergencyDialResponse, respInfo);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
    RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponseV1_4, emergencyDialResponse, respInfo);
  }
  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

uint32_t toHidlCategories(uint32_t categories) {
    uint32_t cat = (uint32_t)V1_4::EmergencyServiceCategory::UNSPECIFIED;
    if (categories & (uint32_t)qcril::interfaces::RIL_EccCategoryMask::POLICE) {
        cat |= (uint32_t)V1_4::EmergencyServiceCategory::POLICE;
    }
    if (categories & (uint32_t)qcril::interfaces::RIL_EccCategoryMask::AMBULANCE) {
        cat |= (uint32_t)V1_4::EmergencyServiceCategory::AMBULANCE;
    }
    if (categories & (uint32_t)qcril::interfaces::RIL_EccCategoryMask::FIRE_BRIGADE) {
        cat |= (uint32_t)V1_4::EmergencyServiceCategory::FIRE_BRIGADE;
    }
    if (categories & (uint32_t)qcril::interfaces::RIL_EccCategoryMask::MARINE_GUARD) {
        cat |= (uint32_t)V1_4::EmergencyServiceCategory::MARINE_GUARD;
    }
    if (categories & (uint32_t)qcril::interfaces::RIL_EccCategoryMask::MOUNTAIN_RESCUE) {
        cat |= (uint32_t)V1_4::EmergencyServiceCategory::MOUNTAIN_RESCUE;
    }
    if (categories & (uint32_t)qcril::interfaces::RIL_EccCategoryMask::MANUAL_ECALL) {
        cat |= (uint32_t)V1_4::EmergencyServiceCategory::MIEC;
    }
    if (categories & (uint32_t)qcril::interfaces::RIL_EccCategoryMask::AUTO_ECALL) {
        cat |= (uint32_t)V1_4::EmergencyServiceCategory::AIEC;
    }
    return cat;
}

uint32_t toHidlSources(uint32_t sources) {
    uint32_t ecc_source  = 0;
    if (sources & (uint32_t)qcril::interfaces::RIL_EccNumberSourceMask::NETWORK_SIGNALING) {
        ecc_source |= (uint32_t)V1_4::EmergencyNumberSource::NETWORK_SIGNALING;
    }
    if (sources & (uint32_t)qcril::interfaces::RIL_EccNumberSourceMask::SIM) {
        ecc_source |= (uint32_t)V1_4::EmergencyNumberSource::SIM;
    }
    if (sources & (uint32_t)qcril::interfaces::RIL_EccNumberSourceMask::MODEM_CONFIG) {
        ecc_source |= (uint32_t)V1_4::EmergencyNumberSource::MODEM_CONFIG;
    }
    if (sources & (uint32_t)qcril::interfaces::RIL_EccNumberSourceMask::DEFAULT) {
        ecc_source |= (uint32_t)V1_4::EmergencyNumberSource::DEFAULT;
    }

    // If there is no ecc source, set it to modem config as this probably from RIL DB.
    if (ecc_source == 0) {
        ecc_source |= (uint32_t)V1_4::EmergencyNumberSource::MODEM_CONFIG;
    }
    return ecc_source;
}

int convertRilEccListToHal
(
    const qcril::interfaces::RIL_EccList& ril_ecc_list,
    hidl_vec<V1_4::EmergencyNumber>& ecc_list
)
{
    ecc_list.resize(ril_ecc_list.size());
    for (size_t i = 0; i < ril_ecc_list.size(); i++) {
        ecc_list[i].number = ril_ecc_list[i]->number;
        QCRIL_LOG_INFO("number - %s", ecc_list[i].number.c_str());
        ecc_list[i].mcc = ril_ecc_list[i]->mcc;
        QCRIL_LOG_INFO("mcc - %s", ecc_list[i].mcc.c_str());
        ecc_list[i].mnc = ril_ecc_list[i]->mnc;
        QCRIL_LOG_INFO("mnc - %s", ecc_list[i].mnc.c_str());
        ecc_list[i].categories = toHidlCategories((uint32_t)ril_ecc_list[i]->categories);
        QCRIL_LOG_INFO("categories - %d", ecc_list[i].categories);
        ecc_list[i].sources = toHidlSources((uint32_t)ril_ecc_list[i]->sources);
        QCRIL_LOG_INFO("sources - %d", ecc_list[i].sources);
        ecc_list[i].urns.resize(ril_ecc_list[i]->urns.size());
        for(size_t j = 0; j < ril_ecc_list[i]->urns.size(); j++) {
            ecc_list[i].urns[j] = ril_ecc_list[i]->urns[j];
            QCRIL_LOG_INFO("urn - %s", ecc_list[i].urns[j].c_str());
        }
    }
    return 0;
}

int RadioImpl_1_4::sendcurrentEmergencyNumberList(
    std::shared_ptr<RilUnsolEmergencyListIndMessage> msg)
{
    auto ind = mRadioIndicationV1_4;
    QCRIL_LOG_INFO("ind null: %s", ind ? "false" : "true" );
    if (ind) {
        hidl_vec<V1_4::EmergencyNumber> ecc_list;
        convertRilEccListToHal(msg->getEmergencyList(), ecc_list);

        QCRIL_LOG_INFO("currentEmergencyNumberList");
        Return<void> ret = ind->currentEmergencyNumberList(
            RadioIndicationType::UNSOLICITED, ecc_list);
        checkReturnStatus(ret);
    }
    QCRIL_LOG_FUNC_RETURN();
    return 0;
}

Return<void> RadioImpl_1_4::getPreferredNetworkTypeBitmap(int32_t serial) {
#if VDBG
    RLOGD("RadioImpl_1_4::getPreferredNetworkTypeBitmap: serial %d", serial);
#endif

    std::shared_ptr<RadioContext> ctx = getContext(serial);
    auto msg = std::make_shared<RilRequestGetPrefNetworkTypeMessage>(ctx);
    if (msg != nullptr) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                          std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            RadioResponseInfo respInfo = {};
            std::shared_ptr<qcril::interfaces::RilGetModePrefResult_t> prefResult;
            if (resp != nullptr) {
                populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, resp->errorCode);
                prefResult = std::static_pointer_cast<qcril::interfaces::RilGetModePrefResult_t>(resp->data);
            } else {
                populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, RIL_E_NO_MEMORY);
            }
            if (prefResult != nullptr) {
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponseV1_4, getPreferredNetworkTypeBitmapResponse,
                    respInfo, (hidl_bitfield<RadioAccessFamily>)prefResult->pref);
            } else {
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponseV1_4, getPreferredNetworkTypeBitmapResponse,
                    respInfo, (int)RadioAccessFamily::UNKNOWN/*default value*/);
            }
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponseV1_4, getPreferredNetworkTypeBitmapResponse,
            respInfo, (int)RadioAccessFamily::UNKNOWN/*default value*/);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl_1_4::setPreferredNetworkTypeBitmap(int32_t serial, hidl_bitfield<V1_4::RadioAccessFamily> networkTypeBitmap) {
#if VDGB
    RLOGD("RadioImpl_1_4::setPreferredNetworkTypeBitmap");
#endif

    std::shared_ptr<RadioContext> ctx = getContext(serial);
    auto msg = std::make_shared<RilRequestSetPrefNetworkTypeMessage>(ctx,
            static_cast<int>(networkTypeBitmap));
    if (msg != nullptr) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                          std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            RadioResponseInfo respInfo = {};
            if (resp != nullptr) {
                populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, resp->errorCode);
            } else {
                populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, RIL_E_NO_MEMORY);
            }
            RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponseV1_4, setPreferredNetworkTypeBitmapResponse, respInfo);
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponseV1_4, setPreferredNetworkTypeBitmapResponse, respInfo);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

void RadioImpl_1_4::fillDataRegistrationStateResponse(V1_4::DataRegStateResult &out, const RIL_DataRegistrationStateResponse &in) {
    RadioImpl::fillDataRegistrationStateResponse(out.base, in);
    if (in.lteVopsInfoValid)
    {
        V1_4::LteVopsInfo lvi;
        lvi.isVopsSupported = static_cast<bool>(in.lteVopsInfo.isVopsSupported);
        lvi.isEmcBearerSupported = static_cast<bool>(in.lteVopsInfo.isEmcBearerSupported);
        out.vopsInfo.lteVopsInfo(std::move(lvi));
    }

    if (in.nrIndicatorsValid)
    {
        out.nrIndicators.isEndcAvailable = static_cast<bool>(in.nrIndicators.isEndcAvailable);
        out.nrIndicators.isDcNrRestricted = static_cast<bool>(in.nrIndicators.isDcNrRestricted);
        out.nrIndicators.isNrAvailable = static_cast<bool>(in.nrIndicators.plmnInfoListR15Available);
    }
}

Return<void> RadioImpl_1_4::getDataRegistrationState(int32_t serial) {
    QCRIL_LOG_FUNC_ENTRY();
    #if VDBG
    RLOGD("RadioImpl_1_4::getDataRegistrationState: serial %d", serial);
    #endif

    qtimutex::QtiSharedMutex *radioServiceRwlockPtr = radio::getRadioServiceRwlock(mSlotId);
    radioServiceRwlockPtr->lock_shared();
    if (mRadioResponseV1_4 == nullptr)
    {
      radioServiceRwlockPtr->unlock_shared();
      QCRIL_LOG_INFO("mRadioResponseV1_4 == nullptr , fallback to v1_3 request");
      return RadioImpl_1_3::getDataRegistrationState(serial);
    }
    radioServiceRwlockPtr->unlock_shared();

    std::shared_ptr<RadioContext> ctx = getContext(serial);
    auto msg = std::make_shared<RilRequestGetDataRegistrationMessage>(ctx);
    if (msg != nullptr) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                          std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            if (resp != nullptr) {
                RadioResponseInfo respInfo = {};
                V1_4::DataRegStateResult dataRegResponse = {};
                populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, resp->errorCode);
                auto rilRegResult = std::static_pointer_cast<qcril::interfaces::RilGetDataRegResult_t>(resp->data);
                if (rilRegResult != nullptr) {
                    fillDataRegistrationStateResponse(dataRegResponse, rilRegResult->respData);
                }
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponseV1_4,
                        getDataRegistrationStateResponse_1_4, respInfo, dataRegResponse);
            }
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo {RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        V1_4::DataRegStateResult dataRegResponse = {};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponseV1_4,
                getDataRegistrationStateResponse_1_4, respInfo, dataRegResponse);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

void convertRilCellInfoToHal_1_4(V1_4::CellInfo &hidlCellInfo, const RIL_CellInfo_v12 &rilCellInfo) {
    CellInfoUnion_1_4 cellInfo;

    switch(rilCellInfo.cellInfoType) {
        case RIL_CELL_INFO_TYPE_GSM: {
            new (&cellInfo.gsm.cellIdentityGsm.base.mcc) hidl_string(
                    rilCellInfo.CellInfo.gsm.cellIdentityGsm.mcc,
                    strnlen(rilCellInfo.CellInfo.gsm.cellIdentityGsm.mcc,
                            sizeof(rilCellInfo.CellInfo.gsm.cellIdentityGsm.mcc) - 1));

            new (&cellInfo.gsm.cellIdentityGsm.base.mnc) hidl_string(
                    rilCellInfo.CellInfo.gsm.cellIdentityGsm.mnc,
                    strnlen(rilCellInfo.CellInfo.gsm.cellIdentityGsm.mnc,
                            sizeof(rilCellInfo.CellInfo.gsm.cellIdentityGsm.mnc) - 1));

            cellInfo.gsm.cellIdentityGsm.base.lac = rilCellInfo.CellInfo.gsm.cellIdentityGsm.lac;
            cellInfo.gsm.cellIdentityGsm.base.cid = rilCellInfo.CellInfo.gsm.cellIdentityGsm.cid;
            cellInfo.gsm.cellIdentityGsm.base.arfcn = rilCellInfo.CellInfo.gsm.cellIdentityGsm.arfcn;
            cellInfo.gsm.cellIdentityGsm.base.bsic = rilCellInfo.CellInfo.gsm.cellIdentityGsm.bsic;
            cellInfo.gsm.signalStrengthGsm.signalStrength = rilCellInfo.CellInfo.gsm.signalStrengthGsm.signalStrength;
            cellInfo.gsm.signalStrengthGsm.bitErrorRate = rilCellInfo.CellInfo.gsm.signalStrengthGsm.bitErrorRate;
            cellInfo.gsm.signalStrengthGsm.timingAdvance = rilCellInfo.CellInfo.gsm.signalStrengthGsm.timingAdvance;

            new (&cellInfo.gsm.cellIdentityGsm.operatorNames.alphaLong) hidl_string(
                    rilCellInfo.CellInfo.gsm.cellIdentityGsm.operatorNames.alphaLong,
                    strnlen(rilCellInfo.CellInfo.gsm.cellIdentityGsm.operatorNames.alphaLong,
                            sizeof(rilCellInfo.CellInfo.gsm.cellIdentityGsm.operatorNames.alphaLong) - 1));

            new (&cellInfo.gsm.cellIdentityGsm.operatorNames.alphaShort) hidl_string(
                    rilCellInfo.CellInfo.gsm.cellIdentityGsm.operatorNames.alphaShort,
                    strnlen(rilCellInfo.CellInfo.gsm.cellIdentityGsm.operatorNames.alphaShort,
                            sizeof(rilCellInfo.CellInfo.gsm.cellIdentityGsm.operatorNames.alphaShort) - 1));

            hidlCellInfo.info.gsm(std::move(cellInfo.gsm));
            break;
        }

        case RIL_CELL_INFO_TYPE_WCDMA: {
            new (&cellInfo.wcdma.cellIdentityWcdma.base.mcc) hidl_string(
                    rilCellInfo.CellInfo.wcdma.cellIdentityWcdma.mcc,
                    strnlen(rilCellInfo.CellInfo.wcdma.cellIdentityWcdma.mcc,
                            sizeof(rilCellInfo.CellInfo.wcdma.cellIdentityWcdma.mcc) - 1));

            new (&cellInfo.wcdma.cellIdentityWcdma.base.mnc) hidl_string(
                    rilCellInfo.CellInfo.wcdma.cellIdentityWcdma.mnc,
                    strnlen(rilCellInfo.CellInfo.wcdma.cellIdentityWcdma.mnc,
                            sizeof(rilCellInfo.CellInfo.wcdma.cellIdentityWcdma.mnc) - 1));

            cellInfo.wcdma.cellIdentityWcdma.base.lac = rilCellInfo.CellInfo.wcdma.cellIdentityWcdma.lac;
            cellInfo.wcdma.cellIdentityWcdma.base.cid = rilCellInfo.CellInfo.wcdma.cellIdentityWcdma.cid;
            cellInfo.wcdma.cellIdentityWcdma.base.psc = rilCellInfo.CellInfo.wcdma.cellIdentityWcdma.psc;
            cellInfo.wcdma.cellIdentityWcdma.base.uarfcn = rilCellInfo.CellInfo.wcdma.cellIdentityWcdma.uarfcn;
            cellInfo.wcdma.signalStrengthWcdma.base.signalStrength = rilCellInfo.CellInfo.wcdma.signalStrengthWcdma.signalStrength;
            cellInfo.wcdma.signalStrengthWcdma.base.bitErrorRate = rilCellInfo.CellInfo.wcdma.signalStrengthWcdma.bitErrorRate;
            cellInfo.wcdma.signalStrengthWcdma.rscp = rilCellInfo.CellInfo.wcdma.signalStrengthWcdma.rscp;
            cellInfo.wcdma.signalStrengthWcdma.ecno = rilCellInfo.CellInfo.wcdma.signalStrengthWcdma.ecio;

            new (&cellInfo.wcdma.cellIdentityWcdma.operatorNames.alphaLong) hidl_string(
                    rilCellInfo.CellInfo.wcdma.cellIdentityWcdma.operatorNames.alphaLong,
                    strnlen(rilCellInfo.CellInfo.wcdma.cellIdentityWcdma.operatorNames.alphaLong,
                            sizeof(rilCellInfo.CellInfo.wcdma.cellIdentityWcdma.operatorNames.alphaLong) - 1));

            new (&cellInfo.wcdma.cellIdentityWcdma.operatorNames.alphaShort) hidl_string(
                    rilCellInfo.CellInfo.wcdma.cellIdentityWcdma.operatorNames.alphaShort,
                    strnlen(rilCellInfo.CellInfo.wcdma.cellIdentityWcdma.operatorNames.alphaShort,
                            sizeof(rilCellInfo.CellInfo.wcdma.cellIdentityWcdma.operatorNames.alphaShort) - 1));

            hidlCellInfo.info.wcdma(std::move(cellInfo.wcdma));
            break;
        }

        case RIL_CELL_INFO_TYPE_CDMA: {
            cellInfo.cdma.cellIdentityCdma.base.networkId = rilCellInfo.CellInfo.cdma.cellIdentityCdma.networkId;
            cellInfo.cdma.cellIdentityCdma.base.systemId = rilCellInfo.CellInfo.cdma.cellIdentityCdma.systemId;
            cellInfo.cdma.cellIdentityCdma.base.baseStationId = rilCellInfo.CellInfo.cdma.cellIdentityCdma.basestationId;
            cellInfo.cdma.cellIdentityCdma.base.longitude = rilCellInfo.CellInfo.cdma.cellIdentityCdma.longitude;
            cellInfo.cdma.cellIdentityCdma.base.latitude = rilCellInfo.CellInfo.cdma.cellIdentityCdma.latitude;
            cellInfo.cdma.signalStrengthCdma.dbm = rilCellInfo.CellInfo.cdma.signalStrengthCdma.dbm;
            cellInfo.cdma.signalStrengthCdma.ecio = rilCellInfo.CellInfo.cdma.signalStrengthCdma.ecio;
            cellInfo.cdma.signalStrengthEvdo.dbm = rilCellInfo.CellInfo.cdma.signalStrengthEvdo.dbm;
            cellInfo.cdma.signalStrengthEvdo.ecio = rilCellInfo.CellInfo.cdma.signalStrengthEvdo.ecio;
            cellInfo.cdma.signalStrengthEvdo.signalNoiseRatio = rilCellInfo.CellInfo.cdma.signalStrengthEvdo.signalNoiseRatio;

            new (&cellInfo.cdma.cellIdentityCdma.operatorNames.alphaLong) hidl_string(
                    rilCellInfo.CellInfo.cdma.cellIdentityCdma.operatorNames.alphaLong,
                    strnlen(rilCellInfo.CellInfo.cdma.cellIdentityCdma.operatorNames.alphaLong,
                            sizeof(rilCellInfo.CellInfo.cdma.cellIdentityCdma.operatorNames.alphaLong) - 1));

            new (&cellInfo.cdma.cellIdentityCdma.operatorNames.alphaShort) hidl_string(
                    rilCellInfo.CellInfo.cdma.cellIdentityCdma.operatorNames.alphaShort,
                    strnlen(rilCellInfo.CellInfo.cdma.cellIdentityCdma.operatorNames.alphaShort,
                            sizeof(rilCellInfo.CellInfo.cdma.cellIdentityCdma.operatorNames.alphaShort) - 1));

            hidlCellInfo.info.cdma(std::move(cellInfo.cdma));
            break;
        }

        case RIL_CELL_INFO_TYPE_LTE: {
            new (&cellInfo.lte.base.cellIdentityLte.base.mcc) hidl_string(
                    rilCellInfo.CellInfo.lte.cellIdentityLte.mcc,
                    strnlen(rilCellInfo.CellInfo.lte.cellIdentityLte.mcc,
                            sizeof(rilCellInfo.CellInfo.lte.cellIdentityLte.mcc) - 1));

            new (&cellInfo.lte.base.cellIdentityLte.base.mnc) hidl_string(
                    rilCellInfo.CellInfo.lte.cellIdentityLte.mnc,
                    strnlen(rilCellInfo.CellInfo.lte.cellIdentityLte.mnc,
                            sizeof(rilCellInfo.CellInfo.lte.cellIdentityLte.mnc) - 1));

            cellInfo.lte.base.cellIdentityLte.base.ci = rilCellInfo.CellInfo.lte.cellIdentityLte.ci;
            cellInfo.lte.base.cellIdentityLte.base.pci = rilCellInfo.CellInfo.lte.cellIdentityLte.pci;
            cellInfo.lte.base.cellIdentityLte.base.tac = rilCellInfo.CellInfo.lte.cellIdentityLte.tac;
            cellInfo.lte.base.cellIdentityLte.base.earfcn = rilCellInfo.CellInfo.lte.cellIdentityLte.earfcn;
            cellInfo.lte.base.signalStrengthLte.signalStrength = rilCellInfo.CellInfo.lte.signalStrengthLte.signalStrength;
            cellInfo.lte.base.signalStrengthLte.rsrp = rilCellInfo.CellInfo.lte.signalStrengthLte.rsrp;
            cellInfo.lte.base.signalStrengthLte.rsrq = rilCellInfo.CellInfo.lte.signalStrengthLte.rsrq;
            cellInfo.lte.base.signalStrengthLte.rssnr = rilCellInfo.CellInfo.lte.signalStrengthLte.rssnr;
            cellInfo.lte.base.signalStrengthLte.cqi = rilCellInfo.CellInfo.lte.signalStrengthLte.cqi;
            cellInfo.lte.base.signalStrengthLte.timingAdvance = rilCellInfo.CellInfo.lte.signalStrengthLte.timingAdvance;

            new (&cellInfo.lte.base.cellIdentityLte.operatorNames.alphaLong) hidl_string(
                    rilCellInfo.CellInfo.lte.cellIdentityLte.operatorNames.alphaLong,
                    strnlen(rilCellInfo.CellInfo.lte.cellIdentityLte.operatorNames.alphaLong,
                            sizeof(rilCellInfo.CellInfo.lte.cellIdentityLte.operatorNames.alphaLong) - 1));

            new (&cellInfo.lte.base.cellIdentityLte.operatorNames.alphaShort) hidl_string(
                    rilCellInfo.CellInfo.lte.cellIdentityLte.operatorNames.alphaShort,
                    strnlen(rilCellInfo.CellInfo.lte.cellIdentityLte.operatorNames.alphaShort,
                            sizeof(rilCellInfo.CellInfo.lte.cellIdentityLte.operatorNames.alphaShort) - 1));

            cellInfo.lte.base.cellIdentityLte.bandwidth = rilCellInfo.CellInfo.lte.cellIdentityLte.bandwidth;
            // TODO: Populate ENDC availability information once modem's network scan and cell location info APIs support it
            cellInfo.lte.cellConfig.isEndcAvailable = false;
            hidlCellInfo.info.lte(std::move(cellInfo.lte));
            break;
        }

        case RIL_CELL_INFO_TYPE_TD_SCDMA: {
            new (&cellInfo.tdscdma.cellIdentityTdscdma.base.mcc) hidl_string(
                    rilCellInfo.CellInfo.tdscdma.cellIdentityTdscdma.mcc,
                    strnlen(rilCellInfo.CellInfo.tdscdma.cellIdentityTdscdma.mcc,
                            sizeof(rilCellInfo.CellInfo.tdscdma.cellIdentityTdscdma.mcc) - 1));

            new (&cellInfo.tdscdma.cellIdentityTdscdma.base.mnc) hidl_string(
                    rilCellInfo.CellInfo.tdscdma.cellIdentityTdscdma.mnc,
                    strnlen(rilCellInfo.CellInfo.tdscdma.cellIdentityTdscdma.mnc,
                            sizeof(rilCellInfo.CellInfo.tdscdma.cellIdentityTdscdma.mnc) - 1));

            cellInfo.tdscdma.cellIdentityTdscdma.base.lac = rilCellInfo.CellInfo.tdscdma.cellIdentityTdscdma.lac;
            cellInfo.tdscdma.cellIdentityTdscdma.base.cid = rilCellInfo.CellInfo.tdscdma.cellIdentityTdscdma.cid;
            cellInfo.tdscdma.cellIdentityTdscdma.base.cpid = rilCellInfo.CellInfo.tdscdma.cellIdentityTdscdma.cpid;


            new (&cellInfo.tdscdma.cellIdentityTdscdma.operatorNames.alphaLong) hidl_string(
                    rilCellInfo.CellInfo.tdscdma.cellIdentityTdscdma.operatorNames.alphaLong,
                    strnlen(rilCellInfo.CellInfo.tdscdma.cellIdentityTdscdma.operatorNames.alphaLong,
                            sizeof(rilCellInfo.CellInfo.tdscdma.cellIdentityTdscdma.operatorNames.alphaLong) - 1));

            new (&cellInfo.tdscdma.cellIdentityTdscdma.operatorNames.alphaShort) hidl_string(
                    rilCellInfo.CellInfo.tdscdma.cellIdentityTdscdma.operatorNames.alphaShort,
                    strnlen(rilCellInfo.CellInfo.tdscdma.cellIdentityTdscdma.operatorNames.alphaShort,
                            sizeof(rilCellInfo.CellInfo.tdscdma.cellIdentityTdscdma.operatorNames.alphaShort) - 1));

            cellInfo.tdscdma.cellIdentityTdscdma.uarfcn = 0; //default value
            cellInfo.tdscdma.signalStrengthTdscdma.rscp = rilCellInfo.CellInfo.tdscdma.signalStrengthTdscdma.rscp;
            hidlCellInfo.info.tdscdma(std::move(cellInfo.tdscdma));
            break;
        }

        case RIL_CELL_INFO_TYPE_NR: {
            new (&cellInfo.nr.cellidentity.mcc) hidl_string(
                    rilCellInfo.CellInfo.nr.cellIdentityNr.mcc,
                    strnlen(rilCellInfo.CellInfo.nr.cellIdentityNr.mcc,
                            sizeof(rilCellInfo.CellInfo.nr.cellIdentityNr.mcc) - 1));

            new (&cellInfo.nr.cellidentity.mnc) hidl_string(
                    rilCellInfo.CellInfo.nr.cellIdentityNr.mnc,
                    strnlen(rilCellInfo.CellInfo.nr.cellIdentityNr.mnc,
                            sizeof(rilCellInfo.CellInfo.nr.cellIdentityNr.mnc) - 1));

            cellInfo.nr.cellidentity.nci = rilCellInfo.CellInfo.nr.cellIdentityNr.nci;
            cellInfo.nr.cellidentity.pci = rilCellInfo.CellInfo.nr.cellIdentityNr.pci;
            cellInfo.nr.cellidentity.tac = rilCellInfo.CellInfo.nr.cellIdentityNr.tac;
            cellInfo.nr.cellidentity.nrarfcn = rilCellInfo.CellInfo.nr.cellIdentityNr.nrarfcn;

            new (&cellInfo.nr.cellidentity.operatorNames.alphaLong) hidl_string(
                    rilCellInfo.CellInfo.nr.cellIdentityNr.operatorNames.alphaLong,
                    strnlen(rilCellInfo.CellInfo.nr.cellIdentityNr.operatorNames.alphaLong,
                            sizeof(rilCellInfo.CellInfo.nr.cellIdentityNr.operatorNames.alphaLong) - 1));

            new (&cellInfo.nr.cellidentity.operatorNames.alphaShort) hidl_string(
                    rilCellInfo.CellInfo.nr.cellIdentityNr.operatorNames.alphaShort,
                    strnlen(rilCellInfo.CellInfo.nr.cellIdentityNr.operatorNames.alphaShort,
                            sizeof(rilCellInfo.CellInfo.nr.cellIdentityNr.operatorNames.alphaShort) - 1));

            cellInfo.nr.signalStrength.ssRsrp = rilCellInfo.CellInfo.nr.signalStrengthNr.ssRsrp;
            cellInfo.nr.signalStrength.ssRsrq = rilCellInfo.CellInfo.nr.signalStrengthNr.ssRsrq;
            cellInfo.nr.signalStrength.ssSinr = rilCellInfo.CellInfo.nr.signalStrengthNr.ssSinr;
            cellInfo.nr.signalStrength.csiRsrp = rilCellInfo.CellInfo.nr.signalStrengthNr.csiRsrp;
            cellInfo.nr.signalStrength.csiRsrq = rilCellInfo.CellInfo.nr.signalStrengthNr.csiRsrq;
            cellInfo.nr.signalStrength.csiSinr = rilCellInfo.CellInfo.nr.signalStrengthNr.csiSinr;
            hidlCellInfo.info.nr(std::move(cellInfo.nr));
            break;
        }

        default: {
            break;
        }
    }
}

void convertRilCellInfoListToHal_1_4(const std::vector<RIL_CellInfo_v12>& rillCellInfo, hidl_vec<V1_4::CellInfo>& records) {
    auto num = rillCellInfo.size();
    records.resize(num);

    for (unsigned int i = 0; i < num; i++) {
        records[i].isRegistered = rillCellInfo[i].registered;
        records[i].connectionStatus = (V1_2::CellConnectionStatus) rillCellInfo[i].connStatus;
        convertRilCellInfoToHal_1_4(records[i], rillCellInfo[i]);
    }
}

void convertRilCellInfoListToHal_1_4(void *response, size_t responseLen, hidl_vec<V1_4::CellInfo>& records) {
    int num = responseLen / sizeof(RIL_CellInfo_v12);
    records.resize(num);

    RIL_CellInfo_v12 *rillCellInfo = (RIL_CellInfo_v12*) response;
    if (rillCellInfo) {
        for (int i = 0; i < num; i++) {
            records[i].isRegistered = rillCellInfo[i].registered;
            records[i].connectionStatus = (V1_2::CellConnectionStatus) rillCellInfo[i].connStatus;
            convertRilCellInfoToHal_1_4(records[i], rillCellInfo[i]);
        }
    }
}

Return<void> RadioImpl_1_4::startNetworkScan_1_4(int32_t serial, const V1_2::NetworkScanRequest& request) {
    QCRIL_LOG_FUNC_ENTRY();
    #if VDBG
    RLOGD("startNetworkScan_1_4: serial %d", serial);
    #endif

    std::shared_ptr<RilRequestStartNetworkScanMessage> msg;
    RIL_NetworkScanRequest scanRequest = {};
    auto res = fillNetworkScanRequest_1_2(request, scanRequest);
    if (res == RadioError::NONE) {
        std::shared_ptr<RadioContext> ctx = getContext(serial);
        if ((msg = std::make_shared<RilRequestStartNetworkScanMessage>(ctx, scanRequest)) == nullptr) {
            res = RadioError::NO_MEMORY;
        }
    }

    if (msg != nullptr) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            [this, serial] (std::shared_ptr<Message> msg, Message::Callback::Status status,
                          std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            RadioResponseInfo respInfo = {};
            if (resp != nullptr) {
                populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, resp->errorCode);
            } else {
                populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, RIL_E_NO_MEMORY);
            }
            RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponseV1_4, startNetworkScanResponse_1_4, respInfo);
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo {RadioResponseType::SOLICITED, serial, res};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponseV1_4, startNetworkScanResponse_1_4, respInfo);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

int RadioImpl_1_4::sendNetworkScanResult(std::shared_ptr<RilUnsolNetworkScanResultMessage> msg) {
    QCRIL_LOG_FUNC_ENTRY();

    auto ind = mRadioIndicationV1_4;
    QCRIL_LOG_INFO("ind null: %s", ind ? "false" : "true" );

    if (ind) {
        V1_4::NetworkScanResult resultV1_4;
        resultV1_4.status = (V1_1::ScanStatus) msg->getStatus();
        resultV1_4.error = (V1_0::RadioError) msg->getError();

        convertRilCellInfoListToHal_1_4(msg->getNetworkInfo(), resultV1_4.networkInfos);
        Return<void> retStatus = ind->networkScanResult_1_4(
                RadioIndicationType::UNSOLICITED, resultV1_4);
        checkReturnStatus(retStatus);
    } else {
        RadioImpl_1_3::sendNetworkScanResult(msg);
    }

    QCRIL_LOG_FUNC_RETURN();
    return 0;
}

int RadioImpl_1_4::sendGetCellInfoListResponse(int slotId, int responseType,
        int serial, RIL_Errno e, void *response, size_t responseLen) {
    QCRIL_LOG_FUNC_ENTRY();
    #if VDBG
    RLOGD("getCellInfoListResponse: serial %d", serial);
    #endif

    auto rsp = mRadioResponseV1_4;
    if (rsp) {
        RadioResponseInfo responseInfo = {};
        populateResponseInfo(responseInfo, serial, responseType, e);

        hidl_vec<V1_4::CellInfo> records;
        if ((response == NULL && responseLen != 0)
                || responseLen % sizeof(RIL_CellInfo_v12) != 0) {
            RLOGE("getCellInfoListResponse: Invalid response");
            if (e == RIL_E_SUCCESS) responseInfo.error = RadioError::INVALID_RESPONSE;
        } else {
            convertRilCellInfoListToHal_1_4(response, responseLen, records);
        }

        Return<void> retStatus;
        retStatus = rsp->getCellInfoListResponse_1_4(responseInfo, records);
        checkReturnStatus(retStatus);
    } else {
        RadioImpl_1_3::sendGetCellInfoListResponse(slotId, responseType, serial,
                e, response, responseLen);
    }

    QCRIL_LOG_FUNC_RETURN();
    return 0;
}

int RadioImpl_1_4::sendCellInfoListInd(int slotId, int indicationType, int token,
        RIL_Errno e, void *response, size_t responseLen)
{
    QCRIL_LOG_FUNC_ENTRY();

    if (mRadioIndicationV1_4) {
        hidl_vec<V1_4::CellInfo> records;
        convertRilCellInfoListToHal_1_4(response, responseLen, records);
        Return<void> retStatus = mRadioIndicationV1_4->cellInfoList_1_4(
                convertIntToRadioIndicationType(indicationType), records);
        checkReturnStatus(retStatus);
    } else {
        RLOGE("cellInfoListInd: radioService[%d]->mRadioIndication == NULL", slotId);
        RadioImpl_1_3::sendCellInfoListInd(slotId, indicationType, token, e,
                response, responseLen);
    }

    QCRIL_LOG_FUNC_RETURN();
    return 0;
}

void convertToHal(V1_4::NrSignalStrength &out, const RIL_NR_SignalStrength &in) {
    out.ssRsrp = in.ssRsrp;
    out.ssRsrq = in.ssRsrq;
    out.ssSinr = in.ssSinr;
    out.csiRsrp = in.csiRsrp;
    out.csiRsrq = in.csiRsrq;
    out.csiSinr = in.csiSinr;
}

void convertRilSignalStrengthToHal(const RIL_SignalStrength &rilSignalStrength,
        V1_4::SignalStrength &signalStrength) {
    convertToHal(signalStrength.gsm, rilSignalStrength.GW_SignalStrength);
    convertToHal(signalStrength.wcdma, rilSignalStrength.WCDMA_SignalStrength);
    convertToHal(signalStrength.cdma, rilSignalStrength.CDMA_SignalStrength);
    convertToHal(signalStrength.evdo, rilSignalStrength.EVDO_SignalStrength);
    convertToHal(signalStrength.lte, rilSignalStrength.LTE_SignalStrength);
    convertToHal(signalStrength.tdscdma, rilSignalStrength.TD_SCDMA_SignalStrength);
    convertToHal(signalStrength.nr, rilSignalStrength.NR_SignalStrength);
}

Return<void> RadioImpl_1_4::getSignalStrength_1_4(int32_t serial) {
    QCRIL_LOG_FUNC_ENTRY();
    #if VDBG
    RLOGD("getSignalStrength: serial %d", serial);
    #endif

    std::shared_ptr<RadioContext> ctx = getContext(serial);
    auto msg = std::make_shared<RilRequestGetSignalStrengthMessage>(ctx);
    if (msg != nullptr) {
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            [this, serial](std::shared_ptr<Message> msg, Message::Callback::Status status,
                          std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
            if (resp != nullptr) {
                RadioResponseInfo respInfo = {};
                V1_4::SignalStrength signalStrength = {};
                populateResponseInfo(respInfo, serial, RESPONSE_SOLICITED, resp->errorCode);
                auto rilSigResult = std::static_pointer_cast<qcril::interfaces::RilGetSignalStrengthResult_t>(resp->data);
                if (rilSigResult != nullptr) {
                    convertRilSignalStrengthToHal(rilSigResult->respData, signalStrength);
                }
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponseV1_4,
                        getSignalStrengthResponse_1_4, respInfo, signalStrength);
            }
        });
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo respInfo {RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        V1_4::SignalStrength signalStrength = {};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponseV1_4,
                getSignalStrengthResponse_1_4, respInfo, signalStrength);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

int RadioImpl_1_4::sendSignalStrength(std::shared_ptr<RilUnsolSignalStrengthMessage> msg) {
    QCRIL_LOG_FUNC_ENTRY();

    auto ind = mRadioIndicationV1_4;
    QCRIL_LOG_INFO("ind null: %s", ind ? "false" : "true");

    if (ind) {
        V1_4::SignalStrength signalStrength = {};
        convertRilSignalStrengthToHal(msg->getSignalStrength(), signalStrength);
        Return<void> retStatus = ind->currentSignalStrength_1_4(
                RadioIndicationType::UNSOLICITED, signalStrength);
        checkReturnStatus(retStatus);
    } else {
        RadioImpl_1_3::sendSignalStrength(msg);
    }

    QCRIL_LOG_FUNC_RETURN();
    return 0;
}

Return<void> RadioImpl_1_4::setAllowedCarriers_1_4(int32_t serial, const V1_4::CarrierRestrictionsWithPriority &carriers,
        V1_4::SimLockMultiSimPolicy multiSimPolicy) {
    QCRIL_LOG_FUNC_ENTRY();

    if (mRadioResponseV1_4) {
        RadioResponseInfo responseInfo {};
        populateResponseInfo(responseInfo, serial, RESPONSE_SOLICITED, RadioError::REQUEST_NOT_SUPPORTED);
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponseV1_4, setAllowedCarriersResponse_1_4, responseInfo);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl_1_4::getAllowedCarriers_1_4(int32_t serial) {
    QCRIL_LOG_FUNC_ENTRY();

    if (mRadioResponseV1_4) {
        RadioResponseInfo responseInfo {};
        populateResponseInfo(responseInfo, serial, RESPONSE_SOLICITED, RadioError::REQUEST_NOT_SUPPORTED);
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponseV1_4, getAllowedCarriersResponse_1_4, responseInfo, {},
                V1_4::SimLockMultiSimPolicy::NO_MULTISIM_POLICY);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> RadioImpl_1_4::getIccCardStatus(int32_t serial) {
#if VDBG
        RLOGD("getIccCardStatus: serial %d", serial);
#endif

  qtimutex::QtiSharedMutex *radioServiceRwlockPtr = radio::getRadioServiceRwlock(mSlotId);
  radioServiceRwlockPtr->lock_shared();

  if (mRadioResponseV1_4 == nullptr)
  {
    radioServiceRwlockPtr->unlock_shared();
    return RadioImpl_1_3::getIccCardStatus(serial);
  }

  radioServiceRwlockPtr->unlock_shared();

  auto msg = std::make_shared<UimGetCardStatusRequestMsg>(qmi_ril_get_process_instance_id());
    if (msg) {
        GenericCallback<RIL_UIM_CardStatus> cb(
            ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                            std::shared_ptr<RIL_UIM_CardStatus> responseDataPtr) -> void {
                RadioResponseInfo responseInfo{RadioResponseType::SOLICITED, serial, RadioError::INTERNAL_ERR};
                V1_4::CardStatus cardStatus = {};

                if (solicitedMsg && responseDataPtr &&
                    status == Message::Callback::Status::SUCCESS) {
                    convertGetIccCardStatusResponse(responseInfo, cardStatus.base.base,
                            RESPONSE_SOLICITED, serial, responseDataPtr);
                    cardStatus.base.physicalSlotId = responseDataPtr->physical_slot_id;
                    cardStatus.base.atr            = responseDataPtr->atr;
                    cardStatus.base.iccid          = responseDataPtr->iccid;

                    if(!responseDataPtr->eid.empty())
                    {
                      cardStatus.eid               = responseDataPtr->eid;
                    }
                }
                RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponseV1_4, getIccCardStatusResponse_1_4,
                        responseInfo, cardStatus);
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        RadioResponseInfo rsp{RadioResponseType::SOLICITED, serial, RadioError::NO_MEMORY};
        V1_4::CardStatus cardStatus = {};
        RADIO_HIDL_SEND_RESPONSE(mSlotId, mRadioResponseV1_4, getIccCardStatusResponse_1_4, rsp, cardStatus);
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

std::shared_ptr<RadioContext> RadioImpl_1_4::getContext(int32_t serial)
{
    std::shared_ptr<RadioContext> ctx;
    ctx = std::make_shared<RadioContext>(sp<RadioImpl_1_4>(this), serial);
    return ctx;
}

void RadioImpl_1_4::dataCallListChanged(RadioIndicationType indType, hidl_vec<V1_4::SetupDataCallResult> dcResultList) {
    QCRIL_LOG_FUNC_ENTRY();

    auto ind = mRadioIndicationV1_4;
    if (ind) {
        QCRIL_LOG_INFO("dataCallListChanged");
        Return<void> retStatus = ind->dataCallListChanged_1_4(indType, dcResultList);
        checkReturnStatus(retStatus);
    } else {
        RLOGE("dataCallListChanged: radioService[%d]->mRadioResponse == NULL", mSlotId);
    }

    QCRIL_LOG_FUNC_RETURN();
}

void RadioImpl_1_4::currentPhysicalChannelConfigs(RadioIndicationType indType, hidl_vec<V1_4::PhysicalChannelConfig> physChanConfig) {
    QCRIL_LOG_FUNC_ENTRY();

    auto rsp = mRadioIndicationV1_4;
    if (rsp) {
        std::shared_lock<qtimutex::QtiSharedMutex> sl(*radio::getRadioServiceRwlock(mSlotId));

         Return<void> retStatus = rsp->currentPhysicalChannelConfigs_1_4(indType, physChanConfig);
         checkReturnStatus(retStatus);
    }
    else {
        std::shared_ptr<std::vector<NasPhysChanInfo>> physChanInfo =
                std::make_shared<std::vector<NasPhysChanInfo>>();
        if(physChanInfo == nullptr) {
            RLOGE("currentPhysicalChannelConfigs: Failed to create vector.");
            return;
        }
        for (auto entry: physChanConfig) {
            if(entry.base.status == V1_2::CellConnectionStatus::PRIMARY_SERVING) {
                physChanInfo->push_back(NasPhysChanInfo {NasPhysChanInfo::Status::PRIMARY,
                                                        entry.base.cellBandwidthDownlink,
                                                        entry.physicalCellId});
            }
            else {
                physChanInfo->push_back(NasPhysChanInfo {NasPhysChanInfo::Status::SECONDARY,
                                                        entry.base.cellBandwidthDownlink,
                                                        entry.physicalCellId});
            }
        }
        std::shared_ptr<NasPhysChanConfigMessage> msg = std::make_shared<NasPhysChanConfigMessage>(physChanInfo);
        if(msg) {
            RadioImpl_1_2::sendCurrentPhysicalChannelConfigs(msg);
        }
    }

    QCRIL_LOG_FUNC_RETURN();
}

void RadioImpl_1_4::setupDataCallResponse_1_4(RadioResponseInfo responseInfo, V1_4::SetupDataCallResult dcResult) {
    QCRIL_LOG_FUNC_ENTRY();

    auto rsp = mRadioResponseV1_4;
    if (rsp) {
        QCRIL_LOG_INFO("setupDataCallResponse_1_4");
        Return<void> retStatus = rsp->setupDataCallResponse_1_4(responseInfo, dcResult);
        checkReturnStatus(retStatus);
    } else {
        RLOGE("setupDataCallResponse_1_4: radioService[%d]->mRadioResponse == NULL", mSlotId);
    }

    QCRIL_LOG_FUNC_RETURN();
}

void RadioImpl_1_4::deactivateDataCallResponse(RadioResponseInfo responseInfo) {
    QCRIL_LOG_FUNC_ENTRY();

    auto rsp = mRadioResponseV1_4;
    if (rsp) {
        QCRIL_LOG_INFO("deactivateDataCallResponse");
        Return<void> retStatus = rsp->deactivateDataCallResponse(responseInfo);
        checkReturnStatus(retStatus);
    } else {
        RLOGE("deactivateDataCallResponse: radioService[%d]->mRadioResponse == NULL", mSlotId);
    }

    QCRIL_LOG_FUNC_RETURN();
}

::android::status_t RadioImpl_1_4::registerAsService(const std::string &serviceName) {
    QCRIL_LOG_INFO("RadioImpl_1_4::registerAsService(%s)", serviceName.c_str());
    createRilServiceModule();
    return V1_4::IRadio::registerAsService(serviceName);
}

static void __attribute__((constructor)) registerRadioImpl_1_4();
void registerRadioImpl_1_4()
{
    QCRIL_LOG_INFO("Calling registerRadioImpl_1_4");
    getQcrildServiceFactory().registerRadioImpl<RadioImpl_1_4>(nullptr);
}

Module* RadioImpl_1_4::getRilServiceModule() {
    return mModule;
}

void RadioImpl_1_4::createRilServiceModule() {
    mModule = new RilServiceModule_1_4(getInstanceId(), *this);
    if (mModule != nullptr) {
        mModule->init();
    }
}

const QcrildServiceVersion &RadioImpl_1_4::getVersion() {
    static QcrildServiceVersion version(1, 4);
    return version;
}

RadioImpl_1_4::RadioImpl_1_4(qcril_instance_id_e_type instance): RadioImpl_1_3(instance) {
    QCRIL_LOG_INFO("RadioImpl_1_4()");
}
