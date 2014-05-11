CFLAGS = -Wall

OBJECTS = lab2.o fbputchar.o 

TARFILES = Makefile lab2.c \
	fbputchar.h fbputchar.c \
	

lab2 : $(OBJECTS)
	cc $(CFLAGS) -o lab2 $(OBJECTS) -lm

lab2.tar.gz : $(TARFILES)
	rm -rf lab2
	mkdir lab2
	ln $(TARFILES) lab2
	tar zcf lab2.tar.gz lab2
	rm -rf lab2

lab2.o : lab2.c fbputchar.h 
fbputchar.o : fbputchar.c fbputchar.h

.PHONY : clean
clean :
	rm -rf *.o lab2
