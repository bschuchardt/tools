#define EMXCMD_C TRUE
/*=========================================================================
 * Copyright (C) Servio Corp. 1991-1994.  All Rights Reserved.
 *
 * Name -
 *
 * Purpose -
 *
 * $Id: emxcmd.c,v 30.6 1994/09/14 00:31:42 marcs Exp $
 *
 *========================================================================*/

/*
    EMX - Various commands.

   The file contains the command processors for a large assortment of unrelated
   commands.  The only thing they have in common is that they are all command
   processors.

   The routines in this file deal with the region, that magic space
   between "." and mark. Some functions are commands. Some functions are
   just for internal use.

   The routines in this file implement commands that work word at a time.
   There are all sorts of word mode commands.  If I do any sentence and/or
   paragraph mode commands, they are likely to be put in this file.
*/


#include "emx.h"
#include <time.h>


#if !defined(GEODE)
#include <pwd.h>
/* [bruce - this isn't used here]
#define HostOpenFile(name,mode,err,userfile) fopen(name,mode)
*/
#define HOST_FCLOSE(f) fclose(f)
#define HostRemoveSingleFile(name,userfile,err) unlink(name)
#define HostErrSType int
#define HostSpawn(x) system(x)
#else
#include <host.hf>
#endif

int emxok (VOID)

{
    emxmsgprint(g_msgok);
    return TRUE;
}


/* This function checks and sets global variables related to the kill
   buffer. Whenever a kill-type command is issued, a check is first made to
   see if the previous command was one also. If not, the kill buffer is
   cleared. Then, the kill-type flag is set so the next command will work
   right. */

static void killcommand _ARGS0()

{
    if (!(g_lastflag & LASTKILL))
	emxkdelete();

    g_thisflag |= LASTKILL;
}


/* Kill the region. Ask "emxgetregion" to figure out the bounds of the
   region. Move "." to the start, and kill the characters. */

COMMAND2(emxkillregion)

{
    REGION  region;

    if (!emxgetregion(&region))
	return FALSE;

    killcommand();

    g_curwp->dotp = region.firstlp;
    g_curwp->doto = region.firsto;
    if (!emxldelete(region.chars, TRUE))
	return FALSE;

    emxmsgprint("[Deleted]");
    return TRUE;
}


/* Cut all selected text to the kill buffer */

int emxcutselected _ARGS0()

{
    BUFFER  *bp;
    HIGHSEG *highp;
    int	    count;

    killcommand();

    for (bp = g_bheadp; bp; bp = bp->nextp)
	{
	/* Do nothing if nothing is selected */
	if (highp = bp->selectp)
	    {
	    /* Go to this buffer */
	    if (bp != g_curbp)
		emxusebuf(bp);

	    /* Watch for switching to a read-only buffer */
	    if (bp->flag & BFVIEW)
		{
		emxmsgprint(g_errreadonly);
		return FALSE;
		}

	    g_curwp->dotp = highp->lp;
	    g_curwp->doto = highp->firsto;

	    /* Delete each chunk in the select list */
	    while (highp->lp)
		{
		count = highp->endo - highp->firsto;

		/* See about removing the whole line */
		if (highp->endo == highp->lp->used &&
		    (highp + 1)->lp == highp->lp->nextp &&
		    (highp + 1)->firsto == 0)
		    count++;

		if (!emxldelete(count, TRUE))
		    return FALSE;

		highp++;
		}

	    bp->selectp = NULL;
	    }

	}

    return TRUE;
}


/* Select the characters in the region, not moving dot */

int emxselectregion _ARGS0()

{
    REGION  region;

    if (!emxgetregion(&region))
	return FALSE;

    emxquickunselectall();
    emxselectrange(g_curbp, &region);
    return TRUE;
}


/* Copy the word under the cursor to the kill buffer */

COMMAND2(emxcopyword)

{
    int	    len;
    char    buf[2048];

    if (n < 0)
	return FALSE;

    killcommand();

    while (n--)
	{
	emxforwword(FALSE, 1);
	emxbackword(FALSE, 1);
	if (!(len = emxrecallword(g_curwp, buf, sizeof(buf))))
	    return FALSE;

	if (!emxkinsstr(len, buf))
	    return FALSE;

	emxforwword(FALSE, 1);
	}

    emxmsgprint(g_msgcopied);
    return TRUE;
}


/* Copy the characters in the region to the kill buffer, not moving dot */

COMMAND0(emxcopyregion)

{
    LINE	    *linep;
    int	    loffs;
    unsigned int   count;
    REGION		    region;

    if (!emxgetregion(&region))
	return FALSE;

    killcommand();

    linep = region.firstlp;		    /* Current line */
    loffs = region.firsto;		    /* Current offset */
    if (region.chars > KILLCHUNK)
	g_killincr = region.chars;

    while (region.chars)
	{
	/* End of line */
	if (loffs == linep->used)
	    {
	    if (!emxkinsert('\n'))
		return FALSE;

	    linep = linep->nextp;
	    loffs = 0;
	    region.chars--;
	    }

	else
	    {
	    count = linep->used - loffs;
	    if (count > region.chars)
		count = region.chars;

	    if (!emxkinsstr(count, &linep->text[loffs]))
		return FALSE;

	    loffs += count;
	    region.chars -= count;
	    }

	}

    g_killincr = KILLCHUNK;
    emxmsgprint(g_msgcopied);
    return TRUE;
}


/* Copy the characters in the current line to the kill buffer, not moving
   dot */

COMMAND(emxcopyline)

{
    LINE   *linep;

    if (n <= 0)
	return FALSE;

    killcommand();

    while (n--)
	{
	if (g_curwp->dotp == g_curbp->linep)
	    break;

	linep = g_curwp->dotp;
	if (linep->used)
	    if (!emxkinsstr(linep->used, linep->text))
		return FALSE;

	if (!emxkinsert('\n'))
	    return FALSE;

	/* Move the cursor to the start of the next line */
	g_curwp->dotp = g_curwp->dotp->nextp;
	g_curwp->doto = 0;
	g_curwp->flag |= WFMOVE;
	}

    emxmsgprint(g_msgcopied);
    return TRUE;
}


/* This function indents all of the lines in a region 'n' times. It simply
   loops through the lines in the region, indenting each one. indent is
   smart enough to figure out how to handle each line. The line is indented
   from the current cursor (dot) position on each line. */

COMMAND(emxregindent)

{
    LINE   *linep;
    REGION	    region;
    unsigned int    len;

    if (!emxgetregion(&region))
	return FALSE;

    emxbchange(g_curbp, WFHARD);
    linep = region.firstlp;
    while (TRUE)
	{
	len = linep->used + 1;
	if (linep == region.firstlp)
	    len -= region.firsto;

	g_curwp->dotp = linep;
	if (!emxindent(f, n))
	    break;

	if (region.chars < len)
	    break;

	region.chars -= len;

	/* indent may change dotp */
	linep = g_curwp->dotp->nextp;
	}

    return emxok();
}


/* This function dedents all of the lines in a region 'n' times. It simply
   loops through the lines in the region, dedenting each one. dedent is
   smart enough to figure out how to handle each line. The line is dedented
   from the current cursor (dot) position on each line. */

COMMAND(emxregdedent)

{
    LINE   *linep;
    REGION	    region;
    unsigned int    len;

    if (!emxgetregion(&region))
	return FALSE;

    emxbchange(g_curbp, WFHARD);
    linep = region.firstlp;
    while (TRUE)
	{
	len = linep->used + 1;
	if (linep == region.firstlp)
	    len -= region.firsto;

	g_curwp->dotp = linep;
	if (!emxdedent(f, n))
	    break;

	if (region.chars < len)
	    break;

	region.chars -= len;

	/* indent may change dotp */
	linep = g_curwp->dotp->nextp;
	}

    return emxok();
}


/* Lower case region. Zap all of the upper case characters in the region to
   lower case. Use the region code to set the limits. Scan the buffer,
   doing the changes. Call "emxbchange" to ensure that redisplay is done in
   all buffers.
*/

COMMAND(emxlowerregion)

{
    LINE   *linep;
    char   *textp;
    char   *endp;
    int    c;
    REGION	region;

    if (!emxgetregion(&region))
	return FALSE;

    emxbchange(g_curbp, WFHARD);
    linep = region.firstlp;
    textp = linep->text + region.firsto;
    endp = linep->text + linep->used;
    while (region.chars--)
	{
	if (textp == endp)
	    {
	    linep = linep->nextp;
	    textp = linep->text;
	    endp = textp + linep->used;
	    }

	else
	    {
	    c = *textp;
	    if (ISUPPER(c))
		*textp += 0x20;

	    textp++;
	    }

	}

    return TRUE;
}

/* Upper case region. Zap all of the lower case characters in the region to
   upper case. Use the region code to set the limits. Scan the buffer,
   doing the changes. Call "emxbchange" to ensure that redisplay is done in
   all buffers.
*/

COMMAND(emxupperregion)

