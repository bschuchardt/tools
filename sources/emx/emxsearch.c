#define EMXSEARCH_C TRUE
/*=========================================================================
 * Copyright (C) Servio Corp. 1991-1994.  All Rights Reserved.
 *
 * Name - 
 *
 * Purpose -
 *
 * $Id: emxsearch.c,v 30.3 1994/01/29 02:06:32 marcs Exp $
 *
 *========================================================================*/

/*
    EMX - Search commands.

    The functions in this file implement the search commands (both plain and
    incremental searches are supported) and the query-replace command.

    The plain old search code is part of the original EMX. The
    incremental search code, and the query-replace code, is by Rich Ellison.

    Note funky code to highlight search/replace string.
*/

#include "emx.h"


#if defined(FLG_PROTOTYPE)
static int backsrch (int highlight);
#else
static int backsrch ();
#endif

#if defined(FLG_PROTOTYPE)
static int forwsrch (int highlight);
#else
static int forwsrch ();
#endif

#if defined(FLG_PROTOTYPE)
static int isearch (int dir);
#else
static int isearch ();
#endif

#if defined(FLG_PROTOTYPE)
static void isrchcpush (SRCHCOM cmds[], register int cmd);
#else
static void isrchcpush ();
#endif

#if defined(FLG_PROTOTYPE)
static int isrchfind (register int dir);
#else
static int isrchfind ();
#endif

#if defined(FLG_PROTOTYPE)
static void isrchdspl (const char *prompt, int flag);
#else
static void isrchdspl ();
#endif

#if defined(FLG_PROTOTYPE)
static int isrchpeek(SRCHCOM cmds[]);
#else
static int isrchpeek();
#endif

#if defined(FLG_PROTOTYPE)
static void isrchprompt (int dir, int flag, int success);
#else
static void isrchprompt ();
#endif

#if defined(FLG_PROTOTYPE)
static void isrchlpush(SRCHCOM cmds[]);
#else
static void isrchlpush();
#endif

#if defined(FLG_PROTOTYPE)
static int isrchundo (SRCHCOM cmds[], register int *pptr, register int *dir);
#else
static int isrchundo ();
#endif

/* Read a pattern. Stash it in the external variable "g_pattern". The "g_pattern" is not
   updated if the user types in an empty line. If the user typed an empty line,
   and there is no old pattern, it is an error. Display the old pattern, in the
   style of Jeff Lomicka. There is some do-it-yourself control expansion. */

#if defined(FLG_PROTOTYPE)
static int readpattern (const char *prompt, int (*funcp)())
#else
static int readpattern (prompt, funcp)

const char	*prompt;
int	(*funcp)();
#endif
{
    char    buf[256];

    sprintf(buf, "%s %s [%s]: ", prompt, g_srchstr, g_pattern);
    return emxmsgreply(buf, g_cmdbuf, sizeof(g_cmdbuf), funcp);
}


/* Search forward. Get a search string from the user, and search for it,
   starting at ".". If found, "." gets moved to just after the matched
   characters, and display does all the hard stuff. If not found, it just
   prints a message. */

static COMPLETION(emxforwsearchcomplete)

{
    if (status == TRUE)			    /* Specified */
	strcpy(g_pattern, g_cmdbuf);

    else if (status == FALSE)
	{
	if (!g_pattern[0])	    /* No old string */
	    return FALSE;
	}

    else
	return status;

    g_srchlastdir = SRCH_FORW;
    return emxsearchagain();
}


int emxforwsearch (VOID)

{
    g_srchall = FALSE;
    return readpattern("Search", emxforwsearchcomplete);
}


int emxforwsearchall (VOID)

{
    g_srchall = TRUE;
    return readpattern("Search", emxforwsearchcomplete);
}


/* Reverse search. Get a search string from the user, and search, starting at
   "." and proceeding toward the front of the buffer. If found "." is left
   pointing at the first character of the pattern [the last character that was
   matched]. */

