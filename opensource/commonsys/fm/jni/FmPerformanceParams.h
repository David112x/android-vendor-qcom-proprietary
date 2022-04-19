/*
 * Copyright (c) 2014, The Linux Foundation. All rights reserved.

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *        * Redistributions of source code must retain the above copyright
 *            notice, this list of conditions and the following disclaimer.
 *        * Redistributions in binary form must reproduce the above copyright
 *            notice, this list of conditions and the following disclaimer in the
 *            documentation and/or other materials provided with the distribution.
 *        * Neither the name of The Linux Foundation nor
 *            the names of its contributors may be used to endorse or promote
 *            products derived from this software without specific prior written
 *            permission.

 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT ARE DISCLAIMED.    IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __FM_PERFORMANCE_PARAMS_H__
#define __FM_PERFORMANCE_PARAMS_H__

#include "FmConst.h"

class FmPerformanceParams
{
      private:
      public:
          signed char SetAfRmssiTh(UINT fd, unsigned short th);
          signed char SetAfRmssiSamplesCnt(UINT fd, unsigned char cnt);
          signed char SetGoodChannelRmssiTh(UINT fd, signed char th);
          signed char SetSrchAlgoType(UINT fd, unsigned char algo);
          signed char SetSinrFirstStage(UINT fd, signed char th);
          signed char SetRmssiFirstStage(UINT fd, signed char th);
          signed char SetCf0Th12(UINT fd, int th);
          signed char SetSinrSamplesCnt(UINT fd, unsigned char cnt);
          signed char SetIntfLowTh(UINT fd, unsigned char th);
          signed char SetIntfHighTh(UINT fd, unsigned char th);
          signed char SetSinrFinalStage(UINT fd, signed char th);
          signed char SetHybridSrchList(UINT fd, unsigned int *freqs, signed char *sinrs, unsigned int n);

          signed char GetAfRmssiTh(UINT fd, unsigned short &th);
          signed char GetAfRmssiSamplesCnt(UINT fd, unsigned char &cnt);
          signed char GetGoodChannelRmssiTh(UINT fd, signed char &th);
          signed char GetSrchAlgoType(UINT fd, unsigned char &algo);
          signed char GetSinrFirstStage(UINT fd, signed char &th);
          signed char GetRmssiFirstStage(UINT fd, signed char &th);
          signed char GetCf0Th12(UINT fd, int &th);
          signed char GetSinrSamplesCnt(UINT fd, unsigned char &cnt);
          signed char GetIntfLowTh(UINT fd, unsigned char &th);
          signed char GetIntfHighTh(UINT fd, unsigned char &th);
          signed char GetIntfDet(UINT fd, unsigned char &th);
          signed char GetSinrFinalStage(UINT fd, signed char &th);
};

#endif //__FM_PERFORMANCE_PARAMS_H__
