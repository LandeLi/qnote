#ifndef MIGRATION_H
#define MIGRATION_H

#include <QString>
#include <QStringList>
#include <QList>
#include <functional>
#include <memory>

namespace qnote {

/**
 * @brief 数据库迁移信息
 */
struct MigrationInfo {
    int version;            ///< 目标版本号
    QString description;    ///< 迁移描述
    QStringList upSql;      ///< 升级 SQL 语句列表
    QStringList downSql;    ///< 降级 SQL 语句列表（可选）
};

/**
 * @brief 数据库迁移管理器
 *
 * 负责数据库 schema 的版本管理和迁移。
 * 采用版本号递增的方式管理迁移历史。
 */
class Migration {
public:
    /**
     * @brief 迁移函数类型
     * @param db 当前数据库版本
     * @return 成功返回新版本号，失败返回 -1
     */
    using MigrationFunc = std::function<int(int currentVersion)>;

    /**
     * @brief 获取单例实例
     */
    static Migration* instance();

    /**
     * @brief 初始化迁移管理器
     * @return 成功返回 true
     */
    bool init();

    /**
     * @brief 获取当前数据库版本
     * @return 版本号
     */
    int currentVersion() const;

    /**
     * @brief 获取最新版本号
     * @return 最新版本号
     */
    int latestVersion() const;

    /**
     * @brief 执行所有待执行的迁移
     * @return 成功返回 true
     */
    bool migrateAll();

    /**
     * @brief 迁移到指定版本
     * @param targetVersion 目标版本号
     * @return 成功返回 true
     */
    bool migrateTo(int targetVersion);

    /**
     * @brief 检查是否有待执行的迁移
     * @return 有待执行迁移返回 true
     */
    bool hasPendingMigrations() const;

    /**
     * @brief 获取待执行的迁移列表
     * @return 迁移描述列表
     */
    QStringList pendingMigrations() const;

    /**
     * @brief 获取最后的错误信息
     * @return 错误信息
     */
    QString lastError() const;

    /**
     * @brief 重置数据库（删除所有表并重新迁移）
     * @return 成功返回 true
     */
    bool reset();

private:
    Migration();
    ~Migration();
    Migration(const Migration&) = delete;
    Migration& operator=(const Migration&) = delete;

    /**
     * @brief 注册所有迁移
     */
    void registerMigrations();

    /**
     * @brief 注册单个迁移
     * @param info 迁移信息
     */
    void registerMigration(const MigrationInfo& info);

    /**
     * @brief 执行单个迁移
     * @param version 目标版本
     * @return 成功返回 true
     */
    bool executeMigration(int version);

    /**
     * @brief 创建笔记表
     * @return SQL 语句列表
     */
    QStringList createNotesTable();

    /**
     * @brief 创建标签表
     * @return SQL 语句列表
     */
    QStringList createTagsTable();

    /**
     * @brief 创建笔记-标签关联表
     * @return SQL 语句列表
     */
    QStringList createNoteTagsTable();

    /**
     * @brief 添加全文搜索支持
     * @return SQL 语句列表
     */
    QStringList addFullTextSearch();

private:
    static std::unique_ptr<Migration> s_instance;

    QList<MigrationInfo> m_migrations;
    QString m_lastError;
    bool m_initialized;
};

} // namespace qnote

#endif // MIGRATION_H
