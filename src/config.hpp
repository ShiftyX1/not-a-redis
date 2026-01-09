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

    void check_required_keys() const;

private:
    std::string address;
    int port;

    void load_from_file(const std::string& filename);
};
