#include "system.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/statvfs.h>
#include <vector>

namespace fs = std::filesystem;

// =========================
// CPU STATS
// =========================

CpuStats real_cpu_stats() {
  std::ifstream file("/proc/stat");
  if (!file.is_open()) {
    throw std::runtime_error("Failded to open /proc/stat");
  }

  std::string line;
  std::getline(file, line);

  std::istringstream iss(line);

  std::string cpu_label;
  CpuStats stats{};

  iss >> cpu_label >> stats.user >> stats.nice >> stats.system >> stats.idle >>
      stats.iowait >> stats.irq >> stats.softirq >> stats.steal;

  if (cpu_label != "cpu") {
    throw std::runtime_error("Unexpected format in /proc/stat");
  }

  return stats;
}

double calculate_cpu_usage(const CpuStats &prev, const CpuStats &curr) {
  long long prevIdle = prev.idle + prev.iowait;
  long long currIdle = curr.idle + curr.iowait;

  long long prevNonIdle = prev.user + prev.nice + prev.system + prev.irq +
                          prev.softirq + prev.steal;
  long long currNonIdle = curr.user + curr.nice + curr.system + curr.irq +
                          curr.softirq + curr.steal;

  long long prevTotal = prevIdle + prevNonIdle;
  long long currTotal = currIdle + currNonIdle;

  long long totald = currTotal - prevTotal;
  long long idled = currIdle - prevIdle;

  if (totald == 0)
    return 0.0;

  return (double)(totald - idled) * 100.0 / (double)totald;
}

// =========================
// MEMORY STATS
// =========================

MemoryStats read_memory_stats() {
  std::ifstream file("/proc/meminfo");

  if (!file.is_open()) {
    throw std::runtime_error("Failed to open /proc/meminfo");
  }

  MemoryStats mem{};

  std::string line;

  while (std::getline(file, line)) {
    std::istringstream iss(line);

    std::string key;
    long long value;
    std::string unit;

    iss >> key >> value >> unit;

    if (key == "MemTotal:") {
      mem.total_kb = value;
    } else if (key == "MemAvailable:") {
      mem.available_kb = value;
    }
  }

  return mem;
}

double calculate_memory_usage(const MemoryStats &mem) {
  long long used_kb = mem.total_kb - mem.available_kb;

  return (double)used_kb * 100.0 / (double)mem.total_kb;
}

// =========================
// UPTIME STATS
// =========================

double read_uptime_seconds() {
  std::ifstream file("/proc/uptime");

  if (!file.is_open()) {
    throw std::runtime_error("Failed to open /proc/uptime");
  }

  double uptime_seconds;

  file >> uptime_seconds;

  return uptime_seconds;
}

// =========================
// PROCESS COUNT
// =========================

static bool is_numeric_name(const std::string &name) {
  if (name.empty()) {
    return false;
  }

  for (char ch : name) {
    if (!std::isdigit(static_cast<unsigned char>(ch))) {
      return false;
    }
  }

  return true;
}

int count_processes() {
  int count = 0;

  for (const auto &entry : std::filesystem::directory_iterator("/proc")) {
    if (!entry.is_directory()) {
      continue;
    }

    std::string name = entry.path().filename().string();

    if (is_numeric_name(name)) {
      count++;
    }
  }

  return count;
}

// =========================
// PROCESS LIST
// =========================

std::vector<ProcessInfo> read_process_list() {
  std::vector<ProcessInfo> processes;

  for (const auto &entry : fs::directory_iterator("/proc")) {
    if (!entry.is_directory()) {
      continue;
    }

    std::string pid_str = entry.path().filename().string();

    if (!is_numeric_name(pid_str)) {
      continue;
    }

    int pid = std::stoi(pid_str);

    ProcessInfo proc{};
    proc.pid = pid;
    proc.name = "unknown";
    proc.state = '?';
    proc.memory_kb = 0;

    {
      std::ifstream comm_file("/proc/" + pid_str + "/comm");
      if (comm_file.is_open()) {
        std::getline(comm_file, proc.name);
        if (!proc.name.empty() && proc.name.back() == '\n') {
          proc.name.pop_back();
        }
      }
    }

    {
      std::ifstream status_file("/proc/" + pid_str + "/status");
      if (status_file.is_open()) {
        std::string line;
        while (std::getline(status_file, line)) {
          if (line.rfind("State:", 0) == 0) {
            std::istringstream iss(line);
            std::string key;
            iss >> key >> proc.state;
          } else if (line.rfind("VmRSS:", 0) == 0) {
            std::istringstream iss(line);
            std::string key;
            std::string unit;
            iss >> key >> proc.memory_kb >> unit;
          }
        }
      }
    }

    processes.push_back(proc);
  }

  return processes;
}

// =========================
// DISK STATS
// =========================

DiskStats get_disk_stats(const std::string &path) {
  struct statvfs fs_stats;

  if (statvfs(path.c_str(), &fs_stats) != 0) {
    throw std::runtime_error("Failed to get disk state from path: " + path);
  }

  DiskStats disk{};

  disk.total_bytes = static_cast<unsigned long long>(fs_stats.f_blocks) *
                     static_cast<unsigned long long>(fs_stats.f_frsize);

  disk.free_bytes = static_cast<unsigned long long>(fs_stats.f_bavail) *
                    static_cast<unsigned long long>(fs_stats.f_frsize);

  return disk;
}

double calculate_disk_usage(const DiskStats &disk) {
  if (disk.total_bytes == 0) {
    return 0.0;
  }

  unsigned long long used_bytes = disk.total_bytes - disk.free_bytes;

  return static_cast<double>(used_bytes) * 100.0 /
         static_cast<double>(disk.total_bytes);
}

// =========================
// NETWORK STATS
// =========================

NetworkStats get_network_stats(const std::string &interface) {
  std::ifstream file("/proc/net/dev");

  if (!file.is_open()) {
    throw std::runtime_error("Failed to open /proc/net/dev");
  }

  NetworkStats net{};

  std::string line;

  std::getline(file, line);
  std::getline(file, line);

  while (std::getline(file, line)) {
    std::size_t pos = line.find_first_not_of(" ");
    if (pos != std::string::npos) {
      line = line.substr(pos);
    }

    if (line.rfind(interface + ":", 0) == 0) {
      std::replace(line.begin(), line.end(), ':', ' ');

      std::istringstream iss(line);

      std::string iface;
      iss >> iface;

      iss >> net.rx_bytes;

      for (int i = 0; i < 7; i++) {
        unsigned long long dummy;
        iss >> dummy;
      }

      iss >> net.tx_bytes;

      return net;
    }
  }

  throw std::runtime_error("Network interface not found: " + interface);
}