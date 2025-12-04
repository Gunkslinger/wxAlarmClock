# C++ project
C = clang++
DEBUG_SYMBOLS = -g -DDEBUG
CFLAGS = -I/usr/lib/wx/include/gtk3-unicode-3.2\
                 -I/usr/include/wx-3.2\
                -DWXUSINGDLL -D__WXGTK3__ -D__WXGTK__ -D_FILE_OFFSET_BITS=64 #-ferror-limit=128
LDFLAGS = -pthread -lXss -lX11 -lwx_gtk3u_xrc-3.2 -lwx_gtk3u_html-3.2 -lwx_gtk3u_qa-3.2\
                        -lwx_gtk3u_core-3.2 -lwx_baseu_xml-3.2 -lwx_baseu_net-3.2 -lwx_baseu-3.2
OBJS = main.o mainwindow.o pulse_audio.o alarms_dlg.o

all: wxAlarmClock

main.o: main.cpp mainwindow.hpp
		${C} ${DEBUG_SYMBOLS} ${CFLAGS} -c main.cpp -o main.o

mainwindow.o: mainwindow.cpp mainwindow.hpp enums.hpp pulse_audio.cpp pulse_audio.hpp
		${C} ${DEBUG_SYMBOLS} ${CFLAGS} -c mainwindow.cpp -o mainwindow.o

pulse_audio.o: pulse_audio.cpp pulse_audio.hpp
		${C} ${DEBUG_SYMBOLS} ${CFLAGS} -c pulse_audio.cpp -o pulse_audio.o

alarms_dlg.o: alarms_dlg.cpp alarms_dlg.hpp
		$(C) $(DEBUG_SYMBOLS) $(CFLAGS) -c alarms_dlg.cpp -o alarms_dlg.o

wxAlarmClock: ${OBJS}
		${C} ${DEBUG_SYMBOLS} ${LDFLAGS} ${OBJS} -o wxAlarmClock
