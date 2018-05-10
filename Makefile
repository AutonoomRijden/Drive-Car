TARGET ?= driveCar
LIBS = -lusb-1.0
CC = gcc
CFLAGS = -g -Wall

.PHONE: default all clean

default: $(TARGET)
all: default


OBJS = $(patsubst %.c, %.o, $(wildcard *.c))
HDRS = $(wildcard *.h)

%.o: %.c $(HDRS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJS)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -Wall $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)
