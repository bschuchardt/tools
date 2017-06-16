#define EMXDISPLAY_C TRUE
/*=========================================================================
 * Copyright (C) Servio Corp. 1991-1994.  All Rights Reserved.
 *
 * Name -
 *
 * Purpose -
 *
 * $Id: emxdisplay.c,v 30.8 1994/09/14 00:31:48 marcs Exp $
 *
 *========================================================================*/

/*
    EMX - Display

    The functions in this file handle redisplay. The redisplay system knows
    almost nothing about the editing process; the editing functions do,
    however, set some hints to eliminate a lot of the grinding.
*/

#include "emx.h"

#ifdef STANDALONE
#include "emxicon.h"
#endif

#include <X11/keysym.h>
#include <X11/Xresource.h>
#include <X11/cursorfont.h>


#define IMAGESIZE (sizeof(IMAGEROW) + (g_ncol - 2) * sizeof(unshort))

#define VIMAGE(row) ((IMAGEROW *) (((char *) g_vimage) + \
	(row) * (IMAGESIZE)))

#define PIMAGE(row) ((IMAGEROW *) (((char *) g_pimage) + \
	(row) * (IMAGESIZE)))


static void imexpose _DEF4(int, row, int, col, int, rows, int, cols);
static void imresize (VOID);

static void infoline _DEF1(WINDOW *, wp);

static void uline _DEF5(unsigned int, row, unshort *, vtextp, unshort *, ptextp,
    unsigned int, color, int, copy);



/* Refresh the display */
COMMAND(emxrefresh)

{
    g_scrwiped = TRUE;
    return TRUE;
}


#ifdef STANDALONE

/* Set the title */

void scrsettitle _ARGS1(char *, title)

{
    XStoreName(g_display, g_mainwin, title);
}

#endif


/* Erase to end of line */

static void screraseline (VOID)

{
    int xpos;

    xpos = HMARGIN + g_scrcol * g_charwidth;
    XFillRectangle(g_display, g_mainwin, g_gchi, xpos,
		   (VMARGIN + g_scrrow * g_charheight),
		   g_windowwidth - xpos, g_charheight);
}


/* Erase to end of page. */

static void screrasetoend (VOID)

{
    int ypos;

    screraseline();
    if (g_scrrow < (g_nrow - 1))
	{
	ypos = VMARGIN + (g_scrrow+1) * g_charheight;
	XFillRectangle(g_display, g_mainwin, g_gchi, 0, ypos,
		    g_windowwidth, g_windowheight - ypos);
	}

}


/* Get the user's attention */

void emxhoot (VOID)

{
    if (g_windowopen)
	XBell(g_display, 50);
}


/*
 * Insert a block of blank lines onto the screen, using a scrolling region
 * that starts at row "row" and extends down to row "bot".
 */

static void scrshiftdown _ARGS3(int, row, int, rows, int, shiftby)

{
    int	    top, shiftsize, delta;

    top = VMARGIN + row * g_charheight;
    delta = shiftby * g_charheight;
    shiftsize = rows * g_charheight - delta;

    XCopyArea(g_display, g_mainwin, g_mainwin, g_gclo,
		0, top, g_windowwidth, shiftsize, 0, top + delta);

    XClearArea(g_display, g_mainwin, 0, top, g_windowwidth, delta, False);
}


/*
 * Delete a block of lines, with the uppermost line at row "row", in a screen
 * slice that extends to row "bot".  The "shiftby" is the number of lines that
 * have to be deleted.	The block delete is used by the slightly more optimal
 * display code.
 */

static void scrshiftup _ARGS3(int, row, int, rows, int, shiftby)

{
    int top, shiftsize, delta;

    top = VMARGIN + row * g_charheight;
    delta = shiftby * g_charheight;
    shiftsize = rows * g_charheight - delta;

    XCopyArea(g_display, g_mainwin, g_mainwin, g_gclo, 0, top + delta,
		    g_windowwidth, shiftsize, 0, top);

    XClearArea(g_display, g_mainwin, 0, top + shiftsize, g_windowwidth,
		delta, False);
}


/* Resize the window to the given row and column count */

int emxscrsetsize _ARGS2(int, rows, int, cols)

{
#ifndef STANDALONE

    XtWidgetGeometry	request;
    XtWidgetGeometry	reply;
    XtGeometryResult	answer;

    if (rows < MINROW)
	rows = MINROW;

    if (rows > MAXROW)
	rows = MAXROW;

    if (cols < MINCOL)
	cols = MINCOL;

    if (cols > MAXCOL)
	cols = MAXCOL;

    request.height = rows * g_charheight + 2 * VMARGIN;
    request.width = cols * g_charwidth + 2 * HMARGIN;
    request.request_mode = CWWidth | CWHeight;
    answer = XtMakeGeometryRequest((Widget) emx, &request, &reply);
    if (answer == XtGeometryNo)
	return FALSE;

    if (answer == XtGeometryAlmost)
	{
	request = reply;
	answer = XtMakeGeometryRequest((Widget) emx, &request, &reply);
	if (answer != XtGeometryYes)
	    return FALSE;
	}

    /* The resize function will do the rest */
    emxscrresize(emx->core.width, emx->core.height, TRUE);

#endif

    return TRUE;
}


void emxscrresize _ARGS3(int, width, int, height, int, showmsg)