{
    LINE   *linep;
    char   *textp;
    char   *endp;
    int    c;
    REGION	region;

    if (!emxgetregion(&region))
	return FALSE;

    emxbchange(g_curbp, WFHARD);
    linep = region.firstlp;
    textp = linep->text + region.firsto;
    endp = linep->text + linep->used;
    while (region.chars--)
	{
	if (textp == endp)
	    {
	    linep = linep->nextp;
	    textp = linep->text;
	    endp = textp + linep->used;
	    }

	else
	    {
	    c = *textp;
	    if (ISLOWER(c))
		*textp -= 0x20;

	    textp++;
	    }

	}

    return TRUE;
}


/* This routine figures out the bound of the region in the current window,
   and stores the results into the fields of the REGION structure. Dot and
   mark are usually close together, but I don't know the order, so I scan
   outward from dot, in both directions, looking for mark. The size is kept
   in a long. At the end, after the size is figured out, it is assigned to
   the size field of the region structure. If this assignment loses any
   bits, then we print an error. This is "type independent" overflow
   checking. All of the callers of this routine should be ready to get an
   ABORT status, because I might add a "if regions is big, ask before
   clobberring" flag. */

int emxgetregion _ARGS1(REGION *, rp)

{
    LINE   *blinep;
    WINDOW *cwp;

    blinep = g_curbp->linep;
    cwp = g_curwp;
    if (cwp->markp == NULL)
	{
	emxmsgprint(g_msgnomark);
	return FALSE;
	}

    /* "chars" always ok */
    return emxsetregion(rp, blinep, cwp->dotp, cwp->doto, cwp->markp, cwp->marko);
}


/* Save some region in a buffer (use emxusebuf to handle non-existent
   buffers) hack as it uses kill buffer to transfer stuff (quick and
   dirty!) and doesn't do clever things at all with dot in destination
   buffer! */

static COMPLETION(emxsavetobuffercomplete)

{
    BUFFER *bp;

    if (status != TRUE)
	return status;

    if (strcmp(g_cmdbuf, g_curbp->bufname) == 0)
	{
	emxmsgprint("[Can't save to self!]");
	return FALSE;
	}

    /* Save the buffer pointer */
    bp = g_curbp;

    /* copy stuff using killbuffer as work space -  hack !!
       then move it to named place using yank - Quick AND Dirty */

    emxcopyregion();
    emxusebufname(g_cmdbuf,TRUE);

    if (g_curbp->filename[0] == 0)    /* if a name, already existed, */
	g_curbp->flag |= BFSAV;	    /* else mark as save buffer */

    emxyank(FALSE, 1);
    emxkdelete();	    /* clean out kill buffer */

    emxusebuf(bp);
    return emxok();
}


COMMAND(emxsavetobuffer)

{
    return emxmsgreply("Save in buffer: ", g_cmdbuf, sizeof(g_cmdbuf),
		  emxsavetobuffercomplete);
}


COMMAND(emxwordwrap)

{
    g_modes.wordwrap ^= 1;
    if (g_modes.wordwrap)
	emxmsgprint("[Word wrap enabled]");
    else
	emxmsgprint("[Word wrap disabled]");

    return TRUE;
}


/* This routine will insert the user's id at the cursor position */

COMMAND(emxputuserid)

{
    char	userid[2048];

    /* Get the user ID */
#if defined(MACOS)
    strcpy(userid, getlogin());
#else
    cuserid(userid);
#endif
    if (userid[0] == '\0')
	return FALSE;

    /* Put the user id into the buffer at the cursor position */
    return emxlinsert(strlen(userid), EMXSTR, userid);
}


/* This routine will insert the current time at the cursor position */

COMMAND(emxputtime)

{
    char    *ptr;
    long    clock;
    char    *bufp;

    /* Get the date and time */
    if ((clock = time(0)) == -1)
	return FALSE;

    bufp = ctime(&clock);

    /* select the part of the string to be used */
    ptr = bufp + 11;
    *(bufp + 19) = '\0';

    /* Put the time into the current buffer at the cursor location */
    return emxlinsert(strlen(ptr), EMXSTR, ptr);
}


/* This routine will insert the current date at the cursor position */

COMMAND(emxputdate)

{
    long    clock;
    char    date[10];
    struct tm	*dt;

    /* Get the date and time */
    if ((clock = time(0)) == -1)
	return FALSE;

    dt = localtime(&clock);

    sprintf(date,"%02d/%02d/%02d", dt->tm_mon + 1, dt->tm_mday, dt->tm_year);

    /* Put the date into the current buffer at the cursor location */
    return emxlinsert(strlen(date), EMXSTR, date);
}


/* Set fill column to n */

static COMPLETION(emxsetfillcolumncomplete)

{
    int	    n;

    if (status != TRUE)
	return status;

    n = atoi(g_cmdbuf);
    return emxsetfillcolumn(TRUE, n);
}


COMMAND2(emxsetfillcolumn)

{
    if (!f)
	return emxmsgreply("Fill column: ", g_cmdbuf, sizeof(g_cmdbuf),
		      emxsetfillcolumncomplete);

    if (n <= 0 || n > 255)
	{
	emxmsgprintint("[%d is not a legal fill column]", n);
	return FALSE;
	}

    g_fillcol = n;
    emxmsgprintint("[Fill column is %d]", n);
    return TRUE;
}


/* Set the current tab stop width. This affects only the emxinserttab and
   indent/dedent commands */

COMMAND(emxsettabwidth)

{
    if (f)
	g_tabset = n;

    emxmsgprintint("[Tab stop width is %d]", g_tabset);
    return TRUE;
}


/* Put the current line number into the text at the cursor position */

COMMAND(emxputlinenumber)

{
    LINE   *clp;
    WINDOW *cwp;
    int    nline;
    char	    buf[256];

    cwp = g_curwp;
    clp = g_curbp->linep->nextp;	    /* Collect the data */
    nline = 1;				    /* Origin 1 */
    while (clp != cwp->dotp)
	{
	nline++;
	clp = clp->nextp;
	}

    /* Insert the number as text */
    sprintf(buf, "%d", nline);
    return emxlinsert(strlen(buf), EMXSTR, buf);
}


/* Display a bunch of useful information about the current location of dot.
   The character under the cursor (in octal), the current line, row, and
   column, and approximate position of the cursor in the file (as a
   percentage) is displayed. The column position assumes an infinite
   position display; it does not truncate just because the screen does.
   This is normally bound to "^X-=". */

COMMAND(emxdisplayposition)

{
    LINE   *clp;
    WINDOW *cwp;
    LINE   *blinep;
    int    nchar;
    int    cchar;
    int    nline;
    int    cline;
    int    cbyte;
    int    row;
    int    col;
    char	    buf[MAXCOL];

    cwp = g_curwp;
    blinep = g_curbp->linep;
    clp = blinep->nextp;	    /* Collect the data */
    nchar = 0;
    nline = 1;				    /* Origin 1 */
    for (;;)
	{
	if (clp == cwp->dotp)
	    {
	    cline = nline;
	    cchar = nchar + cwp->doto + 1;
	    if (cwp->doto == clp->used)
		cbyte = '\n';
	    else
		cbyte = clp->text[cwp->doto];
	    }

	if (clp == blinep)
	    break;

	nchar += clp->used + 1;
	nline++;
	clp = clp->nextp;
	}

    row = cwp->toprow;			/* Determine row */
    clp = cwp->linep;
    while (clp!= blinep && clp!=cwp->dotp)
	{
	++row;
	clp = clp->nextp;
	}

    ++row;				    /* Convert to origin 1 */

    col = emxcursorcol(cwp->dotp->text, cwp->doto) + 1;

    /* Print to a buffer first because print only takes 3 arguments */
    sprintf(buf, "[Ch:%02x Line: %d of %d Row: %d Col: %d Char: %d of %d]",
	    cbyte, cline, nline, row, col, cchar, nchar);

    emxmsgprint(buf);
    return TRUE;
}


/* Twiddle two characters. This corrects a common typing error easily. If
   no argument, or argument is <= zero, swap the character under the cursor
   with the character before it. If the argument is greater than zero, swap
   two characters 'n' characters previous, if possible. Don't do anything
   if the cursor is at the beginning of the line. */

COMMAND(emxtwiddle)

{
    LINE   *dotp;
    int    doto;
    char   ch;

    /* Force n to be zero or positive */
    if (!f || n < 0)
	n = 0;

    dotp = g_curwp->dotp;
    doto = g_curwp->doto;

    /* Too close to the start of the line? */
    if (doto < n + 1)
	return FALSE;

    doto -= n;
    ch = dotp->text[doto];
    dotp->text[doto] = dotp->text[doto - 1];
    dotp->text[doto - 1] = ch;
    emxbchange(g_curbp, WFEDIT);
    return TRUE;
}


/* Quote the next character, and insert it into the buffer. All the
   characters are taken literally, with the exception of the newline, which
   always has its line splitting meaning. The character is always read,
   even if it is inserted 0 times, for regularity. */

int emxquotedchar _ARGS2(KEY *, keyp, unsigned char *, chp)

