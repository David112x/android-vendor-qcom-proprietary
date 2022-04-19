#ifndef AecProcessing_API_H
#define AecProcessing_API_H

/* Source Code Version */
#define MIDI_MAJOR_VERSION       3
#define MIDI_MINOR_VERSION       1
#define MIDI_REVISION_VERSION    0

#define PRINTF(x)

typedef void* AEC_ProcessingHandle;

#define DEBUG_MIC_ERLE     (0x1)
#define DEBUG_BESTMIC_VOTE (0x2)
#define DEBUG_TALK_STATE   (0x4)

typedef enum
{
    FULL_BAND   = 0,
    NARROWBAND   = 1,
    WIDE_7KHZ   = 2,
    SUPERWIDE_14KHZ  = 3
} E_BandLimitMode;

typedef enum
{
    NO_TALK = 0,
    NEAR_END_ONLY, 
    FAR_END_ONLY, 
    BOTH_ENDS_TALK 
} E_TalkState;


typedef enum
{
    ADAPTIVE_FILTER_FLAG            = (1<<0),
    NOISE_FILL_FLAG					= (1<<1),
    NOISE_REDUCTION_FLAG            = (1<<2),
    ECHO_SUPPRESSION_FLAG           = (1<<3),
    AUTO_GAIN_CONTROL_FLAG          = (1<<4),
	POST_FILTER_FLAG				= (1<<5),
	SPEECH_DETECTOR_DEBUG_FLAG		= (1<<6)

} E_EnableMask;

//---------------------------------------------------------

#define DEF_ENABLE_MASK    ( ADAPTIVE_FILTER_FLAG  | POST_FILTER_FLAG | NOISE_REDUCTION_FLAG | ECHO_SUPPRESSION_FLAG | AUTO_GAIN_CONTROL_FLAG | NOISE_FILL_FLAG  )
             


AEC_ProcessingHandle AecProcessing_Create(void);                 
void AecProcessing_Delete(AEC_ProcessingHandle h_AEC_Processing);
void AecProcessing_Reset(AEC_ProcessingHandle h_AEC_Processing);


 void AecProcessing_SetEnableFlag (E_EnableMask EnableMask,  int EnableFlag, AEC_ProcessingHandle h_AEC_Processing);
 void AecProcessing_SetAllEnableFlags (unsigned long EnableMask, AEC_ProcessingHandle h_AEC_Processing);
 int AecProcessing_GetEnableFlag (E_EnableMask EnableMask, AEC_ProcessingHandle h_AEC_Processing);
 int AecProcessing_GetFarEndTalkState         (AEC_ProcessingHandle h_AEC_Processing);       
 int AecProcessing_GetSpeechFlag   (AEC_ProcessingHandle h_AEC_Processing);        
/* Get the version of AEC library */
unsigned int AecProcessing_GetLibVersion();

void  AecProcessing_SetBandLimitMode (E_BandLimitMode BandLimitMode,  
                                    AEC_ProcessingHandle   h_AEC_Processing);


void AecProcessing_SetAutoGainControlGain (float gain,      
                                  AEC_ProcessingHandle  h_AEC_Processing);
void AecProcessing_SetDebugMask(unsigned int DebugMask,
    AEC_ProcessingHandle  h_AEC_Processing);


void AecProcessing_EnableAEC  (int flag, AEC_ProcessingHandle h_AEC_Processing); 
void AecProcessing_EnableAutoGainControl  (int flag, AEC_ProcessingHandle h_AEC_Processing); 
void AecProcessing_EnableEchoSuppression  (int flag, AEC_ProcessingHandle h_AEC_Processing); 
void AecProcessing_EnableNoiseSuppression  (int flag, AEC_ProcessingHandle h_AEC_Processing); 
void AecProcessing_EnableNoiseFill (int flag, AEC_ProcessingHandle h_AEC_Processing);

E_TalkState AecProcessing_GetDTState(AEC_ProcessingHandle h_AEC_Processing);

void AecProcessing_FrameProcess
(
	short int*        mic[],
	short int*        SpeakerRef,
	short int*        RefMic,
	short int*        Out_MicSelection,
    AEC_ProcessingHandle     h_AEC_Processing
);



void AecProcessing_Init (void *);

float* AecProcessing_GetSubband_BackgroundNoisePower(AEC_ProcessingHandle h_AEC_Processing, int MicIndex);

void AecProcessing_ExtendScalarParameters_ToVirtualMics(short int OneScalarParam_FromRealMic[16][640], float *alpha);

float *GetRoatatingTable_API(AEC_ProcessingHandle h_AEC_Processing);

void AecProcessing_Create_Instance
(
void         **pAEC
);
/****************************************************************************/
#endif

