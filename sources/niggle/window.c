/*  w i n d o w   m a n a g e m e n t---------------------------------- */


/*
 * name - %M%
 * ccsid - %W% - %G% %U%
 * from  - %F%
 * date  - %H% %T%
 */



#include "types.mh"
#include "defn.h"
#include "edit.h"
#include "buffer.h"




/*****/
int newFileWindow()
{
  /* add a new window to the screen */

  int x, nSize;
  aBuffer *buff;
  char fName[1000];

  if (getFileName(promptLine,"add window for what file? ",&nSize) < 0)
    return 1;

  for (buff=buffers; buff!=NULL  &&
    strcmp(buff->fileName,lineBuff) != 0; buff=buff->next);

  strcpy(fName,lineBuff);	/* copy out of global variable to avoid
    				/* conflicts with other routines */

  if (buff == NULL)		/* new buffer */
    newWindow(readFile(fName,TRUE));
  else
    newWindow(buff);

  if (currentWindow->last != NULL) {
    if (currentWindow->last->cursorLine > currentWindow->last->lastLine)
      displayWindow(currentWindow->last);
    displayLabel(currentWindow->last);
  }

  displayWindow(currentWindow);
  displayLabel(currentWindow);
  setCursor();
}



/*****/
int changeName()
{
  int nSize;

  if (getFileName(promptLine,"change name to what? ",&nSize) < 0)
    return 1;

  strcpy(currentWindow->name,lineBuff);
  strcpy(currentBuffer->fileName,lineBuff);
  currentBuffer->modified = TRUE;
  currentBuffer->readOnly = FALSE;
  currentBuffer->notSaveable = FALSE;
  displayLabel(currentWindow);
  setCursor();
}




/*****/
int insertFile()
{

  int x, nSize;
  aBuffer *buff;
  char fName[80];

  if (getFileName(promptLine,"insert text from what file? ",&nSize) < 0)
    return 1;
  strcpy(fName,lineBuff);
  buff = readFile(fName,FALSE);
  pasteBuffer(buff,FALSE);
  free(buff);
}






/*****/
int chngFileWindow()
{
   /* replace the current window with one for the file to be specified
      by the user */

  int x, nSize;
  aBuffer *buff;
  char fName[80];

  if (getFileName(promptLine,"change to what file? ",&nSize) < 0)
    return 1;

  strcpy(fName,lineBuff);	/* copy out of global variable to avoid
    				/* conflicts with other routines */

  /* do we already have the file? */
  for (buff=buffers; buff!=NULL  &&
    strcmp(buff->fileName,fName) != 0; buff=buff->next);

  if (buff == NULL)		/* new buffer */
    buff =readFile(fName,TRUE);

  strcpy(currentWindow->name,fName);
  currentBuffer = buff;
  currentWindow->buffer = buff;
  currentWindow->horzScroll = 0;

  displayWindow(currentWindow);
  displayLabel(currentWindow);
  setCursor();
}





/*****/
int delWindow()
{
  aWindow *wind;
  int x;
  aLine *line;

  if (currentWindow->last == NULL &&  currentWindow->next == NULL) {
    bell();
    return 1;
  }

  if (currentWindow->last != NULL) {
    wind = currentWindow->last;
    wind->next = currentWindow->next;
    if (wind->next != NULL) wind->next->last = wind;

    /* adjust the size and label line.  The  "+ 1" accounts for
       the label line of the deleted window */

    wind->numLines = wind->numLines + currentWindow->numLines + 1;
    wind->labelLine = wind->labelLine + currentWindow->numLines + 1;

    /* display text in the new window area */
    
    setScroll(0,terminalLines);

    for (x=wind->cursorLine, line=wind->buffer->dotLine;
       x <= wind->lastLine  &&  line != NULL; x++)
      line = line->next;

    /* line now points to that after the last on old window */
    for (x=wind->lastLine+1; x < wind->labelLine; x++)
      if (line != NULL) {
	displayLine(wind,line,x);
	line = line->next;
	}
      else {
	positionCursor(x,0);
	eraseToEnd();
	}


    wind->lastLine = wind->labelLine - 1;

    displayLabel(wind);
  }
  else {
    /* here we are deleting the first window in the chain of windows */
    wind = currentWindow->next;
    windows = wind;
    wind->last = NULL;
    wind->firstLine = currentWindow->firstLine;
    wind->numLines = wind->numLines + currentWindow->numLines + 1;
    displayWindow(wind);
  }

  free(currentWindow);
  selectWindow(wind);
  setCursor();
}







