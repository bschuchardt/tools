/*----- m a i n   e d i t   f u n c t i o n -----------*/


/*
 * name - %M%
 * ccsid - %W% - %G% %U%
 * from  - %F%
 * date  - %H% %T%
 */



#include "types.mh"
#include "defn.h"
#include "buffer.h"
#include "mgetc.h"

#ifdef MSDOS
#include <alloc.h>		/* for coreleft() function */
#endif

int StickyF1, StickyF2, StickyF3;
boolean exitProgram;
int recSizeAfterLastCmd = 0;
unsigned long lastCoreLeft = 0;

extern char *getenv(const char *name);
extern void *malloc(size_t amount);
extern void *realloc(void *ptr, size_t size);

/* sprintf() description for spawned commands */
#ifdef UNIX
char spawnLine[] = "csh $HOME/.ngtemp1 1>$HOME/.ngtemp2 2>&1";
#endif


/*****/
int doSigInt(sig)
  int sig;
{
  signal(SIGINT,doSigInt);
  intAbort = TRUE;
}


/*****/
extern boolean waitingForKBFlag;  /* from mgetc.c module */

int doSigUSR1(sig)
  int sig;
{
  int w;

  /*
   * USR1 can be used to force a detached niggle to save its files and exit
   *
   * if the program isn't waiting for keyboard input, the state of the
   * active buffer is unknown, so it isn't saved
   */

  if (!waitingForKBFlag) {
    currentBuffer->modified = FALSE;
}
  w = waitingForKBFlag;

  writeModifiedBuffers(FALSE);

  /* in case someone is really looking at the output stream, print out some
   * indication of what's happening.  This is done after writing buffers
   * in case it kills our process
   */

  mdeassign();
  eraseScreen();
  fflush(stdout);
  positionCursor(0,0);
  fflush(stdout);
  puts("niggle was forced to exit with SIGUSR1");
  if (w)
    puts("  all modified buffers were saved");
  else
    puts("  all buf the active buffer was saved (its state was unstable)");

  exit(2);
}



/*****/
edit()
{
doSigInt();
  int x;
  char promptc();
  char tempstr[30];

  remakeScreen();
  mode = "command";
  StickyF1 = StickyF2 = StickyF3 = FALSE;
  displayStatus();
  if (numFilesNotFound > 0) {
    if (numFilesNotFound == 1)
      notify("1 file not found");
    else {
      sprintf(tempstr, "%d files not found", numFilesNotFound);
      notify(tempstr);
    }
    numFilesNotFound = 0;
  }
  else if (numFilesNotLocked > 0) {
    if (numFilesNotLocked == 1) {
      notify("1 file could not be write-locked");
    }
    else {
      sprintf(tempstr, "%d files could not be write-locked", numFilesNotLocked);
      notify(tempstr);
    }
  }

  signal(SIGINT,doSigInt);		/* interrupt processing */
  signal(SIGUSR1, doSigUSR1);

keepGoing:
  exitProgram = quitProgram = FALSE;
  while (!exitProgram && !quitProgram) {
    if (intAbort) {
      remakeScreen();
      displayStatus();
      redrawStatus = FALSE;
      intAbort = FALSE;
    }
#ifdef MSDOS
    showCoreLeft(FALSE);
#endif
    x = mgetc();
    repCount = 1;
    doCommand(x);
    if (recording)
      recSizeAfterLastCmd = recordSize;
}


  if (quitProgram  &&  anyModifiedBuffers()) {
    x = promptc("Are you sure you want to quit? ",TRUE);
    if (x != 'y'  && x != 'Y') {
      goto keepGoing;
    }
  }
#if defined(WIMPY_USER)
  else if (exitProgram) {
    x = promptc("Hit ^E to exit: ", TRUE);
    if (x != 5)
      goto keepGoing;
  }
#endif
}




/*****/
lower(aChar)
  int aChar;
{
  if (aChar <= 'Z'  &&  'A' <= aChar) return(aChar + 32);
  return(aChar);
}




