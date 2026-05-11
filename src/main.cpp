#include <algorithm>
#include <chrono>
#include <csignal>
#include <iostream>
#include <thread>
#include <vector>

#include "arguments.hpp"
#include "display.hpp"
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
    std::cerr << "Eroor: " << e.what() << "\n";
    return 1;
  }

  // =========================
  // TERMINAL SETUP
  // =========================
  std::signal(SIGINT, restore_cursor);
  std::cout << "\033[?25l";

  while (true) {

    // =========================
    // CPU USAGE
    // =========================
    static CpuStats prev_cpu = real_cpu_stats();
    CpuStats curr_cpu = real_cpu_stats();
    double cpu_usage = calculate_cpu_usage(prev_cpu, curr_cpu);
    prev_cpu = curr_cpu;

    // =========================
    // MEMORY USAGE
    // =========================
    MemoryStats mem = read_memory_stats();
    double memory_usage = calculate_memory_usage(mem);

    // =========================
    // DISK USAGE
    // =========================
    DiskStats disk = get_disk_stats("/");
    double disk_usage = calculate_disk_usage(disk);

    // =========================
    // NETWROK STATS
    // =========================
    NetworkStats net{};
    net = get_network_stats("wlo1");

    static NetworkStats prev_net = net;

    unsigned long long download_speed = net.rx_bytes - prev_net.rx_bytes;
    unsigned long long upload_speed = net.tx_bytes - prev_net.tx_bytes;
    prev_net = net;

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
        if (proc.name.find(options.search) != std::string::npos) {
          filtered.push_back(proc);
        }
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
    // DISPLAY
    // =========================
    clear_screen();

    if (process_list.size() > static_cast<size_t>(options.limit)) {
      process_list.resize(options.limit);
    }

    print_dashboard(cpu_usage, memory_usage, disk_usage, net, download_speed,
                    upload_speed, uptime, processes, process_list);

    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  return 0;
}