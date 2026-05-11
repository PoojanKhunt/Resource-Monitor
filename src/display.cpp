#include "display.hpp"

#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>

// =========================
// CLEAR SCREEN
// =========================

void clear_screen() { std::system("clear"); }

void print_dashboard(double cpu_usage, double memory_usage, double disk_usage,
                     const NetworkStats &net, double uptime, int process_count,
                     const std::vector<ProcessInfo> &process_list) {
  std::cout << std::fixed << std::setprecision(2);

  std::cout << "=========================================\n";
  std::cout << "           RESOURCE MONITOR\n";
  std::cout << "=========================================\n\n";

  std::cout << "CPU Usage      : " << cpu_usage << "%\n";
  std::cout << "RAM Usage      : " << memory_usage << "%\n";
  std::cout << "Disk Usage     : " << disk_usage << "%\n";
  std::cout << "Network RX     : " << net.rx_bytes << " bytes\n";
  std::cout << "Network TX     : " << net.tx_bytes << " bytes\n";
  std::cout << "Uptime         : " << uptime << " seconds\n";
  std::cout << "Process Count  : " << process_count << "\n";

  std::cout << "\nProcess rows found: " << process_list.size() << "\n\n";

  std::cout << std::left << std::setw(8) << "PID" << std::setw(28) << "NAME"
            << std::setw(8) << "STATE"
            << "MEMORY(KB)\n";

  std::cout << std::string(56, '-') << "\n";

  int limit = std::min(10, static_cast<int>(process_list.size()));

  for (int i = 0; i < limit; i++) {
    const auto &proc = process_list[i];

    std::cout << std::left << std::setw(8) << proc.pid << std::setw(28)
              << proc.name << std::setw(8) << proc.state << proc.memory_kb
              << "\n";
  }

  std::cout << "\n=========================================\n";

  std::cout.flush();
}
