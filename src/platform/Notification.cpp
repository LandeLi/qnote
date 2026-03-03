/**
 * @file Notification.cpp
 * @brief 系统通知管理实现
 * @author QNote Team
 * @date 2026-03-04
 */

#include "Notification.h"
#include "TrayIcon.h"

#include <QUuid>
#include <QDebug>
#include <QCoreApplication>

#ifdef Q_OS_LINUX
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusArgument>
#endif

namespace qnote {

// 私有数据结构
struct Notification::Private {
    QString appName;
    QString appIcon;
    uint dbusSerial = 0;
};

// 静态成员初始化
Notification* Notification::s_instance = nullptr;

/**
 * @brief 获取单例实例
 */
Notification* Notification::instance()
{
    if (!s_instance) {
        s_instance = new Notification();
    }
    return s_instance;
}

/**
 * @brief 构造函数
 */
Notification::Notification(QObject* parent)
    : QObject(parent)
    , d(std::make_unique<Private>())
{
    d->appName = QCoreApplication::applicationName();
    if (d->appName.isEmpty()) {
        d->appName = "QNote";
    }
}

/**
 * @brief 析构函数
 */
Notification::~Notification()
{
    closeAll();
}

/**
 * @brief 检查系统是否支持通知
 */
bool Notification::isAvailable()
{
#ifdef Q_OS_LINUX
    // 检查 DBus 通知服务是否可用
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected()) {
        return false;
    }

    QDBusInterface iface("org.freedesktop.Notifications",
                         "/org/freedesktop/Notifications",
                         "org.freedesktop.Notifications",
                         bus);
    return iface.isValid();
#else
    // 其他平台使用托盘通知
    return TrayIcon::isSystemTrayAvailable();
#endif
}

/**
 * @brief 初始化通知管理器
 */
bool Notification::initialize()
{
#ifdef Q_OS_LINUX
    if (!initDBus()) {
        qWarning() << "Notification: DBus initialization failed, will use tray fallback";
    }
#endif

    qDebug() << "Notification: Initialized successfully";
    return true;
}

/**
 * @brief 发送简单通知
 */
bool Notification::send(const QString& title, const QString& body)
{
    QString id = generateId();
    return send(id, title, body);
}

/**
 * @brief 发送带 ID 的通知
 */
bool Notification::send(const QString& id,
                        const QString& title,
                        const QString& body,
                        NotificationPriority priority,
                        int timeout)
{
    NotificationData data;
    data.id = id;
    data.title = title;
    data.body = body;
    data.priority = priority;
    data.timeout = timeout;
    data.iconPath = d->appIcon;

    return send(data);
}

/**
 * @brief 发送完整通知
 */
bool Notification::send(const NotificationData& data)
{
#ifdef Q_OS_LINUX
    // 优先使用 DBus 通知
    if (sendViaDBus(data)) {
        emit sent(data.id);
        return true;
    }
#endif

    // 回退到托盘通知
    if (sendViaTray(data)) {
        emit sent(data.id);
        return true;
    }

    emit failed(data.id, tr("Failed to send notification"));
    return false;
}

/**
 * @brief 发送提醒通知
 */
bool Notification::sendReminder(const QString& noteId,
                                const QString& title,
                                const QString& body)
{
    QString id = QString("reminder_%1").arg(noteId);

    NotificationData data;
    data.id = id;
    data.title = title;
    data.body = body;
    data.priority = NotificationPriority::High;
    data.timeout = 0;  // 不自动关闭
    data.iconPath = d->appIcon;

    return send(data);
}

/**
 * @brief 更新已存在的通知
 */
bool Notification::update(const QString& id, const QString& title, const QString& body)
{
    // 对于 DBus 通知，可以通过相同的 replaces_id 更新
    NotificationData data;
    data.id = id;
    data.title = title;
    data.body = body;

    return send(data);
}

/**
 * @brief 关闭通知
 */
void Notification::close(const QString& id)
{
#ifdef Q_OS_LINUX
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (bus.isConnected()) {
        QDBusInterface iface("org.freedesktop.Notifications",
                             "/org/freedesktop/Notifications",
                             "org.freedesktop.Notifications",
                             bus);

        if (iface.isValid()) {
            // CloseNotification 方法
            iface.call("CloseNotification", d->dbusSerial);
        }
    }
#endif

    emit closed(id);
}

/**
 * @brief 关闭所有通知
 */
void Notification::closeAll()
{
    // 对于 DBus，没有关闭所有的 API
    // 只能逐个关闭已知的通知
}

/**
 * @brief 设置应用图标
 */
