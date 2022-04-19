/***************************************************************************
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
***************************************************************************/
#include <cutils/properties.h>
#include <linux/media.h>
#include "mct_module.h"
#include "module_faceproc.h"
#include "module_faceproc_newstab.h"

static const uint32_t WithinThresholdRatio = 10;  ///< Ratio to divide with while calculating thresholds for WithinThreshold mode
static const float  ScalingFactor          = 0.7f; ///< ROI size percentage for comparison with X-coord when face is at boundaries


int module_faceproc_newstab_setConfig(fd_chromatix_t *p_fd_chromatix, StabilizationConfig* pStabilizationConfig)
{
  pStabilizationConfig->historyDepth = 10;

    // FD Tuning config for FD ROI center position
    pStabilizationConfig->attributeConfigs[ObjectPositionIndex].enable                            =
         p_fd_chromatix->stab_pos.enable;
    pStabilizationConfig->attributeConfigs[ObjectPositionIndex].mode                              =
        p_fd_chromatix->stab_pos.mode;
    pStabilizationConfig->attributeConfigs[ObjectPositionIndex].threshold                         =
        p_fd_chromatix->stab_pos.threshold;
    pStabilizationConfig->attributeConfigs[ObjectPositionIndex].stateCount                        =
        p_fd_chromatix->stab_pos.state_cnt ;
    pStabilizationConfig->attributeConfigs[ObjectPositionIndex].stableThreshold                   =
        p_fd_chromatix->stab_pos.stable_threshold ;
    pStabilizationConfig->attributeConfigs[ObjectPositionIndex].minStableState                    =
        p_fd_chromatix->stab_pos.min_count_for_stable_state ;
    pStabilizationConfig->attributeConfigs[ObjectPositionIndex].filterType                        =
        p_fd_chromatix->stab_pos.filter_type;
    pStabilizationConfig->attributeConfigs[ObjectPositionIndex].averageFilter.historyLength       =
        p_fd_chromatix->stab_pos.average.history_length ;
    pStabilizationConfig->attributeConfigs[ObjectPositionIndex].useReference                      =
        p_fd_chromatix->stab_pos.use_reference;
    pStabilizationConfig->attributeConfigs[ObjectPositionIndex].movingThreshold                   =
        p_fd_chromatix->stab_pos.movingThreshold;
    pStabilizationConfig->attributeConfigs[ObjectPositionIndex].movingInitStateCount              =
        p_fd_chromatix->stab_pos.movingInitStateCount;
    pStabilizationConfig->attributeConfigs[ObjectPositionIndex].movingLinkFactor                  =
        p_fd_chromatix->stab_pos.movingLinkFactor;
    pStabilizationConfig->attributeConfigs[ObjectPositionIndex].averageFilter.movingHistoryLength =
        p_fd_chromatix->stab_pos.average.moving_history_length;

    // FD Tuning config for FD ROI size
    pStabilizationConfig->attributeConfigs[ObjectSizeIndex].enable                            =
        p_fd_chromatix->stab_size.enable;
    pStabilizationConfig->attributeConfigs[ObjectSizeIndex].mode                              =
        p_fd_chromatix->stab_size.mode;
    pStabilizationConfig->attributeConfigs[ObjectSizeIndex].threshold                         =
        p_fd_chromatix->stab_size.threshold;
    pStabilizationConfig->attributeConfigs[ObjectSizeIndex].stateCount                        =
        p_fd_chromatix->stab_size.state_cnt;
    pStabilizationConfig->attributeConfigs[ObjectSizeIndex].stableThreshold                   =
        p_fd_chromatix->stab_size.stable_threshold;
    pStabilizationConfig->attributeConfigs[ObjectSizeIndex].minStableState                    =
        p_fd_chromatix->stab_size.min_count_for_stable_state;
    pStabilizationConfig->attributeConfigs[ObjectSizeIndex].filterType                        =
        p_fd_chromatix->stab_size.filter_type;
    pStabilizationConfig->attributeConfigs[ObjectSizeIndex].averageFilter.historyLength       =
        p_fd_chromatix->stab_size.average.history_length;
    pStabilizationConfig->attributeConfigs[ObjectSizeIndex].useReference                      =
        p_fd_chromatix->stab_size.use_reference;
    pStabilizationConfig->attributeConfigs[ObjectSizeIndex].movingThreshold                   =
        p_fd_chromatix->stab_size.movingThreshold;
    pStabilizationConfig->attributeConfigs[ObjectSizeIndex].movingInitStateCount              =
        p_fd_chromatix->stab_size.movingInitStateCount;
    pStabilizationConfig->attributeConfigs[ObjectSizeIndex].movingLinkFactor                  =
        p_fd_chromatix->stab_size.movingLinkFactor;
    pStabilizationConfig->attributeConfigs[ObjectSizeIndex].averageFilter.movingHistoryLength =
        p_fd_chromatix->stab_size.average.moving_history_length;

    IDBG_MED("Position(%d %d %d %d %d %d %d %d %d), Size(%d %d %d %d %d %d %d %d %d)",
                     pStabilizationConfig->attributeConfigs[ObjectPositionIndex].enable,
                     pStabilizationConfig->attributeConfigs[ObjectPositionIndex].mode,
                     pStabilizationConfig->attributeConfigs[ObjectPositionIndex].threshold,
                     pStabilizationConfig->attributeConfigs[ObjectPositionIndex].stateCount,
                     pStabilizationConfig->attributeConfigs[ObjectPositionIndex].stableThreshold,
                     pStabilizationConfig->attributeConfigs[ObjectPositionIndex].minStableState,
                     pStabilizationConfig->attributeConfigs[ObjectPositionIndex].filterType,
                     pStabilizationConfig->attributeConfigs[ObjectPositionIndex].averageFilter.historyLength,
                     pStabilizationConfig->attributeConfigs[ObjectPositionIndex].useReference,
                     pStabilizationConfig->attributeConfigs[ObjectSizeIndex].enable,
                     pStabilizationConfig->attributeConfigs[ObjectSizeIndex].mode,
                     pStabilizationConfig->attributeConfigs[ObjectSizeIndex].threshold,
                     pStabilizationConfig->attributeConfigs[ObjectSizeIndex].stateCount,
                     pStabilizationConfig->attributeConfigs[ObjectSizeIndex].stableThreshold,
                     pStabilizationConfig->attributeConfigs[ObjectSizeIndex].minStableState,
                     pStabilizationConfig->attributeConfigs[ObjectSizeIndex].filterType,
                     pStabilizationConfig->attributeConfigs[ObjectSizeIndex].averageFilter.historyLength,
                     pStabilizationConfig->attributeConfigs[ObjectSizeIndex].useReference);
    m_configuration = *pStabilizationConfig;
return IMG_SUCCESS;
}

