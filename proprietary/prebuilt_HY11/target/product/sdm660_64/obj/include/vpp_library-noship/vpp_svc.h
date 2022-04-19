/*!
 * @file vpp_svc.h
 *
 * @cr
 * Copyright (c) 2015-2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * @services
 */
#ifndef _VPP_SVC_H
#define _VPP_SVC_H
/// @file vpp_svc.idl
///
#include "AEEStdDef.h"
#ifndef __QAIC_HEADER
#define __QAIC_HEADER(ff) ff
#endif //__QAIC_HEADER

#ifndef __QAIC_HEADER_EXPORT
#define __QAIC_HEADER_EXPORT
#endif // __QAIC_HEADER_EXPORT

#ifndef __QAIC_HEADER_ATTRIBUTE
#define __QAIC_HEADER_ATTRIBUTE
#endif // __QAIC_HEADER_ATTRIBUTE

#ifndef __QAIC_IMPL
#define __QAIC_IMPL(ff) ff
#endif //__QAIC_IMPL

#ifndef __QAIC_IMPL_EXPORT
#define __QAIC_IMPL_EXPORT
#endif // __QAIC_IMPL_EXPORT

#ifndef __QAIC_IMPL_ATTRIBUTE
#define __QAIC_IMPL_ATTRIBUTE
#endif // __QAIC_IMPL_ATTRIBUTE
#ifdef __cplusplus
extern "C" {
#endif
///////////*** Constants ***////////////////////////////////////
/** Control defines **/
#define MAX_NAME_LEN 64
#define MAX_TS_LEN 128
#define MIN_NAME_LEN 3
#define MAX_BUFFS_IN_GROUP 4
#define MAX_NUM_PLANES 3
#define PLANE_ID_Y 0
#define PLANE_ID_U 1
#define PLANE_ID_V 2
#define FUNC_ID_MASK 0xffff
#define VPP_FUNC_ID_INVALID 0xffffff
/** VPP Function Feature Fields **/
#define VPP_FUNC_DISABLED_MASK 0x0
#define VPP_FUNC_ENABLED_MASK 0x1
#define MVP_MIN_INPUT_WID 1
#define MVP_MAX_INPUT_WID 1920
#define MVP_MIN_INPUT_HT 1
#define MVP_MAX_INPUT_HT 1088
#define MVP_NUM_IN_BUFFS 4
#define MVP_NUM_OUT_BUFFS 2
#define NR_MIN_INPUT_WID 1
#define NR_MAX_INPUT_WID 4096
#define NR_MIN_INPUT_HT 1
#define NR_MAX_INPUT_HT 2176
#define OPSTATES 10
#define OPSTATE_LEN 16
#define BAR_STAT_COUNT_LEN 8
#define BAR_STAT_LEN 2
#define NP_ARR_LEN 17
#define MOTION_MAP_SIZE 16
#define NR_NUM_IN_BUFFS 1
#define NR_NUM_OUT_BUFFS 1
#define QBR_MIN_INPUT_WID 1
#define QBR_MAX_INPUT_WID 4096
#define QBR_MIN_INPUT_HT 1
#define QBR_MAX_INPUT_HT 2176
#define AIE_MIN_INPUT_WID 1
#define AIE_MAX_INPUT_WID 4096
#define AIE_MIN_INPUT_HT 1
#define AIE_MAX_INPUT_HT 2176
#define AIE_NUM_IN_BUFFS 1
#define AIE_NUM_OUT_BUFFS 1
#define FRC_MIN_INPUT_WID 324
#define FRC_MAX_INPUT_WID 1920
#define FRC_MIN_INPUT_HT 128
#define FRC_MAX_INPUT_HT 1088
#define FRC_NUM_IN_PIX_BUFFS 2
#define FRC_NUM_IN_MBI_BUFFS 1
#define FRC_NUM_OUT_BUFFS 1
#define FRC_MBI_BUF_IDX 2
///////////*** Interfaces ***////////////////////////////////////
/******* VPP_SVC Interface ******************/
/**
    * @enum     pd_status_t
    * @brief    PD Status: Non-CPZ or CPZ
    */
enum vpp_svc_pd_status_t {
   PD_STATUS_OPEN,
   PD_STATUS_PROTECTED,
   PD_STATUS_NUM,
   _32BIT_PLACEHOLDER_vpp_svc_pd_status_t = 0x7fffffff
};
typedef enum vpp_svc_pd_status_t vpp_svc_pd_status_t;
/////////////////////////////////////////////////////////
/**
    * @enum     pixel_fmt_t
    * @brief    Known pixel formats
    */
enum vpp_svc_pixel_fmt_t {
   PIXEL_FMT_NV12,
   PIXEL_FMT_NV21,
   PIXEL_FMT_YU12,
   PIXEL_FMT_YV12,
   PIXEL_FMT_NUM,
   _32BIT_PLACEHOLDER_vpp_svc_pixel_fmt_t = 0x7fffffff
};
typedef enum vpp_svc_pixel_fmt_t vpp_svc_pixel_fmt_t;
/////////////////////////////////////////////////////////////////////////////////
/**
    * @enum     field_fmt_t
    * @brief    Field arrangement
    */
enum vpp_svc_field_fmt_t {
   FIELD_FMT_PROGRESSIVE,
   FIELD_FMT_INTERLEAVED_TFF,
   FIELD_FMT_INTERLEAVED_BFF,
   FIELD_FMT_TFF,
   FIELD_FMT_BFF,
   FIELD_FMT_NUM,
   _32BIT_PLACEHOLDER_vpp_svc_field_fmt_t = 0x7fffffff
};
typedef enum vpp_svc_field_fmt_t vpp_svc_field_fmt_t;
/////////////////////////////////////
/**
    * @enum     compression_fmt_t
    * @brief    Compression format
    */
enum vpp_svc_compression_fmt_t {
   COMPRESSION_FMT_NONE,
   COMPRESSION_FMT_UBWC,
   _32BIT_PLACEHOLDER_vpp_svc_compression_fmt_t = 0x7fffffff
};
typedef enum vpp_svc_compression_fmt_t vpp_svc_compression_fmt_t;
/**
    * @enum     sess_support_t
    * @brief    Session support response
    */
enum vpp_svc_sess_support_t {
   SESS_SUPPORT_NONE,
   SESS_SUPPORT_PART,
   SESS_SUPPORT_FULL,
   SESS_SUPPORT_NUM,
   _32BIT_PLACEHOLDER_vpp_svc_sess_support_t = 0x7fffffff
};
typedef enum vpp_svc_sess_support_t vpp_svc_sess_support_t;
/////////////////////////////////////
/**
     * @Enum     vpp_func_id_t
     * @brief    VPP Function/block identifiers. This sequence of IDs also
     *          identifies the sequence in which the VPP functions should be triggered.
     */
enum vpp_svc_vpp_func_id_t {
   VPP_FUNC_ID_MVP,
   VPP_FUNC_ID_NR,
   VPP_FUNC_ID_IE,
   VPP_FUNC_ID_FRC,
   VPP_FUNC_ID_CUSTOM,
   VPP_FUNC_ID_NUM,
   _32BIT_PLACEHOLDER_vpp_svc_vpp_func_id_t = 0x7fffffff
};
typedef enum vpp_svc_vpp_func_id_t vpp_svc_vpp_func_id_t;
/////////////////////////////////////
/**
     * @Enum     vpp_func_state_t
     * @brief    VPP Function/Block states
     */
enum vpp_svc_vpp_func_state_t {
   VPP_FUNC_STATE_UNINIT,
   VPP_FUNC_STATE_RUNNING,
   VPP_FUNC_STATE_NUM,
   _32BIT_PLACEHOLDER_vpp_svc_vpp_func_state_t = 0x7fffffff
};
typedef enum vpp_svc_vpp_func_state_t vpp_svc_vpp_func_state_t;
/////////////////////////////////////
/**
     * @struct     cap_resource_t
     * @brief     Shared libs and other resources used to implement VPP and other functionality
     */
typedef struct vpp_svc_cap_resource_t vpp_svc_cap_resource_t;
struct vpp_svc_cap_resource_t {
   unsigned int vpp_func_id;
   unsigned int insertion_point;
   unsigned int feature_flags;
   unsigned int version_flags;
   char name_loc[64];
   char build_ts[128];
};
/**
     * @struct     cap_resource_list_t
     * @brief     List of VPP feature resources and operation flags. Each VPP block/function is referenced via ID.
     */
typedef struct vpp_svc_cap_resource_list_t vpp_svc_cap_resource_list_t;
struct vpp_svc_cap_resource_list_t {
   vpp_svc_cap_resource_t* resource;
   int resourceLen;
};
/**
     * @struct     buffer_attribute_t
     * @brief     Frame/field attributes
     */
typedef struct vpp_svc_buffer_attribute_t vpp_svc_buffer_attribute_t;
struct vpp_svc_buffer_attribute_t {
   unsigned int index;
   unsigned int offset;
   unsigned int plane_stride[3];
   unsigned int plane_sizebytes[3];
};
/**
     * @struct     raw_buffer_t
     * @brief     Raw buffer type
     */
typedef struct vpp_svc_raw_buffer_t vpp_svc_raw_buffer_t;
struct vpp_svc_raw_buffer_t {
   unsigned char* user_data;
   int user_dataLen;
};
/**
     * @struct     frame_group_descriptor_t
     * @brief     Descriptor for a group of frame/fields of same resolution and format
     */
typedef struct vpp_svc_frame_group_descriptor_t vpp_svc_frame_group_descriptor_t;
struct vpp_svc_frame_group_descriptor_t {
   unsigned short width;
   unsigned short height;
   vpp_svc_pixel_fmt_t pixelformat;
   vpp_svc_field_fmt_t fieldformat;
   vpp_svc_compression_fmt_t compressiontype;
   unsigned short numbuffers;
   vpp_svc_buffer_attribute_t bufferattributes[4];
   vpp_svc_raw_buffer_t* pixel_data;
   int pixel_dataLen;
};
/**
     * @struct     frame_group_hdr_t
     * @brief     Descriptor for a group of frame/fields of same resolution and format. Does not include buffer pointers.
     */
typedef struct vpp_svc_frame_group_hdr_t vpp_svc_frame_group_hdr_t;
struct vpp_svc_frame_group_hdr_t {
   unsigned short width;
   unsigned short height;
   vpp_svc_pixel_fmt_t pixelformat;
   vpp_svc_field_fmt_t fieldformat;
   vpp_svc_compression_fmt_t compressiontype;
   unsigned short numbuffers;
   vpp_svc_buffer_attribute_t bufferattributes[4];
};
//////////////// VPP Function/Block data //////////////////////////////////////////////////////////////
/**
     * @struct     session_info_t
     * @brief     Session information
     */
typedef struct vpp_svc_session_info_t vpp_svc_session_info_t;
struct vpp_svc_session_info_t {
   unsigned short in_frame_width;
   unsigned short in_frame_height;
   unsigned short frame_rate;
   unsigned short active_1080p30_sess;
};
////// De-Interlacer (MVP) ////////////////////////
/**
    * @enum     mvp_op_mode_t
    * @brief    Deinterlacer operation mode
    */
enum vpp_svc_mvp_op_mode_t {
   MVP_MODE_PASSTHROUGH,
   MVP_MODE_3FIELD,
   MVP_MODE_1FIELD,
   MVP_MODE_AUTO,
   MVP_MODE_NUM,
   _32BIT_PLACEHOLDER_vpp_svc_mvp_op_mode_t = 0x7fffffff
};
typedef enum vpp_svc_mvp_op_mode_t vpp_svc_mvp_op_mode_t;
//////////////////////////////
/**
     * @struct     mvp_params_t
     * @brief     De-Interlacer config params
     */
typedef struct vpp_svc_mvp_params_t vpp_svc_mvp_params_t;
struct vpp_svc_mvp_params_t {
   unsigned int update_flags;
   unsigned short in_frame_width;
   unsigned short in_frame_height;
   vpp_svc_mvp_op_mode_t mode;
};
////// Quantization Banding Reduction (QBR) ////////////////////////
/**
    * @enum     qbr_op_mode_t
    * @brief    QBR operation mode
    */
enum vpp_svc_qbr_op_mode_t {
   QBR_MODE_DISABLED,
   QBR_MODE_ENABLED,
   QBR_MODE_NUM,
   _32BIT_PLACEHOLDER_vpp_svc_qbr_op_mode_t = 0x7fffffff
};
typedef enum vpp_svc_qbr_op_mode_t vpp_svc_qbr_op_mode_t;
//////////////////////////////
/**
     * @struct     qbr_params_t
     * @brief     QBR config params
     */
typedef struct vpp_svc_qbr_params_t vpp_svc_qbr_params_t;
struct vpp_svc_qbr_params_t {
   unsigned int update_flags;
   unsigned short in_frame_width;
   unsigned short in_frame_height;
   vpp_svc_qbr_op_mode_t qbr_mode;
   unsigned char offset;
   unsigned char err_c0;
   unsigned char err_c1;
   unsigned char err_c2;
   unsigned char m_vertical_thr1;
   unsigned char m_vertical_thr2;
   unsigned char m_horizontal_thr1;
   unsigned char m_horizontal_thr2;
};
/**
    * @enum     nr_op_mode_t
    * @brief    NR operation mode
    */
enum vpp_svc_nr_op_mode_t {
   NR_MODE_PASSTHROUGH,
   NR_MODE_MANUAL,
   NR_MODE_AUTO,
   NR_MODE_NUM,
   _32BIT_PLACEHOLDER_vpp_svc_nr_op_mode_t = 0x7fffffff
};
typedef enum vpp_svc_nr_op_mode_t vpp_svc_nr_op_mode_t;
//////////////////////////////
////// Noise Reducer (NR) ////////////////////////
/**
     * @struct     nr_params_t
     * @brief     Noise Reducer config params
     */
typedef struct vpp_svc_nr_params_t vpp_svc_nr_params_t;
struct vpp_svc_nr_params_t {
   unsigned int update_flags;
   unsigned int in_frame_width;
   unsigned int in_frame_height;
   vpp_svc_nr_op_mode_t mode;
   unsigned int fb_mode;
   boolean force_input_feedback;
   unsigned int MNRlevel;
   unsigned int RNRlevel;
   unsigned int TNRlevel;
   unsigned int SNRlevel;
   unsigned int AutoBAR;
   unsigned int AutoMNR;
   unsigned int DEen;
   unsigned int DEgain;
   unsigned int DenhMin;
   unsigned int DenhMax;
   unsigned int NP;
   unsigned int NPcurveEn;
   unsigned int ChromaSpFact;
   unsigned int SpatialNRFact;
   unsigned int TNRen;
   unsigned int NP0;
   unsigned int NP1;
   unsigned int NP2;
   unsigned int NP3;
   unsigned int NP4;
   unsigned int NP5;
   unsigned int NP6;
   unsigned int NP7;
   unsigned int NP8;
   unsigned int NP9;
   unsigned int NP10;
   unsigned int NP11;
   unsigned int NP12;
   unsigned int NP13;
   unsigned int NP14;
   unsigned int NP15;
   unsigned int NP16;
   vpp_svc_qbr_params_t qbr_params;
};
/**
    * @enum     aie_op_mode_t
    * @brief    AIE operation mode
    */
enum vpp_svc_aie_op_mode_t {
   AIE_MODE_PASSTHROUGH,
   AIE_MODE_MANUAL,
   AIE_MODE_AUTO,
   AIE_MODE_NUM,
   _32BIT_PLACEHOLDER_vpp_svc_aie_op_mode_t = 0x7fffffff
};
typedef enum vpp_svc_aie_op_mode_t vpp_svc_aie_op_mode_t;
//////////////////////////////
////// Adaptive Image Enhancement (AIE) ////////////////////////
/**
     * @struct     aie_params_t
     * @brief     AIE config params
     */
typedef struct vpp_svc_aie_params_t vpp_svc_aie_params_t;
struct vpp_svc_aie_params_t {
   unsigned int update_flags;
   unsigned short in_frame_width;
   unsigned short in_frame_height;
   vpp_svc_aie_op_mode_t mode;
   int ltm_nEN_DE;
   float de_fDeGain0;
   float de_fDeOff0;
   int de_nDeMin0;
   int de_nDeMax0;
   float de_fDeGain1;
   float de_fDeOff1;
   int de_nDeMin1;
   int de_nDeMax1;
   float de_fDeGain2;
   float de_fDeOff2;
   int de_nDeMin2;
   int de_nDeMax2;
   float de_fDeGain3;
   float de_fDeOff3;
   int de_nDeMin3;
   int de_nDeMax3;
   unsigned int de_nDePower;
   float de_fDeSlope;
   int ltm_nEN_CA;
   int ltm_nCA_EN_LPF;
   short ltm_CA_params_EN_Y;
   short ltm_CA_params_LMODE_Y;
   short ltm_CA_params_TMODE0_Y;
   short ltm_CA_params_TMODE1_Y;
   short ltm_CA_params_TSIGN0_Y;
   short ltm_CA_params_TSIGN1_Y;
   short ltm_CA_params_HMIN_Y;
   short ltm_CA_params_HMAX_Y;
   unsigned short ltm_CA_params_YMIN_Y;
   unsigned short ltm_CA_params_YMAX_Y;
   unsigned short ltm_CA_params_SMIN_Y;
   unsigned short ltm_CA_params_SMAX_Y;
   short ltm_CA_params_EN_U;
   short ltm_CA_params_LMODE_U;
   short ltm_CA_params_TMODE0_U;
   short ltm_CA_params_TMODE1_U;
   short ltm_CA_params_TSIGN0_U;
   short ltm_CA_params_TSIGN1_U;
   short ltm_CA_params_HMIN_U;
   short ltm_CA_params_HMAX_U;
   unsigned short ltm_CA_params_YMIN_U;
   unsigned short ltm_CA_params_YMAX_U;
   unsigned short ltm_CA_params_SMIN_U;
   unsigned short ltm_CA_params_SMAX_U;
   short ltm_CA_params_EN_V;
   short ltm_CA_params_LMODE_V;
   short ltm_CA_params_TMODE0_V;
   short ltm_CA_params_TMODE1_V;
   short ltm_CA_params_TSIGN0_V;
   short ltm_CA_params_TSIGN1_V;
   short ltm_CA_params_HMIN_V;
   short ltm_CA_params_HMAX_V;
   unsigned short ltm_CA_params_YMIN_V;
   unsigned short ltm_CA_params_YMAX_V;
   unsigned short ltm_CA_params_SMIN_V;
   unsigned short ltm_CA_params_SMAX_V;
   int ltm_nLTM_EN_MAP;
   int ltm_nLTM_EN_SAT;
   int ltm_nLTM_NUMX;
   int ltm_nLTM_NUMY;
   int ltm_nLTM_BITS_SIZEX;
   int ltm_nLTM_BITS_SIZEY;
   int ltm_nLTM_SAT_GAIN;
   int ltm_nLTM_SAT_OFF;
   int ltm_nLTM_SAT_THR;
   int ace_nStrCon;
   int ace_nStrBriL;
   int ace_nStrBriH;
};
/**
    * @enum     frc_op_mode_t
    * @brief    FRC operation mode
    */
enum vpp_svc_frc_op_mode_t {
   FRC_MODE_PASSTHROUGH,
   FRC_MODE_MANUAL,
   FRC_MODE_AUTO,
   FRC_MODE_NUM,
   _32BIT_PLACEHOLDER_vpp_svc_frc_op_mode_t = 0x7fffffff
};
typedef enum vpp_svc_frc_op_mode_t vpp_svc_frc_op_mode_t;
//////////////////////////////
/**
    * @enum     frc_mc_qual_t
    * @brief    FRC MC quality setting
    */
enum vpp_svc_frc_mc_qual_t {
   FRC_MC_QUAL_MED,
   FRC_MC_QUAL_WEAK,
   FRC_MC_QUAL_STRONG,
   FRC_MC_QUAL_REPEAT,
   FRC_MC_QUAL_INTERP,
   FRC_MC_QUAL_NUM,
   _32BIT_PLACEHOLDER_vpp_svc_frc_mc_qual_t = 0x7fffffff
};
typedef enum vpp_svc_frc_mc_qual_t vpp_svc_frc_mc_qual_t;
//////////////////////////////
////// Frame Rate Conversion (FRC) ////////////////////////
/**
     * @struct     frc_params_t
     * @brief     FRC config params
     */
typedef struct vpp_svc_frc_params_t vpp_svc_frc_params_t;
struct vpp_svc_frc_params_t {
   unsigned int update_flags;
   unsigned short in_frame_width;
   unsigned short in_frame_height;
   vpp_svc_frc_op_mode_t mode;
   unsigned short mc_quality;
   unsigned short no_frame_copy;
   unsigned short RepeatMode_repeatPeriod;
   unsigned short TH_MOTION;
   unsigned short TH_MOTION_LOW;
   unsigned short TH_MVOUTLIER_COUNT;
   unsigned short TH_MVOUTLIER_COUNT_LOW;
   unsigned short TH_OCCLUSION;
   unsigned short TH_OCCLUSION_LOW;
   unsigned short TH_MOTION00;
   unsigned short TH_MOTION00_LOW;
   unsigned short TH_MVOUTLIER_VARIANCE_COUNT;
   unsigned short TH_MVOUTLIER_VARIANCE_COUNT_LOW;
   unsigned short TH_SCENECUT;
   unsigned short TH_VARIANCE;
   unsigned short TH_SAD_FR_RATIO;
};
////// VPP Configuration Parameters ////////////////////////
/**
     * @struct     gen_params_t
     * @brief     Generic parameters. Applied at the end (and independently) of the pipeline.
     */
typedef struct vpp_svc_gen_params_t vpp_svc_gen_params_t;
struct vpp_svc_gen_params_t {
   unsigned short roi_en;
   unsigned short roi_x_tl;
   unsigned short roi_y_tl;
   unsigned short roi_x_br;
   unsigned short roi_y_br;
   unsigned short transit_reg_wid;
   unsigned char transit_reg_yval;
   unsigned char transit_reg_c1val;
   unsigned char transit_reg_c2val;
};
/**
     * @struct     config_hdr_t
     * @brief     Header which precedes configuration data
     */
typedef struct vpp_svc_config_hdr_t vpp_svc_config_hdr_t;
struct vpp_svc_config_hdr_t {
   unsigned short vpp_func_id;
   unsigned int process_flags;
   boolean diagnostics_en;
};
/**
     * @struct     params_t
     * @brief     VPP config params
     */
typedef struct vpp_svc_params_t vpp_svc_params_t;
struct vpp_svc_params_t {
   vpp_svc_gen_params_t gen_params;
   vpp_svc_config_hdr_t* header;
   int headerLen;
   unsigned char* user_data;
   int user_dataLen;
};
////// VPP Operating Context ////////////////////////
/**
     * @struct     context_t
     * @brief     VPP operating context
     */
typedef struct vpp_svc_context_t vpp_svc_context_t;
struct vpp_svc_context_t {
   unsigned char* user_data;
   int user_dataLen;
};
/**
    * @enum     cfg_type_t
    * @brief    Determine configuration parameters being addressed
    */
enum vpp_svc_cfg_type_t {
   CFG_TYPE_STATSLOGS,
   CFG_TYPE_MCPSBW,
   CFG_TYPE_USECASE,
   CFG_TYPE_NUM,
   _32BIT_PLACEHOLDER_vpp_svc_cfg_type_t = 0x7fffffff
};
typedef enum vpp_svc_cfg_type_t vpp_svc_cfg_type_t;
//////////////////////////////
////// Stats/logs management ////////////////////////
/**
     * @struct     statlogs_cfg_t
     * @brief
     */
typedef struct vpp_svc_statlogs_cfg_t vpp_svc_statlogs_cfg_t;
struct vpp_svc_statlogs_cfg_t {
   unsigned int log_flag;
   unsigned int stats_flag;
   unsigned int stats_period;
};
/**
    * @enum     dcvs_mode_t
    * @brief    DCVS configuration
    */
enum vpp_svc_dcvs_mode_t {
   DCVS_OFF,
   DCVS_ADJUST_UP_DOWN,
   DCVS_ADJUST_ONLY_UP,
   _32BIT_PLACEHOLDER_vpp_svc_dcvs_mode_t = 0x7fffffff
};
typedef enum vpp_svc_dcvs_mode_t vpp_svc_dcvs_mode_t;
////// Clock/bus management ////////////////////////
/**
     * @struct     q6_mcpsbw_attr_t
     * @brief     Q6 frequency, bus attributes
     */
typedef struct vpp_svc_q6_mcpsbw_attr_t vpp_svc_q6_mcpsbw_attr_t;
struct vpp_svc_q6_mcpsbw_attr_t {
   unsigned int q6_poweron;
   unsigned int dsp_mcps_per_thread;
   unsigned int peak_bus_bandwidth_mbps;
   unsigned int bus_usage_percent;
   unsigned int hvx_poweron;
   vpp_svc_dcvs_mode_t dcvs_mode;
};
////// Use-case definition for automatic block/bus management ////////////////////////
/**
     * @struct     q6_usecase_attr_t
     * @brief     Use-case description
     */
typedef struct vpp_svc_q6_usecase_attr_t vpp_svc_q6_usecase_attr_t;
struct vpp_svc_q6_usecase_attr_t {
   unsigned int auto_adjust_en;
   unsigned int feature_flags;
   unsigned short frame_width;
   unsigned short frame_height;
};
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
__QAIC_HEADER_EXPORT AEEResult __QAIC_HEADER(vpp_svc_get_buildts)(unsigned char* ts_string, int ts_stringLen) __QAIC_HEADER_ATTRIBUTE;
__QAIC_HEADER_EXPORT AEEResult __QAIC_HEADER(vpp_svc_get_buildts_id)(unsigned int vpp_func_id, unsigned char* ts_string, int ts_stringLen) __QAIC_HEADER_ATTRIBUTE;
__QAIC_HEADER_EXPORT AEEResult __QAIC_HEADER(vpp_svc_is_func_present)(unsigned short vpp_func_id) __QAIC_HEADER_ATTRIBUTE;
__QAIC_HEADER_EXPORT unsigned int __QAIC_HEADER(vpp_svc_get_func_id)(const unsigned char* module_name, int module_nameLen) __QAIC_HEADER_ATTRIBUTE;
__QAIC_HEADER_EXPORT AEEResult __QAIC_HEADER(vpp_svc_set_ctx_size)(const vpp_svc_context_t* context, unsigned int size) __QAIC_HEADER_ATTRIBUTE;
__QAIC_HEADER_EXPORT AEEResult __QAIC_HEADER(vpp_svc_set_diag_ctx_size)(const vpp_svc_context_t* context, unsigned int size) __QAIC_HEADER_ATTRIBUTE;
__QAIC_HEADER_EXPORT AEEResult __QAIC_HEADER(vpp_svc_set_ctx_processing_flag)(const vpp_svc_context_t* context, unsigned int flag) __QAIC_HEADER_ATTRIBUTE;
__QAIC_HEADER_EXPORT AEEResult __QAIC_HEADER(vpp_svc_prepare_ctx)(unsigned char* context_data, int context_dataLen, unsigned int frame_size_bytes, unsigned int flag, unsigned int size, unsigned int diag_size) __QAIC_HEADER_ATTRIBUTE;
__QAIC_HEADER_EXPORT int __QAIC_HEADER(vpp_svc_get_ctx_size)(unsigned int processing_flags, unsigned int frame_size_bytes) __QAIC_HEADER_ATTRIBUTE;
__QAIC_HEADER_EXPORT int __QAIC_HEADER(vpp_svc_get_diag_ctx_size)(unsigned int processing_flags, unsigned int frame_hist) __QAIC_HEADER_ATTRIBUTE;
__QAIC_HEADER_EXPORT unsigned int __QAIC_HEADER(vpp_svc_get_pd_stat)(void) __QAIC_HEADER_ATTRIBUTE;
__QAIC_HEADER_EXPORT AEEResult __QAIC_HEADER(vpp_svc_migrate_pd)(int heap_id) __QAIC_HEADER_ATTRIBUTE;
__QAIC_HEADER_EXPORT unsigned int __QAIC_HEADER(vpp_svc_validate_session)(const vpp_svc_session_info_t* sess_info, const vpp_svc_params_t* params) __QAIC_HEADER_ATTRIBUTE;
__QAIC_HEADER_EXPORT AEEResult __QAIC_HEADER(vpp_svc_init)(const vpp_svc_cap_resource_list_t* capability_resources) __QAIC_HEADER_ATTRIBUTE;
__QAIC_HEADER_EXPORT AEEResult __QAIC_HEADER(vpp_svc_deinit)(const vpp_svc_cap_resource_list_t* capability_resources) __QAIC_HEADER_ATTRIBUTE;
__QAIC_HEADER_EXPORT AEEResult __QAIC_HEADER(vpp_svc_process)(const vpp_svc_params_t* params, unsigned char* context_data, int context_dataLen, const vpp_svc_frame_group_descriptor_t* bufferdata_in, vpp_svc_frame_group_descriptor_t* bufferdata_out) __QAIC_HEADER_ATTRIBUTE;
__QAIC_HEADER_EXPORT AEEResult __QAIC_HEADER(vpp_svc_process_outraw)(const vpp_svc_params_t* params, unsigned char* context_data, int context_dataLen, const vpp_svc_frame_group_descriptor_t* bufferdata_in, vpp_svc_raw_buffer_t* pixel_data, int pixel_dataLen) __QAIC_HEADER_ATTRIBUTE;
__QAIC_HEADER_EXPORT AEEResult __QAIC_HEADER(vpp_svc_process_mvp_ex)(const vpp_svc_params_t* params, unsigned char* context_data, int context_dataLen, vpp_svc_frame_group_hdr_t* frame_hdr, unsigned char* infld0, int infld0Len, unsigned char* infld1, int infld1Len, unsigned char* infld2, int infld2Len, unsigned char* infld3, int infld3Len, unsigned char* outfr0, int outfr0Len, unsigned char* outfr1, int outfr1Len) __QAIC_HEADER_ATTRIBUTE;
__QAIC_HEADER_EXPORT AEEResult __QAIC_HEADER(vpp_svc_process_nr_ex)(const vpp_svc_params_t* params, unsigned char* context_data, int context_dataLen, vpp_svc_frame_group_hdr_t* frame_hdr, unsigned char* infr0, int infr0Len, unsigned char* outfr0, int outfr0Len) __QAIC_HEADER_ATTRIBUTE;
__QAIC_HEADER_EXPORT AEEResult __QAIC_HEADER(vpp_svc_process_aie_ex)(const vpp_svc_params_t* params, unsigned char* context_data, int context_dataLen, vpp_svc_frame_group_hdr_t* frame_hdr, unsigned char* infr0, int infr0Len, unsigned char* outfr0, int outfr0Len) __QAIC_HEADER_ATTRIBUTE;
__QAIC_HEADER_EXPORT AEEResult __QAIC_HEADER(vpp_svc_set_config)(unsigned int type, const unsigned char* cfg_params, int cfg_paramsLen) __QAIC_HEADER_ATTRIBUTE;
#ifdef __cplusplus
}
#endif
#endif //_VPP_SVC_H