/*****/
int selectWindow(window)
  aWindow *window;
{
  currentWindow = window;
  currentBuffer = window->buffer;
  setScroll(0,terminalLines);
}




/*****/
int newWindow(buffer)
  aBuffer *buffer;	/* create a window for this buffer */
{
  aWindow *wind;

  if (windows == NULL) {
    /* create the first window of the list */
    windows = (aWindow *)myalloc(sizeof(aWindow));
    windows->next = NULL;
    windows->last = NULL;
    wind = windows;
    wind->firstLine = 0;
    wind->lastLine  = terminalLines - 3;
    wind->numLines  = terminalLines - 2;
  }
  else {
    /* add a window after the current window, stealing space from it */
    if (currentWindow->numLines < 5) {
      notify("no room for new window");
      bell();
      return 1;
    }
    wind = (aWindow *)myalloc(sizeof(aWindow));
    wind->last = currentWindow;
    wind->next = currentWindow->next;
    if (wind->next != NULL) wind->next->last = wind;
    currentWindow->next = wind;
    wind->numLines = currentWindow->numLines/2;

    currentWindow->numLines = currentWindow->numLines - wind->numLines - 1;
    currentWindow->lastLine = currentWindow->firstLine + currentWindow->numLines
    	- 1;
    currentWindow->labelLine = currentWindow->lastLine + 1;

    wind->firstLine = currentWindow->labelLine + 1;
    wind->lastLine = wind->firstLine + wind->numLines - 1;

  }

  wind->cursorColumn = 0;
  wind->cursorLine = wind->firstLine + (wind->numLines / 2);

  wind->horzScroll = 0;

  wind->buffer = buffer;
  strcpy(wind->name,buffer->fileName);
  wind->labelLine = wind->lastLine + 1;

  selectWindow(wind);	/* select the new window.
    			/* initial buffer creation requires this */

}



/*****/
int equWindowSizes()
{
  int x,y,z,leftovers;
  aWindow *wind;

  /* count the windows */
  for(x=0,wind=windows; wind!=NULL; wind=wind->next) x++;

  if (x==1) {
    currentWindow->firstLine = 0;
    currentWindow->lastLine  = terminalLines - 3;
    currentWindow->numLines  = terminalLines - 2;
    currentWindow->labelLine = currentWindow->lastLine + 1;
    return 1;
  }

  y = terminalLines - 1 - x;	/* number of lines available for windows,
				   not counting labels */
  z = y / x;			/* per window size without label */
  leftovers = y - (z * x);

  for (x=0, wind=windows; wind!=NULL; wind=wind->next) {
    if (wind->next == NULL) z = z + leftovers;
    wind->numLines  = z;
    wind->firstLine = x;
    wind->lastLine  = x + z - 1;
    wind->labelLine = wind->lastLine + 1;
    x = wind->labelLine + 1;
  }
}



/*****/
int nextWindow()
{
  if (currentWindow->next == NULL) {
    if (currentWindow->last == NULL) return 1;
    selectWindow(windows);
  }
  else {
    selectWindow(currentWindow->next);
  }
  setCursor();
}


/*****/
int prevWindow()
{
  aWindow *wind;

  if (currentWindow->last == NULL) {
    for (wind = windows; wind->next != NULL; wind = wind->next) ;
    selectWindow(wind);
  }
  else {
    selectWindow(currentWindow->last);
  }
  setCursor();
}








/*****/
int remakeScreen()
{
  int i;
  aWindow *wind;

  setScroll(0,terminalLines);
  eraseScreen();
  for (i=0; i<=terminalLines; i++) screen[i][0] = 0;

  for (wind = windows; wind != NULL; wind = wind->next) {
    displayWindow(wind);
    displayLabel(wind);
  }
  
  displayStatus();
  setCursor();
}


