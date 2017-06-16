# make file for VMS Niggle
#
# name - %M%
# ccsid - %W% - %G% %U%
# from  - %F%
# date  - %H% %T%
#
# use Make -f vmsniggle.mak
#
#

OBJS = niggle.obj,mgetc.obj,edit.obj,nterm.obj,window.obj, -
	buffer.obj,dot.obj,delete.obj,cuttext.obj,insert.obj, -
	search.obj,pastebuf.obj,findrep.obj,copytext.obj,getch.obj

default: niggle.exe

niggle.exe: $(OBJS)
	$$ link/nodebug $(OBJS)

getch.obj: getch.mar
	$$ macro $*

niggle.obj: niggle.c types.mh defn.mh
	$$ cc/define=VMS $*

mgetc.obj: mgetc.c types.mh defn.h
	$$ cc/define=VMS $*

edit.obj: edit.c types.mh defn.h
	$$ cc/define=VMS $*

nterm.obj: nterm.c types.mh defn.h
	$$ cc/define=VMS $*

window.obj: window.c types.mh defn.h
	$$ cc/define=VMS $*

buffer.obj: buffer.c types.mh defn.h
	$$ cc/define=VMS $*

dot.obj: dot.c types.mh defn.h
	$$ cc/define=VMS $*

search.obj: search.c types.mh defn.h
	$$ cc/define=VMS $*

insert.obj: insert.c types.mh defn.h
	$$ cc/define=VMS $*

findrep.obj: findrep.c types.mh defn.h
	$$ cc/define=VMS $*

pastebuf.obj: pastebuf.c types.mh defn.h
	$$ cc/define=VMS $*

delete.obj: delete.c types.mh defn.h
	$$ cc/define=VMS $*

cuttext.obj: cuttext.c types.mh defn.h
	$$ cc/define=VMS $*

copytext.obj: copytext.c types.mh defn.h
	$$ cc/define=VMS $*
