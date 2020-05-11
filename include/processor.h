#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <string>
#include <vector>

class Processor {
public:
  float Utilization();

private:
  void updatePreTimes();

  unsigned long pre_idlealltime_;
  unsigned long pre_totaltime_;
  std::vector<int> pre_cpuTimes_;
};

#endif