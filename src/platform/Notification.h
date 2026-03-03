/**
 * @file Notification.h
 * @brief 系统通知管理
 * @author QNote Team
 * @date 2026-03-04
 *
 * 提供跨平台的系统通知功能
 */

#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <QObject>
#include <QString>
#include <QIcon>
#include <memory>

namespace qnote {

/**
 * @enum NotificationPriority
 * @brief 通知优先级
 */
enum class NotificationPriority {
    Low,       ///< 低优先级
    Normal,    ///< 普通优先级
    High       ///< 高优先级
};

/**
 * @struct NotificationData
 * @brief 通知数据结构
 */
struct NotificationData {
    QString id;                      ///< 通知ID
    QString title;                   ///< 标题
    QString body;                    ///< 内容
    QString iconPath;                ///< 图标路径
    NotificationPriority priority;   ///< 优先级
    int timeout;                     ///< 超时时间（毫秒），0 表示永不过期
    bool actionable;                 ///< 是否包含操作按钮
};

/**
 * @class Notification
 * @brief 系统通知管理类
 *
 * 封装平台特定的通知系统，提供：
 * - 发送系统通知
 * - 通知点击处理
 * - 通知优先级
 * - 提醒通知（便签提醒）
 *
 * 使用示例：
 * @code
 * auto* notify = Notification::instance();
 * notify->send("reminder_1", "Reminder", "Don't forget the meeting!");
 * connect(notify, &Notification::clicked, [](const QString& id) {
 *     // 用户点击了通知
 * });
 * @endcode
 */
class Notification : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 获取单例实例
     * @return Notification 实例指针
     */
    static Notification* instance();

    /**
     * @brief 析构函数
     */
    ~Notification() override;

    /**
     * @brief 初始化通知管理器
     * @return 初始化是否成功
     */
    bool initialize();

    /**
     * @brief 检查系统是否支持通知
     * @return 是否支持
     */
    static bool isAvailable();

    /**
     * @brief 发送简单通知
     * @param title 标题
     * @param body 内容
     * @return 是否发送成功
     */
    bool send(const QString& title, const QString& body);

    /**
     * @brief 发送带 ID 的通知
     * @param id 通知ID（用于后续更新或关闭）
     * @param title 标题
     * @param body 内容
     * @param priority 优先级（默认 Normal）
     * @param timeout 超时时间（毫秒，默认 -1 表示系统默认）
     * @return 是否发送成功
     */
    bool send(const QString& id,
              const QString& title,
              const QString& body,
              NotificationPriority priority = NotificationPriority::Normal,
              int timeout = -1);

    /**
     * @brief 发送完整通知
     * @param data 通知数据
     * @return 是否发送成功
     */
    bool send(const NotificationData& data);

    /**
     * @brief 发送提醒通知
     * @param noteId 关联的便签ID
     * @param title 标题
     * @param body 内容
     * @return 是否发送成功
     */
    bool sendReminder(const QString& noteId, const QString& title, const QString& body);

    /**
     * @brief 更新已存在的通知
     * @param id 通知ID
     * @param title 新标题
     * @param body 新内容
     * @return 是否更新成功
     */
    bool update(const QString& id, const QString& title, const QString& body);

    /**
     * @brief 关闭通知
     * @param id 通知ID
     */
    void close(const QString& id);

    /**
     * @brief 关闭所有通知
     */
    void closeAll();

    /**
     * @brief 设置应用图标（用于通知）
     * @param iconPath 图标路径
     */
    void setApplicationIcon(const QString& iconPath);

    /**
     * @brief 设置应用名称
     * @param name 应用名称
     */
    void setApplicationName(const QString& name);

signals:
    /**
     * @brief 通知被点击
     * @param id 通知ID
     */
    void clicked(const QString& id);

    /**
     * @brief 通知被关闭
     * @param id 通知ID
     */
    void closed(const QString& id);

    /**
     * @brief 通知操作被触发
     * @param id 通知ID
     * @param action 操作标识
     */
    void actionTriggered(const QString& id, const QString& action);

    /**
     * @brief 通知发送成功
     * @param id 通知ID
     */
    void sent(const QString& id);

    /**
     * @brief 通知发送失败
     * @param id 通知ID
     * @param error 错误信息
     */
    void failed(const QString& id, const QString& error);

private slots:
#ifdef Q_OS_LINUX
    /**
     * @brief 处理 DBus 通知关闭信号
     * @param id 通知ID
     * @param reason 关闭原因
     */
    void onDBusNotificationClosed(uint id, uint reason);

    /**
     * @brief 处理 DBus 动作触发信号
     * @param id 通知ID
     * @param action 动作标识
     */
    void onDBusActionInvoked(uint id, const QString& action);
#endif

private:
    /**
     * @brief 私有构造函数（单例模式）
     * @param parent 父对象
     */
    explicit Notification(QObject* parent = nullptr);

    // 禁止拷贝
    Notification(const Notification&) = delete;
    Notification& operator=(const Notification&) = delete;

    /**
     * @brief 初始化 DBus 通知接口（Linux）
     * @return 是否成功
     */
    bool initDBus();

    /**
     * @brief 通过 DBus 发送通知（Linux）
     */
    bool sendViaDBus(const NotificationData& data);

    /**
     * @brief 通过 QSystemTrayIcon 发送通知（回退方案）
     */
    bool sendViaTray(const NotificationData& data);

    /**
     * @brief 生成唯一通知ID
     */
    QString generateId() const;

    /**
     * @brief 优先级转换为字符串
     */
    QString priorityToString(NotificationPriority priority) const;

private:
    static Notification* s_instance;  ///< 单例实例

    struct Private;
    std::unique_ptr<Private> d;
};

} // namespace qnote

#endif // NOTIFICATION_H