/*****/
displayWindow(window)  /* create the window display from scratch,
      			   positioning the cursor near the center */
  aWindow *window;
{
  int x, y;
  aLine *line1;
  aLine *dotLine;
  int cLine, cCol, dotPos, inLine1;
  int aChar;


  /* determine which line the cursor should go on. Attempt
     to keep the existing line. */
  if (window->firstLine <= window->cursorLine &&
      window->cursorLine <= window->lastLine)
    y = window->cursorLine - window->firstLine;
  else
    y = (window->numLines / 2) + (window->numLines % 2);

  /* find the first line to print */
  line1 = window->buffer->dotLine;
  x = 0;
  while ((x < y)  && (line1 != NULL)) {
    line1 = line1->last;
    x++;
  }

  if (line1 == NULL) {
    line1 = window->buffer->firstLine;
    x--;
  }
  
  window->cursorLine = window->firstLine + x;
  
  cLine = window->firstLine;
  while (cLine <= window->lastLine) {
    formatLine(window, cLine++, line1);
    if (!line1->isEof) line1 = line1->next;
  }

  /* display the screen memory */
  setScroll(0,terminalLines);
  positionCursor(window->firstLine, 0);
  for (x=window->firstLine; x<=window->lastLine; x++) {
#ifdef PC_VIDEO
    clearLine(x);
    toVideo(x, screen[x]);
#else
    positionCursor(x, 0);
    fputs(screen[x], stdout);
    eraseToEnd();
#endif
  }

  /* compute the cursor column for this window */
  window->cursorColumn = 0;
  line1 = window->buffer->dotLine;
  for (x=0; x<window->buffer->dotPos; x++)
    toScreen(window, &window->cursorLine, &window->cursorColumn, line1->value[x]);

  window->cursorColumn -= window->horzScroll;
  if (window->cursorColumn < 0)
    window->cursorColumn = 0;

  if (window == currentWindow) setCursor();
}



/*****/
displayLabel(window)
  aWindow *window;
{
  int x;
  /* assumes the scrolling region is set to allow printing at the
     label line */
  strcpy(screen[window->labelLine],window->name);
  if (window->buffer->modified)
    strcat(screen[window->labelLine], "*");
  if (window->buffer->crlf)
    strcat(screen[window->labelLine], " (PC)");
  if (window->buffer->readOnly)
    strcat(screen[window->labelLine], " (read-only)");
  positionCursor(window->labelLine,0);
  if (isColor) {
#ifdef CYGNUS
    setBold();
#endif
    fputs(fg_green, stdout);
    fputs(bg_black, stdout);
}
  else
    setInverse();
  printf("======%s", screen[window->labelLine]);
  if (isColor) {
    setNormal();
    fputs(fg_black, stdout);
    fputs(bg_default, stdout);
}
  else
    setNormal();
  eraseToEnd();
}



/*****/
toScreen(window, line, col, aChar)
  aWindow *window;
  int *line, *col;
  int aChar;
{  /* 'aChar' is put to the screen buffer.  'line' and 'col' are updated to
      reflect the printing of 'aChar'.  They point to the next print-position.
      Window bounds are not checked. */
  register int x, y, mChar;
  int horz = 0;
  
  if (window != NULL)
    horz = window->horzScroll;

  mChar = (unsigned char)aChar;
  if (mChar < 32)
    switch (mChar) {
      case 9: x = 9 - ((*col+1) % 8);
	      if (x == 9) x = 1;
	      for (y=1; y <= x; ++y) {
                if (*col >= horz)
                    screen[*line][((*col)++) - horz] = ' ';
            }
              if (*col-horz >= terminalColumns) goto rightCol;
	      break;
      case '\n':
              if (horz >= *col)
                screen[(*line)++][0] = 0;
              else
                screen[(*line)++][*col-horz] = 0;
	      *col = 0;
	      break;
      default:
              if (horz <= *col)
                screen[*line][(*col)++] = '^';
              else
                (*col)++;
              if ((*col)-horz >= terminalColumns) goto rightCol;
              if (horz <= *col)
                screen[*line][(*col)++ - horz] = mChar + 64;
	      break;
     }
  else if (*col-horz < terminalColumns) {
      if (*col >= horz)
        screen[*line][(*col)-horz] = mChar;
      (*col)++;
    }
  else {
rightCol:
      *col = terminalColumns;
      screen[*line][(*col)++ - horz] = '<';
      screen[*line][(*col) - horz] = 0;
    }
}


