#ifndef HWTRACKERAPI_H__
#define HWTRACKERAPI_H__

#include "Common.h"

typedef VOID*          HHWTRACKER;              /* HWTracker handle */
typedef VOID*          HHWTRACKERRESULT;        /* HWTracker result handle */

#define HWTRACKER_FACE_MAX_NUM    35



// This is just for a pseudo implementation
// A pointer of this struct is passed to the false positive filter and
// not referred by Tracking
#ifndef DEFINE_FPFILTER
#define DEFINE_FPFILTER
typedef struct tagFPFILTER{
	VOID*        hCommon;   /* Common Functions handle */
	VOID*        hDt;       /* Face Detection handle */
	VOID*        hDt2;       /* Face Detection handle */
	VOID*        hDtResult; /* Face Detection result handle */
	VOID*        hDtResult2; /* Face Detection result handle */
	RAWIMAGE*    pImg;      /* Input image buffer pointer */
	INT32        nWidth;    /* Input image width */
	INT32        nHeight;   /* Input image height */
	INT32        nDepth;    /* Input image depth */
} FPFILTER;
#endif



/* Enum to describe the tracking status */
typedef enum tagHWT_TrackStatus{
	HWT_NONE,
	HWT_DELAY,      /* Delayed (already detected/tracked but the specified delay count has not been reached yet) */
	HWT_CONSISTENT, /* Tracked (now tracking) */
	HWT_LOST_HW,    /* Lost 1 (not detected by HW FD but the specified hold count has not been reached yet) */
	HWT_LOST_SW,    /* Lost 2 (filtered out by SW filter but the specified hold count has not been reached yet) */
	HWT_INVALID     /* Invalid (rejected by tracking or false positive filter)*/
}HWT_TRACKSTATUS;


/* Enum indicating SW, HW, both when It detected first time.  */
typedef enum tagDET_TYPE
{
	DET_TYPE_NONE,       /* Default */
	DET_TYPE_HWNG_SWOK,  /* Only SW */
	DET_TYPE_HWOK_SWNG,  /* Only HW */
	DET_TYPE_HWOK_SWOK,  /* Both HW & SW */
}DET_TYPE;


/* Struct of H/W face detection results */
typedef struct tagHWTDtResult{
	INT32            nID_HW;      /* Detection ID (HW) */
	INT32            nConf;       /* Degree of confidence */
	INT32            nPose;       /* Face pose */
	INT32            nAngle;      /* Face angle */
	INT32            nSize;       /* Face size */
	POINT            ptCenter;    /* Face position (center) */
} HWTDtResult;



/* Struct of face tracking results */
typedef struct tagHWTResult{
	INT32                 nID_HW;       /* DetectionID (HW)  same as that of HWTDtResult. "-1" if HW FD doesn't detect a corresponding one */
	INT32                 nDstID_HW;    /* id of face detected HW  */
	INT32                 nDstID_IM;    /* id for intermittent SW  */
	INT32                 nID_Tr;       /* TrackingID: tracking ID that is kept over frames */
	INT32                 nConf_HW;     /* Confidence value from HW FD. "-1" if HW FD doesn't detect a corresponding one */
	INT32                 nConf_SW;     /* Confidence value from SW FD. "-1" if HW FD doesn't detect a corresponding one */
	INT32                 nConf_SW_FP;  /* FP confidence  */
	INT32                 nConf_SW_IM;  /* intermittent SW confidence  */
	INT32                 nConf_Tr;     /* Modified confidence. "-1" if tracker rejects the target */
	HWT_TRACKSTATUS       STATUS;       /* Tracking status  */
	INT32                 nHold;        /* Number of frames that the taget is not detected/tracked in */
	INT32                 nDelay;       /* Number of frames that the target is not accepted in */
	INT32                 nHistory;     /* Number of frames that the target is tracked in */
	INT32                 nPose;        /* Modified face pose */
	INT32                 nAngle;       /* Modified face angle */
	INT32                 nSize;        /* Modified face size */
	POINT                 ptCenter;     /* Modified face position (center) */
	DET_TYPE              dstType;      /* Enum indicating SW, HW, both when It detected first time.  */
} HWTResult ; 

/* Enum to describe a Filter Mode */
typedef enum tagFPFilterMode{
	FPF_MODE_Filtration,
	FPF_MODE_HWSalvation
}FPFilterMode;

