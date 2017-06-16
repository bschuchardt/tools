#define EMXWIDGET_C TRUE
/*=========================================================================
 * Copyright (C) Servio Corp. 1991-1994.  All Rights Reserved.
 *
 * Name -
 *
 * Purpose -
 *
 * $Id: emxwidget.c,v 30.11 1994/09/03 02:35:08 marcs Exp $
 *
 *========================================================================*/

/*
    EMX - Widget Source

*/

#include "emx.h"


/* Methods */

static void ClassInit _ARGS0();

static void EmxInitialize	_DEF2(EmxWidget, req, EmxWidget, w);
static void EmxRedisplay	_DEF2(EmxWidget, w, XExposeEvent *, event);
static void EmxResize		_DEF1(EmxWidget, w);
static void EmxDestroy		_DEF1(EmxWidget, w);
static Boolean EmxSetValues	_DEF3(EmxWidget, curr, EmxWidget, req, EmxWidget, new);
static XtGeometryResult EmxQueryGeometry	_DEF3(EmxWidget, w, XtWidgetGeometry *, proposed, XtWidgetGeometry *, desired);


#define EmxRBoolean XtRBoolean
#define EmxRCallback XtRCallback
#define EmxRImmediate XtRImmediate
static const char EmxRStringInt[] = "EmxStringInt";


