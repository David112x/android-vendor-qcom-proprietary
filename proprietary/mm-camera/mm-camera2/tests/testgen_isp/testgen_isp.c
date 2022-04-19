/* testgen_isp.c
 *
 * Copyright (c) 2014,2016 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <time.h>
#include <media/msmb_isp.h>
#include <poll.h>
#include "mtype.h"
#include "mct_controller.h"
#include "mct_pipeline.h"
#include "mct_module.h"
#include "mct_port.h"
#include "cam_intf.h"
#include "camera_dbg.h"
#include "isp_pipeline_reg.h"
#if TARGET_ION_ABI_VERSION >=2
#include <ion/ion.h>
#endif
#define MAX_VFE_IMAGE_BUF   6
#define NUM_STATS_BUF      10
#define SENSOR_WIDTH      640
#define SENSOR_HEIGHT     480
#define RUN_FOR_FRAMES    1000

#define FLOAT_TO_Q(exp, f) ((int32_t)((f*(1<<(exp))) + ((f<0) ? -0.5 : 0.5)))
uint32_t dump_every_n_frames = 0;
uint32_t run_for_n_frames = RUN_FOR_FRAMES;
uint32_t frame_width = SENSOR_WIDTH;
uint32_t frame_height = SENSOR_HEIGHT;

struct isp_frame_buffer
{
  struct v4l2_buffer buffer;
  unsigned long addr[VIDEO_MAX_PLANES];
  struct ion_allocation_data ion_alloc[VIDEO_MAX_PLANES];
  struct ion_fd_data fd_data[VIDEO_MAX_PLANES];
};

struct isp_driver
{
  int vfe_fd;
  int isp_id;
  int ion_fd;
  int stats_stream_id;

  struct isp_frame_buffer stream_buf[2][MAX_VFE_IMAGE_BUF];
  uint32_t stats_bufq_handle[MSM_ISP_STATS_MAX];
  uint32_t stats_stream_handle[MSM_ISP_STATS_MAX];

  int thread_ready;
  int exitting;
  int frames_rcvd;
};

void vfe_thread(struct isp_driver *isp_drv);

uint8_t* do_mmap_ion(int ion_fd, struct ion_allocation_data *alloc,
                     struct ion_fd_data *ion_info_fd, int *mapFd)
{
  void *ret; /* returned virtual address */
  int rc = 0;
  int map_fd = -1;
  struct ion_handle_data handle_data;

  /* to make it page size aligned */
  alloc->len = (alloc->len + 4095) & (~4095);
#ifndef TARGET_ION_ABI_VERSION
  rc = ioctl(ion_fd, ION_IOC_ALLOC, alloc);
#else
   rc = ion_alloc_fd(ion_fd, alloc->len, alloc->align, alloc->heap_id_mask,
                      alloc->flags, &map_fd);
  *mapFd = map_fd;
#endif//TARGET_ION_ABI_VERSION endif
  if (rc < 0)
  {
    printf("ION allocation failed\n");
    goto ION_ALLOC_FAILED;
  }
#ifndef TARGET_ION_ABI_VERSIO
  ion_info_fd->handle = alloc->handle;
  rc = ioctl(ion_fd, ION_IOC_SHARE, ion_info_fd);
  if (rc < 0)
  {
    printf("ION map failed %s\n", strerror(errno));
    goto ION_MAP_FAILED;
  }
  *mapFd = ion_info_fd->fd;
#else
  (void) ion_info_fd;
#endif
  ret = mmap(NULL,
             alloc->len,
             PROT_READ  | PROT_WRITE,
             MAP_SHARED,
             *mapFd,
             0);

  if (ret == MAP_FAILED)
  {
    printf("ION_MMAP_FAILED: %s (%d)\n", strerror(errno), errno);
    goto ION_MAP_FAILED;
  }

  return ret;

ION_MAP_FAILED:
#ifndef TARGET_ION_ABI_VERSION
  handle_data.handle = ion_info_fd->handle;
  ioctl(ion_fd, ION_IOC_FREE, &handle_data);
#endif//TARGET_ION_ABI_VERSION endif
ION_ALLOC_FAILED:
  return NULL;
}

int do_munmap_ion(int ion_fd, struct ion_fd_data *ion_info_fd,
                  void *addr, size_t size)
{
  int rc = 0;
  rc = munmap(addr, size);
  close(ion_info_fd->fd);
#ifndef TARGET_ION_ABI_VERSION
  struct ion_handle_data handle_data;
  handle_data.handle = ion_info_fd->handle;
  ioctl(ion_fd, ION_IOC_FREE, &handle_data);
#else
  (void) ion_fd;
#endif//TARGET_ION_ABI_VERSION endif
  return rc;
}

struct isp_plane_alloc_info
{
  unsigned long len;
  uint32_t offset;
};

struct isp_buf_alloc_info
{
  struct isp_plane_alloc_info plane_info[VIDEO_MAX_PLANES];
  int num_planes;
};