void Notification::setApplicationIcon(const QString& iconPath)
{
    d->appIcon = iconPath;
}

/**
 * @brief 设置应用名称
 */
void Notification::setApplicationName(const QString& name)
{
    d->appName = name;
}

/**
 * @brief 生成唯一通知ID
 */
QString Notification::generateId() const
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

/**
 * @brief 优先级转换为字符串
 */
QString Notification::priorityToString(NotificationPriority priority) const
{
    switch (priority) {
    case NotificationPriority::Low:
        return "low";
    case NotificationPriority::Normal:
        return "normal";
    case NotificationPriority::High:
        return "high";
    default:
        return "normal";
    }
}

#ifdef Q_OS_LINUX

/**
 * @brief 初始化 DBus 通知接口
 */
bool Notification::initDBus()
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected()) {
        qWarning() << "Notification: Cannot connect to session bus";
        return false;
    }

    // 连接通知关闭信号
    bool connected = bus.connect("org.freedesktop.Notifications",
                                  "/org/freedesktop/Notifications",
                                  "org.freedesktop.Notifications",
                                  "NotificationClosed",
                                  this,
                                  SLOT(onDBusNotificationClosed(uint, uint)));

    if (!connected) {
        qWarning() << "Notification: Failed to connect NotificationClosed signal";
    }

    // 连接通知动作信号
    connected = bus.connect("org.freedesktop.Notifications",
                            "/org/freedesktop/Notifications",
                            "org.freedesktop.Notifications",
                            "ActionInvoked",
                            this,
                            SLOT(onDBusActionInvoked(uint, QString)));

    return true;
}

/**
 * @brief 通过 DBus 发送通知
 */
bool Notification::sendViaDBus(const NotificationData& data)
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected()) {
        return false;
    }

    QDBusInterface iface("org.freedesktop.Notifications",
                         "/org/freedesktop/Notifications",
                         "org.freedesktop.Notifications",
                         bus);

    if (!iface.isValid()) {
        qWarning() << "Notification: Invalid DBus interface";
        return false;
    }

    // 准备参数
    // Notify(app_name, replaces_id, app_icon, summary, body, actions, hints, expire_timeout)
    QVariantList args;

    args << d->appName;                    // app_name
    args << d->dbusSerial;                 // replaces_id (0 for new)
    args << data.iconPath;                 // app_icon
    args << data.title;                    // summary
    args << data.body;                     // body
    args << QStringList();                 // actions
    args << QVariantMap{                   // hints
        {"urgency", static_cast<int>(data.priority)},
        {"desktop-entry", "qnote"}
    };
    args << data.timeout;                  // expire_timeout (-1 for default)

    QDBusReply<uint> reply = iface.call("Notify", args);

    if (reply.isValid()) {
        d->dbusSerial = reply.value();
        qDebug() << "Notification: DBus notification sent, serial:" << d->dbusSerial;
        return true;
    }

    qWarning() << "Notification: DBus error:" << reply.error().message();
    return false;
}

#endif // Q_OS_LINUX

/**
 * @brief 通过托盘发送通知（回退方案）
 */
bool Notification::sendViaTray(const NotificationData& data)
{
    if (!TrayIcon::isSystemTrayAvailable()) {
        return false;
    }

    TrayIcon* tray = TrayIcon::instance();
    if (!tray->isVisible()) {
        return false;
    }

    // 转换优先级为图标类型
    QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information;
    switch (data.priority) {
    case NotificationPriority::High:
        icon = QSystemTrayIcon::Warning;
        break;
    case NotificationPriority::Low:
        icon = QSystemTrayIcon::NoIcon;
        break;
    default:
        icon = QSystemTrayIcon::Information;
    }

    int timeout = data.timeout > 0 ? data.timeout : 10000;
    tray->showMessage(data.title, data.body, icon, timeout);

    qDebug() << "Notification: Sent via tray";
    return true;
}

#ifdef Q_OS_LINUX

/**
 * @brief 处理 DBus 通知关闭信号
 */
void Notification::onDBusNotificationClosed(uint id, uint reason)
{
    Q_UNUSED(reason)
    qDebug() << "Notification: DBus notification closed:" << id;
    emit closed(QString::number(id));
}

/**
 * @brief 处理 DBus 动作触发信号
 */
void Notification::onDBusActionInvoked(uint id, const QString& action)
{
    qDebug() << "Notification: DBus action invoked:" << id << action;
    emit actionTriggered(QString::number(id), action);
    emit clicked(QString::number(id));
}

#endif // Q_OS_LINUX

} // namespace qnote
