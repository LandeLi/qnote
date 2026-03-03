#include "Tag.h"
#include "../core/Logger.h"

#include <QUuid>
#include <QRegularExpression>
#include <QRandomGenerator>

namespace qnote {

// 预定义颜色列表
static const QStringList PRESET_COLORS = {
    "#e74c3c",  // 红色
    "#e67e22",  // 橙色
    "#f1c40f",  // 黄色
    "#2ecc71",  // 绿色
    "#1abc9c",  // 青色
    "#3498db",  // 蓝色
    "#9b59b6",  // 紫色
    "#34495e",  // 深灰
    "#95a5a6",  // 浅灰
    "#fd79a8",  // 粉色
};

Tag::Tag()
    : m_color("#3498db")
    , m_noteCount(0)
{
    m_createdAt = QDateTime::currentDateTime();
    m_updatedAt = m_createdAt;
}

Tag::Tag(const QString& id, const QString& name)
    : Tag()
{
    m_id = id;
    m_name = name;
}

void Tag::setName(const QString& name)
{
    m_name = normalizeName(name);
    m_updatedAt = QDateTime::currentDateTime();
}

QJsonObject Tag::toJson() const
{
    QJsonObject json;
    json["id"] = m_id;
    json["name"] = m_name;
    json["color"] = m_color;
    json["createdAt"] = m_createdAt.toMSecsSinceEpoch();
    json["updatedAt"] = m_updatedAt.toMSecsSinceEpoch();
    json["noteCount"] = m_noteCount;
    return json;
}

Tag::Ptr Tag::fromJson(const QJsonObject& json)
{
    auto tag = std::make_shared<Tag>();
    tag->m_id = json["id"].toString();
    tag->m_name = json["name"].toString();
    tag->m_color = json["color"].toString("#3498db");
    tag->m_createdAt = QDateTime::fromMSecsSinceEpoch(json["createdAt"].toLongLong());
    tag->m_updatedAt = QDateTime::fromMSecsSinceEpoch(json["updatedAt"].toLongLong());
    tag->m_noteCount = json["noteCount"].toInt(0);
    return tag;
}

QString Tag::generateId()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

bool Tag::isValidName(const QString& name)
{
    QString normalized = normalizeName(name);
    if (normalized.isEmpty()) {
        return false;
    }

    // 长度限制：1-50 字符
    if (normalized.length() > 50) {
        return false;
    }

    // 不允许纯空白
    if (normalized.trimmed().isEmpty()) {
        return false;
    }

    // 不允许特殊字符（保留用于系统的字符）
    QRegularExpression invalidChars(R"([<>\"'&/\\])");
    if (normalized.contains(invalidChars)) {
        return false;
    }

    return true;
}

QString Tag::normalizeName(const QString& name)
{
    // 去除首尾空白
    QString normalized = name.trimmed();

    // 将多个连续空白替换为单个空格
    normalized.replace(QRegularExpression("\\s+"), " ");

    return normalized;
}

QString Tag::randomColor()
{
    static int lastIndex = -1;

    // 尝试选择与上次不同的颜色
    int index;
    do {
        index = QRandomGenerator::global()->bounded(PRESET_COLORS.size());
    } while (index == lastIndex && PRESET_COLORS.size() > 1);

    lastIndex = index;
    return PRESET_COLORS[index];
}

Tag::Ptr Tag::create(const QString& name)
{
    if (!isValidName(name)) {
        LOG_WARN("Invalid tag name: " + name);
        return nullptr;
    }

    auto tag = std::make_shared<Tag>();
    tag->m_id = generateId();
    tag->m_name = normalizeName(name);
    tag->m_color = randomColor();
    tag->m_createdAt = QDateTime::currentDateTime();
    tag->m_updatedAt = tag->m_createdAt;
    return tag;
}

} // namespace qnote
