#define EMXBASIC_C TRUE
/*=========================================================================
 * Copyright (C) Servio Corp. 1991-1994.  All Rights Reserved.
 *
 * Name -
 *
 * Purpose -
 *
 * $Id: emxbasic.c,v 30.5 1994/01/29 02:06:06 marcs Exp $
 *
 *========================================================================*/

/*
    EMX - Basic cursor motion and buffer commands.

    The routines in this file are the basic command functions for moving the
    cursor around on the screen, setting mark, and swapping dot with mark. Only
    moves between lines, which might make the current buffer framing bad, are
    hard.

    The functions in this file are a general set of line management utilities.
    They are the only routines that touch the text. They also touch the buffer
    and window structures, to make sure that the necessary updating gets done.
    There are routines in this file that handle the kill buffer too. It isn't
    here for any good reason.
*/

#include "emx.h"


static int makelist _ARGS0();
static int pickone _ARGS0();


int emxallocerr _ARGS1(unsigned int, size)

{
    emxhoot();
    emxmsgprintint("Out of memory - can't allocate %d bytes", size);
    return FALSE;
}


/* Go to beginning of line */

COMMAND(emxgotobol)

{
    g_curwp->doto = 0;
    return TRUE;
}


/* Go to end of line */

COMMAND(emxgotoeol)

{
    WINDOW *cwp;

    cwp = g_curwp;
    cwp->doto = cwp->dotp->used;
    return TRUE;
}


/* Go to the beginning of the buffer. Setting WFHARD is conservative, but
   almost always the case */

int emxgotobob _ARGS0()

{
    WINDOW *cwp;

    cwp = g_curwp;
    cwp->dotp  = g_curbp->linep->nextp;
    cwp->doto  = 0;
    cwp->flag |= WFHARD;
    return TRUE;
}


/* Go to the end of the buffer. Setting WFHARD is conservative, but almost
   always the case */

COMMAND(emxgotoeob)

{
    WINDOW *cwp;

    cwp = g_curwp;
    cwp->dotp  = g_curbp->linep;
    cwp->doto  = 0;
    cwp->flag |= WFHARD;
    return TRUE;
}


/* Move cursor backwards. Do the right thing if the count is less than 0.
   Error if you try to move back from the beginning of the buffer */

COMMAND2(emxbackchar)

{
    LINE   *lp;
    WINDOW *wp;

    if (n < 0)
	 return emxforwchar(f, -n);

    wp = g_curwp;
    while (n--)
	{
	if (wp->doto == 0)
	    {
	    if ((lp = wp->dotp->prevp) == g_curbp->linep)
		return FALSE;

	    wp->dotp  = lp;
	    wp->doto  = lp->used;
	    wp->flag |= WFMOVE;
	    }

	else
	    wp->doto--;
	}

   return TRUE;
}


/* Move cursor forwards. Do the right thing if the count is less than 0.
   Error if you try to move forward from the end of the buffer */

COMMAND2(emxforwchar)

{
    WINDOW *wp;

    if (n < 0)
	return emxbackchar(f, -n);

    wp = g_curwp;
    while (n--)
	{
	if (wp->doto == wp->dotp->used)
	    {
	    if (wp->dotp == g_curbp->linep)
		return FALSE;

	    wp->dotp  = wp->dotp->nextp;
	    wp->doto  = 0;
	    wp->flag |= WFMOVE;
	    }

	else
	    wp->doto++;
	}

    return TRUE;
}


/* Set the current goal column, which is saved in the external variable
   g_curgoal, to the current cursor column. The column is never off the
   edge of the screen; it's more like display than show position. */

static void setgoal _ARGS0()

{
    WINDOW	    *cwp;
    unsigned int   goal;

    cwp = g_curwp;
    goal = emxcursorcol(cwp->dotp->text, cwp->doto);

    /* Chop to screen width */
    if (goal >= cwp->leftcol + g_ncol)
	goal = cwp->leftcol + g_ncol - 1;

    g_curgoal = goal;
}


/* Move forward by 6 full lines */

COMMAND2(emxforwline6)

{
  emxforwline(f, 6);
}


/* Move backward by 6 full lines */

COMMAND2(emxbackline6)

{
  emxforwline(f, -6);
}


/* Move forward by full lines. If the number of lines to move is less than
   zero, call the backward line function to actually do it. The last
   command controls how the goal column is set. */

COMMAND2(emxforwline)

{
    WINDOW *wp;
    LINE   *lp;

    if (n < 0)
	return emxbackline(f, -n);

    if ((g_lastflag & LASTUPDOWN) == 0)	      /* Fix goal */
	setgoal();

    g_thisflag |= LASTUPDOWN;
    wp = g_curwp;
    lp = wp->dotp;
    while (n-- && lp != g_curbp->linep)
	lp = lp->nextp;

    wp->dotp  = lp;
    wp->doto  = emxcursoroffset(lp, g_curgoal);
    wp->flag |= WFMOVE;
    return TRUE;
}


/* This function is like emxforwline, but goes backwards */

COMMAND2(emxbackline)

{
    LINE   *lp;
    WINDOW *wp;

    if (n < 0)
	return emxforwline(f, -n);

    if ((g_lastflag & LASTUPDOWN) == 0)	      /* Fix goal */
	setgoal();

    g_thisflag |= LASTUPDOWN;
    wp = g_curwp;
    lp = wp->dotp;
    while (n-- && lp->prevp != g_curbp->linep)
	lp = lp->prevp;

    wp->dotp  = lp;
    wp->doto  = emxcursoroffset(lp, g_curgoal);
    wp->flag |= WFMOVE;
    return TRUE;
}


/* Return TRUE if the current character is text, ie not a space, tab, or
   end of line. */

static int intext _ARGS0()

{
    WINDOW *wp;

    wp = g_curwp;
    if (wp->doto == wp->dotp->used)
	return FALSE;

    if (!ISSPACE(wp->dotp->text[wp->doto]))
	return TRUE;

    return FALSE;
}


/* Go to the start of the current paragraph. A paragraph is delimited by a
   change in indentation, with the obscure but traditional exception that a
   fully left-aligned paragraph can begin with an indented line. This code
   can't handle hanging indents, leading indents on indented paragraphs, or
   variable indents. */

COMMAND2(emxbackpara)

{
    WINDOW  *wp;
    int	    pindent;
    int	    indent;

    /* Negative backward is forward */
    if (n < 0)
	return emxforwpara(f, -n);

    wp = g_curwp;
    while (n--)
	{
	/* Move backward into text */
	while (emxbackchar(FALSE, 1) && !intext())
	    ;

	/* Get the initial indent */
	pindent = emxfirstnonblank(wp->dotp);

	/* Check for leading indent */
	if (pindent && wp->dotp->nextp != wp->bufp->linep &&
	    emxfirstnonblank(wp->dotp->nextp) == 0 &&
	    (wp->dotp->prevp == wp->bufp->linep ||
	     emxfirstnonblank(wp->dotp->prevp) != pindent))
	    pindent = 0;

	/* Scan lines until a non-matching indent is found */
	wp->dotp = wp->dotp->prevp;
	while (wp->dotp != wp->bufp->linep)
	    {
	    /* Incompatible indent? */
	    if (wp->dotp->used == 0 ||
		((indent = emxfirstnonblank(wp->dotp)) != pindent &&
		 pindent != 0))
		break;

	    /* Leading indent? */
	    if (indent > 0 && !pindent)
		{
		/* Previous paragraph indented? */
		if (wp->dotp->prevp != wp->bufp->linep &&
		    emxfirstnonblank(wp->dotp->prevp) == indent)
		    break;

		wp->dotp = wp->dotp->prevp;
		break;
		}

	    wp->dotp = wp->dotp->prevp;
	    }

	/* Set up on the next line */
	wp->dotp = wp->dotp->nextp;
	wp->doto = 0;

	/* Move to the first word */
	while (!intext() && emxforwchar(FALSE, 1))
	    ;
	}

    wp->flag |= WFMOVE;	       /* force screen update */
    return TRUE;
}


/* Go to the end of the current paragraph. A paragraph is delimited by a
   change in indentation, with the obscure but traditional exception that a
   fully left-aligned paragraph can begin with an indented line. This code
   can't handle hanging indents, leading indents on indented paragraphs, or
   variable indents. */

COMMAND2(emxforwpara)

{
    WINDOW	*wp;
    int		pindent;

    /* Negative count - move backwards */
    if (n < 0)
	return emxbackpara(f, -n);

    wp = g_curwp;
    while (n--)
	{
	/* first scan forward until we are in a word */
	while (!intext() && emxforwchar(FALSE, 1))
	    ;

	/* Get the initial indent */
	pindent = emxfirstnonblank(wp->dotp);

	/* Check for leading indent */
	if (pindent && wp->dotp->nextp != wp->bufp->linep &&
	    emxfirstnonblank(wp->dotp->nextp) == 0 &&
	    (wp->dotp->prevp == wp->bufp->linep ||
	     emxfirstnonblank(wp->dotp->prevp) != pindent))
	    pindent = 0;

	/* Scan lines until a non-matching indent is found */
	wp->dotp = wp->dotp->nextp;
	while (wp->dotp != wp->bufp->linep)
	    {
	    if (wp->dotp->used == 0 ||
		emxfirstnonblank(wp->dotp) != pindent)
		break;

	    wp->dotp = wp->dotp->nextp;
	    }

	/* Set up on the previous line */
	wp->dotp = wp->dotp->prevp;
	wp->doto = wp->dotp->used;
	}

    wp->flag |= WFMOVE;	       /* force screen update */
    return TRUE;
}


