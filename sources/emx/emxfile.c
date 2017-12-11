#define EMXFILE_C TRUE
/*=========================================================================
 * Copyright (C) Servio Corp. 1991-1994.  All Rights Reserved.
 *
 * Name -
 *
 * Purpose -
 *
 * $Id: emxfile.c,v 30.8 1994/09/22 19:35:16 marcs Exp $
 *
 *========================================================================*/

/*
    EMX - File commands

*/

#include "emx.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/file.h>
#if !defined(GEODE) || !defined(FLG_SEQUENT_UNIX)
#include <unistd.h>
#endif

#if !defined(GEODE)
#include <pwd.h>

#ifdef NO_GZIP
#define HostOpenFile(name,mode,err,userfile) fopen(name,mode)
#define HOST_FCLOSE(f) fclose(f)
#define EmxGets(buff,count) fgets(buff,count,g_fileid)
#define EmxWrite(buff,start,end) fwrite(buff,start,end,g_fileid)
#define EmxPutc(c) fputc(c,g_fileid)
#else
#include <zlib.h>
#define HostOpenFile(name,mode,err,userfile) myfopen(name,mode)
#define HOST_FCLOSE(f) myfclose(f)
#define EmxGets(buff,count) mygets(buff,count)
#define EmxWrite(buff,start,end) mywrite(buff,start,end)
#define EmxPutc(c) myputc(c)
#endif

#define HostRemoveSingleFile(name,userfile,err) unlink(name)
#define HostErrSType int;
#else
#include <host.hf>
#endif


static int insfile (VOID);

static int makebackup _DEF2(char *, filename, BUFFER *, bp);

static int writeout _DEF3(BUFFER *, bp, const char *, namep, int, append);


#ifndef NO_GZIP
static int myfopen(char *name, char *mode) {
  int len = strlen(name);
  if (len > 3
      &&  name[len-3] == '.' && name[len-2] == 'g' && name[len-1] == 'z') {
    g_fileid = gzopen(name,mode);
    g_isGzipped = 1;
    return (g_fileid != 0);
  }
  else {
    g_plainFileId = fopen(name,mode);
    g_isGzipped = 0;
    return (g_plainFileId != 0);
  }
}

static int myfclose() {
  if (g_isGzipped == 1) {
    return gzclose(g_fileid);
  }
  else {
    return fclose(g_plainFileId);
  }
}

static char *mygets(char *buff, int count) {
  if (g_isGzipped == 1) {
    return gzgets(g_fileid,buff,count);
  }
  else {
    return fgets(buff, count, g_plainFileId);
  }
}

static int mywrite(char *buff, int start, int count) {
  if (g_isGzipped == 1) {
    return gzwrite(g_fileid,&buff[start-1],count);
  }
  else {
    return fwrite(buff, start, count, g_plainFileId);
  }
}

static int myputc(char c) {
  if (g_isGzipped == 1) {
    return gzputc(g_fileid,c);
  }
  else {
    return fputc(c,g_plainFileId);
  }
}


#endif


int emxexpandname _ARGS1(char *, origs)

{
    char dstr[512];	       /* temporary buffer for output */
    char *s = origs;	   /* pointer into source text */
    char *d = dstr;	       /* pointer into destination buffer */
    char *ss;		   /* sub-string pointer (past shell variable) */
    char rs;		   /* temporary variable during getenv() */
    char *envs;		   /* pointer to returned environment string */
    int mFlg = 0;      /* flag showing need to update user's buffer */
    struct passwd *pass;

    /*
    This routine searches for shell variable within the string provided
    and replaces the variables with their contents.
    */

    /* look for the shell variables */
    while (ss = strchr(s, '$'))
	{
	while (s < ss)
	    *d++ = *s++;	/* copy the previous characters */

	while (*++ss && !strchr("/ %=>\n<&;|/$", *ss))
	    ;			/* find the delimiter */

	rs = *ss;
	*ss = '\0';
	envs = getenv(s+1);
	*ss = rs;/* get value */
	if (envs)
	    {
	    mFlg = 1;	    /* show there was a shell variable */
	    while((*d++ = *envs++) != '\0')
		;		/* copy in the value of the variable */

	    s = ss; d--;	/* adjust src and dest pointers */
	    }

	else
	    {
	    while(s < ss)
		*d++ = *s++;	/* copy from shell character to delimiter */
	    }

	}

    while((*d++ = *s++) != '\0')
	;			/* copy any remaining text */

    if (mFlg)
       strcpy(origs, dstr);    /* made a change, copy in the new string */

    /* Now check for a HOME directory lookup */
    if (origs[0] == '~') {
	dstr[0] = 0;
	s = origs + 1;
	if (*s == '/' || *s == '\\')
	    {
	    /* Use the HOME directory */
	    if (d = getenv("HOME"))
		strcpy(dstr, d);
	    }

	/* Pick up the string before the next slash */
	else if ((d = strchr(s, '/')) != 0 || (d = strchr(s, '\\')) != 0) {
	    rs = *d;
	    *d = 0;
	    pass = (struct passwd *) getpwnam(s);
	    *d = rs;
	    if (pass) {
		strcpy(dstr, pass->pw_dir);
		s = d;
	    }
	}

	strcat(dstr, s);
	strcpy(origs, dstr);
    }

    return TRUE;
}


