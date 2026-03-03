#ifndef TAG_H
#define TAG_H

#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include <QSharedPointer>
#include <QList>

namespace qnote {

/**
 * @brief 标签模型类
 *
 * 表示一个标签的完整数据结构。
 * 标签可以与笔记建立多对多关系。
 */
class Tag {
public:
    using Ptr = QSharedPointer<Tag>;
    using List = QList<Ptr>;

    /**
     * @brief 默认构造函数
     */
    Tag();

    /**
     * @brief 带参数构造函数
     * @param id 标签 ID
     * @param name 标签名称
     */
    Tag(const QString& id, const QString& name);

    /**
     * @brief 析构函数
     */
    ~Tag() = default;

    // ========== 属性访问器 ==========

    /**
     * @brief 获取标签 ID
     * @return ID 字符串 (UUID)
     */
    QString id() const { return m_id; }

    /**
     * @brief 设置标签 ID
     * @param id ID 字符串
     */
    void setId(const QString& id) { m_id = id; }

    /**
     * @brief 获取标签名称
     * @return 名称字符串
     */
    QString name() const { return m_name; }

    /**
     * @brief 设置标签名称
     * @param name 名称字符串
     */
    void setName(const QString& name);

    /**
     * @brief 获取标签颜色
     * @return 颜色字符串 (#RRGGBB 格式)
     */
    QString color() const { return m_color; }

    /**
     * @brief 设置标签颜色
     * @param color 颜色字符串
     */
    void setColor(const QString& color) { m_color = color; }

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
     * @brief 获取关联的笔记数量
     * @return 笔记数量
     */
    int noteCount() const { return m_noteCount; }

    /**
     * @brief 设置关联的笔记数量
     * @param count 笔记数量
     */
    void setNoteCount(int count) { m_noteCount = count; }

    // ========== 序列化 ==========

    /**
     * @brief 转换为 JSON 对象
     * @return JSON 对象
     */
    QJsonObject toJson() const;

    /**
     * @brief 从 JSON 对象创建 Tag
     * @param json JSON 对象
     * @return Tag 指针
     */
    static Ptr fromJson(const QJsonObject& json);

    // ========== 工具方法 ==========

    /**
     * @brief 生成新的标签 ID (UUID)
     * @return UUID 字符串
     */
    static QString generateId();

    /**
     * @brief 验证标签名称是否有效
     * @param name 标签名称
     * @return 有效返回 true
     */
    static bool isValidName(const QString& name);

    /**
     * @brief 规范化标签名称
     * @param name 原始名称
     * @return 规范化后的名称
     */
    static QString normalizeName(const QString& name);

    /**
     * @brief 获取随机颜色
     * @return 颜色字符串
     */
    static QString randomColor();

    /**
     * @brief 创建新标签
     * @param name 标签名称
     * @return Tag 指针
     */
    static Ptr create(const QString& name);

    // ========== 比较运算符 ==========

    /**
     * @brief 相等比较
     */
    bool operator==(const Tag& other) const {
        return m_id == other.m_id;
    }

    /**
     * @brief 不等比较
     */
    bool operator!=(const Tag& other) const {
        return !(*this == other);
    }

private:
    QString m_id;           ///< 标签 ID (UUID)
    QString m_name;         ///< 标签名称
    QString m_color;        ///< 标签颜色
    QDateTime m_createdAt;  ///< 创建时间
    QDateTime m_updatedAt;  ///< 更新时间
    int m_noteCount;        ///< 关联笔记数量（运行时属性，不持久化）
};

} // namespace qnote

#endif // TAG_H
