/* eeprom_bin.c
 *
 * Copyright (c) 2014,2017,2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <dlfcn.h>
#include "eebin.h"
#include "eebin_interface.h"
#include "eebin_compress.h"
#include "sensor_common.h"

//#define USE_DEBUG_BUFF
#undef USE_DEBUG_BUFF

#define DATA "/data"

#define PATH_SIZE_255 255
#define BUFF_SIZE_255 255

#define VERSION_CRC(x) ((((x>>0)&0xF)+ \
                       ((x>>4)&0xF)+ \
                       ((x>>8)&0xF)+ \
                       ((x>>16)&0xF)+ \
                       ((x>>20)&0xF)+ \
                       ((x>>24)&0xF)+ \
                       ((x>>28)&0xF)) + 0xA)

#ifdef USE_DEBUG_BUFF
char test_buff[] = {
  #include "bin2h_comp.h"
};
#endif

/** eebin_get_parse_version: Geting parse version
 *
 *  @eebin_hdl: eebin module handle
 *  @buff: CMM data buffer
 *
 *  Return: Version number
 **/
static uint32_t eebin_get_parse_version(eebin_hdl_t *eebin_hdl, char *buff)
{
  bin_file_header_t *master_h;
  uint32_t v_num, v_num_crc, v_num_crc_l;

  if (!buff)
    return 0;
  master_h = (bin_file_header_t *)buff;

  v_num = master_h->version;
  v_num_crc = master_h->version_crc;
  v_num_crc_l = VERSION_CRC(v_num);

  SLOW("Version num: %d, crc:%d, calc crc:%d", v_num, v_num_crc, v_num_crc_l);

  if(v_num_crc != v_num_crc_l) {
    SERR("Bad CRC buff");
    return 0;
  }

  eebin_hdl->version = v_num;
  eebin_hdl->version_crc = v_num_crc;

  return v_num;
}

/** eebin_write_lib_file: Lib file write function
 *
 *  @path: path string
 *  @lib_name: lib name string
 *  @buff: CMM data buffer
 *  @size: CMM data buffer size
 *
 *  Return: 0 for Success
 **/
static int32_t eebin_write_lib_file(char *path, char *lib_name,
                                    char *buff, uint32_t size)
{
  int32_t rc = 0;
  int32_t file;
  char name[64];

  if (!lib_name || !size || !buff)
    return FALSE;

  snprintf(name, sizeof(name), "%s/%s", DATA, lib_name);
  file = open(name, O_RDWR | O_CREAT, 0777);

  if (file < 0 ) {
    SERR("Open file fail  %d, file name %s\n", file, lib_name);
    return rc;
  }

  rc = write(file, buff, size);
  if (rc < 0) {
    SERR("Write file fail  %d\n", rc);
    close(file);
    return rc;
  }

  rc = close(file);
  if (rc < 0 ) {
    SERR("Close file fail  %d\n", rc);
    return rc;
  }

  if (path)
    snprintf(path, PATH_SIZE_255, "%s/", DATA);

  SLOW("Create lib  name %s path:%s\n", name, path);
  return rc;
}

/** eebin_conv_type: Enum convert form eebin sensor to drv sensor
 *
 *  @type: eebin lib type
 *
 *  Return: bin device tyoe
 **/
static bin_dev_type_t eebin_conv_type(eebin_lib_t type)
{
 bin_dev_type_t type_l = BIN_DEV_INVALID;

  switch (type)
  {
    case EEPROM_BIN_LIB_SENSOR:
      type_l = BIN_DEV_SENSOR;
    break;
    case EEPROM_BIN_LIB_EEPROM:
      type_l = BIN_DEV_EEPROM;
    break;
    case EEPROM_BIN_LIB_ACTUATOR:
      type_l = BIN_DEV_ACTUATOR;
    break;
    case EEPROM_BIN_LIB_CHROMATIX:
      type_l = BIN_DEV_CHROMATIX;
    break;

    default:
      SERR("failed: Incorrect type  %d\n", type);
      type_l = BIN_DEV_INVALID;
    break;
  }
  return type_l;
}

/** eebin_load_parselib: Loading parse lib
 *
 *  @eebin_hdl: eebin module handle
 *  @name: parse lib name string
 *  @name: parse lib path string
 *
 *  Return: 0 for Success
 **/