/*****/
doCommand(val)
  int val;
{
  int x,y;
  char aChar;

  aChar = val & 0x7f;

  if (redrawStatus) displayStatus();
  if (!keepColumn) lastColumn = currentWindow->cursorColumn;
  keepColumn = FALSE;
doit:
  switch (aChar) {
#if !defined(MSDOS)
    case DELETE:
      if (!BS8)  /* backspace key generates a 128, but should deleteLeft */
        goto deleteLeft;
      for (x=1; x<=repCount && !intAbort; x++) {
        moveRight(1);
        setCursor();
	rubChar();
    }
      break;
    case BS:
deleteLeft:
#else
    case BS:
    case DELETE:
#endif
      for (x=1; x<=repCount && !intAbort; x++) rubChar();
      break;

#ifdef MSDOS
    case 0:	 /* msdos extended key set */
      doExtended(mgetc());
      break;
#endif

    case 1:  /* ^a */
      doF1('a');
      break;

    case 2:  /* ^b */
      doF1('b');
      break;

    case 3:  /* ^c */
      doF1('c');
      break;

    case 4:  /* ^d */
      doF1('d');
      break;

    case 5:  /* ^e */
      doF1('e');
      break;

    case 6: /* ^f */
      doF1('f');
      break;

    case 7:    /* set stickyF1 mode */
      StickyF1 = !StickyF1;
      displayStatus();
      break;

    case 11: /* ^k */
      doF1('k');
      break;

    case 12:
      doF1('l');
      break;

    case 14:	/* ^n - go up/down */
      doF1('n');
      break;

    case 16:
      doF1('p');
      break;

    case 18:
      remakeScreen();
      displayStatus();
      break;

    case 19:  /* ^s */
      doF1('s');
      break;
      
    case 21:  /* ^u */
      delToBeg();
      break;

    case 22: /* ^v */
      doF1('v');
      break;

    case 23:
      doF1('w');
      break;

    case 24: /* ^x */
      doF1('x');
      break;

    case 25: /* ^Y */
      doExit('q');
      break;

    case 26: /* ^z */
      /* ^z^z = exit */
      if (lastCommand == 26)
        doExit('x');
      else
        doF1('z');
      break;

    case 27:
      doEscChar(mgetc());
      break;

    case 28:    /* same as f1 */
      doF1(mgetc());
      break;

    case 29:    /* same as f2 */
      doF2(mgetc());
      break;

    case 30:    /* same as f3 */
      doF3(mgetc());
      break;

    case 31:  /* ^_ - adjust leading white space */
      if (currentBuffer->readOnly) {
        notifyReadOnly();
        break;
      }

      if (cmdDirection == '<') {
        for (x=1; x<=repCount && !intAbort  &&
                !currentBuffer->dotLine->isEof; x++) {
	  y = nonWhiteCol();
	  setTab(y - 4);
	  if (y > 0) tabAdjust(FALSE);
	  moveDown(1);
	}
      }
      else {
	for (x=1; x<=repCount  && !currentBuffer->dotLine->isEof && !intAbort; x++) {
	  y = nonWhiteCol();
	  setTab(y + 4);
          tabAdjust(FALSE);
	  moveDown(1);
	}
      }
      setCursor();
      break;

    default:
      if (StickyF1) doF1(aChar);
      else if (StickyF2) doF2(aChar);
      else if (StickyF3) doF3(aChar);
      else if (aChar != 0) repInsert(aChar);
      break;
    }

  lastCommand = aChar;
  return(0);
}



/*****/
doExit(aChar)
  char aChar;
{
  switch (aChar) {
  case 'x':  case 'X':
    exitProgram = TRUE;  quitProgram = FALSE;
    break;

  case 'a': case 'A': case 'q':  case 'Q':
    exitProgram = FALSE; quitProgram = TRUE;
    break;
}
}

/*****/
doEscChar(aChar)
  char aChar;
{
  int x;
  switch (aChar) {
    case 'O':
      doFunctKey(mgetc());
      break;
    case '1':
      doF1(mgetc());
      break;
    case '2':
      doF2(mgetc());
      break;
    case '3':
      doF3(mgetc());
      break;
    case '[':
      doBracketKey(mgetc());
      break;
    case 'f': /* fn-right on Mac */
      x = cmdDirection;
      cmdDirection = '>';
      moveWords();
      setCursor();
      cmdDirection = x;
      break;
    case 'b': /* fn-left on Mac */
      x = cmdDirection;
      cmdDirection = '<';
      moveWords();
      setCursor();
      cmdDirection = x;
      break;
    default:
      insertChar(aChar);
      break;
  }
}



