# make file for Linux Niggle
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

COPTS = -c -g -DLINUX=1 -DUNIX=1 -DMACOS=1 -Wno-implicit-int -Wno-implicit-function-declaration -Wno-return-type
OPTS=-m64

default : newnig64

niggle.o : niggle.c types.mh defn.mh buffer.h
	 gcc $(COPTS) $(OPTS) $*.c

.c.o:
	 gcc $(COPTS) $(OPTS) $*.c
 
$(OBJS) :

newnig64 : $(OBJS)
	gcc $(OPTS) -o newnig64 $(OBJS) -lcurses



