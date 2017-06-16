#if !defined(LITTLE_EMX_H)
#define LITTLE_EMX_H TRUE
/*=========================================================================
 * Copyright (C) Servio Corp. 1991-1994.  All Rights Reserved.
 *
 * Name -
 *
 * Purpose -
 *
 * $Id: emx.h,v 30.7 1994/09/14 00:31:36 marcs Exp $
 *
 *========================================================================*/


/*
    EMX - Definitions

*/
#ifdef GEODE
#include <global.ht>
#include <utl.hf>
#include <l2unix.hf>
#undef FREE
#define FREE(x) free((char *)x)
#else
#define FREE(x) free(x)
#define const

#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>
#endif

#if defined(MACOS)
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif

#ifdef sun
#include <prof.h>
#else
#define MARK(x)
#endif

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>

#ifdef STANDALONE
typedef char Boolean;
#else
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#ifdef MOTIF
#include <Xm/XmP.h>
#include <Xm/Xm.h>
#else
#include <X11/CoreP.h>
#endif
#endif

#ifndef NO_GZIP
#include <zlib.h>
#endif

/* Set up define for GLOBAL */
#ifdef DEFINE
#define GLOBAL
#define VALUE(x) = x
#else
#define GLOBAL extern
#define VALUE(x)
#endif

/* Macros for arguments and definitions */
#if defined(FLG_PROTOTYPE)
#define _ARGS0() (void)
#define _ARGS1(t1, a1) (t1 a1)
#define _ARGS2(t1, a1, t2, a2) (t1 a1, t2 a2)
#define _ARGS3(t1, a1, t2, a2, t3, a3) (t1 a1, t2 a2, t3 a3)
#define _ARGS4(t1, a1, t2, a2, t3, a3, t4, a4) (t1 a1, t2 a2, t3 a3, t4 a4)
#define _ARGS5(t1, a1, t2, a2, t3, a3, t4, a4, t5, a5) (t1 a1, t2 a2, t3 a3, t4 a4, t5 a5)
#define _ARGS6(t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6) (t1 a1, t2 a2, t3 a3, t4 a4, t5 a5, t6 a6)
#define _DEF1(t1, a1) (t1 a1)
#define _DEF2(t1, a1, t2, a2) (t1 a1, t2 a2)
#define _DEF3(t1, a1, t2, a2, t3, a3) (t1 a1, t2 a2, t3 a3)
#define _DEF4(t1, a1, t2, a2, t3, a3, t4, a4) (t1 a1, t2 a2, t3 a3, t4 a4)
#define _DEF5(t1, a1, t2, a2, t3, a3, t4, a4, t5, a5) (t1 a1, t2 a2, t3 a3, t4 a4, t5 a5)
#define _DEF6(t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6) (t1 a1, t2 a2, t3 a3, t4 a4, t5 a5, t6 a6)
#define _COM() (int f, int n, KEY *keyp)
#define _COM2() (int f, int n)
#define _COM0() (void)
#define _COMP() (int status)
#define COMMAND(x) int x (int f, int n, KEY *keyp)
#define COMMAND0(x) int x (void)
#define COMMAND2(x) int x (int f, int n)
#define COMPLETION(x) int x (int status)
#define VOID void

#else

#define _ARGS0() ()
#define _ARGS1(t1, a1) (a1) t1 a1;
#define _ARGS2(t1, a1, t2, a2) (a1, a2) t1 a1; t2 a2;
#define _ARGS3(t1, a1, t2, a2, t3, a3) (a1, a2, a3) t1 a1; t2 a2; t3 a3;
#define _ARGS4(t1, a1, t2, a2, t3, a3, t4, a4) (a1, a2, a3, a4) t1 a1; t2 a2; t3 a3; t4 a4;
#define _ARGS5(t1, a1, t2, a2, t3, a3, t4, a4, t5, a5) (a1, a2, a3, a4, a5) t1 a1; t2 a2; t3 a3; t4 a4; t5 a5;
#define _ARGS6(t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6) (a1, a2, a3, a4, a5, a6) t1 a1; t2 a2; t3 a3; t4 a4; t5 a5; t6 a6;
#define _DEF1(t1, a1) ()
#define _DEF2(t1, a1, t2, a2) ()
#define _DEF3(t1, a1, t2, a2, t3, a3) ()
#define _DEF4(t1, a1, t2, a2, t3, a3, t4, a4) ()
#define _DEF5(t1, a1, t2, a2, t3, a3, t4, a4, t5, a5) ()
#define _DEF6(t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6) ()
#define _COM() ()
#define _COM2() ()
#define _COM0() ()
#define _COMP() ()
#define COMMAND(x) int x (f, n, keyp) int f; int n; KEY *keyp;
#define COMMAND0(x) int x ()
#define COMMAND2(x) int x (f, n) int f; int n;
#define COMPLETION(x) int x (status) int status;
#define VOID
#endif


#ifdef FLG_AIX_UNIX
#define STRING String
#else
#define STRING const String
#endif


typedef unsigned short unshort;


/*--------------------*/
/* Screen definitions */
/*--------------------*/

/*
    #define MAXROW	64
    #define MAXVIS	132
    #define MAXCOL	254 */
/*  #define MAXROW	200
    #define MAXVIS	256
    #define MAXCOL	511 */
#define MAXROW	200
#define MAXVIS	512
#define MAXCOL	1023

/*#define INITROW 25 */
#define INITROW 35
#define INITCOL 80
#define MINROW	4
#define MINCOL	10

/* Window margins */

#define VMARGIN 4
/* #define HMARGIN 0 */
#define HMARGIN 5
#define MARGIN	40

/* Underbar cursor height */

#define CURSORTALL	4

/* Desired window border size */

#define WINBORDERWIDTH	4

/* Cursor toggle values */

#define OFF	0
#define ON	1

/* Special character values */

#define CR	0x0d
#define LF	0x0a
#define CTLZ	0x1a


/*--------------*/
/* Key handling */
/*--------------*/

/* The symbol table links editing functions to names. Entries in the key
   map point at the symbol table entry. */

typedef int (*FUNC)();	/* sloppy code WILL NOT tolerate a prototype here! */

#define SYMFUNC		1
#define SYMMACRO	2
#define SYMMODIFY	4

/* Bound key description structure */
typedef struct ky
    {
    KeySym	keysym;
    int		modifier;
    struct fn	*funcp;
    struct ky	*hashp;
    struct ky	*orderp;
    } KEY;

/* Key binding holder structure */
typedef struct bi
    {
    KEY		    *keyp;
    unsigned short  original;
    struct bi	    *nextp;
    } KEYBINDING;

/* Function description structure */
typedef struct fn
    {
    char	*namep;
    FUNC	procp;
    int		type;
    KEYBINDING	*bindingp;
    struct fn	*nextp;
    struct fn	*prevp;
    } FUNCTION;

/* Keysym to key name mapping */
typedef struct
    {
    KeySym  keysym;
    const char	  *namep;
    } KEYSTRING;

/* Key modifier bit values beyond X's */
#define KEscapeMask	(1 << 16)
#define KCtlxMask	(1 << 17)

/* X11 values from X.h

#define ShiftMask	1<<0
#define LockMask	1<<1
#define ControlMask	1<<2
#define Mod1Mask	1<<3
#define Mod2Mask	1<<4
#define Mod3Mask	1<<5
#define Mod4Mask	1<<6
#define Mod5Mask	1<<7
*/

/* Artificial keysyms for button presses */
#define KButton		0x71710000
#define KButton1	KButton+1
#define KButton2	KButton+2
#define KButton3	KButton+3
#define KButton4        KButton+4
#define KButton5        KButton+5

#define KButtonUp	0x71710010
#define KButtonUp1	KButtonUp+1
#define KButtonUp2	KButtonUp+2
#define KButtonUp3	KButtonUp+3
#define KButtonUp4        KButtonUp+4
#define KButtonUp5        KButtonUp+5