static int isp_discover_subdev_nodes(int isp_id)
{
  struct media_device_info mdev_info;
  int num_media_devices = 0;
  char dev_name[128];
  int rc = 0, dev_fd = 0;
  int num_isps = 0;
  int num_entities;
  int found = 0;
  while (1)
  {
    snprintf(dev_name, sizeof(dev_name), "/dev/media%d", num_media_devices);
    dev_fd = open(dev_name, O_RDWR | O_NONBLOCK);
    if (dev_fd < 0)
    {
      printf("Done discovering media devices\n");
      return open("/dev/v4l-subdev0", O_RDWR | O_NONBLOCK);
    }
    num_media_devices++;
    rc = ioctl(dev_fd, MEDIA_IOC_DEVICE_INFO, &mdev_info);
    if (rc < 0)
    {
      printf("Error: ioctl media_dev failed: %s\n", strerror(errno));
      close(dev_fd);
      break;
    }

    if (strncmp(mdev_info.model, "msm_config", sizeof(mdev_info.model)) != 0)
    {
      close(dev_fd);
      continue;
    }

    num_entities = 1;
    while (1)
    {
      struct media_entity_desc entity;
      memset(&entity, 0, sizeof(entity));
      entity.id = num_entities++;
      rc = ioctl(dev_fd, MEDIA_IOC_ENUM_ENTITIES, &entity);
      if (rc < 0)
      {
        printf("Done enumerating media entities\n");
        rc = 0;
        break;
      }
      printf("%s:%d entity name %s type %d group id %d\n", __func__, __LINE__,
           entity.name, entity.type, entity.group_id);
      if (entity.type == MEDIA_ENT_T_V4L2_SUBDEV &&
          entity.group_id == MSM_CAMERA_SUBDEV_VFE)
      {
        snprintf(dev_name,
                 sizeof(dev_name), "/dev/%s", entity.name);

        printf("entity.name: %s\n", dev_name);
        if (found++ == isp_id)
        {
          printf("entity.name: %s\n", dev_name);
          return open(dev_name, O_RDWR | O_NONBLOCK);
        }
      }
    }
    close(dev_fd);
  }
  return -1;
}

int isp_smmu_attach(int fd)
{
  int rc;
  struct msm_vfe_smmu_attach_cmd cmd;
  cmd.security_mode = 0;
  cmd.iommu_attach_mode = IOMMU_ATTACH;
  rc = ioctl(fd, VIDIOC_MSM_ISP_SMMU_ATTACH, &cmd);
  if (rc)
    printf("error in attaching smmu: rc = %d\n", rc);
  return rc;
}

int isp_init_buffer(struct isp_frame_buffer *buf,
                    int ion_fd, struct isp_buf_alloc_info *alloc_info)
{
  int current_fd = -1, i;
  unsigned long current_addr = 0;
  memset(buf, 0, sizeof(struct isp_frame_buffer));
  buf->buffer.m.planes = malloc(
     sizeof(struct v4l2_plane) * alloc_info->num_planes);
  if (!buf->buffer.m.planes)
  {
    printf("%s: no mem\n", __func__);
    return -1;
  }

  for (i = 0; i < alloc_info->num_planes; i++)
  {
    if (alloc_info->plane_info[i].offset == 0)
    {
      buf->ion_alloc[i].len = alloc_info->plane_info[i].len;
      buf->ion_alloc[i].flags = 0;
#ifndef TARGET_ION_ABI_VERSION
      buf->ion_alloc[i].heap_id_mask = ION_HEAP(ION_IOMMU_HEAP_ID);
#else
      buf->ion_alloc[i].heap_id_mask = ION_HEAP(ION_SYSTEM_HEAP_ID);
#endif
      buf->ion_alloc[i].align = 4096;
      current_addr = (unsigned long)do_mmap_ion(ion_fd,
                                                &(buf->ion_alloc[i]), &(buf->fd_data[i]), &current_fd);
      memset((void *)current_addr, 128, buf->ion_alloc[i].len);
    }
    buf->addr[i] = current_addr + alloc_info->plane_info[i].offset;
    buf->buffer.m.planes[i].m.userptr = current_fd;
    buf->buffer.m.planes[i].data_offset = alloc_info->plane_info[i].offset;
  }
  buf->buffer.length = alloc_info->num_planes;
  return 0;
}

void isp_release_buffer(struct isp_frame_buffer *buf, int ion_fd)
{
  int i;
  for (i = 0; i < (int)buf->buffer.length; i++)
  {
    if (buf->ion_alloc[i].len != 0) do_munmap_ion(ion_fd, &(buf->fd_data[0]),
                                                  (void *)buf->addr[i], buf->ion_alloc[i].len);
  }
  free(buf->buffer.m.planes);
}

