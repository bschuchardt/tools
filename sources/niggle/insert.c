/*  t e x t   i n s e r t i o n ---------------------------------------- */


/*
 * name - %M%
 * ccsid - %W% - %G% %U%
 * from  - %F%
 * date  - %H% %T%
 */
     


#include "types.mh"
#include "defn.h"
#include "buffer.h"

insertChar(aChar)
  int aChar;
{
  aLine *line;
  char *newValue, *holder, *pChar;
  int x, y, z, row, col;
  int numTabs,numSpaces;
  int linePos;
  char chr;
  
  if (currentBuffer->readOnly) {
    notifyReadOnly();
    return 1;
}

  line = currentBuffer->dotLine;
  strcpy(lineBuff,line->value);

  switch (aChar) {

  case '\n':
  case 13:			/** VMS kludge **/
      aChar = '\n';
      /* create a new buffer for the old line's contents up
	 to dotPos and populate it*/

      linePos = currentBuffer->dotPos;

      newValue = myalloc(linePos + (linePos % 2) + 4);
      for (x=0; x<currentBuffer->dotPos; x++)
	newValue[x] = lineBuff[x];
      newValue[linePos] = '\n';
      newValue[linePos+1] = 0;
      if (line->value != NULL) free(line->value);
      line->value = newValue;

      /* create the new line */
      line = (aLine *)myalloc(sizeof(aLine));	/*'line' is nolonger dotLine*/
      line->last = currentBuffer->dotLine;
      line->next = currentBuffer->dotLine->next;
      currentBuffer->dotLine->next = line;
      if (!(line->next == NULL)) line->next->last = line;
      line->isEof = currentBuffer->dotLine->isEof;
      currentBuffer->dotLine->isEof = FALSE;
      line->value = NULL;
      /* (note that no line->value is created for this new line) */

      /* fix up the line buffer */
      for (x=linePos; x<=currentBuffer->dotLine->sz; x++)
	lineBuff[x - linePos] = lineBuff[x];

      /* take care of line sizes */
      line->sz = currentBuffer->dotLine->sz - linePos;
      currentBuffer->dotLine->sz = linePos;

      /* make the new line the current line */
      currentBuffer->dotLine = line;
      currentBuffer->dotPos = 0;


      /* take care of the screen */
      if (autoIndent) setTab(nonWhiteCol());
      eraseToEnd();
      screen[currentWindow->cursorLine][currentWindow->cursorColumn] = 0;
      currentWindow->cursorLine++;
      currentWindow->cursorColumn = 0;
      if (currentWindow->cursorLine > currentWindow->lastLine) {
	currentWindow->cursorLine = currentWindow->lastLine;
	scrollUp(1);
	}
      else {
	/* make room for the next line */
        if (termType == VT100TERM) {
          setScroll(currentWindow->cursorLine,currentWindow->lastLine);
          scrDnWindow(1);
          setCursor();
          setScroll(0,terminalLines);
        }
#ifdef CYGNUS
        /* something in the cygnus console support causes the first line
           to be blanked during the preceding operation.  It happens
           during upLines in scrDnWindow.  */
        positionCursor(0,0);
        fputs(screen[0], stdout);
#endif
        holder = screen[currentWindow->lastLine];
	for (x=currentWindow->lastLine-1; x>=currentWindow->cursorLine; x--) {
	  screen[x+1] = screen[x];
          if (termType != VT100TERM) {
            positionCursor(x+1,0);
            fputs(screen[x],stdout); eraseToEnd();
          }
        }
        screen[currentWindow->cursorLine] = holder;
	}

      if (autoIndent) {
	ins_cleanUp();
	setCursor();
	if (tabColumn > 0  &&  strlen(currentBuffer->dotLine->value) <= 1)
            insertChar(9);
	}

      break;


  case 9:
      if (autoIndent && 
          (currentWindow->cursorColumn+currentWindow->horzScroll) < tabColumn) {
        if (useTabs) {
	  numTabs = tabColumn / 8;
	  numSpaces = tabColumn % 8;
	  x = (currentWindow->cursorColumn + currentWindow->horzScroll) / 8;
	  if (numTabs > x) {		/* crossing one or more tab stops */
	    numTabs -= x;
	  }
	  else {			/* not crossing a tab stop */
	    numSpaces = tabColumn - (currentWindow->cursorColumn + currentWindow->horzScroll);
	    numTabs = 0;
	  }
        }
        else {
          numTabs = 0;
          numSpaces = tabColumn - (currentWindow->cursorColumn + currentWindow->horzScroll);
        }
      }
      else {
        if (useTabs) {
          numSpaces = 0;
          numTabs = 1;
        }
        else {
          numTabs = 0;
          numSpaces = 5 - ((currentWindow->cursorColumn+currentWindow->horzScroll+1) % 4);
          if (numSpaces == 5) numSpaces = 1;
        }
      }

      /* make room for the new characters */
      y = numSpaces + numTabs;
      for (x=line->sz+1; x>=currentBuffer->dotPos; x--)
        lineBuff[x+y] = lineBuff[x];

      /* save the current cursor column for blank-filling */
      z = currentWindow->cursorColumn;


      /* add the new characters and update screen memory */
      for (x=0; x<numTabs; x++) {
        lineBuff[currentBuffer->dotPos+x] = 9;
        toScreen(currentWindow, &currentWindow->cursorLine,&currentWindow->cursorColumn,9);
      }
      for (x=numTabs; x<y; x++) {
        lineBuff[currentBuffer->dotPos+x] = ' ';
        toScreen(currentWindow, &currentWindow->cursorLine,&currentWindow->cursorColumn,' ');
      }

      /* blank fill the display to the new cursor column position */
      for (; z<currentWindow->cursorColumn; z++)
        putchar(screen[currentWindow->cursorLine][z]);

      currentBuffer->dotPos += numTabs + numSpaces;
      line->sz += numTabs + numSpaces;
      break;


  default:
      /* if wrapping and this is not a separator and the previous character
         WAS a separator, wrap to the next line */
      if (wrapOn) if (currentWindow->cursorColumn >= wrapMargin &&
          currentBuffer->dotPos>0 &&
	  !isSeparator(aChar) && isSeparator(lineBuff[currentBuffer->dotPos-1])
          ) {
	wrapOn = FALSE;
        insertChar(13);
        insertChar(aChar);
	wrapOn = TRUE;
        return 1;
	}
      for (x=line->sz+2; x>currentBuffer->dotPos; x--)
	lineBuff[x] = lineBuff[x-1];
      lineBuff[currentBuffer->dotPos++] = aChar;
      line->sz++;
      /* display the new character */
      x = currentWindow->cursorColumn;
      toScreen(currentWindow, &currentWindow->cursorLine,&currentWindow->cursorColumn,aChar);
      for (;x<currentWindow->cursorColumn; x++)
	putchar(screen[currentWindow->cursorLine][x]);
      break;


    }

  /* redisplay to the end of the current line */
  if (currentWindow->cursorColumn < terminalColumns) {
    col = currentWindow->cursorColumn;
    row = currentWindow->cursorLine;
    for (x=currentBuffer->dotPos; x <= currentBuffer->dotLine->sz; x++)
      toScreen(currentWindow, &row,&col,lineBuff[x]);
    setCursor();
    fputs(&screen[currentWindow->cursorLine][currentWindow->cursorColumn], stdout);
    eraseToEnd();
    setCursor();
  }

  setModified(TRUE);
  ins_cleanUp();
}