{
    WINDOW	*wp;
    WINDOW	*nextwp, *prevwp;
    int		rowsleft;
    char	buf[80];
    int		nextrow, oldrows, oldcols, i, newrows, slop;

    /* Calculate a row count from the given space */
    oldrows = g_nrow;
    oldcols = g_ncol;

    g_nrow = (height - 2 * VMARGIN) / g_charheight;
    g_ncol = (width - 2 * HMARGIN) / g_charwidth;

    if (g_nrow > MAXROW)
	g_nrow = MAXROW;
    if (g_ncol > MAXCOL)
	g_ncol = MAXCOL;
    if (g_nrow < MINROW)
	g_nrow = MINROW;
    if (g_ncol < MINCOL)
	g_ncol = MINCOL;

    g_windowwidth = g_ncol * g_charwidth + 2 * HMARGIN;
    g_windowheight = g_nrow * g_charheight + 2 * VMARGIN;

    /* If window size changed, adjust windows */
    if (oldrows != g_nrow || oldcols != g_ncol)
	{
	/* Calculate the amount to change each window */
	for (i = 0, wp = g_wheadp->nextp; wp; i++, wp = wp->nextp)
	    ;

	/* Change the window sizes */
	rowsleft = g_nrow - oldrows;
	oldrows--;
	nextrow = 0;
	slop = 0;
	prevwp = NULL;
	for (wp = g_wheadp->nextp; wp->nextp; prevwp = wp, wp = nextwp)
	    {
	    nextwp = wp->nextp;

	    /* Calculate the new size based on resize ratios */
	    newrows = ((wp->rows + 1) * (g_nrow - 1)) / oldrows - 1 + slop;
	    slop = 0;

	    /* Can't have windows smaller than 2 rows */
	    if (newrows < 2)
		{
		if (prevwp)
		    {
		    slop = wp->rows + 1;
		    prevwp->rows += slop;
		    nextrow += slop;
		    slop = 0;
		    }
		else
		    {
		    slop = wp->rows + 1;
		    rowsleft += wp->rows + 1;
		    }

		emxdeletewindow(wp);
		wp = prevwp;
		}
	    else
		{
		wp->flag |= WFINFO|WFHARD;
		wp->toprow = nextrow;
		rowsleft -= newrows - wp->rows;
		wp->rows = newrows;
		nextrow += newrows + 1;
		}
	    }

	/* Fix the last window */
	wp->flag |= WFINFO|WFHARD;
	wp->toprow = nextrow;
	wp->rows += rowsleft;

	/* Fix the prompt window */
	g_promptwp->flag |= WFINFO|WFHARD;
	g_promptwp->toprow = g_nrow - 1;

	if (!g_initialload && showmsg)
	    {
	    sprintf(buf, "[New size %d by %d]", g_nrow, g_ncol);
	    emxmsgprint(buf);
	    }

	g_scrwiped = TRUE;
	imresize();
	}

}


void emxscrinitsize _ARGS0()

{
    int	    rowsleft;
    int	    w;
    int	    average;
    int	    current;
    WINDOW  *wp;

    /* Set reasonable values for width and height, and adjust windows
       accordingly */

    if (g_modes.usedefaultsize && g_initwidth)
        g_ncol = g_initwidth;
#ifndef STANDALONE
    else
        g_ncol = ((int) emx->core.width - 2 * HMARGIN) / g_charwidth;
#endif

    if (g_modes.usedefaultsize && g_initheight)
        g_nrow = g_initheight;
#ifndef STANDALONE
    else
        g_nrow = ((int) emx->core.height - 2 * VMARGIN) / g_charheight;
#endif

    if (g_nrow > MAXROW)
	g_nrow = MAXROW;
    if (g_ncol > MAXCOL)
	g_ncol = MAXCOL;
    if (g_nrow < MINROW)
	g_nrow = MINROW;
    if (g_ncol < MINCOL)
	g_ncol = MINCOL;

    g_windowwidth = g_ncol * g_charwidth + 2 * HMARGIN;
    g_windowheight = g_nrow * g_charheight + 2 * VMARGIN;

#ifndef STANDALONE

    /* Set the initial widget dimensions */
    if (g_modes.usedefaultsize && g_initheight)
	emx->core.height = g_windowheight;

    if (g_modes.usedefaultsize && g_initwidth)
	emx->core.width = g_windowwidth;

#endif

    /* Allocate the available rows to needy windows */
    rowsleft = g_nrow - 1;

    g_promptwp->rows = 1;
    g_promptwp->toprow = rowsleft;

    /* Calculate the amount to change each window */
    for (w = 0, wp = g_wheadp->nextp; wp; w++, wp = wp->nextp)
	;

    average = rowsleft / w;
    current = 0;

    for (wp = g_wheadp->nextp; --w; wp = wp->nextp)
	{
	wp->toprow = current;
	wp->rows = average - 1;
	current += average;
	}

    if (wp)
	{
	wp->toprow = current;
	wp->rows = rowsleft - current - 1;
	}

    g_scrwiped = TRUE;
    imresize();
}


#ifdef STANDALONE

/* Set up a GC */

static void scrsetgc _ARGS6(Window, win, GC *, gcp, XFontStruct *, fontp,
    unsigned long, foreground, unsigned long, background, int, function)

{
    *gcp = XCreateGC(g_display, win, 0, NULL);
    if (fontp)
	XSetFont (g_display, *gcp, fontp->fid);
    XSetForeground(g_display, *gcp, foreground);
    XSetBackground(g_display, *gcp, background);
    if (function)
	XSetFunction(g_display, *gcp, function);
}

#endif


/* Write lotsa characters. */

static void scrput _ARGS2(unshort *, cp, int, k)

