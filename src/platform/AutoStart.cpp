/**
 * @file AutoStart.cpp
 * @brief 开机自启动管理实现
 * @author QNote Team
 * @date 2026-03-04
 */

#include "AutoStart.h"
#include "core/Settings.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
#include <QDebug>

#ifdef Q_OS_WIN
#include <QSettings>
#endif

namespace qnote {

// 私有数据结构
struct AutoStart::Private {
    QStringList startupArgs;
    bool startMinimized = false;
};

// 静态成员初始化
AutoStart* AutoStart::s_instance = nullptr;

/**
 * @brief 获取单例实例
 */
AutoStart* AutoStart::instance()
{
    if (!s_instance) {
        s_instance = new AutoStart();
    }
    return s_instance;
}

/**
 * @brief 构造函数
 */
AutoStart::AutoStart(QObject* parent)
    : QObject(parent)
    , d(std::make_unique<Private>())
{
}

/**
 * @brief 析构函数
 */
AutoStart::~AutoStart()
{
}

/**
 * @brief 检查是否支持自启动
 */
bool AutoStart::isSupported()
{
    return true; // Linux/Windows/macOS 都支持
}

/**
 * @brief 初始化自启动管理器
 */
bool AutoStart::initialize()
{
    loadSettings();
    qDebug() << "AutoStart: Initialized successfully";
    return true;
}

/**
 * @brief 启用开机自启动
 */
bool AutoStart::enable()
{
    if (setEnabled(true)) {
        emit enabledChanged(true);
        return true;
    }
    return false;
}

/**
 * @brief 禁用开机自启动
 */
bool AutoStart::disable()
{
    if (setEnabled(false)) {
        emit enabledChanged(false);
        return true;
    }
    return false;
}

/**
 * @brief 设置启动参数
 */
bool AutoStart::setStartupArgs(const QStringList& args)
{
    d->startupArgs = args;

    // 如果已启用，需要更新配置
    if (isEnabled()) {
        // 先禁用再启用以更新配置
        setEnabled(false);
        return setEnabled(true);
    }

    saveSettings();
    return true;
}

/**
 * @brief 获取启动参数
 */
QStringList AutoStart::startupArgs() const
{
    return d->startupArgs;
}

/**
 * @brief 设置是否最小化启动
 */
bool AutoStart::setStartMinimized(bool minimized)
{
    d->startMinimized = minimized;

    // 如果已启用，需要更新配置
    if (isEnabled()) {
        setEnabled(false);
        return setEnabled(true);
    }

    saveSettings();
    return true;
}

/**
 * @brief 从配置加载设置
 */
void AutoStart::loadSettings()
{
    Settings* settings = Settings::instance();

    // 加载启动参数
    QString argsStr = settings->value("autostart/args").toString();
    if (!argsStr.isEmpty()) {
        d->startupArgs = argsStr.split(' ', Qt::SkipEmptyParts);
    }

    // 加载最小化设置
    d->startMinimized = settings->value("autostart/minimized", false).toBool();
}

/**
 * @brief 保存设置到配置
 */
void AutoStart::saveSettings()
{
    Settings* settings = Settings::instance();

    settings->setValue("autostart/args", d->startupArgs.join(' '));
    settings->setValue("autostart/minimized", d->startMinimized);
    settings->sync();
}

// ==================== Linux 实现 ====================

#ifdef Q_OS_LINUX

/**
 * @brief 获取 XDG autostart 目录
 */
QString AutoStart::autostartDir() const
{
    QString configHome = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    return configHome + "/autostart";
}

/**
 * @brief 获取 desktop 文件路径
 */
QString AutoStart::desktopFilePath() const
{
    return autostartDir() + "/qnote.desktop";
}

/**
 * @brief 生成 desktop 文件内容
 */
QString AutoStart::generateDesktopContent() const
{
    QString executable = QCoreApplication::applicationFilePath();

    // 构建启动命令
    QString exec = QString("\"%1\"").arg(executable);
    if (!d->startupArgs.isEmpty()) {
        exec += " " + d->startupArgs.join(' ');
    }
    if (d->startMinimized) {
        exec += " --minimized";
    }

    QString content;
    QTextStream stream(&content);

    stream << "[Desktop Entry]\n";
    stream << "Version=1.0\n";
    stream << "Type=Application\n";
    stream << "Name=QNote\n";
    stream << "Comment=Quick Note Taking Application\n";
    stream << "Exec=" << exec << "\n";
    stream << "Icon=qnote\n";
    stream << "Terminal=false\n";
    stream << "Categories=Qt;Utility;TextEditor;\n";
    stream << "StartupNotify=true\n";
    stream << "X-GNOME-Autostart-enabled=true\n";

    return content;
}

/**
 * @brief 切换自启动状态
 */
bool AutoStart::setEnabled(bool enabled)
{
    if (enabled) {
        QString dir = autostartDir();
        QString filePath = desktopFilePath();

        // 确保目录存在
        QDir().mkpath(dir);

        // 写入 desktop 文件
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning() << "AutoStart: Failed to write desktop file:" << filePath;
            return false;
        }

        QTextStream stream(&file);
        stream << generateDesktopContent();
        file.close();

        // 设置可执行权限
        file.setPermissions(file.permissions() | QFile::ExeOwner);

        qDebug() << "AutoStart: Enabled (Linux):" << filePath;
        return true;
    } else {
        QString filePath = desktopFilePath();

        QFile file(filePath);
        if (file.exists()) {
            if (!file.remove()) {
                qWarning() << "AutoStart: Failed to remove desktop file:" << filePath;
                return false;
            }
        }

        qDebug() << "AutoStart: Disabled (Linux)";
        return true;
    }
}

