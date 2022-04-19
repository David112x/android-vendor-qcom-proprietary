/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#include <framework/Log.h>
#include <framework/PendingMessageList.h>

PendingMessageList::PendingMessageList(std::string name) : mName(name), mToken(0) {}

bool PendingMessageList::empty() {
  return mList.empty();
}

bool PendingMessageList::size() {
  return mList.size();
}

std::shared_ptr<Message> PendingMessageList::pop() {
  std::shared_ptr<Message> msg = nullptr;
  std::lock_guard<qtimutex::QtiSharedMutex> lock(mMutex);
  auto iter = mList.begin();
  if (iter != mList.end()) {
    PendingMessage pendingMsg = iter->second;
    msg = pendingMsg.mMessage;
    mList.erase(iter);
  }
  return msg;
}

std::pair<uint16_t, bool> PendingMessageList::insert(std::shared_ptr<Message> message) {
  bool result = false;
  uint16_t token = 0;
  std::lock_guard<qtimutex::QtiSharedMutex> lock(mMutex);
  if (message != nullptr) {
    // Find a free token
    uint16_t current = mToken++;
    while ((mList.find(mToken) != mList.end()) && current != mToken) {
      mToken++;
    }
    if (current != mToken) {
      token = mToken;
      PendingMessage val{message};
      mList.insert({token, val});
      result = true;
    } else {
      // PendingMessageList is full!!
    }
  }
  return std::make_pair(token, result);
}

std::shared_ptr<Message> PendingMessageList::find(uint16_t token) {
  std::shared_ptr<Message> msg = nullptr;
  std::lock_guard<qtimutex::QtiSharedMutex> lock(mMutex);
  auto search = mList.find(token);
  if (search != mList.end()) {
    msg = search->second.mMessage;
  }
  return msg;
}

std::shared_ptr<Message> PendingMessageList::find(message_id_t messageId) {
  std::shared_ptr<Message> msg = nullptr;
  std::lock_guard<qtimutex::QtiSharedMutex> lock(mMutex);
  for (auto it = mList.begin(); it != mList.end(); it++) {
    if (it->second.mMessage->get_message_id() == messageId) {
      msg = it->second.mMessage;
      break;
    }
  }
  return msg;
}

std::shared_ptr<Message> PendingMessageList::extract(uint16_t token) {
  std::shared_ptr<Message> msg = nullptr;
  std::lock_guard<qtimutex::QtiSharedMutex> lock(mMutex);
  auto search = mList.find(token);
  if (search != mList.end()) {
    msg = search->second.mMessage;
    mList.erase(search);
  }
  return msg;
}
std::shared_ptr<Message> PendingMessageList::extract(message_id_t messageId) {
  std::shared_ptr<Message> msg = nullptr;
  std::lock_guard<qtimutex::QtiSharedMutex> lock(mMutex);
  for (auto it = mList.begin(); it != mList.end(); it++) {
    if (it->second.mMessage->get_message_id() == messageId) {
      msg = it->second.mMessage;
      mList.erase(it);
      break;
    }
  }
  return msg;
}

bool PendingMessageList::erase(uint16_t token) {
  bool result = false;
  std::lock_guard<qtimutex::QtiSharedMutex> lock(mMutex);
  auto search = mList.find(token);
  if (search != mList.end()) {
    mList.erase(search);
    result = true;
  }
  return result;
}

bool PendingMessageList::erase(std::shared_ptr<Message> message) {
  bool result = false;
  uint16_t token = -1;
  std::lock_guard<qtimutex::QtiSharedMutex> lock(mMutex);

  for (auto it = mList.begin(); it != mList.end(); it++) {
    if (it->second.mMessage == message) {
      token = it->first;
      break;
    }
  }
  if (token != -1) {
    auto search = mList.find(token);
    if (search != mList.end()) {
      mList.erase(search);
      result = true;
    }
  }
  return result;
}

void PendingMessageList::clear() {
  std::lock_guard<qtimutex::QtiSharedMutex> lock(mMutex);
  mList.clear();
}

void PendingMessageList::print() {
  std::lock_guard<qtimutex::QtiSharedMutex> lock(mMutex);
  std::string logString = "[";
  for (auto entry : mList) {
    logString = logString + "(token: " + std::to_string(entry.first) +
                " msg: " + entry.second.mMessage->to_string() + ") ";
  }
  if (mList.empty()) {
    logString += "<empty>";
  }
  logString += "]";
  Log::getInstance().d("PendingMessageList[" + mName + "]: " + logString);
}
