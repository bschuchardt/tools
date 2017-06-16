/* N i g g l e   E d i t o r   M a i n  M o d u l e -----------------------*/

/*
 * name - %M%
 * ccsid - %W% - %G% %U%
 * from  - %F%
 * date  - %H% %T%
 */


#include "types.mh"
#include "defn.mh"
#include "buffer.h"
#include "mgetc.h"

extern char *getenv();

#ifdef MSDOS
getCurrentVideoAttr() {
  char far *lstr;

  /* address page 1 of video memory and get the first attribute byte */
  lstr = (char far *)((unsigned long)0xb8000000 +
            (2 * 80 * (wherey() - 1)) + (2 * (wherex() - 1)));
  lstr++;
  videoAttr = *lstr;
}
#endif

char *getHome() {
    char *home = getenv("HOME");
    if (home == 0)
        home = getenv("USERPROFILE");
    return home;
}

processEnvironment(e)
  char *e;
  {

  if (e) {
    while (*e) {
      switch (*e) {
      case 'r':
        regularExpressions = TRUE;
        break;
      case 'b':
        backupFiles = TRUE;
        break;
      case 'c':
        caseSensitive = TRUE;
        break;
      case 'i':
        autoIndent = TRUE;
        break;
      case 'l':
        lockFiles = TRUE;
        break;
      case 't':
        useTabs = TRUE;
        break;
      case 'w':
        wrapOn = TRUE;
        break;
      case 'R':
        regularExpressions = FALSE;
        break;
      case 'B':
        backupFiles = FALSE;
        break;
      case 'C':
        caseSensitive = FALSE;
        break;
      case 'I':
        autoIndent = FALSE;
        break;
      case 'L':
        lockFiles = FALSE;
        break;
      case 'T':
        useTabs = FALSE;
        break;
      case 'W':
        wrapOn = FALSE;
        break;
      default:
        fprintf(stderr, "illegal env character - %c\n", *e);
        exit(1);
        break;
      }  /* end switch */
      e++;
    }  /* end if */
  }  /* end while */
#ifdef MSDOS  /* get the video attributes */
  getCurrentVideoAttr();
#endif
}




/* m a i n------------------------------------------------------*/

/*****/
main(argc, argv)
  int argc;
  char *argv[];
{
  int i, x, y, firstArgv;
  aBuffer *buff;

  gotoAlternateScreen();

  puts("---  niggle v3.4");
  puts("              (c) Bruce Schuchardt 1984 - 2009  ---");

  if (argc < 2) {
    gotoPrimaryScreen();
    printf("\nusage is \"niggle [-bBcCiIlLrRtTwW] [+line] <fileName>\"\n");
    goto fastExit;
  }

  firstArgv = 1;
  processEnvironment(getenv("NIGGLE"));

  if (argv[1][0] == '-' && argv[1][1]) {
    processEnvironment(argv[1]+1);
    firstArgv = 2;
  }
    
  if (getenv("BS8"))
    BS8=TRUE;

  /* initialize some buffers */
  lineBuff[0] =
  sString[0]  =
  rString[0]  = '\0';


  mgetcInit();                  /* gets terminal characteristics */

  for (x=0; x<=MaxScreenLines; x++)
    screen[x] = NULL;
  for (x=0; x<=terminalLines; x++) 	/* allocate the screen memory */
    screen[x] = myalloc(terminalColumns+1027);
                                 /* unallocated lines may be allocated in the
    				    edit.c module */

  isColor = (strcmp(getenv("TERM"), "color_xterm") == 0 ||
    strcmp(getenv("TERM"), "cx") == 0 ||
    getenv("COLOR_TERMINAL") != 0);

  /* load initial files */
  numFilesNotFound = 0;
  numFilesNotLocked = 0;
  y = 0;
  for (x=firstArgv; x<argc; x++)  {
    /* get initial line number for file (to be compatible with 'less') */
    if (argv[x][0] == '+'  &&  (y=atoi(argv[x]+1)) != 0)
      continue;

    if (argv[x][0] == '+' && argv[x][1] == 'l') {
      x++;
      y = atoi(argv[x]);
      continue;
    }

    buff = readFile(argv[x], TRUE);
    if (x < 4) newWindow(buff);

    if (y) {
      for (y--; y>0; y--)
        if (!buff->dotLine->isEof)
          { buff->dotLine = buff->dotLine->next; }
      }
    y = 0;
  }

  if (!buff)  {
    gotoPrimaryScreen();
    puts("no files to edit...");
    goto fastExit;
  }

  massign();  /* assign a channel for terminal entry */
  equWindowSizes();
  createCutBuffer();


  /* select the first window for editing */
  selectWindow(windows);

retry:
  /* make sure we have the whole screen */
  setScroll(0,terminalLines);
  saveTerminalLines = terminalLines;

  /* go do the edit */
  initializing = FALSE;
  edit();

  /* clean up */
  if (termType == VT100TERM) {
    /* for VT100 terminals, set scrolling region to whole screen */
    setScroll(0,terminalLines);
  }
  else {
    /* for SUN terminals, set scroll to value a "reset" would give.
       Note that Curses endwin() is suppose to reset the terminal
       properly, but it does not.  It sets the terminal scrolling
       parameter to 34, which causes the screen to clear on a scroll */
#ifndef MSDOS
    fputs("\033[1r", stdout);
#endif
  }

  positionCursor(0,0);
  eraseScreen();

  mdeassign();

  if (!quitProgram) if (!writeModifiedBuffers(TRUE)) {
    massign();
    goto retry;
  }

  gotoPrimaryScreen();

fastExit:
  exit(0);
}
