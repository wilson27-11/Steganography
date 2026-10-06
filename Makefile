CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -g
TARGET  = a.out
SRCS    = main.c encode.c decode.c common.c
OBJS    = $(SRCS:.c=.o)
HDRS    = common.h types.h encode.h decode.h

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c $(HDRS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) stego.bmp

.PHONY: all clean