#define KMotion		0x71710020


/* Table sizes, etc. */

#define MAXFILE		256	/* Length, file name */
#define MAXBUF		64	/* Length, buffer name */
#define MAXLINE		64000	/* Length, line */
#define MAXMACRO	512	/* Length, keyboard macro */
#define MAXPAT		256	/* Length, pattern */
#define MAXSRCH		128	/* Undoable search commands */
#define MAXCMD		64	/* Length, extended command */
#define MAXDIR		256	/* Length of current dir path +2*/
#define MAXSAVE		256	/* Length, save buffer */
#define LINECHUNK	16	/* Line block chunk size */
#define KILLCHUNK	2048	/* Kill buffer block size */


#define FALSE	0
#define TRUE	1
#define ABORT	2
#define PARTIAL 3

#define READ	0
#define WRITE	1

#define TABSET	4			/* default tab stop */

#define REMINDER    500			/* buffer flush count */

/* 'emxlinsert' control */

#define EMXSTR	0xeeee


/* These flag bits keep track of some aspects of the last command. The LASTUPDOWN
   flag controls goal column setting. The LASTKILL flag controls the clearing
   versus appending of data in the kill buffer. */

#define LASTUPDOWN	0x0001		/* Last was cursor north/south */
#define LASTKILL	0x0002		/* Last was a kill */
#define LASTARG		0x0004		/* Last was set numarg */
#define LASTSELECT	0x0008		/* Last was selection */

/* File I/O */

#define FIOSUC	0			/* Success */
#define FIOFNF	1			/* File not found */
#define FIOEOF	2			/* End of file */
#define FIOERR	3			/* Error */

/* Flushing buffers types */

#define MANUALFLUSH	1		/* Flush (manual save) */
#define AUTOFLUSH	2		/* Flush (auto-save) */

/* Backup results */

#define NOT_DONE    3		       /* Backup not done, unsuccesful */
#define NEW_FILE    1			/* Backup not done, new file */
#define DONE	    0			/* Backup done, succesful */

/* File Handling */

#define BACK_OFF	0
#define BACK_NEW	1
#define BACK_IDENTICAL	2
#define BACK_BADFILE	4

/* Display colors */

#define TEXTCOLOR	1			/* Text color */
#define INFOCOLOR	2			/* Infoline color */

/* Flags for emxmsgread */

#define NEW		1		/* New prompt */
#define AUTO		2		/* Autocompletion enabled */
#define BUFCMPL		4		/* Autocomplete buffer names */
#define FILECMPL	8		/* Autocomplete file names */

/* Selection modes - monotonically increasing */

#define SELECTCHAR	0
#define SELECTWORD	1
#define SELECTLINE	2


/* These flags, and the macros below them, make up a do-it-yourself set of
   ctype macros that understand the DEC multinational set, and let us
   ask a slightly different set of questions. */

#define eW	0x01			/* Word */
#define eU	0x02			/* Upper case letter */
#define eL	0x04			/* Lower case letter */
#define eC	0x08			/* Control */
#define eS	0x10			/* White space */
#define eN	0x20			/* Number */

#define ISWORD(c)	((emxcinfo[(c)]&eW)!=0)
#define ISCTRL(c)	((emxcinfo[(c)]&eC)!=0)
#define ISSPACE(c)	((emxcinfo[(c)]&eS)!=0)
#define ISUPPER(c)	((emxcinfo[(c)]&eU)!=0)
#define ISLOWER(c)	((emxcinfo[(c)]&eL)!=0)
#define ISNUMBER(c)	((emxcinfo[(c)]&eN)!=0)
#define TOUPPER(c)	((c)-0x20)
#define TOLOWER(c)	((c)+0x20)


/* All text is kept in circularly linked lists of LINE structures.  These
   begin at the header line (which is the blank line beyond the end of the
   buffer). This line is pointed to by the BUFFER. Each line contains the
   number of bytes in the line (the used size), the size of the text array,
   and the text. The end of line is not stored as a byte; it's implied. */

typedef struct LINE
    {
    struct LINE	    *nextp;	    /* Link to the next line */
    struct LINE	    *prevp;	    /* Link to the previous line */
    unsigned short  size;	    /* Allocated size */
    unsigned short  used;	    /* Used size */
    char	    text[1];	    /* A bunch of characters */
    } LINE;


/* This structure defines a segment of a buffer which is highlighted. A list
   of such segments is kept in each buffer structure, and is interpreted by the
   display routines and by commands which modify it. */

typedef struct HIGHSEG
    {
    LINE	    *lp;
    int		    firsto;
    int		    endo;
    } HIGHSEG;

#define HIGHLIGHT   0x0100


/* Text is kept in buffers. A buffer header, described below, exists for every
   buffer in the system. The buffers are kept in a big list, so that commands
   that search for a buffer by name can find the buffer header. There is a safe
   store for the dot and mark in the header, but this is only valid if the
   buffer is not being displayed (that is, if windows is 0). The text for the
   buffer is kept in a circularly linked list of lines, with a pointer to the
   header line in linep. */

typedef struct	BUFFER
    {
    struct BUFFER   *nextp;		/* Link to next BUFFER */
    char	    bufname[MAXBUF];	/* Buffer name */
    LINE	    *linep;		/* Link to the header LINE */
    int		    windows;		/* Count of windows on buffer */
    int		    flag;		/* Flags */
    int		    mode;		/* Language mode for the buffer */
    LINE	    *dotp;		/* Link to cursor LINE structure */
    int		    doto;		/* Offset of cursor in above LINE */
    LINE	    *markp;		/* The same as the above two, */
    int		    marko;		/* but for the mark */
    int		    selsize;		/* Size of the select list area */
    HIGHSEG	    *sellistp;		/* Ptr to selection list area */
    HIGHSEG	    *selectp;		/* Ptr to selection list */
    int		    highsize;		/* Size of the highlight list area */
    HIGHSEG	    *highlistp;		/* Ptr to highlight list area */
    HIGHSEG	    *highlightp;	/* Ptr to highlight list element */
    LINE	    *matchp;		/* Ptr to line with match */
    int		    matcho;		/* Offset of match in above LINE*/
    char	    filename[MAXFILE];	/* File name */
    } BUFFER;

#define BFCHG	    0x01		/* Changed */
#define BFUNUSED    0x02		/* Currently unused */
#define BFBACK	    0x04		/* Time to write backup to %editback */
#define BFVIEW	    0x08		/* Buffer is read-only */
#define BFNL	    0x10		/* newline flag for yank/save buffer */
#define BFSAV	    0x20		/* saved buffer */
#define BFISPC	    0x40		/* PC-style line endings */


/* There is a window structure allocated for every active display window.
   The windows are kept in a big list, in top to bottom screen order, with the
   listhead at g_wheadp. Each window contains its own values of dot and mark.
   The flag field contains some bits that are set by commands to guide
   redisplay; although this is a bit of a compromise in terms of decoupling,
   the full blown redisplay is just too expensive to run for every input
   character. */

typedef struct WINDOW
    {
    struct WINDOW   *nextp;	/* Next window */
    BUFFER	    *bufp;	/* Buffer displayed in window */
    LINE	    *linep;	/* Top line in the window */
    LINE	    *dotp;	/* Line containing cursor */
    int		    doto;	/* Byte offset for cursor */
    LINE	    *markp;	/* Line containing mark */
    int		    marko;	/* Byte offset for mark */
    int		    leftcol;	/* Left column if horizontally scrolled */
    int		    toprow;	/* Origin 0 top row of window */
    int		    rows;	/* # of rows of text in window */
    int		    force;	/* If NZ, forcing row */
    int		    flag;	/* Flags */
    } WINDOW;

/* Window flags are set by command processors to tell the display system
   what has happened to the buffer mapped by the window. Setting WFHARD is
   always a safe thing to do, but it may do more work than is necessary. Always
   try to set the simplest action that achieves the required update. Because
   commands set bits in the flag, update will see all change flags, and do
   the most general one. */

