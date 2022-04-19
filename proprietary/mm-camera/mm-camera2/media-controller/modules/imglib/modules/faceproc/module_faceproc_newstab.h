/***************************************************************************
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
***************************************************************************/

#ifndef __MODULE_FACEPROC_NEWSTAB_H__
#define __MODULE_FACEPROC_NEWSTAB_H__

static const uint32_t StabilizationMaxHistory          = 10;
static const uint32_t StabilizationMaxObjectAttributes = 10;
static const uint32_t StabilizationMaxObjects          = 10;

/// @brief enum to define the history object attribute reserved indexes
typedef enum
{
    ObjectPositionIndex,   ///< Index in the stabilization history that is reserved for position of the object
    ObjectSizeIndex,       ///< Index in the stabilization history that is reserved for size of the object
    ObjectUnreservedIndex  ///< Index in the stabilization history that starts the unreserved index section
}StabilizationIndex;

/// @brief enum to define all the different stabilization states
typedef enum
{
    StableState,     ///< State is stable, previous data will be used
    UnstableState,   ///< State is unstable, previous data will be used
    StabilizingState ///< Stabilizing, switch to stable state along with new position data
}StabilizationState;

/// @brief enum to define all the stabilization object position types
typedef enum
{
    ObjectSame,   ///< Object is considered to be the same
    ObjectBefore, ///< Object is considered to be before
    ObjectAfter   ///< Object is considered to be after
}StabilizationPosition;

/// @brief enum to define all the different stabilization modes
typedef enum
{
    Equal,                          ///< Values will be marked as stable when two consecutive values are equal
    Smaller,                        ///< Values will be marked as stable if new values are bigger than old ones
    Bigger,                         ///< Values will be marked as stable if new values are smaller than old ones
    CloserToReference,              ///< Values will be marked as stable when the distance to reference is smaller
    ContinuesEqual,                 ///< The same as Equal, but it works in continues mode
    ContinuesSmaller,               ///< The same as Smaller, but it works in continues mode
    ContinuesBigger,                ///< The same as Bigger, but it works in continues mode
    ContinuesCloserToReference,     ///< The same as CloserToReference, but it works in continues mode
    WithinThreshold                 ///< Values are marked as stable when values are within threshold
}StabilizationMode;

/// @brief enum to define all the different stabilization filters
typedef enum
{
    NoFilter,      ///< No stabilization filter
    Temporal,      ///< Temporal filter
    Hysteresis,    ///< Hysteresis
    Average,       ///< Average filter
    Median         ///< Median filter
}StabilizationFilter;

/// @brief Structure to define Temporal Filter
typedef struct
{
    uint32_t numerator;    ///< Strength numerator
    uint32_t denominator;  ///< Strength denominator
}TemporalFilter;

/// @brief Structure to define Hysteresis Filter
typedef struct
{
    uint32_t startA;    ///< Start point of Zone A
    uint32_t endA;      ///< End point of Zone A
    uint32_t startB;    ///< Start point of Zone B
    uint32_t endB;      ///< End point of Zone B
}HysteresisFilter;

/// @brief Structure to define Average Filter
typedef struct
{
    uint32_t historyLength;         ///< History length of the filter
    uint32_t movingHistoryLength;   ///< History length of the filter for moving face
}AverageFilter;

/// @brief Structure to define Median Filter
typedef struct
{
    uint32_t historyLength;  ///< History length of the filter
}MedianFilter;

/// @brief Structure to hold stabilization tuning parameters
typedef struct
{
    bool                 enable;                ///< Enable stabilization
    fd_face_stab_mode_t    mode;                  ///< Stabilization Mode
    uint32_t               minStableState;        ///< Minimum state count needed to go to a stable state
    uint32_t               stableThreshold;       ///< Stabilization threshold to go into stable state
    uint32_t               threshold;             ///< Within threshold, new values will not be accepted
    uint32_t               stateCount;            ///< Number of consecutive frames to wait until determining stable
    uint32_t               useReference;          ///< Stabilize data by a reference
    fd_face_stab_filter_t  filterType;            ///< Filter type to be used for stabilization
    uint32_t               movingThreshold;       ///< With threshold, object is not moved
    uint32_t               movingInitStateCount;  ///< Init state count for moving face during stabilizing state
    float                movingLinkFactor;      ///< Factor to determine moving face with previous 2 history
    union
    {
        TemporalFilter   temporalFilter;     ///< Temporal Filter
        HysteresisFilter hysteresisFilter;   ///< HysteresisFilter
        AverageFilter    averageFilter;      ///< Average Filter
        MedianFilter     medianFilter;       ///< Median Filter
    };
}StabilizationAttributeConfig;

