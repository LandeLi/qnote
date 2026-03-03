/**
 * @file AutoStart.h
 * @brief 开机自启动管理
 * @author QNote Team
 * @date 2026-03-04
 *
 * 提供跨平台的开机自启动配置功能
 */

#ifndef AUTOSTART_H
#define AUTOSTART_H

#include <QObject>
#include <QString>
#include <memory>

namespace qnote {

/**
 * @class AutoStart
 * @brief 开机自启动管理类
 *
 * 提供开机自启动的配置和管理功能，支持：
 * - 启用/禁用自启动
 * - 检查自启动状态
 * - 跨平台支持（Linux/Windows/macOS）
 *
 * 使用示例：
 * @code
 * auto* autoStart = AutoStart::instance();
 * autoStart->enable();
 * if (autoStart->isEnabled()) {
 *     qDebug() << "Auto-start is enabled";
 * }
 * @endcode
 *
 * 平台实现：
 * - Linux: XDG autostart (~/.config/autostart/qnote.desktop)
 * - Windows: 注册表 (HKCU\Software\Microsoft\Windows\CurrentVersion\Run)
 * - macOS: LaunchAgent (~/Library/LaunchAgents/com.qnote.plist)
 */
class AutoStart : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 获取单例实例
     * @return AutoStart 实例指针
     */
    static AutoStart* instance();

    /**
     * @brief 析构函数
     */
    ~AutoStart() override;

    /**
     * @brief 初始化自启动管理器
     * @return 初始化是否成功
     */
    bool initialize();

    /**
     * @brief 启用开机自启动
     * @return 是否设置成功
     */
    bool enable();

    /**
     * @brief 禁用开机自启动
     * @return 是否取消成功
     */
    bool disable();

    /**
     * @brief 切换自启动状态
     * @param enabled 是否启用
     * @return 是否设置成功
     */
    bool setEnabled(bool enabled);

    /**
     * @brief 检查是否已启用自启动
     * @return 是否启用
     */
    bool isEnabled() const;

    /**
     * @brief 检查是否支持自启动
     * @return 是否支持
     */
    static bool isSupported();

    /**
     * @brief 设置启动参数
     * @param args 启动参数列表
     * @return 是否设置成功
     */
    bool setStartupArgs(const QStringList& args);

    /**
     * @brief 获取启动参数
     * @return 启动参数列表
     */
    QStringList startupArgs() const;

    /**
     * @brief 设置是否最小化启动
     * @param minimized 是否最小化
     * @return 是否设置成功
     */
    bool setStartMinimized(bool minimized);

    /**
     * @brief 获取自启动配置文件路径
     * @return 配置文件路径
     */
    QString configPath() const;

signals:
    /**
     * @brief 自启动状态改变
     * @param enabled 是否启用
     */
    void enabledChanged(bool enabled);

private:
    /**
     * @brief 私有构造函数（单例模式）
     * @param parent 父对象
     */
    explicit AutoStart(QObject* parent = nullptr);

    // 禁止拷贝
    AutoStart(const AutoStart&) = delete;
    AutoStart& operator=(const AutoStart&) = delete;

#ifdef Q_OS_LINUX
    /**
     * @brief 生成 desktop 文件内容
     * @return desktop 文件内容
     */
    QString generateDesktopContent() const;

    /**
     * @brief 获取 XDG autostart 目录
     * @return 目录路径
     */
    QString autostartDir() const;

    /**
     * @brief 获取 desktop 文件路径
     * @return 文件路径
     */
    QString desktopFilePath() const;
#endif

#ifdef Q_OS_WIN
    /**
     * @brief 获取注册表键路径
     * @return 注册表路径
     */
    QString registryKey() const;
#endif

#ifdef Q_OS_MACOS
    /**
     * @brief 获取 LaunchAgent plist 路径
     * @return 文件路径
     */
    QString launchAgentPath() const;

    /**
     * @brief 生成 plist 内容
     * @return plist 文件内容
     */
    QString generatePlistContent() const;
#endif

    /**
     * @brief 从配置加载设置
     */
    void loadSettings();

    /**
     * @brief 保存设置到配置
     */
    void saveSettings();

private:
    static AutoStart* s_instance;  ///< 单例实例

    struct Private;
    std::unique_ptr<Private> d;
};

} // namespace qnote

#endif // AUTOSTART_H
