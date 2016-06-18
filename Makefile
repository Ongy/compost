TARGET=compost
LDFLAGS=-lweston -lwayland-server
CFLAGS=-fdiagnostics-color=auto -Wall -Wextra -g
CFLAGS+=-I/usr/include/pixman-1

OBJECTS=src/main.o

$(TARGET): $(OBJECTS)
	$(CC) -o $(TARGET) $(LDFLAGS) $(OBJECTS)

run: $(TARGET)
	./$(TARGET)
