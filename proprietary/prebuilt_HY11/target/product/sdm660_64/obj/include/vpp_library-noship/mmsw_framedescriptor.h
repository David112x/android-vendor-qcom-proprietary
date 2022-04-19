/*========================================================================*//**
@file
    mmsw_framedescriptor.h

@par
    Frame descriptor for multi-media software architecture (MMSW)

@par
    Copyright (c) 2015 Qualcomm Technologies, Inc.  All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
*//*=========================================================================*/

#ifndef MMSW_FRAMEDESCRIPTOR_H
#define MMSW_FRAMEDESCRIPTOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

/*========================================================================
 Data Types
 ========================================================================*/

/** @enum  mmsw_frame_format_t
 *  @brief Enumeration describing the underlying data of the frame
 */
typedef enum mmsw_frame_format
{
    mmsw_frame_format_nv21adreno = 1,  /**< frame format is nv21 adreno, y plane followed by subsampled uv plane in v,u order */
    mmsw_frame_format_xrgb8888   = 2,  /**< frame format is xrgb, 8 bits to each component */
    mmsw_frame_format_xhsv8888   = 3,  /**< frame format is xhsv, 8 bits to each component */
    mmsw_frame_format_h8         = 4,  /**< frame format is h8, hue, 8 bits */
    mmsw_frame_format_y8         = 5,  /**< frame format is y8, 8 bits for each luma pixel */
    mmsw_frame_format_uv88       = 6,  /**< frame format is uv88, 8 bits for u, 8 bits for v */
    mmsw_frame_format_nv12adreno = 7,  /**< frame format is nv12 adreno, y plane followed by subsampled uv plane in u,v order */
} mmsw_frame_format_t;

/** @struct mmsw_frame_desc_t
 *  @brief  Structure containing the descriptor of the underlying frame
 */
typedef struct _mmsw_frame_desc_t
{
    mmsw_frame_format_t format;      /**< Required. Format of frame @see mmsw_frame_format_t */
    unsigned int width;              /**< Required. Full width of this entire frame in pixels */
    unsigned int height;             /**< Required. Full height of this frame in pixels */
    unsigned int stride;             /**< Required. Stride in number of bytes -not pixels- */
    int fd;                          /**< Required. File descriptor - applicable in linux */
    void *baseAddress;               /**< Required. Base address of memory block - applicable in linux */
    size_t yOffset;                  /**< Required. This is the offset to the Y plane relative to the baseAddress, in bytes */
    size_t uvOffset;                 /**< Required. This is the offset to the UV plane relative to the yOffset, in bytes */
    void* userData;                  /**< Arbitrary user data, a frame handler can use this for keeping state */
    size_t baseSize;                 /**< Required. The size of the base input frame, e.g. current frame could be a subset of the base frame */
} mmsw_frame_desc_t;

#ifdef __cplusplus
}
#endif

#endif // MMSW_FRAMEDESCRIPTOR_H