int ConvertToStabilizationData(
    faceproc_result_t*         pResults,
    StabilizationData* pStabilizationData)
{
    int result = IMG_SUCCESS;
    uint32_t     index  = 0;

   if (StabilizationMaxObjects < pResults->num_faces_detected)
   {
       IDBG_MED("Unsupported number of faces %d (%d)",
                pResults->num_faces_detected, StabilizationMaxObjects);
       result = IMG_ERR_NOT_SUPPORTED;
   }

    if (IMG_SUCCESS == result)
    {
        for (uint32_t i = 0; i < pResults->num_faces_detected; i++)
        {
            int32_t               eyesDistance     = 0;
            // StabilizationEntry  leftEyeCenter    = { 0 };
            // StabilizationEntry  rightEyeCenter   = { 0 };
            StabilizationEntry  averageEyeCenter = { 0 };
            // int32_t               deltaEyesX       = 0;
            // int32_t               deltaEyesY       = 0;
            bool                validReference   = FALSE;

            pStabilizationData->objectData[i].id = pResults->roi[i].unique_id;

            // Calculate the reference positions for the facial parts
            /* if (TRUE == pResults->roi[i].facialParts.valid)
            {
                leftEyeCenter.data0    = pResults->faceInfo[i].facialParts.facialPoint[FDFacialPointLeftEye].position.x;
                leftEyeCenter.data1    = pResults->faceInfo[i].facialParts.facialPoint[FDFacialPointLeftEye].position.y;
                rightEyeCenter.data0   = pResults->faceInfo[i].facialParts.facialPoint[FDFacialPointRightEye].position.x;
                rightEyeCenter.data1   = pResults->faceInfo[i].facialParts.facialPoint[FDFacialPointRightEye].position.y;

                averageEyeCenter.data0 = ((leftEyeCenter.data0 + rightEyeCenter.data0) / 2);
                averageEyeCenter.data1 = ((leftEyeCenter.data1 + rightEyeCenter.data1) / 2);

                deltaEyesX             = (leftEyeCenter.data0 - rightEyeCenter.data0);
                deltaEyesY             = (leftEyeCenter.data1 - rightEyeCenter.data1);
                eyesDistance           = static_cast<INT32>(sqrt((deltaEyesX * deltaEyesX) + (deltaEyesY * deltaEyesY)));

                validReference         = TRUE;
            } */

            // FD will always be trying to produce face center position data, this index is reserved for center
            index = ObjectPositionIndex;

            pStabilizationData->objectData[i].pFaceInfo = &pResults->roi[i];

            // This is always valid, otherwise we would not get this as a detected face
            pStabilizationData->objectData[i].attributeData[index].entry.data0      = pResults->roi[i].face_boundary.x;
            pStabilizationData->objectData[i].attributeData[index].entry.data1      = pResults->roi[i].face_boundary.y;
            pStabilizationData->objectData[i].attributeData[index].valid            = TRUE;
            pStabilizationData->objectData[i].attributeReference[index].entry       = averageEyeCenter;
            pStabilizationData->objectData[i].attributeReference[index].valid       = validReference;
            pStabilizationData->objectData[i].numAttributes++;

            // FD will always be trying to produce faceROI data, this index is reserved for FaceROI size
            index = ObjectSizeIndex;

            // This is always valid, otherwise we would not get this as a detected face
            pStabilizationData->objectData[i].attributeData[index].entry.data0      = pResults->roi[i].face_boundary.dx;
            pStabilizationData->objectData[i].attributeData[index].entry.data1      = pResults->roi[i].face_boundary.dy;
            pStabilizationData->objectData[i].attributeData[index].valid            = TRUE;
            pStabilizationData->objectData[i].attributeReference[index].entry.data0 = eyesDistance;
            pStabilizationData->objectData[i].attributeReference[index].entry.data1 = eyesDistance;
            pStabilizationData->objectData[i].attributeReference[index].valid       = validReference;
            pStabilizationData->objectData[i].numAttributes++;

            // Below indexes can be used for other stabilization attributes for the face
            // Use the below index value to store attribute to the correct index, then increment index
            index = ObjectUnreservedIndex;
        }

        pStabilizationData->numObjects = pResults->num_faces_detected;

        IDBG_MED("numObjects = %d", pStabilizationData->numObjects);
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// SortObjectPosition
///
/// @brief  Comparison function that will be used for sorting objects
///
/// @param  pArg0 Pointer to object to compare for sorting
/// @param  pArg1 Pointer to object to compare for sorting
///
/// @return 1 if pArg0 is greater than pArg1, -1 otherwise
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int SortObjectPosition(
    const void* pArg0,
    const void* pArg1)
{

    const StabilizationHolder*  pFirst      = (const StabilizationHolder*)(pArg0);
    const StabilizationHolder*  pSecond     = (const StabilizationHolder*)(pArg1);
    int                         comparison  = 0;
    uint32_t                      size        = 0;
    uint32_t                      diff        = 0;

    size = (pFirst->attributeData[ObjectSizeIndex].entry.data0 / 2) +
           (pSecond->attributeData[ObjectSizeIndex].entry.data0 / 2);
    diff = abs((pFirst->attributeData[ObjectPositionIndex].entry.data1) -
                                (pSecond->attributeData[ObjectPositionIndex].entry.data1));

    if (diff < size)
    {
        if ((pFirst->attributeData[ObjectPositionIndex].entry.data0) >
            (pSecond->attributeData[ObjectPositionIndex].entry.data0))
        {
            comparison = 1;
        }
        else
        {
            comparison = -1;
        }
    }
    else
    {
        if ((pFirst->attributeData[ObjectPositionIndex].entry.data1     >
             pSecond->attributeData[ObjectPositionIndex].entry.data1)   ||
            ((pFirst->attributeData[ObjectPositionIndex].entry.data1    ==
              pSecond->attributeData[ObjectPositionIndex].entry.data1)  &&
             (pFirst->attributeData[ObjectPositionIndex].entry.data0    >
              pSecond->attributeData[ObjectPositionIndex].entry.data0)))
        {
            comparison = 1;
        }
        else
        {
            comparison = -1;
        }
    }

    return comparison;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Stabilization::InitializeObjectEntry
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void InitializeObjectEntry(
    StabilizationObject* pHistoryEntry,
    StabilizationHolder* pNewObjectEntry)
{

    uint32_t index = 0;

    // Clear out the current location in history
    memset(pHistoryEntry, 0x0, sizeof(*pHistoryEntry));

    pHistoryEntry->numAttributes = pNewObjectEntry->numAttributes;

    for (uint32_t i = 0; i < pNewObjectEntry->numAttributes; i++)
    {
        if (TRUE == m_configuration.attributeConfigs[i].enable)
        {
            pHistoryEntry->objectAttributes[i].maxStateCount = m_configuration.attributeConfigs[i].stateCount;
            pHistoryEntry->objectAttributes[i].state         = StabilizingState;
        }
    }

    AddObjectEntry(pHistoryEntry, pNewObjectEntry, &m_configuration);

    for (uint32_t i = 0; i < pNewObjectEntry->numAttributes; i++)
    {
        if (TRUE == m_configuration.attributeConfigs[i].enable)
        {
            index                                          = pHistoryEntry->objectAttributes[i].index;
            pHistoryEntry->objectAttributes[i].stableEntry = pHistoryEntry->objectAttributes[i].entryHistory[index];
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Stabilization::AddObjectEntry
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AddObjectEntry(
    StabilizationObject* pHistoryEntry,
    StabilizationHolder* pNewObjectEntry,
    StabilizationConfig* pConfig)
{

    uint32_t index = 0;

    pHistoryEntry->id = pNewObjectEntry->id;

    for (uint32_t i = 0; i < pNewObjectEntry->numAttributes; i++)
    {
        if (TRUE == pConfig->attributeConfigs[i].enable)
        {
            index = (pHistoryEntry->objectAttributes[i].index + 1) % (pConfig->historyDepth);

            pHistoryEntry->objectAttributes[i].index               = index;
            pHistoryEntry->objectAttributes[i].entryHistory[index] = pNewObjectEntry->attributeData[i].entry;
            pHistoryEntry->objectAttributes[i].historySize = pConfig->historyDepth;
            if (pHistoryEntry->objectAttributes[i].numEntries < pConfig->historyDepth)
            {
                pHistoryEntry->objectAttributes[i].numEntries++;
            }
        }
    }
}
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Stabilization::CheckObject
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
StabilizationPosition CheckObject(
    StabilizationObject* pHistoryEntry,
    StabilizationHolder* pCurrentObject)
{

    StabilizationPosition result         = ObjectSame;
    uint32_t                thresholdX     = 0;
    uint32_t                thresholdY     = 0;
    uint32_t                centerCurrentX = 0;
    uint32_t                centerCurrentY = 0;
    uint32_t                centerHistoryX = 0;
    uint32_t                centerHistoryY = 0;
    uint32_t                deltaX         = 0;
    uint32_t                deltaY         = 0;

    thresholdX = (pHistoryEntry->objectAttributes[ObjectSizeIndex].stableEntry.data0*
        m_configuration.attributeConfigs[ObjectSizeIndex].threshold) / 1000;
    thresholdY = (pHistoryEntry->objectAttributes[ObjectSizeIndex].stableEntry.data1*
        m_configuration.attributeConfigs[ObjectSizeIndex].threshold) / 1000;

    centerHistoryX = pHistoryEntry->objectAttributes[ObjectPositionIndex].stableEntry.data0;
    centerHistoryY = pHistoryEntry->objectAttributes[ObjectPositionIndex].stableEntry.data1;

    centerCurrentX = pCurrentObject->attributeData[ObjectPositionIndex].entry.data0;
    centerCurrentY = pCurrentObject->attributeData[ObjectPositionIndex].entry.data1;

    deltaX         = abs((int32_t)(centerHistoryX) - (int32_t)(centerCurrentX));
    deltaY         = abs((int32_t)(centerHistoryY) - (int32_t)(centerCurrentY));

    if (((0 == pCurrentObject->id) ||
         (0 == pHistoryEntry->id)) &&
        ((deltaX < thresholdX)     &&
         (deltaY < thresholdY)))
    {
        result = ObjectSame;
    }
    else if ((0                 != pHistoryEntry->id)  &&
             (0                 != pCurrentObject->id) &&
             (pHistoryEntry->id == pCurrentObject->id))
    {
        result = ObjectSame;
    }
    else if ((deltaX < thresholdX) && (deltaY < thresholdY))
    {
        result = ObjectSame;
    }
    else if ((centerHistoryY   > centerCurrentY)   ||
             ((centerHistoryY == centerCurrentY)   &&
              (centerHistoryX  > centerCurrentX)))
    {
        result = ObjectBefore;
    }
    else
    {
        result = ObjectAfter;
    }

    return result;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Stabilization::WithinThreshold
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool WithinThresholdCheck(
    StabilizationEntry* pCurrentEntry,
    StabilizationEntry* pStableEntry,
    uint32_t              threshold0,
    uint32_t              threshold1)
{

    return ((abs(pCurrentEntry->data0 - pStableEntry->data0)) < (int32_t)threshold0) &&
           ((abs(pCurrentEntry->data1 - pStableEntry->data1)) < (int32_t)threshold1);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Stabilization::IsContinuesMode
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool IsContinuesMode(
    fd_face_stab_mode_t mode)
{
    bool continuesMode = FALSE;

    switch (mode)
    {
        case FD_STAB_EQUAL:
        case FD_STAB_SMALLER:
        case FD_STAB_BIGGER:
        case FD_STAB_CLOSER_TO_REFERENCE:
        case FD_STAB_WITHIN_THRESHOLD:
            continuesMode = FALSE;
            break;
        case FD_STAB_CONTINUES_EQUAL:
        case FD_STAB_CONTINUES_SMALLER:
        case FD_STAB_CONTINUES_BIGGER:
        case FD_STAB_CONTINUES_CLOSER_TO_REFERENCE:
            continuesMode = TRUE;
            break;
        default:
            IDBG_MED("Invalid stabilization mode");
            break;
    }

    return continuesMode;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Stabilization::IsStable
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool IsStable(
    StabilizationAttribute*       pCurrentEntry,
    StabilizationEntry*           pReference,
    StabilizationAttributeConfig* pConfig,
    StabilizationAttribute*       pSizeAttribute,
    bool                          bFaceAtBoundary)
{

    bool    stable        = FALSE;
    int32_t   deltaStableX  = 0;
    int32_t   deltaStableY  = 0;
    int32_t   deltaCurrentX = 0;
    int32_t   deltaCurrentY = 0;
    uint32_t  threshold0;
    uint32_t  threshold1;

    switch (pConfig->mode)
    {
        case FD_STAB_CONTINUES_SMALLER:
        case FD_STAB_SMALLER:
            stable = ((pCurrentEntry->stableEntry.data0 < pCurrentEntry->entryHistory[pCurrentEntry->index].data0) &&
                      (pCurrentEntry->stableEntry.data1 < pCurrentEntry->entryHistory[pCurrentEntry->index].data1));
            break;
        case FD_STAB_CONTINUES_BIGGER:
        case FD_STAB_BIGGER:
            stable = ((pCurrentEntry->stableEntry.data0 > pCurrentEntry->entryHistory[pCurrentEntry->index].data0) &&
                      (pCurrentEntry->stableEntry.data1 > pCurrentEntry->entryHistory[pCurrentEntry->index].data1));
            break;
        case FD_STAB_CONTINUES_CLOSER_TO_REFERENCE:
        case FD_STAB_CLOSER_TO_REFERENCE:
            if (NULL != pReference)
            {
                deltaStableX  = pCurrentEntry->stableEntry.data0 - pReference->data0;
                deltaStableY  = pCurrentEntry->stableEntry.data1 - pReference->data1;
                deltaCurrentX = pCurrentEntry->entryHistory[pCurrentEntry->index].data0 - pReference->data0;
                deltaCurrentY = pCurrentEntry->entryHistory[pCurrentEntry->index].data1 - pReference->data1;
                stable = (((deltaStableX * deltaStableX) + (deltaStableY * deltaStableY)) <
                          ((deltaCurrentX * deltaCurrentX) + (deltaCurrentY * deltaCurrentY)));
            }
            else
            {
                stable = TRUE;
            }
            break;
        case FD_STAB_WITHIN_THRESHOLD:
            threshold0 = (uint32_t) (((pConfig->stableThreshold *
                                                (uint32_t)(sqrt(pSizeAttribute->stableEntry.data0))) /
                                               WithinThresholdRatio) + 1);
            threshold1 = (uint32_t) (((pConfig->stableThreshold *
                                                (uint32_t)(sqrt(pSizeAttribute->stableEntry.data1))) /
                                               WithinThresholdRatio) + 1);

            if (TRUE == bFaceAtBoundary)
            {
                threshold0 = threshold0 * 2;
                threshold1 = threshold1 * 2;
            }

            stable = WithinThresholdCheck(&pCurrentEntry->entryHistory[pCurrentEntry->index],
                                     &pCurrentEntry->stableEntry,
                                     threshold0, threshold1);
            break;
        case FD_STAB_CONTINUES_EQUAL:
        default:
            stable = ((pCurrentEntry->stableEntry.data0 == pCurrentEntry->entryHistory[pCurrentEntry->index].data0) &&
                      (pCurrentEntry->stableEntry.data1 == pCurrentEntry->entryHistory[pCurrentEntry->index].data1));
            break;
    }

    return stable;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Stabilization::CheckObjectMovement
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CheckObjectMovement(
    StabilizationEntry* pCurrentEntry,
    StabilizationEntry* pLastEntry,
    StabilizationEntry* pLastLastEntry,
    StabilizationEntry* pStableSizeEntry,
    uint32_t            threshold,
    float               movingLinkFactor,
    uint32_t            lastframewidth)
{

    int32_t frameWidth         = lastframewidth;
    pCurrentEntry->isChanged   = FALSE;

    // Check if face is moving
    float move1 = sqrtf((float)((pLastEntry->data0 - pLastLastEntry->data0)*
        ((pLastEntry->data0 - pLastLastEntry->data0) +
        (pLastEntry->data1 - pLastLastEntry->data1)*(pLastEntry->data1 - pLastLastEntry->data1))));
    float move2 =sqrtf((float)(((pCurrentEntry->data0 - pLastEntry->data0) *
        (pCurrentEntry->data0 - pLastEntry->data0)) +
        ((pCurrentEntry->data1 - pLastEntry->data1) *
        (pCurrentEntry->data1 - pLastEntry->data1))));
    float move3 = sqrtf((float)(((pCurrentEntry->data0 - pLastLastEntry->data0) *
        (pCurrentEntry->data0 - pLastLastEntry->data0)) +
        ((pCurrentEntry->data1 - pLastLastEntry->data1) *
        (pCurrentEntry->data1 - pLastLastEntry->data1))));

    // Check if face is at left or right boundary. ROI fluctuation is small at top or bottom boundary,
    // so boundary check will be skipped on Y-axis.
    if ((TRUE == (pCurrentEntry->data0 < ((int32_t)(pStableSizeEntry->data0 * ScalingFactor)))) ||
        (TRUE == ((frameWidth-pCurrentEntry->data0) < ((int32_t)(pStableSizeEntry->data0 * ScalingFactor)))))
    {
        // Face is at boundary, check with higher threshold
        if ((move3 > (move1 * movingLinkFactor * 2)) && (move1 > (threshold * 2)) && (move2 > (threshold * 2)))
        {
            pCurrentEntry->isChanged = TRUE;
        }
    }
    else
    {
        // Face is not at boundary, check with normal threshold
        if ((move3 > (move1 * movingLinkFactor)) && (move1 > threshold) && (move2 > threshold))
        {
            pCurrentEntry->isChanged = TRUE;
        }
    }

    IDBG_MED("Stabilization state Move1=%f, Move2=%f, Move3=%f, threshold=%d, faceIsMoving=%d",
                     move1, move2, move3, threshold, pCurrentEntry->isChanged);

    return pCurrentEntry->isChanged;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Stabilization::ApplyFilter
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplyFilter(
    StabilizationAttribute*       pAttribute,
    StabilizationAttributeConfig* pConfig,
    bool                          faceIsMoving)
{

    StabilizationEntry* pCurrentEntry = &pAttribute->stableEntry;
    StabilizationEntry* pNewEntry     = &pAttribute->entryHistory[pAttribute->index];

    switch (pConfig->filterType)
    {
        case FD_STAB_HYSTERESIS:
            if (pNewEntry->data0 > pCurrentEntry->data0)
            {
                ApplyHysteresis(&pCurrentEntry->data0, &pNewEntry->data0, pConfig, TRUE);
            }
            else
            {
                ApplyHysteresis(&pCurrentEntry->data0, &pNewEntry->data0, pConfig, FALSE);
            }
            if (pNewEntry->data1 > pCurrentEntry->data1)
            {
                ApplyHysteresis(&pCurrentEntry->data1, &pNewEntry->data1, pConfig, TRUE);
            }
            else
            {
                ApplyHysteresis(&pCurrentEntry->data1, &pNewEntry->data1, pConfig, FALSE);
            }
            break;
        case FD_STAB_TEMPORAL:
            ApplyTemporal(&pCurrentEntry->data0,
                          pConfig->temporalFilter.numerator,
                          &pNewEntry->data0,
                          pConfig->temporalFilter.denominator);
            ApplyTemporal(&pCurrentEntry->data1,
                          pConfig->temporalFilter.numerator,
                          &pNewEntry->data1,
                          pConfig->temporalFilter.denominator);
            break;
        case FD_STAB_AVERAGE:
            ApplyAverage(pAttribute, pConfig, faceIsMoving);
            break;
        case FD_STAB_MEDIAN:
            ApplyMedian(pAttribute, pConfig);
            break;
        case FD_STAB_NO_FILTER:
        default:
            *pCurrentEntry = *pNewEntry;
            break;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Stabilization::ApplyTemporal
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplyTemporal(
    int32_t* pFirst,
    uint32_t weightFirst,
    int32_t* pSecond,
    uint32_t weightSecond)
{

    if ((weightFirst + weightSecond) != 0)
    {
        *pFirst = (((*pSecond * weightSecond) +
                    (*pFirst * weightFirst)) /
                   (weightFirst + weightSecond));
    }
    else
    {
        *pFirst = 0;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Stabilization::ApplyHysteresis
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void  ApplyHysteresis(
    int32_t*                        pCurrentEntry,
    int32_t*                        pNewEntry,
    StabilizationAttributeConfig* pConfig,
    bool                          upDirection)
{

    if (TRUE == upDirection)
    {
        if (*pNewEntry > (int32_t)(pConfig->hysteresisFilter.endB))
        {
            *pCurrentEntry = *pNewEntry;
        }
        else if (*pNewEntry > (int32_t)(pConfig->hysteresisFilter.startB))
        {
            *pNewEntry = pConfig->hysteresisFilter.startB;
        }
        else if (*pNewEntry > (int32_t)(pConfig->hysteresisFilter.endA))
        {
            *pCurrentEntry = *pNewEntry;
        }
        else if (*pNewEntry > (int32_t)(pConfig->hysteresisFilter.startA))
        {
            *pNewEntry = pConfig->hysteresisFilter.startA;
        }
        else
        {
            *pCurrentEntry = *pNewEntry;
        }
    }
    else
    {
        if (*pNewEntry < (int32_t)(pConfig->hysteresisFilter.startA))
        {
            *pCurrentEntry = *pNewEntry;
        }
        else if (*pNewEntry < (int32_t)(pConfig->hysteresisFilter.endA))
        {
            *pNewEntry = pConfig->hysteresisFilter.endA;
        }
        else if (*pNewEntry < (int32_t)(pConfig->hysteresisFilter.startB))
        {
            *pCurrentEntry = *pNewEntry;
        }
        else if (*pNewEntry < (int32_t)(pConfig->hysteresisFilter.endB))
        {
            *pNewEntry = pConfig->hysteresisFilter.endB;
        }
        else
        {
            *pCurrentEntry = *pNewEntry;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Stabilization::StabilizationFilter
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void StabilizationFilterExe(
    StabilizationAttribute*       pAttribute,
    StabilizationAttributeConfig* pConfig,
    StabilizationEntry*           pReference,
    uint32_t                        threshold0,
    uint32_t                        threshold1,
    uint32_t                        movingThreshold,
    StabilizationAttribute*       pSizeAttribute,
    bool                          bFaceAtBoundary,
    uint32_t                      framewidth)
{

    StabilizationState newState      = StabilizingState;
    bool               withinLimit   = FALSE;
    bool               objectStable  = FALSE;
    uint32_t             lastIndex     = 0;
    uint32_t             lastLastIndex = 0;
    bool               faceIsMoving  = FALSE;

    if (pAttribute->numEntries == 1)
    {
       IDBG_MED(" not enough faces skip");
       return;
    }

    if (StabilizingState != pAttribute->state)
    {
        if (NULL != pReference)
        {
            withinLimit = WithinThresholdCheck(pReference,
            &pAttribute->stableEntry,
            threshold0,
            threshold1);
        }
        else
        {
            withinLimit = WithinThresholdCheck(&pAttribute->entryHistory[pAttribute->index],
            &pAttribute->stableEntry,
            threshold0,
            threshold1);
        }
    }

    if (pAttribute->numEntries > 2)
    {
        lastIndex = (pAttribute->index + pAttribute->numEntries - 1) % pAttribute->numEntries;
        lastLastIndex = (pAttribute->index + pAttribute->numEntries - 2) % pAttribute->numEntries;

        // check if face is moving
        faceIsMoving = CheckObjectMovement(&pAttribute->entryHistory[pAttribute->index],
            &pAttribute->entryHistory[lastIndex],
            &pAttribute->entryHistory[lastLastIndex],
            &pSizeAttribute->stableEntry,
            movingThreshold,
            pConfig->movingLinkFactor, framewidth);
    }

    newState = pAttribute->state;

    switch (pAttribute->state)
    {
        case UnstableState:
            if (TRUE == withinLimit)
            {
                IDBG_MED("Stabilization state change to S");
                newState = StableState;
            }
            else if (TRUE == faceIsMoving)
            {
                IDBG_MED("Stabilization state U. face is moving");
                ApplyFilter(pAttribute, pConfig, faceIsMoving);

                IDBG_MED("Stabilization state change to B");
                newState = StabilizingState;
            }
            else if (pAttribute->maxStateCount <= pAttribute->stateCount)
            {
                IDBG_MED("Stabilization state change to B");
                newState = StabilizingState;
            }
            break;
        case StableState:
            if (TRUE == withinLimit)
            {
                if (TRUE == IsContinuesMode(pConfig->mode))
                {
                    objectStable = IsStable(pAttribute, pReference, pConfig, pSizeAttribute, bFaceAtBoundary);
                    if (FALSE == objectStable)
                    {
                        ApplyFilter(pAttribute, pConfig, faceIsMoving);
                    }
                }
                break;
            }
            else
            {
                if (TRUE == faceIsMoving)
                {
                    IDBG_MED("Stabilization state S. face is moving");
                    IDBG_MED("Stabilization state change to B");
                    ApplyFilter(pAttribute, pConfig, faceIsMoving);
                    newState = StabilizingState;
                }
                else
                {
                    IDBG_MED("Stabilization state change to U");
                    newState = UnstableState;
                }
                break;
            }
        case StabilizingState:
            if (TRUE == faceIsMoving)
            {
                IDBG_MED("Stabilization state B. face is moving");
                pAttribute->stateCount = pConfig->movingInitStateCount;
            }
            else
            {
                // check if the state count execeeds the max state count
                if ((pConfig->minStableState > 0) && (pAttribute->stateCount < pConfig->minStableState))
                {
                    objectStable = FALSE;
                }
                else
                {
                    objectStable = IsStable(pAttribute, pReference, pConfig, pSizeAttribute, bFaceAtBoundary);
                }
                if (TRUE == objectStable)
                {
                    if (NULL != pReference)
                    {
                        pAttribute->referenceEntry = *pReference;
                    }
                    IDBG_MED("Stabilization state change to S");
                    newState = StableState;
                }
            }

            // apply filter
            if (TRUE == IsContinuesMode(pConfig->mode))
            {
                pAttribute->stableEntry = pAttribute->entryHistory[pAttribute->index];
            }
            else
            {
                ApplyFilter(pAttribute, pConfig, faceIsMoving);
            }
            break;
        default:
            IDBG_MED("Invalid stabilization state");
            break;
    }

    if (newState != pAttribute->state)
    {
        pAttribute->state = newState;
        pAttribute->stateCount = 0;
    }
    else
    {
        pAttribute->stateCount++;
    }

    IDBG_MED("Stabilization State %d, state count = %d", pAttribute->state, pAttribute->stateCount);
}

int ExecuteStabilization(
    StabilizationData* pCurrentData,
    StabilizationData* pStabilizedData,
    img_size_t *p_frame_dim,
    StabilizationHistory *p_stab)
{
    int                     result               = IMG_SUCCESS;
    uint32_t                i                    = 0;
    uint32_t                j                    = 0;
//  uint32_t                k                    = 0;
    uint32_t                numObjects           = 0;
    StabilizationPosition   position             = ObjectSame;
    uint32_t                maxAllowedThreshold0 = 0xFFFF;
    uint32_t                maxAllowedThreshold1 = 0xFFFF;
    uint32_t                threshold0           = 0;
    uint32_t                threshold1           = 0;
    uint32_t                movingThreshold      = 0;
    StabilizationEntry      reference            = { 0 };
    StabilizationEntry*     pReference           = NULL;
//  int32_t                 halfSize             = 0;
    int32_t                 frameWidth           = 0;
    int32_t                 frameHeight          = 0;
    bool                    enableShrinking      = FALSE;
    int32_t                 left                 = 0;
    int32_t                 top                  = 0;
    int32_t                 width                = 0;
    int32_t                 height               = 0;
    int32_t                 boundaryOffset       = 0;
    int32_t*                pWidth               = NULL;
    int32_t*                pHeight              = NULL;
    int32_t*                pCenterX             = NULL;
    int32_t*                pCenterY             = NULL;
    bool                    bFaceAtBoundary      = FALSE;

    frameWidth  = p_frame_dim->width;
    frameHeight = p_frame_dim->height;

    if ((NULL == pCurrentData) || (NULL == pStabilizedData))
    {
        IDBG_MED("Unable to execute stabilization, input data is NULL");
        result = IMG_ERR_INVALID_INPUT;
    }
    else
    {
        if (pCurrentData->numObjects > StabilizationMaxObjects)
        {
            IDBG_MED("Invalid number of objects %d", pCurrentData->numObjects);
            result = IMG_ERR_INVALID_INPUT;
        }
    }

    if (m_configuration.historyDepth > StabilizationMaxHistory)
    {
        IDBG_MED("Invalid history size %d", m_configuration.historyDepth);
        result = IMG_ERR_INVALID_INPUT;
    }

    if ((IMG_SUCCESS == result) && (0 == pCurrentData->numObjects))
    {
        IDBG_MED("No objects detected, history reset");
    }

    if ((IMG_SUCCESS == result) && (pCurrentData->numObjects > 1))
    {
        // Sort objects from smallest coordinate entry to largest
        qsort(&pCurrentData->objectData[0], pCurrentData->numObjects, sizeof(pCurrentData->objectData[0]),
                     SortObjectPosition);
    }

    if (IMG_SUCCESS == result)
    {
        numObjects = pCurrentData->numObjects;
        IDBG_MED("numobjs %d stab faces %d", numObjects, p_stab->numObjects );
        while (i < numObjects)
        {
            // If there are new objects, put them in history
            if (i >= p_stab->numObjects)
            {
                IDBG_MED("Face sorting: face is new");
                InitializeObjectEntry(&p_stab->objects[j], &pCurrentData->objectData[i]);
                p_stab->objects[j].pFaceInfo = pCurrentData->objectData[i].pFaceInfo;
                i++;
                j++;
                p_stab->numObjects++;
                continue;
            }



            position = CheckObject(&p_stab->objects[j], &pCurrentData->objectData[i]);
            IDBG_MED("Values i %d j%d", i, j);

            if (ObjectSame == position)
            {
                IDBG_MED("Face sorting: position is same");
                AddObjectEntry(&p_stab->objects[j],
                               &pCurrentData->objectData[i],
                               &m_configuration);
                p_stab->objects[j].pFaceInfo = pCurrentData->objectData[i].pFaceInfo;
                i++;
                j++;
            }
            else if (ObjectBefore == position)
            {
                IDBG_MED("Face sorting: position is before");
                // Move the objects to the right if it is not the last element
                if (i < (pCurrentData->numObjects - 1))
                {
                    if (p_stab->numObjects < StabilizationMaxObjects)
                    {
                        memmove(&p_stab->objects[j + 1],
                                       &p_stab->objects[j],
                                       (sizeof(p_stab->objects[0]) * (p_stab->numObjects - j)));

                        p_stab->numObjects++;
                    }
                    else
                    {
                        memmove(&p_stab->objects[j + 1],
                                       &p_stab->objects[j],
                                       (sizeof(p_stab->objects[0]) * (StabilizationMaxObjects - (j + 1))));
                    }
                }

                InitializeObjectEntry(&p_stab->objects[j], &pCurrentData->objectData[i]);
                p_stab->objects[j].pFaceInfo = pCurrentData->objectData[i].pFaceInfo;
                i++;
                j++;
            }
            else if (ObjectAfter == position)
            {
                IDBG_MED("Face sorting: position is after");
                memmove(&p_stab->objects[j],
                               &p_stab->objects[j + 1],
                               (sizeof(p_stab->objects[0]) * (p_stab->numObjects - (j + 1))));

                p_stab->numObjects--;
            }
        }

        if (p_stab->numObjects > numObjects)
        {
            p_stab->numObjects = numObjects;
        }
    }

    if (IMG_SUCCESS == result)
    {
        // Stabilize all objects and their attributes
        for (i = 0; i < numObjects; i++)
        {
            // Check if face ROI is beyond boundaries
            pCenterX       = &(pCurrentData->objectData[i].attributeData[ObjectPositionIndex].entry.data0);
            pCenterY       = &(pCurrentData->objectData[i].attributeData[ObjectPositionIndex].entry.data1);
            pWidth         = &(pCurrentData->objectData[i].attributeData[ObjectSizeIndex].entry.data0);
            pHeight        = &(pCurrentData->objectData[i].attributeData[ObjectSizeIndex].entry.data1);
            left           = (*pCenterX) - ((*pWidth) / 2);
            top            = (*pCenterY) - ((*pHeight) / 2);
            width          = *pWidth;
            height         = *pHeight;
            boundaryOffset = frameHeight / 30;

            if ((left < boundaryOffset) || (left + width > frameWidth - boundaryOffset) ||
                (top < boundaryOffset) || (top + height > frameHeight - boundaryOffset))
            {
                bFaceAtBoundary = TRUE;
                IDBG_MED("Face %d reaches boundary", i);
            }

            pStabilizedData->objectData[i].id            = p_stab->objects[i].id;
            pStabilizedData->objectData[i].numAttributes = p_stab->objects[i].numAttributes;
            pStabilizedData->objectData[i].pFaceInfo     = p_stab->objects[i].pFaceInfo;

            for (j = 0; j < pCurrentData->objectData[i].numAttributes; j++)
            {
                if (TRUE == m_configuration.attributeConfigs[j].enable)
                {
                    maxAllowedThreshold0 = 0xFFFF;
                    maxAllowedThreshold1 = 0xFFFF;
                    threshold0           = 0;
                    threshold1           = 0;
                    pReference           = NULL;

                    // Determine if stabilizing with reference is being used
                    if ((TRUE == m_configuration.attributeConfigs[j].useReference)  &&
                        (TRUE == pCurrentData->objectData[i].attributeReference[j].valid))
                    {
                        reference = pCurrentData->objectData[i].attributeReference[j].entry;
                        pReference = &reference;
                    }

                    if (ObjectPositionIndex == j)
                    {
                        // Check boundaries
                        if (TRUE == bFaceAtBoundary)
                        {
                            threshold0 = (p_stab->objects[i].objectAttributes[ObjectSizeIndex].stableEntry.data0*
                                          m_configuration.attributeConfigs[j].threshold) / 700;
                            threshold1 = (p_stab->objects[i].objectAttributes[ObjectSizeIndex].stableEntry.data1*
                                          m_configuration.attributeConfigs[j].threshold) / 700;
                        }
                        else
                        {
                            threshold0 = (p_stab->objects[i].objectAttributes[ObjectSizeIndex].stableEntry.data0*
                                          m_configuration.attributeConfigs[j].threshold) / 1000;
                            threshold1 = (p_stab->objects[i].objectAttributes[ObjectSizeIndex].stableEntry.data1*
                                          m_configuration.attributeConfigs[j].threshold) / 1000;
                        }

                        movingThreshold = (p_stab->objects[i].objectAttributes[ObjectSizeIndex].stableEntry.data0*
                                           m_configuration.attributeConfigs[j].movingThreshold) / 1000;

                        maxAllowedThreshold0 = (pCurrentData->objectData[i].attributeData[ObjectSizeIndex].entry.data0 *
                                                m_configuration.attributeConfigs[j].threshold) / 100;
                        maxAllowedThreshold1 = (pCurrentData->objectData[i].attributeData[ObjectSizeIndex].entry.data1 *
                                                m_configuration.attributeConfigs[j].threshold) / 100;
                    }
                    else
                    {
                        // Size uses reference to calculate threshold
                        if ((ObjectSizeIndex == j) && (NULL != pReference))
                        {
                            threshold0 = (pReference->data0 *
                                          m_configuration.attributeConfigs[j].threshold) / 1000;
                            threshold1 = (pReference->data1 *
                                          m_configuration.attributeConfigs[j].threshold) / 1000;
                        }
                        else
                        {
                            // Check boundaries
                            if (TRUE == bFaceAtBoundary)
                            {
                                threshold0 = (p_stab->objects[i].objectAttributes[j].stableEntry.data0*
                                              m_configuration.attributeConfigs[j].threshold) / 700;
                                threshold1 = (p_stab->objects[i].objectAttributes[j].stableEntry.data1*
                                              m_configuration.attributeConfigs[j].threshold) / 700;
                            }
                            else
                            {
                                threshold0 = (p_stab->objects[i].objectAttributes[j].stableEntry.data0*
                                              m_configuration.attributeConfigs[j].threshold) / 1000;
                                threshold1 = (p_stab->objects[i].objectAttributes[j].stableEntry.data1*
                                              m_configuration.attributeConfigs[j].threshold) / 1000;
                            }

                            movingThreshold = (p_stab->objects[i].objectAttributes[j].stableEntry.data0 *
                                               m_configuration.attributeConfigs[j].movingThreshold) / 1000;

                            maxAllowedThreshold0 = (pCurrentData->objectData[i].attributeData[j].entry.data0 *
                                                    m_configuration.attributeConfigs[j].threshold) / 100;
                            maxAllowedThreshold1 = (pCurrentData->objectData[i].attributeData[j].entry.data1 *
                                                    m_configuration.attributeConfigs[j].threshold) / 100;
                        }
                    }

                    if (threshold0 > maxAllowedThreshold0)
                    {
                        threshold0 = maxAllowedThreshold0;
                    }
                    if (threshold1 > maxAllowedThreshold1)
                    {
                        threshold1 = maxAllowedThreshold1;
                    }

                    IDBG_MED("Stabilizing object %d:%d current %d %d stable %d %d threshold %d %d %d",
                                     i,
                                     j,
                                     pCurrentData->objectData[i].attributeData[j].entry.data0,
                                     pCurrentData->objectData[i].attributeData[j].entry.data1,
                                     p_stab->objects[i].objectAttributes[j].stableEntry.data0,
                                     p_stab->objects[i].objectAttributes[j].stableEntry.data1,
                                     threshold0,
                                     threshold1,
                                     movingThreshold);

                    StabilizationFilterExe(&p_stab->objects[i].objectAttributes[j],
                                        &m_configuration.attributeConfigs[j],
                                        pReference,
                                        threshold0,
                                        threshold1,
                                        movingThreshold,
                                        &p_stab->objects[i].objectAttributes[ObjectSizeIndex],
                                        bFaceAtBoundary, frameWidth);

                    pStabilizedData->objectData[i].attributeData[j].entry =
                        p_stab->objects[i].objectAttributes[j].stableEntry;
                    pStabilizedData->objectData[i].attributeData[j].valid = TRUE;

                    IDBG_MED(
                                     "Stabilzed Output %d:%d current %d %d final %d %d",
                                     i,
                                     j,
                                     pCurrentData->objectData[i].attributeData[j].entry.data0,
                                     pCurrentData->objectData[i].attributeData[j].entry.data1,
                                     pStabilizedData->objectData[i].attributeData[j].entry.data0,
                                     pStabilizedData->objectData[i].attributeData[j].entry.data1);
                }

                if (TRUE == enableShrinking)
                {
                    // Handle object position corner cases
                    // Shrink size so that it does not go outside of frame

                    int32_t* pWidth;
                    int32_t* pHeight;
                    int32_t* pCenterX;
                    int32_t* pCenterY;

                    int32_t  left;
                    int32_t  top;
                    int32_t  width;
                    int32_t  height;

                    pCenterX = &(pStabilizedData->objectData[i].attributeData[ObjectPositionIndex].entry.data0);
                    pCenterY = &(pStabilizedData->objectData[i].attributeData[ObjectPositionIndex].entry.data1);
                    pWidth   = &(pStabilizedData->objectData[i].attributeData[ObjectSizeIndex].entry.data0);
                    pHeight  = &(pStabilizedData->objectData[i].attributeData[ObjectSizeIndex].entry.data1);

                    left   = (*pCenterX) - ((*pWidth) / 2);
                    top    = (*pCenterY) - ((*pHeight) / 2);
                    width  = *pWidth;
                    height = *pHeight;

                    // Check left and right boundaries
                    if (left < 0)
                    {
                        width = width + left;
                        left  = 0;
                    }
                    else if (left + width > frameWidth)
                    {
                        width = frameWidth - left;
                    }

                    // Check upper and lower boundaries
                    if (top < 0)
                    {
                        height = height + top;
                        top    = 0;
                    }
                    else if (top + height > frameHeight)
                    {
                        height = frameHeight - top;
                    }

                    *pCenterX = left + (width / 2);
                    *pCenterY = top + (height / 2);
                    *pWidth   = width;
                    *pHeight  = height;

                    IDBG_MED(
                                     "Stabilzed Output 2 %d:%d current %d %d final %d %d",
                                     i,
                                     j,
                                     pCurrentData->objectData[i].attributeData[j].entry.data0,
                                     pCurrentData->objectData[i].attributeData[j].entry.data1,
                                     pStabilizedData->objectData[i].attributeData[j].entry.data0,
                                     pStabilizedData->objectData[i].attributeData[j].entry.data1);
                }
            }
        }

        pStabilizedData->numObjects = numObjects;

        IDBG_MED( "Stabilization history is tracking %d objects", p_stab->numObjects);
    }

    return result;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Stabilization::ApplyAverage
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplyAverage(
    StabilizationAttribute*       pAttribute,
    StabilizationAttributeConfig* pConfig,
    bool                          faceIsMoving)
{
    uint32_t index         = 0;
    uint32_t historyLength = pConfig->averageFilter.historyLength;
    uint64_t sumData0      = 0;
    uint64_t sumData1      = 0;

    if (TRUE == faceIsMoving)
    {
        historyLength = pConfig->averageFilter.movingHistoryLength;
    }

    if (historyLength < 2)
    {
        pAttribute->stableEntry = pAttribute->entryHistory[pAttribute->index];
    }
    else
    {
        if (historyLength > pAttribute->numEntries)
        {
            historyLength = pAttribute->numEntries;
        }

        index = (pAttribute->index + pAttribute->numEntries - historyLength + 1) % pAttribute->numEntries;

        for (uint32_t i = 0; i < historyLength; i++)
        {
            IDBG_MED("Stabilization apply average. history %d, x = %d, y = %d",
                             i,
                             pAttribute->entryHistory[index].data0,
                             pAttribute->entryHistory[index].data1);
            index    %= pAttribute->historySize;
            sumData0 += pAttribute->entryHistory[index].data0;
            sumData1 += pAttribute->entryHistory[index].data1;

            index++;
        }

        pAttribute->stableEntry.data0 = (int32_t)(sumData0 / historyLength);
        pAttribute->stableEntry.data1 = (int32_t)(sumData1 / historyLength);
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// MedianSort
///
/// @brief  Comparison function that will be used for determining the median, sorts smaller to greater
///
/// @param  pArg0 Pointer to object to compare for sorting
/// @param  pArg1 Pointer to object to compare for sorting
///
/// @return 1 if pArg0 is greater than pArg1, 0 if equal, -1 otherwise
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int32_t MedianSort(
    const void* pArg0,
    const void* pArg1)
{
    uint32_t*   pValue0     = (uint32_t *)(pArg0);
    uint32_t*   pValue1     = (uint32_t *)(pArg1);
    int             comparison  = 0;

    if (*pValue0 == *pValue1)
    {
        comparison = 0;
    }
    else if (*pValue0 > *pValue1)
    {
        comparison = 1;
    }
    else
    {
        comparison = -1;
    }

    return comparison;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Stabilization::ApplyMedian
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ApplyMedian(
    StabilizationAttribute*       pAttribute,
    StabilizationAttributeConfig* pConfig)
{
    uint32_t              index                                   = pAttribute->index;
    StabilizationEntry  medianEntry[StabilizationMaxHistory]    = { {0} };

    if (pConfig->medianFilter.historyLength > pAttribute->numEntries)
    {
        pAttribute->stableEntry = pAttribute->entryHistory[index];
    }
    else
    {
        for (uint32_t i = 0; i < pConfig->medianFilter.historyLength; i++)
        {
            medianEntry[i] = pAttribute->entryHistory[index % pAttribute->historySize];
            index++;
        }

        qsort(&medianEntry[0].data0, pConfig->medianFilter.historyLength, sizeof(medianEntry[0]), MedianSort);

        index                         = (pConfig->medianFilter.historyLength / 2);
        pAttribute->stableEntry.data0 = medianEntry[index].data0;

        qsort(&medianEntry[0].data1, pConfig->medianFilter.historyLength, sizeof(medianEntry[0]), MedianSort);

        index                         = (pConfig->medianFilter.historyLength / 2);
        pAttribute->stableEntry.data1 = medianEntry[index].data1;
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FDUtils::ConvertFromStabilizationData
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int ConvertFromStabilizationData(
    StabilizationData* pStabilizationData,
    faceproc_result_t*         pResults)
{
    int result = IMG_SUCCESS;
    uint32_t     index  = 0;

    if ((pStabilizationData == NULL) || (pResults == NULL))
    {
        IDBG_ERROR("Inputs are NULL");
        return IMG_ERR_INVALID_INPUT;
    }

    for (uint32_t i = 0; i < pStabilizationData->numObjects; i++)
    {
        memcpy(&pResults->roi[i], pStabilizationData->objectData[i].pFaceInfo, sizeof(faceproc_info_t));

        pResults->roi[i].unique_id = pStabilizationData->objectData[i].id;

        index = ObjectPositionIndex;
        if (TRUE == pStabilizationData->objectData[i].attributeData[index].valid)
        {
            pResults->roi[i].face_boundary.x = pStabilizationData->objectData[i].attributeData[index].entry.data0;
            pResults->roi[i].face_boundary.y = pStabilizationData->objectData[i].attributeData[index].entry.data1;
        }

        index = ObjectSizeIndex;
        if (TRUE == pStabilizationData->objectData[i].attributeData[index].valid)
        {
            pResults->roi[i].face_boundary.dx  = pStabilizationData->objectData[i].attributeData[index].entry.data0;
            pResults->roi[i].face_boundary.dy = pStabilizationData->objectData[i].attributeData[index].entry.data1;
        }

        // Below indexes can be used for other stabilization attributes for the face
        // Use the below index value to pull attribute from correct index, then increment index
        index = ObjectUnreservedIndex;
    }

    pResults->num_faces_detected = pStabilizationData->numObjects;

    IDBG_MED("numObjects = %d", pStabilizationData->numObjects);

    return result;
}


