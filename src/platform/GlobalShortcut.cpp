/**
 * @file GlobalShortcut.cpp
 * @brief 全局快捷键管理实现
 * @author QNote Team
 * @date 2026-03-04
 */

#include "GlobalShortcut.h"
#include "core/Settings.h"

#include <QApplication>
#include <QDebug>
#include <QSettings>

#ifdef Q_OS_LINUX
#include <QX11Info>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <xcb/xcb.h>
#endif

namespace qnote {

// 静态成员初始化
GlobalShortcut* GlobalShortcut::s_instance = nullptr;

/**
 * @brief 获取单例实例
 */
GlobalShortcut* GlobalShortcut::instance()
{
    if (!s_instance) {
        s_instance = new GlobalShortcut();
    }
    return s_instance;
}

/**
 * @brief 构造函数
 */
GlobalShortcut::GlobalShortcut(QObject* parent)
    : QObject(parent)
    , m_initialized(false)
{
#ifdef Q_OS_LINUX
    // 安装原生事件过滤器
    qApp->installNativeEventFilter(this);
#endif
}

/**
 * @brief 析构函数
 */
GlobalShortcut::~GlobalShortcut()
{
    // 注销所有快捷键
    unregisterAll();

    // 清理平台资源
    cleanupNative();

#ifdef Q_OS_LINUX
    // 移除事件过滤器
    qApp->removeNativeEventFilter(this);
#endif
}

/**
 * @brief 获取默认快捷键列表
 */
QHash<QString, ShortcutInfo> GlobalShortcut::defaultShortcuts()
{
    QHash<QString, ShortcutInfo> defaults;

    {
        ShortcutInfo info;
        info.id = "new_note";
        info.keySequence = QKeySequence("Ctrl+Alt+N");
        info.description = tr("Create New Note");
        info.enabled = true;
        defaults[info.id] = info;
    }

    {
        ShortcutInfo info;
        info.id = "toggle_window";
        info.keySequence = QKeySequence("Ctrl+Alt+Q");
        info.description = tr("Toggle Main Window");
        info.enabled = true;
        defaults[info.id] = info;
    }

    {
        ShortcutInfo info;
        info.id = "quick_note";
        info.keySequence = QKeySequence("Ctrl+Shift+N");
        info.description = tr("Quick Note");
        info.enabled = true;
        defaults[info.id] = info;
    }

    {
        ShortcutInfo info;
        info.id = "search";
        info.keySequence = QKeySequence("Ctrl+Alt+F");
        info.description = tr("Quick Search");
        info.enabled = true;
        defaults[info.id] = info;
    }

    return defaults;
}

/**
 * @brief 初始化全局快捷键管理器
 */
bool GlobalShortcut::initialize()
{
    if (m_initialized) {
        return true;
    }

    // 初始化平台资源
    if (!initNative()) {
        qWarning() << "GlobalShortcut: Failed to initialize native backend";
        return false;
    }

    // 从配置加载快捷键
    loadFromSettings();

    m_initialized = true;
    qDebug() << "GlobalShortcut: Initialized successfully";
    return true;
}

/**
 * @brief 注册快捷键
 */
bool GlobalShortcut::registerShortcut(const QString& id,
                                      const QKeySequence& keySequence,
                                      const QString& description)
{
    if (id.isEmpty() || keySequence.isEmpty()) {
        qWarning() << "GlobalShortcut: Invalid shortcut parameters";
        return false;
    }

    // 检查是否已注册
    if (m_shortcuts.contains(id)) {
        qWarning() << "GlobalShortcut: Shortcut already registered:" << id;
        return false;
    }

    // 检查冲突
    QString conflict = checkConflict(keySequence);
    if (!conflict.isEmpty()) {
        qWarning() << "GlobalShortcut: Key sequence conflict:" << conflict;
        emit conflictDetected(keySequence, conflict);
        return false;
    }

    // 注册原生快捷键
    if (!registerNativeShortcut(id, keySequence)) {
        qWarning() << "GlobalShortcut: Failed to register native shortcut:" << id;
        return false;
    }

    // 保存快捷键信息
    ShortcutInfo info;
    info.id = id;
    info.keySequence = keySequence;
    info.description = description;
    info.enabled = true;
    m_shortcuts[id] = info;

    qDebug() << "GlobalShortcut: Registered:" << id << "->" << keySequence.toString();
    emit registered(id);
    return true;
}

/**
 * @brief 注销快捷键
 */
bool GlobalShortcut::unregisterShortcut(const QString& id)
{
    if (!m_shortcuts.contains(id)) {
        return false;
    }

    // 注销原生快捷键
    if (!unregisterNativeShortcut(id)) {
        qWarning() << "GlobalShortcut: Failed to unregister native shortcut:" << id;
        return false;
    }

    m_shortcuts.remove(id);
    m_nativeHandles.remove(id);

    qDebug() << "GlobalShortcut: Unregistered:" << id;
    emit unregistered(id);
    return true;
}

/**
 * @brief 注销所有快捷键
 */
void GlobalShortcut::unregisterAll()
{
    // 复制键列表，避免在遍历时修改
    QStringList ids = m_shortcuts.keys();
    for (const QString& id : ids) {
        unregisterShortcut(id);
    }
}

/**
 * @brief 检查快捷键是否已注册
 */
bool GlobalShortcut::isRegistered(const QString& id) const
{
    return m_shortcuts.contains(id);
}

/**
 * @brief 启用/禁用快捷键
 */
void GlobalShortcut::setEnabled(const QString& id, bool enabled)
{
    if (!m_shortcuts.contains(id)) {
        return;
    }

    m_shortcuts[id].enabled = enabled;
    qDebug() << "GlobalShortcut:" << id << (enabled ? "enabled" : "disabled");
}

/**
 * @brief 检查快捷键是否启用
 */
bool GlobalShortcut::isEnabled(const QString& id) const
{
    if (!m_shortcuts.contains(id)) {
        return false;
    }
    return m_shortcuts[id].enabled;
}

/**
 * @brief 更新快捷键绑定
 */
bool GlobalShortcut::updateShortcut(const QString& id, const QKeySequence& newKeySequence)
{
    if (!m_shortcuts.contains(id)) {
        return false;
    }

    // 检查冲突（排除自身）
    QString conflict = checkConflict(newKeySequence, id);
    if (!conflict.isEmpty()) {
        qWarning() << "GlobalShortcut: Key sequence conflict:" << conflict;
        emit conflictDetected(newKeySequence, conflict);
        return false;
    }

    // 先注销旧的
    unregisterNativeShortcut(id);

    // 注册新的
    if (!registerNativeShortcut(id, newKeySequence)) {
        // 恢复旧的
        registerNativeShortcut(id, m_shortcuts[id].keySequence);
        return false;
    }

    // 更新信息
    m_shortcuts[id].keySequence = newKeySequence;

    qDebug() << "GlobalShortcut: Updated:" << id << "->" << newKeySequence.toString();
    return true;
}

/**
 * @brief 获取快捷键信息
 */
ShortcutInfo GlobalShortcut::shortcutInfo(const QString& id) const
{
    return m_shortcuts.value(id);
}

/**
 * @brief 获取所有已注册的快捷键
 */
QList<ShortcutInfo> GlobalShortcut::allShortcuts() const
{
    return m_shortcuts.values();
}

/**
 * @brief 检查按键序列是否冲突
 */
QString GlobalShortcut::checkConflict(const QKeySequence& keySequence,
                                      const QString& excludeId) const
{
    for (auto it = m_shortcuts.constBegin(); it != m_shortcuts.constEnd(); ++it) {
        if (it.key() != excludeId && it.value().keySequence == keySequence) {
            return it.key();
        }
    }
    return QString();
}

/**
 * @brief 从配置加载快捷键
 */
void GlobalShortcut::loadFromSettings()
{
    // 获取默认快捷键
    auto defaults = defaultShortcuts();

    // 从 Settings 加载用户配置
    Settings* settings = Settings::instance();

    for (auto it = defaults.constBegin(); it != defaults.constEnd(); ++it) {
        const ShortcutInfo& defaultInfo = it.value();

        // 尝试从配置加载按键序列
        QString keyStr = settings->value(
            QString("shortcuts/%1").arg(it.key()),
            defaultInfo.keySequence.toString()
        ).toString();

        QKeySequence keySeq(keyStr);
        if (keySeq.isEmpty()) {
            keySeq = defaultInfo.keySequence;
        }

        // 注册快捷键
        registerShortcut(it.key(), keySeq, defaultInfo.description);
    }
}

/**
 * @brief 保存快捷键到配置
 */
void GlobalShortcut::saveToSettings()
{
    Settings* settings = Settings::instance();

    for (auto it = m_shortcuts.constBegin(); it != m_shortcuts.constEnd(); ++it) {
        settings->setValue(
            QString("shortcuts/%1").arg(it.key()),
            it.value().keySequence.toString()
        );
    }

    settings->sync();
}

/**
 * @brief 恢复默认快捷键
 */
void GlobalShortcut::restoreDefaults()
{
    // 注销所有
    unregisterAll();

    // 重新注册默认值
    auto defaults = defaultShortcuts();
    for (auto it = defaults.constBegin(); it != defaults.constEnd(); ++it) {
        const ShortcutInfo& info = it.value();
        registerShortcut(info.id, info.keySequence, info.description);
    }

    // 保存到配置
    saveToSettings();

    qDebug() << "GlobalShortcut: Restored default shortcuts";
}

// ==================== 平台特定实现 ====================

#ifdef Q_OS_LINUX

/**
 * @brief 初始化 X11 资源
 */
bool GlobalShortcut::initNative()
{
    Display* display = QX11Info::display();
    if (!display) {
        qWarning() << "GlobalShortcut: X11 display not available";
        return false;
    }

    // 创建一个不可见的窗口用于接收快捷键事件
    return true;
}

/**
 * @brief 清理 X11 资源
 */
void GlobalShortcut::cleanupNative()
{
    // X11 资源会自动清理
}

/**
 * @brief 注册 X11 快捷键
 */
bool GlobalShortcut::registerNativeShortcut(const QString& id, const QKeySequence& keySequence)
{
    Display* display = QX11Info::display();
    if (!display) {
        return false;
    }

    Window window = QX11Info::appRootWindow();

    // 解析按键序列
    if (keySequence.isEmpty()) {
        return false;
    }

    // 获取第一个键
    uint key = keySequence[0];

    // 提取修饰键和键码
    int modifiers = 0;
    int keyCode = 0;

    // Qt 键转换为 X11 键码
    Qt::Key qtKey = static_cast<Qt::Key>(key & ~Qt::KeyboardModifierMask);

    // 修饰键
    if (key & Qt::CTRL)  modifiers |= ControlMask;
    if (key & Qt::ALT)   modifiers |= Mod1Mask;
    if (key & Qt::SHIFT) modifiers |= ShiftMask;

    // 转换键码
    KeySym keysym = XStringToKeysym(QKeySequence(qtKey).toString().toLatin1().data());
    if (keysym == NoSymbol) {
        // 尝试直接映射
        keyCode = XKeysymToKeycode(display, qtKey);
    } else {
        keyCode = XKeysymToKeycode(display, keysym);
    }

    if (keyCode == 0) {
        qWarning() << "GlobalShortcut: Failed to convert key to X11 keycode";
        return false;
    }

    // 捕获按键
    XGrabKey(display, keyCode, modifiers, window, True, GrabModeAsync, GrabModeAsync);

    // 同时捕获 NumLock 状态
    XGrabKey(display, keyCode, modifiers | Mod2Mask, window, True, GrabModeAsync, GrabModeAsync);

    // 保存句柄（使用 keyCode 作为句柄）
    m_nativeHandles[id] = reinterpret_cast<void*>(static_cast<intptr_t>(keyCode));

    qDebug() << "GlobalShortcut: X11 registered key code:" << keyCode;
    return true;
}

/**
 * @brief 注销 X11 快捷键
 */
bool GlobalShortcut::unregisterNativeShortcut(const QString& id)
{
    Display* display = QX11Info::display();
    if (!display) {
        return false;
    }

    if (!m_nativeHandles.contains(id)) {
        return true;
    }

    Window window = QX11Info::appRootWindow();

    // 获取键码
    int keyCode = static_cast<int>(reinterpret_cast<intptr_t>(m_nativeHandles[id]));

    // 释放按键
    XUngrabKey(display, keyCode, AnyModifier, window);

    XFlush(display);

    return true;
}

/**
 * @brief 处理 X11 事件
 */
bool GlobalShortcut::nativeEventFilter(const QByteArray& eventType, void* message, long* result)
{
    Q_UNUSED(result)

    if (eventType != "xcb_generic_event_t") {
        return false;
    }

    xcb_generic_event_t* event = static_cast<xcb_generic_event_t*>(message);
    if (!event) {
        return false;
    }

    // 检查是否是按键事件
    if (event->response_type != XCB_KEY_PRESS && event->response_type != XCB_KEY_RELEASE) {
        return false;
    }

    xcb_key_press_event_t* keyEvent = reinterpret_cast<xcb_key_press_event_t*>(event);

    // 只处理按键按下事件
    if (event->response_type != XCB_KEY_PRESS) {
        return false;
    }

    // 查找匹配的快捷键
    for (auto it = m_shortcuts.constBegin(); it != m_shortcuts.constEnd(); ++it) {
        const ShortcutInfo& info = it.value();
        if (!info.enabled) {
            continue;
        }

        int keyCode = static_cast<int>(reinterpret_cast<intptr_t>(m_nativeHandles.value(it.key())));
        if (keyCode == keyEvent->detail) {
            qDebug() << "GlobalShortcut: Triggered:" << it.key();
            emit triggered(it.key());
            return true;
        }
    }

    return false;
}

#else // 其他平台

/**
 * @brief 初始化（非 Linux 平台）
 */
bool GlobalShortcut::initNative()
{
    // TODO: 实现 Windows/macOS 支持
    qWarning() << "GlobalShortcut: Platform not fully supported";
    return true;
}

/**
 * @brief 清理（非 Linux 平台）
 */
void GlobalShortcut::cleanupNative()
{
}

/**
 * @brief 注册（非 Linux 平台）
 */
bool GlobalShortcut::registerNativeShortcut(const QString& id, const QKeySequence& keySequence)
{
    Q_UNUSED(id)
    Q_UNUSED(keySequence)
    // TODO: 实现平台特定代码
    return true;
}

/**
 * @brief 注销（非 Linux 平台）
 */
bool GlobalShortcut::unregisterNativeShortcut(const QString& id)
{
    Q_UNUSED(id)
    return true;
}

#endif

} // namespace qnote
