#include <assert.h>
#include <fstream>
#include <iostream>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"
#include "processor.h"

using LinuxParser::CPUStates;

float Processor::Utilization() {

  std::vector<int> cpuTimes_(LinuxParser::CpuUtilization());

  unsigned long systemalltime, idlealltime, totaltime, virtalltime;

  idlealltime = cpuTimes_[CPUStates::kIdle_] + cpuTimes_[CPUStates::kIOwait_];
  virtalltime =
      cpuTimes_[CPUStates::kGuest_] + cpuTimes_[CPUStates::kGuestNice_];

  systemalltime =
      (cpuTimes_[CPUStates::kSystem_] + cpuTimes_[CPUStates::kIRQ_] +
       cpuTimes_[CPUStates::kSoftIRQ_]);

  totaltime = (systemalltime + idlealltime + virtalltime +
               cpuTimes_[CPUStates::kUser_] + cpuTimes_[CPUStates::kNice_] +
               cpuTimes_[CPUStates::kSteal_]);

  float currentIdleTime = idlealltime - pre_idlealltime_;
  float currenttotaltime = totaltime - pre_totaltime_;

  float fraction = currentIdleTime / currenttotaltime;

  if (pre_cpuTimes_.empty()) {
    for (int value : cpuTimes_)
      pre_cpuTimes_.push_back(value);

    updatePreTimes();
  }

  return 1 - fraction;
}

void Processor::updatePreTimes() {
  pre_totaltime_ =
      std::accumulate(pre_cpuTimes_.begin(), pre_cpuTimes_.end(), 0);
  pre_idlealltime_ =
      pre_cpuTimes_[CPUStates::kIdle_] + pre_cpuTimes_[CPUStates::kIOwait_];
}