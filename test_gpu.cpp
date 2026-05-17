#include "gpu.hpp"
#include <iostream>

int main() {
  GpuStats gpu = get_gpu_stats();

  if (!gpu.available) {
    std::cout << "No supported GPU detected.\n";
    return 0;
  }

  std::cout << "GPU Name: " << gpu.name << '\n';
  std::cout << "Utilization: " << gpu.utilization << "%\n";
  std::cout << "Memory Used: " << gpu.memory_used_mb << " MB\n";
  std::cout << "Memory Total: " << gpu.memory_total_mb << " MB\n";
  std::cout << "Temperature: " << gpu.temperature_c << " C\n";
  std::cout << "Power Draw: " << gpu.power_draw_w << " W\n";
  std::cout << "Fan Speed: " << gpu.fan_speed_percent << "%\n";

  return 0;
}