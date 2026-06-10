#include "Utils/Logger.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace PixelForge {

std::ofstream Logger::s_logFile;
std::mutex Logger::s_writeMutex;
std::mutex Logger::s_queueMutex;
std::condition_variable Logger::s_queueCv;
std::queue<LogMessage> Logger::s_logQueue;
std::thread Logger::s_logThread;
std::atomic<bool> Logger::s_running {false};
std::atomic<bool> Logger::s_asyncMode {true};
std::atomic<LogLevel> Logger::s_minLogLevel {LogLevel::Debug};
std::atomic<bool> Logger::s_fileOutputEnabled {true};
std::atomic<bool> Logger::s_consoleOutputEnabled {true};

void Logger::init(const std::string &logFile, bool consoleOutput, bool asyncMode)
{
    shutdown();

    s_consoleOutputEnabled = consoleOutput;
    s_asyncMode = asyncMode;

    {
        std::lock_guard<std::mutex> lock(s_writeMutex);
        if (s_fileOutputEnabled) {
            s_logFile.open(logFile, std::ios::out | std::ios::app);
            if (s_logFile.is_open()) {
                s_logFile << "\n========== PixelForge started at " << timestamp() << " ==========\n";
                s_logFile.flush();
            }
        }
    }

    if (s_asyncMode) {
        s_running = true;
        s_logThread = std::thread(processLogQueue);
    }
}

void Logger::shutdown()
{
    if (s_asyncMode && s_logThread.joinable()) {
        s_running = false;
        s_queueCv.notify_all();
        s_logThread.join();
    }

    flush();

    std::lock_guard<std::mutex> lock(s_writeMutex);
    if (s_logFile.is_open()) {
        s_logFile << "========== PixelForge shutdown at " << timestamp() << " ==========\n\n";
        s_logFile.close();
    }
}

void Logger::debug(const std::string &message)
{
    log(LogLevel::Debug, message);
}

void Logger::info(const std::string &message)
{
    log(LogLevel::Info, message);
}

void Logger::warning(const std::string &message)
{
    log(LogLevel::Warning, message);
}

void Logger::error(const std::string &message)
{
    log(LogLevel::Error, message);
}

void Logger::fatal(const std::string &message)
{
    log(LogLevel::Fatal, message);
    flush();
}

void Logger::setLogLevel(LogLevel level)
{
    s_minLogLevel = level;
}

void Logger::enableFileOutput(bool enable)
{
    s_fileOutputEnabled = enable;
}

void Logger::enableConsoleOutput(bool enable)
{
    s_consoleOutputEnabled = enable;
}

void Logger::flush()
{
    for (;;) {
        {
            std::lock_guard<std::mutex> lock(s_queueMutex);
            if (s_logQueue.empty()) {
                break;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    std::lock_guard<std::mutex> lock(s_writeMutex);
    if (s_logFile.is_open()) {
        s_logFile.flush();
    }
}

void Logger::log(LogLevel level, const std::string &message)
{
    if (level < s_minLogLevel.load()) {
        return;
    }

    LogMessage logMessage;
    logMessage.level = level;
    logMessage.message = message;
    logMessage.timestamp = timestamp();
    logMessage.threadId = std::this_thread::get_id();

    if (s_asyncMode && s_running) {
        {
            std::lock_guard<std::mutex> lock(s_queueMutex);
            s_logQueue.push(std::move(logMessage));
        }
        s_queueCv.notify_one();
        return;
    }

    writeLog(logMessage);
}

void Logger::processLogQueue()
{
    for (;;) {
        std::unique_lock<std::mutex> lock(s_queueMutex);
        s_queueCv.wait(lock, [] {
            return !s_logQueue.empty() || !s_running;
        });

        if (!s_running && s_logQueue.empty()) {
            break;
        }

        LogMessage logMessage = std::move(s_logQueue.front());
        s_logQueue.pop();
        lock.unlock();

        writeLog(logMessage);
    }
}

void Logger::writeLog(const LogMessage &logMessage)
{
    std::lock_guard<std::mutex> lock(s_writeMutex);

    const std::string line = "[" + logMessage.timestamp + "] [" + threadIdString(logMessage.threadId) + "] ["
        + levelString(logMessage.level) + "] " + logMessage.message;

    if (s_consoleOutputEnabled) {
        std::cout << levelColor(logMessage.level) << line << "\033[0m" << std::endl;
    }

    if (s_fileOutputEnabled && s_logFile.is_open()) {
        s_logFile << line << std::endl;
        if (logMessage.level >= LogLevel::Error) {
            s_logFile.flush();
        }
    }
}

std::string Logger::timestamp()
{
    const auto now = std::chrono::system_clock::now();
    const auto time = std::chrono::system_clock::to_time_t(now);
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::tm localTime {};
#ifdef _WIN32
    localtime_s(&localTime, &time);
#else
    localtime_r(&time, &localTime);
#endif

    std::ostringstream stream;
    stream << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
    stream << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return stream.str();
}

std::string Logger::levelString(LogLevel level)
{
    switch (level) {
    case LogLevel::Debug:
        return "DEBUG";
    case LogLevel::Info:
        return "INFO ";
    case LogLevel::Warning:
        return "WARN ";
    case LogLevel::Error:
        return "ERROR";
    case LogLevel::Fatal:
        return "FATAL";
    }

    return "UNKWN";
}

std::string Logger::levelColor(LogLevel level)
{
    switch (level) {
    case LogLevel::Debug:
        return "\033[36m";
    case LogLevel::Info:
        return "\033[32m";
    case LogLevel::Warning:
        return "\033[33m";
    case LogLevel::Error:
        return "\033[31m";
    case LogLevel::Fatal:
        return "\033[35m";
    }

    return "\033[0m";
}

std::string Logger::threadIdString(std::thread::id threadId)
{
    std::ostringstream stream;
    stream << "T-" << std::hex << std::setw(4) << std::setfill('0')
           << (std::hash<std::thread::id> {}(threadId) % 0xFFFF);
    return stream.str();
}

}