/* This routine will hoot and print out an error message */

static int honk _ARGS2(int, msgnum, const char *, msgstring)

{
    emxhoot();
    emxmsgprint(msgstring);
    return msgnum;
}


/* This function will copy a file, always replacing the original */

static int copyfile _ARGS2(char *, srcp, char *, destp)

{
    int	    fd1;
    int	    fd2;
    int	    count;
    int	    cnt;
    char    buf[2048];

    if ((fd1 = open(srcp, O_RDONLY)) < 0)
	return 0;

    count = -1;
    if ((fd2 = open(destp, O_RDWR | O_CREAT | O_TRUNC, 0x1ff)) >= 0)
	{
	while ((count = read(fd1, buf, sizeof(buf))) > 0)
	    {
	    cnt = count;
	    if ((count = write(fd2, buf, count)) != cnt)
		break;
	    }


	}

    close(fd1);
    if (fd2 > 0)
	close(fd2);

    if (count >= 0)
	return 1;
    else
	return 0;
}


/* This routine will copy a file, returning backup status */

static int dobackup _ARGS2(char *, srcp, char *, destp)

{
    char    buf[256];

    sprintf(buf, "[Backing up %s to %s]", srcp, destp);
    emxmsgprint(buf);
    if (!copyfile(srcp, destp))
	{
	if (errno == ENOENT)
	    return NEW_FILE;
	else
	    return NOT_DONE;
	}

    return DONE;
}


/* Open a file for reading - check that it is a file and its access */

int emxffropen _ARGS2(const char *, namep, int *, accessp)

{
    struct stat info;
    int		s;
    char	buf[256];
#if defined(GEODE)
    HostErrSType theErr;
#endif

    /* Open file for shared read access */
    s = FIOSUC;
    if (strcmp(namep, g_strstdin) == 0) {
#ifdef NO_GZIP
	g_fileid = stdin;
#else
        /* gzip on stdin not supported - could use gzdopen(fileno(stdin), "r") */
        g_plainFileId = stdin;
        g_isGzipped = 0;
#endif
    }
    else
	{
	/* stat the name to see if it is exists and is a file */
	if ((s = stat(namep, &info)) < 0)
	    {
	    /* No such file */
	    if (errno == ENOENT)
		return FIOFNF;

	    emxmsgprintstr("[Cannot stat %s]", namep);
	    return FIOERR;
	    }

	if (!(info.st_mode & (S_IFREG | S_IFIFO)))
	    {
	    emxmsgprintstr("[%s is not a file]", namep);
	    return FIOERR;
	    }

	/* access the file to see if it is read-only */
	if (access(namep, W_OK) < 0)
	    {
	    if (errno == EACCES || errno == EROFS)
		*accessp = READ;
	    else
		{
		emxmsgprintstr("[Cannot access %s]", namep);
		return FIOERR;
		}

	    }

	if ((HostOpenFile(namep, "r", &theErr, TRUE)) == 0)
	    {
	    if (errno == ENOENT)
		s = FIOFNF;
	    else
		s = FIOERR;

	    emxhoot();
	    sprintf(buf, "[Can't open %s: %x]", namep, errno);
	    emxmsgprint(buf);
	    }

	}

    return s;
}


/* Write a line to the already opened file. The "buf" points to the buffer,
   and the "nbuf" is its length, less the free newline.	 Return the status.
   Check only at the newline. */

static int ffputline _ARGS3(char *, buf, int, nbuf, int, ispc)

{
    /* Put all the characters in the line into the file */
    if (EmxWrite(buf, 1, nbuf) == nbuf) {
	if (ispc)
	    EmxPutc(CR);

	if (EmxPutc(LF) == LF)
	    return FIOSUC;
	}

    return honk(FIOERR, "[Write I/O error]");
}


/* Read a line from a file, and store the bytes in the supplied buffer. Stop on
   end of file or end of line. Don't get upset by files that don't have an end
   of line on the last line. Delete any CR followed by an LF.
 */

/* Simpler is better */

static int ffgetline _ARGS4(char *, buf, int, nbuf, int *, lengthp,
			    int *, ispc)

