/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc. 
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __HVX_STUB_STUB_H__
#define __HVX_STUB_STUB_H__

/* Maximu number for HVX buffers used for rotation for stats usecase */
#define MAX_HVX_BUFFERS 8
#define MAX_HVX_VFE     2

enum hvx_stub_event_type_t {
   HVX_STUB_EVENT_QUERY_CAPS,
   HVX_STUB_EVENT_OPEN,
   HVX_STUB_EVENT_STATIC_CONFIG,
   HVX_STUB_EVENT_DYNAMIC_CONFIG,
   HVX_STUB_EVENT_START,
   HVX_STUB_EVENT_UPDATE,
   HVX_STUB_EVENT_RESET,
   HVX_STUB_EVENT_STOP,
   HVX_STUB_EVENT_EXCEPTION_STOP,
   HVX_STUB_EVENT_CLOSE,
   HVX_STUB_REQUEST_BUFFERS,
   HVX_STUB_SEND_BUFFER,
   HVX_STUB_SET_CALLBACK_FUNC,
   HVX_STUB_EVENT_ENABLE_STATS,
   HVX_STUB_EVENT_DISABLE_STATS,
   HVX_STUB_EVENT_ENABLE_DUMP,
   HVX_STUB_EVENT_DISABLE_DUMP,
   HVX_STUB_EVENT_MAX,
};
typedef enum hvx_stub_event_type_t hvx_stub_event_type_t;
enum hvx_stub_vfe_type_t {
   HVX_STUB_VFE_NULL,
   HVX_STUB_VFE0,
   HVX_STUB_VFE1,
   HVX_STUB_VFE_BOTH,
   HVX_STUB_VFE_MAX,
};
typedef enum hvx_stub_vfe_type_t hvx_stub_vfe_type_t;

enum hvx_stub_request_buffer_type_t {
   HVX_STUB_STATS,
   HVX_STUB_FRAME_DUMP,
   HVX_STUB_REQUEST_BUFFER_TYPE_MAX,
};
typedef enum hvx_stub_request_buffer_type_t hvx_stub_request_buffer_type_t;

enum hvx_stub_pixel_format_t {
   HVX_STUB_BAYER_RGGB,
   HVX_STUB_BAYER_BGGR,
   HVX_STUB_BAYER_GRBG,
   HVX_STUB_BAYER_GBRG,
   HVX_STUB_BAYER_UYVY,
   HVX_STUB_BAYER_VYUY,
   HVX_STUB_BAYER_YUYV,
   HVX_STUB_BAYER_YVYU,
   HVX_STUB_BAYER_MAX,
};
typedef enum hvx_stub_pixel_format_t hvx_stub_pixel_format_t;
enum hvx_stub_vector_mode_t {
   HVX_STUB_VECTOR_NULL,
   HVX_STUB_VECTOR_32,
   HVX_STUB_VECTOR_64,
   HVX_STUB_VECTOR_INVALID,
   HVX_STUB_VECTOR_128,
   HVX_STUB_VECTOR_MAX,
};
typedef enum hvx_stub_vector_mode_t hvx_stub_vector_mode_t;
typedef struct hvx_stub_query_caps_t hvx_stub_query_caps_t;
struct hvx_stub_query_caps_t {
   hvx_stub_vector_mode_t hvx_stub_vector_mode;
   int max_hvx_unit;
};
typedef struct hvx_stub_open_t hvx_stub_open_t;
struct hvx_stub_open_t {
   char name[32];
   hvx_stub_vfe_type_t vfe_id;
   int dsp_clock;
   int bus_clock;
   int dsp_latency;
   int handle;
};
typedef struct hvx_stub_static_config_t hvx_stub_static_config_t;
struct hvx_stub_static_config_t {
   int handle;
   hvx_stub_vfe_type_t vfe_id;
   int hvx_unit_no[2];
   hvx_stub_vector_mode_t hvx_stub_vector_mode;
   int width;
   int height;
   int image_overlap;
   int right_image_offset;
   hvx_stub_pixel_format_t pixel_format;
   int bits_per_pixel;
   int rx_lines[2];
   int rx_line_width[2];
   int rx_line_stride[2];
   int tx_lines[2];
   int tx_line_width[2];
   int tx_line_stride[2];
   unsigned int vfe_clk_freq[2];
};

typedef enum hvx_stub_state_t hvx_stub_state_t;
enum hvx_stub_state_t {
  HVX_STUB_STATE_IFACE,
  HVX_STUB_STATE_3A,
  HVX_STUB_STATE_ADSP,
  HVX_STUB_STATE_MAX,
};

typedef struct hvx_stub_request_buffers_t hvx_stub_request_buffers_t;
struct hvx_stub_request_buffers_t {
  int              buffer_size;
  int              num_buffers;
  char             buf_label[MAX_HVX_BUFFERS];
  uint64_t         buffers[MAX_HVX_BUFFERS];
  hvx_stub_state_t state[MAX_HVX_BUFFERS];
  hvx_stub_request_buffer_type_t buf_type;
};

typedef struct hvx_stub_send_buffer_t hvx_stub_send_buffer_t;
struct hvx_stub_send_buffer_t {
  hvx_stub_vfe_type_t vfe_type;
  hvx_stub_request_buffer_type_t buf_type;
  int                 buffer_idx[MAX_HVX_VFE];
  char                buf_label[MAX_HVX_VFE];
  char               *addr[MAX_HVX_VFE];
  int                 addr_Len[MAX_HVX_VFE];
};

typedef struct hvx_stub_set_callback_func_t hvx_stub_set_callback_func_t;
struct hvx_stub_set_callback_func_t {
  int (*callback_func)(int handle, enum hvx_stub_vfe_type_t stub_vfe_type,
    unsigned char buf_label);
  int (*Error_callback)(int handle, enum hvx_stub_vfe_type_t stub_vfe_type,
    const char* error_msg, int error_msgLen, int frame_id);
};

int hvx_stub_callback_data(int handle, uint32_t vfe_type,
  unsigned char buf_label);

int hvx_stub_callback_error(int handle, uint32_t vfe_type,
   const char* error_msg, int error_msgLen, int frame_id);

#endif
