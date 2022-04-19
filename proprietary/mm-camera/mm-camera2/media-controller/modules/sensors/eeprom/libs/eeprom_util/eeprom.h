/* eeprom.h
 *
 * Copyright (c) 2012-2015 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __EEPROM_H__
#define __EEPROM_H__

void eeprom_get_calibration_items(void *e_ctrl);
int32_t eeprom_format_calibration_data(void *e_ctrl);
int eeprom_autofocus_calibration(void *e_ctrl);
void eeprom_print_matrix(float *paramlist);
void eeprom_lensshading_calibration(void *e_ctrl);
void eeprom_whitebalance_calibration(void *e_ctrl);
void eeprom_whitebalance_mlczone_calibration(void *e_ctrl);
void eeprom_whitebalance_eoh_calibration(void *e_ctrl);
void eeprom_whitebalance_sct_calibration(void *e_ctrl);
void eeprom_whitebalance_green_calibration(void *e_ctrl);
int32_t eeprom_get_raw_data(void *e_ctrl, void *data);
int eeprom_do_af_calibration(void *ctrl);
void eeprom_do_chroma_calibration(void *ctrl);
float wbgain_calibration(float gain, float calib_factor);

#endif
