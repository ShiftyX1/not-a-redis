#pragma once

#include <string>
#include <map>

class Config {
public:
    // Конструктор
    Config(const std::string& filename = "config.ini");

    // Геттеры
    std::string get_address() const;
    int get_port() const;
    std::string get_log_level() const;
    std::string get_log_file() const;

    void check_required_keys() const;

private:
    std::string address;
    int port;
    std::string log_level;
    std::string log_file;

    void load_from_file(const std::string& filename);
};