/* Scroll forward by a specified number of lines, or by a full page if no
   argument. The "2" is the window overlap. Because the top line in the
   window is zapped, we have to do a hard update and get it back. */

COMMAND2(emxforwpage)

{
    LINE   *lp;
    WINDOW *wp;
    LINE   *blinep;
    int offset, dot_ptr;

    wp = g_curwp;
    offset = wp->doto;
    dot_ptr = 0;
    lp = wp->linep;
    while (lp != wp->dotp)
	{
	lp = lp->nextp;
	dot_ptr++;
	}

    if (f == FALSE)
	{
	n = wp->rows - 2;	    /* Default scroll */
	if (n <= 0)		    /* Forget the overlap */
	    n = 1;		    /* if tiny window */
	}

    else if (n < 0)
	return emxbackpage(f, -n);

    lp = wp->linep;
    blinep = g_curbp->linep;
    while (n-- && lp != blinep)
	lp = lp->nextp;

    wp->linep = lp;
    while (dot_ptr-- && lp != blinep)
	lp = lp->nextp;

    wp->dotp  = lp;
    if ((int) lp->used < offset)
	offset = lp->used;

    wp->doto  = offset;
    wp->flag |= WFHARD;
    return TRUE;
}


/* This command is like emxforwpage, but it goes backwards. The "2", like
   above, is the overlap between the two windows. The hard update is done
   because the top line in the window is zapped. */

COMMAND2(emxbackpage)

{
    LINE   *lp;
    WINDOW *wp;
    LINE   *blinep;
    int offset, dot_ptr;

    wp = g_curwp;
    offset = wp->doto;
    dot_ptr = 0;
    lp = wp->linep;
    while (lp != wp->dotp)
	{
	lp = lp->nextp;
	dot_ptr++;
	}

    if (f == FALSE)
	{
	n = wp->rows - 2;	    /* Default scroll */
	if (n <= 0)		    /* Don't blow up if the */
	    n = 1;		    /* window is tiny */
	}

    else if (n < 0)
	return emxforwpage(f, -n);

    lp = wp->linep;
    blinep = g_curbp->linep;
    while (n-- && lp->prevp != blinep)
	lp = lp->prevp;

    wp->linep = lp;
    while (dot_ptr-- && lp != blinep)
	lp = lp->nextp;

    wp->dotp = lp;
    if ((int) lp->used < offset)
	offset = lp->used;

    wp->doto  = offset;
    wp->flag |= WFHARD;
    return TRUE;
}


/* Set the mark in the current window to the value of dot. */

COMMAND(emxsetmark)

{
    WINDOW *cwp;
    BUFFER *cbp;

    /* If we're in the buffer list buffer, then the MARK key will */
    /* have special meaning (give user option to save, kill or goto */
    /* a buffer, else it is just the set-mark operation */

    cbp = g_curbp;
    if (cbp->bufname[0] == 'B' && strcmp(cbp->bufname, g_bufferlist) == 0)
	return pickone();

    cwp = g_curwp;
    cwp->markp = cwp->dotp;
    cwp->marko = cwp->doto;
    emxmsgprint("[Mark set]");
    return TRUE;
}


/* Swap the values of dot and mark in the current window. This is pretty
   easy, because all of the hard work gets done by the standard routine
   that moves the mark about. The only possible error is "no mark". */

COMMAND(emxswapdotandmark)

{
    WINDOW *wp;
    LINE   *odotp;
    int    odoto;

    wp = g_curwp;
    if (wp->markp == NULL)
	{
	emxmsgprint(g_msgnomark);
	return FALSE;
	}

    odotp = wp->dotp;
    odoto = wp->doto;
    wp->dotp  = wp->markp;
    wp->doto  = wp->marko;
    wp->markp = odotp;
    wp->marko = odoto;
    wp->flag |= WFMOVE;
    return TRUE;
}


/* This routine will return TRUE if the contents of the buffer is a number.
   If there is a single non-number in the buffer, the contents will be
   assumed not to be a buffer but a buffer name. */

static int isanumber _ARGS1(char *, bufp)

{
    while (*bufp)
	{
	if (*bufp > '9' || *bufp < '0')
	    return FALSE;

	bufp++;
	}

    return TRUE;
}


/* This routine will go to a line number, a buffer or the mark in the
   current window. If there is an argument present, it will goto a line. */

int emxgotolinenumber _ARGS1(int, n)

{
    WINDOW *wp;
    LINE   *clp;

    if (n <= 0)
	{
	emxmsgprint("[Bad line number]");
	return FALSE;
	}

    clp = g_curbp->linep->nextp;
    while (n != 1)
	{
	if (clp == g_curbp->linep)
	    {
	    emxmsgprint("[Line number too large]");
	    return FALSE;
	    }

	clp = clp->nextp;
	--n;
	}

    wp = g_curwp;
    wp->dotp = clp;
    wp->doto = 0;
    wp->flag |= WFMOVE;
    return TRUE;
}


static COMPLETION(gotobuffercomplete)

{
    if (status != TRUE)
	return status;

    return emxusebufname(g_bufname, TRUE);
}


static COMPLETION(emxgotolinecomplete)

{
    WINDOW *wp;
    char	    strbuf[80];

    if (status == ABORT)
	return status;

    wp = g_curwp;
    if (status == FALSE)
	{
	/* Zero means no input - goto mark */
	if (wp->markp == NULL)
	    emxmsgprint(g_msgnomark);
	else
	    {
	    wp->dotp  = wp->markp;
	    wp->doto  = wp->marko;
	    wp->flag |= WFMOVE;
	    }

	return TRUE;
	}

    /* Check for gotobuffer */
    if (isanumber(g_cmdbuf))
	emxgotolinenumber(atoi(g_cmdbuf));

    /* Go to a buffer */
    else if (emxusebufname(g_cmdbuf, FALSE) == FALSE)
	{
	strcpy(g_bufname, g_cmdbuf);
	sprintf(strbuf, "Buffer %s doesn't exist - OK to create?", g_bufname);
	return emxmsgyesno(strbuf, gotobuffercomplete);
	}

    return TRUE;
}


COMMAND(emxgotoline)

{
    /* If no argument, prompt */
    if (f == FALSE)
	return emxmsgread("Goto [line #, buffer or mark] :", g_cmdbuf,
		       sizeof(g_cmdbuf), NEW|AUTO|BUFCMPL, emxgotolinecomplete);

    return emxgotolinenumber(n);
}


/* Update the buffer list display if it's displayed */

void emxbuflistupdate _ARGS0()

{
    if (g_blistp->windows && strcmp(g_bufferlist, g_blistp->bufname) == 0)
	emxdisplaybuffers();
}


/* Attach a buffer to a window. The values of dot and mark come from the
   buffer if the use count is 0. Otherwise, they come from some other
   window. */

static COMPLETION(emxusebuffercomplete)

{
    if (status != TRUE)
	return status;

    return emxusebufname(g_cmdbuf, TRUE);
}


COMMAND(emxusebuffer)

{
    return emxmsgread("Use buffer: ", g_cmdbuf, sizeof(g_cmdbuf), NEW|AUTO|BUFCMPL,
		   emxusebuffercomplete);
}


/* Does all the work for changing to a new buffer for emxusebuffer,
   prev-buf and next-buf. */

int emxusebuf _ARGS1(BUFFER *, bp)

{
    WINDOW *wp;
    WINDOW *cwp;
    BUFFER *cbp;

    cwp = g_curwp;
    cbp = g_curbp;

    /* Last use of current buffer? Update position data */
    if (--cbp->windows == 0)
	{
	cbp->dotp  = cwp->dotp;
	cbp->doto  = cwp->doto;
	cbp->markp = cwp->markp;
	cbp->marko = cwp->marko;
	}

    g_curbp = bp;				/* Switch */
    if (g_modes.automatch)
	(void) emxmatchtable(bp->mode);

    cwp->bufp  = bp;
    cwp->linep = bp->linep;		/* For macros, ignored */
    cwp->flag |= WFINFO|WFFORCE|WFHARD; /* Quite nasty */

    /* First window on this buffer? */
    if (bp->windows++ == 0)
	{
	cwp->dotp  = bp->dotp;
	cwp->doto  = bp->doto;
	cwp->markp = bp->markp;
	cwp->marko = bp->marko;
	}

    else
	/* Look for old */
	for (wp = g_wheadp->nextp; wp; wp = wp->nextp)
	    {
	    if (wp != cwp && wp->bufp == bp)
		{
		cwp->dotp  = wp->dotp;
		cwp->doto  = wp->doto;
		cwp->markp = wp->markp;
		cwp->marko = wp->marko;
		break;
		}

	    }

    return TRUE;
}


/* Does all the work for changing to a new buffer for emxusebuffer,
   prev-buf and next-buf. */

int emxusebufname _ARGS2(const char *, bufn, int, create)