static COMPLETION(emxbacksearchcomplete)

{
    if (status == TRUE)			    /* Specified */
	strcpy(g_pattern, g_cmdbuf);

    else if (status == FALSE)
	{
	if (!g_pattern[0])	    /* No old string */
	    return FALSE;
	}

    else
	return status;

    g_srchlastdir = SRCH_BACK;
    return emxsearchagain();
}


COMMAND(emxbacksearch)

{
    g_srchall = FALSE;
    return readpattern("Reverse Search", emxbacksearchcomplete);
}


/* Search again, using the same search string and direction as the last search
   command. The direction has been saved in "g_srchlastdir", so you know which
   way to go. */

int emxsearchagain (VOID)

{
    BUFFER  *bp;

    bp = NULL;
    if (g_srchlastdir == SRCH_FORW && g_pattern[0])
	while (TRUE)
	    {
	    if (forwsrch(TRUE))
		return TRUE;

	    /* Search all buffers? */
	    if (g_srchall)
		{
		/* Finished the buffer we started in? */
		if (bp != g_curbp)
		    {
		    if (!bp)
			bp = g_curbp;

		    /* Move to the next buffer */
		    emxforwbuffer();
		    emxgotobob();
		    continue;
		    }
		}

	    break;
	    }

    else if (g_srchlastdir == SRCH_BACK)
	{
	if (backsrch(TRUE))
	    return TRUE;
	}

    else
	return emxforwsearch();

    emxmsgprint("[Not found]");
    return FALSE;
}


/* Use incremental searching, initially in the forward direction.
   isearch ignores any explicit arguments. */

COMMAND(emxforwisearch)

{
    return isearch(SRCH_FORW);
}


/* Use incremental searching, initially in the reverse direction.
   isearch ignores any explicit arguments. */

COMMAND(emxbackisearch)

{
    return isearch(SRCH_BACK);
}


/*
   Incremental Search.
	dir is used as the initial direction to search.
	^N	find next occurance  (if first thing typed reuse old string).
	^P	find prev occurance  (if first thing typed reuse old string).
	^S	switch direction to forward, find next
	^R	switch direction to reverse, find prev
	^Q	quote next character (allows searching for ^N etc.)
	<ESC>	exit from Isearch.
	<DEL>	undoes last character typed. (tricky job to do this correctly).
	else	accumulate into search string
 */

static int isearchcomplete();


static COMPLETION(isearchquote)

{
    unsigned char ch;

    /* If the key is a reasonable quote char, insert it and return */
    if (emxquotedchar(g_keyp, &ch))
	{
	g_keyp = &g_keycurrent;
	g_keyp->keysym = ch;
	g_keyp->modifier = 0;
	g_keyp->funcp = NULL;
	return isearchcomplete(TRUE);
	}

    return emxonekey(isearchcomplete);
}


#if defined(FLG_PROTOTYPE)
static int isearchadd (char c)
#else
static int isearchadd (c)

char	c;
#endif
{
    if (g_srchindex == -1)
	g_srchindex = 0;

    if (g_srchindex == 0)
	g_srchstat = TRUE;

    g_pattern[g_srchindex++] = c;
    if (g_srchindex == MAXPAT)
	{
	emxmsgprint("[Pattern too long]");
	emxabort();
	g_srchstat = ABORT;
	return ABORT;
	}

    g_pattern[g_srchindex] = 0;
    isrchlpush(g_srchcmds);
    if (g_srchstat)
	{
	if (isrchfind(g_srchlastdir))
	    isrchcpush(g_srchcmds, c);
	else
	    {
	    g_srchstat = FALSE;
	    emxhoot();
	    isrchcpush(g_srchcmds, SRCH_ACCM);
	    }

	}

    else
	isrchcpush(g_srchcmds, SRCH_ACCM);

    isrchprompt(g_srchlastdir, FALSE, g_srchstat);
    return g_srchstat;
}



