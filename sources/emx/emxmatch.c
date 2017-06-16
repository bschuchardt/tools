#define EMXMATCH_C TRUE
/*=========================================================================
 * Copyright (C) Servio Corp. 1991-1994.  All Rights Reserved.
 *
 * Name - 
 *
 * Purpose -
 *
 * $Id: emxmatch.c,v 30.4 1994/03/25 23:59:24 marcs Exp $
 *
 *========================================================================*/

/*
    EMX - Delimiter Matching

    This module contains the code to implement delimiter matching. The
    delimiter matching is generalized to support several different
    languages by using tables for each language. Three types of matching
    are supported:

    Forward-matching - Put the cursor on a starting delimiter (a left
    parenthesis, for example) and try to match the corresponding ending
    delimiter (right parenthesis, in this example).

    Back-matching - Put the cursor on an ending delimiter (a right
    parenthesis, for example) and match the corresponding starting
    delimiter (left parenthesis, in this example).

    Auto-matching - Each supported language has one delimiter which will
    trigger an auto-matching. When this delimiter is typed in the
    back-match feature will automatically be invoked and the corresponding
    starting delimiter will be found.

    In either of the above cases, when the match is found, it will be
    highlit.
*/

/*-----------------------DESCRIPTION OF TABLES USED-------------------------*/
/*

This pattern matching algorithm uses tables of the following three
structures (the structure name for each table appears in parentheses): a
table of all the delimiters for a language (DELIMITER), a table for all
character checking <to check if a character begins a delimiter>
(DELIM_BOUND) and a table which describes all the languages supported
(MODE_DESC).

Table of DELIMITERs - This table describes all the recognized delimiters
for a language. For each delimiter, the following information is contained:

delim_string - This is the actual delimiter in textual form.
matchforwaction	 - This is a pointer to the routine describing the action to be
	       taken when this delimiter is recognized as part of a forward
	       match.
matchbackaction	 - This is a pointer to the routine describing the action to be
	       taken when this delimiter is recognized as part of a backward
	       match.
matchforwcontext - This is a pointer to the routine which will verify the context
	       in which this delimiter is found as part of a forward match.
matchbackcontext - This is a pointer to the routine which will verify the context
	       in which this delimiter is found as part of a backward match.
forw_opposite- This is the index into the table of the target of the delimiter
	       during a forward match.	If this value is NIL, then a forward
	       match of the delimiter is invalid.
back_opposite- This is the index into the table of the target of the delimiter
	       during a backward match.	 If this value is NIL, then a
	       backward match of the delimiter is invalid.
bit_masks    - This byte is a bit mask which contains the following information
	       about each delimiter in the table.  The literal for the bit mask
	       is in parentheses.

	       (FIRST_IN_GROUP) - If this bit in on, then the delimiter is the
		   first delimiter in the table to use it's first character in
		   the first table.  The relevance of this will be described in
		   the description of the DELIM_BOUND structure.
	       (SPECIAL) - If this bit is on, then the delimiter is known as
		   a special delimiter for the language.  That is, the delimiter
		   will be recognized in every match.  For example, when in 'C'
		   mode, the double quote delimiters (") are special delimiters
		   and they will be recognized in every match.	On the other hand,
		   the braces ({,}) will only be recognized when a match is made
		   for one of them.
		(AUTOMATCH) - If this bit is on, then the delimiter is the
		   one that, when the auto-match-mode is turned on, will auto-
		   matically trigger a back match.
relvnt_delims- This is	a list of the indices of the delimiters which are relevant
	       to a particular match of the delimiter (not including special
	       delimiters).  For example, if we we're trying to match a '#if',
	       the following delimiters would be relevant to the search: #if,
	       #ifdef, #ifndef, #endif.
delim_len    - The length of the delimiter.
relevant_to_search - This flag will be on if the particular delimiter is relevant
	       to the current match.  This is the only field of the structure
	       which changes from match to match.  A delimiter may be relevant
	       for one match but not another.
NOTE: In this table, delimiters which begin with the same character are listed
      in order of decreasing size.  For example, #ifdef will be listed before
      #if but after #ifndef.  The last entry for the delimiters which start
      with a '#' is the '#' character.

Table of DELIM_BOUNDs - This table has an entry for all characters 0-7f and for
		two special characters, End of line and End of buffer.	For each
		character, there are the following fields.
beginning_ch  - This byte is either on or off depending if the corresponding
		character (the index of the byte) is the first character of a
		delimiter.  This entry is created by going through the delimiter
		table and for each delimiter, turning on the entry in this table.
begin_index	- This byte is the index into the delimiter table of the first entry
		which has the corresponding character (the index of the byte) as
		the first character of the delimiter.  This is used to cut down
		the search time.  Rather than always starting our search at the
		beginning of the table, we get the first character of the
		prospective delimiter that we've found and start searching
		from the index indicated by this field.

Table of MODE_DESCs - This table has an entry for each language and describes
		each supported language.
modedelims   - Table of delimiters for the language.
modestring   - String describing the language (to be displayed on the infoline)
		'C-mode' for example.
sense_flag    - Indicates case sensitivity on language.
*/
/*-------------------------ALGORITHM DESCRIPTION----------------------------*/
/*

Before starting any search, we must first create any tables which will be
needed as part of the search. We call the 'emxmatchtable' routine for this.
In this routine we first set the new language mode. With this done, we go
through the delimiter table and do the following for each delimiter.

    1)We set the length field for the delimiter.
    2)We check the first character of the delimiter and set the corresponding
      entry in 'DELIM_BOUND' to 1.
    3)If the 'FIRST_IN_GROUP' bit is on, we set the 'begin_index' of the
      'DELIM_BOUND' table to the index of the current delimiter. Later when we
      find a delimter starting with the corresponding charcter, we will start
      searching from this index in the table rather than from the beginning of
      the table.
    4)Finally, we check to see if the current delimiter is the auto-matching
      delimiter and check the case sensitivity flag.

Now our preparation is done and we're ready to do some delimiter matching.
As an example of our algorithm, we'll use the case of a forward match. When
the user places the cursor on a character and executes a forward match, we
do the following:

We take the character the cursor is on and search all the delimiters in the
delimiter table to find that character. If we find that character in a
delim- iter, then we check to see if the character is part of the
delimiter. We do this by checking the text around the character. If the
character isn't part of a delimiter, we exit with an error message. If it
is part of a delimiter, we start to search for the matching delimiter. At
this point we're in a search and must set up for this particular search.
First we go through all of the delimiters in the table and check if the
'SPECIAL' bit is on. If it is on, then they're relevant to this (and all
searches) so we turn on the 'relevant_- to_search' flag. We then go through
the 'relvnt_delims' array and turn on the same flag for all the delimiter's
whose indices are in the array. Now we're ready to search. At this point,
we search forward character by character. We get each character and check
it against the 'beginning_index' entry. If the corresponding entry is on
then this character is the start of a delimiter. For now, let's call the
current character we're checking an '#'. We go to the 'begin_index' for '#'
and check the delimiter table for the corresponding entry. We know that all
entries which begin with an '#' start at index 'begin_index' We also know
that the entry at 'begin_index' is the longest of those beginning with an
'#'. We also know that the last of the entries beginning with '#' is an '#'
so that we know we'll always find a match. Using the 'delim_len' of the
current delimiter, we get that many character sequentially from the text
and compare against the delimiter at 'begin_index'. If it doesn't match we
go to next delimiter in the table and put a NULL character at the
'delim_len'th location and compare. We do this until we get a match. We're
assured of a match because, for every multiple character delimiter, we have
an entry for the first character (single character delmiters have single
entries, by default, '[' for example. For example, in 'C', we have entries
for (in this order): '#ifndef', '#endif', '#ifdef', '#if' and '#'. In this
case, the '#' trigger our search and we'll match, whether to a delimiter or
to the '#'. Once we've made a match, we check to see if the
'relevant_to_search' flag is on. If it is, the match is OK, else it isn't
an we continue matching. Once we've found a valid match, we perform the
appropriate action (according to the table). This action routine will call
a context routine to check the context of the delimiter. The results of the
action routine will determine what happens next. We continue searching this
way until the action routine tells us to terminate the search, either
successfully or not. And that's all there is to it!! NOTE- Back search
works the same way except that it searches backward. It still looks for the
starting character of a delimiter to trigger the comparison. Auto_matching
is just a back search combined with the insertion of a character. */