#define WFFORCE 0x01		/* Force reframe */
#define WFMOVE	0x02		/* Movement from line to line */
#define WFEDIT	0x04		/* Editing within a line */
#define WFHARD	0x08		/* Better to a full display */
#define WFINFO	0x10		/* Update infoline */
#define WFSHIFT 0x20		/* Shifted horizontally */


/* mouse structure */

typedef struct mouse
    {
    WINDOW	*wp;		/* window containing this point */
    LINE	*dotp;		/* line containing point */
    int		doto;		/* offset in above line */
    int		flag;		/* various flags */
    } MOUSE;

#define MFINFO	 0x01		/* point in infoline */
#define MFBADPT	 0x02		/* illegal point */

#define MOUSE_ICON_START    6	/* Starting location of the mouse "icons" */


/* This structure holds the starting position (as a line/offset pair) and the
   number of characters in a region of a buffer. This makes passing the
   specification of a region around a little bit easier. */

typedef struct
    {
    LINE	*firstlp;	/* Origin LINE address */
    int		firsto;		/* Origin LINE offset */
    LINE	*lastlp;	/* Last LINE address */
    int		lasto;		/* Last LINE offset */
    long	lines;		/* Length in lines */
    long	chars;		/* Length in characters */
    } REGION;


#define STACKMAX 16
#define NOFUNC	 (int (*)())1


/* Literals for delimeter matching */

/* Language modes */

#define C_MODE		0
#define ST_MODE		1

#define MAX_RLVNT_DELIMS 8	    /* Maximum # of delimiter which can */
				    /* participate in a search, excluding */
				    /* 'special' delimiters. */
#define MAX_DELIM_LN	8	    /* Maximum length  of a delimiter. */

#define MAX_CHARS	130

typedef struct
    {
    unsigned char   delim_string[MAX_DELIM_LN];
    int		    (*forw_action)();
    int		    (*back_action)();
    int		    (*forw_context)();
    int		    (*back_context)();
    char	    forw_opposite;
    char	    back_opposite;
    char	    confuseindex;
    char	    bit_masks;
    unsigned char   relvnt_delims[MAX_RLVNT_DELIMS];
    int		    delim_len;
    char	    relevant_to_search;
    } DELIMITER;

/* The following table describes a language mode to be supported.
   This table is needed so that the buffer list can display the language
   mode (modestring) of a buffer. */

typedef struct
    {
    DELIMITER	*modedelims;
    const char	*modestring;
    int		sense_flag;
    } MODE_DESC;

/* The following structure indicates, for each character, whether or not it is
   the first character of a delimiter and if so, where in the table to find
   that delimiter. */

typedef struct
    {
    char	    beginning_ch;
    unsigned char   begin_index;
    } DELIM_BOUND;

typedef struct
    {
    DELIM_BOUND	    delimcheck[MAX_CHARS];
    int		    mode;
    char	    automatchchar;
    char	    automatchchar2;
    DELIMITER	    *modedelims;
    const char	    *modestring;
    int		    (*oppositep)();
    int		    (*actionp)();
    char	    (*newcharp)();
    int		    (*contextp)();
    int		    matchcase;
    int		    autoindex;
    int		    charoffset;
    LINE	    *srchlinep;
    int		    srchindex;
    int		    linespassed;
    char	    currchar;
    int		    nestcount;
    int		    delimindex;
    int		    wordstart;
    int		    wordend;
    int		    wordlen;
    int		    silent;
    int		    matchdirection;
    char	    cmpbuf[MAX_DELIM_LN];
    } MATCH;


/* An IMAGE structure always holds an array of characters whose length is equal
   to the longest visible line possible. Only some of this is used if g_ncol
   isn't the same as MAXVIS. */

typedef struct
    {
    unsigned char   change;		/* Change flag */
    unsigned char   color;		/* Line color */
    unshort	    text[MAXVIS];	/* The characters */
    } IMAGE;

typedef struct
    {
    unsigned char   change;		/* Change flag */
    unsigned char   color;		/* Line color */
    unshort	    text[2];		/* The characters */
    } IMAGEROW;


/* Search */

#define SRCH_BEGIN  (0)			    /* Search sub-codes.    */
#define SRCH_FORW   (-1)
#define SRCH_BACK   (-2)
#define SRCH_PREV   (-3)
#define SRCH_NEXT   (-4)
#define SRCH_NOPR   (-5)
#define SRCH_ACCM   (-6)

typedef struct
    {
    int	    code;
    LINE    *dotp;
    int	    doto;
    } SRCHCOM;



/*------------------*/
/* Global Variables */
/*------------------*/


extern MODE_DESC emxmodetable[]; /* in emxmatch.c */


/* The date is updated by 'make version' */

GLOBAL char g_version[]		VALUE("EMX - Version 1.0: Tue May 23 11:34:05 PDT 2017");
GLOBAL char g_emptybuf[]	VALUE("?");
GLOBAL char g_bufferlist[]	VALUE("Buffer List");
GLOBAL char g_strhelp[]		VALUE("Help Buffer");
GLOBAL char g_strkeylist[]	VALUE("Key Binding List");
GLOBAL char g_strfunclist[]	VALUE("Function List");
GLOBAL char g_strstdin[]	VALUE("<stdin>");
GLOBAL char g_strunnamed[]	VALUE("<unnamed macro>");
GLOBAL char g_msglinealloc[]	VALUE("Cannot allocate %d bytes for a line");
GLOBAL char g_msggoto[]		VALUE("%s: G(oto), K(ill), or S(ave)");
GLOBAL char g_errbindnull[]	VALUE("[Cannot bind null key]");
GLOBAL char g_errbindtyping[]	VALUE("[Cannot bind typing characters]");
GLOBAL char g_errimpossible[]	VALUE("[Impossible change]");
GLOBAL char g_errreadonly[]	VALUE("[Buffer is read-only]");
GLOBAL char g_msgdiscard[]	VALUE("Discard changes?");
GLOBAL char g_msgreminderoff[]	VALUE("[Buffer flush reminder off]");
GLOBAL char g_msgcannotmatch[]	VALUE("[Cannot match delimiter]");
GLOBAL char g_msgok[]		VALUE("[ok]");
GLOBAL char g_msgbytesrem[]	VALUE("[%d bytes removed]");
GLOBAL char g_msgcopied[]	VALUE("[Copied]");
GLOBAL char g_msgnotnow[]	VALUE("[Macro already in progress]");
GLOBAL char g_msgnomacro[]	VALUE("[No macro defined]");
GLOBAL char g_msgonlyonewin[]	VALUE("[Only one window]");
GLOBAL char g_msgdelimiter[]	VALUE(" delimiter: ");
GLOBAL char g_msgvisitfile[]	VALUE("Visit file: ");
GLOBAL char g_msgnewmacro[]	VALUE("New macro name: ");
GLOBAL char g_msgoldstring[]	VALUE("Old String");
GLOBAL char g_ssense[]		VALUE("[case sense]");
GLOBAL char g_nsense[]		VALUE("[case non sense]");
GLOBAL char g_msgnomark[]	VALUE("[No mark in this window]");
GLOBAL char g_fontdef[]		VALUE("8x13");

GLOBAL const char *g_msg_badbackup[]
#ifdef DEFINE
    = {
    "New File: %s - ",
    "Original and backup files identical - ",
    "Backup Unsuccessful - ",
    "Invalid filename: %s - "
    }
#endif
    ;

GLOBAL	const char emxhex[]
#if defined(DEFINE)
    = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'}
#endif
    ;


/* Global Variables */

#ifdef STANDALONE
#define VAR(x) emx.x
#else
#define VAR(x) emx->emx.x
#endif

