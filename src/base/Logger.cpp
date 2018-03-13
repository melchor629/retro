#include "Logger.hpp"
#include <cstdarg>
#include <ctime>
#include <cstring>

#ifdef __ANDROID__
#include <android/log.h>
#endif

using namespace retro;
using namespace std;

map<string, Logger> Logger::loggers;
#ifndef NDEBUG
Logger::LogLevel Logger::logLevel = Logger::LogLevel::Debug;
#else
Logger::LogLevel Logger::logLevel = Logger::LogLevel::Warning;
#endif

Logger& Logger::getLogger(const string &name) {
    if(loggers.find(name) == loggers.end()) {
        loggers[name] = Logger(name);
    }
    return loggers[name];
}

void Logger::setLogLevel(LogLevel l) {
    cout << "[!!] Log level changed from " << logLevelToString(logLevel) << " to " << logLevelToString(l) << endl;
    logLevel = l;
}

const char* Logger::logLevelToString(Logger::LogLevel level) {
    switch(level) {
        case Error: return("error");
        case Warning: return("warning");
        case Info: return("info");
        case Debug: return("debug");
        default: return nullptr;
    }
}

Logger::Logger(const string &name): name(name) {}
Logger::Logger(): name("undefined") {}
Logger::Logger(const Logger &l): name(l.name) {}

#ifndef _WIN32
#define attrPrintf __attribute__((format(printf, 2, 3)))
#else
#define attrPrintf //TODO
#endif

attrPrintf
void Logger::error(const char* msg, ...) {
    va_list valist;
    va_start(valist, msg);
    print(Error, msg, valist);
    va_end(valist);
}

attrPrintf
void Logger::warn(const char* msg, ...) {
    va_list valist;
    va_start(valist, msg);
    print(Warning, msg, valist);
    va_end(valist);
}

attrPrintf
void Logger::info(const char* msg, ...) {
    va_list valist;
    va_start(valist, msg);
    print(Info, msg, valist);
    va_end(valist);
}

attrPrintf
void Logger::debug(const char* msg, ...) {
    va_list valist;
    va_start(valist, msg);
    print(Debug, msg, valist);
    va_end(valist);
}

void Logger::printTime() {
    time_t t = time(nullptr);
	struct tm tm;
#ifdef _WIN32
	localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    printf("%d-%c%d-%c%d %c%d:%c%d:%c%d - ",
           tm.tm_year + 1900,
           tm.tm_mon <  9 ? '0' : '\0', tm.tm_mon + 1,
           tm.tm_mday< 10 ? '0' : '\0', tm.tm_mday,
           tm.tm_hour< 10 ? '0' : '\0', tm.tm_hour,
           tm.tm_min < 10 ? '0' : '\0', tm.tm_min,
           tm.tm_sec < 10 ? '0' : '\0', tm.tm_sec);
}

#ifndef __ANDROID__
void Logger::print(LogLevel level, const char* msg, va_list args) {
    va_list args_copy;
    va_copy(args_copy, args);
    if(level <= logLevel) {
        printTime();
        int len = 7-int(strlen(logLevelToString(level)));
        printf("%.*s%s [%s] ", len, "   ", logLevelToString(level), name.c_str());
        vprintf(msg, args_copy);
        printf("\n");
    }
    va_end(args_copy);
}
#else
void Logger::print(LogLevel level, const char* msg, va_list args) {
    va_list args_copy;
    va_copy(args_copy, args);
    //On Android we don't care about log level, Android Studio is who applies the filter
    int androidLevel;
    switch(level) {
        case Error: androidLevel = ANDROID_LOG_ERROR; break;
        case Warning: androidLevel = ANDROID_LOG_WARN; break;
        case Info: androidLevel = ANDROID_LOG_INFO; break;
        case Debug: androidLevel = ANDROID_LOG_DEBUG; break;
        default: androidLevel = ANDROID_LOG_VERBOSE; break;
    }
    __android_log_vprint(androidLevel, name.c_str(), msg, args_copy);
    va_end(args_copy);
}
#endif
