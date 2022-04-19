/**********************************************************************
*  Copyright (c) 2013-2019 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#ifndef __IMG_COMMON_H__
#define __IMG_COMMON_H__

#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <linux/msm_ion.h>
#if TARGET_ION_ABI_VERSION >= 2
#include <ion/ion.h>
#endif
#include <stdbool.h>
#include "img_mem_ops.h"
#include "img_thread_ops.h"
#include <stdbool.h>
#include "cam_types.h"

/**
 * CONSTANTS and MACROS
 **/
#define BAYERPROC_FEATURE_SVHDR           (1<<1)
#define BAYERPROC_FEATURE_BINCORR         (1<<2)
#define BAYERPROC_MAX_USED_BUFS  16

#define MAX_PLANE_CNT 3
#define MAX_FRAME_CNT 2
#define GAMMA_TABLE_ENTRIES 64
#define RNR_LUT_SIZE 164
#define SKINR_LUT_SIZE 255
// Below should be dependent on FD setting.
#define MAX_FD_ROI 10
#define IMG_MAX_INPUT_FRAME 8
#define IMG_MAX_OUTPUT_FRAME 1
#define IMG_MAX_META_FRAME 8
#define MAX_IS_MESH_Y 10
#define MAX_MULTICAM_SESSIONS 4

#undef TRUE
#undef FALSE
#undef MIN
#undef MAX

#define TRUE 1
#define FALSE 0

#define IMG_LIKELY(x) __builtin_expect((x),1)
#define IMG_UNLIKELY(x) __builtin_expect((x),0)

#define MIN(a,b) ((a)>(b)?(b):(a))
#define MAX(a,b) ((a)<(b)?(b):(a))

#define MIN2(a,b)      ((a<b)?a:b)
#define MIN4(a,b,c,d)  (MIN2(MIN2(a,b),MIN2(c,d)))
#define MAX2(a,b)      ((a>b)?a:b)
#define MAX4(a,b,c,d)  (MAX2(MAX2(a,b),MAX2(c,d)))
#define CLIP(x, lower, upper)  {x = ((x < lower) ? lower : \
                               ((x > upper) ? upper : x)); }
#define CEILING8(X)  (((X) + 0x0007) & 0xFFF8)

/** BILINEAR_INTERPOLATION
 *
 *   Bilinear interpolation
 **/
#ifndef BILINEAR_INTERPOLATION
#define BILINEAR_INTERPOLATION(v1, v2, ratio) ((v1) + ((ratio) * ((v2) - (v1))))
#endif

/** FLOAT_TO_Q:
 *
 *   convert from float to integer
 **/
#ifndef FLOAT_TO_Q
#define FLOAT_TO_Q(exp, f) \
  ((int32_t)((f*(1<<(exp))) + ((f<0) ? -0.5 : 0.5)))
#endif

/** Sign:
 *
 *   Sign of number
 **/
#ifndef sign
#define sign(x) ((x < 0) ?(-1) : (1))
#endif

/** Round:
 *
 *   Round the value
 **/
#ifndef Round
#define Round(x) (int)(x + sign(x)*0.5)
#endif

/**
 *   feature enable/disable
 **/
#define IMG_ON  1
#define IMG_OFF 0

/**
 *   indices for semiplanar frame
 **/
#define IY 0
#define IC 1

/**
 *   chroma indices for planar frame
 **/
#define IC1 1
#define IC2 2

/** IMG_FLIP_VAR
 *   @x: variable to be flipped
 *
 *   Flip variable
 **/
#define IMG_FLIP_VAR(x) ((x) = !(x))

/** IMG_PAD_TO_X
 *   @v: value to be padded
 *   @x: alignment
 *
 *   Returns padded value w.r.t x
 **/
#define IMG_PAD_TO_X(v, x) (((v)+(x-1))&~(x-1))

/** IMG_MIN
 *
 *   returns minimum of the two values
 **/
#define IMG_MIN(a,b) ((a)>(b)?(b):(a))

/** IMG_MAX
 *
 *   returns maximum among the two values
 **/
#define IMG_MAX(a,b) ((a)<(b)?(b):(a))

/** IMG_AVG
 *
 *   returns average of the two values
 **/
#define IMG_AVG(a,b) (((a)+(b))/2)

/* utility functions to get frame info */
/** IMG_ADDR
 *   @p: pointer to the frame
 *
 *   Returns the Y address from the frame
 **/
#define IMG_ADDR(p) ((p)->frame[0].plane[0].addr)


/** IMG_PLANE_ADDR
 *   @p: pointer to the frame
 *   @ind: plane index to get the address
 *
 *   Returns the Y address from the frame
 **/
#define IMG_PLANE_ADDR(p, ind) ((p)->frame[0].plane[ind].addr)


/** IMG_WIDTH
 *   @p: pointer to the frame
 *
 *   Returns the Y plane width
 **/
#define IMG_WIDTH(p) ((p)->frame[0].plane[0].width)

/** FD_WIDTH
 *   @p: pointer to the frame
 *
 *   Returns the Y plane width.
 *   In SW Face detection stride is not supported, return width as stride.
 **/
#define IMG_FD_WIDTH(p) \
  ((p)->frame[0].plane[0].stride > (p)->frame[0].plane[0].width ? \
  (p)->frame[0].plane[0].stride : (p)->frame[0].plane[0].width)

/** IMG_HEIGHT
 *   @p: pointer to the frame
 *
 *   Returns the Y plane height
 **/
#define IMG_HEIGHT(p) ((p)->frame[0].plane[0].height)

/** IMG_Y_LEN
 *   @p: pointer to the frame
 *
 *   Returns the length of Y plane
 **/
#define IMG_Y_LEN(p) ((p)->frame[0].plane[0].length)

/** IMG_FD
 *   @p: pointer to the frame
 *
 *   Returns the fd of the frame
 **/
#define IMG_FD(p) ((p)->frame[0].plane[0].fd)

/** IMG_2_MASK
 *
 *   converts integer to 2^x
 **/
#define IMG_2_MASK(x) (1 << (x))

/** QIMG_CEILINGN
 *   @X: input 32-bit data
 *   @N: value to which input should be aligned
 *
 *   Align input X w.r.t N
 **/
#define QIMG_CEILINGN(X, N) (((X) + ((N)-1)) & ~((N)-1))

/** QIMG_WIDTH
 *   @p: pointer to the frame
 *
 *   Returns the ith plane width
 **/
#define QIMG_WIDTH(p, i) ((p)->frame[0].plane[i].width)

/** QIMG_HEIGHT
 *   @p: pointer to the frame
 *
 *   Returns the ith plane height
 **/
#define QIMG_HEIGHT(p, i) ((p)->frame[0].plane[i].height)

/** QIMG_STRIDE
 *   @p: pointer to the frame
 *
 *   Returns the ith plane stride
 **/
#define QIMG_STRIDE(p, i) ((p)->frame[0].plane[i].stride)

/** QIMG_SCANLINE
 *   @p: pointer to the frame
 *
 *   Returns the ith plane scanline
 **/
#define QIMG_SCANLINE(p, i) ((p)->frame[0].plane[i].scanline)

/** QIMG_LEN
 *   @p: pointer to the frame
 *
 *   Returns the ith plane length
 **/
#define QIMG_LEN(p, i) ((p)->frame[0].plane[i].length)

/** QIMG_FD
 *   @p: pointer to the frame
 *
 *   Returns the fd of the ith frame
 **/
#define QIMG_FD(p, i) ((p)->frame[0].plane[i].fd)

/** QIMG_ADDR
 *   @p: pointer to the frame
 *
 *   Returns the addr of the ith frame
 **/
#define QIMG_ADDR(p, i) ((p)->frame[0].plane[i].addr)

/** QIMG_LOCK
 *   @p: pointer to the mutex
 *
 *   macro for thread lock
 **/
#define QIMG_LOCK(p) pthread_mutex_lock(p)

/** QIMG_UNLOCK
 *   @p: pointer to the mutex
 *
 *   macro for thread unlock
 **/
#define QIMG_UNLOCK(p) pthread_mutex_unlock(p)

/** QIMG_PL_TYPE
 *   @p: pointer to the frame
 *
 *   Returns the plane type of the ith frame
 **/
#define QIMG_PL_TYPE(p, i) ((p)->frame[0].plane[i].plane_type)

/** QIMG_PL_TYPE
 *   @i: input
 *   @o: output
 *
 *   Returns margin between input and output
 **/
#define QIMG_CALC_MARGIN(i, o) (IMG_CEIL_FL1(((float)(i - o) / (float) o)))

/** IMG_FRAME_LEN
 *   @p: pointer to the frame
 *
 *   Returns the fd of the frame
 **/
#define IMG_FRAME_LEN(p) ({ \
  uint32_t i = 0, len = 0; \
  for (i = 0; i < (p)->frame[0].plane_cnt; i++) { \
    len += (p)->frame[0].plane[i].length; \
  } \
  len; \
})

/** IMG_CHECK_CROP_NEEDED
 *   @new_crop: new crop struct
 *   @curr_crop: current crop struct
 *
 *   Returns the fd of the frame
 *
 *   Note: crop is of struct img_rect_t
 **/
#define IMG_CHECK_CROP_NEEDED(new_crop, curr_crop) ({ \
  bool ret = false; \
  if (((new_crop.pos.y != curr_crop.pos.y) || \
    (new_crop.pos.x != curr_crop.pos.x) || \
    (new_crop.size.width != curr_crop.size.width) || \
    (new_crop.size.height != curr_crop.size.height)) && \
    ((new_crop.size.width != 0) && (new_crop.size.height != 0))) { \
    ret = true; \
  } \
  ret; \
})