/*****/
doBracketKey(aChar)
  char aChar;
  {
  int t,x,oldx,oldy;
  char c;

  switch (aChar) {
  case 'A':
    keepColumn = TRUE;
    moveUp(repCount);
    gotoColumn();
    setCursor();
    break;
  case 'B':
    keepColumn = TRUE;
    moveDown(repCount);
    gotoColumn();
    setCursor();
    break;
  case 'C':
    moveRight(repCount);
    setCursor();
    break;
  case 'D':
    moveLeft(repCount);
    setCursor();
    break;
  case 'F': /* xfree86 end */
    doF1('e');
    break;
  case 'H': /* xfree86 home */
    doF1('b');
    break;
  case 0:
    insertChar(mgetc());
    break;
  case '1':   /* DS or Sun F key? */
    x = mgetc();
    if (x == '~') { /* cygwin cmd home key */
      doF1('b');
      break;
    }
    if ('1' <= x  &&  x <= '9'  &&  '~' == (c=mgetc())) { /* F1 - F8 */
      switch(x) {
        case '1':
          doF1(mgetc());
          break;
        case '2':
          doF2(mgetc());
          break;
        case '3':
          doF3(mgetc());
          break;
        case '4':
          doF2('n');        /* next buffer */
          break;
        case '5':
          cmdDirection = '>';
          doCommand(31);
          break;
        case '6':
          insertChar(aChar);
          insertChar(x);
          insertChar(c);
          break;
        case '7':           /* cygwin, xwin f6 */
          cmdDirection = '<';
          doCommand(31);
          break;
        case '8':
          doF1('r');        /* f7 - replace */
          break;
        case '9':
          doF1('s');        /* f8 - search */
          break;
      }
    }
    else if ( x == ';' && '5' == mgetc() ) { /* xterm ctrl-arrow */
      x = mgetc();
      switch (x) {
        case 'A': /* up,down */
        case 'B':
            bell();
            break;
        case 'C': /* right */
            x = cmdDirection;
            cmdDirection = '>';
            moveWords();
            setCursor();
            cmdDirection = x;
            break;
        case 'D': /* left */
            x = cmdDirection;
            cmdDirection = '<';
            moveWords();
            setCursor();
            cmdDirection = x;
            break;
        default:
          bell();
          insertChar(x);
          break;
      }
    }
    else {
      insertChar(aChar);
      insertChar(x);
      bell();
  }
    break;
  case '2':   /* DEC or Sun Function key? */
    x = mgetc();
    if (x == '~')  /* DEC/Cygwin Insert key */
      bell();

    /* $[22 */
    else if (x == '2') {
      x = mgetc();
      mgetc();  /* assume it's 'z' (Sun Fkey) */
      switch (x) {
        case '0':  doF1('e');  break;      /* 386i end key */
        case '1':  doBracketKey('B'); break; /* 386i down key */
        case '2':  doF1(']'); break;       /* 386i PgDn key */
        case '4':  doF1(mgetc()); break;   /* old sun f1 */
        case '5':  doF2(mgetc()); break;   /* old sun f2 */
        case '6':  doF3(mgetc()); break;   /* old sun f3 */
        default: insertChar(aChar); insertChar(x); bell();
      }
    }
    /* $[21 */
    else if (x == '1') {
      x = mgetc();
      if (x == '~') { /* DS & Sunt4 F10 */
        doF1('c');
        break;
      }
      else c=mgetc();     /* assume it's a 'z' */
      switch(x) {
        case '4':  doF1('b'); break;       /* 386i home */
        case '5':  doBracketKey('A'); break; /* 386i up */
        case '6':  doF1('['); break;       /* 386i PgUp key */
        case '7':  doBracketKey('D'); break; /* 386i left */
        case '9':  doBracketKey('C'); break; /* 386i right */
        default: insertChar(aChar); insertChar(x); insertChar(c); bell();
      }
    }
    /* $[2x */
    else if ('0' <= x  &&  x <= '9') {  /* DS F9 - F14, help, Do */
     if ('~' != (c=mgetc())) {
       insertChar(aChar);
       insertChar(x);
       insertChar(c);
       bell();
       break;
     }
     switch (x) {
       case '0':        /* X-Windows F9 */
         doF1('z');     /* set mark */
         break;
       /* case '1':
         doF1('c');
         break;  removed for Cygwin support 12/98 */
       case '3':
         /* On ANSI keyboards, F11 is often mapped to Escape.  This key
            should not be relied upon for required functions */
         doF1('x');
         break;
       case '4':
         doF1('v');
         break;
       case '5':
         break;
       case '6':
         break;
       case '8':      /* help key */
         doF1('?');
         break;
      case '9':       /* DS Do key */
         doF3('D');
         break;
       default:
         insertChar(aChar);
         insertChar(x);
         insertChar(c);
         bell();
         break;
     }
  }
    else {
      insertChar(aChar);
      insertChar(x);
      bell();
  }
    break;

  case '3':         /* cygwin delete */
  case '4':         /* end */
  case '5':         /* pgup */
  case '6':         /* pgdn */
    if ('~' != (x=mgetc())) {
      insertChar(aChar);
      insertChar(x);
      bell();
  }
    else {
      switch (aChar) {
        case '1':
          doF1('b');
          break;
        case '3':
          /* xterm Delete key */
          for (x=1; x<=repCount && !intAbort; x++) {
            moveRight(1);
            setCursor();
            rubChar();
        }
          break;
        case '4':
          doF1('e');
          break;
        case '5':
          doF1('[');
          break;
        case '6':
          doF1(']');
          break;
      }
    }
    break;

  case '7':  /* color_xterm Home */
    c = mgetc(); /* twiddle */
    doF1('b');
    break;
  case '8':  /* color_xterm End */
    c = mgetc(); /* twiddle (~) */
    doF1('e');
    break;
    
  case '[':  /* cygwin F1-F5 */
    c = mgetc();
    switch (c) {
      case 'A':
        doF1(mgetc());
        break;
      case 'B':
        doF2(mgetc());
        break;
      case 'C':
        doF3(mgetc());
        break;
      case 'D':
        doF2('n');
        break;
      case 'E':
        cmdDirection = '>';
        doCommand(31);
        break;
      default:
        bell();
        break;
  }
    break;
  default:
    insertChar(aChar);
    bell();
  }
}



