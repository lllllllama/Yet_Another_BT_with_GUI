#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <pthread.h>
#include "gui_gtk.h"
#include "parse_metafile.h"
#include "bitfield.h"
#include "data.h"
#include "peer.h"

// GUI组件
static GtkWidget *window = NULL;
static GtkWidget *progress_bar = NULL;
static GtkWidget *label_percent = NULL;
static GtkWidget *label_pieces = NULL;
static GtkWidget *label_down_speed = NULL;
static GtkWidget *label_up_speed = NULL;
static GtkWidget *label_downloaded = NULL;
static GtkWidget *label_peers = NULL;
static GtkTextBuffer *log_buffer = NULL;
static GtkWidget *log_view = NULL;

// GUI状态
static int gui_initialized = 0;
static pthread_mutex_t gui_mutex = PTHREAD_MUTEX_INITIALIZER;
static guint update_timer_id = 0;

// 外部变量声明
extern Bitmap *bitmap;
extern int pieces_length;
extern int piece_length;
extern int download_piece_num;
extern Peer *peer_head;
extern float total_down_rate;
extern float total_up_rate;
extern int total_peers;
extern long long total_down;
extern long long total_up;
extern float total_up_rate;
extern int total_peers;
extern long long total_down;
extern long long total_up;
extern Peer *peer_head;

// 格式化数据大小
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

// 格式化速度
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

// 统计peer数量
static int count_peers()
{
    int count = 0;
    Peer *p = peer_head;
    while(p != NULL) {
        count++;
        p = p->next;
    }
    return count;
}

// 计算总下载/上传速度
static void compute_total_rates(float *down_rate, float *up_rate)
{
    *down_rate = 0.0;
    *up_rate = 0.0;
    
    Peer *p = peer_head;
    while(p != NULL) {
        *down_rate += p->down_rate;
        *up_rate += p->up_rate;
        p = p->next;
    }
}

// 更新GUI显示（在GTK主线程中调用）
static gboolean update_gui_timeout(gpointer data)
{
    if(!gui_initialized) {
        return FALSE;
    }
    
    pthread_mutex_lock(&gui_mutex);
    
    // 计算进度
    float percent = 0.0;
    int total_pieces = pieces_length / 20;
    if(total_pieces > 0) {
        percent = (float)download_piece_num / total_pieces;
    }
    
    // 更新进度条
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), percent);
    
    // 更新百分比标签
    char percent_text[32];
    snprintf(percent_text, sizeof(percent_text), "%.1f%%", percent * 100);
    gtk_label_set_text(GTK_LABEL(label_percent), percent_text);
    
    // 更新piece信息
    char pieces_text[64];
    snprintf(pieces_text, sizeof(pieces_text), "Pieces: %d / %d", 
             download_piece_num, total_pieces);
    gtk_label_set_text(GTK_LABEL(label_pieces), pieces_text);
    
    // 更新速度信息
    float down_rate, up_rate;
    compute_total_rates(&down_rate, &up_rate);
    
    char speed_text[64];
    format_speed(down_rate, speed_text, sizeof(speed_text));
    char down_label[128];
    snprintf(down_label, sizeof(down_label), "下载速度: %s", speed_text);
    gtk_label_set_text(GTK_LABEL(label_down_speed), down_label);
    
    format_speed(up_rate, speed_text, sizeof(speed_text));
    char up_label[128];
    snprintf(up_label, sizeof(up_label), "上传速度: %s", speed_text);
    gtk_label_set_text(GTK_LABEL(label_up_speed), up_label);
    
    // 更新已下载数据
    long long downloaded = (long long)download_piece_num * piece_length;
    char size_text[64];
    format_size(downloaded, size_text, sizeof(size_text));
    char down_size_label[128];
    snprintf(down_size_label, sizeof(down_size_label), "已下载: %s", size_text);
    gtk_label_set_text(GTK_LABEL(label_downloaded), down_size_label);
    
    // 更新peer数量
    int peer_count = count_peers();
    char peers_text[64];
    snprintf(peers_text, sizeof(peers_text), "连接的Peers: %d", peer_count);
    gtk_label_set_text(GTK_LABEL(label_peers), peers_text);
    
    pthread_mutex_unlock(&gui_mutex);
    
    return TRUE; // 继续定时更新
}

// 窗口关闭回调
static void on_window_destroy(GtkWidget *widget, gpointer data)
{
    cleanup_gui_gtk();
    gtk_main_quit();
    exit(0); // 退出程序
}

