# make file for SUN Niggle
#
# name - %M%
# ccsid - %W% - %G% %U%
# from  - %F%
# date  - %H% %T%
#
# use Make -f %M%
#
OBJS = niggle.o mgetc.o edit.o nterm.o window.o buffer.o dot.o \
	delete.o cuttext.o insert.o oldsearch.o pastebuf.o \
	oldfindrep.o copytext.o paragr.o winch.o filecomp.o

COPTS = -c -g -DCYGNUS -DUNIX

default : newnig

niggle.o : niggle.c types.mh defn.mh buffer.h
	 gcc $(COPTS) $*.c

.c.o:
	 gcc $(COPTS) $*.c
 
$(OBJS) :

newnig : $(OBJS)
	gcc -o newnig -Bstatic $(OBJS) -lncursesw

static : $(OBJS)
	gcc -o static -Bstatic $(OBJS) /usr/lib/libncurses.a
	strip static

bound : $(OBJS)
	gcc -o bound -Bstatic $(OBJS) -lncurses
	strip bound



