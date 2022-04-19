/**
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**/

#include <QtiMutex.h>
#include <DataModuleMutex.h>

qtimutex::QtiRecursiveMutex infoTblMutex;
qtimutex::QtiSharedMutex dsiCbTblMutex;
qtimutex::QtiSharedMutex lqeCbTblMutex;
qtimutex::QtiSharedMutex lqeMutex;

extern "C" void qcril_data_mutex_lock(unsigned int mutex)
{
  switch( mutex )
  {
    case INFO_TBL_MUTEX:
      return infoTblMutex.lock();
    case DSI_CB_TBL_MUTEX:
      return dsiCbTblMutex.lock();
    case LQE_CB_TBL_MUTEX:
      return lqeCbTblMutex.lock();
    case LQE_MUTEX:
      return lqeMutex.lock();
    default:
      break;
  }
};

extern "C" void qcril_data_mutex_unlock(unsigned int mutex)
{
  switch( mutex )
  {
    case INFO_TBL_MUTEX:
      return infoTblMutex.unlock();
    case DSI_CB_TBL_MUTEX:
      return dsiCbTblMutex.unlock();
    case LQE_CB_TBL_MUTEX:
      return lqeCbTblMutex.unlock();
    case LQE_MUTEX:
      return lqeMutex.unlock();
    default:
      break;
  }
};


extern "C" boolean qcril_data_mutex_trylock(unsigned int mutex)
{
  switch( mutex )
  {
    case INFO_TBL_MUTEX:
      return infoTblMutex.try_lock();
    case DSI_CB_TBL_MUTEX:
      return dsiCbTblMutex.try_lock();
    case LQE_CB_TBL_MUTEX:
      return lqeCbTblMutex.try_lock();
    case LQE_MUTEX:
      return lqeMutex.try_lock();
    default:
      return TRUE;
  }
};
