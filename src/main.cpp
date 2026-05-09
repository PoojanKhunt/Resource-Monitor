#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include "system.hpp"

int main() {

    // =========================
    // CPU USAGE
    // =========================

    CpuStats prev = real_cpu_stats();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    CpuStats curr = real_cpu_stats();

    double cpu_usage = calculate_cpu_usage(prev, curr);

    // =========================
    // MEMORY USAGE
    // =========================

    MemoryStats mem = read_memory_stats();
    double memory_usage = calculate_memory_usage(mem);

    // =========================
    // UPTIME
    // =========================

    double uptime = read_uptime_seconds();

    // =========================
    // PROCESS COUNT
    // =========================
    
    int processes = count_processes();

    // =========================
    // PROCESS LIST
    // =========================

    std::vector<ProcessInfo> process_list =
        read_process_list();
    
    std::sort(process_list.begin(), process_list.end(),
        [](const ProcessInfo& a, const ProcessInfo& b) {
            return a.memory_kb > b.memory_kb;
        });

    // =========================
    // OUTPUT
    // =========================
    std::cout << std::fixed << std::setprecision(2); 

    std::cout << "CPU Usage: " << cpu_usage << "%\n";
    std::cout << "RAM Usage: " << memory_usage << "%\n";
    std::cout << "Uptime: " << uptime << " seconds\n";
    std::cout << "Process Count: " << processes << "\n";

    std::cout << "Process rows found: " << process_list.size() << "\n\n";

    std::cout << std::left
          << std::setw(8)  << "PID"
          << std::setw(28) << "NAME"
          << std::setw(8)  << "STATE"
          << "MEMORY(KB)\n";
    
    std::cout << std::string(56, '-') << "\n";

    int limit = 10;
    

    for(const auto& proc : process_list) {
        
        std::cout << std::left
              << std::setw(8)  << proc.pid
              << std::setw(28) << proc.name
              << std::setw(8)  << proc.state
              << proc.memory_kb
              << "\n";

        limit--;

        if(limit == 0){
            break;
        }
    }

    return 0;
}