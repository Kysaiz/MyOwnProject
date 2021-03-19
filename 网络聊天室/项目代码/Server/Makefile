CC=gcc

T=./BIN/server

override CONFIG += -I./INC -L./LIB -lm -lpthread

SRCS=$(wildcard ./SRC/*.c)
OBJS=$(patsubst %.c, %.o, $(SRCS))

$(T):$(OBJS)
	$(CC) -o $@ $^ $(CONFIG)

%.o:%.c
	$(CC) -c $< -o $@ $(CONFIG)

clean:
	rm ./SRC/*.o $(TARGET)