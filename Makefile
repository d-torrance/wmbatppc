# Modified for Debian packaging -- jblache@debian.org
DESTDIR=

PREFIX= $(DESTDIR)/usr
CC = /usr/bin/gcc
INSTALL = /usr/bin/install
LIBDIR = -L/usr/X11R6/lib
LIBS = -lXpm -lXext -lX11 -lm
FLAGS = -O2
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
# let dh_strip handle this
#	strip wmbatppc
	$(INSTALL) -m 755 wmbatppc $(PREFIX)/bin
# let dh_installman handle it
#	$(INSTALL) -m 644 wmbatppc.1 $(PREFIX)/share/man/man1
	@echo "wmbatppc Installation finished..."