void write_masked_val_to_hw(int fd, uint32_t val, uint32_t mask, uint32_t reg_offset)
{
  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd;

  cfg_cmd.cfg_data = (void *)&val;
  cfg_cmd.cmd_len = sizeof(uint32_t);
  cfg_cmd.cfg_cmd = (void *)&reg_cfg_cmd;
  cfg_cmd.num_cfg = 1;

  reg_cfg_cmd.cmd_type = VFE_CFG_MASK;
  reg_cfg_cmd.u.mask_info.reg_offset = reg_offset;
  reg_cfg_cmd.u.mask_info.mask = mask;
  reg_cfg_cmd.u.mask_info.val = val;
  ioctl(fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
}

void write_struct_to_hw(int fd, void *data, uint32_t size, uint32_t reg_offset)
{
  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd;

  cfg_cmd.cfg_data = (void *)data;
  cfg_cmd.cmd_len = size;
  cfg_cmd.cfg_cmd = (void *)&reg_cfg_cmd;
  cfg_cmd.num_cfg = 1;

  reg_cfg_cmd.cmd_type = VFE_WRITE;
  reg_cfg_cmd.u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd.u.rw_info.len = size;
  reg_cfg_cmd.u.rw_info.reg_offset = reg_offset;
  ioctl(fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
}

void isp_config_testgen(int fd)
{
  ISP_TestGenCmdType testgen;
  uint32_t hbi = 0x158 * 20;

  memset(&testgen, 0, sizeof(testgen));
  testgen.numFrame           = 0; /* run continuously */
  testgen.pixelDataSel       = 0; /* systematic */
  testgen.systematicDataSel  = 0; /* color bar */
  testgen.pixelDataSize      = 1; /* 8 bpp */
  testgen.hSyncEdge          = 0; /* active high */
  testgen.vSyncEdge          = 0; /* active high */
  testgen.imageWidth         = frame_width - 1;
  testgen.imageHeight        = frame_height - 1;
#ifdef VFE44
  testgen.hbi                = hbi;
#else /* VFE46 has hbi in core_cfg (0x50) bit 8-21 */
  write_masked_val_to_hw(fd, hbi << 8, 0x3fff << 8, 0x50);
#endif
  testgen.vbl                = 0x50 * 20;
  testgen.vblEn              = 1;
  testgen.unicolorBarSel     = 0;
  testgen.unicolorBarEn      = 0;
  testgen.splitEn            = 0;
  testgen.pixelPattern       = 0;
  testgen.rotatePeriod       = 1;
  testgen.randSeed           = 0;

  write_struct_to_hw(fd, &testgen, sizeof(testgen), ISP_TESTGEN_OFF);
}

void isp_config_demosaic(int fd)
{
  ISP_DemosaicCfgCmdType demo_cfg;
  ISP_DemosaicCmdType demo_cmd;
  uint16_t wInterpDefault[ISP_DEMOSAIC_CLASSIFIER_CNT] =
    { 137, 91, 91, 1023, 922, 93, 195, 99, 64, 319, 197, 88, 84, 109, 151, 98, 66, 76 };
  uint16_t bInterpDefault[ISP_DEMOSAIC_CLASSIFIER_CNT] =
    { 1, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 1, 1, 1, 1, 0, 1, 1 };
  uint16_t lInterpDefault[ISP_DEMOSAIC_CLASSIFIER_CNT] =
    { 0, 0, 1, 2, 2, 3, 9, 9, 9, 4, 4, 5, 6, 7, 8, 8, 10, 10 };
  int16_t tInterpDefault[ISP_DEMOSAIC_CLASSIFIER_CNT] =
    { 2, 1, 0, 0, -1, 2, 0, -1, 1, 0, -1, 2, 0, 2, 2, 1, 0, 100 };

  float wk_default[] = {0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1};
  int bk_default[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  int lk_default[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 0, 1, 2, 3, 4, 5, 6, 7, 8};
  int tk_default[] = {3, 3, 3, 3, 3, 3, 3, 3, 3, -2, -2, -2, -2, -2, -2, -2, -2, -2};
  int i, temp;
  float bL = 0.0f, aG = 0.0f;

  memset(&demo_cfg, 0, sizeof(demo_cfg));
  memset(&demo_cmd, 0, sizeof(demo_cmd));

#ifdef VFE44
  demo_cfg.demoV4Enable = 1;
#endif
  demo_cfg.cositedRgbEnable = 0;
  demo_cfg.dirGInterpDisable = 0;
  demo_cfg.dirRBInterpDisable = 0;
  demo_cfg.dynGClampEnable = 1;
  demo_cfg.dynRBClampEnable = 0;
  demo_cfg.dynGClampCDisable = 1;
  demo_cfg.dynRBClampCDisable = 1;

  for (i = 0 ; i < ISP_DEMOSAIC_CLASSIFIER_CNT ; ++i) {
    demo_cmd.interpClassifier[i].w_n = FLOAT_TO_Q(10,wk_default[i]);
    demo_cmd.interpClassifier[i].t_n = tk_default[i];
    demo_cmd.interpClassifier[i].l_n = lk_default[i];
    demo_cmd.interpClassifier[i].b_n = bk_default[i];
  }

  /* Interp WB */
  demo_cmd.wgr1 = 2;
  demo_cmd.wgr2 = 2;

  /* lambda values need to multiply by Q8 */
  demo_cmd.lambdaG = FLOAT_TO_Q(8, 0.5f);
  demo_cmd.lambdaRB = FLOAT_TO_Q(8, 0.0f);

  /* Interp G */
  demo_cmd.noiseLevelG = 0;
  demo_cmd.noiseLevelRB = 0;

  temp = FLOAT_TO_Q(8, bL);
  demo_cmd.bl = MIN(MAX(0, temp), 118);
  temp = FLOAT_TO_Q(8, (1.0 - bL));
  demo_cmd.bu = MIN(MAX(138, temp), 255);
  temp = FLOAT_TO_Q(5, (1.0/(1.0 - 2 * bL)));
  demo_cmd.dblu = MIN(MAX(0, temp), 511);
  temp = FLOAT_TO_Q(6, aG);
#ifdef VFE44
  demo_cmd.a = MIN(MAX(0, temp), 63);
#endif

  write_struct_to_hw(fd, &demo_cfg, sizeof(demo_cfg), ISP_DEMOSAIC_CFG_OFF);
  write_struct_to_hw(fd, &demo_cmd, sizeof(demo_cmd), ISP_DEMOSAIC_OFF);
  write_masked_val_to_hw(fd,
    (1 << ISP_DEMOSAIC_EN_BIT),
    (1 << ISP_DEMOSAIC_EN_BIT),
    ISP_DEMOSAIC_EN_OFF);
}

void isp_config_color_conv_chroma_enh(int fd)
{
  ISP_Chroma_Enhance_CfgCmdType cc;

  memset(&cc, 0, sizeof(cc));
  cc.RGBtoYConversionV0 = 0x4c;
  cc.RGBtoYConversionV1 = 0x96;
  cc.RGBtoYConversionV2 = 0x1d;
  cc.RGBtoYConversionOffset  = 0;
  cc.ap  |= ~0;
  cc.ap  &= 0x5e;
  cc.am  |= ~0;
  cc.am  &= 0x72;
  cc.bp  |= ~0;
  cc.bp  &= 0xfa3;
  cc.bm  |= ~0;
  cc.bm  &= 0xfa9;
  cc.cp  |= ~0;
  cc.cp  &= 0x72;
  cc.cm  |= ~0;
  cc.cm  &= 0x67;
  cc.dp  |= ~0;
  cc.dp  &= 0xfc8;
  cc.dm  |= ~0;
  cc.dm  &= 0xfce;
  cc.kcb |= ~0;
  cc.kcb &= 0x80;
  cc.kcr |= ~0;
  cc.kcr &= 0x80;

  write_struct_to_hw(fd, &cc, sizeof(cc), ISP_CC_OFF);
  write_masked_val_to_hw(fd,
    (1 << ISP_CC_EN_BIT),
    (1 << ISP_CC_EN_BIT),
    ISP_CC_EN_OFF);
}

void isp_config_demux(int fd)
{
  ISP_DemuxConfigCmdType demux;

  memset(&demux, 0, sizeof(demux));
  demux.period  = 1;
  demux.ch0EvenGain = 0x80;
  demux.ch0OddGain = 0x80;
  demux.ch1Gain  = 0x80;
  demux.ch2Gain = 0x80;
  demux.R_ch0EvenGain = 0x80;
  demux.R_ch0OddGain = 0x80;
  demux.R_ch1Gain  = 0x80;
  demux.R_ch2Gain = 0x80;
  demux.evenCfg = 0xc9;
  demux.oddCfg = 0xac;

  write_struct_to_hw(fd, &demux, sizeof(demux), ISP_DEMUX_OFF);
  write_masked_val_to_hw(fd,
    (1 << ISP_DEMUX_EN_BIT),
    (1 << ISP_DEMUX_EN_BIT),
    ISP_DEMUX_EN_OFF);
}

void isp_config_clamp(int fd)
{
  ISP_OutputClampConfigCmdType clamp;

  memset(&clamp, 0, sizeof(clamp));
  clamp.yChanMax  = 0xff;
  clamp.cbChanMax = 0xff;
  clamp.crChanMax = 0xff;
  clamp.yChanMin  = 0;
  clamp.cbChanMin = 0;
  clamp.crChanMin = 0;

  write_struct_to_hw(fd, &clamp, sizeof(clamp), ISP_CLAMP_ENC_OFF);
  write_struct_to_hw(fd, &clamp, sizeof(clamp), ISP_CLAMP_VIEW_OFF);
}

void isp_config_scaler(int fd)
{
  ISP_ScaleCfgCmdType scaler;

  memset(&scaler, 0, sizeof(scaler));
  scaler.y.hEnable = 1;
  scaler.y.vEnable = 1;
  scaler.y.hIn  = frame_width - ISP_SCALER_HVINOUT_DIFF;
  scaler.y.hOut = frame_width - ISP_SCALER_HVINOUT_DIFF;
  scaler.y.vIn  = frame_height - ISP_SCALER_HVINOUT_DIFF;
  scaler.y.vOut = frame_height - ISP_SCALER_HVINOUT_DIFF;
  scaler.cbcr.hEnable = 1;
  scaler.cbcr.vEnable = 1;
  scaler.cbcr.hIn  = frame_width - ISP_SCALER_HVINOUT_DIFF;
  scaler.cbcr.hOut = frame_width / 2 - ISP_SCALER_HVINOUT_DIFF;
  scaler.cbcr.vIn  = frame_height - ISP_SCALER_HVINOUT_DIFF;
  scaler.cbcr.vOut = frame_height / 2 - ISP_SCALER_HVINOUT_DIFF;

  write_struct_to_hw(fd, &scaler, sizeof(scaler), ISP_SCALER_ENC_OFF);
  write_struct_to_hw(fd, &scaler, sizeof(scaler), ISP_SCALER_VIEW_OFF);
  write_masked_val_to_hw(fd,
    (1 << ISP_SCALER_ENC_EN_BIT) | (1 << ISP_SCALER_VIEW_EN_BIT),
    (1 << ISP_SCALER_ENC_EN_BIT) | (1 << ISP_SCALER_VIEW_EN_BIT),
    ISP_SCALER_EN_OFF);
}

void isp_config_stats_bg(int fd)
{
  ISP_StatsBg_CfgCmdType bg;
  uint32_t rgn_width = 4;
  uint32_t rgn_height = 4;
  memset(&bg, 0, sizeof(bg));
  bg.rgnHOffset  = 0;
  bg.rgnVOffset  = 0;
  bg.rgnWidth    = rgn_width - 1;
  bg.rgnHeight   = rgn_height - 1;
  bg.rgnHNum     = MIN(frame_width / rgn_width, 72) - 1;
  bg.rgnVNum     = MIN(frame_height / rgn_height, 54) - 1;
  bg.rMax        = 0xef;
  bg.grMax       = 0xef;
  bg.bMax        = 0xef;
  bg.gbMax       = 0xef;

  write_struct_to_hw(fd, &bg, sizeof(bg), ISP_BG_STATS_OFF);
}

void isp_enable_stats(struct isp_driver *isp_drv, enum msm_isp_stats_type stats_type, uint8_t enable)
{
  struct msm_vfe_stats_stream_cfg_cmd stats_cfg_cmd;
  stats_cfg_cmd.enable = enable;
  stats_cfg_cmd.num_streams = 1;
  stats_cfg_cmd.stream_handle[0] = isp_drv->stats_stream_handle[stats_type];
  ioctl(isp_drv->vfe_fd, VIDIOC_MSM_ISP_CFG_STATS_STREAM, &stats_cfg_cmd);
  printf("%s stats stream: %d isp id %d\n", enable ? "Started" : "Stopped", stats_cfg_cmd.stream_handle[0],
      isp_drv->isp_id);
}

void copy_planes_from_v4l2_buffer(
  struct msm_isp_qbuf_buffer *qbuf_buf,
  const struct v4l2_buffer *v4l2_buf)
{
  uint32_t i;
  qbuf_buf->num_planes = v4l2_buf->length;
  for (i = 0; i < qbuf_buf->num_planes; i++) {
    qbuf_buf->planes[i].addr = v4l2_buf->m.planes[i].m.userptr;
    qbuf_buf->planes[i].offset = v4l2_buf->m.planes[i].data_offset;
  }
}


void isp_request_stats(struct isp_driver *isp_drv,
  enum msm_isp_stats_type stats_type, uint32_t stats_size)
{
  int i;
  int isp_bit = (isp_drv->isp_id == 0) ? ISP0_BIT : ISP1_BIT;
  struct isp_buf_alloc_info stats_alloc_info;
  stats_alloc_info.num_planes = 1;
  stats_alloc_info.plane_info[0].len = stats_size;
  stats_alloc_info.plane_info[0].offset = 0;

  struct isp_frame_buffer stats_buffer[NUM_STATS_BUF];
  for (i = 0; i < NUM_STATS_BUF; i++)
    isp_init_buffer(&stats_buffer[i], isp_drv->ion_fd, &stats_alloc_info);

  struct msm_isp_buf_request stats_buf_request;
  stats_buf_request.session_id = 1;
  stats_buf_request.stream_id = isp_drv->stats_stream_id | isp_bit;
  stats_buf_request.num_buf = NUM_STATS_BUF;
  stats_buf_request.handle = 0;
  stats_buf_request.buf_type = ISP_PRIVATE_BUF;
  ioctl(isp_drv->vfe_fd, VIDIOC_MSM_ISP_REQUEST_BUF, &stats_buf_request);

  struct msm_isp_qbuf_info stats_qbuf_info;
  stats_qbuf_info.handle = stats_buf_request.handle;
  for (i = 0; i < NUM_STATS_BUF; i++)
  {
    copy_planes_from_v4l2_buffer(&stats_qbuf_info.buffer, &stats_buffer[i].buffer);
    stats_qbuf_info.buf_idx = i;
    ioctl(isp_drv->vfe_fd, VIDIOC_MSM_ISP_ENQUEUE_BUF, &stats_qbuf_info);
  }

  struct msm_vfe_stats_stream_request_cmd stats_request_cmd;
  memset(&stats_request_cmd, 0x00, sizeof(stats_request_cmd));
  stats_request_cmd.session_id = 1;
  stats_request_cmd.stream_id = isp_drv->stats_stream_id | isp_bit;
  stats_request_cmd.stats_type = stats_type;
  stats_request_cmd.framedrop_pattern = NO_SKIP;
  stats_request_cmd.composite_flag = 0;
  stats_request_cmd.irq_subsample_pattern = 0;
  ioctl(isp_drv->vfe_fd, VIDIOC_MSM_ISP_REQUEST_STATS_STREAM, &stats_request_cmd);

  isp_drv->stats_stream_id++;
  isp_drv->stats_bufq_handle[stats_type] = stats_buf_request.handle;
  isp_drv->stats_stream_handle[stats_type] = stats_request_cmd.stream_handle;
}

void isp_config_input(int fd)
{
  struct msm_vfe_input_cfg input_cfg;
  input_cfg.input_src = VFE_PIX_0;
  input_cfg.d.pix_cfg.input_mux = TESTGEN;
  input_cfg.d.pix_cfg.pixel_pattern = ISP_BAYER_RGRGRG;
  input_cfg.d.pix_cfg.camif_cfg.camif_input = CAMIF_DISABLED;
  input_cfg.d.pix_cfg.camif_cfg.lines_per_frame = frame_height;
  input_cfg.d.pix_cfg.camif_cfg.pixels_per_line = frame_width;
  input_cfg.d.pix_cfg.camif_cfg.first_pixel = 0;
  input_cfg.d.pix_cfg.camif_cfg.last_pixel = frame_width - 1;
  input_cfg.d.pix_cfg.camif_cfg.first_line = 0;
  input_cfg.d.pix_cfg.camif_cfg.last_line = frame_height - 1;
  input_cfg.d.pix_cfg.input_format = V4L2_PIX_FMT_QBGGR10;
  input_cfg.input_pix_clk = 320000000;
  ioctl(fd, VIDIOC_MSM_ISP_INPUT_CFG, &input_cfg);
}

void vfe_test_func(struct isp_driver *isp_drv)
{
  int i;
  int vfe_fd = isp_drv->vfe_fd;
  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd[11];
  struct msm_vfe_axi_stream_update_cmd update_cmd;

  int ion_fd = 0;
  int isp_bit = (isp_drv->isp_id == 0) ? ISP0_BIT : ISP1_BIT;
#ifndef TARGET_ION_ABI_VERSION
  ion_fd = open("/dev/ion", O_RDONLY | O_SYNC);
#else
  ion_fd = ion_open();
#endif
  if (ion_fd < 0)
  {
    printf("Ion device open failed\n");
  }
  isp_drv->ion_fd = ion_fd;

  isp_smmu_attach(vfe_fd);

  uint32_t buffer_width, buffer_height;
  uint32_t sensor_width, sensor_height;

  sensor_width = frame_width;
  sensor_height = frame_height;

  buffer_width = sensor_width;
  buffer_height = sensor_height;

  isp_request_stats(isp_drv, MSM_ISP_STATS_BG, ISP_STATS_BG_BUF_SIZE);

  isp_config_testgen(vfe_fd);

  /* configure CAMIF which triggers TESTGEN to start */
  isp_config_input(vfe_fd);

  struct isp_buf_alloc_info alloc_info;

  alloc_info.num_planes = 2;
  alloc_info.plane_info[0].len = buffer_width * buffer_height * 1.5;
  alloc_info.plane_info[0].offset = 0;
  alloc_info.plane_info[1].len = 0;
  alloc_info.plane_info[1].offset = buffer_width * buffer_height;

  for (i = 0; i < MAX_VFE_IMAGE_BUF; i++)
  {
    isp_init_buffer(&isp_drv->stream_buf[0][i], ion_fd, &alloc_info);
  }

  struct msm_isp_buf_request buf_request;
  buf_request.session_id = 1;
  buf_request.stream_id = 0x10001 | isp_bit;
  buf_request.num_buf = MAX_VFE_IMAGE_BUF;
  buf_request.buf_type = ISP_PRIVATE_BUF;
  buf_request.handle = 0;
  ioctl(vfe_fd, VIDIOC_MSM_ISP_REQUEST_BUF, &buf_request);

  struct msm_isp_qbuf_info qbuf_info;
  qbuf_info.handle = buf_request.handle;
  for (i = 0; i < MAX_VFE_IMAGE_BUF; i++)
  {
    copy_planes_from_v4l2_buffer(&qbuf_info.buffer, &(isp_drv->stream_buf[0][i].buffer));
    qbuf_info.buf_idx = i;
    ioctl(vfe_fd, VIDIOC_MSM_ISP_ENQUEUE_BUF, &qbuf_info);
  }

  struct msm_vfe_axi_stream_request_cmd stream_req;
  memset(&stream_req, 0, sizeof(stream_req));
  stream_req.axi_stream_handle = 0;
  stream_req.session_id = 1;
  stream_req.stream_id = 0x10001 | isp_bit;

  stream_req.output_format = V4L2_PIX_FMT_NV21;
  stream_req.stream_src = PIX_ENCODER;
  stream_req.plane_cfg[0].output_plane_format = Y_PLANE;
  stream_req.plane_cfg[0].output_width = buffer_width;
  stream_req.plane_cfg[0].output_height = buffer_height;
  stream_req.plane_cfg[0].output_stride = buffer_width;
  stream_req.plane_cfg[0].output_scan_lines = buffer_height;
  stream_req.plane_cfg[1].output_plane_format = CRCB_PLANE;
  stream_req.plane_cfg[1].output_width = buffer_width;
  stream_req.plane_cfg[1].output_height = buffer_height / 2;
  stream_req.plane_cfg[1].output_stride = buffer_width;
  stream_req.plane_cfg[1].output_scan_lines = buffer_height / 2;
  stream_req.buf_divert = 1;

  ioctl(vfe_fd, VIDIOC_MSM_ISP_REQUEST_STREAM, &stream_req);

  update_cmd.num_streams = 1;
  update_cmd.update_info[0].user_stream_id = 0x10001 | isp_bit;
  update_cmd.update_info[0].stream_handle = stream_req.axi_stream_handle;
  update_cmd.update_type = UPDATE_STREAM_ADD_BUFQ;

  ioctl(vfe_fd, VIDIOC_MSM_ISP_UPDATE_STREAM, &update_cmd);

  isp_config_scaler(vfe_fd);
  isp_config_clamp(vfe_fd);
  isp_config_demux(vfe_fd);
  isp_config_color_conv_chroma_enh(vfe_fd);
  isp_config_demosaic(vfe_fd);
  isp_config_stats_bg(vfe_fd);

  write_masked_val_to_hw(vfe_fd, 1, 1, ISP_REG_UPDATE_OFF);

  struct msm_vfe_axi_stream_cfg_cmd stream_cfg;
  stream_cfg.cmd = START_STREAM;
  stream_cfg.num_streams = 1;
  stream_cfg.stream_handle[0] = stream_req.axi_stream_handle;
  ioctl(vfe_fd, VIDIOC_MSM_ISP_CFG_STREAM, &stream_cfg);
  printf("Started AXI stream: %d isp id %d\n", stream_req.axi_stream_handle,
    isp_drv->isp_id);

  isp_enable_stats(isp_drv, MSM_ISP_STATS_BG, 1);

  vfe_thread(isp_drv);

  isp_enable_stats(isp_drv, MSM_ISP_STATS_BG, 0);

  stream_cfg.cmd = STOP_STREAM;
  ioctl(vfe_fd, VIDIOC_MSM_ISP_CFG_STREAM, &stream_cfg);
  printf("Stopped AXI stream: %d\n", stream_req.axi_stream_handle);
  usleep(50000);

  ioctl(vfe_fd, VIDIOC_MSM_ISP_RELEASE_BUF, &buf_request);
  printf("Released ISP buffers\n");

  for (i = 0; i < MAX_VFE_IMAGE_BUF; i++)
  {
    isp_release_buffer(&isp_drv->stream_buf[0][i], ion_fd);
  }
  close(ion_fd);

  struct msm_vfe_axi_stream_release_cmd stream_release;
  stream_release.stream_handle = stream_req.axi_stream_handle;
  ioctl(vfe_fd, VIDIOC_MSM_ISP_RELEASE_STREAM, &stream_release);
  printf("Released AXI stream: %d isp id %d\n", stream_req.axi_stream_handle, isp_drv->isp_id);

  for (i = 0; i < MSM_ISP_STATS_MAX; i++) {
    uint32_t stats_handle = isp_drv->stats_stream_handle[i];
    if (stats_handle) {
      struct msm_vfe_stats_stream_release_cmd stats_release;
      stats_release.stream_handle = stats_handle;
      ioctl(vfe_fd, VIDIOC_MSM_ISP_RELEASE_STATS_STREAM, &stats_release);
      printf("Released stats stream: %d isp id %d\n", stats_handle, isp_drv->isp_id);
    }
  }

  close(vfe_fd);

}

#define VFE_EVENT_IDX_MASK ((1 << 8) - 1)
#define VFE_EVENT_TYPE_MASK ~VFE_EVENT_IDX_MASK


void dump_frame(struct isp_driver *isp_drv,
                uint32_t stream_id, int32_t buf_idx, uint32_t frame_id)
{
  int out_file_fd;
  struct isp_frame_buffer *buf = &isp_drv->stream_buf[stream_id][buf_idx];
  char filename[256];
  struct stat st;
  memset(&st, 0 , sizeof(struct stat));

  if (stat("/data/vfedump", &st) == -1) {
    mkdir("/data/vfedump", 0700);
  }
  snprintf(filename, 256, "/data/vfedump/%dx%d_%.4d.raw", frame_width, frame_height, frame_id);
  out_file_fd = open(filename, O_RDWR | O_CREAT, 0777);
  if (out_file_fd < 0)
  {
    printf("Cannot open file\n");
  }
  write(out_file_fd, (const void *)buf->addr[0], buf->ion_alloc[0].len);
  close(out_file_fd);
}

void proc_vfe_event(struct isp_driver *isp_drv, struct v4l2_event *ev)
{
  struct msm_isp_event_data *isp_event_data =
     (struct msm_isp_event_data *)ev->u.data;
  uint32_t event_type = ev->type;
  uint32_t idx = 0;
  int i;
  if (event_type >= ISP_EVENT_BUF_DIVERT)
  {
    idx = event_type & VFE_EVENT_IDX_MASK;
    event_type &= VFE_EVENT_TYPE_MASK;
  }

  switch (event_type)
  {
  case ISP_EVENT_SOF:
    printf("%s: SOF %d isp id %d\n", __func__, isp_event_data->frame_id, isp_drv->isp_id);
    break;
  case ISP_EVENT_BUF_DIVERT:
    {
      int rc;
      struct msm_isp_qbuf_info qbuf_info;
      isp_drv->frames_rcvd = isp_event_data->frame_id;
      fprintf(stderr, "\rframes rcvd = %d/%d", isp_drv->frames_rcvd, run_for_n_frames);
      if (dump_every_n_frames && isp_event_data->frame_id % dump_every_n_frames == 0)
      {
        fprintf(stderr, "\t\t dumped frame %d", isp_event_data->frame_id);
        dump_frame(isp_drv, 0, isp_event_data->u.buf_done.buf_idx,
                   isp_event_data->frame_id);
      }
      qbuf_info.handle = isp_event_data->u.buf_done.handle;
      qbuf_info.buf_idx = isp_event_data->u.buf_done.buf_idx;
      qbuf_info.dirty_buf = 1;
      ioctl(isp_drv->vfe_fd, VIDIOC_MSM_ISP_ENQUEUE_BUF, &qbuf_info);
      //printf("\t%s: BUF EVENT %d\n", __func__, isp_event_data->frame_id);
      break;
    }
  case ISP_EVENT_STATS_NOTIFY:
    {
      struct msm_isp_qbuf_info qbuf_info;
      //printf("\t%s: STATS EVENT mask: 0x%x idx: %d\n", __func__,
      //    isp_event_data->u.stats.stats_mask, idx);
      memset(&qbuf_info, 0, sizeof(qbuf_info));
      qbuf_info.handle = isp_drv->stats_bufq_handle[idx];
      qbuf_info.buf_idx = isp_event_data->u.stats.stats_buf_idxs[idx];
      qbuf_info.dirty_buf = 1;
      ioctl(isp_drv->vfe_fd, VIDIOC_MSM_ISP_ENQUEUE_BUF, &qbuf_info);
      break;
    }
  default:
    printf("\t%s: ??? 0x%x\n", __func__, ev->type);
  }
}

void vfe_thread(struct isp_driver *isp_drv)
{
  int rc, i;
  struct v4l2_event_subscription sub;

  for (i = 0; i < ISP_EVENT_MAX; i++)
  {
    sub.id = 0;
    sub.type = ISP_EVENT_BASE + i;
    rc = ioctl(isp_drv->vfe_fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
  }
  for (i = 0; i < MAX_NUM_STREAM; i++)
  {
    sub.id = 0;
    sub.type = ISP_EVENT_BUF_DIVERT + i;
    rc = ioctl(isp_drv->vfe_fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
  }
  for (i = 0; i < MSM_ISP_STATS_MAX; i++)
  {
    sub.id = 0;
    sub.type = ISP_EVENT_STATS_NOTIFY + i;
    rc = ioctl(isp_drv->vfe_fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
  }
  sub.id = 0;
  sub.type = ISP_EVENT_COMP_STATS_NOTIFY;
  rc = ioctl(isp_drv->vfe_fd, VIDIOC_SUBSCRIBE_EVENT, &sub);

  struct pollfd fds[1];
  fds[0].fd = isp_drv->vfe_fd;
  fds[0].events = POLLPRI;

  do
  {
    rc = poll(fds, 1, 150);
    if (rc != 0)
    {
      struct v4l2_event ev;
      rc = ioctl(isp_drv->vfe_fd, VIDIOC_DQEVENT, &ev);
      proc_vfe_event(isp_drv, &ev);
    } else {
      printf(" Poll error isp id %d\n", isp_drv->isp_id);
    }
  }
  while (run_for_n_frames == 0 ||
         (uint32_t)isp_drv->frames_rcvd < run_for_n_frames);
  printf("\n");
}

int main(int argc, char **argv)
{
  struct isp_driver isp_drv;
  int c, isp = 0;
  char *dev_name = NULL;

  printf("usage:\n");
  printf("\t-i <isp_id - 0 or 1>\n");
  printf("\t-n <run for n frames (0 for non-stop)>\n");
  printf("\t-w <width of image>\n");
  printf("\t-h <height of image>\n");
  while ((c = getopt(argc, argv, "i:n:d:w:h:")) != -1) {
    switch (c) {
    case 'i':
      isp = atoi(optarg);
      fprintf(stderr, "Selected isp id %d\n", isp);
      break;
    case 'n':
      run_for_n_frames = atoi(optarg);
      break;
    case 'd':
      dump_every_n_frames = atoi(optarg);
      break;
    case 'w':
      frame_width = atoi(optarg);
      break;
    case 'h':
      frame_height = atoi(optarg);
      break;
    }
  }

  memset(&isp_drv, 0, sizeof(isp_drv));
  isp_drv.isp_id = isp;
#ifdef VFE44
  printf("Isp Kernel Test for VFE 4.4\nISP ID: %d\n", isp);
#else
  printf("Isp Kernel Test for VFE 4.6\nISP ID: %d\n", isp);
#endif
  if (dev_name) {
    isp_drv.vfe_fd  = open(dev_name, O_RDWR | O_NONBLOCK);
  } else {
    isp_drv.vfe_fd = isp_discover_subdev_nodes(isp);
  }

  if (isp_drv.vfe_fd < 0) {
    printf("Error opening device node %d\n", isp);
    return 0;
  }
  isp_drv.stats_stream_id = 0x10005 + isp_drv.isp_id;

  vfe_test_func(&isp_drv);
  return 0;
}