{
    unshort	*endp;
    char	*bufp;
    unshort	color;
    GC		gc;
    int		xpos, ypos;
    char	buf[MAXVIS];

    /* Loop through, looking for segments with similar highlighting */
    ypos = g_scrrow * g_charheight + VMARGIN + g_charascent;
    endp = cp + k;
    while (cp < endp)
	{
	bufp = buf;
	color = *cp & 0xff00;
	do
	    {
	    *bufp++ = (*cp++) & 0xff;
	    }
	    while (cp < endp && (color == (*cp & 0xff00)));

	k = bufp - buf;

	if ((g_scrcolor == INFOCOLOR && color == 0) ||
	    (g_scrcolor == TEXTCOLOR && color == HIGHLIGHT))
	    gc = g_gchi;
	else
	    gc = g_gclo;

	xpos = g_scrcol * g_charwidth + HMARGIN;
	XDrawImageString(g_display, g_mainwin, gc, xpos, ypos, buf, k);
	g_scrcol += k;
	}

}


/* Toggle the cursor */

void emxswitchcursor _ARGS1(int, set)

{
    int ypos;

    /* ignore cursor if window not yet open or if mouse move enabled */
    if (!g_windowopen || g_mousedo || set == g_cursorstate)
	return;

    /* ON requires setting the position */
    if (set == ON)
	{
	g_cursorx = g_scrcol * g_charwidth + HMARGIN;
	ypos = g_scrrow * g_charheight + VMARGIN;
	if (!g_modes.cursorstyle)
	    {
	    g_cursory = ypos + g_charascent;
	    g_cursorh = CURSORTALL;
	    }

	else
	    {
	    g_cursory = ypos;
	    g_cursorh = g_charheight;
	    }

	}

    /* Erase the old or draw the new */
    XFillRectangle(g_display, g_mainwin, g_gcinv, g_cursorx, g_cursory,
		   g_charwidth, g_cursorh);

    g_cursorstate = set;
}


/* Calculate the character positions affected by an exposed rectangle */

void emxscrexpose _ARGS4(int, x, int, y, int, width, int, height)

{
    int	    cx, cy, cw, ch;
    int row, col, rows, cols;
    int firstx, firsty;

    /* Calculate the current cursor position */
    cx = g_scrcol * g_charwidth + HMARGIN;
    cy = g_scrrow * g_charheight + VMARGIN;
    cw = g_charwidth;
    if (!g_modes.cursorstyle)
	{
	cy += g_charascent;
	ch = CURSORTALL;
	}

    else
	ch = g_charheight;

    /* See if any part of the cursor rectangle has been zapped */
    if (!(cx >= x + width || cy >= y + height ||
	  cx + cw <= x	  || cy + ch <= y))
	{
	/* Clear the entire cursor rectangle */
	XFillRectangle(g_display, g_mainwin, g_gchi, cx, cy, cw, ch);
	}

    /* Now figure out which character positions have been erased */
    if (y < VMARGIN)
	row = 0;
    else
	row = (y - VMARGIN) / g_charheight;

    /* Ignore exposes at the fringes */
    if (row >= g_nrow)
	return;

    if (x < HMARGIN)
	col = 0;
    else
	col = (x - HMARGIN) / g_charwidth;

    /* Ignore exposes at the fringes */
    if (col >= g_ncol)
	return;

    /* Include entire affected rows and columns... */
    firsty = row * g_charheight + VMARGIN;
    firstx = col * g_charwidth + HMARGIN;

    rows = ((y + height - firsty) + g_charheight - 1) / g_charheight;
    cols = ((x + width - firstx) + g_charwidth - 1) / g_charwidth;

    if (row + rows > g_nrow)
	rows = g_nrow - row;

    if (col + cols > g_ncol)
	cols = g_ncol - col;

    imexpose(row, col, rows, cols);
}


/* Shift the physical screen by 'cols' - positive means bits to the left */

static void scrhorizontal _ARGS3(int, row, int, rows, int, cols)

{
    int	    srccol, destcol;
    int	    delta;

    /* Can't scroll zero */
    if (cols == 0)
	return;

    /* Scrolling right */
    if (cols > 0)
	{
	delta = cols;
	srccol = delta;
	destcol = 0;
	}

    /* Scrolling left */
    else
	{
	delta = -cols;
	srccol = 0;
	destcol = delta;
	}

    /* Do the scroll */
    XCopyArea(g_display, g_mainwin, g_mainwin, g_gclo,
	      srccol * g_charwidth + HMARGIN,
	      row * g_charheight + VMARGIN,
	      (g_ncol - delta) * g_charwidth, rows * g_charheight,
	      destcol * g_charwidth + HMARGIN,
	      row * g_charheight + VMARGIN);
}


/* This routine will toggle the scrolling between single line and half
   buffer. This scrolling toggle is only active when scrolling occurs due to
   the cursor moving off the screen */

COMMAND(emxscrolltoggle)

{
    g_modes.verticalscroll ^= 1;
    return TRUE;
}


#ifdef STANDALONE

/* Initialize the display code */

void iminit _ARGS5(int, argc, char *, argv, char *, titlep,
		   int, height, int, width)

