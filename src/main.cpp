#include <algorithm>
#include <chrono>
#include <csignal>
#include <iostream>
#include <thread>
#include <vector>

#include "arguments.hpp"
#include "display.hpp"
#include "gpu.hpp"
#include "history.hpp"
#include "system.hpp"

void restore_cursor(int) {
  std::cout << "\033[?25h" << std::flush;
  std::exit(0);
}

int main(int argc, char *argv[]) {
  // =========================
  // PARSE COMMAND-LINE OPTIONS
  // =========================
  ProgramOptions options;

  try {
    options = parse_arguments(argc, argv);
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }

  // =========================
  // TERMINAL SETUP
  // =========================
  std::signal(SIGINT, restore_cursor);
  std::cout << "\033[?25l";

  std::deque<double> cpu_history;
  std::deque<double> ram_history;
  // Network histories store raw bytes/s (not a normalised percentage).
  // make_net_graph() picks the right unit (B/s, KB/s, MB/s, GB/s) dynamically.
  std::deque<double> download_history;
  std::deque<double> upload_history;

  CpuStats prev_cpu = real_cpu_stats();
  NetworkStats prev_net = get_network_stats("wlo1");

  const int graph_w = panel_graph_width(); // bars that fit inside a panel (15)
  const int graph_h = 8;

  while (true) {

    // =========================
    // CPU USAGE
    // =========================
    CpuStats curr_cpu = real_cpu_stats();
    double cpu_usage = calculate_cpu_usage(prev_cpu, curr_cpu);
    std::vector<double> per_core_usage = get_per_core_cpu_usage();
    prev_cpu = curr_cpu;

    update_history(cpu_history, cpu_usage);
    std::vector<std::string> cpu_graph =
        make_graph(cpu_history, graph_w, graph_h);

    // =========================
    // MEMORY USAGE
    // =========================
    MemoryStats mem = read_memory_stats();
    double memory_usage = calculate_memory_usage(mem);

    update_history(ram_history, memory_usage);
    std::vector<std::string> ram_graph =
        make_graph(ram_history, graph_w, graph_h);

    // =========================
    // DISK USAGE
    // =========================
    DiskStats disk = get_disk_stats("/");
    double disk_usage = calculate_disk_usage(disk);

    // =========================
    // NETWORK STATS
    // =========================
    NetworkStats net = get_network_stats("wlo1");

    // Raw bytes transferred in the last ~1 second interval.
    unsigned long long download_speed = net.rx_bytes - prev_net.rx_bytes;
    unsigned long long upload_speed = net.tx_bytes - prev_net.tx_bytes;
    prev_net = net;

    // Store raw bytes/s — make_net_graph handles unit selection.
    update_history(download_history, static_cast<double>(download_speed));
    update_history(upload_history, static_cast<double>(upload_speed));

    // Dynamic axis: Y labels switch between B/s, KB/s, MB/s, GB/s based on
    // the current peak in history.
    std::vector<std::string> download_graph =
        make_net_graph(download_history, graph_w, graph_h);
    std::vector<std::string> upload_graph =
        make_net_graph(upload_history, graph_w, graph_h);

    // =========================
    // UPTIME
    // =========================
    double uptime = read_uptime_seconds();

    // =========================
    // PROCESS COUNT
    // =========================
    int processes = count_processes();

    // =========================
    // PROCESS LIST
    // =========================
    std::vector<ProcessInfo> process_list = read_process_list();

    // -------------------------
    // SEARCH FILTER
    // -------------------------
    if (!options.search.empty()) {
      std::vector<ProcessInfo> filtered;
      for (const auto &proc : process_list) {
        if (proc.name.find(options.search) != std::string::npos)
          filtered.push_back(proc);
      }
      process_list = filtered;
    }

    // -------------------------
    // SORTING
    // -------------------------
    if (options.sort_by == "memory") {
      std::sort(process_list.begin(), process_list.end(),
                [](const ProcessInfo &a, const ProcessInfo &b) {
                  return a.memory_kb > b.memory_kb;
                });
    } else if (options.sort_by == "name") {
      std::sort(process_list.begin(), process_list.end(),
                [](const ProcessInfo &a, const ProcessInfo &b) {
                  return a.name < b.name;
                });
    } else if (options.sort_by == "state") {
      std::sort(process_list.begin(), process_list.end(),
                [](const ProcessInfo &a, const ProcessInfo &b) {
                  return a.state < b.state;
                });
    }

    // =========================
    // GPU STATS
    // =========================
    GpuStats gpu = get_gpu_stats();

    // =========================
    // =========================

    double cpu_temp = get_cpu_temperature();

    double battery_percent = get_battery_percentage();
    std::string battery_status = get_battery_status();

    double cpu_freq = get_cpu_frequency_mhz();

    unsigned long long disk_read_speed = get_disk_read_speed();
    unsigned long long disk_write_speed = get_disk_write_speed();

    // =========================
    // DISPLAY
    // =========================
    clear_screen();

    if (process_list.size() > static_cast<size_t>(options.limit))
      process_list.resize(options.limit);

    print_dashboard(cpu_usage, per_core_usage, memory_usage, disk_usage,
                    cpu_temp, battery_percent, battery_status, cpu_freq,
                    disk_read_speed, disk_write_speed, gpu, net, download_speed,
                    upload_speed, uptime, processes, process_list, cpu_graph,
                    ram_graph, download_graph, upload_graph);

    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  return 0;
}