static int32_t eebin_load_parselib(eebin_hdl_t *eebin_hdl,
  char *name, char *path)
{
  char lib_name[BUFF_SIZE_255] = {0};
  char open_lib_str[BUFF_SIZE_255] = {0};
  void *(*eebin_open_lib)(void) = NULL;

  SDBG("enter");

  if (path)
    snprintf(lib_name, PATH_SIZE_255,"%slibmmcamera_%s.so", path, name);
  else
    snprintf(lib_name, MAX_MODULE_NAME, "libmmcamera_%s.so", name);

  SLOW("lib_name %s", lib_name);
  eebin_hdl->eebin_parselib_hdl = dlopen(lib_name, RTLD_NOW);
  if (!eebin_hdl->eebin_parselib_hdl) {
    SLOW("fail");
    return -EINVAL;
  }
  snprintf(open_lib_str, MAX_MODULE_NAME, "%s_open_lib", name);
  *(void **)&eebin_open_lib = dlsym(eebin_hdl->eebin_parselib_hdl,
    open_lib_str);
  if (!eebin_open_lib) {
    SERR("failed");
    return -EINVAL;
  }
  eebin_hdl->eebin_parselib_ptr = (eebinparse_lib *)eebin_open_lib();
  if (!eebin_hdl->eebin_parselib_ptr) {
    SERR("failed");
    return -EINVAL;
  }
  SDBG("exit");
  return 0;
}

/** eebin_get_ext_parse: Get external parse
 *
 *  @eebin_hdl: eebin module handle
 *  @buff: eebin data buffer
 *
 *  Return: Parse function for Success, NULL for fail
 **/
static void* eebin_get_ext_parse(eebin_hdl_t *eebin_hdl, char* buff)
{
  int32_t rc = 0;
  bin_file_header_t *master_h = (bin_file_header_t*)buff;
  char  path[PATH_SIZE_255];
  char *path_l = NULL;
  char *name_l = NULL;
  char *buff_w = NULL;

  SDBG("Enter");

  if (master_h->size) {
    SLOW("parse lib size:%d", master_h->size);
    SLOW("parse lib name:%s", master_h->name);
    SLOW("parse lib path:%s", master_h->path);
    SLOW("parse lib lib_name:%s", master_h->lib_name);

    buff_w = (char*)buff + sizeof(bin_file_header_t);
    memset(path, 0, sizeof(path));
    rc = eebin_write_lib_file((char*)path, master_h->lib_name,
      buff_w, master_h->size);
    if (rc < 0)
      SERR("Write file fail  %d\n", rc);

    memcpy(eebin_hdl->lib_name, master_h->lib_name,
      sizeof(eebin_hdl->lib_name));

    SLOW("Update parse lib name %s\n", eebin_hdl->lib_name);
    name_l = master_h->name;
    path_l = (char*)path;
    memcpy(eebin_hdl->path, path_l, sizeof(eebin_hdl->path));
  } else {
    name_l = "eebinparse";
    path_l = NULL;
  }

  SLOW("name:%s", name_l);
  memcpy(eebin_hdl->name, name_l, strlen(name_l) + 1);
  SLOW("parse lib name:%s path:%s", name_l, path_l);
  rc = eebin_load_parselib(eebin_hdl, name_l, path_l);
  if (rc < 0)
    SERR("Load lib fail  %s\n", eebin_hdl->lib_name);

  if (path_l) {
    snprintf(path, PATH_SIZE_255,"%slibmmcamera_%s.so", path_l, name_l);
    remove(path);
    SLOW("Delete Tmp lib");
  }

  if (eebin_hdl->eebin_parselib_ptr) {
    eebinparse_lib *eebinp_lib =
      (eebinparse_lib *)eebin_hdl->eebin_parselib_ptr;

    SLOW("version num :%d", eebinp_lib->version_num);
    SLOW("version crc :%d", eebinp_lib->version_crc);
    SLOW("Name :%s", eebinp_lib->name);
    if (eebinp_lib->version_num != eebin_hdl->version) {
      return NULL;
    }
    return eebinp_lib->parse_f;
  }

  SDBG("Exit");
  return NULL;
}

/** eebin_get_parse_func: Get external parse
 *
 *  @eebin_hdl: eebin module handle
 *  @buff: eebin data buffer
 *
 *  Return: Parse function for Success, NULL for fail
 **/
static void* eebin_get_parse_func(eebin_hdl_t *eebin_hdl, char *buff)
{
  void *ext_p = NULL;

  if (!buff)
    return NULL;

  ext_p = eebin_get_ext_parse(eebin_hdl, buff);
  if (ext_p) {
    SHIGH("External parser\n");
    return ext_p;
  }

  return NULL;
}

