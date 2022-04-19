/******************************************************************************
 * Copyright (C) 2017, The Linux Foundation. All rights reserved.
 * Not a Contribution.
 ******************************************************************************/
/******************************************************************************
 *
 *  Copyright (C) 2002-2012 Broadcom Corporation
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

/******************************************************************************
 *
 *  This module contains the action functions associated with the stream
 *  control block state machine.
 *
 ******************************************************************************/

#include <string.h>
#include "a2dp_codec_api.h"
#include "avdt_api.h"
#include "avdt_int.h"
#include "avdtc_api.h"
#include "bt_common.h"
#include "bt_target.h"
#include "bt_types.h"
#include "bt_utils.h"
#include "btu.h"
#include "btif/include/btif_av.h"
#include "osi/include/osi.h"
#include "stack/include/a2dp_sbc_constants.h"

/* This table is used to lookup the callback event that matches a particular
 * state machine API request event.  Note that state machine API request
 * events are at the beginning of the event list starting at zero, thus
 * allowing for this table.
*/
const uint8_t avdt_scb_cback_evt[] = {
    0,                     /* API_REMOVE_EVT (no event) */
    AVDT_WRITE_CFM_EVT,    /* API_WRITE_REQ_EVT */
    0,                     /* API_GETCONFIG_REQ_EVT (no event) */
    0,                     /* API_DELAY_RPT_REQ_EVT (no event) */
    AVDT_OPEN_CFM_EVT,     /* API_SETCONFIG_REQ_EVT */
    AVDT_OPEN_CFM_EVT,     /* API_OPEN_REQ_EVT */
    AVDT_CLOSE_CFM_EVT,    /* API_CLOSE_REQ_EVT */
    AVDT_RECONFIG_CFM_EVT, /* API_RECONFIG_REQ_EVT */
    AVDT_SECURITY_CFM_EVT, /* API_SECURITY_REQ_EVT */
    0                      /* API_ABORT_REQ_EVT (no event) */
};

#define INIT_DELAY_RPT    4000      /* Initial deg=fault Delay Sent after SETCONFIG*/
                                    /* Delay value given is 1/10 millisecond */
#define accure_range      150       /* Value difference considered for sending next DELAY_REPORT*/
                                    /* Delay value given is 1/10 millisecond */
static alarm_t* delay_rpt_alarm = NULL;
static uint16_t reported_delay = INIT_DELAY_RPT;

/*******************************************************************************
 *
 * Function         avdt_scb_gen_ssrc
 *
 * Description      This function generates a SSRC number unique to the stream.
 *
 * Returns          SSRC value.
 *
 ******************************************************************************/
uint32_t avdt_scb_gen_ssrc(tAVDT_SCB* p_scb) {
  /* combine the value of the media type and codec type of the SCB */
  return (
      (uint32_t)(p_scb->cs.cfg.codec_info[1] | p_scb->cs.cfg.codec_info[2]));
}

/*******************************************************************************
 *
 * Function         avdt_scb_hdl_abort_cmd
 *
 * Description      This function sends the SCB an AVDT_SCB_API_ABORT_RSP_EVT
 *                  to initiate sending of an abort response message.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_hdl_abort_cmd(tAVDT_SCB* p_scb, tAVDT_SCB_EVT* p_data) {
  p_scb->role = AVDT_CLOSE_ACP;
  avdt_scb_event(p_scb, AVDT_SCB_API_ABORT_RSP_EVT, p_data);
}

/*******************************************************************************
 *
 * Function         avdt_scb_hdl_abort_rsp
 *
 * Description      This function is an empty function; it serves as a
 *                  placeholder for a conformance API action function.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_hdl_abort_rsp(UNUSED_ATTR tAVDT_SCB* p_scb,
                            UNUSED_ATTR tAVDT_SCB_EVT* p_data) {
  return;
}

/*******************************************************************************
 *
 * Function         avdt_scb_hdl_close_cmd
 *
 * Description      This function sends the SCB an AVDT_SCB_API_CLOSE_RSP_EVT
 *                  to initiate sending of a close response message.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_hdl_close_cmd(tAVDT_SCB* p_scb, tAVDT_SCB_EVT* p_data) {
  p_scb->role = AVDT_CLOSE_ACP;
  avdt_scb_event(p_scb, AVDT_SCB_API_CLOSE_RSP_EVT, p_data);
}

/*******************************************************************************
 *
 * Function         avdt_scb_hdl_close_rsp
 *
 * Description      This function sets the close_code variable to the error
 *                  code returned in the close response.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_hdl_close_rsp(tAVDT_SCB* p_scb, tAVDT_SCB_EVT* p_data) {
  AVDT_TRACE_WARNING("%s: err_code: %x", __func__, p_data->msg.hdr.err_code);
  p_scb->close_code = p_data->msg.hdr.err_code;
}

/*******************************************************************************
 *
 * Function         avdt_scb_hdl_getconfig_cmd
 *
 * Description      This function retrieves the configuration parameters of
 *                  the SCB and sends the SCB an AVDT_SCB_API_GETCONFIG_RSP_EVT
 *                  to initiate sending of a get configuration response message.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_hdl_getconfig_cmd(tAVDT_SCB* p_scb, tAVDT_SCB_EVT* p_data) {
  p_data->msg.svccap.p_cfg = &p_scb->curr_cfg;

  avdt_scb_event(p_scb, AVDT_SCB_API_GETCONFIG_RSP_EVT, p_data);
}

/*******************************************************************************
 *
 * Function         avdt_scb_hdl_getconfig_rsp
 *
 * Description      This function is an empty function; it serves as a
 *                  placeholder for a conformance API action function.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_hdl_getconfig_rsp(UNUSED_ATTR tAVDT_SCB* p_scb,
                                UNUSED_ATTR tAVDT_SCB_EVT* p_data) {
  return;
}

/*******************************************************************************
 *
 * Function         avdt_scb_hdl_open_cmd
 *
 * Description      This function sends the SCB an AVDT_SCB_API_OPEN_RSP_EVT
 *                  to initiate sending of an open response message.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_hdl_open_cmd(tAVDT_SCB* p_scb, tAVDT_SCB_EVT* p_data) {
  if (p_scb->role == AVDT_DELAY_RPT_OPEN_INT || p_scb->role == AVDT_CONF_INT) {
    AVDT_TRACE_WARNING("Outgoing conn in progress, Reject Remote initiated AV Open scb role %d", p_scb->role);
    avdt_scb_rej_state(p_scb, p_data);
  } else {
    avdt_scb_event(p_scb, AVDT_SCB_API_OPEN_RSP_EVT, p_data);
  }
}

/*******************************************************************************
 *
 * Function         avdt_scb_hdl_open_rej
 *
 * Description      This function calls the application callback function
 *                  indicating the open request has failed.  It initializes
 *                  certain SCB variables and sends a AVDT_CCB_UL_CLOSE_EVT
 *                  to the CCB.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_hdl_open_rej(tAVDT_SCB* p_scb, tAVDT_SCB_EVT* p_data) {
  AVDT_TRACE_WARNING("%s:", __func__);
  /* do exactly same as setconfig reject */
  avdt_scb_hdl_setconfig_rej(p_scb, p_data);
}

/*******************************************************************************
 *
 * Function         avdt_scb_hdl_open_rsp
 *
 * Description      This function calls avdt_ad_open_req() to initiate
 *                  connection of the transport channel for this stream.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_hdl_open_rsp(tAVDT_SCB* p_scb,
                           UNUSED_ATTR tAVDT_SCB_EVT* p_data) {
  /* initiate opening of trans channels for this SEID */
  p_scb->role = AVDT_OPEN_INT;
  avdt_ad_open_req(AVDT_CHAN_MEDIA, p_scb->p_ccb, p_scb, AVDT_INT);

  /* start tc connect timer */
  alarm_set_on_mloop(p_scb->transport_channel_timer,
                     AVDT_SCB_TC_CONN_TIMEOUT_MS,
                     avdt_scb_transport_channel_timer_timeout, p_scb);
}

