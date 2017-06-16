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
	delete.o cuttext.o insert.o search.o pastebuf.o \
	findrep.o copytext.o filecomp.o paragr.o winch.o

CFLAGS = -c -g -DULTRIX -DUNIX


newnig : $(OBJS)
	cc  -g -o newnig $(OBJS) -ltermcap
#	strip newnig

niggle.o :: niggle.c types.mh defn.mh buffer.h

