#pragma once

#include <string>

struct ProgramOptions {
  std::string sort_by = "memory";

  int limit = 10;

  std::string search = "";
};

ProgramOptions parse_arguments(int argc, char *argv[]);