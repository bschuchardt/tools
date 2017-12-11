#define EMXMAIN_C TRUE
/*=========================================================================
 * Copyright (C) Servio Corp. 1991-1994.  All Rights Reserved.
 *
 * Name -
 *
 * Purpose -
 *
 * $Id: emxmain.c,v 30.6 1994/01/29 02:06:23 marcs Exp $
 *
 *========================================================================*/

/*
    EMX - Main

*/


/* Force data to be declared and initialized */
#define DEFINE

#include "emx.h"

#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#ifndef GEODE
#include <dirent.h>
#include <pwd.h>
#endif

#include <errno.h>

#if defined(STANDALONE)
/* the clipboard atom isn't defined by XAtom.h and has to be established */
static Atom XA_CLIPBOARD;
static int clipboardDefined = 0;
#endif

/* Externs for comparison */


static void defaultfixup (VOID);


static void eventinit (VOID);


#if defined(FLG_PROTOTYPE)
static int flushall (int terminate, int flag);
#else
static int flushall ();
#endif

#if defined(FLG_PROTOTYPE)
static int funcfinish (int status);
#else
static int funcfinish ();
#endif

#if defined(FLG_PROTOTYPE)
static int makecurrentdir (char *newdir);
#else
static int makecurrentdir ();
#endif

#if defined(FLG_PROTOTYPE)
static void emxmsgputchar (register int c);
#else
static void emxmsgputchar ();
#endif

#if defined(FLG_PROTOTYPE)
static void emxmsgputstr (int count, char *str);
#else
static void emxmsgputstr ();
#endif

#if defined(FLG_PROTOTYPE)
static void processargs (char *argv[], char *titlep);
#else
static void processargs ();
#endif

#if defined(FLG_PROTOTYPE)
static void requeuekey (KEY *keyp);
#else
static void requeuekey ();
#endif


/* Load the user's default customizations */

int emxdefaultcustomization _ARGS0()

{
    char  *envp;
    char  buf[1024];

    if (g_customization)
	return emxplaystring(g_customization);

    else if (envp = getenv("EMXRC"))
	return emxplayfile(envp, TRUE);

    else if (!g_customization && (envp = getenv("HOME")))
	{
	strcpy(buf, envp);
	strcat(buf, "/.emxrc");
	return emxplayfile(buf, FALSE);
	}

    return TRUE;
}


#if defined(FLG_PROTOTYPE)
void emxeditinit (int argc, char **argv)
#else
void emxeditinit (argc, argv)

int	argc;
char	**argv;
#endif
{
    BUFFER  *bp;
    WINDOW  *wp;
    char    *loadp;
#ifdef STANDALONE
    char    *sizep;
#endif
    char    title[256];

    g_savebuf[0] = 0;
    g_pattern[0] = 0;

    /* Set up the default key bindings */
    emxkeymapinit();
    eventinit();

    /* Get the current working directory */
#if defined(hpux) || defined(FLG_HPUX_UNIX) || defined(_SEQUENT_) || defined(FLG_SOLARIS_UNIX) || defined(_SOLARIS_)
    if (!getcwd(g_currentdir, sizeof(g_currentdir)))
#else
    if (!getwd(g_currentdir))
#endif
	strcpy(g_currentdir, ".");

    strcpy(g_initialdir, g_currentdir);

    g_curbp = NULL;

    /* Create the private list buffer for help, buffer list, displaybindings, etc. */
    g_blistp = emxbcreate(g_bufferlist);
    g_blistp->flag = BFVIEW;
    g_blistp->filename[0] = 0;

    /* Create the prompt line buffer and window */
    g_promptbp = bp = emxbcreate("#emx#prompt#");
    bp->windows = 1;

    /* Give it an empty line */
    emxaddline(bp, "");

    g_promptwp = wp = (WINDOW *) calloc(1, sizeof(WINDOW));
    g_wheadp = wp;

    wp->bufp  = bp;
    wp->linep = bp->linep;
    wp->dotp  = bp->linep->nextp;
    wp->flag  = WFINFO|WFHARD;

    /* Create a default empty text buffer in case no files are read in */
    bp = g_curbp = emxbfind(g_emptybuf, TRUE);
    bp->windows = 1;			/* Displayed */

    /* Create a single window */
    wp = (WINDOW *) calloc(1, sizeof(WINDOW));
    g_curwp = wp;
    g_wheadp->nextp = wp;

    wp->bufp  = bp;
    wp->linep = bp->linep;
    wp->dotp  = bp->linep;
    wp->flag  = WFINFO|WFHARD;

#ifdef STANDALONE
    /* See if there is an initial size */
    if (sizep = getenv("EMXSIZE"))
	sscanf(sizep, "%dx%d", &g_initheight, &g_initwidth);
#endif

    /* See if there is a customization or pre-init file to be started */
    emxdefaultcustomization();

#ifdef STANDALONE
    /* Open the window and prepare the screen */
    iminit(argc, argv, "EMX", g_initheight, g_initwidth);
#endif

    /* Read in any files, execute startup file, etc */
    loadp = NULL;
    title[0] = 0;
    if (argc)
	processargs(argv, title);

    /* Set initial window size and adjust internal structures */
    emxscrinitsize();

    /* Delete default buffer ("?") if any other buffer was created */
    if (g_bheadp->nextp)
	{
	emxdeletebuffer(bp);
	emxforwbuffer();	  /* Change displayed buffer from last to first */
	}

    if (g_modes.automatch)
	(void) emxmatchtable(g_curbp->mode);

    g_lastflag = 0;
    g_initialload = FALSE;	/* End pre-fetch loop */

#ifdef STANDALONE
    /* Set the title */
    if (title[0])
	scrsettitle(title);
#endif
}


/*  Process the command line arguments.	 The command line's syntax is:
	emx [[options] [filenames]]

    -l	The next file named in the list will be loaded and executed as a
	list of commands to emx.  This start file is loaded after all
	files for editing and viewing are processed.
    -r	All files listed following the option will be loaded into readonly
	buffers.  The -e option overrides this option.
    -e	All files listed following the option will be loaded into read/write
	buffers.  The -r option overrides this option.	This is the default
	means of loading files.
    -cd The following directory will be used as the current directroy,
	If EMX is given a filename which isn't fully qualified, EMX
	will search this directory for the file.
    -s	The following string will be the default search string on entry.

    e.g.:  emx -l startfile -r *.h -e *.c

	which reads all include files into buffers for READ only access
	reads in all of my 'C' sources into buffers for read/write access, and
	executes the commands found in the file "startfile"
*/

#if defined(FLG_PROTOTYPE)
static void processargs (char *argv[], char *titlep)
#else
static void processargs (argv, titlep)

