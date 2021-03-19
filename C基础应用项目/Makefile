CC=arm-linux-gcc
TAG=./BIN/masterMa
SRC=$(wildcard ./SRC/*.c)
OBJ=$(SRC:%.c=%.o)
override CONFIG += -I./INC -L./LIB -lmaster

$(TAG):$(OBJ)
	$(CC) $^ -o $@ $(CONFIG)

%.o:%.c
	$(CC) $c -o $@ -c $(CONFIG)

clean
	$(RM) $(OBJ) $(TAG)

.PHONY:clean