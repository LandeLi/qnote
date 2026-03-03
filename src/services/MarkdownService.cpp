#include "MarkdownService.h"
#include "core/Logger.h"
#include <QTextStream>
#include <QRegularExpressionMatch>

namespace qnote {

std::unique_ptr<MarkdownService> MarkdownService::s_instance = nullptr;

/**
 * @brief 构造函数
 * 
 * 初始化所有正则表达式模式
 */
MarkdownService::MarkdownService()
    : m_headingPattern(R"(^(#{1,6})\s+(.+)$)")
    , m_boldPattern(R"(\*\*(.+?)\*\*|__(.+?)__)")
    , m_italicPattern(R"(\*(.+?)\*|_(.+?)_)")
    , m_codePattern(R"(`(.+?)`)")
    , m_linkPattern(R"(\[([^\]]+)\]\(([^)]+)\))")
    , m_imagePattern(R"(!\[([^\]]*)\]\(([^)]+)\))")
    , m_listPattern(R"(^[\*\-\+]\s+(.+)$|^\d+\.\s+(.+)$)")
    , m_blockquotePattern(R"(^>\s+(.+)$)")
    , m_codeBlockPattern(R"(^```(\w*)\n(.*?)\n```)")
    , m_hrPattern(R"(^(-{3,}|\*{3,})$)")
{
    // 优化正则表达式性能
    m_headingPattern.optimize();
    m_boldPattern.optimize();
    m_italicPattern.optimize();
    m_codePattern.optimize();
    m_linkPattern.optimize();
    m_imagePattern.optimize();
    m_listPattern.optimize();
    m_blockquotePattern.optimize();
    m_codeBlockPattern.optimize();
    m_hrPattern.optimize();
    
    LOG_DEBUG("MarkdownService initialized");
}

/**
 * @brief 析构函数
 */
MarkdownService::~MarkdownService() {
    LOG_DEBUG("MarkdownService destroyed");
}

/**
 * @brief 获取单例实例
 */
MarkdownService* MarkdownService::instance() {
    if (!s_instance) {
        s_instance = std::unique_ptr<MarkdownService>(new MarkdownService());
    }
    return s_instance.get();
}

// ========== M02-02: Markdown 解析 ==========

/**
 * @brief 将 Markdown 文本转换为 HTML
 */
QString MarkdownService::toHtml(const QString& markdown) {
    if (markdown.isEmpty()) {
        return QString();
    }
    
    QString html;
    QStringList lines = markdown.split('\n');
    bool inCodeBlock = false;
    QString codeBlockContent;
    QString codeLanguage;
    
    for (const QString& line : lines) {
        // 处理代码块
        if (line.startsWith("```")) {
            if (!inCodeBlock) {
                // 开始代码块
                inCodeBlock = true;
                codeLanguage = line.mid(3).trimmed();
                codeBlockContent.clear();
            } else {
                // 结束代码块
                inCodeBlock = false;
                html += "<pre><code";
                if (!codeLanguage.isEmpty()) {
                    html += QString(" class=\"language-%1\"").arg(codeLanguage);
                }
                html += ">" + escapeHtml(codeBlockContent) + "</code></pre>\n";
            }
            continue;
        }
        
        if (inCodeBlock) {
            if (!codeBlockContent.isEmpty()) {
                codeBlockContent += "\n";
            }
            codeBlockContent += line;
            continue;
        }
        
        // 空行
        if (line.trimmed().isEmpty()) {
            html += "<br/>\n";
            continue;
        }
        
        // 分割线
        if (m_hrPattern.match(line).hasMatch()) {
            html += "<hr/>\n";
            continue;
        }
        
        // 标题
        QRegularExpressionMatch headingMatch = m_headingPattern.match(line);
        if (headingMatch.hasMatch()) {
            int level = headingMatch.captured(1).length();
            QString text = processInlineElements(headingMatch.captured(2));
            html += QString("<h%1>%2</h%1>\n").arg(level).arg(text);
            continue;
        }
        
        // 引用
        QRegularExpressionMatch quoteMatch = m_blockquotePattern.match(line);
        if (quoteMatch.hasMatch()) {
            QString text = processInlineElements(quoteMatch.captured(1));
            html += "<blockquote>" + text + "</blockquote>\n";
            continue;
        }
        
        // 无序列表
        QRegularExpressionMatch listMatch = m_listPattern.match(line);
        if (listMatch.hasMatch()) {
            QString text = processInlineElements(listMatch.captured(1).isEmpty() 
                ? listMatch.captured(2) : listMatch.captured(1));
            html += "<li>" + text + "</li>\n";
            continue;
        }
        
        // 普通段落
        html += "<p>" + processInlineElements(line) + "</p>\n";
    }
    
    return html;
}

/**
 * @brief 从 Markdown 文本中提取标题
 */
QString MarkdownService::extractTitle(const QString& markdown) {
    if (markdown.isEmpty()) {
        return QString();
    }
    
    // 从一级标题开始查找
    for (int level = 1; level <= 6; ++level) {
        QRegularExpression pattern(QString(R"(^#{%1}\s+(.+)$)").arg(level), 
                                   QRegularExpression::MultilineOption);
        QRegularExpressionMatch match = pattern.match(markdown);
        if (match.hasMatch()) {
            return match.captured(1).trimmed();
        }
    }
    
    return QString();
}

/**
 * @brief 提取所有标题
 */
QList<QPair<int, QString>> MarkdownService::extractHeadings(const QString& markdown) {
    QList<QPair<int, QString>> headings;
    if (markdown.isEmpty()) {
        return headings;
    }
    
    QStringList lines = markdown.split('\n');
    for (const QString& line : lines) {
        QRegularExpressionMatch match = m_headingPattern.match(line);
        if (match.hasMatch()) {
            int level = match.captured(1).length();
            QString text = match.captured(2).trimmed();
            headings.append(qMakePair(level, text));
        }
    }
    
    return headings;
}

/**
 * @brief 提取所有链接
 */
QList<QPair<QString, QString>> MarkdownService::extractLinks(const QString& markdown) {
    QList<QPair<QString, QString>> links;
    if (markdown.isEmpty()) {
        return links;
    }
    
    QRegularExpressionMatchIterator it = m_linkPattern.globalMatch(markdown);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString text = match.captured(1);
        QString url = match.captured(2);
        links.append(qMakePair(text, url));
    }
    