char	*argv[];
char	*titlep;
#endif
{
    char	*argp;
    int		startfile;
    int		readwrite;
    int		newcd;
    int		fontset;
    int		linenumber;
    int		searchstring;
    char	filname[512];
    char	directory[MAXDIR];

    /* Init variables to defaults for command line processing */
    readwrite = WRITE;		/* Read/write access */
    startfile = FALSE;		/* "-l" option not seen yet in arg list */
    newcd = FALSE;		/* "-cd" or -"dir' option not seen yet in arg list */
    fontset = FALSE;		/* "-f" option not seen yet in arg list */
    searchstring = FALSE;	/* "-s" option not seen yet in arg list */
    linenumber = 0;

    /* Read the parameters */
    while (*++argv)
	{
	argp = *argv;

	/* "-" or "--" options: read stdin */
	if (strcmp(argp, "--") == 0)
	    g_piping = TRUE;

	if ((strcmp(argp, "-") == 0) || (strcmp(argp, "--") == 0))
	    {
	    if (emxloadfile(g_strstdin, readwrite))
		{
		if ((int) strlen(titlep) < 80)
		    {
		    strcat(titlep, g_strstdin);
		    strcat(titlep, " ");
		    }

		if (linenumber)
		    emxgotolinenumber(linenumber);

		linenumber = 0;
		}

	    }

	/* +nn: line number to edit next file */
	else if (*argp == '+')
	    linenumber = atoi(argp + 1);

	/* "-s" option:	 search string follows */
	else if (strcmp(argp, "-s")==0)
	    searchstring = TRUE;

	/* "-l" option:	 name of start file follows */
	else if (strcmp(argp, "-l")==0)
	    startfile = TRUE;

	/* "-cd" option:  Set new current directory */
	else if (strcmp(argp, "-cd")==0)
	    newcd = TRUE;

	/* "-f" option already processed; ignore next arg */
	else if (strcmp(argp, "-f") == 0 ||
		 strcmp(argp, "-fn") == 0 ||
		 strcmp(argp, "-font") == 0)
	    fontset = TRUE;

	/* "-r" option:	 all succeeding files treated as read only */
	else if (strcmp(argp, "-r")==0)
	    readwrite = READ;

	/* "-e" option:	 all succeeding files treated as read/write (default) */
	else if (strcmp(argp, "-e")==0)
	    readwrite = WRITE;

	/* "-m" option:	 man mode - clobber incoming backspaces */
	else if (strcmp(argp, "-m")==0)
	    g_modes.manstyle = TRUE;

	/* Search string provided */
	else if (searchstring)
	    {
	    strcpy(g_pattern, argp);
	    searchstring = FALSE;
	    }

	else if (startfile)	     /* Filename is for start file */
	    {
	    emxplayfile(argp, TRUE);
	    startfile = FALSE;	    /* Any more file names are for loading
				       unless another "-l" option is found */
	    }

	else if (fontset)    /* Arg is font size - handled by screen init */
	    {
	    /* ignore this arg */
	    fontset = FALSE;
	    }

	else if (newcd)	   /* Arg is new Current directory */
	    {
	    strcpy(directory, argp); /* There is an new current directory */
	    makecurrentdir(directory);
	    newcd = FALSE;     /* Allow properly processing args */
	    }

	else			    /* Filename is for loading into buffer */
	    {
	    strcpy(filname, argp);
	    if (emxloadfile(filname, readwrite))
		{
		if ((int) strlen(titlep) < 80)
		    {
		    strcat(titlep, filname);
		    strcat(titlep, " ");
		    }

		if (linenumber)
		    emxgotolinenumber(linenumber);

		linenumber = 0;
		}
	    }
	}

}


/* Command execution. Look up the binding in the the binding array, and do
   what it says. Return a very bad status if there is no binding, or if the
   symbol has a type that is not usable. Also fiddle with the flags. */

#if defined(FLG_PROTOTYPE)
int emxexecute (int f, int n, KEY *kp)
#else
int emxexecute (f, n, kp)

int f, n;
KEY *kp;
#endif
{
    if (!kp->funcp)
	{
	g_lastflag = 0;
	return ABORT;
	}

    g_thisflag = 0;
    return emxexecfunc(kp->funcp, f, n, kp);
}


/* Symbol execution - do a function associated with a symbol table entry */

static COMPLETION(emxexecfunccomplete)

{
    /* See if buffers are to be flushed */
    if (status != TRUE)
	return g_status;

    return flushall(FALSE, AUTOFLUSH);
}


#if defined(FLG_PROTOTYPE)
int emxexecfunc (FUNCTION *fp, int f, int n, KEY *kp)
#else
int emxexecfunc (fp, f, n, kp)

int f, n;
FUNCTION *fp;
KEY	 *kp;
#endif
{
    if (fp->type & SYMFUNC)
	{
	if (fp->type & SYMMODIFY)
	    {
	    if (g_curbp->flag & BFVIEW)
		{
		 emxmsgprint(g_errreadonly);
		 return ABORT;
		}

	    g_filechanges++;
	    }

	/* make the function call */
	g_status = (*fp->procp)(f, n, kp);
	g_lastflag = g_thisflag;

	if (g_status == TRUE && (fp->type & SYMMODIFY))
	    emxunselectbuf(g_curbp);

	/* check for auto-flush or reminder */
	if (g_savereminder && g_filechanges >= g_savereminder &&
	    !(g_macroinp || g_macrooutp))    /* wait till macro done */
	    {
	    emxhoot();
	    g_filechanges = 0;
	    return emxmsgyesno("Reminder: Write buffers", emxexecfunccomplete);
	    }

	return g_status;
	}

    /* Play this macro */
    if (fp->type & SYMMACRO)
	return emxplaymacro((unsigned long *)(fp->procp), n);

    return ABORT;
}


/* Set the numeric argument to be passed to the next command */

static COMPLETION(emxnumericargcomplete)

{
    KEY	    *keyp;

    keyp = g_keyp;
    if (keyp->modifier)
	goto out;

    g_count++;
    if (keyp->keysym == '-')
	{
	if (g_count > 1)
	    goto out;

	g_flag = 1;
	g_num = 1;
	return emxonekey(emxnumericargcomplete);
	}

    if (keyp->modifier == 0 && keyp->keysym >= '0' && keyp->keysym <= '9')
	{
	if (g_count == 1 ||
	    (g_count == 2 && g_flag))
	    g_num = keyp->keysym - '0';
	else
	    g_num = 10 * g_num + keyp->keysym - '0';

	return emxonekey(emxnumericargcomplete);
	}

out:

    /* Process this key normally */
    requeuekey(keyp);

    if (g_flag)
	g_num = -g_num;

    g_argval = g_argval * g_num;
    g_argflag = TRUE;
    g_thisflag |= LASTARG;
    return TRUE;
}


COMMAND(emxnumericarg)

{
    g_flag = 0;
    g_num = 4;
    g_count = 0;
    g_thisflag |= LASTARG;
    return emxonekey(emxnumericargcomplete);
}


#if defined(FLG_PROTOTYPE)
static int flushall (int terminate, int flag)
#else
static int flushall (terminate, flag)

int	terminate;
int	flag;
#endif
{
    BUFFER  *bp;
    int	    changes;

    changes = FALSE;
    g_filechanges = 0;
    flag |= MANUALFLUSH;

    for (bp = g_bheadp; bp; bp = bp->nextp)
	{
	if (bp->filename[0] == 0)
	    continue;

	if (bp->flag & BFCHG)
	    changes = TRUE;

	if (!emxflushbuffer(bp, flag))
	    {
	    emxusebuf(bp);
	    return FALSE;
	    }

	}

    if (terminate)
	emxquit(FALSE);

    if (!changes)
	emxmsgprint("[No buffers modified]");
    else
	emxok();

    emxbuflistupdate();
    return TRUE;
}


/* Flush all modified buffers */

COMMAND(emxflushbuffers)

{
    return flushall(FALSE, 0);
}


/* Flush all dirty buffers that have file names associated, and exit. */

COMMAND(emxexitflushall)

{
    return flushall(TRUE, 0);
}


/* Quit command. If an argument, always quit.  Otherwise confirm if a buffer
   has been changed and not written out.  Normally bound to "C-X C-C". */

static COMPLETION(quitcomplete)

{
    if (status != TRUE)
	return status;

    if (g_piping)
	emxwritestdout();

#ifdef STANDALONE
    exit(0);
#else
    return emxwidgetquit(0);
#endif
}