static COMPLETION(isearchcomplete)

{
    KEY	    *keyp;
    int	    newpptr;
    int	    newdir;
    char    c;
    char    *bufp;
    char    buf[256];

    /* Got a key - see what to do with it */
    emximlowlight();
    keyp = g_keyp;
    if (keyp->keysym == XK_F8)
	{
	/* Recall the current word */
	if (emxrecallword(g_oldcurwp, buf, MAXPAT - g_srchindex))
	    {
	    bufp = buf;
	    while (c = *bufp++)
		{
		if (isearchadd(c) == ABORT)
		    goto exit;
		}

	    }

	else
	    emxhoot();
	}

    else if (keyp->keysym == XK_Return)
	{
	emxmsgprint("[Done]");
	g_srchstat = TRUE;
	goto exit;
	}

    else if (keyp->funcp == g_fabortp)
	{
	g_curwp->dotp = g_srchlinep;
	g_curwp->doto = g_srchoffset;
	g_curwp->flag |= WFMOVE;
	emxabort();
	g_srchstat = ABORT;
	goto exit;
	}

    else if (keyp->keysym == 's' && keyp->modifier == ControlMask)
	{
	/* Not found - start a new search */
	if (g_srchstat == FALSE && g_srchlastdir == SRCH_FORW)
	    return isearch(g_srchlastdir);

	if (g_srchlastdir == SRCH_BACK)
	    {
	    g_srchlastdir = SRCH_FORW;
	    isrchlpush(g_srchcmds);
	    isrchcpush(g_srchcmds, SRCH_FORW);
	    g_srchstat = TRUE;
	    }

	/* Drop through to find next. */
	goto next;
	}

    else if (keyp->keysym == 'n' && keyp->modifier == ControlMask)
	{
next:
	if (g_srchstat==FALSE && g_srchlastdir==SRCH_FORW)
	    goto exit;

	isrchlpush(g_srchcmds);
	emxforwchar(FALSE, 1);
	if (isrchfind(SRCH_NEXT))
	    {
	    isrchcpush(g_srchcmds, SRCH_NEXT);
	    g_srchindex = strlen(g_pattern);
	    }

	else
	    {
	    emxbackchar(FALSE, 1);
	    emxhoot();
	    g_srchstat = FALSE;
	    }

	isrchprompt(g_srchlastdir, FALSE, g_srchstat);
	}

    else if (keyp->keysym == 'r' && keyp->modifier == ControlMask)
	{
	/* Not found - start a new search */
	if (g_srchstat == FALSE && g_srchlastdir == SRCH_BACK)
	    return isearch(g_srchlastdir);

	if (g_srchlastdir == SRCH_FORW)
	    {
	    g_srchlastdir = SRCH_BACK;
	    isrchlpush(g_srchcmds);
	    isrchcpush(g_srchcmds, SRCH_BACK);
	    g_srchstat = TRUE;
	    }

	/* Drop through to find previous. */
	goto prev;
	}

    else if (keyp->keysym == 'p' && keyp->modifier == ControlMask)
	{
prev:
	if (g_srchstat == FALSE && g_srchlastdir == SRCH_BACK)
	    goto exit;

	isrchlpush(g_srchcmds);
	emxbackchar(FALSE, 1);
	if (isrchfind(SRCH_PREV))
	    {
	    isrchcpush(g_srchcmds, SRCH_PREV);
	    g_srchindex = strlen(g_pattern);
	    }

	else
	    {
	    emxforwchar(FALSE, 1);
	    emxhoot();
	    g_srchstat = FALSE;
	    }

	isrchprompt(g_srchlastdir, FALSE, g_srchstat);
	}

    else if (keyp->funcp &&
	     (keyp->funcp->procp == emxforwdel ||
	      keyp->funcp->procp == emxbackdel))
	{
	newpptr = g_srchindex;
	newdir = g_srchlastdir;
	if (isrchundo(g_srchcmds, &newpptr, &newdir) != TRUE)
	    {
	    g_srchstat = ABORT;
	    goto exit;
	    }

	g_srchindex = newpptr;
	g_srchlastdir = newdir;
	if (isrchpeek(g_srchcmds) != SRCH_ACCM)
	    g_srchstat = TRUE;

	isrchprompt(g_srchlastdir, FALSE, g_srchstat);
	}

    else if (keyp->keysym == 'q' && keyp->modifier == ControlMask)
	{
	/* Wait for another key and handle it as a quote */
	return emxonekey(isearchquote);
	}

    else if (keyp->keysym == XK_Tab)
	{
	c = '\t';
	goto addchar;
	}

    else
	{
	if (keyp->keysym > 0xff || keyp->modifier)
	    {
	    emxmsgerase();
	    g_curwp->flag |= WFMOVE;
	    return emxexecute(FALSE, 1, keyp);
	    }

	c = keyp->keysym;

addchar:

	if (isearchadd(c) == ABORT)
	    goto exit;
	}

    /* Get another key and do it again */
    return emxonekey(isearchcomplete);

exit:

    return g_srchstat;
}