    return links;
}

/**
 * @brief 提取所有图片
 */
QList<QPair<QString, QString>> MarkdownService::extractImages(const QString& markdown) {
    QList<QPair<QString, QString>> images;
    if (markdown.isEmpty()) {
        return images;
    }
    
    QRegularExpressionMatchIterator it = m_imagePattern.globalMatch(markdown);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString alt = match.captured(1);
        QString url = match.captured(2);
        images.append(qMakePair(alt, url));
    }
    
    return images;
}

// ========== M02-03: 格式化辅助函数 ==========

/**
 * @brief 将选中文本加粗
 */
QString MarkdownService::formatBold(const QString& text, int start, int end) {
    if (start < 0 || end < 0) {
        // 没有选区，插入空标记
        return text + "****";
    }
    
    QString result = text;
    QString selected = text.mid(start, end - start);
    QString replacement = "**" + selected + "**";
    result.replace(start, end - start, replacement);
    return result;
}

/**
 * @brief 将选中文本设置为斜体
 */
QString MarkdownService::formatItalic(const QString& text, int start, int end) {
    if (start < 0 || end < 0) {
        return text + "__";
    }
    
    QString result = text;
    QString selected = text.mid(start, end - start);
    QString replacement = "*" + selected + "*";
    result.replace(start, end - start, replacement);
    return result;
}

/**
 * @brief 将选中文本添加删除线
 */
QString MarkdownService::formatStrikethrough(const QString& text, int start, int end) {
    if (start < 0 || end < 0) {
        return text + "~~~~";
    }
    
    QString result = text;
    QString selected = text.mid(start, end - start);
    QString replacement = "~~" + selected + "~~";
    result.replace(start, end - start, replacement);
    return result;
}

/**
 * @brief 将选中文本设置为行内代码
 */
