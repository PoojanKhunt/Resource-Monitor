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
void print_dashboard(double cpu_usage,
                     const std::vector<double> &per_core_usage,
                     double memory_usage, double disk_usage,

                     // Additional system metrics
                     double cpu_temp, double battery_percent,
                     const std::string &battery_status, double cpu_freq_mhz,
                     unsigned long long disk_read_speed,
                     unsigned long long disk_write_speed,

                     // GPU
                     const GpuStats &gpu,

                     // Network
                     const NetworkStats &net, unsigned long long download_speed,
                     unsigned long long upload_speed,

                     // General system info
                     double uptime, int process_count,

                     // Processes
                     const std::vector<ProcessInfo> &process_list,

                     // Graphs
                     const std::vector<std::string> &cpu_graph,
                     const std::vector<std::string> &ram_graph,
                     const std::vector<std::string> &download_graph,
                     const std::vector<std::string> &upload_graph);

double get_cpu_temperature();

double get_battery_percentage();
std::string get_battery_status();

double get_cpu_frequency_mhz();

unsigned long long get_disk_read_speed();
unsigned long long get_disk_write_speed();