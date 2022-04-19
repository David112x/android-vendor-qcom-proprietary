/*
* Copyright (C) 2014 The Android Open Source Project
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#ifndef QMI_RIL_UTF
#include <pthread.h>
#include <utils/Log.h>

using namespace std;

/**
 * Template queue class to handling requests for a rild socket.
 * <p>
 * This class performs the following functions :
 * <ul>
 *     <li>Enqueue.
 *     <li>Dequeue.
 *     <li>Check and dequeue.
 * </ul>
 */

template <typename T>
class Ril_queue {
   /**
     * Queue mutex variable for synchronized queue access.
     */
    qtimutex::QtiSharedMutex mutex_instance;

   /**
     * Condition to be waited on for dequeuing.
     */
    std::condition_variable_any cond;

   /**
     * Front of the queue.
     */
    T *front;

    public:

       /**
         * Remove the first element of the queue.
         *
         * @return first element of the queue.
         */
        T* dequeue(void);

       /**
         * Add a request to the front of the queue.
         *
         * @param Request to be added.
         */
        void enqueue(T* request);

       /**
         * Check if the queue is empty.
         */
        int empty(void);

       /**
         * Check and remove an element with a particular message id and token.
         *
         * @param Request message id.
         * @param Request token.
         */
        int checkAndDequeue( MsgId id, int token);

       /**
         * Queue constructor.
         */
        Ril_queue(void);
};

template <typename T>
Ril_queue<T>::Ril_queue(void) {
    cond = PTHREAD_COND_INITIALIZER;
    front = NULL;
}

template <typename T>
T* Ril_queue<T>::dequeue(void) {
    T* temp = NULL;

    mutex_instance.lock();
    while(empty()) {
        cond.wait(mutex_instance);
    }
    temp = this->front;
    if(NULL != this->front->p_next) {
        this->front = this->front->p_next;
    } else {
        this->front = NULL;
    }
    mutex_instance.unlock();

    return temp;
}

template <typename T>
void Ril_queue<T>::enqueue(T* request) {

    mutex_instance.lock();

    if(NULL == this->front) {
        this->front = request;
        request->p_next = NULL;
    } else {
        request->p_next = this->front;
        this->front = request;
    }
    cond.notify_all();
    mutex_instance.unlock();
}

template <typename T>
int Ril_queue<T>::checkAndDequeue(MsgId id, int token) {
    int ret = 0;
    T* temp;

    mutex_instance.lock();

    for(T **ppCur = &(this->front); *ppCur != NULL; ppCur = &((*ppCur)->p_next)) {
        if (token == (*ppCur)->token && id == (*ppCur)->curr->id) {
            ret = 1;
            temp = *ppCur;
            *ppCur = (*ppCur)->p_next;
            free(temp);
            break;
        }
    }

    mutex_instance.unlock();

    return ret;
}


template <typename T>
int Ril_queue<T>::empty(void) {

    if(this->front == NULL) {
        return 1;
    } else {
        return 0;
    }
}
#endif
