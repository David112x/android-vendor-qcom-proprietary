/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_ims_payload.cpp
  @brief   Message print functions and validation for IMS messages

  DESCRIPTION
  ---------------------------------------------------------------------------
******************************************************************************/
#include "ril_utf_ims_payload.h"
#include <string.h>

void print_ims_CallList_Call (ims_CallList_Call *call)
{
  RIL_UTF_ESSENTIAL("\nIMS CALL DUMP:");
  RIL_UTF_ESSENTIAL("\nhas_state %d state %d", call->has_state, call->state);
  RIL_UTF_ESSENTIAL("\nhas_index %d index %d", call->has_index, call->index);
  RIL_UTF_ESSENTIAL("\nhas_toa %d toa %d", call->has_toa, call->toa);
  RIL_UTF_ESSENTIAL("\nhas_isMpty %d isMpty %d", call->has_isMpty, call->isMpty);
  RIL_UTF_ESSENTIAL("\nhas_isMT %d isMT %d", call->has_isMT, call->isMT);
  RIL_UTF_ESSENTIAL("\nhas_als %d als %d", call->has_als, call->als);
  RIL_UTF_ESSENTIAL("\nhas_isVoice %d isVoice %d", call->has_isVoice, call->isVoice);
  RIL_UTF_ESSENTIAL("\nhas_isVoicePrivacy %d isVoicePrivacy %d", call->has_isVoicePrivacy, call->isVoicePrivacy);
  RIL_UTF_ESSENTIAL("\nhas_numberPresentation %d, numberPresentation = %d", call->has_numberPresentation, call->numberPresentation);
  RIL_UTF_ESSENTIAL("\nhas_namePresentation %d, namePresentation = %d", call->has_namePresentation, call->namePresentation);
  RIL_UTF_ESSENTIAL("\nhas_isEncrypted %d, isEncrypted = %d", call->has_isEncrypted, call->isEncrypted);
  RIL_UTF_ESSENTIAL("\nhas_isCalledPartyRinging %d isCalledPartyRinging %d", call->has_isCalledPartyRinging, call->isCalledPartyRinging);

  RIL_UTF_ESSENTIAL("\nhas_callDetails %d", call->has_callDetails);
  RIL_UTF_ESSENTIAL("\ncallDetails.has_callType %d callDetails.callType %d",
                    call->callDetails.has_callType, call->callDetails.callType);
  RIL_UTF_ESSENTIAL("\ncallDetails.has_callDomain %d callDetails.callDomain %d",
                    call->callDetails.has_callDomain, call->callDetails.callDomain);
  RIL_UTF_ESSENTIAL("\ncallDetails.has_callSubstate %d callDetails.callSubstate %d",
                    call->callDetails.has_callSubstate, call->callDetails.callSubstate);
  RIL_UTF_ESSENTIAL("\ncallDetails.has_mediaId %d callDetails.mediaId %d",
                    call->callDetails.has_mediaId, call->callDetails.mediaId);
  if (call->callDetails.extras.arg != NULL)
  {
    int i = 0;
    char **extras = (char **)(call->callDetails.extras.arg);
    while (extras[i])
    {
      RIL_UTF_ESSENTIAL("\ncallDetails.extras[%d]: %s", i, extras[i]);
      i++;
    }
  }

  RIL_UTF_ESSENTIAL("\nhas_failCause %d", call->has_failCause);
  RIL_UTF_ESSENTIAL("\nfailCause.has_failcause %d", call->failCause.has_failcause);
  RIL_UTF_ESSENTIAL("\nfailCause.failcause %d", call->failCause.failcause);
  RIL_UTF_ESSENTIAL("\nfailCause.errorDetails.sipErrorCode %d", call->failCause.errorDetails.sipErrorCode);
  RIL_UTF_ESSENTIAL("\nhas_peerConfAbility %d", call->has_peerConfAbility);
  RIL_UTF_ESSENTIAL("\npeerConfAbility.has_isVideoConfSupported %d peerConfAbility.isVideoConfSupported %d",
      call->peerConfAbility.has_isVideoConfSupported, call->peerConfAbility.isVideoConfSupported);
}
