CC=gcc
CFLAGS= -Iinclude -Wall -g -DDEBUG
GTK_CFLAGS= `pkg-config --cflags gtk+-3.0`
GTK_LIBS= `pkg-config --libs gtk+-3.0`

# 默认目标 - ncurses版本(终端界面)
ttorrent: main.o parse_metafile.o tracker.o bitfield.o message.o peer.o data.o policy.o torrent.o bterror.o log.o signal_hander.o bt_hash.o gui.o
	$(CC) -o $@ $^ -ldl -lssl -lcrypto -lncurses

# GTK版本(图形化窗口)
ttorrent-gtk: main_gtk.o parse_metafile.o tracker.o bitfield.o message.o peer.o data.o policy.o torrent.o bterror.o log.o signal_hander.o bt_hash.o gui_gtk.o
	$(CC) -o $@ $^ -ldl -lssl -lcrypto -lpthread $(GTK_LIBS)

# GTK主程序编译
main_gtk.o: main_gtk.c
	$(CC) $(CFLAGS) $(GTK_CFLAGS) -c main_gtk.c

# GTK GUI模块编译
gui_gtk.o: gui_gtk.c gui_gtk.h
	$(CC) $(CFLAGS) $(GTK_CFLAGS) -c gui_gtk.c

# 通用目标文件编译
%.o: %.c
	$(CC) $(CFLAGS) -c $<

.PHONY: clean all gtk

# 编译两个版本
all: ttorrent ttorrent-gtk

# 只编译GTK版本
gtk: ttorrent-gtk

clean:
	rm -rf *.o ttorrent ttorrent-gtk

check-syntax:
	gcc -o nul -S ${CHK_SOURCES}