{
    BUFFER *bp;
    int		    status;

    /* Watch for the buffer list name */
    if (*bufn == 'B' && strcmp(g_bufferlist, bufn) == 0)
	bp = g_blistp;

    else if ((bp = emxbfind(bufn, create)) == NULL)
	return FALSE;

    if (status = emxusebuf(bp))
	emxbuflistupdate();

    return status;
}


/* Dispose of a buffer, by name. Ask for the name. Look it up (don't get
   too upset if it isn't there at all!). Clear the buffer (ask if the
   buffer has been changed). Then free the header line and the buffer
   header. Bound to "C-X K". */

static COMPLETION(emxkillbufferverified)

{
    if (status == TRUE)
	if ((status = emxdeletebuffer(g_bufp)) == TRUE)
	    emxok();

    return status;
}


static COMPLETION(emxkillbuffercomplete)

{
    BUFFER *bp;

    if (status != TRUE)
	return status;

    if ((bp = emxbfind(g_cmdbuf, FALSE)) == NULL)
	{
	emxmsgnotfound(g_cmdbuf);
	return TRUE;
	}

    /* Should we ask? */
    if (bp->flag & BFCHG)
	{
	g_bufp = bp;
	return emxmsgyesno(g_msgdiscard, emxkillbufferverified);
	}

    if ((status = emxdeletebuffer(bp)) == TRUE)
	emxok();

    return status;
}


COMMAND(emxkillbuffer)

{
    return emxmsgread("Kill buffer: ", g_cmdbuf, sizeof(g_cmdbuf),
		    NEW|AUTO|BUFCMPL, emxkillbuffercomplete);
}


/* Delete the current buffer */

int emxdeletecurbuf _ARGS0()

{
    int	    status;

    /* Should we ask? */
    if (g_curbp->flag & BFCHG)
	{
	g_bufp = g_curbp;
	return emxmsgyesno(g_msgdiscard, emxkillbufferverified);
	}

    if ((status = emxdeletebuffer(g_curbp)) == TRUE)
	emxok();

    return status;
}


/* Delete a buffer, given a pointer to it */

int emxdeletebuffer _ARGS1(BUFFER *, bp)

{
    WINDOW *wp;
    WINDOW *cwp;
    BUFFER *bp1;
    BUFFER *listbp;
    int    onlybuf;

    listbp = g_blistp;
    if (bp == g_bheadp && !bp->nextp)
	onlybuf = TRUE;
    else
	onlybuf = FALSE;

    /* Watch for attempt to kill solitary '?' buffer */
    if (onlybuf && strcmp(bp->bufname, g_emptybuf) == 0)
	{
	emxmsgprint("[Can't delete '?']");
	return FALSE;
	}

    /* Kill the buffer, asking permission if it has been modified */
    if (bp != listbp)
	{
	emxbclear(bp);

	/* Take the buffer out of the buffer chain and get previous buffer */
	for (bp1 = g_bheadp; bp1 && bp1->nextp != bp; bp1 = bp1->nextp)
	    ;

	if (!bp1)
	    g_bheadp = bp->nextp;
	else
	    bp1->nextp = bp->nextp;

	if (!(bp1 = bp->nextp))
	    bp1 = g_bheadp;
	}

    else
	bp1 = g_bheadp;

    /* Go through all the windows setting each to the correct buffer */
    cwp = g_curwp;
    for (wp = g_wheadp->nextp; wp; wp = wp->nextp)
	{
	/* This one is showing the dead buffer */
	if (wp->bufp == bp)
	    {
	    /* If the only real buffer is dead, use a temporary */
	    g_curwp = wp;
	    g_curbp = bp;

	    /* Use the next buffer, or the buffer list, or the empty buffer */
	    if (bp1)
		emxusebuf(bp1);

	    else if (bp != listbp && listbp->windows)
		emxusebuf(listbp);

	    else
		emxusebufname(g_emptybuf, TRUE);

	    wp->flag |= WFINFO|WFFORCE|WFHARD;
	    }

	}

    /* Reset to the original window */
    emxselectwind(cwp);

    if (bp != listbp)
	{
	FREE(bp->linep);	  /* Release header line */
	FREE(bp);		  /* Release buffer block */
	}

    emxbuflistupdate();
    return TRUE;
}


/* Display the buffer list. This is done in two parts. The "makelist"
   routine figures out the text, and puts it in the buffer whoses header is
   pointed to by the external "g_blistp". The "emxpopblist" then pops the
   data onto the screen. Bound to "C-X C-B". */

int emxdisplaybuffers _ARGS0()

{
    int    s;

    if ((s = makelist()) != TRUE)
	return s;

    return emxpopblist(FALSE);
}


/* Pop the special buffer whose buffer header is pointed to by the external
   variable "g_blistp" onto the screen. This is used by the
   "emxdisplaybuffers" routine (above) and by some other packages. Returns
   a status. If "cur" is TRUE, select the pop-up window. */

int emxpopblist _ARGS1(int, cur)

{
    WINDOW *wp;
    BUFFER *bp;
    BUFFER *listbp;

    listbp = g_blistp;

    /* Not on screen yet */
    if (listbp->windows == 0)
	{
	if ((wp=emxwpopup()) == NULL)
	    return FALSE;

	bp = wp->bufp;
	if (--bp->windows == 0)
	    {
	    bp->dotp  = wp->dotp;
	    bp->doto  = wp->doto;
	    bp->markp = wp->markp;
	    bp->marko = wp->marko;
	    }

	wp->bufp = listbp;
	++listbp->windows;
	}

    for (wp = g_wheadp->nextp; wp; wp = wp->nextp)
	{
	if (wp->bufp == listbp)
	    {
	    wp->linep = wp->dotp = listbp->linep->nextp;
	    wp->markp = NULL;
	    wp->doto  = 0;
	    wp->marko = 0;
	    wp->flag |= WFINFO|WFHARD;
	    break;
	    }

	}

    if (cur)					    /* fitz */
	emxselectwind (wp);
    else if (wp == g_curwp)
	g_curbp = listbp;

    return TRUE;
}


/* Return a count of the characters in a buffer */

static int bufcharcount _ARGS1(BUFFER *, bp)

{
    unsigned int count;
    LINE    *lp;

    count = 0;
    for (lp = bp->linep->nextp; lp != bp->linep; lp = lp->nextp)
	count += lp->used + 1;

    return count;
}


/* This routine will be used soley by 'pickone()'. It will be used to
   determine whether the cursor line is one of the two header lines. It
   uses simple text comparison. Due to the structure and contents of the
   buffer list, certain characters are guaranteed to appear in certain
   locations. This routine will take advantage of this fact. The character
   at 'HEADER_CHAR' in either of the two header lines will be checked to
   see if we're in part of the header. */

#define BUF_MODE	0
#define BUF_READ	5
#define BUF_MODIFY	6
#define BUF_SIZE       10
#define BUF_DASH       14
#define BUF_NAME       17
#define BUF_FILE       34

#define BUF_NAME_MAX   16

#define HEADER_CHAR 12


/* pick a buffer to goto/kill/save */

static COMPLETION(pickonecomplete)

{
    char    ch;
    int	    count;

    if (status != TRUE)
	return status;

    if (g_keyp->modifier == 0 && g_keyp->keysym < 0xff)
	{
	ch = g_keyp->keysym;
	if (ISLOWER(ch))
	    ch = TOUPPER(ch);

	if (ch == 'K')
	    emxdeletebuffer(g_bufp);
	else if (ch == 'G')
	    emxusebuf(g_bufp);
	else if (ch == 'S')
	    {
	    emxusebuf(g_bufp);	    /* goto this buffer, but don't show the user */
	    emxfilesave();
	    emxusebuf(g_blistp);	    /* jump back to this window - HACK ! */
	    emxdisplaybuffers();      /* update the list */
	    }

	else
	    {
	    /* Highlight it again */
	    emxmsgprintstr(g_msggoto, g_bufname);
	    count = strlen(g_bufname);
	    count = count <= BUF_NAME_MAX ? count : BUF_NAME_MAX;
	    emximhighlight(g_blistp, g_linep, BUF_NAME, g_linep,
			BUF_NAME + count);
	    return emxonekey(pickonecomplete);
	    }

	}

    return TRUE;
}


static int pickone _ARGS0()

{
    LINE   *lp;
    BUFFER *bp;
    int		    count;

    /* Figure out which line in the buffer has been picked */
    count = 0;
    for (lp = g_curbp->linep->nextp; lp != g_curwp->dotp; lp = lp->nextp)
	count++;

    if (count < 2 || lp == g_curbp->linep)
	{
	emxmsgprint("[Not a buffer]");
	return FALSE;
	}

    /* Walk the buffer list looking for the right buffer */
    count -= 2;
    for (bp = g_bheadp; count && bp; bp = bp->nextp, count--)
	;

    g_bufp = bp;
    g_linep = lp;
    strcpy(g_bufname, bp->bufname);
    emxmsgprintstr(g_msggoto, g_bufname);

    count = strlen(g_bufname);
    count = count <= BUF_NAME_MAX ? count : BUF_NAME_MAX;
    emximhighlight(g_blistp, lp, BUF_NAME, lp, BUF_NAME + count);
    return emxonekey(pickonecomplete);
}


