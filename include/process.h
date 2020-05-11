#ifndef PROCESS_H
#define PROCESS_H

#include <string>
/*
Basic class for Process representation
It contains relevant attributes as shown below
*/
class Process {
public:
  Process(int id);

  int Pid();
  std::string User();
  std::string Command();
  float CpuUtilization();
  std::string Ram();
  long int UpTime();
  bool operator<(Process const &a) const;

private:
  int pid_;
  int uid_;
  long start_time_;
  float cpu_utilization_;
  float ram_;
  std::string username_;
  std::string command_;
};

#endif