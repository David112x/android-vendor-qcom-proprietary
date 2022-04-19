/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_stubs.h
  @brief   RIL UTF QMI stubs for non QMI RIL functions

  DESCRIPTION
  ---------------------------------------------------------------------------
  Copyright (c) 1998 Todd C. Miller <Todd.Miller@courtesan.com>

  Permission to use, copy, modify, and distribute this software for any 
  purpose with or without fee is hereby granted, provided that the above 
  copyright notice and this permission notice appear in all copies.

  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES 
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF 
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR 
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES 
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN 
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF 
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

******************************************************************************/
#include <ril_utf_wake_lock_redef.h>
#include "ril_utf_defs.h"
#ifndef RIL_UTF_IOE
#include "time_genoff.h"
#include "qcrili.h"
#endif
// Local Globals
struct malloc_entry
{
  void* ptr;
  char* func;
  int line_num;
};

#define THREAD_MAX 300

struct malloc_entry *malloc_list = NULL;
int malloc_use_count = 0;
int malloc_list_max = 3000;

pthread_t *thread_list = NULL;


//===================================================================================

// TBD: lilcensing info
// use g_strlcpy
size_t
strlcpy(char *dst, const char *src, size_t siz)
{
    char *d = dst;
    const char *s = src;
    size_t n = siz;

    /* Copy as many bytes as will fit */
    if (n != 0) {
        while (--n != 0) {
            if ((*d++ = *s++) == '\0')
                break;
        }
    }

    /* Not enough room in dst, add NUL and traverse rest of src */
    if (n == 0) {
        if (siz != 0)
            *d = '\0';      /* NUL-terminate dst */
        while (*s++)
            ;
    }

    return(s - src - 1);    /* count does not include NUL */
}

// while you have a lock held, the device will stay on at least at the
// level you request.
int acquire_wake_lock(int lock, const char* id)
{
  return 1;
}
int release_wake_lock(const char* id)
{
  return 1;
}

/*===========================================================================
FUNCTION   Diag_LSM_Init

DESCRIPTION
  Initializes the Diag Legacy Mapping Layer. This should be called
  only once per process.

DEPENDENCIES
  Successful initialization requires Diag CS component files to be present
  and accessible in the file system.

RETURN VALUE
  FALSE = failure, else TRUE

SIDE EFFECTS
  None

===========================================================================*/
boolean Diag_LSM_Init (byte* pIEnv)
{
  return 1;
}

// Temp defs.
// To be removed when actual functions are supported

void qcril_log_msg_to_adb
(
  int  lvl,
  char *msg_ptr
)
{
}

#ifndef UTF_TARGET_BUILD
// use g_strlcat
size_t strlcat(char *dest, const char *src, size_t dest_size)
{
    char *d = dest;
    const char *s = src;
    size_t n = dest_size;
    size_t dlen;

    /* Find the end of dest and adjust bytes left but don't go past end */
    while (n-- != 0 && *d != '\0')
        d++;

    dlen = d - dest;
    n = dest_size - dlen;

    if (n == 0)
        return(dlen + strlen(s));

    while (*s != '\0')
    {
        if (n != 1)
        {
            *d++ = *s;
            n--;
        }
        s++;
    }
    *d = '\0';

    return(dlen + (s - src));/* count does not include NUL */
}
#endif

int qcril_data_get_dds_sub_id(void)
{
    return 0;
}

#ifndef RIL_UTF_IOE
RIL_Errno qcril_data_request_set_lte_attach_profile(RIL_InitialAttachApn* InitialAttachApnMsg)
{
 return RIL_E_SUCCESS;
}
#endif

/*===========================================================================

  FUNCTION:  qcril_data_get_active_calls

===========================================================================*/
/*!
    @brief
    Used to query the current acive calls within QCRIL-Data module.
    The caller should pass a call_list array of dimension
    QCRIL_DATA_MAX_CALL_RECORDS.  If call_list is NULL, only the
    num_calls is updated.

    @return
    QCRIL_DATA_SUCCESS on successful operation,
    QCRIL_DATA_FAILURE otherwise
*/
/*=========================================================================*/
#ifndef RIL_UTF_IOE
int qcril_data_get_active_calls
(
  qcril_data_hndl_t              hndl,
  unsigned int                  *num_calls,
  qcril_data_active_call_info_t *call_list
)
{
  return 0;
}
void qcril_data_client_release
(
  qcril_data_hndl_t   hndl
)
{

}
qcril_data_hndl_t qcril_data_client_register
(
  qcril_data_net_ev_cb  cb_fn,
  void                 *user_data
)
{

  return 0;
}
#endif

