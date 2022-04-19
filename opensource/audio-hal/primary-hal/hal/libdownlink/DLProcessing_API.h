#ifndef DLProcessing_API_H
#define DLProcessing_API_H

/* Source Code Version */
#define DL_MINI_MAJOR_VERSION       1
#define DL_MINI_MINOR_VERSION       1
#define DL_MINI_REVISION_VERSION    0
#define FRAME_20ms_48	960

typedef void* DL_ProcessingHandle;

/* Set 0 represent equalizer and limiter are off, Set 1 represent equalizer and limiter are on. Default value is 1.*/
#define EQUALIZER_FLAG     1
/* Set 0 represent automatic level control is off, Set 1 represent automatic level control is on. Default value is 1.*/
#define ALC_FLAG           2
/* Set the value of speaker volume, the value range is 0~128(not sure). Default value is 128.*/
#define SPEAKER_VOLUME     3
/* Set the value of automatic reference level, the value range is 0~10 (not sure). Default value is 0.*/
#define ALC_REF_LEVEL      4
/* Set the idx value of qualizer table should be used in downlink, the value range is 0~3(not sure). Default value is 0.*/
#define EQUALIZER_TABLE    5
/* Set the DTState gotten form AEC process. Default value is 0.*/
#define DT_STATE           6


void DLProcessing_FrameProcess
(
    short int*   DL_input,
    short int*   DL_output,
    DL_ProcessingHandle     h_DL_Processing
);

void DLProcessing_Create_Instance
(
    void    **pDL
);
void DLProcessing_Delete(DL_ProcessingHandle h_DL_Processing);
void DLProcessing_SetParam(int param_id, int param_value, DL_ProcessingHandle h_DL_Processing);
void DLProcessing_GetParam(int param_id, int *p_param_value, DL_ProcessingHandle h_DL_Processing);
/* Get the version of DL library */
unsigned int DLProcessing_GetLibVersion();
/****************************************************************************/
#endif