{
    unsigned char ch;

    if (keyp->keysym == 0 || keyp->keysym > 0xff)
	return FALSE;

    /* Generate ASCII control characters */
    ch = keyp->keysym;
    if (keyp->modifier)
	{
	if (keyp->modifier != ControlMask)
	    return FALSE;

	if (keyp->keysym >= 0x40 && keyp->keysym <= 0x5f)
	    ch = keyp->keysym - 0x40;
	else if (keyp->keysym >= 'a' && keyp->keysym <= 'z')
	    ch = keyp->keysym - 0x60;
	else
	    return FALSE;
	}

    *chp = ch;
    return TRUE;
}


static COMPLETION(emxquotecomplete)

{
    unsigned char   ch;
    int		    s;

    if (!emxquotedchar(g_keyp, &ch))
	return FALSE;

    if (g_num < 0)
	return FALSE;

    if (g_num == 0)
	return TRUE;

    if (ch == '\n')
	{
	do
	    {
	    s = emxlnewline();
	    }
	    while (s == TRUE && --g_num);

	return s;
	}

    return emxlinsert(g_num, ch, NULL);
}


COMMAND(emxquote)

{
    g_num = n;
    return emxonekey(emxquotecomplete);
}


/* Toggle trailing blank stripping in file-writes */

COMMAND(emxtrimoutputtoggle)

{
    g_modes.trimoutput ^= 1;
    return TRUE;
}


/* Toggle insert/overstrike modes */

COMMAND(emxinstoggle)

{
    g_modes.insert ^= 1;
    emxinfoupdate();
    return TRUE;
}


/* perform the word wrap */

static void dowrap _ARGS0()

{
    LINE   *lp;
    char   *ptr;
    char   *endp;
    int    skipped;

    /* Look backwards for whitespace */
    lp = g_curwp->dotp;
    ptr = lp->text + g_curwp->doto;
    skipped = 0;
    while (--ptr >= lp->text && *ptr != ' ' && *ptr != '\t')
	skipped++;

    /* No whitespace? */
    if (ptr < lp->text)
	return;

    /* Put the cursor on the first non-blank and insert a newline */
    g_curwp->doto = ptr - lp->text + 1;
    emxlnewline();

    /* Force the window to be left-aligned again */
    g_curwp->leftcol = 0;

    /* Copy any whitespace from the previous line's start */
    lp = g_curwp->dotp;
    ptr = lp->prevp->text;
    endp = ptr + lp->prevp->used;

    /* First see if there are any non-blanks */
    while (ptr < endp && (*ptr == ' ' || *ptr == '\t'))
	ptr++;

    /* Don't copy them if that's all there is */
    if (ptr < endp)
	{
	ptr = lp->prevp->text;
	while (*ptr == ' ' || *ptr == '\t')
	    {
	    emxlinsert(1, *ptr, NULL);
	    ptr++;
	    }

	}

    /* Move to where the cursor was before */
    g_curwp->doto += skipped;
}


/* Insert some text - used most always from execute-file mode seems useless
   otherwise this can behave oddly if overstrike mode is active (see above)
   */

COMPLETION(emxinserttextcomplete)

{
    if (status != TRUE)
	return status;

    return emxlinsert(strlen(g_cmdbuf), EMXSTR, g_cmdbuf);
}


COMMAND(emxinserttext)

{
    return emxmsgreply("Text: ", g_cmdbuf, sizeof(g_cmdbuf), emxinserttextcomplete);
}


/* Insert a single charcter into the current buffer and check for wordwrap */

int emxinsertchar _ARGS2(int, n, char, c)

{
    int s;

    /* Delete the character if in overstrike mode */
    if (!g_modes.insert && (g_curwp->doto < (int) g_curwp->dotp->used) &&
	(g_curwp->dotp->text[g_curwp->doto] != '\t' || g_curwp->doto % 8 == 7))
	emxldelete(1, FALSE);

    s = emxlinsert(n, c, NULL);
    if (g_modes.wordwrap)
	{
	if (g_fillcol)
	    {
	    if (g_scrcol + g_curwp->leftcol >= g_fillcol - 1)
		dowrap();
	    }

	else if (g_scrcol >= g_ncol - 1)
	    dowrap();
	}

    return s;
}


/* Ordinary text characters are bound to this function, which inserts them
   into the buffer. Characters marked as control characters (using the CTRL
   flag) may be remapped to their ASCII equivalent. This makes TAB (C-I)
   work right, and also makes the world look reasonable if a control
   character is bound to this this routine by hand. This is one of the few
   routines that actually looks at the "k" argument. */

COMMAND(emxselfinsert)

{
    int c;

    if (n < 0)
	return FALSE;

    if (n == 0)
	return TRUE;

    c = keyp->keysym;
    if (keyp->modifier & ControlMask && c>='@' && c<='_')   /* ASCII-ify */
	c -= '@';

    return emxinsertchar(n, c);
}


/* Find the screen column which corresponds to an offset in a string */

int emxcursorcol _ARGS2(char *, textp, unsigned int, offset)

{
    char	  *endp;
    unsigned int col;

    endp = textp + offset;
    col = 0;

    while (textp < endp)
	{
	if (*textp == '\t')
	    {
	    col |= 7;
	    textp++;
	    }

	else if (ISCTRL(*textp++))
	    col++;

	col++;
	}

    return col;
}


/* Find the line offset which corresponds to a given screen column */

int emxcursoroffset _ARGS2(LINE *, lp, int, target)

{
    char   *textp;
    char   *endp;
    int    col;

    textp = lp->text;
    endp = textp + lp->used;
    col = 0;

    while (col < target && textp < endp)
	{
	if (*textp == '\t')
	    {
	    col |= 7;
	    textp++;
	    }

	else if (ISCTRL(*textp++))
	    col++;

	col++;
	}

    if (col <= target)
	return textp - lp->text;
    else
	return textp - lp->text - 1;
}


/* Insert spaces until we reach the nth tab stop.  This is usually bound to
   the tab key */

COMMAND(emxinserttab)

{
    WINDOW *cwp;
    unsigned int   col;

    if (n < 0)
	return FALSE;

    if (n == 0)
	return TRUE;

    /* Find the current cursor column */
    cwp = g_curwp;
    col = emxcursorcol(cwp->dotp->text, cwp->doto);
    return emxlinsert(g_tabset - (col % g_tabset), ' ', NULL);
}


/* Delete spaces backward until we reach the nth previous tab stop or a
   non-blank character. By default this is bound to ESC-Tab */

COMMAND(emxbacktab)

{
    int	    col;
    int	    curcol;
    int	    newcol;
    int	    dist;
    char    *textp;
    char    *endp;

    if (n < 0)
	return FALSE;

    if (n == 0)
	return TRUE;

    /* Figure out how far to back up in screen columns */
    textp = g_curwp->dotp->text;
    col = emxcursorcol(textp, g_curwp->doto);
    if ((dist = col % g_tabset) == 0)
	dist = g_tabset;

    if (n > 1)
	dist += g_tabset * (n - 1);

    if (dist > col)
	dist = col;

    /* Loop through the line looking for the offset matching the backtab */
    curcol = 0;
    col -= dist;
    while (curcol < col)
	{
	newcol = curcol;
	if (*textp == '\t')
	    newcol |= 7;

	else if (ISCTRL(*textp))
	    newcol++;

	newcol++;
	if (newcol >= col)
	    break;

	curcol = newcol;
	textp++;
	}

    /* Scan backward to see if any non-blanks are in the way */
    endp = g_curwp->dotp->text + g_curwp->doto;
    while (endp > textp)
	{
	endp--;
	if (*endp != ' ' && *endp != '\t')
	    {
	    endp++;
	    break;
	    }

	}

    /* Delete chars from this point to the original cursor location */
    dist = g_curwp->doto - (endp - g_curwp->dotp->text);
    g_curwp->doto = endp - g_curwp->dotp->text;
    if (dist)
	{
	emxldelete(dist, FALSE);
	if (endp == textp && curcol < col)
	    emxlinsert(col - curcol, ' ', NULL);
	}

    return TRUE;
}


/* Insert a real tab */

COMMAND(emxrealtab)

{
    if (n < 0)
	return FALSE;

    if (n == 0)
	return TRUE;

    return emxlinsert(n, '\t', NULL);
}


/* Trim a line: remove extra spaces amid and at the end */

int emxtrimoneline _ARGS1(LINE *, lp)

{
    int	    pos;
    int	    spos;
    char    *ptr;
    char    *spacep;
    char    *endp;
    char    *lastp;
    int	    used;
    int	    count;

    pos = 0;
    spos = 0;
    ptr = lp->text;
    lastp = ptr - 1;
    endp = ptr + lp->used;
    spacep = ptr;
    while (ptr < endp)
	{
	/* Adjust the current offset */
	if (*ptr == '\t')
	    pos |= 7;

	else if (*ptr != ' ')
	    {
	    /* Spaces to compress? */
	    while (spos < pos)
		{
		if ((pos > (spos | 7)) && (pos - spos > 1))
		    {
		    spos = (spos | 7) + 1;
		    *spacep++ = '\t';
		    }

		else
		    {
		    memset(spacep, ' ', pos - spos);
		    spacep += (pos - spos);
		    spos = pos;
		    }
		}

	    lastp = spacep;
	    if (ISCTRL(*ptr))
		pos++;

	    *spacep++ = *ptr;
	    spos = pos + 1;
	    }

	ptr++;
	pos++;
	}

    used = lastp + 1 - lp->text;
    count = lp->used - used;
    lp->used = used;
    return count;
}


