/*
 * Copyright (c) 2019, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <unistd.h>
#include "bta_ag_swb.h"
#include "internal_include/bt_trace.h"
#include "bta_ag_int.h"
#include "utl.h"
#include "device/include/interop.h"
#include <hardware/vendor_hf.h>
#include <cutils/properties.h>

#if (SWB_ENABLED == TRUE)

#define SWB_CODECS_SUPPORTED "0,4,6,7"
#define SWB_CODECS_UNSUPPORTD "0xFFFF"

void bta_ag_swb_handle_vs_at_events(tBTA_AG_SCB* p_scb, uint16_t cmd, int16_t int_arg, tBTA_AG_VAL val)
{
  APPL_TRACE_DEBUG("%s: p_scb : %x cmd : %d", __func__, p_scb, cmd);
  switch(cmd) {
    case BTA_AG_AT_QAC_EVT:
      if (!get_swb_codec_status()) {
        bta_ag_send_qac(p_scb, NULL);
        break;
      }
      p_scb->codec_updated = true;
      if (p_scb->peer_codecs &  BTA_AG_SCO_SWB_SETTINGS_Q0_MASK) {
        p_scb->sco_codec = BTA_AG_SCO_SWB_SETTINGS_Q0;
      } else if (p_scb->peer_codecs & BTA_AG_CODEC_MSBC) {
        p_scb->sco_codec = UUID_CODEC_MSBC;
      }
      bta_ag_send_qac(p_scb, NULL);
      APPL_TRACE_DEBUG("Received AT+QAC, updating sco codec to SWB: %d", p_scb->sco_codec);
      val.num = p_scb->peer_codecs;
      break;
    case BTA_AG_AT_QCS_EVT: {
      tBTA_AG_PEER_CODEC codec_type, codec_sent;
      alarm_cancel(p_scb->codec_negotiation_timer);

      switch (int_arg) {
        case BTA_AG_SCO_SWB_SETTINGS_Q0:
          codec_type = BTA_AG_SCO_SWB_SETTINGS_Q0;
          break;
        case BTA_AG_SCO_SWB_SETTINGS_Q1:
          codec_type = BTA_AG_SCO_SWB_SETTINGS_Q1;
          break;
        case BTA_AG_SCO_SWB_SETTINGS_Q2:
          codec_type = BTA_AG_SCO_SWB_SETTINGS_Q2;
          break;
        case BTA_AG_SCO_SWB_SETTINGS_Q3:
          codec_type = BTA_AG_SCO_SWB_SETTINGS_Q3;
          break;
        default:
          APPL_TRACE_ERROR("Unknown codec_uuid %d", int_arg);
          p_scb->is_swb_codec = false;
          codec_type = BTA_AG_CODEC_MSBC;
          p_scb->codec_fallback = true;
          p_scb->sco_codec = BTA_AG_CODEC_MSBC;
          break;
      }

      if (p_scb->codec_fallback)
        codec_sent = BTA_AG_CODEC_MSBC;
      else
        codec_sent = p_scb->sco_codec;

      if (codec_type == codec_sent)
        bta_ag_sco_codec_nego(p_scb, true);
      else
        bta_ag_sco_codec_nego(p_scb, false);

      /* send final codec info to callback */
      val.num = codec_sent;
      break;
    }
  }
}

/*******************************************************************************
 *
 * Function         bta_ag_send_qcs
 *
 * Description      Send +%QCS AT command to peer.
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_ag_send_qcs(tBTA_AG_SCB* p_scb, tBTA_AG_DATA* p_data) {
  uint16_t codec_uuid;
  if (p_scb->codec_fallback) {
    if (p_scb->peer_codecs & BTA_AG_CODEC_MSBC) {
      codec_uuid = UUID_CODEC_MSBC;
    } else {
      codec_uuid = UUID_CODEC_CVSD;
    }
  } else {
    codec_uuid = BTA_AG_SCO_SWB_SETTINGS_Q0;
  }

  /* send +BCS */
  APPL_TRACE_DEBUG("send +QCS codec is %d", codec_uuid);
  bta_ag_send_result(p_scb, BTA_AG_LOCAL_RES_QCS, NULL, codec_uuid);
}

/*******************************************************************************
 *
 * Function         bta_ag_send_qac
 *
 * Description      Send +%QAC AT command to peer.
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_ag_send_qac(tBTA_AG_SCB* p_scb, tBTA_AG_DATA* p_data) {
  char value[PROPERTY_VALUE_MAX];
  /* send +QAC */
  APPL_TRACE_DEBUG("send +QAC codec response");

  if (property_get("persist.vendor.qcom.bluetooth.enable.swb", value, "true")
      && strncmp(value, "true", sizeof("true")) != 0) {
    bta_ag_send_result(p_scb, BTA_AG_LOCAL_RES_QAC, SWB_CODECS_UNSUPPORTD, 0);
    return;
  }
  bta_ag_send_result(p_scb, BTA_AG_LOCAL_RES_QAC, SWB_CODECS_SUPPORTED, 0);
  if (get_swb_codec_status() && p_scb->sco_codec == BTA_AG_SCO_SWB_SETTINGS_Q0) {
      p_scb->is_swb_codec = true;
  }
}

tBTA_AG_PEER_CODEC bta_ag_parse_qac(tBTA_AG_SCB* p_scb, char* p_s) {
  tBTA_AG_PEER_CODEC retval = BTA_AG_CODEC_NONE;
  uint16_t codec_modes;
  bool cont = false; /* Continue processing */
  char* p;

  while (p_s) {
    /* skip to comma delimiter */
    for (p = p_s; *p != ',' && *p != 0; p++)
      ;

    /* get integre value */
    if (*p != 0) {
      *p = 0;
      cont = true;
    } else
      cont = false;

    codec_modes = utl_str2int(p_s);
    switch (codec_modes) {
      case BTA_AG_SCO_SWB_SETTINGS_Q0:
        retval |= BTA_AG_SCO_SWB_SETTINGS_Q0_MASK;
        break;
      case BTA_AG_SCO_SWB_SETTINGS_Q1:
        retval |= BTA_AG_SCO_SWB_SETTINGS_Q1_MASK;
        break;
      case BTA_AG_SCO_SWB_SETTINGS_Q2:
        retval |= BTA_AG_SCO_SWB_SETTINGS_Q2_MASK;
        break;
      case BTA_AG_SCO_SWB_SETTINGS_Q3:
        retval |= BTA_AG_SCO_SWB_SETTINGS_Q3_MASK;
        break;
      default:
        APPL_TRACE_ERROR("Unknown Codec UUID(%d) received", codec_modes);
        break;
    }

    if (cont)
      p_s = p + 1;
    else
      break;
  }

  return (retval);
}
#endif //#if (SWB_ENABLED == TRUE)

