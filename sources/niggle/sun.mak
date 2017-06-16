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
	findrep.o copytext.o filecomp.o paragr.o winch.o

COPTS = -c -DSUN -DUNIX

default : newnig

niggle.o : niggle.c types.mh defn.mh buffer.h
	 cc $(COPTS) $*.c

.c.o:
	 cc $(COPTS) $*.c
 
$(OBJS) :

newnig : $(OBJS)
	cc -o newnig $(OBJS) -lcurses

static : $(OBJS)
	cc -o static -Bstatic $(OBJS) /usr/lib/libtermcap.a
	strip static

bound : $(OBJS)
	cc -o bound -Bstatic $(OBJS) -ltermcap
	strip bound
