/* isp_defs.h
 *
 * Copyright (c) 2012-2016 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#ifndef __ISP_DEFS_H__
#define __ISP_DEFS_H__

#include "chromatix.h"

#if OVERRIDE_FUNC
#include "isp_ext_defs.h"
#endif

#define ISP_SUBDEV_NAME_LEN 100
#define ISP_HW_NAME_LEN     100
#define ISP_SUBMOD_LIB_NAME 100
#define ISP_SUBMOD_NAME     100

#define ISP_MAX_STREAMS  8
#define ISP_MAX_META_REPORTING_QUEUE_DEPTH  4
#define ISP_MAX_STREAMS_TO_HAL        6
#define ISP_MAX_SESSION 8

#define ISP_MSM8960V1    0x3030b
#define ISP_MSM8960V2    0x30408
#define ISP_MSM8930      0x3040f
#define ISP_MSM8610      0x3050A
#define ISP_MSM8974_V1   0x10000018
#define ISP_MSM8974_V2   0x1001001A
#define ISP_MSM8974_V3   0x1001001B
#define ISP_MSM8916_V1   0x10030000
#define ISP_MSM8939      0x10040000
#define ISP_MSM8956      0x10050000
#define ISP_MSM8952      0x10060000
#define ISP_MSM8937      0x10080000
#define ISP_MSM8917      0x10080001
#define ISP_MSM8953      0x10090000
#define ISP_MSM8226_V1   0x20000013
#define ISP_MSM8226_V2   0x20010014
#define ISP_MSM8084_V1   0x4000000A
#define ISP_MSM8992_V1   0x4002000A
#define ISP_MSM8994_V1   0x60000000
#define ISP_MSM8996_V1   0x70000000
#define ISP_MSM8998      0x80000000
#define ISP_SDM660       0x80000003

#define ISP_REVISION_V1  1
#define ISP_REVISION_V2  2
#define ISP_REVISION_V3  3

#define ISP_PIX_CLK_266MHZ 266000000
#define ISP_PIX_CLK_320MHZ 320000000
#define ISP_PIX_CLK_450MHZ 450000000
#define ISP_PIX_CLK_465MHZ 465000000
#define ISP_PIX_CLK_480MHZ 480000000
#define ISP_PIX_CLK_576MHZ 576000000
#define ISP_PIX_CLK_600MHZ 600000000

#define Q14   0x00004000
#define Q13   0x00002000
#define Q12   0x00001000
#define Q12_2 0x00002000
#define Q12_4 0x00004000
#define Q11   0x00000800
#define Q10   0x00000400
#define Q9    0x00000200
#define Q8    0x00000100
#define Q7    0x00000080
#define Q6    0x00000040
#define Q4    0x00000010

/* bit 16 - 31 used for isp version
 * bit 0 -15 used for isp revision */
#define GET_ISP_MAIN_VERSION(ver) ((ver & 0xFFFF0000) >> 16)
#define GET_ISP_SUB_VERSION(ver) (ver & 0xFFFF)
#define SET_ISP_VERSION(main_ver, sub_ver) \
(((main_ver & 0xFFFF) << 16) | (sub_ver & 0xFFFF))

#define ISP48_NUM_REG_DUMP 854 /* 0xD58(hex) = 3416 / 4 (4 byte per register) */
#define ISP47_NUM_REG_DUMP 821 /* 0xCD4(hex) = 3284 / 4 (4 byte per register) */
#define ISP46_NUM_REG_DUMP 740 /* 0xB90(hex) = 2960 / 4 (4 byte per register) */
#define ISP44_NUM_REG_DUMP 719 /* 0xB3C(hex) = 2876 / 4(4 byte per register) */
#define ISP40_NUM_REG_DUMP 748 /* 0x900(hex) = 2304 / 4(4 byte per register) */
#define ISP32_NUM_REG_DUMP 576

/* macros for unpacking identity */
#define ISP_GET_STREAM_ID(identity) ((identity) & 0xFFFF)
#define ISP_GET_SESSION_ID(identity) (((identity) & 0xFFFF0000) >> 16)
#define ISP_PACK_IDENTITY(sessionid,streamid) ((((sessionid) & 0xFFFF) << 16) |\
                                               ((streamid) & 0xFFFF))

#define READ_FD   0
#define WRITE_FD  1