/** IMG_CMP_DIM
 *   @new_dim: new dimension
 *   @curr_dim: current dimension
 *
 *   Returns the fd of the frame
 *
 *   Note: dimensions are of struct img_dim_t
 **/
#define IMG_CMP_DIM(new_dim, curr_dim) ({ \
  bool ret = true; \
  if((new_dim.width != curr_dim.width) || \
    (new_dim.height != curr_dim.height) || \
    (new_dim.stride != curr_dim.stride) || \
    (new_dim.scanline != curr_dim.scanline)) { \
    ret = false; \
  } \
  ret; \
})

/** Imaging values error values
*    IMG_SUCCESS - success
*    IMG_ERR_GENERAL - any generic errors which cannot be defined
*    IMG_ERR_NO_MEMORY - memory failure ION or heap
*    IMG_ERR_NOT_SUPPORTED -  mode or operation not supported
*    IMG_ERR_INVALID_INPUT - input passed by the user is invalid
*    IMG_ERR_INVALID_OPERATION - operation sequence is invalid
*    IMG_ERR_TIMEOUT - operation timed out
*    IMG_ERR_NOT_FOUND - object is not found
*    IMG_GET_FRAME_FAILED - get frame failed
*    IMG_ERR_OUT_OF_BOUNDS - input to function is out of bounds
*    IMG_ERR_SSR - DSP sub system restart error
*    IMG_ERR_EAGAIN - Execution not complete
*    IMG_ERR_FRAME_DROP: Frame needs to be dropped
*
**/
#define IMG_SUCCESS                   0
#define IMG_ERR_GENERAL              -1
#define IMG_ERR_NO_MEMORY            -2
#define IMG_ERR_NOT_SUPPORTED        -3
#define IMG_ERR_INVALID_INPUT        -4
#define IMG_ERR_INVALID_OPERATION    -5
#define IMG_ERR_TIMEOUT              -6
#define IMG_ERR_NOT_FOUND            -7
#define IMG_GET_FRAME_FAILED         -8
#define IMG_ERR_OUT_OF_BOUNDS        -9
#define IMG_ERR_BUSY                 -10
#define IMG_ERR_CONNECTION_FAILED    -11
#define IMG_ERR_SSR                  -12
#define IMG_ERR_EAGAIN               -13
#define IMG_ERR_FRAME_DROP           -14

/** SUBSAMPLE_TABLE
*    @in: input table
*    @in_size: input table size
*    @out: output table
*    @out_size: output table size
*    @QN: number of bits to shift while generating output tables
*
*    Macro to subsample the tables
**/
#define SUBSAMPLE_TABLE(in, in_size, out, out_size, QN) ({ \
  int i, j = 0, inc = (in_size)/(out_size); \
  for (i = 0, j = 0; j < (out_size) && i < (in_size); j++, i += inc) \
    out[j] = ((int32_t)in[i] << QN); \
})

/** IMG_ERROR
*    @v: status value
*
*    Returns true if the status is error
**/
#define IMG_ERROR(v) ((v) != IMG_SUCCESS)

/** IMG_SUCCEEDED
*    @v: status value
*
*    Returns true if the status is success
**/
#define IMG_SUCCEEDED(v) ((v) == IMG_SUCCESS)

/** IMG_LENGTH
*    @size: image size structure
*
*    Returns the length of the frame
**/
#define IMG_LENGTH(size) (size.width * size.height)

/** IMG_CEIL_FL1
*    @x: image to be converted
*
*    Ceil the image to one decimal point.
*    For eq:- 1.12 will be converted to 1.2
**/
#define IMG_CEIL_FL1(x) (((float)((long long)((x * 10) + .5f)) / 10))


/** IMG_TRANSLATE2
*    @v: value to be converted
*    @s: scale factor
*    @o: offset
*
*    Translate the coordinates w.r.t scale factors and offset
*    Use this if final coordinates need to be the reverse of
*    crop + downscale
**/
#define IMG_TRANSLATE2(v, s, o) ((float)(v) * (s) + (float)(o))

/** IMG_TRANSLATE
*    @v: value to be converted
*    @s: scale factor
*    @o: offset
*
*    Translate the coordinates w.r.t scale factors and offset
*    Use this if final coordinates need to be the reverse of
*    downscale + crop
**/
#define IMG_TRANSLATE(v, s, o) (((float)(v) - (float)(o)) * (s))

/** IMG_OFFSET_FLIP
*    @x: frame width/height
*    @v: value to be converted
*    @o: offset
*
*    Translate the scale factors w.r.t coordinates and offset
**/
#define IMG_OFFSET_FLIP(x, v, o) ((uint32_t)(x) - (uint32_t)(v) - (uint32_t)(o))

/** IMG_FLIP
*    @x: frame width/height
*    @v: value to be converted
*
*    Flip the factors w.r.t org width/height
**/
#define IMG_FLIP(x, v) ((uint32_t)(x) - (uint32_t)(v))

/** IMG_DUMP_TO_FILE:
 *  @filename: file name
 *  @p_addr: address of the buffer
 *  @len: buffer length
 *
 *  dump the image to the file
 **/
#define IMG_DUMP_TO_FILE(filename, p_addr, len) ({ \
  size_t rc = 0; \
  FILE *fp = fopen(filename, "w+"); \
  if (fp) { \
    rc = fwrite(p_addr, 1, len, fp); \
    IDBG_INFO("%s:%d] written size %zu", __func__, __LINE__, len); \
    fclose(fp); \
  } else { \
    IDBG_ERROR("%s:%d] open %s failed", __func__, __LINE__, filename); \
  } \
})


/** IMG_PRINT_RECT:
   *  @p: img rect
   *
   *  prints the crop region
   **/
#define IMG_PRINT_RECT(p) ({ \
  IDBG_MED("%s:%d] crop info (%d %d %d %d)", __func__, __LINE__, \
    (p)->pos.x, \
    (p)->pos.y, \
    (p)->size.width, \
    (p)->size.height); \
})

/** IMG_RECT_IS_VALID:
   *  @p: img rect
   *  @w: width of the main image
   *  @h: height of the main image
   *
   *  check if the region is valid
   **/
#define IMG_RECT_IS_VALID(p, w, h) (((p)->pos.x >= 0) && ((p)->pos.y >= 0) && \
  ((p)->size.width > 0) && ((p)->size.height > 0) && \
  (((p)->pos.x + (p)->size.width) < w) && \
  (((p)->pos.y + (p)->size.height) < h))


/** IMG_POINT_WITHIN_RECT:
   *  @p: [IN] point
   *  @r: [IN] region
   *
   *  check if the point p is with the region r
   **/
#define IMG_POINT_WITHIN_RECT(p, r) ( \
  (p.x >= r.pos.x) && \
  (p.y >= r.pos.y) && \
  (p.x < (r.pos.x + r.size.width)) && \
  (p.y < (r.pos.y + r.size.height)))

/** IMG_RECT_CENTER:
   *  @r: [IN] region
   *  @c: [OUT] center of the region
   *
   *  Returns the center of the rectangle
   **/
#define IMG_RECT_CENTER(r, c) ({ \
  c.x = r.pos.x + (r.size.width >> 1); \
  c.y = r.pos.y + (r.size.height >> 1); \
})

/** IMG_SQ:
   *  @x: [IN] variable
   *
   *  returns the square of a variable
   **/
#define IMG_SQ(x) ((x) * (x))

/** IMG_PT_SQ_DIST:
   *  @c1: [IN] point 1
   *  @c2: [IN] point 2
   *
   *  returns the square of distance between c1 and c2
   **/
#define IMG_PT_SQ_DIST(c1, c2) (IMG_SQ(c2.x - c1.x) + \
  IMG_SQ(c2.y - c1.y));

/** IMG_F_EQUAL:
 *  @a: floating point input
 *  @b: floating point input
 *
 *  checks if the floating point numbers are equal
 **/
#define IMG_F_EQUAL(a, b) (fabs(a-b) < 1e-4)

/** IMG_SWAP
 *  @a: input a
 *  @b: input b
 *
 *  Swaps the input values
 **/
#define IMG_SWAP(a, b) ({typeof(a) c; c=a; a=b; b=c;})

/** IMG_UNUSED
 *  @x: parameter to be supressed
 *
 *  Supress build warning for unused parameter
 **/
 #define IMG_UNUSED(x) (void)(x)

/**sigma_lut_in
  * Default sigma table for nornal lighting conditions
**/

/** IMG_CLEAR_BIT
 *  Macro to clear a bit at a given position
**/
#define IMG_CLEAR_BIT(mask, bit_pos) (mask &= ~(1 << bit_pos))

/** IMGLIB_ARRAY_SIZE:
 *    @a: array to be processed
 *
 * Returns number of elements in array
 **/
#define IMGLIB_ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

/** IMG_LINEAR_INTERPOLATE:
 *    @v1: start point
 *    @v2: end point
 *    @ratio: interpolation ratio
 *
 * interpolates b/w v1 and v2 based on ratio
 **/
#define IMG_LINEAR_INTERPOLATE(v1, v2, ratio) \
  ((v2) + ((ratio) * ((v1) - (v2))))

/** IMG_GET_INTERPOLATION_RATIO:
 *    @ct: current values
 *    @s: start
 *    @e: end
 *
 * Returns interpolation ratio
 **/
#define IMG_GET_INTERPOLATION_RATIO(ct, s, e) \
  (1.0 - ((ct) - (s))/((e) - (s)))

/** IMG_CONVERT_TO_Q:
 *    @v: start point
 *    @q: qfactor to be converted to
 *
 * convert the value to qfactor
 **/
#define IMG_CONVERT_TO_Q(v, q) \
  ((v) * (1 << (q)))