/*******************************************************************************
 *
 * Function         avdt_scb_hdl_pkt_no_frag
 *
 * Description
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_hdl_pkt_no_frag(tAVDT_SCB* p_scb, tAVDT_SCB_EVT* p_data) {
  uint8_t *p, *p_start;
  uint8_t o_v, o_p, o_x, o_cc;
  uint8_t m_pt;
  uint8_t marker;
  uint16_t seq;
  uint32_t time_stamp;
  uint16_t offset;
  uint16_t ex_len;
  uint8_t pad_len = 0;

  p = p_start = (uint8_t*)(p_data->p_pkt + 1) + p_data->p_pkt->offset;

  /* parse media packet header */
  AVDT_MSG_PRS_OCTET1(p, o_v, o_p, o_x, o_cc);
  AVDT_MSG_PRS_M_PT(p, m_pt, marker);
  BE_STREAM_TO_UINT16(seq, p);
  BE_STREAM_TO_UINT32(time_stamp, p);
  p += 4;

  /* skip over any csrc's in packet */
  p += o_cc * 4;

  /* check for and skip over extension header */
  if (o_x) {
    p += 2;
    BE_STREAM_TO_UINT16(ex_len, p);
    p += ex_len * 4;
  }

  /* save our new offset */
  offset = (uint16_t)(p - p_start);

  /* adjust length for any padding at end of packet */
  if (o_p) {
    /* padding length in last byte of packet */
    pad_len = *(p_start + p_data->p_pkt->len);
  }

  /* do sanity check */
  if ((offset > p_data->p_pkt->len) ||
      ((pad_len + offset) > p_data->p_pkt->len)) {
    AVDT_TRACE_WARNING("Got bad media packet");
    osi_free_and_reset((void**)&p_data->p_pkt);
  }
  /* adjust offset and length and send it up */
  else {
    p_data->p_pkt->len -= (offset + pad_len);
    p_data->p_pkt->offset += offset;

    if (p_scb->cs.p_sink_data_cback != NULL) {
      /* report sequence number */
      p_data->p_pkt->layer_specific = seq;
      APPL_TRACE_LATENCY_AUDIO("AVDTP Recv Packet, seq number %d", seq);
      (*p_scb->cs.p_sink_data_cback)(avdt_scb_to_hdl(p_scb), p_data->p_pkt,
                                     time_stamp,
                                     (uint8_t)(m_pt | (marker << 7)));
    } else {
      osi_free_and_reset((void**)&p_data->p_pkt);
    }
  }
}

#if (AVDT_REPORTING == TRUE)
/*******************************************************************************
 *
 * Function         avdt_scb_hdl_report
 *
 * Description
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
uint8_t* avdt_scb_hdl_report(tAVDT_SCB* p_scb, uint8_t* p, uint16_t len) {
  uint16_t result = AVDT_SUCCESS;
  uint8_t* p_start = p;
  uint32_t ssrc;
  uint8_t o_v, o_p, o_cc;
  AVDT_REPORT_TYPE pt;
  tAVDT_REPORT_DATA report;

  AVDT_TRACE_DEBUG("%s", __func__);
  if (p_scb->cs.p_report_cback) {
    /* parse report packet header */
    AVDT_MSG_PRS_RPT_OCTET1(p, o_v, o_p, o_cc);
    pt = *p++;
    p += 2;
    BE_STREAM_TO_UINT32(ssrc, p);

    switch (pt) {
      case AVDT_RTCP_PT_SR: /* the packet type - SR (Sender Report) */
        BE_STREAM_TO_UINT32(report.sr.ntp_sec, p);
        BE_STREAM_TO_UINT32(report.sr.ntp_frac, p);
        BE_STREAM_TO_UINT32(report.sr.rtp_time, p);
        BE_STREAM_TO_UINT32(report.sr.pkt_count, p);
        BE_STREAM_TO_UINT32(report.sr.octet_count, p);
        break;

      case AVDT_RTCP_PT_RR: /* the packet type - RR (Receiver Report) */
        report.rr.frag_lost = *p;
        BE_STREAM_TO_UINT32(report.rr.packet_lost, p);
        report.rr.packet_lost &= 0xFFFFFF;
        BE_STREAM_TO_UINT32(report.rr.seq_num_rcvd, p);
        BE_STREAM_TO_UINT32(report.rr.jitter, p);
        BE_STREAM_TO_UINT32(report.rr.lsr, p);
        BE_STREAM_TO_UINT32(report.rr.dlsr, p);
        break;

      case AVDT_RTCP_PT_SDES: /* the packet type - SDES (Source Description) */
        uint8_t sdes_type;
        BE_STREAM_TO_UINT8(sdes_type, p);
        if (sdes_type == AVDT_RTCP_SDES_CNAME) {
          uint8_t name_length;
          BE_STREAM_TO_UINT8(name_length, p);
          if (name_length > len - 2 || name_length > AVDT_MAX_CNAME_SIZE) {
            result = AVDT_BAD_PARAMS;
          } else {
            BE_STREAM_TO_ARRAY(p, &(report.cname[0]), name_length);
          }
        } else {
          AVDT_TRACE_WARNING(" - SDES SSRC=0x%08x sc=%d %d len=%d %s", ssrc,
                             o_cc, *p, *(p + 1), p + 2);
          result = AVDT_BUSY;
        }
        break;

      default:
        AVDT_TRACE_ERROR("Bad Report pkt - packet type: %d", pt);
        result = AVDT_BAD_PARAMS;
    }

    if (result == AVDT_SUCCESS)
      (*p_scb->cs.p_report_cback)(avdt_scb_to_hdl(p_scb), pt, &report);
  }
  p_start += len;
  return p_start;
}
#endif

/*******************************************************************************
 *
 * Function         avdt_scb_hdl_pkt
 *
 * Description
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_hdl_pkt(tAVDT_SCB* p_scb, tAVDT_SCB_EVT* p_data) {
#if (AVDT_REPORTING == TRUE)
  if (p_data->p_pkt->layer_specific == AVDT_CHAN_REPORT) {
    uint8_t* p = (uint8_t*)(p_data->p_pkt + 1) + p_data->p_pkt->offset;
    avdt_scb_hdl_report(p_scb, p, p_data->p_pkt->len);
    osi_free_and_reset((void**)&p_data->p_pkt);
  } else
#endif
    avdt_scb_hdl_pkt_no_frag(p_scb, p_data);
}

/*******************************************************************************
 *
 * Function         avdt_scb_drop_pkt
 *
 * Description      Drop an incoming media packet.  This function is called if
 *                  a media packet is received in any state besides streaming.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_drop_pkt(UNUSED_ATTR tAVDT_SCB* p_scb, tAVDT_SCB_EVT* p_data) {
  AVDT_TRACE_ERROR("%s dropped incoming media packet", __func__);
  osi_free_and_reset((void**)&p_data->p_pkt);
}

/*******************************************************************************
 *
 * Function         avdt_scb_hdl_reconfig_cmd
 *
 * Description      This function calls the application callback function
 *                  with a reconfiguration indication.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_hdl_reconfig_cmd(tAVDT_SCB* p_scb, tAVDT_SCB_EVT* p_data) {
  /* if command not supported */
  if (p_scb->cs.nsc_mask & AVDT_NSC_RECONFIG) {
    /* send reject */
    p_data->msg.hdr.err_code = AVDT_ERR_NSC;
    p_data->msg.hdr.err_param = 0;
    avdt_scb_event(p_scb, AVDT_SCB_API_RECONFIG_RSP_EVT, p_data);
  } else {
    /* store requested configuration */
    memcpy(&p_scb->req_cfg, p_data->msg.reconfig_cmd.p_cfg, sizeof(tAVDT_CFG));

    /* call application callback */
    (*p_scb->cs.p_ctrl_cback)(avdt_scb_to_hdl(p_scb), NULL,
                              AVDT_RECONFIG_IND_EVT,
                              (tAVDT_CTRL*)&p_data->msg.reconfig_cmd);
  }
}

/*******************************************************************************
 *
 * Function         avdt_scb_hdl_reconfig_rsp
 *
 * Description      This function calls the application callback function
 *                  with a reconfiguration confirm.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_hdl_reconfig_rsp(tAVDT_SCB* p_scb, tAVDT_SCB_EVT* p_data) {
  if (p_data->msg.hdr.err_code == 0) {
    /* store new configuration */
    if (p_scb->req_cfg.num_codec > 0) {
      p_scb->curr_cfg.num_codec = p_scb->req_cfg.num_codec;
      memcpy(p_scb->curr_cfg.codec_info, p_scb->req_cfg.codec_info,
             AVDT_CODEC_SIZE);
    }
    if (p_scb->req_cfg.num_protect > 0) {
      p_scb->curr_cfg.num_protect = p_scb->req_cfg.num_protect;
      memcpy(p_scb->curr_cfg.protect_info, p_scb->req_cfg.protect_info,
             AVDT_PROTECT_SIZE);
    }
  }

  p_data->msg.svccap.p_cfg = &p_scb->curr_cfg;

  /* call application callback */
  (*p_scb->cs.p_ctrl_cback)(avdt_scb_to_hdl(p_scb), NULL, AVDT_RECONFIG_CFM_EVT,
                            (tAVDT_CTRL*)&p_data->msg.svccap);
}

