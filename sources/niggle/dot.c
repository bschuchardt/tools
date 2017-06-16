/*  d o t   m o v e m e n t------------------------------------------------*/


/*
 * name - %M%
 * ccsid - %W% - %G% %U%
 * from  - %F%
 * date  - %H% %T%
 */



#include "types.mh"
#include "defn.h"
#include "buffer.h"

/*****/
saveDot(dot)
  int dot;
  {
  int x;

  if (dot > numDots) {
    setScroll(0,terminalLines);
    positionCursor(terminalLines,statusColumn);
    if (isColor)
      fputs(fg_red, stdout);
    printf("%cinvalid pointer number -> %d",7,dot);
    if (isColor)
      fputs(fg_black, stdout);
    eraseToEnd();
    redrawStatus = TRUE;
    setCursor();
    return 1;
  }

  currentBuffer->aDot[dot] = curCharNo(&x);
}




curCharNo(lineNo)
  int *lineNo;
  {
  return curCharNoForBuffer(currentBuffer, lineNo);
}



/*****/
curCharNoForBuffer(buff, lineNo)
  aBuffer *buff;
  int *lineNo;
{
  /* compute the current character's position in the buffer and, as
     a bonus, compute it's line number */

  aLine *line;
  int x;

  *lineNo = 1;
  line = buff->firstLine;
  x = 0;
  while (line != buff->dotLine  && !line->isEof) {
    x += line->sz + 1;
    line = line->next;
    (*lineNo)++;
  }
  return(x+buff->dotPos+1);
}





/*****/
gotoSavedDot(dot)
{
  gotoPos(currentBuffer->aDot[dot]);
}






/*****/
gotoPos(aPos)
  int aPos;
{
  aLine *line;
  int pos, x, lineCount;

  line = currentBuffer->dotLine;
  pos  = currentBuffer->dotPos;
  x = curCharNo(&lineCount);
  lineCount = 0;
  if (aPos < x) {
    while (aPos < x) {
      lastChar(&line,&pos);
      if (newLine) lineCount++;
      x--;
    }
    x = currentWindow->cursorLine - currentWindow->firstLine;
    if (lineCount < currentWindow->numLines) {
      moveUp(lineCount);
      currentBuffer->dotPos = pos;
      computeCursorCol();
    }
    else {
      currentBuffer->dotLine = line;
      currentBuffer->dotPos = pos;
      displayWindow(currentWindow);
    }
  }
  else {
    while (aPos > x  && !line->isEof) {
      nextChar(&line,&pos);
      if (newLine) lineCount++;
      x++;
    }
    x = currentWindow->lastLine - currentWindow->cursorLine;
    if (lineCount < currentWindow->numLines) {
      moveDown(lineCount);
      currentBuffer->dotPos = pos;
      computeCursorCol();
    }
    else {
      currentBuffer->dotLine = line;
      currentBuffer->dotPos = pos;
      displayWindow(currentWindow);
    }
  }
}







/*****/
gotoLine(lineNo)
{
  aLine *line;
  int x;
  int lineCount;

  x = curCharNo(&lineCount);
  line = currentBuffer->dotLine;

  currentBuffer->dotPos = 0;
  if (lineNo < lineCount) {		/* go backward some lines */
    x = lineCount - lineNo;
    if (x < currentWindow->numLines) {
      moveUp(x);
    }
    else {
      for (;x > 0; x--)
	currentBuffer->dotLine = currentBuffer->dotLine->last;
      displayWindow(currentWindow);
    }
  }
  else {  				/* go forward some lines */
    x = lineNo - lineCount;
    if (x < currentWindow->numLines) {
      moveDown(x);
    }
    else {
      for (;x > 0  && !currentBuffer->dotLine->isEof; x--)
	currentBuffer->dotLine = currentBuffer->dotLine->next;
      displayWindow(currentWindow);
    }
  }
  setCursor();
}







/*****/
gotoColumn()
{
  /* set dotPos to point to the first column >= that in the variable
     lastColumn */

  int row,col,x;

  col = 0;
  row = currentWindow->cursorLine;
  for (x=0; col < lastColumn && x<currentBuffer->dotLine->sz; x++)
    toScreen(currentWindow, &row,&col,currentBuffer->dotLine->value[x]);
  currentWindow->cursorColumn = col - currentWindow->horzScroll;
  currentBuffer->dotPos = x;
}







/*****/
moveUp(count)
{
  int realCount;
  

  for (realCount=0; realCount<count && currentBuffer->dotLine->last;
            realCount++) {
    currentBuffer->dotLine = currentBuffer->dotLine->last;
    currentBuffer->dotPos = 0;   /* lines should have at least one character (newline) */

    if (termType != SUNTERM) mvupscr();
  }
  if (termType == SUNTERM) {
    currentWindow->cursorLine -= realCount;
    if (currentWindow->cursorLine < currentWindow->firstLine) {
      currentWindow->cursorLine = currentWindow->firstLine +
                    currentWindow->numLines/2;
      displayWindow(currentWindow);
    }
  }
}




/*****/
mvupscr()
{     /* adjust the screen for a move to previous line in buffer */
  currentWindow->cursorColumn = 0;
  currentWindow->cursorLine--;
  if (currentWindow->cursorLine < currentWindow->firstLine) {
    currentWindow->cursorLine = currentWindow->firstLine;
    scrollDown(1);
    displayLine(currentWindow,currentBuffer->dotLine,currentWindow->firstLine);
  }
}


