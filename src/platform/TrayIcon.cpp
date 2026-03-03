/**
 * @file TrayIcon.cpp
 * @brief 系统托盘图标管理实现
 * @author QNote Team
 * @date 2026-03-04
 */

#include "TrayIcon.h"
#include "core/Settings.h"

#include <QApplication>
#include <QAction>
#include <QStyle>
#include <QFile>
#include <QDebug>

namespace qnote {

// 静态成员初始化
TrayIcon* TrayIcon::s_instance = nullptr;

/**
 * @brief 获取单例实例
 */
TrayIcon* TrayIcon::instance()
{
    if (!s_instance) {
        s_instance = new TrayIcon();
    }
    return s_instance;
}

/**
 * @brief 构造函数
 */
TrayIcon::TrayIcon(QObject* parent)
    : QObject(parent)
    , m_trayIcon(std::make_unique<QSystemTrayIcon>())
    , m_menu(std::make_unique<QMenu>())
    , m_showAction(nullptr)
    , m_newNoteAction(nullptr)
    , m_quitAction(nullptr)
    , m_noteCount(0)
    , m_initialized(false)
{
}

/**
 * @brief 析构函数
 */
TrayIcon::~TrayIcon()
{
    if (m_trayIcon) {
        m_trayIcon->hide();
    }
}

/**
 * @brief 检查系统是否支持托盘
 */
bool TrayIcon::isSystemTrayAvailable()
{
    return QSystemTrayIcon::isSystemTrayAvailable();
}

/**
 * @brief 初始化托盘图标
 */
bool TrayIcon::initialize()
{
    if (m_initialized) {
        return true;
    }

    // 检查系统托盘是否可用
    if (!isSystemTrayAvailable()) {
        qWarning() << "TrayIcon: System tray is not available";
        return false;
    }

    // 加载图标
    loadIcon();

    // 创建菜单
    createMenu();

    // 连接信号
    connectSignals();

    // 加载设置
    loadSettings();

    m_initialized = true;
    qDebug() << "TrayIcon: Initialized successfully";
    return true;
}

/**
 * @brief 显示托盘图标
 */
void TrayIcon::show()
{
    if (!m_initialized) {
        qWarning() << "TrayIcon: Not initialized, call initialize() first";
        return;
    }

    m_trayIcon->show();
    qDebug() << "TrayIcon: Shown";
}

/**
 * @brief 隐藏托盘图标
 */
void TrayIcon::hide()
{
    if (m_trayIcon) {
        m_trayIcon->hide();
        qDebug() << "TrayIcon: Hidden";
    }
}

/**
 * @brief 检查托盘图标是否可见
 */
bool TrayIcon::isVisible() const
{
    return m_trayIcon && m_trayIcon->isVisible();
}

/**
 * @brief 设置工具提示
 */
void TrayIcon::setToolTip(const QString& tip)
{
    if (m_trayIcon) {
        m_trayIcon->setToolTip(tip);
    }
}

/**
 * @brief 设置托盘图标
 */
void TrayIcon::setIcon(const QIcon& icon)
{
    if (m_trayIcon) {
        m_trayIcon->setIcon(icon);
    }
}

/**
 * @brief 显示消息通知
 */
void TrayIcon::showMessage(const QString& title,
                           const QString& message,
                           QSystemTrayIcon::MessageIcon icon,
                           int millisecondsTimeoutHint)
{
    if (m_trayIcon && m_trayIcon->isVisible()) {
        m_trayIcon->showMessage(title, message, icon, millisecondsTimeoutHint);
    }
}

/**
 * @brief 更新便签数量显示
 */
void TrayIcon::updateNoteCount(int count)
{
    m_noteCount = count;

    // 更新工具提示
    QString tip = tr("QNote - %n note(s)", "", count);
    setToolTip(tip);

    // 更新菜单项状态
    if (m_showAction) {
        m_showAction->setText(tr("Show Window (%1 notes)").arg(count));
    }
}

/**
 * @brief 创建托盘菜单
 */
void TrayIcon::createMenu()
{
    // 显示/隐藏窗口
    m_showAction = m_menu->addAction(tr("Show Window"));
    m_showAction->setIcon(qApp->style()->standardIcon(QStyle::SP_ComputerIcon));
    connect(m_showAction, &QAction::triggered, this, &TrayIcon::onShowWindow);

    m_menu->addSeparator();

    // 新建便签
    m_newNoteAction = m_menu->addAction(tr("New Note"));
    m_newNoteAction->setIcon(qApp->style()->standardIcon(QStyle::SP_FileDialogNewFolder));
    connect(m_newNoteAction, &QAction::triggered, this, &TrayIcon::onNewNote);

    m_menu->addSeparator();

    // 退出
    m_quitAction = m_menu->addAction(tr("Quit"));
    m_quitAction->setIcon(qApp->style()->standardIcon(QStyle::SP_DialogCloseButton));
    connect(m_quitAction, &QAction::triggered, this, &TrayIcon::onQuit);

    // 设置托盘菜单
    m_trayIcon->setContextMenu(m_menu.get());
}

/**
 * @brief 加载托盘图标
 */
void TrayIcon::loadIcon()
{
    QIcon icon;

    // 尝试从资源加载
    if (QFile::exists(":/icons/app.svg")) {
        icon = QIcon(":/icons/app.svg");
    }

    // 如果没有图标，使用系统默认图标
    if (icon.isNull()) {
        icon = qApp->style()->standardIcon(QStyle::SP_MessageBoxInformation);
    }

    m_trayIcon->setIcon(icon);

    // 设置默认工具提示
    setToolTip(tr("QNote - Quick Note Taking"));
}

/**
 * @brief 连接信号槽
 */
void TrayIcon::connectSignals()
{
    // 托盘图标激活
    connect(m_trayIcon.get(), &QSystemTrayIcon::activated,
            this, &TrayIcon::onActivated);

    // 消息点击
    connect(m_trayIcon.get(), &QSystemTrayIcon::messageClicked,
            this, &TrayIcon::messageClicked);
}

/**
 * @brief 从配置加载设置
 */
void TrayIcon::loadSettings()
{
    // 可以从 Settings 加载托盘相关配置
    // 例如是否显示托盘、默认行为等
}

/**
 * @brief 处理托盘激活事件
 */
void TrayIcon::onActivated(QSystemTrayIcon::ActivationReason reason)
{
    qDebug() << "TrayIcon: Activated with reason:" << reason;

    // 转发信号
    emit activated(reason);

    // 根据激活原因执行不同操作
    switch (reason) {
    case QSystemTrayIcon::Trigger:
        // 单击 - 切换窗口显示状态
        emit showWindowRequested();
        break;

    case QSystemTrayIcon::DoubleClick:
        // 双击 - 显示窗口
        emit showWindowRequested();
        break;

    case QSystemTrayIcon::MiddleClick:
        // 中键点击 - 新建便签
        emit newNoteRequested();
        break;

    default:
        break;
    }
}

/**
 * @brief 处理显示窗口菜单项
 */
void TrayIcon::onShowWindow()
{
    emit showWindowRequested();
}

/**
 * @brief 处理新建便签菜单项
 */
void TrayIcon::onNewNote()
{
    emit newNoteRequested();
}

/**
 * @brief 处理退出菜单项
 */
void TrayIcon::onQuit()
{
    emit quitRequested();
}

} // namespace qnote
