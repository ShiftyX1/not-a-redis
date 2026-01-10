#include "config.hpp"
#include <fstream>
#include <stdexcept>
#include <iostream>

Config::Config(const std::string& filename) {
    if (filename.empty()) {
        std::cerr << "Filename is empty\nUsing default filename: config.ini\n";
    }
    load_from_file(filename);
    check_required_keys();
}

void Config::load_from_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open config file: " << filename << "\n";
        std::cout << "Using default parameters: 127.0.0.1:6379\n";
        address = "127.0.0.1";
        port = 6379;
        log_level = "INFO";
        log_file = "";
        return;
    }

    std::string key;
    while (file >> key) {
        if (key == "address") {
            file >> address;
        } else if (key == "port") {
            file >> port;
        } else if (key == "log_level") {
            file >> log_level;
        } else if (key == "log_file") {
            file >> log_file;
        } else {
            std::cerr << "Unknown key: " << key << "\n";
            continue;
        }
    }

    // Set defaults if not provided
    if (log_level.empty()) log_level = "INFO";

    file.close();
}

std::string Config::get_address() const {
    return address;
}

int Config::get_port() const {
    return port;
}

std::string Config::get_log_level() const {
    return log_level;
}

std::string Config::get_log_file() const {
    return log_file;
}

void Config::check_required_keys() const {
    if (address.empty()) {
        std::cerr << "Address is not set\n";
        exit(EXIT_FAILURE);
    }
    if (port == 0 ) {
        std::cerr << "Port is not set\n";
        exit(EXIT_FAILURE);
    }
}
