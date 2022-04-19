/******************************************************************************
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

#include "eSEClient.h"
#include <log/log.h>
#include <dirent.h>
#include <stdlib.h>
#include <IChannel.h>
#include <pthread.h>
#include <JcDnld.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <LsClient.h>
#include "gpqese-api.h"
#include "phEseStatus.h"

void seteSEClientState(uint8_t state);

IChannel_t Ch;
se_extns_entry se_intf;
void* eSEClientUpdate_ThreadHandler(void* data);
void* eSEClientUpdate_Thread(void* data);
SESTATUS ESE_ChannelInit(IChannel *ch);
SESTATUS handleJcopOsDownload();
void* LSUpdate_Thread(void* data);
uint8_t performLSUpdate();
SESTATUS initializeEse(phNxpEse_initMode mode, SEDomainID Id);
SESTATUS eSEUpdate_SeqHandler();
int16_t SE_Open()
{
    return SESTATUS_SUCCESS;
}

void SE_Reset()
{

}

bool SE_Raw_Transmit(uint8_t* xmitBuffer, int32_t xmitBufferSize, uint8_t* recvBuffer,
                     int32_t recvBufferMaxSize, int32_t& recvBufferActualSize, int32_t timeoutMillisec)
{
    if ((NULL == xmitBuffer) || (xmitBufferSize == 0) || (NULL == recvBuffer)) return false;

    timeoutMillisec++;

    prepare_and_do_transceive(xmitBuffer, xmitBufferSize, recvBuffer, recvBufferMaxSize, true, &recvBufferActualSize);

    ALOGE("%s: size = 0x%x recv[0] = 0x%x", __FUNCTION__, recvBufferActualSize, recvBuffer[0]);
    if (recvBufferActualSize < MIN_RSP_LEN)
      return false;
    else
      return true;
}

bool SE_Transmit(uint8_t* xmitBuffer, int32_t xmitBufferSize, uint8_t* recvBuffer,
                     int32_t recvBufferMaxSize, int32_t& recvBufferActualSize, int32_t timeoutMillisec)
{
    if ((NULL == xmitBuffer) || (xmitBufferSize == 0) || (NULL == recvBuffer)) return false;

    timeoutMillisec++;

    prepare_and_do_transceive(xmitBuffer, xmitBufferSize, recvBuffer, recvBufferMaxSize, false, &recvBufferActualSize);

    ALOGE("%s: size = 0x%x recv[0] = 0x%x", __FUNCTION__, recvBufferActualSize, recvBuffer[0]);
    if (recvBufferActualSize < MIN_RSP_LEN)
      return false;
    else
      return true;
}

void SE_JcopDownLoadReset()
{
    ALOGD("%s: ", __FUNCTION__);
    resetJcopUpdate();
}

bool SE_Close(int16_t mHandle)
{
    if(mHandle != 0)
      return true;
    else
      return false;
}
uint8_t SE_getInterfaceInfo()
{
  return INTF_SE;
}

/***************************************************************************
**
** Function:        checkEseClientUpdate
**
** Description:     Check the initial condition
                    and interafce for eSE Client update for LS and JCOP download
**
** Returns:         SUCCESS of ok
**
*******************************************************************************/
void checkEseClientUpdate()
{
  ALOGD("%s enter:  ", __func__);
  checkeSEClientRequired(ESE_INTF_SPI);
  se_intf.isJcopUpdateRequired = getJcopUpdateRequired();
  se_intf.isLSUpdateRequired = getLsUpdateRequired();
  se_intf.sJcopUpdateIntferface = getJcopUpdateIntf();
  se_intf.sLsUpdateIntferface = getLsUpdateIntf();
  if((se_intf.isJcopUpdateRequired && se_intf.sJcopUpdateIntferface)||
   (se_intf.isLSUpdateRequired && se_intf.sLsUpdateIntferface))
    seteSEClientState(ESE_UPDATE_STARTED);
}