COMMAND(emxtrimline)

{
    LINE    *lp;
    int	    count;

    if (n <= 0)
	return FALSE;

    lp = g_curwp->dotp;
    count = 0;
    while (n-- && lp != g_curbp->linep)
	{
	count += emxtrimoneline(lp);
	lp = lp->nextp;
	}

    g_curwp->dotp = lp;
    g_curwp->doto = 0;
    if (count > 0)
	emxbchange(g_curbp, WFHARD);
    else
	g_curwp->flag |= WFMOVE;

    emxmsgprintint(g_msgbytesrem, count);
    return TRUE;
}


COMMAND(emxtrimbuffer)

{
    LINE    *lp;
    int	    offset;
    int	    count;

    offset = emxcursorcol(g_curwp->dotp->text, g_curwp->doto);

    lp = g_curbp->linep->nextp;
    count = 0;
    while (lp != g_curbp->linep)
	{
	count += emxtrimoneline(lp);
	lp = lp->nextp;
	}

    g_curwp->doto = emxcursoroffset(g_curwp->dotp, offset);
    if (count > 0)
	emxbchange(g_curbp, WFHARD);
    else
	g_curwp->flag |= WFMOVE;

    emxmsgprintint(g_msgbytesrem, count);
    return TRUE;
}


/* Open up some blank space. The basic plan is to insert a bunch of
   newlines, and then back up over them. Everything is done by the
   subcommand processors. They even handle the looping. Normally this is
   bound to "C-O". */

COMMAND(emxinsnlandbackup)

{
    int	i;
    int	s;

    if (n < 0)
	return FALSE;

    if (n == 0)
	return TRUE;

    i = n;					/* Insert newlines */
    do {
	s = emxlnewline();
	} while (s==TRUE && --i);

    if (s == TRUE)				/* Then back up overtop */
	s = emxbackchar(f, n);	  /* of them all */

    return s;
}


/* Change the mode for inserting newlines in front of blank lines. See
   'newline' */

COMMAND(emxpushnewlines)

{
    g_modes.pushnewlines ^= 1;
    return TRUE;
}


/* Insert a newline. If you are at the end of the line and the next line is
   a blank line, just move into the blank line. This makes "C-O" and "C-X
   C-O" work nicely. */

COMMAND(emxnewline)

{
    LINE   *lp;

    if (n < 0)
	return FALSE;

    while (n--)
	{
	lp = g_curwp->dotp;
	if (lp->used == g_curwp->doto &&
	    lp != g_curbp->linep &&
	    lp->nextp->used == 0 &&
	    (!g_modes.pushnewlines || lp->nextp == g_curbp->linep))
	    {
	    if (!emxforwchar(FALSE, 1))
		return FALSE;
	    }

	else if (!emxlnewline())
	    return FALSE;
	}

    return TRUE;
}


/* Delete blank lines around dot. What this command does depends if dot is
   sitting on a blank line. If dot is sitting on a blank line, this command
   deletes all the blank lines above and below the current line. If it is
   sitting on a non blank line then it deletes all of the blank lines after
   the line. Normally this command is bound to "C-X C-O". Any argument is
   ignored.
*/

COMMAND(emxdelblanklines)

{
    LINE	*lp1;
    LINE	*lp2;
    int	lines;

    lp1 = g_curwp->dotp;
    while (lp1->used==0 && (lp2=lp1->prevp)!=g_curbp->linep)
	lp1 = lp2;

    lp2 = lp1;
    lines = 0;
    while ((lp2=lp2->nextp)!=g_curbp->linep && lp2->used==0)
	++lines;

    if (lines == 0)
	return TRUE;

    g_curwp->dotp = lp1->nextp;
    g_curwp->doto = 0;
    return emxldelete(lines, FALSE);
}


/* Insert a newline, then enough tabs and spaces to duplicate the
   indentation of the previous line. Assumes tabs are every eight
   characters. Quite simple. Figure out the indentation of the current
   line. Insert a newline by calling the standard routine. Insert the
   indentation by inserting the right number of tabs and spaces. Return
   TRUE if all ok. Return FALSE if one of the subcomands failed. Normally
   bound to "C-J". */

COMMAND(emxinsnlandindent)

{
    int    indent;
    int    firstchar;
    int    i;

    if (n < 0)
	return FALSE;

    /* Calculate the desired indent */
    indent = emxcursorcol(g_curwp->dotp->text, g_curwp->doto);
    firstchar = emxfirstnonblank(g_curwp->dotp);
    if (firstchar >= 0 && firstchar < indent)
	indent = firstchar;

    /* Insert as many indented newlines as requested */
    while (n--)
	{
	if (emxlnewline() == FALSE)
	    return FALSE;

	if (g_modes.tabsonindent)
	    {
	    if (((i = indent / 8) && emxlinsert(i, '\t', NULL) == FALSE) ||
		((i = indent % 8) && emxlinsert(i,  ' ', NULL) == FALSE))
	    return FALSE;
	    }

	else if (emxlinsert(indent, ' ', NULL) == FALSE)
	    return FALSE;
	}

    return TRUE;
}


/* Delete forward. This is real easy, because the basic delete routine does
   all of the work. Watches for negative arguments, and does the right
   thing. If any argument is present, it kills rather than deletes, to
   prevent loss of text if typed with a big argument. Normally bound to
   "C-D". */

COMMAND2(emxforwdel)

{
    HIGHSEG *segp;
    LINE    *lp;

    /* Check for immediately adjacent selected text */
    if (segp = g_curbp->selectp)
	{
	lp = g_curwp->dotp;
	while (segp->lp && (segp->lp != lp))
	    segp++;

	if (segp->lp)
	    {
	    if (g_curwp->doto >= segp->firsto &&
		g_curwp->doto <= segp->endo)
		return emxcutselected();
	    }

	else
	    {
	    segp--;
	    if (segp->lp->nextp == lp &&
		segp->endo == segp->lp->used + 1)
		return emxcutselected();
	    }
	}

    if (n < 0)
	return emxbackdel(f, -n);

    /* Really a kill */
    if (f)
	killcommand();

    return emxldelete(n, f);
}


/* Delete backwards. This is quite easy too, because it's all done with
   other functions. Just move the cursor back, and delete forwards. Like
   delete forward, this actually does a kill if presented with an argument.
   */

COMMAND2(emxbackdel)

{
    int    s;
    int    count;

    if (n == 0)
	return TRUE;

    if (n < 0)
	return emxforwdel(f, -n);

    /* Really a kill */
    if (f && g_modes.insert)
	killcommand();

    emxbchange(g_curbp, WFEDIT);

    /* Do the backspacing.  We must differentiate between insert and */
    /* overstrike modes */
    count = n;
    while (n > 0)
	{
	if (s = emxbackchar(f, 1))
	    {
	    if (!g_modes.insert)
		g_curwp->dotp->text[g_curwp->doto] = 0x20;

	    --n;
	    }

	else
	    return s;
	}

    if (g_modes.insert)
	s = emxldelete(count, f);

    return s;
}


/* Kill line. If called without an argument, it kills from dot to the end
   of the line, unless it is at the end of the line, when it kills the
   newline. If called with an argument of 0, it kills from the start of the
   line to dot. If called with a positive argument, it kills from dot
   forward over that number of newlines. If called with a negative argument
   it kills any text before dot on the current line, then it kills back
   abs(arg) lines. */

COMMAND(emxkillline)

{
    WINDOW *cwp;
    int    chunk;
    LINE   *nextp;

    cwp = g_curwp;
    killcommand();
    if (f == FALSE)
	{
	chunk = cwp->dotp->used - cwp->doto;
	if (chunk == 0)
	    chunk = 1;
	}

    else if (n > 0)
	{
	chunk = cwp->dotp->used - cwp->doto + 1;
	nextp = cwp->dotp->nextp;
	while (--n)
	    {
	    if (nextp == g_curbp->linep)
		return FALSE;

	    chunk += nextp->used+1;
	    nextp = nextp->nextp;
	    }

	}

    else
	{				 /* n <= 0 */
	chunk = cwp->doto;
	cwp->doto = 0;
	while (n++)
	    {
	    if (cwp->dotp->prevp == g_curbp->linep)
		break;

	    cwp->dotp = cwp->dotp->prevp;
	    cwp->flag |= WFMOVE;
	    chunk += cwp->dotp->used+1;
	    }

	}

    return emxldelete(chunk, TRUE);
}



/* This routine will delete whole lines. It will also free up the heap
   space used and put the deleted lines into the kill buffer. */

COMMAND(emxdeleteline)

