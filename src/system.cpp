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

std::vector<double> get_per_core_cpu_usage() {
  static std::vector<unsigned long long> prev_total;
  static std::vector<unsigned long long> prev_idle;

  std::ifstream file("/proc/stat");
  std::string label;

  std::vector<unsigned long long> curr_total;
  std::vector<unsigned long long> curr_idle;
  std::vector<double> usage;

  while (std::getline(file, label)) {
    // Keep only lines starting with cpu followed by a digit: cpu0, cpu1, ...
    if (label.size() < 4 || label.substr(0, 3) != "cpu" ||
        !std::isdigit(static_cast<unsigned char>(label[3]))) {
      continue;
    }

    std::istringstream iss(label);

    std::string cpu_name;
    unsigned long long user, nice, system, idle, iowait;
    unsigned long long irq, softirq, steal;

    iss >> cpu_name >> user >> nice >> system >> idle >> iowait >> irq >>
        softirq >> steal;

    unsigned long long idle_time = idle + iowait;
    unsigned long long total_time =
        user + nice + system + idle + iowait + irq + softirq + steal;

    curr_idle.push_back(idle_time);
    curr_total.push_back(total_time);
  }

  // First call initializes state
  if (prev_total.empty()) {
    prev_total = curr_total;
    prev_idle = curr_idle;
    return std::vector<double>(curr_total.size(), 0.0);
  }

  usage.resize(curr_total.size());

  for (size_t i = 0; i < curr_total.size(); ++i) {
    unsigned long long total_diff = curr_total[i] - prev_total[i];
    unsigned long long idle_diff = curr_idle[i] - prev_idle[i];

    if (total_diff == 0) {
      usage[i] = 0.0;
    } else {
      usage[i] = 100.0 * (total_diff - idle_diff) / total_diff;
    }
  }

  prev_total = curr_total;
  prev_idle = curr_idle;

  return usage;
}

double get_cpu_temperature() {
  const char *paths[] = {"/sys/class/thermal/thermal_zone0/temp",
                         "/sys/class/hwmon/hwmon0/temp1_input"};

  for (const char *path : paths) {
    std::ifstream file(path);

    if (file) {
      double temp;
      file >> temp;
      if (temp > 1000.0)
        temp /= 1000.0;
      return temp;
    }
  }

  return -1.0;
}

double get_battery_percentage() {
  std::ifstream file("/sys/class/power_supply/BAT0/capacity");
  double value;
  if (file >> value)
    return value;
  return -1.0;
}

double get_cpu_frequency_mhz() {
  std::ifstream file("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq");

  double freq_khz;
  if (file >> freq_khz)
    return freq_khz / 1000.0;

  return -1.0;
}

unsigned long long get_disk_read_speed() {
  static unsigned long long prev = 0;

  std::ifstream file("/proc/diskstats");
  std::string line;
  unsigned long long sectors = 0;

  while (std::getline(file, line)) {
    std::istringstream iss(line);

    unsigned long long major, minor;
    std::string name;
    unsigned long long reads_completed, reads_merged;
    unsigned long long sectors_read;
    unsigned long long writes_completed, writes_merged;
    unsigned long long sectors_written;

    if (!(iss >> major >> minor >> name >> reads_completed >> reads_merged >>
          sectors_read >> writes_completed >> writes_merged >> sectors_written))
      continue;

    // Use your main SSD. Adjust if needed.
    if (name == "nvme0n1" || name == "sda") {
      sectors = sectors_read;
      break;
    }
  }

  unsigned long long bytes = sectors * 512ULL;
  unsigned long long diff = bytes - prev;
  prev = bytes;
  return diff;
}

unsigned long long get_disk_write_speed() {
  static unsigned long long prev = 0;

  std::ifstream file("/proc/diskstats");
  std::string line;
  unsigned long long sectors = 0;

  while (std::getline(file, line)) {
    std::istringstream iss(line);

    unsigned long long major, minor;
    std::string name;
    unsigned long long reads_completed, reads_merged;
    unsigned long long sectors_read;
    unsigned long long writes_completed, writes_merged;
    unsigned long long sectors_written;

    if (!(iss >> major >> minor >> name >> reads_completed >> reads_merged >>
          sectors_read >> writes_completed >> writes_merged >> sectors_written))
      continue;

    if (name == "nvme0n1" || name == "sda") {
      sectors = sectors_written;
      break;
    }
  }

  unsigned long long bytes = sectors * 512ULL;
  unsigned long long diff = bytes - prev;
  prev = bytes;
  return diff;
}

std::string get_battery_status() {
  std::ifstream file("/sys/class/power_supply/BAT0/status");
  std::string status;

  if (std::getline(file, status)) {
    return status;
  }

  return "Unavailable";
}