#define ISP_APPEND_STREAMING_MODE(streaming_mode_mask, streaming_mode) \
  ((streaming_mode_mask) |= (1 << (streaming_mode)))

#define ISP_RESET_STREAMING_MODE_MASK(streaming_mode_mask) \
  ((streaming_mode_mask) = 0)

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define PAD_TO_SIZE(size, padding)  ((size + padding - 1) & ~(padding - 1))

#define IS_BAYER_FORMAT(fmt) \
  (fmt == CAMIF_BAYER_G_B || fmt == CAMIF_BAYER_B_G || \
   fmt == CAMIF_BAYER_G_R || fmt == CAMIF_BAYER_R_G)

#define IS_BURST_STREAMING(streaming_mode) \
  (((streaming_mode) & (1 << CAM_STREAMING_MODE_BURST)) ? TRUE : FALSE)

#define ISP_IS_AEC_CHECK_SETTLED(params) \
  ((params) && (params)->settled == 1) ? 1 : 0

#define FLOAT_TO_Q(exp, f) \
  ((int32_t)((f*(1<<(exp))) + ((f<0) ? -0.5 : 0.5)))

#define Q_TO_FLOAT(exp, i) \
  ((float)i/( 1 << (exp)))

/* Return v1 * ratio + v2 * ( 1.0 - ratio ) */
#define LINEAR_INTERPOLATION(v1, v2, ratio) \
  ((v2) + ((ratio) * ((v1) - (v2))))

#define LINEAR_INTERPOLATION_INT(v1, v2, ratio) \
  (roundf((v2) + ((ratio) * ((int)(v1) - (int)(v2)))))

#define MIN(x,y) (((x)<(y)) ? (x) : (y))
#define MAX(x,y) (((x)>(y)) ? (x) : (y))
#define CEILING16(X) (((X) + 0x000F) & 0xFFF0)
#define CEILING8(X)  (((X) + 0x0007) & 0xFFF8)
#define CEILING4(X)  (((X) + 0x0003) & 0xFFFC)
#define CEILING2(X)  (((X) + 0x0001) & 0xFFFE)

#define FLOOR32(X) ((X) & (~0x1F))
#define FLOOR16(X) ((X) & 0xFFF0)
#define FLOOR8(X)  ((X) & 0xFFF8)
#define FLOOR4(X)  ((X) & 0xFFFC)
#define FLOOR2(X)  ((X) & 0xFFFE)
#define ODD_FLOOR(X) ((X) & 0x0001) ? (X) : ((X) - 0x0001)
#define EVEN_FLOOR(X) ((X) & 0x0001) ? ((X) - 0x0001) : (X)
#define ODD_CEIL(X) ((X) & 0x0001) ? (X) : ((X) + 0x0001)
#define EVEN_CEIL(X) ((X) & 0x0001) ? ((X) + 0x0001) : (X)
#define IF_ODD(X) ((X) & 0x0001) ? TRUE : FALSE
#define IF_EVEN(X) ((X) & 0x0001) ? FALSE : TRUE

#define CameraExp(x) (exp(x))
#define CameraSquareRoot(x) (sqrt(x))

#define Clamp(x, t1, t2) (((x) < (t1))? (t1): ((x) > (t2))? (t2): (x))

#define DEFAULT_COLOR_TEMP 4100

#define MIRED(in, out) { \
  if (in) { \
    out = (1000000.0f / (float)(in)); \
  } else { \
    ISP_ERR("invalid color temp %d", (uint32_t)in); \
    out = ((float)1 / (float)DEFAULT_COLOR_TEMP); \
  } \
}

#define F_EQUAL(a, b) \
  ( fabs(a-b) < 1e-4 )

#define GET_INTERPOLATION_RATIO(ct, s, e) (1.0 - ((ct) - (s))/((e) - (s)))

#define MATCH(v1, v2, th) ((abs(v2-v1) <= (th)))

#define CALC_CCT_TRIGGER_MIRED(out, in) ({ \
  MIRED(in.CCT_start, out.mired_start); \
  MIRED(in.CCT_end, out.mired_end); \
})

#define TBL_INTERPOLATE(in1, in2, out, ratio, size, i) ({\
  for (i=0; i<size; i++) \
    out[i] = LINEAR_INTERPOLATION(in1[i], in2[i], ratio); })

#define TBL_INTERPOLATE_INT(in1, in2, out, ratio, size, i) ({\
  for (i=0; i<size; i++) \
    out[i] = LINEAR_INTERPOLATION_INT(in1[i], in2[i], ratio); })

