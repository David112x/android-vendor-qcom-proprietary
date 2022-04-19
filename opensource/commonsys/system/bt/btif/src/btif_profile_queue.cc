/******************************************************************************
 *
 *  Copyright (C) 2009-2012 Broadcom Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
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

/*******************************************************************************
 *
 *  Filename:      btif_profile_queue.c
 *
 *  Description:   Bluetooth remote device connection queuing implementation.
 *
 ******************************************************************************/

#define LOG_TAG "bt_btif_queue"

#include "btif_profile_queue.h"

#include <base/logging.h>
#include <string.h>

#include "bt_common.h"
#include "btif_common.h"
#include "osi/include/allocator.h"
#include "osi/include/list.h"
#include "stack_manager.h"
#include "osi/include/thread.h"
/*******************************************************************************
 *  Local type definitions
 ******************************************************************************/

typedef enum {
  BTIF_QUEUE_CONNECT_EVT,
  BTIF_QUEUE_ADVANCE_EVT,
  BTIF_QUEUE_ADVANCE_BY_UUID_EVT,
  BTIF_QUEUE_CLEANUP_EVT
} btif_queue_event_t;

typedef struct {
  RawAddress bda;
  uint16_t uuid;
  uint16_t max_connections;
  bool busy;
  btif_connect_cb_t connect_cb;
} connect_node_t;


/*******************************************************************************
 *  Static variables
 ******************************************************************************/

static list_t* connect_queue;

static const size_t MAX_REASONABLE_REQUESTS = 20;

extern thread_t *bt_jni_workqueue_thread;
/*******************************************************************************
 *  Queue helper functions
 ******************************************************************************/

static void queue_int_add(connect_node_t* p_param) {
  uint16_t counter = 0;
  if (!connect_queue) {
    LOG_INFO(LOG_TAG, "%s: allocating profile queue", __func__);
    connect_queue = list_new(osi_free);
    CHECK(connect_queue != NULL);
  }

  // Sanity check to make sure we're not leaking connection requests
  CHECK(list_length(connect_queue) < MAX_REASONABLE_REQUESTS);

  for (const list_node_t* node = list_begin(connect_queue);
       node != list_end(connect_queue); node = list_next(node)) {
    if (((connect_node_t*)list_node(node))->uuid == p_param->uuid) {
      if (p_param->uuid == UUID_SERVCLASS_AUDIO_SOURCE ||
          p_param->uuid == UUID_SERVCLASS_AG_HANDSFREE ||
          p_param->uuid == UUID_SERVCLASS_AUDIO_SINK) {
          counter++;
          LOG_INFO(LOG_TAG, "%s add  connect request for uuid: %04x",
               __func__, counter);
          continue;
      }
      LOG_INFO(LOG_TAG, "%s dropping duplicate connect request for uuid: %04x",
               __func__, p_param->uuid);
      return;
    }
  }
  uint16_t max_conn = p_param->max_connections;
  if ((counter >= max_conn && p_param->uuid == UUID_SERVCLASS_AUDIO_SOURCE) ||
      (counter >= max_conn && p_param->uuid == UUID_SERVCLASS_AG_HANDSFREE)) {
          LOG_INFO(LOG_TAG, "%s connect request exceeded max supported connection: %04x",
               __func__, p_param->uuid);
          return;
  }
  connect_node_t* p_node = (connect_node_t*)osi_malloc(sizeof(connect_node_t));
  memcpy(p_node, p_param, sizeof(connect_node_t));
  list_append(connect_queue, p_node);
}

static void queue_int_advance() {
  if (connect_queue && !list_is_empty(connect_queue)) {
    connect_node_t* p_head = (connect_node_t*)list_front(connect_queue);
    LOG_INFO(LOG_TAG,
             "%s: removing connection request UUID=%04X, bd_addr=%s, busy=%d",
             __func__, p_head->uuid, p_head->bda.ToString().c_str(),
             p_head->busy);
    list_remove(connect_queue, p_head);
  }
}

static void queue_int_advance_by_uuid(connect_node_t* p_param) {

  if (!connect_queue || list_is_empty(connect_queue)) return;

  list_node_t* node = list_begin(connect_queue);
  connect_node_t* p_head = (connect_node_t*)list_node(node);

  if (p_head == NULL)
    return;

  if (((p_head->bda == p_param->bda) || (p_param->bda.IsEmpty()))
       && (p_head->uuid == p_param->uuid)) {
    LOG_WARN(LOG_TAG,"%s: queue advance UUID=%04X, bd_addr=%s",
        __func__, p_head->uuid, p_head->bda.ToString().c_str());
    btif_queue_advance();
    return;
  }

  // move the node to next
  node = list_next(node);

  for (; node != list_end(connect_queue);) {
    p_head = (connect_node_t*)list_node(node);
    node = list_next(node);

    if (((p_head->bda == p_param->bda) || (p_param->bda.IsEmpty()))
        && (p_head->uuid == p_param->uuid)) {
      LOG_WARN(LOG_TAG,"%s: deleting entry from queue UUID=%04X, bd_addr=%s",
               __func__, p_head->uuid, p_head->bda.ToString().c_str());
      list_remove(connect_queue,p_head);
      return;
    }
  }

  LOG_WARN(LOG_TAG,"%s: no entry found in queue UUID=%04X, bd_addr=%s",
         __func__, p_param->uuid, p_param->bda.ToString().c_str());
  return;
}

