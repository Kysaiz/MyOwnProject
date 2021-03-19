CC=arm-linux-gcc

T=./BIN/main

override CONFIG += -I./INC -L./LIB -ljpeg -lfont -lm -lpthread

SRCS=$(wildcard ./SRC/*.c)
OBJS=$(patsubst %.c, %.o, $(SRCS))

$(T):$(OBJS)
	$(CC) -o $@ $^ $(CONFIG)

%.o:%.c
	$(CC) -c $< -o $@ $(CONFIG)

clean:
	rm ./SRC/*.o $(TARGET)