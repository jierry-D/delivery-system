#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <ctime>

class Logger {
public:
    static Logger& instance() {
        static Logger inst;
        return inst;
    }

    void setLogFile(const std::string& path) {
        if (logFile.is_open()) logFile.close();
        logFile.open(path, std::ios::app);
    }

    void log(const std::string& level, const std::string& msg) {
        std::string entry = "[" + timestamp() + "][" + level + "] " + msg;
        std::cout << entry << "\n";
        if (logFile.is_open()) logFile << entry << "\n";
    }

private:
    std::ofstream logFile;
    Logger() = default;

    std::string timestamp() {
        time_t t = time(nullptr);
        char buf[20];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&t));
        return buf;
    }
};

#define LOG_INFO(msg)  Logger::instance().log("INFO",  msg)
#define LOG_WARN(msg)  Logger::instance().log("WARN",  msg)
#define LOG_ERROR(msg) Logger::instance().log("ERROR", msg)
