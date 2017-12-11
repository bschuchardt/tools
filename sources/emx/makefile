
MOTIFINC =  /home/bruce/usr/motif124/usr/include

MOTIFLIB =  /home/bruce/usr/motif124/usr/lib/libXm.a
MOTIFDEB =  /home/bruce/usr/motif124/usr/lib/libXm.a

XTDEB =     /usr/lib/libXt.a
XTPROF =    /usr/lib/libXt.a

X11DEB =    /usr/lib/libX11.a
X11PROF =   /usr/lib/libX11.a

#HEAP =      /ash1/users/marcs/hp/libhpd.a
#HEAPXT =    /ash1/users/marcs/hp/xtfront.o
SOL64_OPTS = -xarch=v9 -dalign -xO3 -xcode=pic32 -xildoff
LIN64_OPTS = -m64

#GEODE=/home/bruce/devel/g30/C/slow7

OBJS = 	emxbasic.o emxcmd.o emxdisplay.o emxfile.o emxmain.o emxmatch.o emxsearch.o emxsymbol.o
G_OBJS = $(GEODE)/emxbasicM.o \
	 $(GEODE)/emxcmdM.o \
	 $(GEODE)/emxdisplayM.o \
	 $(GEODE)/emxfileM.o \
	 $(GEODE)/emxmainM.o \
	 $(GEODE)/emxmatchM.o \
	 $(GEODE)/emxsearchM.o \
	 $(GEODE)/emxsymbolM.o

EMX =	emx.o
EMXW =	emxw.o emxwidget.o

SOURCES = emxbasic.c emxcmd.c emxdisplay.c emxfile.c emxmain.c emxmatch.c emxsearch.c symbol.c
EMXC =	emx.c
EMXWC =	emxw.c emxwidget.c

INC =	emx.h

WINC =	EmxI.h EmxP.h

default: clean linux64

rel:	version
	$(MAKE) -f makef \
		TARGET="emxr"			\
		OBJECTS="$(EMXW) $(OBJS)"	\
		INCLUDES="$(INC) $(WINC)"	\
		CFLAGS="-O -DWIDGET -I/usr/X11R6/include" \
		FLAGS="-O"		\
		LIBS="-L/usr/X11R6/lib -lXt -lX11 -lz"

geode:
	$(MAKE) -f makef \
		TARGET="emxg"			\
                SOURCES=""				\
		OBJECTS="$(EMXW) $(G_OBJS)"	\
		INCLUDES="$(INC) $(WINC)"	\
		CFLAGS="-g -DWIDGET -I/usr/X11R6/include"		\
		LIBS="$(HEAPXT) -lXt -lX11 $(HEAP)"

solaris:
	$(MAKE) -f makef \
		TARGET="emxg"		\
		OBJECTS="$(EMXW) $(OBJS)"	\
		INCLUDES="$(INC) $(WINC)"	\
		CFLAGS="-g -DWIDGET -DFLG_PROTOTYPE"	\
		FLAGS="-Bdynamic"	\
		LIBS="-lXt -lX11 -lsocket -lnsl -lICE -lSM $(HEAP) -lz"


o4:
	$(MAKE) -f makef \
		TARGET="emxf"			\
		OBJECTS="$(EMXW) $(OBJS)"	\
		INCLUDES="$(INC) $(WINC)"	\
		CFLAGS="-O4 -DWIDGET -I/usr/X11R6/include"		\
		FLAGS="-O4"			\
		LIBS="-lXt -lX11 usr/lib/libz"

emx:
	$(MAKE) -f makef \
		TARGET="emxg"		\
		OBJECTS="$(EMX) $(OBJS)"\
		INCLUDES="$(INC)"	\
		CFLAGS="-g"		\
		FLAGS="-Bstatic"	\
		LIBS="-lX11 $(HEAP) -lz"

