OBJS= ls.c
EXEC= PS_server

CFLAGS= -g
CC= gcc

all : $(OBJS)
	$(CC) -o $(EXEC) $(OBJS) -lpthread
clean:
	rm -rf $(EXEC) 
