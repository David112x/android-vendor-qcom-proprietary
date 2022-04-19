#ifndef __3A_DEBUG_DATA_TYPES_H__
#define __3A_DEBUG_DATA_TYPES_H__

/*===========================================================================

         CameraPackedEnd D a t a  S t r u c t u r e  D e c l a r a t i o n

*//** @file AWBDebugData.h


* Copyright (c) 2014-2016 Qualcomm Technologies, Inc. All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/

/* ==========================================================================

                             Edit History






when       who     what, where, why
--------   ---     -------------------------------------------------------
03/27/14   vb      Initial Revision

========================================================================== */

/* ==========================================================================
                     INCLUDE FILES FOR MODULE
========================================================================== */
#include <stdint.h>

/* ==========================================================================
                       Preprocessor Definitions and Constants
========================================================================== */

/* ==========================================================================
                       Static Declarations
========================================================================== */


/* ==========================================================================
                       Type Declarations
========================================================================== */
#pragma pack(push, 1)

/// This structure defines the format of the 3A Tuning version information.
typedef struct
{
    /// The chromatix tuning header version that is in use.
    uint16 chromatixVersionNumber;

    /// The tuning header revision number that is in use.
    uint16 tuningHeaderRevisionNumber;

} TuningDataInfoType;

#pragma pack(pop)


#endif // __3A_DEBUG_DATA_TYPES_H__

