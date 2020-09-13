#include <unistd.h>
#include <cctype>
#include <ctime>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"
#include "process.h"

using std::string;
using std::to_string;
using std::vector;

// Constructor.
Process::Process(int pid) : pid_(pid) {}

int Process::Pid() { return pid_; }

void Process::GetCpuUtil() {
  // Reference:
  // https://stackoverflow.com/questions/16726779/how-do-i-get-the-total-cpu-usage-of-an-application-from-proc-pid-stat/16736599
  auto totalTimeJiff =
      LinuxParser::ActiveJiffies(pid_); /* Active CPU jiffie. */
  float cpuUsageTime = static_cast<float>(
      totalTimeJiff / sysconf(_SC_CLK_TCK)); /* Number of ticks. */
  float totalProcessTime = static_cast<float>(
      LinuxParser::UpTime(pid_)); /* Process uptime sine bootup. */
  this->cpuUsageNow_ =
      (cpuUsageTime - cpuUsageOld_) /
      (totalProcessTime - totalProcessTimeOld_); /* Current Process CPU usage.
                                                    Compare with previous one.*/

  cpuUsageOld_ = cpuUsageTime;
  totalProcessTimeOld_ = totalProcessTime;
}

float Process::CpuUtilization() { return cpuUsageNow_; }

string Process::Command() { return LinuxParser::Command(pid_); }

string Process::Ram() { return LinuxParser::Ram(pid_); }

string Process::User() { return LinuxParser::User(pid_); }

long int Process::UpTime() { return LinuxParser::UpTime(pid_); }

bool Process::operator<(Process const& a) const {
  return (a.cpuUsageNow_ < this->cpuUsageNow_);
}