/***************************************************************************
**
** Function:        perform_eSEClientUpdate
**
** Description:     Perform LS and JCOP download during hal service init
**
** Returns:         SUCCESS / SESTATUS_FAILED
**
*******************************************************************************/
SESTATUS perform_eSEClientUpdate() {
  SESTATUS status = SESTATUS_FAILED;
  ALOGD("%s enter:  ", __func__);
  if(getJcopUpdateRequired()) {
    if(se_intf.sJcopUpdateIntferface == ESE_INTF_NFC) {
      seteSEClientState(ESE_JCOP_UPDATE_REQUIRED);
      return SESTATUS_SUCCESS;
    }
    else if(se_intf.sJcopUpdateIntferface == ESE_INTF_SPI) {
      seteSEClientState(ESE_JCOP_UPDATE_REQUIRED);
    }
  }

  if((ESE_JCOP_UPDATE_REQUIRED != ese_update) && (getLsUpdateRequired())) {
    if(se_intf.sLsUpdateIntferface == ESE_INTF_NFC) {
      seteSEClientState(ESE_LS_UPDATE_REQUIRED);
      return SESTATUS_SUCCESS;
    }
    else if(se_intf.sLsUpdateIntferface == ESE_INTF_SPI) {
      seteSEClientState(ESE_LS_UPDATE_REQUIRED);
    }
  }

  if((ese_update == ESE_JCOP_UPDATE_REQUIRED) ||
    (ese_update == ESE_LS_UPDATE_REQUIRED))
    eSEClientUpdate_Thread();
  return status;
}

SESTATUS ESE_ChannelInit(IChannel *ch)
{
  ch->open = SE_Open;
  ch->close = SE_Close;
  ch->transceive = SE_Transmit;
  ch->transceiveRaw = SE_Raw_Transmit;
  ch->doeSE_Reset = SE_Reset;
  ch->doeSE_JcopDownLoadReset = SE_JcopDownLoadReset;
  ch->getInterfaceInfo = SE_getInterfaceInfo;
  return SESTATUS_SUCCESS;
}

/*******************************************************************************
**
** Function:        eSEClientUpdate_Thread
**
** Description:     Perform eSE update
**
** Returns:         SUCCESS of ok
**
*******************************************************************************/
void eSEClientUpdate_Thread()
{
  SESTATUS status = SESTATUS_FAILED;
  pthread_t thread;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  if (pthread_create(&thread, &attr, &eSEClientUpdate_ThreadHandler, NULL) != 0) {
    ALOGD("Thread creation failed");
    status = SESTATUS_FAILED;
  } else {
    status = SESTATUS_SUCCESS;
    ALOGD("Thread creation success");
  }
  pthread_attr_destroy(&attr);
}
/*******************************************************************************
**
** Function:        eSEClientUpdate_ThreadHandler
**
** Description:     Perform JCOP Download
**
** Returns:         SUCCESS of ok
**
*******************************************************************************/
void* eSEClientUpdate_ThreadHandler(void* data) {
  (void)data;
  ALOGD("%s Enter\n", __func__);
  eSEUpdate_SeqHandler();
  ALOGD("%s Exit eSEClientUpdate_Thread\n", __func__);
  return NULL;
}

/*******************************************************************************
**
** Function:        handleJcopOsDownload
**
** Description:     Perform JCOP update
**
** Returns:         SUCCESS of ok
**
*******************************************************************************/
SESTATUS handleJcopOsDownload()
{
  SESTATUS status = SESTATUS_FAILED;
  uint8_t retstat;
  status = initializeEse(ESE_MODE_OSU, ESE);
  if(status == SESTATUS_SUCCESS) {
    retstat = JCDNLD_Init(&Ch);
    if(retstat != STATUS_SUCCESS) {
      ALOGE("%s: JCDND initialization failed", __FUNCTION__);
      ese_close();
      return status;
    } else {
      retstat = JCDNLD_StartDownload();
      if(retstat != SESTATUS_SUCCESS) {
        ALOGE("%s: JCDNLD_StartDownload failed", __FUNCTION__);
      }
    }
    JCDNLD_DeInit();
    ese_close();
  }
  status = SESTATUS_SUCCESS;
  return status;
}

/*******************************************************************************
**
** Function:        performLSUpdate
**
** Description:     Perform LS update
**
** Returns:         SUCCESS of ok
**
*******************************************************************************/
uint8_t performLSUpdate()
{
  uint8_t status = SESTATUS_FAILED;
  ALOGE("%s:On eSE domain ", __FUNCTION__);

  status = initializeEse(ESE_MODE_NORMAL, ESE);
  if(status == SESTATUS_SUCCESS) {
    seteSEClientState(ESE_UPDATE_STARTED);
    status = performLSDownload(&Ch);
    ese_close();
  }
  return status;
}

