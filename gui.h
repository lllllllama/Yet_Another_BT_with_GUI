#ifndef GUI_H
#define GUI_H

// GUI刷新频率(秒)
#define GUI_REFRESH_INTERVAL 1

// 初始化GUI界面
int init_gui();

// 更新GUI显示(显示下载进度、速度等信息)
void update_gui();

// 清理GUI资源
void cleanup_gui();

// 检查GUI是否已初始化
int is_gui_initialized();

#endif
