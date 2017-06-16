#ifndef EMX_H
#define EMX_H
/*=========================================================================
 * Copyright (C) Servio Corp. 1991-1994.  All Rights Reserved.
 *
 * Name - 
 *
 * Purpose -
 *
 * $Id: Emx.h,v 30.6 1994/08/18 00:19:09 marcs Exp $
 *
 *========================================================================*/

extern WidgetClass  emxWidgetClass;

typedef struct _EmxClassRec *EmxWidgetClass;
typedef struct _EmxRec	    *EmxWidget;


/* Resource Names and Classes */

#define XtNallowBufWrites	"allowBufWrites"
#define XtNautoBackup		"autoBackup"
#define XtNautoMatch		"autoMatch"
#define XtNcaseSensitive	"caseSensitive"
#define XtNcolumns		"columns"
#define XtNcursorStyle		"cursorStyle"
#define XtNcustomization	"customization"
#define XtNdefaultBack		"defaultBack"
#define XtNdefaultMode		"defaultMode"
#define XtNfillColumn		"fillColumn"
#define XtNfillDouble		"fillDouble"
#define XtNinsert		"insert"
#define XtNmanStyle		"manStyle"
#define XtNpushNewlines		"pushNewlines"
#define XtNquitCallback		"quitCallback"
#define XtNreadCallback		"readCallback"
#define XtNrows			"rows"
#define XtNsaveReminder		"saveReminder"
#define XtNsearchAll		"searchAll"
#define XtNstandAlone		"standAlone"
#define XtNtabsOnIndent		"tabsOnIndent"
#define XtNtabset		"tabset"
#define XtNtrimOutput		"trimOutput"
#define XtNuseDefaultSize	"useDefaultSize"
#define XtNverticalScroll	"verticalScroll"
#define XtNwordWrap		"wordWrap"
#define XtNwriteCallback	"writeCallback"

#ifndef XtNargc
#define XtNargc			"argc"
#endif
#ifndef XtNargv
#define XtNargv			"argv"
#endif

#define XtCAllowBufWrites	"AllowBufWrites"
#define XtCAutoBackup		"AutoBackup"
#define XtCAutoMatch		"AutoMatch"
#define XtCCaseSensitive	"CaseSensitive"
#define XtCColumns		"Columns"
#define XtCCursorStyle		"CursorStyle"
#define XtCCustomization	"Customization"
#define XtCDefaultBack		"DefaultBack"
#define XtCDefaultMode		"DefaultMode"
#define XtCFillColumn		"FillColumn"
#define XtCFillDouble		"FillDouble"
#define XtCInsert		"Insert"
#define XtCManStyle		"ManStyle"
#define XtCPushNewlines		"PushNewlines"
#define XtCRows			"Rows"
#define XtCSaveReminder		"SaveReminder"
#define XtCSearchAll		"SearchAll"
#define XtCStandAlone		"StandAlone"
#define XtCTabsOnIndent		"TabsOnIndent"
#define XtCTabset		"Tabset"
#define XtCTrimOutput		"TrimOutput"
#define XtCUseDefaultSize	"UseDefaultSize"
#define XtCVerticalScroll	"VerticalScroll"
#define XtCWordWrap		"WordWrap"

#ifndef XtCArgc
#define XtCArgc			"Argc"
#endif
#ifndef XtCArgv
#define XtCArgv			"Argv"
#endif


typedef struct
    {
    int	    status;
    char    *filename;
    char    *bufname;
    } EmxFileCallback;


int EmxAppendString (
#if defined(FLG_PROTOTYPE)
    EmxWidget w, const char *name, Boolean file, char *string
#endif
    );

int EmxChangeFilename (
#if defined(FLG_PROTOTYPE)
    EmxWidget w, const char *oldname, Boolean file, const char *newname,
    const char *newbufname
#endif
    );

int EmxCreateBuffer (
#if defined(FLG_PROTOTYPE)
    EmxWidget w, const char *name, const char *filename, const char *string
#endif
    );

int EmxExecuteCommand (
#if defined(FLG_PROTOTYPE)
    EmxWidget w, char *cmd
#endif
    );

int EmxGetBufname (
#if defined(FLG_PROTOTYPE)
    EmxWidget w, char *name, char *bufp
#endif
    );

int EmxGetFilename (
#if defined(FLG_PROTOTYPE)
    EmxWidget w, const char *name, char *bufp
#endif
    );

int EmxGetLastPosition (
#if defined(FLG_PROTOTYPE)
    EmxWidget w, long *posp
#endif
    );

char **EmxGetMatchingFilenames (
#if defined(FLG_PROTOTYPE)
    EmxWidget w, char *pattern, int pos
#endif
    );

int EmxGetModifiable (
#if defined(FLG_PROTOTYPE)
    EmxWidget w, const char *name, Boolean file
#endif
    );

int EmxGetModified (
#if defined(FLG_PROTOTYPE)
    EmxWidget w, const char *name, Boolean file
#endif
    );

int EmxGetPosition (
#if defined(FLG_PROTOTYPE)
    EmxWidget w, long *posp
#endif
    );

int EmxGetSelectedString (
#if defined(FLG_PROTOTYPE)
    EmxWidget w, char **strpp
#endif
    );

int EmxGetSelectionPosition (
#if defined(FLG_PROTOTYPE)
    EmxWidget w, long *startp, long *endp
#endif
    );

int EmxGetString (
#if defined(FLG_PROTOTYPE)
    EmxWidget w, const char *name, Boolean file, char **strpp
#endif
    );

int EmxInsertString (
#if defined(FLG_PROTOTYPE)
    EmxWidget w, const char *string
#endif
    );

void EmxKeyString (
#if defined(FLG_PROTOTYPE)
    int keysym,
    int modifier,
    char *bufp
#endif
    );

int EmxNextBuffer (
#if defined(FLG_PROTOTYPE)
    EmxWidget w
#endif
    );

int EmxRemoveBuffer (
#if defined(FLG_PROTOTYPE)
    EmxWidget w, const char *name, Boolean file
#endif
    );

int EmxSetModifiable (
#if defined(FLG_PROTOTYPE)
    EmxWidget w, const char *name, Boolean file, int state
#endif
    );

int EmxSetModified (
#if defined(FLG_PROTOTYPE)
    EmxWidget w, const char *name, Boolean file, int state
#endif
    );

int EmxSetPosition (
#if defined(FLG_PROTOTYPE)
    EmxWidget w, long pos
#endif
    );

void EmxSetSearchString (
#if defined(FLG_PROTOTYPE)
    EmxWidget w, const char *string
#endif
    );

int EmxSetSelection (
#if defined(FLG_PROTOTYPE)
    EmxWidget w, long start, long end
#endif
    );

int EmxSetString (
#if defined(FLG_PROTOTYPE)
    EmxWidget w, const char *name, Boolean file, const char *string
#endif
    );

int EmxShowBuffer (
#if defined(FLG_PROTOTYPE)
    EmxWidget w, const char *name, Boolean file
#endif
    );

int EmxShowMsg (
#if defined(FLG_PROTOTYPE)
    EmxWidget w, const char *string
#endif
    );

Boolean EmxFindStr (
#if defined(FLG_PROTOTYPE)
    EmxWidget w, const char * str, Boolean forward
#endif
    );

Boolean EmxDeleteSelection (
#if defined(FLG_PROTOTYPE)
    EmxWidget w
#endif
    );

#endif /* EMX_H */
