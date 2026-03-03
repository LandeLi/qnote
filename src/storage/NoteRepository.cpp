#include "NoteRepository.h"
#include "Database.h"
#include "../core/Logger.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QVariantMap>
#include <QDateTime>

namespace qnote {

// 静态成员初始化
std::unique_ptr<NoteRepository> NoteRepository::s_instance = nullptr;

NoteRepository* NoteRepository::instance()
{
    if (!s_instance) {
        s_instance = std::unique_ptr<NoteRepository>(new NoteRepository());
    }
    return s_instance.get();
}

NoteRepository::NoteRepository()
    : m_initialized(false)
{
}

NoteRepository::~NoteRepository()
{
}

bool NoteRepository::init()
{
    if (m_initialized) {
        return true;
    }

    if (!Database::instance()->isInitialized()) {
        m_lastError = "Database not initialized";
        LOG_ERROR(m_lastError);
        return false;
    }

    m_initialized = true;
    LOG_INFO("NoteRepository initialized");
    return true;
}

Note::Ptr NoteRepository::findById(const QString& id)
{
    if (!m_initialized || id.isEmpty()) {
        return nullptr;
    }

    QSqlQuery query(Database::instance()->database());
    query.prepare("SELECT * FROM notes WHERE id = :id");
    query.bindValue(":id", id);

    if (query.exec() && query.next()) {
        return fromQuery(query);
    }

    return nullptr;
}

Note::List NoteRepository::findAll(const QueryOptions& options)
{
    Note::List result;

    if (!m_initialized) {
        return result;
    }

    auto [whereClause, params] = buildWhereClause(options);
    QString orderBy = buildOrderBy(options);

    QString sql = QString("SELECT * FROM notes %1 %2").arg(whereClause, orderBy);

    if (options.limit > 0) {
        sql += QString(" LIMIT %1 OFFSET %2").arg(options.limit).arg(options.offset);
    }

    QSqlQuery query(Database::instance()->database());
    query.prepare(sql);

    for (auto it = params.begin(); it != params.end(); ++it) {
        query.bindValue(":" + it.key(), it.value());
    }

    if (query.exec()) {
        while (query.next()) {
            result.append(fromQuery(query));
        }
    } else {
        m_lastError = query.lastError().text();
        LOG_ERROR("Query failed: " + m_lastError);
    }

    return result;
}

bool NoteRepository::save(Note::Ptr note)
{
    if (!m_initialized || !note) {
        m_lastError = "Invalid note or repository not initialized";
        return false;
    }

    // 更新时间戳
    note->setUpdatedAt(QDateTime::currentDateTime());

    // 检查是否已存在
    bool exists = this->exists(note->id());

    QSqlQuery query(Database::instance()->database());

    if (exists) {
        // 更新
        query.prepare(R"(
            UPDATE notes SET
                title = :title,
                content = :content,
                content_type = :contentType,
                folder_id = :folderId,
                is_pinned = :isPinned,
                is_archived = :isArchived,
                is_deleted = :isDeleted,
                updated_at = :updatedAt,
                deleted_at = :deletedAt,
                word_count = :wordCount,
                reminder_at = :reminderAt
            WHERE id = :id
        )");
    } else {
        // 插入
        query.prepare(R"(
            INSERT INTO notes (
                id, title, content, content_type, folder_id,
                is_pinned, is_archived, is_deleted,
                created_at, updated_at, deleted_at,
                word_count, reminder_at
            ) VALUES (
                :id, :title, :content, :contentType, :folderId,
                :isPinned, :isArchived, :isDeleted,
                :createdAt, :updatedAt, :deletedAt,
                :wordCount, :reminderAt
            )
        )");
        query.bindValue(":createdAt", note->createdAt().toMSecsSinceEpoch());
    }

    query.bindValue(":id", note->id());
    query.bindValue(":title", note->title());
    query.bindValue(":content", note->content());
    query.bindValue(":contentType", static_cast<int>(note->contentType()));
    query.bindValue(":folderId", note->folderId());
    query.bindValue(":isPinned", note->isPinned() ? 1 : 0);
    query.bindValue(":isArchived", note->isArchived() ? 1 : 0);
    query.bindValue(":isDeleted", note->isDeleted() ? 1 : 0);
    query.bindValue(":updatedAt", note->updatedAt().toMSecsSinceEpoch());
    query.bindValue(":deletedAt", note->deletedAt().isValid() ? note->deletedAt().toMSecsSinceEpoch() : QVariant());
    query.bindValue(":wordCount", note->wordCount());
    query.bindValue(":reminderAt", note->reminderAt().isValid() ? note->reminderAt().toMSecsSinceEpoch() : QVariant());

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        LOG_ERROR("Failed to save note: " + m_lastError);
        return false;
    }