/*******************************************************************************
 *
 * Function         avdt_scb_hdl_security_cmd
 *
 * Description      This function calls the application callback with a
 *                  security indication.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_hdl_security_cmd(tAVDT_SCB* p_scb, tAVDT_SCB_EVT* p_data) {
  /* if command not supported */
  if (p_scb->cs.nsc_mask & AVDT_NSC_SECURITY) {
    /* send reject */
    p_data->msg.hdr.err_code = AVDT_ERR_NSC;
    avdt_scb_event(p_scb, AVDT_SCB_API_SECURITY_RSP_EVT, p_data);
  } else {
    /* call application callback */
    (*p_scb->cs.p_ctrl_cback)(avdt_scb_to_hdl(p_scb), NULL,
                              AVDT_SECURITY_IND_EVT,
                              (tAVDT_CTRL*)&p_data->msg.security_cmd);
  }
}

/*******************************************************************************
 *
 * Function         avdt_scb_hdl_security_rsp
 *
 * Description      This function calls the application callback with a
 *                  security confirm.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_hdl_security_rsp(tAVDT_SCB* p_scb, tAVDT_SCB_EVT* p_data) {
  /* call application callback */
  (*p_scb->cs.p_ctrl_cback)(avdt_scb_to_hdl(p_scb), NULL, AVDT_SECURITY_CFM_EVT,
                            (tAVDT_CTRL*)&p_data->msg.security_cmd);
}

void avdt_set_scbs_busy(tAVDT_SCB *ptr_scb) {
  AVDT_TRACE_DEBUG(" avdt_set_scbs_busy ");
  tAVDT_SCB       *p_scb = &avdt_cb.scb[0];
  uint8_t reg_id = ptr_scb->cs.registration_id;
  int i = 0;
  for (i = 0; i < AVDT_NUM_SEPS; i++, p_scb++) {
    AVDT_TRACE_DEBUG(" avdt_set_scbs_busy SCB[%d] reg_id, sep_type ", i, p_scb->cs.registration_id, p_scb->cs.tsep);
    if ((p_scb->allocated) && (p_scb->cs.registration_id == reg_id) && (p_scb->cs.tsep == ptr_scb->cs.tsep)) {
      AVDT_TRACE_DEBUG(" Setting SCB[%d].in_use as true", i);
      p_scb->in_use = TRUE;
    }
  }
}
void avdt_set_scbs_free(tAVDT_SCB *ptr_scb) {
  AVDT_TRACE_DEBUG(" avdt_set_scbs_free ");
  tAVDT_SCB       *p_scb = &avdt_cb.scb[0];
  uint8_t reg_id = ptr_scb->cs.registration_id;
  int i = 0;
  for (i = 0; i < AVDT_NUM_SEPS; i++, p_scb++) {
    AVDT_TRACE_DEBUG(" avdt_set_scbs_free SCB[%d] reg_id, sep_type ", i, p_scb->cs.registration_id, p_scb->cs.tsep);
    if ((p_scb->allocated) && (p_scb->cs.registration_id == reg_id) && (p_scb->cs.tsep == ptr_scb->cs.tsep)) {
      AVDT_TRACE_DEBUG(" Setting SCB[%d].in_use as false ", i);
      p_scb->in_use = FALSE;
    }
  }
}
/*******************************************************************************
 *
 * Function         avdt_check_sep_state
 *
 * Description      This function checks if either of the SEID is in use in the
 *                  cluster of a stream in which the ACP SEP for the remote
 *                  initiated connection belongs to.
 *
 * Returns          True if one SEID in the cluster is busy, False otherwise
 *
 ******************************************************************************/
bool avdt_check_sep_state(tAVDT_SCB *p_scb) {
  int i,j;
  int num_sep = 0,sep_offset;
  int num_stream = avdt_scb_get_max_av_client();
  if (num_stream == 1)
    return false;
  for (i = 0;i < AVDT_NUM_SEPS; i++) {
    tAVDT_SCB *temp_scb = &avdt_cb.scb[i];
    if (p_scb == temp_scb)
      break;
  }
  if (i < AVDT_NUM_SEPS) {
    sep_offset = i;
    tAVDT_SCB *temp_scb = &avdt_cb.scb[0];
    for (j = 0; j < AVDT_NUM_SEPS; j++, temp_scb++) {
      if (temp_scb->allocated)
        num_sep++;
    }
    int num_stream  = avdt_scb_get_max_av_client();
    int num_codecs = num_sep/num_stream;
    for (i = 0; i < num_sep;i += num_codecs) {
      bool in_use = false;
      for (j = i;j < (i+num_codecs); j++) {
        tAVDT_SCB *temp_scb = &avdt_cb.scb[j];
        if (temp_scb->in_use)
          in_use = true;
      }
      if (in_use && (sep_offset >= i && sep_offset < j))
        return true;
    }
  }
  return false;
}

/*******************************************************************************
 *
 * Function         avdt_scb_hdl_setconfig_cmd
 *
 * Description      This function marks the SCB as in use and copies the
 *                  configuration and peer SEID to the SCB.  It then calls
 *                  the application callback with a configuration indication.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_hdl_setconfig_cmd(tAVDT_SCB* p_scb, tAVDT_SCB_EVT* p_data) {
  tAVDT_CFG* p_cfg;
  tA2DP_CODEC_TYPE codec_type;
  AVDT_TRACE_WARNING("avdt_scb_hdl_setconfig_cmd: SCB in use: %d, Conn in progress: %d, avdt_check_sep_state: %d",
       p_scb->in_use, avdt_cb.conn_in_progress, avdt_check_sep_state(p_scb));

  if ((!p_scb->in_use) && !(avdt_check_sep_state(p_scb)) &&
      (!avdt_cb.conn_in_progress)) {
    A2DP_DumpCodecInfo(p_scb->cs.cfg.codec_info);
    A2DP_DumpCodecInfo(p_data->msg.config_cmd.p_cfg->codec_info);
    p_cfg = p_data->msg.config_cmd.p_cfg;
    codec_type = A2DP_GetCodecType(p_cfg->codec_info);
    AVDT_TRACE_DEBUG("%s: Incoming codec_type: %x, min/max bitpool: %x/%x", __func__, codec_type,
                       p_cfg->codec_info[A2DP_SBC_IE_MIN_BITPOOL_OFFSET],
                       p_cfg->codec_info[A2DP_SBC_IE_MAX_BITPOOL_OFFSET]);
    if (A2DP_GetCodecType(p_scb->cs.cfg.codec_info) == codec_type) {
      /* set sep as in use */
      p_scb->in_use = true;

      /* copy info to scb */
      p_scb->p_ccb = avdt_ccb_by_idx(p_data->msg.config_cmd.hdr.ccb_idx);
      avdt_set_scbs_busy(p_scb);
      p_scb->peer_seid = p_data->msg.config_cmd.int_seid;
      if (codec_type == A2DP_MEDIA_CT_SBC) {
        if (p_scb->cs.tsep == AVDT_TSEP_SNK) {
          //SNK minbitool > 86, then set minbitpool = 86
          if ((p_cfg->codec_info[A2DP_SBC_IE_MIN_BITPOOL_OFFSET]) > A2DP_SBC_SINK_MAX_BITPOOL) {
             p_cfg->codec_info[A2DP_SBC_IE_MIN_BITPOOL_OFFSET] = A2DP_SBC_SINK_MAX_BITPOOL;
          }
          //SNK maxbitool > 86, then set maxbitpool = 86
          if ((p_cfg->codec_info[A2DP_SBC_IE_MAX_BITPOOL_OFFSET]) > A2DP_SBC_SINK_MAX_BITPOOL) {
             p_cfg->codec_info[A2DP_SBC_IE_MAX_BITPOOL_OFFSET] = A2DP_SBC_SINK_MAX_BITPOOL;
          }
          AVDT_TRACE_DEBUG("%s: SNK min/max bitpool: %x/%x", __func__,
                              p_cfg->codec_info[A2DP_SBC_IE_MIN_BITPOOL_OFFSET],
                              p_cfg->codec_info[A2DP_SBC_IE_MAX_BITPOOL_OFFSET]);
        }
        //minbitpool < 2, then set minbitpool = 2
        if ((p_cfg->codec_info[A2DP_SBC_IE_MIN_BITPOOL_OFFSET]) < A2DP_SBC_IE_MIN_BITPOOL) {
          p_cfg->codec_info[A2DP_SBC_IE_MIN_BITPOOL_OFFSET] = A2DP_SBC_IE_MIN_BITPOOL;
          AVDT_TRACE_DEBUG("%s: Incoming connection set min bitpool: %x", __func__,
                              p_cfg->codec_info[A2DP_SBC_IE_MIN_BITPOOL_OFFSET]);
        }

        if (p_scb->cs.tsep == AVDT_TSEP_SRC) {
          //minbitpool > 250, then set minbitpool = 250
          if ((p_cfg->codec_info[A2DP_SBC_IE_MIN_BITPOOL_OFFSET]) > A2DP_SBC_IE_MAX_BITPOOL) {
            p_cfg->codec_info[A2DP_SBC_IE_MIN_BITPOOL_OFFSET] = A2DP_SBC_IE_MAX_BITPOOL;
            AVDT_TRACE_DEBUG("%s: Incoming connection set min bitpool: %x", __func__,
                                p_cfg->codec_info[A2DP_SBC_IE_MIN_BITPOOL_OFFSET]);
          }

          //maxbitpool > 250, then set maxbitpool = 250
          if ((p_cfg->codec_info[A2DP_SBC_IE_MAX_BITPOOL_OFFSET]) > A2DP_SBC_IE_MAX_BITPOOL) {
            p_cfg->codec_info[A2DP_SBC_IE_MAX_BITPOOL_OFFSET] = A2DP_SBC_IE_MAX_BITPOOL;
            AVDT_TRACE_DEBUG("%s: Incoming connection set max bitpool: %x", __func__,
                                p_cfg->codec_info[A2DP_SBC_IE_MAX_BITPOOL_OFFSET]);
          }
        }

        //minbitpool > maxbitpool, then set maxbitpool = minbitpool
        if ((p_cfg->codec_info[A2DP_SBC_IE_MIN_BITPOOL_OFFSET]) >
            (p_cfg->codec_info[A2DP_SBC_IE_MAX_BITPOOL_OFFSET])) {
          p_cfg->codec_info[A2DP_SBC_IE_MAX_BITPOOL_OFFSET] =
                              p_cfg->codec_info[A2DP_SBC_IE_MIN_BITPOOL_OFFSET];
          AVDT_TRACE_DEBUG("%s: Incoming connection minbitpool set by remote exceeds"
                           "maxbitpool value, So set maxbitbool to minbitpool: %x to %x",
                           __func__, p_cfg->codec_info[A2DP_SBC_IE_MAX_BITPOOL_OFFSET],
                                     p_cfg->codec_info[A2DP_SBC_IE_MIN_BITPOOL_OFFSET]);
        }
      }
      memcpy(&p_scb->req_cfg, p_cfg, sizeof(tAVDT_CFG));
      /* call app callback */
      /* handle of scb- which is same as sep handle of bta_av_cb.p_scb*/
      (*p_scb->cs.p_ctrl_cback)(avdt_scb_to_hdl(p_scb),
                                p_scb->p_ccb ? &p_scb->p_ccb->peer_addr : NULL,
                                AVDT_CONFIG_IND_EVT,
                                (tAVDT_CTRL*)&p_data->msg.config_cmd);
    } else {
      p_data->msg.hdr.err_code = AVDT_ERR_UNSUP_CFG;
      p_data->msg.hdr.err_param = 0;
      AVDT_TRACE_DEBUG("%s: called avdt_msg_send_rej()", __func__);
      avdt_msg_send_rej(avdt_ccb_by_idx(p_data->msg.hdr.ccb_idx),
                        p_data->msg.hdr.sig_id, &p_data->msg);
    }
  } else {
    AVDT_TRACE_DEBUG("%s: calling avdt_scb_rej_in_use()", __func__);
    avdt_scb_rej_in_use(p_scb, p_data);
  }
}

