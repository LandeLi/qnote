#ifndef NOTEREPOSITORY_H
#define NOTEREPOSITORY_H

#include "../models/Note.h"
#include <QString>
#include <QList>
#include <functional>
#include <memory>

namespace qnote {

/**
 * @brief 查询选项
 */
struct QueryOptions {
    QString folderId;       ///< 文件夹过滤
    QString tagId;          ///< 标签过滤
    bool includeDeleted;    ///< 是否包含已删除
    bool includeArchived;   ///< 是否包含已归档
    bool pinnedOnly;        ///< 仅置顶
    QString sortBy;         ///< 排序字段 (created, updated, title)
    bool sortAsc;           ///< 升序排序
    int limit;              ///< 限制数量
    int offset;             ///< 偏移量

    QueryOptions()
        : includeDeleted(false)
        , includeArchived(false)
        , pinnedOnly(false)
        , sortBy("updated")
        , sortAsc(false)
        , limit(0)
        , offset(0)
    {}
};

/**
 * @brief 笔记数据仓库
 *
 * 提供笔记的 CRUD 操作和查询功能。
 * 封装数据库访问细节，对外提供领域模型接口。
 */
class NoteRepository {
public:
    /**
     * @brief 获取单例实例
     */
    static NoteRepository* instance();

    /**
     * @brief 初始化仓库
     * @return 成功返回 true
     */
    bool init();

    // ========== CRUD 操作 ==========

    /**
     * @brief 根据 ID 查找笔记
     * @param id 笔记 ID
     * @return 笔记指针，未找到返回 nullptr
     */
    Note::Ptr findById(const QString& id);

    /**
     * @brief 查找所有笔记
     * @param options 查询选项
     * @return 笔记列表
     */
    Note::List findAll(const QueryOptions& options = QueryOptions());

    /**
     * @brief 保存笔记（插入或更新）
     * @param note 笔记对象
     * @return 成功返回 true
     */
    bool save(Note::Ptr note);

    /**
     * @brief 删除笔记（软删除）
     * @param id 笔记 ID
     * @return 成功返回 true
     */
    bool remove(const QString& id);

    /**
     * @brief 永久删除笔记
     * @param id 笔记 ID
     * @return 成功返回 true
     */
    bool permanentRemove(const QString& id);

    /**
     * @brief 恢复已删除的笔记
     * @param id 笔记 ID
     * @return 成功返回 true
     */
    bool restore(const QString& id);

    // ========== 查询操作 ==========

    /**
     * @brief 全文搜索
     * @param keyword 关键词
     * @param options 查询选项
     * @return 笔记列表
     */
    Note::List search(const QString& keyword, const QueryOptions& options = QueryOptions());

    /**
     * @brief 根据文件夹查找
     * @param folderId 文件夹 ID
     * @param options 查询选项
     * @return 笔记列表
     */
    Note::List findByFolder(const QString& folderId, const QueryOptions& options = QueryOptions());

    /**
     * @brief 根据标签查找
     * @param tagId 标签 ID
     * @param options 查询选项
     * @return 笔记列表
     */
    Note::List findByTag(const QString& tagId, const QueryOptions& options = QueryOptions());

    /**
     * @brief 查找置顶笔记
     * @return 笔记列表
     */
    Note::List findPinned();

    /**
     * @brief 查找已归档笔记
     * @return 笔记列表
     */
    Note::List findArchived();

    /**
     * @brief 查找已删除笔记（回收站）
     * @return 笔记列表
     */
    Note::List findDeleted();

    /**
     * @brief 查找有提醒的笔记
     * @return 笔记列表
     */
    Note::List findWithReminder();

    /**
     * @brief 统计笔记数量
     * @param options 查询选项
     * @return 数量
     */
    int count(const QueryOptions& options = QueryOptions());

    /**
     * @brief 检查笔记是否存在
     * @param id 笔记 ID
     * @return 存在返回 true
     */
    bool exists(const QString& id);

    // ========== 标签关联 ==========

    /**
     * @brief 为笔记添加标签
     * @param noteId 笔记 ID
     * @param tagId 标签 ID
     * @return 成功返回 true
     */
    bool addTag(const QString& noteId, const QString& tagId);

    /**
     * @brief 移除笔记的标签
     * @param noteId 笔记 ID
     * @param tagId 标签 ID
     * @return 成功返回 true
     */
    bool removeTag(const QString& noteId, const QString& tagId);

    /**
     * @brief 获取笔记的所有标签 ID
     * @param noteId 笔记 ID
     * @return 标签 ID 列表
     */
    QList<QString> getTagIds(const QString& noteId);

    /**
     * @brief 设置笔记的标签
     * @param noteId 笔记 ID
     * @param tagIds 标签 ID 列表
     * @return 成功返回 true
     */
    bool setTags(const QString& noteId, const QList<QString>& tagIds);

    // ========== 工具方法 ==========

    /**
     * @brief 获取最后的错误信息
     * @return 错误信息
     */
    QString lastError() const;

    /**
     * @brief 清空回收站
     * @return 成功返回 true
     */
    bool emptyTrash();

private:
    NoteRepository();
    ~NoteRepository();
    NoteRepository(const NoteRepository&) = delete;
    NoteRepository& operator=(const NoteRepository&) = delete;

    /**
     * @brief 从数据库记录创建 Note 对象
     * @param query 查询结果
     * @return Note 指针
     */
    Note::Ptr fromQuery(class QSqlQuery& query);

    /**
     * @brief 构建查询 WHERE 子句
     * @param options 查询选项
     * @return WHERE 子句和参数
     */
    QPair<QString, QMap<QString, QVariant>> buildWhereClause(const QueryOptions& options);

    /**
     * @brief 构建排序子句
     * @param options 查询选项
     * @return ORDER BY 子句
     */
    QString buildOrderBy(const QueryOptions& options);

private:
    static std::unique_ptr<NoteRepository> s_instance;
    QString m_lastError;
    bool m_initialized;
};

} // namespace qnote

#endif // NOTEREPOSITORY_H
