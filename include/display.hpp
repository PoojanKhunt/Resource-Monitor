#pragma once

#include "system.hpp"
#include <vector>

void clear_screen();

std::string format_bytes(unsigned long long bytes);
std::string format_uptime(double uptime_seconds);

std::string get_usage_color(double percentage);
std::string colorize_percentage(double percentage);

std::string make_progress_bar(double percentage, int width = 20);
std::string format_usage(double percentage, int width = 20);

void print_dashboard(double cpu_usage, double memory_usage, double disk_usage,
                     const NetworkStats &net, unsigned long long downlaod_speed,
                     unsigned long long upload_speed, double uptime,
                     int process_count,
                     const std::vector<ProcessInfo> &process_list);
