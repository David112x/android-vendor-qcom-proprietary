/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

typedef struct {
    int level_value;
    int bit_rate_value;
} bit_rate_level_48k_96k_table_t;

static const bit_rate_level_48k_96k_table_t bit_rate_level_48k_96k_database[] = {
    /* Level to Bit rate for 48k, 96k sample freq, in kbps */
    {5, 990000},
    {4, 660000},
    {3, 492000},
    {2, 396000},
    {1, 330000},
};


typedef struct {
    int level_value;
    int bit_rate_value;
} bit_rate_level_44_1k_88_2k_table_t;

static const bit_rate_level_44_1k_88_2k_table_t bit_rate_level_44_1k_88_2k_database[] = {
    /* Level to Bit rate for 44.1k, 88.2k sample freq, in kbps */
    {5, 909000},
    {4, 606000},
    {3, 452000},
    {2, 363000},
    {1, 303000},
};


