#!/bin/bash
# GTK GUI 进度同步问题 - 完整修复方案
# 基于项目详细分析.md的优化建议

echo "========================================="
echo "GTK GUI 进度同步修复脚本"
echo "========================================="
echo ""

cd /mnt/d/学校课程/linnux操作系统/yet-another-bt

# 备份原文件
echo "[1/5] 备份原文件..."
cp gui_gtk.c gui_gtk.c.backup_$(date +%Y%m%d_%H%M%S)

# 修复1: 添加缺失的外部变量声明
echo "[2/5] 添加外部变量声明..."
sed -i '/^extern Peer \*peer_head;$/a\
// 全局统计变量声明（来自torrent.c）\
extern float total_down_rate;    // 总下载速率(B/s)\
extern float total_up_rate;      // 总上传速率(B/s)\
extern int total_peers;          // 当前peer数量\
extern long long total_down;     // 累计下载量(B)\
extern long long total_up;       // 累计上传量(B)' gui_gtk.c

# 修复2: 检查是否成功添加
echo "[3/5] 验证修改..."
if grep -q "extern float total_down_rate" gui_gtk.c; then
    echo "✅ 外部变量声明添加成功"
else
    echo "❌ 外部变量声明添加失败"
    echo "请手动添加以下内容到 gui_gtk.c 的第33行之后:"
    echo ""
    echo "extern float total_down_rate;"
    echo "extern float total_up_rate;"
    echo "extern int total_peers;"
    echo "extern long long total_down;"
    echo "extern long long total_up;"
    echo ""
    exit 1
fi

# 重新编译
echo "[4/5] 重新编译..."
make clean
if make gtk 2>&1 | grep -q "error"; then
    echo "❌ 编译失败，请检查错误信息"
    make gtk
    exit 1
else
    echo "✅ 编译成功"
fi

# 验证可执行文件
echo "[5/5] 验证可执行文件..."
if [ -f "ttorrent-gtk" ]; then
    echo "✅ ttorrent-gtk 生成成功"
    ls -lh ttorrent-gtk
    echo ""
    echo "========================================="
    echo "修复完成！"
    echo "========================================="
    echo ""
    echo "运行命令: ./ttorrent-gtk 你的种子.torrent"
    echo ""
    echo "预期效果:"
    echo "  ✓ GTK窗口显示下载进度"
    echo "  ✓ 进度条每秒更新"
    echo "  ✓ 下载/上传速度实时显示"
    echo "  ✓ Peer数量动态更新"
    echo ""
else
    echo "❌ 可执行文件未生成"
    exit 1
fi
