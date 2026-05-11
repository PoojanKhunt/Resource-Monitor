#include "history.hpp"

#include <algorithm>
#include <cmath>
#include <deque>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

void update_history(std::deque<double> &history, double value,
                    std::size_t max_size) {
  history.push_back(value);
  while (history.size() > max_size) {
    history.pop_front();
  }
}

// ─────────────────────────────────────────────────────────────
// Original make_graph  (percentage, 0-100)
// ─────────────────────────────────────────────────────────────
std::vector<std::string> make_graph(const std::deque<double> &history,
                                    int width, int height) {
  const std::string blocks[] = {" ", "▁", "▂", "▃", "▄", "▅", "▆", "▇", "█"};

  std::vector<std::string> rows;

  int start = 0;
  if (static_cast<int>(history.size()) > width)
    start = static_cast<int>(history.size()) - width;

  std::vector<int> units;
  for (int i = start; i < static_cast<int>(history.size()); i++) {
    double value = history[i];
    if (value < 0.0)
      value = 0.0;
    if (value > 100.0)
      value = 100.0;
    units.push_back(static_cast<int>(value * height * 8 / 100.0 + 0.5));
  }

  for (int row = 0; row < height; row++) {
    int level = 100 - row * 100 / (height - 1);

    std::string line;
    if (level == 100)
      line = " 100% ┤ ";
    else if (level < 10)
      line = "   " + std::to_string(level) + "% ┤ ";
    else
      line = "  " + std::to_string(level) + "% ┤ ";

    for (int total_units : units) {
      int remaining = total_units - (height - row - 1) * 8;
      if (remaining >= 8)
        line += "█ ";
      else if (remaining > 0)
        line += blocks[remaining] + std::string(" ");
      else
        line += "  ";
    }
    rows.push_back(line);
  }

  std::string axis = "   0% └";
  int axis_width = static_cast<int>(units.size()) * 2 + 1;
  for (int i = 0; i < axis_width; i++)
    axis += "─";
  rows.push_back(axis);

  return rows;
}

// ─────────────────────────────────────────────────────────────
// make_graph_dynamic  (raw values, dynamic Y axis)
// ─────────────────────────────────────────────────────────────

// Format a raw speed value as "N.NN <unit>" fitting in at most `field_width`
// visible characters (right-padded with spaces).
static std::string format_axis_label(double value, const std::string &unit,
                                     int field_width) {
  std::ostringstream oss;
  if (value == 0.0) {
    oss << "0 " << unit;
  } else if (value >= 1.0) {
    // Show at most 1 decimal place; drop it when the integer part is >= 10
    // to keep labels short.
    int decimals = (value < 10.0) ? 1 : 0;
    oss << std::fixed << std::setprecision(decimals) << value << " " << unit;
  } else {
    oss << std::fixed << std::setprecision(2) << value << " " << unit;
  }

  std::string s = oss.str();
  // Right-pad to field_width
  while (static_cast<int>(s.size()) < field_width)
    s += ' ';
  // Truncate if somehow wider (shouldn't happen with reasonable units)
  if (static_cast<int>(s.size()) > field_width)
    s = s.substr(0, field_width);
  return s;
}

std::vector<std::string> make_graph_dynamic(const std::deque<double> &history,
                                            int width, int height,
                                            double max_value,
                                            const std::string &unit_label) {
  const std::string blocks[] = {" ", "▁", "▂", "▃", "▄", "▅", "▆", "▇", "█"};

  std::vector<std::string> rows;

  // Guard against zero max (e.g. no traffic yet)
  if (max_value <= 0.0)
    max_value = 1.0;

  int start = 0;
  if (static_cast<int>(history.size()) > width)
    start = static_cast<int>(history.size()) - width;

  // Convert raw values to 1/8-cell units relative to max_value
  std::vector<int> units;
  for (int i = start; i < static_cast<int>(history.size()); i++) {
    double value = history[i];
    if (value < 0.0)
      value = 0.0;
    if (value > max_value)
      value = max_value;
    units.push_back(static_cast<int>(value * height * 8 / max_value + 0.5));
  }

  // Label column width: we want consistent padding.
  // The longest label will be at max_value; measure it.
  // We fix field_width = 8 (same as the "% " graph) so panel widths match.
  const int label_field = 6; // e.g. "3.5 KB" or "100 B " — 6 chars before " ┤ "
  // Full prefix = label_field + " ┤ " (3) = label_field + 3 chars

  for (int row = 0; row < height; row++) {
    // Axis value at this row (linear scale)
    double frac = 1.0 - static_cast<double>(row) / (height - 1);
    double level = frac * max_value;

    std::string label = format_axis_label(level, unit_label, label_field);
    std::string line = label + " ┤ ";

    for (int total_units : units) {
      int remaining = total_units - (height - row - 1) * 8;
      if (remaining >= 8)
        line += "█ ";
      else if (remaining > 0)
        line += blocks[remaining] + std::string(" ");
      else
        line += "  ";
    }
    rows.push_back(line);
  }

  // Bottom axis — label "0 <unit>" left-aligned in the same field
  std::string zero_label = format_axis_label(0.0, unit_label, label_field);
  std::string axis = zero_label + " └";
  int axis_width = static_cast<int>(units.size()) * 2 + 1;
  for (int i = 0; i < axis_width; i++)
    axis += "─";
  rows.push_back(axis);

  return rows;
}