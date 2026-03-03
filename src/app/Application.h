#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#include <QSharedPointer>
#include <memory>

namespace qnote {

// 前向声明
class Logger;
class Settings;

/**
 * @brief 应用程序主类
 * 
 * 负责应用程序的初始化、启动和清理
 */
class Application : public QApplication {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param argc 参数个数
     * @param argv 参数数组
     */
    Application(int& argc, char** argv);
    
    /**
     * @brief 析构函数
     */
    ~Application() override;
    
    /**
     * @brief 获取单例实例
     */
    static Application* instance();
    
    /**
     * @brief 初始化应用程序
     * @return 初始化是否成功
     */
    bool initialize();
    
    /**
     * @brief 获取应用版本
     */
    QString version() const;
    
    /**
     * @brief 获取应用名称
     */
    QString name() const;
    
    /**
     * @brief 获取应用组织
     */
    QString organization() const;
    
    /**
     * @brief 获取数据目录路径
     */
    QString dataPath() const;
    
    /**
     * @brief 获取配置目录路径
     */
    QString configPath() const;
    
    /**
     * @brief 获取日志目录路径
     */
    QString logPath() const;
    
    /**
     * @brief 检查是否是首次运行
     */
    bool isFirstRun() const;
    
    /**
     * @brief 设置首次运行标记
     */
    void setFirstRunComplete();

signals:
    /**
     * @brief 应用即将退出
     */
    void aboutToQuit();
    
    /**
     * @brief 主题改变
     */
    void themeChanged(const QString& theme);

protected:
    /**
     * @brief 事件过滤器
     */
    bool event(QEvent* event) override;

private:
    /**
     * @brief 初始化路径
     */
    void initPaths();
    
    /**
     * @brief 初始化日志
     */
    void initLogger();
    
    /**
     * @brief 初始化配置
     */
    void initSettings();
    
    /**
     * @brief 初始化数据库
     */
    bool initDatabase();
    
    /**
     * @brief 加载样式表
     */
    void loadStyleSheet();
    
    /**
     * @brief 检查单实例
     */
    bool checkSingleInstance();

private:
    static Application* s_instance;
    
    struct Private;
    std::unique_ptr<Private> d;
};

} // namespace qnote

#endif // APPLICATION_H