/*******************************************************************************
 *
 * Function         avdt_scb_hdl_setconfig_rej
 *
 * Description      This function marks the SCB as not in use and calls the
 *                  application callback with an open confirm indicating
 *                  failure.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_hdl_setconfig_rej(tAVDT_SCB* p_scb, tAVDT_SCB_EVT* p_data) {
  AVDT_TRACE_DEBUG("%s: ", __func__);
  /* clear scb variables */
  avdt_scb_clr_vars(p_scb, p_data);

  /* tell ccb we're done with signaling channel */
  avdt_ccb_event(avdt_ccb_by_idx(p_data->msg.hdr.ccb_idx),
                 AVDT_CCB_UL_CLOSE_EVT, NULL);

  /* call application callback */
  (*p_scb->cs.p_ctrl_cback)(avdt_scb_to_hdl(p_scb), NULL, AVDT_OPEN_CFM_EVT,
                            (tAVDT_CTRL*)&p_data->msg.hdr);
}

/*******************************************************************************
 *
 * Function         avdt_scb_hdl_setconfig_rsp
 *
 * Description      This function sends the SCB an AVDT_SCB_API_OPEN_REQ_EVT
 *                  to initiate sending of an open command message.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_hdl_setconfig_rsp(tAVDT_SCB* p_scb,
                                UNUSED_ATTR tAVDT_SCB_EVT* p_data) {
  tAVDT_EVT_HDR single;

  if (p_scb->p_ccb != NULL) {
    /* save configuration */
    memcpy(&p_scb->curr_cfg, &p_scb->req_cfg, sizeof(tAVDT_CFG));
    p_scb->role = AVDT_CONF_INT;

    if ((p_scb->cs.tsep == AVDT_TSEP_SNK) && (p_scb->curr_cfg.psc_mask & AVDT_PSC_DELAY_RPT)) {
      reported_delay = INIT_DELAY_RPT;
      AVDT_TRACE_DEBUG(" %s ~~ support DELAY_RPT , begin init Delay report procedure",__func__);
      AVDT_DelayReport(avdt_scb_to_hdl(p_scb), p_scb->peer_seid, reported_delay);
    }

    if (!(p_scb->curr_cfg.psc_mask & AVDT_PSC_DELAY_RPT)) {
      /* initiate open */
      single.seid = p_scb->peer_seid;
      tAVDT_SCB_EVT avdt_scb_evt;
      avdt_scb_evt.msg.single = single;
      avdt_scb_event(p_scb, AVDT_SCB_API_OPEN_REQ_EVT, &avdt_scb_evt);
    } else {
      alarm_set_on_mloop(p_scb->delay_report_timer,
                         AVDT_DELAY_REPORT_TIMEOUT_MS,
                         avdt_delay_report_timer_timeout, p_scb);
    }
  }
}

/*******************************************************************************
 *
 * Function         avdt_scb_hdl_start_cmd
 *
 * Description      This function calls the application callback with a
 *                  start indication.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_hdl_start_cmd(tAVDT_SCB* p_scb,
                            UNUSED_ATTR tAVDT_SCB_EVT* p_data) {
  (*p_scb->cs.p_ctrl_cback)(avdt_scb_to_hdl(p_scb),
                            p_scb->p_ccb ? &p_scb->p_ccb->peer_addr : NULL,
                            AVDT_START_IND_EVT, NULL);
}

/*******************************************************************************
 *
 * Function         avdt_scb_hdl_start_rsp
 *
 * Description      This function calls the application callback with a
 *                  start confirm.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_hdl_start_rsp(tAVDT_SCB* p_scb, tAVDT_SCB_EVT* p_data) {
  (*p_scb->cs.p_ctrl_cback)(avdt_scb_to_hdl(p_scb),
                            p_scb->p_ccb ? &p_scb->p_ccb->peer_addr : NULL,
                            AVDT_START_CFM_EVT, (tAVDT_CTRL*)&p_data->msg.hdr);
}

/*******************************************************************************
 *
 * Function         avdt_scb_hdl_suspend_cmd
 *
 * Description      This function calls the application callback with a suspend
 *                  indication.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_hdl_suspend_cmd(tAVDT_SCB* p_scb,
                              UNUSED_ATTR tAVDT_SCB_EVT* p_data) {
  (*p_scb->cs.p_ctrl_cback)(avdt_scb_to_hdl(p_scb),
                            p_scb->p_ccb ? &p_scb->p_ccb->peer_addr : NULL,
                            AVDT_SUSPEND_IND_EVT, NULL);
}

/*******************************************************************************
 *
 * Function         avdt_scb_hdl_suspend_rsp
 *
 * Description      This function calls the application callback with a suspend
 *                  confirm.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_hdl_suspend_rsp(tAVDT_SCB* p_scb, tAVDT_SCB_EVT* p_data) {
  (*p_scb->cs.p_ctrl_cback)(
      avdt_scb_to_hdl(p_scb), p_scb->p_ccb ? &p_scb->p_ccb->peer_addr : NULL,
      AVDT_SUSPEND_CFM_EVT, (tAVDT_CTRL*)&p_data->msg.hdr);
}

/*******************************************************************************
 *
 * Function         avdt_scb_hdl_tc_close
 *
 * Description      This function is called when the transport channel is
 *                  closed.  It marks the SCB as not in use and
 *                  initializes certain SCB parameters.  It then sends
 *                  an AVDT_CCB_UL_CLOSE_EVT to the CCB if the SCB
 *                  initiated the close.  It then checks to see if the SCB
 *                  is to be removed.  If it is it deallocates the SCB.
 *                  Finally, it calls the application callback with a close
 *                  indication.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_hdl_tc_close(tAVDT_SCB* p_scb, tAVDT_SCB_EVT* p_data) {
  uint8_t hdl = avdt_scb_to_hdl(p_scb);
  tAVDT_CTRL_CBACK* p_ctrl_cback = p_scb->cs.p_ctrl_cback;
  tAVDT_CTRL avdt_ctrl;
  uint8_t event;
  tAVDT_CCB* p_ccb = p_scb->p_ccb;
  RawAddress remote_addr = p_ccb->peer_addr;

  AVDT_TRACE_DEBUG(" %s: role: %x, remove: %d, close_code: %x",
                 __func__, p_scb->role, p_scb->remove, p_scb->close_code);

  /* set up hdr */
  avdt_ctrl.hdr.err_code = p_scb->close_code;

  /* clear sep variables */
  avdt_scb_clr_vars(p_scb, p_data);
  p_scb->media_seq = 0;
  p_scb->cong = false;

  /* free pkt we're holding, if any */
  osi_free_and_reset((void**)&p_scb->p_pkt);

  alarm_cancel(p_scb->transport_channel_timer);

  if (delay_rpt_alarm != NULL) {
    alarm_free(delay_rpt_alarm);
    delay_rpt_alarm = NULL;
  }

  if ((p_scb->role == AVDT_CLOSE_INT) || (p_scb->role == AVDT_OPEN_INT)) {
    /* tell ccb we're done with signaling channel */
    avdt_ccb_event(p_ccb, AVDT_CCB_UL_CLOSE_EVT, NULL);
  }
  event =
      (p_scb->role == AVDT_CLOSE_INT) ? AVDT_CLOSE_CFM_EVT : AVDT_CLOSE_IND_EVT;
  p_scb->role = AVDT_CLOSE_ACP;

  if (p_scb->remove) {
    avdt_scb_dealloc(p_scb, NULL);
  }

  /* call app callback */
  (*p_ctrl_cback)(hdl, &remote_addr, event, &avdt_ctrl);
}

