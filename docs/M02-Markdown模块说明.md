# M02 Markdown 服务模块

## 概述

M02 模块提供了完整的 Markdown 解析、格式化和统计功能。

## 功能列表

### M02-01: MarkdownService 基础结构
- 单例模式设计
- 线程安全
- 性能优化的正则表达式

### M02-02: Markdown 解析
- `toHtml()` - 将 Markdown 转换为 HTML
- `extractTitle()` - 提取文档标题
- `extractHeadings()` - 提取所有标题
- `extractLinks()` - 提取所有链接
- `extractImages()` - 提取所有图片

支持的 Markdown 语法：
- 标题（# ~ ######）
- 粗体（**text** 或 __text__）
- 斜体（*text* 或 _text_）
- 删除线（~~text~~）
- 行内代码（`code`）
- 代码块（```code```）
- 链接（[text](url)）
- 图片（![alt](url)）
- 无序列表（- 或 *）
- 有序列表（1.）
- 引用（>）
- 分割线（--- 或 ***）

### M02-03: 格式化辅助函数
- `formatBold()` - 加粗
- `formatItalic()` - 斜体
- `formatStrikethrough()` - 删除线
- `formatCode()` - 行内代码
- `insertLink()` - 插入链接
- `insertImage()` - 插入图片
- `formatHeading()` - 设置标题
- `insertCodeBlock()` - 插入代码块

### M02-04: 字数统计
- `wordCount()` - 统计字数（中文按字符，英文按单词）
- `charCount()` - 统计字符数（含空格）
- `charCountWithoutSpaces()` - 统计字符数（不含空格）
- `lineCount()` - 统计行数
- `paragraphCount()` - 统计段落数
- `getTextStats()` - 获取完整统计信息

## 使用示例

### 基础使用

```cpp
#include "services/MarkdownService.h"

using namespace qnote;

// 获取服务实例
MarkdownService* service = MarkdownService::instance();

// Markdown 转 HTML
QString markdown = "# 标题\n\n这是一段**粗体**文本。";
QString html = service->toHtml(markdown);

// 提取标题
QString title = service->extractTitle(markdown);

// 字数统计
int words = service->wordCount(markdown);
```

### 格式化文本

```cpp
QString text = "Hello World";

// 加粗
QString bold = service->formatBold(text, 0, 5); // **Hello** World

// 插入链接
QString link = service->insertLink(text, "Link", "https://example.com");
// Link[Link](https://example.com)

// 设置为标题
QString heading = service->formatHeading(text, 2); // ## Hello World
```

### 获取统计信息

```cpp
QString text = "这是一段中文文本 with English words.";

auto stats = service->getTextStats(text);
qDebug() << "字数:" << stats.words;
qDebug() << "字符数:" << stats.chars;
qDebug() << "中文字符:" << stats.chineseChars;
qDebug() << "英文单词:" << stats.englishWords;
```

## 性能指标

- 1000行 Markdown 文本转 HTML < 50ms
- 使用预编译的正则表达式提高性能
- 单例模式避免重复初始化

## 后续优化

当前实现使用正则表达式，后续可以：
1. 集成 md4c 库提高解析性能
2. 支持更多 Markdown 扩展语法
3. 添加 HTML 转 Markdown 功能
4. 支持自定义渲染器

## 文件结构

```
src/services/
├── MarkdownService.h      # 服务接口定义
└── MarkdownService.cpp    # 服务实现

tests/
└── test_markdown.cpp      # 单元测试
```

## 代码规范

- 使用 Doxygen 风格注释
- 命名空间：qnote
- 类名：PascalCase
- 方法名：camelCase
- 成员变量：m_camelCase

## 验收标准

- [x] Markdown 可转换为 HTML
- [x] 格式化函数可用
- [x] 性能达标（1000行 < 50ms）
- [x] 代码注释完整（Doxygen 风格）
- [x] 单元测试覆盖

## 作者

二哈 🐕
