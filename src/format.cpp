#include <iostream>
#include <string>

#include "format.h"

using std::string;

string Format::ElapsedTime(long seconds) {
  long min = seconds / 60;
  long sec = seconds % 60;

  long hr = min / 60;
  min %= 60;

  string time = To_String(hr) + ":" + To_String(min) + ":" + To_String(sec);
  return time;
}

string Format::To_String(long time) {
  string time_ = std::to_string(time);
  return string(2 - time_.length(), '0') + time_;
}