    // 保存标签关联
    if (!exists) {
        setTags(note->id(), note->tagIds());
    }

    LOG_DEBUG("Note saved: " + note->id());
    return true;
}

bool NoteRepository::remove(const QString& id)
{
    if (!m_initialized || id.isEmpty()) {
        return false;
    }

    QSqlQuery query(Database::instance()->database());
    query.prepare("UPDATE notes SET is_deleted = 1, deleted_at = :deletedAt WHERE id = :id");
    query.bindValue(":deletedAt", QDateTime::currentDateTime().toMSecsSinceEpoch());
    query.bindValue(":id", id);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        LOG_ERROR("Failed to soft delete note: " + m_lastError);
        return false;
    }

    LOG_INFO("Note moved to trash: " + id);
    return true;
}

bool NoteRepository::permanentRemove(const QString& id)
{
    if (!m_initialized || id.isEmpty()) {
        return false;
    }

    // 先删除标签关联
    QSqlQuery query(Database::instance()->database());
    query.prepare("DELETE FROM note_tags WHERE note_id = :noteId");
    query.bindValue(":noteId", id);
    query.exec();

    // 删除笔记
    query.prepare("DELETE FROM notes WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        LOG_ERROR("Failed to permanently delete note: " + m_lastError);
        return false;
    }

    LOG_WARN("Note permanently deleted: " + id);
    return true;
}

bool NoteRepository::restore(const QString& id)
{
    if (!m_initialized || id.isEmpty()) {
        return false;
    }

    QSqlQuery query(Database::instance()->database());
    query.prepare("UPDATE notes SET is_deleted = 0, deleted_at = NULL WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        LOG_ERROR("Failed to restore note: " + m_lastError);
        return false;
    }

    LOG_INFO("Note restored: " + id);
    return true;
}

Note::List NoteRepository::search(const QString& keyword, const QueryOptions& options)
{
    Note::List result;

    if (!m_initialized || keyword.isEmpty()) {
        return result;
    }

    // 使用 FTS5 全文搜索
    QString sql = R"(
        SELECT n.* FROM notes n
        JOIN notes_fts fts ON n.id = fts.id
        WHERE notes_fts MATCH :keyword
    )";

    if (!options.includeDeleted) {
        sql += " AND n.is_deleted = 0";
    }

    if (!options.includeArchived) {
        sql += " AND n.is_archived = 0";
    }

    sql += " ORDER BY n.is_pinned DESC, n.updated_at DESC";

    if (options.limit > 0) {
        sql += QString(" LIMIT %1").arg(options.limit);
    }

    QSqlQuery query(Database::instance()->database());
    query.prepare(sql);
    query.bindValue(":keyword", keyword);

    if (query.exec()) {
        while (query.next()) {
            result.append(fromQuery(query));
        }
    } else {
        m_lastError = query.lastError().text();
        LOG_ERROR("Search failed: " + m_lastError);
    }

    return result;
}

Note::List NoteRepository::findByFolder(const QString& folderId, const QueryOptions& options)
{
    QueryOptions opts = options;
    opts.folderId = folderId;
    return findAll(opts);
}

Note::List NoteRepository::findByTag(const QString& tagId, const QueryOptions& options)
{
    Note::List result;

    if (!m_initialized || tagId.isEmpty()) {
        return result;
    }

    QString sql = R"(
        SELECT n.* FROM notes n
        JOIN note_tags nt ON n.id = nt.note_id
        WHERE nt.tag_id = :tagId
    )";

    if (!options.includeDeleted) {
        sql += " AND n.is_deleted = 0";
    }

    if (!options.includeArchived) {
        sql += " AND n.is_archived = 0";
    }

    sql += " ORDER BY n.is_pinned DESC, n.updated_at DESC";

    QSqlQuery query(Database::instance()->database());
    query.prepare(sql);
    query.bindValue(":tagId", tagId);

    if (query.exec()) {
        while (query.next()) {
            result.append(fromQuery(query));
        }
    }

    return result;
}