/*****/
#ifdef MSDOS
doExtended(aChar)
  unsigned char aChar;
  {
  int x;

  switch (aChar) {
    case 59:  doF1(mgetc()); break;
    case 60:  doF2(mgetc()); break;
    case 61:  doF3(mgetc()); break;
    case 63:  		/* f5 */
      doF1('s'); break;
    case 64:            /* f6 */
      doF1('r'); break;
    case 71:  		/* home */
      doF1('b'); break;
    case 72:  		/* up */
      doBracketKey('A'); break;
    case 73:  		/* pgup */
      doF1('['); break;
    case 75:  		/* left */
      doBracketKey('D'); break;
    case 77:  		/* right */
      doBracketKey('C'); break;
    case 79:            /* end */
      doF1('e'); break;
    case 80:  		/* down */
      doBracketKey('B'); break;
    case 81:  		/* pgdn */
      doF1(']'); break;
    case 82:  		/* insert */
      bell(); break;
    case 83:  		/* delete */
      for (x=1; x<=repCount && !intAbort; x++) {
	 moveRight(1);
         setCursor();
	 rubChar();
	 }
      break;
    case 118:		/* ctrl-pgdn */
      doF1('E'); break;
    case 132: 		/* ctrl-pgup */
      doF1('B'); break;
    default:
      bell(); break;
  }
}
#endif



/*****/
doFunctKey(aChar)
  char aChar;
  {
  int x;

  switch (aChar) {
  case 'A': case 'B': case 'C': case 'D':
    doBracketKey(aChar);
    break;
  case 'X':  /* xterm shifted kp= on sparcs */
  case 'P':
    doF1(mgetc());
    break;
  case 'o':  /* xterm shifted kp/ on sparcs */
  case 'Q':
    doF2(mgetc());
    break;
  case 'j':  /* xterm shifted kp* on sparcs */
  case 'R':
    doF3(mgetc());
    break;
  case 'p':     /* keypad '0' */
    doF1('n');
    break;
  case 'n':     /* keypad '.' */
    doF1('z');
    break;
  case 'q':     /* keypad '1' */
    doF1('w');
    break;
  case 'r':     /* keypad '2' */
    if (charAt(currentBuffer->dotLine,currentBuffer->dotPos) == '\n')  {
      if (cmdDirection == '>')
        moveRight();
      else
        moveUp(1);
    }
    doF1('e');
    break;
  case 's':     /* keypad '3' */
    if (cmdDirection == '>')
      moveRight(repCount);
    else
      moveLeft(repCount);
    setCursor();
    break;
  case 't':     /* keypad '4' */
    cmdDirection = '>';
    displayStatus();
    break;
  case 'u':     /* keypad '5' */
    cmdDirection = '<';
    displayStatus();
    break;
  case 'v':     /* keypad '6' */
    doF1('x');
    break;
  case 'w':     /* keypad '7' */
    doF1('p');
    break;
  case 'x':     /* keypad '8' */
    doF1('l');
    break;
  case 'Y':     /* keypad '9' */
    doF1('c');
    break;
  case 'S':     /* keypad Pf4 */
    doF1('d');
    break;
  case 'm':     /* mac keypad '-' */
    doF1('a');
    break;
  case 'l':     /* mac keypad '+' */
    doCommand(BS);
    break;
  case 'M':     /* keypad Enter */
    doCommand('\n');
    break;
  default:
    bell();
  }
}


