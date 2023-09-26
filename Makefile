TARGET = vm_riskxvii #wherevermain is, the name of the binary produced

CC = gcc

CFLAGS     = -Wall -Wvla -Werror -std=c11 -c -Os
ASAN_FLAGS = -fsanitize=address
SRC        = vm_riskxvii.c heap.c memory.c file_read.c instruction_functions.c#add other c files here
OBJ        = $(SRC:.c=.o)

all:$(TARGET)

$(TARGET):$(OBJ)
	$(CC)  -o $@ $(OBJ)

.SUFFIXES: .c .o

.c.o:
	 $(CC) $(CFLAGS) $<

run:
	./$(TARGET)

test:
	echo what are we testing?!

clean:
	rm -f *.o *.obj $(TARGET)


