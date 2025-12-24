#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <gtk/gtk.h>

#include "data.h"
#include "tracker.h"
#include "bitfield.h"
#include "torrent.h"
#include "parse_metafile.h"
#include "signal_hander.h"
#include "policy.h"
#include "log.h"
#include "gui_gtk.h"

// 下载线程函数
void* download_thread(void* arg)
{
    char *metafile = (char*)arg;
    int ret;
    
    // 设置信号处理函数
    ret = set_signal_hander();
    if(ret != 0) {
        gui_log("错误: 设置信号处理失败");
        return NULL;
    }
    
    // 解析种子文件
    gui_log("正在解析种子文件...");
    ret = parse_metafile(metafile);
    if(ret != 0) {
        gui_log("错误: 解析种子文件失败");
        return NULL;
    }
    gui_log("种子文件解析完成");
    
    // 初始化非阻塞peer
    init_unchoke_peers();
    gui_log("Peer列表初始化完成");
    
    // 创建用于保存下载数据的文件
    gui_log("正在创建下载文件...");
    ret = create_files();
    if(ret != 0) {
        gui_log("错误: 创建文件失败");
        return NULL;
    }
    gui_log("下载文件创建完成");
    
    // 创建位图
    gui_log("正在创建位图...");
    ret = create_bitfield();
    if(ret != 0) {
        gui_log("错误: 创建位图失败");
        return NULL;
    }
    gui_log("位图创建完成");
    
    // 创建缓冲区
    gui_log("正在创建数据缓冲区...");
    ret = create_btcache();
    if(ret != 0) {
        gui_log("错误: 创建缓冲区失败");
        return NULL;
    }
    gui_log("缓冲区创建完成 (16MB)");
    
    // 开始下载
    gui_log("开始连接tracker和peers...");
    download_upload_with_peers();
    
    gui_log("下载完成!");
    
    // 做一些清理工作
    do_clear_work();
    
    return NULL;
}

int main(int argc, char *argv[])
{
    if(argc != 2) {
        printf("用法: %s <种子文件>\n", argv[0]);
        printf("示例: %s example.torrent\n", argv[0]);
        exit(-1);
    }
    
    // 初始化GTK GUI
    if(init_gui_gtk(argc, argv) != 0) {
        printf("GUI初始化失败，退出程序\n");
        return -1;
    }
    
    // 创建下载线程
    pthread_t download_tid;
    if(pthread_create(&download_tid, NULL, download_thread, argv[1]) != 0) {
        gui_log("错误: 创建下载线程失败");
        cleanup_gui_gtk();
        return -1;
    }
    
    // 进入GTK主循环
    gtk_main();
    
    // 等待下载线程结束
    pthread_cancel(download_tid);
    pthread_join(download_tid, NULL);
    
    // 清理GUI资源
    cleanup_gui_gtk();
    
    return 0;
}