/*****/
doF1(aChar)
  char aChar;
  {
  int x,y,i;

  switch (aChar) {

#ifdef MSDOS
  case BS:
#endif
  case DELETE:
    repInsert(deletedChar);
    break;

  case '\\':
    StickyF1 = !StickyF1;
    displayStatus();
    break;

  case 's':
    for (i=0; i<repCount; i++) search(FALSE);
    break;

  case 'a':
    setScroll(0,terminalLines);
    positionCursor(promptLine,0); eraseToEnd();
    fputs("searching", stdout); fflush(stdout);
    for (i=0; i<repCount; i++) search(TRUE);
    redrawStatus = TRUE;
    break;

  case 'r':
    findrep(FALSE);
    break;


  case '@':  /* center line */
    currentWindow->cursorLine = currentWindow->firstLine +
        (currentWindow->numLines / 2);
    displayWindow(currentWindow);
    break;

  case '[':  /* down page */
  case ']':  /* up page */
    x = cmdDirection;
    cmdDirection = (aChar == ']' ? '>' : '<');
    movePage();
    setCursor();
    cmdDirection = x;
    break;

  case '(':  /* down some */
  case ')':  /* up some */
    x = cmdDirection;
    cmdDirection = (aChar == ')' ? '>' : '<');
    doF1('l');
    cmdDirection = x;
    break;

  case 'p':
    movePage();
    setCursor();
    break;

  case '{':  /* down word */
  case '}':  /* up word   */
    x = cmdDirection;
    cmdDirection = (aChar == '}' ? '>' : '<');
    moveWords();
    setCursor();
    cmdDirection = x;
    break;

  case '!':
    y = curCharNo(&x);
    sprintf(lineBuff, "line %d of %d, char %d", x, linecount(currentBuffer), y);
    notify(lineBuff);
    break;

  case 'u':
    delToEnd();
    break;

  case 'f':
    for (x=0; x<repCount && !intAbort; x++)
      restoreDeletion();
    break;

  case 'b':
    currentBuffer->dotPos = 0;
    computeCursorCol();
    setCursor();
    break;

  case 'B':
    y = curCharNo(&x);
    if (x > currentWindow->numLines) {
      currentBuffer->dotLine = currentBuffer->firstLine;
      currentBuffer->dotPos = 0;
      displayWindow(currentWindow);
    }
    else {
      moveUp(x);
      setCursor();
    }
    break;

  case 'e':
    currentBuffer->dotPos = currentBuffer->dotLine->sz;
    computeCursorCol();
    setCursor();
    break;

  case 'E':
    currentBuffer->dotLine = currentBuffer->lastLine;
    currentBuffer->dotPos = 0;
    if (currentWindow->cursorLine <= (currentWindow->firstLine + 4))
      currentWindow->cursorLine = currentWindow->firstLine +
	(currentWindow->numLines / 2);
    displayWindow(currentWindow);
    break;

  case 'n':
    keepColumn = TRUE;
    if (cmdDirection == '<') moveUp(repCount); else moveDown(repCount);
    gotoColumn();
    setCursor();
    break;


  case '?':
#ifdef VMS
    newWindow(readFile("user1:[bruce.bin]niggle.doc",FALSE));
#endif
#ifdef UNIX
    newWindow(readFile(NIGGLE_DOC, FALSE));
#endif
#ifdef MSDOS
    newWindow(readFile(NIGGLE_DOC, FALSE));
#endif
    equWindowSizes();
    remakeScreen();
    displayStatus();
    currentBuffer->notSaveable = TRUE;
    break;


  case 'A':  /* hex dump of lines */
    analyseLines();
    remakeScreen();
    displayStatus();
    break;


  case 'm':
    x = mgetc() - '1';
    if (x < 0) x = 0;
    if (x > 9) x = 9;
    saveDot(x);
    sprintf(tempBuff, "mark %d set", x+1);
    notify(tempBuff);
    break;

  case 'j':
    x = mgetc() - '1';
    if (x < 0) x = 0;
    if (x > 9) x = 9;
    gotoSavedDot(x);
    doF1('!');
    break;


  case 'g':
    getTtyData(promptLine, "goto what line? ", &i);
    x = atoi(lineBuff);
    if (x==0)
      notify("invalid line number");
    else
      gotoLine(x);
    doF1('!');
    break;

#ifdef MSDOS
  case 0:
    doF1Extended();
    break;
#endif

  case 27:
    doF1Esc();
    break;

  case 'x':   /* cut */
    cutText();
    break;

  case 'c':  /* copy */
    copyText();
    break;

  case 'v':   /* paste contents of cut buffer */
    if (currentBuffer->readOnly) {
      notifyReadOnly();
      break;
    }
    saveDot(9);
    for (x=0; x<repCount && !intAbort; x++)
      pasteBuffer(cutBuffer,TRUE);
    break;

  case 'w':
    moveWords();
    setCursor();
    break;

  case 'z':  /* z - drop beginning marker for cut */
    saveDot(9);
    notify("beginning mark set");
    break;

  case 'i':
    insertChar(repCount);
    break;

  case 'I':
    /* undelete words */
    bell();
    break;

  case 'd':  /* del current line */
    if (currentBuffer->readOnly) {
      notifyReadOnly();
      break;
    }
    for (x=0; x<repCount && !intAbort; x++) {
      currentBuffer->dotPos = 0;
      computeCursorCol();
      delThruEnd(TRUE);
      setModified(TRUE);
    }
    break;

  case 'l':
    if (cmdDirection == '<')
      moveUp(currentWindow->numLines*2/3);
    else
      moveDown(currentWindow->numLines*2/3);
    setCursor();
    break;

  case 'k':
    if (cmdDirection == '<')
      cmdDirection = '>';
    else
      cmdDirection = '<';
    displayStatus();
    break;

  case 'P':
    if (recording) {
      recordSize = recSizeAfterLastCmd;  /* remove this command */
      recording = FALSE;
    }
    playback = TRUE;
    playbackIdx = 0;
    displayStatus();
    break;

  case 'R':
    recording = !recording;
    playback  = FALSE;
    displayStatus();
    if (recording) {
      recordSize = 0;
      playbackIdx = 0;
    }
    else {
      recordSize = recSizeAfterLastCmd;  /* remove termination command */
    }
    break;


  case 'q':
    if (currentBuffer->readOnly) {
      notifyReadOnly();
      break;
    }
    wrapCurrentLine(repCount);
    break;
  case 'Q':
    if (currentBuffer->readOnly) {
      notifyReadOnly();
      break;
    }
    wrapCurrentParagraph(repCount);
    break;

  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
     y = 0;
     i = aChar - '0';
     positionCursor(promptLine,0);
     printf("%d", i); eraseToEnd();
     for (x=mgetc(); '0'<=x && x<='9'; x=mgetc()) {
       i *= 10;
       i += x - '0';
       positionCursor(promptLine,0);
       printf("%d", i); eraseToEnd();
     }
     if (x == 7  ||  x == 3) {  /* ^c or ^g breaks */
       setCursor();
       break;
     }
     repCount = i;
     positionCursor(promptLine,0); eraseToEnd();
     setCursor();
     doCommand(x);
     break;

  default:
    bell();
    break;
  }
}