{
    char		    *display_name;
    unsigned long	    valuemask;
    Pixmap		    pixmap;
    XSetWindowAttributes    attributes;
    XTextProperty	    winname;
    XTextProperty	    iconname;
    XSizeHints		    s_hints;
    XWMHints		    w_hints;
    int			    s_width, s_height;
    char		    **arglst;
    char		    *argp;
    char		    fontname[80];
    int			    fontset;
    char		    *fnamep;
    XFontStruct		    *font_info;
    XrmDatabase		    applicationDB;
    XrmValue		    value;
    char		    str_type[20];
    int			    maxw;
    Colormap		    colormap;
    XColor		    color;
    char		    *colorp;
    char		    *iconp;
    int			    winwidth;
    int			    winheight;
    Cursor                  caret_cursor;
    

    /* connect to X server */
    display_name = getenv("DISPLAY");
    if ((g_display = XOpenDisplay(display_name)) == 0)
	{
	printf("emx: cannot connect to X server %s\n",
		XDisplayName(display_name));
	exit(-1);
	}

    g_screen = DefaultScreen(g_display);

    /* copy default font name */
    strcpy(fontname, g_fontdef);

    /* if new font size was entered on command line, change font name */
    fontset = False;
    arglst = argv;
    while (*++arglst)
	{
	argp = *arglst;
	if (strcmp(argp, "-f") == 0 ||
	    strcmp(argp, "-fn") == 0 ||
	    strcmp(argp, "-font") == 0)
	    fontset = True;

	else if (fontset)
	    {
	    strcpy(fontname, argp);
	    break;
	    }

	}

    /* if no font on command line, check resource files - user and application */
    if (!fontset)
	{
	fnamep = XGetDefault(g_display, "emx", "font");
	if (fnamep && *fnamep != '\0')
	    {
	    strcpy(fontname, fnamep);
	    fontset = True;
	    }

	}

    if (!fontset)
	{
	applicationDB = XrmGetFileDatabase("/usr/lib/X11/app-defaults/emx");
	if (applicationDB && (XrmGetResource(applicationDB, "font",
				"Font", str_type, &value) == True))
	    {
	    strncpy(fontname, value.addr, value.size);
	    *(fontname + value.size) = '\0';
	    }
	}

    /* load font */
    if ((font_info = XLoadQueryFont(g_display, fontname)) == 0)
	{
	if ((font_info = XLoadQueryFont(g_display, "fixed")) == 0)
	    {
	    printf("emx: cannot open font '%s'\n", fontname);
	    exit (-1);
	    }

	}

    /* set char size values from font */
    g_charheight = font_info->ascent + font_info->descent;
    g_charwidth = font_info->max_bounds.width;
    g_charascent = font_info->ascent;

    /* set preferred window size */
    if (!height)
	height = INITROW;

    if (!width)
	width = INITCOL;

    /* first determine size slightly smaller than screen */
    s_width = DisplayWidth(g_display, g_screen);
    s_height = DisplayHeight(g_display, g_screen);
#ifndef STANDALONE
    s_width -= MARGIN; /* widget margin hack */
    s_height -= MARGIN;
#endif
    winwidth = g_charwidth * width;
    winheight = g_charheight * (height + 1);
#ifdef STANDALONE
    winwidth += 2 * g_charwidth;
#endif

    if (winwidth > s_width)
	winwidth = s_width;
    if (winheight > s_height)
	winheight = s_height;

    /* Fetch some color data */
    g_foreground = BlackPixel(g_display, g_screen);
    g_background = WhitePixel(g_display, g_screen);
    colormap = DefaultColormap(g_display, g_screen);

    if (colorp = XGetDefault(g_display, "emx", "foreground"))
	{
	if (XParseColor(g_display, colormap, colorp, &color))
	    if (XAllocColor(g_display, colormap, &color))
		g_foreground = color.pixel;
	}

    if (colorp = XGetDefault(g_display, "emx", "background"))
	{
	if (XParseColor(g_display, colormap, colorp, &color))
	    if (XAllocColor(g_display, colormap, &color))
		g_background = color.pixel;
	}

    /* set attributes for window */
    attributes.background_pixel = g_background;
    attributes.border_pixel = g_foreground;
    valuemask = CWBackPixel | CWBorderPixel;

    /* create main window */
    g_mainwin = XCreateWindow(g_display, RootWindow(g_display, g_screen),
		0/*250*/, 0/*150*/, winwidth, winheight, WINBORDERWIDTH,
		CopyFromParent, InputOutput, CopyFromParent, valuemask,
		&attributes);

    caret_cursor = XCreateFontCursor(g_display, XC_xterm);
    XDefineCursor(g_display, g_mainwin, caret_cursor);

    /* set standard properties for window manager */
    s_hints.flags = (long)(PPosition | PSize | PMinSize | PMaxSize |
			   PResizeInc);
    s_hints.x = 0;
    s_hints.y = 0;
    s_hints.width = winwidth;
    s_hints.height = winheight;
/*  s_hints.min_width = MINCOL * g_charwidth + 2 * HMARGIN;
    s_hints.min_height = MINROW * g_charheight + 2 * VMARGIN; */
    s_hints.min_width = 0;
    s_hints.min_height = 0;
    s_hints.width_inc = g_charwidth;
    s_hints.height_inc = g_charheight;
    maxw = MAXVIS * g_charwidth;
    s_hints.max_width = (s_width > maxw) ? maxw : s_width;
    s_hints.max_height = s_height;

    w_hints.flags = InputHint;
    w_hints.input = TRUE;

#ifdef UGLY_ICON    
    /* Create the icon pixmap */
    pixmap = XCreateBitmapFromData(g_display, g_mainwin, emxicon_bits,
				   emxicon_width, emxicon_height);
    w_hints.flags |= IconPixmapHint;
    w_hints.icon_pixmap = pixmap;
#endif

    iconp = "EMX";
    XStringListToTextProperty(&titlep, 1, &winname);
    XStringListToTextProperty(&iconp, 1, &iconname);

    XSetWMProperties(g_display, g_mainwin, &winname, &iconname, argv, argc,
		     &s_hints, &w_hints, NULL);

    /* select input types wanted */
    XSelectInput(g_display, g_mainwin, ExposureMask | KeyPressMask |
		ButtonPressMask | ButtonReleaseMask | VisibilityChangeMask |
		Button1MotionMask | StructureNotifyMask);

    /* set up GC's for normal and highlight drawing and for cursor */
    scrsetgc(g_mainwin, &g_gclo, font_info, g_foreground, g_background, NULL);
    scrsetgc(g_mainwin, &g_gchi, font_info, g_background, g_foreground, NULL);
    scrsetgc(g_mainwin, &g_gcinv, NULL, g_foreground, g_background, GXinvert);

    /* Set the invert mask for sensible highlighting */
    XSetPlaneMask(g_display, g_gcinv, (long) g_foreground ^ g_background);

    /* map window */
    XMapWindow(g_display, g_mainwin);
}

