/******************************************************************************
 *
 *  Copyright (C) 2017 Google, Inc.
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
#include <gtest/gtest.h>

#include "btif/include/btif_profile_queue.h"
#include "stack_manager.h"

static bool sStackRunning;

bool get_stack_is_running(void) { return sStackRunning; }

bt_status_t btif_test_queue_connect(uint16_t uuid, const RawAddress* bda,
                               btif_connect_cb_t connect_cb) {
  return btif_queue_connect(uuid, *bda, connect_cb, 1);
}

static stack_manager_t sStackManager = {nullptr, nullptr, nullptr, nullptr,
                                        get_stack_is_running};

const stack_manager_t* stack_manager_get_interface() { return &sStackManager; }

typedef void(tBTIF_CBACK)(uint16_t event, char* p_param);
typedef void(tBTIF_COPY_CBACK)(uint16_t event, char* p_dest, char* p_src);
bt_status_t btif_transfer_context(tBTIF_CBACK* p_cback, uint16_t event,
                                  char* p_params, int param_len,
                                  tBTIF_COPY_CBACK* p_copy_cback) {
  p_cback(event, p_params);
  return BT_STATUS_SUCCESS;
}

enum ResultType {
  NOT_SET = 0,
  UNKNOWN,
  UUID1_ADDR1,
  UUID1_ADDR2,
  UUID2_ADDR1,
  UUID2_ADDR2
};

static ResultType sResult;

class BtifProfileQueueTest : public ::testing::Test {
 public:
  static const uint16_t kTestUuid1 = 0x9527;
  static const uint16_t kTestUuid2 = 0x819F;
  static const RawAddress kTestAddr1;
  static const RawAddress kTestAddr2;

 protected:
  void SetUp() override {
    sStackRunning = true;
    sResult = NOT_SET;
  };
  void TearDown() override { btif_queue_release(); };
};

const RawAddress BtifProfileQueueTest::kTestAddr1{
    {0x11, 0x22, 0x33, 0x44, 0x55, 0x66}};
const RawAddress BtifProfileQueueTest::kTestAddr2{
    {0xAB, 0xCD, 0xEF, 0x12, 0x34, 0x56}};

static bt_status_t test_connect_cb(RawAddress* bda, uint16_t uuid) {
  sResult = UNKNOWN;
  if (*bda == BtifProfileQueueTest::kTestAddr1) {
    if (uuid == BtifProfileQueueTest::kTestUuid1) {
      sResult = UUID1_ADDR1;
    } else if (uuid == BtifProfileQueueTest::kTestUuid2) {
      sResult = UUID2_ADDR1;
    }
  } else if (*bda == BtifProfileQueueTest::kTestAddr2) {
    if (uuid == BtifProfileQueueTest::kTestUuid1) {
      sResult = UUID1_ADDR2;
    } else if (uuid == BtifProfileQueueTest::kTestUuid2) {
      sResult = UUID2_ADDR2;
    }
  }
  return BT_STATUS_SUCCESS;
}

TEST_F(BtifProfileQueueTest, test_connect) {
  sResult = NOT_SET;
  btif_test_queue_connect(kTestUuid1, &kTestAddr1, test_connect_cb);
  EXPECT_EQ(sResult, UUID1_ADDR1);
}

TEST_F(BtifProfileQueueTest, test_connect_same_uuid_do_not_repeat) {
  sResult = NOT_SET;
  btif_test_queue_connect(kTestUuid1, &kTestAddr1, test_connect_cb);
  EXPECT_EQ(sResult, UUID1_ADDR1);
  // Second connection request on the same UUID do not repeat
  sResult = NOT_SET;
  btif_test_queue_connect(kTestUuid1, &kTestAddr1, test_connect_cb);
  EXPECT_EQ(sResult, NOT_SET);
  // Not even after we advance the queue
  sResult = NOT_SET;
  btif_queue_advance();
  btif_queue_connect_next();
  EXPECT_EQ(sResult, NOT_SET);
}

TEST_F(BtifProfileQueueTest, test_multiple_connects) {
  // First item is executed
  sResult = NOT_SET;
  btif_test_queue_connect(kTestUuid1, &kTestAddr1, test_connect_cb);
  EXPECT_EQ(sResult, UUID1_ADDR1);
  // Second item with advance is executed
  sResult = NOT_SET;
  btif_queue_advance();
  btif_test_queue_connect(kTestUuid2, &kTestAddr1, test_connect_cb);
  EXPECT_EQ(sResult, UUID2_ADDR1);
}

TEST_F(BtifProfileQueueTest, test_multiple_connects_without_advance) {
  // First item is executed
  sResult = NOT_SET;
  btif_test_queue_connect(kTestUuid1, &kTestAddr1, test_connect_cb);
  EXPECT_EQ(sResult, UUID1_ADDR1);
  // Second item without advance is not executed
  sResult = NOT_SET;
  btif_test_queue_connect(kTestUuid2, &kTestAddr1, test_connect_cb);
  EXPECT_EQ(sResult, NOT_SET);
  sResult = NOT_SET;
  // Connect next doesn't work
  btif_queue_connect_next();
  EXPECT_EQ(sResult, NOT_SET);
  // Advance moves queue to execute next item
  sResult = NOT_SET;
  btif_queue_advance();
  EXPECT_EQ(sResult, UUID2_ADDR1);
}

TEST_F(BtifProfileQueueTest, test_cleanup_first_allow_second) {
  // First item is executed
  sResult = NOT_SET;
  btif_test_queue_connect(kTestUuid1, &kTestAddr1, test_connect_cb);
  EXPECT_EQ(sResult, UUID1_ADDR1);
  // Second item without advance is not executed
  sResult = NOT_SET;
  btif_test_queue_connect(kTestUuid2, &kTestAddr1, test_connect_cb);
  EXPECT_EQ(sResult, NOT_SET);
  // Connect next doesn't work
  sResult = NOT_SET;
  btif_queue_connect_next();
  EXPECT_EQ(sResult, NOT_SET);
  // Cleanup UUID1 allows the next profile connection to be executed
  sResult = NOT_SET;
  btif_queue_cleanup(kTestUuid1);
  btif_queue_connect_next();
  EXPECT_EQ(sResult, UUID2_ADDR1);
}

TEST_F(BtifProfileQueueTest, test_cleanup_both) {
  // First item is executed
  sResult = NOT_SET;
  btif_test_queue_connect(kTestUuid1, &kTestAddr1, test_connect_cb);
  EXPECT_EQ(sResult, UUID1_ADDR1);
  // Second item without advance is not executed
  sResult = NOT_SET;
  btif_test_queue_connect(kTestUuid2, &kTestAddr1, test_connect_cb);
  EXPECT_EQ(sResult, NOT_SET);
  // Connect next doesn't work
  sResult = NOT_SET;
  btif_queue_connect_next();
  EXPECT_EQ(sResult, NOT_SET);
  // Cleanup both leaves nothing to execute
  sResult = NOT_SET;
  btif_queue_cleanup(kTestUuid1);
  btif_queue_cleanup(kTestUuid2);
  btif_queue_connect_next();
  EXPECT_EQ(sResult, NOT_SET);
}

TEST_F(BtifProfileQueueTest, test_cleanup_both_reverse_order) {
  // First item is executed
  sResult = NOT_SET;
  btif_test_queue_connect(kTestUuid1, &kTestAddr1, test_connect_cb);
  EXPECT_EQ(sResult, UUID1_ADDR1);
  // Second item without advance is not executed
  sResult = NOT_SET;
  btif_test_queue_connect(kTestUuid2, &kTestAddr1, test_connect_cb);
  EXPECT_EQ(sResult, NOT_SET);
  // Connect next doesn't work
  sResult = NOT_SET;
  btif_queue_connect_next();
  EXPECT_EQ(sResult, NOT_SET);
  // Cleanup both in reverse order leaves nothing to execute
  sResult = NOT_SET;
  btif_queue_cleanup(kTestUuid2);
  btif_queue_cleanup(kTestUuid1);
  btif_queue_connect_next();
  EXPECT_EQ(sResult, NOT_SET);
}