/* This routine rebuilds the text in the special secret buffer that holds
   the buffer list. It is called by the list buffers command. Return TRUE
   if everything works. Return FALSE if there is an error (if there is no
   memory).
*/

static int makelist _ARGS0()

{
    BUFFER *bp;
    const char	  *ptr;
    char	    line[256];

    strcpy(g_bufferlist, g_blistp->bufname);
    emxbclear(g_blistp);

    if (emxaddline(g_blistp, "Lang Mod    Size Buffer		File") == FALSE ||
	emxaddline(g_blistp, "---- ---    ---- ------		----") == FALSE)
	return FALSE;

    for (bp = g_bheadp; bp != NULL; bp = bp->nextp)
	{
	ptr = " . ";
	if (bp->flag & BFVIEW)
	    ptr = "R/O";
	else if (bp->flag & BFCHG)
	    ptr = " * ";

	sprintf(line, "%4s %3s %7d %-16.16s %s", emxmodetable[bp->mode].modestring,
		ptr, bufcharcount(bp), bp->bufname, bp->filename);

	if (emxaddline(g_blistp, line) == FALSE)
	    return FALSE;
	}

    return TRUE;
}


/* The argument "text" points to a string. Append this line to the buffer
   list buffer. Handcraft the EOL on the end. Return TRUE if it worked and
   FALSE if you ran out of room. */

int emxaddline _ARGS2(BUFFER *, bp, const char *, text)

{
    LINE   *lp;
    int    ntext;

    ntext = strlen(text);
    if ((lp = emxlalloc(ntext)) == NULL)
	return FALSE;

    if (ntext)
	memcpy(lp->text, text, ntext);

    /* Chain it in at the end */
    lp->prevp = bp->linep->prevp;
    if (lp->prevp)
	lp->prevp->nextp = lp;
    bp->linep->prevp = lp;
    lp->nextp = bp->linep;
    return TRUE;
}


/* Look through the list of buffers. Return TRUE if there are any changed
   buffers. Special buffers like the buffer list buffer don't count, as
   they are not in the list. Return FALSE if there are no changed buffers.
   (Don't count save-buffer buffers) or the pipeline buffer */

int emxanymodified _ARGS0()

{
    BUFFER  *bp;

    for (bp = g_bheadp; bp; bp = bp->nextp)
       {
       if ((bp->flag & BFCHG) && !(bp->flag & BFSAV) && !emxpipeline(bp))
	   return TRUE;
       }

    return FALSE;
}


/* Search for a buffer, by name. If not found, and the "create" is TRUE,
   create a buffer and put it in the list of all buffers. Return pointer to
   the BUFFER block for the buffer. */

BUFFER *emxbfind _ARGS2(const char *, bname, int, create)

{
    BUFFER *bp;
    BUFFER *bufp;

    for (bp = g_bheadp; bp; bp = bp->nextp)
	{
	if (strcmp(bname, bp->bufname) == 0)
	    return bp;
	}

    if (create && (bp = emxbcreate(bname)) != NULL)
	{
	/* Attach to the end of the list */
	for (bufp = g_bheadp; bufp && bufp->nextp; bufp = bufp->nextp)
	    ;

	if (bufp)
	    bufp->nextp = bp;
	else
	    g_bheadp = bp;

	bp->nextp = NULL;
	}

    return bp;
}


/* Search for a buffer, by filename. If not found, and the "create" is
   TRUE, create a buffer and put it in the list of all buffers. Return
   pointer to the BUFFER block for the buffer. */

BUFFER *emxbfindfile _ARGS1(const char *, bname)

{
    BUFFER *bp;

    for (bp = g_bheadp; bp; bp = bp->nextp)
	{
	if (strcmp(bname, bp->filename) == 0)
	    break;
	}

    return bp;
}


/* Create a buffer, by name. Return a pointer to the BUFFER header block,
   or NULL if the buffer cannot be created. The BUFFER is not put in the
   list of all buffers. This is called by "edinit" to create the buffer
   list buffer. */

BUFFER *emxbcreate _ARGS1(const char *, bname)

{
    BUFFER *bp;
    LINE   *lp;

    if ((bp = (BUFFER *) malloc(sizeof(BUFFER))) == 0)
	{
	emxallocerr(sizeof(BUFFER));
	return NULL;
	}

    memset(bp, 0, sizeof(BUFFER));

    if ((lp = emxlalloc(0)) == NULL)
	{
	FREE(bp);
	return NULL;
	}

    lp->nextp = lp;
    lp->prevp = lp;

    bp->dotp  = lp;
    bp->linep = lp;
    if (g_curbp)
	bp->mode = g_curbp->mode;
    else
	bp->mode = g_defaultmode;

    strcpy(bp->bufname, bname);
    return bp;
}


/* This routine blows away all of the text in a buffer. If the buffer is
   marked as changed then we ask if it is ok to blow it away; this is to
   save the user the grief of losing text. The window chain is nearly
   always wrong if this gets called; the caller must arrange for the
   updates that are required. Return TRUE if everything looks good. */

int emxbclear _ARGS1(BUFFER *, bp)

{
    WINDOW  *wp;
    LINE    *lp;
    LINE    *nextp;

    /* Point all windows to the empty line */
    for (wp = g_wheadp->nextp; wp; wp = wp->nextp)
	{
	if (wp->bufp == bp)
	    {
	    wp->linep = bp->linep;
	    wp->dotp =	bp->linep;
	    wp->doto  = 0;
	    if (wp->markp)
		{
		wp->markp = bp->linep;
		wp->marko = 0;
		}
	    }
	}

    nextp = bp->linep->nextp;
    while ((lp = nextp) != bp->linep)
	{
	nextp = lp->nextp;
	FREE(lp);
	}

    bp->linep->nextp = bp->linep;
    bp->linep->prevp = bp->linep;

    bp->flag &= ~BFCHG;
    bp->dotp = bp->linep;		/* Fix "." */
    bp->doto = 0;
    bp->markp = NULL;			  /* Invalidate "mark" */
    bp->marko = 0;
    bp->matcho = 0;
    bp->matchp = NULL;
    bp->selectp = NULL;
    bp->highlightp = NULL;
    return TRUE;
}


/* Clear and free a buffer */
void emxbfree _ARGS1(BUFFER *, bp)

{
    emxbclear(bp);

    if (bp->sellistp)
	FREE(bp->sellistp);

    if (bp->highlistp)
	FREE(bp->highlistp);

    FREE(bp->linep);
    FREE(bp);
}


/* Flip to next buffer in the list, wrap to beginning if required */

int emxforwbuffer _ARGS0()

{
    BUFFER *bp;

    if ((bp = g_curbp->nextp) == NULL)
	bp = g_bheadp;

    return emxusebuf(bp);
}


/* Flip to prev buffer in the list, wrap to end if required (wrap around) */

COMMAND(emxbackbuffer)

{
    BUFFER  *bp, *sp, *nextp;

    bp = g_curbp;
    while (TRUE)
	{
	sp = g_bheadp;
	while (TRUE)
	    {
	    nextp = sp->nextp;
	    if ((nextp == NULL) || (nextp == bp))
		break;
	    sp = nextp;
	    }

	if (sp->flag & BFSAV)
	    ;
	else
	    break;

	bp = sp;
	}

    return emxusebuf(sp);
}


/* Yank a buffer into current buffer (try to prevent recursion!!) */

static COMPLETION(emxyankfrombuffercomplete)

{
    LINE   *lp;
    BUFFER *bp = g_curbp;

    if (status != TRUE)
	return status;

    if ((bp = emxbfind(g_cmdbuf, FALSE)) == NULL)
	{
	emxmsgnotfound(g_cmdbuf);
	return FALSE;
	}

    if (strcmp(bp->bufname, g_curbp->bufname) == 0)
	{
	emxmsgprint("[Can't yank to self!]");
	return(FALSE);
	}

    lp = bp->linep->nextp;
    while (TRUE)
	{
	emxlinsert(lp->used, EMXSTR, lp->text);

	if ((lp = lp->nextp) == bp->linep)
	    {
	    if (bp->flag & BFNL)  /* need to know if newline !! (jam ) */
		emxlnewline();
	    break;
	    }
	emxlnewline();
	}

    return emxok();
}


COMMAND(emxyankfrombuffer)

{
    return emxmsgread("Yank from buffer: ", g_cmdbuf, sizeof(g_cmdbuf),
		   NEW|AUTO|BUFCMPL, emxyankfrombuffercomplete);
}


/* Change a buffer's name */

static COMPLETION(emxbuffernamecomplete)

{
    char *p;

    if (status == ABORT)
	return status;

    if (emxbfind(g_cmdbuf, FALSE))
	{
	emxmsgprint("[Another buffer has that name]");
	return FALSE;
	}

    /* Accept only up to first blank */
    for (p = g_cmdbuf; *p && *p != ' '; p++)
      ;

    *p = 0;

    strcpy(g_curbp->bufname, g_cmdbuf);
    emxbupdate(g_curbp, WFINFO);
    emxbuflistupdate();
    return TRUE;
}


COMMAND(emxbuffername)

{
    if (g_curbp == g_blistp)
	{
	emxmsgprint("[Can't change special buffer]");
	return FALSE;
	}

    return emxmsgreply("New buffer name: ", g_cmdbuf, sizeof(g_cmdbuf),
		  emxbuffernamecomplete);
}


