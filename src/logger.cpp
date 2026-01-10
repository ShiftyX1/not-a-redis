#include "logger.hpp"
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <sstream>

LogLevel Logger::current_level = LogLevel::INFO;
std::ofstream Logger::log_file;
std::mutex Logger::log_mutex;
bool Logger::to_file = false;

void Logger::init(const std::string& level, const std::string& file_path) {
    std::lock_guard<std::mutex> lock(log_mutex);
    current_level = string_to_level(level);
    
    if (!file_path.empty()) {
        log_file.open(file_path, std::ios::app);
        if (log_file.is_open()) {
            to_file = true;
        } else {
            std::cerr << "Failed to open log file: " << file_path << ". Logging to console only.\n";
        }
    }
}

void Logger::log(LogLevel level, const std::string& msg) {
    if (level < current_level) return;

    std::lock_guard<std::mutex> lock(log_mutex);
    std::string timestamp = get_timestamp();
    std::string level_str = level_to_string(level);
    
    std::ostream& out = (level == LogLevel::ERROR) ? std::cerr : std::cout;
    
    std::string full_msg = "[" + timestamp + "] [" + level_str + "] " + msg + "\n";
    
    out << full_msg;
    if (to_file) {
        log_file << full_msg;
        log_file.flush();
    }
}

void Logger::log_info(const std::string& msg) { log(LogLevel::INFO, msg); }
void Logger::log_error(const std::string& msg) { log(LogLevel::ERROR, msg); }
void Logger::log_warning(const std::string& msg) { log(LogLevel::WARNING, msg); }
void Logger::log_debug(const std::string& msg) { log(LogLevel::DEBUG, msg); }

LogLevel Logger::string_to_level(const std::string& level) {
    std::string upper_level = level;
    std::transform(upper_level.begin(), upper_level.end(), upper_level.begin(), ::toupper);
    
    if (upper_level == "DEBUG") return LogLevel::DEBUG;
    if (upper_level == "INFO") return LogLevel::INFO;
    if (upper_level == "WARNING") return LogLevel::WARNING;
    if (upper_level == "ERROR") return LogLevel::ERROR;
    
    return LogLevel::INFO;
}

std::string Logger::level_to_string(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

std::string Logger::get_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}