/*-----------------------INCLUDE files--------------------------------------*/

#include "emx.h"


#if defined(FLG_PROTOTYPE)
static void fillcomparebuf (MATCH *mp, int limit, int theIndex);
#else
static void fillcomparebuf();
#endif

#if defined(FLG_PROTOTYPE)
static int blindsearch(MATCH *mp);
#else
static int blindsearch();
#endif

#if defined(FLG_PROTOTYPE)
static int endsearch (MATCH *mp);
#else
static int endsearch();
#endif

#if defined(FLG_PROTOTYPE)
static int endpair (MATCH *mp, int theIndex);
#else
static int endpair();
#endif

#if defined(FLG_PROTOTYPE)
static int invsearch (MATCH *mp);
#else
static int invsearch();
#endif

#if defined(FLG_PROTOTYPE)
static int noaction (MATCH *mp);
#else
static int noaction();
#endif

#if defined(FLG_PROTOTYPE)
static int nomatch (MATCH *mp);
#else
static int nomatch();
#endif

#if defined(FLG_PROTOTYPE)
static int spaceafter (MATCH *mp);
#else
static int spaceafter ();
#endif

#if defined(FLG_PROTOTYPE)
static int startpair (MATCH *mp, int theIndex);
#else
static int startpair();
#endif

#if defined(FLG_PROTOTYPE)
static int contsearch (MATCH *mp);
#else
static int contsearch();
#endif

/* Context routines */
#if defined(FLG_PROTOTYPE)
static int nocontext (MATCH *mp);
#else
static int   nocontext();
#endif

#if defined(FLG_PROTOTYPE)
static int startline (MATCH *mp);
#else
static int   startline();
#endif


/*------------------------DEFINE's------------------------------------------*/

/* Delimiter match search results */

#define NO_MATCH_FOUND	    2
#define UN_MATCHABLE	    3
#define MATCH_FOUND	    4

#define ELIP_LEN	    80


/* Bit masks for characteristics of each delimiter */

/* First delimiter in the table with its first character as first */

#define FIRST_IN_GROUP	1

/* Special delimiter, recognized in any search */

#define SPECIAL		2

/* The auto-match character for this language */

#define AUTOMATCH	4


/* Language case sensitivity */

#define CASE_SENSE	0
#define CASE_INSENSE	1


/* Search direction */

#define FORWARD		0
#define BACKWARD	1


/* Special character values */

#define END_LINE	1
#define END_BUFFER	2


/* List terminator */

#define NIL		99



/* The language delimiter tables */


