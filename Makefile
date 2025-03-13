OBJS	= main.o
SOURCE	= main.c
HEADER	= 
OUT	= quadtree
CC	 = gcc
FLAGS	 = -g -c -Wall
LFLAGS	 = 

build: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

main.o: main.c
	$(CC) $(FLAGS) main.c 


clean:
	rm -f $(OBJS) $(OUT)