QString MarkdownService::formatCode(const QString& text, int start, int end) {
    if (start < 0 || end < 0) {
        return text + "``";
    }
    
    QString result = text;
    QString selected = text.mid(start, end - start);
    QString replacement = "`" + selected + "`";
    result.replace(start, end - start, replacement);
    return result;
}

/**
 * @brief 插入链接
 */
QString MarkdownService::insertLink(const QString& text, const QString& displayText,
                                     const QString& url, int position) {
    QString link = QString("[%1](%2)").arg(displayText, url);
    
    if (position < 0) {
        return text + link;
    }
    
    QString result = text;
    result.insert(position, link);
    return result;
}

/**
 * @brief 插入图片
 */
QString MarkdownService::insertImage(const QString& text, const QString& altText,
                                      const QString& url, int position) {
    QString image = QString("![%1](%2)").arg(altText, url);
    
    if (position < 0) {
        return text + image;
    }
    
    QString result = text;
    result.insert(position, image);
    return result;
}

/**
 * @brief 将选中文本转换为标题
 */
QString MarkdownService::formatHeading(const QString& text, int level, int start, int end) {
    if (level < 1 || level > 6) {
        LOG_WARN(QString("Invalid heading level: %1").arg(level));
        return text;
    }
    
    QString prefix = QString("#").repeated(level) + " ";
    
    if (start < 0 || end < 0) {
        return prefix + text;
    }
    
    QString result = text;
    result.insert(start, prefix);
    return result;
}

/**
 * @brief 插入代码块
 */
QString MarkdownService::insertCodeBlock(const QString& text, const QString& code,
                                          const QString& language, int position) {
    QString block;
    if (language.isEmpty()) {
        block = QString("```\n%1\n```").arg(code);
    } else {
        block = QString("```%1\n%2\n```").arg(language, code);
    }
    
    if (position < 0) {
        return text + "\n" + block + "\n";
    }
    
    QString result = text;
    result.insert(position, block + "\n");
    return result;
}

// ========== M02-04: 字数统计 ==========

/**
 * @brief 统计字数
 */
int MarkdownService::wordCount(const QString& text) {
    if (text.isEmpty()) {
        return 0;
    }
    
    int count = 0;
    int chineseCount = 0;
    int englishWordCount = 0;
    
    // 统计中文字符
    for (const QChar& ch : text) {
        if (isChineseChar(ch)) {
            chineseCount++;
        }
    }
    
    // 统计英文单词（连续的字母、数字）
    QString textCopy = text;
    QRegularExpression wordPattern(R"(\b[a-zA-Z0-9]+\b)");
    QRegularExpressionMatchIterator it = wordPattern.globalMatch(textCopy);
    while (it.hasNext()) {
        it.next();
        englishWordCount++;
    }
    
    count = chineseCount + englishWordCount;
    return count;
}

/**
 * @brief 统计字符数（包含空格）
 */
int MarkdownService::charCount(const QString& text) {
    return text.length();
}

/**
 * @brief 统计字符数（不包含空格）
 */
int MarkdownService::charCountWithoutSpaces(const QString& text) {
    int count = 0;
    for (const QChar& ch : text) {
        if (!ch.isSpace()) {
            count++;
        }
    }
    return count;
}

/**
 * @brief 统计行数
 */
int MarkdownService::lineCount(const QString& text) {
    if (text.isEmpty()) {
        return 0;
    }
    return text.count('\n') + 1;
}

/**
 * @brief 统计段落数
 */
int MarkdownService::paragraphCount(const QString& text) {
    if (text.isEmpty()) {
        return 0;
    }
    
    QStringList paragraphs = text.split(QRegularExpression(R"(\n\s*\n)"), 
                                        Qt::SkipEmptyParts);
    return paragraphs.count();
}

/**
 * @brief 获取统计信息
 */
