#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/cursorfont.h>
#ifdef MOTIF
#include <Xm/RowColumn.h>
#include <Xm/PushBG.h>
#endif
#include "EmxI.h"

extern exit();
extern void Quit();
extern void Read();
extern void Write();


XtAppContext	appcontext;
Widget		toplevel, editor;
char		buf[256];


void Shutdown (data, id)

int data;
int id;

{
#ifdef PROFILE
    XtDestroyApplicationContext(appcontext);
#endif
    exit(0);
}


void Quit (w, event, params, numparams)

Widget	    w;
XEvent	    *event;
String	    *params;
Cardinal    *numparams;

{
    XtDestroyWidget(toplevel);
    XtAddTimeOut(10, Shutdown, 0);
}


void Read (w, datap, callp)

Widget	    w;
caddr_t	    datap;
EmxFileCallback *callp;

{
    /* Load a bogus buffer */
    if (!EmxShowBuffer(w, callp->filename, TRUE))
	EmxCreateBuffer(w, callp->filename, callp->filename, callp->filename);

    callp->status = TRUE;
}


void Write (w, datap, callp)

Widget	    w;
caddr_t	    datap;
EmxFileCallback *callp;

{
    /* Make it unmodified */
    EmxSetModified(w, callp->bufname, FALSE, FALSE);
    callp->status = TRUE;
}


main (argc, argv)

int	argc;
char	*argv[];

{
    Arg	    args[10];
    int	    i;
    int	    alone;
    int	    argcnt;
    char    title[256];
    char    **listp;
    char    **namepp;
    char    *ptr;

#ifdef MOTIF
    Widget  form, button, editor2;
#endif

    alone = TRUE;
    if (argc > 1 && strcmp(argv[1], "-x") == 0)
	alone = FALSE;

    argcnt = 1;
    toplevel = XtInitialize(argv[0], "Emxw", NULL, 0, &argcnt, &argv);

    appcontext = XtWidgetToApplicationContext(toplevel);

    XtVaSetValues(toplevel, XtNallowShellResize, TRUE,
      XtNinput, TRUE, NULL);

#ifdef MOTIF
    i = 0;
    XtSetArg(args[i], XmNorientation, XmVERTICAL); i++;
    form = XtCreateManagedWidget("rowcol", xmRowColumnWidgetClass, toplevel, args, i);

    i = 0;
    button = XtCreateManagedWidget("Quit", xmPushButtonGadgetClass,
				   form, args, i);

    i = 0;
    XtSetArg(args[i], XtNargc, argc); i++;
    XtSetArg(args[i], XtNargv, argv); i++;
    XtSetArg(args[i], XtNrows, 24); i++;
    XtSetArg(args[i], XtNstandAlone, alone); i++;
    editor = XtCreateManagedWidget("editor", emxWidgetClass,
				 form, args, i);
    i = 0;
    XtSetArg(args[i], XtNrows, 24); i++;
    XtSetArg(args[i], XtNstandAlone, alone); i++;
    editor2 = XtCreateManagedWidget("editor2", emxWidgetClass,
				 form, args, i);
#else
    i = 0;
    XtSetArg(args[i], XtNargc, argc); i++;
    XtSetArg(args[i], XtNargv, argv); i++;
    XtSetArg(args[i], XtNstandAlone, alone); i++;
    editor = XtCreateManagedWidget("editor", emxWidgetClass,
				   toplevel, args, i);
    
#endif

#ifdef MOTIF
    XtAddCallback(button, XmNactivateCallback, Quit, NULL);
    XtAddCallback(editor2, XtNquitCallback, Quit, NULL);
#endif

    XtAddCallback(editor, XtNquitCallback, Quit, NULL);
    XtAddCallback(editor, XtNreadCallback, Read, NULL);
    XtAddCallback(editor, XtNwriteCallback, Write, NULL);

    /* Set up the title */
    title[0] = 0;
    if (listp = (char **) EmxGetMatchingFilenames(editor, NULL, 0))
	{
	for (namepp = listp; *namepp; namepp++)
	    {
	    if (ptr = (char *) strrchr(*namepp, '/'))
		ptr++;
	    else if (ptr = (char *)strrchr(*namepp, '\\'))
                ptr++;
            else
		ptr = *namepp;

	    if (strlen(title) < 100)
		{
		strcat(title, ptr);
		strcat(title, " ");
		}

	    free(*namepp);
	    }

	free(listp);
	}

    else
	strcpy(title, "EMX");

    i = 0;
    XtSetArg(args[i], XtNtitle, title); i++;
    XtSetArg(args[i], XtNiconName, argv[0]); i++;
    XtSetValues(toplevel, args, i);

    /* Default focus */
#ifdef MOTIF
    XmAddTabGroup(editor);
    XmAddTabGroup(button);
#else
    XtSetKeyboardFocus(toplevel, editor);
#endif

    /* Show and go */
    XtRealizeWidget(toplevel);

    XDefineCursor(XtDisplay(editor), XtWindow(editor),
                    XCreateFontCursor(XtDisplay(editor), XC_xterm));

    XtMainLoop();
}
