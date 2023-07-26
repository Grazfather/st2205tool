SRC	=	main.c
OBJ	=	main.o
FLAGS	=	-g -fPIC
LIBS	=	

all:	libst2205.so

libst2205.so: 	$(OBJ) $(SRC) 
	ar -rv libst2205.a $(OBJ) 
	gcc -shared -Wl,-soname,libst2205.so.1 -o libst2205.so $(OBJ) $(LIBS)

clean:	
	rm -f $(OBJ) test *.a *.so

install: libst2205.so
	test -z "/usr/local/lib" || /bin/mkdir -p "/usr/local/lib"
	test -z "/usr/local/include" || /bin/mkdir -p "/usr/local/include"
	install libst2205.so /usr/local/lib
	install st2205.h /usr/local/include
	ldconfig