#if defined(FLG_PROTOTYPE)
int emxquit(int f)
#else
int emxquit(f)
int f;
#endif
{
    BUFFER  *bp;

    /* An argument or all buffers unmodified forces a quit */
    if (f != FALSE || emxanymodified() == FALSE)
	return quitcomplete(TRUE);

    /* Show the first modified buffer */
    if (!(g_curbp->flag & BFCHG))
	for (bp = g_bheadp; bp; bp = bp->nextp)
	    if (bp->flag & BFCHG)
		{
		emxusebuf(bp);
		break;
		}

    return emxmsgyesno("There are still some modified buffers - Quit", quitcomplete);
}


/* Abort. Hoot the hooter. Kill off any keyboard macro, etc., that is in
   progress. Sometimes called as a routine, to do general aborting of stuff. */

int emxabort (VOID)

{
    emxhoot();
    if (g_macroinp)
	{
	*g_macroinp = 0;
	g_macroinp = NULL;
	g_macroplays = 0;
	}

    return ABORT;
}


/* Set the window size */

static COMPLETION(emxsetsizecomplete)

{
    char    *ptr;
    int	    rows;
    int	    cols;

    if (status != TRUE)
	return status;

    /* Pull out the rows */
    ptr = g_cmdbuf;
    rows = 0;
    cols = 0;
    if (ISNUMBER(*ptr))
	rows = atoi(ptr);

    while (*ptr && *ptr != 'x')
	ptr++;

    if (*ptr)
	cols = atoi(ptr + 1);

    if (!rows)
	rows = g_nrow;

    if (!cols)
	cols = g_ncol;

    if (!g_windowopen)
	{
	g_initheight = rows;
	g_initwidth = cols;
	return TRUE;
	}

    else
	return emxscrsetsize(rows, cols);
}


COMMAND(emxsetsize)

{
    return emxmsgreply("New size (RxC): ", g_cmdbuf, sizeof(g_cmdbuf),
		  emxsetsizecomplete);
}


/* Display the version string */

COMMAND(emxdisplayversion)

{
    emxmsgprint(g_version);
    return TRUE;
}


/* toggle the auto save flag */

static COMPLETION(emxautosavecomplete)

{
    int	    num;

    if (status != TRUE)
	return status;

    num = atoi(g_cmdbuf);
    if (num >= 0)
	g_savereminder = num;

    emxinfoupdate();
    if (g_savereminder == 0)
	emxmsgprint(g_msgreminderoff);

    return TRUE;
}


COMMAND(emxautosave)

{
    if (g_savereminder)
	g_savereminder = 0;
    else
	return emxmsgreply("Changes per update: ", g_cmdbuf, sizeof(g_cmdbuf),
		      emxautosavecomplete);

    emxinfoupdate();
    emxmsgprint(g_msgreminderoff);
    return TRUE;
}


/* Toggle the cursor type */

COMMAND(emxcursortype)

{
    g_modes.cursorstyle = g_modes.cursorstyle ^ 1;
    return TRUE;
}


/* This routine will ask the user for a new current directory path. This path
   will be searched if emx is given an incomplete filename. */

static COMPLETION(emxcdcomplete)

{
    if (status == ABORT)
	return status;

    if (g_filename[0])
	{
	/* Change to original setting */
	if (strcmp(g_filename, "-") == 0)
	    makecurrentdir(g_initialdir);
	else
	    makecurrentdir(g_filename);
	}

    emxmsgprintstr("[Current directory is %s]", g_currentdir);
    return TRUE;
}


COMMAND(emxcd)

{
    return emxmsgreply("New directory: ", g_filename, MAXFILE, emxcdcomplete);
}


/* This routine will set EMX's copy of the current directory to that of the
   shell. This is done when the shell has successfully reset its copy of the
   current directory. */

#if defined(FLG_PROTOTYPE)
static int makecurrentdir (char *newdir)
#else
static int makecurrentdir (newdir)

char	*newdir;
#endif
{
    if (newdir[strlen(newdir) - 1] != '/' && newdir[strlen(newdir) - 1] != '\\')
	strcat(newdir, "/");

    emxexpandname(newdir);
    chdir(newdir);
    strcpy(g_currentdir, newdir);
    return TRUE;
}


/* Print the standard "Not found" message */
#if defined(FLG_PROTOTYPE)
void emxmsgnotfound (const char *str)
#else
void emxmsgnotfound (str)

char	*str;
#endif
{
    emxmsgprintstr("[%s not found]", str);
}


/* Erase the message line */

void emxmsgerase (VOID)

{
    g_promptwp->dotp->used = 0;
    g_promptwp->doto = 0;
    g_promptwp->leftcol = 0;
    g_promptwp->flag |= WFMOVE | WFEDIT;
    g_epresent = FALSE;
}


/* Ask "yes" or "no" question.	Return ABORT if the user answers the question
   with the abort ("^G") character.  Return FALSE for "no" and TRUE for "yes".
   No formatting services are available. */

#if defined(FLG_PROTOTYPE)
static int yesnokeyhandler (KEY *keyp)
#else
static int yesnokeyhandler (keyp)

KEY	*keyp;
#endif
{
    int	    status;

    defaultfixup();

    /* Check for abort */
    g_readinginput = FALSE;
    status = PARTIAL;
    if (keyp->funcp && keyp->funcp->procp == emxabort)
	status = ABORT;

    else if (keyp->modifier == 0)
	{
	if (keyp->keysym == 'y' || keyp->keysym == 'Y' ||
	    keyp->keysym == XK_Return)
	    status = TRUE;

	else if (keyp->keysym == 'n' || keyp->keysym == 'N')
	    status = FALSE;
	}

    /* Not complete yet - wait for more */
    if (status == PARTIAL)
	{
	emxmsgprint(g_cmdbuf);
	g_readinginput = TRUE;
	return status;
	}

    /* Exit the state, and call the function */
    g_curwp = g_oldcurwp;
    g_curbp = g_oldcurbp;
    emxstackpop();
    status = (*g_readfunc)(status);
    return funcfinish(status);
}


#if defined(FLG_PROTOTYPE)
int emxmsgyesno (const char *str, int (*funcp)())
#else
int emxmsgyesno (str, funcp)

const char	*str;
int	(*funcp)();
#endif
{
    /* Move to the prompt window */
    g_oldcurwp = g_curwp;
    g_oldcurbp = g_curbp;
    g_curwp = g_promptwp;
    g_curbp = g_promptbp;

    sprintf(g_cmdbuf, "%s [y/n]: ", str);
    emxmsgprint(g_cmdbuf);

    /* Initialize state variables */
    g_readinginput = TRUE;
    g_readfunc = funcp;

    /* Enter a new state */
    emxstackpush(yesnokeyhandler);
    return TRUE;
}


/* Write out a prompt, and read back a reply. This is always a new message,
   there is no auto completion, and the return is echoed as such. */

#if defined(FLG_PROTOTYPE)
int emxmsgreply (const char *str, char *buf, int nbuf, int (*funcp)())
#else
int emxmsgreply (str, buf, nbuf, funcp)

const char	*str;
char	*buf;
int	nbuf;
int	(*funcp)();
#endif
{
    return emxmsgread(str, buf, nbuf, NEW, funcp);
}


/* Attempt to complete a partial filename by scanning its directory */

#if defined(FLG_PROTOTYPE)
static int filecomplete (char *partname)
#else
static int filecomplete (partname)

