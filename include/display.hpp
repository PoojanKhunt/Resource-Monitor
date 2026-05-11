#pragma once

#include "system.hpp"
#include <vector>

void clear_screen();

void print_dashboard(double cpu_usage, double memory_usage, double disk_usage,
                     const NetworkStats &net, double uptime, int process_count,
                     const std::vector<ProcessInfo> &process_list);