{
    LINE *lp;
    LINE *nextp;
    int  count;

    lp = g_curwp->dotp;

    /* Return if there is nothing to delete */
    if (n == 0 || g_curbp->linep == g_curbp->linep->nextp)
	return TRUE;

    killcommand();

    /* Save the cursor column */
    g_curgoal = emxcursorcol(lp->text, g_curwp->doto);

    /* Forward or backward deletion */
    if ((count = n) < 0)
	{
	/* Figure out how many there are above this spot. Lines have to be
	   deleted top-down, in order to end up in the kill buffer correctly */
	count = -n;
	while (lp != g_curbp->linep && --count)
	    lp = lp->prevp;

	/* Watch for the top of the buffer */
	if (count)
	    lp = lp->nextp;

	count = -n - count;
	}

    /* Delete the lines and put them into the kill buffer. */
    while (count-- && lp != g_curbp->linep)
	{
	nextp = lp->nextp;
	emxkinsstr(lp->used, lp->text);
	emxkinsert('\n');
	emxlfree(lp);
	lp = nextp;
	}

    /* Reset the cursor depending on direction of delete */
    if (lp == g_curbp->linep && n < 0)
	g_curwp->dotp = lp->nextp;
    else
	g_curwp->dotp = lp;

    /* Put the cursor on the appropriate line*/
    g_curwp->doto = emxcursoroffset(g_curwp->dotp, g_curgoal);
    g_curwp->flag |= WFMOVE;
    emxbchange(g_curbp, WFHARD);
    return TRUE;
}


/* Yank text back from the kill buffer. This is really easy. All of the
   work is done by the standard insert routines. All you do is run the
   loop, and check for errors. The blank lines are inserted with a call to
   "newline" instead of a call to "emxlnewline" so that the magic stuff
   that happens when you type a carriage return also happens when a
   carriage return is yanked back from the kill buffer. An attempt has been
   made to fix the cosmetic bug associated with a yank when dot is on the
   top line of the window (nothing moves, because all of the new text
   landed off screen). */

COMMAND2(emxyank)

{
    char   *kp;
    char   *kptr;
    char   *kendp;
    char   endchar;
    unsigned int   len;
    LINE   *lp;
    int    nline;

    if (n < 0)
	return FALSE;

    /* Prepare the spot for insertion */
    emxlnewline();
    emxbackchar(0, 1);

    endchar = 0;
    nline = 0;
    while (n--)
	{
	kptr = g_kbufp;
	kp = kptr;
	kendp = g_kbufp + g_killused;

	/* Loop on the kill buffer */
	while (kptr < kendp)
	    {
	    /* Get a line from the kill buffer */
	    while (kp < kendp && *kp++ != '\n')
		;

	    /* Full line? */
	    len = kp - kptr;
	    if ((endchar = *(kp - 1)) == '\n')
		len--;

	    /* Insert this string */
	    emxlinsert(len, EMXSTR, kptr);

	    /* Set up for the next line */
	    if (endchar == '\n')
		{
		emxlnewline();
		nline++;
		}

	    kptr = kp;
	    }

	}

    /* Remove the extra newline */
    emxldelete(1, FALSE);

    lp = g_curwp->linep;			  /* Cosmetic adjustment */
    if (g_curwp->dotp == lp)
	{
	while (nline-- && lp->prevp != g_curbp->linep)
	    lp = lp->prevp;

	g_curwp->linep = lp;	      /* Adjust framing */
	g_curwp->flag |= WFHARD;
	}

    if (endchar == '\n')	       /* last char a newline ? */
	g_curbp->flag |= BFNL;	   /* for yank buffer */

    return emxok();
}


/* This function updates infolines for all windows */

void emxinfoupdate _ARGS0()

{
    WINDOW  *wp;

    for (wp = g_wheadp->nextp; wp; wp = wp->nextp)
	 wp->flag |= WFINFO;
}


/* This nasty dangerous little function clears the current buffer's
   modified flag, with all the expected potential side effects. */

COMMAND(emxunmodify)

{
    g_curbp->flag &= ~BFCHG;
    emxbupdate(g_curbp, WFINFO);
    return emxok();
}


/* Return TRUE if the character at dot is a character that is considered to
   be part of a word. The word character list is hard coded. Should be
   setable. */

static int inword _ARGS0()

{
    WINDOW *wp;

    wp = g_curwp;
    if (wp->doto == wp->dotp->used)
	return FALSE;
    if (ISWORD(wp->dotp->text[wp->doto]))
	return TRUE;
    return FALSE;
}


/* Move the cursor backward by "n" words. All of the details of motion are
   performed by the "emxbackchar" and "emxforwchar" routines. Error if you
   try to move beyond the buffers. */

COMMAND2(emxbackword)

{
    if (n < 0)
	return emxforwword(f, -n);

    if (emxbackchar(FALSE, 1) == FALSE)
	return FALSE;

    while (n--)
	{
	while (inword() == FALSE)
	    {
	    if (emxbackchar(FALSE, 1) == FALSE)
		return FALSE;
	    }

	while (inword() != FALSE)
	    {
	    if (emxbackchar(FALSE, 1) == FALSE)
		return FALSE;
	    }

	}

    return emxforwchar(FALSE, 1);
}


/* Move the cursor forward by the specified number of words. All of the
   motion is done by "emxforwchar". Error if you try and move beyond the
   buffer's end. */

COMMAND2(emxforwword)

{
    if (n < 0)
	return emxbackword(f, -n);

    while (n--)
	{
	while (inword() == FALSE)
	    {
	    if (emxforwchar(FALSE, 1) == FALSE)
		return FALSE;
	    }

	while (inword() != FALSE)
	    {
	    if (emxforwchar(FALSE, 1) == FALSE)
		return FALSE;
	    }

	}

    return TRUE;
}


/* Move the cursor forward by the specified number of words. As you move,
   convert any characters to upper case. Error if you try and move beyond
   the end of the buffer. */

COMMAND(emxupperword)

{
    char    *textp;
    char    *endp;

    if (n < 0)
	return FALSE;

    while (n--)
	{
	while (inword() == FALSE)
	    {
	    if (emxforwchar(FALSE, 1) == FALSE)
		return FALSE;
	    }

	textp = &g_curwp->dotp->text[g_curwp->doto];
	endp = &g_curwp->dotp->text[g_curwp->dotp->used];
	do
	    {
	    if (ISLOWER(*textp))
		*textp = TOUPPER(*textp);
	    }
	    while (++textp < endp && ISWORD(*textp));

	g_curwp->doto = textp - g_curwp->dotp->text;
	emxbchange(g_curbp, WFEDIT);
	}

    return TRUE;
}


/* Move the cursor forward by the specified number of words. As you move
   convert characters to lower case. Error if you try and move over the end
   of the buffer. */

COMMAND(emxlowerword)

{
    char    *textp;
    char    *endp;

    if (n < 0)
	return FALSE;

    while (n--)
	{
	while (inword() == FALSE)
	    {
	    if (emxforwchar(FALSE, 1) == FALSE)
		return FALSE;
	    }

	textp = &g_curwp->dotp->text[g_curwp->doto];
	endp = &g_curwp->dotp->text[g_curwp->dotp->used];
	do
	    {
	    if (ISUPPER(*textp))
		*textp = TOLOWER(*textp);
	    }
	    while (++textp < endp && ISWORD(*textp));

	g_curwp->doto = textp - g_curwp->dotp->text;
	emxbchange(g_curbp, WFEDIT);
	}

    return TRUE;
}


/* Move the cursor forward by the specified number of words. As you move
   convert the first character of the word to upper case, and subsequent
   characters to lower case. Error if you try and move past the end of the
   buffer. */

COMMAND(emxcapword)

{
    char    *textp;
    char    *endp;

    if (n < 0)
	return FALSE;

    while (n--)
	{
	while (inword() == FALSE)
	    {
	    if (emxforwchar(FALSE, 1) == FALSE)
		return FALSE;
	    }

	textp = &g_curwp->dotp->text[g_curwp->doto];
	endp = &g_curwp->dotp->text[g_curwp->dotp->used];
	if (ISLOWER(*textp))
	    *textp = TOUPPER(*textp);

	textp++;

	while (textp < endp && ISWORD(*textp))
	    {
	    if (ISUPPER(*textp))
		*textp = TOLOWER(*textp);

	    textp++;
	    }

	g_curwp->doto = textp - g_curwp->dotp->text;
	emxbchange(g_curbp, WFEDIT);
	}

    return TRUE;
}


/* Kill forward by "n" words. The rules for final status are now different.
   It is not considered an error to delete fewer words than you asked. This
   lets you say "kill lots of words" and have the command stop in a
   reasonable way when it hits the end of the buffer. */

COMMAND(emxforwdelword)