char	*partname;
#endif
{
    char	    *partialp;
    char	    *completep;
    int		    partlen;
    char	    found;
    char	    *ptr;
    char	    *matchp;
    DIR		    *dir;
    struct dirent   *direntry;
    char	    directory[256];


    /* Find the terminal portion of the pathname, if there is one */
    if (!(partialp = strrchr(partname, '/')) && !(partialp = strrchr(partname, '\\')))
	{
	/* No '/' - try to complete in the current directory */
	strcpy(directory, ".");
	partialp = partname - 1;
	}

    /* Root directory lookup */
    else if (partialp == partname)
	strcpy(directory, "/");

    /* Lookup in a directory */
    else
	{
	memcpy(directory, partname, partialp - partname);
	directory[partialp - partname] = 0;
	}

    /* Point to the part to compare and the place to start adding */
    partialp++;
    partlen = strlen(partialp);
    completep = partname + strlen(partname);

    /* Open the target directory */
    if ((dir = opendir(directory)) == 0)
	return errno;

    found = FALSE;
    while (TRUE)
	{
	/* Get a filename */
	if (!(direntry = readdir(dir)))
	    break;

	if (strncmp(direntry->d_name, partialp, partlen) == 0)
	    {
	    matchp = direntry->d_name + partlen;

	    /* Found a match. First time, copy the completion. */
	    if (!found)
		{
		strcpy(completep, matchp);
		found = TRUE;
		}

	    /* Each other time, trim down to the common characters */
	    else
		{
		ptr = completep;
		while (*ptr == *matchp++)
		    ptr++;

		/* The entered name is a complete name - quit now */
		if (ptr == completep)
		    {
		    found = FALSE;
		    break;
		    }

		/* Cut the completion off here */
		*ptr = 0;
		}
	    }
	}

    closedir(dir);

    /* Return 0 for nothing, -1 for a match of some kind */
    if (found)
	return -1;

    return 0;
}


/* If the cursor is currently on a word, this routine will put that word
   into a buffer and return its length. Otherwise, 0 will be returned. */

int emxrecallword _ARGS3(WINDOW *, wp, char *, buf, int, max)

{
    char    *textp;
    char    *endp;
    char    *bufp;
    char    *maxp;

    textp = wp->dotp->text;
    endp = textp + wp->dotp->used;
    textp += wp->doto;
    if (textp >= endp)
	return 0;

    bufp = buf;
    maxp = bufp + max;

    if (ISWORD(*textp))
	{
	/* Find the beginning of the word */
	while (textp > wp->dotp->text && ISWORD(*textp))
	    textp--;

	if (!ISWORD(*textp))
	    textp++;

	/* Copy it into the buffer */
	while (textp < endp && ISWORD(*textp))
	    {
	    *bufp++ = *textp++;
	    if (bufp == maxp)
		break;
	    }

	*bufp = 0;
	return bufp - buf;
	}

    return 0;
}


/* Return the number of characters for which the two input strings match,
   up to the given maximum */

#if defined(FLG_PROTOTYPE)
static int strmatch (char *str1, char *str2)
#else
static int strmatch (str1, str2)

char	*str1;
char	*str2;
#endif
{
    int	    match;

    match = 0;
    while ((*str1 == *str2) && *str1 && *str2)
	{
	str1++;
	str2++;
	match++;
	}

    return match;
}


static COMPLETION(readquotecomplete)

{
    unsigned char ch;

    /* If the key is a reasonable quote char, insert it and return */
    if (emxquotedchar(g_keyp, &ch)) {
/*    if (ch <= 0xff) { */
	g_readbuf[g_readpos++] = ch;
	emxmsgputchar(ch);
	return TRUE;
/*    } */

    }

    return FALSE;
}


/* Key handler for the read state */

#if defined(FLG_PROTOTYPE)
static int readkeyhandler (KEY *keyp)
#else
static int readkeyhandler (keyp)

KEY	*keyp;
#endif
{
    register FUNCTION *sp1;
    register BUFFER *bp;
    register char   *matchp;
    register int    i;

    register int    c;
    register int    nhits;
    register int    nxtra;
    register int    bxtra;
    int		    status;

    char	    buf[256];

    /* Trap auto-completion first */
    if ((keyp->keysym == ' ' || keyp->keysym == XK_Linefeed) &&
	keyp->modifier == 0 && (g_readflag & AUTO))
	{
	g_readbuf[g_readpos] = 0;
	nhits = 0;
	nxtra = 1000;
	if (g_readflag & BUFCMPL)
	    for (bp = g_bheadp; bp; bp = bp->nextp)
		{
		i = strmatch(g_readbuf, bp->bufname);
		if (i == g_readpos)
		    {
		    if (nhits == 0)
			matchp = bp->bufname + i;

		    nhits++;
		    bxtra = strmatch(bp->bufname + g_readpos, matchp);
		    if (bxtra < nxtra)
			nxtra = bxtra;
		    }

		}

	/* File name matching */
	else if (g_readflag & FILECMPL)
	    {
	    strcpy(buf, g_readbuf);
	    emxexpandname(buf);
	    if (filecomplete(buf) < 0)
		{
		emxbackdel(FALSE, g_readpos);
		g_readpos = 0;
		strcpy(g_readbuf, buf);
		g_readpos = strlen(buf);
		emxmsgputstr(g_readpos, buf);
		}

	    goto more;
	    }

	/* Command name matching */
	else
	    for (sp1 = g_flistp; sp1; sp1 = sp1->nextp)
		{
		i = strmatch(g_readbuf, sp1->namep);
		if (i == g_readpos)
		    {
		    if (nhits == 0)
			matchp = sp1->namep + i;

		    nhits++;
		    bxtra = strmatch(sp1->namep + g_readpos, matchp);
		    if (bxtra < nxtra)
			nxtra = bxtra;
		    }

		}

	if (nhits == 0)			/* No completion */
	    goto more;

	nxtra = nxtra < (g_readmax - 1 - g_readpos) ?
		nxtra : (g_readmax - 1 - g_readpos);

	for (i = 0; i < nxtra; i++)
	    {
	    c = *matchp++;
	    g_readbuf[g_readpos++] = c;
	    emxmsgputchar(c);
	    }

	if (nhits != 1)		/* If there is one choice, use it */
	    goto more;

	keyp = &g_keycurrent;
	keyp->keysym = XK_Return;
	keyp->modifier = 0;
	keyp->funcp = NULL;
	}

    /* Process the key */
    if ((keyp->modifier == 0 && keyp->keysym <= 0xff) ||
	(keyp->modifier == KEscapeMask && keyp->keysym == ' '))
	{
inschar:
	if (g_readpos < g_readmax - 1)
	    {
	    g_readbuf[g_readpos++] = keyp->keysym;
	    if (!g_initialload)
		emxmsgputchar(keyp->keysym);
	    }

	/* Not a valid character. Hoot unless initializing. */
	else if (!g_initialload)
	    emxhoot();
	}

    else if (keyp->keysym == XK_Tab)
	{
	keyp = &g_keycurrent;
	keyp->keysym = '\t';
	goto inschar;
	}

    else if (keyp->funcp &&
	(keyp->funcp->procp == emxcasesense ||
	 keyp->funcp->procp == emxendmacro ||
	 keyp->funcp->procp == emxexitflushall))
	{
	(*keyp->funcp->procp)();
	}

    /* Handle quoted control characters */
    else if (keyp->funcp &&
	     keyp->funcp->procp == emxquote)
	{
	return emxonekey(readquotecomplete);
	}

    else if (keyp->keysym == XK_F8)
	{
	/* Recall the current word */
	if (emxrecallword(g_oldcurwp, buf, g_readmax - g_readpos))
	    {
	    nxtra = strlen(buf);
	    memcpy(&g_readbuf[g_readpos], buf, nxtra);
	    g_readpos += nxtra;
	    emxmsgputstr(nxtra, buf);
	    }

	else
	    emxhoot();
	}

    /* Recall the path of the current filename */
    else if (keyp->keysym == XK_F1)
	{
        matchp = (char *) strrchr(g_oldcurbp->filename, '/');
        if (!matchp)
            matchp = (char *)strrchr(g_oldcurbp->filename, '\\');
	if (matchp) {
	    nxtra = matchp - g_oldcurbp->filename;
	    memcpy(&g_readbuf[g_readpos], g_oldcurbp->filename, nxtra);
	    g_readpos += nxtra;
	    emxmsgputstr(nxtra, g_oldcurbp->filename);
	}
	else
	    emxhoot();
	}

    /* Recall the last entered string */
    else if (keyp->keysym == XK_F7)
	{
	nxtra = strlen(g_savebuf);
	memcpy(&g_readbuf[g_readpos], g_savebuf, nxtra);
	g_readpos += nxtra;
	emxmsgputstr(nxtra, g_savebuf);
	}

    else if (keyp->keysym == XK_Return)
	{
	g_readbuf[g_readpos] = 0;
	status = TRUE;
	goto done;
	}

    else if (keyp->funcp && keyp->funcp->procp == emxabort)
	{
	emxabort();
	status = ABORT;
	goto done;
	}

    else if (keyp->funcp && keyp->funcp->procp == emxbackdel)
	{
	if (g_readpos)
	    {
	    emxbackdel(FALSE, 1);
	    g_readpos--;
	    }

	}

more:

    /* Not complete yet - wait for more */
    return PARTIAL;


done:

    /* Exit the state, and call the function */
    g_curwp = g_oldcurwp;
    g_curbp = g_oldcurbp;
    g_promptwp->leftcol = 0;  /* Realign to the left */
    g_promptwp->doto = 0;
    g_promptwp->flag |= WFHARD;
    g_readinginput = FALSE;
    emxstackpop();

    if (status == TRUE && g_readbuf[0] == 0)
	status = FALSE;

    /* Save a copy */
    nxtra = g_readpos + 1 > MAXSAVE ? MAXSAVE - 1 : g_readpos;
    if (nxtra)
	{
	strncpy(g_savebuf, g_readbuf, nxtra);
	g_savebuf[nxtra] = 0;
	}

    status = (*g_readfunc)(status);
    return funcfinish(status);
}


