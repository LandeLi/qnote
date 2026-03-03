#include "services/MarkdownService.h"
#include <iostream>
#include <QDebug>

using namespace qnote;

/**
 * @brief 测试 Markdown 转 HTML
 */
void testToHtml() {
    std::cout << "=== Test toHtml ===" << std::endl;
    
    MarkdownService* service = MarkdownService::instance();
    
    QString markdown = R"(# 标题一

这是一段**粗体**和*斜体*文本。

## 标题二

- 列表项1
- 列表项2

> 这是一段引用

```cpp
int main() {
    return 0;
}
```

---

[链接文本](https://example.com)

![图片](https://example.com/image.png)
)";
    
    QString html = service->toHtml(markdown);
    std::cout << "HTML Output:" << std::endl;
    std::cout << html.toStdString() << std::endl;
    std::cout << std::endl;
}

/**
 * @brief 测试标题提取
 */
void testExtractTitle() {
    std::cout << "=== Test extractTitle ===" << std::endl;
    
    MarkdownService* service = MarkdownService::instance();
    
    QString markdown1 = "# 主标题\n## 副标题";
    QString markdown2 = "## 副标题\n### 三级标题";
    QString markdown3 = "没有标题的文本";
    
    std::cout << "Markdown1 title: " << service->extractTitle(markdown1).toStdString() << std::endl;
    std::cout << "Markdown2 title: " << service->extractTitle(markdown2).toStdString() << std::endl;
    std::cout << "Markdown3 title: " << service->extractTitle(markdown3).toStdString() << std::endl;
    std::cout << std::endl;
}

/**
 * @brief 测试格式化功能
 */
void testFormatting() {
    std::cout << "=== Test Formatting ===" << std::endl;
    
    MarkdownService* service = MarkdownService::instance();
    
    // 测试加粗
    QString text1 = "这是一段文本";
    QString result1 = service->formatBold(text1, 2, 4);
    std::cout << "Bold: " << result1.toStdString() << std::endl;
    
    // 测试斜体
    QString result2 = service->formatItalic(text1, 2, 4);
    std::cout << "Italic: " << result2.toStdString() << std::endl;
    
    // 测试链接
    QString result3 = service->insertLink(text1, "链接", "https://example.com");
    std::cout << "Link: " << result3.toStdString() << std::endl;
    
    // 测试标题
    QString result4 = service->formatHeading(text1, 2, 0, text1.length());
    std::cout << "Heading: " << result4.toStdString() << std::endl;
    
    std::cout << std::endl;
}

/**
 * @brief 测试字数统计
 */
void testWordCount() {
    std::cout << "=== Test Word Count ===" << std::endl;
    
    MarkdownService* service = MarkdownService::instance();
    
    QString text = R"(这是一段中文文本，包含一些 English words。
还有数字 123 和更多内容。

这是第二段。)";
    
    auto stats = service->getTextStats(text);
    
    std::cout << "Text: " << std::endl << text.toStdString() << std::endl;
    std::cout << "Words: " << stats.words << std::endl;
    std::cout << "Characters (with spaces): " << stats.chars << std::endl;
    std::cout << "Characters (no spaces): " << stats.charsNoSpaces << std::endl;
    std::cout << "Lines: " << stats.lines << std::endl;
    std::cout << "Paragraphs: " << stats.paragraphs << std::endl;
    std::cout << "Chinese characters: " << stats.chineseChars << std::endl;
    std::cout << "English words: " << stats.englishWords << std::endl;
    std::cout << std::endl;
}

/**
 * @brief 测试性能（1000行 < 50ms）
 */
void testPerformance() {
    std::cout << "=== Test Performance ===" << std::endl;
    
    MarkdownService* service = MarkdownService::instance();
    
    // 生成1000行测试文本
    QString testText;
    for (int i = 0; i < 1000; ++i) {
        testText += QString("# 标题 %1\n\n这是一段**粗体**和*斜体*文本。\n\n").arg(i);
    }
    
    // 测试转换性能
    QElapsedTimer timer;
    timer.start();
    QString html = service->toHtml(testText);
    qint64 elapsed = timer.elapsed();
    
    std::cout << "1000 lines conversion time: " << elapsed << " ms" << std::endl;
    std::cout << "HTML length: " << html.length() << " characters" << std::endl;
    
    if (elapsed < 50) {
        std::cout << "✓ Performance test PASSED (< 50ms)" << std::endl;
    } else {
        std::cout << "✗ Performance test FAILED (>= 50ms)" << std::endl;
    }
    std::cout << std::endl;
}

int main() {
    std::cout << "MarkdownService Test Suite" << std::endl;
    std::cout << "==========================" << std::endl << std::endl;
    
    testToHtml();
    testExtractTitle();
    testFormatting();
    testWordCount();
    testPerformance();
    
    std::cout << "All tests completed!" << std::endl;
    return 0;
}