/*****/
mvdnscr()
{     /* adjust the cursor/screen for a move to the next line */
  currentWindow->cursorColumn = 0;
  currentWindow->cursorLine++;
  if (currentWindow->cursorLine > currentWindow->lastLine) {
    currentWindow->cursorLine = currentWindow->lastLine;
    scrollUp(1);
    displayLine(currentWindow,currentBuffer->dotLine,currentWindow->lastLine);
#ifdef CYGNUS
    displayLabel(currentWindow);
#endif
  }
}




/*****/
moveDown(count)
{
  int realCount;
  
  for (realCount=0; realCount<count && currentBuffer->dotLine->next;
            realCount++) {
    currentBuffer->dotLine = currentBuffer->dotLine->next;
    currentBuffer->dotPos = 0;   /* lines should have at least one character (newline) */

    if (termType != SUNTERM)
      mvdnscr();
  }
  if (termType == SUNTERM) {
    currentWindow->cursorLine += realCount;
    if (currentWindow->cursorLine > currentWindow->lastLine) {
      currentWindow->cursorLine = currentWindow->firstLine +
                    currentWindow->numLines/2;
      displayWindow(currentWindow);
    }
  }
}




/*****/
moveRight()
{
  int x, aChar;
  if (currentBuffer->dotLine->isEof) return 1;
  x = currentBuffer->dotPos + 1;
  if (x > currentBuffer->dotLine->sz) {
    currentBuffer->dotPos = 0;
    moveDown(1);
    computeCursorCol();
  }
  else {
    aChar = charAt(currentBuffer->dotLine,currentBuffer->dotPos);
    toScreen(currentWindow, &currentWindow->cursorLine,&currentWindow->cursorColumn,aChar);
    currentBuffer->dotPos = x;
  }
}



/*****/
moveLeft()
{
  int i, x, aChar;
  x = currentBuffer->dotPos - 1;
  if (x < 0) {

    if (currentBuffer->dotLine->last == NULL) return 1;

    /* go back a line */
    moveUp(1);
    currentBuffer->dotPos = currentBuffer->dotLine->sz;
    currentWindow->cursorColumn =
	strlen(screen[currentWindow->cursorLine]);

  }
  else {
    aChar = charAt(currentBuffer->dotLine,x);
    currentBuffer->dotPos = x;
    /* see if we're skipping back over a control character (^x) or we're
       out past the end of the screen. If so, we must recompute the
       cursor column from scratch */
    if (aChar < 32 || currentWindow->cursorColumn >= terminalLines)
      computeCursorCol();
    else
      currentWindow->cursorColumn--;
  }
}



/*****/
movePage()
{
  int x = 0;
  int rep;
  aLine *line;


  rep = repCount * currentWindow->numLines;
  line = currentBuffer->dotLine;


  if (cmdDirection == '>') {
    for (x=1; x<rep && (! line->isEof) && !intAbort; x++)
      line = line->next;
    if (x < currentWindow->numLines) {
      moveDown(x-1);    
      return 1;
    }
  }
  else {
    for (x=1; x<rep && line !=NULL && !intAbort; x++)
      line = line->last;
    if (line == NULL) {
      line = currentBuffer->firstLine;
      moveUp(x-2);
      return 1;
    }
  }
  currentBuffer->dotLine = line;
  currentBuffer->dotPos = 0;
  displayWindow(currentWindow);
}







/*****/
boolean isSeparator(aChar)
  char aChar;
{
  int i;

  for (i=0; i<seplen; i++) {
    if (aChar == separators[i]) return(TRUE);
  }

/*  this is the is/1 c version:
  if (any(aChar,separators) != 0) return(TRUE);
*/

  return(FALSE);
}




/*****/
moveWords()
{
  int x, aChar;

  if (cmdDirection == '>') {
    for (x=1; x<=repCount && !currentBuffer->dotLine->isEof && !intAbort; x++) {
      /* if on a separator, skip to next non-separator.  Otherwise
         skip to the next separator first */
      aChar = charAt(currentBuffer->dotLine,currentBuffer->dotPos);
      if (!isSeparator(aChar)) {
	do {
	  aChar = nextCchar();
	} while (!isSeparator(aChar) && aChar != EOF);
	if (aChar != EOF && currentBuffer->dotPos == 0)
	  mvdnscr();
	}
      do {
	aChar = nextCchar();
	if (aChar != EOF && currentBuffer->dotPos == 0)
	  mvdnscr();
	} while (isSeparator(aChar) && aChar != EOF);
    }
  }
  else {
    for (x=1; x<=repCount && !(currentBuffer->dotLine == NULL) && !intAbort; x++) {
      do {
	aChar = lastCchar();
	} while (!isSeparator(aChar) && aChar != EOF);
      if (aChar == '\n') mvupscr();
      do {
        aChar = lastCchar();
	if (aChar == '\n') mvupscr();
	} while (isSeparator(aChar) && aChar != EOF);
      if (aChar != EOF)
	aChar = nextCchar();
      else
	aChar = nextCchar();
    }
  }
  computeCursorCol();
}