/*****/
formatLine(window, row, line)
  aWindow *window;
  int row;
  aLine *line;
{  /* write the contents of the given line to the screen buffer at the
      given row */
  register int mChar, x, y, col, size;
  char *value, *srow;
  int horz = window->horzScroll;
  
  value = line->value;
  size = line->sz;
  srow = screen[row];
  col = 0;

  for (mChar = *value++; size >= 0; size--, mChar = *value++) {
    if (col < terminalColumns+horz) {
      if (mChar >= 32) {
        if (col >= horz)
            srow[col-horz] = mChar;
        col++;
      }
      else
        switch (mChar) {
        case 9: x = 9 - ((col+1) % 8);
                if (x == 9) x = 1;
                for (y=1; y <= x; ++y) {
                  if (col >= horz)
                    srow[col - horz] = ' ';
                  col++;
              }
                if (col-horz >= terminalColumns) goto rightCol;
                break;
        case '\n':
        case 0:
                if (col >= horz)
                  srow[col-horz] = 0;
                else
                  srow[0] = 0;
                return 1;
        default:
                if (col >= horz)
                  srow[col - horz] = '^';
                col++;
                if (col-horz >= terminalColumns) goto rightCol;
                if (col >= horz)
                  srow[col - horz] = mChar + 64;
                col++;
                break;
       }
    }
    else {
rightCol:
      srow[terminalColumns] = '<';
      srow[terminalColumns+1] = 0;
      return 0;
    }
  }
}  




/*****/
int charWidth(aChar, col)
  unsigned int aChar;
  int col;
{  /* return the width of the given character at the given screen column.
      The screen column is required for processing tab characters.
      Margins are not respected in this function.
      Newline and null have zero width. */
  register int result;

  if (aChar < 32)
    switch (aChar) {
      case 9: result = 9 - ((col+1) % 8);
	      if (result == 9) result = 1;
              break;
      case '\n':
      case 0:
              result = 0;
              break;
      default:
              result = 2;
	      break;
    }
  else
      result = 1;
  return result;
}





/*****/
int computeCursorCol()
{     /* determine the cursor column on the current line */
  int i;

  currentWindow->cursorColumn = 0;
  for (i=0; i<currentBuffer->dotPos; i++)
    toScreen(currentWindow, &currentWindow->cursorLine,&currentWindow->cursorColumn
	,currentBuffer->dotLine->value[i]);
  currentWindow->cursorColumn -= currentWindow->horzScroll;
  if (currentWindow->cursorColumn < 0)
    currentWindow->cursorColumn = 0;
}




/*****/
int scrollDown(nLines)
  int nLines;
{
  int i, x;
  char *holder[60];

  if (termType == VT100TERM) {
    setScroll(currentWindow->firstLine,currentWindow->lastLine);
    scrDnWindow(nLines);
  }

  /***  book-keep the virtual screen */

  /* scroll down the contents */
  /* cut out the last nLines */
  for (i=currentWindow->lastLine-nLines+1, x=0; x<nLines; i++, x++)
    holder[x] = screen[i];
  /* shuffle down the other lines */
  for (i=currentWindow->lastLine - nLines; i>=currentWindow->firstLine; i--)
    screen[i+nLines] = screen[i];
  /* paste in the cut lines */
  for (i=currentWindow->firstLine, x=0; x<nLines; i++, x++)
    screen[i] = holder[x];

  /* blank out the top nLines */
  x = currentWindow->firstLine + nLines;
  for (i=currentWindow->firstLine; i<x; i++)
    screen[i][0] = 0;

  if (termType != VT100TERM) {
    for (i=currentWindow->firstLine; i<=currentWindow->lastLine; i++) {
#ifdef PC_VIDEO
      clearLine(i);
      toVideo(i, screen[i]);
#else
      positionCursor(i,0);
      fputs(screen[i],stdout);
      eraseToEnd();
#endif
    }
  }
}




/*****/
int scrollUp(nLines)
  int nLines;
{
  int i, x;
  char *holder[60];

  if (termType == VT100TERM) {
    setScroll(currentWindow->firstLine,currentWindow->lastLine);
    scrUpWindow(nLines);
  }

  /***  book-keep the virtual screen */

  /* scroll up the contents */
  /* cut out first nLines lines */
  for (i=currentWindow->firstLine, x=0; x<nLines; i++, x++)
    holder[x] = screen[i];
  /* shuffle up the other lines */
  for (i=currentWindow->firstLine + nLines; i<=currentWindow->lastLine; i++)
    screen[i-nLines] = screen[i];
  /* paste in the cut lines */
  for (i=currentWindow->lastLine-nLines+1, x=0; x<nLines; i++, x++)
    screen[i] = holder[x];

  /* clear out the bottom nLines */
  x = currentWindow->lastLine - nLines;
  for (i=currentWindow->lastLine; i>x; i--)
    screen[i][0] = 0;
  
  if (termType != VT100TERM) {
    for (i=currentWindow->firstLine; i<=currentWindow->lastLine; i++) {
#ifdef PC_VIDEO
      clearLine(i);
      toVideo(i, screen[i]);
#else
      positionCursor(i,0);
      fputs(screen[i],stdout);
      eraseToEnd();
#endif
    }
  }
}