/** IMG_HYSTERESIS:
 *    @cur: current value
 *    @trig_a: point A trigger
 *    @trig_b: point B trigger
 *    @prev_st: previous state
 *    @a_st: state for region A
 *    @b_st: state for region B
 *
 *  Hysterisis for 3 region, 2 point (A, B)
 **/
#define IMG_HYSTERESIS(cur, trig_a, trig_b, prev_st, a_st, b_st) ({ \
  typeof(prev_st) _cur_st = prev_st; \
  if (cur < trig_a) \
    _cur_st = a_st; \
  else if (cur >= trig_b) \
    _cur_st = b_st; \
  _cur_st; \
})

/** IMG_RETURN_IF_NULL
 *   @p: pointer to be checked
 *
 *   Returns if pointer is null
 **/
#define IMG_RETURN_IF_NULL(ret, p) {if (!p) {\
  IDBG_ERROR("%s:%d Null pointer detected %s %p\n",\
    __func__, __LINE__, #p, p);\
  ret;\
}}

/** img_timer_granularity
 *  IMG_TIMER_MODE_MS: millisecond granularity
 *  IMG_TIMER_MODE_US: microsecond granularity
 *
 *  granularity of timer
 **/
typedef enum {
  IMG_TIMER_MODE_MS,
  IMG_TIMER_MODE_US,
} img_timer_granularity;

/** GET_TIME_IN_MICROS
 *   @time: struct timeval time
 *
 *   API to calculate the time in micro sec
 **/
#define GET_TIME_IN_MICROS(time) \
  ((1000000L * time.tv_sec) + time.tv_usec) \

/** GET_TIME_IN_MILLIS
 *   @time: struct timeval time
 *
 *   API to calculate the time in milli sec
 **/
#define GET_TIME_IN_MILLIS(time) \
  (((1000000L * time.tv_sec) + time.tv_usec) / 1000) \

/** IMG_TIMER_START
 *   @start: struct timeval start time
 *
 *   API to start timer
 **/
#define IMG_TIMER_START(start) ({ \
  gettimeofday(&start, NULL); \
})

/** IMG_TIMER_END
 *   @start: start time
 *   @end: used to store end time
 *   @str: string identifier to print in log
 *   @gran: granularity of time (ms or micros)
 *
 *   API to end timer and return delta
 **/
#define IMG_TIMER_END(start, end, str, gran) ({ \
  uint32_t delta = 0; \
  gettimeofday(&end, NULL); \
  switch (gran) { \
  case IMG_TIMER_MODE_US: \
    delta = GET_TIME_IN_MICROS(end) - GET_TIME_IN_MICROS(start); \
    IDBG_HIGH("%s:%d] %s time in micros: %d", __func__, __LINE__, \
      str, delta); \
    break; \
  case IMG_TIMER_MODE_MS: \
  default: \
    delta = GET_TIME_IN_MILLIS(end) - GET_TIME_IN_MILLIS(start); \
    IDBG_HIGH("%s:%d] %s time in ms: %d", __func__, __LINE__, str, delta); \
    break; \
  } \
  delta; \
})

/** IMG_DBG_TIMER_START
 *   @start: struct timeval start time
 *
 *   API to start timer when debug level is enabled
 **/
#define IMG_DBG_TIMER_START(start) ({ \
  if (g_imgloglevel > 3) { \
    IMG_TIMER_START(start); \
  } \
})

/** IMG_DBG_TIMER_END
 *   @start: start time
 *   @end: used to store end time
 *   @str: string identifier to print in log
 *   @gran: granularity of time (ms or micros)
 *
 *   API to end timer and return delta when debug level is enabled
 **/
#define IMG_DBG_TIMER_END(start, end, str, gran) ({ \
  uint32_t time = 0; \
  if (g_imgloglevel > 3) { \
    time = IMG_TIMER_END(start, end, str, gran); \
  } \
  time; \
})

#define IMG_DLSYM_MUST(p_lib_ptr, p_func_struct, fptr_name, func_name) ({ \
  *(void **)&(p_func_struct->fptr_name) = dlsym(p_lib_ptr, func_name); \
  if (p_func_struct->fptr_name == NULL) { \
    IDBG_ERROR("Loading %s error %s", func_name, dlerror()); \
    return IMG_ERR_NOT_FOUND; \
  } \
})

#define IMG_DLSYM_WARN(p_lib_ptr, p_func_struct, fptr_name, func_name) ({ \
  *(void **)&(p_func_struct->fptr_name) = dlsym(p_lib_ptr, func_name); \
  if (p_func_struct->fptr_name == NULL) { \
    IDBG_WARN("Loading %s error %s", func_name, dlerror()); \
  } \
})

/** IMG_IS_POINT_WITHIN_RECT
 *   @rect: rectangle to check within
 *   @point: point with x y coordinate
 *
 *   Checks if point is in the rectangle.
**/
#define IMG_IS_POINT_WITHIN_RECT(rect, point) \
  ((point.x >= (int32_t)rect.x) && (point.x < (int32_t)(rect.x + rect.dx)) \
  && (point.y >= (int32_t)rect.y) && (point.y < (int32_t)(rect.y + rect.dy)))

/** IMG_IS_POINT_VALID
 *   @point: point with x y coordinate
 *
 *   Checks if point is  a non zero point.
**/
#define IMG_IS_POINT_VALID(point) ((point.x != 0) && (point.y != 0))

/** IMG_SET_POINT_TO_RECT_CENTER
 *   @rect: rectangle to check within
 *   @point: point with x y coordinate
 *
 *   Checks if point is in the rectangle.
**/
#define IMG_SET_POINT_TO_RECT_CENTER(rect, point) \
  point.x = rect.x + rect.dx / 2; \
  point.y = rect.y + rect.dy / 2;

/** QIMG_CALC_10BPP_PACKED_SIZE
 *   @w: width
 *   @h: height
 *
 *   Returns size of buffer needed for 10BPP RAW buffer
 *   based on width and height
 **/
#define QIMG_CALC_10BPP_PACKED_SIZE(w, h) (w * h *5 / 4)

/** QIMG_CALC_PLAIN16_UNPACKED_SIZE
 *   @w: width
 *   @h: height
 *
 *   Returns size of buffer needed for PLAIN16 RAW buffer based
 *   on width and height
 **/
#define QIMG_CALC_PLAIN16_UNPACKED_SIZE(w, h) (w * h * 2)

extern float sigma_lut_in[RNR_LUT_SIZE];


typedef enum {
  DEPTH_AE_VAL
}img_get_data_lib_type_t;

/** img_plane_type_t
*    @PLANE_Y: Y plane
*    @PLANE_CB_CR: C plane for pseudo planar formats
*    @PLANE_CR_CB: C plane for interleaved CbCr components
*    @PLANE_CB: Cb plane for planar format
*    @PLANE_CR: Cr plane for planar format
*    @PLANE_Y_CB_Y_CR: Y Cb Y Cr interleaved format
*    @PLANE_Y_CR_Y_CB: Y Cr Y Cb interleaved format
*    @PLANE_CB_Y_CR_Y: Cb Y Cb Y interleaved format
*    @PLANE_CR_Y_CB_Y: Cr Y Cr Y interleaved format
*    @PLANE_ARGB: ARGB plane
*    @PLANE_BAYER_MIPI_RAW_10BPP: single plane
*      MIPI Raw 10BPP packed
*    @PLANE_BAYER_IDEAL_RAW_PLAIN16_12BPP:
*      PLAIN16 12BPP unpacked
*
*    Plane type. Sequence of the color components in each plane
**/
typedef enum {
  PLANE_Y,
  PLANE_CB_CR,
  PLANE_CR_CB,
  PLANE_CB,
  PLANE_CR,
  PLANE_Y_CB_Y_CR,
  PLANE_Y_CR_Y_CB,
  PLANE_CB_Y_CR_Y,
  PLANE_CR_Y_CB_Y,
  PLANE_ARGB,
  PLANE_BAYER_MIPI_RAW_10BPP,
  PLANE_BAYER_IDEAL_RAW_PLAIN16_12BPP,
} img_plane_type_t;