/* This function is just like emxlalloc except that it allocates exactly as
   much space as requested. It is used by the read-in functions so that
   large files can be read into memory. emxlinsert is smart enough to
   extend each line when jamming characters into it. */

LINE *emxlinit _ARGS1(unsigned int, used)

{
    LINE	*linep;
    LINE   *lp;

    if ((linep = (LINE *) malloc(sizeof(LINE) + used)) == 0)
	{
	g_curwp->flag |= WFINFO;
	emxallocerr(sizeof(LINE) + used);
	return NULL;
	}

    lp = linep;
    lp->size = used;
    lp->used = used;
    return lp;
}


/* This routine allocates a block of memory large enough to hold a LINE
   containing "used" characters. The block is always rounded up a bit.
   Return a pointer to the new block, or NULL if there isn't any memory
   left. Print a message in the message line if no space. */

LINE *emxlalloc _ARGS1(int, used)

{
    LINE	*linep;
    LINE   *lp;
    int    size;

    /* Adjust to the block size */
    size = (used + LINECHUNK - 1) & ~(LINECHUNK - 1);
    if (size == 0)
	size = LINECHUNK;

    if ((linep = (LINE *) malloc(sizeof(LINE) + size)) == 0)
	{
	g_curwp->flag |= WFINFO;
	emxallocerr(sizeof(LINE) + size);
	return NULL;
	}

    lp = linep;
    lp->size = size;
    lp->used = used;
    return lp;
}


/* Delete line "lp". Fix all of the links that might point at it (they are
   moved to offset 0 of the next line. Unlink the line from whatever buffer
   it might be in. Release the memory. The buffers are updated too; the
   magic conditions described in the above comments don't hold here. */

void emxlfree _ARGS1(LINE *, lp)

{
    BUFFER *bp;
    WINDOW *wp;

    for (wp = g_wheadp->nextp; wp; wp = wp->nextp)
	{
	if (wp->linep == lp)
	    wp->linep = lp->nextp;

	if (wp->dotp  == lp)
	    {
	    wp->dotp  = lp->nextp;
	    wp->doto  = 0;
	    }

	if (wp->markp == lp)
	    {
	    wp->markp = lp->nextp;
	    wp->marko = 0;
	    }

	}

    for (bp = g_bheadp; bp; bp = bp->nextp)
	{
	if (bp->windows == 0)
	    {
	    if (bp->dotp  == lp)
		{
		bp->dotp = lp->nextp;
		bp->doto = 0;
		}

	    if (bp->markp == lp)
		{
		bp->markp = lp->nextp;
		bp->marko = 0;
		}

	    }

	}

    lp->prevp->nextp = lp->nextp;
    lp->nextp->prevp = lp->prevp;
    FREE(lp);
}


/* Goto a position in a buffer - all views */

void emxbgoto _ARGS3(BUFFER *, bp, LINE *, lp, int, offset)

{
    WINDOW  *wp;

    if (!bp->windows)
	{
	bp->dotp = lp;
	bp->doto = offset;
	}

    else
	for (wp = g_wheadp; wp; wp = wp->nextp)
	    {
	    if (wp->bufp == bp)
		{
		wp->dotp = lp;
		wp->doto = offset;
		wp->flag |= WFMOVE;
		}

	    }

}


/* This routine updates all windows showing the given buffer with the given
   flag. */

void emxbupdate _ARGS2(BUFFER *, bp, int, flag)

{
    WINDOW  *wp;

    for (wp = g_wheadp; wp; wp = wp->nextp)
	if (wp->bufp == bp)
	    wp->flag |= flag;
}


/* This routine gets called when a character is changed in place in the
   current buffer. It updates all of the required flags in the buffer and
   window system. The flag used is passed as an argument; if the buffer is
   being displayed in more than 1 window we change EDIT to HARD. Set MODE
   if the infoline needs to be updated (the "*" has to be set). */

void emxbchange _ARGS2(BUFFER *, bp, int, flag)

{
    if (bp->windows != 1)			     /* Ensure hard */
	flag = WFHARD;

    if ((bp->flag & BFCHG) == 0)	  /* First change, so */
	{
	flag |= WFINFO;				/* update infolines */
	bp->flag |= BFCHG;
	}

    emxbupdate(bp, flag);
}


/* Insert "n" copies of the character "c" at the current location of dot.
   In the easy case all that happens is the text is stored in the line. In
   the hard case, the line has to be reallocated. When the window list is
   updated, take special care; I screwed it up once. You always update dot
   in the current window. You update mark, and a dot in another window, if
   it is greater than the place where you did the insert. Return TRUE if
   all is well, and FALSE on errors. */

int emxbinsert _ARGS6(BUFFER *, bp, LINE *, lp1, int, offset, int, n, int, c, char *, ptr)

{
    char   *cp1;
    char   *cp2;
    LINE   *lp2;
    LINE   *lp3;
    char   *limitp;
    WINDOW *wp;

    emxbchange(bp, WFEDIT);

    /* At the end: special */
    if (lp1 == bp->linep)
	{
	if ((lp2=emxlalloc(n)) == NULL) /* Allocate new line */
	    return FALSE;

	lp2->used = 0;
	lp3 = lp1->prevp;		 /* Previous line */
	lp3->nextp = lp2;		 /* Link in */
	lp2->prevp = lp3;
	lp2->nextp = lp1;
	lp1->prevp = lp2;

	/* Reset and fall through */
	lp1 = lp2;
	}

    /* Hard: reallocate */
    if ((int) lp1->used + n > (int) lp1->size)
	{
	if ((lp2=emxlalloc(lp1->used + n)) == NULL)
	    return FALSE;

	/* Copy the characters, leaving a hole for the insert at 'offset' */
	if (offset)
	    memcpy(lp2->text, lp1->text, offset);

	if (lp1->used - offset)
	    memcpy(&lp2->text[offset] + n, &lp1->text[offset], lp1->used - offset);

	lp1->prevp->nextp = lp2;
	lp2->nextp = lp1->nextp;
	lp1->nextp->prevp = lp2;
	lp2->prevp = lp1->prevp;
	FREE(lp1);
	}

    else
	{				 /* Easy: in place */
	lp2 = lp1;			/* Pretend new line */
	lp2->used += n;
	cp2 = &lp1->text[lp1->used];
	cp1 = cp2 - n;
	limitp = &lp1->text[offset];
	while (cp1 != limitp)
	    *--cp2 = *--cp1;
	}

    /* Add the characters - EMXSTR char means use string */
    if (c == EMXSTR)
	memcpy(&lp2->text[offset], ptr, n);
    else
	{
	if (n == 1)
	    lp2->text[offset] = c;
	else
	    memset(&lp2->text[offset], c, n);
	}

    for (wp = g_wheadp; wp; wp = wp->nextp)   /* Update windows */
	{
	if (wp->linep == lp1)
	    wp->linep = lp2;

	if (wp->dotp == lp1)
	    {
	    wp->dotp = lp2;
	    if (wp == g_curwp || wp->doto > offset)
		wp->doto += n;
	    }

	if (wp->markp == lp1)
	    {
	    wp->markp = lp2;
	    if (wp->marko > offset)
		wp->marko += n;
	    }

	}

    return TRUE;
}


/* Insert "n" copies of the character "c" at the current location of dot.
   In the easy case all that happens is the text is stored in the line. In
   the hard case, the line has to be reallocated. When the window list is
   updated, take special care; I screwed it up once. You always update dot
   in the current window. You update mark, and a dot in another window, if
   it is greater than the place where you did the insert. Return TRUE if
   all is well, and FALSE on errors. */

int emxlinsert _ARGS3(int, n, int, c, const char *, ptr)

{
    char   *cp1;
    char   *cp2;
    LINE   *lp1;
    LINE   *lp2;
    LINE   *lp3;
    char   *limitp;
    int    doto;
    WINDOW *wp;

    emxbchange(g_curbp, WFEDIT);
    lp1 = g_curwp->dotp;			  /* Current line */

    /* At the end: special */
    if (lp1 == g_curbp->linep)
	{
	if ((lp2=emxlalloc(n)) == NULL) /* Allocate new line */
	    return FALSE;

	lp2->used = 0;
	lp3 = lp1->prevp;		 /* Previous line */
	lp3->nextp = lp2;		 /* Link in */
	lp2->prevp = lp3;
	lp2->nextp = lp1;
	lp1->prevp = lp2;

	/* Reset and fall through */
	lp1 = lp2;
	g_curwp->dotp = lp1;
	}

    doto = g_curwp->doto;			  /* Save for later */

    /* Hard: reallocate */
    if ((int) lp1->used + n > (int) lp1->size)
	{
	if ((lp2=emxlalloc(lp1->used + n)) == NULL)
	    return FALSE;

	/* Copy the characters, leaving a hole for the insert at 'doto' */
	if (doto)
	    memcpy(lp2->text, lp1->text, doto);

	if (lp1->used - doto)
	    memcpy(&lp2->text[doto] + n, &lp1->text[doto], lp1->used - doto);

	lp1->prevp->nextp = lp2;
	lp2->nextp = lp1->nextp;
	lp1->nextp->prevp = lp2;
	lp2->prevp = lp1->prevp;
	FREE(lp1);
	}

    else
	{				 /* Easy: in place */
	lp2 = lp1;			/* Pretend new line */
	lp2->used += n;
	cp2 = &lp1->text[lp1->used];
	cp1 = cp2 - n;
	limitp = &lp1->text[doto];
	while (cp1 != limitp)
	    *--cp2 = *--cp1;
	}

    /* Add the characters - EMXSTR char means use string */
    if (c == EMXSTR)
	memcpy(&lp2->text[doto], ptr, n);
    else
	{
	if (n == 1)
	    lp2->text[doto] = c;
	else
	    memset(&lp2->text[doto], c, n);
	}

    for (wp = g_wheadp; wp; wp = wp->nextp)   /* Update windows */
	{
	if (wp->linep == lp1)
	    wp->linep = lp2;

	if (wp->dotp == lp1)
	    {
	    wp->dotp = lp2;
	    if (wp == g_curwp || wp->doto > doto)
		wp->doto += n;
	    }

	if (wp->markp == lp1)
	    {
	    wp->markp = lp2;
	    if (wp->marko > doto)
		wp->marko += n;
	    }

	}

    return TRUE;
}


