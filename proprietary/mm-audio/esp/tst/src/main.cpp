/*============================================================================
* Copyright (c) 2018 Qualcomm Technologies, Inc.                       *
* All Rights Reserved.                                                       *
* Confidential and Proprietary - Qualcomm Technologies, Inc.                 *
* ===========================================================================*/

#include <iostream>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <esp_api.h>

using namespace std;

//FILE *energyCsv = NULL;

static int
ParseOptions(int argc, char *argv[], char **pinFileName, char **poutFileName, int *samp_rate, int *frame_size)
{
    int idx;
    int waveInpFile = 0;
    int waveOutFile = 0;

    for (idx = 1; idx < argc; idx++)
    {
        if ('-' == argv[idx][0])
        {
            if ('s' == tolower(argv[idx][1]))
            {
                // Look for integer in next argument
                idx++;
                if (idx == argc)
                {
                    return (-1); // Invalid arguments
                }
                sscanf(argv[idx], "%d", samp_rate);

            }

            else if ('f' == tolower(argv[idx][1]))
            {

                idx++;
                if (idx == argc)
                {
                    return (-1); // Invalid arguments
                }
                sscanf(argv[idx], "%d", frame_size);
            }

        }
        else if (0 == waveInpFile)
        {
            waveInpFile = 1;
            *pinFileName = argv[idx];
        }
        else if (0 == waveOutFile)
        {
            waveOutFile = 1;
            *poutFileName = argv[idx];
        }
    }

    if ((0 == waveInpFile))
    {
        return (-1);    // Not enough arguments
    }

    return (0);
}

void Usage(void)
{
    fprintf(stderr,
        "Usage: esp.exe <input PCM file> <output csv file> [-s <sampling_rate 16000>] [-f <frame size in ms 16>]\n");
}


int main(int argc, char *argv[])
{

    // Initialization
    int freqSamp = 0; // Supports only 16000
    int frameSize_ms = 0; // Supports only 8ms, 15 ms and 16ms
    //int err;
    // Pick one of the below Input Files
    FILE *infile = NULL;
    char *inFilename = NULL, *outFilename = NULL;

    if (ParseOptions(argc, argv, &inFilename, &outFilename, &freqSamp, &frameSize_ms) < 0)
    {
        Usage();
        exit(-1);
    }

     infile = fopen(inFilename,"rb");   // Noise only
    //FILE * infile;

    //err = fopen_s(&infile, "Mic_16bit.pcm", "rb"); // Raw mixdown recorded on Alexa enabled device
    //err = fopen_s(&infile, "Noise.pcm", "rb");
    //err = fopen_s(&infile, "MixDown.pcm", "rb");

    cout << "The expected input file is 16 bits 1 Channel Signed PCM file" << endl;
    cout << "The Main.cpp function creates and calls the two instances - VADClass and FrameEnergyClass" << endl;

    // Create object pointers
  /*  VADClass *m_VAD;
    FrameEnergyClass *m_FrameEnergyCompute;

    // Local Variables
    bool GVAD = false;
    Word64 currentFrameEnergy = 0;
    unsigned int frameNum = 0;
    int frameSize;
    frameSize = (freqSamp/1000) * frameSize_ms;*/

    short procBuff[ESPIN_FRAME_SIZE];

    // Setup objects based on frameSize
 /*   m_VAD = new VADClass (frameSize);
    m_FrameEnergyCompute = new FrameEnergyClass (frameSize);*/

    EspStatusType rc;

    void *esp_obj_ptr = NULL;
    int memSize;
    void *pLib = NULL;


    rc = esp_init(&esp_obj_ptr, frameSize_ms, freqSamp, &memSize);


    if (rc != ESP_SUCCESS)
    {
        printf("Error in esp_init error code rc=%d\n", rc);
    }

    pLib = esp_obj_ptr;

    // Create output csv file
    //energyCsv = fopen("energyVal.csv", "w+");

    //fprintf(energyCsv, "Frame Number, VoiceEnergy, AmbientEnergy, VADFlag, CurrentFrameEnergy\n");

    // Process file in frameSize chunks
    cout << "Processing begins..." << endl;
    while (!feof(infile))
    {

        size_t len_read;

        len_read = fread(procBuff,2, ESPIN_FRAME_SIZE, infile);

        esp_process(esp_obj_ptr, procBuff);

       // pLib->frameNum++;

        // Call VAD Process
    //  pLib->vadObjPtr->process(procBuff, pLib->GVAD, pLib->currentFrameEnergy);

        // Call Frame Energy Compute Process
    //    pLib->frameEnergyComputeObjPtr->process(pLib->GVAD, pLib->currentFrameEnergy);

        // m_FrameEnergyCompute->getVoicedEnergy() returns WakeWord Energy
        // m_FrameEnergyCompute->getAmbientEnergy() returns Ambient Energy
    //    fprintf(energyCsv, "%u, %d, %d, %d, %lld\n", pLib->frameNum, pLib->frameEnergyComputeObjPtr->getVoicedEnergy(), pLib->frameEnergyComputeObjPtr->getAmbientEnergy(), pLib->GVAD, pLib->currentFrameEnergy);
    }

    esp_energy_levels_t *param_buf_ptr;
    int param_sz;
    param_buf_ptr = (esp_energy_levels_t *)malloc(sizeof(esp_energy_levels_t));

    rc =  esp_get_param(esp_obj_ptr, (char *)param_buf_ptr, ESP_ENERGY_LEVELS_PARAM, sizeof(esp_energy_levels_t), &param_sz);

    if (rc != 0)
    {
        printf("Error in GET_PARAM %d", rc);
    }
    else
    {
        printf("Voice Energy %d Noise Energy %d\n", param_buf_ptr->voiceEnergy, param_buf_ptr->ambientNoiseEnergy);
    }

    esp_deinit(pLib);

    fclose(infile);
   // fclose(energyCsv);
    cout << "Processing Completed!" << endl;
    return 0;
}