#endif


static void iminitblanks (VOID)

{
    int	    i;
    unshort *bp;

    g_blanks->color = TEXTCOLOR;
    bp = g_blanks->text;
    for (i = 0; i < g_ncol; i++)
	*bp++ = (unshort) ' ';
}


/* Highlight a chunk of text */

void emximhighlight _ARGS5(BUFFER *, bp, LINE *, firstlp, int, firsto,
			   LINE *, lastlp, int, lasto)

{
    REGION  region;
    HIGHSEG *highp;
    int	    offset;
    LINE    *lp;
    int	    col;

    /* Make up the region for this extent */
    emxsetregion(&region, bp->linep, firstlp, firsto, lastlp, lasto);

    /* Make sure there is space for the select list */
    if (bp->highlistp && (bp->highsize < region.lines + 1))
	{
	FREE(bp->highlistp);
	bp->highlistp = NULL;
	}

    if (!bp->highlistp)
	{
	bp->highsize = region.lines + 1;
	bp->highlistp = (HIGHSEG *) malloc(bp->highsize * sizeof(HIGHSEG));
	}

    bp->highlightp = bp->highlistp;
    highp = bp->highlistp;
    offset = region.firsto;
    lp = region.firstlp;
    while (lp != region.lastlp)
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
    highp->endo = region.lasto;

    /* Adjust framing to get the end in sight */
    if (g_curwp->leftcol)
	{
	col = emxcursorcol((char *)lp, highp->endo);
	if (col < g_ncol)
	    {
	    g_curwp->leftcol = 0;
	    g_curwp->flag |= WFSHIFT|WFHARD;
	    }
	}

    highp++;
    highp->lp = NULL;

    /* Update all windows showing this buffer */
    emxbupdate(bp, WFHARD);
    g_highlit = TRUE;
}


/* Lowlight the currently highlit text */

void emximlowlight (VOID)

{
    BUFFER  *bp;

    /* This assumes that highlighting is always in the current buffer */
    bp = g_curbp;
    if (bp->highlightp)
	{
	/* Clear this list - leave the space */
	bp->highlightp = NULL;

	/* If showing, update it */
	emxbupdate(bp, WFHARD);
	emxupdate();
	XFlush(g_display);

	g_highlit = FALSE;
	}

}

/* Check a line offset against a highlight list */

static int imgethighlight _ARGS3(int, offset, LINE *, lp, HIGHSEG *, highlistp)

{
    while (highlistp->lp == lp)
	{
	if (offset >= highlistp->firsto &&
	    (offset < highlistp->endo ||
	     (highlistp->endo >= (int) lp->used &&
	      (highlistp + 1)->lp == lp->nextp &&
	      (highlistp + 1)->firsto == 0)))
	    return HIGHLIGHT;

	highlistp++;
	}

    return 0;
}


/* Resize the image buffers */

static void imresize (VOID)

{
    if (g_vimage)
	FREE(g_vimage);

    if (g_pimage)
	FREE(g_pimage);

    if (g_blanks)
	FREE(g_blanks);

    g_rowsize = (sizeof(IMAGEROW) + (g_ncol - 2) * sizeof(unshort));
    g_vimage = (IMAGEROW *) malloc(g_nrow * g_rowsize);
    g_pimage = (IMAGEROW *) malloc(g_nrow * g_rowsize);
    g_blanks = (IMAGEROW *) malloc(g_rowsize);
    iminitblanks();
}


/* Put a string of characters to the virtual image */

#if defined(FLG_PROTOTYPE)
static void imputs (IMAGEROW *image, int length, unsigned char *ptr,
    int leftcol, LINE *lp, HIGHSEG *selp, HIGHSEG *highp)
#else
static void imputs (image, length, ptr, leftcol, lp, selp, highp)

