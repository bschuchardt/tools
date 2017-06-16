/*
 * name - %M%
 * ccsid - %W% - %G% %U%
 * from  - %F%
 * date  - %H% %T%
 */



#include <stdio.h>
#include "types.mh"
#include "defn.h"
#include "winch.h"

#ifdef VMS
#include stdlib
#endif

#ifdef UNIX
#if defined(LINUX) & !defined(MACOS)
#include <asm/termbits.h>
#endif
#if defined(MACOS)
#include <term.h>
#include <sys/termios.h>
#include <sgtty.h>
#endif
#if defined(CYGNUS)
#include <termios.h>
#else
#include <sgtty.h>
#endif

#include <sys/ioctl.h>

#if defined(MACOS)
#include <termcap.h>
#else
/* in lieu of a termcap.h file in SunOS, ...*/
extern int tgetent();
extern int tgetnum();
extern int tgetstr();
/* and, in lieu of a file defining getenv(), ...*/
extern char *getenv();
#endif

char *termcapbuff = NULL;
char resetString[128];
#endif


char *ttyname;
boolean waitingForKBFlag;
int old_vmin;   /* termio MIN value */
int old_vtime;  /* termio MAX value */

#ifdef UNIX
static char *getTermcapBuff() {

  termcapbuff = myalloc(8192);
  if (!termcapbuff) {
    exit(2);
  }

#if defined(MACOS)
  ttyname = "xterm-256color";
#else
  if ((ttyname=getenv("TERM")) == NULL  &&
      (ttyname=getenv("term")) == NULL) {
    /* it would be better to have this error message in massign() */
    puts("unknown terminal type - vt100 assumed");
    ttyname = "vt100";
  }
#endif

  if (0 > tgetent(termcapbuff, ttyname)) {
    puts("out of memory");
    exit(2);
  }
  return termcapbuff;
}


static getResetString() {  /* get the terminal reset string out of termcapbuff */
  char *ptr;
  
  ptr = resetString;
  resetString[0] = '\0';
  tgetstr("rs", &ptr);
}

#endif




void mgetcInit() {
#ifdef VMS
  unsigned long devflags;
  unsigned char devtype;
#endif
  unsigned short devColumns, devLines;
  char *tmp;

#if defined(UNIX)
#if !defined(CYGNUS)
  winchInit();
#endif
  if (!termcapbuff) {
    if (getTermcapBuff()) {
      getResetString();
    
      if (strcmp(ttyname, "xterm") != 0 &&  /* xterms are vt100 compatible */
          strcmp(ttyname, "xterms") != 0 &&
          strcmp(ttyname, "color_xterm") != 0 &&
          strcmp(ttyname, "cx") != 0 &&
          strcmp(ttyname, "cygwin") != 0 &&
          strcmp(ttyname, "dtterm") != 0 &&
          (ttyname[0] != 'v'  && ttyname[0] != 'V'))
        termType = SUNTERM;
    }
    else
      resetString[0] = '\0';
  }

  /* now ignore row/column info in termcap and get the real window size */
  ttsize();
#endif

#ifdef VMS
  winchInit();
  LIB$SCREEN_INFO(&devflags,&devtype,&devColumns,&devLines);
  ttyname = getenv("TERM");
  if (ttyname[0] != 'v'  && ttyname[0] != 'V') termType = SUNTERM;
  terminalLines = saveTerminalLines = devLines - 1;
  terminalColumns = devColumns - 2;
  ttsize();
#endif


#ifdef MSDOS
  termType = VT100TERM;
#endif
}





void massign() {
#if defined(CYGNUS) | defined(LINUX) & !defined(MACOS)
  struct termio newSettings;
  ioctl(fileno(stdin), TCGETA, (char *)(&newSettings));
  newSettings.c_lflag &= ~ISIG; /* no signals */
  newSettings.c_lflag &= ~ICANON;  /* no character merging (immediate reads) */
  old_vmin = newSettings.c_cc[VMIN];
  old_vtime = newSettings.c_cc[VTIME];
  newSettings.c_cc[VMIN] = 1;
  newSettings.c_cc[VTIME] = 0;
  newSettings.c_lflag &= ~ECHO; /* no echoing */
  ioctl(fileno(stdin), TCSETA, (char *)(&newSettings));
#else
#if defined(UNIX) | defined(MACOS)
  struct sgttyb newSettings;
  ioctl(fileno(stdin), TIOCGETP, (char *)(&newSettings));
  newSettings.sg_flags |= RAW;
  newSettings.sg_flags &= ~ECHO;
  ioctl(fileno(stdin), TIOCSETP, (char *)(&newSettings));
#endif
#endif

  promptLine = terminalLines - 1;
  if (termType == VT100TERM)
    fputs("\033=", stdout);  /* enter application keypad mode */
}


static byte t_getch() {
  byte rtnVal;

  waitingForKBFlag = TRUE;
#if !defined(UNIX)
  rtnVal = (byte)getch();
#else
  rtnVal = (byte)getchar();
#endif
  waitingForKBFlag = FALSE;
  return rtnVal;
}


byte mgetc() {
  byte result;

restart:
  if (playback) {
    if (playbackIdx >= recordSize) {
      playback = FALSE;
      displayStatus();
    }
    else {
      result = recordBuffer[playbackIdx++];
    }
  }
  if (!playback) {
    result = t_getch();
#ifdef SIMULATE_XON
    if (result == 19) { /* control-s */
      while ((result=t_getch()) != 17) {}  /* wait for control-q */
      goto restart;
    }
    if (result == 17) goto restart;  /* skip control-q's too */
#endif
    if (result == (byte)0xff) goto restart;  /* weird stuff with SIGWINCH */
    if (recording) {
      if (recordSize >= stdTextSize) {
        recording = FALSE;
        displayStatus();
        notify("out of recording space - recording turned off");
      }
      else {
        recordBuffer[recordSize++] = result;
      }
    }
  }
  return result;
}


void mdeassign() {
/* this was "if defined(UNIX) & !defined(CYGNUS) & !defined(LINUX) */
#if defined(MACOS)
  struct sgttyb newSettings;
  
  ioctl(fileno(stdin), TIOCGETP, (char *)(&newSettings));
  newSettings.sg_flags &= ~RAW;
  newSettings.sg_flags |= ECHO;
  newSettings.sg_flags |= CRMOD;
//#if !defined(SOLARIS) /* solaris 2.7 */
//  newSettings.c_cc[VMIN] = old_vmin;
//  newSettings.c_cc[VTIME] = old_vtime;
//#endif
  ioctl(fileno(stdin), TIOCSETP, (char *)(&newSettings));

/**** dumb vt100 termcap reset string causes screen to always
 **** go to white on black.  Can't use resetString...
  fputs(resetString, stdout);
 ****/

#else
#if defined(CYGNUS) | defined(LINUX)
  struct termio newSettings;
  ioctl(fileno(stdin), TCGETA, (char *)(&newSettings));
  newSettings.c_lflag |= ISIG; /* signals */
  newSettings.c_lflag |= ICANON;
  newSettings.c_lflag |= ECHO; /* echoing */
  ioctl(fileno(stdin), TCSETA, (char *)(&newSettings));
#else
  if (termType == VT100TERM)
    fputs("\033>", stdout);    /* exit application keypad mode */
#endif
#endif
}