#define g_access		VAR(access)
#define g_actionp		VAR(actionp)
#define g_argc			VAR(argc)
#define g_argflag		VAR(argflag)
#define g_argv			VAR(argv)
#define g_argval		VAR(argval)
#define g_background		VAR(background)
#define g_bheadp		VAR(bheadp)
#define g_bindkey		VAR(bindkey)
#define g_bindmod		VAR(bindmod)
#define g_blanks		VAR(blanks)
#define g_blistp		VAR(blistp)
#define g_bufp			VAR(bufp)
#define g_bufname		VAR(bufname)
#define g_charascent		VAR(charascent)
#define g_charheight		VAR(charheight)
#define g_charoffset		VAR(charoffset)
#define g_charwidth		VAR(charwidth)
#define g_cip			VAR(cip)
#define g_cmdbuf		VAR(cmdbuf)
#define g_cmpbuf		VAR(cmpbuf)
#define g_compose		VAR(compose)
#define g_contextp		VAR(contextp)
#define g_count			VAR(count)
#define g_curbp			VAR(curbp)
#define g_curgoal		VAR(curgoal)
#define g_currchar		VAR(currchar)
#define g_currentdir		VAR(currentdir)
#define g_cursorh		VAR(cursorh)
#define g_cursorsave		VAR(cursorsave)
#define g_cursorstate		VAR(cursorstate)
#define g_cursorw		VAR(cursorw)
#define g_cursorx		VAR(cursorx)
#define g_cursory		VAR(cursory)
#define g_curwp			VAR(curwp)
#define g_customization		VAR(customization)
#define g_defaultmode		VAR(defaultmode)
#define g_delimcheck		VAR(delimcheck)
#define g_delimindex		VAR(delimindex)
#define g_display		VAR(display)
#define g_elip_bline		VAR(elip_bline)
#define g_elip_fline		VAR(elip_fline)
#define g_eliplinep		VAR(eliplinep)
#define g_elipsis		VAR(elipsis)
#define g_epresent		VAR(epresent)
#define g_event			VAR(event)
#define g_execbuf		VAR(execbuf)
#define g_execline		VAR(execline)
#define g_execpos		VAR(execpos)
#define g_execstr		VAR(execstr)
#define g_executing		VAR(executing)
#define g_execview		VAR(execview)
#define g_fabortp		VAR(fabortp)
#define g_fbindtokeyp		VAR(fbindtokeyp)
#define g_filechanges		VAR(filechanges)

#ifndef NO_GZIP
#define g_isGzipped             VAR(isGzipped)
#define g_plainFileId           VAR(plainFileId)
#endif

#define g_fileid		VAR(fileid)
#define g_filename		VAR(filename)
#define g_filenamep		VAR(filenamep)
#define g_filestatus		VAR(filestatus)
#define g_fillcol		VAR(fillcol)
#define g_flag			VAR(flag)
#define g_flistp		VAR(flistp)
#define g_flushing		VAR(flushing)
#define g_font			VAR(font)
#define g_foreground		VAR(foreground)
#define g_fselfinsertp		VAR(fselfinsertp)
#define g_funcname		VAR(funcname)
#define g_funcp			VAR(funcp)
#define g_gchi			VAR(gchi)
#define g_gcinv			VAR(gcinv)
#define g_gclo			VAR(gclo)
#define g_highlit		VAR(highlit)
#define g_initheight		VAR(initheight)
#define g_initialdir		VAR(initialdir)
#define g_initialload		VAR(initialload)
#define g_initialmessage	VAR(initialmessage)
#define g_initwidth		VAR(initwidth)
#define g_kbufp			VAR(kbufp)
#define g_keycurrent		VAR(keycurrent)
#define g_keyhashtable		VAR(keyhashtable)
#define g_keylastp		VAR(keylastp)
#define g_keylistp		VAR(keylistp)
#define g_keyp			VAR(keyp)
#define g_keynull		VAR(keynull)
#define g_killincr		VAR(killincr)
#define g_killsize		VAR(killsize)
#define g_killused		VAR(killused)
#define g_lastclicktime		VAR(lastclicktime)
#define g_lastflag		VAR(lastflag)
#define g_linep			VAR(linep)
#define g_linespassed		VAR(linespassed)
#define g_longline		VAR(longline)
#define g_macro			VAR(macro)
#define g_macrodelim		VAR(macrodelim)
#define g_macroinp		VAR(macroinp)
#define g_macrokeys		VAR(macrokeys)
#define g_macrolen		VAR(macrolen)
#define g_macrooutp		VAR(macrooutp)
#define g_macroplay		VAR(macroplay)
#define g_macroplays		VAR(macroplays)
#define g_mainwin		VAR(mainwin)
#define g_matchp		VAR(matchp)
#define g_matchkeyp		VAR(matchkeyp)
#define g_modes			VAR(modes)
#define g_modifier		VAR(modifier)
#define g_mouse1		VAR(mouse1)
#define g_mouse2		VAR(mouse2)
#define g_mousedo		VAR(mousedo)
#define g_ncol			VAR(ncol)
#define g_newcharp		VAR(newcharp)
#define g_nochange		VAR(nochange)
#define g_nrow			VAR(nrow)
#define g_num			VAR(num)
#define g_oldcurbp		VAR(oldcurbp)
#define g_oldcurwp		VAR(oldcurwp)
#define g_oldfunc		VAR(oldfunc)
#define g_oldfunc2		VAR(oldfunc2)
#define g_onefunc		VAR(onefunc)
#define g_pattern		VAR(pattern)
#define g_piping		VAR(piping)
#define g_promptbp		VAR(promptbp)
#define g_promptwp		VAR(promptwp)
#define g_pimage		VAR(pimage)
#define g_quitcallback		VAR(quitcallback)
#define g_readbuf		VAR(readbuf)
#define g_readcallback		VAR(readcallback)
#define g_readflag		VAR(readflag)
#define g_readfunc		VAR(readfunc)
#define g_readinginput		VAR(readinginput)
#define g_readmax		VAR(readmax)
#define g_readpos		VAR(readpos)
#define g_replcount		VAR(replcount)
#define g_replhits		VAR(replhits)
#define g_repllen		VAR(repllen)
#define g_replline		VAR(replline)
#define g_reploff		VAR(reploff)
#define g_replstr		VAR(replstr)
#define g_rowsize		VAR(rowsize)
#define g_savebuf		VAR(savebuf)
#define g_savemacroinp		VAR(savemacroinp)
#define g_savereminder		VAR(savereminder)
#define g_scrcol		VAR(scrcol)
#define g_scrcolor		VAR(scrcolor)
#define g_screen		VAR(screen)
#define g_scrrow		VAR(scrrow)
#define g_scrwiped		VAR(scrwiped)
#define g_selectmode		VAR(selectmode)
#define g_srchall		VAR(srchall)
#define g_srchbp		VAR(srchbp)
#define g_srchcmds		VAR(srchcmds)
#define g_srchindex		VAR(srchindex)
#define g_srchlastdir		VAR(srchlastdir)
#define g_srchlinep		VAR(srchlinep)
#define g_srchoffset		VAR(srchoffset)
#define g_srchstat		VAR(srchstat)
#define g_srchstr		VAR(srchstr)
#define g_stack			VAR(stack)
#define g_stackcount		VAR(stackcount)
#define g_stackp		VAR(stackp)
#define g_standalone		VAR(standalone)
#define g_status		VAR(status)
#define g_tabset		VAR(tabset)
#define g_tempbuf		VAR(tempbuf)
#define g_thisflag		VAR(thisflag)
#define g_tokbuf		VAR(tokbuf)
#define g_tokp			VAR(tokp)
#define g_ungottenkeyp		VAR(ungottenkeyp)
#define g_vimage		VAR(vimage)
#define g_wheadp		VAR(wheadp)
#define g_windowheight		VAR(windowheight)
#define g_windowopen		VAR(windowopen)
#define g_windowwidth		VAR(windowwidth)
#define g_winvisible		VAR(winvisible)
#define g_wordend		VAR(wordend)
#define g_wordlen		VAR(wordlen)
#define g_wordstart		VAR(wordstart)
#define g_writecallback		VAR(writecallback)


