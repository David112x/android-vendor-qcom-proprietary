/*============================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef VPU_CLIENT_H
#define VPU_CLIENT_H

#include <linux/videodev2.h>
#include <stdbool.h>

/** vpu_client_t
 *
 * Opaque data pointer for vpu client structure.
 * API does not require knowledge of internal implementation.
 */
typedef struct _vpu_client_t* vpu_client_t;

/** vpu_client_event_t
 *
 * Types of events which are notified via client callback
 */
typedef enum _vpu_client_event_t {
  VPU_CLIENT_EVENT_INPUT_BUF_READY,
  VPU_CLIENT_EVENT_OUTPUT_BUF_READY,
} vpu_client_event_t;

typedef struct _vpu_client_buf_req_t {
  uint32_t stride_padding;
  uint32_t scanline_padding;
} vpu_client_buf_req_t;

/** vpu_client_cb_t
 *
 *  client callback function type
 */
typedef void (*vpu_client_cb_t)(vpu_client_event_t event,
  void* arg, void* userdata);

/** vpu_client_create
 *
 * creates a new client instance
 *
 * @return vpu_client_t client instance
 */
vpu_client_t vpu_client_create();

/** vpu_client_init_streaming
 *
 * Initialize vpu driver for streaming.
 *
 * @param client vpu client instance
 * @param buf_count number of stream buffers
 *
 * @return int32_t (success:0, failure:-1)
 */
int32_t vpu_client_init_streaming(vpu_client_t client, int32_t buf_count);

/** vpu_client_set_format
 *
 *  set stream format(width, height, bytesperline, pixelformat
 *  etc.)
 *
 * @param client vpu client instance
 * @param format v4l2 format information for streaming
 *
 * @return int32_t (success:0, failure:-1)
 */
int32_t vpu_client_set_format(vpu_client_t client, struct v4l2_format format);

/** vpu_client_stream_on
 *
 * Start streaming on driver. Updates driver states and becomes
 * ready to process frames.
 *
 * @param client vpu client instance
 *
 * @return int32_t (success:0, failure:-1)
 */
int32_t vpu_client_stream_on(vpu_client_t client);

/** vpu_client_sched_frame_for_processing
 *
 * Schedules a frame to be processed by the vpu. Processing is
 * done asynchronously. When frame is done, client callback is
 * called to notify the caller.
 *
 * @param client vpu client instance
 * @param in_buf input buffer info
 * @param out_buf output buffer info
 *
 * @return int32_t (success:0, failure:-1)
 */
int32_t vpu_client_sched_frame_for_processing(vpu_client_t client,
  struct v4l2_buffer *in_buf, struct v4l2_buffer *out_buf);

/** vpu_client_stream_off
 *
 * Updates driver state and turns-off streaming. Blocks while
 * pending buffers are processed and then shuts down the stream.
 *
 * @param client vpu client instance
 *
 * @return int32_t (success:0, failure:-1)
 */
int32_t vpu_client_stream_off(vpu_client_t client);

void vpu_client_destroy(vpu_client_t client);

/** vpu_client_init_session
 *
 * Initializes a new session on vpu driver. Registers a callback
 * for notifying about events. Only 2 sessions can run in
 * parallel on vpu. If no sessions are available, this function
 * will fail.
 *
 * @param client vpu client instance
 * @param cb callback function
 * @param userdata
 *
 * @return int32_t (success:0, failure:-1)
 */
int32_t vpu_client_init_session(vpu_client_t client, vpu_client_cb_t cb,
  void *userdata);

/** vpu_client_deinit_session
 *
 * Cleans up session resources and updates driver state.
 *
 * @param client vpu client instance
 *
 * @return int32_t (success:0, failure:-1)
 */
int32_t vpu_client_deinit_session(vpu_client_t client);

/** vpu_client_update_chromatix
 *
 * Update chromatix data used for tuning and control
 *
 * @param client vpu_client instance
 * @param chromatix_ptr pointer to chromatix params
 *
 * @return int32_t (success:0, failure:-1)
 */
int32_t vpu_client_update_chromatix(vpu_client_t client,
  void* chromatix_ptr);

/** vpu_client_aec_update
 *
 * This function is used for on-the-fly control of the VPU TNR
 * Based on the current AEC trigger value, this function calculates the closest
 * region programmed into the VPU. Each region corresponds to noise level which
 * is used to change the NR strength.
 * regions (0, 9) corresponds to NR levels (10, 100)
 * level 0 corresponds to OFF condition, which is not used here. *
 *
 * @param client vpu client instance
 * @param gain exposure gain
 * @param lux_idx exposure lux index
 *
 * @return int32_t (success:0, failure:-1)
 */
int32_t vpu_client_aec_update(vpu_client_t client, float gain, float lux_idx);

/** vpu_client_set_tnr_enable
 *
 * Enables/Disables Temporal Noise Reduction applied in VPU
 * hardware
 *
 * @param client vpu client instance
 * @param enable (true: enable, false:disable)
 *
 * @return int32_t (success:0, failure:-1)
 */
int32_t vpu_client_set_tnr_enable(vpu_client_t client, bool enable);


int32_t vpu_client_get_buf_requirements(vpu_client_t client,
  vpu_client_buf_req_t* buf_req);
#endif
