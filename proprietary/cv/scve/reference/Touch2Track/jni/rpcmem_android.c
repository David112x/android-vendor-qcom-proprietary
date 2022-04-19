/*
 * Copyright (c) 2013-2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc
 */
#ifndef VERIFY_PRINT_ERROR
#define VERIFY_PRINT_ERROR
#endif //VERIFY_PRINT_ERROR

#include "verify.h"
#include "rpcmem.h"
#include "AEEQList.h"
#include "AEEstd.h"
#include "AEEStdErr.h"
#include "remote.h"

#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <linux/types.h>

#define ION_HEAP_ID_SECURE 9
#define ION_HEAP_ID_SYSTEM_CONTIG 21

#define ION_HEAP_ID_ADSP 22

#define ION_HEAP_ID_PIL1 23
#define ION_HEAP_ID_SYSTEM 25

#define HEAP_ID_TO_MASK(bit) (1 << (bit))

#define ION_FLAG_CACHED 1
#define SET_FLAG(__cache, __flag)  ((__cache) | (__flag))

enum rpcmem_ion_version { RPCION_VERSION_UNKNOWN, RPCION_VERSION_MODERN, RPCION_VERSION_LEGACY };

typedef int ion_user_handle_t;

struct ion_allocation_data_new {
       __u64 len;
       __u32 heap_id_mask;
       __u32 flags;
       __u32 fd;
       __u32 unused;
};

struct ion_allocation_data_kk {
 size_t len;
 size_t align;
 unsigned int heap_id_mask;
 unsigned int flags;
 ion_user_handle_t handle;
};

struct ion_allocation_data_ics {
 size_t len;
 size_t align;
 unsigned int flags;
 ion_user_handle_t handle;
};

struct ion_fd_data {
 ion_user_handle_t handle;
 int fd;
};
struct ion_handle_data {
 ion_user_handle_t handle;
};

#define ION_IOC_MAGIC 'I'
#define ION_IOC_ALLOC_NEW _IOWR(ION_IOC_MAGIC, 0, struct ion_allocation_data_new)
#define ION_IOC_ALLOC_ICS _IOWR(ION_IOC_MAGIC, 0, struct ion_allocation_data_ics)
#define ION_IOC_ALLOC_KK _IOWR(ION_IOC_MAGIC, 0, struct ion_allocation_data_kk)
#define ION_IOC_FREE _IOWR(ION_IOC_MAGIC, 1, struct ion_handle_data)
#define ION_IOC_MAP _IOWR(ION_IOC_MAGIC, 2, struct ion_fd_data)

/**
 * this heap id is used when -1 is passed to alloc.
 * users should change this to whatever heap id they expect.
 */
static QList lst;
static pthread_mutex_t mt;
static int rpcmem_ionfd;
static int rpcmem_ionversion = RPCION_VERSION_UNKNOWN;

struct mmap_info {
   QNode qn;
   void *paddr;
   int bufsize;
   uint32 flags;
   struct ion_fd_data data;
};

static int rpcmem_contig_alloc_new(struct mmap_info *h, uint32 heap_mask, uint32 memflags, uint32 ion_flags, int size, void **ppo);
static int rpcmem_contig_alloc(struct mmap_info *h, uint32 heap_mask, uint32 memflags, uint32 ion_flags, int size, void **ppo);
static int rpcmem_contig_alloc_ics(struct mmap_info *h, uint32 heap_mask, int size, void **ppo);

static int get_ionversion(int fd) {
   int err, retval = 0, version = 1;
   struct ion_handle_data data = {
      .handle = 0,
   };

   err = ioctl(fd, ION_IOC_FREE, &data);
   if(err < 0)
    retval = errno;
   version = (retval == ENOTTY) ? RPCION_VERSION_MODERN : RPCION_VERSION_LEGACY;
   return version;
}

void rpcmem_init() {
   QList_Ctor(&lst);
   pthread_mutex_init(&mt, 0);
   rpcmem_ionfd = open("/dev/ion", O_RDONLY);
   if (rpcmem_ionfd < 0) {
       VERIFY_EPRINTF("open /dev/ion failed!\n");
       return;
   }
   rpcmem_ionversion = get_ionversion(rpcmem_ionfd);
}

void rpcmem_deinit() {
   pthread_mutex_destroy(&mt);
   if (rpcmem_ionfd > 0) {
      close(rpcmem_ionfd);
   }
}

//extern int is_smmu_enabled(void);
extern void remote_register_buf(void* buf, int size, int fd);
#pragma weak  remote_register_buf
extern void remote_register_buf_attr(void* buf, int size, int fd, int attrs);
#pragma weak  remote_register_buf_attr
extern void *remote_register_fd(int fd, int size);
#pragma weak  remote_register_fd