{
    int		  ret;
    int		  pc;
    unsigned int  len;

    ret = FIOSUC;
    pc = FALSE;
    buf[0] = 0;
    if (!EmxGets(buf, nbuf))
	{
	len = strlen(buf);
	if (len == nbuf - 1 && buf[nbuf - 1] != LF)
	    return honk(FIOERR, "[File has long line]");

	/* End of file - all is well */
	if (len == 0)
	    ret = FIOEOF;
	else
	    return honk(FIOERR, "[I/O error reading file]");
	}

    else
	{
	if ((len = strlen(buf)) <= 0)
	    {
	    honk(FIOERR, "[File has null line]");
	    len = 1;
	    }

	if (ispc && *ispc && (len == 1) && (buf[0] == CTLZ)) {
	    len = 0;
	    ret = FIOEOF;
	    }

	if (len > 0 && buf[len - 1] == LF)
	    len--;

	if (len > 0 && buf[len - 1] == 13) {
	    pc = TRUE;
	    len--;
	    }
	}

    /* Terminate the line and return its length */
    buf[len] = 0;
    *lengthp = len;
    if (pc && ispc)
	*ispc = pc;

    return ret;
}


/* This routine turns the auto-backup option on and off.		    */

COMMAND(emxautoback)

{
    if (g_modes.autobackup ^= 1)
	emxmsgprint("[Auto backup on]");
    else
	emxmsgprint("[Auto backup off]");

    /* Update infolines */
    emxinfoupdate();
    return TRUE;
}


/* This routine will toggle the file backup (to emx#back) on and off */

COMMAND(emxdefaultback)

{
    if (g_modes.defaultback ^= 1)
	emxmsgprint("[Default backups on]");
    else
	emxmsgprint("[Default backups off]");

    /* Update infolines */
    emxinfoupdate();
    return TRUE;
}


/* Load a file into the current buffer */

int emxloadfile _ARGS2(char *, namep, int, theAccess)

{
    BUFFER  *bp;
    BUFFER  *oldbp;
    WINDOW  *wp;
    WINDOW  *cwp;
    LINE    *lp;
    int	    i;
    int	    unique;

    /* Expand the name */
    if (namep == NULL)
	namep = (char *) g_strstdin;
    else
	emxexpandname(namep);

    cwp = g_curwp;
    oldbp = g_curbp;
    for (bp = g_bheadp; bp; bp = bp->nextp)
	{
	if (strcmp(bp->filename, namep) == 0)
	    {
	    if (theAccess == READ) /* trying to do view-mode */
		{
		emxmsgprint("[Buffer exists]");
		return FALSE;
		}

	    if (--oldbp->windows == 0)
		{
		oldbp->dotp  = cwp->dotp;
		oldbp->doto  = cwp->doto;
		oldbp->markp = cwp->markp;
		oldbp->marko = cwp->marko;
		}

	    g_curbp = bp;
	    cwp->bufp = bp;
	    if (g_modes.automatch)
		(void) emxmatchtable(bp->mode);

	    /* First window on this buffer? */
	    if (bp->windows++ == 0)
		{
		cwp->dotp  = bp->dotp;
		cwp->doto  = bp->doto;
		cwp->markp = bp->markp;
		cwp->marko = bp->marko;
		}

	    /* No - copy the position from another buffer */
	    else
		{
		for (wp = g_wheadp->nextp; wp; wp = wp->nextp)
		    {
		    if (wp->bufp == bp && wp != cwp)
			{
			cwp->dotp  = wp->dotp;
			cwp->doto  = wp->doto;
			cwp->markp = wp->markp;
			cwp->marko = wp->marko;
			break;
			}

		    }

		}

	    lp = cwp->dotp;
	    i = cwp->rows/2;
	    while (i-- && lp->prevp != bp->linep)
		lp = lp->prevp;

	    cwp->linep = lp;
	    cwp->flag |= WFINFO|WFHARD;
	    emxmsgprint("[Old buffer]");
	    return TRUE;
	    }

	}

#ifndef STANDALONE
    /* Check for a file read callback */
    if (*namep == '=' && g_readcallback)
	{
	EmxFileCallback	  rcb;

	rcb.status = FALSE;
	rcb.filename = namep;
	rcb.bufname = NULL;
	XtCallCallbacks((Widget) emx, XtNreadCallback, &rcb);

	/* If the callback ate the read, quit here */
	if (rcb.status)
	    return TRUE;
	}
#endif

    /* Make sure the file can be opened */
    if ((g_filestatus = emxffropen(namep, &theAccess)) == FIOERR)
	return FALSE;

    g_access = theAccess;
    g_filenamep = namep;

    /* Make a unique buffer name */
    emxmakebufname(g_bufname, namep);
    unique = 1;
    i = strlen(g_bufname);
    while (bp = emxbfind(g_bufname, FALSE))
	sprintf(&g_bufname[i], "#%d", unique++);

    /* Create the buffer */
    if ((bp = emxbfind(g_bufname, TRUE)) == NULL)
	return FALSE;

    /* Taking over the current window */
    oldbp = g_curbp;
    if (--oldbp->windows == 0)
	{
	oldbp->dotp = g_curwp->dotp;
	oldbp->doto = g_curwp->doto;
	oldbp->markp = g_curwp->markp;
	oldbp->marko = g_curwp->marko;
	}

    g_curbp = bp;
    g_curwp->bufp = bp;
    bp->windows++;

    /* Read in the file */
    return emxreadin(g_filenamep, g_access, g_filestatus == FIOSUC);
}