/**
 * @brief 检查是否已启用自启动
 */
bool AutoStart::isEnabled() const
{
    return QFile::exists(desktopFilePath());
}

/**
 * @brief 获取配置路径
 */
QString AutoStart::configPath() const
{
    return desktopFilePath();
}

#endif // Q_OS_LINUX

// ==================== Windows 实现 ====================

#ifdef Q_OS_WIN

/**
 * @brief 获取注册表键路径
 */
QString AutoStart::registryKey() const
{
    return "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run";
}

/**
 * @brief 切换自启动状态
 */
bool AutoStart::setEnabled(bool enabled)
{
    QSettings settings(registryKey(), QSettings::NativeFormat);

    if (enabled) {
        QString executable = QCoreApplication::applicationFilePath();
        QString exec = QString("\"%1\"").arg(executable);

        if (!d->startupArgs.isEmpty()) {
            exec += " " + d->startupArgs.join(' ');
        }
        if (d->startMinimized) {
            exec += " --minimized";
        }

        settings.setValue("QNote", exec);
        settings.sync();

        if (settings.status() != QSettings::NoError) {
            qWarning() << "AutoStart: Failed to write registry";
            return false;
        }

        qDebug() << "AutoStart: Enabled (Windows)";
        return true;
    } else {
        settings.remove("QNote");
        settings.sync();

        qDebug() << "AutoStart: Disabled (Windows)";
        return true;
    }
}

/**
 * @brief 检查是否已启用自启动
 */
bool AutoStart::isEnabled() const
{
    QSettings settings(registryKey(), QSettings::NativeFormat);
    return settings.contains("QNote");
}

/**
 * @brief 获取配置路径
 */
QString AutoStart::configPath() const
{
    return registryKey();
}

#endif // Q_OS_WIN

// ==================== macOS 实现 ====================

#ifdef Q_OS_MACOS

/**
 * @brief 获取 LaunchAgent plist 路径
 */
QString AutoStart::launchAgentPath() const
{
    QString home = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    return home + "/Library/LaunchAgents/com.qnote.plist";
}

/**
 * @brief 生成 plist 内容
 */
QString AutoStart::generatePlistContent() const
{
    QString executable = QCoreApplication::applicationFilePath();

    QStringList args;
    args << executable;
    args << d->startupArgs;
    if (d->startMinimized) {
        args << "--minimized";
    }

    QString content;
    QTextStream stream(&content);

    stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    stream << "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" "
           << "\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n";
    stream << "<plist version=\"1.0\">\n";
    stream << "<dict>\n";
    stream << "    <key>Label</key>\n";
    stream << "    <string>com.qnote</string>\n";
    stream << "    <key>ProgramArguments</key>\n";
    stream << "    <array>\n";
    for (const QString& arg : args) {
        stream << "        <string>" << arg << "</string>\n";
    }
    stream << "    </array>\n";
    stream << "    <key>RunAtLoad</key>\n";
    stream << "    <true/>\n";
    stream << "</dict>\n";
    stream << "</plist>\n";

    return content;
}

/**
 * @brief 切换自启动状态
 */
bool AutoStart::setEnabled(bool enabled)
{
    QString filePath = launchAgentPath();

    if (enabled) {
        QString dir = QFileInfo(filePath).path();

        // 确保目录存在
        QDir().mkpath(dir);

        // 写入 plist 文件
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning() << "AutoStart: Failed to write plist file:" << filePath;
            return false;
        }

        QTextStream stream(&file);
        stream << generatePlistContent();
        file.close();

        qDebug() << "AutoStart: Enabled (macOS):" << filePath;
        return true;
    } else {
        QFile file(filePath);
        if (file.exists()) {
            if (!file.remove()) {
                qWarning() << "AutoStart: Failed to remove plist file:" << filePath;
                return false;
            }
        }

        qDebug() << "AutoStart: Disabled (macOS)";
        return true;
    }
}

/**
 * @brief 检查是否已启用自启动
 */
bool AutoStart::isEnabled() const
{
    return QFile::exists(launchAgentPath());
}

/**
 * @brief 获取配置路径
 */
QString AutoStart::configPath() const
{
    return launchAgentPath();
}

#endif // Q_OS_MACOS

} // namespace qnote
