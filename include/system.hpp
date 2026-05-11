#pragma once

#include <string>
#include <vector>

// =========================
// CPU STATS
// =========================

struct CpuStats {
  long long user;
  long long nice;
  long long system;
  long long idle;
  long long iowait;
  long long irq;
  long long softirq;
  long long steal;
};

CpuStats real_cpu_stats();

double calculate_cpu_usage(const CpuStats &prev, const CpuStats &curr);

// =========================
// MEMORY STATS
// =========================

struct MemoryStats {
  long long total_kb;
  long long available_kb;
};

MemoryStats read_memory_stats();

double calculate_memory_usage(const MemoryStats &mem);

// =========================
// UPTIME STATS
// =========================

double read_uptime_seconds();

// =========================
// PROCESS COUNT
// =========================

int count_processes();

// =========================
// PROCESS INFO
// =========================

struct ProcessInfo {
  int pid;
  std::string name;
  char state;
  long long memory_kb;
};

std::vector<ProcessInfo> read_process_list();

// ==========================
// DISK STATS
// ==========================

struct DiskStats {
  unsigned long long total_bytes = 0;
  unsigned long long free_bytes = 0;
};

DiskStats get_disk_stats(const std::string &path = "/");

double calculate_disk_usage(const DiskStats &disk);

// ==========================
// NETWORK STATS
// ==========================

struct NetworkStats {
  unsigned long long rx_bytes = 0;
  unsigned long long tx_bytes = 0;
};

NetworkStats get_network_stats(const std::string &interface = "wlo1");
