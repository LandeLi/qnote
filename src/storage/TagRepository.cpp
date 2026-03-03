#include "TagRepository.h"
#include "Database.h"
#include "../core/Logger.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>

namespace qnote {

// 静态成员初始化
std::unique_ptr<TagRepository> TagRepository::s_instance = nullptr;

TagRepository* TagRepository::instance()
{
    if (!s_instance) {
        s_instance = std::unique_ptr<TagRepository>(new TagRepository());
    }
    return s_instance.get();
}

TagRepository::TagRepository()
    : m_initialized(false)
{
}

TagRepository::~TagRepository()
{
}

bool TagRepository::init()
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
    LOG_INFO("TagRepository initialized");
    return true;
}

Tag::Ptr TagRepository::findById(const QString& id)
{
    if (!m_initialized || id.isEmpty()) {
        return nullptr;
    }

    QString sql = R"(
        SELECT t.*, COUNT(nt.note_id) as note_count
        FROM tags t
        LEFT JOIN note_tags nt ON t.id = nt.tag_id
        WHERE t.id = :id
        GROUP BY t.id
    )";

    QSqlQuery query(Database::instance()->database());
    query.prepare(sql);
    query.bindValue(":id", id);

    if (query.exec() && query.next()) {
        return fromQuery(query);
    }

    return nullptr;
}

Tag::Ptr TagRepository::findByName(const QString& name)
{
    if (!m_initialized || name.isEmpty()) {
        return nullptr;
    }

    QString normalizedName = Tag::normalizeName(name);

    QString sql = R"(
        SELECT t.*, COUNT(nt.note_id) as note_count
        FROM tags t
        LEFT JOIN note_tags nt ON t.id = nt.tag_id
        WHERE t.name = :name
        GROUP BY t.id
    )";

    QSqlQuery query(Database::instance()->database());
    query.prepare(sql);
    query.bindValue(":name", normalizedName);

    if (query.exec() && query.next()) {
        return fromQuery(query);
    }

    return nullptr;
}

Tag::List TagRepository::findAll(bool sortByNoteCount)
{
    Tag::List result;

    if (!m_initialized) {
        return result;
    }

    QString sql = R"(
        SELECT t.*, COUNT(nt.note_id) as note_count
        FROM tags t
        LEFT JOIN note_tags nt ON t.id = nt.tag_id
        GROUP BY t.id
    )";

    if (sortByNoteCount) {
        sql += " ORDER BY note_count DESC, t.name ASC";
    } else {
        sql += " ORDER BY t.name ASC";
    }

    QSqlQuery query(Database::instance()->database());
    if (query.exec(sql)) {
        while (query.next()) {
            result.append(fromQuery(query));
        }
    } else {
        m_lastError = query.lastError().text();
        LOG_ERROR("Failed to get all tags: " + m_lastError);
    }

    return result;
}

bool TagRepository::save(Tag::Ptr tag)
{
    if (!m_initialized || !tag) {
        m_lastError = "Invalid tag or repository not initialized";
        return false;
    }

    // 更新时间戳
    tag->setUpdatedAt(QDateTime::currentDateTime());

    // 检查是否已存在
    bool exists = this->exists(tag->id());

    QSqlQuery query(Database::instance()->database());

    if (exists) {
        // 更新
        query.prepare(R"(
            UPDATE tags SET
                name = :name,
                color = :color,
                updated_at = :updatedAt
            WHERE id = :id
        )");
    } else {
        // 检查名称是否重复
        if (nameExists(tag->name()) && !exists) {
            m_lastError = "Tag name already exists: " + tag->name();
            LOG_WARN(m_lastError);
            return false;
        }

        // 插入
        query.prepare(R"(
            INSERT INTO tags (id, name, color, created_at, updated_at)
            VALUES (:id, :name, :color, :createdAt, :updatedAt)
        )");
        query.bindValue(":createdAt", tag->createdAt().toMSecsSinceEpoch());
    }

    query.bindValue(":id", tag->id());
    query.bindValue(":name", tag->name());
    query.bindValue(":color", tag->color());
    query.bindValue(":updatedAt", tag->updatedAt().toMSecsSinceEpoch());

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        LOG_ERROR("Failed to save tag: " + m_lastError);
        return false;
    }

    LOG_DEBUG("Tag saved: " + tag->name());
    return true;
}

bool TagRepository::remove(const QString& id)
{
    if (!m_initialized || id.isEmpty()) {
        return false;
    }

    // 先删除笔记-标签关联
    QSqlQuery query(Database::instance()->database());
    query.prepare("DELETE FROM note_tags WHERE tag_id = :tagId");
    query.bindValue(":tagId", id);
    query.exec();

    // 删除标签
    query.prepare("DELETE FROM tags WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        LOG_ERROR("Failed to delete tag: " + m_lastError);
        return false;
    }

    LOG_INFO("Tag deleted: " + id);
    return true;
}