static void queue_int_cleanup(uint16_t* p_uuid) {
  if (!p_uuid) {
    LOG_ERROR(LOG_TAG, "%s: UUID is null", __func__);
    return;
  }
  uint16_t uuid = *p_uuid;
  LOG_INFO(LOG_TAG, "%s: UUID=%04X", __func__, uuid);
  if (!connect_queue) {
    return;
  }
  connect_node_t* connection_request;
  const list_node_t* node = list_begin(connect_queue);
  while (node && node != list_end(connect_queue)) {
    connection_request = (connect_node_t*)list_node(node);
    node = list_next(node);
    if (connection_request->uuid == uuid) {
      LOG_INFO(LOG_TAG,
               "%s: removing connection request UUID=%04X, bd_addr=%s, busy=%d",
               __func__, connection_request->uuid,
               connection_request->bda.ToString().c_str(),
               connection_request->busy);
      list_remove(connect_queue, connection_request);
    }
  }
}

static void queue_int_handle_evt(uint16_t event, char* p_param) {
  switch (event) {
    case BTIF_QUEUE_CONNECT_EVT:
      queue_int_add((connect_node_t*)p_param);
      break;

    case BTIF_QUEUE_ADVANCE_EVT:
      queue_int_advance();
      break;

    case BTIF_QUEUE_ADVANCE_BY_UUID_EVT:
      queue_int_advance_by_uuid((connect_node_t*)p_param);
      break;

    case BTIF_QUEUE_CLEANUP_EVT:
      queue_int_cleanup((uint16_t*)(p_param));
      return;
  }

  if (stack_manager_get_interface()->get_stack_is_running())
    btif_queue_connect_next();
}

/*******************************************************************************
 *
 * Function         btif_queue_connect
 *
 * Description      Add a new connection to the queue and trigger the next
 *                  scheduled connection.
 *
 * Returns          BT_STATUS_SUCCESS if successful
 *
 ******************************************************************************/
bt_status_t btif_queue_connect(uint16_t uuid, const RawAddress& bda,
                               btif_connect_cb_t connect_cb, uint16_t max_conn) {
  connect_node_t node;
  memset(&node, 0, sizeof(connect_node_t));
  node.bda = bda;
  node.uuid = uuid;
  node.max_connections = max_conn;
  node.connect_cb = connect_cb;

  return btif_transfer_context(queue_int_handle_evt, BTIF_QUEUE_CONNECT_EVT,
                               (char*)&node, sizeof(connect_node_t), NULL);
}

/*******************************************************************************
 *
 * Function         btif_queue_cleanup
 *
 * Description      Clean up existing connection requests for a UUID
 *
 * Returns          void, always succeed
 *
 ******************************************************************************/
void btif_queue_cleanup(uint16_t uuid) {
  btif_transfer_context(queue_int_handle_evt, BTIF_QUEUE_CLEANUP_EVT,
                        (char*)&uuid, sizeof(uint16_t), NULL);
}

/*******************************************************************************
 *
 * Function         btif_queue_advance
 *
 * Description      Clear the queue's busy status and advance to the next
 *                  scheduled connection.
 *
 * Returns          void
 *
 ******************************************************************************/
void btif_queue_advance() {
/*    if (thread_is_self(bt_jni_workqueue_thread))
    {
        queue_int_handle_evt(BTIF_QUEUE_ADVANCE_EVT, NULL);
    }
    else
    {
        btif_transfer_context(queue_int_handle_evt, BTIF_QUEUE_ADVANCE_EVT,
                        NULL, 0, NULL);
    }*/
    btif_transfer_context(queue_int_handle_evt, BTIF_QUEUE_ADVANCE_EVT,
                        NULL, 0, NULL);
}

/*******************************************************************************
 *
 * Function         btif_queue_advance_by_uuid
 *
 * Description      remove the connected uuid entry from queue,
 *                  adavance queue if entry found at head of the queue
 *
 *
 * Returns          None
 *
 ******************************************************************************/
void btif_queue_advance_by_uuid(uint16_t uuid, const RawAddress* bda) {

   connect_node_t* node = (connect_node_t*)osi_malloc(sizeof(connect_node_t));
   memset(node, 0, sizeof(connect_node_t));
   node->bda = *bda;
   node->uuid = uuid;

   btif_transfer_context(queue_int_handle_evt, BTIF_QUEUE_ADVANCE_BY_UUID_EVT,
                               (char*)node, sizeof(connect_node_t), NULL);
}

// This function dispatches the next pending connect request. It is called from
// stack_manager when the stack comes up.
bt_status_t btif_queue_connect_next(void) {
  if (!connect_queue || list_is_empty(connect_queue)) return BT_STATUS_FAIL;

  connect_node_t* p_head = (connect_node_t*)list_front(connect_queue);

  LOG_INFO(LOG_TAG,
           "%s: executing connection request UUID=%04X, bd_addr=%s, busy=%d",
           __func__, p_head->uuid, p_head->bda.ToString().c_str(),
           p_head->busy);
  // If the queue is currently busy, we return success anyway,
  // since the connection has been queued...
  if (p_head->busy) return BT_STATUS_SUCCESS;

  p_head->busy = true;
  return p_head->connect_cb(&p_head->bda, p_head->uuid);
}

/*******************************************************************************
 *
 * Function         btif_queue_release
 *
 * Description      Free up all the queue nodes and set the queue head to NULL
 *
 * Returns          void
 *
 ******************************************************************************/
void btif_queue_release() {
  LOG_INFO(LOG_TAG, "%s", __func__);
  list_free(connect_queue);
  connect_queue = NULL;
}