/*******************************************************************************
 *
 * Function         avdt_scb_snd_delay_rpt_req
 *
 * Description      This function calls the application callback with a delay
 *                  report.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_snd_delay_rpt_req(tAVDT_SCB* p_scb, tAVDT_SCB_EVT* p_data) {
  avdt_msg_send_cmd(p_scb->p_ccb, p_scb, AVDT_SIG_DELAY_RPT,
                    (tAVDT_MSG*)&p_data->apidelay);
}

/*******************************************************************************
 *
 * Function         avdt_scb_hdl_delay_rpt_cmd
 *
 * Description      This function calls the application callback with a delay
 *                  report.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_hdl_delay_rpt_cmd(tAVDT_SCB* p_scb, tAVDT_SCB_EVT* p_data) {
  tAVDT_EVT_HDR single;
  alarm_cancel(p_scb->delay_report_timer);
  (*p_scb->cs.p_ctrl_cback)(
      avdt_scb_to_hdl(p_scb), p_scb->p_ccb ? &p_scb->p_ccb->peer_addr : NULL,
      AVDT_DELAY_REPORT_EVT, (tAVDT_CTRL*)&p_data->msg.hdr);

  if (p_scb->p_ccb) {
    if ((p_scb->cs.cfg.psc_mask & AVDT_PSC_DELAY_RPT)) {
      avdt_msg_send_rsp(p_scb->p_ccb, AVDT_SIG_DELAY_RPT, &p_data->msg);
      if (p_scb->role == AVDT_CONF_INT) {
        /* initiate open after get initial delay report value*/
        single.seid = p_scb->peer_seid;
        p_scb->role = AVDT_DELAY_RPT_OPEN_INT;
        avdt_scb_event(p_scb, AVDT_SCB_API_OPEN_REQ_EVT, (tAVDT_SCB_EVT*)&single);
      }
    } else {
      p_data->msg.hdr.err_code = AVDT_ERR_NSC;
      avdt_msg_send_rej(p_scb->p_ccb, AVDT_SIG_DELAY_RPT, &p_data->msg);
    }
  }
  else
    avdt_scb_rej_not_in_use(p_scb, p_data);
}

/*******************************************************************************
 *
 * Function         avdt_scb_hdl_delay_rpt_rsp
 *
 * Description      This function calls the application callback with a delay
 *                  report.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_hdl_delay_rpt_rsp(tAVDT_SCB* p_scb, tAVDT_SCB_EVT* p_data) {

  if ((p_scb->cs.tsep == AVDT_TSEP_SNK) &&
      (p_scb->state == AVDT_SCB_CONF_ST) && (p_scb->role == AVDT_CONF_INT)) {
    tAVDT_EVT_HDR single;
    tAVDT_SCB_EVT avdt_scb_evt;

    single.seid = p_scb->peer_seid;
    avdt_scb_evt.msg.single = single;
    avdt_scb_event(p_scb, AVDT_SCB_API_OPEN_REQ_EVT, &avdt_scb_evt);
  }

  (*p_scb->cs.p_ctrl_cback)(
      avdt_scb_to_hdl(p_scb), p_scb->p_ccb ? &p_scb->p_ccb->peer_addr : NULL,
      AVDT_DELAY_REPORT_CFM_EVT, (tAVDT_CTRL*)&p_data->msg.hdr);
}

#if (AVDT_REPORTING == TRUE)
/*******************************************************************************
 *
 * Function         avdt_scb_hdl_tc_close_sto
 *
 * Description      This function is called when a channel is closed in OPEN
 *                  state.  Check the channel type and process accordingly.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_hdl_tc_close_sto(tAVDT_SCB* p_scb, tAVDT_SCB_EVT* p_data) {
  tAVDT_CTRL avdt_ctrl;
  /* AVDT_CHAN_SIG does not visit this action */
  if (p_data && p_data->close.type != AVDT_CHAN_MEDIA) {
    /* it's reporting or recovery channel,
     * the channel close in open state means the peer does not support it */
    if (p_data->close.old_tc_state == AVDT_AD_ST_OPEN) {
      avdt_ctrl.hdr.err_code = 0;
      avdt_ctrl.hdr.err_param = 0;
      /* call app callback */
      (*p_scb->cs.p_ctrl_cback)(avdt_scb_to_hdl(p_scb),
                                p_scb->p_ccb ? &p_scb->p_ccb->peer_addr : NULL,
                                AVDT_REPORT_DISCONN_EVT, &avdt_ctrl);
    }
  } else {
    /* must be in OPEN state. need to go back to idle */
    avdt_scb_event(p_scb, AVDT_SCB_MSG_ABORT_RSP_EVT, NULL);
    avdt_scb_hdl_tc_close(p_scb, p_data);
  }
}
#endif

/*******************************************************************************
 *
 * Function         avdt_delay_rpt_tmr_hdlr
 *
 * Description      Timer to trigger checking average_delay,
 *                  compare the latest reported delay,
 *                  if the current delay is out of accure range,
 *                  start a new Delay report procedure.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
static void avdt_delay_rpt_tmr_hdlr(void* data) {
  uint64_t average_delay = btif_get_average_delay();

  if (average_delay == 0)
    return;

  uint16_t delay_ms = (uint16_t)(average_delay / 100000); /* report value is in 1/10 millisecond */

  if (abs(reported_delay - delay_ms) >= accure_range) {
    reported_delay = delay_ms;

    AVDT_TRACE_DEBUG(" %s ~~ average delay is changed, update delay report  ",__func__);
    tAVDT_SCB *p_scb = (tAVDT_SCB *)data;
    AVDT_DelayReport(avdt_scb_to_hdl(p_scb), p_scb->peer_seid, reported_delay);
  }
}

