#ifndef MARKDOWNSERVICE_H
#define MARKDOWNSERVICE_H

#include <QString>
#include <QStringList>
#include <QRegularExpression>
#include <memory>

namespace qnote {

/**
 * @brief Markdown 解析和转换服务
 * 
 * 提供 Markdown 文本的解析、格式转换、提取信息等功能。
 * 使用正则表达式实现基础 Markdown 语法支持，后续可优化为使用 md4c。
 * 
 * @note 线程安全，所有方法均为无状态方法
 */
class MarkdownService {
public:
    /**
     * @brief 获取单例实例
     * @return MarkdownService 实例指针
     */
    static MarkdownService* instance();
    
    // ========== M02-02: Markdown 解析 ==========
    
    /**
     * @brief 将 Markdown 文本转换为 HTML
     * 
     * 支持的 Markdown 语法：
     * - 标题（# ~ ######）
     * - 粗体（**text** 或 __text__）
     * - 斜体（*text* 或 _text_）
     * - 删除线（~~text~~）
     * - 行内代码（`code`）
     * - 代码块（```code```）
     * - 链接（[text](url)）
     * - 图片（![alt](url)）
     * - 无序列表（- 或 *）
     * - 有序列表（1.）
     * - 引用（>）
     * - 分割线（--- 或 ***）
     * 
     * @param markdown Markdown 格式文本
     * @return 转换后的 HTML 文本
     */
    QString toHtml(const QString& markdown);
    
    /**
     * @brief 从 Markdown 文本中提取标题
     * 
     * 优先提取第一个一级标题（#），如果没有则提取第一个二级标题，以此类推
     * 
     * @param markdown Markdown 格式文本
     * @return 提取的标题文本，如果没有标题则返回空字符串
     */
    QString extractTitle(const QString& markdown);
    
    /**
     * @brief 提取所有标题
     * @param markdown Markdown 格式文本
     * @return 标题列表（包含级别和文本）
     */
    QList<QPair<int, QString>> extractHeadings(const QString& markdown);
    
    /**
     * @brief 提取所有链接
     * @param markdown Markdown 格式文本
     * @return 链接列表（文本, URL）
     */
    QList<QPair<QString, QString>> extractLinks(const QString& markdown);
    
    /**
     * @brief 提取所有图片
     * @param markdown Markdown 格式文本
     * @return 图片列表（alt 文本, URL）
     */
    QList<QPair<QString, QString>> extractImages(const QString& markdown);
    
    // ========== M02-03: 格式化辅助函数 ==========
    
    /**
     * @brief 将选中文本加粗
     * @param text 原始文本
     * @param start 选区起始位置
     * @param end 选区结束位置
     * @return 格式化后的文本
     */
    QString formatBold(const QString& text, int start = -1, int end = -1);
    
    /**
     * @brief 将选中文本设置为斜体
     * @param text 原始文本
     * @param start 选区起始位置
     * @param end 选区结束位置
     * @return 格式化后的文本
     */
    QString formatItalic(const QString& text, int start = -1, int end = -1);
    
    /**
     * @brief 将选中文本添加删除线
     * @param text 原始文本
     * @param start 选区起始位置
     * @param end 选区结束位置
     * @return 格式化后的文本
     */
    QString formatStrikethrough(const QString& text, int start = -1, int end = -1);
    
    /**
     * @brief 将选中文本设置为行内代码
     * @param text 原始文本
     * @param start 选区起始位置
     * @param end 选区结束位置
     * @return 格式化后的文本
     */
    QString formatCode(const QString& text, int start = -1, int end = -1);
    
    /**
     * @brief 插入链接
     * @param text 原始文本
     * @param displayText 显示文本
     * @param url 链接 URL
     * @param position 插入位置（-1 表示末尾）
     * @return 格式化后的文本
     */
    QString insertLink(const QString& text, const QString& displayText, 
                       const QString& url, int position = -1);
    
