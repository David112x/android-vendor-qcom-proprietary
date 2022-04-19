/* mct_list.c
 *
 * This file contains the helper methods and implementation for managing
 * lists.
 *
 * Copyright (c) 2012-2016 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include "mtype.h"
#include "mct_list.h"
#include "camera_dbg.h"

static mct_list_t *mct_list_new(void *data)
{
  mct_list_t *new_list;

  new_list = malloc(sizeof(mct_list_t));
  if (!new_list)
    return NULL;

  new_list->data    = data;
  new_list->next    = NULL;
  new_list->next_num = 0;
  new_list->prev = NULL;

  return new_list;
}

static void mct_list_free(mct_list_t *mct_list)
{
  if (mct_list) {
    free(mct_list);
    mct_list = NULL;
  }
}

/** mct_list_append:
 *    @list - mct_list_t object;
 *    @data - new data to be appended to the list;
 *    @appendto - which node to be appendto.
 *    @func     - tree search function.
 *
 *  MctList which contains the @data;
 *  Tree model is ONLY used for a Stream to manage Modules.
 **/
mct_list_t* mct_list_append(mct_list_t *mct_list, void *data, void *appendto,
  mct_list_find_func list_find)
{
  mct_list_t *new_list = NULL;
  mct_list_t *last = NULL;

  if (!appendto && !list_find) {
    /* Normal List operation */
    new_list = mct_list_new(data);
    if (!new_list)
      return NULL;

    last = mct_list;
    if (last) {
      while (last->next)
        last = last->next[0];

      last->next = calloc(1, sizeof(mct_list_t*));
      if (!last->next)
        goto error;

      last->next[0] = new_list;
      last->next_num = 1;
      new_list->prev = last;
      return mct_list;

    } else {
      return new_list;
    }
  } else if (appendto && list_find) {

    /* This should be ONLY for Stream operates on Modules
     * when perform link or add modules */
    last = mct_list;

    if (last) {
      if (list_find(last->data, appendto) == TRUE) {
        new_list = mct_list_new(data);
        if (!new_list)
          goto error;

        if (!last->next) {
          last->next = calloc(1, sizeof(mct_list_t*));
          if (!last->next) {
            mct_list_free(new_list);
            return NULL;
          }
          last->next[0] = new_list;
          last->next_num = 1;
          new_list->prev     = last;
          return mct_list;
        } else {
          /* create tree branch */
          last->next = realloc(last->next,
            (last->next_num + 1) * sizeof(mct_list_t*));
          if (!last->next)
            goto error;

          last->next[last->next_num] = new_list;
          last->next_num++;
          new_list->prev = last;
          return mct_list;
        }
      } else if (last->next) {
        uint32_t num;
        num  = last->next_num;
        last = last->next[0];
        while (num--) {
          if (mct_list_append(last, data, appendto, list_find))
            return mct_list;
          last++;
        }
      } else {
        return NULL;
      }
    } else {
      /* Starting from an empty list */
      mct_list_t *temp = NULL;

      new_list = mct_list_new(appendto);
      if (!new_list)
        return NULL;

      new_list->next = calloc(1, sizeof(mct_list_t*));
      if (!new_list->next) {
        goto error;
      }

      temp = mct_list_new(data);
      if (!temp)
        goto error1;

      new_list->next[0] = temp;
      new_list->next_num = 1;
      temp->prev = new_list;

      return new_list;
    }
  }

error1:
  if (new_list && new_list->next)
    mct_list_free(*(new_list->next));
error:
  if (new_list)
    mct_list_free(new_list);

  return NULL;
}

#if 0
mct_list_t* mct_list_prepend(mct_list_t *mct_list, void *data)
{
  /*TO DO*/
  mct_list_t *new_list;
  int i = 0;

  new_list = mct_list_new(data);
  if (!new_list)
    return NULL;

  new_list->next = calloc(1, sizeof(mct_list_t*));
  if (!new_list->next)
    goto error;

  new_list->next[0] = mct_list;

  if (mct_list) {
    new_list->prev = mct_list->prev;

    if (mct_list->prev) {
      /*find corresponding mct_list->prev->next[i]*/
      mct_list->prev->next[i] = new_list;
    }
    mct_list->prev = new_list;
  } else
    new_list->prev = NULL;

  return new_list;
error:
  mct_list_free(new_list);
  return NULL;
}
#endif

