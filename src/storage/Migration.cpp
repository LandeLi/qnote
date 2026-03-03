#include "Migration.h"
#include "Database.h"
#include "../core/Logger.h"

#include <QSqlQuery>
#include <QSqlError>

namespace qnote {

// 静态成员初始化
std::unique_ptr<Migration> Migration::s_instance = nullptr;

Migration* Migration::instance()
{
    if (!s_instance) {
        s_instance = std::unique_ptr<Migration>(new Migration());
    }
    return s_instance.get();
}

Migration::Migration()
    : m_initialized(false)
{
}

Migration::~Migration()
{
}

bool Migration::init()
{
    if (m_initialized) {
        return true;
    }

    if (!Database::instance()->isInitialized()) {
        m_lastError = "Database not initialized";
        LOG_ERROR(m_lastError);
        return false;
    }

    registerMigrations();
    m_initialized = true;
    LOG_INFO(QString("Migration initialized with %1 migrations").arg(m_migrations.size()));
    return true;
}

void Migration::registerMigrations()
{
    // 迁移 V1: 创建笔记表
    MigrationInfo v1;
    v1.version = 1;
    v1.description = "Create notes table";
    v1.upSql = createNotesTable();
    v1.downSql = {"DROP TABLE IF EXISTS notes"};
    registerMigration(v1);

    // 迁移 V2: 创建标签表
    MigrationInfo v2;
    v2.version = 2;
    v2.description = "Create tags table";
    v2.upSql = createTagsTable();
    v2.downSql = {"DROP TABLE IF EXISTS tags"};
    registerMigration(v2);

    // 迁移 V3: 创建笔记-标签关联表
    MigrationInfo v3;
    v3.version = 3;
    v3.description = "Create note_tags relation table";
    v3.upSql = createNoteTagsTable();
    v3.downSql = {"DROP TABLE IF EXISTS note_tags"};
    registerMigration(v3);

    // 迁移 V4: 添加全文搜索支持
    MigrationInfo v4;
    v4.version = 4;
    v4.description = "Add full-text search support";
    v4.upSql = addFullTextSearch();
    v4.downSql = {
        "DROP TABLE IF EXISTS notes_fts",
        "DROP TRIGGER IF EXISTS notes_ai",
        "DROP TRIGGER IF EXISTS notes_ad",
        "DROP TRIGGER IF EXISTS notes_au"
    };
    registerMigration(v4);
}

void Migration::registerMigration(const MigrationInfo& info)
{
    m_migrations.append(info);
}

int Migration::currentVersion() const
{
    return Database::instance()->version();
}

int Migration::latestVersion() const
{
    if (m_migrations.isEmpty()) {
        return 0;
    }
    return m_migrations.last().version;
}

bool Migration::migrateAll()
{
    return migrateTo(latestVersion());
}

bool Migration::migrateTo(int targetVersion)
{
    if (!m_initialized) {
        if (!init()) {
            return false;
        }
    }

    int current = currentVersion();
    
    if (current == targetVersion) {
        LOG_INFO(QString("Database already at version %1").arg(current));
        return true;
    }

    if (targetVersion < current) {
        m_lastError = "Downgrading database is not supported";
        LOG_WARN(m_lastError);
        return false;
    }

    LOG_INFO(QString("Migrating database from version %1 to %2").arg(current).arg(targetVersion));

    // 执行迁移
    for (int v = current + 1; v <= targetVersion; ++v) {
        if (!executeMigration(v)) {
            LOG_ERROR(QString("Migration to version %1 failed").arg(v));
            return false;
        }
    }

    LOG_INFO(QString("Database migrated to version %1").arg(targetVersion));
    return true;
}

bool Migration::hasPendingMigrations() const
{
    return currentVersion() < latestVersion();
}

QStringList Migration::pendingMigrations() const
{
    QStringList result;
    int current = currentVersion();

    for (const auto& migration : m_migrations) {
        if (migration.version > current) {
            result.append(QString("V%1: %2").arg(migration.version).arg(migration.description));
        }
    }

    return result;
}

QString Migration::lastError() const
{
    return m_lastError;
}

bool Migration::reset()
{
    if (!Database::instance()->isInitialized()) {
        m_lastError = "Database not initialized";
        return false;
    }

    LOG_WARN("Resetting database - all data will be lost!");

    // 删除所有表
    QStringList tables = {
        "notes_fts",
        "note_tags",
        "tags",
        "notes"
    };

    QSqlQuery query(Database::instance()->database());
    
    for (const QString& table : tables) {
        QString sql = QString("DROP TABLE IF EXISTS %1").arg(table);
        if (!query.exec(sql)) {
            LOG_WARN("Failed to drop table " + table + ": " + query.lastError().text());
        }
    }

    // 删除触发器
    QStringList triggers = {
        "notes_ai", "notes_ad", "notes_au"
    };

    for (const QString& trigger : triggers) {
        QString sql = QString("DROP TRIGGER IF EXISTS %1").arg(trigger);
        if (!query.exec(sql)) {
            LOG_WARN("Failed to drop trigger " + trigger);
        }
    }

    // 重置版本号
    Database::instance()->setVersion(0);

    // 重新执行所有迁移
    return migrateAll();
}

bool Migration::executeMigration(int version)
{
    // 查找迁移信息
    MigrationInfo* target = nullptr;
    for (auto& migration : m_migrations) {
        if (migration.version == version) {
            target = &migration;
            break;
        }
    }

    if (!target) {
        m_lastError = QString("Migration version %1 not found").arg(version);
        LOG_ERROR(m_lastError);
        return false;
    }

    LOG_INFO(QString("Executing migration V%1: %2").arg(version).arg(target->description));

    // 开始事务
    if (!Database::instance()->beginTransaction()) {
        m_lastError = "Failed to begin transaction: " + Database::instance()->lastError();
        LOG_ERROR(m_lastError);
        return false;
    }

    // 执行 SQL 语句
    QSqlQuery query(Database::instance()->database());
    bool success = true;

    for (const QString& sql : target->upSql) {
        if (!query.exec(sql)) {
            m_lastError = QString("Migration failed at version %1: %2")
                          .arg(version)
                          .arg(query.lastError().text());
            LOG_ERROR(m_lastError);
            LOG_ERROR("Failed SQL: " + sql);
            success = false;
            break;
        }
    }

    if (success) {
        // 更新版本号
        if (!Database::instance()->setVersion(version)) {
            m_lastError = "Failed to update database version";
            LOG_ERROR(m_lastError);
            success = false;
        }
    }

    // 提交或回滚事务
    if (success) {
        if (!Database::instance()->commit()) {
            m_lastError = "Failed to commit transaction";
            LOG_ERROR(m_lastError);
            return false;
        }
        LOG_INFO(QString("Migration V%1 completed successfully").arg(version));
    } else {
        Database::instance()->rollback();
        LOG_ERROR(QString("Migration V%1 rolled back").arg(version));
    }

    return success;
}

QStringList Migration::createNotesTable()
{
    return {
        R"(
        CREATE TABLE IF NOT EXISTS notes (
            id TEXT PRIMARY KEY NOT NULL,
            title TEXT NOT NULL DEFAULT '',
            content TEXT,
            content_type TEXT DEFAULT 'markdown',
            folder_id TEXT DEFAULT 'default',
            is_pinned INTEGER DEFAULT 0,
            is_archived INTEGER DEFAULT 0,
            is_deleted INTEGER DEFAULT 0,
            created_at INTEGER NOT NULL,
            updated_at INTEGER NOT NULL,
            deleted_at INTEGER,
            word_count INTEGER DEFAULT 0,
            reminder_at INTEGER
        )
        )",
        "CREATE INDEX IF NOT EXISTS idx_notes_folder ON notes(folder_id)",
        "CREATE INDEX IF NOT EXISTS idx_notes_created ON notes(created_at)",
        "CREATE INDEX IF NOT EXISTS idx_notes_updated ON notes(updated_at)",
        "CREATE INDEX IF NOT EXISTS idx_notes_deleted ON notes(is_deleted)"
    };
}