    /**
     * @brief 插入图片
     * @param text 原始文本
     * @param altText 替代文本
     * @param url 图片 URL
     * @param position 插入位置（-1 表示末尾）
     * @return 格式化后的文本
     */
    QString insertImage(const QString& text, const QString& altText,
                        const QString& url, int position = -1);
    
    /**
     * @brief 将选中文本转换为标题
     * @param text 原始文本
     * @param level 标题级别（1-6）
     * @param start 选区起始位置
     * @param end 选区结束位置
     * @return 格式化后的文本
     */
    QString formatHeading(const QString& text, int level, int start = -1, int end = -1);
    
    /**
     * @brief 插入代码块
     * @param text 原始文本
     * @param code 代码内容
     * @param language 语言标识（可选）
     * @param position 插入位置（-1 表示末尾）
     * @return 格式化后的文本
     */
    QString insertCodeBlock(const QString& text, const QString& code,
                            const QString& language = QString(), int position = -1);
    
    // ========== M02-04: 字数统计 ==========
    
    /**
     * @brief 统计字数（中文按字符计，英文按单词计）
     * @param text 文本内容
     * @return 字数
     */
    int wordCount(const QString& text);
    
    /**
     * @brief 统计字符数（包含空格）
     * @param text 文本内容
     * @return 字符数
     */
    int charCount(const QString& text);
    
    /**
     * @brief 统计字符数（不包含空格）
     * @param text 文本内容
     * @return 字符数
     */
    int charCountWithoutSpaces(const QString& text);
    
    /**
     * @brief 统计行数
     * @param text 文本内容
     * @return 行数
     */
    int lineCount(const QString& text);
    
    /**
     * @brief 统计段落数（空行分隔）
     * @param text 文本内容
     * @return 段落数
     */
    int paragraphCount(const QString& text);
    
    /**
     * @brief 获取统计信息
     * @param text 文本内容
     * @return 统计信息（字数、字符数、行数等）
     */
    struct TextStats {
        int words;              ///< 字数
        int chars;              ///< 字符数（含空格）
        int charsNoSpaces;      ///< 字符数（不含空格）
        int lines;              ///< 行数
        int paragraphs;         ///< 段落数
        int chineseChars;       ///< 中文字符数
        int englishWords;       ///< 英文单词数
    };
    TextStats getTextStats(const QString& text);

private:
    MarkdownService();
    ~MarkdownService();
    MarkdownService(const MarkdownService&) = delete;
    MarkdownService& operator=(const MarkdownService&) = delete;
    
    /**
     * @brief 转义 HTML 特殊字符
     * @param text 原始文本
     * @return 转义后的文本
     */
    QString escapeHtml(const QString& text);
    
    /**
     * @brief 处理行内元素（粗体、斜体、代码等）
     * @param line 单行文本
     * @return 转换后的 HTML
     */
    QString processInlineElements(const QString& line);
    
    /**
     * @brief 判断字符是否为中文字符
     * @param ch 字符
     * @return 是否为中文字符
     */
    bool isChineseChar(QChar ch) const;
    
private:
    static std::unique_ptr<MarkdownService> s_instance;
    
    // 正则表达式模式（编译一次，提高性能）
    QRegularExpression m_headingPattern;        ///< 标题模式
    QRegularExpression m_boldPattern;           ///< 粗体模式
    QRegularExpression m_italicPattern;         ///< 斜体模式
    QRegularExpression m_codePattern;           ///< 行内代码模式
    QRegularExpression m_linkPattern;           ///< 链接模式
    QRegularExpression m_imagePattern;          ///< 图片模式
    QRegularExpression m_listPattern;           ///< 列表模式
    QRegularExpression m_blockquotePattern;     ///< 引用模式
    QRegularExpression m_codeBlockPattern;      ///< 代码块模式
    QRegularExpression m_hrPattern;             ///< 分割线模式
};

} // namespace qnote

#endif // MARKDOWNSERVICE_H