mct_list_t* mct_list_insert(mct_list_t *mct_list __unused,
  void *data __unused, uint32_t pos __unused)
{
  /* TODO */
  return NULL;
}

mct_list_t* mct_list_insert_before(mct_list_t *mct_list __unused,
  mct_list_t *inserted __unused, const void *data __unused)
{
  /*TO DO*/
  return NULL;
}

mct_list_t* mct_list_remove(mct_list_t *mct_list, const void *data)
{
  mct_list_t *temp, *temp_list;
  uint32_t num, i, j;
  boolean moved;

  if (!mct_list)
    return mct_list;

  temp = mct_list;
  num  = mct_list->next_num;

  if (num > 0 && temp->data != data) {

    temp_list = mct_list->next[0];
    while (num--) {
      mct_list_remove(temp_list, data);
      temp_list++;
    }
  } else if (temp->data == data) {

    num = temp->next_num;

    if (temp->prev) {
      if (temp->prev->next_num == 1) {
        temp->prev->next_num = 0;
        free(temp->prev->next);
        temp->prev->next = NULL;
        if (num > 0) {
          temp->prev->next = calloc(num, sizeof(mct_list_t*));
          if (!temp->prev->next) {
            /* this is error, no action for now */
            return mct_list;
          }

          i = 0;
          while (num--) {
            temp->prev->next[i] = temp->next[i];
            temp->next[i]->prev = temp->prev;
            temp->prev->next_num++;
            i++;
          }
          free(temp->next);
          temp->next = NULL;
        } else {
          temp->prev->next = NULL;
        } /* num == 0 */
      } else if (temp->prev->next_num > 1) {
        if (num > 1) {
          temp->prev->next = realloc(temp->prev->next,
            (temp->prev->next_num + num - 1) * sizeof(mct_list_t*));
          if (!temp->prev->next) {
            /* this is error, no action for now */
            return mct_list;
          }

          i = temp->prev->next_num;
          j = 0;
          moved = FALSE;
          while (--i) { /* must use --i */
            if (moved)
              temp->prev->next[j] = temp->prev->next[j+1];

            if (temp->prev->next[j] == temp) {
              moved = TRUE;
              temp->prev->next[j] = temp->prev->next[j+1];
            }

            j++;
          }

          temp->prev->next_num--;

          i = 0;
          while (num--) {
            temp->prev->next[temp->prev->next_num] = temp->next[i];
            temp->next[i]->prev = temp->prev;
            temp->prev->next_num++;
            i++;
          }
        } else if (num == 1) {
          i = temp->prev->next_num;
          j = 0;
          while (i--) {
            if (temp->prev->next[j] == temp) {
              temp->prev->next[j] = temp->next[0];
              temp->next[0]->prev = temp->prev;
              break;
            }

            j++;
          }
        } else if (num == 0) {
          i = temp->prev->next_num;
          j = 0;
          while (--i) { /* must be --i */
            if (temp->prev->next[j] == temp) {
              temp->prev->next[j] = temp->prev->next[j+1];
            }

            j++;
          }

          temp->prev->next_num -= 1;
          temp->prev->next = realloc(temp->prev->next,
            (temp->prev->next_num) * sizeof(mct_list_t*));
          if (!temp->prev->next) {
            /* this is error, no action for now */
            return mct_list;
          }
        }
      } /* temp->prev->next_num > 1 */

      mct_list_free(temp);
      return NULL;

    } else {
      /* temp->prev == NULL */
      if (temp->next_num == 1) {
        temp->next[0]->prev = NULL;
        mct_list = temp->next[0];
        free(temp->next);
        temp->next = NULL;
        mct_list_free(temp);
      } else if (temp->next_num == 0) {
        mct_list = NULL;
        mct_list_free(temp);
      } else {
        /*this condition is not possible*/
      }
    } /* temp->prev == NULL */
  } /* temp->data == data */

  return mct_list;
}

