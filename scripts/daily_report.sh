#!/bin/bash
# 每日工作日报生成脚本
# 执行时间：每天凌晨 4:00
# 生成位置：notes/日报/YYYY-MM-DD.md

WORKSPACE="/home/ubuntu/.openclaw/workspace"
MEMORY_DIR="$WORKSPACE/memory"
REPORT_DIR="$WORKSPACE/notes/日报"
CODE_DIR="$WORKSPACE/code"
LOG_FILE="$WORKSPACE/logs/daily_report.log"

# 创建日报目录
mkdir -p "$REPORT_DIR"
mkdir -p "$(dirname "$LOG_FILE")"

# 获取昨天的日期
YESTERDAY=$(date -d "yesterday" +%Y-%m-%d)

# 日志函数
log() {
    echo "[$(date '+%Y-%m-%d %H:%M:%S')] $1" >> "$LOG_FILE"
}

log "========== 开始生成 $YESTERDAY 的工作日报 =========="

# 日报文件
REPORT_FILE="$REPORT_DIR/$YESTERDAY.md"

echo "# 工作日报 - $YESTERDAY" > "$REPORT_FILE"
echo "" >> "$REPORT_FILE"
echo "> 自动生成时间：$(date '+%Y-%m-%d %H:%M:%S')" >> "$REPORT_FILE"
echo "> 生成者：二哈 🐕" >> "$REPORT_FILE"
echo "" >> "$REPORT_FILE"
echo "---" >> "$REPORT_FILE"
echo "" >> "$REPORT_FILE"

# 1. 收集 memory 目录的记录
echo "## 📝 今日记录" >> "$REPORT_FILE"
echo "" >> "$REPORT_FILE"
if [ -f "$MEMORY_DIR/$YESTERDAY.md" ]; then
    cat "$MEMORY_DIR/$YESTERDAY.md" >> "$REPORT_FILE"
    log "已读取 memory/$YESTERDAY.md"
else
    echo "（无记录）" >> "$REPORT_FILE"
    log "memory/$YESTERDAY.md 不存在"
fi
echo "" >> "$REPORT_FILE"
echo "---" >> "$REPORT_FILE"
echo "" >> "$REPORT_FILE"

# 2. 收集 Git 提交记录
echo "## 🔧 Git 提交记录" >> "$REPORT_FILE"
echo "" >> "$REPORT_FILE"

if [ -d "$CODE_DIR" ]; then
    for repo in "$CODE_DIR"/*/; do
        if [ -d "$repo/.git" ]; then
            repo_name=$(basename "$repo")
            echo "### $repo_name" >> "$REPORT_FILE"
            echo "" >> "$REPORT_FILE"
            
            cd "$repo" || continue
            commits=$(git log --since="$YESTERDAY 00:00:00" --until="$YESTERDAY 23:59:59" --pretty=format:"- %s (%h) - %an" 2>/dev/null)
            
            if [ -n "$commits" ]; then
                echo "$commits" >> "$REPORT_FILE"
                log "已收集 $repo_name 的 git 提交记录"
            else
                echo "（无提交）" >> "$REPORT_FILE"
                log "$repo_name 昨天无提交"
            fi
            echo "" >> "$REPORT_FILE"
        fi
    done
fi
echo "" >> "$REPORT_FILE"
echo "---" >> "$REPORT_FILE"
echo "" >> "$REPORT_FILE"

# 3. 文件变更统计
echo "## 📁 文件变更" >> "$REPORT_FILE"
echo "" >> "$REPORT_FILE"

if [ -d "$CODE_DIR" ]; then
    for repo in "$CODE_DIR"/*/; do
        if [ -d "$repo/.git" ]; then
            repo_name=$(basename "$repo")
            cd "$repo" || continue
            
            stats=$(git log --since="$YESTERDAY 00:00:00" --until="$YESTERDAY 23:59:59" --stat --pretty=format:"" 2>/dev/null | grep -E "file|insertion|deletion" | head -5)
            
            if [ -n "$stats" ]; then
                echo "**$repo_name**:" >> "$REPORT_FILE"
                echo '```' >> "$REPORT_FILE"
                echo "$stats" >> "$REPORT_FILE"
                echo '```' >> "$REPORT_FILE"
                echo "" >> "$REPORT_FILE"
            fi
        fi
    done
fi
echo "" >> "$REPORT_FILE"
echo "---" >> "$REPORT_FILE"
echo "" >> "$REPORT_FILE"

# 4. 群消息（预留）
echo "## 💬 群消息摘要" >> "$REPORT_FILE"
echo "" >> "$REPORT_FILE"
echo "（需手动补充或后续接入）" >> "$REPORT_FILE"
echo "" >> "$REPORT_FILE"
echo "---" >> "$REPORT_FILE"
echo "" >> "$REPORT_FILE"

# 5. 状态标记
echo "## ✅ 日报状态" >> "$REPORT_FILE"
echo "" >> "$REPORT_FILE"
echo "- [x] 自动生成" >> "$REPORT_FILE"
echo "- [ ] 已同步飞书" >> "$REPORT_FILE"
echo "" >> "$REPORT_FILE"

log "日报已生成：$REPORT_FILE"
log "========== 完成 =========="

echo "日报已生成: $REPORT_FILE"