/** bayerproc_format_t
*    IMG_FMT_YUV_420_NV12: Todo
*    IMG_FMT_YUV_420_NV21: Todo
*    IMG_FMT_YUV_420_YV12: Todo
*    IMG_FMT_YUV_422_NV16: Todo
*    IMG_FMT_YUV_422_NV61: Todo
*    IMG_FMT_RAW_8BIT_YUYV: Todo
*    IMG_FMT_RAW_8BIT_YVYU: Todo
*    IMG_FMT_RAW_8BIT_UYVY: Todo
*    IMG_FMT_RAW_8BIT_VYUY: Todo
*    IMG_FMT_RAW8_QTI_GRBG: Todo
*    IMG_FMT_RAW8_QTI_GBRG: Todo
*    IMG_FMT_RAW8_QTI_BGGR: Todo
*    IMG_FMT_RAW8_QTI_RGGB: Todo
*    IMG_FMT_RAW10_QTI_GRBG: Todo
*    IMG_FMT_RAW10_QTI_GBRG: Todo
*    IMG_FMT_RAW10_QTI_BGGR: Todo
*    IMG_FMT_RAW10_QTI_RGGB: Todo
*    IMG_FMT_RAW12_QTI_GRBG: Todo
*    IMG_FMT_RAW12_QTI_GBRG: Todo
*    IMG_FMT_RAW12_QTI_BGGR: Todo
*    IMG_FMT_RAW12_QTI_RGGB: Todo
*    IMG_FMT_RAW8_MIPI_GRBG: Todo
*    IMG_FMT_RAW8_MIPI_GBRG: Todo
*    IMG_FMT_RAW8_MIPI_BGGR: Todo
*    IMG_FMT_RAW8_MIPI_RGGB: Todo
*    IMG_FMT_RAW10_MIPI_GRBG: Todo
*    IMG_FMT_RAW10_MIPI_GBRG: Todo
*    IMG_FMT_RAW10_MIPI_BGGR: Todo
*    IMG_FMT_RAW10_MIPI_RGGB: Todo
*    IMG_FMT_RAW12_MIPI_GRBG: Todo
*    IMG_FMT_RAW12_MIPI_GBRG: Todo
*    IMG_FMT_RAW12_MIPI_BGGR: Todo
*    IMG_FMT_RAW12_MIPI_RGGB: Todo
*    IMG_FMT_RAW8_PLAIN16_GRBG: Todo
*    IMG_FMT_RAW8_PLAIN16_GBRG: Todo
*    IMG_FMT_RAW8_PLAIN16_BGGR: Todo
*    IMG_FMT_RAW8_PLAIN16_RGGB: Todo
*    IMG_FMT_RAW10_PLAIN16_GRBG: Todo
*    IMG_FMT_RAW10_PLAIN16_GBRG: Todo
*    IMG_FMT_RAW10_PLAIN16_BGGR: Todo
*    IMG_FMT_RAW10_PLAIN16_RGGB: Todo
*    IMG_FMT_RAW12_PLAIN16_GRBG: Todo
*    IMG_FMT_RAW12_PLAIN16_GBRG: Todo
*    IMG_FMT_RAW12_PLAIN16_BGGR: Todo
*    IMG_FMT_RAW12_PLAIN16_RGGB: Todo
*
*  Img format type
**/
typedef enum {
  IMG_FMT_YUV_420_NV12,
  IMG_FMT_YUV_420_NV21,
  IMG_FMT_YUV_420_NV12_VENUS,
  IMG_FMT_YUV_420_NV21_VENUS,
  IMG_FMT_YUV_420_YV12,
  IMG_FMT_YUV_422_NV16,
  IMG_FMT_YUV_422_NV61,
  IMG_FMT_RAW_8BIT_YUYV,
  IMG_FMT_RAW_8BIT_YVYU,
  IMG_FMT_RAW_8BIT_UYVY,
  IMG_FMT_RAW_8BIT_VYUY,
  IMG_FMT_RAW8_GBRG,
  IMG_FMT_RAW8_GRBG,
  IMG_FMT_RAW8_BGGR,
  IMG_FMT_RAW8_RGGB,
  IMG_FMT_RAW8_QTI_GRBG,
  IMG_FMT_RAW8_QTI_GBRG,
  IMG_FMT_RAW8_QTI_BGGR,
  IMG_FMT_RAW8_QTI_RGGB,
  IMG_FMT_RAW10_QTI_GRBG,
  IMG_FMT_RAW10_QTI_GBRG,
  IMG_FMT_RAW10_QTI_BGGR,
  IMG_FMT_RAW10_QTI_RGGB,
  IMG_FMT_RAW12_QTI_GRBG,
  IMG_FMT_RAW12_QTI_GBRG,
  IMG_FMT_RAW12_QTI_BGGR,
  IMG_FMT_RAW12_QTI_RGGB,
  IMG_FMT_RAW8_MIPI_GRBG,
  IMG_FMT_RAW8_MIPI_GBRG,
  IMG_FMT_RAW8_MIPI_BGGR,
  IMG_FMT_RAW8_MIPI_RGGB,
  IMG_FMT_RAW10_MIPI_GRBG,
  IMG_FMT_RAW10_MIPI_GBRG,
  IMG_FMT_RAW10_MIPI_BGGR,
  IMG_FMT_RAW10_MIPI_RGGB,
  IMG_FMT_RAW12_MIPI_GRBG,
  IMG_FMT_RAW12_MIPI_GBRG,
  IMG_FMT_RAW12_MIPI_BGGR,
  IMG_FMT_RAW12_MIPI_RGGB,
  IMG_FMT_RAW8_PLAIN16_GRBG,
  IMG_FMT_RAW8_PLAIN16_GBRG,
  IMG_FMT_RAW8_PLAIN16_BGGR,
  IMG_FMT_RAW8_PLAIN16_RGGB,
  IMG_FMT_RAW10_PLAIN16_GRBG,
  IMG_FMT_RAW10_PLAIN16_GBRG,
  IMG_FMT_RAW10_PLAIN16_BGGR,
  IMG_FMT_RAW10_PLAIN16_RGGB,
  IMG_FMT_RAW12_PLAIN16_GRBG,
  IMG_FMT_RAW12_PLAIN16_GBRG,
  IMG_FMT_RAW12_PLAIN16_BGGR,
  IMG_FMT_RAW12_PLAIN16_RGGB
} img_format_t;

/** QIMG_SINGLE_PLN_INTLVD
 *   @p: pointer to the frame
 *
 *   Checks and returns true if the image format is interleaved
 *   YUV
 **/
#define QIMG_SINGLE_PLN_INTLVD(p) ({ \
  int8_t ret; \
  switch(QIMG_PL_TYPE(p, 0)) {\
    case PLANE_Y_CB_Y_CR: \
    case PLANE_Y_CR_Y_CB: \
    case PLANE_CB_Y_CR_Y: \
    case PLANE_CR_Y_CB_Y: \
    case PLANE_BAYER_MIPI_RAW_10BPP: \
    case PLANE_BAYER_IDEAL_RAW_PLAIN16_12BPP: \
      ret = TRUE; \
    break; \
    default: \
      ret = FALSE; \
  } \
  ret; \
})

/** img_subsampling_t
*    IMG_H2V2 - h2v2 subsampling (4:2:0)
*    IMG_H2V1 - h2v1 subsampling (4:2:2)
*    IMG_H1V2 - h1v2 subsampling (4:2:2)
*    IMG_H1V1 - h1v1 subsampling (4:4:4)
*
*    Image subsampling type
**/
typedef enum {
  IMG_H2V2,
  IMG_H2V1,
  IMG_H1V2,
  IMG_H1V1,
} img_subsampling_t;

/* img_sensor_custom_t: Describe sensor custom output
 *
 * enable: set if sensor output is custom format
 * subframes_cnt: number of interleaved frames
 * start_x: start x of active area
 * start_y: start y of active area
 * width: sensor output width (extra data included)
 * height: sensor output height (extra data included)
 */
typedef struct {
  unsigned short enable;
  unsigned int subframes_cnt;
  unsigned int start_x;
  unsigned int start_y;
  unsigned int width;
  unsigned int height;
} img_sensor_custom_t;

/** img_frame_info_t
*    @width: width of the frame
*    @height: height of the frame
*    @stride: stride of the frame
*    @fmt: frame format
*    @custom_format: Sensor custom format
*    @ss: subsampling for the frame
*    @analysis: flag to indicate if this is a analysis frame
*    @client_id: id provided by the client
*    @num_planes: number of planes
*
*    Returns true if the status is success
**/
typedef struct {
  uint32_t width;
  uint32_t height;
  uint32_t stride;
  img_format_t fmt;
  img_sensor_custom_t custom_format;
  img_subsampling_t ss;
  int analysis;
  int client_id;
  int num_planes;
} img_frame_info_t;

/** img_plane_t
*    @plane_type: type of the plane
*    @addr: address of the plane
*    @stride: stride of the plane
*    @length: length of the plane
*    @fd: fd of the plane
*    @height: height of the plane
*    @width: width of the plane
*    @offset: offset of the valid data within the plane
*    @scanline: scanline of the plane
*
*    Represents each plane of the frame
**/
typedef struct {
  img_plane_type_t plane_type;
  uint8_t *addr;
  uint32_t stride;
  size_t length;
  int32_t fd;
  uint32_t height;
  uint32_t width;
  uint32_t offset;
  uint32_t scanline;
} img_plane_t;

/** img_sub_frame_t
*    @plane_cnt: number of planes
*    @plane: array of planes
*
*    Represents each image sub frame.
**/
typedef struct {
  uint32_t plane_cnt;
  img_plane_t plane[MAX_PLANE_CNT];
} img_sub_frame_t;

/** img_frame_t
*    @timestamp: timestamp of the frame
*    @plane: array of planes
*    @frame_cnt: frame count, 1 for 2D, 2 for 3D
*    @idx: unique ID of the frame
*    @info: frame information
*    @private_data: private data associated with the client
*    @ref_count: ref count of the buffer
*    @req_buf_idx: store HAL3 requested buffer index
*
*    Represents a frame (2D or 3D frame). 2D contains only one
*    sub frame where as 3D has 2 sub frames (left/right or
*    top/bottom)
**/
typedef struct {
  uint64_t timestamp;
  img_sub_frame_t frame[MAX_FRAME_CNT];
  int frame_cnt;
  uint32_t idx;
  uint32_t frame_id;
  img_frame_info_t info;
  void *private_data;
  int ref_count;
  uint32_t req_buf_idx;
  uint32_t frame_invalid;
} img_frame_t;

/** img_size_t
*    @width: width of the image
*    @height: height of the image
*
*    Represents the image size
**/
typedef struct {
  int width;
  int height;
} img_size_t;

/** bayerproc_in_out_frame_info_t
*    @in_info: input frame info
*    @out_info: output frame info
*
*    Represents input out frame size
**/
typedef struct {
  img_frame_info_t in_info;
  img_frame_info_t out_info;
} bayerproc_in_out_frame_info_t;


/** bayerproc_frame_map_t
*    @frame_cnt: Frame count
*    @frames: subframes
*
*    Represents mapped frames
**/
typedef struct {
  uint32_t frame_cnt;
  img_sub_frame_t frames[BAYERPROC_MAX_USED_BUFS];
} bayerproc_frame_map_t;


/** bayerproc_frame_map_t
*    @number_of_frames: Num of frames
*    @frame_exp_compensation: Frame exposure
*     compensation values
*
*    Represents svhdr ratios
**/
typedef struct {
  uint32_t number_of_frames;
  float frame_exp_compensation[IMG_MAX_INPUT_FRAME];
}bayerproc_hdr_ratio_t;