static void register_buf(void* buf, int size, int fd, int attr) {
   if(remote_register_buf_attr) {
      remote_register_buf_attr(buf, size, fd, attr);
   } else if(remote_register_buf) {
      remote_register_buf(buf, size, fd);
   }
}

void* rpcmem_alloc(int heapid, uint32 flags, int size) {
   int nErr = AEE_SUCCESS;
   struct mmap_info *m = 0;
   void* po = 0;
   uint32 heap_mask;
   uint32 ion_flags;
   uint32 rpc_flags = flags;
   VERIFYC(NULL != (m = malloc(sizeof(*m))), AEE_ENOMEMORY);
   QNode_CtorZ(&m->qn);

   //! default flags should be the same as ion cached
   C_ASSERT(ION_FLAG_CACHED == RPCMEM_DEFAULT_FLAGS);

   ion_flags = rpc_flags & ~0xff000000;
   //! convert from deprecated flags
   if(rpc_flags & RPCMEM_HEAP_DEFAULT) {
      heapid = RPCMEM_DEFAULT_HEAP;
      if(!(rpc_flags & RPCMEM_HEAP_UNCACHED)) {
         ion_flags = SET_FLAG(ion_flags, ION_FLAG_CACHED);
      }
   }
   if(!(rpc_flags & RPCMEM_HEAP_DEFAULT)) {
      VERIFY(!(rpc_flags & RPCMEM_HEAP_UNCACHED));
   }
   if(heapid == RPCMEM_DEFAULT_HEAP) {
      //if(!is_smmu_enabled())
   {
         VERIFY_IPRINTF("rpcmem: smmu is disabled! allocating from contig heap\n");
         heap_mask = HEAP_ID_TO_MASK(ION_HEAP_ID_ADSP);
         nErr = rpcmem_contig_alloc(m, heap_mask, rpc_flags, ion_flags, size, &po);
      }
      if(!po) {
         heap_mask = HEAP_ID_TO_MASK(ION_HEAP_ID_SYSTEM);
         VERIFY(AEE_SUCCESS == (nErr = rpcmem_contig_alloc(m, heap_mask, rpc_flags, ion_flags, size, &po)));
      }
   } else {
      heap_mask = HEAP_ID_TO_MASK(heapid);
      if(heap_mask & HEAP_ID_TO_MASK(ION_HEAP_ID_SECURE)) {
         rpc_flags = rpc_flags | RPCMEM_HEAP_NOVA;
      }
      if(AEE_SUCCESS != (nErr = rpcmem_contig_alloc(m, heap_mask, rpc_flags, ion_flags, size, &po))) {
         VERIFY_EPRINTF("rpcmem allocation error: %d: trying ics\n", nErr);
         VERIFY(AEE_SUCCESS == (nErr = rpcmem_contig_alloc_ics(m, heap_mask, size, &po)));
      }
   }
   pthread_mutex_lock(&mt);
   QList_AppendNode(&lst, &m->qn);
   pthread_mutex_unlock(&mt);
   m->flags = rpc_flags;
   if (!(rpc_flags & RPCMEM_HEAP_NOREG) && !(rpc_flags & RPCMEM_HEAP_NOVA)) {
      int attrs = 0;

      VERIFY_IPRINTF("rpcmem register buf: heap mask: 0x%x\n", heap_mask);
      if (rpc_flags & RPCMEM_HEAP_NONCOHERENT)
          attrs = FASTRPC_ATTR_NON_COHERENT;
      register_buf(m->paddr, m->bufsize, m->data.fd, attrs);
   }
bail:
   if(nErr != AEE_SUCCESS) {
      VERIFY_EPRINTF("Error 0x%x: ION mem alloc failed for size 0x%x, heapid %d, flags 0x%x\n", nErr, size, heapid, flags);
   }
   if(nErr && m) {
      free(m);
      m = NULL;
      po = 0;
   }
   return po;
}

static int rpcmem_contig_free(struct mmap_info *h, void *po);


void rpcmem_free(void* po) {
   int nErr = 0;
   struct mmap_info *m, *mfree = 0;
   QNode* pn, *pnn;
   pthread_mutex_lock(&mt);
   QLIST_NEXTSAFE_FOR_ALL(&lst, pn, pnn) {
      m = STD_RECOVER_REC(struct mmap_info, qn, pn);
      if(m->paddr == po) {
         mfree = m;
         QNode_Dequeue(&m->qn);
         break;
      }
   }
   pthread_mutex_unlock(&mt);
   VERIFY(mfree);
   if(!(mfree->flags & RPCMEM_HEAP_NOREG)) {
      register_buf(mfree->paddr, mfree->bufsize, -1, 0);
   }
   rpcmem_contig_free(mfree, po);
   free(mfree);
   mfree = NULL;

bail:
   return;
}