/* This is the general "read input from the message line" routine. The basic idea
   is that the prompt string "prompt" is written to the message line, and a one
   line reply is read back into the supplied "buf" (with maximum length "len").
   The "flag" contains NEW (a new prompt), and AUTO (autocomplete). */

#if defined(FLG_PROTOTYPE)
int emxmsgread (const char *str, char *buf, int nbuf, int flag, int (*funcp)())
#else
int emxmsgread (str, buf, nbuf, flag, funcp)

const char	*str;
char	*buf;
int	nbuf;
int	flag;
int	(*funcp)();
#endif
{
    /* Move to the prompt window */
    g_oldcurwp = g_curwp;
    g_oldcurbp = g_curbp;
    g_curwp = g_promptwp;
    g_curbp = g_promptbp;

    /* Show the prompt, if any */
    if ((flag & NEW))
	{
	emxmsgerase();
	g_epresent = TRUE;
	}
    else
	emxmsgputchar(' ');

    emxmsgput(str);

    /* Erase the rest of the prompt line */
    g_promptwp->dotp->used = g_promptwp->doto;
    g_promptwp->flag |= WFEDIT;

    /* Initialize state variables */
    g_readinginput = TRUE;
    g_readpos = 0;
    g_readbuf = buf;
    g_readmax = nbuf;
    g_readflag = flag;
    g_readfunc = funcp;

    /* Enter a new state */
    emxstackpush(readkeyhandler);
    return PARTIAL;
}


/* Special "printf" for the message line. Each call to "emxmsgprint" starts a new
   line in the message window area, and ends with an erase to end of the
   message line. The formatting is done by a call to the standard formatting
   routine. */

#if defined(FLG_PROTOTYPE)
void emxmsgprint (const char *fp)
#else
void emxmsgprint (fp)

const char	*fp;
#endif
{
    char  buf[4096];

    if (g_readinginput)
	return;

    if (!g_windowopen)
	{
	sprintf(buf, fp);
	strncpy(g_initialmessage, buf, MAXCOL);
	return;
	}

    emxmsgerase();
    emxmsgput(fp);
    g_epresent = TRUE;
}

#if defined(FLG_PROTOTYPE)
void emxmsgprintstr (const char *fp, const char *arg)
#else
void emxmsgprintstr (fp, arg)

const char	*fp;
const char    *arg;
#endif
{
    char  buf[4096];

    if (g_readinginput)
	return;

    if (!g_windowopen)
	{
	sprintf(buf, fp, arg);
	strncpy(g_initialmessage, buf, MAXCOL);
	return;
	}

    emxmsgerase();
    emxmsgadd(fp, (long)arg);
    g_epresent = TRUE;
}

#if defined(FLG_PROTOTYPE)
void emxmsgprintint (const char *fp, long arg)
#else
void emxmsgprintint (fp, arg)

const char	*fp;
long	 arg;
#endif
{
    char  buf[4096];

    if (g_readinginput)
	return;

    if (!g_windowopen)
	{
	sprintf(buf, fp, arg);
	strncpy(g_initialmessage, buf, MAXCOL);
	return;
	}

    emxmsgerase();
    emxmsgadd(fp, arg);
    g_epresent = TRUE;
}

/* Printf style formatting. This is called by both "emxmsgprint" and "emxmsgreply",
   to provide formatting services to their clients. The move to the start of
   the message line, and the erase to the end of the line, is done by the
   caller. */

#if defined(FLG_PROTOTYPE)
void emxmsgput (register const char *fp)
#else
void emxmsgput (fp)

register const char	*fp;
#endif
{
    register char   c;
    char	    *p2;
    int		    spaces;
    char	    newbuf[MAXCOL];
    WINDOW	    *wp;


    if (g_initialload)
	return;

    /* Allow the NULL string */
    if (*fp == 0)
	return;

    wp = g_promptwp;
    p2 = newbuf;
    while (*fp)
	{
	c = *fp++;
	if (c == '\t')
	    {
	    spaces = 7 - ((wp->doto + p2 - newbuf) % 8);
	    while (spaces--)
		*p2++ = ' ';

	    c = ' ';
	    }

	else if (ISCTRL(c))
	    {
	    *p2++ = '^';
	    c ^= 0x40;
	    }

	*p2++ = c;
	}

    emxmsgputstr(p2 - newbuf, newbuf);
}


#if defined(FLG_PROTOTYPE)
void emxmsgadd (register const char *fp, register long arg)
#else
void emxmsgadd (fp, arg)

register const char	*fp;
register long	arg;
#endif
{
    char	    buf[MAXCOL];

    /* Allow the NULL string */
    if (*fp == 0)
	return;

    sprintf(buf, fp, arg);
    emxmsgput(buf);
}


/* Write a character to the message line */

#if defined(FLG_PROTOTYPE)
static void emxmsgputchar (register int c)
#else
static void emxmsgputchar (c)

register int	c;
#endif
{
    WINDOW  *wp;
    BUFFER  *bp;

    wp = g_curwp;
    bp = g_curbp;
    g_curwp = g_promptwp;
    g_curbp = g_promptbp;
    emxlinsert(1, c, NULL);
    g_curwp = wp;
    g_curbp = bp;
}


/* Write a character to the message line */

#if defined(FLG_PROTOTYPE)
static void emxmsgputstr (int count, char *str)
#else
static void emxmsgputstr (count, str)

