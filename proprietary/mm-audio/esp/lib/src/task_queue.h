/*============================================================================
* Copyright (c) 2017, 2018 Qualcomm Technologies, Inc.                       *
* All Rights Reserved.                                                       *
* Confidential and Proprietary - Qualcomm Technologies, Inc.                 *
* ===========================================================================*/
#ifndef _TASK_QUEUE_H_
#define _TASK_QUEUE_H_

typedef unsigned int (*run_fn)(void *, void (*) (void*), void *);
typedef int (*wait_fn)(void *, unsigned int);
typedef void (*wait_all_fn)(void *);
typedef void (*wait_all_fn_ec)(void *, unsigned int);

typedef struct {
    run_fn run;
    wait_fn wait;
    wait_all_fn wait_all;
    void *handle;
    run_fn run_ec1;
    run_fn run_ec2;
    run_fn run_ec3;
    run_fn run_ec4;
    wait_all_fn_ec wait_all_ec;

} task_queue_obj;

#endif /* _TASK_QUEUE_H_ */