#if defined(FLG_PROTOTYPE)
static int isearch (int dir)
#else
static int isearch (dir)

int	dir;
#endif
{
    int	    c;

    /* Initialize */
    for (c = 0; c < MAXSRCH; c++)
	g_srchcmds[c].code = SRCH_NOPR;

    g_cip = 0;
    g_srchindex = -1;
    g_srchlinep = g_curwp->dotp;
    g_srchoffset = g_curwp->doto;
    g_srchlastdir = dir;
    isrchlpush(g_srchcmds);
    isrchcpush(g_srchcmds, SRCH_BEGIN);
    g_srchstat = TRUE;
    isrchprompt(g_srchlastdir, TRUE, g_srchstat);
    return emxonekey(isearchcomplete);
}


#if defined(FLG_PROTOTYPE)
static void isrchcpush (SRCHCOM cmds[], register int cmd)
#else
static void isrchcpush (cmds, cmd)

SRCHCOM		cmds[];
register int	cmd;
#endif
{
    if (++g_cip >= MAXSRCH)
	g_cip = 0;

    cmds[g_cip].code = cmd;
}


#if defined(FLG_PROTOTYPE)
static void isrchlpush(SRCHCOM cmds[])
#else
static void isrchlpush(cmds)

SRCHCOM	    cmds[];
#endif
{
    register int    ctp;
    register WINDOW *cwp;

    ctp = g_cip+1;
    if (ctp >= MAXSRCH)
	ctp = 0;

    cwp = g_curwp;
    cmds[ctp].code = SRCH_NOPR;
    cmds[ctp].doto = cwp->doto;
    cmds[ctp].dotp = cwp->dotp;
}


#if defined(FLG_PROTOTYPE)
static void isrchpop (SRCHCOM cmds[])
#else
static void isrchpop (cmds)

SRCHCOM	    cmds[];
#endif
{
    register WINDOW *cwp;

    if (cmds[g_cip].code != SRCH_NOPR)
	{
	cwp = g_curwp;
	cwp->doto  = cmds[g_cip].doto;
	cwp->dotp  = cmds[g_cip].dotp;
	cwp->flag |= WFMOVE;
	cmds[g_cip].code = SRCH_NOPR;
	}

    if (--g_cip <= 0)
	g_cip = MAXSRCH-1;
}


#if defined(FLG_PROTOTYPE)
static int isrchpeek(SRCHCOM cmds[])
#else
static int isrchpeek(cmds)

SRCHCOM	    cmds[];
#endif
{
    if (g_cip == 0)
	return (cmds[MAXSRCH-1].code);
    else
	return (cmds[g_cip-1].code);
}