/** bayerproc_prepare_info_t
*    @in_buf_map: Number of input buffers which will be used
*    @out_buf_map: Input buffers frame info.
*    @black_level_gr: Sensor black level for green red channel.
*    @black_level_r: Sensor black level for red channel.
*    @black_level_gb: Sensor black level for green blue channel.
*    @black_level_b: Sensor black level for blue channel.
*    @calibration_size: Calibration size.
*    @calibration_buf: Calibration buffer.
*    @number_of_hdr_frames: Number of hdr frames
*    @sensor_gain: Sensor gains
*    @linecount:  linecounts
*    @exposure_time: exposure time
*    @hdr_ratios: HDR ratios structure
*    @wb_r_gain: R gain
*    @wb_g_gain: G gain
*    @wb_b_gain: B gain
*    @interlaced: Frame interlaced flag
*    @adc_bit_depth: ADC bit depth
*    @tm_out_bit_depth: Tone mapping out bit depth
*    @bayer_gtm_gamma: GTM gamma
*    @hdr_dark_n1: Normalization factor
*    @hdr_dark_n2_minus_n1_normalization_factor: Algorithm tuning param
*    @hdr_max_weight: HDR max weight
*    @tm_gain: Tone mapping gain
*    @perf_hint: GPU performance mode
*    @num_gpu_passes: number of GPU passes
*    @hdr_ratio: HDR exp ratio
*
*    Represents prepare to start info.
**/
typedef struct {
  bayerproc_frame_map_t in_buf_map;
  bayerproc_frame_map_t out_buf_map;
  int32_t black_level_gr;
  int32_t black_level_r;
  int32_t black_level_gb;
  int32_t black_level_b;
  uint32_t calibration_size;
  void *calibration_buf;
  int32_t number_of_hdr_frames;
  float sensor_gain[IMG_MAX_INPUT_FRAME];
  int32_t linecount[IMG_MAX_INPUT_FRAME];
  int64_t exposure_time[IMG_MAX_INPUT_FRAME]; /* in micro sec */
  bayerproc_hdr_ratio_t hdr_ratios;
  float wb_r_gain;
  float wb_g_gain;
  float wb_b_gain;
  uint32_t interlaced;
  /* Tuning paramaters */
  int adc_bit_depth;
  int tm_out_bit_depth;
  float bayer_gtm_gamma;
  int hdr_dark_n1;
  float hdr_dark_n2_minus_n1_normalization_factor;
  int hdr_max_weight;
  float tm_gain;
  int perf_hint;
  int num_gpu_passes;
  float hdr_ratio;
} bayerproc_prepare_info_t;

/** bc_prestart_info_t
*    @in_buf_map: Number of input buffers which will be used
*    @out_buf_map: Input buffers frame info.
*    @hor_correction_mode: hor correction mode
*    @ver_correction_mode: ver correction mode
*    @alpha_row: Alpha Row filtering intensity
*    @beta_row:  Beta Row filtering intensity
*    @alpha_col: Alpha Column filtering intensity
*    @beta_col: Beta Column filtering intensity
*
*    Represents binning correction prepare to
*    start info.
*/
typedef struct {
  bayerproc_frame_map_t in_buf_map;
  bayerproc_frame_map_t out_buf_map;
  int hor_correction_mode;
  int ver_correction_mode;
  float alpha_row;
  float beta_row;
  float alpha_col;
  float beta_col;
} bc_prestart_info_t;

/** img_trans_info_t
 *   @h_scale: horizontal scale ratio to be applied on the
 *           result
 *   @v_scale: vertical scale ratio to be applied on the result.
*    @h_offset: horizontal offset
*    @v_offset: vertical offset
*
*    Translation information for the face cordinates
**/
typedef struct {
  float h_scale;
  float v_scale;
  int32_t h_offset;
  int32_t v_offset;
} img_trans_info_t;

/** img_pixel_t
*    @x: x cordinate of the pixel
*    @y: y cordinate of the pixel
*
*    Represents the image pixel
**/
typedef struct {
  int x;
  int y;
} img_pixel_t;

/** img_rect_t
*    @pos: position of the region
*    @size: size of the region
*
*    Represents the image region
**/
typedef struct {
  img_pixel_t pos;
  img_size_t size;
} img_rect_t;

/** img_3A_data_t
*    @lux_idx: Lux index
*    @gain: Gain value
*    @s_rnr_enabled: if skin rnr should be enabled
*
*    Common 3a data required by imaging modules
**/

typedef struct {
  float lux_idx;
  float gain;
  float prev_lux_value;
  float prev_gain_value;
  uint32_t s_rnr_enabled;
} img_3A_data_t;

/** img_dim_t
*    @width: Width of the imge
*    @height: height of the image
*    @scanline: scanline of the image
*    @stride: strie of the image
*
*    Image dimensions
**/
typedef struct {
  uint32_t width;
  uint32_t height;
  uint32_t scanline;
  uint32_t stride;
} img_dim_t;

/** img_margin_t
*    @w_margin: width margin
*    @h_margin: height margin
*
*    Image margin
**/
typedef struct {
  float w_margin;
  float h_margin;
} img_margin_t;

/** img_pad_t
*    @w_pad: width padding
*    @h_pad: height padding
*
*    Image margin
**/
typedef struct {
  uint32_t w_pad;
  uint32_t h_pad;
} img_pad_t;

/** img_preload_param_t
*    @max_dim: maximum dimension
*    @full_s_dim: full sensor dimension
*    @filter_arrangement: sensor filter arrangement
*    @pedestal: sensor pedestal value
*    @custom_cal_data: custom cal data from sensor
*
*    Structure to hold preload params to be sent to algo.
**/
typedef struct {
  img_dim_t max_dim;
  img_dim_t full_s_dim;
  int filter_arrangement;
  unsigned int pedestal;
  void* custom_cal_data;
} img_preload_param_t;

typedef struct {
  uint32_t last_frameid;
  uint32_t flush_done;
  uint32_t      eis_num_mesh_x;
  uint32_t      eis_num_mesh_y;
  float transform_matrix[(MAX_IS_MESH_Y + 1) * 9];
} img_eis_config_t;
/** img_gamma_t
*    @table: array of gamma values
*
*    Gamma table of size 64
**/
typedef struct {
  uint16_t table[GAMMA_TABLE_ENTRIES];
} img_gamma_t;

/** img_debug_info_t
*    @camera_dump_enabled: Flag indicating if dump
*        is enabled
*    @timestamp: Timestamp string when buffer was recieved
*    @meta_data: Debug data to be filled in
*
*   Debug Information
**/
typedef struct {
  uint8_t camera_dump_enabled;
  char timestamp[25];
  void *meta_data;
} img_debug_info_t;

/** img_lib_param_t
*
*   Parameters to be set in the core algorithm
*
**/
typedef enum {
  IMG_ALGO_IN_FRAME_DIM,
  IMG_ALGO_OUT_FRAME_DIM,
  IMG_ALGO_PRESTART,
  IMG_ALGO_EXEC_MODE,
} img_lib_param_t;

/** img_camera_role_t
*   @IMG_NO_FALLBACK: No fallback
*   @IMG_WIDE_FALLBACK: wide fallback
*   @IMG_TELE_FALLBACK: tele fallback
*
*    Camera fallback mode
**/
typedef enum {
  IMG_NO_FALLBACK,
  IMG_WIDE_FALLBACK,
  IMG_TELE_FALLBACK,
} img_camera_fallback_mode_t;

/** img_camera_role_t
*   @IMG_CAM_ROLE_DEFAULT: Camera Type Default
*   @IMG_CAM_ROLE_BAYER: Camera type Bayer
*   @IMG_CAM_ROLE_MONO: Camera type Mono
*   @IMG_CAM_ROLE_WIDE: Camera type wide
*   @IMG_CAM_ROLE_TELE: Camera type tele
*   @IMG_CAM_ROLE_WIDE_FALLBACK: Wide fallback
*   @IMG_CAM_ROLE_TELE_FALLBACK: Tele fallback
*   @IMG_CAM_ROLE_INVALID: Invalid role to indicate error
*
*    Camera Role
**/
typedef enum {
  IMG_CAM_ROLE_DEFAULT,
  IMG_CAM_ROLE_BAYER,
  IMG_CAM_ROLE_MONO,
  IMG_CAM_ROLE_WIDE,
  IMG_CAM_ROLE_TELE,
  IMG_CAM_ROLE_WIDE_FALLBACK,
  IMG_CAM_ROLE_TELE_FALLBACK,
  IMG_CAM_ROLE_INVALID,
} img_camera_role_t;

/** img_camera_mode_t
*   @IMG_CAM_MODE_DEFAULT: Camera mode default
*   @IMG_CAM_MODE_PREVIEW: Camera Mode Preview
*   @IMG_CAM_MODE_VIDEO: Camera Mode Video
*   @IMG_CAM_MODE_BOKEH: Camera Mode Bokeh
*
*   Camera mode
**/
typedef enum {
  IMG_CAM_MODE_DEFAULT,
  IMG_CAM_MODE_PREVIEW,
  IMG_CAM_MODE_VIDEO,
  IMG_CAM_MODE_BOKEH,
} img_camera_mode_t;

/** img_lib_param_t
*   @lib_data: data to configure in lib
*   @lib_param: type of data to configure
*   Structure to hold params set to lib
*
**/
typedef struct {
  void *lib_data;
  img_lib_param_t lib_param;
} img_lib_config_t;

/** img_lib_depth_map_config_t
 *   Structure to hold params depth_map
 *
 **/