/** mct_list_find_custom_branch:
 *    @list: list to be traversed
 *    @data: data passed to search the particular node in the list
 *    @list_find: customized function to find node using data.
 *  Traverse whole list in recursion when the list is in form of a tree
 **/

static mct_list_t* mct_list_find_custom_branch(mct_list_t *mct_list,
  void *data, mct_list_find_func list_find)
{
  mct_list_t **temp_list;
  uint32_t num;

  if (!list_find)
    return mct_list;

  if (mct_list) {
    if (list_find(mct_list->data, data) == TRUE)
     return mct_list;

    num = mct_list->next_num;
    if (num > 0 && mct_list->next) {
      temp_list = mct_list->next;
      while (num) {
        mct_list = mct_list_find_custom_branch(temp_list[--num], data, list_find);
        if (mct_list)
          return mct_list;
      }
    }
  }

  return NULL;
}

/** mct_list_find_custom:
 *    @list: list to be traversed
 *    @data: data passed to search the particular node in the list
 *    @list_find: customized function to find node using data.
 *  Traverse whole list linearly if its not a tree.
 **/

mct_list_t* mct_list_find_custom(mct_list_t *mct_list,
  void *data, mct_list_find_func list_find)
{
  uint32_t num;

  if (!list_find)
    return mct_list;

  if (mct_list) {
    do {
      num = mct_list->next_num;
      if (num > 1) {
        mct_list = mct_list_find_custom_branch(mct_list, data, list_find);
        return mct_list;
      }
      if (list_find(mct_list->data, data) == TRUE) {
        return mct_list;
      }
      if (mct_list->next) {
        mct_list = mct_list->next[0];
      } else {
        break;
      }
    } while (mct_list != NULL);
  }
  return NULL;
}

/** mct_list_find_and_add_custom:
 *
 **/
mct_list_t* mct_list_find_and_add_custom(mct_list_t *parent_list,
  mct_list_t *child_list, void *data, mct_list_find_func list_find)
{
  mct_list_t *temp_list;
  uint32_t num;
  uint32_t idx;

  if (parent_list) {
    num = parent_list->next_num;
    if (num > 0 && parent_list->next) {
      temp_list = parent_list->next[0];
      idx = 0;
      while (num--) {
        child_list =
          mct_list_find_and_add_custom(temp_list, child_list, data, list_find);
        idx++;
        if (num)
          temp_list = parent_list->next[idx];
      }
    }
    if (list_find(parent_list->data, data) == TRUE)
      child_list = mct_list_append(child_list, parent_list->data, NULL, NULL);
  }
  return child_list;
}

/** mct_list_traverse_branch:
 *    @list: list to be traversed
 *    @traverse: customized function to find node using user_data
 *    @user_data: data passed to search the particular node in the list
 *  Traverse whole list in recursion when the list is in form of a tree
 **/

static boolean mct_list_traverse_branch(mct_list_t *mct_list,
                  mct_list_traverse_func traverse,
                  void *user_data)
{
  boolean rc = TRUE;
  mct_list_t *temp_list;
  uint32_t num;
  uint32_t idx;

  if (mct_list && traverse) {
    if (traverse(mct_list->data, user_data) == FALSE)
    return FALSE;

    num = mct_list->next_num;
    if (num > 0 && mct_list->next) {
      temp_list = mct_list->next[0];
      idx = 0;
      while (num--) {
        rc &= mct_list_traverse_branch(temp_list, traverse, user_data);
        idx++;
        if (num)
          temp_list = mct_list->next[idx];
      }
    }
  }

  return rc;
}

/** mct_list_traverse:
 *    @list: list to be traversed
 *    @traverse:customized function to find node using user_data
 *    @user_data: data passed to search the particular node in the list
 *  Traverse whole list linearly if its not a tree.
 **/