/*******************************************************************************
 *
 * Function         avdt_scb_hdl_tc_open
 *
 * Description      This function is called when the transport channel is
 *                  opened while in the opening state.  It calls the
 *                  application callback with an open indication or open
 *                  confirm depending on who initiated the open procedure.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_hdl_tc_open(tAVDT_SCB* p_scb, tAVDT_SCB_EVT* p_data) {
  uint8_t event;
#if (AVDT_REPORTING == TRUE)
  uint8_t role;
#endif

  alarm_cancel(p_scb->transport_channel_timer);

  if ((p_scb->cs.tsep == AVDT_TSEP_SNK) && (p_scb->curr_cfg.psc_mask & AVDT_PSC_DELAY_RPT)) {
    delay_rpt_alarm = alarm_new_periodic("avdt.delayreport");
    alarm_set(delay_rpt_alarm, (period_ms_t)1000 ,(alarm_callback_t)avdt_delay_rpt_tmr_hdlr,
              (void*)p_scb);
    AVDT_TRACE_DEBUG(" %s ~~ start update delay report timer",__func__);
  }

  event =
      (p_scb->role == AVDT_OPEN_INT) ? AVDT_OPEN_CFM_EVT : AVDT_OPEN_IND_EVT;
  p_data->open.hdr.err_code = 0;

  AVDT_TRACE_DEBUG("psc_mask: cfg: 0x%x, req:0x%x, cur: 0x%x",
                   p_scb->cs.cfg.psc_mask, p_scb->req_cfg.psc_mask,
                   p_scb->curr_cfg.psc_mask);
#if (AVDT_REPORTING == TRUE)
  if (p_scb->curr_cfg.psc_mask & AVDT_PSC_REPORT) {
    /* open the reporting channel, if both devices support it */
    role = (p_scb->role == AVDT_OPEN_INT) ? AVDT_INT : AVDT_ACP;
    avdt_ad_open_req(AVDT_CHAN_REPORT, p_scb->p_ccb, p_scb, role);
  }
#endif

  /* call app callback */
  (*p_scb->cs.p_ctrl_cback)(avdt_scb_to_hdl(p_scb),
                            p_scb->p_ccb ? &p_scb->p_ccb->peer_addr : NULL,
                            event, (tAVDT_CTRL*)&p_data->open);
}

#if (AVDT_REPORTING == TRUE)
/*******************************************************************************
 *
 * Function         avdt_scb_hdl_tc_open_sto
 *
 * Description      This function is called when the transport channel is
 *                  opened while in the opening state.  It calls the
 *                  application callback with an open indication or open
 *                  confirm depending on who initiated the open procedure.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_hdl_tc_open_sto(tAVDT_SCB* p_scb, tAVDT_SCB_EVT* p_data) {
  tAVDT_CTRL avdt_ctrl;
  /* open reporting channel here, when it is implemented */

  /* call app callback */
  if (p_data->open.hdr.err_code == AVDT_CHAN_REPORT) {
    avdt_ctrl.hdr.err_code = 0;
    avdt_ctrl.hdr.err_param = 1;
    (*p_scb->cs.p_ctrl_cback)(avdt_scb_to_hdl(p_scb),
                              p_scb->p_ccb ? &p_scb->p_ccb->peer_addr : NULL,
                              AVDT_REPORT_CONN_EVT, &avdt_ctrl);
  }
}
#endif

/*******************************************************************************
 *
 * Function         avdt_scb_hdl_write_req
 *
 * Description      This function frees the media packet currently stored in
 *                  the SCB, if any.  Then it builds a new media packet from
 *                  with the passed in buffer and stores it in the SCB.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_hdl_write_req(tAVDT_SCB* p_scb, tAVDT_SCB_EVT* p_data) {
  uint8_t* p;
  uint32_t ssrc;
  bool add_rtp_header = !(p_data->apiwrite.opt & AVDT_DATA_OPT_NO_RTP);
  AVDT_TRACE_DEBUG("%s: add_rtp_header: %d, num_protect: %d",
                        __func__, add_rtp_header, p_scb->curr_cfg.num_protect);

  /* free packet we're holding, if any; to be replaced with new */
  if (p_scb->p_pkt != NULL) {
    /* this shouldn't be happening */
    AVDT_TRACE_WARNING("Dropped media packet; congested");
  }
  osi_free_and_reset((void**)&p_scb->p_pkt);
  AVDT_TRACE_DEBUG("%s:pkt freed and reset",__func__);
  /* Recompute only if the RTP header wasn't disabled by the API */
  if (add_rtp_header) {
    bool is_content_protection = (p_scb->curr_cfg.num_protect > 0);
    add_rtp_header =
        A2DP_UsesRtpHeader(is_content_protection, p_scb->curr_cfg.codec_info);
  }

  /* Build a media packet, and add an RTP header if required. */
  if (add_rtp_header) {
    AVDT_TRACE_DEBUG("%s:add rtp header",__func__);
    ssrc = avdt_scb_gen_ssrc(p_scb);

    p_data->apiwrite.p_buf->len += AVDT_MEDIA_HDR_SIZE;
    p_data->apiwrite.p_buf->offset -= AVDT_MEDIA_HDR_SIZE;
    p_scb->media_seq++;
    p = (uint8_t*)(p_data->apiwrite.p_buf + 1) + p_data->apiwrite.p_buf->offset;

    UINT8_TO_BE_STREAM(p, AVDT_MEDIA_OCTET1);
    UINT8_TO_BE_STREAM(p, p_data->apiwrite.m_pt);
    UINT16_TO_BE_STREAM(p, p_scb->media_seq);
    UINT32_TO_BE_STREAM(p, p_data->apiwrite.time_stamp);
    UINT32_TO_BE_STREAM(p, ssrc);
    AVDT_TRACE_DEBUG("%s:rtp header added",__func__);
  }

  /* store it */
  p_scb->p_pkt = p_data->apiwrite.p_buf;
  AVDT_TRACE_DEBUG("%s:Exit",__func__);
}

/*******************************************************************************
 *
 * Function         avdt_scb_snd_abort_req
 *
 * Description      This function sends an abort command message.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_snd_abort_req(tAVDT_SCB* p_scb,
                            UNUSED_ATTR tAVDT_SCB_EVT* p_data) {
  tAVDT_EVT_HDR hdr;

  AVDT_TRACE_DEBUG("%s: p_scb->p_ccb=%p", __func__, p_scb->p_ccb);

  if (p_scb->p_ccb != NULL) {
    p_scb->role = AVDT_CLOSE_INT;

    hdr.seid = p_scb->peer_seid;

    tAVDT_MSG avdt_msg;
    avdt_msg.hdr = hdr;
    avdt_msg_send_cmd(p_scb->p_ccb, p_scb, AVDT_SIG_ABORT, &avdt_msg);
  }
}

/*******************************************************************************
 *
 * Function         avdt_scb_snd_abort_rsp
 *
 * Description      This function sends an abort response message.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_snd_abort_rsp(UNUSED_ATTR tAVDT_SCB* p_scb,
                            tAVDT_SCB_EVT* p_data) {
  avdt_msg_send_rsp(avdt_ccb_by_idx(p_data->msg.hdr.ccb_idx), AVDT_SIG_ABORT,
                    &p_data->msg);
}

/*******************************************************************************
 *
 * Function         avdt_scb_snd_close_req
 *
 * Description      This function sends a close command message.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_snd_close_req(tAVDT_SCB* p_scb,
                            UNUSED_ATTR tAVDT_SCB_EVT* p_data) {
  tAVDT_EVT_HDR hdr;

  p_scb->role = AVDT_CLOSE_INT;

  hdr.seid = p_scb->peer_seid;

  tAVDT_MSG avdt_msg;
  avdt_msg.hdr = hdr;
  avdt_msg_send_cmd(p_scb->p_ccb, p_scb, AVDT_SIG_CLOSE, &avdt_msg);
}

/*******************************************************************************
 *
 * Function         avdt_scb_snd_stream_close
 *
 * Description      This function sends a close command message.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_snd_stream_close(tAVDT_SCB* p_scb, tAVDT_SCB_EVT* p_data) {
  osi_free_and_reset((void**)&p_scb->p_pkt);
  avdt_scb_snd_close_req(p_scb, p_data);
}

/*******************************************************************************
 *
 * Function         avdt_scb_snd_close_rsp
 *
 * Description      This function sends a close response message.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_snd_close_rsp(tAVDT_SCB* p_scb, tAVDT_SCB_EVT* p_data) {
  avdt_msg_send_rsp(p_scb->p_ccb, AVDT_SIG_CLOSE, &p_data->msg);
}

/*******************************************************************************
 *
 * Function         avdt_scb_snd_getconfig_req
 *
 * Description      This function sends a get configuration command message.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_snd_getconfig_req(tAVDT_SCB* p_scb,
                                UNUSED_ATTR tAVDT_SCB_EVT* p_data) {
  tAVDT_EVT_HDR hdr;

  hdr.seid = p_scb->peer_seid;

  tAVDT_MSG avdt_msg;
  avdt_msg.hdr = hdr;
  avdt_msg_send_cmd(p_scb->p_ccb, p_scb, AVDT_SIG_GETCONFIG, &avdt_msg);
}

/*******************************************************************************
 *
 * Function         avdt_scb_snd_getconfig_rsp
 *
 * Description      This function sends a get configuration response message.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_snd_getconfig_rsp(tAVDT_SCB* p_scb, tAVDT_SCB_EVT* p_data) {
  avdt_msg_send_rsp(p_scb->p_ccb, AVDT_SIG_GETCONFIG, &p_data->msg);
}

/*******************************************************************************
 *
 * Function         avdt_scb_snd_open_req
 *
 * Description      This function sends an open command message.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_snd_open_req(tAVDT_SCB* p_scb,
                           UNUSED_ATTR tAVDT_SCB_EVT* p_data) {
  tAVDT_EVT_HDR hdr;

  hdr.seid = p_scb->peer_seid;

  tAVDT_MSG avdt_msg;
  avdt_msg.hdr = hdr;
  avdt_msg_send_cmd(p_scb->p_ccb, p_scb, AVDT_SIG_OPEN, &avdt_msg);
}

/*******************************************************************************
 *
 * Function         avdt_scb_snd_open_rsp
 *
 * Description      This function sends an open response message.  It also
 *                  calls avdt_ad_open_req() to accept a transport channel
 *                  connection.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_snd_open_rsp(tAVDT_SCB* p_scb, tAVDT_SCB_EVT* p_data) {
  /* notify adaption that we're waiting for transport channel open */
  p_scb->role = AVDT_OPEN_ACP;
  avdt_ad_open_req(AVDT_CHAN_MEDIA, p_scb->p_ccb, p_scb, AVDT_ACP);

  /* send response */
  avdt_msg_send_rsp(p_scb->p_ccb, AVDT_SIG_OPEN, &p_data->msg);

  alarm_set_on_mloop(p_scb->transport_channel_timer,
                     AVDT_SCB_TC_CONN_TIMEOUT_MS,
                     avdt_scb_transport_channel_timer_timeout, p_scb);
}

