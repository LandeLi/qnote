#include "Logger.h"
#include <QDir>
#include <QStandardPaths>
#include <iostream>

namespace qnote {

std::unique_ptr<Logger> Logger::s_instance = nullptr;

Logger* Logger::instance() {
    if (!s_instance) {
        s_instance.reset(new Logger());
    }
    return s_instance.get();
}

Logger::Logger()
    : m_level(LogLevel::Debug)
    , m_toConsole(true)
    , m_toFile(false)
{
}

Logger::~Logger() {
    if (m_stream) {
        m_stream->flush();
    }
    if (m_logFile && m_logFile->isOpen()) {
        m_logFile->close();
    }
}

void Logger::init(const QString& logDir, LogLevel level, 
                  bool toConsole, bool toFile) {
    QMutexLocker locker(&m_mutex);
    
    m_level = level;
    m_toConsole = toConsole;
    m_toFile = toFile;
    
    if (toFile) {
        // 确定日志目录
        if (logDir.isEmpty()) {
            QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
            m_logDir = dataPath + "/logs";
        } else {
            m_logDir = logDir;
        }
        
        // 创建日志目录
        QDir dir(m_logDir);
        if (!dir.exists()) {
            dir.mkpath(".");
        }
        
        // 创建日志文件（按日期命名）
        QString dateStr = QDateTime::currentDateTime().toString("yyyy-MM-dd");
        m_logFilePath = m_logDir + QString("/qnote-%1.log").arg(dateStr);
        
        m_logFile = std::make_unique<QFile>(m_logFilePath);
        if (m_logFile->open(QIODevice::WriteOnly | QIODevice::Append)) {
            m_stream = std::make_unique<QTextStream>(m_logFile.get());
            m_stream->setEncoding(QStringConverter::Utf8);
        }
    }
    
    LOG_INFO("Logger initialized");
}

void Logger::debug(const QString& message, const char* file, int line) {
    log(LogLevel::Debug, message, file, line);
}

void Logger::info(const QString& message, const char* file, int line) {
    log(LogLevel::Info, message, file, line);
}

void Logger::warning(const QString& message, const char* file, int line) {
    log(LogLevel::Warning, message, file, line);
}

void Logger::error(const QString& message, const char* file, int line) {
    log(LogLevel::Error, message, file, line);
}

void Logger::fatal(const QString& message, const char* file, int line) {
    log(LogLevel::Fatal, message, file, line);
}

void Logger::log(LogLevel level, const QString& message, 
                 const char* file, int line) {
    if (level < m_level) {
        return;
    }
    
    QString formatted = formatMessage(level, message, file, line);
    
    QMutexLocker locker(&m_mutex);
    
    if (m_toConsole) {
        writeToConsole(formatted);
    }
    
    if (m_toFile && m_stream) {
        writeToFile(formatted);
    }
}

void Logger::setLevel(LogLevel level) {
    QMutexLocker locker(&m_mutex);
    m_level = level;
}

QString Logger::logFilePath() const {
    return m_logFilePath;
}

QString Logger::levelToString(LogLevel level) const {
    switch (level) {
    case LogLevel::Debug:   return "DEBUG";
    case LogLevel::Info:    return "INFO";
    case LogLevel::Warning: return "WARN";
    case LogLevel::Error:   return "ERROR";
    case LogLevel::Fatal:   return "FATAL";
    default:                return "UNKNOWN";
    }
}

QString Logger::formatMessage(LogLevel level, const QString& message,
                              const char* file, int line) {
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString levelStr = levelToString(level);
    
    QString formatted = QString("[%1] [%2] %3")
        .arg(timestamp)
        .arg(levelStr, -5)
        .arg(message);
    
    if (file && line > 0) {
        // 只取文件名，不取完整路径
        QString fileName = QFileInfo(file).fileName();
        formatted += QString(" (%1:%2)").arg(fileName).arg(line);
    }
    
    return formatted;
}

void Logger::writeToFile(const QString& formattedMessage) {
    if (m_stream) {
        *m_stream << formattedMessage << "\n";
        m_stream->flush();
        
        // 检查日志文件大小，必要时轮转
        if (m_logFile && m_logFile->size() > MAX_LOG_SIZE) {
            rotateLogFile();
        }
    }
}

void Logger::writeToConsole(const QString& formattedMessage) {
    std::cout << formattedMessage.toStdString() << std::endl;
}

void Logger::rotateLogFile() {
    if (!m_logFile) return;
    
    m_stream.reset();
    m_logFile->close();
    
    // 重命名旧日志文件
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss");
    QString rotatedPath = m_logFilePath + "." + timestamp + ".old";
    QFile::rename(m_logFilePath, rotatedPath);
    
    // 重新打开日志文件
    m_logFile = std::make_unique<QFile>(m_logFilePath);
    if (m_logFile->open(QIODevice::WriteOnly | QIODevice::Append)) {
        m_stream = std::make_unique<QTextStream>(m_logFile.get());
        m_stream->setEncoding(QStringConverter::Utf8);
    }
}

} // namespace qnote
