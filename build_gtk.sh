#!/bin/bash
# GTK GUI 编译和测试脚本

set -e

echo "╔══════════════════════════════════════════════════════════════╗"
echo "║        Yet-Another-BT GTK图形界面 编译脚本                   ║"
echo "╚══════════════════════════════════════════════════════════════╝"
echo ""

# 颜色定义
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'

# 检查GTK3安装
echo "=== 1. 检查依赖 ==="
if pkg-config --exists gtk+-3.0; then
    GTK_VERSION=$(pkg-config --modversion gtk+-3.0)
    echo -e "${GREEN}✅ GTK+3 已安装 (版本: $GTK_VERSION)${NC}"
else
    echo -e "${RED}❌ GTK+3 未安装${NC}"
    echo ""
    echo "请安装GTK+3开发库:"
    echo "  Ubuntu/Debian: sudo apt install libgtk-3-dev"
    echo "  Fedora/RHEL:   sudo dnf install gtk3-devel"
    echo "  macOS:         brew install gtk+3"
    exit 1
fi

# 检查OpenSSL
if pkg-config --exists openssl || ldconfig -p | grep -q libssl; then
    echo -e "${GREEN}✅ OpenSSL 已安装${NC}"
else
    echo -e "${RED}❌ OpenSSL 未安装${NC}"
    echo "  Ubuntu/Debian: sudo apt install libssl-dev"
    exit 1
fi

echo ""

# 清理
echo "=== 2. 清理旧文件 ==="
make -f Makefile.new clean 2>/dev/null || true
echo -e "${GREEN}✅ 清理完成${NC}"
echo ""

# 编译
echo "=== 3. 编译GTK版本 ==="
echo "编译命令: make -f Makefile.new gtk"
echo ""

if make -f Makefile.new gtk; then
    echo ""
    echo -e "${GREEN}✅ 编译成功!${NC}"
else
    echo ""
    echo -e "${RED}❌ 编译失败${NC}"
    exit 1
fi

echo ""

# 检查生成的文件
echo "=== 4. 验证生成文件 ==="
if [ -f "./ttorrent-gtk" ]; then
    echo -e "${GREEN}✅ ttorrent-gtk 已生成${NC}"
    ls -lh ttorrent-gtk
    
    echo ""
    echo "依赖库检查:"
    ldd ttorrent-gtk | grep -E "gtk|gdk|glib|ssl|crypto" | head -10
else
    echo -e "${RED}❌ ttorrent-gtk 未生成${NC}"
    exit 1
fi

echo ""
echo "╔══════════════════════════════════════════════════════════════╗"
echo "║                    编译成功!                                  ║"
echo "╚══════════════════════════════════════════════════════════════╝"
echo ""
echo "运行方式:"
echo "  ./ttorrent-gtk your-file.torrent"
echo ""
echo "示例:"
echo "  ./ttorrent-gtk test.torrent"
echo ""
echo -e "${YELLOW}注意: 确保在有图形界面的环境中运行${NC}"
echo ""
