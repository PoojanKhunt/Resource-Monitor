#include "display.hpp"
#include "gpu.hpp"
#include "history.hpp"

#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <string>
#include <vector>

// =========================
// PANEL LAYOUT CONSTANTS
// =========================

static constexpr int PANEL_WIDTH = 42;
static constexpr int PANEL_GRAPH_WIDTH = (PANEL_WIDTH - 11) / 2; // = 15

int panel_graph_width() { return PANEL_GRAPH_WIDTH; }

// =========================
// CLEAR SCREEN
// =========================

void clear_screen() { std::system("clear"); }

// =========================
// FORMAT HELPERS
// =========================

std::string format_bytes(unsigned long long bytes) {
  const char *units[] = {"B", "KB", "MB", "GB", "TB"};
  double size = static_cast<double>(bytes);
  int unit_index = 0;
  while (size >= 1024.0 && unit_index < 4) {
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

  if (days > 0)
    oss << days << "d ";

  if (hours > 0 || days > 0)
    oss << hours << "h ";

  if (minutes > 0 || hours > 0 || days > 0)
    oss << minutes << "m ";

  oss << seconds << "s";

  return oss.str();
}

std::string get_usage_color(double percentage) {
  if (percentage >= 80.0)
    return "\033[31m";

  if (percentage >= 50.0)
    return "\033[33m";

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

  for (int i = 0; i < filled; i++)
    bar += "\xe2\x96\x88"; // █

  for (int i = filled; i < width; i++)
    bar += "-";

  bar += "]";

  return bar;
}

std::string format_usage(double percentage, int width) {
  std::ostringstream oss;

  oss << get_usage_color(percentage) << make_progress_bar(percentage, width)
      << "\033[0m"
      << " " << std::fixed << std::setprecision(2) << percentage << "%";

  return oss.str();
}

// =========================
// NETWORK SPEED HELPERS
// =========================

static std::pair<double, std::string> speed_unit(double peak_bytes_per_sec) {
  if (peak_bytes_per_sec >= 1024.0 * 1024.0 * 1024.0)
    return {1024.0 * 1024.0 * 1024.0, "GB/s"};

  if (peak_bytes_per_sec >= 1024.0 * 1024.0)
    return {1024.0 * 1024.0, "MB/s"};

  if (peak_bytes_per_sec >= 1024.0)
    return {1024.0, "KB/s"};

  return {1.0, "B/s"};
}

std::vector<std::string> make_net_graph(const std::deque<double> &history,
                                        int width, int height) {
  // Determine the start index of the visible window — identical to what
  // make_graph_dynamic will render, so the peak matches exactly.
  int start = 0;

  if (static_cast<int>(history.size()) > width)
    start = static_cast<int>(history.size()) - width;

  // Peak over the CURRENT WINDOW only, not the entire history.
  double peak = 1.0; // floor to avoid division by zero when idle
  for (int i = start; i < static_cast<int>(history.size()); i++)
    if (history[i] > peak)
      peak = history[i];

  auto [divisor, unit] = speed_unit(peak);
  double scaled_max = peak / divisor;

  // Scale the visible window
  std::deque<double> scaled;
  for (int i = start; i < static_cast<int>(history.size()); i++)
    scaled.push_back(history[i] / divisor);

  // Pass exactly `width` pre-sliced samples so make_graph_dynamic doesn't
  // re-slice and potentially use a different window.
  return make_graph_dynamic(scaled, static_cast<int>(scaled.size()), height,
                            scaled_max, unit);
}

// =========================
// PANEL HELPERS
// =========================

static int display_width(const std::string &s) {
  int w = 0;
  bool in_escape = false;

  for (unsigned char c : s) {
    if (in_escape) {
      if (c == 'm')
        in_escape = false;
    } else if (c == '\033') {
      in_escape = true;
    } else if ((c & 0xC0) != 0x80) {
      ++w;
    }
  }

  return w;
}

std::vector<std::string> make_panel(const std::string &title,
                                    const std::string &value,
                                    const std::vector<std::string> &graph) {
  std::vector<std::string> panel;
  const int width = PANEL_WIDTH;

  std::string top = "\xe2\x94\x8c\xe2\x94\x80 " + title + " ";

  while (display_width(top) < width)
    top += "\xe2\x94\x80";

  top += "\xe2\x94\x90";
  panel.push_back(top);

  std::string value_line = "\xe2\x94\x82 " + value;

  while (display_width(value_line) < width)
    value_line += " ";
  value_line += "\xe2\x94\x82";

  panel.push_back(value_line);

  for (const auto &row : graph) {
    std::string line = "\xe2\x94\x82 " + row;

    while (display_width(line) < width)
      line += " ";

    line += "\xe2\x94\x82";
    panel.push_back(line);
  }

  std::string bottom = "\xe2\x94\x94";
  for (int i = 1; i < width; i++)
    bottom += "\xe2\x94\x80";

  bottom += "\xe2\x94\x98";
  panel.push_back(bottom);

  return panel;
}

void print_two_panels(const std::vector<std::string> &left,
                      const std::vector<std::string> &right) {
  std::size_t lines = std::max(left.size(), right.size());

  for (std::size_t i = 0; i < lines; i++) {
    if (i < left.size())
      std::cout << left[i];

    std::cout << "  ";

    if (i < right.size())
      std::cout << right[i];

    std::cout << "\n";
  }
}

// =========================
// MAIN DASHBOARD
// =========================

void print_dashboard(double cpu_usage, double memory_usage, double disk_usage,
                     const GpuStats &gpu, const NetworkStats &net,
                     unsigned long long download_speed,
                     unsigned long long upload_speed, double uptime,
                     int process_count,
                     const std::vector<ProcessInfo> &process_list,
                     const std::vector<std::string> &cpu_graph,
                     const std::vector<std::string> &ram_graph,
                     const std::vector<std::string> &download_graph,
                     const std::vector<std::string> &upload_graph) {
  std::cout << std::fixed << std::setprecision(2);

  std::cout << "=========================================\n";
  std::cout << "           RESOURCE MONITOR\n";
  std::cout << "=========================================\n\n";

  auto cpu_panel = make_panel("CPU Usage", format_usage(cpu_usage), cpu_graph);

  auto ram_panel =
      make_panel("RAM Usage", format_usage(memory_usage), ram_graph);

  auto download_panel = make_panel(
      "Download Speed", format_bytes(download_speed) + "/s", download_graph);

  auto upload_panel = make_panel(
      "Upload Speed", format_bytes(upload_speed) + "/s", upload_graph);

  print_two_panels(cpu_panel, ram_panel);
  std::cout << "\n";
  print_two_panels(download_panel, upload_panel);
  std::cout << "\n";

  std::cout << "Disk Usage     : " << format_usage(disk_usage) << "\n";

  if (gpu.available) {
    std::cout << "GPU Name       : " << gpu.name << "\n";
    std::cout << "GPU Usage      : " << format_usage(gpu.utilization) << "\n";
    std::cout << "GPU Memory     : " << std::fixed << std::setprecision(0)
              << gpu.memory_used_mb << " MB / " << gpu.memory_total_mb
              << " MB\n";
    std::cout << "GPU Temp       : " << std::setprecision(1)
              << gpu.temperature_c << " C\n";
    std::cout << "GPU Power      : " << gpu.power_draw_w << " W\n";
    std::cout << "GPU Fan Speed  : " << gpu.fan_speed_percent << "%\n";
  } else {
    std::cout << "GPU            : Not Detected\n";
  }

  std::cout << "Network RX     : " << format_bytes(net.rx_bytes) << "\n";
  std::cout << "Network TX     : " << format_bytes(net.tx_bytes) << "\n";
  std::cout << "Uptime         : " << format_uptime(uptime) << "\n";
  std::cout << "Process Count  : " << process_count << "\n";

  std::cout << "\nProcess rows found: " << process_list.size() << "\n\n";

  std::cout << std::left << std::setw(8) << "PID" << std::setw(28) << "NAME"
            << std::setw(8) << "STATE"
            << "MEMORY(KB)\n";
  std::cout << std::string(56, '-') << "\n";

  for (const auto &proc : process_list) {
    std::cout << std::left << std::setw(8) << proc.pid << std::setw(28)
              << proc.name << std::setw(8) << proc.state << proc.memory_kb
              << "\n";
  }

  std::cout << "\n=========================================\n";
  std::cout.flush();
}