/* The global variables structure */

typedef struct
    {
    struct
	{
	Boolean insert;
	Boolean defaultback;
	Boolean tabsonindent;
	Boolean usedefaultsize;

	Boolean casesensitive;
	Boolean cursorstyle;
	Boolean autobackup;
	Boolean verticalscroll;
	Boolean manstyle;
	Boolean pushnewlines;
	Boolean wordwrap;
	Boolean filldouble;
	Boolean allowbufwrites;
	Boolean automatch;
	Boolean trimoutput;
	} modes;

    IMAGEROW		*vimage;
    IMAGEROW		*pimage;
    IMAGEROW		*blanks;
    int			rowsize;
    KEY			keycurrent;
    KEY			keynull;
    int			modifier;
    KEY			*ungottenkeyp;
    KEY			*keyp;
    unsigned long	*macroinp;
    unsigned long	*macrooutp;
    int			initialload;
    int			elipsis;
    int			piping;
    char		*kbufp;
    int			killused;
    int			killsize;
    int			killincr;
    int			scrwiped;
    int			cursorstate;
    int			readinginput;
    int			epresent;
    BUFFER		*bheadp;
    WINDOW		*wheadp;
    int			initwidth;
    int			initheight;
    unsigned long	*macro;
    int			tabset;
    int			fillcol;
    int			argflag;
    int			argval;
    int			savereminder;
    int			filechanges;
    int			windowopen;
    char		initialmessage[MAXCOL+1];
    char		*srchstr;
    int			srchlastdir;
    int			srchall;
    int			executing;
    BUFFER		*execbuf;
    const char		*execstr;
    int			highlit;
    int			mousedo;
    long		lastclicktime;
    int			winvisible;
    LINE		*eliplinep;
    int			defaultmode;
    MATCH		*matchp;
    char		*customization;

    FUNC		stack[STACKMAX];
    FUNC		*stackp;
    int			stackcount;
    FUNCTION		*flistp;
    FUNCTION		*fselfinsertp;
    FUNCTION		*fabortp;
    FUNCTION		*fbindtokeyp;
    KEY			*keylistp;
    KEY			*keylastp;
    KEY			*matchkeyp;
    int			readpos;
    char		*readbuf;
    int			readmax;
    int			readflag;
    FUNC		readfunc;
    FUNC		onefunc;
    char		filename[MAXFILE];
    char		cmdbuf[256];
    char		funcname[MAXCMD];
    char		savebuf[MAXSAVE];
    BUFFER		*bufp;
    FUNCTION		*funcp;
    KeySym		bindkey;
    unsigned long	bindmod;
    KEY			macrodelim;
    int			macrolen;
    unsigned long	*macrokeys;
    int			flag;
    int			num;
    char		flushing;
    char		replstr[MAXPAT];
    int			repllen;
    LINE		*replline;
    int			reploff;
    int			replcount;
    int			replhits;
    int			count;
    int			status;
    const char		*filenamep;
    int			filestatus;
    char		bufname[MAXBUF];
    int			nochange;
    int			access;
    unsigned long	foreground;
    unsigned long	background;
#ifdef NO_GZIP
    FILE		*fileid;
#else
    int                 isGzipped;
    FILE                *plainFileId;
    gzFile              fileid;
#endif
    int			scrcolor;
    int			scrrow;
    int			scrcol;
    int			longline;
    char		currentdir[MAXDIR];
    char		initialdir[MAXDIR];
    int			thisflag;
    int			lastflag;
    int			curgoal;
    BUFFER		*curbp;
    BUFFER		*promptbp;
    WINDOW		*curwp;
    WINDOW		*promptwp;
    BUFFER		*blistp;
    WINDOW		*oldcurwp;
    BUFFER		*oldcurbp;
    unsigned long	*macroplay;
    int			macroplays;
    unsigned long	*savemacroinp;
    char		pattern[MAXPAT];
    int			cip;
    int			srchstat;
    BUFFER		*srchbp;
    LINE		*srchlinep;
    int			srchoffset;
    SRCHCOM		srchcmds[MAXSRCH];
    int			srchindex;
    KEY			*keyhashtable[256];
    int			tempbuf;
    LINE		*execline;
    int			execpos;
    int			execview;
    char		tokbuf[256];
    char		*tokp;
    FUNCTION		*oldfunc;
    FUNCTION		*oldfunc2;
    LINE		*linep;
    int			cursorx;
    int			cursory;
    int			cursorh;
    int			cursorw;
    int			cursorsave;
    int			nrow;
    int			ncol;
    int			charheight;
    int			charwidth;
    int			charascent;
    MOUSE		mouse1;
    MOUSE		mouse2;
    int			selectmode;
    Display		*display;
    int			screen;
    Window		mainwin;
    XEvent		*event;
    XComposeStatus	compose;
    GC			gclo;
    GC			gchi;
    GC			gcinv;
    unsigned long	windowwidth;
    unsigned long	windowheight;
    LINE		*elip_bline;
    LINE		*elip_fline;
#ifndef STANDALONE
    XFontStruct		*font;
    XtCallbackList	readcallback;
    XtCallbackList	writecallback;
    XtCallbackList	quitcallback;
    int			argc;
    char		**argv;
    int			standalone;
#endif
    }
    EmxVars;


#ifndef STANDALONE
#include "EmxI.h"
#include "EmxP.h"
#endif


#ifdef STANDALONE
GLOBAL EmxVars emx
#ifdef DEFINE
    = {
    {
    TRUE,			/* insert */
    TRUE,			/* defaultback */
    TRUE,			/* tabsonindent */
    TRUE,			/* usedefaultsize */

    FALSE,			/* casesensitive */
    FALSE,			/* cursorstyle */
    FALSE,			/* autobackup */
    FALSE,			/* verticalscroll */
    FALSE,			/* manstyle */
    FALSE,			/* pushnewlines */
    FALSE,			/* wordwrap */
    FALSE,			/* filldouble */
    FALSE,			/* allowbufwrites */
    FALSE,			/* automatch */
    FALSE,			/* trimoutput */
    },				/* modes */

    NULL,			/* vimage */
    NULL,			/* pimage */
    NULL,			/* blanks */
    0,				/* rowsize */
    {0, 0, NULL, NULL, NULL},	/* keycurrent */
    {0, 0, NULL, NULL, NULL},	/* keynull */
    0,				/* modifier */
    NULL,			/* ungottenkeyp */
    NULL,			/* keyp */
    NULL,			/* macroinp */
    NULL,			/* macrooutp */
    TRUE,			/* initialload */
    FALSE,			/* elipsis */
    FALSE,			/* piping */
    NULL,			/* kbufp */
    0,				/* killused */
    0,				/* killsize */
    KILLCHUNK,			/* killincr */
    TRUE,			/* scrwiped */
    OFF,			/* cursorstate */
    FALSE,			/* readinginput */
    FALSE,			/* epresent */
    NULL,			/* bheadp */
    NULL,			/* wheadp */
    0,				/* initwidth */
    0,				/* initheight */
    NULL,			/* macro */
    TABSET,			/* tabset */
    0,				/* fillcol */
    FALSE,			/* argflag */
    1,				/* argval */
    0,				/* savereminder */
    0,				/* filechanges */
    FALSE,			/* windowopen */
    "",				/* initialmessage[] */
    g_nsense,			/* srchstr */
    SRCH_FORW,			/* srchlastdir */
    FALSE,			/* srchall */
    FALSE,			/* executing */
    NULL,			/* execbuf */
    NULL,			/* execstr */
    FALSE,			/* highlit */
    FALSE,			/* mousedo */
    0,				/* lastclicktime */
    FALSE,			/* winvisible */
    NULL,			/* eliplinep */
    C_MODE,			/* defaultmode */
    NULL,			/* matchp */
    NULL,			/* custmoization */
    }
#endif
    ;

#else

/* The global pointer */