typedef struct {
  img_lib_param_t  lib_param;
  cam_format_t     depth_map_sensor_img_format;
  cam_ae_mode_type depth_map_exp_mode;
  int64_t          depth_map_exp_val;
  cam_fps_range_t  depth_map_fps_range;
} img_lib_depth_map_config_t;

/** img_comp_mode_t
 * IMG_SYNC_MODE: The component will be executed in
 *   syncronous mode - per frame.
 *  IMG_ASYNC_MODE: The component will spawn a thread and will
 *  be executed asyncronously in the context of the component
 *  thread.
 *
 **/
typedef enum {
  IMG_SYNC_MODE,
  IMG_ASYNC_MODE,
} img_comp_mode_t;

/** img_preload_mode_t
 *  IMG_PRELOAD_COMMON_START_SESSION_MODE: preload will happen
 *    only when first session is started.
 *  IMG_PRELOAD_PER_SESSION_MODE: preload will happen for each
 *    session is started on stream info.
 *  IMG_PRELOAD_COMMON_STREAM_CFG_MODE: preload will happen for
 *    each session is started on stream cfg.
 *
 **/
typedef enum {
  IMG_PRELOAD_COMMON_START_SESSION_MODE = 0,
  IMG_PRELOAD_PER_SESSION_MODE,
  IMG_PRELOAD_COMMON_STREAM_CFG_MODE
} img_preload_mode_t;

/** img_comp_execution_mode_t
 * IMG_EXECUTION_SW: The component is executed in SW
 * IMG_EXECUTION_HW: The component is executed in HW
 * IMG_EXECUTION_SW_HW: The component is executed in both SW, HW
 *
 **/
typedef enum {
  IMG_EXECUTION_UNKNOWN,
  IMG_EXECUTION_SW,
  IMG_EXECUTION_HW,
  IMG_EXECUTION_SW_HW,
} img_comp_execution_mode_t;

/** img_comp_access_mode_t
 * IMG_ACCESS_READ: The component is Read only
 * IMG_ACCESS_READ_WRITE: The component reads the input frame
 *   and writes into output frame
 *
 **/
typedef enum {
  IMG_ACCESS_READ,
  IMG_ACCESS_READ_WRITE,
} img_comp_access_mode_t;

/** img_caps_t
 *   @num_input: number of input buffers
 *   @num_output: number of output buffers
 *   @num_slave_output: number of output slave buffers
 *     needed
 *   @num_master_output: number of output master buffers
 *     needed
 *   @num_meta: number of meta buffers
 *   @inplace_algo: Flag to indicate whether the algorithm is
 *     inplace. If not output buffers needs to be obtained
 *   @face_detect_tilt_cut_off: maximum angle for face tilt filter
 *   @num_release_buf: the number of total bufs released to HAL
 *   @ack_required: Flag to indicate whether the ack is required
 *   @share_client_per_session: Flag to indicate whether client is
 *     shared in session
 *   @num_overlap: number of overlap buffers needed for batch
 *     processing, has to be less than num_input
 *   @use_internal_bufs: flag to indicate if internal bufs
 *     should be used
 *   @is_offline_proc: flag to hint if offline processing
 *   @hold_meta: Flag to indicate whether the metadata needs to
 *             be held and not queued automatically to the
 *             component by the base module. if the hold_meta is
 *             set by the module, module should take the
 *             responsibility to queue meta
 *   @preload_mode: mode indicate whether preload
 *                       needs to be done once, per session,
 *                       or on some event
 *   @internal_buf_cnt: Number of internal buffer to be
 *                    allocated. This structure is valid only if
 *                    use_internal_bufs is set. if not set,
 *                    maximum number of buffers will be
 *                    allocated
 *   @num_hold_outbuf: max number of output bufs can
 *     be obtained by module queue for processing. value 0 => no limit
 *   @before_cpp: flag indicating if module before or after cpp,
 *              false = after; true = before
 *   @will_dim_change: flag indicating if module will change
 *                   dimensions, i.e input dimension != output
 *                   dimension
 *   @dual_camfeature: Enable if the feature is related
 *     to dual camera.
 *   Capabilities
 **/
typedef struct {
  int8_t num_input;
  int8_t num_output;
  int8_t num_slave_output;
  int8_t num_master_output;
  int8_t num_meta;
  int8_t inplace_algo;
  uint32_t face_detect_tilt_cut_off;
  int8_t num_release_buf;
  int8_t ack_required;
  int8_t share_client_per_session;
  int8_t num_overlap;
  int8_t use_internal_bufs;
  bool is_offline_proc;
  bool   hold_meta;
  int8_t preload_mode;
  uint32_t internal_buf_cnt;
  int8_t num_hold_outbuf;
  bool before_cpp;
  bool will_dim_change;
  int8_t disable_shared_port;
  int8_t dual_camfeature;
} img_caps_t;

/** img_opaque_data_set_t
 *    @p_data: pointer to the opaque data
 *    @data_size: size of the data in bytes
 *
 *    Opaque data for the 3rd party modules
 **/
typedef struct {
  void *p_data;
  uint32_t data_size;
} img_opaque_data_set_t;

/** img_multicam_dim_info_t
 *    @input_size: Input size
 *    @output_size: Output buffer size
 *    @output_size_changed: output size changed flag
 *    @orignal_output_size: orignal output size as requested by
 *      downstream module, used to store orignal value if
 *      output_size is modified .
 *    @format: Img format
 *    @input_margin: Input margin
 *    @input_pad: Input padding
 *    @output_pad: Output padding
 *    @cam_role: Camera role
 *    @sensor_calib_data: sensor calibration data
 *    @pix_pitch: pixel pitch
 *    @focal_length: focal length
 *    @cam_mode: camera mode
 *    @stream_type: stream type
 *    @fps: fps
 *
 *    Frameproc init params
 **/
typedef struct {
  img_dim_t input_size;
  img_dim_t output_size;
  bool output_size_changed;
  img_dim_t orignal_output_size;
  img_format_t format;
  img_margin_t input_margin;
  img_pad_t input_pad;
  img_pad_t output_pad;
  img_camera_role_t cam_role;
  img_opaque_data_set_t sensor_calib_data;
  float pix_pitch;
  float focal_length;
  img_camera_mode_t cam_mode;
  cam_stream_type_t stream_type;
  uint32_t fps;
} img_multicam_dim_info_t;

/** img_crop_caps_t
 * IMG_CROP_NOT_APPLIED: Algo will not apply crop
 * IMG_CROP_APPLIED: Algo will apply crop
 * IMG_CROP_UPDATED: Algo will update crop value and
 *   provide new crop in algo results
 *
 **/
typedef enum {
  IMG_CROP_NOT_APPLIED,
  IMG_CROP_APPLIED,
  IMG_CROP_UPDATED,
} img_crop_caps_t;

/** img_algo_caps_t
 *    @core_type: Core type
 *    @exec_mode: Execution mode
 *    @buff_access_mode: Buffer access mode
 *    @crop_caps: Crop capabilities
 *
 *    Multi camera algo capabilities
 **/
typedef struct {
  img_core_type_t core_type;
  img_comp_execution_mode_t exec_mode;
  img_comp_access_mode_t buff_access_mode;
  img_crop_caps_t crop_caps;
} img_algo_caps_t;

/** img_init_params_t
 *    @refocus: enable refocus encoding
 *    @client_id: client id of the thread manager
 *    @multicam_dim_info: Multicamera init data
 *    @num_of_sessions: Number of multi cam sessions
 *
 *    Frameproc init params
 **/
typedef struct {
  int refocus_encode;
  uint32_t client_id;
  img_multicam_dim_info_t multicam_dim_info;
} img_init_params_t;

/** img_init_params_t
 *    @client_id: client id of the thread manager
 *    @multicam_dim_info: Multicamera init data
 *    @num_of_sessions: Number of multi cam sessions
 *
 *    Frameproc init params
 **/
typedef struct {
  uint32_t client_id[MAX_MULTICAM_SESSIONS];
  img_multicam_dim_info_t multicam_dim_info[MAX_MULTICAM_SESSIONS];
  uint32_t num_of_sessions;
} img_multicam_init_params_t;

/** img_frame_ops_t
 *    @get_frame: The function pointer to get the frame
 *    @release_frame: The function pointer to release the frame
 *    @dump_frame: The function pointer to dump frame
 *    @get_meta: The function pointer to get meta
 *    @set_meta: The function pointer to set meta
 *    @image_copy: The function pointer to image copy
 *    @image_scale: image downscaling
 *    @p_appdata: app data
 *
 *    Frame operations for intermediate buffer
 **/
typedef struct {
  int (*get_frame)(void *p_appdata, img_frame_t **pp_frame);
  int (*release_frame)(void *p_appdata, img_frame_t *p_frame,
    int is_dirty);
  void (*dump_frame)(img_frame_t *img_frame, const char* file_name,
    uint32_t number, void *p_meta);
  void *(*get_meta)(void *p_meta, uint32_t type);
  int32_t (*set_meta)(void *p_meta, uint32_t type, void* val);
  int (*image_copy)(img_frame_t *out_buff, img_frame_t *in_buff);
  int (*image_scale)(void *p_src, uint32_t src_width, uint32_t src_height,
    uint32_t src_stride, void *p_dst, uint32_t dst_stride);
  void *p_appdata;
  bool (*get_data_lib)(void *p_appdata, uint32_t img_get_data_lib, void* val);
} img_frame_ops_t;

/** img_base_ops_t
 *    @mem_ops: memory operations
 *    @thread_ops: thread operations
 *    @max_w: maximum width.
 *    @max_h: maximum height.
 *
 *    Structure to hold memory/thread ops table and preload
 *    parameters
 **/
typedef struct {
  img_mem_ops_t mem_ops;
  img_thread_ops_t thread_ops;
  uint32_t max_w;
  uint32_t max_h;
} img_base_ops_t;