/*******************************************************************************
 *
 * Function         avdt_scb_snd_reconfig_req
 *
 * Description      This function stores the configuration parameters in the
 *                  SCB and sends a reconfiguration command message.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_snd_reconfig_req(tAVDT_SCB* p_scb, tAVDT_SCB_EVT* p_data) {
  AVDT_TRACE_DEBUG("%s: p_scb->peer_seid=%d p_data->msg.hdr.seid=%d", __func__,
                   p_scb->peer_seid, p_data->msg.hdr.seid);
  A2DP_DumpCodecInfo(p_data->msg.config_cmd.p_cfg->codec_info);

  memcpy(&p_scb->req_cfg, p_data->msg.config_cmd.p_cfg, sizeof(tAVDT_CFG));
  p_data->msg.hdr.seid = p_scb->peer_seid;
  avdt_msg_send_cmd(p_scb->p_ccb, p_scb, AVDT_SIG_RECONFIG, &p_data->msg);
}

/*******************************************************************************
 *
 * Function         avdt_scb_snd_reconfig_rsp
 *
 * Description      This function stores the configuration parameters in the
 *                  SCB and sends a reconfiguration response message.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_snd_reconfig_rsp(tAVDT_SCB* p_scb, tAVDT_SCB_EVT* p_data) {
  if (p_data->msg.hdr.err_code == 0) {
    /* store new configuration */
    if (p_scb->req_cfg.num_codec > 0) {
      p_scb->curr_cfg.num_codec = p_scb->req_cfg.num_codec;
      memcpy(p_scb->curr_cfg.codec_info, p_scb->req_cfg.codec_info,
             AVDT_CODEC_SIZE);
    }
    if (p_scb->req_cfg.num_protect > 0) {
      p_scb->curr_cfg.num_protect = p_scb->req_cfg.num_protect;
      memcpy(p_scb->curr_cfg.protect_info, p_scb->req_cfg.protect_info,
             AVDT_PROTECT_SIZE);
    }

    /* send response */
    avdt_msg_send_rsp(p_scb->p_ccb, AVDT_SIG_RECONFIG, &p_data->msg);
  } else {
    /* send reject */
    avdt_msg_send_rej(p_scb->p_ccb, AVDT_SIG_RECONFIG, &p_data->msg);
  }
}

/*******************************************************************************
 *
 * Function         avdt_scb_snd_security_req
 *
 * Description      This function sends a security command message.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_snd_security_req(tAVDT_SCB* p_scb, tAVDT_SCB_EVT* p_data) {
  p_data->msg.hdr.seid = p_scb->peer_seid;
  avdt_msg_send_cmd(p_scb->p_ccb, p_scb, AVDT_SIG_SECURITY, &p_data->msg);
}

/*******************************************************************************
 *
 * Function         avdt_scb_snd_security_rsp
 *
 * Description      This function sends a security response message.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_snd_security_rsp(tAVDT_SCB* p_scb, tAVDT_SCB_EVT* p_data) {
  if (p_data->msg.hdr.err_code == 0) {
    avdt_msg_send_rsp(p_scb->p_ccb, AVDT_SIG_SECURITY, &p_data->msg);
  } else {
    avdt_msg_send_rej(p_scb->p_ccb, AVDT_SIG_SECURITY, &p_data->msg);
  }
}

/*******************************************************************************
 *
 * Function         avdt_scb_snd_setconfig_rej
 *
 * Description      This function marks the SCB as not in use and sends a
 *                  set configuration reject message.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_snd_setconfig_rej(tAVDT_SCB* p_scb, tAVDT_SCB_EVT* p_data) {
  AVDT_TRACE_DEBUG("%s: ", __func__);
  if (p_scb->p_ccb != NULL) {
    AVDT_TRACE_DEBUG("%s: p_scb->p_ccb is null", __func__);
    avdt_msg_send_rej(p_scb->p_ccb, AVDT_SIG_SETCONFIG, &p_data->msg);

    /* clear scb variables */
    avdt_scb_clr_vars(p_scb, p_data);
  }
}

/*******************************************************************************
 *
 * Function         avdt_scb_snd_setconfig_req
 *
 * Description      This function marks the SCB as in use and copies the
 *                  configuration parameters to the SCB.  Then the function
 *                  sends a set configuration command message and initiates
 *                  opening of the signaling channel.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_snd_setconfig_req(tAVDT_SCB* p_scb, tAVDT_SCB_EVT* p_data) {
  tAVDT_CFG *p_req, *p_cfg;

  AVDT_TRACE_DEBUG("%s", __func__);
  A2DP_DumpCodecInfo(p_data->msg.config_cmd.p_cfg->codec_info);

  /* copy API parameters to scb, set scb as in use */
  p_scb->in_use = true;
  p_scb->p_ccb = avdt_ccb_by_idx(p_data->msg.config_cmd.hdr.ccb_idx);
  p_scb->peer_seid = p_data->msg.config_cmd.hdr.seid;
  p_req = p_data->msg.config_cmd.p_cfg;
  p_cfg = &p_scb->cs.cfg;
  memcpy(&p_scb->req_cfg, p_data->msg.config_cmd.p_cfg, sizeof(tAVDT_CFG));
  avdt_set_scbs_busy(p_scb);
  avdt_msg_send_cmd(p_scb->p_ccb, p_scb, AVDT_SIG_SETCONFIG, &p_data->msg);

  /* tell ccb to open channel */
  avdt_ccb_event(p_scb->p_ccb, AVDT_CCB_UL_OPEN_EVT, NULL);
}

/*******************************************************************************
 *
 * Function         avdt_scb_snd_setconfig_rsp
 *
 * Description      This function copies the requested configuration into the
 *                  current configuration and sends a set configuration
 *                  response message.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_snd_setconfig_rsp(tAVDT_SCB* p_scb, tAVDT_SCB_EVT* p_data) {
  if (p_scb->p_ccb != NULL) {
    memcpy(&p_scb->curr_cfg, &p_scb->req_cfg, sizeof(tAVDT_CFG));
    p_scb->role = AVDT_CONF_ACP;

    avdt_msg_send_rsp(p_scb->p_ccb, AVDT_SIG_SETCONFIG, &p_data->msg);
    if ((p_scb->cs.tsep == AVDT_TSEP_SNK) && (p_scb->curr_cfg.psc_mask & AVDT_PSC_DELAY_RPT)) {
      reported_delay = INIT_DELAY_RPT;
      AVDT_TRACE_DEBUG(" %s ~~ support DELAY_RPT , begin init Delay report procedure", __func__);
      AVDT_DelayReport(avdt_scb_to_hdl(p_scb), p_scb->peer_seid, reported_delay);
    }
  }
}

/*******************************************************************************
 *
 * Function         avdt_scb_snd_tc_close
 *
 * Description      This function calls avdt_ad_close_req() to close the
 *                  transport channel for this SCB.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_snd_tc_close(tAVDT_SCB* p_scb,
                           UNUSED_ATTR tAVDT_SCB_EVT* p_data) {
#if (AVDT_REPORTING == TRUE)
  if (p_scb->curr_cfg.psc_mask & AVDT_PSC_REPORT)
    avdt_ad_close_req(AVDT_CHAN_REPORT, p_scb->p_ccb, p_scb);
#endif
  avdt_ad_close_req(AVDT_CHAN_MEDIA, p_scb->p_ccb, p_scb);
}

/*******************************************************************************
 *
 * Function         avdt_scb_cb_err
 *
 * Description      This function calls the application callback function
 *                  indicating an error.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_cb_err(tAVDT_SCB* p_scb, UNUSED_ATTR tAVDT_SCB_EVT* p_data) {
  tAVDT_CTRL avdt_ctrl;

  /* set error code and parameter */
  avdt_ctrl.hdr.err_code = AVDT_ERR_BAD_STATE;
  avdt_ctrl.hdr.err_param = 0;

  /* call callback, using lookup table to get callback event */
  (*p_scb->cs.p_ctrl_cback)(avdt_scb_to_hdl(p_scb), NULL,
                            avdt_scb_cback_evt[p_scb->curr_evt], &avdt_ctrl);
}