GLOBAL EmxWidget   emx;

#endif


/* KeySym to string mapping, since XLookupString doesn't always give
   a good result */

GLOBAL KEYSTRING g_keystrings[]
#ifdef DEFINE
    = {
{ 0x0020,     "space"		 },
{ 0xff08,     "BackSpace"	 },
{ 0xff09,     "Tab"		 },
{ 0xff0a,     "Linefeed"	 },
{ 0xff0b,     "Clear"		 },
{ 0xff0d,     "Return"		 },
{ 0xff13,     "Pause"		 },
{ 0xff14,     "Scroll_Lock"	 },
{ 0xff1b,     "Escape"		 },
{ 0xffff,     "Delete"		 },
{ 0xff20,     "Multi_key"	 },
{ 0xff50,     "Home"		 },
{ 0xff51,     "Left"		 },
{ 0xff52,     "Up"		 },
{ 0xff53,     "Right"		 },
{ 0xff54,     "Down"		 },
{ 0xff55,     "Prior"		 },
{ 0xff56,     "Next"		 },
{ 0xff57,     "End"		 },
{ 0xff58,     "Begin"		 },
{ 0xff60,     "Select"		 },
{ 0xff61,     "Print"		 },
{ 0xff62,     "Execute"		 },
{ 0xff63,     "Insert"		 },
{ 0xff65,     "Undo"		 },
{ 0xff66,     "Redo"		 },
{ 0xff67,     "Menu"		 },
{ 0xff68,     "Find"		 },
{ 0xff69,     "Cancel"		 },
{ 0xff6a,     "Help"		 },
{ 0xff6b,     "Break"		 },
{ 0xff7e,     "Mode_switch"	 },
{ 0xff7e,     "script_switch"	 },
{ 0xff7f,     "Num_Lock"	 },
{ 0xff80,     "KP_Space"	 },
{ 0xff89,     "KP_Tab"		 },
{ 0xff8d,     "KP_Enter"	 },
{ 0xff91,     "KP_F1"		 },
{ 0xff92,     "KP_F2"		 },
{ 0xff93,     "KP_F3"		 },
{ 0xff94,     "KP_F4"		 },
{ 0xffbd,     "KP_Equal"	 },
{ 0xffaa,     "KP_Multiply"	 },
{ 0xffab,     "KP_Add"		 },
{ 0xffac,     "KP_Separator"	 },
{ 0xffad,     "KP_Subtract"	 },
{ 0xffae,     "KP_Decimal"	 },
{ 0xffaf,     "KP_Divide"	 },
{ 0xffb0,     "KP_0"		 },
{ 0xffb1,     "KP_1"		 },
{ 0xffb2,     "KP_2"		 },
{ 0xffb3,     "KP_3"		 },
{ 0xffb4,     "KP_4"		 },
{ 0xffb5,     "KP_5"		 },
{ 0xffb6,     "KP_6"		 },
{ 0xffb7,     "KP_7"		 },
{ 0xffb8,     "KP_8"		 },
{ 0xffb9,     "KP_9"		 },
{ 0xffbe,     "F1"		 },
{ 0xffbf,     "F2"		 },
{ 0xffc0,     "F3"		 },
{ 0xffc1,     "F4"		 },
{ 0xffc2,     "F5"		 },
{ 0xffc3,     "F6"		 },
{ 0xffc4,     "F7"		 },
{ 0xffc5,     "F8"		 },
{ 0xffc6,     "F9"		 },
{ 0xffc7,     "F10"		 },
{ 0xffc8,     "F11"		 },
{ 0xffc8,     "L1"		 },
{ 0xffc9,     "F12"		 },
{ 0xffc9,     "L2"		 },
{ 0xffca,     "F13"		 },
{ 0xffca,     "L3"		 },
{ 0xffcb,     "F14"		 },
{ 0xffcb,     "L4"		 },
{ 0xffcc,     "F15"		 },
{ 0xffcc,     "L5"		 },
{ 0xffcd,     "F16"		 },
{ 0xffcd,     "L6"		 },
{ 0xffce,     "F17"		 },
{ 0xffce,     "L7"		 },
{ 0xffcf,     "F18"		 },
{ 0xffcf,     "L8"		 },
{ 0xffd0,     "F19"		 },
{ 0xffd0,     "L9"		 },
{ 0xffd1,     "F20"		 },
{ 0xffd1,     "L10"		 },
{ 0xffd2,     "F21"		 },
{ 0xffd2,     "R1"		 },
{ 0xffd3,     "F22"		 },
{ 0xffd3,     "R2"		 },
{ 0xffd4,     "F23"		 },
{ 0xffd4,     "R3"		 },
{ 0xffd5,     "F24"		 },
{ 0xffd5,     "R4"		 },
{ 0xffd6,     "F25"		 },
{ 0xffd6,     "R5"		 },
{ 0xffd7,     "F26"		 },
{ 0xffd7,     "R6"		 },
{ 0xffd8,     "F27"		 },
{ 0xffd8,     "R7"		 },
{ 0xffd9,     "F28"		 },
{ 0xffd9,     "R8"		 },
{ 0xffda,     "F29"		 },
{ 0xffda,     "R9"		 },
{ 0xffdb,     "F30"		 },
{ 0xffdb,     "R10"		 },
{ 0xffdc,     "F31"		 },
{ 0xffdc,     "R11"		 },
{ 0xffdd,     "F32"		 },
{ 0xffdd,     "R12"		 },
{ 0xffde,     "R13"		 },
{ 0xffde,     "F33"		 },
{ 0xffdf,     "F34"		 },
{ 0xffdf,     "R14"		 },
{ 0xffe0,     "F35"		 },
{ 0xffe0,     "R15"		 },
{ 0xffe1,     "Shift_L"		 },
{ 0xffe2,     "Shift_R"		 },
{ 0xffe3,     "Control_L"	 },
{ 0xffe4,     "Control_R"	 },
{ 0xffe5,     "Caps_Lock"	 },
{ 0xffe6,     "Shift_Lock"	 },
{ 0xffe7,     "Meta_L"		 },
{ 0xffe8,     "Meta_R"		 },
{ 0xffe9,     "Alt_L"		 },
{ 0xffea,     "Alt_R"		 },
{ 0xffeb,     "Super_L"		 },
{ 0xffec,     "Super_R"		 },
{ 0xffed,     "Hyper_L"		 },
{ 0xffee,     "Hyper_R"		 },
#if defined(__hpux)
{ 0x1000ff73,     "hpDeleteChar"     },
{ 0x1000ff72,     "hpInsertChar"     },
{ 0x1000ff70,     "hpInsertLine"     },
{ 0x1000ff71,     "hpDeleteLine"     },
{ 0x1000ff6d,     "hpSystem"         },
{ 0x1000ff74,     "hpBackTab"        },
{ 0x1000ff75,     "hpKP_BackTab"     },
{ 0x1000ff6f,     "hpClearLine"      },
#endif
#if JAPAN
{ 0xff21,     "Kanji"		 },
{ 0xff22,     "Muhenkan"	 },
{ 0xff23,     "Henkan_Mode"	 },
{ 0xff23,     "Henkan"		 },
{ 0xff24,     "Romaji"		 },
{ 0xff25,     "Hiragana"	 },
{ 0xff26,     "Katakana"	 },
{ 0xff27,     "Hiragana_Katakana"},
{ 0xff28,     "Zenkaku"		 },
{ 0xff29,     "Hankaku"		 },
{ 0xff2a,     "Zenkaku_Hankaku"	 },
{ 0xff2b,     "Touroku"		 },
{ 0xff2c,     "Massyo"		 },
{ 0xff2d,     "Kana_Lock"	 },
{ 0xff2e,     "Kana_Shift"	 },
{ 0xff2f,     "Eisu_Shift"	 },
{ 0xff30,     "Eisu_toggle"	 },
#endif
{ 0x1000feb0, "ring_accent"	 },
{ 0x1000fe5e, "circumflex_accent"},
{ 0x1000fe2c, "cedilla_accent"	 },
{ 0x1000fe27, "acute_accent"	 },
{ 0x1000fe60, "grave_accent"	 },
{ 0x1000fe7e, "tilde"		 },
{ 0x1000fe22, "diaeresis"	 },
{ 0x1000ff00, "Remove"		 },
{ 0 }}
#endif
    ;


