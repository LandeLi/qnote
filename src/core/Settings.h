#ifndef SETTINGS_H
#define SETTINGS_H

#include <QString>
#include <QVariant>
#include <QSettings>
#include <memory>

namespace qnote {

/**
 * @brief 应用配置管理
 * 
 * 封装 QSettings，提供类型安全的配置访问
 */
class Settings {
public:
    static Settings* instance();
    
    /**
     * @brief 初始化配置
     * @param organization 组织名
     * @param application 应用名
     */
    void init(const QString& organization = "QNote",
              const QString& application = "QNote");
    
    /**
     * @brief 使用自定义配置文件
     * @param filePath 配置文件路径
     */
    void initFromFile(const QString& filePath);
    
    // 通用方法
    QVariant value(const QString& key, const QVariant& defaultValue = QVariant()) const;
    void setValue(const QString& key, const QVariant& value);
    bool contains(const QString& key) const;
    void remove(const QString& key);
    void clear();
    
    // 便捷方法 - 常用配置
    
    // 窗口
    QByteArray windowGeometry() const;
    void setWindowGeometry(const QByteArray& geometry);
    QByteArray windowState() const;
    void setWindowState(const QByteArray& state);
    
    // 外观
    QString theme() const;
    void setTheme(const QString& theme);
    QString fontFamily() const;
    void setFontFamily(const QString& family);
    int fontSize() const;
    void setFontSize(int size);
    
    // 编辑器
    bool autoSave() const;
    void setAutoSave(bool enabled);
    int autoSaveInterval() const;  // 秒
    void setAutoSaveInterval(int seconds);
    bool spellCheck() const;
    void setSpellCheck(bool enabled);
    
    // 系统
    bool minimizeToTray() const;
    void setMinimizeToTray(bool enabled);
    bool closeToTray() const;
    void setCloseToTray(bool enabled);
    bool startWithSystem() const;
    void setStartWithSystem(bool enabled);
    
    // 数据
    QString dataPath() const;
    void setDataPath(const QString& path);
    int maxBackups() const;
    void setMaxBackups(int count);
    
    // 同步
    bool syncEnabled() const;
    void setSyncEnabled(bool enabled);
    QString syncServer() const;
    void setSyncServer(const QString& server);
    
    /**
     * @brief 获取配置文件路径
     */
    QString filePath() const;
    
    /**
     * @brief 同步配置到磁盘
     */
    void sync();

private:
    Settings();
    ~Settings();
    Settings(const Settings&) = delete;
    Settings& operator=(const Settings&) = delete;
    
private:
    static std::unique_ptr<Settings> s_instance;
    std::unique_ptr<QSettings> m_settings;
    QString m_filePath;
};

} // namespace qnote

#endif // SETTINGS_H
