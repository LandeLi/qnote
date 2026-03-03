#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMutex>
#include <memory>

namespace qnote {

/**
 * @brief 日志级别
 */
enum class LogLevel {
    Debug = 0,
    Info = 1,
    Warning = 2,
    Error = 3,
    Fatal = 4
};

/**
 * @brief 简易日志系统
 * 
 * 支持输出到控制台和文件，线程安全
 */
class Logger {
public:
    static Logger* instance();
    
    /**
     * @brief 初始化日志系统
     * @param logDir 日志目录路径
     * @param level 最低日志级别
     * @param toConsole 是否输出到控制台
     * @param toFile 是否输出到文件
     */
    void init(const QString& logDir = QString(),
              LogLevel level = LogLevel::Debug,
              bool toConsole = true,
              bool toFile = true);
    
    void debug(const QString& message, const char* file = nullptr, int line = 0);
    void info(const QString& message, const char* file = nullptr, int line = 0);
    void warning(const QString& message, const char* file = nullptr, int line = 0);
    void error(const QString& message, const char* file = nullptr, int line = 0);
    void fatal(const QString& message, const char* file = nullptr, int line = 0);
    
    void log(LogLevel level, const QString& message, 
             const char* file = nullptr, int line = 0);
    
    /**
     * @brief 设置最低日志级别
     */
    void setLevel(LogLevel level);
    
    /**
     * @brief 获取当前日志文件路径
     */
    QString logFilePath() const;

private:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    QString levelToString(LogLevel level) const;
    QString formatMessage(LogLevel level, const QString& message,
                          const char* file, int line);
    void writeToFile(const QString& formattedMessage);
    void writeToConsole(const QString& formattedMessage);
    void rotateLogFile();

private:
    static std::unique_ptr<Logger> s_instance;
    
    LogLevel m_level;
    bool m_toConsole;
    bool m_toFile;
    QString m_logDir;
    QString m_logFilePath;
    std::unique_ptr<QFile> m_logFile;
    std::unique_ptr<QTextStream> m_stream;
    QMutex m_mutex;
    
    static const qint64 MAX_LOG_SIZE = 10 * 1024 * 1024; // 10MB
};

} // namespace qnote

// 便捷宏
#define LOG_DEBUG(msg)   qnote::Logger::instance()->debug(msg, __FILE__, __LINE__)
#define LOG_INFO(msg)    qnote::Logger::instance()->info(msg, __FILE__, __LINE__)
#define LOG_WARN(msg)    qnote::Logger::instance()->warning(msg, __FILE__, __LINE__)
#define LOG_ERROR(msg)   qnote::Logger::instance()->error(msg, __FILE__, __LINE__)
#define LOG_FATAL(msg)   qnote::Logger::instance()->fatal(msg, __FILE__, __LINE__)

#endif // LOGGER_H
