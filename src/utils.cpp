#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include "logger.hpp"

void stop_program(int signum) {
    Logger::log_info("Interrupt signal (" + std::to_string(signum) + ") received. Shutting down...");
    exit(signum);
}

void print_error(const std::string& msg) {
    Logger::log_error(msg + ": " + std::strerror(errno));
}


void print_logo() {
    std::cout << R"(
╔════════════════════════════════════════════════════════════╗
║             _             _____  ______ _____ _____  _____ ║
║            | |           |  __ \|  ____|  __ \_   _|/ ____|║
║ _ __   ___ | |_    __ _  | |__) | |__  | |  | || | | (___  ║
║| '_ \ / _ \| __|  / _` | |  _  /|  __| | |  | || |  \___ \ ║
║| | | | (_) | |_  | (_| | | | \ \| |____| |__| || |_ ____) |║
║|_| |_|\___/ \__|  \__,_| |_|  \_\______|_____/_____|_____/ ║
╚════════════════════════════════════════════════════════════╝
)" << std::endl;
}