/*******************************************************************************
 *
 * Function         avdt_scb_cong_state
 *
 * Description      This function sets the congestion state of the SCB media
 *                  transport channel.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_cong_state(tAVDT_SCB* p_scb, tAVDT_SCB_EVT* p_data) {
  p_scb->cong = p_data->llcong;
}

/*******************************************************************************
 *
 * Function         avdt_scb_rej_state
 *
 * Description      This function sends a reject message to the peer indicating
 *                  incorrect state for the received command message.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_rej_state(UNUSED_ATTR tAVDT_SCB* p_scb, tAVDT_SCB_EVT* p_data) {
  AVDT_TRACE_DEBUG("%s: ", __func__);
  p_data->msg.hdr.err_code = AVDT_ERR_BAD_STATE;
  p_data->msg.hdr.err_param = 0;
  avdt_msg_send_rej(avdt_ccb_by_idx(p_data->msg.hdr.ccb_idx),
                    p_data->msg.hdr.sig_id, &p_data->msg);
}

/*******************************************************************************
 *
 * Function         avdt_scb_rej_in_use
 *
 * Description      This function sends a reject message to the peer indicating
 *                  the stream is in use.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_rej_in_use(UNUSED_ATTR tAVDT_SCB* p_scb, tAVDT_SCB_EVT* p_data) {
  AVDT_TRACE_DEBUG("%s: ", __func__);
  p_data->msg.hdr.err_code = AVDT_ERR_IN_USE;
  p_data->msg.hdr.err_param = 0;
  avdt_msg_send_rej(avdt_ccb_by_idx(p_data->msg.hdr.ccb_idx),
                    p_data->msg.hdr.sig_id, &p_data->msg);
}

/*******************************************************************************
 *
 * Function         avdt_scb_rej_not_in_use
 *
 * Description      This function sends a reject message to the peer indicating
 *                  the stream is in use.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_rej_not_in_use(UNUSED_ATTR tAVDT_SCB* p_scb,
                             tAVDT_SCB_EVT* p_data) {
  AVDT_TRACE_DEBUG("%s: ", __func__);
  p_data->msg.hdr.err_code = AVDT_ERR_NOT_IN_USE;
  p_data->msg.hdr.err_param = 0;
  avdt_msg_send_rej(avdt_ccb_by_idx(p_data->msg.hdr.ccb_idx),
                    p_data->msg.hdr.sig_id, &p_data->msg);
}

/*******************************************************************************
 *
 * Function         avdt_scb_set_remove
 *
 * Description      This function marks an SCB to be removed.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_set_remove(tAVDT_SCB* p_scb, UNUSED_ATTR tAVDT_SCB_EVT* p_data) {
  AVDT_TRACE_DEBUG("%s: ", __func__);
  p_scb->remove = true;
}

/*******************************************************************************
 *
 * Function         avdt_scb_free_pkt
 *
 * Description      This function frees the media packet passed in.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_free_pkt(tAVDT_SCB* p_scb, tAVDT_SCB_EVT* p_data) {
  tAVDT_CTRL avdt_ctrl;

  /* set error code and parameter */
  avdt_ctrl.hdr.err_code = AVDT_ERR_BAD_STATE;
  avdt_ctrl.hdr.err_param = 0;

  osi_free_and_reset((void**)&p_data->apiwrite.p_buf);

  AVDT_TRACE_WARNING("Dropped media packet");

  /* we need to call callback to keep data flow going */
  (*p_scb->cs.p_ctrl_cback)(avdt_scb_to_hdl(p_scb), NULL, AVDT_WRITE_CFM_EVT,
                            &avdt_ctrl);
}

/*******************************************************************************
 *
 * Function         avdt_scb_clr_pkt
 *
 * Description      This function frees the media packet stored in the SCB.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_clr_pkt(tAVDT_SCB* p_scb, UNUSED_ATTR tAVDT_SCB_EVT* p_data) {
  tAVDT_CTRL avdt_ctrl;
  tAVDT_CCB* p_ccb;
  uint8_t tcid;
  uint16_t lcid;

  /* set error code and parameter */
  avdt_ctrl.hdr.err_code = AVDT_ERR_BAD_STATE;
  avdt_ctrl.hdr.err_param = 0;
  /* flush the media data queued at L2CAP */
  p_ccb = p_scb->p_ccb;
  if (p_ccb != NULL) {
    /* get tcid from type, scb */
    tcid = avdt_ad_type_to_tcid(AVDT_CHAN_MEDIA, p_scb);

    lcid = avdt_cb.ad.rt_tbl[avdt_ccb_to_idx(p_ccb)][tcid].lcid;
    L2CA_FlushChannel(lcid, L2CAP_FLUSH_CHANS_ALL);
  }

  if (p_scb->p_pkt != NULL) {
    osi_free_and_reset((void**)&p_scb->p_pkt);

    AVDT_TRACE_DEBUG("Dropped stored media packet");

    /* we need to call callback to keep data flow going */
    (*p_scb->cs.p_ctrl_cback)(avdt_scb_to_hdl(p_scb), NULL, AVDT_WRITE_CFM_EVT,
                              &avdt_ctrl);
  }
}

/*******************************************************************************
 *
 * Function         avdt_scb_chk_snd_pkt
 *
 * Description      This function checks if the SCB is congested, and if not
 *                  congested it sends a stored media packet, if any.  After it
 *                  sends the packet it calls the application callback function
 *                  with a write confirm.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_chk_snd_pkt(tAVDT_SCB* p_scb, UNUSED_ATTR tAVDT_SCB_EVT* p_data) {
  tAVDT_CTRL avdt_ctrl;
  BT_HDR* p_pkt;

  avdt_ctrl.hdr.err_code = 0;

  if (!p_scb->cong) {
    if (p_scb->p_pkt != NULL) {
      p_pkt = p_scb->p_pkt;
      p_scb->p_pkt = NULL;
      avdt_ad_write_req(AVDT_CHAN_MEDIA, p_scb->p_ccb, p_scb, p_pkt);

      (*p_scb->cs.p_ctrl_cback)(avdt_scb_to_hdl(p_scb), NULL,
                                AVDT_WRITE_CFM_EVT, &avdt_ctrl);
    }
  }
}

/*******************************************************************************
 *
 * Function         avdt_scb_transport_channel_timer
 *
 * Description      This function is called to start a timer when the peer
 *                  initiates closing of the stream.  The timer verifies that
 *                  the peer disconnects the transport channel.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_transport_channel_timer(tAVDT_SCB* p_scb,
                                      UNUSED_ATTR tAVDT_SCB_EVT* p_data) {
  alarm_set_on_mloop(p_scb->transport_channel_timer,
                     AVDT_SCB_TC_DISC_TIMEOUT_MS,
                     avdt_scb_transport_channel_timer_timeout, p_scb);
}

/*******************************************************************************
 *
 * Function         avdt_scb_clr_vars
 *
 * Description      This function initializes certain SCB variables.
 *
 * Returns          Nothing.
 *
 ******************************************************************************/
void avdt_scb_clr_vars(tAVDT_SCB* p_scb, UNUSED_ATTR tAVDT_SCB_EVT* p_data) {
  AVDT_TRACE_DEBUG("%s: Initializes certain SCB variables", __func__);
  avdt_set_scbs_free(p_scb);
  p_scb->in_use = false;
  p_scb->p_ccb->p_proc_data = NULL;
  p_scb->p_ccb = NULL;
  p_scb->peer_seid = 0;
}