/* Read the open file into the current buffer. Make all of the text in the
   buffer go away, after checking for unsaved changes. This is called by
   the "read" command, the "visit" command, and the mainline (for "emx
   file"). Return a standard status. Print a summary (lines read, error
   message) out as well. if flag == TRUE, there is a file */

static COMPLETION(emxreadincomplete)

{
    LINE   *lp1;
    LINE   *lp2;
    int    i;
    WINDOW *wp;
    BUFFER *bp;
    int    s;
    int	   nbytes;
    int	   ispc;
    int    nline;
    char   *line;

    if (status != TRUE)
	{
	HOST_FCLOSE();
	return status;
	}

    /* Clear out the current contents of the buffer */
    bp = g_curbp;
    emxbclear(bp);
    bp->flag &= ~BFCHG;
    bp->flag &= ~BFVIEW;
    ispc = FALSE;

    if (strcmp(g_filenamep, g_strstdin) == 0)
	bp->filename[0] = 0;
    else if (bp->filename != g_filenamep)
        strcpy(bp->filename, g_filenamep);

    s = FIOSUC;

    /* Not found */
    if (!g_flag)
	{
	emxmsgprint("[New file]");
	goto out;
	}

    line = (char *) malloc(MAXLINE);
    nline = 0;
    ispc = (bp->flag & BFISPC) != 0;
    while ((s = ffgetline(line, MAXLINE, &nbytes, &ispc)) == FIOSUC)
	{
	/* Check for special filtering */
	if (g_modes.manstyle)
	    {
	    for (i = 0; i < nbytes; i++)
		{
		/* Watch for backspaces */
		if (line[i] == '' && i > 0)
		    {
		    if (nbytes - i > 1)
			memcpy(&line[i - 1], &line[i + 1], nbytes - i - 1);

		    i -= 2;
		    nbytes -= 2;
		    }

		}

	    }

	if ((lp1=emxlinit(nbytes)) == NULL)
	    {
	    s = FIOERR;		    /* Keep message on the  */
	    break;		    /* display.		    */
	    }

	lp2 = bp->linep->prevp;
	lp2->nextp = lp1;
	lp1->nextp = bp->linep;
	lp1->prevp = lp2;
	bp->linep->prevp = lp1;
	memcpy(lp1->text, line, nbytes);
	++nline;
	}

    FREE(line);
    HOST_FCLOSE();		 /* Ignore errors.	 */

    if (s == FIOEOF && !g_initialload)
	{
	if (nline == 1)
	    emxmsgprint("[Read 1 line]");
	else
	    emxmsgprintint("[Read %d lines]", nline);
	}

out:
    for (wp = g_wheadp->nextp; wp != NULL; wp = wp->nextp)
	{
	if (wp->bufp == bp)
	    {
	    wp->linep = bp->linep->nextp;
	    wp->dotp  = bp->linep->nextp;
	    wp->doto  = 0;
	    wp->markp = NULL;
	    wp->marko = 0;
	    wp->flag |= WFINFO|WFHARD;
	    }

	}

    /* Set read-only flag */
    if (g_access != WRITE)
	bp->flag |= BFVIEW;

    /* Set the PC flag */
    if (ispc)
	bp->flag |= BFISPC;

    bp->flag |= BFNL;		/* files need this code ! */
    bp->flag |= BFBACK;		/* Indicate that an edit session has started
				   and that we should put a copy of the current
				   file into $EMXBACK. */

    emxbuflistupdate();

    if (s == FIOERR)
	return FALSE;

    return TRUE;
}


int emxreadin _ARGS3(const char *, namep, int, acc, int, flag)

{
    g_filenamep = namep;
    g_access = acc;
    g_flag = flag;
    if (g_curbp->flag & BFCHG)
	return emxmsgyesno(g_msgdiscard, emxreadincomplete);

    return emxreadincomplete(TRUE);
}


/* Read a file into the current buffer.	 This is really easy; all you do is
   find the name of the file, and call the standard "read a file into the
   current buffer" code. */

static COMPLETION(emxfilereadcomplete)

{
    int	    theAccess;

    if (status != TRUE)
	return status;

    emxexpandname(g_filename);
    theAccess = WRITE;
    if ((status = emxffropen(g_filename, &theAccess)) == FIOERR)
	return FALSE;

    return emxreadin(g_filename, theAccess, status == FIOSUC);
}


COMMAND(emxfileread)

{
    return emxmsgread("Read file: ", g_filename, MAXFILE, NEW|AUTO|FILECMPL,
		    emxfilereadcomplete);
}


/* Insert a file into the current buffer.  This is real easy; all you do is
   find the name of the file, and call the standard "insert a file into the
   current buffer" code.  Usually bound to "C-X C-I". */