boolean mct_list_traverse(mct_list_t *mct_list,
                  mct_list_traverse_func traverse,
                  void *user_data)
{
  boolean rc = TRUE;
  uint32_t num;

  if (mct_list && traverse) {
    do {
       num = mct_list->next_num;
       if (num > 1) {
         rc = mct_list_traverse_branch(mct_list, traverse, user_data);
         break;
       }
       if (traverse(mct_list->data, user_data) == FALSE) {
          return FALSE;
       }
       if (mct_list->next) {
         mct_list = mct_list->next[0];
       } else {
         break;
       }
    } while (mct_list != NULL);
  }
  return rc;
}

/** mct_list_free_list:
 *
 **/
void mct_list_free_list(mct_list_t *mct_list)
{
  mct_list_t *temp_list;
  uint32_t num;
  uint32_t idx;

  if (mct_list) {
    num = mct_list->next_num;
    if (num > 0 && mct_list->next) {
      temp_list = mct_list->next[0];
      idx = 0;
      while (num--) {
        mct_list_free_list(temp_list);
        idx++;
        if (num)
          temp_list = mct_list->next[idx];
      }
      free(mct_list->next);
      mct_list->next = NULL;
    }
    mct_list_free(mct_list);
  }
}

/** mct_list_free_all:
 *    @list:
 *    @freefunc:
 *
 *
 **/
void mct_list_free_all(mct_list_t *mct_list, mct_list_traverse_func traverse)
{
  mct_list_traverse(mct_list, traverse, NULL);
  mct_list_free_list(mct_list);
}

/** mct_list_free_all:
 *    @list:
 *    @freefunc:
 *
 *
 **/
void mct_list_free_all_on_data(mct_list_t *mct_list,
  mct_list_traverse_func traverse, void *user_data)
{
  mct_list_traverse(mct_list, traverse, user_data);
  mct_list_free_list(mct_list);
}

/** mct_list_operate_nodes:
 *
 **/
void mct_list_operate_nodes(mct_list_t *mct_list,
  mct_list_operate_func list_operate, void *user_data)
{
  mct_list_t *temp_list;
  uint32_t num, idx;

  if (mct_list && list_operate) {
    num = mct_list->next_num;
    if (num > 0 && mct_list->next) {
      temp_list = mct_list->next[0];
      idx = 0;
      while (num--) {
        list_operate(mct_list->data, temp_list->data, user_data);
        mct_list_operate_nodes(temp_list, list_operate, user_data);
        idx++;
        if (num)
          temp_list = mct_list->next[idx];
      }
    }
  }
}

/** mct_list_operate_nodes_multiple_data:
 *
 **/
void mct_list_operate_nodes_multiple_data(mct_list_t *mct_list,
  mct_list_operate_func_multiple_data list_operate,
  void *user_data1, void *user_data2)
{
  mct_list_t *temp_list;
  uint32_t num, idx;

  if (mct_list && list_operate) {
    num = mct_list->next_num;
    if (num > 0 && mct_list->next) {
      temp_list = mct_list->next[0];
      idx = 0;
      while (num--) {
        list_operate(mct_list->data, temp_list->data, user_data1, user_data2);
        mct_list_operate_nodes_multiple_data(temp_list, list_operate,
          user_data1, user_data2);
        idx++;
        if (num)
          temp_list = mct_list->next[idx];
      }
    }
  }
}


/** mct_list_return_node_by_idx:
 *    @list: list to be traversed
 *    @node_idx: Node index
 *  Traverse whole list linearly up to node index provided
 *  and return node pointer.
 **/

void *mct_list_return_node_by_idx(mct_list_t *mct_list,
                  uint32_t node_idx)
{
  boolean rc = TRUE;
  uint32_t idx = 1;
  void *temp_data;

  if (!mct_list) {
    return NULL;
  }

  do {
     temp_data = mct_list->data;
     if (mct_list->next) {
       mct_list = mct_list->next[0];
       idx++;
     } else {
       break;
     }
     if (mct_list == NULL) {
       break;
     }
  }while (idx <= node_idx);
  return temp_data;
}
