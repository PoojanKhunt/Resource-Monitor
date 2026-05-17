#pragma once

#include "gpu.hpp"
#include "system.hpp"

#include <deque>
#include <vector>

void clear_screen();

std::string format_bytes(unsigned long long bytes);
std::string format_uptime(double uptime_seconds);

std::string get_usage_color(double percentage);
std::string colorize_percentage(double percentage);

std::string make_progress_bar(double percentage, int width = 20);
std::string format_usage(double percentage, int width = 20);

// Returns the number of bar samples that fit inside a panel without overflow.
// Use this as the 'width' argument to make_graph() / make_net_graph().
int panel_graph_width();

// Build a network speed graph with a dynamic Y axis (B/s, KB/s, MB/s, GB/s).
// history must contain raw bytes/s values.
std::vector<std::string> make_net_graph(const std::deque<double> &history,
                                        int width, int height);

void print_dashboard(double cpu_usage, double memory_usage, double disk_usage,
                     const GpuStats &gpu, const NetworkStats &net,
                     unsigned long long download_speed,
                     unsigned long long upload_speed, double uptime,
                     int process_count,
                     const std::vector<ProcessInfo> &process_list,
                     const std::vector<std::string> &cpu_graph,
                     const std::vector<std::string> &ram_graph,
                     const std::vector<std::string> &download_graph,
                     const std::vector<std::string> &upload_graph);