/*****/
int scrollToCurr()
{
  int i,x;
  aLine *line;
  char *holder;

    /* scroll up the screen below the current line.  This is used by
	 the deletion routines and cut routines */

  if (currentWindow->cursorLine != currentWindow->lastLine) {
    line = currentBuffer->dotLine->next;

    if (termType == VT100TERM) {
      if (currentWindow->cursorLine+1 != currentWindow->lastLine) {
        /* need >= 2 lines for scrolling operations */
        setScroll(currentWindow->cursorLine+1,currentWindow->lastLine);
        scrUpWindow(1);
      }
    }

    holder = screen[currentWindow->cursorLine+1];
    for (x=currentWindow->cursorLine+1; x < currentWindow->lastLine; x++) {
      screen[x] = screen[x+1];
      if (termType != VT100TERM) {
#ifdef PC_VIDEO
        clearLine(x);
        toVideo(x, screen[x]);
#else
        positionCursor(x,0);
        fputs(screen[x], stdout);
        eraseToEnd();
#endif
      }
      if (line != NULL) line = line->next;
    }
    screen[currentWindow->lastLine] = holder;

    if (line != NULL)
      displayLine(currentWindow,line,currentWindow->lastLine);
    else
      screen[currentWindow->lastLine][0] = 0;

  }

}





/*****/
displayLine(window, line, row)  /* this works okay if line is NULL */
  aWindow *window;
  aLine *line;
  int row;
{  /* row is the line of the screen to print the text upon */
  int i, aChar;
  int col = 0;
  if (line == NULL)
    screen[row][0] = 0;
  else
    formatLine(window, row, line);
#ifdef PC_VIDEO
  clearLine(row);
  toVideo(row, screen[row]);
#else
  positionCursor(row,0);
  fputs(screen[row], stdout);
  eraseToEnd();
#endif
}


/*****/
int dispToEol()
{  /* display from the current position through the end of line */

  int row,col,x,aChar;

  if (currentWindow->cursorColumn < terminalColumns) {
    col = currentWindow->cursorColumn;
    row = currentWindow->cursorLine;
    for (x=currentBuffer->dotPos; x <= currentBuffer->dotLine->sz; x++) {
      aChar = charAt(currentBuffer->dotLine,x);
      toScreen(currentWindow, &row,&col,aChar);
    }
    setCursor();
#ifdef PC_VIDEO
    clearLine(currentWindow->cursorLine);
    toVideo(currentWindow->cursorLine, screen[currentWindow->cursorLine]);
#else
    fputs(&screen[currentWindow->cursorLine][currentWindow->cursorColumn],
    		stdout);
    eraseToEnd();
    setCursor();
#endif
  }
}



/*****/
int nonWhiteCol()
{
  /* return the first non white column # on the current line */
  int x;
  char *ptr;

  ptr = screen[currentWindow->cursorLine];
  for (x=0; (*ptr) == ' '; x++, ptr++);
  return(x);
}


/*****/
int dispFromCurr()		/* display lines after current to end of screen */
{
  aLine *line;
  int    x;

  line = currentBuffer->dotLine;
  for (x=currentWindow->cursorLine; x<=currentWindow->lastLine; x++) {
    displayLine(currentWindow,line,x);
    if (line != NULL) line = line->next;
  }
}


/*****/
int dispToCurr()   /* display lines from top of screen to the current line */
{
  aLine *line;
  int    x;

  line = currentBuffer->dotLine->last;
  for (x=currentWindow->cursorLine-1; x>=0; x--) {
    displayLine(currentWindow,line,x);
    if (line != NULL) line = line->last;
  }
}



scrollLeft() /* scroll current window left */
{
  if (currentWindow->horzScroll >= 8)
    currentWindow->horzScroll -= 8;
  remakeScreen();
  displayStatus();
}


scrollRight() /* scroll current window right */
{
  currentWindow->horzScroll += 8;
  remakeScreen();
  displayStatus();
}