static DELIMITER      emxdelim_c[] = {
{"\"",	   blindsearch, blindsearch,	nocontext, nocontext, 0,    0,	  0, 3, {0,NIL}},
{"#ifndef",startpair,	endpair,	startline, startline, 3,    NIL,  0, 1, {1,2,3,4,NIL}},
{"#ifdef", startpair,	endpair,	startline, startline, 3,    NIL,  0, 0, {1,2,3,4,NIL}},
{"#endif", endpair,	startpair,	startline, startline, NIL,  1,	  0, 0, {1,2,3,4,NIL}},
{"#if",	   startpair,	endpair,	startline, startline, 3,    NIL,  0, 0, {1,2,3,4,NIL}},
{"#",	   noaction,	noaction,	nomatch,   nomatch,   NIL,  NIL,  0, 0, {NIL}},
{"'",	   blindsearch, blindsearch,	nocontext, nocontext, 6,    6,	  0, 3, {6,NIL}},
{"(",	   startpair,	endpair,	nocontext, nocontext, 8,    NIL,  0, 1, {7,8,NIL}},
{")",	   endpair,	startpair,	nocontext, nocontext, NIL,  7,	  0, 1, {7,8,NIL}},
{"*/",	   invsearch,	blindsearch,	nocontext, nocontext, NIL,  10,	  0, 3, {10,NIL}},
{"/*",	   blindsearch, invsearch,	nocontext, nocontext, 9,    NIL,  0, 3, {9,NIL}},
{"*",	   noaction,	noaction,	nomatch,   nomatch,   NIL,  NIL,  0, 0, {NIL}},
{"/",	   noaction,	noaction,	nomatch,   nomatch,   NIL,  NIL,  0, 0, {NIL}},
{"[",	   startpair,	endpair,	nocontext, nocontext, 14,   NIL,  0, 1, {13,14,NIL}},
{"]",	   endpair,	startpair,	nocontext, nocontext, NIL,  13,	  0, 1, {13,14,NIL}},
{"{",	   startpair,	endpair,	nocontext, nocontext, 16,   NIL,  0, 1, {15,16,NIL}},
{"}",	   endpair,	startpair,	nocontext, nocontext, NIL,  15,	  0, 5, {15,16,NIL}},
{"\1",	   contsearch,	contsearch,	nocontext, nocontext, 17,   17,	  0, 1, {17,NIL}},
{"\2",	   endsearch,	endsearch,	nocontext, nocontext, 18,   18,	  0, 3, {18,NIL}},
{0}
};

static DELIMITER      emxdelim_st[] = {
{"\"",	   blindsearch, blindsearch,	nocontext, nocontext, 0,    0,	  0, 3, {0,NIL}},
{"'",	   blindsearch, blindsearch,	nocontext, nocontext, 1,    1,	  0, 3, {1,NIL}},
{"#(",	   startpair,	endpair,	nocontext, nocontext, 6,    NIL,  0, 1, {2,5,6,NIL}},
{"#[",	   startpair,	endpair,	nocontext, nocontext, 8,    NIL,  0, 0, {3,7,8,NIL}},
{"#",	   noaction,	noaction,	nomatch,   nomatch,   NIL,  NIL,  0, 0, {NIL}},
{"(",	   startpair,	endpair,	nocontext, nocontext, 6,    NIL,  2, 1, {2,5,6,NIL}},
{")",	   endpair,	startpair,	nocontext, nocontext, NIL,  5,	  0, 1, {2,5,6,NIL}},
{"[",	   startpair,	endpair,	nocontext, nocontext, 8,    NIL,  3, 1, {3,7,8,NIL}},
{"]",	   endpair,	startpair,	nocontext, nocontext, NIL,  7,	  0, 5, {3,7,8,NIL}},
{"{",	   startpair,	endpair,	nocontext, nocontext, 10,   NIL,  0, 1, {9,10,NIL}},
{"}",	   endpair,	startpair,	nocontext, nocontext, NIL,  9,	  0, 1, {9,10,NIL}},
{"\1",	   contsearch,	contsearch,	nocontext, nocontext, 11,   11,	  0, 1, {11,NIL}},
{"\2",	   endsearch,	endsearch,	nocontext, nocontext, 12,   12,	  0, 3, {12,NIL}},
{0}
};


#define MAX_MODES	2

MODE_DESC emxmodetable[MAX_MODES] = {
    {emxdelim_c,	"C",	CASE_SENSE},
    {emxdelim_st,	"ST",	CASE_SENSE},
    };



/*-------------------------- Functions  -----------------------------------*/

/* This routine will put an elipsis in the middle of the screen. This is
   done when there is more than a windowful between the two matches.  At 
   the first keystroke following the insertion of the elipsis, the screen   
   is restored from the elipsis down.  At this time the elipsis looks like  
   /\/\/\/\/\/\/\/\/\/\	*/

#if defined(FLG_PROTOTYPE)
static void insertelipsis (LINE *bottomlp, LINE *toplp)
#else
static void insertelipsis (bottomlp, toplp)
LINE	*bottomlp, *toplp;
#endif
{
    int	    lines_below, lines_above, theIndex;
    char    *ptr;
    char    *limitp;

    /* Divide the screen up to insert the elipsis*/
    lines_above = ((g_curwp->rows + 1)/2 - 1);
    lines_below = g_curwp->rows/2;

    /* Count the lines from the bottom till the middle of the screen. */
    for (theIndex = 1; theIndex < lines_below; theIndex++)
	bottomlp = bottomlp->prevp;

    g_elip_fline = bottomlp->prevp;  /* Save a pointer to the line which is	 the  */
				    /* bottom line in the group to be removed to */
				    /* make space for the elipsis. */

    /* Count the lines from the top till the  */
    /* middle of the screen. */
    for (theIndex = 1; theIndex < lines_above; theIndex++)
	toplp = toplp->nextp;

    g_elip_bline = toplp->nextp;	 /* Save a pointer to line which is the top   */
				/* line in the group to be removed to make   */
				/* space for the elipsis.   */

    if (!g_eliplinep)
	{
	/* Allocate space for the elipsis */
	if ((g_eliplinep = emxlalloc(ELIP_LEN)) == 0)
	    {
	    emxmsgprint("[Cannot allocate ellipsis line]");
	    return;
	    }

	/* Put the elipsis into the LINE structure */
	ptr = g_eliplinep->text;
	limitp = ptr + ELIP_LEN;
	while (ptr < limitp)
	    {
	    *ptr++ = '/';
	    *ptr++ = '\\';
	    }

	}

    toplp->nextp = g_eliplinep;	 /* Insert the elipsis.	  */
    bottomlp->prevp = g_eliplinep;
    g_eliplinep->prevp = toplp;
    g_eliplinep->nextp = bottomlp;
}


/* This routine will restore the screen to its pre-elipsis format. */

void emxfixelipsis (VOID)

