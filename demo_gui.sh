#!/bin/bash
# GUI功能演示脚本
# 用途: 创建一个简单的GUI功能演示（不需要实际下载）

set -e

echo "╔══════════════════════════════════════════════════════════════╗"
echo "║              Yet-Another-BT GUI 功能演示                      ║"
echo "╚══════════════════════════════════════════════════════════════╝"
echo ""

# 检查是否在支持GUI的终端
if [ -z "$TERM" ] || [ "$TERM" = "dumb" ]; then
    echo "错误: 当前终端不支持GUI"
    exit 1
fi

# 创建测试程序
cat > test_gui_demo.c << 'EOF'
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>
#include <locale.h>

// 模拟进度条
void draw_progress_bar(WINDOW *win, float percent, int y, int x, int width)
{
    int filled = (int)((percent / 100.0) * width);
    int i;
    
    wmove(win, y, x);
    waddch(win, '[');
    
    for(i = 0; i < width; i++) {
        if(i < filled) {
            waddch(win, ACS_CKBOARD);
        } else {
            waddch(win, ' ');
        }
    }
    
    waddch(win, ']');
}

int main()
{
    WINDOW *main_win;
    int i;
    
    // 设置本地化
    setlocale(LC_ALL, "");
    
    // 初始化ncurses
    initscr();
    noecho();
    cbreak();
    curs_set(0);
    
    // 启用颜色
    if(has_colors()) {
        start_color();
        init_pair(1, COLOR_GREEN, COLOR_BLACK);
        init_pair(2, COLOR_CYAN, COLOR_BLACK);
        init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    }
    
    // 创建窗口
    int height = 15;
    int width = 70;
    int start_y = (LINES - height) / 2;
    int start_x = (COLS - width) / 2;
    
    main_win = newwin(height, width, start_y, start_x);
    box(main_win, 0, 0);
    
    // 标题
    if(has_colors()) wattron(main_win, COLOR_PAIR(2) | A_BOLD);
    mvwprintw(main_win, 1, 2, "ttorrent 下载进度演示");
    if(has_colors()) wattroff(main_win, COLOR_PAIR(2) | A_BOLD);
    
    // 分隔线
    mvwhline(main_win, 2, 1, ACS_HLINE, 68);
    
    wrefresh(main_win);
    sleep(1);
    
    // 模拟下载进度
    for(i = 0; i <= 100; i += 5) {
        // 清除内容
        werase(main_win);
        box(main_win, 0, 0);
        
        // 标题
        if(has_colors()) wattron(main_win, COLOR_PAIR(2) | A_BOLD);
        mvwprintw(main_win, 1, 2, "ttorrent 下载进度演示");
        if(has_colors()) wattroff(main_win, COLOR_PAIR(2) | A_BOLD);
        
        mvwhline(main_win, 2, 1, ACS_HLINE, 68);
        
        // 进度条
        mvwprintw(main_win, 4, 2, "进度:");
        draw_progress_bar(main_win, (float)i, 4, 9, 50);
        
        if(has_colors()) wattron(main_win, COLOR_PAIR(3));
        mvwprintw(main_win, 4, 62, "%3d%%", i);
        if(has_colors()) wattroff(main_win, COLOR_PAIR(3));
        
        // piece信息
        int total_pieces = 1000;
        int downloaded = (total_pieces * i) / 100;
        mvwprintw(main_win, 5, 9, "(%d/%d pieces)", downloaded, total_pieces);
        
        // 速度信息
        float speed = 1.5 + ((float)(i % 10)) / 10.0;
        mvwprintw(main_win, 7, 2, "下载速度: ");
        if(has_colors()) wattron(main_win, COLOR_PAIR(1));
        wprintw(main_win, "%.2f MB/s", speed);
        if(has_colors()) wattroff(main_win, COLOR_PAIR(1));
        
        mvwprintw(main_win, 7, 30, "上传速度: ");
        if(has_colors()) wattron(main_win, COLOR_PAIR(1));
        wprintw(main_win, "%.2f KB/s", speed * 100);
        if(has_colors()) wattroff(main_win, COLOR_PAIR(1));
        
        // 已下载
        mvwprintw(main_win, 8, 2, "已下载: ");
        if(has_colors()) wattron(main_win, COLOR_PAIR(3));
        wprintw(main_win, "%.2f MB", (float)i * 10);
        if(has_colors()) wattroff(main_win, COLOR_PAIR(3));
        
        // Peer数量
        int peers = 5 + (i / 10);
        mvwprintw(main_win, 10, 2, "连接的Peers: ");
        if(has_colors()) wattron(main_win, COLOR_PAIR(3));
        wprintw(main_win, "%d", peers);
        if(has_colors()) wattroff(main_win, COLOR_PAIR(3));
        
        // 提示
        mvwprintw(main_win, 12, 2, "[按 Ctrl+C 退出]");
        
        wrefresh(main_win);
        usleep(200000);  // 200ms延迟
    }
    
    // 完成提示
    werase(main_win);
    box(main_win, 0, 0);
    
    if(has_colors()) wattron(main_win, COLOR_PAIR(1) | A_BOLD);
    mvwprintw(main_win, height/2, (width - 20)/2, "✅ 下载完成!");
    if(has_colors()) wattroff(main_win, COLOR_PAIR(1) | A_BOLD);
    
    mvwprintw(main_win, height/2 + 2, (width - 30)/2, "按任意键退出...");
    
    wrefresh(main_win);
    sleep(3);
    
    // 清理
    delwin(main_win);
    endwin();
    
    printf("\n演示完成！\n");
    
    return 0;
}
EOF

echo "正在编译GUI演示程序..."
gcc -o test_gui_demo test_gui_demo.c -lncurses

if [ $? -eq 0 ]; then
    echo "✅ 编译成功!"
    echo ""
    echo "运行演示程序..."
    sleep 1
    ./test_gui_demo
    
    echo ""
    echo "演示已结束。"
    echo "如果界面显示正常，说明GUI功能基本可用。"
else
    echo "❌ 编译失败"
    exit 1
fi

# 清理
rm -f test_gui_demo test_gui_demo.c

exit 0
