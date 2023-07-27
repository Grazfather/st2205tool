SRC	=	main.c
OBJ	=	main.o
CFLAGS	=	-g -Wall -I../libst2205
LIBS	=	-lgd -L../libst2205 -lst2205

all:	setpic

# TOOD: Find a non-homebrew way to do this
UNAME := $(shell uname)
ifeq ($(UNAME), Darwin)
CFLAGS+= -I/opt/homebrew/include
LIBS+= -L/opt/homebrew/lib
endif


setpic:	$(OBJ) $(SRC)
	gcc -o $(@) $(OBJ) $(LIBS) $(FLAGS)

clean:
	rm -f $(OBJ) setpic
