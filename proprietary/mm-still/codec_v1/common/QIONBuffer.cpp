/**********************************************************************************
* Copyright (c) 2012-2015, 2019 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                             *
**********************************************************************************/

#include "QICommon.h"
#include "QIONBuffer.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include <linux/msm_ion.h>
#if TARGET_ION_ABI_VERSION >= 2
#include <linux/ion.h>
#include <ion/ion.h>
#include <linux/dma-buf.h>
#endif

#define PAGE_SZ 4096

/*===========================================================================
 * Function: OpenClient
 *
 * Description: open the ion client
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *
 * Notes: none
 *==========================================================================*/
int QIONBuffer::OpenClient()
{
#ifndef TARGET_ION_ABI_VERSION
  uint32_t flags = (mCached) ? O_RDONLY : (O_RDONLY | O_SYNC);

  mIONFd = open("/dev/ion", flags);
#else
  mIONFd = ion_open();
#endif
  if(mIONFd < 0) {
    QIDBG_ERROR("%s:%d] Ion open failed %s\n",
      __func__, __LINE__, strerror(errno));
    return QI_ERR_GENERAL;
  }
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: DoMmap
 *
 * Description: MMap the buffers
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *
 * Notes: none
 *==========================================================================*/
int QIONBuffer::DoMmap()
{
  int lrc = 0;
  struct ion_handle_data lHandleData;
#ifndef TARGET_ION_ABI_VERSION
  mAllocIon.flags = (mCached) ? ION_FLAG_CACHED : 0;
  mAllocIon.heap_id_mask = (0x1 << ION_IOMMU_HEAP_ID);
  mAllocIon.align = PAGE_SZ;
#else
  mAllocIon.flags = (mCached) ? ION_FLAG_CACHED : 0;
  mAllocIon.heap_id_mask = (0x1 << ION_SYSTEM_HEAP_ID);
#endif

  /* to make it page size aligned */
  mAllocIon.len = (mLength + (PAGE_SZ-1)) & (~(PAGE_SZ-1));
  QIDBG_LOW("%s:%d] ionfd %d len %d", __func__, __LINE__,
    mIONFd, mAllocIon.len);
#ifndef TARGET_ION_ABI_VERSION
  lrc = ioctl(mIONFd, ION_IOC_ALLOC, &mAllocIon);
  if (lrc < 0) {
    QIDBG_ERROR("%s:%d] Ion alloc failed %s\n",
      __func__, __LINE__, strerror(errno));
    goto ION_ALLOC_FAILED;
  }

  mFdIonMap.handle = mAllocIon.handle;
  lrc = ioctl(mIONFd, ION_IOC_SHARE, &mFdIonMap);
  if (lrc < 0) {
    QIDBG_ERROR("%s:%d] Ion map failed %s\n",
      __func__, __LINE__, strerror(errno));
    goto ION_MAP_FAILED;
  }
  mFd = mFdIonMap.fd;
#else
  lrc = ion_alloc_fd(mIONFd, mAllocIon.len, PAGE_SZ, mAllocIon.heap_id_mask,
    mAllocIon.flags, &mFd);
  if (lrc) {
    QIDBG_ERROR("%s:%d] Ion map failed %s\n",
      __func__, __LINE__, strerror(errno));
    goto ION_ALLOC_FAILED;
  }
  mFdIonMap.fd = mFd;
#endif
  if (mMemMapped) {
    mAddr = (uint8_t *)mmap(NULL,
      mAllocIon.len,
      PROT_READ  | PROT_WRITE,
      MAP_SHARED,
      mFd,
      0);
    if (mAddr == MAP_FAILED) {
      QIDBG_ERROR("%s:%d] Ion map failed %s\n",
        __func__, __LINE__, strerror(errno));
      goto ION_MAP_FAILED;
    }
  }
  QIDBG_MED("%s:%d] addr %p len %d alloc_len %d fd %d",
    __func__, __LINE__,
    mAddr, mLength, mAllocIon.len, mFd);
  return QI_SUCCESS;

ION_MAP_FAILED:
#ifndef TARGET_ION_ABI_VERSION
  lHandleData.handle = mFdIonMap.handle;
  lrc = ioctl(mIONFd, ION_IOC_FREE, &lHandleData);
  if (lrc < 0) {
    QIDBG_ERROR("%s:%d] Ion free failed %s\n",
      __func__, __LINE__, strerror(errno));
  }
#else
  close(mFd);
#endif
ION_ALLOC_FAILED:
  return QI_ERR_GENERAL;
}

/*===========================================================================
 * Function: CloseClient
 *
 * Description: close the ION client
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void QIONBuffer::CloseClient()
{
  if (mIONFd > 0) {
#ifndef TARGET_ION_ABI_VERSION
    close(mIONFd);
#else
    ion_close(mIONFd);
#endif
  }
}

/*===========================================================================
 * Function: DoUnmap
 *
 * Description: unmap the buffers
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void QIONBuffer::DoUnmap()
{
  int lrc = 0;
  struct ion_handle_data lHandleData;

  if (mIONFd <= 0) {
    return;
  }
  QIDBG_MED("%s:%d] addr %p len %d alloc_len %d fd %d",
    __func__, __LINE__,
    mAddr, mLength, mAllocIon.len, mFd);

  if (mMemMapped) {
    lrc = munmap(mAddr, mAllocIon.len);
    if (lrc < 0) {
      QIDBG_ERROR("%s:%d] munmap failed %s\n",
        __func__, __LINE__, strerror(errno));
    }
  }
  close(mFdIonMap.fd);

#ifndef TARGET_ION_ABI_VERSION
  lHandleData.handle = mFdIonMap.handle;
  ioctl(mIONFd, ION_IOC_FREE, &lHandleData);
  if (lrc < 0) {
    QIDBG_ERROR("%s:%d] Ion free failed %s\n",
      __func__, __LINE__, strerror(errno));
  }
#endif
}

/*===========================================================================
 * Function: QIONBuffer
 *
 * Description: QIONBuffer constructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QIONBuffer::QIONBuffer()
: QIBuffer(NULL, 0)
{
  mIONFd = -1;
  mMemMapped = true;
  memset(&mAllocIon, 0x0, sizeof(struct ion_allocation_data));
  memset(&mFdIonMap, 0x0, sizeof(struct ion_fd_data));
}

/*===========================================================================
 * Function: ~QIONBuffer
 *
 * Description: QIONBuffer destructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QIONBuffer::~QIONBuffer()
{
  /*unmap the buffer*/
  DoUnmap();

  /*close the ion client*/
  CloseClient();

}