cygwin:
	$(MAKE) -f makef \
		TARGET="emxg"		\
		OBJECTS="$(EMX) $(OBJS)"\
		INCLUDES="$(INC)"	\
		CFLAGS="-g -DCYGWIN -DSTANDALONE -DFLG_PROTOTYPE"	\
		FLAGS="-Bstatic"	\
		LIBS="-L/usr/X11R6/lib -lXt -lSM -lX11 $(HEAP) -lz"

cygwinw:
	$(MAKE) -f makef \
		TARGET="emxg"		\
		OBJECTS="$(EMXW) $(OBJS)"	\
		INCLUDES="$(INC) $(WINC)"	\
		CFLAGS="-g -DCYGWIN -DWIDGET -DFLG_PROTOTYPE"	\
		FLAGS="-Bstatic"	\
		LIBS="-L/usr/X11R6/lib -lXt -lSM -lICE -lX11 $(HEAP) -lz"

linuxw:
	$(MAKE) -f makef \
		TARGET="emx32" \
		OBJECTS="$(EMXW) $(OBJS)" \
		INCLUDES="$(INC) $(WINC)" \
		CFLAGS="-g -DUNIX -DWIDGET -DFLG_PROTOTYPE -DLINUX" \
		FLAGS="" \
		LIBS="-L/usr/X11R6/lib -lXt -lSM -lICE -lX11 $(HEAP) -lz"

# linux standalone 32-bit 8/2007
linux:	version
	$(MAKE) -f makef \
		TARGET="emxr32"			\
		OBJECTS="$(EMX) $(OBJS)"	\
		INCLUDES="$(INC) $(WINC)"	\
		CFLAGS="-O -DSTANDALONE -DUNIX -DFLG_PROTOTYPE -DLINUX -m32" \
		FLAGS="-O -m32"		\
		LIBS="-L/usr/X11R6/lib -lXt -lX11 -lz"

# linux standalone 64-bit 8/2007
linux64:	version
	$(MAKE) -f makef \
		TARGET="emxr64"			\
		OBJECTS="$(EMX) $(OBJS)"	\
		INCLUDES="$(INC) $(WINC)"	\
		CFLAGS="-g -DSTANDALONE -DUNIX -DFLG_PROTOTYPE -DLINUX $(LIN64_OPTS)" \
		FLAGS="-g $(LIN64_OPTS)"	\
		LIBS="-Bstatic -L/usr/X11R6/lib64 -lXt -lX11 -lz"

mac:		version
	$(MAKE) -f makef \
		TARGET="emxr64" \
		OBJECTS="$(EMX) $(OBJS)" \
		INCLUDES="$(INC) $(WINC) -I/usr/X11/include" \
		CFLAGS="-g -I/usr/X11/include -DSTANDALONE -DUNIX -DFLG_PROTOTYPE -DLINUX -DMACOS" \
		FLAGS="" \
		LIBS="-Bstatic -L/usr/X11/lib -lXt -lX11 -lz"

rh64:	version
	$(MAKE) -f makef \
		TARGET="emxr64"			\
		OBJECTS="$(EMX) $(OBJS)"	\
		INCLUDES="$(INC) $(WINC)"	\
		CFLAGS="-O -DSTANDALONE -DUNIX -DFLG_PROTOTYPE -DLINUX -I/usr/X11R6/include $(LIN64_OPTS)" \
		FLAGS="-O $(LIN64_OPTS)"	\
		LIBS="-L/usr/X11/lib -lXt -lX11 -lz"

linuxw64:
	$(MAKE) -f makef \
		TARGET="emx64" \
		OBJECTS="$(EMXW) $(OBJS)" \
		INCLUDES="$(INC) $(WINC)" \
		CFLAGS="-g -DUNIX -DWIDGET -DFLG_PROTOTYPE -DLINUX $(LIN64_OPTS)" \
		FLAGS="$(LIN64_OPTS)" \
		LIBS="-L/usr/X11R6/lib64 -lXt -lSM -lICE -lX11 $(HEAP) -lz"