MarkdownService::TextStats MarkdownService::getTextStats(const QString& text) {
    TextStats stats;
    stats.chars = charCount(text);
    stats.charsNoSpaces = charCountWithoutSpaces(text);
    stats.lines = lineCount(text);
    stats.paragraphs = paragraphCount(text);
    
    // 统计中文和英文
    stats.chineseChars = 0;
    stats.englishWords = 0;
    
    for (const QChar& ch : text) {
        if (isChineseChar(ch)) {
            stats.chineseChars++;
        }
    }
    
    QRegularExpression wordPattern(R"(\b[a-zA-Z0-9]+\b)");
    QRegularExpressionMatchIterator it = wordPattern.globalMatch(text);
    while (it.hasNext()) {
        it.next();
        stats.englishWords++;
    }
    
    stats.words = stats.chineseChars + stats.englishWords;
    
    return stats;
}

// ========== 私有方法 ==========

/**
 * @brief 转义 HTML 特殊字符
 */
QString MarkdownService::escapeHtml(const QString& text) {
    QString result = text;
    result.replace("&", "&amp;");
    result.replace("<", "&lt;");
    result.replace(">", "&gt;");
    result.replace("\"", "&quot;");
    result.replace("'", "&#39;");
    return result;
}

/**
 * @brief 处理行内元素
 */
QString MarkdownService::processInlineElements(const QString& line) {
    QString result = escapeHtml(line);
    
    // 处理图片（必须在链接之前）
    QRegularExpressionMatchIterator imageIt = m_imagePattern.globalMatch(result);
    int offset = 0;
    while (imageIt.hasNext()) {
        QRegularExpressionMatch match = imageIt.next();
        QString alt = match.captured(1);
        QString url = match.captured(2);
        QString replacement = QString(R"(<img src="%1" alt="%2"/>)").arg(url, alt);
        result.replace(match.capturedStart() + offset, 
                      match.capturedLength(), replacement);
        offset += replacement.length() - match.capturedLength();
    }
    
    // 处理链接
    offset = 0;
    QRegularExpressionMatchIterator linkIt = m_linkPattern.globalMatch(result);
    while (linkIt.hasNext()) {
        QRegularExpressionMatch match = linkIt.next();
        QString text = match.captured(1);
        QString url = match.captured(2);
        QString replacement = QString(R"(<a href="%1">%2</a>)").arg(url, text);
        result.replace(match.capturedStart() + offset, 
                      match.capturedLength(), replacement);
        offset += replacement.length() - match.capturedLength();
    }
    
    // 处理粗体
    offset = 0;
    QRegularExpressionMatchIterator boldIt = m_boldPattern.globalMatch(result);
    while (boldIt.hasNext()) {
        QRegularExpressionMatch match = boldIt.next();
        QString text = match.captured(1).isEmpty() ? match.captured(2) : match.captured(1);
        QString replacement = QString("<strong>%1</strong>").arg(text);
        result.replace(match.capturedStart() + offset, 
                      match.capturedLength(), replacement);
        offset += replacement.length() - match.capturedLength();
    }
    
    // 处理斜体
    offset = 0;
    QRegularExpressionMatchIterator italicIt = m_italicPattern.globalMatch(result);
    while (italicIt.hasNext()) {
        QRegularExpressionMatch match = italicIt.next();
        QString text = match.captured(1).isEmpty() ? match.captured(2) : match.captured(1);
        QString replacement = QString("<em>%1</em>").arg(text);
        result.replace(match.capturedStart() + offset, 
                      match.capturedLength(), replacement);
        offset += replacement.length() - match.capturedLength();
    }
    
    // 处理行内代码
    offset = 0;
    QRegularExpressionMatchIterator codeIt = m_codePattern.globalMatch(result);
    while (codeIt.hasNext()) {
        QRegularExpressionMatch match = codeIt.next();
        QString code = match.captured(1);
        QString replacement = QString("<code>%1</code>").arg(code);
        result.replace(match.capturedStart() + offset, 
                      match.capturedLength(), replacement);
        offset += replacement.length() - match.capturedLength();
    }
    
    return result;
}

/**
 * @brief 判断字符是否为中文字符
 */
bool MarkdownService::isChineseChar(QChar ch) const {
    ushort unicode = ch.unicode();
    // 基本汉字范围：0x4E00 - 0x9FFF
    // 扩展A区：0x3400 - 0x4DBF
    return (unicode >= 0x4E00 && unicode <= 0x9FFF) ||
           (unicode >= 0x3400 && unicode <= 0x4DBF);
}

} // namespace qnote