Tag::List TagRepository::findByNoteId(const QString& noteId)
{
    Tag::List result;

    if (!m_initialized || noteId.isEmpty()) {
        return result;
    }

    QString sql = R"(
        SELECT t.*, COUNT(nt2.note_id) as note_count
        FROM tags t
        JOIN note_tags nt ON t.id = nt.tag_id
        LEFT JOIN note_tags nt2 ON t.id = nt2.tag_id
        WHERE nt.note_id = :noteId
        GROUP BY t.id
        ORDER BY t.name ASC
    )";

    QSqlQuery query(Database::instance()->database());
    query.prepare(sql);
    query.bindValue(":noteId", noteId);

    if (query.exec()) {
        while (query.next()) {
            result.append(fromQuery(query));
        }
    }

    return result;
}

Tag::List TagRepository::search(const QString& keyword, int limit)
{
    Tag::List result;

    if (!m_initialized || keyword.isEmpty()) {
        return result;
    }

    QString sql = R"(
        SELECT t.*, COUNT(nt.note_id) as note_count
        FROM tags t
        LEFT JOIN note_tags nt ON t.id = nt.tag_id
        WHERE t.name LIKE :keyword
        GROUP BY t.id
        ORDER BY t.name ASC
        LIMIT :limit
    )";

    QSqlQuery query(Database::instance()->database());
    query.prepare(sql);
    query.bindValue(":keyword", "%" + keyword + "%");
    query.bindValue(":limit", limit);

    if (query.exec()) {
        while (query.next()) {
            result.append(fromQuery(query));
        }
    }

    return result;
}

Tag::List TagRepository::findPopular(int limit)
{
    Tag::List result;

    if (!m_initialized) {
        return result;
    }

    QString sql = R"(
        SELECT t.*, COUNT(nt.note_id) as note_count
        FROM tags t
        JOIN note_tags nt ON t.id = nt.tag_id
        GROUP BY t.id
        HAVING note_count > 0
        ORDER BY note_count DESC, t.name ASC
        LIMIT :limit
    )";

    QSqlQuery query(Database::instance()->database());
    query.prepare(sql);
    query.bindValue(":limit", limit);

    if (query.exec()) {
        while (query.next()) {
            result.append(fromQuery(query));
        }
    }

    return result;
}

int TagRepository::count()
{
    if (!m_initialized) {
        return 0;
    }

    QSqlQuery query(Database::instance()->database());
    if (query.exec("SELECT COUNT(*) FROM tags") && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

bool TagRepository::exists(const QString& id)
{
    if (!m_initialized || id.isEmpty()) {
        return false;
    }

    QSqlQuery query(Database::instance()->database());
    query.prepare("SELECT 1 FROM tags WHERE id = :id LIMIT 1");
    query.bindValue(":id", id);

    return query.exec() && query.next();
}

bool TagRepository::nameExists(const QString& name)
{
    if (!m_initialized || name.isEmpty()) {
        return false;
    }

    QString normalizedName = Tag::normalizeName(name);

    QSqlQuery query(Database::instance()->database());
    query.prepare("SELECT 1 FROM tags WHERE name = :name LIMIT 1");
    query.bindValue(":name", normalizedName);

    return query.exec() && query.next();
}

QString TagRepository::lastError() const
{
    return m_lastError;
}

Tag::Ptr TagRepository::createOrGet(const QString& name)
{
    if (!m_initialized || !Tag::isValidName(name)) {
        return nullptr;
    }

    // 先查找是否已存在
    Tag::Ptr existing = findByName(name);
    if (existing) {
        return existing;
    }

    // 创建新标签
    Tag::Ptr tag = Tag::create(name);
    if (!tag) {
        return nullptr;
    }

    // 保存到数据库
    if (!save(tag)) {
        return nullptr;
    }

    return tag;
}

Tag::Ptr TagRepository::fromQuery(QSqlQuery& query)
{
    auto tag = std::make_shared<Tag>();
    tag->setId(query.value("id").toString());
    tag->setName(query.value("name").toString());
    tag->setColor(query.value("color").toString());
    tag->setCreatedAt(QDateTime::fromMSecsSinceEpoch(query.value("created_at").toLongLong()));
    tag->setUpdatedAt(QDateTime::fromMSecsSinceEpoch(query.value("updated_at").toLongLong()));
    tag->setNoteCount(query.value("note_count").toInt());
    return tag;
}

} // namespace qnote
