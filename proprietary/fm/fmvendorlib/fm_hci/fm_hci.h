/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Copyright (c) 2015-2017 The Linux Foundation. All rights reserved.
 *
 */

#ifndef __FM_HCI__
#define __FM_HCI__

#include "fm_hci_api.h"

#define FM_CMD_COMPLETE 0x0f
#define FM_CMD_STATUS   0x10
#define FM_HW_ERR_EVENT 0x1A
#define HCI_TIMEOUT 3
struct fm_hci_t {
    public:
        fm_power_state_t state;
        std::condition_variable on_cond;
        std::mutex on_mtx;

        bool is_tx_thread_running;
        bool is_rx_thread_running;

        std::condition_variable tx_cond;
        std::mutex tx_cond_mtx;

        std::condition_variable rx_cond;
        std::mutex rx_cond_mtx;

        std::mutex tx_queue_mtx;

        std::condition_variable cmd_credits_cond;

        std::queue<struct fm_command_header_t *> tx_cmd_queue;
        std::queue<struct fm_event_header_t *> rx_event_queue;

        volatile uint16_t command_credits;
        struct fm_hci_callbacks_t *cb;

        std::thread tx_thread_;
        std::thread rx_thread_;
};

#endif