/** eebin_dev_read: Read eebin device data
 *
 *  @eebin_hdl: eebin module handle
 *  @buff: eebin data buffer
 *  @num: eebin data buffer size
 *
 *  Return: True for Success, FALSE for fail
 **/
static boolean eebin_dev_read(eebin_hdl_t *eebin_hdl,
  const char* dev_name, char**buff, uint32_t *num)
{
  uint8_t *buff_l = NULL;
  int32_t dev_fd = 0;
  struct msm_eeprom_cfg_data cfg;

  if (!dev_name) {
#ifdef USE_DEBUG_BUFF
    un_compress(test_buff, sizeof(test_buff),
      &eebin_hdl->out_buff, &eebin_hdl->out_buff_size);

    *num = eebin_hdl->out_buff_size;
    *buff = eebin_hdl->out_buff;
     SERR("size %d",*num);
#endif
    return TRUE;
  }

  dev_fd = open(dev_name, O_RDWR);
  if (dev_fd < 0) {
    SHIGH("Open eeprome dev failed: %s", dev_name);
    return FALSE;
  }

  cfg.cfgtype = CFG_EEPROM_GET_MM_INFO;
  cfg.cfg.get_cmm_data.cmm_support = 0;
  cfg.cfg.get_cmm_data.cmm_compression = 0;
  cfg.cfg.get_cmm_data.cmm_size = 0;
  if (ioctl(dev_fd, VIDIOC_MSM_EEPROM_CFG, &cfg) < 0) {
    SHIGH("VIDIOC_MSM_EEPROM_CFG failed!");
    goto end;
  }

  if (!cfg.cfg.get_cmm_data.cmm_support || !cfg.cfg.get_cmm_data.cmm_size)
    goto end;

  buff_l = malloc(cfg.cfg.get_cmm_data.cmm_size);
  if (!buff_l){
    SERR("%s failed allocating memory\n",__func__);
    goto end;
  }

  cfg.cfgtype = CFG_EEPROM_READ_CAL_DATA;
  cfg.cfg.read_data.num_bytes = cfg.cfg.get_cmm_data.cmm_size;
  cfg.cfg.read_data.dbuffer = buff_l;
  if (ioctl(dev_fd, VIDIOC_MSM_EEPROM_CFG, &cfg) < 0) {
    SERR("CFG_EEPROM_READ_CAL_DATA failed!");
    goto end_free;
  }

  if(cfg.cfg.get_cmm_data.cmm_compression) {
    un_compress((char*)buff_l, cfg.cfg.get_cmm_data.cmm_size,
      &eebin_hdl->out_buff, &eebin_hdl->out_buff_size);
    *num = eebin_hdl->out_buff_size;
    *buff = eebin_hdl->out_buff;
     free(buff_l);
  } else {
    *num = cfg.cfg.get_cmm_data.cmm_size;
    eebin_hdl->out_buff = *buff = (char*)buff_l;
  }
  close(dev_fd);
  return TRUE;
end_free:
  free(buff_l);
end:
  close(dev_fd);
  return FALSE;
}

/** eebin_read: Getting eebin buffer, parse version, get parse func
 *
 *  @eebin: eebin data buffer size
 *  @eebin_hdl: eebin module handle
 *
 *  Return: True for Success, FALSE for fail
 **/
static boolean eebin_read(const char* dev_name, eebin_hdl_t *eebin_hdl)
{
  int32_t rc = 0;
  boolean ret = TRUE;
  uint32_t ver_num, num_bytes = 0;
  char *buff = NULL;

  bin_device_t *bin_device = &eebin_hdl->bin_device;
  int32_t (*parse)(bin_device_t *bin_device, char *buff, uint32_t size,
                               write_lib_file_f f);

  ret = eebin_dev_read(eebin_hdl, dev_name, &buff, &num_bytes);
  if (ret == FALSE) {
    SINFO("failed!");
    return FALSE;
  }

  ver_num = eebin_get_parse_version(eebin_hdl, buff);
  if (ver_num == 0)
    return FALSE;

  parse = eebin_get_parse_func(eebin_hdl, buff);
  if (!parse) {
    SINFO("failed!");
    return FALSE;
  }

  rc = parse(bin_device, buff, num_bytes, eebin_write_lib_file);
  if (rc < 0) {
    SINFO("failed: rc  = %d", rc);
    return FALSE;
  }

  free(eebin_hdl->out_buff);
  return rc;
}

