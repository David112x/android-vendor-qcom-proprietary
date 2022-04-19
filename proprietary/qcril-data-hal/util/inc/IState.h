/*===========================================================================

  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/
#pragma once

#include <string>

using namespace std;

namespace SM {

template<class E>
class IState
{
private:
  const string _name;

protected:
  function<void(int)> transitionTo;

public:
  static constexpr int UNHANDLED = -1;
  static constexpr int HANDLED = 0;
  IState(string statename): IState(statename, [](int){}) {};
  IState(string statename, function<void(int)> setState): _name(statename), transitionTo(setState) {};
  virtual ~IState() {};

  virtual void enter(void) = 0;
  virtual void exit(void) = 0;
  virtual int handleEvent(const E& event) = 0;
  virtual const string& getName() {return _name;}
};

} /* namespace SM */