typedef enum
{
    QCRIL_QMI_PIL_STATE_MIN = -1,
    QCRIL_QMI_PIL_STATE_OFFLINE,
    QCRIL_QMI_PIL_STATE_ONLINE,
    QCRIL_QMI_PIL_STATE_UNKNOWN,
    QCRIL_QMI_PIL_STATE_MAX
} qcril_qmi_pil_state_type;

typedef struct {
    qcril_qmi_pil_state_type state;
} qcril_qmi_pil_state;

typedef void (*qcril_qmi_pil_state_change_hdlr)(const qcril_qmi_pil_state* cur_state);


#ifndef QMI_RIL_HAL_UTF
int qcril_qmi_pil_init_monitor()
{
  return 1;
}
int qcril_qmi_pil_register_for_state_change(qcril_qmi_pil_state_change_hdlr hdlr)
{
  return 1;
}
int qcril_qmi_pil_deregister_for_state_change(qcril_qmi_pil_state_change_hdlr hdlr);

const qcril_qmi_pil_state* qcril_qmi_pil_get_pil_state()
{
  static qcril_qmi_pil_state pil_state;
  pil_state.state = QCRIL_QMI_PIL_STATE_ONLINE;
  return &pil_state;
}
#endif

// ============================================================
// time genoff stubs
// ============================================================

#ifndef RIL_UTF_IOE
int time_genoff_operation(time_genoff_info_type *pargs)
{
  return 0;
}
#endif

uint64_t ril_nano_time()
{
    return 0;
}

#ifndef RIL_UTF_IOE
void *utf_qcril_malloc_adv
(
  size_t size,
  const char* func_name,
  int line_num
)
{
  void *mem_ptr;

  // Create new list
  if (malloc_list == NULL)
  {
    malloc_list = (struct malloc_entry*) malloc(sizeof(struct malloc_entry) * malloc_list_max);
  }

  // Reallocate list if no space left
  if (malloc_use_count >= malloc_list_max)
  {
    malloc_list_max = malloc_list_max * 2;
    malloc_list = realloc(malloc_list, malloc_list_max * sizeof(struct malloc_entry));
  }

  mem_ptr = qcril_malloc_adv( size, func_name, line_num );

  malloc_list[malloc_use_count].ptr = mem_ptr;
  malloc_list[malloc_use_count].func = func_name;
  malloc_list[malloc_use_count].line_num = line_num;

  ++malloc_use_count;
  return mem_ptr;

} /* qcril_malloc_adv */

void utf_qcril_free_adv( void *mem_ptr, const char* func_name, int line_num )
{
  int i;

  // Remove pointer to avoid double free
  for (i = 0; i < malloc_use_count; ++i)
  {
    if (mem_ptr == malloc_list[i].ptr)
    {
      malloc_list[i].ptr = NULL;
    }
  }

  qcril_free_adv( mem_ptr, func_name, line_num );
}
#endif
int utf_free_all_qmi_ril_heap()
{
  int i;
  int found = 0;
  //printf("\nLIST OF STILL ALLOCED\n");

  for (i = 0; i < malloc_use_count; ++i)
  {
    if (malloc_list[i].ptr != NULL)
    {
      found = 1;
      //printf("\nStill allocated!! %x from func %s at line num %d",
        //malloc_list[i].ptr, malloc_list[i].func, malloc_list[i].line_num);
    }
  }

  free(malloc_list);
  malloc_list = NULL;
  malloc_use_count = 0;

  return 0;
}

int utf_pthread_create_handler( pthread_t *thread, const pthread_attr_t *attr,
                                void *(*start_routine) (void *), void *arg)
{
  int ret;
  int x;

  if (thread_list == NULL)
  {
    thread_list = malloc(THREAD_MAX * sizeof(pthread_t));
    memset(thread_list, 0, (THREAD_MAX *sizeof(pthread_t)) );
  }

  ret = pthread_create( thread, attr, start_routine, arg);

  if (ret == 0)
  {
    if (thread_list)
    {
      for ( x = 0; x < THREAD_MAX; ++x)
      {
        if (thread_list[x] == 0)
        {
          thread_list[x] = *thread;
          break;
        }
      } // end for
    }
  }

  return ret;
}

#ifndef UTF_TARGET_BUILD
int utf_join_threads()
{
  int x;

  if (thread_list)
  {
    for (x = 0; x < THREAD_MAX; ++x)
    {
      if (thread_list[x] == 0)
      {
        break;
      }
      else
      {
        pthread_join(thread_list[x], NULL);
      }
    }

    free(thread_list);
    thread_list = NULL;
  }
  return 0;
}
#endif
