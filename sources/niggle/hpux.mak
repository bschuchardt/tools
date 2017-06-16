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
	findrep.o copytext.o filecomp.o paragr.o winch.o grep.o

COPTS = -c -g -DHPUX -DUNIX 

default : newnig

niggle.o : niggle.c types.mh defn.mh buffer.h
	 cc $(COPTS) $*.c

.c.o:
	 cc $(COPTS) $*.c
 
$(OBJS) :

debugnig: $(OBJS)
	cc -g -o debugnig $(OBJS) -ltermcap

newnig : $(OBJS)
	cc -o newnig $(OBJS) -ltermcap
	strip newnig

static : $(OBJS)
	cc -o static -Wl,-a,archive $(OBJS) -ltermcap
	strip static

bound : $(OBJS)
	cc -o bound -Wl,-a,archive $(OBJS) -ltermcap
	strip bound
