#ifndef NOTE_H
#define NOTE_H

#include <QString>
#include <QDateTime>
#include <QList>
#include <QJsonObject>
#include <QSharedPointer>

namespace qnote {

/**
 * @brief 笔记内容类型
 */
enum class ContentType {
    PlainText = 0,      ///< 纯文本
    Markdown = 1,       ///< Markdown 格式
    Html = 2            ///< HTML 格式
};

/**
 * @brief 笔记模型类
 *
 * 表示一条笔记的完整数据结构。
 * 支持序列化到 JSON 和从 JSON 反序列化。
 */
class Note {
public:
    using Ptr = QSharedPointer<Note>;
    using List = QList<Ptr>;

    /**
     * @brief 默认构造函数
     */
    Note();

    /**
     * @brief 带参数构造函数
     * @param id 笔记 ID
     * @param title 标题
     */
    Note(const QString& id, const QString& title);

    /**
     * @brief 析构函数
     */
    ~Note() = default;

    // ========== 属性访问器 ==========

    /**
     * @brief 获取笔记 ID
     * @return ID 字符串 (UUID)
     */
    QString id() const { return m_id; }

    /**
     * @brief 设置笔记 ID
     * @param id ID 字符串
     */
    void setId(const QString& id) { m_id = id; }

    /**
     * @brief 获取标题
     * @return 标题字符串
     */
    QString title() const { return m_title; }

    /**
     * @brief 设置标题
     * @param title 标题字符串
     */
    void setTitle(const QString& title) { m_title = title; }

    /**
     * @brief 获取内容
     * @return 内容字符串
     */
    QString content() const { return m_content; }

    /**
     * @brief 设置内容
     * @param content 内容字符串
     */
    void setContent(const QString& content);

    /**
     * @brief 获取内容类型
     * @return 内容类型枚举
     */
    ContentType contentType() const { return m_contentType; }

    /**
     * @brief 设置内容类型
     * @param type 内容类型
     */
    void setContentType(ContentType type) { m_contentType = type; }

    /**
     * @brief 获取所属文件夹 ID
     * @return 文件夹 ID
     */
    QString folderId() const { return m_folderId; }

    /**
     * @brief 设置所属文件夹 ID
     * @param folderId 文件夹 ID
     */
    void setFolderId(const QString& folderId) { m_folderId = folderId; }

    /**
     * @brief 是否置顶
     * @return 置顶返回 true
     */
    bool isPinned() const { return m_isPinned; }

    /**
     * @brief 设置置顶状态
     * @param pinned 是否置顶
     */
    void setPinned(bool pinned) { m_isPinned = pinned; }

    /**
     * @brief 是否归档
     * @return 归档返回 true
     */
    bool isArchived() const { return m_isArchived; }

    /**
     * @brief 设置归档状态
     * @param archived 是否归档
     */
    void setArchived(bool archived) { m_isArchived = archived; }

    /**
     * @brief 是否已删除（软删除）
     * @return 已删除返回 true
     */
    bool isDeleted() const { return m_isDeleted; }

    /**
     * @brief 设置删除状态
     * @param deleted 是否删除
     */
    void setDeleted(bool deleted) { m_isDeleted = deleted; }

    /**
     * @brief 获取创建时间
     * @return 创建时间
     */
    QDateTime createdAt() const { return m_createdAt; }

    /**
     * @brief 设置创建时间
     * @param time 创建时间
     */
    void setCreatedAt(const QDateTime& time) { m_createdAt = time; }

    /**
     * @brief 获取更新时间
     * @return 更新时间
     */
    QDateTime updatedAt() const { return m_updatedAt; }

    /**
     * @brief 设置更新时间
     * @param time 更新时间
     */
    void setUpdatedAt(const QDateTime& time) { m_updatedAt = time; }

    /**
     * @brief 获取删除时间
     * @return 删除时间，未删除返回空
     */
    QDateTime deletedAt() const { return m_deletedAt; }

    /**
     * @brief 设置删除时间
     * @param time 删除时间
     */
    void setDeletedAt(const QDateTime& time) { m_deletedAt = time; }

    /**
     * @brief 获取字数
     * @return 字数
     */
    int wordCount() const { return m_wordCount; }

    /**
     * @brief 设置字数
     * @param count 字数
     */
    void setWordCount(int count) { m_wordCount = count; }

    /**
     * @brief 获取提醒时间
     * @return 提醒时间，无提醒返回空
     */
    QDateTime reminderAt() const { return m_reminderAt; }

    /**
     * @brief 设置提醒时间
     * @param time 提醒时间
     */
    void setReminderAt(const QDateTime& time) { m_reminderAt = time; }

    /**
     * @brief 获取标签 ID 列表
     * @return 标签 ID 列表
     */
    QList<QString> tagIds() const { return m_tagIds; }

    /**
     * @brief 设置标签 ID 列表
     * @param tagIds 标签 ID 列表
     */
    void setTagIds(const QList<QString>& tagIds) { m_tagIds = tagIds; }

    /**
     * @brief 添加标签
     * @param tagId 标签 ID
     */
    void addTag(const QString& tagId);

    /**
     * @brief 移除标签
     * @param tagId 标签 ID
     */
    void removeTag(const QString& tagId);

    // ========== 序列化 ==========

    /**
     * @brief 转换为 JSON 对象
     * @return JSON 对象
     */
    QJsonObject toJson() const;

    /**
     * @brief 从 JSON 对象创建 Note
     * @param json JSON 对象
     * @return Note 指针
     */
    static Ptr fromJson(const QJsonObject& json);

    // ========== 工具方法 ==========

    /**
     * @brief 生成新的笔记 ID (UUID)
     * @return UUID 字符串
     */
    static QString generateId();

    /**
     * @brief 计算文本字数
     * @param text 文本内容
     * @return 字数
     */
    static int calculateWordCount(const QString& text);

    /**
     * @brief 从 Markdown 内容提取标题
     * @param content Markdown 内容
     * @return 提取的标题
     */
    static QString extractTitle(const QString& content);

    /**
     * @brief 创建新笔记
     * @param title 标题（可选）
     * @return Note 指针
     */
    static Ptr create(const QString& title = QString());

private:
    QString m_id;                   ///< 笔记 ID (UUID)
    QString m_title;                ///< 标题
    QString m_content;              ///< 内容
    ContentType m_contentType;      ///< 内容类型
    QString m_folderId;             ///< 所属文件夹 ID
    bool m_isPinned;                ///< 是否置顶
    bool m_isArchived;              ///< 是否归档
    bool m_isDeleted;               ///< 是否已删除
    QDateTime m_createdAt;          ///< 创建时间
    QDateTime m_updatedAt;          ///< 更新时间
    QDateTime m_deletedAt;          ///< 删除时间
    int m_wordCount;                ///< 字数
    QDateTime m_reminderAt;         ///< 提醒时间
    QList<QString> m_tagIds;        ///< 标签 ID 列表
};

} // namespace qnote

#endif // NOTE_H