/* Do it yourself character classification macros, that understand the
   multinational character set, and let me ask some questions the standard
   macros (in ctype.h) don't let you ask. */

GLOBAL char emxcinfo[256]
#ifdef DEFINE
    = {
	eC,	eC,	eC,	eC,	eC,	eC,	eC,	eC,
	eC,	eC|eS,	eC|eS,	eC,	eC,	eC,	eC,	eC,
	eC,	eC,	eC,	eC,	eC,	eC,	eC,	eC,
	eC,	eC,	eC,	eC,	eC,	eC,	eC,	eC,
	eS,	0,	0,	0,	eW,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	eW|eN,	eW|eN,	eW|eN,	eW|eN,	eW|eN,	eW|eN,	eW|eN,	eW|eN,
	eW|eN,	eW|eN,	0,	0,	0,	0,	0,	0,
	0,	eU|eW,	eU|eW,	eU|eW,	eU|eW,	eU|eW,	eU|eW,	eU|eW,
	eU|eW,	eU|eW,	eU|eW,	eU|eW,	eU|eW,	eU|eW,	eU|eW,	eU|eW,
	eU|eW,	eU|eW,	eU|eW,	eU|eW,	eU|eW,	eU|eW,	eU|eW,	eU|eW,
	eU|eW,	eU|eW,	eU|eW,	0,	0,	0,	0,	0,
	0,	eL|eW,	eL|eW,	eL|eW,	eL|eW,	eL|eW,	eL|eW,	eL|eW,
	eL|eW,	eL|eW,	eL|eW,	eL|eW,	eL|eW,	eL|eW,	eL|eW,	eL|eW,
	eL|eW,	eL|eW,	eL|eW,	eL|eW,	eL|eW,	eL|eW,	eL|eW,	eL|eW,
	eL|eW,	eL|eW,	eL|eW,	0,	0,	0,	0,	eC,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0
}
#endif
    ;

/* Functions */

extern char *getcwd();
extern char *getenv();



