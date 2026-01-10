#include <iostream>
#include <csignal>
#include "config.hpp"
#include "server.hpp"
#include "utils.hpp"
#include "logger.hpp"

int main() {
    std::signal(SIGINT, stop_program);

    try {
        Config config("config.ini");
        config.check_required_keys();

        Logger::init(config.get_log_level(), config.get_log_file());
        Logger::log_info("Logger initialized");

        Server server(config);
        print_logo();
        server.run();
    } catch (const std::exception& e) {
        Logger::log_error("Fatal error: " + std::string(e.what()));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