/*===========================================================================
 * Function: DoCacheOps
 *
 * Description: Static Function to perform cache operation for the ION
 *             buffer
 *
 * Input parameters:
 *   aIONBuffer - pointer to the ION buffer
 *   aType - cache operation type
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *
 * Notes: none
 *==========================================================================*/
int QIONBuffer::DoCacheOps(QIONBuffer *aIONBuffer, QIONCacheOpsType aType)
{

int lrc = QI_SUCCESS;
#ifndef TARGET_ION_ABI_VERSION
  uint32_t lCmd = ION_IOC_CLEAN_INV_CACHES;
  struct ion_custom_data mCustomData;
  struct ion_flush_data mFlushData;
  memset(&mFlushData, 0, sizeof(struct ion_flush_data));
  memset(&mCustomData, 0, sizeof(struct ion_custom_data));

  switch (aType) {
  case CACHE_INVALIDATE:
    lCmd = ION_IOC_INV_CACHES;
    break;
  case CACHE_CLEAN:
    lCmd = ION_IOC_CLEAN_CACHES;
    break;
  default:
  case CACHE_CLEAN_INVALIDATE:
    lCmd = ION_IOC_CLEAN_INV_CACHES;
    break;
  }
  if (aIONBuffer->mIONFd <= 0) {
    QIDBG_ERROR("%s:%d] Invalid ION fd %d", __func__, __LINE__,
      aIONBuffer->mIONFd);
    return lrc;
  }

  mFlushData.vaddr = aIONBuffer->Addr();
  mFlushData.fd = aIONBuffer->Fd();
  mFlushData.handle =  aIONBuffer->mFdIonMap.handle;
  mFlushData.length = aIONBuffer->Length();

  mCustomData.cmd = lCmd;
  mCustomData.arg = (unsigned long)&mFlushData;

  lrc = ioctl(aIONBuffer->mIONFd, ION_IOC_CUSTOM, &mCustomData);
  if (lrc < 0) {
    QIDBG_ERROR("%s:%d] failed with errno %s\n", __func__, __LINE__,
    strerror(errno));
    lrc = QI_ERR_GENERAL;
  } else
    QIDBG_MED("%s:%d] success %d", __func__, __LINE__, aIONBuffer->mCached);

#else
  struct dma_buf_sync buf_sync_start;
  struct dma_buf_sync buf_sync_end;
  /* ION_IOC_CLEAN_CACHES-->call the DMA_BUF_IOCTL_SYNC IOCTL with flags
     DMA_BUF_SYNC_START and DMA_BUF_SYNC_WRITE and then call the
     DMA_BUF_IOCTL_SYNC IOCTL with flags DMA_BUF_SYNC_END
     and DMA_BUF_SYNC_WRITE

     ION_IOC_INV_CACHES-->call the DMA_BUF_IOCTL_SYNC IOCT with flags
     DMA_BUF_SYNC_START and DMA_BUF_SYNC_WRITE and then call the
     DMA_BUF_IOCTL_SYNC IOCT with flags DMA_BUF_SYNC_END
     and DMA_BUF_SYNC_READ
  */
  switch (aType) {
  case CACHE_INVALIDATE:
    buf_sync_start.flags = DMA_BUF_SYNC_START | DMA_BUF_SYNC_WRITE;
    buf_sync_end.flags   = DMA_BUF_SYNC_END   | DMA_BUF_SYNC_READ;
    break;
  case CACHE_CLEAN:
    buf_sync_start.flags = DMA_BUF_SYNC_START | DMA_BUF_SYNC_WRITE;
    buf_sync_end.flags   = DMA_BUF_SYNC_END   | DMA_BUF_SYNC_WRITE;
    break;
  default:
  case CACHE_CLEAN_INVALIDATE:
    buf_sync_start.flags = DMA_BUF_SYNC_START | DMA_BUF_SYNC_RW;
    buf_sync_end.flags   = DMA_BUF_SYNC_END   | DMA_BUF_SYNC_RW;
    break;
  }

  lrc= ioctl(aIONBuffer->mFdIonMap.fd, DMA_BUF_IOCTL_SYNC, &buf_sync_start.flags);
  if (lrc) {
    QIDBG_ERROR("Failed first DMA_BUF_IOCTL_SYNC start\n");
  }

  lrc = ioctl(aIONBuffer->mFdIonMap.fd, DMA_BUF_IOCTL_SYNC, &buf_sync_end.flags);
  if (lrc) {
    QIDBG_ERROR("Failed first DMA_BUF_IOCTL_SYNC End\n");
  }
#endif

return lrc;

}

