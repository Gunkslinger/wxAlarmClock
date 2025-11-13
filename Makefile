# C++ project
C = clang++
DEBUG = -g
CFLAGS = -I/usr/lib/wx/include/gtk3-unicode-3.2\
                 -I/usr/include/wx-3.2\
                -DWXUSINGDLL -D__WXGTK3__ -D__WXGTK__ -D_FILE_OFFSET_BITS=64 -ferror-limit=128
LDFLAGS = -pthread -lwx_gtk3u_xrc-3.2 -lwx_gtk3u_html-3.2 -lwx_gtk3u_qa-3.2\
                        -lwx_gtk3u_core-3.2 -lwx_baseu_xml-3.2 -lwx_baseu_net-3.2 -lwx_baseu-3.2
OBJS = main.o mainwindow.o

all: wxAlarmClock

main.o: main.cpp mainwindow.hpp
		${C} ${DEBUG} ${CFLAGS} -c main.cpp -o main.o

mainwindow.o: mainwindow.cpp mainwindow.hpp
		${C} ${DEBUG} ${CFLAGS} -c mainwindow.cpp -o mainwindow.o

wxAlarmClock: ${OBJS}
		${C} ${DEBUG} ${LDFLAGS} ${OBJS} -o wxAlarmClock