#define CEIL_LOG2(n) ({ \
  int32_t val = 0, n1 = n; \
  if (n <= 1) \
    val = 0; \
  else { \
    while (n1 > 1) { \
      val++; \
      n1 >>= 1; \
    }\
  } \
  val;})

#define MATRIX_INVERSE_3x3(MatIn, MatOut) ({\
  typeof (MatOut[0]) __det; \
  if (MatIn == NULL || MatOut == NULL) { \
    ISP_ERR("failed, NULL pointer, matIn %p. Out %p", MatIn, MatOut); \
    return FALSE; \
  } \
  __det = MatIn[0]*(MatIn[4]*MatIn[8]-MatIn[5]*MatIn[7]) + \
          MatIn[1]*(MatIn[5]*MatIn[6]-MatIn[3]*MatIn[8]) + \
          MatIn[2]*(MatIn[3]*MatIn[7]-MatIn[4]*MatIn[6]); \
  if (__det == 0) { \
     ISP_DBG("failed det = 0!");\
    return FALSE; \
  } \
  MatOut[0] = (MatIn[4]*MatIn[8] - MatIn[5]*MatIn[7]) / __det; \
  MatOut[1] = (MatIn[2]*MatIn[7] - MatIn[1]*MatIn[8]) / __det; \
  MatOut[2] = (MatIn[1]*MatIn[5] - MatIn[2]*MatIn[4]) / __det; \
  MatOut[3] = (MatIn[5]*MatIn[6] - MatIn[3]*MatIn[8]) / __det; \
  MatOut[4] = (MatIn[0]*MatIn[8] - MatIn[2]*MatIn[6]) / __det; \
  MatOut[5] = (MatIn[2]*MatIn[3] - MatIn[0]*MatIn[5]) / __det; \
  MatOut[6] = (MatIn[3]*MatIn[7] - MatIn[4]*MatIn[6]) / __det; \
  MatOut[7] = (MatIn[1]*MatIn[6] - MatIn[0]*MatIn[7]) / __det; \
  MatOut[8] = (MatIn[0]*MatIn[4] - MatIn[1]*MatIn[3]) / __det; \
})


#define COPY_MATRIX(IN, OUT, M, N) ({ \
  int i, j; \
  for (i=0; i<M; i++) \
    for (j=0; j<N; j++) \
      OUT[i][j] = IN[i][j]; })

/* IN1 MxN  IN2 NxL*/
#define MATRIX_MULT(IN1, IN2, OUT, M, N, L) ({ \
  int i, j, k; \
  for (i=0; i<M; i++) \
    for (j=0; j<L; j++) { \
      OUT[i][j] = 0; \
      for (k=0; k<N; k++) \
        OUT[i][j] += (IN1[i][k] * IN2[k][j]); \
    } })

#define IS_UNITY_MATRIX(IN, N) ({\
  int i, j, ret = TRUE; \
  for (i=0; i<N; i++) \
     for (j=0; j<N; j++) \
       if (((i == j) && (!F_EQUAL(IN[i][j], 1))) \
         || ((i != j) && (!F_EQUAL(IN[i][j], 0)))) { \
         ret = FALSE; \
         break; \
       } \
   ret;})

#define SET_UNITY_MATRIX(IN, N) ({\
  int i, j;\
  for (i=0; i<N; i++) \
     for (j=0; j<N; j++) { \
       if (i == j) \
         IN[i][j] = 1; \
       else \
         IN[i][j] = 0; \
     }})


/* Convert M*N rational matrix into M*N float matrix */
#define RATIONAL_TO_FLOAT(In, Out, M, N) ({\
  int i, j; \
  for (i=0; i<M; i++) \
    for (j=0; j<N; j++) \
      Out[i][j] = (float)In[i][j].numerator / In[i][j].denominator; \
})

#define IS_MANUAL_WB(parms) (parms->cfg.wb_mode != CAM_WB_MODE_AUTO)
#define DEGREE_TO_RADIAN(d) \
  (0.0174532925 * d)

/* #define IS_MANUAL_WB(parms) (parms->wb_mode != CAMERA_WB_TYPE_AUTO) */

