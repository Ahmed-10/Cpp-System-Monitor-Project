#include <cctype>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>

#include "linux_parser.h"
#include "process.h"

using std::string;
using std::to_string;
using std::vector;

Process::Process(int id) : pid_(id) {
  uid_ = LinuxParser::Uid(id);
  username_ = LinuxParser::User(uid_);
  start_time_ = LinuxParser::UpTime(id);
  command_ = LinuxParser::Command(id);
  if (command_.size() > 50) {
    command_.resize(47);
    command_ = command_ + "...";
  }
}

// TODO: Return this process's ID
int Process::Pid() { return pid_; }

float Process::CpuUtilization() {
  float activeJiffies = LinuxParser::ActiveJiffies(pid_);
  float totalJiffies = LinuxParser::Jiffies() - start_time_;
  cpu_utilization_ = activeJiffies / totalJiffies;
  return cpu_utilization_;
}

string Process::Command() { return command_; }

string Process::Ram() {
  ram_ = LinuxParser::Ram(pid_) / 1024.0;
  std::stringstream ram;
  ram << std::fixed << std::setprecision(1) << ram_;
  return ram.str();
}

string Process::User() { return username_; }

long int Process::UpTime() {
  long start_time = start_time_ / sysconf(_SC_CLK_TCK);
  return LinuxParser::UpTime() - start_time;
}

bool Process::operator<(Process const &a) const { return this->ram_ > a.ram_; }