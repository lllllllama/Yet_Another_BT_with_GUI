#!/bin/bash
# 修复 GTK GUI 进度同步问题

echo "正在修复 gui_gtk.c..."

# 备份原文件
cp gui_gtk.c gui_gtk.c.backup

# 在声明部分添加缺失的外部变量
sed -i '33a extern float total_down_rate;\nextern float total_up_rate;\nextern int total_peers;\nextern long long total_down;\nextern long long total_up;' gui_gtk.c

echo "修复完成！请重新编译："
echo "make clean && make gtk"
