#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QStringList>
#include <QUuid>
#include <QDateTime>

namespace qnote {

/**
 * @brief 通用工具函数
 */
namespace Utils {

// ===== 字符串工具 =====

/**
 * @brief 生成 UUID
 */
QString generateUuid();

/**
 * @brief 生成带时间戳的 ID
 */
QString generateId();

/**
 * @brief 截断字符串并添加省略号
 */
QString truncate(const QString& text, int maxLength, const QString& suffix = "...");

/**
 * @brief 清理字符串（移除首尾空白，合并多个空白）
 */
QString cleanText(const QString& text);

/**
 * @brief 从 Markdown 文本中提取标题
 */
QString extractTitle(const QString& markdown, int maxLength = 100);

/**
 * @brief 统计字数（中英文混合）
 */
int wordCount(const QString& text);

/**
 * @brief 统计字符数
 */
int charCount(const QString& text, bool includeSpaces = true);

// ===== 时间工具 =====

/**
 * @brief 格式化相对时间（如"3分钟前"）
 */
QString formatRelativeTime(const QDateTime& dateTime);

/**
 * @brief 格式化友好日期时间
 */
QString formatDateTime(const QDateTime& dateTime);

/**
 * @brief 获取今天日期字符串
 */
QString todayString();

/**
 * @brief 判断是否是今天
 */
bool isToday(const QDateTime& dateTime);

/**
 * @brief 判断是否是昨天
 */
bool isYesterday(const QDateTime& dateTime);

// ===== 文件工具 =====

/**
 * @brief 获取文件扩展名
 */
QString fileExtension(const QString& fileName);

/**
 * @brief 获取不带扩展名的文件名
 */
QString baseName(const QString& fileName);

/**
 * @brief 确保目录存在
 */
bool ensureDirExists(const QString& dirPath);

/**
 * @brief 获取人类可读的文件大小
 */
QString formatFileSize(qint64 bytes);

/**
 * @brief 安全的文件名（移除非法字符）
 */
QString safeFileName(const QString& name);

// ===== 颜色工具 =====

/**
 * @brief 生成随机颜色
 */
QString randomColor();

/**
 * @brief 验证颜色字符串是否有效
 */
bool isValidColor(const QString& color);

/**
 * @brief 默认标签颜色列表
 */
QStringList defaultTagColors();

// ===== 其他 =====

/**
 * @brief 安全的除法（避免除零）
 */
template<typename T>
T safeDivide(T numerator, T denominator, T defaultValue = T()) {
    return denominator != T() ? numerator / denominator : defaultValue;
}

} // namespace Utils

} // namespace qnote

#endif // UTILS_H
