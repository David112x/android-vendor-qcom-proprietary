/*
 * Copyright (c) 2017-2018, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the
 * disclaimer below) provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 *  * Neither the name of The Linux Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
 * GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
 * HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __BTA_AG_TWSPLUS_DEV_H_
#define __BTA_AG_TWSPLUS_DEV_H_

#include "bta_ag_int.h"

#define MAX_TWSPLUS_DEVICES 2
#define PRIMARY_EB_IDX 0
#define SECONDARY_EB_IDX 1

#define TWSPLUS_MIN_BATTERY_CHARGE 0
#define TWSPLUS_MIN_BATTERY_CHARGE_STATE_CHARGING 0
#define TWSPLUS_MIN_BATTERY_CHARGE_STATE_DISCHARGING 1
#define TWSPLUS_MIN_BATTERY_CHARGE_STATE_POWERFAULT 2
#define TWSPLUS_MAX_BATTERY_CHARGE 255

#define TWSPLUS_MIN_BATTERY_LEVEL 0
#define TWSPLUS_MAX_BATTERY_LEVEL 100

#define TWSPLUS_INVALID_MICPATH_DELAY  (-1)
#define TWSPLUS_MAX_MICPATH_DELAY   (65535)
#define TWSPLUS_MIN_MICPATH_DELAY  (0)

#define TWSPLUS_MAX_MIC_QUALITY 15
#define TWSPLUS_MIN_MIC_QUALITY 0

#define TWSPLUS_MIN_QDSP 0
#define TWSPLUS_MAX_QDSP 3

#define TWSPLUS_INVALID_QDSP_VALUE -1
#define TWSPLUS_QDSP_FEATURE_MASK      0x02
#define TWSPLUS_QDSP_VALUE_MASK        0x01
#define TWSPLUS_QDSP_NOISE_REDUCTION   0
#define TWSPLUS_QDSP_ECHO_CANCELLATION 1

enum {
  TWSPLUS_EB_STATE_UNKNOWN,
  TWSPLUS_EB_STATE_INCASE,
  TWSPLUS_EB_STATE_OUT_OF_EAR,
  TWSPLUS_EB_STATE_INEAR,
  TWSPLUS_EB_STATE_OFF
};

enum {
  TWSPLUS_EB_ROLE_LEFT,
  TWSPLUS_EB_ROLE_RIGHT,
  TWSPLUS_EB_ROLE_STEREO,
  TWSPLUS_EB_ROLE_MONO,
  TWSPLUS_EB_ROLE_INVALID
};

typedef struct {
   tBTA_AG_SCB *p_scb;
   uint8_t mic_quality;
   uint16_t battery_state;
   uint16_t battery_level;
   uint8_t state;
   int16_t mic_path_delay;
   uint8_t role;
   uint8_t qdsp_nr;
   uint8_t qdsp_ec;
   uint8_t ring_sent;
} tTWSPLUS_DEVICE;

void update_twsp_device(tBTA_AG_SCB* p_scb);
void twsp_handle_vs_at_events(tBTA_AG_SCB* p_scb, uint16_t cmd,
                                    tBTA_AG_VAL* val , int16_t int_arg);
void reset_twsp_device(int eb_idx);
bool twsp_set_ring_sent(tBTA_AG_SCB *p_scb, bool ring_sent);
void twsp_clr_all_ring_sent();
bool twsp_ring_needed(tBTA_AG_SCB *p_scb);

bool twsp_get_right_eb_addr(RawAddress& eb_addr);
bool twsp_get_left_eb_addr(RawAddress& eb_addr);
uint8_t get_twsp_role(tBTA_AG_SCB *p_scb);
uint8_t get_twsp_state(tBTA_AG_SCB *p_scb);
tBTA_AG_SCB* twsp_get_best_mic_scb ();
int twsp_get_idx_by_scb(tBTA_AG_SCB* p_scb);

#endif//__BTA_AG_TWSPLUS_DEV_H_