{
    int	    theIndex;
    LINE    *lp;

    /* Is there an elipsis at all? */
    if (g_elipsis)
	{
	/* EMX makes sure nothing has changed since the elipsis was inserted */
	g_eliplinep->nextp->prevp = g_elip_fline;
	g_eliplinep->prevp->nextp = g_elip_bline;
	g_elipsis = FALSE;
	lp = g_curwp->dotp;

	/* If the search was backward, change the top line in the window */
	if (g_matchp->matchdirection == BACKWARD)
	    {
	    for (theIndex=1;theIndex!= g_curwp->rows;theIndex++)
		lp = lp->prevp;
	    }

	g_curwp->linep = lp;
	g_curwp->flag |= WFHARD;
	}
}


/* This function will return characters according to the setting of the
   variable 'g_matchcase'. */

#if defined(FLG_PROTOTYPE)
static char matchchar (MATCH *mp, LINE *lp, int theIndex)
#else
static char matchchar (mp, lp, theIndex)

MATCH	*mp;
LINE	*lp;
int	theIndex;
#endif
{
    char    ch;

    ch = lp->text[theIndex];
    if (mp->matchcase == CASE_INSENSE && ch >= 'a' && ch <= 'z')
	ch -= 0x20;

    return ch;
}


/* Set up delimiter matching for a particular language */

#if defined(FLG_PROTOTYPE)
MATCH *emxmatchtable (int newmode)
#else
MATCH *emxmatchtable (newmode)

int	newmode;
#endif
{
    MATCH	*mp;
    DELIMITER	*delimp;
    int		i;

    if (g_matchp && g_matchp->mode == newmode)
	return g_matchp;

    if (newmode >= MAX_MODES)
	newmode = C_MODE;

    /* Create a match table if none already */
    if (!g_matchp)
	g_matchp = (MATCH *) malloc(sizeof(MATCH));

    mp = g_matchp;

    /* Clean out the old table */
    memset(mp, 0, sizeof(MATCH));

    /* Set the new globals */
    g_curbp->mode = newmode;
    mp->mode = newmode;

    mp->modedelims = emxmodetable[newmode].modedelims;
    mp->modestring = emxmodetable[newmode].modestring;
    mp->matchcase = emxmodetable[newmode].sense_flag;

    /* Now, go through delimter table and:   */
    for (i = 0, delimp = mp->modedelims; delimp->delim_string[0]; i++, delimp++)
	{
	/* If the current delimiter is first in the group, set it as such */
	if (delimp->bit_masks & FIRST_IN_GROUP)
	    mp->delimcheck[delimp->delim_string[0]].begin_index = i;

	/* Set length */
	delimp->delim_len = strlen((char *)(&delimp->delim_string[0]));

	/* Mark starting character of delimiter	 */
	mp->delimcheck[delimp->delim_string[0]].beginning_ch = 1;

	/* If the current delimiter is the auto match delimiter */
	if (delimp->bit_masks & AUTOMATCH)
	    {
	    /* Set the automatch character to the last char of the delimiter */
	    mp->autoindex = i;
	    mp->automatchchar = delimp->delim_string[delimp->delim_len-1];
	    mp->automatchchar2 = mp->automatchchar;

	    /* If case-insensitive, there may be two automatch characters */
	    if (mp->matchcase == CASE_INSENSE)
		{
		if (ISUPPER(mp->automatchchar))
		    mp->automatchchar2 = TOLOWER(mp->automatchchar);
		}
	    }
	}

    return mp;
}


/* This routine will change the language mode of a buffer */

#if defined(FLG_PROTOTYPE)
static void changelangmode (int mode)
#else
static void changelangmode (mode)

int mode;
#endif
{
    /* Don't switch if already in new mode */
    if (g_curbp->mode != mode)
	{
	g_curbp->mode = mode;
	if (g_modes.automatch)
	    (void) emxmatchtable(mode);

	emxbupdate(g_curbp, WFINFO);
	}
}


/* This command will put the current buffer into 'C' mode */

COMMAND(emxcmode)

{
    changelangmode(C_MODE);
    return TRUE;
}


/* This command will put the current buffer into Smalltalk mode */

COMMAND(emxstmode)

{
    changelangmode(ST_MODE);
    return TRUE;
}


/* This command toggles auto-match mode */

COMMAND(emxautomatch)

{
    if (g_modes.automatch)
	{
	g_modes.automatch = FALSE;
	emxmsgprint("[Automatch off]");
	}
    else
	{
	g_modes.automatch = TRUE;
	emxmsgprint("[Automatch on]");
	(void) emxmatchtable(g_curbp->mode);
	}

    return TRUE;
}


/* Blindly search for a match or the end of the buffer. For example in 'C',
   if we're trying to match a '"', we'll blindly search for the matching
   '"'. We don't recognize nesting. We stop when we find the match or the
   end of the buffer. */

#if defined(FLG_PROTOTYPE)
static int blindsearch(MATCH *mp)
#else
static int blindsearch(mp)

MATCH	*mp;
#endif
{
    char   first_ch;
    int		    length;
    int		    delim_opp;
    char   chr;

    if (!(*mp->contextp)(mp, mp->srchindex))
	return FALSE;

    ++mp->nestcount;				/* Set up for the search.  */
    delim_opp = (*mp->oppositep)(mp, mp->srchindex);
					/* 'first_ch' is the first character of	 */
					/* what we're searching for.   */
    first_ch = mp->modedelims[delim_opp].delim_string[0];
    length = mp->modedelims[delim_opp].delim_len;

    while (TRUE)			/* Search till we find...  */
	{
	while (chr = (*mp->newcharp)(mp))
	    {
	    if (chr == first_ch)	/* The first char of a match or	  */
		{
		mp->wordstart = mp->charoffset;
		mp->wordlen = length;
		if ((*mp->contextp)(mp, delim_opp))
		    break;
		}
	    if (chr == END_BUFFER)	/* Run out of characters   */
		return NO_MATCH_FOUND;
	    }

	fillcomparebuf(mp, length, mp->charoffset); /* Try to compare out find. */
	if (strcmp((char *)mp->cmpbuf,
		(char *)&mp->modedelims[delim_opp].delim_string[0]) == 0) {
	    if (mp->matchdirection == FORWARD)
		mp->charoffset += (length - 1);
	    if (mp->nestcount != 1)		/* If the count wasn't one before the */
					/* search started, then this blind search*/
					/* simply took place while in the middle */
					/* of another search and that origninal	 */
					/* search can not resume.  */
		{
		--mp->nestcount;
		return FALSE;
		}
					/* Otherwise, this was the main search	 */
					/* now we're done.   */
	    else
		{			/* 'mp->srchindex' should be the index of	 */
					/* the found match.  */
		mp->srchindex = delim_opp;
		return MATCH_FOUND;
		}
	    }
	}
}