QStringList Migration::createTagsTable()
{
    return {
        R"(
        CREATE TABLE IF NOT EXISTS tags (
            id TEXT PRIMARY KEY NOT NULL,
            name TEXT NOT NULL UNIQUE,
            color TEXT DEFAULT '#3498db',
            created_at INTEGER NOT NULL,
            updated_at INTEGER NOT NULL
        )
        )",
        "CREATE INDEX IF NOT EXISTS idx_tags_name ON tags(name)"
    };
}

QStringList Migration::createNoteTagsTable()
{
    return {
        R"(
        CREATE TABLE IF NOT EXISTS note_tags (
            note_id TEXT NOT NULL,
            tag_id TEXT NOT NULL,
            created_at INTEGER NOT NULL,
            PRIMARY KEY (note_id, tag_id),
            FOREIGN KEY (note_id) REFERENCES notes(id) ON DELETE CASCADE,
            FOREIGN KEY (tag_id) REFERENCES tags(id) ON DELETE CASCADE
        )
        )",
        "CREATE INDEX IF NOT EXISTS idx_note_tags_note ON note_tags(note_id)",
        "CREATE INDEX IF NOT EXISTS idx_note_tags_tag ON note_tags(tag_id)"
    };
}

QStringList Migration::addFullTextSearch()
{
    return {
        // 创建 FTS5 虚拟表
        R"(
        CREATE VIRTUAL TABLE IF NOT EXISTS notes_fts USING fts5(
            id UNINDEXED,
            title,
            content,
            content='notes',
            content_rowid='rowid'
        )
        )",
        // 触发器: 插入笔记时更新 FTS
        R"(
        CREATE TRIGGER IF NOT EXISTS notes_ai AFTER INSERT ON notes BEGIN
            INSERT INTO notes_fts(rowid, id, title, content)
            VALUES (new.rowid, new.id, new.title, new.content);
        END
        )",
        // 触发器: 删除笔记时更新 FTS
        R"(
        CREATE TRIGGER IF NOT EXISTS notes_ad AFTER DELETE ON notes BEGIN
            INSERT INTO notes_fts(notes_fts, rowid, id, title, content)
            VALUES('delete', old.rowid, old.id, old.title, old.content);
        END
        )",
        // 触发器: 更新笔记时更新 FTS
        R"(
        CREATE TRIGGER IF NOT EXISTS notes_au AFTER UPDATE ON notes BEGIN
            INSERT INTO notes_fts(notes_fts, rowid, id, title, content)
            VALUES('delete', old.rowid, old.id, old.title, old.content);
            INSERT INTO notes_fts(rowid, id, title, content)
            VALUES (new.rowid, new.id, new.title, new.content);
        END
        )"
    };
}

} // namespace qnote