int	emxabort		_ARGS0();
int	emxaddline		_DEF2(BUFFER *, bp, const char *, text);
int	emxallocerr		_DEF1(unsigned int, size);
int	emxallwindown		_COM();
int	emxallwinup		_COM2();
int	emxanymodified		_ARGS0();
int	emxautoback		_COM();
int	emxautomatch		_COM();
int	emxautosave		_COM();
int	emxbackbuffer		_COM();
int	emxbackchar		_COM2();
int	emxbackdel		_COM2();
int	emxbackdelword		_COM();
int	emxbackisearch		_COM();
int	emxbackline		_COM2();
int	emxbackline6		_COM2();
int	emxbackmatch		_COM();
int	emxbackpage		_COM2();
int	emxbackpara		_COM2();
int	emxbacksearch		_COM();
int	emxbacktab		_COM();
int	emxbackwindow		_COM();
int	emxbackword		_COM2();
void	emxbchange		_DEF2(BUFFER *, bp, int, flag);
int	emxbclear		_DEF1(BUFFER *, bp);
BUFFER *emxbcreate		_DEF1(const char *, bname);
BUFFER *emxbfind		_DEF2(const char *, bname, int, create);
BUFFER *emxbfindfile		_DEF1(const char *, bname);
void	emxbfree		_DEF1(BUFFER *, bp);
void	emxbgoto		_DEF3(BUFFER *, bp, LINE *, lp, int, offset);
int	emxbindmacro		_COM();
int	emxbindtokey		_COM();
int	emxbuffername		_COM();
void	emxbuflistupdate	_ARGS0();
void	emxbupdate		_DEF2(BUFFER *, bp, int, flag);
int	emxcapword		_COM();
int	emxcasesense		_COM();
int	emxcd			_COM();
void	emxchangebegin		_ARGS0();
void	emxchangeend		_ARGS0();
int	emxcmode		_COM();
int	emxcopyline		_COM();
int	emxcopyregion		_COM0();
int	emxcopyword		_COM2();
int	emxcopyregiontomacro	_COM();
int	emxcursorcol		_DEF2(char *, textp, unsigned int, offset);
int	emxcursoroffset		_DEF2(LINE *, lp, int, target);
int	emxcursortype		_COM();
int	emxcutselected		_ARGS0();
int	emxdedent		_COM2();
int	emxdefaultback		_COM();
int	emxdefaultcustomization _ARGS0();
int	emxdelblanklines	_COM();
int	emxdelblanks		_COM();
int	emxdeletebuffer		_DEF1(BUFFER *, bp);
int	emxdeletecurbuf		_ARGS0();
int	emxdeleteline		_COM();
void	emxdeletewindow		_DEF1(WINDOW *, wp);
int	emxdescribekey		_COM();
int	emxdisplaybindings	_COM();
int	emxdisplaybuffers	_ARGS0();
int	emxdisplaycommands	_COM();
int	emxdisplayposition	_COM();
int	emxdisplayversion	_COM();
int	emxdownwindow		_COM();
int	emxdragbutton		_COM();
void	emxeditinit		_DEF2(int, argc, char **, argv);
int	emxendmacro		_COM();
int	emxenlargewindow	_COM2();
void	emxeventhandle		_DEF1(XAnyEvent *, event);
int	emxexecfunc		_DEF4(FUNCTION *, fp, int, f, int, n, KEY *, kp);
int	emxexecute		_DEF3(int, f, int, n, KEY *, kp);
int	emxexecutebuffer	_COM();
int	emxexecutefile		_COM();
int	emxexecutemacro		_COM();
int	emxexitflushall		_COM();
int	emxexpandname		_DEF1(char *, origs);
int	emxextendedcommand	_COM();
int	emxextendedcommandcomplete	_COMP();
int	emxffropen		_DEF2(const char *, namep, int *, accessp);
int	emxfileappend		_COM();
int	emxfiledelete		_COM();
int	emxfilename		_COM();
int	emxfileread		_COM();
int	emxfilereload		_COM();
int	emxfilesave		_COM0();
int	emxfilevisit		_COM();
int	emxfilewrite		_COM();
int	emxfillparagraph	_COM();
BUFFER *emxfindbuffer		_DEF2(const char *, name, Boolean, file);
HIGHSEG*emxfindhighlight	_DEF2(HIGHSEG *, segp, LINE *, lp);
int	emxfirstnonblank	_DEF1(LINE *, linep);
void	emxfixelipsis		_ARGS0();
int	emxflushbuffer		_DEF2(BUFFER *, bp, int, flag);
int	emxflushbuffers		_COM();
void	emxfocusin		_ARGS0();
int	emxforwbuffer		_ARGS0();
int	emxforwchar		_COM2();
int	emxforwdel		_COM2();
int	emxforwdelword		_COM();
int	emxforwisearch		_COM();
int	emxforwline		_COM2();
int	emxforwline6		_COM2();
int	emxforwmatch		_COM();
int	emxforwpage		_COM2();
int	emxforwpara		_COM2();
int	emxforwsearch		_ARGS0();
int	emxforwsearchall	_ARGS0();
int	emxforwwindow		_COM();
int	emxforwword		_COM2();
KEY *	emxgetbinding		_DEF1(KEY *, keyp);
KEY *	emxgetbufkey		_ARGS0();
int	emxgetregion		_DEF1(REGION *, rp);
int	emxgetselectstring	_DEF2(BUFFER *, bp, char **, strpp);
int	emxgotobob		_ARGS0();
int	emxgotobol		_COM();
int	emxgotoeob		_COM();
int	emxgotoeol		_COM();
int	emxgotoline		_COM();
int	emxgotolinenumber	_DEF1(int, n);
int	emxhelp			_COM();
void	emxhoot			_ARGS0();
void	emximhighlight		_DEF5(BUFFER *, bp, LINE *, firstlp, int, firsto, LINE *, lastlp, int, lasto);
void	emximhorizontal		_DEF3(int, row, int, rows, int, cols);
void	emximlowlight		_ARGS0();
void	emximvertical		_DEF1(int, n);
int	emxindent		_COM2();
void	emxinfoupdate		_ARGS0();
int	emxinsertafile		_DEF1(char *, namep);
int	emxinsertchar		_DEF2(int, n, char, c);
int	emxinsertfile		_COM();
int	emxinserttab		_COM();
int	emxinserttext		_COM();
int	emxinsnlandbackup	_COM();
int	emxinsnlandindent	_COM();
int	emxinstoggle		_COM();
void	emxkdelete		_ARGS0();
void	emxkeyloop		_ARGS0();
void	emxkeymapinit		_ARGS0();
int	emxkillbuffer		_COM();
int	emxkillline		_COM();
int	emxkillparagraph	_COM();
int	emxkillregion		_COM2();
int	emxkinsert		_DEF1(int, c);
int	emxkinsstr		_DEF2(unsigned int, len, char *, ptr);
int	emxldelete		_DEF2(int, n, int, kflag);
LINE *	emxlalloc		_DEF1(int, used);
void	emxlfree		_DEF1(LINE *, lp);
int	emxlineshellcmd		_COM();
LINE *	emxlinit		_DEF1(unsigned int, used);
int	emxlinsert		_DEF3(int, n, int, c, const char *, ptr);
int	emxlinsertstring	_DEF2(long, count, const char *, string);
int	emxlnewline		_ARGS0();
int	emxloadbindings		_COM0();
int	emxloadfile		_DEF2(char *, namep, int, theAccess);
int	emxlowerregion		_COM();
int	emxlowerword		_COM();
int	emxlreplace		_DEF3(int, plen, const char *, st, int, f);
int	emxmakeboundmacro	_COM();
void	emxmakebufname		_DEF2(char *, bname, const char *, namep);
int	emxmakemacro		_COM();
MATCH * emxmatchtable		_DEF1(int, newmode);
int	emxmovetolastmatch	_COM();
void	emxmsgadd		_DEF2(const char *, fp, long, arg);
void	emxmsgerase		_ARGS0();
void	emxmsgnotfound		_DEF1(const char *, str);
void	emxmsgprint		_DEF1(const char *, fp);
void	emxmsgprintint		_DEF2(const char *, fp, long, arg);
void	emxmsgprintstr		_DEF2(const char *, fp, const char *, arg);
void	emxmsgput		_DEF1(const char *, fp);
int	emxmsgread		_DEF5(const char *, str, char *, buf, int, nbuf, int, flag, FUNC, funcp);
int	emxmsgreply		_DEF4(const char *, str, char *, buf, int, nbuf, FUNC, funcp);
int	emxmsgyesno		_DEF2(const char *, str, FUNC, funcp);
int	emxnewline		_COM();
int	emxnumericarg		_COM();
int	emxok			_ARGS0();
int	emxonekey		_DEF1(FUNC, funcp);
int	emxonlywindow		_ARGS0();
int	emxoriginalbindings	_COM0();
int	emxpasteprimary		_COM();
int	emxpipeline		_DEF1(BUFFER *, bp);
int	emxplayfile		_DEF2(char *, namep, int, holler);
int	emxplaymacro		_DEF2(unsigned long *, macrop, int, n);
int	emxplaystring		_DEF1(const char *, str);
int	emxpopblist		_DEF1(int, cur);
int	emxpresspaste		_COM();
int	emxpressselect		_COM();
void	emxpromptoff		_ARGS0();
void	emxprompton		_ARGS0();
int	emxpushnewlines		_COM();
int	emxputdate		_COM();
int	emxputlinenumber	_COM();
int	emxputtime		_COM();
int	emxputuserid		_COM();
int	emxqueryreplace		_COM();
int	emxqueryreplaceall	_COM();
int	emxquickunselectall	_ARGS0();
int	emxquit			_DEF1(int, status);
int	emxquote		_COM();
int	emxquotedchar		_DEF2(KEY *, keyp, unsigned char *, chp);
int	emxreadin		_DEF3(const char *, namep, int, acc, int, flag);
int	emxrealtab		_COM();
int	emxrecallword		_DEF3(WINDOW *, wp, char *, buf, int, max);
int	emxrefresh		_COM();
int	emxregdedent		_COM();
int	emxregindent		_COM();
int	emxreleaseselect	_COM();
int	emxrepositionwindow	_COM();
int	emxresetbindings	_COM0();
int	emxsavemacro		_COM();
int	emxsavetobuffer		_COM();
void	emxscrexpose		_DEF4(int, x, int, y, int, width, int, height);
void	emxscrinitsize		_ARGS0();
int	emxscrollleft		_COM();
int	emxscrollright		_COM();
int	emxscrolltoggle		_COM();
void	emxscrresize		_DEF3(int, width, int, height, int, showmsg);
int	emxscrsetsize		_DEF2(int, rows, int, cols);
int	emxsearchagain		_ARGS0();
int	emxsearchformatch	_COM();
int	emxselectbackchar	_COM();
int	emxselectbackline	_COM();
int	emxselectbackword	_COM();
int	emxselectbob		_COM();
int	emxselectbol		_COM();
int	emxselecteob		_COM();
int	emxselecteol		_COM();
int	emxselectforwchar	_COM();
int	emxselectforwline	_COM();
int	emxselectforwword	_COM();
int	emxselectline		_COM();
void	emxselectrange		_DEF2(BUFFER *, bp, REGION *, region);
int	emxselectregion		_ARGS0();
void	emxselectwind		_DEF1(WINDOW *, wp);
int	emxselfinsert		_COM();
int	emxsetfillcolumn	_COM2();
int	emxsetfilldouble	_COM();
int	emxsetmark		_COM();
int	emxsetregion		_DEF6(REGION *, rp, LINE *, limitlp, LINE *, firstlp, int, firsto, LINE *, lastlp, int, lasto);
int	emxsetsize		_COM();
int	emxsettabwidth		_COM();
int	emxshrinkwindow		_COM2();
int	emxsplitvisit		_COM();
int	emxsplitwindow		_ARGS0();
int	emxstmode		_COM();
void	emxstackpop		_ARGS0();
void	emxstackpush		_DEF1(FUNC, keyfunc);
int	emxstartmacro		_COM();
int	emxswapdotandmark	_COM();
void	emxswitchcursor		_DEF1(int, set);
int	emxtogglepcfile		_COM();
int	emxtoggletabsonindent	_COM();
int	emxtrimbuffer		_COM();
int	emxtrimline		_COM();
int	emxtrimoneline		_DEF1(LINE *, lp);
int	emxtrimoutputtoggle	_COM();
int	emxtwiddle		_COM();
int	emxtypemacro		_COM();
int	emxunbindkey		_COM();
int	emxunmodify		_COM();
int	emxunselectall		_ARGS0();
void	emxunselectbuf		_DEF1(BUFFER *, bp);
void	emxupdate		_ARGS0();
int	emxupperregion		_COM();
int	emxupperword		_COM();
int	emxupwindow		_COM2();
int	emxusebuf		_DEF1(BUFFER *, bp);
int	emxusebuffer		_COM();
int	emxusebufname		_DEF2(const char *, bufn, int, create);
int	emxviewfile		_COM();
int	emxwidgetquit		_DEF1(int, status);
int	emxwordwrap		_COM();
WINDOW *emxwpopup		_ARGS0();
int	emxwritestdout		_ARGS0();
int	emxyank			_COM2();
int	emxyankfrombuffer	_COM();

#endif /* LITTLE_EMX_H */
