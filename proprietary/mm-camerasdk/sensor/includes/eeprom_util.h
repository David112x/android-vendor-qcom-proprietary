/* eeprom_util.h
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __EEPROM_UTIL_H__
#define __EEPROM_UTIL_H__

int eeprom_util_check_crc16(
  unsigned char *data, unsigned int size, unsigned int ref_crc);

void mesh_rolloff_V4_ScaleOTPMesh_to_13x17(float *MeshIn,
  float *MeshOut, int width, int height, int num_x, int num_y, int offset_x,
  int offset_y, float block_x, float block_y);

void mesh_rolloff_V4_ScaleOTPMesh_to_10x13(float *MeshIn,
  float *MeshOut, int width, int height, int num_x, int num_y, int offset_x,
  int offset_y, float block_x, float block_y);

int eeprom_util_add_margin(sensor_eeprom_data_t *e_ctrl);

#endif /* __EEPROM_UTIL_H__ */