/** face_proc_scale_mn_v_info_t
*    @height: The possiblly cropped input height in whole in
*           pixels (N)
*    @output_height: The required output height in whole in
*                  pixels (M)
*    @step: The vertical accumulated step for a plane
*    @count: The vertical accumulated count for a plane
*    @index: The vertical index of line being accumulated
*    @p_v_accum_line: The intermediate vertical accumulated line
*                   for a plane
*
*    Used for downscaling image
*
**/
typedef struct {
  uint32_t height;
  uint32_t output_height;
  uint32_t step;
  uint32_t count;
  uint16_t *p_v_accum_line;
} img_scale_mn_v_info_t;

// M/N division table in Q10
static const uint16_t mn_division_table[] =
{
  1024,     // not used
  1024,     // 1/1
  512,     // 1/2
  341,     // 1/3
  256,     // 1/4
  205,     // 1/5
  171,     // 1/6
  146,     // 1/7
  128      // 1/8
};

/** img_mmap_info_ion
*    @ion_fd: ION file instance
*    @virtual_addr: virtual address of the buffer
*    @bufsize: size of the buffer
*    @ion_info_fd: File instance for current buffer
*
*    Used for maping data
*
**/
typedef struct img_mmap_info_ion
{
    int               ion_fd;
    unsigned char    *virtual_addr;
    unsigned int      bufsize;
    struct ion_fd_data ion_info_fd;
} img_mmap_info_ion;

/** img_ops_core_type
*
*    Different operations based on core
*
**/
typedef enum {
  IMG_OPS_C,
  IMG_OPS_NEON,
  IMG_OPS_NEON_ASM,
  IMG_OPS_DSP,
  IMG_OPS_GPU,
  IMG_OPS_FCV,
} img_ops_core_type;

/**
 * Array size to save start, end times while profiling.
 * If BUF_DIVERTs, processing are asynchronous,
 * array is required to calculate the correct total time.
 */
#define PROFILE_MAX_VAL 20

/** img_profiling_t
*    @start_time: Starting time
*    @end_time: End time
*    @intermediate1: Time points to save intermediate time intervals
*    @total_time_in_ms: Total time in ms
*    @num_frames_total: Total number of frames requested for processing
*    @num_frames_processed: number frames processed
*    @num_frames_skipped: number frames skipped
*    @intr_num_frames_total: total number frames at a specific interval
*    @intr_num_frames_processed: number frames processed at a specific interval
*
*    Data structure for profiling
*
**/
typedef struct {
  struct timeval start_time[PROFILE_MAX_VAL];
  struct timeval end_time[PROFILE_MAX_VAL];
  struct timeval intermediate1[PROFILE_MAX_VAL];
  uint32_t total_time_in_ms[PROFILE_MAX_VAL];
  uint32_t num_frames_total;
  uint32_t num_frames_processed;
  uint32_t num_frames_skipped;
  uint32_t intr_num_frames_total;
  uint32_t intr_num_frames_processed;
} img_profiling_t;

/** img_gravity_info_t
 *  @gravity:  gravity vector (x/y/z) in m/s/s
 *  @lin_accel: linear acceleration (x/y/z) in m/s/s
 *  @accuracy: sensor accuracy (unreliable/low/medium/high)
 *
 *  This structure is used to store and trasnmit gravity
 *  vector received from the motion sensor.
*
**/
typedef struct {
  float gravity[3];
  float lin_accel[3];
  uint8_t accuracy;
} img_gravity_info_t;

/** img_perf_t
*    @create: create handle
*    @destroy: destroy handle
*    @lock_start: start lock
*    @lock_end: end lock
*
*    Img lib perf handle
*
**/
typedef struct
{
  void* (*handle_create)();
  void (*handle_destroy)(void* p_perf);
  void* (*lock_start)(void* p_perf, int* p_perf_lock_params,
    int perf_lock_params_size, int duration);
  void (*lock_end)(void* p_perf, void* p_perf_lock);
} img_perf_t;

/** img_util_frame_to_find_t
 *   @frameid: frame_id.
 *   @buff_type: buffer type.
 *
 */
typedef struct {
  uint32_t frame_id;
  uint32_t buff_type;
} img_buffer_to_find_t;

/** img_get_subsampling_factor
*    @ss_type: subsampling type
*    @p_w_factor: pointer to the width subsampling factor
*    @p_h_factor: pointer to height subsampling factor
*
*    Get the width and height subsampling factors given the type
**/
int img_get_subsampling_factor(img_subsampling_t ss_type, float *p_w_factor,
  float *p_h_factor);

/** img_wait_for_completion
*    @p_cond: pointer to pthread condition
*    @p_mutex: pointer to pthread mutex
*    @ms: timeout value in milliseconds
*
*    This function waits until one of the condition is met
*    1. conditional variable is signalled
*    2. timeout happens
**/
int img_wait_for_completion(pthread_cond_t *p_cond, pthread_mutex_t *p_mutex,
  int32_t ms);

/** img_image_copy:
 *  @out_buff: output buffer handler
 *  @in_buff: input buffer handler
 *
 * Function to copy image data from source to destination buffer
 *
 * Returns IMG_SUCCESS in case of success
 **/
int img_image_copy(img_frame_t *out_buff, img_frame_t *in_buff);

/**
 * Function: img_translate_cordinates
 *
 * Description: Translate the cordinates from one window
 *             dimension to another
 *
 * Input parameters:
 *   dim1 - dimension of 1st window
 *   dim2 - dimension of 2nd window
 *   p_in_region - pointer to the input region
 *   p_out_region - pointer to the output region
 *   zoom_factor - zoom factor
 *   p_zoom_tbl - zoom table
 *   num_entries - number of zoom table entries
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
int img_translate_cordinates_zoom(img_size_t dim1, img_size_t dim2,
  img_rect_t *p_in_region, img_rect_t *p_out_region,
  double zoom_factor, const uint32_t *p_zoom_tbl,
  uint32_t num_entries);

/**
 * Function: img_translate_cordinates
 *
 * Description: Translate the region from one window
 *             dimension to another
 *
 * Input parameters:
 *   dim1 - dimension of 1st window
 *   dim2 - dimension of 2nd window
 *   p_in_region - pointer to the input region
 *   p_out_region - pointer to the output region
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_INVALID_INPUT
 *
 * Notes:  none
 **/
int img_translate_cordinates(img_size_t dim1, img_size_t dim2,
  img_rect_t *p_in_region, img_rect_t *p_out_region);

/**
 * Function: img_sw_scale_init_mn
 *
 * Description: init downscaling
 *
 * Input parameters:
 *   vInfo - contains width/height info for scaling
 *   pSrc - pointer to original img buffer
 *   srcWidth - original image width
 *   srcHeight - original image height
 *   srcStride - original image stride
 *   pDst - pointer to scaled image buffer
 *   dstWidth - desired width of schaled image
 *   dstHeight - desired height of scaled image
 *   dstStride - scaled image stride
 *
 * Return values: none
 *
 * Notes:  none
 **/
void img_sw_scale_init_mn(img_scale_mn_v_info_t*  vInfo,
  uint8_t  *pSrc,
  uint32_t  srcWidth,
  uint32_t  srcHeight,
  uint32_t  srcStride,
  uint8_t  *pDst,
  uint32_t  dstWidth,
  uint32_t  dstHeight,
  uint32_t  dstStride);

/**
 * Function: img_sw_scale_mn_vscale_byte
 *
 * Description: init Vertical M/N scaling on an input lines,
 * which is one byte per pixel
 *
 * Input parameters:
 *   p_v_info - contains width/height info for scaling
 *   p_output_line
 *   output_width
 *   p_input_line
 *
 * Return values:
 *   0 - accumulating
 *   1 - outputting 1 line
 *
 * Notes:  none
 **/
int img_sw_scale_mn_vscale_byte(img_scale_mn_v_info_t *p_v_info,
  uint8_t *p_output_line,
  uint32_t output_width,
  uint8_t *p_input_line);

/**
 * Function: img_sw_scale_mn_hscale_byte
 *
 * Description: init horizontal scaling
 *
 * Input parameters:
 *   p_output_line
 *   output_width - M value
 *   p_input_line
 *   input_width - N value
 *
 * Return values: None
 *
 * Notes:  none
 **/
void img_sw_scale_mn_hscale_byte (uint8_t *p_output_line,
  uint32_t                          output_width,
  uint8_t                          *p_input_line,
  uint32_t                          input_width     );

/**
 * Function: scalingInitMN
 *
 * Description: Image downscaling using MN method
 *
 * Input parameters:
 *   pSrc - pointer to original img buffer
 *   srcWidth - original image width
 *   srcHeight - original image height
 *   srcStride - original image stride
 *   pDst - pointer to scaled image buffer
 *   dstWidth - desired width of scaled image
 *   dstHeight - desired height of scaled image
 *   dstStride - desired stride of scaled image
 *
 * Return values: none
 *
 * Notes:  none
 **/
void img_sw_downscale(uint8_t *src,uint32_t srcWidth,uint32_t srcHeight,
  uint32_t srcStride, uint8_t *dst, uint32_t dstWidth, uint32_t dstHeight,
  uint32_t dstStride);

/**
 * Function: img_sw_downscale_2by2
 *
 * Description: Optimized version of downscale 2by2.
 *
 * Input parameters:
 *   p_src - Pointer to source buffer.
 *   src_width - Source buffer width.
 *   src_height - Source buffer height.
 *   src_stride - original image stride.
 *   p_dst - Pointer to scaled destination buffer.
 *   dst_stride - Destination stride.
 *
 * Return values: imaging errors
 **/
