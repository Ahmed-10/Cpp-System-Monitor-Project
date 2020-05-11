#include <algorithm>
#include <cstddef>
#include <set>
#include <string>
#include <unistd.h>
#include <vector>

#include "linux_parser.h"
#include "process.h"
#include "processor.h"
#include "system.h"

using std::set;
using std::size_t;
using std::string;
using std::vector;

System::System() {
  this->operatingSystem_ = LinuxParser::OperatingSystem();
  this->kernel_ = LinuxParser::Kernel();
}

// TODO: Return the system's CPU
Processor &System::Cpu() { return cpu_; }

// TODO: Return a container composed of the system's processes
vector<Process> &System::Processes() {
  bool change = false;
  if (processes_.size() > 0) {
    vector<int> id_s = LinuxParser::Pids();

    for (int id : id_s) {
      if (process_id_.end() ==
          std::find(process_id_.begin(), process_id_.end(), id)) {
        process_id_.push_back(id);
        processes_.push_back(Process(id));
        change = true;
      }
    }

    for (size_t i = 0; i < processes_.size(); i++) {
      if (id_s.end() ==
          std::find(id_s.begin(), id_s.end(), processes_[i].Pid())) {
        processes_.erase(processes_.begin() + i);
        process_id_.erase(process_id_.begin() + i);
        change = true;
      }
    }
  } else {
    process_id_ = LinuxParser::Pids();
    change = true;
    for (int id : process_id_)
      processes_.push_back(Process(id));
  }

  if (change)
    std::sort(processes_.begin(), processes_.end(),
              [](Process &process1, Process &process2) {
                return process1 < process2;
              });

  return processes_;
}

// Return the system's kernel identifier (string)
std::string System::Kernel() { return this->kernel_; }

// Return the system's memory utilization
float System::MemoryUtilization() { return LinuxParser::MemoryUtilization(); }

// Return the operating system name
std::string System::OperatingSystem() { return this->operatingSystem_; }

// Return the number of processes actively running on the system
int System::RunningProcesses() { return LinuxParser::RunningProcesses(); }

// Return the total number of processes on the system
int System::TotalProcesses() { return LinuxParser::TotalProcesses(); }

// Return the number of seconds since the system started running
long int System::UpTime() { return LinuxParser::UpTime(); }