{
    int    size;
    LINE   *dotp;
    int    doto;

    if (n < 0)
	return FALSE;

    killcommand();
    dotp = g_curwp->dotp;
    doto = g_curwp->doto;
    size = 0;
    while (n--)
	{
	while (inword() == FALSE)
	    {
	    if (emxforwchar(FALSE, 1) == FALSE)
		goto out;	/* Hit end of buffer */

	    ++size;
	    }

	while (inword() != FALSE)
	    {
	    if (emxforwchar(FALSE, 1) == FALSE)
		goto out;	/* Hit end of buffer */

	    ++size;
	    }

	}

out:
    g_curwp->dotp = dotp;
    g_curwp->doto = doto;
    return emxldelete(size, TRUE);
}


/* Kill backwards by "n" words. The rules for success and failure are now
   different, to prevent strange behavior at the start of the buffer. The
   command only fails if something goes wrong with the actual delete of the
   characters. It is successful even if no characters are deleted, or if
   you say delete 5 words, and there are only 4 words left. I considered
   making the first call to "emxbackchar" special, but decided that that
   would just be wierd. */

COMMAND(emxbackdelword)

{
    int    size;

    if (n < 0)
	return FALSE;

    killcommand();
    if (emxbackchar(FALSE, 1) == FALSE)
	return TRUE;		      /* Hit buffer start */

    size = 1;				    /* One deleted */
    while (n--)
	{
	while (inword() == FALSE)
	    {
	    if (emxbackchar(FALSE, 1) == FALSE)
		goto out;	/* Hit buffer start */

	    ++size;
	    }

	while (inword() != FALSE)
	    {
	    if (emxbackchar(FALSE, 1) == FALSE)
		goto out;	/* Hit buffer start */

	    ++size;
	    }

	}

    if (emxforwchar(FALSE, 1) == FALSE)
	return FALSE;

    --size;					    /* Undo assumed delete */

out:
    return emxldelete(size, TRUE);
}


/* Toggle fill double mode - controls insertion of extra space after
   periods in paragraph fill */

COMMAND(emxsetfilldouble)

{
    emxmsgprint((g_modes.filldouble ^= 1) ? "[Double-space filling]" :
				    "[Single-space filling]");
    return TRUE;
}


/* Toggle tabs-on-indent mode - controls insertion of tabs when indenting
   in ins-nl-and-indent */

COMMAND(emxtoggletabsonindent)

{
    emxmsgprint((g_modes.tabsonindent ^= 1) ? "[Tabs on indent]" :
				    "[No tabs on indent]");
    return TRUE;
}


/* Find the first non-blank character on a line and return its column */

int emxfirstnonblank _ARGS1(LINE *, linep)

{
    char    *ptr;
    char    *endp;
    int	    col;

    endp = linep->text + linep->used;
    col = 0;
    for (ptr = linep->text; ptr < endp; ptr++)
	{
	if (*ptr == '\t')
	    col = (col | 7) + 1;

	else if (*ptr == ' ')
	    col++;

	else
	    break;
	}

    if (ptr == endp)
	return -1;
    else
	return col;
}


/* Fill the current paragraph according to the current fill column */

COMMAND(emxfillparagraph)

{
    int c;		    /* current char durring scan */
    int wordlen;	    /* length of current word */
    int clength;	    /* position on line during fill */
    int newlength;	    /* tentative new line length */
    int eopflag;	    /* Are we at the End-Of-Paragraph? */
    int firstflag;	    /* first word? (needs no space) */
    LINE *eopline;	    /* pointer to line just past EOP */
    int dotflag;	    /* was the last char a period? */
    int		 wrapcol;
    LINE	*linep;
    int		indent;
    char wbuf [MAXLINE];		    /* buffer for current word */

    if ((wrapcol = g_fillcol) == 0)
	wrapcol = g_curwp->leftcol + g_ncol;

    /* Get the line after the end of the paragraph */
    emxforwpara(FALSE, 1);
    if (g_curwp->dotp == g_curbp->linep)
	eopline = g_curbp->linep;
    else
	eopline = g_curwp->dotp->nextp;

    /* Get the first line in the paragraph */
    emxbackpara(FALSE, 1);

    /* See if the paragraph is indented */
    indent = 0;
    linep = g_curwp->dotp;
    if (linep->nextp != eopline)
	{
	if ((indent = emxfirstnonblank(linep)) != emxfirstnonblank(linep->nextp))
	    indent = 0;
	}

    /* initialize various info */
    wordlen = 0;
    dotflag = FALSE;
    clength = emxfirstnonblank(g_curwp->dotp);

    /* scan through lines, filling words */
    firstflag = TRUE;
    eopflag = FALSE;
    while (!eopflag)
	{
	/* get the next character in the paragraph */
	if (g_curwp->doto == g_curwp->dotp->used)
	    {
	    c = ' ';
	    if (g_curwp->dotp->nextp == eopline)
		eopflag = TRUE;
	    }

	else
	    c = g_curwp->dotp->text[g_curwp->doto];

	/* and then delete it */
	emxldelete(1, FALSE);

	/* if not a separator, just add it in */
	if (!ISSPACE(c) && (wordlen < sizeof(wbuf) - 1))
	    {
	    dotflag = (c == '.');		/* was it a dot */
	    wbuf[wordlen++] = c;
	    }

	else if (wordlen)
	    {
	    /* at a word break with a word waiting */
	    /* calculate tentative new length with word added */
	    newlength = clength + 1 + wordlen;
	    if (newlength < wrapcol)
		{
		/* add word to current line */
		if (!firstflag)
		    {
		    emxlinsert(1, ' ', NULL); /* the space */
		    ++clength;
		    }

		firstflag = FALSE;
		}

	    else
		{
		/* start a new line */
		emxlnewline();

		if (clength = indent)
		    emxlinsert(indent, ' ', NULL);
		}

	    /* and add the word in in either case */
	    emxlinsert(wordlen, EMXSTR, wbuf);
	    clength += wordlen;

	    if (dotflag && g_modes.filldouble)
		{
		emxlinsert(1, ' ', NULL);
		++clength;
		}

	    wordlen = 0;
	    }

	}

    /* and add a last newline for the end of our new paragraph */
    if (eopline != g_curbp->linep)
	emxlnewline();
    else
	{
	g_curwp->dotp = eopline;
	g_curwp->doto = 0;
	}

    return TRUE;
}


/* Delete n paragraphs starting with the current */

COMMAND(emxkillparagraph)

{
    int status;    /* returned status of functions */

    /* for each paragraph to delete */
    while (n--)
	{
	/* mark out the end and begining of the para to delete */
	emxforwpara(FALSE, 1);

	/* set the mark here */
	g_curwp->markp = g_curwp->dotp;
	g_curwp->marko = g_curwp->doto;

	/* go to the begining of the paragraph */
	emxbackpara(FALSE, 1);
	g_curwp->doto = 0;	/* force us to the begining of line */

	/* and delete it */
	if ((status = emxkillregion(FALSE, 1)) != TRUE)
	    return(status);

	/* and clean up the 2 extra lines */
	emxldelete(2, TRUE);
    }
    return(TRUE);
}


/* Reposition dot in the current window to line "n". If the argument is
   positive, it is that line. If it is negative it is that line from the
   bottom. If it is 0 the window is centered (this is what the standard
   redisplay code does). With no argument it defaults to 1. */

COMMAND(emxrepositionwindow)

{
    g_curwp->force = n;
    g_curwp->flag |= WFFORCE;
    return TRUE;
}


/* Make a specified window the current window */

void emxselectwind _ARGS1(WINDOW *, wp)

{
    g_curwp = wp;
    g_curbp = wp->bufp;
    if (g_modes.automatch)
	(void) emxmatchtable(g_curbp->mode);
}


/* This command make the next window (next => down the screen) the current
   window. There are no real errors, although the command does nothing if
   there is only 1 window on the screen. */

COMMAND(emxforwwindow)

{
    WINDOW *wp;

    if ((wp = g_curwp->nextp) == 0)
	wp = g_wheadp->nextp;

    emxselectwind(wp);
    return TRUE;
}


/* This command makes the previous window (previous => up the screen) the
   current window. There aren't any errors, although the command does not
   do a lot if there is 1 window. */

COMMAND(emxbackwindow)

{
    WINDOW *wp1;
    WINDOW *wp2;

    wp1 = g_wheadp->nextp;
    wp2 = g_curwp;
    if (wp1 == wp2)
	wp2 = (WINDOW *)NULL;

    while (wp1->nextp != wp2)
	wp1 = wp1->nextp;

    emxselectwind(wp1);
    return TRUE;
}


/* This command moves the current window down by "arg" lines. Recompute the
   top line in the window. The move up and move down code is almost
   completely the same; most of the work has to do with reframing the
   window, and picking a new dot. We share the code by having "move down"
   just be an interface to "move up". */

COMMAND(emxdownwindow)

{
    return emxupwindow(f, -n);
}


/* Move the current window up by "arg" lines. Recompute the new top line of
   the window. Look to see if "." is still on the screen. If it is, you
   win. If it isn't, then move "." to center it in the new framing of the
   window (this command does not really move "."; it moves the frame). */

COMMAND2(emxupwindow)