typedef struct tagFPFILTER_HNDL{
	void *pUserData;

	BOOL (*FP_filter)(
		const HWTDtResult  FInfoHW,     // (I) Result of HW Detection
		HWTDtResult*       pFInfoSW,    // (O) Result of SW Detection
		void*              p_user_data, // (I) SW Filter private data
		const FPFilterMode FilterMode   // (I) Filter mode
	);

	INT32 (*FP_SWDetection)(
	       INT32*       pnFaceSW,       //(O) Faces detected by SW
	       HWTDtResult  aFaceInfoSW[],  //(O) DT Result of SW Detection
	const  INT32        nFaceMax,       //(I) Max faces from SW detection
	       void*        p_user_data     //(I) SW Detection private data
	);
} FPFILTER_HNDL;

#ifdef  __cplusplus
extern "C" {
#endif

	/**********************************************************/
	/* Create/delete handles                                  */
	/**********************************************************/
	/* Create/delete handle */
	HHWTRACKER         HWTracker_CreateHandle(INT32 nMaxTrackCount);
	INT32              HWTracker_DeleteHandle(HHWTRACKER hHWTR);
	/* Create/delete result handle */
	HHWTRACKERRESULT   HWTracker_CreateResultHandle(void);
	INT32              HWTracker_DeleteResultHandle(HHWTRACKERRESULT hHWTR);

	/**********************************************************/
	/* Set detection results of HW FD                         */
	/**********************************************************/

	/* Set the number of detected faces */
	INT32              HWTracker_SetDetectCount(HHWTRACKER hHWTR, INT32 nCount); 
	/* Set each face detection result */
	INT32              HWTracker_SetDetectResult(HHWTRACKER hHWTR,INT32 nI,HWTDtResult* psDtRes); 

	/**********************************************************/
	/* Execute tracking                                       */
	/**********************************************************/
	INT32              HWTracker_Execute(
		HHWTRACKER       hHWTR,
		HHWTRACKERRESULT hHWTresult,
		UINT32           unFrameNo,
		UINT32           unTimeStamp,
		INT32            nWidth,
		INT32            nHeight,
		VOID*            pFPFilter    // this value is just hand over to FPFilter
		);

	/**********************************************************/
	/*  Get Tracking results                                  */
	/**********************************************************/
	/* Get a number of results */
	INT32              HWTracker_GetConsistentCount(HHWTRACKERRESULT hHWTR, INT32 *pnCount);

	/* Get each result */
	INT32              HWTracker_GetConsistentResult(HHWTRACKERRESULT hHWTR,INT32 nID, HWTResult* psDtRes);


	/**********************************************************/
	/* Set tracking parameters                                */
	/**********************************************************/
	INT32              HWTracker_SetHoldCount(
		HHWTRACKER   hHWTracker,           /* (I/O) HWTRACKER handle */
		INT32        nHoldCount            /* (I)   HoldCount */
		);
	INT32              HWTracker_GetHoldCount(
		HHWTRACKER   hHWTracker,          /* (I/O) HWTRACKER handle */
		INT32*       pnHoldCount          /* (O)   HoldCount */
		);

	INT32              HWTracker_SetDelayCount(
		HHWTRACKER   hHWTracker,          /* (I/O) HWTRACKER handle */
		INT32        nDelayCount          /* (I)   DelayCount */
		);
	INT32              HWTracker_GetDelayCount(
		HHWTRACKER   hHWTracker,          /* (I/O) HWTRACKER handle */
		INT32*       pnDelayCount         /* (O)   DelayCount */
		);

	INT32              HWTracker_SetDeviceOrientation(
		HHWTRACKER   hHWTracker,             /*(I/O) HWTracker handle */
		INT32        nCurrDeviceOrientation  /*(I)   device rotation */
		);
	INT32              HWTracker_GetDeviceOrientation(
		HHWTRACKER   hHWTracker,              /*(I/O) HWTracker handle */
		INT32*       pnCurrDeviceOrientation  /*(O)   Device Orientation */
		);

	INT32              HWTracker_SetAngleDiffForStrictThresholds(
		HHWTRACKER   hHWTracker,                   /*(I/O) HWTracker handle */
		INT32        nAngleDiffForStrictThreshold  /*(I)   Angle Difference */
		);
	INT32              HWTracker_GetAngleDiffForStrictThresholds(
		HHWTRACKER   hHWTracker,                    /*(I/O) HWTracker handle */
		INT32*       pnAngleDiffForStrictThreshold  /*(O)   Angle Difference */
		);

	INT32              HWTracker_SetSwThresholds(
		HHWTRACKER   hHWTracker,           /*(I/O) HWTracker handle */
		INT32        nSwGoodFaceThreshold, /*(I)   sw threshold for Good Face */
		INT32        nSwThreshold,         /*(I)   sw threshold for Face */
		INT32        nSwTrackingThreshold  /*(I)   sw threshold for Face while tracking */
		);
	INT32              HWTracker_GetSwThresholds(
		HHWTRACKER   hHWTracker,            /*(I/O) HWTracker handle */
		INT32*       pnSwGoodFaceThreshold, /*(O)   sw threshold for Good Face */
		INT32*       pnSwThreshold,         /*(O)   sw threshold for Face */
		INT32*       pnSwTrackingThreshold  /*(O)   sw threshold for Face while tracking */
		);

	INT32              HWTracker_SetStrictSwThresholds(
		HHWTRACKER   hHWTracker,                 /*(I/O) HWTracker handle */
		INT32        nStrictSwGoodFaceThreshold, /*(I)   sw threshold for Good Face */
		INT32        nStrictSwThreshold,         /*(I)   sw threshold for Face */
		INT32        nStrictSwTrackingThreshold  /*(I)   sw threshold for Face while tracking */
		);
	INT32              HWTracker_GetStrictSwThresholds(
		HHWTRACKER   hHWTracker,                  /*(I/O) HWTracker handle */
		INT32*       pnStrictSwGoodFaceThreshold, /*(O)   sw threshold for Good Face */
		INT32*       pnStrictSwThreshold,         /*(O)   sw threshold for Face */
		INT32*       pnStrictSwTrackingThreshold  /*(O)   sw threshold for Face while tracking */
		);

	INT32              HWTracker_SetFaceLinkTolerance(
		HHWTRACKER   hHWTracker,                 /*(I/O) HWTracker handle */
		FLOAT32      nTrkTolMoveDist,            /*(I)   Tolerance for moving distance */
		FLOAT32      nTrkTolSizeRatioMin,        /*(I)   Minimum allowance of size ratio */
		FLOAT32      nTrkTolSizeRatioMax,        /*(I)   Maximum allowance of size ratio */
		FLOAT32      nTrkTolAngle                /*(I)   Tolerance for angle variation */
		);
	INT32              HWTracker_GetFaceLinkTolerance(
		HHWTRACKER   hHWTracker,                 /*(I/O) HWTracker handle */
		FLOAT32*     pnTrkTolMoveDist,            /*(I)   Tolerance for moving distance */
		FLOAT32*     pnTrkTolSizeRatioMin,        /*(I)   Minimum allowance of size ratio */
		FLOAT32*     pnTrkTolSizeRatioMax,        /*(I)   Maximum allowance of size ratio */
		FLOAT32*     pnTrkTolAngle                /*(I)   Tolerance for angle variation */
		);

	INT32              HWTracker_SetFilterExemptFaceLinkTolerance(
		HHWTRACKER   hHWTracker,                 /*(I/O) HWTracker handle */
		FLOAT32      nTrkTolMoveDist,            /*(I)   Tolerance for moving distance */
		FLOAT32      nTrkTolSizeRatioMin,        /*(I)   Minimum allowance of size ratio */
		FLOAT32      nTrkTolSizeRatioMax,        /*(I)   Maximum allowance of size ratio */
		FLOAT32      nTrkTolAngle                /*(I)   Tolerance for angle variation */
		);
	INT32              HWTracker_GetFilterExemptFaceLinkTolerance(
		HHWTRACKER   hHWTracker,                 /*(I/O) HWTracker handle */
		FLOAT32*     pnTrkTolMoveDist,            /*(I)   Tolerance for moving distance */
		FLOAT32*     pnTrkTolSizeRatioMin,        /*(I)   Minimum allowance of size ratio */
		FLOAT32*     pnTrkTolSizeRatioMax,        /*(I)   Maximum allowance of size ratio */
		FLOAT32*     pnTrkTolAngle                /*(I)   Tolerance for angle variation */
		);
	INT32 HWTracker_ResetTracking(HHWTRACKER hHWTracker);


#ifdef  __cplusplus
}
#endif

#endif /* HWTRACKERAPI_H__ */