// 初始化GUI
int init_gui_gtk(int argc, char *argv[])
{
    if(gui_initialized) {
        return 0;
    }
    
    // 初始化GTK
    gtk_init(&argc, &argv);
    
    // 创建主窗口
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Yet-Another-BT - BitTorrent下载器");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 500);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    g_signal_connect(window, "destroy", G_CALLBACK(on_window_destroy), NULL);
    
    // 创建垂直布局容器
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 15);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    
    // 标题标签
    GtkWidget *title_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title_label), 
        "<span size='large' weight='bold'>BitTorrent 下载进度</span>");
    gtk_box_pack_start(GTK_BOX(vbox), title_label, FALSE, FALSE, 0);
    
    // 分隔线
    GtkWidget *separator1 = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(vbox), separator1, FALSE, FALSE, 0);
    
    // 进度条区域
    GtkWidget *progress_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    
    GtkWidget *progress_label = gtk_label_new("进度:");
    gtk_box_pack_start(GTK_BOX(progress_box), progress_label, FALSE, FALSE, 0);
    
    progress_bar = gtk_progress_bar_new();
    gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(progress_bar), FALSE);
    gtk_widget_set_size_request(progress_bar, 400, 25);
    gtk_box_pack_start(GTK_BOX(progress_box), progress_bar, TRUE, TRUE, 0);
    
    label_percent = gtk_label_new("0.0%");
    gtk_box_pack_start(GTK_BOX(progress_box), label_percent, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(vbox), progress_box, FALSE, FALSE, 0);
    
    // Piece信息
    label_pieces = gtk_label_new("Pieces: 0 / 0");
    gtk_widget_set_halign(label_pieces, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(vbox), label_pieces, FALSE, FALSE, 0);
    
    // 统计信息区域
    GtkWidget *stats_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(stats_grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(stats_grid), 20);
    
    // 下载速度
    label_down_speed = gtk_label_new("下载速度: 0 B/s");
    gtk_widget_set_halign(label_down_speed, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(stats_grid), label_down_speed, 0, 0, 1, 1);
    
    // 上传速度
    label_up_speed = gtk_label_new("上传速度: 0 B/s");
    gtk_widget_set_halign(label_up_speed, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(stats_grid), label_up_speed, 1, 0, 1, 1);
    
    // 已下载
    label_downloaded = gtk_label_new("已下载: 0 B");
    gtk_widget_set_halign(label_downloaded, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(stats_grid), label_downloaded, 0, 1, 1, 1);
    
    // Peer数量
    label_peers = gtk_label_new("连接的Peers: 0");
    gtk_widget_set_halign(label_peers, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(stats_grid), label_peers, 1, 1, 1, 1);
    
    gtk_box_pack_start(GTK_BOX(vbox), stats_grid, FALSE, FALSE, 0);
    
    // 分隔线
    GtkWidget *separator2 = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(vbox), separator2, FALSE, FALSE, 0);
    
    // 日志区域
    GtkWidget *log_label = gtk_label_new("下载日志:");
    gtk_widget_set_halign(log_label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(vbox), log_label, FALSE, FALSE, 0);
    
    // 创建可滚动的文本视图
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(scrolled_window, -1, 200);
    
    log_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(log_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(log_view), GTK_WRAP_WORD);
    log_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(log_view));
    
    gtk_container_add(GTK_CONTAINER(scrolled_window), log_view);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);
    
    // 显示所有组件
    gtk_widget_show_all(window);
    
    // 添加初始日志
    gui_log("GUI已初始化");
    gui_log("开始BitTorrent下载...");
    
    // 设置定时更新（每秒更新一次）
    update_timer_id = g_timeout_add(1000, update_gui_timeout, NULL);
    
    gui_initialized = 1;
    return 0;
}

// 更新GUI（被下载线程调用）
void update_gui_gtk()
{
    // 实际更新由定时器处理，这里只是占位符
    // 可以在这里触发立即更新
}

// 添加日志消息
void gui_log(const char *message)
{
    if(!gui_initialized || log_buffer == NULL) {
        return;
    }
    
    pthread_mutex_lock(&gui_mutex);
    
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(log_buffer, &iter);
    
    char timestamp[64];
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "[%H:%M:%S] ", tm_info);
    
    gtk_text_buffer_insert(log_buffer, &iter, timestamp, -1);
    gtk_text_buffer_insert(log_buffer, &iter, message, -1);
    gtk_text_buffer_insert(log_buffer, &iter, "\n", -1);
    
    // 自动滚动到底部
    GtkTextMark *mark = gtk_text_buffer_get_insert(log_buffer);
    gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(log_view), mark, 0.0, TRUE, 0.0, 1.0);
    
    pthread_mutex_unlock(&gui_mutex);
}

// 清理GUI资源
void cleanup_gui_gtk()
{
    if(!gui_initialized) {
        return;
    }
    
    gui_log("正在关闭GUI...");
    
    // 移除定时器
    if(update_timer_id > 0) {
        g_source_remove(update_timer_id);
        update_timer_id = 0;
    }
    
    gui_initialized = 0;
}

// 检查GUI是否已初始化
int is_gui_initialized_gtk()
{
    return gui_initialized;
}

// ========== Compatibility Functions ==========
// For torrent.c to call using ncurses function names

// Check if GUI is initialized (ncurses compatibility)
int is_gui_initialized()
{
    return is_gui_initialized_gtk();
}

// Update GUI (ncurses compatibility)
void update_gui()
{
    update_gui_gtk();
}
