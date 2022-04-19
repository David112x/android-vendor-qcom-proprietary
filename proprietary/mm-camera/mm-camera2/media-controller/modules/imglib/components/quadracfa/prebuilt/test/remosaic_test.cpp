/**********************************************************************
*  Copyright (c) 2016-2018 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include <math.h>
#include "remosaic_test.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define RT_UNUSED(x) (void)(x)

#include "libremosaiclib.h"

int main()
{
  RT_LOGE("inside quadtest new32bit version 15 july");
  FILE* fp;

  int m_iWidth = 4624;
  int m_iHeight = 3480;

  fp=fopen(QCAMERA_DUMP_FRM_LOCATION"quadracfa_input__undefined_time__4624x3520_2_0_11"
    ".raw","rb");
  if (!fp) {
    return 0;
  }
  unsigned char *m_pInputBuffer = new unsigned char[m_iWidth * m_iHeight * 5 / 4];
  fread(m_pInputBuffer,sizeof(unsigned char),m_iWidth*m_iHeight*5/4,fp);
  fclose(fp);

  uint16_t* m_pOutputBuffer = new uint16_t[m_iWidth*m_iHeight];

  int wb_grgain = 2.010261f*1024;
  int wb_rgain = 1.000000f*1024;
  int wb_bgain = 1.600469f*1024;
  int wb_gbgain = 1.0f*1024;
  int analog_gain = 1;

  int pedestal = 64;

  int mode = 0;
  int* xtalk_gain_map = NULL;

  int port0 = 0;
  int port1 = 0;
  int port2 = 0;
  int port3 = 0;

  int lib_status = 0;

  // run function
  Run4PixelProc((unsigned char*)m_pInputBuffer,
    (unsigned short*) m_pOutputBuffer,
    m_iWidth,
    m_iHeight,
    wb_grgain,
    wb_rgain,
    wb_bgain,
    wb_gbgain,
    analog_gain,
    pedestal,
    0,
    xtalk_gain_map,
    mode,
    &lib_status,
    &port0,
    &port1,
    &port2,
    &port3);

  //Run4PixelProc( m_pInputBuffer,(unsigned short*) m_pOutputBuffer, m_iWidth,
  //  m_iHeight,
  //  wb_grgain, wb_rgain,  wb_bgain, wb_gbgain, analog_gain, &port0);

  FILE* fp_out5;
  fp_out5=fopen(QCAMERA_DUMP_FRM_LOCATION"quadracfa_OUTPUT__undefined_time__4624x3520_"
    "2_0_11.raw","wb");
  if (!fp_out5) {
    return 0;
  }
  fwrite(m_pOutputBuffer,sizeof(uint16),m_iWidth*m_iHeight,fp_out5);
  fclose(fp_out5);
  delete [] m_pOutputBuffer;

}