/* Resources */
static XtResource resources[] = {

   {(STRING)XtNallowBufWrites,
			(STRING)XtCAllowBufWrites,
			(STRING)EmxRBoolean, sizeof(Boolean),
			XtOffset(EmxWidget, emx.modes.allowbufwrites),
			(STRING)EmxRImmediate, (XtPointer) FALSE },
   {(STRING)XtNargc,
			(STRING)XtCArgc,
			(STRING)XtRInt, sizeof(int),
			XtOffset(EmxWidget, emx.argc),
			(STRING)EmxRImmediate, (XtPointer) 0 },
   {(STRING)XtNargv,
			(STRING)XtCArgv,
			(STRING)XtRPointer, sizeof(char **),
			XtOffset(EmxWidget, emx.argv),
			(STRING)EmxRImmediate, (XtPointer) NULL },
   {(STRING)XtNautoBackup,
			(STRING)XtCAutoBackup,
			(STRING)EmxRBoolean, sizeof(Boolean),
			XtOffset(EmxWidget, emx.modes.autobackup),
			(STRING)EmxRImmediate, (XtPointer) FALSE },
   {(STRING)XtNautoMatch,
			(STRING)XtCAutoMatch,
			(STRING)EmxRBoolean, sizeof(Boolean),
			XtOffset(EmxWidget, emx.modes.automatch),
			(STRING)EmxRImmediate, (XtPointer) FALSE },
   {(STRING)XtNcaseSensitive,
			(STRING)XtCCaseSensitive,
			(STRING)EmxRBoolean, sizeof(Boolean),
			XtOffset(EmxWidget, emx.modes.casesensitive),
			(STRING)EmxRImmediate, (XtPointer) FALSE },
   {(STRING)XtNcolumns,
			(STRING)XtCColumns,
			(STRING)XtRInt, sizeof(int),
			XtOffset(EmxWidget, emx.initwidth),
			(STRING)EmxRImmediate, (XtPointer) 80 },
   {(STRING)XtNcursorStyle,
			(STRING)XtCCursorStyle,
			(STRING)EmxRBoolean, sizeof(Boolean),
			XtOffset(EmxWidget, emx.modes.cursorstyle),
			(STRING)EmxRImmediate, (XtPointer) FALSE },
   {(STRING)XtNcustomization,
			(STRING)XtCCustomization,
			(STRING)XtRString, sizeof(char *),
			XtOffset(EmxWidget, emx.customization),
			(STRING)EmxRImmediate, (XtPointer) NULL },
   {(STRING)XtNdefaultBack,
			(STRING)XtCDefaultBack,
			(STRING)EmxRBoolean, sizeof(Boolean),
			XtOffset(EmxWidget, emx.modes.defaultback),
			(STRING)EmxRImmediate, (XtPointer) TRUE },
   {(STRING)XtNdefaultMode,
			(STRING)XtCDefaultMode,
			(STRING)EmxRStringInt, sizeof(int),
			XtOffset(EmxWidget, emx.defaultmode),
			(STRING)EmxRImmediate, (XtPointer) C_MODE },
   {(STRING)XtNfillColumn,
			(STRING)XtCFillColumn,
			(STRING)XtRInt, sizeof(int),
			XtOffset(EmxWidget, emx.fillcol),
			(STRING)EmxRImmediate, (XtPointer) 0 },
   {(STRING)XtNfillDouble,
			(STRING)XtCFillDouble,
			(STRING)EmxRBoolean, sizeof(Boolean),
			XtOffset(EmxWidget, emx.modes.filldouble),
			(STRING)EmxRImmediate, (XtPointer) FALSE },
   {(STRING)XtNfont,
			(STRING)XtCFont,
			(STRING)XtRFontStruct, sizeof(XFontStruct *),
			XtOffset(EmxWidget, emx.font),
			(STRING)XtRString, (XtPointer) g_fontdef },
   {(STRING)XtNforeground,
			(STRING)XtCForeground,
			(STRING)XtRPixel, sizeof(Pixel),
			XtOffset(EmxWidget, emx.foreground),
			(STRING)XtRString, (XtPointer) "Black" },
   {(STRING)XtNinsert,
			(STRING)XtCInsert,
			(STRING)EmxRBoolean, sizeof(Boolean),
			XtOffset(EmxWidget, emx.modes.insert),
			(STRING)EmxRImmediate, (XtPointer) TRUE },
   {(STRING)XtNmanStyle,
			(STRING)XtCManStyle,
			(STRING)EmxRBoolean, sizeof(Boolean),
			XtOffset(EmxWidget, emx.modes.manstyle),
			(STRING)EmxRImmediate, (XtPointer) FALSE },
   {(STRING)XtNpushNewlines,
			(STRING)XtCPushNewlines,
			(STRING)EmxRBoolean, sizeof(Boolean),
			XtOffset(EmxWidget, emx.modes.pushnewlines),
			(STRING)EmxRImmediate, (XtPointer) FALSE },
   {(STRING)XtNquitCallback,
			(STRING)XtCCallback,
			(STRING)EmxRCallback, sizeof(XtPointer),
			XtOffset(EmxWidget, emx.quitcallback),
			(STRING)EmxRCallback, (XtPointer) NULL },
   {(STRING)XtNreadCallback,
			(STRING)XtCCallback,
			(STRING)EmxRCallback, sizeof(XtPointer),
			XtOffset(EmxWidget, emx.readcallback),
			(STRING)EmxRCallback, (XtPointer) NULL },
   {(STRING)XtNrows,
			(STRING)XtCRows,
			(STRING)XtRInt, sizeof(int),
			XtOffset(EmxWidget, emx.initheight),
			(STRING)EmxRImmediate, (XtPointer) 25 },
   {(STRING)XtNsaveReminder,
			(STRING)XtCSaveReminder,
			(STRING)XtRInt, sizeof(int),
			XtOffset(EmxWidget, emx.savereminder),
			(STRING)EmxRImmediate, (XtPointer) 0 },
   {(STRING)XtNsearchAll,
			(STRING)XtCSearchAll,
			(STRING)EmxRBoolean, sizeof(Boolean),
			XtOffset(EmxWidget, emx.srchall),
			(STRING)EmxRImmediate, (XtPointer) FALSE },
   {(STRING)XtNstandAlone,
			(STRING)XtCStandAlone,
			(STRING)XtRInt, sizeof(int),
			XtOffset(EmxWidget, emx.standalone),
			(STRING)EmxRImmediate, (XtPointer) FALSE },
   {(STRING)XtNtabsOnIndent,
			(STRING)XtCTabsOnIndent,
			(STRING)EmxRBoolean, sizeof(Boolean),
			XtOffset(EmxWidget, emx.modes.tabsonindent),
			(STRING)EmxRImmediate, (XtPointer) TRUE },
   {(STRING)XtNtabset,
			(STRING)XtCTabset,
			(STRING)XtRInt, sizeof(int),
			XtOffset(EmxWidget, emx.tabset),
			(STRING)EmxRImmediate, (XtPointer) TABSET },
   {(STRING)XtNtrimOutput,
			(STRING)XtCTrimOutput,
			(STRING)EmxRBoolean, sizeof(Boolean),
			XtOffset(EmxWidget, emx.modes.trimoutput),
			(STRING)EmxRImmediate, (XtPointer) FALSE },
   {(STRING)XtNuseDefaultSize,
			(STRING)XtCUseDefaultSize,
			(STRING)EmxRBoolean, sizeof(Boolean),
			XtOffset(EmxWidget, emx.modes.usedefaultsize),
			(STRING)EmxRImmediate, (XtPointer) TRUE },
   {(STRING)XtNverticalScroll,
			(STRING)XtCVerticalScroll,
			(STRING)EmxRBoolean, sizeof(Boolean),
			XtOffset(EmxWidget, emx.modes.verticalscroll),
			(STRING)EmxRImmediate, (XtPointer) FALSE },
   {(STRING)XtNwordWrap,
			(STRING)XtCWordWrap,
			(STRING)EmxRBoolean, sizeof(Boolean),
			XtOffset(EmxWidget, emx.modes.wordwrap),
			(STRING)EmxRImmediate, (XtPointer) FALSE },
   {(STRING)XtNwriteCallback,
			(STRING)XtCCallback,
			(STRING)EmxRCallback, sizeof(XtPointer),
			XtOffset(EmxWidget, emx.writecallback),
			(STRING)EmxRCallback, (XtPointer) NULL },

    };