/*****/
#ifdef MSDOS
doF1Extended()
  {
  unsigned char x;

  x = mgetc();
  switch (x) {
    case 72:  /* up */
      prevWindow(); break;
    case 80:  /* down */
      nextWindow(); break;
    default:
      bell(); break;
  }
}
#endif




/*****/
int doF1Esc()
  {
  int i,x,y;

  x = mgetc();
  if (x == '[') {
    x = mgetc();
    switch (x) {
    case 'A': prevWindow(); break;
    case 'B': nextWindow(); break;
    case 'C': scrollRight(); break;
    case 'D': scrollLeft(); break;
    case '2':  /* 386i arrow key? */
      if (mgetc() == '1') {
        y = mgetc();
        mgetc();  /* grab the 'z' */
        switch (y) {
        case '7':  /* -> on 386i */
          doF1('C'); break;
        case '9':  /* <- on 386i */
          doF1('D'); break;
        default:
          bell();
        }
      }
      else bell();
      break;
    default: bell(); break;
    }
    return 1;
  }
  if (x != 'O') {
    bell();
    return 1;
  }
  x = mgetc();
  switch (x) {
  case 'A': prevWindow(); break;
  case 'B': nextWindow(); break;
  case 'p':     /* keypad '0' */
    if (currentBuffer->readOnly) {
      notifyReadOnly();
      break;
    }
    x = autoIndent;
    autoIndent = FALSE;
    repInsert('\n');
    autoIndent = x;
    for (i=0; i<repCount; i++) moveLeft();
    setCursor();
    break;
  case 'n':     /* keypad '.' */
    break;
  case 'q':     /* keypad '1' */
    break;
  case 'r':     /* keypad '2' */
    doF1('u');
    break;
  case 's':     /* keypad '3' */
    if (currentBuffer->readOnly) {
      notifyReadOnly();
      break;
    }
    insertChar(repCount);
    moveLeft();
    break;
  case 't':     /* keypad '4' */
    doF1('E');
    break;
  case 'u':     /* keypad '5' */
    doF1('B');
    break;
  case 'v':     /* keypad '6' */
    doF1('v');
    break;
  case 'w':     /* keypad '7' */
    bell();  /* command entry in edt */
    break;
  case 'x':     /* keypad '8' */
    bell();  /* word-fill in edt */
    break;
  case 'Y':     /* keypad '9' */
    bell();
    break;
  case 'S':     /* keypad Pf4 */
    doF1('f');
    break;
  case 'm':     /* mac keypad '-' */
    doF1('s');
    break;
  case 'k':     /* SUN type-4 kb '+' */
  case 'l':     /* mac keypad '+' */
    if (currentBuffer->readOnly) {
      notifyReadOnly();
      break;
    }
    repInsert(deletedChar);
    break;
  case 'M':     /* keypad Enter */
    bell();
    break;
  default:  bell(); break;
  }
}


