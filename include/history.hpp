#pragma once

#include <deque>
#include <string>
#include <vector>

void update_history(std::deque<double> &history, double value,
                    std::size_t max_size = 60);

std::string make_sparkline(const std::deque<double> &history);

// Standard graph: values are 0-100 (percentages).
// Y-axis labels are shown as "N%".
std::vector<std::string> make_graph(const std::deque<double> &history,
                                    int width = 50, int height = 20);

// Dynamic graph: values are raw (e.g. bytes/s).
// max_value   string appended to each axis tick, e.g. "B/s", "KB/s", "MB/s".
std::vector<std::string> make_graph_dynamic(const std::deque<double> &history,
                                            int width, int height,
                                            double max_value,
                                            const std::string &unit_label);