/* Translations and Actions */


static void Event _DEF2(EmxWidget, w, XEvent *, event);
static void EmxDoNothing _ARGS0();


#ifdef MOTIF
static char translations[] =
 "<Key>:	EmxEvent()\n\
<BtnDown>:	EmxEvent()\n\
<BtnUp>:	EmxEvent()\n\
<BtnMotion>:	EmxEvent()\n\
<Visible>:	EmxEvent()\n\
<Enter>:	EmxEnter()\n\
<Leave>:	EmxLeave()\n\
<FocusIn>:	EmxFocusIn()\n\
<FocusOut>:	EmxFocusOut()";

#else

static char translations[] =
 "<Key>:	EmxEvent()\n\
<BtnDown>:	EmxEvent()\n\
<BtnUp>:	EmxEvent()\n\
<BtnMotion>:	EmxEvent()\n\
<Visible>:	EmxEvent()";
#endif


static XtActionsRec actions[] = {
#ifdef MOTIF
    { (STRING)"EmxEnter",	(XtActionProc) _XmPrimitiveEnter	},
    { (STRING)"EmxLeave",	(XtActionProc) _XmPrimitiveLeave	},
    { (STRING)"EmxFocusIn",	(XtActionProc) _XmPrimitiveFocusIn	},
    { (STRING)"EmxFocusOut",	(XtActionProc) _XmPrimitiveFocusOut	},
#endif
    { (STRING)"EmxEvent",	(XtActionProc) Event	},
    };


/* Class Record */

static EmxClassRec emxClassRec = {
    {
    /* core_class fields  */
#ifdef MOTIF
    (WidgetClass) &xmPrimitiveClassRec, /* superclass	      */
#else
    (WidgetClass) &widgetClassRec,	/* superclass	      */
#endif
    (STRING)"Emx",			/* class_name	      */
    sizeof(EmxRec),			/* widget_size	      */
    ClassInit,				/* class_init	      */
    (XtWidgetClassProc) NULL,		/* class_part_initialize    */
    FALSE,				/* class_inited	      */
    (XtInitProc) EmxInitialize,		/* initialize	      */
    (XtArgsProc) NULL,			/* initialize_hook    */
    XtInheritRealize,			/* realize	      */
    actions,				/* actions	      */
    XtNumber(actions),			/* num_actions	      */
    resources,				/* resources	      */
    XtNumber(resources),		/* num_resources      */
    NULLQUARK,				/* xrm_class	      */
    TRUE,				/* compress_motion    */
    TRUE,				/* compress_exposure  */
    TRUE,				/* compress_enterleave*/
    FALSE,				/* visible_interest   */
    (XtWidgetProc) EmxDestroy,		/* destroy	      */
    (XtWidgetProc) EmxResize,		/* resize	      */
    (XtExposeProc) EmxRedisplay,		/* expose	      */
    (XtSetValuesFunc) EmxSetValues,	/* set_values	      */
    (XtArgsFunc) NULL,			/* set_values_hook    */
    XtInheritSetValuesAlmost,		/* set_values_almost  */
    (XtArgsProc) NULL,			/* get_values_hook    */
    (XtAcceptFocusProc) NULL,		/* accept_focus	      */
    XtVersion,				/* version	      */
    NULL,				/* callback_private   */
    translations,			/* tm_table	      */
    (XtGeometryHandler) EmxQueryGeometry,	/* query_geometry     */
    (XtStringProc) NULL,		/* display_accelerator*/
    NULL,				/* extension	      */
    },

#ifdef MOTIF
    {
    (XtWidgetProc) EmxDoNothing,	/* Primitive border_highlight	*/
    (XtWidgetProc) EmxDoNothing,	/* Primitive border_unhighlight */
    translations,			/* translations			*/
    0 /* (XmArmAndActivate) NULL */,	/* arm_and_activate		*/
    NULL,				/* get resources		*/
    0,					/* num get_resources		*/
    NULL,				/* extension			*/
    },
#endif

    /* Emx class fields */
    {
    0,				    /* dummy		  */
    }
    };

WidgetClass emxWidgetClass = (WidgetClass) &emxClassRec;


/* Strings to be converted to legitimate values */

typedef struct
    {
    const char    *string;
    int	    value;
    } CONVERT;

static CONVERT stringtable[] = {
    { "C",			C_MODE },
    { "ST",			ST_MODE },
    { "SMALLTALK",		ST_MODE },
    { 0, 0 }};



static void ConvertToValue _ARGS4(XrmValuePtr, args, Cardinal, numargs,
    XrmValuePtr, fromval, XrmValuePtr, toval)