/* This routine will check to see if the character the user has placed the   */
/* cursor on is part of a matchable delimiter.	If it is a matchable  */
/* delimiter, then the search can start.  */

#if defined(FLG_PROTOTYPE)
int emxmatchcompare (MATCH *mp, int wordindex)
#else
int emxmatchcompare (mp, wordindex)

MATCH	*mp;
int	wordindex;
#endif
{
    /* Is there enough text to fit the delimiter on the line?  */
    if ((int) g_curwp->dotp->used < g_curwp->doto + mp->wordlen - wordindex)
	return FALSE;

    mp->wordstart = g_curwp->doto - wordindex;
    mp->wordend = g_curwp->doto + mp->wordlen - 1 - wordindex;
    fillcomparebuf(mp, mp->wordlen, mp->wordstart);

    /* If the delimiter matches, set  */
    /* 'mp->charoffset' to the starting search location.   */
    if (strcmp((char *)mp->cmpbuf, (char *)mp->modedelims[mp->delimindex].delim_string) == 0)
	{
	if (mp->matchdirection == FORWARD)
	    mp->charoffset = mp->wordend;
	else
	    mp->charoffset = mp->wordstart;

	return TRUE;
	}

    return FALSE;
}


/* Get the previous character */
#if defined(FLG_PROTOTYPE)
static char matchprevch (MATCH *mp)
#else
static char matchprevch (mp)
MATCH	*mp;
#endif
{
    /* More on the line? Get one */
    if (mp->charoffset > 0)
	{
	--mp->charoffset;
	mp->currchar = matchchar(mp, mp->srchlinep, mp->charoffset);
	}

    else
	{
	/* Move back a line, or return end of buffer */
	if (mp->srchlinep == g_curbp->linep)
	    mp->currchar = END_BUFFER;
	else
	    {
	    mp->srchlinep = mp->srchlinep->prevp;
	    ++mp->linespassed;
	    mp->charoffset = mp->srchlinep->used;
	    mp->currchar = END_LINE;
	    }

	}

    return mp->currchar;
}


/* Get the next character in the buffer */
#if defined(FLG_PROTOTYPE)
static char matchnextch (MATCH *mp)
#else
static char matchnextch (mp)
MATCH	*mp;
#endif
{
    /* End of line - move to the next */
    if (mp->currchar == END_LINE)
	{
	mp->srchlinep = mp->srchlinep->nextp;
	++mp->linespassed;
	mp->charoffset = 0;
	}

    else
	mp->charoffset++;

    /* Get a character from the line */
    if (mp->charoffset < (int) mp->srchlinep->used)
	mp->currchar = matchchar(mp, mp->srchlinep, mp->charoffset);
    else
	{
	/* No more on the line - check end of buffer */
	if (mp->srchlinep == g_curbp->linep)
	    mp->currchar = END_BUFFER;
	else
	    mp->currchar = END_LINE;
	}

    return mp->currchar;
}


/* This routine will fill our comparison buffer with characters to compare   */
/* against a delimiter in the table.   */

#if defined(FLG_PROTOTYPE)
static void fillcomparebuf (MATCH *mp, int limit, int theIndex)
#else
static void fillcomparebuf (mp, limit, theIndex)
MATCH	*mp;

int	limit, theIndex;
#endif
{
    int     count;

    /* Since delimiters cannot span line bounds, check how many we can get */
    if (mp->srchlinep == g_curbp->linep)
	{
	count = 1;
	mp->cmpbuf[0] = END_BUFFER;
	}

    else
	{
	count = mp->srchlinep->used - theIndex;
	count = count < limit ? count : limit;
	if (count)
	    memcpy(mp->cmpbuf, &mp->srchlinep->text[theIndex], count);
	}

    mp->cmpbuf[count] = 0;
}


/* This routine will search through the text until it matches a token.	It   */
/* always match a token because the end of buffer is a token to match. */
#if defined(FLG_PROTOTYPE)
static int    matchistoken (MATCH *mp)
#else
static int    matchistoken (mp)
MATCH	*mp;
#endif
{
    DELIMITER	*delim_ptr;
    int         confused;
    int         diff;
    int         oldoffset;
    int         oldindex;

    /* Get characters until we find a char which can start a delimiter.	  */
    while (!(mp->delimcheck[(*mp->newcharp)(mp)].beginning_ch))
	;

    /* Set the start searching index. */
    mp->srchindex = mp->delimcheck[mp->currchar].begin_index;

    /* Check for a possibly confused backward search */
    if ((confused = mp->modedelims[mp->srchindex].confuseindex) &&
	mp->matchdirection == BACKWARD)
	{
	diff = mp->modedelims[confused].delim_len -
		mp->modedelims[mp->srchindex].delim_len;
	if (diff <= mp->charoffset)
	    {
	    oldoffset = mp->charoffset;
	    oldindex = mp->srchindex;
	    mp->charoffset -= diff;
	    mp->srchindex = confused;
	    }

	else
	    confused = 0;
	}

    /* Maybe do this twice */
    while (TRUE)
	{
	/* Load the comparison buffer from the text */
        fillcomparebuf(mp, mp->modedelims[mp->srchindex].delim_len,mp->charoffset);

	/* Try to match a delimiter to the comparison buffer */
        while (mp->modedelims[mp->srchindex].delim_string[0])
	    {
	    delim_ptr = &mp->modedelims[mp->srchindex];

	    /* Put a null at end of the current delim to compare. */
	    mp->cmpbuf[delim_ptr->delim_len] = '\0';

	    /* If we've found a match, then... */
	    if (strcmp((char *)mp->cmpbuf, (char *)delim_ptr->delim_string) == 0)
	        {
	        mp->wordlen = delim_ptr->delim_len;
	        mp->wordstart = mp->charoffset;
				        /* Put search index past the end of the  */
				        /* matched delimiter.  We only have to   */
				        /* the index if we're doing a forward */
				        /* match.  If we were doing a backward   */
				        /* match, the index would already be  */
				        /* there.  Remember, it's always at the  */
				        /* start of a delimiter which for a back */
				        /* match is the starting location for the*/
				        /* match.	*/
	        if (mp->matchdirection == FORWARD)
		    mp->charoffset += (mp->wordlen - 1);
				        /* Is the current delimiter relevant to  */
				        /* our matching?  */
	        return delim_ptr->relevant_to_search;
	        }

	    else
		{
		if (!confused)
		    ++mp->srchindex;		    /* No match yet, get next delimiter */
		else
		    break;
		}
	    }

        if (!confused)
	    return FALSE;

        mp->srchindex = oldindex;
	mp->charoffset = oldoffset;
	confused = 0;
	}

}


