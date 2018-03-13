#pragma once

#include <iostream>
#include <string>
#include <map>

namespace retro {

    /// Logs things.
    /**
     *  Utility class to make some nice logs. Manages
     *  unique instances of named loggers.
     **/
    class Logger {
    public:
        
        enum LogLevel { Error, Warning, Info, Debug };
        
    private:
        
        static std::map<std::string, Logger> loggers;
        static LogLevel logLevel;
        std::string name;
        
        static const char* logLevelToString(LogLevel);
        
        void printTime();
        void print(LogLevel level, const char* msg, va_list args);
        
        Logger(const std::string &name);
    public:
        Logger();
        Logger(const Logger &l);
        
#ifndef _WIN32
#define attrPrintf __attribute__((format(printf, 2, 3)))
#else
#define attrPrintf //TODO
#endif
        
        /**
         *  Prints an error message to the log. Uses a printf-like
         *  sintax to format the string.
         *  @param msg The formatted message
         *  @param ... The arguments for the formatted string
         **/
        attrPrintf
        void error(const char* msg, ...);
        
        /**
         *  Prints a warning message to the log. Uses a printf-like
         *  sintax to format the string.
         *  @param msg The formatted message
         *  @param ... The arguments for the formatted string
         **/
        attrPrintf
        void warn(const char* msg, ...);
        
        /**
         *  Prints an informational message to the log. Uses a printf-like
         *  sintax to format the string.
         *  @param msg The formatted message
         *  @param ... The arguments for the formatted string
         **/
        attrPrintf
        void info(const char* msg, ...);
        
        /**
         *  Prints a debug message to the log. Uses a printf-like
         *  sintax to format the string.
         *  @param msg The formatted message
         *  @param ... The arguments for the formatted string
         **/
        attrPrintf
        void debug(const char* msg, ...);
        
#undef attrPrintf
        
        /**
         *  Gets the unique logger for that name (usually a class)
         *  @param name Name of the logger, or name of the class for the logger
         *  @return A reference to the Logger
         **/
        static Logger& getLogger(const std::string &name);
        
        /**
         *  Sets the global level for the logging
         *  @param level LogLevel
         **/
        static void setLogLevel(LogLevel level);
    };

}