/// @brief Structure to hold stabilization configuration
typedef struct
{
    bool                         enable;                                             ///< Enable stabilization
    uint32_t                       historyDepth;                                       ///< Depth of historical data
    StabilizationAttributeConfig attributeConfigs[StabilizationMaxObjectAttributes]; ///< Array of attibute configurations
}StabilizationConfig;

/// @brief Structure to define a stabilization entry
typedef struct
{
    int32_t data0;     ///< Arbitrary data 0
    int32_t data1;     ///< Arbitrary data 1
    bool  isChanged; ///< Arbitrary state
}StabilizationEntry;

/// @brief Structure to hold stabilization data for a particular object attribute
typedef struct
{
    uint32_t             index;                                    ///< Current acive index in the history
    uint32_t             historySize;                              ///< Size of the history
    uint32_t             numEntries;                               ///< Number of data entries in history
    uint32_t             stateCount;                               ///< Number of frames in current state
    uint32_t             maxStateCount;                            ///< For unstable state, max count the state can be active
    StabilizationState state;                                    ///< Current state
    StabilizationEntry stableEntry;                              ///< Current stable entry
    StabilizationEntry referenceEntry;                           ///< Reference if stabilization is used with reference
    StabilizationEntry entryHistory[StabilizationMaxHistory];    ///< Array of historical data entries
}StabilizationAttribute;

/// @brief Structure to hold stabilization data for a particular object
typedef struct
{
    uint32_t                 id;                                                 ///< Unique object identification
    faceproc_info_t*                  pFaceInfo;                                          ///< Face info for this object
    uint32_t                 numAttributes;                                      ///< Number of attributes to track
    StabilizationAttribute objectAttributes[StabilizationMaxObjectAttributes]; ///< Attributes of an object to stabilize
}StabilizationObject;

/// @brief Structure to hold the historical stabilization data
typedef struct
{
    uint32_t              numObjects;                       ///< Number of objects in the history
    StabilizationObject objects[StabilizationMaxObjects]; ///< Array of objects for stabilization
}StabilizationHistory;

/// @brief Structure to hold the stabilization data values for the stabilization calculation
typedef struct
{
    bool               valid;    ///< Wheter the data here is valid
    StabilizationEntry entry;    ///< Data entry
}StabilizationPoint;

/// @brief Structure to hold stabilization for an object's attributes for the current frame
typedef struct
{
    uint32_t             id;                                                   ///< Unique object identification
    uint32_t             numAttributes;                                        ///< Number of attributes for this object
    StabilizationPoint attributeData[StabilizationMaxObjectAttributes];      ///< Array of attribute data of an object
    StabilizationPoint attributeReference[StabilizationMaxObjectAttributes]; ///< Array of attribute references of an object
    faceproc_info_t*              pFaceInfo;                                            ///< Pointer to original face info for this
}StabilizationHolder;