linux64g:	version
	$(MAKE) -f makef \
		TARGET="emxr64"			\
		OBJECTS="$(EMX) $(OBJS)"	\
		INCLUDES="$(INC) $(WINC)"	\
		CFLAGS="-g -DSTANDALONE -DUNIX -DFLG_PROTOTYPE -DLINUX $(LIN64_OPTS)" \
		FLAGS="-g $(LIN64_OPTS)"	\
		LIBS="-L/usr/X11R6/lib64 -lXt -lX11 -lz"

# solaris standalone 64 bit 9/2007
sol64:
	$(MAKE) -f makef \
		TARGET="emxr64" \
		OBJECTS="$(EMX) $(OBJS)" \
		INCLUDES="$(INC)" \
		CFLAGS="-g -DSTANDALONE -DUNIX -DFLG_PROTOTYPE $(SOL64_OPTS)" \
		FLAGS="$(SOL64_OPTS)" \
		LIBS="-L/usr/X11R6/lib64 -lXt -lSM -lICE -lX11 $(HEAP) -lz"

solw64:
	$(MAKE) -f makef \
		TARGET="emx64" \
		OBJECTS="$(EMXW) $(OBJS)" \
		INCLUDES="$(INC) $(WINC)" \
		CFLAGS="-g -DUNIX -DWIDGET -DFLG_PROTOTYPE $(SOL64_OPTS)" \
		FLAGS="$(SOL64_OPTS)" \
		LIBS="-L/usr/X11R6/lib64 -lXt -lSM -lICE -lX11 $(HEAP) -lz"

emxO4:
	$(MAKE) -f makef \
		TARGET="emxg"		\
		OBJECTS="$(EMX) $(OBJS)"\
		INCLUDES="$(INC)"	\
		CFLAGS="-O4"		\
		LIBS="-lX11 $(HEAP) -lz"

motif:
	$(MAKE) -f makef \
		TARGET="emxm"		\
		OBJECTS="$(EMXW) $(OBJS)"	\
		INCLUDES="$(INC) $(WINC)"	\
		CFLAGS="-g -DWIDGET -I/usr/X11R6/include -DMOTIF -D_NO_PROTO -I$(MOTIFINC)"	\
		FLAGS="-Bstatic"	\
		LIBS="$(MOTIFLIB) $(HEAPXT) -lXt -lX11 $(HEAP)"

debugw:
	$(MAKE) -f makef \
		TARGET="emxwd"		\
		OBJECTS="$(EMXW) $(OBJS)"	\
		INCLUDES="$(INC) $(WINC)"	\
		CFLAGS="-g -DWIDGET -I/usr/X11R6/include"	\
		FLAGS="-Bstatic"	\
		LIBS="$(HEAPXT) $(XTDEB) $(X11DEB) $(HEAP)"

debug:
	$(MAKE) -f makef \
		TARGET="emxd"		\
		OBJECTS="$(EMX) $(OBJS)"\
		INCLUDES="$(INC)"	\
		CFLAGS="-g"		\
		FLAGS="-Bstatic"	\
		LIBS="$(X11DEB) $(HEAP)"

pgw:
	$(MAKE) -f makef \
		TARGET="emxpgw"		\
		OBJECTS="$(EMXW) $(OBJS)"	\
		INCLUDES="$(INC) $(WINC)"	\
		CFLAGS="-pg -DWIDGET -I/usr/X11R6/include -DMARK"	\
		FLAGS="-pg -Bstatic"	\
		LIBS="$(XTPROF) $(X11PROF)"

version:
	sed "s/^\(.*EMX - Version.*: \).*\"/\1`date`\"/" < emx.h > emx.j
	grep "EMX - Version" emx.j
	mv emx.j emx.h