IMAGEROW *image;
int	length;
unsigned char	*ptr;
int	leftcol;
LINE	*lp;
HIGHSEG *selp;
HIGHSEG *highp;
#endif
{
    unshort *imp;
    int	    i;
    int	    col;
    unshort high;
    int	    endcol;
    int	    tabcount;
    int	    tabcol;
    int	    j;

    i = 0;
    if (length == 0)
	{
	if (selp)
	    high = imgethighlight(i, lp, selp);
	else
	    high = 0;

	if (highp)
	    high ^= imgethighlight(i, lp, highp);
	}

    col = 0;
    endcol = leftcol + g_ncol;
    imp = image->text;

    /* Loop on the necessary checks */
    while (i < length && col < endcol)
	{
	/* Record highlight positions */
	if (selp)
	    high = imgethighlight(i, lp, selp);
	else
	    high = 0;

	if (highp)
	    high ^= imgethighlight(i, lp, highp);

	/* Tab characters get a bunch of space */
	if (*ptr == '\t')
	    {
	    MARK(imputstab);
	    tabcol = (col | 7) + 1;
	    tabcount = tabcol - col;

	    if (col < leftcol)
		tabcount -= leftcol - col;

	    if (tabcount > 0)
		{
		for (j = 0; j < tabcount; j++)
		    *imp++ = ' ' + high;
		}

	    col = tabcol;
	    ptr++;
	    }

	/* Control characters get double screen space */
	else if (ISCTRL(*ptr))
	    {
	    MARK(imputsctrl);
	    if (col >= leftcol)
		*imp++ = '^' + high;

	    col++;
	    if (col >= leftcol)
		*imp++ = ((*ptr) ^ 0x40) + high;

	    col++;
	    ptr++;
	    }

	else
	    {
	    MARK(imputschar);
	    if (col >= leftcol)
		*imp++ = (unshort) *ptr + high;

	    col++;
	    ptr++;
	    }

	i++;
	}

    /* For profiling */
    MARK(imputsclear);

    /* Clear the rest, if any */
    if (endcol > col)
	{
	if (selp)
	    high = imgethighlight(i, lp, selp);
	else
	    high = 0;

	if (highp)
	    high ^= imgethighlight(i, lp, highp);

	/* Clear the rest, propagating the last highlight state */
	high += ' ';

	/* Adjust for shifted lines */
	if (col < leftcol)
	    col = leftcol;

	for (j = endcol - col; j > 0; j--)
	    *imp++ = high;
	}

    /* Set the long line flag */
    if (i < length)
	g_longline = TRUE;
}


/* Scroll the physical and virtual images n lines in a positive or negative
   direction. Positive means movement upwards. */

void emximvertical _ARGS1(int, n)

{
    int	    rows;
    int	    lastrow;
    int	    count;
    char    *srcp;
    char    *destp;

    rows = g_curwp->rows;
    if (n == 0 || n > rows - 1 || -n > rows - 1)
	return;

    lastrow = g_curwp->toprow + rows - 1;

    if (n > 0)
	{
	/* Physically shift the screen */
	scrshiftdown(g_curwp->toprow, g_curwp->rows, n);

	/* Shift the screen image */
	srcp = (char *) PIMAGE(lastrow - n);
	destp = (char *) PIMAGE(lastrow);
	count = rows - n;
	do
	    {
	    memcpy(destp, srcp, g_rowsize);
	    srcp -= g_rowsize;
	    destp -= g_rowsize;
	    }
	    while (--count);

	/* Blank the lines at the top */
	destp = (char *) PIMAGE(g_curwp->toprow);
	}

    else
	{
	n = -n;

	/* Physically shift the screen */
	scrshiftup(g_curwp->toprow, g_curwp->rows, n);

	/* Shift the screen image */
	srcp = (char *) PIMAGE(g_curwp->toprow + n);
	destp = (char *) PIMAGE(g_curwp->toprow);
	count = rows - n;

	/* 'update' will pick up on this and do less work */
	do
	    {
	    memcpy(destp, srcp, g_rowsize);
	    srcp += g_rowsize;
	    destp += g_rowsize;
	    }
	    while (--count);

	}

    /* Blank out the other rows */
    do
	{
	memcpy(destp, g_blanks, g_rowsize);
	destp += g_rowsize;
	}
	while (--n);

}


/* Scroll the physical and virtual images n columns in a positive or negative
   direction. Positive means movement to the right. */

void emximhorizontal _ARGS3(int, row, int, rows, int, cols)

{
    int	    i;
    int	    r;
    IMAGEROW	*rowp;
    unshort *srcp;
    unshort *destp;
    unshort *endp;

    /* Weed out useless scrolls */
    if (cols == 0 || (cols > 0 && g_ncol <= cols) ||
	(cols < 0 && (g_ncol + cols < 1)))
	return;

    /* Physically shift the screen */
    scrhorizontal(row, rows, cols);

    /* Shift the portion of the physical image which is intact */
    i = rows;
    r = row;
    while (i--)
	{
	rowp = PIMAGE(r);
	if (cols > 0)
	    memcpy(&rowp->text[0], &rowp->text[cols],
		    (g_ncol - cols) * 2);
	else
	    {
	    srcp = &rowp->text[g_ncol + cols];
	    destp = &rowp->text[g_ncol];
	    endp = rowp->text;
	    while (srcp > endp)
		*--destp = *--srcp;
	    }

	r++;
	}

}


/* Clear a rectangle in the display, and mark rows as needing redisplay.
   This is called by anyone who knows that some of the window has been
   exposed. */

static void imexpose _ARGS4(int, row, int, col, int, rows, int, cols)

{
    IMAGEROW	*rowp;

    while (rows--)
	{
	rowp = PIMAGE(row);
	memcpy(&rowp->text[col], g_blanks->text, cols * 2);
	rowp->color = TEXTCOLOR;
	(VIMAGE(row))->change = TRUE;
	row++;
	};

}


static int horizontalframe _ARGS1(WINDOW *, wp)

{
    int	    edge, col;

    /* Shift until the cursor is visible */
    col = emxcursorcol(wp->dotp->text, wp->doto);
    edge = col < wp->leftcol ? col : wp->leftcol;
    if (col >= edge + g_ncol)
	edge = col - g_ncol + 1;

    return edge;
}


/* Make sure that the display is right. This is a three part process. First,
   scan through all of the windows looking for dirty ones. Check the framing,
   and refresh the screen. Second, make sure that "currow" and "curcol" are
   correct for the current window. Third, make the virtual and physical images
   the same. */

void emxupdate (VOID)

