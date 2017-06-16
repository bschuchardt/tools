# make file for SUN Solaris Niggle
#
# name - %M%
# ccsid - %W% - %G% %U%
# from  - %F%
# date  - %H% %T%
#
# use Make -f %M%
#
CC = /forte/SUNWspro/bin/cc

OBJS = niggle.o mgetc.o edit.o nterm.o window.o buffer.o dot.o \
	delete.o cuttext.o insert.o oldsearch.o pastebuf.o \
	findrep.o copytext.o filecomp.o paragr.o winch.o grep.o

COPTS = -c -DSUN -DSOLARIS -DUNIX

default : newnig

niggle.o : niggle.c types.mh defn.mh buffer.h
	$(CC) $(COPTS) $*.c

.c.o:
	 $(CC) $(COPTS) $*.c
 
$(OBJS) :

newnig : $(OBJS)
	$(CC) -o newnig $(OBJS) -lcurses
	strip newnig

static : $(OBJS)
	$(CC) -o static -Bstatic $(OBJS) -lcurses
	strip static

bound : $(OBJS)
	$(CC) -o bound -Bstatic $(OBJS) -lcurses
	strip bound