/*******************************************************************************
**
** Function:        initializeEse
**
** Description:     Open & Initialize libese
**
** Returns:         SUCCESS of ok
**
*******************************************************************************/
SESTATUS initializeEse(phNxpEse_initMode mode, SEDomainID Id)
{
  (void)Id;
  uint8_t retstat;
  SESTATUS status = SESTATUS_FAILED;

  ALOGE("%s: Mode = %d", __FUNCTION__, mode);
  retstat = ese_open();
  if (retstat != ESESTATUS_SUCCESS) {
    ALOGE("%s: ese open failed", __FUNCTION__);
    return status;
  }
  ALOGD("%s: ese open success", __FUNCTION__);

  /* hard reset required in case of JCOP os download */
  retstat = resetJcopUpdate();
  if(retstat != ESESTATUS_SUCCESS) {
    ALOGE("%s: ese reset failed", __FUNCTION__);
    ese_close();
    return status;
  }
  ALOGD("%s: hard reset & getATR success", __FUNCTION__);

  ESE_ChannelInit(&Ch);
  status = SESTATUS_SUCCESS;
  return status;
}

/*******************************************************************************
**
** Function:        seteSEClientState
**
** Description:     Function to set the eSEUpdate state
**
** Returns:         SUCCESS of ok
**
*******************************************************************************/
void seteSEClientState(uint8_t state)
{
  ALOGE("%s: State = %d", __FUNCTION__, state);
  ese_update = (ese_update_state_t)state;
}

/*******************************************************************************
**
** Function:        sendeSEUpdateState
**
** Description:     Notify NFC HAL LS / JCOP download state
**
** Returns:         SUCCESS of ok
**
*******************************************************************************/
void sendeSEUpdateState(uint8_t state)
{
  ALOGE("%s: State = %d", __FUNCTION__, state);
}

/*******************************************************************************
**
** Function:        eSEUpdate_SeqHandler
**
** Description:     ESE client update handler
**
** Returns:         SUCCESS of ok
**
*******************************************************************************/
SESTATUS eSEUpdate_SeqHandler()
{
    switch(ese_update)
    {
      case ESE_UPDATE_STARTED:
        break;
      case ESE_JCOP_UPDATE_REQUIRED:
        ALOGE("%s: ESE_JCOP_UPDATE_REQUIRED", __FUNCTION__);
        if(se_intf.isJcopUpdateRequired) {
          if(se_intf.sJcopUpdateIntferface == ESE_INTF_SPI) {
            handleJcopOsDownload();
            sendeSEUpdateState(ESE_JCOP_UPDATE_COMPLETED);
          }
          else if(se_intf.sJcopUpdateIntferface == ESE_INTF_NFC) {
            return SESTATUS_SUCCESS;
          }
        }
      case ESE_JCOP_UPDATE_COMPLETED:
        ALOGE("%s: ESE_JCOP_UPDATE_COMPLETED", __FUNCTION__);
        /*
         * fall through as we need to check for loader service update as well
         * JCOP download finished, check for loader service update required
         */
      case ESE_LS_UPDATE_REQUIRED:
        if(se_intf.isLSUpdateRequired) {
          if(se_intf.sLsUpdateIntferface == ESE_INTF_SPI) {
            performLSUpdate();
            sendeSEUpdateState(ESE_LS_UPDATE_COMPLETED);
          }
          else if(se_intf.sLsUpdateIntferface == ESE_INTF_NFC) {
            seteSEClientState(ESE_LS_UPDATE_REQUIRED);
            return SESTATUS_SUCCESS;
          }
        }
        ALOGE("%s: ESE_LS_UPDATE_REQUIRED", __FUNCTION__);
      case ESE_LS_UPDATE_COMPLETED:
        ALOGE("%s: ESE_LS_UPDATE_COMPLETED", __FUNCTION__);
      case ESE_UPDATE_COMPLETED:
        seteSEClientState(ESE_UPDATE_COMPLETED);
        sendeSEUpdateState(ESE_UPDATE_COMPLETED);
        /*
         * JCOP OS and Loader service update completed,
         * there is a possibility that we received init()
         * from OMAPI client while download was in progress.
         * Invoking below API to initialize eSE interface,
         * if it's still pending otherwise it will be ignored.
         */
        initSEService();
        ALOGE("%s: ESE_UPDATE_COMPLETED", __FUNCTION__);
      break;
    }
    return SESTATUS_SUCCESS;
}