{
    CONVERT	*cvp;
    char	*ptr;
    char	*p;
    char	buf[1024];

    /* Make the input string lowercase */
    for (ptr = fromval->addr, p = buf; *ptr; ptr++, p++)
	{
	if (islower(*ptr))
	    *p = toupper(*ptr);
	else
	    *p = *ptr;
	}

    *p = 0;

    /* Loop through the table of conversions */
    for (cvp = stringtable; cvp->string; cvp++)
	{
	if (strcmp(cvp->string, buf) == 0)
	    break;
	}

    /* Find one? */
    if (cvp->string)
	{
	toval->size = sizeof(int);
	toval->addr = (XtPointer) &cvp->value;
	}

    else
	{
	toval->size = 0;
	toval->addr = NULL;
	XtStringConversionWarning(fromval->addr, (String) EmxRStringInt);
	}

}


/*---------*/
/* Methods */
/*---------*/


static void ClassInit _ARGS0()

{
    /* Add a converter to turn strings into useable values */
    XtAddConverter(XtRString, (String) EmxRStringInt, (XtConverter) ConvertToValue, NULL, 0);
}


static void EmxInitialize _ARGS2(EmxWidget, req, EmxWidget, w)

{
    XGCValues	    values;
    unsigned long   mask;
    Window	    window;

    emx = w;

#ifdef MOTIF
    w->primitive.shadow_thickness = 0;
    w->primitive.highlight_thickness = 0;
#endif

    /* Clear clearables */
    g_savebuf[0] =	0;
    g_pattern[0] =	0;

    g_vimage =		NULL;
    g_pimage =		NULL;
    g_blanks =		NULL;
    g_rowsize =		0;
    memset(&g_keycurrent, 0, sizeof(KEY));
    memset(&g_keynull, 0, sizeof(KEY));
    g_modifier =	0;
    g_ungottenkeyp =	NULL;
    g_keyp =		NULL;
    g_macroinp =	NULL;
    g_macrooutp =	NULL;
    g_initialload =	TRUE;
    g_elipsis =		FALSE;
    g_piping =		FALSE;
    g_kbufp =		NULL;
    g_killused =	0;
    g_killsize =	0;
    g_killincr =	KILLCHUNK;
    g_scrwiped =	TRUE;
    g_cursorstate =	OFF;
    g_readinginput =	FALSE;
    g_epresent =	FALSE;
    g_bheadp =		NULL;
    g_wheadp =		NULL;
    g_macro =		NULL;
    g_argflag =		FALSE;
    g_argval =		1;
    g_filechanges =	0;
    g_windowopen =	FALSE;
    g_initialmessage[0] = 0;
    g_srchstr =		g_nsense;
    g_srchlastdir =	SRCH_FORW;
    g_executing =	FALSE;
    g_execbuf =		NULL;
    g_execstr =		NULL;
    g_highlit =		FALSE;
    g_mousedo =		FALSE;
    g_lastclicktime =	0;
    g_winvisible =	FALSE;
    g_eliplinep =	NULL;
    g_matchp =		NULL;

    window = RootWindowOfScreen(DefaultScreenOfDisplay(XtDisplay(w)));
    g_display = XtDisplay(w);

    /* set char size values from font */
    g_charheight = g_font->ascent + g_font->descent;
    g_charwidth = g_font->max_bounds.width;
    g_charascent = g_font->ascent;


    /* Create the GC's for all drawing styles */
    mask = GCForeground | GCBackground | GCFont;

    g_background = w->core.background_pixel;

#ifdef MOTIF
    g_foreground = w->primitive.foreground;
#endif

    values.foreground = g_background;
    values.background = g_foreground;
    values.font = g_font->fid;
    g_gchi = XtGetGC((Widget) w, mask, &values);

    values.foreground = g_foreground;
    values.background = g_background;
    g_gclo = XtGetGC((Widget) w, mask, &values);


    mask = GCForeground | GCBackground | GCFunction | GCPlaneMask;
    values.function = GXinvert;
    values.plane_mask = g_foreground ^ g_background;
    g_gcinv = XtGetGC((Widget) w, mask, &values);

    emxeditinit(g_argc, g_argv);
}


static void EmxResize _ARGS1(EmxWidget, w)

{
    emx = w;
    emxscrresize(w->core.width, w->core.height, TRUE);
    emxupdate();
}


static XtGeometryResult EmxQueryGeometry _ARGS3(EmxWidget, w,
	XtWidgetGeometry *, proposed, XtWidgetGeometry *, desired)