/// @brief Structure to hold stabilization for an object for current frame
typedef struct
{
    uint32_t              numObjects;                             ///< Number of objects for current frame
    StabilizationHolder objectData[StabilizationMaxObjects];    ///< Array of object data
}StabilizationData;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Initialize
    ///
    /// @brief  Initialize the stabilization object
    ///
    /// @return Success if success
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    int InitializeStab();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// GetConfig
    ///
    /// @brief  Get the stabilization configuration
    ///
    /// @param  pConfig Pointer to the structure to store the current config
    ///
    /// @return Success if success
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    int GetConfig(
        StabilizationConfig* pConfig);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// SetConfig
    ///
    /// @brief  Set the stabilization configuration
    ///
    /// @param  pConfig  Pointer to config data to set
    ///
    /// @return Success if success
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    int module_faceproc_newstab_setConfig(
        fd_chromatix_t* pConfig,
        StabilizationConfig* pStabilizationConfig);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// ExecuteStabilization
    ///
    /// @brief  Execute stabilization, main function to stabilize current data using history
    ///
    /// @param  pCurrentData    Pointer to current stabilization data
    /// @param  pStabilizedData Pointer to structure that this function will write stabilized data to
    /// @param  p_stab          Pointer to StabilizationHistory
    ///
    /// @return return Result
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    int ExecuteStabilization(
        StabilizationData* pCurrentData,
        StabilizationData* pStabilizedData,
        img_size_t *p_frame_dim,
        StabilizationHistory *p_stab);

    int ConvertToStabilizationData(
        faceproc_result_t*         pResults,
        StabilizationData* pStabilizationData);

    int ConvertFromStabilizationData(
        StabilizationData* pStabilizationData,
        faceproc_result_t*    pResults);


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// StabilizationFilterExe
    ///
    /// @brief  Calculate the stabilization state and apply the appropriate filter to the object's attribute
    ///
    /// @param  pAttribute          Pointer to object's attribute to stabilize
    /// @param  pConfig             Pointer to attribute's configuration info
    /// @param  pReference          Pointer to attribute reference entry
    /// @param  threshold0          Threshold for stabilization of data0
    /// @param  threshold1          Threshold for stabilization of data1
    /// @param  movingThreshold     Threshold for moving face
    /// @param  pSizeAttribute      Pointer to object's size attribute
    /// @param  bFaceAtBoundary     Flag to indicate if face is at boundaries
    /// @param  framewidth          Frame width
    /// 
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void StabilizationFilterExe(
        StabilizationAttribute*       pAttribute,
        StabilizationAttributeConfig* pConfig,
        StabilizationEntry*           pReference,
        uint32_t                        threshold0,
        uint32_t                        threshold1,
        uint32_t                        movingThreshold,
        StabilizationAttribute*       pSizeAttribute,
        bool                          bFaceAtBoundary,
        uint32_t                      framewidth);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// InitializeObjectEntry
    ///
    /// @brief  Initialized and add new object entry to the history
    ///
    /// @param  pHistoryEntry Pointer to data in the history to write to
    /// @param  pObjectEntry  Pointer to data of the object to add to the history
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void InitializeObjectEntry(
        StabilizationObject* pHistoryEntry,
        StabilizationHolder* pObjectEntry);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// AddObjectEntry
    ///
    /// @brief  Add new object entry to the history
    ///
    /// @param  pHistoryEntry Pointer to data in the history to write to
    /// @param  pObjectEntry  Pointer to data of the object to add to the history
    /// @param  pConfig       Pointer to the object configuration info
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void AddObjectEntry(
        StabilizationObject* pHistoryEntry,
        StabilizationHolder* pObjectEntry,
        StabilizationConfig* pConfig);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// CheckObject
    ///
    /// @brief  Check and determine the position of the current object with respect to the history
    ///
    /// @param  pHistoryEntry  Pointer to data in the history object
    /// @param  pCurrentObject Pointer to data of the current object
    ///
    /// @return StabilizationPosition position used to determine location of current object
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    StabilizationPosition CheckObject(
        StabilizationObject* pHistoryEntry,
        StabilizationHolder* pCurrentObject);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// WithinThresholdCheck
    ///
    /// @brief  Determine if the current data entry is within the threshold of the historical stable entry
    ///
    /// @param  pCurrentEntry Pointer to object attribute's current entry
    /// @param  pStableEntry  Pointer to object attribute's stable entry
    /// @param  threshold0    Threshold for stabilization of data0
    /// @param  threshold1    Threshold for stabilization of data1
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool WithinThresholdCheck(
        StabilizationEntry* pCurrentEntry,
        StabilizationEntry* pStableEntry,
        uint32_t              threshold0,
        uint32_t              threshold1);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// IsContinuesMode
    ///
    /// @brief  Determines if the given stabilization filter mode is continues mode
    ///
    /// @param  type The stabilization mode
    ///
    /// @return TRUE if given mode is a continues mode
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool IsContinuesMode( fd_face_stab_mode_t type);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// IsStable
    ///
    /// @brief  Determine if the given Object's attribute is stable
    ///
    /// @param  pAttribute       Pointer to object's attribute to stabilize
    /// @param  pReference       Pointer to attribute reference entry
    /// @param  pConfig          Pointer to attribute's configuration info
    /// @param  pSizeAttribute   Pointer to object's size attribute
    /// @param  bFaceAtBoundary  Flag to indicate if face is at boundaries
    ///
    /// @return TRUE if stable
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool IsStable(
        StabilizationAttribute*       pAttribute,
        StabilizationEntry*           pReference,
        StabilizationAttributeConfig* pConfig,
        StabilizationAttribute*       pSizeAttribute,
        bool                          bFaceAtBoundary);


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// CheckObjectMovement
    ///
    /// @brief  Determine if the given Object's is moving
    ///
    /// @param  pCurrentEntry    Pointer to current object's attribute
    /// @param  pLastEntry       Pointer to last object's attribute
    /// @param  pLastLastEntry   Pointer to last last object's attribute
    /// @param  pStableSizeEntry Current size stable entry
    /// @param  threshold0       Threshold for stabilization of data0
    /// @param  movingLinkFactor Factor to determine moving face with previous 2 history
    /// @param  framewidth       Frame width
    ///
    /// @return TRUE if moving
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool CheckObjectMovement(
        StabilizationEntry*    pCurrentEntry,
        StabilizationEntry*    pLastEntry,
        StabilizationEntry*    pLastLastEntry,
        StabilizationEntry*    pStableSizeEntry,
        uint32_t               threshold0,
        float                  movingLinkFactor,
        uint32_t               framewidth);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// ApplyFilter
    ///
    /// @brief  Initialized and add new object entry to the history
    ///
    /// @param  pAttribute   Pointer to object's attribute to stabilize
    /// @param  pConfig      Pointer to attribute's configuration info
    /// @param  faceIsMoving Flag to check if face is moving
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void ApplyFilter(
        StabilizationAttribute*       pAttribute,
        StabilizationAttributeConfig* pConfig,
        bool                          faceIsMoving);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// ApplyHysteresis
    ///
    /// @brief  Apply the Hysteresis filter
    ///
    /// @param  pCurrentEntry Pointer to object's attribute current entry data
    /// @param  pNewEntry     Pointer to attribute's new entry data
    /// @param  pConfig       Pointer to attribute's configuration info
    /// @param  upDirection   Is the Hysteresis input data in the up direction
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void ApplyHysteresis(
        int32_t*                        pCurrentEntry,
        int32_t*                        pNewEntry,
        StabilizationAttributeConfig* pConfig,
        bool                          upDirection);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// ApplyTemporal
    ///
    /// @brief  Apply the Temporal filter
    ///
    /// @param  pFirst       Pointer to data of the first parameter for the temporal filter
    /// @param  weightFirst  Weight of the first data value
    /// @param  pSecond      Pointer to data of the second parameter for the temporal filter
    /// @param  weightSecond Weight of the second data value
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void ApplyTemporal(
        int32_t* pFirst,
        uint32_t weightFirst,
        int32_t* pSecond,
        uint32_t weightSecond);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// ApplyAverage
    ///
    /// @brief  Apply the Average filter
    ///
    /// @param  pAttribute   Pointer to object's attribute to stabilize
    /// @param  pConfig      Pointer to attribute's configuration info
    /// @param  faceIsMoving Flag to check if face is moving
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void ApplyAverage(
        StabilizationAttribute*       pAttribute,
        StabilizationAttributeConfig* pConfig,
        bool                          faceIsMoving);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// ApplyMedian
    ///
    /// @brief  Apply the Median filter
    ///
    /// @param  pAttribute Pointer to object's attribute to stabilize
    /// @param  pConfig    Pointer to attribute's configuration info
    ///
    /// @return None
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void ApplyMedian(
        StabilizationAttribute*       pAttribute,
        StabilizationAttributeConfig* pConfig);

    StabilizationConfig  m_configuration;   ///< Stabilization configuration info

#endif // CAMXSTABILIZATION_H
