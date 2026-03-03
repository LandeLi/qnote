#include "Database.h"
#include "../core/Logger.h"
#include "../core/Settings.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QFile>
#include <QDir>
#include <QStandardPaths>

namespace qnote {

// 静态常量初始化
const QString Database::DB_FILE_NAME = QStringLiteral("qnote.db");
const QString Database::CONNECTION_NAME = QStringLiteral("qnote_connection");

// 静态成员初始化
std::unique_ptr<Database> Database::s_instance = nullptr;

Database* Database::instance()
{
    if (!s_instance) {
        s_instance = std::unique_ptr<Database>(new Database());
    }
    return s_instance.get();
}

Database::Database()
    : m_initialized(false)
    , m_version(0)
{
    m_connectionName = CONNECTION_NAME;
}

Database::~Database()
{
    close();
}

bool Database::init(const QString& dbPath)
{
    if (m_initialized) {
        LOG_WARN("Database already initialized");
        return true;
    }

    // 确定数据库路径
    if (dbPath.isEmpty()) {
        QString dataPath = Settings::instance()->dataPath();
        if (dataPath.isEmpty()) {
            dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        }
        
        QDir dir(dataPath);
        if (!dir.exists()) {
            if (!dir.mkpath(".")) {
                m_lastError = "Failed to create data directory: " + dataPath;
                LOG_ERROR(m_lastError);
                return false;
            }
        }
        
        m_dbPath = dataPath + "/" + DB_FILE_NAME;
    } else {
        m_dbPath = dbPath;
    }

    LOG_INFO("Initializing database at: " + m_dbPath);

    // 检查 SQLite 驱动是否可用
    if (!QSqlDatabase::isDriverAvailable("QSQLITE")) {
        m_lastError = "SQLite driver not available";
        LOG_ERROR(m_lastError);
        return false;
    }

    // 添加数据库连接
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
    db.setDatabaseName(m_dbPath);

    // 打开数据库
    if (!db.open()) {
        m_lastError = "Failed to open database: " + db.lastError().text();
        LOG_ERROR(m_lastError);
        return false;
    }

    // 启用外键约束
    QSqlQuery query(db);
    if (!query.exec("PRAGMA foreign_keys = ON")) {
        LOG_WARN("Failed to enable foreign keys: " + query.lastError().text());
    }

    // 启用 WAL 模式（提高并发性能）
    if (!query.exec("PRAGMA journal_mode = WAL")) {
        LOG_WARN("Failed to enable WAL mode: " + query.lastError().text());
    }

    // 创建元数据表
    bool isNewDatabase = !databaseExists();
    if (isNewDatabase) {
        if (!createMetaTable()) {
            m_lastError = "Failed to create meta table";
            LOG_ERROR(m_lastError);
            db.close();
            return false;
        }
    }

    // 验证数据库完整性
    if (!validateDatabase()) {
        m_lastError = "Database validation failed";
        LOG_ERROR(m_lastError);
        db.close();
        return false;
    }

    // 读取版本号
    QSqlQuery versionQuery(db);
    versionQuery.prepare("SELECT value FROM db_meta WHERE key = 'version'");
    if (versionQuery.exec() && versionQuery.next()) {
        m_version = versionQuery.value(0).toInt();
    } else {
        m_version = 0;
    }

    m_initialized = true;
    LOG_INFO(QString("Database initialized successfully (version: %1)").arg(m_version));
    return true;
}

bool Database::isInitialized() const
{
    return m_initialized;
}

QSqlDatabase Database::database() const
{
    if (!m_initialized) {
        LOG_WARN("Database not initialized");
        return QSqlDatabase();
    }
    return QSqlDatabase::database(m_connectionName);
}

bool Database::execute(const QString& sql)
{
    if (!m_initialized) {
        m_lastError = "Database not initialized";
        return false;
    }

    QSqlQuery query(database());
    if (!query.exec(sql)) {
        m_lastError = query.lastError().text();
        LOG_ERROR("SQL execution failed: " + m_lastError);
        LOG_ERROR("SQL: " + sql);
        return false;
    }
    return true;
}

bool Database::execute(QSqlQuery& query)
{
    if (!m_initialized) {
        m_lastError = "Database not initialized";
        return false;
    }

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        LOG_ERROR("Query execution failed: " + m_lastError);
        return false;
    }
    return true;
}

bool Database::beginTransaction()
{
    if (!m_initialized) {
        m_lastError = "Database not initialized";
        return false;
    }
    return database().transaction();
}

bool Database::commit()
{
    if (!m_initialized) {
        m_lastError = "Database not initialized";
        return false;
    }
    return database().commit();
}

bool Database::rollback()
{
    if (!m_initialized) {
        m_lastError = "Database not initialized";
        return false;
    }
    return database().rollback();
}

QString Database::lastError() const
{
    return m_lastError;
}

QString Database::databasePath() const
{
    return m_dbPath;
}

bool Database::databaseExists() const
{
    return QFile::exists(m_dbPath);
}

void Database::close()
{
    if (m_initialized) {
        QSqlDatabase db = database();
        if (db.isOpen()) {
            db.close();
            LOG_INFO("Database connection closed");
        }
        QSqlDatabase::removeDatabase(m_connectionName);
        m_initialized = false;
    }
}

bool Database::backup(const QString& backupPath)
{
    if (!m_initialized) {
        m_lastError = "Database not initialized";
        return false;
    }

    // 先关闭数据库连接
    close();

    // 复制文件
    bool success = QFile::copy(m_dbPath, backupPath);

    // 重新打开数据库
    init(m_dbPath);

    if (success) {
        LOG_INFO("Database backed up to: " + backupPath);
    } else {
        m_lastError = "Failed to backup database";
        LOG_ERROR(m_lastError);
    }

    return success;
}

int Database::version() const
{
    return m_version;
}

bool Database::setVersion(int version)
{
    if (!m_initialized) {
        m_lastError = "Database not initialized";
        return false;
    }

    QSqlQuery query(database());
    query.prepare("UPDATE db_meta SET value = :version WHERE key = 'version'");
    query.bindValue(":version", version);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        LOG_ERROR("Failed to update version: " + m_lastError);
        return false;
    }

    m_version = version;
    LOG_INFO(QString("Database version set to: %1").arg(version));
    return true;
}

bool Database::createMetaTable()
{
    QString sql = R"(
        CREATE TABLE IF NOT EXISTS db_meta (
            key TEXT PRIMARY KEY NOT NULL,
            value TEXT
        )
    )";

    QSqlQuery query(database());
    if (!query.exec(sql)) {
        m_lastError = "Failed to create db_meta table: " + query.lastError().text();
        LOG_ERROR(m_lastError);
        return false;
    }

    // 插入初始版本
    query.prepare("INSERT INTO db_meta (key, value) VALUES ('version', '0')");
    if (!query.exec()) {
        m_lastError = "Failed to insert version: " + query.lastError().text();
        LOG_ERROR(m_lastError);
        return false;
    }

    return true;
}

bool Database::validateDatabase()
{
    QSqlQuery query(database());
    
    // 执行完整性检查
    if (!query.exec("PRAGMA integrity_check")) {
        m_lastError = "Failed to run integrity check: " + query.lastError().text();
        LOG_ERROR(m_lastError);
        return false;
    }

    if (query.next()) {
        QString result = query.value(0).toString();
        if (result != "ok") {
            m_lastError = "Database integrity check failed: " + result;
            LOG_ERROR(m_lastError);
            return false;
        }
    }

    return true;
}

} // namespace qnote
