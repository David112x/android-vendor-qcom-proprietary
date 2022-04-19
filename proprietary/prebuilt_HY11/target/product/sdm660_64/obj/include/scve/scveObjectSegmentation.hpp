#ifndef _SCVE_OBJECT_SEGMENTATION_H_
#define _SCVE_OBJECT_SEGMENTATION_H_

/**=============================================================================

@file
scveObjectSegmentation.hpp

@brief
Header file for Object Extraction.

Copyright (c) 2013-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

=============================================================================**/

#include "scveTypes.hpp"
#include "scveContext.hpp"

namespace SCVE
{

typedef enum
{
    SCVE_OBJECT_SEGMENTATION_UNKNOWN = 0, // initial value of the stroke
    SCVE_OBJECT_SEGMENTATION_BACKGROUND = 1, // background label
    SCVE_OBJECT_SEGMENTATION_FOREGROUND = 2 // foreground label
} ObjectSegmentationType;

typedef enum
{
    SCVE_SCRIBBLE_SELECTION = 0, // output mask when both foreground and background strokes are provided
    SCVE_QUICK_SELECTION = 1, // output mask when foreground is provided
} ObjectSegmentationMethod;

typedef void (*ObjectSegmentationCB)( Status status,
                                    void* pSessionUserData,
                                    void* pTaskUserData);

class SCVE_API ObjectSegmentation
{
public:
   // @brief
   // instantiate ObjectSegmentation instance
   // @param cbFunc
   //     callback function
   // @param pSessionUserData
   //     anything user want to preverse
   static ObjectSegmentation* newInstance(Context *pScveContext,
                                        ObjectSegmentationCB cbFunc = NULL,
                                        void *pSessionUserData = NULL);

   // @brief
   // delete ObjectSegmentation instance
   static Status deleteInstance(ObjectSegmentation *pInstance);

   // @brief
   // Method of sync call.
   // @param pSrc
   //     Input image. 4-channel rgba image
   // @param method
   //     specify segmentation method, default is SCVE_SCRIBBLE_SELECTION
   virtual Status init(const Image *pSrc,
       ObjectSegmentationMethod method = SCVE_SCRIBBLE_SELECTION) = 0;

   // @brief
   // Method of sync call.
   // @param pStroke
   //     Input stroke image. 1-channel image where each pixel indicates
   //     the labelling by user according to ObjectSegmentationStrokeType
   // @param pMask
   //     Output mask image. 1-channel image where each pixel indicates
   //     the labelling by user according to ObjectSegmentationStrokeType
   // @param pPrevMask
   //     previous mask, used as the start point of current operation
   virtual Status Run_Sync( const Image *pStroke,
                                 Image *pMask,
                            const Image *pPrevMask = NULL) = 0;

   // @brief
   // Method of async call.
   // @param pStroke
   //     Input stroke image. 1-channel image where each pixel indicates
   //     the labelling by user according to ObjectSegmentationStrokeType
   // @param pMask
   //     Output mask image. 1-channel image where each pixel indicates
   //     the labelling by user according to ObjectSegmentationStrokeType
   // @param pTaskUserData
   //     User data when the async call is invoked
   // @param pPrevMask
   //     previous mask, used as the start point of current operation
   virtual Status Run_Async(const Image *pStroke,
                                  Image *pMask,
                            void* pTaskUserData = NULL,
                           const Image *pPrevMask = NULL) = 0;

protected:
   // @brief
   // destructor
   virtual ~ObjectSegmentation();
};

}

#endif //_SCVE_IMAGE_OBJECT_SEGMENTATION_H_

