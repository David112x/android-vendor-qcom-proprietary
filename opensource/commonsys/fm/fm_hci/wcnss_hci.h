/*
 * Copyright (c) 2015, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *        * Redistributions of source code must retain the above copyright
 *            notice, this list of conditions and the following disclaimer.
 *        * Redistributions in binary form must reproduce the above
 *            copyright notice, this list of conditions and the following
 *            disclaimer in the documentation and/or other materials provided
 *            with the distribution.
 *        * Neither the name of The Linux Foundation nor the names of its
 *            contributors may be used to endorse or promote products derived
 *            from this software without specific prior written permission.
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

#ifndef __WCNSS_HCI__
#define __WCNSS_HCI__
static void vendor_fwcfg_cb(bt_vendor_op_result_t result) {
}

static void vendor_scocfg_cb(bt_vendor_op_result_t result) {
}

static void vendor_lpm_vnd_cb(bt_vendor_op_result_t result) {
}

static void sco_audiostate_cb(bt_vendor_op_result_t result) {
}

static void* vendor_alloc(int size) {
    return NULL;
}

static void vendor_dealloc(void *p_buf) {
}

static uint8_t vendor_xmit_cb(uint16_t opcode, void *p_buf, tINT_CMD_CBACK p_cback) {
    return 0;
}

static void vendor_epilog_cb(bt_vendor_op_result_t result) {
}


static const bt_vendor_callbacks_t fm_vendor_callbacks = {
  sizeof(fm_vendor_callbacks),
   vendor_fwcfg_cb,
   vendor_scocfg_cb,
   vendor_lpm_vnd_cb,
   sco_audiostate_cb,
   vendor_alloc,
   vendor_dealloc,
   vendor_xmit_cb,
   vendor_epilog_cb
};
#endif
