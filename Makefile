#

LIBDIR = -L/usr/X11R6/lib
LIBS   = -lXpm -lXext -lX11 -lm
FLAGS = -O6
OBJS =	 \
		wmgeneral.o \

default:all

.c.o:
	cc -I/usr/X11R6/share/include $(FLAGS) -c -Wall $< -o $*.o

wmbatppc.o: wmbatppc.c wmbatppc-master.xpm
	cc -I/usr/X11R6/share/include $(FLAGS) -c -Wall wmbatppc.c -o $*.o

wmbatppc: $(OBJS) wmbatppc.o
	cc $(FLAGS) -o wmbatppc $(OBJS) -lXext $(LIBDIR) $(LIBS) wmbatppc.o

all:: wmbatppc

clean::
	rm -f *.o
	rm -f wmbatppc 
	rm -f *~

install:: wmbatppc
	strip wmbatppc
	cp -f wmbatppc /usr/local/bin/
	chmod 755 /usr/local/bin/wmbatppc
	chown root:root /usr/local/bin/wmbatppc
	cp -f wmbatppc.1 /usr/local/man/man1
	chmod 644 /usr/local/man/man1/wmbatppc.1
	chown root:root /usr/local/man/man1/wmbatppc.1
	@echo "wmbatppc Installation finished..."
