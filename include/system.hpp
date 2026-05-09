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

struct MemoryStats {
    long long total_kb;
    long long available_kb;
};


// =========================
// MEMORY STATS
// =========================

CpuStats real_cpu_stats();

double calculate_cpu_usage(
    const CpuStats& prev,
    const CpuStats& curr
);

MemoryStats read_memory_stats();

double calculate_memory_usage(
    const MemoryStats& mem
);


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

struct ProcessInfo{
    int pid;
    std::string name;
    char state;
    long long memory_kb;
};

std::vector<ProcessInfo> read_process_list();