/*===========================================================================
 * Function: DoCacheOps
 *
 * Description: Function to perform cache operation for external ION
 *             buffer
 *
 * Input parameters:
 *   aFd - Fd of ion buffer
 *   aLength: Length of ion buffer
 *   aCacheType - cache operation type
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *
 * Notes: none
 *==========================================================================*/
int QIONBuffer::DoCacheOps(int aFd, uint32_t aLength, QIONCacheOpsType aType)
{

int lrc = QI_SUCCESS;

#ifndef TARGET_ION_ABI_VERSION
  uint32_t lCmd;
  struct ion_custom_data lCustomData;
  struct ion_flush_data lFlushData;
  struct ion_fd_data lFdData;
  int ldrvRet;
  int lIonFd;

  if (aFd < 0) {
    QIDBG_MED("%s:%d] aFd not valid, skip cache operations %s\n",
      __func__, __LINE__, strerror(errno));
    return QI_SUCCESS;
  }

  lIonFd = open("/dev/ion", O_RDONLY);
  if (lIonFd < 0) {
    QIDBG_ERROR("%s:%d] Ion fd failed to create %s\n",
      __func__, __LINE__, strerror(errno));
    return QI_ERR_GENERAL;
  }

  switch (aType) {
  case CACHE_INVALIDATE:
    lCmd = ION_IOC_INV_CACHES;
    break;
  case CACHE_CLEAN:
    lCmd = ION_IOC_CLEAN_CACHES;
    break;
  default:
  case CACHE_CLEAN_INVALIDATE:
    lCmd = ION_IOC_CLEAN_INV_CACHES;
    break;
  }
  memset(&lFdData, 0x0, sizeof(lFdData));

  lFdData.fd = aFd;
  ldrvRet = ioctl (lIonFd, ION_IOC_IMPORT, &lFdData);
  if (!ldrvRet) {
    memset(&lFlushData, 0, sizeof(struct ion_flush_data));
    lFlushData.fd = aFd;
    lFlushData.handle = lFdData.handle;
    lFlushData.length = aLength;

    memset(&lCustomData, 0, sizeof(lCustomData));
    lCustomData.cmd = lCmd;
    lCustomData.arg = (unsigned long)&lFlushData;
    ldrvRet = ioctl(lIonFd, ION_IOC_CUSTOM, &lCustomData);
    if (ldrvRet < 0) {
      QIDBG_ERROR("%s:%d] failed with errno %s\n", __func__, __LINE__,
        strerror(errno));
      lrc = QI_ERR_GENERAL;
    }
  } else {
    QIDBG_ERROR("%s:%d: ION_IOC_IMPORT failed%s\n", __func__, __LINE__,
      strerror(errno));
    lrc = QI_ERR_GENERAL;
  }
  close(lIonFd);
#else
  (void)(aLength);
  struct dma_buf_sync buf_sync_start;
  struct dma_buf_sync buf_sync_end;
  switch (aType) {
  case CACHE_INVALIDATE:
    buf_sync_start.flags = DMA_BUF_SYNC_START | DMA_BUF_SYNC_WRITE;
    buf_sync_end.flags   = DMA_BUF_SYNC_END   | DMA_BUF_SYNC_READ;
    break;
  case CACHE_CLEAN:
    buf_sync_start.flags = DMA_BUF_SYNC_START | DMA_BUF_SYNC_WRITE;
    buf_sync_end.flags   = DMA_BUF_SYNC_END   | DMA_BUF_SYNC_WRITE;
    break;
  default:
  case CACHE_CLEAN_INVALIDATE:
    buf_sync_start.flags = DMA_BUF_SYNC_START | DMA_BUF_SYNC_RW;
    buf_sync_end.flags   = DMA_BUF_SYNC_END   | DMA_BUF_SYNC_RW;
    break;
  }

  lrc= ioctl(aFd, DMA_BUF_IOCTL_SYNC, &buf_sync_start.flags);
  if (lrc) {
    QIDBG_ERROR("Failed first DMA_BUF_IOCTL_SYNC start\n");
  }

  lrc = ioctl(aFd, DMA_BUF_IOCTL_SYNC, &buf_sync_end.flags);
  if (lrc) {
    QIDBG_ERROR("Failed first DMA_BUF_IOCTL_SYNC End\n");
  }
#endif
return lrc;

}

/*===========================================================================
 * Function: New
 *
 * Description: 2 phase constructor for heap buffer
 *
 * Input parameters:
 *   aLength - length of the buffer
 *
 * Return values:
 *   pointer to QIONBuffer object
 *
 * Notes: none
 *==========================================================================*/
QIONBuffer *QIONBuffer::New(uint32_t aLength, bool aCached, bool aMemMap)
{
  int lrc = QI_SUCCESS;
  QIONBuffer *lBuffer = new QIONBuffer();
  if (NULL == lBuffer) {
    return NULL;
  }

  lBuffer->mLength = aLength;
  lBuffer->mCached = aCached;
  lBuffer->mMemMapped = aMemMap;
  lrc = lBuffer->OpenClient();
  if (lrc < 0) {
    QIDBG_ERROR("%s:%d] open client failed\n", __func__, __LINE__);
    goto error;
  }

  lrc = lBuffer->DoMmap();
  if (lrc < 0) {
    QIDBG_ERROR("%s:%d] mmap failed\n", __func__, __LINE__);
    goto error;
  }

  return lBuffer;

error:
  if (lBuffer) {
    delete lBuffer;
    lBuffer = NULL;
  }
  return NULL;
}
