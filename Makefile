OBJ_O:= main.o\
	    pty_fork.o\
		loop.o\
		apue.o
TARGET:=pty
COMPLIER:=gcc
all:$(OBJ_O)
	$(COMPLIER) -o ${TARGET} ${OBJ_O}
%.o:%.c
	$(COMPLIER) -o $@ -c $<
clean:
	rm -f *.o $(OBJ_O) $(TARGET)