/*****/
doF2(aChar)
  char aChar;
  {
  int i,x,y;
  aBuffer *buff;
  char promptc();

  switch (aChar) {
    case '\\':
      StickyF2 = !StickyF2;
      displayStatus();
      break;
    case DELETE:
    case BS:
      if (currentBuffer->notSaveable) {
        notify("you can not delete this buffer");
        break;
      }
      if (currentBuffer->modified) {
        x = promptc("Buffer is modified - are you sure? ",TRUE);
        if (x!= 'y' && x != 'Y') break;
      }
      buff = currentBuffer;
      do {
        buff = buff->next;
        if (!buff) buff = buffers;
      } while (buff!=currentBuffer && buff->notSaveable);
      if (buff == currentBuffer) {
        notify("there are no other buffers");
        break;
      }
      deleteCurrentBuffer();
      currentBuffer = buff;
      currentWindow->buffer = buff;
      strcpy(currentWindow->name, buff->fileName);
      displayWindow(currentWindow);
      displayLabel(currentWindow);
      setCursor();
      break;

    case 'n':
      buff = currentBuffer;
      do {
        buff = buff->next;
        if (!buff) buff=buffers;
      } while (buff!=currentBuffer && buff->notSaveable);
      if (buff==currentBuffer) break;
      strcpy(currentWindow->name, buff->fileName);
      currentBuffer = buff;
      currentWindow->buffer = buff;
      displayWindow(currentWindow);
      displayLabel(currentWindow);
      setCursor();
      break;

    case '-':
      leftColumn -= 8;
      if (leftColumn < 0) leftColumn = 0;
      remakeScreen();
      break;
    case '+':
      leftColumn += 8;
      remakeScreen();
      break;

    case '=':
      equWindowSizes();
      remakeScreen();
      displayStatus();
      break;
    case 'i':
      if (currentBuffer->readOnly) {
        notifyReadOnly();
        break;
      }
      insertFile();
      break;
    case 'y':
      if (currentBuffer->readOnly) {
        notifyReadOnly();
        break;
      }
      currentBuffer->crlf = !currentBuffer->crlf;
      setModified(TRUE);
      displayLabel(currentWindow);
      break;
    case 'd':
      delWindow();
      break;
    case 'a':
      newFileWindow();		/* add new window */
      break;
    case 'e':
      chngFileWindow();		/* replaces current window */
      break;
    case 'l':
      bufferStatus();
      remakeScreen();
      displayStatus();
      break;
    case 'z':
      windowStatus();
      remakeScreen();
      displayStatus();
      break;
    case 'w':
      if (currentBuffer->notSaveable)
        notify("you can't write out this buffer");
      else
        writeCurrentBuffer();
      break;
    case 'W':
      positionCursor(0, 0);
      eraseScreen();
      writeModifiedBuffers(TRUE);
      remakeScreen();
      displayStatus();
      break;
    case 'u':
      setModified(FALSE);
      notify("buffer marked as not modified");
      break;
    case 'x':
      if (currentBuffer == cutBuffer)
        notify("you can't change the name of the cut buffer");
      else
        changeName();
      break;
    default:
      bell();
      break;
  }
}



/*****/
doF3(aChar)
  char aChar;
  {
  int i,x,y;
  aBuffer *buff;
  FILE *afile;
  char *home;

  switch (aChar) {
  case '\\':
    StickyF3 = !StickyF3;
    displayStatus();
    break;
  case 'x':  case 'X':
  case 'a':  case 'A': case 'q':  case 'Q':
    doExit(aChar);
    break;
  case 'b':
    BS8 = !BS8;
    break;
  case 'c':
    caseSensitive = !caseSensitive;
    positionCursor(promptLine, 0);
    fputs("searches will be case ", stdout);
    fputs(caseSensitive? "sensitive" : "insensitive", stdout);
    setCursor();
    break;
  case 'd':
    printf("%c%s",Esc,"[;r");
    eraseScreen();
#ifdef VMS
    puts("spawning subprocess...  type 'logout' to get back to niggle");
    system("");
#endif
#ifdef UNIX
    mdeassign();
    fflush(stdout);
    system("csh -i");  /* spawn an interactive C-shell */
    massign();
#endif
#ifdef MSDOS
    system("");
#endif
    remakeScreen();
    break;

#ifdef UNIX
  case 'D':
    if (currentBuffer->readOnly) {
      notifyReadOnly();
      break;
    }
    if (getTtyData(promptLine,"command? ", &x) < 0) break;
    if (x == 0) break;
    home = (char *)getenv("HOME");
      strcpy(tempBuff, home);  strcat(tempBuff, "/.ngtemp2");
    unlink(tempBuff);
      strcpy(tempBuff, home);  strcat(tempBuff, "/.ngtemp1");
    afile = fopen(tempBuff, "w");
    if (!afile) {
      positionCursor(promptLine, 0); eraseToEnd();
      perror(tempBuff);
      setCursor();
      break;
    }
    fputs(lineBuff, afile);
    fputs("\n", afile);
    fclose(afile);
    system(spawnLine);
    { char *tmp;
      tmp = (char *)malloc(1024);
      if (tmp) {
        strcpy(tmp, home); strcat(tmp, "/.ngtemp2");
        buff = readFile(tmp, FALSE);
        if (buff) {
          saveDot(9);
          pasteBuffer(buff, FALSE);
          free(buff);
        }
        free(tmp);
      }
    }
    strcpy(tempBuff, home);  strcat(tempBuff, "/.ngtemp1");
    unlink(tempBuff);
    strcpy(tempBuff, home);  strcat(tempBuff, "/.ngtemp2");
    unlink(tempBuff);
    setCursor();
    break;
#endif

  case 'i':
    autoIndent = !autoIndent;
    displayStatus();
    break;
  case 'l':
    if (getTtyData(promptLine, "how many lines on your screen?", &i) < 0)
      break;
    if (i > 0) {
      x = atoi(lineBuff);
      if (x > MaxScreenLines) x = MaxScreenLines;
      if (x > 9) {
        terminalLines = x-1;
        /* make sure that all the lines required are allocated */
        if (screen[terminalLines] == NULL)
          for (i=0; i<=terminalLines; i++)
	    if (screen[i] == NULL) screen[i] = myalloc(terminalColumns+3);
        promptLine = terminalLines - 1;
        equWindowSizes();
        remakeScreen();
        displayStatus();
      }
      else {
        notify("You must specify at least 9 lines");
      }
    }
    break;
  case 'r':
    regularExpressions = !regularExpressions;
    displayStatus();
    break;
  case 's':
    if (getTtyData(promptLine, "how many columns on your screen?", &i) < 0)
      break;
    if (i > 0) {
      x = atoi(lineBuff);
      if (x < 10 || x > 1000) break;
      for (i=0; i<43; i++)
        screen[i] = realloc(screen[i], x+3);
      terminalColumns = x-2;
      remakeScreen();
      displayStatus();
    }
    break;
  case 't':
    useTabs = !useTabs;
    displayStatus();
    break;
  case 'w':			/* set wrap mode and margin */
    wrapOn = !wrapOn;
    displayStatus();
    positionCursor(promptLine,0);
    printf("Right margin is %d",wrapMargin);
    eraseToEnd();
    redrawStatus = TRUE;
    setCursor();
    break;
  case 'm':			/* set wrap mode and margin */
    getTtyData(promptLine, "new right margin?", &i);
    x = atoi(lineBuff);
    if (x > 0  &&  x < terminalColumns) {
      wrapMargin = x;
      wrapOn = TRUE;
      displayStatus();
      positionCursor(promptLine,0);
      eraseToEnd();
      redrawStatus = TRUE;
      setCursor();
    }
    else
      bell();
    break;

  case 'u':  case 'U':
    if (termType == SUNTERM) {
      termType = VT100TERM;
      terminalLines = 23;
      promptLine = 22;
    }
    else {
      termType = SUNTERM;
      terminalLines = 33;
      promptLine = 32;
    }
    if (screen[terminalLines] == NULL)
      for (i=0; i<=terminalLines; i++)
	if (screen[i] == NULL) screen[i] = myalloc(terminalColumns+3);
    equWindowSizes();
    remakeScreen();
    displayStatus();
    break;

  default:
    insertChar(aChar);
    break;
  }
}