int emxlinsertstring _ARGS2(long, count, const char *, string)

{
    const char	  *ptr;
    const char	  *endp;

    endp = string + count;
    while (string < endp)
	{
	ptr = string;
	while (ptr < endp && *ptr != '\n')
	    ptr++;

	if (ptr > string)
	    emxlinsert(ptr - string, EMXSTR, string);

	if (ptr < endp)
	    emxlnewline();

	string = ptr + 1;
	}
    return TRUE;
}


/* This function moves the characters to the right of the cursor over to
   the next tab stop, set by 'g_tabset'. It scans from the cursor to find
   the first character, then inserts enough blanks to move that character
   and its neighbors over to a tab stop. */

COMMAND2(emxindent)

{
    LINE    *lp;
    char    *ptr;
    char    *endp;
    unsigned int    offset;
    unsigned int    bytes;
    unsigned int    col;

    if (n <= 0)
	return FALSE;

    lp = g_curwp->dotp;
    offset = g_curwp->doto;

    /* At the end of the line - nothing to do */
    if (offset >= lp->used)
	return TRUE;

    /* Find the first non-blank to the right of the cursor */
    ptr = lp->text + offset;
    endp = lp->text + lp->used;
    while (ptr < endp && (*ptr == ' ' || *ptr == '\t'))
	ptr++;

    /* None */
    if (ptr == endp)
	return TRUE;

    /* Calculate the number of spaces to insert */
    col = emxcursorcol(lp->text, ptr - lp->text);
    bytes = (g_tabset * n) - (col % g_tabset);
    g_curwp->doto = ptr - lp->text;
    if (!emxlinsert(bytes, ' ', NULL))
	return FALSE;

    g_curwp->doto = offset;
    return TRUE;

}

/* This function pulls the characters to the right of the cursor back to
   the previous tab stop, set by 'g_tabset'. It scans from the cursor to
   find the first character, then removes enough blanks to move that
   character and its neighbors back to a tab stop. It does nothing if there
   are no spaces before the next character. If there aren't enough to move
   to a tab stop, the characters are pulled back to the cursor. */

COMMAND2(emxdedent)

{
    LINE    *lp;
    unsigned int    offset;
    char    *ptr;
    char    *endp;
    char    *savep;
    unsigned int    bytes;
    unsigned int    col;
    unsigned int    goalcol;
    int		    overshoot;

    if (n <= 0)
	return FALSE;

    lp = g_curwp->dotp;
    offset = g_curwp->doto;

    /* At the end of the line - nothing to do */
    if (offset >= lp->used)
	return TRUE;

    /* Find the first non-blank to the right of the cursor */
    ptr = lp->text + offset;
    endp = lp->text + lp->used;
    while (ptr < endp && (*ptr == ' ' || *ptr == '\t'))
	ptr++;

    /* None */
    if (ptr == endp)
	return TRUE;

    /* Calculate the number of screen spaces to move backward */
    col = emxcursorcol(lp->text, ptr - lp->text);
    bytes = (g_tabset * (n - 1)) + ((col - 1) % g_tabset) + 1;
    goalcol = col - bytes;

    /* Loop back through the spaces and tabs, removing until the desired
       point is hit or overshot */
    savep = ptr;
    endp = lp->text + offset;
    overshoot = 0;
    while (ptr > endp)
	{
	ptr--;
	if ((overshoot = goalcol - emxcursorcol(lp->text, ptr - lp->text)) >= 0)
	    break;
	}

    /* Delete the intervening bytes */
    g_curwp->doto = ptr - lp->text;
    if (savep - ptr)
	if (!emxldelete(savep - ptr, FALSE))
	    return FALSE;

    /* Overshot? Insert some spaces to make for the deleted tab */
    if (overshoot > 0)
	if (!emxlinsert(overshoot, ' ', NULL))
	    return FALSE;

    g_curwp->doto = offset;
    return TRUE;
}


/* Insert a newline into the buffer at the current location of dot in the
   current window. The funny ass-backwards way it does things is not a
   botch; it just makes the last line in the file not a special case.
   Return TRUE if everything works out and FALSE on error (memory
   allocation failure). The update of dot and mark is a bit easier then in
   the above case, because the split forces more updating. */

int emxlnewline _ARGS0()

{
    LINE   *lp1;
    LINE   *lp2;
    int    doto;
    WINDOW *wp;

    emxbchange(g_curbp, WFHARD);
    lp1	 = g_curwp->dotp;			/* Get the address and */
    doto = g_curwp->doto;			/* offset of "." */
    if ((lp2=emxlalloc(doto)) == NULL)	/* New first half line */
	return FALSE;

    /* Shuffle text around */
    if (doto)
	memcpy(lp2->text, lp1->text, doto);

    lp1->used -= doto;
    if (lp1->used)
	memcpy(lp1->text, &lp1->text[doto], lp1->used);

    lp2->prevp = lp1->prevp;
    lp1->prevp = lp2;
    lp2->prevp->nextp = lp2;
    lp2->nextp = lp1;

    for (wp = g_wheadp->nextp; wp; wp = wp->nextp)	    /* Windows */
	{
	if (wp->linep == lp1)
	    wp->linep = lp2;

	if (wp->dotp == lp1)
	    {
	    if (wp->doto < doto)
		wp->dotp = lp2;
	    else
		wp->doto -= doto;
	    }

	if (wp->markp == lp1)
	    {
	    if (wp->marko < doto)
		wp->markp = lp2;
	    else
		wp->marko -= doto;
	    }

	}

    return TRUE;
}


/* Delete a newline. Join the current line with the next line. If the next
   line is the magic header line always return TRUE; merging the last line
   with the header line can be thought of as always being a successful
   operation, even if nothing is done, and this makes the kill buffer work
   "right". Easy cases can be done by shuffling data around. Hard cases
   require that lines be moved about in memory. Return FALSE on error and
   TRUE if all looks ok. Called by "emxldelete" only. */

static int ldelnewline _ARGS0()

{
    char   *cp2;
    LINE   *lp1;
    LINE   *lp2;
    LINE   *lp3;
    WINDOW *wp;

    lp1 = g_curwp->dotp;
    lp2 = lp1->nextp;

    /* At the buffer end */
    if (lp2 == g_curbp->linep)
	{
	/* Removing a blank line at the end */
	if (lp1->used == 0)
	    emxlfree(lp1);

	return TRUE;
	}

    /* The next line's text fits into available space on this line */
    if ((int) lp2->used <= (int) lp1->size - (int) lp1->used)
	{
	if (lp2->used)
	    memcpy(&lp1->text[lp1->used], lp2->text, lp2->used);

	for (wp = g_wheadp->nextp; wp; wp = wp->nextp)
	    {
	    if (wp->linep == lp2)
		wp->linep = lp1;

	    if (wp->dotp == lp2)
		{
		wp->dotp  = lp1;
		wp->doto += lp1->used;
		}

	    if (wp->markp == lp2)
		{
		wp->markp  = lp1;
		wp->marko += lp1->used;
		}
	    }

	lp1->used += lp2->used;
	lp1->nextp = lp2->nextp;
	lp2->nextp->prevp = lp1;

	FREE(lp2);
	return TRUE;
	}

    if ((lp3=emxlalloc(lp1->used+lp2->used)) == NULL)
	return FALSE;

    cp2 = lp3->text;
    if (lp1->used)
	{
	memcpy(cp2, lp1->text, lp1->used);
	cp2 = cp2 + lp1->used;
	}

    if (lp2->used)
	memcpy(cp2, lp2->text, lp2->used);

    lp1->prevp->nextp = lp3;
    lp3->nextp = lp2->nextp;
    lp2->nextp->prevp = lp3;
    lp3->prevp = lp1->prevp;

    for (wp = g_wheadp->nextp; wp; wp = wp->nextp)
	{
	if (wp->linep == lp1 || wp->linep == lp2)
	    wp->linep = lp3;

	if (wp->dotp == lp1)
	    wp->dotp  = lp3;

	else if (wp->dotp == lp2)
	    {
	    wp->dotp  = lp3;
	    wp->doto += lp1->used;
	    }

	if (wp->markp == lp1)
	    wp->markp  = lp3;

	else if (wp->markp == lp2)
	    {
	    wp->markp  = lp3;
	    wp->marko += lp1->used;
	    }
	}

    FREE(lp1);
    FREE(lp2);
    return TRUE;
}