/*****/
ins_cleanUp()
{
  /* this routine is used by insertChar, and tabAdjust (and others??)
     to move info from lineBuff into a new line value for the current
     line (currentBuffer->dotLine).  The current line's size is
     assumed to reflect the size of the text in lineBuff */

  int x;
  char *newValue;
  aLine *line;

  /* copy the modified line to a new buffer and replace the old line */
  x = currentBuffer->dotLine->sz;
  newValue = myalloc(x + (x % 2) + 4);
  for (; 0 <= x; x--)
    newValue[x] = lineBuff[x];
  newValue[currentBuffer->dotLine->sz+1] = 0;
  if (currentBuffer->dotLine->value != NULL)
    free(currentBuffer->dotLine->value);
  currentBuffer->dotLine->value = newValue;
  if (currentBuffer->dotLine->isEof) {
      currentBuffer->dotLine->isEof = FALSE;	/* create new EOF record */
      line = (aLine *)myalloc(sizeof(aLine));
      line->value = myalloc(4);
      line->value[0] = '\n';
      line->value[1] = 0;
      line->sz = 0;
      line->isEof = TRUE;
      line->next = NULL;
      line->last = currentBuffer->dotLine;
      currentBuffer->dotLine->next = line;
      currentBuffer->lastLine = line;
    }
}