int rpcmem_to_fd(void* po) {
   struct mmap_info *m;
   int fd = -1;
   QNode* pn;
   pthread_mutex_lock(&mt);
   QLIST_FOR_ALL(&lst, pn) {
      m = STD_RECOVER_REC(struct mmap_info, qn, pn);
      if(STD_BETWEEN(po, m->paddr, (uintptr_t)m->paddr + m->bufsize)) {
         fd = m->data.fd;
         break;
      }
   }
   pthread_mutex_unlock(&mt);

   return fd;
}

static int rpcmem_contig_free(struct mmap_info *h, void *po) {
   struct mmap_info *m = (struct mmap_info *)h;
   int nErr = AEE_SUCCESS;
   (void)po;
   int size = (m->bufsize + 4095) & (~4095);
   if(m->paddr) {
      if(m->flags & RPCMEM_HEAP_NOVA) {
         VERIFY((uintptr_t)remote_register_buf);
         remote_register_buf(m->paddr, size, -1);
      } else {
         munmap(m->paddr, size);
      }
      m->paddr = 0;
   }
   if(m->data.fd) {
      close(m->data.fd);
      m->data.fd = 0;
   }
   if(m->data.handle) {
      ioctl(rpcmem_ionfd, ION_IOC_FREE, &m->data);
      m->data.handle = 0;
   }
bail:
   return AEE_SUCCESS;
}

static int rpcmem_contig_alloc_new(struct mmap_info *h, uint32 heap_mask, uint32 rpcflags, uint32 ion_flags, int size, void **ppo) {
   struct mmap_info *m = (struct mmap_info *)h;
   struct ion_allocation_data_new alloc;
   int nErr = AEE_SUCCESS;
   m->paddr = 0;
   m->data.handle = 0;
   m->data.fd = 0;
   m->bufsize = size;
   VERIFYC(0 < rpcmem_ionfd, AEE_ENOTINITIALIZED);

   //TODO: Will align remove from allocation request results any performance impact?
   alloc.len = (m->bufsize + 4095) & (~4095);
   alloc.heap_id_mask = heap_mask;
   alloc.flags = ion_flags;
   if(rpcflags & RPCMEM_HEAP_NOVA) {
      alloc.flags |= ION_SECURE_FLAGS; //ION_FLAG_SECURE | ION_FLAG_CP_PIXEL;
   }
   VERIFY_IPRINTF("alloc data 0x%p, heap mask 0x%x, flags 0x%x", &m->data, alloc.heap_id_mask, alloc.flags);
   nErr = ioctl(rpcmem_ionfd, ION_IOC_ALLOC_NEW, &alloc);
   if (nErr < 0) {
      VERIFY_EPRINTF("ION allocation ioctl failed %d\n", nErr);
      goto bail;
   }
   m->data.fd = alloc.fd;
   VERIFY_IPRINTF("mmap data 0x%p", &m->data);
   if(rpcflags & RPCMEM_HEAP_NOVA) {
      VERIFY((uintptr_t)remote_register_fd);
      m->paddr = remote_register_fd(m->data.fd, alloc.len);
      VERIFY(0 != m->paddr);
      VERIFY((void*)-1 != m->paddr);
   } else {
      VERIFYC(MAP_FAILED != (m->paddr = (void *)mmap(NULL, alloc.len,
                           PROT_READ|PROT_WRITE, MAP_SHARED, m->data.fd, 0)), AEE_EMMAP);
   }
   *ppo = m->paddr;
bail:
   if (nErr != AEE_SUCCESS) {
      VERIFY_EPRINTF("Error 0x%x (errno 0x%x): rpc contig allocation failed. size 0x%x, heap_mask 0x%x, flags 0x%x\n", nErr, errno, size, heap_mask, rpcflags);
      rpcmem_contig_free((struct mmap_info *)m, 0);
   }
   return nErr;
}