{
    int minwidth;
    int minheight;
    int askingwidth;
    int askingheight;

    askingwidth = proposed->request_mode & CWWidth;
    askingheight = proposed->request_mode & CWHeight;

    /* Figure out our minimum size */
    minheight = MINROW * g_charheight + 2 * VMARGIN;
    minwidth = MINCOL * g_charwidth + 2 * HMARGIN;

    /* Accept anything above the minimums */
    if ((!askingwidth || (int) proposed->width >= minwidth) &&
	(!askingheight || (int) proposed->height >= minheight))
	return XtGeometryYes;

    if (w->core.width == minwidth && w->core.height == minheight)
	return XtGeometryNo;

    if (askingwidth && (int) proposed->width < minwidth)
	{
	desired->width = minwidth;
	desired->request_mode |= CWWidth;
	}

    if (askingheight && (int) proposed->height < minheight)
	{
	desired->height = minheight;
	desired->request_mode |= CWHeight;
	}

    return XtGeometryAlmost;
}


static void EmxRedisplay _ARGS2(EmxWidget, w, XExposeEvent *, event)

{
    emx = w;

    /* First time, fetch the window */
    if (!g_windowopen)
	g_mainwin = XtWindow(w);

    emxeventhandle((XAnyEvent *)event);
}


static void EmxDestroy _ARGS1(EmxWidget, w)

{
    WINDOW	*wp;
    WINDOW	*nwp;
    BUFFER	*bp;
    BUFFER	*nbp;
    FUNCTION	*fp;
    FUNCTION	*nfp;
    KEY		*kp;
    KEY		*nkp;
    KEYBINDING	*kbp;
    KEYBINDING	*nkbp;

    emx = w;
    if (!g_standalone)
	{
	/* Release the GC's */
	XtReleaseGC((Widget) w, g_gclo);
	XtReleaseGC((Widget) w, g_gchi);
	XtReleaseGC((Widget) w, g_gcinv);

	/* Free all allocated memory */
	FREE(g_vimage);
	FREE(g_pimage);
	FREE(g_blanks);

	for (bp = g_bheadp; bp; bp = nbp)
	    {
	    nbp = bp->nextp;
	    emxbfree(bp);
	    }

	emxbfree(g_blistp);
	emxbfree(g_promptbp);

	for (wp = g_wheadp; wp; wp = nwp)
	    {
	    nwp = wp->nextp;
	    FREE(wp);
	    }

	if (g_kbufp)
	    FREE(g_kbufp);

	if (g_macro)
	    FREE(g_macro);

	for (fp = g_flistp; fp; fp = nfp)
	    {
	    nfp = fp->nextp;
	    if (fp->type & SYMMACRO)
		{
		FREE((char *) fp->procp);
		FREE(fp->namep);
		}

	    for (kbp = fp->bindingp; kbp; kbp = nkbp)
		{
		nkbp = kbp->nextp;
		FREE(kbp);
		}

	    FREE(fp);
	    }

	for (kp = g_keylistp; kp; kp = nkp)
	    {
	    nkp = kp->orderp;
	    FREE(kp);
	    }

	if (g_matchp)
	    FREE(g_matchp);
	}

}


static Boolean EmxSetValues _ARGS3(EmxWidget, curr, EmxWidget, req, EmxWidget, new)

{
    int	    redisp;

    emx = new;
    redisp = FALSE;

    if (curr->core.width != new->core.width ||
	curr->core.height != new->core.height)
	{
	EmxResize(new);
	redisp = TRUE;
	}

    return redisp;
}


/*---------*/
/* Actions */
/*---------*/


static void EmxDoNothing _ARGS0()

{
}


static void Event _ARGS2(EmxWidget, w, XEvent *, event)

{
    emx = w;
    emxeventhandle((XAnyEvent *) event);
}


/* Call the Quit callback */

int emxwidgetquit _ARGS1(int, status)

{
    XtCallCallbacks((Widget) emx, XtNquitCallback, (XtPointer) status);
    return FALSE;
}


/*-------------------*/
/* Utility Functions */
/*-------------------*/


static void loadfromstring _ARGS2(BUFFER *, bp, const char *, str)

