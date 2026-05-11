#include "arguments.hpp"

#include <cstdlib>
#include <stdexcept>
#include <string>

static bool starts_with(const std::string &text, const std::string &prefix) {
  return text.rfind(prefix, 0) == 0;
}

ProgramOptions parse_arguments(int argc, char *argv[]) {
  ProgramOptions options;

  for (int i = 1; i < argc; i++) {
    std::string arg = argv[i];

    // -------------------------
    // --sort=<value>
    // -------------------------
    if (starts_with(arg, "--sort=")) {
      options.sort_by = arg.substr(7);

      if (options.sort_by != "memory" && options.sort_by != "pid" &&
          options.sort_by != "name" && options.sort_by != "state") {
        throw std::runtime_error("Invalid sort option: " + options.sort_by);
      }
    }

    // -------------------------
    // --limit=<number>
    // -------------------------
    else if (starts_with(arg, "--limit=")) {
      std::string value = arg.substr(8);
      options.limit = std::stoi(value);

      if (options.limit <= 0) {
        throw std::runtime_error("Limit must be greater than 0");
      }
    }

    // -------------------------
    // --search=<text>
    // -------------------------
    else if (starts_with(arg, "--search=")) {
      options.search = arg.substr(9);
    }

    // -------------------------
    // Unknown argument
    // -------------------------
    else {
      throw std::runtime_error("Unknown argument: " + arg);
    }
  }

  return options;
}