/** eebin_name_data: Getting eebin  name, lib name and path
 *
 *  @eebin: eebin data buffer size
 *  @name_data: query data
 *
 *  Return: 0 for Success
 **/
int32_t eebin_name_data(eebin_hdl_t *eebin_hdl,
  eebin_query_name_t *name_data)
{
  int32_t rc = 0;
  uint32_t i, j;
  bin_device_t *bin_device = &eebin_hdl->bin_device;
  bin_file_header_t *master_h;
  bin_dev_type_t type_l;

  RETURN_ERR_ON_NULL(name_data, -EINVAL);

  name_data->lib_name = NULL;
  name_data->path = NULL;

  type_l = eebin_conv_type(name_data->type);
  if (type_l == BIN_DEV_INVALID)
    return -EINVAL;

  master_h = &bin_device->master_h;

  for (i = 0; i < master_h->num_modules; i++) {
    for(j = 0; j < bin_device->moddev[i].module_h.max_devices; j++) {
      if(bin_device->moddev[i].device_h[j].type == type_l) {
        if(!strncmp(bin_device->moddev[i].device_h[j].name, name_data->name,
          sizeof(bin_device->moddev[i].device_h[j].name))) {
          name_data->lib_name = bin_device->moddev[i].device_h[j].lib_name;
          name_data->path = bin_device->moddev[i].device_h[j].path;
          SLOW("name:%s lib_name:%s, path:%s",
            bin_device->moddev[i].device_h[j].name,
            bin_device->moddev[i].device_h[j].lib_name,
            bin_device->moddev[i].device_h[j].path);
        }
      }
    }
  }
  SDBG("Exit");
  return rc;
}

/** eebin_lib_name_data: Getting eebin  name, lib name and path
 *
 *  @eebin: eebin data buffer size
 *  @name_data: query data
 *
 *  Return: 0 for Success
 **/
int32_t eebin_lib_name_data(eebin_hdl_t *eebin_hdl,
  eebin_query_name_t *name_data)
{
  int32_t rc = 0;
  uint32_t i, j;
  bin_device_t *bin_device = &eebin_hdl->bin_device;
  bin_file_header_t *master_h;
  bin_dev_type_t type_l;

  SDBG("ENTER");

  RETURN_ERR_ON_NULL(name_data, -EINVAL);

  name_data->name = NULL;
  name_data->path = NULL;

  type_l = eebin_conv_type(name_data->type);
  if (type_l == BIN_DEV_INVALID)
    return -EINVAL;

  master_h = &bin_device->master_h;

  for (i = 0; i < master_h->num_modules; i++) {
    for(j = 0; j < bin_device->moddev[i].module_h.max_devices; j++) {
      if(bin_device->moddev[i].device_h[j].type == type_l) {
        if(!strncmp(bin_device->moddev[i].device_h[j].lib_name, name_data->lib_name,
          sizeof(bin_device->moddev[i].device_h[j].lib_name))) {
          name_data->name = bin_device->moddev[i].device_h[j].name;
          name_data->path = bin_device->moddev[i].device_h[j].path;
          SLOW("name:%s lib_name:%s, path:%s",
            bin_device->moddev[i].device_h[j].name,
            bin_device->moddev[i].device_h[j].lib_name,
            bin_device->moddev[i].device_h[j].path);
        }
      }
    }
  }
  SDBG("Exit");
  return rc;
}

/** eebin_dev_data: Getting eebin  name, lib name and path
 *
 *  @eebin: eebin data buffer size
 *  @dev_data: query data
 *
 *  Return: 0 for Success
 **/
int32_t eebin_dev_data(eebin_hdl_t *eebin_hdl,
  eebin_query_dev_t *dev_data)
{
  int32_t rc = 0;
  uint32_t i, j;
  uint32_t dnum = 0;
  bin_device_t *bin_device = &eebin_hdl->bin_device;
  bin_file_header_t *master_h;
  bin_dev_type_t type_l;

  RETURN_ERR_ON_NULL(dev_data, -EINVAL);

  type_l = eebin_conv_type(dev_data->type);
  if (type_l == BIN_DEV_INVALID)
    return -EINVAL;

  master_h = &bin_device->master_h;

  for (i = 0; i < master_h->num_modules; i++) {
    for(j = 0; j < bin_device->moddev[i].module_h.max_devices; j++) {
      if(bin_device->moddev[i].device_h[j].type == type_l) {
        if(dnum == dev_data->num) {
          dev_data->name = bin_device->moddev[i].device_h[j].name;
          dev_data->lib_name = bin_device->moddev[i].device_h[j].lib_name;
          dev_data->path = bin_device->moddev[i].device_h[j].path;
          SLOW("name:%s lib_name:%s, path:%s",
            bin_device->moddev[i].device_h[j].name,
            bin_device->moddev[i].device_h[j].lib_name,
            bin_device->moddev[i].device_h[j].path);
        }
        dnum++;
      }
    }
  }
  SLOW("Exit");
  return rc;
}

