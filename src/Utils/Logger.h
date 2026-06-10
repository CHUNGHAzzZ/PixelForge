#pragma once

#include <atomic>
#include <condition_variable>
#include <cstdio>
#include <fstream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

namespace PixelForge {

#ifndef PIXELFORGE_LOG_CONSOLE_OUTPUT
#define PIXELFORGE_LOG_CONSOLE_OUTPUT 1
#endif

enum class LogLevel
{
    Debug,
    Info,
    Warning,
    Error,
    Fatal,
};

struct LogMessage
{
    LogLevel level {LogLevel::Info};
    std::string message;
    std::string timestamp;
    std::thread::id threadId;
};

class Logger
{
public:
    static void init(
        const std::string &logFile = "PixelForge.log",
        bool consoleOutput = PIXELFORGE_LOG_CONSOLE_OUTPUT,
        bool asyncMode = true);
    static void shutdown();

    static void debug(const std::string &message);
    static void info(const std::string &message);
    static void warning(const std::string &message);
    static void error(const std::string &message);
    static void fatal(const std::string &message);

    static void setLogLevel(LogLevel level);
    static void enableFileOutput(bool enable);
    static void enableConsoleOutput(bool enable);
    static void flush();

private:
    Logger() = default;

    static void log(LogLevel level, const std::string &message);
    static void processLogQueue();
    static void writeLog(const LogMessage &logMessage);
    static std::string timestamp();
    static std::string levelString(LogLevel level);
    static std::string levelColor(LogLevel level);
    static std::string threadIdString(std::thread::id threadId);

    static std::ofstream s_logFile;
    static std::mutex s_writeMutex;
    static std::mutex s_queueMutex;
    static std::condition_variable s_queueCv;
    static std::queue<LogMessage> s_logQueue;
    static std::thread s_logThread;
    static std::atomic<bool> s_running;
    static std::atomic<bool> s_asyncMode;
    static std::atomic<LogLevel> s_minLogLevel;
    static std::atomic<bool> s_fileOutputEnabled;
    static std::atomic<bool> s_consoleOutputEnabled;
};

template<typename... Args>
std::string formatLogMessage(const char *format, Args... args)
{
    const int size = std::snprintf(nullptr, 0, format, args...);
    if (size <= 0) {
        return {};
    }

    std::vector<char> buffer(static_cast<size_t>(size) + 1);
    std::snprintf(buffer.data(), buffer.size(), format, args...);
    return std::string(buffer.data(), static_cast<size_t>(size));
}

}

#define PF_LOG_DEBUG(message) ::PixelForge::Logger::debug(message)
#define PF_LOG_INFO(message) ::PixelForge::Logger::info(message)
#define PF_LOG_WARNING(message) ::PixelForge::Logger::warning(message)
#define PF_LOG_ERROR(message) ::PixelForge::Logger::error(message)
#define PF_LOG_FATAL(message) ::PixelForge::Logger::fatal(message)
