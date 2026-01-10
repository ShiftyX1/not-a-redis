#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <mutex>

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Logger {
public:
    static void init(const std::string& level, const std::string& file_path = "");
    
    static void log(LogLevel level, const std::string& msg);
    static void log_info(const std::string& msg);
    static void log_error(const std::string& msg);
    static void log_warning(const std::string& msg);
    static void log_debug(const std::string& msg);

private:
    Logger() = default;
    
    static LogLevel string_to_level(const std::string& level);
    static std::string level_to_string(LogLevel level);
    static std::string get_timestamp();

    static LogLevel current_level;
    static std::ofstream log_file;
    static std::mutex log_mutex;
    static bool to_file;
};
