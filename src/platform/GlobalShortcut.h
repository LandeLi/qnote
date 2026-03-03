/**
 * @file GlobalShortcut.h
 * @brief 全局快捷键管理
 * @author QNote Team
 * @date 2026-03-04
 *
 * 提供全局快捷键注册和管理功能
 */

#ifndef GLOBALSHORTCUT_H
#define GLOBALSHORTCUT_H

#include <QObject>
#include <QKeySequence>
#include <QHash>
#include <memory>

namespace qnote {

/**
 * @struct ShortcutInfo
 * @brief 快捷键信息结构
 */
struct ShortcutInfo {
    QString id;              ///< 快捷键标识
    QKeySequence keySequence; ///< 按键序列
    QString description;      ///< 描述
    bool enabled;             ///< 是否启用
};

/**
 * @class GlobalShortcut
 * @brief 全局快捷键管理类
 *
 * 提供全局快捷键的注册、管理和触发功能。
 * 跨平台实现，支持：
 * - 快捷键注册/注销
 * - 快捷键冲突检测
 * - 快捷键配置持久化
 *
 * 使用示例：
 * @code
 * auto* shortcut = GlobalShortcut::instance();
 * shortcut->registerShortcut("new_note", QKeySequence("Ctrl+Alt+N"), "New Note");
 * connect(shortcut, &GlobalShortcut::triggered, [](const QString& id) {
 *     if (id == "new_note") {
 *         // 创建新便签
 *     }
 * });
 * @endcode
 */
class GlobalShortcut : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 获取单例实例
     * @return GlobalShortcut 实例指针
     */
    static GlobalShortcut* instance();

    /**
     * @brief 析构函数
     */
    ~GlobalShortcut() override;

    /**
     * @brief 初始化全局快捷键管理器
     * @return 初始化是否成功
     */
    bool initialize();

    /**
     * @brief 注册快捷键
     * @param id 快捷键唯一标识
     * @param keySequence 按键序列
     * @param description 描述（可选）
     * @return 注册是否成功
     */
    bool registerShortcut(const QString& id,
                          const QKeySequence& keySequence,
                          const QString& description = QString());

    /**
     * @brief 注销快捷键
     * @param id 快捷键标识
     * @return 注销是否成功
     */
    bool unregisterShortcut(const QString& id);

    /**
     * @brief 注销所有快捷键
     */
    void unregisterAll();

    /**
     * @brief 检查快捷键是否已注册
     * @param id 快捷键标识
     * @return 是否已注册
     */
    bool isRegistered(const QString& id) const;

    /**
     * @brief 启用/禁用快捷键
     * @param id 快捷键标识
     * @param enabled 是否启用
     */
    void setEnabled(const QString& id, bool enabled);

    /**
     * @brief 检查快捷键是否启用
     * @param id 快捷键标识
     * @return 是否启用
     */
    bool isEnabled(const QString& id) const;

    /**
     * @brief 更新快捷键绑定
     * @param id 快捷键标识
     * @param newKeySequence 新的按键序列
     * @return 更新是否成功
     */
    bool updateShortcut(const QString& id, const QKeySequence& newKeySequence);

    /**
     * @brief 获取快捷键信息
     * @param id 快捷键标识
     * @return 快捷键信息（如果不存在返回空的 ShortcutInfo）
     */
    ShortcutInfo shortcutInfo(const QString& id) const;

    /**
     * @brief 获取所有已注册的快捷键
     * @return 快捷键列表
     */
    QList<ShortcutInfo> allShortcuts() const;

    /**
     * @brief 检查按键序列是否冲突
     * @param keySequence 要检查的按键序列
     * @param excludeId 排除的快捷键ID（用于更新时排除自身）
     * @return 冲突的快捷键ID，如果没有冲突返回空字符串
     */
    QString checkConflict(const QKeySequence& keySequence,
                          const QString& excludeId = QString()) const;

    /**
     * @brief 从配置加载快捷键
     */
    void loadFromSettings();

    /**
     * @brief 保存快捷键到配置
     */
    void saveToSettings();

    /**
     * @brief 恢复默认快捷键
     */
    void restoreDefaults();

    /**
     * @brief 获取默认快捷键列表
     * @return 默认快捷键映射
     */
    static QHash<QString, ShortcutInfo> defaultShortcuts();

signals:
    /**
     * @brief 快捷键触发信号
     * @param id 快捷键标识
     */
    void triggered(const QString& id);

    /**
     * @brief 快捷键注册成功信号
     * @param id 快捷键标识
     */
    void registered(const QString& id);

    /**
     * @brief 快捷键注销信号
     * @param id 快捷键标识
     */
    void unregistered(const QString& id);

    /**
     * @brief 快捷键冲突信号
     * @param keySequence 冲突的按键序列
     * @param existingId 已存在的快捷键ID
     */
    void conflictDetected(const QKeySequence& keySequence, const QString& existingId);

protected:
#ifdef Q_OS_LINUX
    /**
     * @brief 处理 X11 事件（Linux 平台）
     */
    bool nativeEventFilter(const QByteArray& eventType, void* message, long* result) override;
#endif

private:
    /**
     * @brief 私有构造函数（单例模式）
     * @param parent 父对象
     */
    explicit GlobalShortcut(QObject* parent = nullptr);

    // 禁止拷贝
    GlobalShortcut(const GlobalShortcut&) = delete;
    GlobalShortcut& operator=(const GlobalShortcut&) = delete;

    /**
     * @brief 注册平台特定的快捷键
     * @param id 快捷键标识
     * @param keySequence 按键序列
     * @return 是否成功
     */
    bool registerNativeShortcut(const QString& id, const QKeySequence& keySequence);

    /**
     * @brief 注销平台特定的快捷键
     * @param id 快捷键标识
     * @return 是否成功
     */
    bool unregisterNativeShortcut(const QString& id);

    /**
     * @brief 初始化平台特定资源
     */
    bool initNative();

    /**
     * @brief 清理平台特定资源
     */
    void cleanupNative();

private:
    static GlobalShortcut* s_instance;              ///< 单例实例

    QHash<QString, ShortcutInfo> m_shortcuts;       ///< 快捷键映射
    QHash<QString, void*> m_nativeHandles;          ///< 平台相关句柄

    bool m_initialized;                              ///< 是否已初始化
};

} // namespace qnote

#endif // GLOBALSHORTCUT_H