{
    LINE	*lp;
    WINDOW	*wp;
    IMAGEROW	*imp1;
    IMAGEROW	*imp2;
    int		i;
    int		rows;
    LINE	*endlp;
    int		currow;
    int		curcol;
    HIGHSEG	*selp;
    HIGHSEG	*sp;
    HIGHSEG	*highp;
    HIGHSEG	*hp;

    if (!g_windowopen)
	return;

    /* Watch for a cursor in blank-trimmed space */
    if (g_curwp->doto > (int) g_curwp->dotp->used)
	g_curwp->doto = g_curwp->dotp->used;

    /* See if any windows need repositioning */
    for (wp = g_wheadp; wp; wp = wp->nextp)
	{
	/* This window needs updating */
	if (wp->flag)
	    {
	    rows = wp->rows;
	    endlp = wp->bufp->linep;
	    if ((wp->flag & WFFORCE) == 0)
		{
		lp = wp->linep;
		if (lp != endlp)
		    for (i = 0; i < rows; ++i)
			{
			if (lp == wp->dotp)
			    goto done;

			if (lp == endlp)
			    break;

			lp = lp->nextp;
			}

		}

	    i = wp->force;	  /* Reframe this one */
	    lp = wp->dotp;
	    if (i > 0)
		{
		--i;
		if (i >= rows)
		    i = rows - 1;
		}

	    else if (i < 0)
		{
		i += rows;
		if (i < 0)
		    i = 0;
		}

	    else
		{
		if (g_modes.verticalscroll)
		    {
		    /* Scrolled past the bottom of the screen.*/
		    /* Do a one line scroll by making the 2nd */
		    /* line in the current window become the */
		    /* top line in reframed window */

		    if (lp->nextp != wp->linep || lp == endlp)
			i = rows - 1;
		    }

		else
		    i = rows / 2;
		}

	    while (i && lp->prevp != endlp)
		{
		--i;
		lp = lp->prevp;
		}

	    wp->linep = lp;
	    wp->flag |= WFHARD;	  /* Force full */
	    }

    done:
	i = 0;
	}

    /* Locate the cursor */
    lp = g_curwp->linep;
    currow = g_curwp->toprow;
    while (lp != g_curwp->dotp)
	{
	++currow;
	lp = lp->nextp;
	}

    /* If the cursor is off-screen, reframe so it shows */
    if (!(g_curwp->flag & WFSHIFT) &&
	(i = horizontalframe(g_curwp)) != g_curwp->leftcol)
	{
	emximhorizontal(g_curwp->toprow, g_curwp->rows, i - g_curwp->leftcol);
	g_curwp->leftcol = i;
	g_curwp->flag |= WFHARD;
	}

    /* Compute the cursor offset */
    if ((curcol = emxcursorcol(lp->text, g_curwp->doto)) < g_curwp->leftcol)
	curcol = 0;
    else
	curcol -= g_curwp->leftcol;

    /* Now update the virtual image */
    for (wp = g_wheadp; wp; wp = wp->nextp)
	{
	/* This window needs updating */
	if (wp->flag)
	    {
	    g_longline = FALSE;	   /* no lines exceed window bound */

	    /* Make the virtual image match the text buffers */
	    lp = wp->linep;
	    selp = wp->bufp->selectp;
	    highp = wp->bufp->highlightp;
	    i  = wp->toprow;
	    if ((wp->flag & ~WFINFO) == WFEDIT)
		{
		while (lp != wp->dotp)
		    {
		    ++i;
		    lp = lp->nextp;
		    }

		imp1 = VIMAGE(i);
		imp1->color = TEXTCOLOR;
		imp1->change = TRUE;
		selp = (HIGHSEG *) emxfindhighlight(selp, lp);
		highp = (HIGHSEG *) emxfindhighlight(highp, lp);
		imputs(imp1, lp->used, (unsigned char *)lp->text, wp->leftcol,
		    lp, selp, highp);
		}

	    else if (wp->flag & (WFEDIT|WFHARD))
		{
		endlp = wp->bufp->linep;
		rows = wp->toprow + wp->rows;
		while (i < rows)
		    {
		    imp1 = VIMAGE(i);
		    imp1->color = TEXTCOLOR;
		    imp1->change = TRUE;
		    if (lp != endlp)
			{
			sp = (HIGHSEG *) emxfindhighlight(selp, lp);
			if (sp)
			    selp = sp;

			hp = (HIGHSEG *) emxfindhighlight(highp, lp);
			if (hp)
			    highp = hp;

			imputs(imp1, lp->used, (unsigned char *)lp->text,
			    wp->leftcol, lp, sp, hp);
			lp = lp->nextp;
			}

		    else
			memcpy(imp1->text, g_blanks->text, g_ncol * 2);

		    i++;
		    }

		}

	    if (g_longline || (wp->flag & WFINFO))
		infoline(wp);

	    wp->flag = 0;
	    if (g_longline)
		wp->flag |= WFINFO;

	    wp->force = 0;
	    }

	}

    /* Display the changes */
    if (g_scrwiped)
	{
	/* Screen has been wiped - redisplay everything */
	g_scrwiped = FALSE;
	g_scrrow = 0;
	g_scrcol = 0;
	screrasetoend();

	for (i = 0; i < g_nrow; ++i)
	    {
	    imp1 = VIMAGE(i);
	    imp2 = PIMAGE(i);
	    if (imp1->color != TEXTCOLOR)
		{
		g_scrrow = i;
		g_scrcol = 0;
		g_scrcolor = imp1->color;
		scrput(imp1->text, g_ncol);
		}

	    else
		uline(i, imp1->text, g_blanks->text, imp1->color, FALSE);

	    imp1->change = FALSE;		/* Changes done */
	    memcpy(imp2, imp1, g_rowsize);	/* Update model */
	    }

	}

    /* Update only changed lines */
    else
	{
	for (i=0; i < g_nrow; ++i)
	    {
	    imp1 = VIMAGE(i);
	    imp2 = PIMAGE(i);
	    if (imp1->change)
		{
		if (imp1->color != imp2->color)
		    {
		    g_scrrow = i;
		    g_scrcol = 0;
		    g_scrcolor = imp1->color;
		    scrput(imp1->text, g_ncol);
		    memcpy(imp2->text, imp1->text, g_ncol * 2);
		    }

		else
		    uline(i, imp1->text, imp2->text, imp1->color, TRUE);

		imp1->change = FALSE;		      /* Changes done */
		imp2->color = imp1->color;
		}
	    }
	}

    g_scrrow = currow;
    g_scrcol = curcol;
}


