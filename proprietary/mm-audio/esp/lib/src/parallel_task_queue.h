/*============================================================================
* Copyright (c) 2017 Qualcomm Technologies, Inc.                             *
* All Rights Reserved.                                                       *
* Confidential and Proprietary - Qualcomm Technologies, Inc.                 *
* ===========================================================================*/
#ifndef _PARALLEL_TASK_QUEUE_INTF_H_
#define _PARALLEL_TASK_QUEUE_INTF_H_

#include "task_queue.h"

task_queue_obj * init_parallel_task_queue(void);
void deinit_parallel_task_queue(task_queue_obj *obj);

#endif /* _PARALLEL_TASK_QUEUE_INTF_H_ */