static COMPLETION(emxinsertfilecomplete)

{
    int	    theAccess;

    if (status != TRUE)
	return status;

    emxexpandname(g_filename);
    theAccess = READ;
    if ((status = emxffropen(g_filename, &theAccess)) == FIOERR)
	return FALSE;

    if (status == FIOFNF)
	{
	emxmsgnotfound(g_filename);
	return FALSE;
	}

    return insfile();
}


COMMAND(emxinsertfile)

{
    return emxmsgread("Insert file: ", g_filename, MAXFILE, NEW|AUTO|FILECMPL,
		    emxinsertfilecomplete);
}


/* Like emxfilevisit, though it will try to just go to the window containing
   that file, if possible, or visit the file in a pop-up window. */

static COMPLETION(emxsplitvisitcomplete)

{
    WINDOW  *wp;

    if (status != TRUE)
	return status;

    emxexpandname(g_filename);
    for (wp = g_wheadp->nextp; wp != NULL; wp = wp->nextp)
	if (strcmp(g_filename, wp->bufp->filename) == 0)
	    break;

    if (wp)
	{
	emxselectwind(wp);
	return TRUE;
	}

    /* Not found - load it in */
    emxselectwind(emxwpopup());
    return emxloadfile(g_filename, WRITE);
}


COMMAND(emxsplitvisit)

{
    return emxmsgread(g_msgvisitfile, g_filename, MAXFILE, NEW|AUTO|FILECMPL,
		    emxsplitvisitcomplete);
}


/* Select a file for editing.  Look around to see if you can find the file in
   another buffer; if you can find it just switch to the buffer.  If you
   cannot find the file, create a new buffer, read in the text, and switch to
   the new buffer. */

static COMPLETION(emxfilevisitcomplete)

{
    if (status != TRUE)
	return status;

    return emxloadfile(g_filename, WRITE);
}


COMMAND(emxfilevisit)

{
    /* Prompt for the name */
    return emxmsgread(g_msgvisitfile, g_filename, MAXFILE, NEW|AUTO|FILECMPL,
		    emxfilevisitcomplete);
}


/* like file visit, but file is read only */

static COMPLETION(emxviewfilecomplete)

{
    if (status != TRUE)
	return status;

    return emxloadfile(g_filename, READ);
}


COMMAND(emxviewfile)

{
    return emxmsgread("View file: ", g_filename, MAXFILE, NEW|AUTO|FILECMPL,
		    emxviewfilecomplete);
}


/* Ask for a file name, and write the contents of the current buffer to that
   file. Update the remembered file name and clear the buffer changed flag. */

static COMPLETION(emxfilewritecomplete)

{
    if (status != TRUE)
	return status;

    emxexpandname(g_filename);
    if (status = makebackup(g_filename, g_curbp))
	{
	if ((status = writeout(g_curbp, g_filename, FALSE)) == TRUE)
	    {
	    emxbuflistupdate();
	    strcpy(g_curbp->filename, g_filename);
	    g_curbp->flag &= ~BFCHG;
	    emxbupdate(g_curbp, WFINFO);		    /* Update infolines */
	    return TRUE;
	    }

	}

    return status;
}


/* Ask for a file name, and write the contents of the current buffer to that
   file. Update the remembered file name and clear the buffer changed flag. */

COMMAND(emxfilewrite)

{
    return emxmsgread("Write to file: ", g_filename, MAXFILE, NEW|AUTO|FILECMPL,
		    emxfilewritecomplete);
}


/* Ask for a file name, and append the contents of the current buffer to that
   file. No change to the buffer's status */

static COMPLETION(emxfileappendcomplete)

{
    if (status != TRUE)
	return status;

    emxexpandname(g_filename);
    return writeout(g_curbp, g_filename, TRUE);
}


COMMAND(emxfileappend)

{
    return emxmsgread("Append to file: ", g_filename, MAXFILE, NEW|AUTO|FILECMPL,
		    emxfileappendcomplete);
}


/* Reload a file, not bothering to write it out if modified */

COMMAND(emxfilereload)

{
    int theAccess;
    int s;

    theAccess = (g_curbp->flag & BFVIEW) ? READ : WRITE;
    if ((s = emxffropen(g_curbp->filename, &theAccess)) == FIOERR)
	return FALSE;

    return emxreadin(g_curbp->filename, theAccess, s == FIOSUC);
}


/* Use the last node of a filename to create a buffer name. Limit the length to
   MAXBUF-4 to leave room for a uniquifier. */

void emxmakebufname _ARGS2(char *, bname, const char *, namep)

