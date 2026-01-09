#include <iostream>
#include <csignal>
#include "config.hpp"
#include "server.hpp"
#include "utils.hpp"

int main() {
    std::signal(SIGINT, stop_program);

    try {
        Config config("config.ini");
        config.check_required_keys();

        Server server(config);
        server.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
