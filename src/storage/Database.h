#ifndef DATABASE_H
#define DATABASE_H

#include <QString>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <memory>

namespace qnote {

/**
 * @brief 数据库管理类
 *
 * 单例模式，负责 SQLite 数据库的创建、连接和基础操作。
 * 使用 Qt SQL 模块进行数据库访问。
 */
class Database {
public:
    /**
     * @brief 获取单例实例
     * @return Database 实例指针
     */
    static Database* instance();

    /**
     * @brief 初始化数据库
     * @param dbPath 数据库文件路径，默认为应用数据目录下的 qnote.db
     * @return 成功返回 true
     */
    bool init(const QString& dbPath = QString());

    /**
     * @brief 检查数据库是否已初始化
     * @return 已初始化返回 true
     */
    bool isInitialized() const;

    /**
     * @brief 获取数据库连接
     * @return QSqlDatabase 引用
     */
    QSqlDatabase database() const;

    /**
     * @brief 执行 SQL 查询
     * @param sql SQL 语句
     * @return 成功返回 true
     */
    bool execute(const QString& sql);

    /**
     * @brief 执行预编译查询
     * @param query 已准备的 QSqlQuery 对象
     * @return 成功返回 true
     */
    bool execute(QSqlQuery& query);

    /**
     * @brief 开始事务
     * @return 成功返回 true
     */
    bool beginTransaction();

    /**
     * @brief 提交事务
     * @return 成功返回 true
     */
    bool commit();

    /**
     * @brief 回滚事务
     * @return 成功返回 true
     */
    bool rollback();

    /**
     * @brief 获取最后的错误信息
     * @return 错误信息字符串
     */
    QString lastError() const;

    /**
     * @brief 获取数据库文件路径
     * @return 数据库文件路径
     */
    QString databasePath() const;

    /**
     * @brief 检查数据库文件是否存在
     * @return 存在返回 true
     */
    bool databaseExists() const;

    /**
     * @brief 关闭数据库连接
     */
    void close();

    /**
     * @brief 备份数据库
     * @param backupPath 备份文件路径
     * @return 成功返回 true
     */
    bool backup(const QString& backupPath);

    /**
     * @brief 获取数据库版本
     * @return 版本号，未初始化返回 0
     */
    int version() const;

    /**
     * @brief 设置数据库版本
     * @param version 版本号
     * @return 成功返回 true
     */
    bool setVersion(int version);

private:
    Database();
    ~Database();
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    /**
     * @brief 创建数据库元数据表
     * @return 成功返回 true
     */
    bool createMetaTable();

    /**
     * @brief 验证数据库完整性
     * @return 完整返回 true
     */
    bool validateDatabase();

private:
    static std::unique_ptr<Database> s_instance;

    bool m_initialized;
    QString m_dbPath;
    QString m_connectionName;
    QString m_lastError;
    int m_version;

    static const QString DB_FILE_NAME;
    static const QString CONNECTION_NAME;
};

} // namespace qnote

#endif // DATABASE_H
