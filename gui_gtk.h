#ifndef GUI_GTK_H
#define GUI_GTK_H

// 初始化GTK GUI界面
int init_gui_gtk(int argc, char *argv[]);

// 更新GUI显示
void update_gui_gtk();

// 添加日志消息到GUI
void gui_log(const char *message);

// 清理GUI资源
void cleanup_gui_gtk();

// 检查GUI是否已初始化
int is_gui_initialized_gtk();

#endif