/*****/
restoreDeletion()
{
  /* insert the text in delLin */

  aLine *line, *newLine;
  char *newValue, *oldValue, *holder;
  int x, newSize;

  if (currentBuffer->readOnly) {
    bell();
    return 1;
}

  line = currentBuffer->dotLine;

  if (delLin[delLinSize] == '\n') {
    if (currentBuffer->dotPos < 0) currentBuffer->dotPos++;
    newSize = currentBuffer->dotPos + delLinSize;
    newValue = myalloc(newSize + 5 + (newSize % 2));

    for (x=0; x<currentBuffer->dotPos; x++)
      newValue[x] = line->value[x];
    for (x=0; x<=delLinSize+1; x++)
      newValue[currentBuffer->dotPos+x] = delLin[x];

    oldValue = line->value;
    for (x=currentBuffer->dotPos; x<=line->sz+1; x++)
      oldValue[x-currentBuffer->dotPos] = oldValue[x];

    line->value = newValue;
    line->sz = newSize;

    newLine = (aLine *)myalloc(sizeof(aLine));
    newLine->value = oldValue;
    newLine->sz  = strlen(oldValue)-1;
    newLine->isEof = FALSE;
    newLine->last = line;
    newLine->next = line->next;
    line->next = newLine;

    if (line->isEof) {
      newLine->next = NULL;
      line->isEof = FALSE;
      newLine->isEof = TRUE;
      currentBuffer->lastLine = newLine;
    }
    else
      newLine->next->last = newLine;

    currentWindow->cursorLine++;
    currentWindow->cursorColumn = 0;
    if (currentWindow->cursorLine > currentWindow->lastLine) {
      currentWindow->cursorLine = currentWindow->lastLine;
      scrollUp(1);
    }
    else {
      if (termType == VT100TERM) {
        setScroll(currentWindow->cursorLine,currentWindow->lastLine);
        scrDnWindow(1);
        setScroll(0,terminalLines);
      }
      holder = screen[currentWindow->lastLine];
      for (x=currentWindow->lastLine-1; x>=currentWindow->cursorLine; x--) {
	screen[x+1] = screen[x];
        if (termType != VT100TERM) {
          positionCursor(x+1,0);
          fputs(screen[x],stdout); eraseToEnd();
        }
      }
      screen[currentWindow->cursorLine] = holder;
    }
    currentWindow->cursorLine--;
    displayLine(currentWindow,line,currentWindow->cursorLine);
    displayLine(currentWindow,newLine,currentWindow->cursorLine+1);
  }
  else {
    x = line->sz + delLinSize + 1;
    newValue = myalloc(x + 4 + (x % 2));
    strcpy(newValue,line->value);
    strcpy(&newValue[currentBuffer->dotPos],delLin);
    strcpy(&newValue[currentBuffer->dotPos+delLinSize+1],
        &line->value[currentBuffer->dotPos]);
    free(line->value);
    line->value = newValue;
    line->sz = x;
    displayLine(currentWindow,line,currentWindow->cursorLine);
    currentBuffer->dotPos += delLinSize + 1;
  }

  computeCursorCol();
  setCursor();
  setModified(TRUE);
}




/*****/
setTab(col)
  int col;
{

  if (col <= 0) {
    tabColumn = 0;
  }
  else {
    if (col > (terminalColumns-2)) col = terminalColumns-2;
    tabColumn = col;
  }
}




/*****/
tabAdjust(forInsert)
  boolean forInsert;

{  /* adjust the current line to start at the current tab position */

  int x,y,numTabs,numSpaces,ntx8;
  boolean white;
  aLine *line;


  numTabs = tabColumn / 8;
  numSpaces = tabColumn % 8;

  line = currentBuffer->dotLine;

  for (x=0, white=TRUE; white && x<line->sz;)
    if (line->value[x] != ' '  && line->value[x] != 9) white = FALSE;
    else x++;

  ntx8 = numTabs * 8;
  line->sz = line->sz - x + ntx8 + numSpaces;

  if (forInsert) {
    if (currentBuffer->dotPos > x)
      currentBuffer->dotPos += ntx8 + numSpaces - x;
    else
      currentBuffer->dotPos = ntx8 + numSpaces;
    if (currentBuffer->dotPos < 0)
      currentBuffer->dotPos = 0;
  }
  else
    currentBuffer->dotPos = 0;


  strcpy(&lineBuff[ntx8 + numSpaces],&line->value[x]);
  for (x=1; x<=ntx8; x++)
    lineBuff[x-1] = 32;
  for (x=1; x<=numSpaces; x++)
    lineBuff[ntx8+x-1] = ' ';

  ins_cleanUp();

  displayLine(currentWindow,line,currentWindow->cursorLine);
  if (forInsert) {
    computeCursorCol();
    setCursor();
  }
  return 1;
}