int	count;
char	*str;
#endif
{
    WINDOW  *wp;
    BUFFER  *bp;

    wp = g_curwp;
    bp = g_curbp;
    g_curwp = g_promptwp;
    g_curbp = g_promptbp;
    emxlinsert(count, EMXSTR, str);
    g_curwp = wp;
    g_curbp = bp;
}


/* Shouldn't the next two routines be in emxsymbol.c? */

void emxprompton (VOID)

{
    g_oldcurwp = g_curwp;
    g_oldcurbp = g_curbp;
    g_curwp = g_promptwp;
    g_curbp = g_promptbp;
}


void emxpromptoff (VOID)

{
    g_curwp = g_oldcurwp;
    g_curbp = g_oldcurbp;
}



/* Which window has the pointer? */

static WINDOW *getwindow _ARGS2(int *, rowp, MOUSE *, mouse)

{
    WINDOW *wp;
    int    r = *rowp;

    for (wp = g_wheadp->nextp; wp; wp = wp->nextp)
	if (wp->toprow <= r && r <= wp->toprow + wp->rows)
	    {
	    if (r == wp->toprow + wp->rows)
		mouse->flag |= MFINFO;	/* infoline */

	    *rowp = r - wp->toprow;
	    return wp;
	    }

    /* case of 1 window and mouse below infoline - force it! */
    mouse->wp = g_curwp;
    mouse->dotp = g_curwp->dotp;
    mouse->doto = g_curwp->doto;
    mouse->flag = MFBADPT;
    return (WINDOW *) NULL;
}


/* Figure out where a mouse event took place, and load a MOUSE struct */

static void evalpointer _ARGS3(int, x, int, y, MOUSE *, mouse)

{
    int   i;
    int	  row, col;
    LINE  *lp, *endlp;

    /* see if mouse is inside window */
    mouse->flag = 0;
    if (x >= 0 && x <= g_windowwidth && y >= 0 && y <= g_windowheight)
	{
	/* Find pointer window */
	row = (y - VMARGIN) / g_charheight;
	if (!(mouse->wp = getwindow(&row, mouse)))
	    return;

	/* Adjust column offset by window scroll amount */
	col = (x - HMARGIN) / g_charwidth + mouse->wp->leftcol;

	/* Find text line with pointer */
	endlp = mouse->wp->bufp->linep; /* 1st line in buffer */
	lp = mouse->wp->linep;		/* 1st line in window */
	for (i = 0; i < row; i++)
	    {
	    lp = lp->nextp;
	    if (lp == endlp)	/* buffer wrap (ie mouse out of text bounds) */
		break;
	    }

	/* set up mouse position */
	mouse->dotp = lp;
	if (mouse->flag & MFINFO)
	    mouse->doto = col;
	else
	    {
	    mouse->doto = emxcursoroffset(lp, col);
	    if (col > emxcursorcol(lp->text, mouse->doto))
		mouse->doto++;
	    }

	}

    /* or else use cursor row & col */
    else
	{
	mouse->wp = g_curwp;
	mouse->dotp = g_curwp->dotp;
	mouse->doto = g_curwp->doto;
	mouse->flag = MFBADPT;
	}

}


/* move dot to mouse */

#if defined(FLG_PROTOTYPE)
static void mousemoveto (MOUSE *mouse)
#else
static void mousemoveto (mouse)

MOUSE *mouse;
#endif
{
    emxselectwind(mouse->wp);
    g_curwp->dotp = mouse->dotp;
    g_curwp->doto = mouse->doto;
    g_curwp->flag |= WFMOVE;
}


/* Infoline "icons" - columns hardcoded */

#if defined(FLG_PROTOTYPE)
static int mouseicon (MOUSE *mouse)
#else
static int mouseicon (mouse)

MOUSE *mouse;
#endif
{
    emxselectwind(mouse->wp);
    if (mouse->doto >= MOUSE_ICON_START && mouse->doto <= MOUSE_ICON_START+2)
	return emxforwbuffer();
    else if (mouse->doto >= MOUSE_ICON_START+3 &&
	     mouse->doto <= MOUSE_ICON_START+5)
	return emxonlywindow();
    else if (mouse->doto >= MOUSE_ICON_START+6 &&
	     mouse->doto <= MOUSE_ICON_START+8)
	return emxsplitwindow();

    return FALSE;
}


#if defined(FLG_PROTOTYPE)
static int mouseinword (LINE *lp, int offset)
#else
static int mouseinword (lp, offset)

LINE	*lp;
int	offset;
#endif
{
    if (offset >= (int) lp->used)
	return FALSE;

    if (ISWORD(lp->text[offset]))
	return TRUE;

    return FALSE;
}


#if defined(FLG_PROTOTYPE)
static int mouseforword (LINE *lp, int offset)
#else
static int mouseforword (lp, offset)

LINE	*lp;
int	offset;
#endif
{
    /* Move to the end of the non-word or the word */
    if (mouseinword(lp, offset))
	{
	do
	    {
	    if (offset >= (int) lp->used)
		break;

	    offset++;
	    }
	    while (mouseinword(lp, offset));

	}

    else
	{
	do
	    {
	    if (offset >= (int) lp->used)
		break;

	    offset++;
	    }
	    while (!mouseinword(lp, offset));
	}

    return offset;
}


#if defined(FLG_PROTOTYPE)
static int mousebackword (LINE *lp, int offset)
#else
static int mousebackword (lp, offset)

LINE	*lp;
int	offset;
#endif
{
    /* Move to the start of the non-word or the word */
    if (mouseinword(lp, offset))
	{
	do
	    {
	    if (offset == 0)
		return offset;

	    offset--;
	    }
	    while (mouseinword(lp, offset));

	}

    else
	{
	do
	    {
	    if (offset == 0)
		return offset;

	    offset--;
	    }
	    while (!mouseinword(lp, offset));
	}

    offset++;
    return offset;
}


/* Select the range specified by the mouse endpoints, using the current
   selection mode */

static void mouseselect (VOID)

{
    REGION  region;

    /* Build a region for the two mouse endpoints */
    emxsetregion(&region, g_mouse1.wp->bufp->linep,
		g_mouse1.dotp, g_mouse1.doto,
		g_mouse2.dotp, g_mouse2.doto);

    /* Move the endpoints according to the selection mode */
    if (g_selectmode == SELECTCHAR)
	;

    else if (g_selectmode == SELECTWORD)
	{
	/* Move point 1 backword, and point 2 forword */
	region.firsto = mousebackword(region.firstlp, region.firsto);
	region.lasto = mouseforword(region.lastlp, region.lasto);
	emxsetregion(&region, g_mouse1.wp->bufp->linep,
		    region.firstlp, region.firsto,
		    region.lastlp, region.lasto);
	}

    else
	{
	/* Select the entire line */
	region.firsto = 0;
	region.lasto = region.lastlp->used + 1;
	emxsetregion(&region, g_mouse1.wp->bufp->linep,
		    region.firstlp, region.firsto,
		    region.lastlp, region.lasto);
	}

    /* Set the mark and cursor to include the region */
    g_mouse1.wp->markp = region.firstlp;
    g_mouse1.wp->marko = region.firsto;
    g_mouse1.wp->dotp = region.lastlp;
    if (((g_mouse1.wp->doto = region.lasto) == region.lastlp->used + 1) &&
	g_mouse1.wp->dotp != g_mouse1.wp->bufp->linep)
	{
	g_mouse1.wp->dotp = g_mouse1.wp->dotp->nextp;
	g_mouse1.wp->doto = 0;
	}

    g_mouse1.wp->flag |= WFMOVE;

    /* Select from mouse 1 to mouse 2 */
    emxquickunselectall();
    emxselectrange(g_mouse1.wp->bufp, &region);
}


/* Specify a click location for moving or starting a selection */