#if defined(FLG_PROTOTYPE)
static int isrchundo (SRCHCOM cmds[], register int *pptr, register int *dir)
#else
static int isrchundo (cmds, pptr, dir)

SRCHCOM	    cmds[];
register int	*pptr;
register int	*dir;
#endif
{
    switch (cmds[g_cip].code)
	{
	case SRCH_NOPR:
	case SRCH_BEGIN:
	case SRCH_NEXT:
	case SRCH_PREV:
	    break;

	case SRCH_FORW:
	    *dir = SRCH_BACK;
	    break;

	case SRCH_BACK:
	    *dir = SRCH_FORW;
	    break;

	case SRCH_ACCM:
	default:
	    *pptr -= 1;
	    if (*pptr < 0)
		*pptr = 0;

	    g_pattern[*pptr] = 0;
	    break;
	}

    isrchpop(cmds);
    return TRUE;
}


#if defined(FLG_PROTOTYPE)
static int isrchfind (register int dir)
#else
static int isrchfind (dir)

register int	dir;
#endif
{
    register int    plen;

    plen = strlen(g_pattern);
    if (plen != 0)
	{
	if (dir == SRCH_FORW || dir == SRCH_NEXT)
	    {
	    emxbackchar(FALSE, plen);
	    if (forwsrch(TRUE) == FALSE)
		{
		emxforwchar(FALSE, plen);
		return FALSE;
		}

	    return TRUE;
	    }

	if (dir == SRCH_BACK || dir == SRCH_PREV)
	    {
	    emxforwchar(FALSE, plen);
	    if (backsrch(TRUE) == FALSE)
		{
		emxbackchar(FALSE, plen);
		return FALSE;
		}

	    return TRUE;
	    }

	emxmsgprint("bug");
	emxabort();
	return FALSE;
	}

    return FALSE;
}


/* Prompt for the interactive search */

#if defined(FLG_PROTOTYPE)
static void isrchprompt (int dir, int flag, int success)
#else
static void isrchprompt (dir, flag, success)
int dir, flag, success;
#endif
{
    const char *p3 = (g_modes.casesensitive ?
		"i-search (case sense)" : "i-search");
    const char *q3 = (g_modes.casesensitive ?
		"i-search backward (case sense)" : "i-search backward");

    if (dir == SRCH_FORW)
	{
	if (success)
	    isrchdspl(p3, flag);
	else
	   isrchdspl("Can't find", flag);
	}

    else if (dir == SRCH_BACK)
	{
	if (success)
	   isrchdspl(q3, flag);
	else
	    isrchdspl("Can't find backward", flag);
	}

}


/* Prompt writing routine for the incremental search.
   The "prompt" is just a string. The "flag" determines
   if a "[ ]" or ":" embelishment is used. */

#if defined(FLG_PROTOTYPE)
static void isrchdspl (const char *prompt, int flag)
#else
static void isrchdspl (prompt, flag)

const char	*prompt;
int flag;
#endif
{
    const char    *ptr;
    char    buf[128];

    if (flag)
	ptr = "%s [%s]";
    else
	ptr = "%s: %s";

    sprintf(buf, ptr, prompt, g_pattern);
    emxmsgprint(buf);
}


/* Query Replace. Replace strings selectively. Does a search and replace
   operation. A space or a comma replaces the string, a period replaces and
   quits, an n doesn't replace, a C-G quits.  */

static COMPLETION(emxqueryreplacecomplete)

