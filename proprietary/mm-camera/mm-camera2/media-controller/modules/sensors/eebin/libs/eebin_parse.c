/* eebin_parse.c
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include <string.h>
#include <media/msm_cam_sensor.h>
#include "../module/include/eebin_lib.h"


#define VERSION_NAME "eebinparse v0.1"
#define VERSION_NUM 0x11111234
#define VERSION_CRC(x) ((((x>>0)&0xF)+ \
                        ((x>>4)&0xF)+ \
                        ((x>>8)&0xF)+ \
                        ((x>>16)&0xF)+ \
                        ((x>>20)&0xF)+ \
                        ((x>>24)&0xF)+ \
                        ((x>>28)&0xF)) + 0xA)

#define PATH_SIZE_255 255


static int32_t eebin_parse_data(bin_device_t *bin_device, char *buff,
  __attribute__((unused)) int32_t size, write_lib_file_f func_w)
{
  int32_t rc = 0;
  uint32_t i, j,k;
  uint32_t num_modules, num_mod;
  char path[PATH_SIZE_255] = {0};
  char *lib_name;
  char *buff_w = NULL;
  bin_file_header_t *master_h;
  bin_module_header_t *module_h;
  bin_device_header_t *device_h;

  master_h = (bin_file_header_t *)buff;

  bin_device->master_h = *master_h;

  for (i = 0; i < master_h->num_modules; i++) {
    num_mod = bin_device->master_h.num_modules;
    module_h = (bin_module_header_t *)(buff + master_h->modules_offsets[i]);

    bin_device->master_h.modules_offsets[num_mod] =
      bin_device->master_h.modules_offsets[num_mod-1] +
      module_h->size;

      bin_device->moddev[num_mod].module_h = *module_h;

    for (j = 0; j < module_h->max_devices; j++){
      if (!j)
        device_h = (bin_device_header_t *)
          ((char*)module_h+sizeof(bin_module_header_t));
      else
        device_h = (bin_device_header_t *)
          ((char*)device_h+sizeof(bin_device_header_t)+device_h->size);
      if (device_h->type) {

        bin_device->moddev[num_mod].device_h[j] = *device_h;

        buff_w = (char*)device_h + sizeof(bin_device_header_t);
        lib_name = bin_device->moddev[num_mod].device_h[j].lib_name;

        memset(path, 0, sizeof(path));
        if (func_w)
          rc = func_w(path, lib_name, buff_w, device_h->size);
        if (rc < 0)
          return rc;

        memcpy(bin_device->moddev[num_mod].device_h[j].path,
          path, sizeof(device_h->path));
      }
    }
    bin_device->master_h.num_modules++;
  }
  return rc;
}

static eebinparse_lib eebinparse_lib_ptr = {
  .version_num = VERSION_NUM,
  .version_crc = VERSION_CRC(VERSION_NUM),
  .name = VERSION_NAME,
  .parse_f = eebin_parse_data,
};

/*===========================================================================
 * FUNCTION    - eebinparse_open_lib -
 *
 * DESCRIPTION:
 *==========================================================================*/
void* eebinparse_open_lib(void) {
  return &eebinparse_lib_ptr;
}