/* Update a single line. This function compares the virtual and physical images
   for the line and draws only as much as it needs to, doing an erase for as
   much as possible. */

static void uline _ARGS5(unsigned int, row, unshort *, vtextp, unshort *, ptextp,
    unsigned int, color, int, copy)

{
    register unshort	*cpleft;
    register unshort	*cpphys;
    register unshort	*cpright;
    unsigned int	cols;
    int			nbflag;

    cpleft = vtextp;			/* Compute left match */
    cpphys = ptextp;
    cols = g_ncol;
    while ((*cpleft++ == *cpphys++) && --cols)
	;

    if (cols == 0)			/* All equal */
	return;

    cpleft--;
    nbflag = FALSE;
    cpright = vtextp + g_ncol;		/* Compute right match */
    cpphys =  ptextp + g_ncol;
    while (*(--cpright) == *(--cpphys))
	{
	if (*cpright != ' ')		/* Note non-blanks in right match */
	    nbflag = TRUE;
	}

    /* Point to the last matching character */
    cpright++;
    cpphys = cpright;			/* Is erase good? */

    if (nbflag == FALSE && color == TEXTCOLOR)
	{
	while (*(--cpphys) == ' ' && cpphys != cpleft)
	    ;

	cpphys++;
	}

    g_scrrow = row;
    g_scrcol = cpleft - vtextp;
    g_scrcolor = color;
    scrput(cpleft, cpphys - cpleft);
    if (cpphys != cpright)
	screraseline();

    if (copy)
	memcpy(ptextp + (cpleft - vtextp), cpleft, (cpright - cpleft) * 2);
}


/* Redisplay the infoline for the window pointed to by the "wp".  This WAS the
   only routine that has any idea of how the infoline is formatted. You can
   change the infoline format by hacking at this routine. Called by "update"
   any time there is a dirty window.

   WARNING column offsets for mouse icons are hard coded */

static void infoline _ARGS1(WINDOW *, wp)

{
    register char   *ptr;
    register BUFFER *bp;
    char	buf[2048];
    int		row;
    IMAGEROW	*rowp;

    if (wp == g_promptwp)
	return;

    bp = wp->bufp;
    ptr = buf;
    *ptr++ = ' ';
    *ptr++ = bp->flag & BFCHG	? '*' : '.';  /* Buffer modified indicator */
    *ptr++ = g_modes.insert	? 'I' : 'O';  /* Insertion flag */
    *ptr++ = g_savereminder	? 'U' : '.';  /* Auto-update flag */
    *ptr++ = !wp->leftcol	? ' ' : '>';  /* Screen shift flag */

    /* Mouse icons (NOTE HARD CODED OFFSET USED IN MOUSE.C ! */
    strcpy(ptr, " [B][1][2] L:");
    ptr += strlen(ptr);

    /* put out "L: (language)" */
    if (bp->mode < 5)
	{
	strcpy(ptr, (char *)(&emxmodetable[bp->mode].modestring[0]));
	ptr += strlen(ptr);
	}

    /* If there is a buffers, put 2 spaces between */
    /* the language and the current buffer name */
    if (bp->bufname[0] != 0)
	{
	/* put out "B" (buffer name)" */
	*ptr++ = ' ';
	*ptr++ = ' ';
	*ptr++ = 'B';
	*ptr++ = ':';
	strcpy(ptr, bp->bufname);
	ptr += strlen(ptr);
	}

    /* If there is a file associated with the buffer, */
    /* put out two space and then "F; (filename)" */
    if (bp->filename[0] != 0)
	{
	*ptr++ = ' ';
	*ptr++ = ' ';
	*ptr++ = 'F';
	*ptr++ = ':';
	strcpy(ptr, bp->filename);
	ptr += strlen(ptr);
	}

    if (bp->flag & BFVIEW)	/* read only */
	{
	strcpy(ptr, " (Read only)");
	ptr += strlen(ptr);
	}

    /* PC file? */
    if (bp->flag & BFISPC) {
	strcpy(ptr, " [PC]");
	ptr += strlen(ptr);
	}

    /* Inform the user as to the status of the auto- */
    /* backup flag (only if file is open for edit) */
    if (!(bp->flag & BFVIEW) && !g_modes.autobackup && !g_modes.defaultback)
	{
	strcpy(ptr, "  [No backups!]");
	ptr += strlen(ptr);
	}

    if (g_longline)	  /* long line */
	{
	*ptr++ = ' ';
	*ptr++ = '>';
	}

    /* Make sure there are no zeroes on the line */
    if (ptr - buf < MAXVIS)
	memset(ptr, ' ', MAXVIS - (ptr - buf));

    /* Put it out */
    row = wp->toprow + wp->rows;
    rowp = VIMAGE(row);
    rowp->color = INFOCOLOR;
    rowp->change = TRUE;
    imputs(rowp, g_ncol, (unsigned char *)&buf[0], 0, NULL, NULL, NULL);
}
