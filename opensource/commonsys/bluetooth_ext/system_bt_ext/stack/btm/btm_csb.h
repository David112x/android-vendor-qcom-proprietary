/*
 * Copyright (c) 2017, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of The Linux Foundation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
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
 
/* Connectionless Broadcast Events*/
#define HCI_CSB_TIMEOUT_EVT  0x52  //Connectionless Slave Broadcast Timeout Event
#define HCI_SLAVE_PAGE_RESPONSE_TIMEOUT_EVT 0x54  // Slave Page Response Timeout
#define HCI_CSB_CHANNEL_MAP_CHANGE_EVT  0x55  /* Connectionless Slave Broadcast
                                                Channel Map Change Event */
#define HCI_SYNCHRONIZATION_TRAIN_COMPLETE_EVENT 0x4F

extern void btm_hci_csb_timeout_evt(uint8_t* p);
extern void btm_hci_start_sync_train_complete(uint8_t* p);
extern void btm_hci_set_reserved_lt_addr_complete(uint8_t* p);
extern void btm_hci_delete_reserved_lt_addr_complete(uint8_t* p);
extern void btm_hci_write_sync_train_param_complete(uint8_t* p);
extern void btm_hci_set_csb_complete(uint8_t* p);