COMMAND(emxpressselect)

{
    emxunselectall();
#ifndef STANDALONE
    emxfocusin();
#endif
    evalpointer(g_event->xbutton.x, g_event->xbutton.y, &g_mouse1);
    if (!(g_mouse1.flag & MFBADPT))
	{
	g_mousedo = TRUE;
	g_mouse2 = g_mouse1;

	/* Check for multiple clicking */
	if (g_event->xbutton.time - g_lastclicktime > 200)
	    g_selectmode = SELECTCHAR;
	else
	    {
	    g_selectmode++;
	    mouseselect();
	    }

	g_lastclicktime = g_event->xbutton.time;
	return TRUE;
	}

    return FALSE;
}


/* Paste the primary selection */

COMMAND(emxpresspaste)

{
    /* Paste primary selection */
    emxpasteprimary(FALSE, 1, &g_keynull);
    return TRUE;
}


/* Handle a continuing drag selection */

COMMAND(emxdragbutton)

{

    MOUSE   mousetemp;

    /* if mousemove events out of sync with buttons, ignore */
    if (!g_mousedo)
	return FALSE;

    /* Turn off multi-click */
    g_lastclicktime = 0;

    evalpointer(g_event->xmotion.x, g_event->xmotion.y, &mousetemp);

    /* landed in special buffer	 - no dragging */
    if (g_mouse1.wp->bufp == g_blistp)
	{
	g_mouse2 = g_mouse1;
	return TRUE;
	}

    if (mousetemp.flag & MFBADPT ||
	(g_mouse1.flag & MFINFO && !(mousetemp.flag & MFINFO)))
	{
	g_mousedo = FALSE;
	return FALSE;
	}

    if (g_mouse1.wp != mousetemp.wp || mousetemp.flag & MFINFO)
	return FALSE;

    g_mouse2 = mousetemp;
    mouseselect();
    return TRUE;
}


/* Handle the release of the selection button */

COMMAND(emxreleaseselect)

{
    if (!g_mousedo)
	return FALSE;

    g_mousedo = FALSE;
    evalpointer(g_event->xbutton.x, g_event->xbutton.y, &g_mouse2);

    /* Can't switch windows */
    if (g_mouse1.wp != g_mouse2.wp)
	return FALSE;

    if (g_mouse1.flag & MFINFO)
	return mouseicon(&g_mouse1);

    if (g_mouse2.flag & MFINFO)
	return FALSE;

    if (g_mouse1.dotp == g_mouse2.dotp && g_mouse1.doto == g_mouse2.doto &&
	g_selectmode == SELECTCHAR)
	{
	mousemoveto(&g_mouse1);
	return TRUE;
	}

    /* set mark */
    if (g_mouse1.wp->bufp == g_blistp)
	return TRUE;

    mouseselect();
    emxmsgprint("[Mouse region marked]");
    return TRUE;
}


/* Standard post-command processing */
#if defined(FLG_PROTOTYPE)
static int funcfinish (int status)
#else
static int funcfinish (status)
int	status;
#endif
{
    /* Reset the numeric arg if none supplied */
    if ((g_thisflag & LASTARG) == 0)
	{
	g_argflag = FALSE;
	g_argval = 1;
	}

    return status;
}


/* Standard things to do when the user takes explicit action of some kind */

static void defaultfixup (VOID)

{
    /* Clear out any delimeter-matching elipsis line */
    if (g_elipsis)
	emxfixelipsis();

    /* Erase any messages */
    if (g_epresent)
	emxmsgerase();

    /* Lowlight any highlighting */
    if (g_highlit)
	emximlowlight();
}


/* Default handler for key events */

#if defined(FLG_PROTOTYPE)
static void defaultkeyhandler (KEY *keyp)
#else
static void defaultkeyhandler (keyp)

KEY	*keyp;
#endif
{
    int	    status;

    defaultfixup();

    /* Execute the key - this may change the handler stack */
    status = emxexecute(g_argflag, g_argval, keyp);

    /* Standard completion */
    funcfinish(status);
}


/* Put a key back to be fetched next time.  This is used by emxnumericarg to put
   back the first key it sees which isn't a numeric argument key. */

#if defined(FLG_PROTOTYPE)
static void requeuekey (KEY *keyp)
#else
static void requeuekey (keyp)

KEY *keyp;
#endif
{
    g_ungottenkeyp = keyp;
}


void emxkeyloop (VOID)

{
    KEY	    *keyp;
    int	    len;

   /* Loop here in case the executed command creates a keystroke buffer */
    while (TRUE)
	{
	if (g_keyp)
	    {
	    /* Save the macro character if need be */
	    if (g_macroinp)
		{
		/* Macro's getting a might long */
		if ((len = g_macroinp - g_macro) > 0 && (len % MAXMACRO) == 0)
		    {
		    emxhoot();
		    g_macro = (unsigned long *) realloc((char *)g_macro,
			(len + MAXMACRO) * sizeof(unsigned long));
		    g_macroinp = g_macro + len;
		    }

		g_savemacroinp = g_macroinp;
		*g_macroinp++ = g_keyp->keysym;
		*g_macroinp++ = g_keyp->modifier;
		}

	    /* Get the binding for the key, if any */
	    g_keyp = emxgetbinding(g_keyp);

	    /* Let the handler have at it */
	    (*g_stackp)(g_keyp);
	    }

	/* Forget everything that just happened */
	g_keycurrent.keysym = 0;
	g_keycurrent.modifier = 0;
	g_keycurrent.funcp = NULL;

	/* Now see if there are some artificial keys to process */
	while (TRUE)
	    {
	    /* Check for a key returned by 'emxquote' */
	    if (keyp = g_ungottenkeyp)
		g_ungottenkeyp = NULL;

	    /* Check for a playing macro */
	    if (!keyp && g_macrooutp)
		{
		keyp = &g_keycurrent;
		keyp->modifier = 0;
		if (keyp->keysym = *g_macrooutp)
		    {
		    g_macrooutp++;
		    keyp->modifier = *g_macrooutp++;
		    }

		/* Restart it? */
		else if (--g_macroplays)
		    {
		    g_macrooutp = g_macroplay;
		    keyp->keysym = *g_macrooutp++;
		    keyp->modifier = *g_macrooutp++;
		    }

		else
		    {
		    keyp = NULL;
		    g_macrooutp = NULL;
		    }
		}

	    /* Check for executing a keystroke buffer */
	    if (!keyp && g_executing)
		keyp = emxgetbufkey();

	    /* Nothing - wait for something else */
	    if (!keyp)
		return;


	    /* See if this is an interim key */
	    if (keyp->keysym == XK_Escape &&
		(g_modifier & KEscapeMask) == 0)
		g_modifier |= KEscapeMask;

	    else if (keyp->keysym == 'x' &&
		     keyp->modifier == ControlMask &&
		     (g_modifier & KCtlxMask) == 0)
		g_modifier |= KCtlxMask;

	    else
		break;
	    }

	/* Got a key - use it */
	g_keyp = &g_keycurrent;
	g_keyp->keysym = keyp->keysym;
	g_keyp->modifier = keyp->modifier | g_modifier;
	g_keyp->funcp = NULL;
	g_modifier = 0;
	}

}


#if defined(FLG_PROTOTYPE)
static void keyprocess (XEvent *event)
#else
static void keyprocess (event)

