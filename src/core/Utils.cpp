#include "Utils.h"
#include <QRegularExpression>
#include <QDir>
#include <QFileInfo>
#include <cmath>

namespace qnote {
namespace Utils {

// ===== 字符串工具 =====

QString generateUuid() {
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QString generateId() {
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMddHHmmsszzz");
    QString random = QString::number(QRandomGenerator::global()->bounded(1000, 9999));
    return timestamp + "_" + random;
}

QString truncate(const QString& text, int maxLength, const QString& suffix) {
    if (text.length() <= maxLength) {
        return text;
    }
    return text.left(maxLength - suffix.length()) + suffix;
}

QString cleanText(const QString& text) {
    QString result = text.trimmed();
    result.replace(QRegularExpression("\\s+"), " ");
    return result;
}

QString extractTitle(const QString& markdown, int maxLength) {
    if (markdown.isEmpty()) {
        return "无标题";
    }
    
    // 尝试从 H1 标题提取
    QRegularExpression h1Regex("^#\\s+(.+)$", QRegularExpression::MultilineOption);
    QRegularExpressionMatch match = h1Regex.match(markdown);
    
    if (match.hasMatch()) {
        QString title = match.captured(1).trimmed();
        // 移除 Markdown 格式
        title.remove(QRegularExpression("[*_`~\\[\\]]"));
        return truncate(title, maxLength);
    }
    
    // 没有标题，取第一行非空文本
    QStringList lines = markdown.split('\n');
    for (const QString& line : lines) {
        QString cleaned = line.trimmed();
        // 跳过空行和纯格式标记
        if (cleaned.isEmpty() || cleaned.startsWith("---") || cleaned.startsWith("```")) {
            continue;
        }
        // 移除 Markdown 格式
        cleaned.remove(QRegularExpression("[#*_`~\\[\\]]"));
        cleaned = cleanText(cleaned);
        if (!cleaned.isEmpty()) {
            return truncate(cleaned, maxLength);
        }
    }
    
    return "无标题";
}

int wordCount(const QString& text) {
    if (text.isEmpty()) {
        return 0;
    }
    
    int count = 0;
    bool inWord = false;
    
    for (const QChar& c : text) {
        if (c.isLetterOrNumber() || c == '_') {
            if (!inWord) {
                count++;
                inWord = true;
            }
        } else {
            inWord = false;
        }
        
        // 中文字符单独计数
        if (c.script() == QChar::Script_Han) {
            count++;
        }
    }
    
    return count;
}

int charCount(const QString& text, bool includeSpaces) {
    if (includeSpaces) {
        return text.length();
    }
    
    int count = 0;
    for (const QChar& c : text) {
        if (!c.isSpace()) {
            count++;
        }
    }
    return count;
}

// ===== 时间工具 =====

QString formatRelativeTime(const QDateTime& dateTime) {
    if (!dateTime.isValid()) {
        return "未知";
    }
    
    qint64 seconds = dateTime.secsTo(QDateTime::currentDateTime());
    
    if (seconds < 60) {
        return "刚刚";
    } else if (seconds < 3600) {
        int minutes = seconds / 60;
        return QString("%1分钟前").arg(minutes);
    } else if (seconds < 86400) {
        int hours = seconds / 3600;
        return QString("%1小时前").arg(hours);
    } else if (seconds < 604800) {
        int days = seconds / 86400;
        return QString("%1天前").arg(days);
    } else if (seconds < 2592000) {
        int weeks = seconds / 604800;
        return QString("%1周前").arg(weeks);
    } else if (seconds < 31536000) {
        int months = seconds / 2592000;
        return QString("%1个月前").arg(months);
    } else {
        int years = seconds / 31536000;
        return QString("%1年前").arg(years);
    }
}

QString formatDateTime(const QDateTime& dateTime) {
    if (!dateTime.isValid()) {
        return "未知";
    }
    
    if (isToday(dateTime)) {
        return "今天 " + dateTime.toString("HH:mm");
    } else if (isYesterday(dateTime)) {
        return "昨天 " + dateTime.toString("HH:mm");
    } else {
        return dateTime.toString("yyyy-MM-dd HH:mm");
    }
}

QString todayString() {
    return QDate::currentDate().toString("yyyy-MM-dd");
}

bool isToday(const QDateTime& dateTime) {
    return dateTime.date() == QDate::currentDate();
}

bool isYesterday(const QDateTime& dateTime) {
    return dateTime.date() == QDate::currentDate().addDays(-1);
}

// ===== 文件工具 =====

QString fileExtension(const QString& fileName) {
    int pos = fileName.lastIndexOf('.');
    if (pos > 0 && pos < fileName.length() - 1) {
        return fileName.mid(pos + 1).toLower();
    }
    return QString();
}

QString baseName(const QString& fileName) {
    QString name = QFileInfo(fileName).fileName();
    int pos = name.lastIndexOf('.');
    if (pos > 0) {
        return name.left(pos);
    }
    return name;
}

bool ensureDirExists(const QString& dirPath) {
    QDir dir(dirPath);
    if (dir.exists()) {
        return true;
    }
    return dir.mkpath(".");
}

QString formatFileSize(qint64 bytes) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unitIndex = 0;
    double size = static_cast<double>(bytes);
    
    while (size >= 1024.0 && unitIndex < 4) {
        size /= 1024.0;
        unitIndex++;
    }
    
    if (unitIndex == 0) {
        return QString("%1 %2").arg(bytes).arg(units[unitIndex]);
    } else {
        return QString("%1 %2").arg(size, 0, 'f', 1).arg(units[unitIndex]);
    }
}

QString safeFileName(const QString& name) {
    QString result = name;
    // 移除或替换非法字符
    result.replace(QRegularExpression("[\\\\/:*?\"<>|]"), "_");
    // 移除首尾空白和点
    result = result.trimmed();
    while (result.endsWith('.')) {
        result.chop(1);
    }
    return result;
}

// ===== 颜色工具 =====

QString randomColor() {
    QStringList colors = defaultTagColors();
    return colors[QRandomGenerator::global()->bounded(colors.size())];
}

bool isValidColor(const QString& color) {
    // 检查十六进制颜色
    QRegularExpression hexRegex("^#([0-9A-Fa-f]{3}|[0-9A-Fa-f]{6})$");
    if (hexRegex.match(color).hasMatch()) {
        return true;
    }
    
    // 检查 RGB/RGBA
    QRegularExpression rgbRegex("^rgba?\\s*\\(\\s*\\d+\\s*,\\s*\\d+\\s*,\\s*\\d+\\s*(,\\s*[\\d.]+\\s*)?\\)$");
    return rgbRegex.match(color).hasMatch();
}

QStringList defaultTagColors() {
    return {
        "#FF6B6B",  // 红色
        "#4ECDC4",  // 青色
        "#45B7D1",  // 蓝色
        "#96CEB4",  // 绿色
        "#FFEAA7",  // 黄色
        "#DDA0DD",  // 紫色
        "#98D8C8",  // 薄荷绿
        "#F7DC6F",  // 金色
        "#BB8FCE",  // 淡紫
        "#85C1E9",  // 淡蓝
    };
}

} // namespace Utils
} // namespace qnote