/** eebin_dev_data: Getting eebin num modules
 *
 *  @eebin: eebin data buffer size
 *  @type: module type
 *  @num: number of devices found
 *
 *  Return: 0 for Success
 **/
int32_t eebin_num_dev(eebin_hdl_t *eebin_hdl,
  eebin_lib_t type, uint32_t *num)
{
  int32_t rc = 0;
  uint32_t i, j;
  uint32_t dnum = 0;
  bin_device_t *bin_device = &eebin_hdl->bin_device;
  bin_file_header_t *master_h;
  bin_dev_type_t type_l = eebin_conv_type(type);

  RETURN_ERR_ON_NULL(eebin_hdl, -EINVAL);

  SLOW("Enter, type:%d, num:%d", type_l, *num);

  if (type_l != BIN_DEV_INVALID) {
    SLOW("Enter, type:%d, num:%d", type, *num);
    master_h = &bin_device->master_h;
    for (i = 0; i < master_h->num_modules; i++) {
      for(j = 0; j < bin_device->moddev[i].module_h.max_devices; j++) {
        if(bin_device->moddev[i].device_h[j].type == type_l) {
          SLOW("da %d", dnum);
          dnum++;
        }
      }
    }
  }

  *num = dnum;
  SLOW("Exit %d", *num);
  return rc;
}

/** eebin_dev_data: Searching for eebin v4l subdevices
 *
 *  @eebin: eebin data buffer size
 *
 *  Return: 0 for Success
 **/
int32_t eebin_get_bin_data(eebin_hdl_t *eebin_hdl)
{
  int32_t rc = 0, dev_fd = 0;
  char dev_name[32];
  char subdev_name[32];
  uint32_t num_media_devices = 0;
  struct media_device_info mdev_info;

  RETURN_ERR_ON_NULL(eebin_hdl, -EINVAL);

  SLOW("Enter %d", eebin_hdl->ready);
  if (eebin_hdl->ready)
    return 0;

  SDBG("Enter");
  while (1) {
    uint32_t num_entities = 0;
    snprintf(dev_name, sizeof(dev_name), "/dev/media%d", num_media_devices);
    dev_fd = open(dev_name, O_RDWR | O_NONBLOCK);
    if (dev_fd < 0) {
      SLOW("Done enumerating media devices");
      break;
    }
    num_media_devices++;
    if (ioctl(dev_fd, MEDIA_IOC_DEVICE_INFO, &mdev_info) < 0) {
      SLOW("Done enumerating media devices");
      close(dev_fd);
      break;
    }

    if (strncmp(mdev_info.model, "msm_config", sizeof(mdev_info.model)) != 0) {
      close(dev_fd);
      continue;
    }

    while (1) {
      struct media_entity_desc entity;
      memset(&entity, 0, sizeof(entity));
      entity.id = num_entities | MEDIA_ENT_ID_FLAG_NEXT;
      SLOW("entity id %d", entity.id);
      if (ioctl(dev_fd, MEDIA_IOC_ENUM_ENTITIES, &entity) < 0) {
        SLOW("Done enumerating media entities");
        break;
      }
      num_entities = entity.id;
      SLOW("entity name %s type %d group id %d",
      entity.name, entity.type, entity.group_id);
      if (mct_util_match_subdev_type(entity,MSM_CAMERA_SUBDEV_EEPROM,
        MEDIA_ENT_T_V4L2_SUBDEV)) {
        snprintf(subdev_name, sizeof(dev_name), "/dev/%s", entity.name);
        if (eebin_read(subdev_name, eebin_hdl) == FALSE) continue;
      }
    }
    close(dev_fd);
  }

#ifdef USE_DEBUG_BUFF
  eebin_read(NULL, eebin_hdl);
#endif

 eebin_hdl->ready = 1;
 SDBG("Exit");
 return rc;
}
