/*
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __ASD_EXT__
#define __ASD_EXT__

/**
 * Custom output structure. External core should define
 * the parameters here.
 */
typedef struct {
  uint32_t mask;
  /* Add OEM parameters here */
} asd_ext_output_t;

typedef enum asd_port_ext_update_type_t {
  ASD_DETECTION_FULL_ENABLE, /* Default */
  ASD_DETECTION_ONLY_SCENE_INFO,
  ASD_DETECTION_DISABLE,
} asd_port_ext_update_type_t;

typedef struct asd_port_ext_scene_update_type_t {
  asd_port_ext_update_type_t  snow_scene_detection;
  asd_port_ext_update_type_t  backlight_detection;
  asd_port_ext_update_type_t  landscape_detection;
  asd_port_ext_update_type_t  portrait_detection;
  asd_port_ext_update_type_t  hazy_detection;
  asd_port_ext_update_type_t auto_hdr_detect;
} asd_port_ext_scene_update_type_t;


typedef struct asd_ext_param_t {
  asd_port_ext_scene_update_type_t asd_update_type;
  asd_ext_output_t custom_output;
} asd_ext_param_t;


boolean asd_port_ext_is_extension_required(asd_port_private_t *private);
boolean asd_port_ext_update_port_func_table(void *ptr);
void * asd_port_ext_load_interface(asd_object_t *asd_object);
void asd_port_ext_unload_interface(asd_ops_t *asd_ops, void* asd_handle);

#endif