{
    LINE   *lp;
    int    i;
    LINE   *blinep;
    WINDOW *cwp;

    cwp = g_curwp;
    i = n;
    lp = cwp->linep;
    blinep = g_curbp->linep;
    if (n < 0)
	{
	do
	    {
	    if (lp == blinep)
		break;

	    lp = lp->nextp;
	    }
	    while (++n);
	}

    else
	{
	do
	    {
	    if (lp->prevp == blinep)
		break;

	    lp = lp->prevp;
	    }
	    while (--n);
	}

    emximvertical(i - n);
    cwp->linep = lp;
    cwp->flag |= WFHARD;
    for (i=0; i<cwp->rows; ++i)
	{
	if (lp == cwp->dotp)
	    return TRUE;
	if (lp == blinep)
	    break;
	lp = lp->nextp;
	}

    lp = cwp->linep;
    i  = cwp->rows/2;
    while (i-- && lp != blinep)
	lp = lp->nextp;

    cwp->dotp  = lp;
    cwp->doto  = 0;
    return TRUE;
}


/* This function moves all windows up 'n' lines */

COMMAND2(emxallwinup)

{
    WINDOW *wp;
    WINDOW *cwp;

    cwp = g_curwp;
    for (wp = g_wheadp->nextp; wp; wp = wp->nextp)
	{
	g_curwp = wp;
	g_curbp = wp->bufp;
	emxupwindow(f, n);
	}

    emxselectwind(cwp);
    return TRUE;
}


/* This function moves all windows down 'n' lines */

COMMAND(emxallwindown)

{
    return emxallwinup(f, -n);
}


void emxdeletewindow _ARGS1(WINDOW *, wp)

{
    WINDOW  *p;

    if (--wp->bufp->windows == 0)
	{
	/* Only window on this buffer - update buffer data */
	wp->bufp->dotp	= wp->dotp;
	wp->bufp->doto	= wp->doto;
	wp->bufp->markp = wp->markp;
	wp->bufp->marko = wp->marko;
	}

    if (wp == g_curwp)
	{
	if (wp->nextp)
	    emxselectwind(wp->nextp);
	else
	    emxselectwind(g_wheadp->nextp);
	}

    /* Take it out of the chain */
    if (wp == g_wheadp)
	g_wheadp = wp->nextp;
    else
	for (p = g_wheadp; p->nextp; p = p->nextp)
	    if (p->nextp == wp)
		{
		p->nextp = wp->nextp;
		break;
		}

    FREE(wp);
}


/* This command makes the current window the only window on the screen. Try
   to set the framing so that "." does not have to move on the display.
   Some care has to be taken to keep the values of dot and mark in the
   buffer structures right if the destruction of a window makes a buffer
   become undisplayed. */

int emxonlywindow _ARGS0()

{
    WINDOW *wp;
    LINE   *lp;
    int    i;
    WINDOW *nextp;

    /* Loop on all windows */
    for (wp = g_wheadp->nextp; wp; wp = nextp)
	{
	nextp = wp->nextp;
	if (wp != g_curwp)
	    emxdeletewindow(wp);
	}

    /* Update the screen data */
    wp = g_curwp;
    lp = wp->linep;
    i  = wp->toprow;
    while (i!=0 && lp->prevp!=g_curbp->linep)
	{
	--i;
	lp = lp->prevp;
	}

    wp->toprow = 0;
    wp->rows = g_nrow-2;		     /* 2 = mode, echo.	     */
    wp->linep  = lp;
    wp->flag  |= WFINFO|WFHARD;
    wp->nextp = (WINDOW *)NULL;
    g_wheadp->nextp = wp;
    return TRUE;
}


/* Split the current window. A window smaller than 3 lines cannot be split.
   The only other error that is possible is a "malloc" failure allocating
   the structure for the new window. */

int emxsplitwindow _ARGS0()

{
    WINDOW *wp;
    LINE   *lp;
    WINDOW *cwp;
    int    ntru;
    int    ntrl;
    int    ntrd;
    WINDOW *wp1;
    WINDOW *wp2;
    WINDOW	*winp;

    cwp = g_curwp;
    if (cwp->rows == 0)
	return FALSE;

    if (cwp->rows < 5)
	{
	emxhoot();
	emxmsgprintint("[Cannot split a %d line window]", cwp->rows);
	return FALSE;
	}

    if ((winp = (WINDOW *) calloc(1, sizeof(WINDOW))) == 0)
	return emxallocerr(sizeof(WINDOW));

    wp = winp;
    ++g_curbp->windows;			     /* Displayed twice.     */
    wp->bufp  = g_curbp;
    wp->dotp  = cwp->dotp;
    wp->doto  = cwp->doto;
    wp->markp = cwp->markp;
    wp->marko = cwp->marko;
    ntru = (cwp->rows-1) / 2;		      /* Upper size	      */
    ntrl = (cwp->rows-1) - ntru;      /* Lower size	      */
    lp = cwp->linep;
    ntrd = 0;
    while (lp != cwp->dotp)
	{
	++ntrd;
	lp = lp->nextp;
	}

    lp = cwp->linep;
    if (ntrd <= ntru)
	{		      /* Old is upper window.	      */
	if (ntrd == ntru)		/* Hit infoline.	*/
	    lp = lp->nextp;

	cwp->rows = ntru;
	wp->nextp = cwp->nextp;
	cwp->nextp = wp;
	wp->toprow = cwp->toprow+ntru+1;
	wp->rows = ntrl;
	}

    else
	{				 /* Old is lower window	 */
	wp1 = (WINDOW *)NULL;
	wp2 = g_wheadp->nextp;
	while (wp2 != cwp)
	    {
	    wp1 = wp2;
	    wp2 = wp2->nextp;
	    }

	if (wp1 == 0)
	    g_wheadp->nextp = wp;
	else
	    wp1->nextp = wp;

	wp->nextp = cwp;
	wp->toprow = cwp->toprow;
	wp->rows = ntru;
	++ntru;					/* Infoline */
	cwp->toprow += ntru;
	cwp->rows  = ntrl;
	while (ntru--)
	    lp = lp->nextp;
	}

    cwp->linep = lp;			/* Adjust the top lines		*/
    wp->linep = lp;			  /* if necessary.	  */
    cwp->flag |= WFINFO|WFHARD;
    wp->flag |= WFINFO|WFHARD;
    return TRUE;
}


/* Enlarge the current window. Find the window that loses space. Make sure
   it is big enough. If so, hack the window descriptions, and ask redisplay
   to do all the hard work. You don't just set "force reframe" because dot
   would move. */

COMMAND2(emxenlargewindow)

{
    WINDOW *adjwp;
    LINE   *lp;
    LINE   *blp;
    int    i;
    WINDOW *cwp;

    if (n < 0)
	return emxshrinkwindow(f, -n);

    if (g_wheadp->nextp->nextp == 0)
	{
	emxmsgprint(g_msgonlyonewin);
	return FALSE;
	}

    cwp = g_curwp;
    if ((adjwp = cwp->nextp) == 0)
	{
	adjwp = g_wheadp->nextp;
	while (adjwp->nextp != cwp)
	    adjwp = adjwp->nextp;
	}

    if (adjwp->rows <= n)
	{
	emxmsgprint(g_errimpossible);
	return FALSE;
	}

    /* Shrink below */
    if (cwp->nextp == adjwp)
	{
	lp = adjwp->linep;
	blp = adjwp->bufp->linep;
	for (i = 0; i < n && lp != blp; ++i)
	    lp = lp->nextp;

	adjwp->linep  = lp;
	adjwp->toprow += n;
	}

    else
	{
	lp = cwp->linep;
	for (i = 0; i < n && lp->prevp != g_curbp->linep; ++i)
	    lp = lp->prevp;

	cwp->linep  = lp;
	cwp->toprow -= n;
	}

    cwp->rows += n;
    adjwp->rows -= n;
    cwp->flag |= WFINFO|WFHARD;
    adjwp->flag |= WFINFO|WFHARD;
    return TRUE;
}


/* Shrink the current window. Find the window that gains space. Hack at the
   window descriptions. Ask the redisplay to do all the hard work. */

COMMAND2(emxshrinkwindow)

{
    WINDOW *adjwp;
    LINE   *lp;
    int    i;
    WINDOW *cwp;

    if (n < 0)
	return emxenlargewindow(f, -n);

    if (g_wheadp->nextp->nextp == 0)
	{
	emxmsgprint(g_msgonlyonewin);
	return FALSE;
	}

    cwp = g_curwp;
    if ((adjwp=cwp->nextp) == 0)
	{
	adjwp = g_wheadp->nextp;
	while (adjwp->nextp != cwp)
	    adjwp = adjwp->nextp;
	}

    if (cwp->rows <= n)
	{
	emxmsgprint(g_errimpossible);
	return FALSE;
	}

    /* Grow below */
    if (cwp->nextp == adjwp)
	{
	lp = adjwp->linep;
	for (i=0; i<n && lp->prevp!=adjwp->bufp->linep; ++i)
	    lp = lp->prevp;

	adjwp->linep  = lp;
	adjwp->toprow -= n;
	}

    else
	{
	lp = cwp->linep;
	for (i=0; i<n && lp!=g_curbp->linep; ++i)
	    lp = lp->nextp;

	cwp->linep  = lp;
	cwp->toprow += n;
	}

    cwp->rows -= n;
    adjwp->rows += n;
    cwp->flag |= WFINFO|WFHARD;
    adjwp->flag |= WFINFO|WFHARD;
    return TRUE;
}