#define CUBIC_F(fs, fc0, fc1, fc2, fc3) ({ \
  double fs3, fs2;\
  fs2 = fs * fs; \
  fs3 = fs * fs2; \
  fc0 = 0.5 * (-fs3 + (2.0 * fs2) - fs); \
  fc1 = 0.5 * ((3.0 * fs3) - (5.0 * fs2) + 2.0); \
  fc2 = 0.5 * ((-3.0 * fs3) + (4.0 * fs2) + fs); \
  fc3 = 0.5 * (fs3 - fs2); \
})

#ifndef sign
#define sign(x) (((x) < 0) ? (-1) : (1))
#endif

#ifndef Round
#define Round(x) (int)((x) + sign(x)*0.5)
#endif

#define LINEAR_INTERPOLATION_BET(v1, v2, ratio) \
  Round((v2)*(1 - ratio) + (ratio * (v1)))

#define ASPECT_TOLERANCE 0.01
#define MAX_IDENTITES 8

/* Number of additional ports, currently 1 for session based steram
 */
#define ADDITIONAL_PORTS 1

/* Run tintless every alternate frames */
#define TINTLESS_FRAME_SKIP_STRING "2"
#define TINTLESS_FRAME_SKIP 2

/* Move common macros can be overridden in ext*/
#ifndef LTM_TYPE
#define LTM_TYPE(ltm_type, field) (ltm_type->field)
#endif

#ifndef LTM_CORE_TYPE
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION != 0x304)
  #define LTM_CORE_TYPE(ltm_type, field, idx) (idx >= LTM_MAX_LIGHT) ? 0 : \
    (ltm_type->chromatix_ltm_core_data[idx].field)
#else
  #define LTM_CORE_TYPE(ltm_type, field, idx) (ltm_type->field)
#endif
#endif

#ifndef BACKLIT_INTERP
#define BACKLIT_INTERP(field) ((backlit_ltm->field * backlight_scene_severity \
    + normal_ltm->field * (255 - backlight_scene_severity))/255)
#endif

#ifndef BACKLIT_INTERP_CORE
#define BACKLIT_INTERP_CORE(field, idx) BACKLIT_INTERP(field)
#endif

#ifndef GAMMA_LOWLIGHT_TABLE
#define GAMMA_LOWLIGHT_TABLE(pchromatix_gamma) ((pchromatix_gamma->lowlight_gamma_table))
#endif

#ifndef GAMMA_OUTDOOR_TABLE
#define GAMMA_OUTDOOR_TABLE(pchromatix_gamma) ((pchromatix_gamma->outdoor_gamma_table))
#endif

#ifndef GAMMA_DEFAULT_TABLE
#define GAMMA_DEFAULT_TABLE(pchromatix_gamma) ((pchromatix_gamma->default_gamma_table))
#endif

#ifndef GAMMA_OUTDOOR_TRIGGER
#define GAMMA_OUTDOOR_TRIGGER(pchromatix_gamma) ((pchromatix_gamma->gamma_outdoor_trigger))
#endif

#ifndef GAMMA_LOWLIGHT_TRIGGER
#define GAMMA_LOWLIGHT_TRIGGER(pchromatix_gamma) ((pchromatix_gamma->gamma_lowlight_trigger))
#endif

#ifndef AF_ROLLOFF_CHROMATIX_TYPE
#define AF_ROLLOFF_CHROMATIX_TYPE chromatix_rolloff_type
#endif

#ifndef ROLLOFF_CHROMATIX_TABLE_STROBE
#define ROLLOFF_CHROMATIX_TABLE_STROBE(pchromatix) (pchromatix->chromatix_mesh_rolloff_table_Strobe)
#endif

#ifndef CHROMATIX_BPC_BCC
#define CHROMATIX_BPC_BCC(chromatix_ptr, c) (chromatix_ptr->chromatix_VFE.chromatix_BPC.c##_enable)
#endif

#ifndef MAX_LIGHT_TYPES_FOR_SPATIAL_ABF
#define MAX_LIGHT_TYPES_FOR_SPATIAL_ABF MAX_LIGHT_TYPES_FOR_SPATIAL
#endif

/* Number of buffers for rotation between isp and 3A */
#define MAX_STATS_BUFFERS 3
/* Number of buffers for rotation between polling thread and parser thread */
#define MAX_STATS_CONFIG_BUF 3

/* ISO factor for conversion from gain to ISO */
#define MULTIPLY_FACTOR_FOR_ISO     100

/* Min scaling ratio */
#define DEFAULT_MIN_SCALE_RATIO  16
#endif