{
    int	    dotmoved;
    int	    s;
    KEY	    *kp;
    char    buf[256];

    if (g_replhits == 0)
	{
	if (status == ABORT)
	    return status;

	sprintf(buf, "Replace: [%s] -> [%s]", g_pattern, g_replstr);
	emxmsgprint(buf);

	g_repllen = strlen(g_pattern);

	/* Save the starting cursor position */
	g_replline = g_curwp->dotp;
	g_reploff = g_curwp->doto;
	g_replcount = 0;

	/* Look for the string first */
	goto again;
	}

    /* Search forward repeatedly, checking each time whether to insert
       or not.	The "!" case makes the check always true, so it gets put
       into a tighter loop for efficiency. */


    /* Key entered - see what to do */
    emximlowlight();
    kp = g_keyp;
    if (kp->funcp == g_fabortp)
	{
	emxabort();
	goto stopsearch;
	}

    else if (kp->funcp &&
	(kp->funcp->procp == emxcasesense ||
	 kp->funcp->procp == emxendmacro ||
	 kp->funcp->procp == emxexitflushall))
	{
	(*kp->funcp->procp)();
	return emxonekey(emxqueryreplacecomplete);
	}

    else if (kp->modifier == 0)
	{
	if (kp->keysym == ' ' ||
	    kp->keysym == ',' ||
	    kp->keysym == '.' ||
	    kp->keysym == '1')
	    {
	    dotmoved = (g_curwp->dotp == g_replline);
	    if ((s = emxlreplace(g_repllen, g_replstr,
			      g_modes.casesensitive)) == FALSE)
		goto exit;

	    g_replcount++;
	    if (dotmoved)
		g_replline = g_curwp->dotp;

	    if (kp->keysym == '.')
		goto stopsearch;

	    if (kp->keysym == '1')
		goto holdhere;

	    goto again;
	    }

	else if (kp->keysym == '!')
	    {
	    while (TRUE)
		{
		dotmoved = (g_curwp->dotp == g_replline);
		if ((s = emxlreplace(g_repllen, g_replstr,
				  g_modes.casesensitive)) == FALSE)
		    goto exit;

		g_replcount++;
		if (dotmoved)
		    g_replline = g_curwp->dotp;

		while (!forwsrch(FALSE))
		    {
		    /* Roaming the buffers? */
		    if (!g_srchall)
			goto stopsearch;

		    if (g_srchbp == g_curbp)
			goto stopsearch;

		    if (!g_srchbp)
			g_srchbp = g_curbp;

		    /* Move to the next buffer */
		    emxforwbuffer();
		    emxgotobob();
		    }
		}
	    }

	else if (kp->keysym == 'n')
	    goto again;
	}

    emxmsgprint("<SP>[,]Replace, [1]Rep-one, [.]Rep-end, [n]Skip, [!]All, [^-g]Quit");
    return emxonekey(emxqueryreplacecomplete);

again:

    /* Look for another string and wait for a key */
    g_replhits++;
    while (TRUE)
	{
	if (forwsrch(TRUE) == TRUE)
	    return emxonekey(emxqueryreplacecomplete);

	/* Roaming the buffers? */
	if (!g_srchall)
	    break;

	if (g_srchbp == g_curbp)
	    break;

	if (!g_srchbp)
	    g_srchbp = g_curbp;

	/* Move to the next buffer */
	emxforwbuffer();
	emxgotobob();
	}

stopsearch:

    /* Return to the start */
    if (g_srchall && g_srchbp && g_srchbp != g_curbp)
	emxusebuf(g_srchbp);

    g_curwp->dotp = g_replline;
    g_curwp->doto = g_reploff;
    g_curwp->flag |= WFHARD;

holdhere:

    if (g_replcount == 0)
	emxmsgprint("[No replacements]");
    else if (g_replcount == 1)
	emxmsgprint("[1 replacement]");
    else
	emxmsgprintint("[%d replacements]", g_replcount);

    g_filechanges += g_replcount;
    s = TRUE;

exit:

    g_readinginput = FALSE;
    return s;
}


static COMPLETION(emxqueryreplacepartial)

{
    if (status == TRUE)			    /* Specified */
	strcpy(g_pattern, g_cmdbuf);

    else if (status == FALSE)
	{
	if (!g_pattern[0])	    /* No old string */
	    return FALSE;
	}

    else
	return status;

    return emxmsgreply("New string: ", g_replstr, MAXPAT, emxqueryreplacecomplete);
}


