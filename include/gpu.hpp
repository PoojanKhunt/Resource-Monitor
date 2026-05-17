#pragma once

#include <string>

struct GpuStats {
  bool available = false;

  std::string name;

  double utilization = 0.0;

  double memory_used_mb = 0.0;
  double memory_total_mb = 0.0;

  double temperature_c = 0.0;

  double power_draw_w = 0.0;

  double fan_speed_percent = 0.0;
};

GpuStats get_gpu_stats();