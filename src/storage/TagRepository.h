#ifndef TAGREPOSITORY_H
#define TAGREPOSITORY_H

#include "../models/Tag.h"
#include <QString>
#include <QList>
#include <memory>

namespace qnote {

/**
 * @brief 标签数据仓库
 *
 * 提供标签的 CRUD 操作和查询功能。
 * 封装数据库访问细节，对外提供领域模型接口。
 */
class TagRepository {
public:
    /**
     * @brief 获取单例实例
     */
    static TagRepository* instance();

    /**
     * @brief 初始化仓库
     * @return 成功返回 true
     */
    bool init();

    // ========== CRUD 操作 ==========

    /**
     * @brief 根据 ID 查找标签
     * @param id 标签 ID
     * @return 标签指针，未找到返回 nullptr
     */
    Tag::Ptr findById(const QString& id);

    /**
     * @brief 根据名称查找标签
     * @param name 标签名称
     * @return 标签指针，未找到返回 nullptr
     */
    Tag::Ptr findByName(const QString& name);

    /**
     * @brief 查找所有标签
     * @param sortByNoteCount 按笔记数量排序
     * @return 标签列表
     */
    Tag::List findAll(bool sortByNoteCount = false);

    /**
     * @brief 保存标签（插入或更新）
     * @param tag 标签对象
     * @return 成功返回 true
     */
    bool save(Tag::Ptr tag);

    /**
     * @brief 删除标签
     * @param id 标签 ID
     * @return 成功返回 true
     */
    bool remove(const QString& id);

    // ========== 查询操作 ==========

    /**
     * @brief 根据笔记 ID 获取标签
     * @param noteId 笔记 ID
     * @return 标签列表
     */
    Tag::List findByNoteId(const QString& noteId);

    /**
     * @brief 搜索标签（按名称模糊匹配）
     * @param keyword 关键词
     * @param limit 限制数量
     * @return 标签列表
     */
    Tag::List search(const QString& keyword, int limit = 20);

    /**
     * @brief 获取常用标签
     * @param limit 限制数量
     * @return 标签列表
     */
    Tag::List findPopular(int limit = 10);

    /**
     * @brief 统计标签数量
     * @return 数量
     */
    int count();

    /**
     * @brief 检查标签是否存在
     * @param id 标签 ID
     * @return 存在返回 true
     */
    bool exists(const QString& id);

    /**
     * @brief 检查标签名称是否存在
     * @param name 标签名称
     * @return 存在返回 true
     */
    bool nameExists(const QString& name);

    // ========== 工具方法 ==========

    /**
     * @brief 获取最后的错误信息
     * @return 错误信息
     */
    QString lastError() const;

    /**
     * @brief 创建或获取标签
     * @param name 标签名称
     * @return 标签指针
     */
    Tag::Ptr createOrGet(const QString& name);

private:
    TagRepository();
    ~TagRepository();
    TagRepository(const TagRepository&) = delete;
    TagRepository& operator=(const TagRepository&) = delete;

    /**
     * @brief 从数据库记录创建 Tag 对象
     * @param query 查询结果
     * @return Tag 指针
     */
    Tag::Ptr fromQuery(class QSqlQuery& query);

private:
    static std::unique_ptr<TagRepository> s_instance;
    QString m_lastError;
    bool m_initialized;
};

} // namespace qnote

#endif // TAGREPOSITORY_H