int32_t img_sw_downscale_2by2(void *p_src, uint32_t src_width, uint32_t src_height,
  uint32_t src_stride, void *p_dst, uint32_t dst_stride);

/** img_image_stride_fill:
 *  @out_buff: output buffer handler
 *
 * Function to fill image stride with image data
 *
 * Returns IMG_SUCCESS in case of success
 **/
int img_image_stride_fill(img_frame_t *out_buff);

/** img_alloc_ion:
 *  @mapion_list: Ion structure list to memory blocks to be allocated
 *  @num: number of buffers to be allocated
 *  @ionheapid: ION heap ID
 *  @cached:
 *    TRUE: mappings of this buffer should be cached, ion will do cache
            maintenance when the buffer is mapped for dma
 *    FALSE: mappings of this buffer should not be cached
 *
 * Function to allocate a physically contiguous memory
 *
 * Returns IMG_SUCCESS in case of success
 **/
int img_alloc_ion(img_mmap_info_ion *mapion_list, int num, uint32_t ionheapid,
  int cached);

/** img_free_ion:
 *  @mapion_list: Ion structure list to the allocated memory blocks
 *  @num: number of buffers to be freed
 *
 * Free ion memory
 *
 *
 * Returns IMG_SUCCESS in case of success
 **/
int img_free_ion(img_mmap_info_ion* mapion_list, int num);

int img_cache_ops_external (void *p_buffer, size_t size, uint32_t offset, int fd,
  img_cache_ops_t type, int ion_device_fd);

/** img_get_timestamp
 *  @timestamp: pointer to a char buffer. The buffer should be
 *    allocated by the caller
 *  @size: size of the char buffer
 *
 *  Get the current timestamp and convert it to a string
 *
 *  Return: None.
 **/
void img_get_timestamp(char *timestamp, uint32_t size);

/** img_dump_frame
 *    @img_frame: frame handler
 *    @file_name: file name prefix
 *    @number: number to be appended at the end of the file name
 *    @p_meta: metadata handler
 *
 * Saves specified frame to folder /data/vendor/camera/
 *
 * Returns None.
 **/
void img_dump_frame(img_frame_t *img_frame, const char* file_name,
  uint32_t number, void *p_meta);

/** img_perf_lock_handle_create
 *
 * Creates new performance handle
 *
 * Returns new performance handle
 **/
void* img_perf_handle_create();

/** img_perf_handle_destroy
 *    @p_perf: performance handle
 *
 * Destoyes performance handle
 *
 * Returns None.
 **/
void img_perf_handle_destroy(void* p_perf);

/** img_perf_lock_start
 *    @p_perf: performance handle
 *    @p_perf_lock_params: performance lock parameters
 *    @perf_lock_params_size: size of performance lock parameters
 *    @duration: duration
 *
 * Locks performance with specified parameters
 *
 * Returns new performance lock handle
 **/
void* img_perf_lock_start(void* p_perf, int* p_perf_lock_params,
  int perf_lock_params_size, int duration);

/** img_perf_lock_end
 *    @p_perf: performance handle
 *    @p_perf_lock: performance lock handle
 *
 * Locks performance with specified parameters
 *
 * Returns None.
 **/
void img_perf_lock_end(void* p_perf, void* p_perf_lock);

/**
 * parse callback function
 */
typedef void (*img_parse_cb_t) (void *, char *key, char *value);

/** img_parse_main
 *    @datafile: file to be parsed
 *    @p_userdata: userdata provided by the client
 *    @p_parse_cb: parse function provided by the client
 *
 *   Main function for parsing
 *
 * Returns imglib error values.
 **/
int img_parse_main(const char* datafile, void *p_userdata,
  img_parse_cb_t p_parse_cb);

/**
 * Function: img_plane_deinterleave.
 *
 * Description: Deinterleave single plane YUV format to
 *         semi-planar.
 *
 * Arguments:
 *   @p_src_buff - Pointer to src buffer.
 *   @type: format of the src buffer
 *   @p_frame - Pointer to face component frame where converted
 *     frame will be stored.
 *
 * Return values:
 *   IMG error codes.
 *
 * Notes: conversion to planar formats is not supported
 **/
int img_plane_deinterleave(uint8_t *p_src, img_plane_type_t type,
  img_frame_t *p_frame);

/**
 * Function: img_boost_linear_k
 *
 * Description: API to boost luma
 *
 * Arguments:
 *   @p_src_buff - Pointer to src buffer.
 *   @width: frame width
 *   @height: frame height
 *   @stride: frame stride
 *   @K: boost factor
 *   @use_asm: indicates whether assembly of C routine needs to
 *           be used
 *
 * Return values:
 *   None
 *
 **/
void img_boost_linear_k(uint8_t *p_src, uint32_t width, uint32_t height,
  int32_t stride, float K, int8_t use_asm);

/**
 * Function: img_sw_cds
 *
 * Description: Software CDS routine in neon intrinsics
 *
 * Input parameters:
 *   @p_src - Pointer to source buffer.
 *   @src_width - Source buffer width.
 *   @src_height - Source buffer height.
 *   @src_stride - original image stride.
 *   @p_dst - Pointer to scaled destination buffer.
 *   @dst_stride - Destination stride.
 *   @type: operation type
 *
 * Return values: none
 **/
int32_t img_sw_cds(uint8_t *p_src, uint32_t src_width,
  uint32_t src_height,
  uint32_t src_stride,
  uint8_t *p_dst,
  uint32_t dst_stride,
  img_ops_core_type type);

/**
 * Function: img_common_get_orientation_angle
 *
 * Description: This function returns the orientation angle
 *   based on gravity info
 *
 * Arguments:
 *   @gravity: gravity information
 *   @p_orientation_angle: pointer to fill orientation angle
 *
 * Return values:
 *    IMG_xx error codes
 *
 **/
int32_t img_common_get_orientation_angle(float *gravity,
  int32_t *p_orientation_angle);

/** img_common_align_gravity_to_camera:
 *
 * Description: This function aligns the gravity data to match the
 *   camera coordinate system.
 *
 * Arguments:
 *    @gravity: gravity information
 *    @sensor_mount_angle: camera mount angle (0, 90, 180, 270 degrees)
 *    @camera_position: camera position (front or back)
 *
 * Return values:
 *    IMG_xx error codes
 **/
int img_common_align_gravity_to_camera(float *gravity,
  uint32_t sensor_mount_angle, int camera_position);

/**
 * Function: img_common_handle_input_frame_cache_op
 *
 * Description: Handle input frame cache operations
 *   based on buffer_access flag
 *
 * Arguments:
 *   @exec_mode: Current module's execution mode
 *   @p_frame: img frame
 *   @ion_fd: ion fd handle
 *   @p_buffer_access: input buffer access flags. This function will update
 *     the same pointer with updated buffer access based on cache operation
 *     this function does.
 *
 * Return values:
 *    img error
 *
 * Notes: None
 **/
int32_t img_common_handle_input_frame_cache_op(
  img_comp_execution_mode_t exec_mode, img_frame_t* p_frame,
  int ion_fd, bool force_cache_op, uint32_t *p_buffer_access);

/**
 * Function: img_common_handle_output_frame_cache_op
 *
 * Description: Handle output frame cache operations
 *   based on buffer_access flag
 *
 * Arguments:
 *   @exec_mode: Current module's execution mode
 *   @p_frame: img frame
 *   @ion_fd: ion fd handle
 *   @p_buffer_access: input buffer access flags. This function will update
 *     the same pointer with updated buffer access based on cache operation
 *     this function does.
 *
 * Return values:
 *    img error
 *
 * Notes: None
 **/
int32_t img_common_handle_output_frame_cache_op(
  img_comp_execution_mode_t exec_mode, img_frame_t* p_frame,
  int ion_fd, bool force_cache_op, uint32_t *p_buffer_access);

/**
 * Function: img_common_handle_release_frame_cache_op
 *
 * Description: Handle cache operations while releasing the buf
 *
 * Arguments:
 *   @p_frame: img frame
 *   @ion_fd: ion fd handle
 *   @p_buffer_access: input buffer access flags. This function will update
 *     the same pointer with updated buffer access based on cache operation
 *     this function does.
 *
 * Return values:
 *    img error
 *
 * Notes: None
 **/
int32_t img_common_handle_release_frame_cache_op(
  img_frame_t* p_frame, int ion_fd, uint32_t *p_buffer_access);

/**
 * Function: img_common_handle_bufdone_frame_cache_op
 *
 * Description: Handle cache operations while doing buf done
 *
 * Arguments:
 *   @p_frame: img frame
 *   @ion_fd: ion fd handle
 *   @p_buffer_access: input buffer access flags. This function will update
 *     the same pointer with updated buffer access based on cache operation
 *     this function does.
 *
 * Return values:
 *    img error
 *
 * Notes: None
 **/
int32_t img_common_handle_bufdone_frame_cache_op(
  img_frame_t* p_frame, int ion_fd, uint32_t *p_buffer_access);

/**
 * Function: img_common_get_prop
 *
 * Description: This function returns property value in 32-bit
 * integer
 *
 * Arguments:
 *   @prop_name: name of the property
 *   @def_val: default value of the property
 *
 * Return values:
 *    value of the property in 32-bit integer
 *
 * Notes: API will return 0 in case of error
 **/
int32_t img_common_get_prop(const char* prop_name,
  const char* def_val);

/**
 * Function: img_common_get_frame_fmt
 *
 * Description: This function updates format required for
 *    graphic buffer map.
 *
 * Arguments:
 *   @frame_fmt: image frame format
 *   @update_format: graphic frame format
 *
 * Return values:
 *    img error
 *
 * Notes: None.
 **/
int32_t img_common_get_frame_fmt
  (img_format_t frame_fmt, int32_t *update_format);
#endif //__IMG_COMMON_H__