{
    /*const*/ char   *cp1;
    /*const*/ char   *cp2;

    /* Skip over leading blanks */
    cp1 = namep;
    while (*cp1 && *cp1 == ' ')
	++cp1;

    if (!(cp2 = (char *) strrchr(cp1, '/'))) {
        if (!(cp2 = (char *) strrchr(cp1, '\\')))
            cp2 = cp1;
        else
            cp2++;
    }
    else
	cp2++;

    strncpy(bname, cp2, MAXBUF - 4);
    bname[MAXBUF - 4] = 0;
}


int emxpipeline _ARGS1(BUFFER *, bp)

{
    if ((strcmp(bp->bufname, g_strstdin) == 0) && g_piping)
	return TRUE;

    return FALSE;
}


/* Save the contents of the specified buffer back into its associated file.
   Error if there is no remembered file name.  If this is the first write
   since the read or visit, then a backup copy of the file is made. */

int emxflushbuffer _ARGS2(BUFFER *, bp, int, flag)

{
    int	    s;

    /* Do nothing if not changed */
    if ((bp->flag & BFCHG) == 0)
	{
	/* Don't print message if flushing buffers */
	if (!flag)
	    emxmsgprint("[Buffer has not been modified]");

	return TRUE;
	}

    /* Must have a name or be a special */
    if (!g_modes.allowbufwrites && bp->filename[0] == 0)
	{
	emxmsgprint("[No file name!]");
	return FALSE;
	}

#if 0
    /* Don't write out <stdin> if g_piping through */
    if (emxpipeline(bp))
	return TRUE;
#endif

    /* Don't bother with backups if passing through a special */
    s = TRUE;
    if (!bp->filename[0] || makebackup(bp->filename, bp))
	{
	if ((s = writeout(bp, bp->filename, FALSE)) == TRUE)
	    {
	    bp->flag &= ~BFCHG;
	    emxbuflistupdate();
	    emxbupdate(bp, WFINFO);
	    }

	}

    /* Turn on backups for this buffer again, unless asked not to */
    if (!(flag & AUTOFLUSH))
	bp->flag |= BFBACK;

    return s;
}


/* Save the contents of the current buffer back into its associated file.
   Error if there is no remembered file name.  If this is the first write
   since the read or visit, then a backup copy of the file is made. */

COMMAND0(emxfilesave)

{
    return emxflushbuffer(g_curbp, 0);
}


/* This routine will do the work of writing out the file. */

static int writeout _ARGS3(BUFFER *, bp, const char *, namep, int, append)

{
    int    s;
    int    ispc;
    LINE   *lp;
    int    nline;
    char   *ptr;
    unsigned int   len;
#if defined(GEODE)
    HostErrSType    theErr;
#endif

#ifndef STANDALONE
    /* Check for a file write callback */
    if ((!namep || (*namep == 0) || (*namep == '=')) &&
	g_writecallback)
	{
	EmxFileCallback	  wcb;

	wcb.status = FALSE;
	if (!namep)
	    namep = "";

	wcb.filename = (char *) namep;
	wcb.bufname = bp->bufname;
	XtCallCallbacks((Widget) emx, XtNwriteCallback, &wcb);

	/* If the callback ate the read, quit here */
	if (wcb.status)
	    {
	    /* Return success only if no longer marked modified */
	    return (bp->flag & BFCHG) ? FALSE : TRUE;
	    }

	}
#endif

    /* Open file for exclusive read/write access */
    s = FIOSUC;
    if (!(HostOpenFile(namep, (append ? "a" : "w"), &theErr, TRUE)))
	{
	emxhoot();
	emxmsgprintstr("[Cannot open %s for writing]", namep);
	return FALSE;
	}

    emxmsgprintstr("[Writing %s]", namep);
    lp = bp->linep->nextp;		  /* First line.	  */
    nline = 0;				    /* Number of lines.	    */
    ispc = (bp->flag & BFISPC) != 0;

    /* Write out all the lines */
    while (lp != bp->linep)
	{
	/* Scan for trailing spaces and fail to write them */
	if ((len = lp->used) && ISSPACE(lp->text[len - 1]) && g_modes.trimoutput)
	    {
	    ptr = &lp->text[len - 1];
	    while (ptr > lp->text && ISSPACE(*(--ptr)))
		;

	    if (ISSPACE(*ptr))
		len = 0;
	    else
		len = ptr - lp->text + 1;

	    lp->used = len;
	    }

	if ((s = ffputline(lp->text, len, ispc)) != FIOSUC)
	    break;

	++nline;
	lp = lp->nextp;
	}

    if (s == FIOSUC)
	{
        /* bjs 9/23/02 - I don't want ctl-z at the end of my files
	if (ispc)
	    fputc(CTLZ, g_fileid);
        */

	s = HOST_FCLOSE();
	if (s == FIOSUC)
	    {
	    if (nline == 1)
		emxmsgprint("[Wrote 1 line]");
	    else
		emxmsgprintint("[Wrote %d lines]", nline);
	    }

	}

    /* Ignore close result if a write error occurred */
    else
	HOST_FCLOSE();

    if (s != FIOSUC)
	return FALSE;

    return TRUE;
}


