/* eebin_dev.h
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __EEBIN_DEV_H__
#define __EEBIN_DEV_H__

#define MAX_MODULE_NAME (64)
#define MAX_MODULES (32)
#define PATH_SIZE_255 255
#define MAX_DEVICES (32)

/*
==============================================================================
BUFFER HEADER
- version
- version_crc
- num modules
- module offsets
==============================================================================
MODULE DEVICES #0

MODULE HEADER
Module description
- version
- name
- max_csi_lanes
Sensor Description
- name
- format
- version
- aspect ratioo
- sixe w
- sixe h
Actuator Description
- name
- version
- type
Eeprom Description
- name
- version
- size
------------------------------------------------------------------------------
DEVICE HEADER #0
- type
- version
- name
- path
- size
------------------------------------------------------------------------------
DEVICE #0 LIB
-lib bin data
------------------------------------------------------------------------------
------------------------------------------------------------------------------
DEVICE HEADER #1
- type
- version
- name
- path
- size
------------------------------------------------------------------------------
DEVICE #1 LIB
-lib bin data
------------------------------------------------------------------------------
~ ~ ~ ~ ~
------------------------------------------------------------------------------
DEVICE HEADER #n
- type
- version
- name
- path
- size
------------------------------------------------------------------------------
DEVICE #n LIB
-lib bin data
------------------------------------------------------------------------------
==============================================================================
MODULE DEVICES #1
~
==============================================================================
MODULE DEVICES #n
~
==============================================================================
*/

/*
 * MASTER HEADER
 */
typedef struct {
  uint32_t      version; /*pack version*/
  uint32_t      version_crc;
  uint32_t      num_modules;
  uint32_t      modules_offsets[MAX_MODULES];
  /* custom parse data*/
  char          name[MAX_MODULE_NAME];
  char          lib_name[MAX_MODULE_NAME];
  char          path[PATH_SIZE_255];
  uint32_t      size;
} bin_file_header_t;

/*
 * MODULE HEADER
 */
typedef enum{
  BIN_SENSOR_OUT_INVALID = -1,
  BIN_SENSOR_OUT_BAYER,
  BIN_SENSOR_OUT_YUV,
  BIN_SENSOR_OUT_JPEG,
}output_format_t;

typedef enum{
  BIN_ACTUATOR_VCM,
  BIN_ACTUATOR_PIEZO,
  BIN_ACTUATOR_MEMS,
}actuator_type_t;

typedef struct {
  /*camera module HW version*/
  uint32_t version;
  char name[MAX_MODULE_NAME];
  uint32_t max_csi_lanes;
  uint32_t max_devices;
  uint32_t size;
  struct {
    char name[MAX_MODULE_NAME];
    output_format_t format;
    /*sensor module HW version*/
    uint32_t version;
    uint32_t aspect_ratio;
    struct{
      uint32_t w;
      uint32_t h;
    }size;
  }sensor;
  struct {
    char name[MAX_MODULE_NAME];
    /*sensor module HW version*/
    uint32_t version;
    actuator_type_t type;
  }actuator;
  struct {
    char name[MAX_MODULE_NAME];
    /*sensor module HW version*/
    uint32_t version;
    uint32_t size;
  }eeprom;
}bin_module_header_t;

typedef enum {
  BIN_DEV_INVALID,
  BIN_DEV_SENSOR,
  BIN_DEV_ACTUATOR,
  BIN_DEV_EEPROM,
  BIN_DEV_CHROMATIX,
  BIN_DEV_MAX,
} bin_dev_type_t;


/*
 * DEVICE HEADER
 */
typedef struct {
  bin_dev_type_t type;
  /*sensor driver source code version*/
  uint32_t version;
  /*sensor/actuarot/eeprom name*/
  char name[MAX_MODULE_NAME];
  char lib_name[MAX_MODULE_NAME];
  /*sensor/actuarot/eeprom name*/
  char path[PATH_SIZE_255];
  uint32_t size;
} bin_device_header_t;

typedef struct {
  bin_file_header_t master_h;
  struct moddev{
    bin_module_header_t module_h;
    bin_device_header_t device_h[MAX_DEVICES];
  }moddev[MAX_MODULES];
} bin_device_t;

#endif