/* This routine will check if the character that the user has placed the  */
/* cursor on is a matchable delimiter. */

#if defined(FLG_PROTOTYPE)
static char matchgettoken (MATCH *mp, int startindex)
#else
static char matchgettoken (mp, startindex)

MATCH	*mp;
int	startindex;
#endif
{
    int         theIndex;
    char        curchr;
    DELIMITER   *delimp;


    mp->delimindex = startindex;
				    /* Basically, we take the character that */
				    /* cursor is on and compare it against   */
				    /* each character of each delimiter in   */
				    /* table.  If the character matches, then*/
				    /* we try to match the delimiter around  */
				    /* the character. */
    curchr = matchchar(mp, g_curwp->dotp,g_curwp->doto);

    delimp = &mp->modedelims[mp->delimindex];
    while (delimp->delim_string[0])
	{
	theIndex = 0;
	mp->wordlen = delimp->delim_len;
	while (theIndex != mp->wordlen)
	    {
	    if (delimp->delim_string[theIndex] == (unsigned char) curchr)
				    /* At this point we've matched the */
				    /* character, now we'll try to match the */
				    /* delimiter.  */
		if (emxmatchcompare(mp, theIndex))
				    /* The delimiter matched. Now the context*/
				    /* should match and it should be a valid */
				    /* search.	A valid search means the  */
				    /* opposite index is not NIL */
		    return( ((*mp->contextp)(mp, mp->delimindex)) &&
			    ((*mp->oppositep)(mp, mp->delimindex) != NIL));

	    ++theIndex;
	    }

	++mp->delimindex;
	++delimp;
	}

    return FALSE;
}



/* This routine will contain a pointer to the current forward match action.  */

#if defined(FLG_PROTOTYPE)
static int matchforwaction (MATCH *mp, int theIndex)
#else
static int matchforwaction (mp, theIndex)
MATCH	*mp;

int	theIndex;
#endif
{
    return (*mp->modedelims[theIndex].forw_action)(mp, theIndex);
}


/* This routine will contain a pointer to the current backward match action. */

#if defined(FLG_PROTOTYPE)
static int matchbackaction (MATCH *mp, int theIndex)
#else
static int matchbackaction (mp, theIndex)
MATCH	*mp;

int	theIndex;
#endif
{
    return (*mp->modedelims[theIndex].back_action)(mp, theIndex);
}


/* This routine will contain a pointer to the current forward match context  */
/* verification routine.   */

#if defined(FLG_PROTOTYPE)
static int matchforwcontext (MATCH *mp, int theIndex)
#else
static int matchforwcontext (mp, theIndex)
MATCH	*mp;

int	theIndex;
#endif
{
    return (*mp->modedelims[theIndex].forw_context)(mp);
}


/* This routine will contain a pointer to the current backward match context */
/* verification routine.   */

#if defined(FLG_PROTOTYPE)
static int matchbackcontext (MATCH *mp, int theIndex)
#else
static int matchbackcontext (mp, theIndex)
MATCH	*mp;

int	theIndex;
#endif
{
    return (*mp->modedelims[theIndex].back_context)(mp);
}


/* This routine will return the index into the table of the target of the */
/* current forward match.  */

#if defined(FLG_PROTOTYPE)
static int matchgetforwopp (MATCH *mp, int theIndex)
#else
static int matchgetforwopp (mp, theIndex)
MATCH	*mp;

int theIndex;
#endif
{
    return (int) mp->modedelims[theIndex].forw_opposite;
}


/* This routine will return the index into the table of the target of the */
/* current backward match. */

#if defined(FLG_PROTOTYPE)
static int matchgetbackopp (MATCH *mp, int theIndex)
#else
static int matchgetbackopp (mp, theIndex)
MATCH	*mp;

int theIndex;
#endif
{
    return (int) mp->modedelims[theIndex].back_opposite;
}


/* This routine will indicate which delimiters are relevant to the current   */
/* match..	  */

#if defined(FLG_PROTOTYPE)
static void setrelevanceflags(MATCH *mp)
#else
static void setrelevanceflags(mp)
MATCH	*mp;
#endif
{
    DELIMITER	    *delimp;
    unsigned char   *reldelp;
    unsigned int    theIndex;

    /* Clear out all relevance flags. All special delimiters are relevant */
    delimp = mp->modedelims;
    while (delimp->delim_string[0])
	{
	if (delimp->bit_masks & SPECIAL)
	    delimp->relevant_to_search = 1;
	else
	    delimp->relevant_to_search = 0;

	delimp++;
	}

    /* Mark all delimiters which are relevant to this particular match */
    reldelp = mp->modedelims[mp->delimindex].relvnt_delims;
    while ((theIndex = *reldelp++) != NIL)
	mp->modedelims[theIndex].relevant_to_search = 1;
}