/* This function will write the <stdin> buffer to stdout */

int emxwritestdout (VOID)

{
    int		  ispc;
    BUFFER	  *bp;
    LINE	  *lp;
    char	  *ptr;
    unsigned int  len;

    /* Find the buffer containing what was once <stdin> */
    if (!(bp = emxbfind(g_strstdin, FALSE)))
	return FALSE;

#ifdef NO_GZIP
    g_fileid = stdout;
#else
    g_isGzipped = 0;
    g_plainFileId = stdout;
#endif
    lp = bp->linep->nextp;
    ispc = (bp->flag & BFISPC) != 0;

    /* Write out all the lines */
    while (lp != bp->linep)
	{
	/* Scan for trailing spaces and fail to write them */
	if ((len = lp->used) && ISSPACE(lp->text[len - 1]) && g_modes.trimoutput)
	    {
	    ptr = &lp->text[len - 1];
	    while (ptr > lp->text && ISSPACE(*(--ptr)))
		;

	    if (ISSPACE(*ptr))
		len = 0;
	    else
		len = ptr - lp->text + 1;

	    lp->used = len;
	    }

	if (ffputline(lp->text, len, ispc) != FIOSUC)
	    break;

	lp = lp->nextp;
	}

    HOST_FCLOSE();
    return TRUE;
}


/* This routine will write out a backup copy of the source file. The backup
   will go to either '$EMXBACK' or 'emx#back', depending on the situation. The
   only real special case here is when the file is new and we can't backup the
   file because there nothing to backup. We let this just fall through and
   detect it at the end. */

static int makebackup _ARGS2(char *, filename, BUFFER *, bp)

{
    char    *ptr;
    char    backname[MAXFILE + 1];
    char    extraname[MAXFILE + 1];
    int	    back_result;
    int	    backstatus;
    char    *p;

    backname[0] = 0;
    backstatus = NOT_DONE;
    back_result = BACK_OFF;

    /* We will backup to $EMXBACK only if the option is set (autobackup)*/
    /* and it's time (BFBACK = on). */
    if (g_modes.autobackup && (bp->flag & BFBACK))
	{
	strcpy(backname, "$EMXBACK/");
	ptr = backname + strlen(backname);
	emxmakebufname(ptr, filename);
	emxexpandname(backname);

	/* '$EMXBACK' not defined */
	if (backname[0] == '$')
	    back_result = BACK_BADFILE;
	else
	    {
	    /* src and backup files will be same, don't do backup */
	    if (strcmp(filename, backname) == 0)
		back_result = BACK_IDENTICAL;
	    else
		{
		/* Things look good, it's okay to attempt the backup */
		if (backstatus = dobackup(filename, backname))
		    back_result = backstatus;
		else
		    bp->flag &= ~BFBACK;
		}
	    }
	}

    /* Do the backup to 'emx#back' only if we didn't already success-	*/
    /* fully backup the file (backstatus non-zero) and the user has  */
    /* turned the option on (g_modes.defaultback).			 */

    if (backstatus && g_modes.defaultback)
	{
	strcpy(extraname, filename);
	if (!(p = strrchr(extraname, '/')))
	    p = extraname;
	else if (!(p = strrchr(extraname, '\\')))
            p = extraname;
        else
	    p++;

	strcpy(p, "emx#back");
	backstatus = dobackup(filename, extraname);

	/* If the backup failed because this is a new file, then we want */
	/* to put this file name in the status message.			 */
	if (!g_modes.autobackup)
	    back_result = backstatus;

	if (back_result == BACK_NEW)
	    strcpy(backname, filename);

	if (back_result)
	    emxmsgprintstr(g_msg_badbackup[back_result - 1], backname);

	if (backstatus)
	    {
	    emxhoot();
	    emxmsgadd("No backup written", (long)NULL);

	    /* No backup because the file is new isn't an error */
	    if (backstatus != NEW_FILE)
		return FALSE;
	    }

	}

    return TRUE;
}


/* The command allows the user to modify the file name associated with the
   current buffer.  It is like the "f" command in UNIX "ed".  The operation is
   simple; just zap the name in the BUFFER structure, and mark the windows as
   needing an update.  You can type a blank line at the prompt if you wish. */

static COMPLETION(emxfilenamecomplete)

{
    if (status != TRUE)
	return status;

    emxexpandname(g_filename);
    g_curbp->flag |= BFCHG;
    g_curbp->flag &= ~BFSAV;	/* No longer a save buffer, if ever */
    strcpy(g_curbp->filename, g_filename);
    emxbupdate(g_curbp, WFINFO);
    return TRUE;
}


COMMAND(emxfilename)

{
    return emxmsgread("New filename: ", g_filename, MAXFILE, NEW|AUTO|FILECMPL,
		    emxfilenamecomplete);
}


/* Insert file "fname" into the current buffer.	 Called by insert file
   command.  Return the final status of the read. */