COMMAND(emxqueryreplace)

{
    g_readinginput = TRUE;
    g_replhits = 0;
    g_srchall = FALSE;
    g_srchbp = NULL;
    return readpattern(g_msgoldstring, emxqueryreplacepartial);
}


COMMAND(emxqueryreplaceall)

{
    g_readinginput = TRUE;
    g_replhits = 0;
    g_srchall = TRUE;
    g_srchbp = NULL;
    return readpattern(g_msgoldstring, emxqueryreplacepartial);
}


/* Build lower and uppercase strings */

#if defined(FLG_PROTOTYPE)
static void changepattern (char *lbufp, char *ubufp)
#else
static void changepattern (lbufp, ubufp)

char	*lbufp;
char	*ubufp;
#endif
{
    char    *lpp;
    char    *upp;

    strcpy(lbufp, g_pattern);
    for (lpp = lbufp, upp = ubufp; *upp = *lpp; upp++, lpp++)
	{
	if (*lpp <= 'Z' && *lpp >= 'A')
	    *lpp += 0x20;

	else if (*lpp >= 'a' && *lpp <= 'z')
	    *upp = *lpp - 0x20;
	}

}


/* This routine does the real work of a forward search. The pattern is sitting
   in the external variable "g_pattern". If found, dot is updated, the window system
   is notified of the change, and TRUE is returned. If the string isn't found,
   FALSE is returned. */

#if defined(FLG_PROTOTYPE)
static int forwsrch (int highlight)
#else
static int forwsrch (highlight)

int	highlight;
#endif
{
    register LINE   *clp;
    register char   *textp;
    register char   *endp;
    register char   c;
    register char   *patp;
    char	*altp;
    int		lasto;
    int		i, count;

    LINE	*tlp;
    char	*ntextp;
    char	*nendp;
    char	*pp;
    char	*app;
    LINE	*endlp;

    char	lpattern[80];
    char	upattern[80];

    /* Make the pattern case-insensitive if need be */
    if (g_modes.casesensitive)
	{
	patp = g_pattern;
	altp = (char *) NULL;
	}

    else
	{
	changepattern(lpattern, upattern);
	patp = lpattern;
	altp = upattern;
	}

    endlp = g_curbp->linep;
    clp = g_curwp->dotp;
    if (clp == endlp)
	return FALSE;

    endp = clp->text + clp->used;
    textp = clp->text + g_curwp->doto;

    while (TRUE)
	{
	/* Get the next character */
	if (textp == endp)
	    {
	    if ((clp = clp->nextp) == endlp)
		break;

	    endp = clp->text + clp->used;
	    textp = clp->text;
	    c = '\n';
	    }

	else
	    c = *textp++;

	/* Compare */
	if (c == *patp || (altp && c == *altp))
	    {
	    /* First character matched */
	    tlp = clp;
	    ntextp = textp;
	    nendp = endp;
	    pp = patp;
	    app = altp;
	    while (TRUE)
		{
		/* More to match */
		if (*(++pp))
		    {
		    app++;

		    /* Get the next character */
		    if (ntextp == nendp)
			{
			if ((tlp = tlp->nextp) == endlp)
			    break;

			nendp = tlp->text + tlp->used;
			ntextp = tlp->text;
			c = '\n';
			}

		    else
			c = *ntextp++;

		    /* Compare */
		    if (c == *pp || (altp && c == *app))
			continue;

		    /* No match - go back to the previous spot */
		    break;
		    }

		/* Matched the whole string */
		g_curwp->dotp  = tlp;
		g_curwp->doto  = ntextp - tlp->text;
		g_curwp->flag |= WFMOVE;
		if (highlight)
		    {
		    /* Find the first and last positions */
		    endlp = g_curwp->dotp;
		    lasto = g_curwp->doto;
		    count = strlen(g_pattern);
		    for (i = 0; i < count; i++)
			emxbackchar(FALSE, 1);

		    emximhighlight(g_curbp, g_curwp->dotp, g_curwp->doto,
				endlp, lasto);

		    g_curwp->dotp  = tlp;
		    g_curwp->doto  = ntextp - tlp->text;
		    g_curwp->flag |= WFMOVE;
		    }

		return TRUE;
		}

	    }

	}

    return FALSE;
}


