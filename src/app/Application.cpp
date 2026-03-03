#include "Application.h"
#include "core/Logger.h"
#include "core/Settings.h"
#include "core/Utils.h"

#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QLockFile>
#include <QMessageBox>
#include <QStyleFactory>

namespace qnote {

// Application 私有数据
struct Application::Private {
    QString dataPath;
    QString configPath;
    QString logPath;
    std::unique_ptr<QLockFile> lockFile;
    bool initialized = false;
};

Application* Application::s_instance = nullptr;

Application::Application(int& argc, char** argv)
    : QApplication(argc, argv)
    , d(std::make_unique<Private>())
{
    s_instance = this;
    
    // 设置应用信息
    setApplicationName("QNote");
    setApplicationVersion("0.1.0");
    setOrganizationName("QNote");
    setOrganizationDomain("qnote.local");
}

Application::~Application() {
    LOG_INFO("Application shutting down");
    
    if (d->lockFile) {
        d->lockFile->unlock();
    }
    
    s_instance = nullptr;
}

Application* Application::instance() {
    return s_instance;
}

bool Application::initialize() {
    if (d->initialized) {
        return true;
    }
    
    LOG_INFO("Initializing QNote...");
    
    // 检查单实例
    if (!checkSingleInstance()) {
        LOG_ERROR("Another instance is already running");
        return false;
    }
    
    // 初始化路径
    initPaths();
    
    // 初始化日志
    initLogger();
    
    // 初始化配置
    initSettings();
    
    // 初始化数据库
    if (!initDatabase()) {
        LOG_ERROR("Failed to initialize database");
        return false;
    }
    
    // 加载样式表
    loadStyleSheet();
    
    d->initialized = true;
    LOG_INFO("QNote initialized successfully");
    
    return true;
}

QString Application::version() const {
    return applicationVersion();
}

QString Application::name() const {
    return applicationName();
}

QString Application::organization() const {
    return organizationName();
}

QString Application::dataPath() const {
    return d->dataPath;
}

QString Application::configPath() const {
    return d->configPath;
}

QString Application::logPath() const {
    return d->logPath;
}

bool Application::isFirstRun() const {
    return !Settings::instance()->contains("app/firstRunComplete");
}

void Application::setFirstRunComplete() {
    Settings::instance()->setValue("app/firstRunComplete", true);
}

bool Application::event(QEvent* event) {
    return QApplication::event(event);
}

void Application::initPaths() {
    // 数据目录
    d->dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    Utils::ensureDirExists(d->dataPath);
    
    // 配置目录
    d->configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    Utils::ensureDirExists(d->configPath);
    
    // 日志目录
    d->logPath = d->dataPath + "/logs";
    Utils::ensureDirExists(d->logPath);
    
    // 数据库目录
    Utils::ensureDirExists(d->dataPath + "/data");
}

void Application::initLogger() {
    Logger::instance()->init(
        d->logPath,
        LogLevel::Debug,  // TODO: 根据构建类型调整
        true,  // toConsole
        true   // toFile
    );
    
    LOG_INFO(QString("QNote %1 starting...").arg(version()));
    LOG_INFO(QString("Data path: %1").arg(d->dataPath));
    LOG_INFO(QString("Config path: %1").arg(d->configPath));
}

void Application::initSettings() {
    QString configFile = d->configPath + "/qnote.ini";
    Settings::instance()->initFromFile(configFile);
    LOG_INFO(QString("Config file: %1").arg(configFile));
}

bool Application::initDatabase() {
    // TODO: 实现数据库初始化（M01 模块）
    LOG_INFO("Database initialization (TODO - M01 module)");
    return true;
}

void Application::loadStyleSheet() {
    QString theme = Settings::instance()->theme();
    QString qssFile = QString(":/styles/%1.qss").arg(theme);
    
    QFile file(qssFile);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString style = QString::fromUtf8(file.readAll());
        setStyleSheet(style);
        LOG_INFO(QString("Loaded theme: %1").arg(theme));
    } else {
        LOG_WARN(QString("Failed to load theme: %1, using default").arg(qssFile));
    }
}

bool Application::checkSingleInstance() {
    QString lockPath = d->dataPath + "/qnote.lock";
    d->lockFile = std::make_unique<QLockFile>(lockPath);
    d->lockFile->setStaleLockTime(0);
    
    if (!d->lockFile->tryLock(100)) {
        switch (d->lockFile->error()) {
        case QLockFile::LockFailedError:
            // 另一个实例正在运行
            QMessageBox::warning(
                nullptr,
                tr("QNote"),
                tr("QNote is already running. Check the system tray.")
            );
            break;
        case QLockFile::UnknownError:
            QMessageBox::warning(
                nullptr,
                tr("QNote"),
                tr("Unable to create lock file at %1").arg(lockPath)
            );
            break;
        default:
            break;
        }
        return false;
    }
    
    return true;
}

} // namespace qnote