/* This function deletes "n" bytes, starting at dot. It understands how to
   deal with end of lines, etc. It returns TRUE if all of the characters
   were deleted, and FALSE if they were not (because dot ran into the end
   of the buffer. The "kflag" is TRUE if the text should be put in the kill
   buffer. */

int emxldelete _ARGS2(int, n, int, kflag)

{
    char   *cp1;
    char   *cp2;
    LINE   *dotp;
    int    doto;
    int    chunk;
    WINDOW *wp;
    unsigned int   count;

    while (n)
	{
	dotp = g_curwp->dotp;
	doto = g_curwp->doto;
	if (dotp == g_curbp->linep)	/* Hit end of buffer */
	    return FALSE;

	chunk = dotp->used - doto;	/* Size of chunk */
	if (chunk > n)
	    chunk = n;

	/* End of line, merge */
	if (chunk == 0)
	    {
	    emxbchange(g_curbp, WFHARD);
	    if (ldelnewline() == FALSE ||
		(kflag && emxkinsert('\n') == FALSE))
		return FALSE;

	    --n;
	    continue;
	    }

	emxbchange(g_curbp, WFEDIT);
	cp1 = &dotp->text[doto];      /* Scrunch text */
	cp2 = cp1 + chunk;

	/* Kill? */
	if (kflag)
	    if (!emxkinsstr(cp2 - cp1, cp1))
		return FALSE;

	if (count = dotp->used - doto - chunk)
	    memcpy(cp1, cp2, count);

	dotp->used -= chunk;

	/* Fix windows */
	for (wp = g_wheadp; wp; wp = wp->nextp)
	    {
	    if (wp->dotp == dotp && wp->doto >= doto)
		wp->doto = (wp->doto < doto + chunk) ? doto : doto - chunk;

	    if (wp->markp == dotp && wp->marko >= doto)
		wp->marko = (wp->marko < doto + chunk) ? doto : doto - chunk;
	    }

	n -= chunk;
	}

    return TRUE;
}


/* Delete all blanks from the cursor position to the next non-blank */

COMMAND(emxdelblanks)

{
    WINDOW  *wp;
    LINE    *dotp;
    int	    doto;
    int	    count;

    wp = g_curwp;
    dotp = wp->dotp;
    doto = wp->doto;
    count = 0;

    /* Move to the first non-blank */
    while ((wp->doto == wp->dotp->used ||
	    wp->dotp->text[wp->doto] == ' ' ||
	    wp->dotp->text[wp->doto] == '\t') &&
	   emxforwchar(FALSE, 1))
	count++;

    /* Move back and delete the blanks */
    wp->dotp = dotp;
    wp->doto = doto;
    if (count)
	return emxldelete(count, TRUE);

    return TRUE;
}


/* Replace plen characters before dot with argument string. Control-J
   characters in st are interpreted as newlines. There is a casehack
   disable flag (normally it likes to match case of replacement to what was
   there). */

int emxlreplace _ARGS3(int, plen, const char *, st, int, f)

{
    WINDOW *cwp;
    int    rlen;	    /* replacement length */
    int    rtype;	    /* capitalization */
    int    c;		    /* used for random characters */
    int    doto;	    /* offset into line */

    /* Find the capitalization of the word that was found.
       f says use exact case of replacement string (same thing that
       happens with lowercase found), so bypass check. */

    cwp = g_curwp;
    emxbackchar(TRUE, plen);
    rtype = eL;
    c = cwp->dotp->text[cwp->doto];
    if (ISUPPER(c) &&  f==FALSE)
	{
	rtype = eU|eL;
	if (cwp->doto+1 < (int) cwp->dotp->used)
	    {
	    c = cwp->dotp->text[cwp->doto + 1];
	    if (ISUPPER(c))
		rtype = eU;
	    }
	}

    /* Make the string lengths match (either pad the line so that it will fit,
       or remove the excess). Be careful with dot's offset. */
    rlen = strlen(st);
    doto = cwp->doto;
    if (plen > rlen)
	emxldelete(plen-rlen, FALSE);

    else if (plen < rlen)
	{
	if (emxlinsert(rlen-plen, ' ', NULL) == FALSE)
	    return FALSE;
	}

    cwp->doto = doto;

    /* Do the replacement:  If was capital, then place first
       char as if upper, and subsequent chars as if lower.
       If inserting upper, check replacement for case. */

    while ((c = *st++&0xff) != '\0')
	{
	if ((rtype & eU)!=0	&&  ISLOWER(c))
	    c = TOUPPER(c);
	if (rtype == (eU|eL))
	    rtype = eL;
	if (c == '\n')
	    {
	    if (cwp->doto == cwp->dotp->used)
		emxforwchar(FALSE, 1);
	    else
		{
		emxldelete(1, FALSE);
		emxlnewline();
		}
	    }

	else if (cwp->dotp == g_curbp->linep)
	    emxlinsert(1, c, NULL);

	else if (cwp->doto == cwp->dotp->used)
	    {
	    emxldelete(1, FALSE);
	    emxlinsert(1, c, NULL);
	    }

	else
	    cwp->dotp->text[cwp->doto++] = c;
	}

    emxbchange(g_curbp, WFHARD);
    return TRUE;
}


/* Delete all of the text saved in the kill buffer. Called by commands when
   a new kill context is being created. The kill buffer array is released
   if the buffer has grown to immense size. No errors. */

void emxkdelete _ARGS0()

{
    if (g_kbufp)
	{
	g_killused = 0;
	if (g_killsize > g_killincr)
	    {
	    FREE(g_kbufp);
	    g_kbufp = NULL;
	    g_killsize = 0;
	    }
	}

}


static int kenlarge _ARGS1(unsigned int, len)

{
    unsigned int   size;
    char	*nbufp;

    size = (len > g_killincr) ? len : g_killincr;
    if (g_killsize == 0)			    /* first time through? */
	nbufp = (char *) malloc(size);
    else
	nbufp = (char *) realloc(g_kbufp, g_killsize + size);

    /* abort if it fails */
    if (nbufp == 0)
	return emxallocerr(size);

    g_kbufp  = nbufp;		/* point our global at it */
    g_killsize += size;		/* and adjust the size */
    return TRUE;
}


/* Insert a character to the kill buffer, enlarging the buffer if there
   isn't any room. Always grow the buffer in chunks, on the assumption that
   if you put something in the kill buffer you are going to put more stuff
   there too later. Return TRUE if all is well, and FALSE on errors. Print
   a message on errors. */

int emxkinsert _ARGS1(int, c)

{
    if (g_killused == g_killsize)
	if (!kenlarge(1))
	    return FALSE;

    g_kbufp[g_killused++] = c;
    return TRUE;
}


int emxkinsstr _ARGS2(unsigned int, len, char *, ptr)

{
    if (!len)
	return TRUE;

    if (g_killsize < g_killused + len)
	if (!kenlarge(len))
	    return FALSE;

    memcpy(&g_kbufp[g_killused], ptr, len);
    g_killused += len;
    return TRUE;
}


/* Find the portion of a highlight list which matches the given line pointer */

HIGHSEG *emxfindhighlight _ARGS2(HIGHSEG *, segp, LINE *, lp)

{
    if (segp)
	{
	while (segp->lp && (segp->lp != lp))
	    segp++;

	if (segp->lp)
	    return segp;
	}

    return NULL;
}


/* Clear out all select lists */

int emxquickunselectall _ARGS0()

{
    BUFFER  *bp;

    /* Visit all buffers */
    for (bp = g_bheadp; bp; bp = bp->nextp)
	{
	if (bp->selectp)
	    {
	    /* Clear this list - leave the space */
	    bp->selectp = NULL;
	    emxbupdate(bp, WFHARD);
	    }

	}

    if (g_blistp->selectp)
	{
	g_blistp->selectp = NULL;
	emxbupdate(g_blistp, WFHARD);
	}

    return TRUE;
}


int emxunselectall _ARGS0()

{
#ifdef STANDALONE
    Window Sown;
#endif
    if (emxquickunselectall()) {
#ifdef STANDALONE
      Sown = XGetSelectionOwner(g_display, XA_PRIMARY);
      if (Sown == g_mainwin) {
        XSetSelectionOwner(g_display, XA_PRIMARY, None, CurrentTime);
      }
#else
      /* Disown the selection, if ours */
      XtDisownSelection((Widget) emx, XA_PRIMARY, XtLastTimestampProcessed(g_display));
#endif
      return TRUE;
    }
    else {
      return FALSE;
    }
}


/* Unselect a single buffer */

void emxunselectbuf _ARGS1(BUFFER *, bp)

{
#ifdef STANDALONE
    Window Sown;
#endif
    if (bp->selectp) {
	/* Clear this list */
	bp->selectp = NULL;
	emxbupdate(bp, WFHARD);

#ifdef STANDALONE
      Sown = XGetSelectionOwner(g_display, XA_PRIMARY);
      if (Sown == g_mainwin) {
        XSetSelectionOwner(g_display, XA_PRIMARY, None, CurrentTime);
      }
#else
      /* Disown the selection, if ours */
      XtDisownSelection((Widget) emx, XA_PRIMARY, XtLastTimestampProcessed(g_display));
#endif
    }
}