{
    const char	  *ptr;
    int	    i;
    int	    nbytes;
    LINE    *lp1;
    LINE    *lp2;
    char    *line;

    /* Emulate the file read-in code, but from a string */

    line = (char *) malloc(MAXLINE);
    while (*str)
	{
	if (ptr = (const char *) strchr(str, '\n'))
	    nbytes = ptr - str;
	else
	    {
	    nbytes = strlen(str);
	    ptr = str + nbytes - 1;
	    }

	/* If the line is too long, wrap it */
	if (nbytes > MAXLINE-1)
	    {
	    nbytes = MAXLINE-1;
	    ptr = str + nbytes - 1;
	    }

	memcpy(line, str, nbytes);
	line[nbytes] = 0;
	str = ptr + 1;

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

	lp1 = emxlinit(nbytes);
	lp2 = bp->linep->prevp;
	lp2->nextp = lp1;
	lp1->nextp = bp->linep;
	lp1->prevp = lp2;
	bp->linep->prevp = lp1;
	memcpy(lp1->text, line, nbytes);
	lp1->used = nbytes;
	}

    FREE(line);
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


/* Set the current position to the given buffer offset */

static int setposition _ARGS1(long, pos)

{
    LINE    *lp;
    LINE    *blinep;

    blinep = g_curbp->linep;
    lp = blinep->nextp;
    while (lp != blinep && (long) lp->used < pos)
	{
	pos -= lp->used + 1;
	lp = lp->nextp;
	}

    if (lp == blinep)
	pos = 0;

    g_curwp->dotp = lp;
    g_curwp->doto = pos;
    g_curwp->flag |= WFMOVE;
    return TRUE;
}


void emxchangebegin _ARGS0()

{
    if ((g_cursorsave = g_cursorstate) == ON)
	{
	emxswitchcursor(OFF);
	}
}


void emxchangeend _ARGS0()

{
    emxupdate();
    if (g_cursorsave == ON)
	{
	emxswitchcursor(ON);
	}
}


/* Move the keyboard focus in here */

void emxfocusin _ARGS0()

{
#ifndef MOTIF
    XSetInputFocus(XtDisplay(emx), XtWindow(emx), RevertToParent, CurrentTime);
#else
    XmProcessTraversal((Widget)emx, XmTRAVERSE_CURRENT);
#endif
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


BUFFER *emxfindbuffer _ARGS2(const char *, name, Boolean, file)

{
    if (name)
	{
	if (file)
	    return emxbfindfile(name);
	else
	    return emxbfind(name, FALSE);
	}

    else
	return g_curbp;
}


/* make what's highlighted be the selection as well */
emxselecthighlight _ARGS0()

{
  REGION region;
  HIGHSEG *highp;

  if (highp = g_curbp->highlightp) {
    emxsetregion(&region, g_curbp->linep, highp->lp, highp->firsto,
       g_curbp->highlistp->lp, g_curbp->highlistp->endo);
    emxquickunselectall();
    emxselectrange(g_curbp, &region);
    emxbupdate(g_curbp, WFHARD);
    }

}



/*------------------*/
/* Public Functions */
/*------------------*/


int EmxShowBuffer _ARGS3(EmxWidget, w, const char *, name, Boolean, file)

{
    BUFFER  *bp;
    int	    result;

    emx = w;
    if (!(bp = emxfindbuffer(name, file)))
	return FALSE;

    emxchangebegin();
    result = emxusebufname(bp->bufname, FALSE);
    emxchangeend();
    return result;
}


int EmxCreateBuffer _ARGS4(EmxWidget, w, const char *, name, const char *, filename,
			   const char *, string)
{
    BUFFER  *bp;
    char    buf[MAXBUF];
    int     pos;
    int     digit;

    if (!name)
	return FALSE;

    emx = w;
    emxchangebegin();

    /* Look for a buffer with this name */
    strncpy(buf, name, MAXBUF - 4);
    buf[MAXBUF - 4] = 0;
    pos = strlen(buf);
    digit = 1;

    /* Make up a unique name */
    while (bp = emxbfind(buf, FALSE))
      sprintf(&buf[pos], "#%d", digit++);

    /* Now create it */
    bp = emxbfind(buf, TRUE);
    if (filename)
	strcpy(bp->filename, filename);

    /* Load it from the string */
    if (string)
	loadfromstring(bp, string);

    /* Make it current */
    emxusebuf(bp);
    setposition(0);
    emxchangeend();
    return TRUE;
}


int EmxRemoveBuffer _ARGS3(EmxWidget, w, const char *, name, Boolean, file)

{
    BUFFER  *bp;
    int	    result;

    emx = w;
    if (bp = emxfindbuffer(name, file))
	{
	emxchangebegin();
	result = emxdeletebuffer(bp);
	emxchangeend();
	return result;
	}

    return FALSE;
}


int EmxNextBuffer _ARGS1(EmxWidget, w)

{
    int	    result;

    emx = w;
    emxchangebegin();
    result = emxforwbuffer();
    emxchangeend();
    return result;
}


int EmxSetModified _ARGS4(EmxWidget, w, const char *, name, Boolean, file, 
			  int, state)
{
    BUFFER  *bp;

    emx = w;
    if (!(bp = emxfindbuffer(name, file)))
	return FALSE;

    emxchangebegin();
    if (state)
	{
	if (bp->flag & BFVIEW)
	    return FALSE;

	bp->flag |= BFCHG;
	}

    else
	bp->flag &= ~BFCHG;

    emxbupdate(bp, WFINFO);
    emxchangeend();
    return TRUE;
}


int EmxGetModified _ARGS3(EmxWidget, w, const char *, name, Boolean, file)

{
    BUFFER  *bp;

    emx = w;
    if (!(bp = emxfindbuffer(name, file)))
	return FALSE;

    return (bp->flag & BFCHG) ? TRUE : FALSE;
}


int EmxSetModifiable _ARGS4(EmxWidget, w, const char *, name, Boolean, file, 
			  int, state)
{
    BUFFER  *bp;

    emx = w;
    if (!(bp = emxfindbuffer(name, file)))
	return FALSE;

    emxchangebegin();
    if (state)
	bp->flag &= ~BFVIEW;
    else
	{
	/* Make the buffer read-only, and turn off the modified flag */
	bp->flag |= BFVIEW;
	bp->flag &= ~BFCHG;
	}

    emxbupdate(bp, WFINFO);
    emxchangeend();
    return TRUE;
}


int EmxGetModifiable _ARGS3(EmxWidget, w, const char *, name, Boolean, file)

{
    BUFFER  *bp;

    emx = w;
    if (!(bp = emxfindbuffer(name, file)))
	return FALSE;

    return (bp->flag & BFVIEW) ? FALSE : TRUE;
}


int EmxSetString _ARGS4(EmxWidget, w, const char *, name, Boolean, file,
			const char *, string)
{
    BUFFER  *bp;

    emx = w;
    if (!(bp = emxfindbuffer(name, file)))
	return FALSE;

    if (bp->flag & BFVIEW)
	return FALSE;

    /* Erase the buffer and replace the contents */
    emxchangebegin();
    emxunselectbuf(bp);
    emxbclear(bp);
    if (string)
	loadfromstring(bp, string);

    emxbchange(bp, WFHARD);
    emxbgoto(bp, bp->linep->nextp, 0);
    emxchangeend();
    return TRUE;
}


int EmxAppendString _ARGS4(EmxWidget, w, const char *, name, Boolean, file,
			   char *, string)
{
    BUFFER  *bp;
    int	    result;

    emx = w;
    if (!(bp = emxfindbuffer(name, file)))
	return FALSE;

    if (bp->flag & BFVIEW)
	return FALSE;

    /* Move to the end of the buffer and insert the string */
    emxchangebegin();
    emxbgoto(bp, bp->linep, 0);
    emxunselectbuf(bp);
    result = emxlinsertstring(strlen(string), string);
    emxbgoto(bp, bp->linep, 0);

    emxchangeend();
    return result;
}


int EmxShowMsg _ARGS2(EmxWidget, w, const char *, string)

{
    emx = w;
    emxchangebegin();
    emxmsgprint(string);
    emxchangeend();
    return TRUE;
}


int EmxGetString _ARGS4(EmxWidget, w, const char *, name, Boolean, file,
			char **, strpp)
{
    BUFFER  *bp;
    char    *ptr;
    LINE    *lp;
    long    count;

    emx = w;
    if (!(bp = emxfindbuffer(name, file)))
	return FALSE;

    count = getposition(bp, bp->linep, 0);
    *strpp = (char *) malloc(count + 1);
    ptr = *strpp;
    for (lp = bp->linep->nextp; lp != bp->linep; lp = lp->nextp)
	{
	if (lp->used)
	    {
	    memcpy(ptr, lp->text, lp->used);
	    ptr += lp->used;
	    }

	*ptr++ = '\n';
	}

    *ptr = 0;
    return TRUE;
}


int EmxGetBufname _ARGS3(EmxWidget, w, char *, name, char *, bufp)

{
    BUFFER  *bp;

    emx = w;
    if (!(bp = emxfindbuffer(name, TRUE)))
	return FALSE;

    strcpy(bufp, bp->bufname);
    return TRUE;
}


int EmxGetFilename _ARGS3(EmxWidget, w, const char *, name, char *, bufp)

{
    BUFFER  *bp;

    emx = w;
    if (!(bp = emxfindbuffer(name, FALSE)))
	return FALSE;

    strcpy(bufp, bp->filename);
    return TRUE;
}


char **EmxGetMatchingFilenames _ARGS3(EmxWidget, w, char *, pattern, int, pos)

{
    BUFFER  *bp;
    char    **listp;
    char    **namepp;
    int	    count;

    emx = w;

    /* Count the buffers with filenames */
    count = 0;
    for (bp = g_bheadp; bp; bp = bp->nextp)
	{
	if (bp->filename[0])
	    count++;
	}

    if (!count)
	return NULL;

    listp = (char **) malloc((count + 1) * sizeof(char *));
    namepp = listp;
    for (bp = g_bheadp; bp; bp = bp->nextp)
	{
	if (bp->filename[0])
	    {
	    if (pattern)
		{
		/* pos == -1: just find the pattern somewhere */
		if (pos < 0)
		    {
		    if (!strstr(bp->filename, pattern))
			continue;
		    }
		else
		    {
		    /* Find the pattern at the given position */
		    if (pos > (int) strlen(bp->filename) ||
			strncmp(&bp->filename[pos], pattern, strlen(pattern)) != 0)
			continue;
		    }
		}

	    *namepp = (char *) malloc(strlen(bp->filename) + 1);
	    strcpy(*namepp, bp->filename);
	    namepp++;
	    }

	}

    *namepp = NULL;
    return listp;
}


int EmxGetPosition _ARGS2(EmxWidget, w, long *, posp)

{
    emx = w;
    *posp = getposition(g_curbp, g_curwp->dotp, g_curwp->doto);
    return TRUE;
}


int EmxGetLastPosition _ARGS2(EmxWidget, w, long *, posp)

{
    emx = w;
    *posp = getposition(g_curbp, g_curbp->linep, 0);
    return TRUE;
}


int EmxSetPosition _ARGS2(EmxWidget, w, long, pos)

{
    int	    result;

    emx = w;
    emxchangebegin();
    result = setposition(pos);
    emxchangeend();
    return result;
}


int EmxGetSelectionPosition _ARGS3(EmxWidget, w, long *, startp, long *, endp)

{
    emx = w;
    return getselectpos(g_curbp, startp, endp);
}


int EmxGetSelectedString _ARGS2(EmxWidget, w, char **, strpp)

{
    emx = w;
    return emxgetselectstring(g_curbp, strpp);
}


int EmxSetSelection _ARGS3(EmxWidget, w, long, start, long, end)

{
    WINDOW  *cwp;
    REGION  region;

    emx = w;

    setposition(start);

    /* Set the mark at the beginning of the region */
    cwp = g_curwp;
    cwp->markp = cwp->dotp;
    cwp->marko = cwp->doto;

    setposition(end);

    emxchangebegin();
    emxsetregion(&region, g_curbp->linep, cwp->markp, cwp->marko,
	      cwp->dotp, cwp->doto);
    emxselectrange(g_curbp, &region);
    emxchangeend();
    return TRUE;
}


/* Set the search string and default to search forward */

void EmxSetSearchString _ARGS2(EmxWidget, w, const char *, string)

{
    emx = w;
    strcpy(g_pattern, string);
}


int EmxInsertString _ARGS2(EmxWidget, w, const char *, string)

{
    int	    result;

    emx = w;
    if (g_curbp->flag & BFVIEW)
	return FALSE;

    emxchangebegin();
    emxunselectbuf(g_curbp);
    result = emxlinsertstring(strlen(string), string);
    emxchangeend();
    return result;
}


int EmxChangeFilename _ARGS5(EmxWidget, w, const char *, oldname, Boolean, file,
			     const char *, newname, const char *, newbufname)
{
    BUFFER  *bp, *bufp;
    int     digit;
    int     pos;
    char    buf[MAXBUF];

    if (!oldname || !newname)
	return FALSE;

    emx = w;
    if (!(bp = emxfindbuffer(oldname, file)))
	return FALSE;

    /* Apply the new filename and generate a unique buffer name */
    strncpy(bp->filename, newname, MAXFILE);
    bp->filename[MAXFILE - 1] = 0;

    if (newbufname)
      strcpy(buf, newbufname);
    else
      emxmakebufname(buf, bp->filename);

    buf[MAXBUF - 4] = 0;
    pos = strlen(buf);
    digit = 1;
    while (bufp = emxbfind(buf, FALSE))
      {
      if (bufp == bp)
	break;

      sprintf(&buf[pos], "#%d", digit++);
      }

    strcpy(bp->bufname, buf);

    /* Now effect the change visually */
    if (bp->windows)
	{
	emxchangebegin();
	emxbupdate(bp, WFINFO);
	emxchangeend();
	}

    return TRUE;
}


int EmxExecuteCommand _ARGS2(EmxWidget, w, char *, cmd)

{
    int	    result;

    emx = w;
    emxchangebegin();
    strcpy(g_funcname, cmd);
    result = emxextendedcommandcomplete(TRUE);
    if (!emx->core.being_destroyed)
        emxchangeend();

    return result == TRUE ? TRUE : FALSE;
}


/* find the string, setting it as the selection if found */

Boolean EmxFindStr _ARGS3(EmxWidget, w, const char *, str, Boolean, forward)

{
  Boolean result;

  emx = w;
  emxchangebegin();
  emxunselectbuf(g_curbp);
  strcpy(g_pattern, str);
  g_srchall = FALSE;
  g_srchlastdir = forward? SRCH_FORW : SRCH_BACK;
  result = emxsearchagain();
  if (result) {
    emxselecthighlight();
  }
  emxchangeend();
  return result;
}


Boolean EmxDeleteSelection _ARGS1(EmxWidget, w)

{
  Boolean result;

  emx = w;
  emxchangebegin();
  result = emxcutselected();
  emxchangeend();
  return result;
}


