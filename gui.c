#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <locale.h>
#include "gui.h"
#include "parse_metafile.h"
#include "bitfield.h"
#include "data.h"
#include "peer.h"

// GUI初始化标志
static int gui_initialized = 0;

// 窗口指针
static WINDOW *main_win = NULL;

// 绘制进度条
// percent: 进度百分比(0-100)
// y, x: 进度条起始位置
// width: 进度条宽度
static void draw_progress_bar(WINDOW *win, float percent, int y, int x, int width)
{
    int filled = (int)((percent / 100.0) * width);
    int i;
    
    wmove(win, y, x);
    waddch(win, '[');
    
    for(i = 0; i < width; i++) {
        if(i < filled) {
            waddch(win, ACS_CKBOARD);  // 实心块
        } else {
            waddch(win, ' ');  // 空格
        }
    }
    
    waddch(win, ']');
}

// 格式化数据大小(字节转换为KB/MB/GB)
static void format_size(long long bytes, char *output, int output_size)
{
    double size = bytes;
    
    if(size < 1024) {
        snprintf(output, output_size, "%.0f B", size);
    } else if(size < 1024 * 1024) {
        snprintf(output, output_size, "%.2f KB", size / 1024);
    } else if(size < 1024 * 1024 * 1024) {
        snprintf(output, output_size, "%.2f MB", size / (1024 * 1024));
    } else {
        snprintf(output, output_size, "%.2f GB", size / (1024 * 1024 * 1024));
    }
}

// 格式化速度(B/s转换为KB/s或MB/s)
static void format_speed(float bytes_per_sec, char *output, int output_size)
{
    if(bytes_per_sec < 1024) {
        snprintf(output, output_size, "%.2f B/s", bytes_per_sec);
    } else if(bytes_per_sec < 1024 * 1024) {
        snprintf(output, output_size, "%.2f KB/s", bytes_per_sec / 1024);
    } else {
        snprintf(output, output_size, "%.2f MB/s", bytes_per_sec / (1024 * 1024));
    }
}

// 初始化GUI界面
int init_gui()
{
    if(gui_initialized) {
        return 0;  // 已经初始化
    }
    
    // 设置本地化,支持UTF-8
    setlocale(LC_ALL, "");
    
    // 初始化ncurses
    initscr();
    
    // 不回显输入
    noecho();
    
    // 禁用行缓冲
    cbreak();
    
    // 隐藏光标
    curs_set(0);
    
    // 启用颜色(如果终端支持)
    if(has_colors()) {
        start_color();
        init_pair(1, COLOR_GREEN, COLOR_BLACK);   // 进度条颜色
        init_pair(2, COLOR_CYAN, COLOR_BLACK);    // 标题颜色
        init_pair(3, COLOR_YELLOW, COLOR_BLACK);  // 数据颜色
    }
    
    // 创建主窗口
    int height = 15;
    int width = 70;
    int start_y = (LINES - height) / 2;
    int start_x = (COLS - width) / 2;
    
    main_win = newwin(height, width, start_y, start_x);
    if(main_win == NULL) {
        endwin();
        return -1;
    }
    
    // 启用窗口边框
    box(main_win, 0, 0);
    
    // 刷新窗口
    wrefresh(main_win);
    
    gui_initialized = 1;
    return 0;
}

// 更新GUI显示
void update_gui()
{
    if(!gui_initialized || main_win == NULL) {
        return;
    }
    
    // 清除窗口内容(保留边框)
    werase(main_win);
    box(main_win, 0, 0);
    
    // 标题
    if(has_colors()) wattron(main_win, COLOR_PAIR(2) | A_BOLD);
    mvwprintw(main_win, 1, 2, "ttorrent 下载进度");
    if(has_colors()) wattroff(main_win, COLOR_PAIR(2) | A_BOLD);
    
    // 分隔线
    mvwhline(main_win, 2, 1, ACS_HLINE, 68);
    
    // 计算进度百分比
    float percent = 0.0;
    int total_pieces = pieces_length / 20;
    if(total_pieces > 0) {
        percent = (float)download_piece_num / total_pieces * 100.0;
    }
    
    // 显示进度条
    mvwprintw(main_win, 4, 2, "进度:");
    draw_progress_bar(main_win, percent, 4, 9, 50);
    
    // 显示进度百分比和piece信息
    if(has_colors()) wattron(main_win, COLOR_PAIR(3));
    mvwprintw(main_win, 4, 62, "%.1f%%", percent);
    mvwprintw(main_win, 5, 9, "(%d/%d pieces)", download_piece_num, total_pieces);
    if(has_colors()) wattroff(main_win, COLOR_PAIR(3));
    
    // 下载速度
    char down_speed[32], up_speed[32];
    format_speed(total_down_rate, down_speed, sizeof(down_speed));
    format_speed(total_up_rate, up_speed, sizeof(up_speed));
    
    mvwprintw(main_win, 7, 2, "下载速度: ");
    if(has_colors()) wattron(main_win, COLOR_PAIR(1));
    wprintw(main_win, "%s", down_speed);
    if(has_colors()) wattroff(main_win, COLOR_PAIR(1));
    
    mvwprintw(main_win, 7, 30, "上传速度: ");
    if(has_colors()) wattron(main_win, COLOR_PAIR(1));
    wprintw(main_win, "%s", up_speed);
    if(has_colors()) wattroff(main_win, COLOR_PAIR(1));
    
    // 已下载/上传数据量
    long long downloaded = (long long)download_piece_num * piece_length;
    long long uploaded = 0;  // 暂时没有总上传量的全局变量
    
    char down_size[32], up_size[32];
    format_size(downloaded, down_size, sizeof(down_size));
    format_size(uploaded, up_size, sizeof(up_size));
    
    mvwprintw(main_win, 8, 2, "已下载: ");
    if(has_colors()) wattron(main_win, COLOR_PAIR(3));
    wprintw(main_win, "%s", down_size);
    if(has_colors()) wattroff(main_win, COLOR_PAIR(3));
    
    // 连接的Peer数量
    mvwprintw(main_win, 10, 2, "连接的Peers: ");
    if(has_colors()) wattron(main_win, COLOR_PAIR(3));
    wprintw(main_win, "%d", total_peers);
    if(has_colors()) wattroff(main_win, COLOR_PAIR(3));
    
    // 提示信息
    mvwprintw(main_win, 12, 2, "[按 Ctrl+C 退出]");
    
    // 刷新窗口
    wrefresh(main_win);
}

// 清理GUI资源
void cleanup_gui()
{
    if(!gui_initialized) {
        return;
    }
    
    // 删除窗口
    if(main_win != NULL) {
        delwin(main_win);
        main_win = NULL;
    }
    
    // 结束ncurses
    endwin();
    
    gui_initialized = 0;
}

// 检查GUI是否已初始化
int is_gui_initialized()
{
    return gui_initialized;
}
