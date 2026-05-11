#include "display.hpp"

#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <string>

// =========================
// CLEAR SCREEN
// =========================

void clear_screen() { std::system("clear"); }

std::string format_bytes(unsigned long long bytes) {
  const char *units[] = {"B", "KB", "MB", "GB", "TB"};

  double size = static_cast<double>(bytes);
  int unit_index = 0;

  while (size >= 1024.0 and unit_index < 4) {
    size /= 1024.0;
    unit_index++;
  }

  std::ostringstream oss;
  oss << std::fixed << std::setprecision(2) << size << ' ' << units[unit_index];

  return oss.str();
}

std::string format_uptime(double uptime_seconds) {
  long long total_seconds = static_cast<long long>(uptime_seconds);

  long long days = total_seconds / 86400;
  total_seconds %= 86400;

  long long hours = total_seconds / 3600;
  total_seconds %= 3600;

  long long minutes = total_seconds / 60;
  long long seconds = total_seconds % 60;

  std::ostringstream oss;

  if (days > 0) {
    oss << days << "d ";
  }

  if (hours > 0 || days > 0) {
    oss << hours << "h ";
  }

  if (minutes > 0 || hours > 0 || days > 0) {
    oss << minutes << "m ";
  }

  oss << seconds << "s";

  return oss.str();
}

std::string get_usage_color(double percentage) {
  if (percentage >= 80.0) {
    return "\033[31m";
  }
  if (percentage >= 50.0) {
    return "\033[33m";
  }
  return "\033[32m";
}

std::string colorize_percentage(double percentage) {
  std::ostringstream oss;

  oss << get_usage_color(percentage) << std::fixed << std::setprecision(2)
      << percentage << "%"
      << "\033[0m";

  return oss.str();
}

std::string make_progress_bar(double percentage, int width) {
  int filled = static_cast<int>(percentage * width / 100.0 + 0.5);

  if (filled < 0)
    filled = 0;
  if (filled > width)
    filled = width;

  std::string bar = "[";

  for (int i = 0; i < filled; i++) {
    bar += "█";
  }

  for (int i = filled; i < width; i++) {
    bar += "-";
  }

  bar += "]";

  return bar;
}

std::string format_usage(double percentage, int width) {
  std::ostringstream oss;

  std::string color = get_usage_color(percentage);

  oss << color << make_progress_bar(percentage, width) << " " << std::fixed
      << std::setprecision(2) << percentage << "%"
      << "\033[0m";

  return oss.str();
}

void print_dashboard(double cpu_usage, double memory_usage, double disk_usage,
                     const NetworkStats &net, unsigned long long download_speed,
                     unsigned long long upload_speed, double uptime,
                     int process_count,
                     const std::vector<ProcessInfo> &process_list) {
  std::cout << std::fixed << std::setprecision(2);

  std::cout << "=========================================\n";
  std::cout << "           RESOURCE MONITOR\n";
  std::cout << "=========================================\n\n";

  std::cout << "CPU Usage      : " << format_usage(cpu_usage) << "%\n";
  std::cout << "RAM Usage      : " << format_usage(memory_usage) << "%\n";
  std::cout << "Disk Usage     : " << format_usage(disk_usage) << "%\n";
  std::cout << "Network RX     : " << format_bytes(net.rx_bytes) << "\n";
  std::cout << "Network TX     : " << format_bytes(net.tx_bytes) << "\n";
  std::cout << "Download Speed : " << format_bytes(download_speed) << "/s\n";
  std::cout << "Upload Speed   : " << format_bytes(upload_speed) << "/s\n";
  std::cout << "Uptime         : " << format_uptime(uptime) << "\n";
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