/*****/
displayStatus()
{
  setScroll(0,terminalLines);
  positionCursor(promptLine,0);
  eraseToEnd();
  positionCursor(terminalLines,statusColumn);
  if (isColor) {
#ifdef CYGNUS
    setBold();
#endif
    fputs(fg_cyan, stdout);
    fputs(bg_black, stdout);
}
  fputs((cmdDirection == '<'? "<-" : "->"),stdout);
  if (recording)     fputs(" (rec)", stdout);
  if (playback)      fputs(" (play)", stdout);
  if (autoIndent)    fputs(" (indent)",stdout);
  if (regularExpressions) fputs(" (regex)",stdout);
  if (useTabs)       fputs(" (tabs)",stdout);
  if (wrapOn)        fputs(" (wrap)",stdout);
  if (StickyF1)      fputs(" (F1)",stdout);
  else if (StickyF2) fputs(" (F2)",stdout);
  else if (StickyF3) fputs(" (F3)",stdout);
  if (isColor) {
    setNormal();
    fputs(fg_black, stdout);
    fputs(bg_default, stdout);
}
  eraseToEnd();
#ifdef MSDOS
  showCoreLeft(TRUE);
#endif
  setCursor();
  redrawStatus = FALSE;
  fflush(stdout);
}





/*****/
notifyReadOnly() {
  notify("This file is read-only");
}


/*****/
notify(aString)
  char *aString;
  {
  setScroll(0,terminalLines);
  positionCursor(promptLine,0);
  if (isColor)
    fputs(fg_red, stdout);
  printf("%s",aString);
  if (isColor)
    fputs(fg_black, stdout);
  eraseToEnd();
  redrawStatus = TRUE;
  setCursor();
  fflush(stdout);
}



/*****/
#ifdef MSDOS
showCoreLeft(forceDraw)
  boolean forceDraw;
  {
  unsigned long core;

  core = coreleft();
  if (!forceDraw  &&  core == lastCoreLeft) return 1;
  positionCursor(terminalLines, terminalColumns-20);
  fprintf(stdout, "%9ld bytes free", core);
  eraseToEnd();
  setCursor();
  lastCoreLeft = core;
}
#endif



/*****/
char promptc(aString,promptWithCursor)
  char *aString;
  boolean promptWithCursor;	/* TRUE if cursor should stay with prompt
	      			   until input is collected */
{
  int x;

  setScroll(0,terminalLines);
  positionCursor(promptLine,0);
  printf("%s",aString);
  eraseToEnd();
  if (!promptWithCursor) setCursor();
  x =mgetc();
  positionCursor(promptLine,0);
  printf("%s%c",aString,x);
  setCursor();
  redrawStatus = TRUE;
  fflush(stdout);
  return(x);
}


/*****/
bell()
{
  printf("%c",7);
  fflush(stdout);
}


/*****/
repInsert(aChar)
  char aChar;
  {
  int x;

  for (x=0; x<repCount; x++)
    insertChar(aChar);
}




insertString(aString)
  char *aString;
{
  while (*aString)
    insertChar(*aString++);
}


