#!/bin/bash
# GUI自动化测试脚本
# 用途: 验证GUI功能的编译和基础功能

set -e  # 遇到错误立即退出

echo "╔══════════════════════════════════════════════════════════════╗"
echo "║        Yet-Another-BT GUI 自动化测试脚本                     ║"
echo "╚══════════════════════════════════════════════════════════════╝"
echo ""

# 颜色定义
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 测试计数器
PASS_COUNT=0
FAIL_COUNT=0

# 测试结果函数
pass_test() {
    echo -e "${GREEN}✅ PASS${NC}: $1"
    ((PASS_COUNT++))
}

fail_test() {
    echo -e "${RED}❌ FAIL${NC}: $1"
    ((FAIL_COUNT++))
}

warn_test() {
    echo -e "${YELLOW}⚠️  WARN${NC}: $1"
}

# ============================================================
# 测试1: 环境检查
# ============================================================
echo "=== 测试1: 环境检查 ==="

# 检查ncurses库
if ldconfig -p | grep -q libncurses; then
    pass_test "ncurses库已安装"
else
    fail_test "ncurses库未安装"
    echo "请运行: sudo apt install libncurses5-dev"
    exit 1
fi

# 检查OpenSSL
if ldconfig -p | grep -q libssl; then
    pass_test "OpenSSL库已安装"
else
    fail_test "OpenSSL库未安装"
    echo "请运行: sudo apt install libssl-dev"
    exit 1
fi

echo ""

# ============================================================
# 测试2: 编译测试
# ============================================================
echo "=== 测试2: 编译测试 ==="

# 清理旧文件
if make clean > /dev/null 2>&1; then
    pass_test "make clean 成功"
else
    fail_test "make clean 失败"
fi

# 完整编译
echo "正在编译..."
if make > /tmp/make_output.log 2>&1; then
    pass_test "make 编译成功"
else
    fail_test "make 编译失败"
    echo "错误日志:"
    cat /tmp/make_output.log
    exit 1
fi

# 检查生成的可执行文件
if [ -f "./ttorrent" ]; then
    pass_test "ttorrent 可执行文件已生成"
    ls -lh ttorrent
else
    fail_test "ttorrent 可执行文件未生成"
    exit 1
fi

echo ""

# ============================================================
# 测试3: 依赖链接检查
# ============================================================
echo "=== 测试3: 依赖链接检查 ==="

# 检查ncurses链接
if ldd ttorrent | grep -q libncurses; then
    pass_test "ncurses库已正确链接"
    ldd ttorrent | grep ncurses
else
    fail_test "ncurses库未链接"
fi

# 检查ssl链接
if ldd ttorrent | grep -q libssl; then
    pass_test "OpenSSL库已正确链接"
else
    fail_test "OpenSSL库未链接"
fi

# 检查crypto链接
if ldd ttorrent | grep -q libcrypto; then
    pass_test "crypto库已正确链接"
else
    fail_test "crypto库未链接"
fi

echo ""

# ============================================================
# 测试4: 代码质量检查
# ============================================================
echo "=== 测试4: 代码质量检查 ==="

# 检查gui.c中的关键函数
if grep -q "int init_gui()" gui.c; then
    pass_test "init_gui() 函数存在"
else
    fail_test "init_gui() 函数不存在"
fi

if grep -q "void update_gui()" gui.c; then
    pass_test "update_gui() 函数存在"
else
    fail_test "update_gui() 函数不存在"
fi

if grep -q "void cleanup_gui()" gui.c; then
    pass_test "cleanup_gui() 函数存在"
else
    fail_test "cleanup_gui() 函数不存在"
fi

# 检查main.c中的GUI调用
if grep -q "init_gui()" main.c; then
    pass_test "main.c 调用了 init_gui()"
else
    warn_test "main.c 未调用 init_gui()"
fi

if grep -q "cleanup_gui()" main.c; then
    pass_test "main.c 调用了 cleanup_gui()"
else
    warn_test "main.c 未调用 cleanup_gui()"
fi

echo ""

# ============================================================
# 测试5: 符号表检查
# ============================================================
echo "=== 测试5: GUI符号检查 ==="

# 检查GUI相关符号
if nm ttorrent | grep -q "init_gui"; then
    pass_test "init_gui 符号存在"
else
    fail_test "init_gui 符号不存在"
fi

if nm ttorrent | grep -q "update_gui"; then
    pass_test "update_gui 符号存在"
else
    fail_test "update_gui 符号不存在"
fi

if nm ttorrent | grep -q "cleanup_gui"; then
    pass_test "cleanup_gui 符号存在"
else
    fail_test "cleanup_gui 符号不存在"
fi

echo ""

# ============================================================
# 测试6: 文件检查
# ============================================================
echo "=== 测试6: 项目文件检查 ==="

FILES=("gui.c" "gui.h" "main.c" "Makefile" "USAGE.md" "GUI_USAGE.md")
for file in "${FILES[@]}"; do
    if [ -f "$file" ]; then
        pass_test "$file 存在"
    else
        fail_test "$file 不存在"
    fi
done

echo ""

# ============================================================
# 测试总结
# ============================================================
echo "╔══════════════════════════════════════════════════════════════╗"
echo "║                       测试总结                                ║"
echo "╚══════════════════════════════════════════════════════════════╝"
echo ""
echo -e "${GREEN}通过: $PASS_COUNT${NC}"
echo -e "${RED}失败: $FAIL_COUNT${NC}"
echo ""

if [ $FAIL_COUNT -eq 0 ]; then
    echo -e "${GREEN}✅ 所有测试通过!${NC}"
    echo ""
    echo "下一步:"
    echo "1. 准备测试种子文件"
    echo "2. 运行: ./ttorrent your-test.torrent"
    echo "3. 观察GUI界面显示是否正常"
    echo ""
    exit 0
else
    echo -e "${RED}❌ 存在失败的测试，请修复后重试${NC}"
    exit 1
fi
