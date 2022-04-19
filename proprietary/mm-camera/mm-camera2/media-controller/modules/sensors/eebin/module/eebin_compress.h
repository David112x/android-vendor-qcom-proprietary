
/* eeprom_compress.h
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __EEPROM_COMPRESS_H__
#define __EEPROM_COMPRESS_H__

int32_t un_compress(char* in, uint32_t in_len,
                char** out, uint32_t* out_len);

#endif