/* This routine does the real work of a backward search. The pattern is sitting
   in the external variable "g_pattern". If found, dot is updated, the window system
   is notified of the change, and TRUE is returned. If the string isn't found,
   FALSE is returned */

#if defined(FLG_PROTOTYPE)
static int backsrch (int highlight)
#else
static int backsrch (highlight)

int	highlight;
#endif
{
    register LINE   *clp;
    register char   *textp;
    register char   *endp;
    register char   c;
    register char   *patp;
    char	*altp;

    LINE	*tlp;
    char	*ntextp;
    char	*nendp;
    char	*pp;
    char	*pstartp;
    char	*app;
    LINE	*endlp;
    LINE	*startlp;
    int		starto;

    char	lpattern[80];
    char	upattern[80];

    /* Make the pattern case-insensitive if need be */
    if (g_modes.casesensitive)
	{
	pstartp = g_pattern;
	altp = (char *) NULL;
	}

    else
	{
	changepattern(lpattern, upattern);
	pstartp = lpattern;
	altp = upattern + strlen(upattern) - 1;
	}

    /* Make the pattern case-insensitive if need be */
    patp = pstartp + strlen(pstartp) - 1;

    endlp = g_curbp->linep;
    clp = g_curwp->dotp;
    endp = clp->text;
    textp = clp->text + g_curwp->doto;

    while (TRUE)
	{
	/* Get the next character */
	if (textp == endp)
	    {
	    if ((clp = clp->prevp) == endlp)
		break;

	    textp = clp->text + clp->used;
	    endp = clp->text;
	    c = '\n';
	    }

	else
	    c = *--textp;

	/* Compare */
	if (c == *patp || (altp && c == *altp))
	    {
	    /* First character matched */
	    tlp = clp;
	    ntextp = textp;
	    nendp = endp;
	    pp = patp;
	    app = altp;
	    while (TRUE)
		{
		/* Matched the whole string */
		if (--pp >= pstartp)
		    {
		    app--;

		    /* Get the previous character */
		    if (ntextp == nendp)
			{
			if ((tlp = tlp->prevp) == endlp)
			    break;

			ntextp = tlp->text + tlp->used;
			nendp = tlp->text;
			c = '\n';
			}

		    else
			c = *--ntextp;

		    /* Compare */
		    if (c == *pp || (altp && c == *app))
			continue;

		    /* No match - go back to the previous spot */
		    break;
		    }

		/* Matched the whole string */
		if (highlight)
		    {
		    startlp = tlp;
		    starto = ntextp - tlp->text;
		    g_curwp->dotp = clp;
		    g_curwp->doto = textp - clp->text;
		    emxforwchar(FALSE, 1);
		    emximhighlight(g_curbp, startlp, starto, g_curwp->dotp,
				g_curwp->doto);
		    }

		g_curwp->dotp  = tlp;
		g_curwp->doto  = ntextp - tlp->text;
		g_curwp->flag |= WFMOVE;
		return TRUE;
		}

	    }

	}

    return FALSE;
}


/* toggle the case sense flag */

COMMAND(emxcasesense)

{
    if (g_modes.casesensitive)
	{
	g_modes.casesensitive = FALSE;
	g_srchstr = g_nsense;
	}

    else
	{
	g_modes.casesensitive = TRUE;
	g_srchstr = g_ssense;
	}

    emxmsgprint(g_srchstr);
    return TRUE;
}