/* Pick a window for a pop-up. Pick a window that looks onto an empty
   buffer, if any. If there is only one window and it has stuff, split it
   and return the half that doesn't have the cursor in it. Otherwise pick
   the uppermost window that isn't the current window. An LRU algorithm
   might be better. Return a pointer, or NULL on error. */

WINDOW	*emxwpopup _ARGS0()

{
    WINDOW *wp;
    LINE   *lp;

    /* Look for an empty buffer */
    for (wp = g_wheadp->nextp; wp; wp = wp->nextp)
	{
	lp = wp->bufp->linep;
	if (lp == lp->nextp ||
	    (lp->nextp == lp->prevp && lp->nextp->used == 0))
	    return wp;
	}

    if (g_wheadp->nextp->nextp == 0 && emxsplitwindow() == FALSE)
	return 0;

    /* Find window to use */
    for (wp = g_wheadp->nextp; wp && wp == g_curwp; wp = wp->nextp)
	;

    return wp;
}


/* Scroll the current window by n characters */

static int scrollwindow _ARGS1(int, n)

{
    int	    delta;
    WINDOW  *wp;

    /* Can't scroll zero */
    if (n == 0)
	return FALSE;

    wp = g_curwp;

    /* Scrolling right */
    if (n > 0)
	{
	/* Can't move further than the limit */
	if (wp->leftcol == MAXCOL - 1)
	    return FALSE;

	delta = (n < (MAXCOL - 1 - wp->leftcol)) ? n :
		    (MAXCOL - 1 - wp->leftcol);
	}

    /* Scrolling left */
    else
	{
	if (wp->leftcol == 0)
	    return FALSE;

	delta = (-n < wp->leftcol) ? n : -wp->leftcol;
	}

    /* Do the scroll */
    emximhorizontal(wp->toprow, wp->rows, delta);
    wp->flag |= WFSHIFT|WFHARD;
    wp->leftcol += delta;
    return TRUE;
}


/* Scroll the current window left by some amount */

COMMAND(emxscrollleft)

{
    return scrollwindow(-n);
}


/* Scroll the current window right by some amount */

COMMAND(emxscrollright)

{
    return scrollwindow(n);
}


int emxsetregion _ARGS6(REGION *, rp, LINE *, limitlp, LINE *, firstlp, int, firsto,
    LINE *, lastlp, int, lasto)

{
    LINE    *blp;
    LINE    *flp;
    int	    bsize;
    int	    fsize;

    /* Special case if the lines are the same */
    rp->lines = 1;
    if (firstlp == lastlp)
	{
	rp->firstlp = firstlp;
	rp->lastlp = firstlp;
	if (firsto < lasto)
	    {
	    rp->firsto = firsto;
	    rp->lasto = lasto;
	    rp->chars = lasto - firsto;
	    }

	else
	    {
	    rp->firsto = lasto;
	    rp->lasto = firsto;
	    rp->chars = firsto - lasto;
	    }

	/* Catch a starting offset past the end */
	if (rp->firsto > (int) firstlp->used)
	    {
	    rp->firsto = firstlp->used;
	    rp->chars++;
	    }

	return TRUE;
	}

    blp = firstlp;			/* Get region size */
    flp = firstlp;
    bsize = firsto;
    fsize = flp->used - bsize + 1;
    while (flp != limitlp || blp->prevp != limitlp)
	{
	rp->lines++;
	if (flp != limitlp)
	    {
	    flp = flp->nextp;
	    if (flp == lastlp)
		{
		rp->firstlp = firstlp;
		rp->firsto = firsto;
		rp->lastlp = lastlp;
		rp->lasto = lasto;
		rp->chars = fsize + lasto;

		/* Catch a starting offset past the end */
		if (rp->firsto > (int) firstlp->used)
		    {
		    rp->firsto = firstlp->used;
		    rp->chars++;
		    }

		return TRUE;
		}

	    fsize += flp->used + 1;
	    }

	if (blp->prevp != limitlp)
	    {
	    blp = blp->prevp;
	    bsize += blp->used + 1;
	    if (blp == lastlp)
		{
		rp->firstlp = lastlp;
		rp->firsto = lasto;
		rp->lastlp = firstlp;
		rp->lasto = firsto;
		rp->chars = bsize - lasto;

		/* Catch a starting offset past the end */
		if (rp->firsto > (int) rp->firstlp->used)
		    {
		    rp->firsto = rp->firstlp->used;
		    rp->chars++;
		    }

		return TRUE;
		}

	    }

	}

    return FALSE;
}


/* Start a selection */

static void selectcheck _ARGS0()

{
    if ((g_lastflag & LASTSELECT) == 0)	      /* Restart selection */
	{
	/* Clear old selection */
	emxunselectall();

	/* Set the mark to here */
	g_curwp->markp = g_curwp->dotp;
	g_curwp->marko = g_curwp->doto;
	}

    g_thisflag |= LASTSELECT;
}


COMMAND(emxselectline)

{
    if ((g_lastflag & LASTSELECT) == 0)	      /* Restart selection */
	{
	/* Clear old selection */
	emxquickunselectall();

	/* Set the mark to here */
	g_curwp->markp = g_curwp->dotp;
	g_curwp->marko = 0;
	}

    g_thisflag |= LASTSELECT;

    if (g_curwp->dotp != g_curbp->linep)
	{
	g_curwp->dotp = g_curwp->dotp->nextp;
	g_curwp->doto = 0;
	g_curwp->flag |= WFMOVE;
	}

    return emxselectregion();
}


COMMAND(emxselectforwchar)

{
    selectcheck();
    emxforwchar(f, n);
    return emxselectregion();
}


COMMAND(emxselectbackchar)

{
    selectcheck();
    emxbackchar(f, n);
    return emxselectregion();
}


COMMAND(emxselectforwword)

{
    selectcheck();
    emxforwword(f, n);
    return emxselectregion();
}


COMMAND(emxselectbackword)

{
    selectcheck();
    emxbackword(f, n);
    return emxselectregion();
}


COMMAND(emxselectforwline)

{
    selectcheck();
    emxforwline(f, n);
    return emxselectregion();
}


COMMAND(emxselectbackline)

{
    selectcheck();
    emxbackline(f, n);
    return emxselectregion();
}


COMMAND(emxselectbol)

{
    selectcheck();
    emxgotobol(f, n, keyp);
    return emxselectregion();
}


COMMAND(emxselecteol)

{
    selectcheck();
    emxgotoeol(f, n, keyp);
    return emxselectregion();
}


COMMAND(emxselectbob)

{
    selectcheck();
    emxgotobob();
    return emxselectregion();
}


COMMAND(emxselecteob)

{
    selectcheck();
    emxgotoeob(f, n, keyp);
    return emxselectregion();
}


/* Send a string to the shell */

static int sendtoshell _ARGS3(char *, str, char *, outfile, int, input)

{
    char    *cmd;
    int	    result;
    char    infile[256];

    sprintf(infile, "/tmp/emxin%d", (int) getpid());
    sprintf(outfile, "/tmp/emxout%d", (int) getpid());

    /* Build the full command line */
    cmd = (char *) malloc(strlen(str) + strlen(infile) + (2 * strlen(outfile)) + 20);
    if (input)
	(void) sprintf(cmd, "%s < %s > %s 2>> %s", str, infile, outfile, outfile);
    else
	(void) sprintf(cmd, "%s > %s 2>> %s", str, outfile, outfile);

    /* Execute it */
    result = HostSpawn(cmd);
    free(cmd);
    return result;
}


/* Send the current line to a shell and insert the output after the line */

COMMAND(emxlineshellcmd)

{
    char    *str;
    char    outfile[512];
    HostErrSType theErr;

    if (g_curwp->dotp->used == 0)
	return TRUE;

    /* Copy the current line */
    str = (char *) malloc(g_curwp->dotp->used + 2);
    memcpy(str, g_curwp->dotp->text, g_curwp->dotp->used);
    str[g_curwp->dotp->used] = 0;

    /* Send it to the shell and get back the result */
    sendtoshell(str, outfile, FALSE);
    FREE(str);

    /* Move the the start of the next line */
    g_curwp->dotp = g_curwp->dotp->nextp;
    g_curwp->doto = 0;

    /* Insert the results, if any */
    emxinsertafile(outfile);
    HostRemoveSingleFile(outfile,TRUE,&theErr);
    return TRUE;
}


/* Toggle the 'PC' state of a buffer */

COMMAND(emxtogglepcfile)

{
    if (g_curbp != g_blistp) {
	g_curbp->flag ^= BFISPC;
	g_curbp->flag |= BFCHG;
	emxbupdate(g_curbp, WFINFO);
	return emxok();
	}
}