static int insfile (VOID)

{
    LINE   *lp0;
    LINE   *lp1;
    LINE   *lp2;
    BUFFER *bp;
    WINDOW *cwp;
    int    s;
    int		nbytes;
    int		ispc;
    int    nline;

    char	*line;

    bp = g_curbp;

    /* back up a line and save the mark here */
    cwp = g_curwp;
    cwp->dotp = cwp->dotp->prevp;
    cwp->doto = 0;
    cwp->markp = cwp->dotp;
    cwp->marko = 0;

    line = (char *) malloc(MAXLINE);
    nline = 0;
    ispc = (bp->flag & BFISPC) != 0;
    while ((s = ffgetline(line, MAXLINE, &nbytes, &ispc)) == FIOSUC)
	{
	if ((lp1=emxlinit(nbytes)) == NULL)
	    {
	    s = FIOERR;		    /* Keep message on the  */
		break;			/* display.		*/
	    }

	lp0 = cwp->dotp;    /* line previous to insert */
	lp2 = lp0->nextp;	 /* line after insert */

	/* re-link new line between lp0 and lp2 */
	lp2->prevp = lp1;
	lp0->nextp = lp1;
	lp1->prevp = lp0;
	lp1->nextp = lp2;

	/* and advance and write out the current line */
	cwp->dotp = lp1;
	memcpy(lp1->text, line, nbytes);
	++nline;
	}

    FREE(line);
    HOST_FCLOSE();				 /* Ignore errors.	 */
    cwp->markp = cwp->markp->nextp;
    if (s == FIOEOF)
	{		       /* Don't zap message!   */
	if (nline == 1)
	    emxmsgprint("[Inserted 1 line]");
	else
	    emxmsgprintint("[Inserted %d lines]", nline);
	}

    /* advance to the next line */
    cwp->dotp = cwp->dotp->nextp;

    /* copy window parameters back to the buffer structure */
    bp->dotp = cwp->dotp;
    bp->doto = cwp->doto;
    bp->markp = cwp->markp;
    bp->marko = cwp->marko;

    /* update windows */
    emxbchange(bp, WFHARD);

    if (s == FIOERR)
	return FALSE;

    return TRUE;
}


int emxinsertafile _ARGS1(char *, namep)

{
#if defined(GEODE)
    HostErrSType theErr;
#endif

    /* Open the file */
    if (HostOpenFile(namep, "r", &theErr, TRUE))
	return insfile();

    return FALSE;
}


/* Read in the help text */

COMMAND(emxhelp)

{
    int		status;
    char	ln[256];

    int		x;
    char	name[MAXFILE];
    int		opened;
    char	*path;
    char	*ptr;
#if defined(GEODE)
    HostErrSType theErr;
#endif

    /* Make sure the help buffer isn't already displayed */
    if (g_blistp->windows && strcmp(g_blistp->bufname, g_strhelp) == 0)
	return FALSE;

    /* Clear out the buffer list or whatever */
    emxbclear(g_blistp);

    /* Open the help file. check path for possible locations. */
    opened = FALSE;
    name[0] = 0;
    path = (char *) getenv("PATH");
    while (path)
	{
	if (ptr = strchr(path, ':'))
	    {
	    strncpy(name, path, ptr - path);
	    name[ptr - path] = 0;
	    path = ptr + 1;
	    }
	else
	    {
	    strcpy(name, path);
	    path = NULL;
	    }

	if (name[0])
	    strcat(name, "/");

	strcat(name, "emxhelp");

	if ((HostOpenFile(name, "r", &theErr, TRUE)) != 0)
	    {
	    opened = TRUE;
	    break;
	    }
	}

    if (!opened)
	return honk(FALSE, "[Can't find help file 'emxhelp' in your PATH]");

    strcpy(g_blistp->bufname, g_strhelp);
    while ((status = ffgetline(ln, sizeof(ln), &x, NULL)) == FIOSUC)
	emxaddline(g_blistp, ln);

    HOST_FCLOSE();
    if (status != FIOEOF)
	emxmsgprint("[Warning - help text incomplete]");

    return emxpopblist(TRUE);
}


/* Delete the current buffer and the file associated with it */

static COMPLETION(emxfiledeletecomplete)

{
#if defined(GEODE)
    HostErrSType theErr;
#endif

    if (status != TRUE)
	return status;

    /* Blast the file first */
    if (HostRemoveSingleFile(g_curbp->filename, TRUE, &theErr) != 0)
	{
	emxmsgprint("[Could not delete file]");
	return FALSE;
	}

    /* Clear the modified flag and clobber the buffer */
    g_curbp->flag &= ~BFCHG;
    return emxdeletecurbuf();
}


COMMAND(emxfiledelete)

{
    char    buf[256];

    /* Confirm */
    sprintf(buf, "Delete file %s?", g_curbp->filename);
    return emxmsgyesno(buf, emxfiledeletecomplete);
}
