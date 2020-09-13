#include <dirent.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;
using std::cout;

/**
 * Retrieve operating system name from /etc/os-release.
 */
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

/* Retrieve kernel version from /proc/version */
string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

/* Retrieve all the pids of active processes in the system. Read from /proc
 * entry. */
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

/* Retrieve total memory utilization. Read from /proc/meminfo */
float LinuxParser::MemoryUtilization() {
  float total_mem = 0.0f;
  float total_free_mem = 0.0f;
  float buffers_mem = 0.0f;
  string line{};
  string key{}, value{};
  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "MemTotal:") {
          total_mem = stof(value);
        } else if (key == "MemFree:") {
          total_free_mem = stof(value);
        } else if (key == "Buffers:") {
          buffers_mem = stof(value);
        }
      }
    }
    stream.close();
  }
  return (1.0f - (total_free_mem / (total_mem - buffers_mem)));
}

/* Retrieve Uptime of the system. Read from /proc/uptime */
long LinuxParser::UpTime() {
  string line;
  string k, val;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    if (getline(stream, line)) {
      std::istringstream is(line);
      is >> k >> val;
    }
  }
  return stoi(k + val);
}

/* Number of ticks sytem made since the last boot. */
long LinuxParser::Jiffies() {
  long uptime = LinuxParser::UpTime();
  return (uptime * sysconf(_SC_CLK_TCK));
}

/* Amount of time for the process scheduled since the boot. Read from
 * /proc/pid/stat. The actual jiffie
 * is the sum of 14-17 fields.
 */
long LinuxParser::ActiveJiffies(int pid) {
  string line;
  long active_jiffies = 0;
  string line_str;
  std::ifstream ifs(kProcDirectory + to_string(pid) + kStatFilename);
  if (ifs.is_open()) {
    getline(ifs, line);
    std::istringstream is(line);
    int iter = 1;
    while (getline(is, line_str, ' ')) {
      if (iter >= 14 && iter <= 17) {
        active_jiffies += stoi(line_str);
      }
      ++iter;
    }
  }
  return active_jiffies;
}

/* CPU time. Read from /proc/stat. The actual jiffie is the sum of all the time
 * entries. */
long LinuxParser::ActiveJiffies() {
  string line{};
  string key{};
  long active_jiffies = 0;

  std::ifstream ifs(kProcDirectory + kStatFilename);
  if (ifs.is_open()) {
    if (getline(ifs, line)) {
      std::istringstream is(line);
      while (is >> key) {
        if (key == "cpu") {
          continue;
        }
        active_jiffies += stol(key);
      }
    }
    ifs.close();
  }
  return active_jiffies;
}

/* Time spent in idle task. Read from /proc/stat and the actual jiffie is sum of
 * time spent in idle task
 * and time waiting for I/O to complete.
 */
long LinuxParser::IdleJiffies() {
  long idle_time = 0;
  long iowait_time = 0;
  long user_time = 0;
  long nice_time = 0;
  long system_time = 0;
  string line{};
  string cpu_str{};
  std::ifstream ifs(kProcDirectory + kStatFilename);
  if (ifs.is_open()) {
    if (getline(ifs, line)) {
      std::istringstream is(line);
      is >> cpu_str >> user_time >> nice_time >> system_time >> idle_time >>
          iowait_time;
    }
    ifs.close();
  }

  return (idle_time + iowait_time);
}

/* Total Cpu Utilization. */
vector<string> LinuxParser::CpuUtilization() {
  string line{};
  string key{};
  vector<string> cpu_utilization{};

  std::ifstream ifs(kProcDirectory + kStatFilename);
  if (ifs.is_open()) {
    if (getline(ifs, line)) {
      std::istringstream is(line);
      while (is >> key) {
        if (key == "cpu") {
          continue;
        }
        cpu_utilization.push_back(key);
      }
    }
    ifs.close();
  }

  return cpu_utilization;
}

/* Total processes. */
int LinuxParser::TotalProcesses() {
  string line{};
  int total_process = 0;
  string key{};
  string val{};

  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (getline(stream, line)) {
      std::istringstream istream(line);
      while (istream >> key >> val) {
        if (key == "processes") {
          total_process = stoi(val);
          break;
        }
      }
    }
    stream.close();
  }

  return total_process;
}

/* Running processes. */
int LinuxParser::RunningProcesses() {
  string line{};
  int running_process = 0;
  string key{};
  string val{};

  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (getline(stream, line)) {
      std::istringstream istream(line);
      while (istream >> key >> val) {
        if (key == "procs_running") {
          running_process = stoi(val);
          break;
        }
      }
    }
    stream.close();
  }
  return running_process;
}

/* Read from /proc/pid/cmdline. Commandline for the program.*/
string LinuxParser::Command(int pid) {
  string line{};
  std::ifstream ifs(kProcDirectory + to_string(pid) + kCmdlineFilename);
  if (ifs.is_open()) {
    if (getline(ifs, line)) {
      return line;
    }
  }
  return string();
}

/* Read from /proc/pid/status. Memory usage of the program.*/
string LinuxParser::Ram(int pid) {
  string line{};
  string key{};
  int val = 0;
  std::ifstream ifs(kProcDirectory + to_string(pid) + kStatusFilename);
  if (ifs.is_open()) {
    while (getline(ifs, line)) {
      std::istringstream is(line);
      while (is >> key >> val) {
        if (key == "VmSize:") {
          return to_string(val / 1000);
        }
      }
    }
  }
  return string();
}

/* Read from /proc/pid/status. User ID of the program.*/
string LinuxParser::Uid(int pid) {
  string line{};
  string key{};
  string val{};
  std::ifstream ifs(kProcDirectory + to_string(pid) + kStatusFilename);
  if (ifs.is_open()) {
    while (getline(ifs, line)) {
      std::istringstream is(line);
      while (is >> key >> val) {
        if (key == "Uid:") {
          return val;
        }
      }
    }
  }
  return string();
}

/* Read from /proc/pid/status. User name of the program.*/
string LinuxParser::User(int pid) {
  string uid = LinuxParser::Uid(pid);
  string line{};
  string split{};
  std::ifstream ifs(kPasswordPath);
  if (ifs.is_open()) {
    while (getline(ifs, line)) {
      /* user:x:uid */
      std::stringstream is(line);
      std::vector<string> user_str{};
      while (getline(is, split, ':')) {
        user_str.push_back(split);
      }
      if (user_str[2] == uid) {
        return user_str[0];
      }
    }
  }
  return string();
}

/* Read from /proc/pid/stat. Start time of the program is 22th entry.
 * Calculate total Uptime of process is obtained by find the difference with
 * total uptime of system.
 */
long LinuxParser::UpTime(int pid) {
  long uptime = 0;
  string line{};
  string value{};
  std::ifstream ifs(kProcDirectory + to_string(pid) + kStatFilename);
  if (ifs.is_open()) {
    std::getline(ifs, line);
    std::istringstream is(line);

    // Iter
    int iter = 1;
    while (std::getline(is, value, ' ')) {
      if (22 == iter) {
        uptime = atol(value.c_str());
        break;
      }
      ++iter;
    }
  }
  return UpTime() - uptime / sysconf(_SC_CLK_TCK);
}