Note::List NoteRepository::findPinned()
{
    QueryOptions options;
    options.pinnedOnly = true;
    return findAll(options);
}

Note::List NoteRepository::findArchived()
{
    QueryOptions options;
    options.includeArchived = true;
    options.includeDeleted = false;

    QString sql = "SELECT * FROM notes WHERE is_archived = 1 AND is_deleted = 0 ORDER BY updated_at DESC";

    Note::List result;
    QSqlQuery query(Database::instance()->database());
    if (query.exec(sql)) {
        while (query.next()) {
            result.append(fromQuery(query));
        }
    }
    return result;
}

Note::List NoteRepository::findDeleted()
{
    QString sql = "SELECT * FROM notes WHERE is_deleted = 1 ORDER BY deleted_at DESC";

    Note::List result;
    QSqlQuery query(Database::instance()->database());
    if (query.exec(sql)) {
        while (query.next()) {
            result.append(fromQuery(query));
        }
    }
    return result;
}

Note::List NoteRepository::findWithReminder()
{
    QString sql = "SELECT * FROM notes WHERE reminder_at IS NOT NULL AND is_deleted = 0 ORDER BY reminder_at ASC";

    Note::List result;
    QSqlQuery query(Database::instance()->database());
    if (query.exec(sql)) {
        while (query.next()) {
            result.append(fromQuery(query));
        }
    }
    return result;
}

int NoteRepository::count(const QueryOptions& options)
{
    if (!m_initialized) {
        return 0;
    }

    auto [whereClause, params] = buildWhereClause(options);
    QString sql = QString("SELECT COUNT(*) FROM notes %1").arg(whereClause);

    QSqlQuery query(Database::instance()->database());
    query.prepare(sql);

    for (auto it = params.begin(); it != params.end(); ++it) {
        query.bindValue(":" + it.key(), it.value());
    }

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

bool NoteRepository::exists(const QString& id)
{
    if (!m_initialized || id.isEmpty()) {
        return false;
    }

    QSqlQuery query(Database::instance()->database());
    query.prepare("SELECT 1 FROM notes WHERE id = :id LIMIT 1");
    query.bindValue(":id", id);

    return query.exec() && query.next();
}

bool NoteRepository::addTag(const QString& noteId, const QString& tagId)
{
    if (!m_initialized || noteId.isEmpty() || tagId.isEmpty()) {
        return false;
    }

    QSqlQuery query(Database::instance()->database());
    query.prepare("INSERT OR IGNORE INTO note_tags (note_id, tag_id, created_at) VALUES (:noteId, :tagId, :createdAt)");
    query.bindValue(":noteId", noteId);
    query.bindValue(":tagId", tagId);
    query.bindValue(":createdAt", QDateTime::currentDateTime().toMSecsSinceEpoch());

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        LOG_ERROR("Failed to add tag to note: " + m_lastError);
        return false;
    }

    return true;
}

bool NoteRepository::removeTag(const QString& noteId, const QString& tagId)
{
    if (!m_initialized || noteId.isEmpty() || tagId.isEmpty()) {
        return false;
    }

    QSqlQuery query(Database::instance()->database());
    query.prepare("DELETE FROM note_tags WHERE note_id = :noteId AND tag_id = :tagId");
    query.bindValue(":noteId", noteId);
    query.bindValue(":tagId", tagId);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        LOG_ERROR("Failed to remove tag from note: " + m_lastError);
        return false;
    }

    return true;
}

QList<QString> NoteRepository::getTagIds(const QString& noteId)
{
    QList<QString> result;

    if (!m_initialized || noteId.isEmpty()) {
        return result;
    }

    QSqlQuery query(Database::instance()->database());
    query.prepare("SELECT tag_id FROM note_tags WHERE note_id = :noteId");
    query.bindValue(":noteId", noteId);

    if (query.exec()) {
        while (query.next()) {
            result.append(query.value(0).toString());
        }
    }

    return result;
}

