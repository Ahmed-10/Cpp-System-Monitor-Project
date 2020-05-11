#include <iostream>

#include "ncurses_display.h"
#include "process.h"
#include "processor.h"
#include "system.h"

#include "linux_parser.h"

int main() {
  System system;
  // auto p = system.Processes();
  NCursesDisplay::Display(system, 20);
  // Process process(2422);
  // std::cout << process.Pid() << "\n";
  // std::cout << process.User() << "\n";
  // std::cout << process.UpTime() << "\n";
  // std::cout << process.Ram() << "\n";
  // std::cout << process.CpuUtilization() << "\n";
  // std::cout << process.Command() << "\n";
}