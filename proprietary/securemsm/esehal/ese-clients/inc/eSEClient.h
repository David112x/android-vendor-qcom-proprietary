/*******************************************************************************
 *
 *  Copyright 2018 NXP
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/

#if(ESE_CLIENT_LIB_PRESENT == true)
#include "eSEClientIntf.h"
#endif

#ifndef ESE_UPDATE_2_H_
#define ESE_UPDATE_2_H_

//status code
#define MIN_RSP_LEN  2

typedef enum {
  ESE_MODE_NORMAL = 0, /*!< All wired transaction other OSU */
  ESE_MODE_OSU         /*!< Jcop Os update mode */
} phNxpEse_initMode;

typedef enum {
  ESE = 0,
  EUICC = 1,
} SEDomainID;

#if(ESE_CLIENT_LIB_PRESENT == true)
SESTATUS perform_eSEClientUpdate();
void checkEseClientUpdate();
void eSEClientUpdate_Thread();
void seteSEClientState(uint8_t state);
#else
typedef enum {
  ESE_UPDATE_COMPLETED = 0
}ese_update_state_t;
#endif

extern ese_update_state_t ese_update;
#endif /* ESE_UPDATE_2_H_ */
