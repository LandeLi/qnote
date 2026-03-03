# QNote - Qt 桌面便签应用

一款轻量、高性能、支持 Markdown 的跨平台桌面便签应用。

## 项目信息

| 项目 | 内容 |
|------|------|
| 项目名称 | QNote |
| 技术栈 | Qt 6 + C++17 + SQLite + CMake + md4c |
| 目标平台 | Linux (Kylin/UOS/Ubuntu), Windows |
| 许可证 | MIT |

## 目录结构

```
qnote/
├── docs/                   # 项目文档
│   ├── 00-调研报告.md
│   ├── 01-开发计划.md
│   ├── 02-详细开发计划.md
│   ├── 03-详细设计文档.md
│   └── 04-模块化开发计划.md
├── src/                    # 源代码
│   ├── app/               # 应用程序
│   ├── core/              # 核心工具
│   ├── models/            # 数据模型
│   ├── storage/           # 数据存储
│   ├── managers/          # 业务管理
│   ├── services/          # 服务层
│   ├── ui/                # UI组件
│   └── platform/          # 平台适配
├── resources/              # 资源文件
│   ├── icons/             # 图标
│   ├── styles/            # 样式表
│   └── translations/      # 国际化
├── tests/                  # 测试代码
├── third-party/            # 第三方库
└── README.md
```

## 文档索引

| 文档 | 说明 |
|------|------|
| [调研报告](docs/00-调研报告.md) | 桌面便签应用市场调研 |
| [开发计划](docs/01-开发计划.md) | 项目概述和基础计划 |
| [详细开发计划](docs/02-详细开发计划.md) | 包含 Markdown 功能的详细计划 |
| [详细设计文档](docs/03-详细设计文档.md) | 架构、数据库、类、UI 设计 |
| [模块化开发计划](docs/04-模块化开发计划.md) | 模块拆分和并行开发策略 |

## 快速开始

### 环境要求

- Qt 6.5+
- CMake 3.16+
- C++17 编译器
- SQLite 3

### 构建

```bash
mkdir build && cd build
cmake ..
make
```

## 开发进度

- [ ] M00: 项目基础
- [ ] M01: 数据层
- [ ] M02: Markdown服务
- [ ] M03: 核心业务层
- [ ] M04: 主窗口UI
- [ ] M05: 编辑器UI
- [ ] M06: 系统集成
- [ ] M07: 平台适配
- [ ] M08: 测试优化

## 飞书文档

- [调研报告](https://feishu.cn/docx/PFaId9CgXoXd8kxwhpMcEu5enje)
- [详细开发计划](https://feishu.cn/docx/IcSmdYfuiodlQsxLGFIcpoDEnZb)
- [详细设计文档](https://feishu.cn/docx/SzAKdd3Mno03rixjsRXcYVJ6nHf)
- [模块化开发计划](https://feishu.cn/docx/BOoAd7BStoofjExDQDxcXlZ9nqe)

---

🐕 二哈 | 2026年3月3日