/* Move the cursor to the start of the last match */

COMMAND(emxmovetolastmatch)

{
    register WINDOW *cwp;
    register BUFFER *cbp;

    cwp = g_curwp;
    cbp = g_curbp;
    if (cbp->matchp == NULL)
	emxmsgprint("[No last match]");
    else
	{
	cwp->dotp = cbp->matchp;
	cwp->doto = cbp->matcho;
	cwp->flag |= WFMOVE;
	}

    return TRUE;
}


/* This routine will highlight the match that we found.	 */

#if defined(FLG_PROTOTYPE)
static int matchhighlight (MATCH *mp)
#else
static int matchhighlight (mp)
MATCH	*mp;
#endif
{
    int		    matcho;	    /* The starting location to highlight */
    LINE	    *lp;
    int		    linestotop;

    /* Count lines from cursor to top of window */
    linestotop = 0;
    for (lp = g_curwp->dotp; lp != g_curwp->linep; lp = lp->prevp)
	++linestotop;

    /* We highlight differently for back and forward matches */
    if (mp->matchdirection == FORWARD)
	{
				    /* Get index of start of delimiter.	  */
	matcho = mp->charoffset - (mp->modedelims[mp->srchindex].delim_len - 1);

				    /* If the match is off the screen...  */
	if ((mp->linespassed + linestotop) >= g_curwp->rows)
	    {
				    /* If the # of lines between the matches */
				    /* is more that a screenful, then we put */
				    /* an elipsis on the screen and display  */
				    /* both matches on the screen. */
	    if (mp->linespassed >= g_curwp->rows)
		{
		g_elipsis = TRUE;
		insertelipsis(mp->srchlinep,g_curwp->dotp);
		}

	    g_curwp->linep = g_curwp->dotp;
	    g_curwp->flag |= WFHARD;
	    }

	}

    else				    /* Backwards search	 */
	{
	matcho = mp->charoffset;
				    /* Is the match off the screen?   */
	if (mp->linespassed > linestotop)
	    {
				    /* If the match won't fit on the screen, */
				    /* put in an elipsis.   */
	    if (mp->linespassed >= g_curwp->rows)
		{
		g_elipsis = TRUE;
		insertelipsis(g_curwp->dotp,mp->srchlinep);
		}

	    g_curwp->linep = mp->srchlinep;
	    g_curwp->flag |= WFHARD;
	    }

	}

    /* Do the highlighting  */
    emximhighlight(g_curbp, mp->srchlinep, matcho, mp->srchlinep,
	  matcho + strlen((char *)mp->modedelims[mp->srchindex].delim_string));

    g_curbp->matchp = mp->srchlinep;	    /* Remember these values so we can */
    g_curbp->matcho = matcho;	    /* return to the last match.   */
    return TRUE;
}


/* This routine is called when a search ends.  It interprets the result of   */
/* the search and informs the user about the result.  */

static int checksearchresult

#if defined(FLG_PROTOTYPE)
    (MATCH *mp, int result)
#else
    (mp, result)

MATCH	*mp;
int	result;
#endif

{
    if (result != MATCH_FOUND)
	{
	if (!mp->silent)
	    emxhoot();

	if (result == UN_MATCHABLE)
	    emxmsgprint(g_msgcannotmatch);
	else
	    emxmsgprint("[Matching delimiter not found]");

	return FALSE;
	}

    else				    /* Great, we found a match. */
	return matchhighlight(mp);
}


/* This routine will try to match the delimiter */

#if defined(FLG_PROTOTYPE)
static int matchthedelim (MATCH *mp)
#else
static int matchthedelim (mp)
MATCH	*mp;
#endif
{
    int result;

    setrelevanceflags(mp);

    /* Loop until a match or the end of the buffer */
    while (TRUE)
	{
	if (matchistoken(mp))
	    if (result = (*mp->actionp)(mp, mp->srchindex))
		return checksearchresult(mp, result);
	}

}


/* This routine is the guts of the matching algorithm.	It is called for both*/
/* the forward and backward matching.  */

#if defined(FLG_PROTOTYPE)
static int patternsearch (MATCH *mp)
#else
static int patternsearch (mp)
MATCH	*mp;
#endif
{
    int result;

    /* See if the cursor is currently on a matchable delimiter */
    if (matchgettoken(mp, 0))
	{
	mp->currchar = 0;
	mp->nestcount = 0;
	mp->linespassed = 0;
	mp->srchindex = mp->delimindex;

	/* Do the initial action to prepare for the search. If the action
	   is blind search, the search will end right here */

	if (result = (*mp->actionp)(mp, mp->delimindex))
	    return checksearchresult(mp, result);

	return matchthedelim(mp);
	}

    /* Not on a matchable delimiter */
    else
	{
	emxhoot();
	emxmsgprint(g_msgcannotmatch);
	return FALSE;
	}

}


/* This routine will do the forward matching.	*/

COMMAND(emxforwmatch)

{
    MATCH   *mp;

    mp = emxmatchtable(g_curbp->mode);
    mp->matchdirection = FORWARD;
    mp->srchlinep = g_curwp->dotp;
    mp->oppositep = matchgetforwopp;
    mp->actionp = matchforwaction;
    mp->contextp = matchforwcontext;
    mp->newcharp = matchnextch;
    mp->silent = FALSE;
    return patternsearch(mp);
}


/* This routine will do the backward matching	*/

COMMAND(emxbackmatch)

{
    MATCH   *mp;

    mp = emxmatchtable(g_curbp->mode);
    mp->matchdirection = BACKWARD;
    mp->srchlinep = g_curwp->dotp;
    mp->oppositep = matchgetbackopp;
    mp->actionp = matchbackaction;
    mp->contextp = matchbackcontext;
    mp->newcharp = matchprevch;
    mp->silent = FALSE;
    return patternsearch(mp);
}


/* This routine will do the auto-matching */

COMMAND(emxsearchformatch)

