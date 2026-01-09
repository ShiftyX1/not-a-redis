#include <iostream>
#include <cstdlib>
#include <string>

void stop_program(int signum) {
    std::cout << "\nInterrupt signal (" << signum << ") received.\n";
    std::cout << "Exiting program...\n";
    exit(signum); 
}

void print_error(const std::string& msg) {
    std::cerr << msg << ": " << std::strerror(errno) << "\n";
}
