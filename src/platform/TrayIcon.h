/**
 * @file TrayIcon.h
 * @brief 系统托盘图标管理
 * @author QNote Team
 * @date 2026-03-04
 *
 * 提供系统托盘图标功能，支持最小化到托盘、托盘菜单等
 */

#ifndef TRAYICON_H
#define TRAYICON_H

#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>
#include <memory>

namespace qnote {

// 前向声明
class Settings;

/**
 * @class TrayIcon
 * @brief 系统托盘图标管理类
 *
 * 封装 QSystemTrayIcon，提供：
 * - 托盘图标显示
 * - 托盘菜单（显示/隐藏窗口、新建便签、退出等）
 * - 最小化到托盘
 * - 关闭到托盘
 * - 托盘点击事件
 */
class TrayIcon : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 获取单例实例
     * @return TrayIcon 实例指针
     */
    static TrayIcon* instance();

    /**
     * @brief 析构函数
     */
    ~TrayIcon() override;

    /**
     * @brief 初始化托盘图标
     * @return 初始化是否成功
     *
     * 会创建托盘图标和菜单，但不会立即显示
     */
    bool initialize();

    /**
     * @brief 显示托盘图标
     */
    void show();

    /**
     * @brief 隐藏托盘图标
     */
    void hide();

    /**
     * @brief 检查托盘图标是否可见
     * @return 是否可见
     */
    bool isVisible() const;

    /**
     * @brief 检查系统是否支持托盘
     * @return 是否支持托盘
     */
    static bool isSystemTrayAvailable();

    /**
     * @brief 设置工具提示
     * @param tip 提示文本
     */
    void setToolTip(const QString& tip);

    /**
     * @brief 设置托盘图标
     * @param icon 图标
     */
    void setIcon(const QIcon& icon);

    /**
     * @brief 显示消息通知
     * @param title 标题
     * @param message 消息内容
     * @param icon 图标类型
     * @param millisecondsTimeoutHint 显示时长（毫秒）
     */
    void showMessage(const QString& title,
                     const QString& message,
                     QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information,
                     int millisecondsTimeoutHint = 10000);

    /**
     * @brief 更新便签数量显示
     * @param count 便签数量
     */
    void updateNoteCount(int count);

signals:
    /**
     * @brief 显示主窗口请求
     */
    void showWindowRequested();

    /**
     * @brief 隐藏主窗口请求
     */
    void hideWindowRequested();

    /**
     * @brief 新建便签请求
     */
    void newNoteRequested();

    /**
     * @brief 退出应用请求
     */
    void quitRequested();

    /**
     * @brief 托盘图标被点击
     * @param reason 点击原因
     */
    void activated(QSystemTrayIcon::ActivationReason reason);

    /**
     * @brief 托盘消息被点击
     */
    void messageClicked();

private slots:
    /**
     * @brief 处理托盘激活事件
     * @param reason 激活原因
     */
    void onActivated(QSystemTrayIcon::ActivationReason reason);

    /**
     * @brief 处理显示窗口菜单项
     */
    void onShowWindow();

    /**
     * @brief 处理新建便签菜单项
     */
    void onNewNote();

    /**
     * @brief 处理退出菜单项
     */
    void onQuit();

private:
    /**
     * @brief 私有构造函数（单例模式）
     * @param parent 父对象
     */
    explicit TrayIcon(QObject* parent = nullptr);

    // 禁止拷贝
    TrayIcon(const TrayIcon&) = delete;
    TrayIcon& operator=(const TrayIcon&) = delete;

    /**
     * @brief 创建托盘菜单
     */
    void createMenu();

    /**
     * @brief 加载托盘图标
     */
    void loadIcon();

    /**
     * @brief 连接信号槽
     */
    void connectSignals();

    /**
     * @brief 从配置加载设置
     */
    void loadSettings();

private:
    static TrayIcon* s_instance;            ///< 单例实例

    std::unique_ptr<QSystemTrayIcon> m_trayIcon;  ///< 托盘图标
    std::unique_ptr<QMenu> m_menu;                ///< 托盘菜单

    // 菜单项
    QAction* m_showAction;        ///< 显示窗口
    QAction* m_newNoteAction;     ///< 新建便签
    QAction* m_quitAction;        ///< 退出应用

    int m_noteCount;              ///< 便签数量
    bool m_initialized;           ///< 是否已初始化
};

} // namespace qnote

#endif // TRAYICON_H