/* Get a character position from a given line and offset in a buffer */

static long getposition _ARGS3(BUFFER *, bp, LINE *, lp, int, offset)

{
    LINE   *clp;
    int    nchar;

    clp = bp->linep->nextp;
    nchar = 0;
    while (clp != lp)
	{
	nchar += clp->used + 1;
	clp = clp->nextp;
	}

    nchar += offset;
    return nchar;
}


/* Return the start and end positions of the selection in a buffer */

static int getselectpos _ARGS3(BUFFER *, bp, long *, startp, long *, endp)

{
    HIGHSEG *highp;

    if (!(highp = bp->selectp))
	return FALSE;

    if (startp)
	*startp = getposition(bp, highp->lp, highp->firsto);

    while ((highp + 1)->lp)
	highp++;

    if (endp)
	*endp = getposition(bp, highp->lp, highp->endo);

    return TRUE;
}


int emxgetselectstring _ARGS2(BUFFER *, bp, char **, strpp)

{
    long    start, end;
    char    *ptr;
    HIGHSEG *highp;
    int	    cnt;

    if (!bp->selectp)
	return FALSE;

    getselectpos(bp, &start, &end);

    *strpp = (char *) malloc(end - start + 1 + 1 + 1);
    ptr = *strpp;

    /* Copy out the string */
    highp = bp->selectp;

    while (highp->lp)
	{
	cnt = highp->endo - highp->firsto;
	if (highp->endo > (int) highp->lp->used)
	    cnt--;

	if (cnt > 0)
	    {
	    memcpy(ptr, &highp->lp->text[highp->firsto], cnt);
	    ptr += cnt;
	    }

	if (highp->endo > (int) highp->lp->used ||
	    (highp->endo >= (int) highp->lp->used &&
	     (highp + 1)->lp == highp->lp->nextp &&
	     (highp + 1)->firsto == 0))
	    *ptr++ = '\n';

	highp++;
	}

    *ptr = 0;
    return TRUE;
}



#ifndef STANDALONE

#if defined(FLG_PROTOTYPE)
static int convertselected (EmxWidget w, Atom *selection, Atom *target,
    Atom *typep, XtPointer *valuep, unsigned long *lenp, int *formatp)
#else
static int convertselected (w, selection, target, typep, valuep, lenp, formatp)

EmxWidget   w;
Atom	    *selection;
Atom	    *target;
Atom	    *typep;
XtPointer	    *valuep;
unsigned long *lenp;
int	    *formatp;
#endif
{
    BUFFER  *bp;

    emx = w;

    /* Convert to STRING only */
    if (*target == XA_STRING)
	{
	/* Current buffer selected? */
	bp = g_curbp;
	if (!bp->selectp)
	    {
	    for (bp = g_bheadp; bp; bp = bp->nextp)
		if (bp->selectp)
		    break;
	    }

	if (!bp || !emxgetselectstring(bp, (char **) valuep))
	    return FALSE;

	*lenp = strlen(*valuep);
	*formatp = 8;
	*typep = XA_STRING;
	return TRUE;
	}

    return FALSE;
}


/* This function is a callback, and so has not been entered through the
   normal event handler */

#if defined(FLG_PROTOTYPE)
static void pastedata (EmxWidget w, XtPointer ignored, Atom *selection,
    Atom *type, XtPointer value, unsigned long *len, int *format)
#else
static void pastedata (w, ignored, selection, type, value, len, format)

EmxWidget   w;
XtPointer   ignored;
Atom	    *selection;
Atom	    *type;
XtPointer	    value;
unsigned long *len;
int	    *format;
#endif
{
    emx = w;
#ifndef CYGWIN
    emxchangebegin();
#endif

    /* See if anything came in */
    if (*type != XA_STRING || *len == 0)
	{
	emxmsgprint("[Nothing to paste]");
#ifndef CYGWIN
	emxchangeend();
#endif
	return;
	}

    /* Insert the string at the current position */
    emxlinsertstring(*len, value);
    FREE(value);
#ifndef CYGWIN
    emxchangeend();
#endif
}

#endif /* standalone */


/* Mark a range of lines and offsets as highlit */

void emxselectrange _ARGS2(BUFFER *, bp, REGION *, region)
{
    HIGHSEG *highp;
    int	    offset;
    LINE    *lp;
    Window Sown;

    /* Make sure there is space for the select list */
    if (bp->sellistp && (bp->selsize < region->lines + 1))
	{
	FREE(bp->sellistp);
	bp->sellistp = NULL;
	}

    if (!bp->sellistp)
	{
	bp->selsize = region->lines + 1;
	bp->sellistp = (HIGHSEG *) malloc(bp->selsize * sizeof(HIGHSEG));
	}

    bp->selectp = bp->sellistp;
    highp = bp->sellistp;
    offset = region->firsto;
    lp = region->firstlp;
    while (lp != region->lastlp)
	{
	highp->lp = lp;
	highp->firsto = offset;
	highp->endo = lp->used;
	highp++;
	offset = 0;
	lp = lp->nextp;
	}

    highp->lp = lp;
    highp->firsto = offset;
    highp->endo = region->lasto;
    highp++;

    highp->lp = NULL;

    emxbupdate(bp, WFHARD);

#ifdef STANDALONE
    /* Take ownership of the primary selection.  selection events will
     * be delivered to the window [bjs 8/8/07]*/
    /*printf("emxselectrange calling XSetSelectionOwner\n");*/
    Sown = XGetSelectionOwner(g_display, XA_PRIMARY);
    /* some apps (exceed) don't request property conversion if there's already
       a property there, so force conversion of the selection now by always
       calling XSetSelectionOwner */
    /*if (Sown != g_mainwin) {*/
      XSetSelectionOwner(g_display, XA_PRIMARY, g_mainwin, CurrentTime);
    /*}*/
#else
    /* Take ownership of the primary selection */
    XtOwnSelection((Widget) emx, XA_PRIMARY, XtLastTimestampProcessed(g_display),
		(XtConvertSelectionProc) convertselected,
		(XtLoseSelectionProc) NULL,
		(XtSelectionDoneProc) NULL);
#endif
}

COMMAND(emxpasteprimary)

{
#ifdef STANDALONE
    Window Sown;
    XEvent event;
    Atom type;
    unsigned char *data;
    int format, result;
    unsigned long len, bytes_left, bytes_after_return;
    
    Sown = XGetSelectionOwner(g_display, XA_PRIMARY);
    /*printf("in emxpasteprimary\n");*/
    /* if it's this editor, don't transfer the data through X */
    if (Sown == g_mainwin) {
      char *valuep;
      BUFFER *bp;
      bp = g_curbp;
      if (!bp->selectp) {
	for (bp=g_bheadp; bp; bp=bp->nextp) {
	  if (bp->selectp) {
	    break;
	  }
	}
      }
      if (bp && emxgetselectstring(bp, &valuep)) {
	len = strlen(valuep);
        emxlinsertstring(len, valuep);
        FREE(valuep);
      }
    }
    else if (Sown != None) {
      XConvertSelection(g_display, XA_PRIMARY, XA_STRING, XA_PRIMARY, g_mainwin, CurrentTime); /* myprop was None */
      XFlush(g_display);
      XPeekEvent(g_display, &event);
      if (event.type != SelectionNotify) {
        emxmsgprint("did not receive SelectionNotify event for past");
        return FALSE;
      }
      else {
        XNextEvent(g_display, &event);
      }
      /*printf("proceeding with paste. mainwin=%d, sown=%d\n", g_mainwin, Sown);*/
      if (event.xselection.property == None) {
        emxmsgprint("[Nothing to paste]");
        return FALSE;
      }
      /* trick to find out how much data is there */
      XGetWindowProperty(g_display, g_mainwin, XA_PRIMARY, 0, 0, 0, AnyPropertyType, /* myprop was XA_PRIMARY */
        &type, &format, &len, &bytes_left, &data);
      if (type != XA_STRING) {
        emxmsgprint("[Nothing appropriate to paste]");
        return FALSE;
      }
      if (bytes_left <= 0) {
        emxmsgprint("[Nothing to paste]");
        return FALSE;
      }
      /*printf("len=%d\n", bytes_left);*/
      result = XGetWindowProperty(g_display, g_mainwin, XA_PRIMARY, 0, bytes_left, /* was XA_PRIMARY */
	0, XA_STRING, &type, &format, &len, &bytes_after_return, &data);
      if (result == Success) {
	/* bytes are now pointed to by 'data' */
        len = strlen(data);
	emxlinsertstring((long)len, (const char *)data);
	FREE(data);
      }
      else {
        emxmsgprint("[Unable to get PRIMARY buffer]");
      }
    }
#else
    /* Request the current primary selection */
    XtGetSelectionValue((Widget) emx, XA_PRIMARY, XA_STRING,
			(XtSelectionCallbackProc) pastedata,
			(XtPointer) NULL, XtLastTimestampProcessed(g_display));
#endif
    return TRUE;
}

