
DESTDIR=

PREFIX= $(DESTDIR)/usr
CC = /usr/bin/gcc
INSTALL = /usr/bin/install
LIBDIR = -L/usr/X11R6/lib
LIBS = -lXpm -lXext -lX11 -lm $(shell /usr/bin/xosd-config --libs)
FLAGS = -O2 -DENABLE_XOSD $(shell /usr/bin/xosd-config --cflags)
OBJS = wmgeneral.o

default:all

.c.o:
	$(CC) -I/usr/X11R6/share/include $(FLAGS) -c -Wall $< -o $*.o

wmbatppc.o: wmbatppc.c wmbatppc-master.xpm
	$(CC) -I/usr/X11R6/share/include $(FLAGS) -c -Wall wmbatppc.c -o $*.o

wmbatppc: $(OBJS) wmbatppc.o
	$(CC) $(FLAGS) -o wmbatppc $(OBJS) -lXext $(LIBDIR) $(LIBS) wmbatppc.o

all:: wmbatppc

clean::
	rm -f *.o
	rm -f wmbatppc 
	rm -f *~

install:: wmbatppc
	$(INSTALL) -d $(PREFIX)/bin
	$(INSTALL) -s -m 755 wmbatppc $(PREFIX)/bin
	ln -s $(PREFIX)/bin/wmbatppc $(PREFIX)/bin/batppc
	$(INSTALL) -d $(PREFIX)/share/man/man1
	$(INSTALL) -m 644 wmbatppc.1 $(PREFIX)/share/man/man1
	ln -s $(PREFIX)/share/man/man1/wmbatppc.1 $(PREFIX)/share/man/man1/batppc.1
	@echo "wmbatppc Installation finished..."
