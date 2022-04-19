/*
 * Copyright (C) 2017 The Linux Foundation. All rights reserved
 * Not a Contribution.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the
 * disclaimer below) provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *
 * * Neither the name of The Linux Foundation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
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

/*
 *  Copyright (C) 2009-2012 Broadcom Corporation
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

#ifndef BTIF_VENDOR_H
#define BTIF_VENDOR_H

#include <hardware/vendor.h>
#include "bta_api.h"

#define BTIF_STORAGE_PATH_TWS_PLUS_DEV_TYPE "TwsPlusDeviceType"
#define BTIF_STORAGE_PATH_TWS_PLUS_PEER_ADDR "TwsPlusPeerAddr"

extern btvendor_callbacks_t *bt_vendor_callbacks;

enum { LK_DERIVATION_REASON_PAIR, LK_DERIVATION_REASON_REPLACE_EB };

typedef uint8_t tLK_DERIVATION_REASON;

extern bool twsplus_enabled;

/*******************************************************************************
 *  Functions
 ******************************************************************************/

bool btif_tws_plus_set_peer_eb_addr(RawAddress *addr, RawAddress *peer_addr);
bool btif_tws_plus_get_peer_eb_addr(RawAddress *remote_bd_addr,
                                            RawAddress *peer_bd_addr);

bool btif_tws_plus_set_dev_type(RawAddress *addr, int tws_plus_dev_type);
bool btif_tws_plus_get_dev_type(RawAddress *addr, int *tws_plus_dev_type);

bool btif_is_tws_plus_device(const RawAddress *remote_bd_addr);

bool btif_tws_plus_derive_link_key ( RawAddress eb_addr, RawAddress peer_eb_addr,
                    LinkKey src_key, tLK_DERIVATION_REASON reason);

bool btif_tws_plus_process_eir(tBTA_DM_SEARCH *p_search_data,
                                        RawAddress *peer_eb_bd_addr);
bool btif_tws_plus_get_services(RawAddress *bd_addr);
bt_status_t btif_tws_plus_replace_earbud ( RawAddress *addr,
                                    RawAddress* peer_bd_addr);
bt_status_t btif_tws_plus_execute_service(bool b_enable);
bool btif_tws_plus_load_tws_devices(void);

#endif