{
    MATCH   *mp;
    int	    save_doto;

    emxselfinsert(FALSE, 1, keyp);		/* Insert the character.   */

    mp = emxmatchtable(g_curbp->mode);
    mp->matchdirection = BACKWARD;
    mp->srchlinep = g_curwp->dotp;
    mp->oppositep = matchgetbackopp;
    mp->actionp = matchbackaction;
    mp->contextp = matchbackcontext;
    mp->newcharp = matchprevch;
    mp->silent = TRUE;

    save_doto = g_curwp->doto;	      /* Save the cursor location because   */
					/* we'll be changing it.   */
    --g_curwp->doto;		      /* Decrement the cursor so we're pointing*/
					/* to the character before the one we */
					/* inserted.   */

    if (matchgettoken(mp, mp->srchindex=mp->autoindex)) /* Try to match the token because */
					/* although we've been triggered to match*/
					/* it may not be a matchable token */
					/* TREND vs END (Ha Ha) */
	{
	g_curwp->doto = save_doto;	/* Restore the cursor.	*/
	mp->currchar = 0;
	mp->linespassed = 0;
	mp->nestcount = 1;		/* Count starts at one because the auto	 */
					/* match char is our first delimiter  */

	return matchthedelim(mp);	/* From here, search as normal.	  */
	}

    g_curwp->doto = save_doto;		/* Restore the cursor.	Do it here also	 */
					/* in case we didn't do the above match. */
					/* We could have been triggered by SEND	 */
					/* rather than END and we wouldn't have	 */
					/* done the above match and wouldn't have*/
					/* updated the cursor.	*/
    return TRUE;
}


/*-------------------------- Action routines -------------------------------

    Context routines are called when a delimiter is matched to check if the
    delimiter has been found in the correct context of the text. A return
    of 'TRUE' indicates that the delimiter is in the correct context.

    Action routines are called when a delimiter has been matched. Upon a
    successful verification of the context, it will perform some action
    based on the type of delimiter matched.

---------------------------------------------------------------------------*/

/*  This routine is the generic 'start of pair' action routine.	This is	  
    called when we have the start of a pair of delimiters. It just checks
    to see if the context of the delimiter is correct and if it is
    increments the number of open delimiters to be matched. */

#if defined(FLG_PROTOTYPE)
static int startpair (MATCH *mp, int theIndex)
#else
static int startpair (mp, theIndex)
MATCH	*mp;

int	theIndex;
#endif
{
    if ((*mp->contextp)(mp, theIndex))
	++mp->nestcount;

    return FALSE;
}


/* This is the opposite of the above routine.  It is called when we have   */
/* the matching end delimiter.	It checks the context of the delimiter.	 If*/
/* the context is correct and this was the last open delimiter to match */
/* (mp->nestcount = 1), then this returns that the match has been found, else this */
/* just decrements the mp->nestcount. */

#if defined(FLG_PROTOTYPE)
static int endpair (MATCH *mp, int theIndex)
#else
static int endpair (mp, theIndex)
MATCH	*mp;

int	theIndex;
#endif
{
    if ((*mp->contextp)(mp, theIndex))
	{
	if (mp->nestcount != 1)
	    --mp->nestcount;
	else
	    return MATCH_FOUND;
	}

    return FALSE;
}


/* This routine ends a search unsuccessfully */

#if defined(FLG_PROTOTYPE)
static int endsearch (MATCH *mp)
#else
static int endsearch (mp)
MATCH	*mp;
#endif
{
    return NO_MATCH_FOUND;
}


/* This routine keeps the search going. */

#if defined(FLG_PROTOTYPE)
static int contsearch (MATCH *mp)
#else
static int contsearch (mp)
MATCH	*mp;
#endif
{
    return FALSE;
}


/* This routine returns telling us that we have tried to perform*/
/* some sort of invalid search.	 It could be invalid for  */
/* several reasons, among them starting within a comment and */
/* trying to forward match an ending delimiter.	 */

#if defined(FLG_PROTOTYPE)
static int invsearch (MATCH *mp)
#else
static int invsearch (mp)
MATCH	*mp;
#endif
{
    return UN_MATCHABLE;
}


/* Just a generic routine to indicate that there was no match	*/

#if defined(FLG_PROTOTYPE)
static int nomatch (MATCH *mp)
#else
static int nomatch (mp)
MATCH	*mp;
#endif
{
    return FALSE;
}


/* This is for a delimiter which requires no action to be performed */

#if defined(FLG_PROTOTYPE)
static int noaction (MATCH *mp)
#else
static int noaction (mp)
MATCH	*mp;
#endif
{
    return FALSE;
}


/* This routine checks to see if a delimiter is at the start of a line */

#if defined(FLG_PROTOTYPE)
static int startline (MATCH *mp)
#else
static int startline (mp)
MATCH	*mp;
#endif
{
    if (mp->wordstart == 0)
	{
	if (spaceafter(mp))
	    return TRUE;
	}

    return FALSE;
}


/* This routine indicates that there is no special context for */
/* the current delimiter.  */

#if defined(FLG_PROTOTYPE)
static int nocontext (MATCH *mp)
#else
static int nocontext (mp)
MATCH	*mp;
#endif
{
    return TRUE;
}


/* Compare the next character after the delimiter   */

#if defined(FLG_PROTOTYPE)
static char checknextchar (MATCH *mp)
#else
static char checknextchar (mp)
MATCH	*mp;
#endif
{
    return mp->srchlinep->text[mp->wordlen + mp->wordstart];
}


/* This routine checks to see if there is space after the */
/* delimiter.  Space could be the end of line, tab or space. */

#if defined(FLG_PROTOTYPE)
static int spaceafter (MATCH *mp)
#else
static int spaceafter (mp)
MATCH	*mp;
#endif
{
    char    ch;

    /* End of line?  */
    if (mp->srchlinep->used == mp->wordlen + mp->wordstart)
	return TRUE;

    /* TAB or space? */
    ch = checknextchar(mp);
    return ((ch == '\t') || (ch == ' '));
}