bool NoteRepository::setTags(const QString& noteId, const QList<QString>& tagIds)
{
    if (!m_initialized || noteId.isEmpty()) {
        return false;
    }

    // 先删除所有现有关联
    QSqlQuery query(Database::instance()->database());
    query.prepare("DELETE FROM note_tags WHERE note_id = :noteId");
    query.bindValue(":noteId", noteId);
    query.exec();

    // 添加新关联
    for (const QString& tagId : tagIds) {
        if (!addTag(noteId, tagId)) {
            return false;
        }
    }

    return true;
}

QString NoteRepository::lastError() const
{
    return m_lastError;
}

bool NoteRepository::emptyTrash()
{
    if (!m_initialized) {
        return false;
    }

    QSqlQuery query(Database::instance()->database());

    // 删除已删除笔记的标签关联
    if (!query.exec("DELETE FROM note_tags WHERE note_id IN (SELECT id FROM notes WHERE is_deleted = 1)")) {
        m_lastError = query.lastError().text();
        LOG_ERROR("Failed to delete note tags: " + m_lastError);
        return false;
    }

    // 删除已删除的笔记
    if (!query.exec("DELETE FROM notes WHERE is_deleted = 1")) {
        m_lastError = query.lastError().text();
        LOG_ERROR("Failed to empty trash: " + m_lastError);
        return false;
    }

    LOG_INFO("Trash emptied");
    return true;
}

Note::Ptr NoteRepository::fromQuery(QSqlQuery& query)
{
    auto note = std::make_shared<Note>();
    note->setId(query.value("id").toString());
    note->setTitle(query.value("title").toString());
    note->setContent(query.value("content").toString());
    note->setContentType(static_cast<ContentType>(query.value("content_type").toInt()));
    note->setFolderId(query.value("folder_id").toString());
    note->setPinned(query.value("is_pinned").toInt() == 1);
    note->setArchived(query.value("is_archived").toInt() == 1);
    note->setDeleted(query.value("is_deleted").toInt() == 1);
    note->setCreatedAt(QDateTime::fromMSecsSinceEpoch(query.value("created_at").toLongLong()));
    note->setUpdatedAt(QDateTime::fromMSecsSinceEpoch(query.value("updated_at").toLongLong()));

    QVariant deletedAt = query.value("deleted_at");
    if (!deletedAt.isNull()) {
        note->setDeletedAt(QDateTime::fromMSecsSinceEpoch(deletedAt.toLongLong()));
    }

    note->setWordCount(query.value("word_count").toInt());

    QVariant reminderAt = query.value("reminder_at");
    if (!reminderAt.isNull()) {
        note->setReminderAt(QDateTime::fromMSecsSinceEpoch(reminderAt.toLongLong()));
    }

    // 加载标签
    note->setTagIds(getTagIds(note->id()));

    return note;
}

QPair<QString, QMap<QString, QVariant>> NoteRepository::buildWhereClause(const QueryOptions& options)
{
    QStringList conditions;
    QMap<QString, QVariant> params;

    if (!options.includeDeleted) {
        conditions << "is_deleted = 0";
    }

    if (!options.includeArchived) {
        conditions << "is_archived = 0";
    }

    if (options.pinnedOnly) {
        conditions << "is_pinned = 1";
    }

    if (!options.folderId.isEmpty()) {
        conditions << "folder_id = :folderId";
        params["folderId"] = options.folderId;
    }

    QString whereClause;
    if (!conditions.isEmpty()) {
        whereClause = "WHERE " + conditions.join(" AND ");
    }

    return {whereClause, params};
}

QString NoteRepository::buildOrderBy(const QueryOptions& options)
{
    QString order = "ORDER BY ";

    // 置顶笔记优先
    order += "is_pinned DESC, ";

    QString field;
    if (options.sortBy == "created") {
        field = "created_at";
    } else if (options.sortBy == "title") {
        field = "title COLLATE NOCASE";
    } else {
        field = "updated_at";
    }

    order += field;
    order += options.sortAsc ? " ASC" : " DESC";

    return order;
}

} // namespace qnote
