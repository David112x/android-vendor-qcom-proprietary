/*
 *
 * Copyright (c) 2018, The Linux Foundation. All rights reserved.
 *
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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