static int rpcmem_contig_alloc_kk(struct mmap_info *h, uint32 heap_mask, uint32 rpcflags, uint32 ion_flags, int size, void **ppo) {
   static uint32_t align[] = {
      0x100000, 0x40000, 0x10000, 0x4000, 0x1000
   };
   struct mmap_info *m = (struct mmap_info *)h;
   struct ion_allocation_data_kk alloc;
   int ii, nErr = AEE_SUCCESS;
   m->paddr = 0;
   m->data.handle = 0;
   m->data.fd = 0;
   m->bufsize = size;
   VERIFYC(0 < rpcmem_ionfd, AEE_ENOTINITIALIZED);

   alloc.len = (m->bufsize + 4095) & (~4095);
   alloc.align = 0x1000;
   if((0 == (heap_mask & HEAP_ID_TO_MASK(ION_HEAP_ID_SYSTEM))) &&
      (0 == (heap_mask & HEAP_ID_TO_MASK(ION_HEAP_ID_SECURE)))) {
      for(ii = 0; ii < STD_ARRAY_SIZE(align); ++ii) {
         if(alloc.len >= align[ii]) {
            alloc.align = align[ii];
            break;
         }
      }
   }
   if(rpcflags & RPCMEM_HEAP_NOVA) {
      alloc.align = 0x1000;
   }

   alloc.heap_id_mask = heap_mask;
   alloc.flags = ion_flags;
   if(rpcflags & RPCMEM_HEAP_NOVA) {
      alloc.flags |= ION_SECURE_FLAGS; //ION_FLAG_SECURE | ION_FLAG_CP_PIXEL;
   }
   VERIFY_IPRINTF("alloc data 0x%p, heap mask 0x%x, flags 0x%x", &m->data, alloc.heap_id_mask, alloc.flags);
   VERIFYC(ioctl(rpcmem_ionfd, ION_IOC_ALLOC_KK, &alloc) >= 0, AEE_EIONALLOC);
   m->data.handle = alloc.handle;
   VERIFY_IPRINTF("mmap data 0x%p", &m->data);
   VERIFYC(0 <= ioctl(rpcmem_ionfd, ION_IOC_MAP, &m->data), AEE_EIONMAP);
   if(rpcflags & RPCMEM_HEAP_NOVA) {
      VERIFY((uintptr_t)remote_register_fd);
      VERIFY((uintptr_t)remote_register_buf);
      m->paddr = remote_register_fd(m->data.fd, alloc.len);
      VERIFY(0 != m->paddr);
      VERIFY((void*)-1 != m->paddr);
   } else {
      VERIFYC(MAP_FAILED != (m->paddr = (void *)mmap(NULL, alloc.len,
                           PROT_READ|PROT_WRITE, MAP_SHARED, m->data.fd, 0)), AEE_EMMAP);
   }
   *ppo = m->paddr;
bail:
   if (nErr != AEE_SUCCESS) {
      VERIFY_EPRINTF("Error 0x%x (errno 0x%x): rpc contig allocation failed. size 0x%x, heap_mask 0x%x, flags 0x%x\n", nErr, errno, size, heap_mask, rpcflags);
      rpcmem_contig_free((struct mmap_info *)m, 0);
   }
   return nErr;
}

static int rpcmem_contig_alloc(struct mmap_info *h, uint32 heap_mask, uint32 rpcflags, uint32 ion_flags, int size, void **ppo) {

   if(rpcmem_ionversion == RPCION_VERSION_MODERN) {
       VERIFY_IPRINTF("ION version is new");
       return rpcmem_contig_alloc_new(h, heap_mask, rpcflags, ion_flags, size, ppo);
   } else {
    VERIFY_IPRINTF("ION version is legacy");
       return rpcmem_contig_alloc_kk(h, heap_mask, rpcflags, ion_flags, size, ppo);
   }
}

static int rpcmem_contig_alloc_ics(struct mmap_info *h, uint32 heap_mask, int size, void **ppo) {
   static uint32_t align[] = {
      0x100000, 0x40000, 0x10000, 0x4000, 0x1000
   };
   struct mmap_info *m = (struct mmap_info *)h;
   struct ion_allocation_data_ics alloc;
   int ii, nErr = AEE_SUCCESS;
   m->paddr = 0;
   m->data.handle = 0;
   m->data.fd = 0;
   m->bufsize = size;
   VERIFYC(0 < rpcmem_ionfd, AEE_ENOTINITIALIZED);

   alloc.len = (m->bufsize + 4095) & (~4095);
   for(ii = 0; ii < STD_ARRAY_SIZE(align); ++ii) {
      if(alloc.len >= align[ii]) {
         alloc.align = align[ii];
         break;
      }
   }

   alloc.flags = heap_mask;

   VERIFYC(0 <= ioctl(rpcmem_ionfd, ION_IOC_ALLOC_ICS, &alloc), AEE_EIONALLOC);
   m->data.handle = alloc.handle;
   VERIFYC(0 <= ioctl(rpcmem_ionfd, ION_IOC_MAP, &m->data), AEE_EIONMAP);
   VERIFYC(MAP_FAILED != (m->paddr = (void *)mmap(NULL, alloc.len,
                           PROT_READ|PROT_WRITE, MAP_SHARED, m->data.fd, 0)), AEE_EMMAP);
   *ppo = m->paddr;
bail:
   if (nErr != AEE_SUCCESS) {
      VERIFY_EPRINTF("Error 0x%x (errno 0x%x): rpc contig ics allocation failed. mask 0x%x, size 0x%x\n", nErr, errno, heap_mask, size);
      rpcmem_contig_free((struct mmap_info *)m, 0);
   }
   return nErr;
}
