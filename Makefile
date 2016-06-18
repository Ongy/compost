TARGET=compost
LDFLAGS=-lweston -lwayland-server -lpixman-1
CFLAGS=-fdiagnostics-color=auto -Wall -Wextra -ggdb3
CFLAGS+=-I/usr/include/pixman-1 -Iinclude

XDG-CURRENT=xdg-shell-unstable-v5.xml
XDG-HEADER=include/xdg-shell-server-protocol.h
XDG-SOURCE=src/xdg-shell-server-protocol.c

GENERATED=$(XDG-SOURCE)
OBJECTS=main.o wlsignals.o xdg-shell.o xdg-surface.o xdg-popup.o
SOBJECTS=$(addprefix src/, $(OBJECTS))

.PHONY: run clean fresh

all:  $(XDG-HEADER) $(TARGET)


$(XDG-HEADER): $(XDG-CURRENT)
	<$< wayland-scanner server-header >$@

$(XDG-SOURCE): $(XDG-CURRENT) $(XDG-HEADER)
	<$< wayland-scanner code >$@

src/xdg-shell.o: $(XDG-HEADER)
src/xdg-surface.o: $(XDG-HEADER)
src/xdg-popup.o: $(XDG-HEADER)

$(TARGET): $(SOBJECTS) $(GENERATED)
	$(CC) -o $(TARGET) $(LDFLAGS) $(SOBJECTS) $(GENERATED)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
	rm -f $(SOBJECTS)
	rm -f $(XDG-HEADER)
	rm -f $(GENERATED)


fresh: clean all
