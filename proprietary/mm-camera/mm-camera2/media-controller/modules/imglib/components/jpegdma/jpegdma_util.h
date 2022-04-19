/*************************************************************
* Copyright (c) 2016-2017 Qualcomm Technologies, Inc.        *
* All Rights Reserved.                                       *
* Confidential and Proprietary - Qualcomm Technologies, Inc. *
*************************************************************/

/* jpeg dma v4l2 device driver name*/
#define JDMA_UTIL_V4l2_DRV_NAME "msm_jpegdma"
/* jpeg dma node max name length. */
#define JDMA_UTIL_MAX_NAME_SIZE 128
/* v4l2 system path*/
#define JDMA_UTIL_V4L2_SYS_PATH "/sys/class/video4linux/"
/* max num devices to find jpeg v4l2 device from*/
#define JDMA_UTIL_MAX_DEVICES 20
/* max v4l2 request buffers*/
#define JDMA_UTIL_MAX_REQBUF 1
/* Timeout for message ack .*/
#define JDMA_UTIL_MSG_ACK_TIMEOUT_MS 150
/* Max number of failed calls allowed in worker thread. */
#define JDMA_UTIL_MAX_FAIL_CNT 10

/* Convert frame buffer index to driver buffer index */
#define JDMA_UTIL_FRAME_TO_BUF_IDX(a) ((a) % JDMA_UTIL_MAX_REQBUF)

/** jpegdma_util_msg_mode_t
 *   @JDMA_MSG_MODE_NON_BLOCK: Message will not block .
 *   @JDMA_MSG_MODE_BLOCK: Message will block until is processed
 *
 *   Jpeg dma working thread message mode.
 */
typedef enum {
  JDMA_MSG_MODE_NON_BLOCK,
  JDMA_MSG_MODE_BLOCK,
} jpegdma_util_msg_mode_t;

/** jpegdma_util_msg_type_t
 *   @JDMA_MSG_QUEUE_BUNDLE: Frame bundle is received
 *   @JDMA_MSG_STOP: Stop HW Faceproc.
 *   @JDMA_MSG_EXIT: Worker Thread exit command.
 *
 *   Jpeg dma working thread message type.
 */
typedef enum {
  JDMA_MSG_QUEUE_BUNDLE,
  JDMA_MSG_STOP,
  JDMA_MSG_EXIT,
} jpegdma_util_msg_type_t;

/** jpegdma_util_thread_msg_t
 *   @type: Jpeg dma message type.
 *   @mode: Jpeg dma message mode.
 *   @p_frame: Pointer to frame.
 *   @p_meta: Pointer to meta.
 *
 *   Jpeg dma working thread message.
 */
typedef struct {
  jpegdma_util_msg_type_t type;
  jpegdma_util_msg_mode_t mode;
  img_frame_bundle_t *p_bundle;
} jpegdma_util_thread_msg_t;
