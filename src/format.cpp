#include <string>

#include "format.h"

using std::string;

// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
// REMOVE: [[maybe_unused]] once you define the function
string Format::ElapsedTime(long seconds) {
  int hour = 0;
  int min = 0;
  int sec = 0;
  string time{};

  hour = seconds / 3600;
  seconds = seconds % 3600;
  min = seconds / 60;
  seconds = seconds % 60;
  sec = seconds;

  time = std::to_string(hour) + ":" + std::to_string(min) + ":" +
         std::to_string(sec);

  return time;
}