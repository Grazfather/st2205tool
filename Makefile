SRC	=	main.c
OBJ	=	main.o
LIBS	=	-lgd	

all:	phack splice

phack:	$(OBJ) $(SRC)
	gcc -o $(@) $(OBJ) $(LIBS) $(FLAGS)

splice:	splice.o splice.c
	gcc -o splice splice.o

clean:	
	-rm -f $(OBJ) phack splice splice.o