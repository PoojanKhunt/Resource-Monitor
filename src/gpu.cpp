#include "gpu.hpp"

#include <array>
#include <cstdio>
#include <sstream>
#include <string>
#include <vector>

namespace {

std::string exec_command(const std::string &cmd) {
  std::array<char, 256> buffer{};
  std::string result;

  FILE *pipe = popen(cmd.c_str(), "r");
  if (!pipe)
    return result;

  while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
    result += buffer.data();
  }

  pclose(pipe);
  return result;
}

std::string trim(const std::string &s) {
  size_t start = s.find_first_not_of("\t\n\r");
  if (start == std::string::npos)
    return "";

  size_t end = s.find_last_not_of("\t\n\r");
  return s.substr(start, end - start + 1);
}

std::vector<std::string> split_csv(const std::string &line) {
  std::vector<std::string> fields;
  std::stringstream ss(line);
  std::string token;

  while (std::getline(ss, token, ',')) {
    fields.push_back(trim(token));
  }

  return fields;
}

double to_double(const std::string &s) {
  try {
    return std::stod(s);
  } catch (...) {
    return 0.0;
  }
}

} // namespace

GpuStats get_gpu_stats() {
  GpuStats stats;

  const std::string cmd =
      "nvidia-smi "
      "--query-gpu=name,utilization.gpu,memory.used,memory.total,"
      "temperature.gpu,power.draw,fan.speed,clocks.current.graphics "
      "--format=csv,noheader,nounits 2>/dev/null";

  std::string output = exec_command(cmd);

  if (output.empty()) {
    return stats;
  }

  std::stringstream ss(output);
  std::string line;
  if (!std::getline(ss, line)) {
    return stats;
  }

  auto fields = split_csv(line);

  if (fields.size() < 8) {
    return stats;
  }

  stats.available = true;
  stats.name = fields[0];
  stats.utilization = to_double(fields[1]);
  stats.memory_used_mb = to_double(fields[2]);
  stats.memory_total_mb = to_double(fields[3]);
  stats.temperature_c = to_double(fields[4]);
  stats.power_draw_w = to_double(fields[5]);
  stats.fan_speed_percent = to_double(fields[6]);
  stats.graphics_clock_mhz = to_double(fields[7]);

  return stats;
}