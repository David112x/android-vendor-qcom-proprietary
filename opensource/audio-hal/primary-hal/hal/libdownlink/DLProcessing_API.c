
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "DLProcessing_API.h"

#define AEC_FRAME_SIZE          (960) 
#define NUMBER_OF_MICS          (10)
#define NUMBER_OF_VIRTUAL_MICS  (6)

typedef struct 
{
    float Table1[100];
    float Table2[100];
    float Table3[100];
} S_AecProcessing;


void DLProcessing_Create_Instance(
void         **hAEC
)
{
    *hAEC = malloc(sizeof(S_AecProcessing));

    assert(*hAEC != NULL);
}

void DLProcessing_FrameProcess
(
    short int*   DL_input,
    short int*   DL_output,
    DL_ProcessingHandle     h_DL_Processing
)
{
	printf("DLProcessing_FrameProcess\n");
    int i;
    S_AecProcessing* This = (S_AecProcessing*)h_DL_Processing;
    assert(This != NULL);

    for(i=0;i<AEC_FRAME_SIZE/2;i++)
    {
       DL_output[i] = (short int)DL_input[i];
    }
}


void DLProcessing_Delete(DL_ProcessingHandle h_DL_Processing)
{
	printf("delet\n");
}



