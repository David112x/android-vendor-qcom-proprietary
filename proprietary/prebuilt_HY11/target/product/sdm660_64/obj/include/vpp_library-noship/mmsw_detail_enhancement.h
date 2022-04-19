/*========================================================================*//**
@file
    mmsw_detail_enhancement.h

@par
    Scalar video processing

@par Externalized Functions:
    See below.

@par
    Copyright (c) 2015 Qualcomm Technologies, Inc.  All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
*//*=========================================================================*/

#ifndef MMSW_DETAIL_ENHANCEMENT_H
#define MMSW_DETAIL_ENHANCEMENT_H

/*========================================================================
 Includes
 ========================================================================*/

#include "mmsw_error.h"
#include "mmsw_framedescriptor.h"

#ifdef __cplusplus
extern "C" {
#endif

/*========================================================================
 Data Types
 ========================================================================*/

/** @typedef mmsw_detail_enhancement_ctx_t
 *  @brief   A de context handle for video processing
 */
typedef void* mmsw_detail_enhancement_ctx_t;

/** @enum mmsw_detail_enhancement_quality_option_t
 *  @brief Quality option for the denoising functionality
 */
typedef enum _mmsw_detail_enhancement_quality_option_t
{
    mmsw_detail_enhancement_quality_normal    = 0, /**< Quality level - basic de */
    mmsw_detail_enhancement_quality_luma_only = 1, /**< Quality level - basic de, only luma */
    mmsw_detail_enhancement_quality_cade      = 2, /**< Quality level - CADE */
    mmsw_detail_enhancement_quality_cade_hue  = 3, /**< Quality level - CADE with HUE adaptive */
} mmsw_detail_enhancement_quality_option_t;

/** @enum mmsw_detail_enhancement_param_t
 *  @brief  Context specific parameters for de video processing
 */
typedef enum _mmsw_detail_enhancement_param_t
{
    mmsw_detail_enhancement_param_quality                = 0, /**< Quality level */
    mmsw_detail_enhancement_param_de_level               = 1, /**< Detail enhancement level - float value, from -1.0 to 4.0 */
    mmsw_detail_enhancement_param_luma_csc_level         = 2, /**< Contrast/saturation luma value - float value 0.0 to 2.0 */
    mmsw_detail_enhancement_param_chroma_csc_level       = 3, /**< Contrast/saturation chroma value - float value 0.0 to 2.0 */
    mmsw_detail_enhancement_param_process_percent_left   = 4, /**< Process width percentage left - float value 0.0 to 100.0, default is 0.0 */
    mmsw_detail_enhancement_param_process_percent_right  = 5, /**< Process width percentage right - float value 0.0 to 100.0, default is 100.0 */
    mmsw_detail_enhancement_param_process_percent_top    = 6, /**< Process height percentage top - float value 0.0 to 100.0, default is 0.0 */
    mmsw_detail_enhancement_param_process_percent_bottom = 7, /**< Process height percentage bottom - float value 0.0 to 100.0, default is 100.0 */
    mmsw_detail_enhancement_param_cache_frames           = 8, /**< Cache frames - int value 1 or 0 for true or false, default is 1 */
} mmsw_detail_enhancement_param_t;

/** @enum mmsw_detail_enhancement_param_t
 *  @brief  Context specific parameters for de video processing
 */
typedef enum _mmsw_detail_enhancement_program_type_t
{
    mmsw_detail_enhancement_program_binary = 0, /**< Program type is binary data */
    mmsw_detail_enhancement_program_source = 1, /**< Program type is source */
} mmsw_detail_enhancement_program_type_t;

typedef enum _mmsw_detail_enhancement_session_flags_t
{
    mmsw_detail_enhancement_session_flag_secure = (1 << 0), /**< Denotes that this is a protected session */
} mmsw_detail_enhancement_session_flags_t;

/*========================================================================
 Entry points
 ========================================================================*/

/*========================================================================*/
/**
 * Initializes the detail enhancement video processing module
 * @param[out] detail_enhancement_ctx      The de context that is created
 * @param[in]  flags            Bitfield of flags from @see
 *                              mmsw_detail_enhancement_session_flags_t.
 * @param[in]  qualiy_option    The quality option
 * @param[in]  program_type     Determines the format of the program parameter.
 * @param[in]  program          Array of size program_length, filled with the algo.
 *                              The format of data in program must correspond to
 *                              program_type.
 * @param[in]  program_length   Length of program.
 * @return                      The return value
 * @see mmsw_detail_enhancement_ctx_t @see mmsw_detail_enhancement_params_t @see mmsw_error_t
*/
/*========================================================================*/
mmsw_error_t mmsw_detail_enhancement_init(mmsw_detail_enhancement_ctx_t *detail_enhancement_ctx,
                                          uint32_t flags,
                                          mmsw_detail_enhancement_quality_option_t qualiy_option,
                                          mmsw_detail_enhancement_program_type_t program_type,
                                          char* program,
                                          size_t program_length);

/*========================================================================*/
/**
 * Deinitializes the de video processing module for the given context
 * @param[in] detail_enhancement_ctx  The de context to deinitialize
 * @return                The return value
 * @see mmsw_detail_enhancement_ctx_t @see mmsw_error_t
*/
/*========================================================================*/
mmsw_error_t mmsw_detail_enhancement_deinit(mmsw_detail_enhancement_ctx_t detail_enhancement_ctx);

/*========================================================================*/
/**
 * Open the de video processing module for post processing
 * @param[in] detail_enhancement_ctx  The de context to deinitialize
 * @return                The return value
 * @see mmsw_detail_enhancement_ctx_t @see mmsw_error_t
*/
/*========================================================================*/
mmsw_error_t mmsw_detail_enhancement_open(mmsw_detail_enhancement_ctx_t detail_enhancement_ctx);

/*========================================================================*/
/**
 * Open the de video processing module for post processing
 * @param[in] detail_enhancement_ctx  The de context to deinitialize
 * @return                The return value
 * @see mmsw_detail_enhancement_ctx_t @see mmsw_error_t
*/
/*========================================================================*/
mmsw_error_t mmsw_detail_enhancement_close(mmsw_detail_enhancement_ctx_t detail_enhancement_ctx);

/*========================================================================*/
/**
 * Updates the de processing context with the new parameter
 * @param[in] detail_enhancement_ctx  The de context to update
 * @param[in] parameter   Which parameter to modify
 * @param[in] value       The value of the parameter, specific to the parameter, must be casted as void*
 * @return                The return value
 * @see mmsw_detail_enhancement_ctx_t @see mmsw_detail_enhancement_param_t @see mmsw_error_t
*/
/*========================================================================*/
mmsw_error_t mmsw_detail_enhancement_set_param(mmsw_detail_enhancement_ctx_t detail_enhancement_ctx,
                                               mmsw_detail_enhancement_param_t parameter,
                                               void* value);

/*========================================================================*/
/**
 * Perform a detail enhancement operation
 * @param[in] detail_enhancement_ctx  The de context
 * @param[in] input_frame  The input frame, input format must be mmsw_frame_format_nv12adreno
 * @param[in] output_frame The output frame, output format must be mmsw_frame_format_nv12adreno
 * @return                 The return value
 * @see mmsw_detail_enhancement_ctx_t @see mmsw_frame_desc_t @see mmsw_error_t
*/
/*========================================================================*/
mmsw_error_t mmsw_detail_enhancement(mmsw_detail_enhancement_ctx_t detail_enhancement_ctx,
                                     mmsw_frame_desc_t* input_frame,
                                     mmsw_frame_desc_t* output_frame);


#ifdef __cplusplus
}
#endif

#endif // MMSW_DETAIL_ENHANCEMENT_H
