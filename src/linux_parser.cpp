#include <dirent.h>
#include <iostream>
#include <numeric>
#include <string>
#include <unistd.h>
#include <vector>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

static unsigned int totalProcesses_;
static unsigned int runningProcesses_;

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

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR *directory = opendir(kProcDirectory.c_str());
  struct dirent *file;
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

float LinuxParser::MemoryUtilization() {
  unsigned long freeMem, availMem, cached_, sReclaim, sharedMem;
  int dummyValue;
  bool flag = true;
  float totalMem, usedMem, buffers, cachedMem;

  string line, key;
  std::ifstream statFile(LinuxParser::kProcDirectory +
                         LinuxParser::kMeminfoFilename);

  if (statFile.is_open()) {
    while (getline(statFile, line) && flag) {
      std::stringstream lineStream(line);
      lineStream >> key;

      switch (key[0]) {
      case 'M':
        if (key == "MemTotal:") {
          lineStream >> totalMem;
        } else if (key == "MemFree:") {
          lineStream >> freeMem;
        } else if (key == "MemAvailable:") {
          lineStream >> availMem;
        } else {
          lineStream >> dummyValue;
        }
        break;
      case 'B':
        lineStream >> buffers;
        break;
      case 'C':
        lineStream >> cached_;
        break;
      case 'S':
        if (key == "Shmem:") {
          lineStream >> sharedMem;
        } else if (key == "SReclaimable:") {
          lineStream >> sReclaim;
          flag = false;
        } else {
          lineStream >> dummyValue;
        }
        break;
      default:
        lineStream >> dummyValue;
        break;
      }
    }
  }

  usedMem = totalMem - freeMem;
  cachedMem = cached_ + sReclaim - sharedMem;

  return ((usedMem - (cachedMem + buffers)) / totalMem);
}

long LinuxParser::UpTime() {
  long seconds;
  string line;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> seconds;
  }
  return seconds;
}

long LinuxParser::Jiffies() { return UpTime() * sysconf(_SC_CLK_TCK); }

long LinuxParser::ActiveJiffies(int pid) {
  string line, temp;
  unsigned int iterator = 1;
  long utime, stime, cutime, cstime;

  std::ifstream statFile(kProcDirectory + to_string(pid) + kStatFilename);

  if (statFile.is_open()) {
    std::getline(statFile, line);
    std::stringstream stream(line);

    while (iterator <= 17) {
      switch (iterator) {
      case 14:
        stream >> utime;
        iterator++;
        break;
      case 15:
        stream >> stime;
        iterator++;
        break;
      case 16:
        stream >> cutime;
        iterator++;
        break;
      case 17:
        stream >> cstime;
        iterator++;
        break;

      default:
        stream >> temp;
        iterator++;
        break;
      }
    }
  }
  return utime + stime + cutime + cstime;
}

long LinuxParser::ActiveJiffies() {
  vector<int> cpu_times = CpuUtilization();
  long activeJiffies = std::accumulate(cpu_times.begin(), cpu_times.end(), 0);
  return activeJiffies;
}

long LinuxParser::IdleJiffies() {
  vector<int> cpu_times = CpuUtilization();
  return cpu_times[kIdle_] + cpu_times[kIOwait_];
}

vector<int> LinuxParser::CpuUtilization() {
  std::string line, key;
  unsigned int value, iterator = 0;
  vector<int> cpuTimes_;
  std::ifstream statFile(kProcDirectory + kStatFilename);

  if (statFile.is_open()) {
    std::getline(statFile, line);
    std::stringstream lineStream(line);

    lineStream >> key;
    if (key == "cpu") {
      while (lineStream >> value) {
        cpuTimes_.push_back(value);
        iterator++;
      }

      // userTimeCalibration
      cpuTimes_[kUser_] = cpuTimes_[kUser_] - cpuTimes_[kGuest_];
      cpuTimes_[kNice_] = cpuTimes_[kNice_] - cpuTimes_[kGuestNice_];
    }

    totalProcesses_ = 0;
    runningProcesses_ = 0;
    while (std::getline(statFile, line)) {
      std::stringstream lineStream(line);

      while (lineStream >> key >> value) {
        if (key == "processes") {
          totalProcesses_ = value;
          value = 0;
        }

        else if (key == "procs_running") {
          runningProcesses_ = value;
        }

        else {
          value = 0;
        }
      }
    }
  }
  return cpuTimes_;
}

int LinuxParser::TotalProcesses() { return totalProcesses_; }

int LinuxParser::RunningProcesses() { return runningProcesses_; }

string LinuxParser::Command(int pid) {
  string command;
  std::ifstream commandFile(kProcDirectory + to_string(pid) + kCmdlineFilename);

  if (commandFile.is_open()) {
    getline(commandFile, command);
    return command;
  }
  return string();
}

float LinuxParser::Ram(int pid) {
  string line, key;
  float value;

  std::ifstream statusFile(kProcDirectory + to_string(pid) + kStatusFilename);

  if (statusFile.is_open()) {
    while (getline(statusFile, line)) {
      std::stringstream stream(line);

      stream >> key;
      if (key == "VmData:") {
        stream >> value;
        return value;
      }
    }
  }
  return 0;
}

int LinuxParser::Uid(int pid) {
  string line, key;
  int uid;
  std::ifstream statusFile(kProcDirectory + to_string(pid) + kStatusFilename);

  if (statusFile.is_open()) {
    while (getline(statusFile, line)) {
      std::stringstream stream(line);

      stream >> key;
      if (key == "Uid:") {
        stream >> uid;
        return uid;
      }
    }
  }
  return 0;
}

string LinuxParser::User(int uid_) {
  string line, username, x;
  int uid;
  std::ifstream passFile(kPasswordPath);

  if (passFile.is_open()) {
    while (getline(passFile, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::stringstream stream(line);

      stream >> username >> x >> uid;
      if (uid == uid_)
        return username;
    }
  }
  return string();
}

long LinuxParser::UpTime(int pid) {
  string line, temp;
  unsigned int iterator = 1;

  std::ifstream statFile(kProcDirectory + to_string(pid) + kStatFilename);

  if (statFile.is_open()) {
    std::getline(statFile, line);
    std::stringstream stream(line);

    while (iterator < 22) {
      stream >> temp;
      iterator++;
    }
    stream >> temp;
  }
  return stol(temp);
}