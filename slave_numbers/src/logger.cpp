#include "logger.h"

// Inicialização de variáveis estáticas
std::mutex Logger::log_mutex;
LogLevel Logger::current_level = LogLevel::INFO;
std::string Logger::component_name = "SYSTEM";

void Logger::set_component_name(const std::string& name) {
    std::lock_guard<std::mutex> lock(log_mutex);
    component_name = name;
}

void Logger::set_log_level(LogLevel level) {
    std::lock_guard<std::mutex> lock(log_mutex);
    current_level = level;
}

std::string Logger::get_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()
    ) % 1000;

    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

std::string Logger::level_to_string(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:   return "DEBUG";
        case LogLevel::INFO:    return "INFO ";
        case LogLevel::WARNING: return "WARN ";
        case LogLevel::ERROR:   return "ERROR";
        default:                return "UNKNW";
    }
}

std::string Logger::get_color_code(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:   return "\033[36m"; // Cyan
        case LogLevel::INFO:    return "\033[32m"; // Green
        case LogLevel::WARNING: return "\033[33m"; // Yellow
        case LogLevel::ERROR:   return "\033[31m"; // Red
        default:                return "\033[0m";  // Reset
    }
}

std::string Logger::get_reset_color() {
    return "\033[0m";
}

void Logger::log(LogLevel level, const std::string& message) {
    if (level < current_level) {
        return;
    }

    std::lock_guard<std::mutex> lock(log_mutex);

    std::cout << get_color_code(level)
              << "[" << get_timestamp() << "]"
              << "[" << level_to_string(level) << "]"
              << "[" << component_name << "] "
              << message
              << get_reset_color()
              << std::endl;
}

void Logger::debug(const std::string& message) {
    log(LogLevel::DEBUG, message);
}

void Logger::info(const std::string& message) {
    log(LogLevel::INFO, message);
}

void Logger::warning(const std::string& message) {
    log(LogLevel::WARNING, message);
}

void Logger::error(const std::string& message) {
    log(LogLevel::ERROR, message);
}