sun:
	$(MAKE) -f makef \
		TARGET="emx4"		\
		SOURCES="$(EMXC) $(SRCS)"	\
		INCLUDES="$(INC)"	\
		CFLAGS="-O3"		\
		FLAGS="-Bstatic"	\
		LIBS="-lX11"		\
		fast

hpmotif:
	$(MAKE) -f makef \
		TARGET="emxm7"		\
		OBJECTS="$(EMXW) $(OBJS)"	\
		INCLUDES="$(INC) $(WINC)"	\
		SOURCES="emxmain.c $(SRCS)" \
		CFLAGS="-O -Wp,-H,1000000 -DWIDGET -DMOTIF  -D_NO_PROTO -I/usr/X11R6/include -I$(MOTIFINC)"	\
		FLAGS=""	\
		LIBS="$(MOTIFLIB) $(HEAPXT) -L/usr/X11R6/lib -lXt -lX11 $(HEAP)"

hpw:
	$(MAKE) -f makef \
		TARGET="emx7"		\
		OBJECTS="$(EMXW) $(OBJS)"	\
		INCLUDES="$(INC) $(WINC)"	\
		SOURCES="$(EMXW) $(SRCS)" \
		CFLAGS="+O3 -Wp,-H,1000000 -DWIDGET -I/usr/X11R6/include"	\
		FLAGS=""	\
		LIBS="$(HEAPXT) -L/usr/X11R6/lib -lXt -lX11 $(HEAP)"

hpnative:
	$(MAKE) -f makef \
		TARGET="emxn7"		\
		OBJECTS="$(EMXW) $(OBJS)"	\
		INCLUDES="$(INC) $(WINC)"	\
		SOURCES="$(EMXW) $(SRCS)" \
		CFLAGS="+O3 -Wp,-H,1000000 -DWIDGET -I/usr/include/X11R6"	\
		FLAGS=""	\
		LIBS="$(HEAPXT) -L/usr/lib/X11R6 -lXt -lX11 $(HEAP)"

ash:
	$(MAKE) -f makef \
		TARGET="emx4"		\
		SOURCES="$(EMXC) $(SRCS)"	\
		INCLUDES="$(INC)"	\
		CFLAGS="-O3"		\
		FLAGS=""		\
		LIBS="-lX11"		\
		fast

dec:
	$(MAKE) -f makef \
		TARGET="emx1"		\
		SOURCES="$(EMXC) $(SRCS)"	\
		INCLUDES="$(INC)"	\
		CFLAGS="-O3"		\
		FLAGS=""		\
		LIBS="-lX11"		\
		fast

clean:
	rm -f emx1 emx4 emxg emxd emxw emxdw emxm *.o *.exe emxr64 emxr32
	rm -rf ackack

install:	sun
	mv /home/marcs/ashbin/emx /home/marcs/ashbin/emx1
	cp emx4 /home/marcs/ashbin/emx
	strip /home/marcs/ashbin/emx
	cp /home/marcs/ashbin/emx /home/marcs/bin4

decinst:	dec
	cp emx1 /home/marcs/bin1/emx
	strip /home/marcs/bin1/emx

saber:
	#load -g $(EMXC) $(SRCS) -lX11 $(HEAP) -lc

saberw:
	#load -g -DWIDGET -I/usr/X11R6/include $(EMXWC) $(SRCS) $(XTDEB) $(X11DEB) $(HEAP) -lc

sabero:
	#load *.o $(XTDEB) $(X11DEB) $(HEAP) -lc

shar:
	rm -rf shar
	mkdir shar
	shar -f -c -l 63 -n emx -a -s marcs@slc.com -o shar/emxshar \
	symbol.c	\
	match.c		\
	makefile	\
	makef		\
	emxicon.h	\
	cmd.c		\
	basic.c		\
	file.c		\
	main.c		\
	display.c	\
	emx.h		\
	search.c	\
	widget.c	\
	emx.c		\
	emxw.c		\
	EmxI.h		\
	EmxP.h		\
	emxhelp