XEvent	*event;
#endif
{
    int	    modifier;
    KeySym  keysym;
    char    keyname[32];

    /* Do standard pre-processing that everybody wants */
    modifier = event->xkey.state;
    XLookupString((XKeyEvent *) event, keyname, sizeof(keyname), &keysym,
			&g_compose);

    /* ignore modifier keys alone */
    if (keysym >= XK_Shift_L && keysym <= XK_Hyper_R)
	return;

    /* Ignore the Caps Lock modifier */
    modifier &= (~LockMask);

    /* Alphabetic characters don't care about shifted state */
    if (keysym <= 0xff)
	{
	modifier &= (~ShiftMask);

	/* And are case-independent when modified */
	if (modifier && ISUPPER(keysym))
	    keysym = TOLOWER(keysym);
	}

    /* Watch for synthetic modifiers */
    if (keysym == XK_Escape && (g_modifier & KEscapeMask) == 0)
	{
	g_modifier |= KEscapeMask;
	return;
	}

    else if (keysym == 'x' && modifier == ControlMask &&
	     (g_modifier & KCtlxMask) == 0)
	{
	g_modifier |= KCtlxMask;
	return;
	}

    /* A real keystroke has been entered */
    g_keyp = &g_keycurrent;
    g_keyp->keysym = keysym;
    g_keyp->modifier = modifier | g_modifier;
    g_keyp->funcp = NULL;
    g_modifier = 0;

    emxkeyloop();
}


static void buttonprocess _ARGS1(XButtonEvent *, event)

{
    KeySym  keysym;

    /* Make up an artificial keysym for the event */
    if (event->type == ButtonPress)
	keysym = KButton + event->button;

    else if (event->type == ButtonRelease)
	keysym = KButtonUp + event->button;

    else if (event->type == MotionNotify)
	keysym = KMotion;

    else
	return;

    /* A "keystroke" has been entered */
    g_keyp = &g_keycurrent;
    g_keyp->keysym = keysym;

    /* Ignore the Caps Lock modifier */
    g_keyp->modifier = (event->state | g_modifier) & (~LockMask);
    g_keyp->funcp = NULL;
    g_modifier = 0;

    emxkeyloop();
}


/* Default handler for window events */
#if defined(FLG_PROTOTYPE)
static void windowprocess (XEvent *event)
#else
static void windowprocess (event)

XEvent	*event;
#endif
{
    if (event->type == Expose)
	{
	if (!g_windowopen)
	    {
	    /* first display of window */
	    g_windowopen = TRUE;
#ifndef STANDALONE
	    g_mainwin = XtWindow(emx);
#endif

	    if (g_initialmessage[0])
		emxmsgprint(g_initialmessage);
	    else
		emxmsgprint("For help about EMX, press ESC, then ?");
	    }

	else
	    emxscrexpose(event->xexpose.x, event->xexpose.y,
			event->xexpose.width, event->xexpose.height);
	}

    else if (event->type == ConfigureNotify)
	emxscrresize(event->xconfigure.width, event->xconfigure.height, TRUE);

    else if (event->type == VisibilityNotify)
	{
#ifdef STANDALONE
	if (event->xvisibility.state == VisibilityUnobscured)
#else
	if (event->xvisibility.state != VisibilityFullyObscured)
#endif
	    g_winvisible = TRUE;
	else
	    g_winvisible = FALSE;
	}

}


/* Set up the default event handlers and the stack */

static void eventinit (VOID)

{
    g_stackcount = 1;
    g_stackp = (FUNC *) g_stack;
    *g_stackp = (FUNC) defaultkeyhandler;
}


/* All events come through here first */

#if defined(FLG_PROTOTYPE)
void emxeventhandle (XAnyEvent *event)
#else
void emxeventhandle (event)

XAnyEvent   *event;
#endif
{
    BUFFER *bp;
    char *valuep;
    unsigned long len;
    XSelectionRequestEvent *req;
    XEvent response;

    emxswitchcursor(OFF);
    g_event = (XEvent *) event;

    /* Figure out what kind of event it is and dispatch to the handler */
    if (event->type == KeyPress)
	keyprocess(g_event);

    else if (event->type == ButtonPress ||
	     event->type == ButtonRelease ||
	     event->type == MotionNotify)
	buttonprocess((XButtonEvent *)event);

    else if (event->type == Expose ||
	     event->type == ConfigureNotify ||
	     event->type == VisibilityNotify)
	windowprocess(g_event);
#ifdef STANDALONE
    else if (event->type == SelectionRequest) {
      if (!clipboardDefined) {
        XA_CLIPBOARD = XInternAtom(g_display, "CLIPBOARD", 0);
        clipboardDefined = 1;
      }
      /*printf("processing SelectionRequest\n");*/
      req = (XSelectionRequestEvent *)(event);
      response.xselection.property = None;
      response.xselection.type = SelectionNotify;
      response.xselection.display = req->display;
      response.xselection.requestor = req->requestor;
      response.xselection.selection = req->selection;
      response.xselection.target = req->target;
      response.xselection.time = req->time;
      if (req->target == XA_STRING) {
        /*printf("selection XA_STRING request received\n");*/
	bp = g_curbp;
	if (!bp->selectp) {
	  for (bp=g_bheadp; bp; bp=bp->nextp) {
	    if (bp->selectp) {
	      break;
	    }
	  }
	}
        if (bp != 0  &&  !bp->selectp) {
          bp = 0;
        }
	
        /*
        if (bp) {
          printf("selection is in buffer %s\n", bp->bufname);
        }
        */
        
        if (!bp) {
          emxmsgprint("no selection available for clipboard");
        }
        else if (!emxgetselectstring(bp, &valuep)) {
          emxmsgprint("unable to get selected string for clipboard");
        }
        else {
	  len = strlen(valuep);
          /*printf("selected text is %d bytes\n", len);*/
	  if (req->property == 0) {
	    req->property = XA_PRIMARY;
	  }
          /*printf("copying to clipboard '%s'\n", valuep);*/
          /*printf("setting property in %d from %d\n", req->requestor, g_mainwin);*/
	  XChangeProperty(g_display, req->requestor, req->property,
	    XA_STRING, 8, PropModeReplace, (unsigned char *)valuep, len);
          /* put the selection in the clipboard, too, if it's not there already */
          if (req->property != XA_CLIPBOARD) {
//	    XChangeProperty(g_display, req->requestor, XA_CLIPBOARD,
//	      XA_STRING, 8, PropModeReplace, (unsigned char *)valuep, len);
            XStoreBuffer(g_display, (char *)valuep, (int)len, 0);
          }
	  response.xselection.property=req->property;
	}
      }
      XSendEvent(g_display, req->requestor, 0, 0, &response);
      XFlush(g_display);
    }
#endif
#if 0
    switch (event->type) {
      case ButtonPress:
        printf("event->type=ButtonPress\n");
        break;
      case ButtonRelease:
        printf("event->type=ButtonRelease\n");
        break;
      case MotionNotify:
        printf("event->type=MotionNotify\n");
        break;
      case Expose:
        printf("event->type=Expose\n");
        break;
      case VisibilityNotify:
        printf("event->type=VisibilityNotify\n");
        break;
      default:
        printf("event->type=%d\n", event->type);
        break;
    }
#endif

    emxupdate();
    emxswitchcursor(ON);
}


#if defined(FLG_PROTOTYPE)
void emxstackpush (FUNC keyfunc)
#else
void emxstackpush (keyfunc)

FUNC  keyfunc;
#endif

{
    if (g_stackcount == STACKMAX)
	{
	puts("Event stack overflow");
	emxabort();
	return;
	}

    g_stackcount++;
    g_stackp++;
    *g_stackp = keyfunc;
}


/* Remove a set of event handlers */

void emxstackpop (VOID)

{
    if (g_stackcount == 1)
	{
	puts("Event stack underflow");
	emxabort();
	return;
	}

    /* The next event will be handled by somebody else */
    g_stackcount--;
    g_stackp--;
}
