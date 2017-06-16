
/*
 * name - %M%
 * ccsid - %W% - %G% %U%
 * from  - %F%
 * date  - %H% %T%
 * dynamic window sizing for NIGGLE/UNIX/Xwindows */

#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>

#if defined(HPUX)
#include <sys/termio.h>
#else
#if defined(SOLARIS) | defined(CYGNUS) | defined(MACOS)
#include <sys/termios.h>
#else
#if defined(LINUX)
#include <asm/termios.h>
#else
#include <sgtty.h>
#endif
#endif
#endif

#include "types.mh"
#include "defn.h"
#include "mgetc.h"

#if !defined(CYGNUS)
extern procWinch();
#endif

#ifndef SUN
volatile int winchReceivedFlag = 0;
#else
int winchReceivedFlag = 0;
#endif

void ttsize() {
  struct winsize win;
  word devColumns, devLines;

  /* default to sun console since it seems to fail the cwinsz and csize
   * requests */

  devColumns = 80;
  devLines   = 34;
  
  if (ioctl (0, TIOCGWINSZ, &win) == 0) {
    if (win.ws_col)
      devColumns = win.ws_col;
    if (win.ws_row)
      devLines = win.ws_row;
}
  else {
    devColumns = 80;
    devLines = 24;
}
  /*
   * bounds check for absurd values
   */
  if (devColumns >= stdTextSize)
    devColumns = 80;
  if (devLines > MaxScreenLines)
    devLines = 24;
  if (devColumns < 4)
    devColumns = 80;
  if (devLines < 4)
    devLines = 24;

  terminalLines = saveTerminalLines = devLines - 1;
  terminalColumns = devColumns - 1;
}

#if !defined(CYGNUS)
procWinch () {
  int i, size;
  char str[80];

  winchReceivedFlag = 0;
  ttsize();
  for (i=0; i<=terminalLines; i++) {
    if (screen[i] != NULL)
      free(screen[i]);
    screen[i] = myalloc(terminalColumns + 3);
  }
  promptLine = terminalLines - 1;
  equWindowSizes();
  remakeScreen();
  displayStatus();
  sprintf(str, "screen is %d lines by %d columns",
              terminalLines+1, terminalColumns+1);
  notify(str);
  signal (SIGWINCH, procWinch);
}

void winchInit () {
  winchReceivedFlag = 0;
  signal (SIGWINCH, procWinch);
}
#endif

