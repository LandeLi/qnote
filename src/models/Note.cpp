#include "Note.h"
#include "../core/Logger.h"

#include <QUuid>
#include <QRegularExpression>
#include <QJsonArray>

namespace qnote {

Note::Note()
    : m_contentType(ContentType::Markdown)
    , m_folderId("default")
    , m_isPinned(false)
    , m_isArchived(false)
    , m_isDeleted(false)
    , m_wordCount(0)
{
    m_createdAt = QDateTime::currentDateTime();
    m_updatedAt = m_createdAt;
}

Note::Note(const QString& id, const QString& title)
    : Note()
{
    m_id = id;
    m_title = title;
}

void Note::setContent(const QString& content)
{
    m_content = content;
    m_wordCount = calculateWordCount(content);
    m_updatedAt = QDateTime::currentDateTime();
}

void Note::addTag(const QString& tagId)
{
    if (!m_tagIds.contains(tagId)) {
        m_tagIds.append(tagId);
    }
}

void Note::removeTag(const QString& tagId)
{
    m_tagIds.removeAll(tagId);
}

QJsonObject Note::toJson() const
{
    QJsonObject json;
    json["id"] = m_id;
    json["title"] = m_title;
    json["content"] = m_content;
    json["contentType"] = static_cast<int>(m_contentType);
    json["folderId"] = m_folderId;
    json["isPinned"] = m_isPinned;
    json["isArchived"] = m_isArchived;
    json["isDeleted"] = m_isDeleted;
    json["createdAt"] = m_createdAt.toMSecsSinceEpoch();
    json["updatedAt"] = m_updatedAt.toMSecsSinceEpoch();
    if (m_deletedAt.isValid()) {
        json["deletedAt"] = m_deletedAt.toMSecsSinceEpoch();
    }
    json["wordCount"] = m_wordCount;
    if (m_reminderAt.isValid()) {
        json["reminderAt"] = m_reminderAt.toMSecsSinceEpoch();
    }

    // 标签列表
    QJsonArray tags;
    for (const QString& tagId : m_tagIds) {
        tags.append(tagId);
    }
    json["tagIds"] = tags;

    return json;
}

Note::Ptr Note::fromJson(const QJsonObject& json)
{
    auto note = std::make_shared<Note>();
    
    note->m_id = json["id"].toString();
    note->m_title = json["title"].toString();
    note->m_content = json["content"].toString();
    note->m_contentType = static_cast<ContentType>(json["contentType"].toInt(1));
    note->m_folderId = json["folderId"].toString("default");
    note->m_isPinned = json["isPinned"].toBool(false);
    note->m_isArchived = json["isArchived"].toBool(false);
    note->m_isDeleted = json["isDeleted"].toBool(false);
    note->m_createdAt = QDateTime::fromMSecsSinceEpoch(json["createdAt"].toLongLong());
    note->m_updatedAt = QDateTime::fromMSecsSinceEpoch(json["updatedAt"].toLongLong());
    
    if (json.contains("deletedAt")) {
        note->m_deletedAt = QDateTime::fromMSecsSinceEpoch(json["deletedAt"].toLongLong());
    }
    
    note->m_wordCount = json["wordCount"].toInt();
    
    if (json.contains("reminderAt")) {
        note->m_reminderAt = QDateTime::fromMSecsSinceEpoch(json["reminderAt"].toLongLong());
    }

    // 解析标签列表
    QJsonArray tags = json["tagIds"].toArray();
    for (const auto& tag : tags) {
        note->m_tagIds.append(tag.toString());
    }

    return note;
}

QString Note::generateId()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

int Note::calculateWordCount(const QString& text)
{
    if (text.isEmpty()) {
        return 0;
    }

    // 简单的字数统计：移除 Markdown 标记后统计
    QString cleanText = text;
    
    // 移除代码块
    cleanText.remove(QRegularExpression("```[\\s\\S]*?```"));
    cleanText.remove(QRegularExpression("`[^`]+`"));
    
    // 移除链接，保留文字
    cleanText.replace(QRegularExpression("\\[([^\\]]+)\\]\\([^)]+\\)"), "\\1");
    
    // 移除图片
    cleanText.remove(QRegularExpression("!\\[[^\\]]*\\]\\([^)]+\\)"));
    
    // 移除标题标记
    cleanText.remove(QRegularExpression("^#+\\s*", QRegularExpression::MultilineOption));
    
    // 移除粗体/斜体标记
    cleanText.remove(QRegularExpression("[*_]{1,3}([^*_]+)[*_]{1,3}"));
    
    // 移除引用标记
    cleanText.remove(QRegularExpression("^>\\s*", QRegularExpression::MultilineOption));
    
    // 移除列表标记
    cleanText.remove(QRegularExpression("^[\\s]*[-*+]\\s+", QRegularExpression::MultilineOption));
    cleanText.remove(QRegularExpression("^[\\s]*\\d+\\.\\s+", QRegularExpression::MultilineOption));
    
    // 移除水平线
    cleanText.remove(QRegularExpression("^[-*_]{3,}\\s*$", QRegularExpression::MultilineOption));

    // 统计字符（包括中文和英文单词）
    int count = 0;
    bool inWord = false;
    
    for (const QChar& ch : cleanText) {
        if (ch.isLetterOrNumber()) {
            // 中文字符单独计数
            if (ch.script() == QChar::Script_Han) {
                count++;
                inWord = false;
            } else if (!inWord) {
                // 英文单词开始
                count++;
                inWord = true;
            }
        } else {
            inWord = false;
        }
    }

    return count;
}

QString Note::extractTitle(const QString& content)
{
    if (content.isEmpty()) {
        return "Untitled";
    }

    // 尝试从第一行 H1 标题提取
    QRegularExpression h1Regex("^#\\s+(.+)$");
    QRegularExpressionMatch match = h1Regex.match(content);
    if (match.hasMatch()) {
        return match.captured(1).trimmed();
    }

    // 尝试从第一个非空行提取
    QStringList lines = content.split('\n');
    for (const QString& line : lines) {
        QString trimmed = line.trimmed();
        if (!trimmed.isEmpty()) {
            // 限制长度
            if (trimmed.length() > 50) {
                return trimmed.left(50) + "...";
            }
            return trimmed;
        }
    }

    return "Untitled";
}

Note::Ptr Note::create(const QString& title)
{
    auto note = std::make_shared<Note>();
    note->m_id = generateId();
    note->m_title = title.isEmpty() ? "New Note" : title;
    note->m_createdAt = QDateTime::currentDateTime();
    note->m_updatedAt = note->m_createdAt;
    return note;
}

} // namespace qnote
