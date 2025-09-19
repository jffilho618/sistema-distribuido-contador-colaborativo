#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <mutex>

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Logger {
private:
    static std::mutex log_mutex;
    static LogLevel current_level;
    static std::string component_name;
    
    static std::string get_timestamp();
    static std::string level_to_string(LogLevel level);
    static std::string get_color_code(LogLevel level);
    static std::string get_reset_color();

public:
    static void set_component_name(const std::string& name);
    static void set_log_level(LogLevel level);
    
    static void debug(const std::string& message);
    static void info(const std::string& message);
    static void warning(const std::string& message);
    static void error(const std::string& message);
    
    static void log(LogLevel level, const std::string& message);
    
    // Macros para facilitar o uso com formatação
    template<typename... Args>
    static void debug_f(const std::string& format, Args... args) {
        debug(format_string(format, args...));
    }
    
    template<typename... Args>
    static void info_f(const std::string& format, Args... args) {
        info(format_string(format, args...));
    }
    
    template<typename... Args>
    static void warning_f(const std::string& format, Args... args) {
        warning(format_string(format, args...));
    }
    
    template<typename... Args>
    static void error_f(const std::string& format, Args... args) {
        error(format_string(format, args...));
    }

private:
    template<typename... Args>
    static std::string format_string(const std::string& format, Args... args) {
        int size_s = std::snprintf(nullptr, 0, format.c_str(), args...) + 1;
        if (size_s <= 0) { 
            return format; 
        }
        auto size = static_cast<size_t>(size_s);
        std::unique_ptr<char[]> buf(new char[size]);
        std::snprintf(buf.get(), size, format.c_str(), args...);
        return std::string(buf.get(), buf.get() + size - 1);
    }
};