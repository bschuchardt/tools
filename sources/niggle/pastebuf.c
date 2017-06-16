/*  r o u t i n e   f o r   p a s t i n g   o n e   b u f f e r   i n t o
      a n o t h e r ------------------------------------------------------- */
      

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
pasteBuffer(buff,copyLines)
  aBuffer *buff;
  boolean copyLines;	/* TRUE if lines should be cloned */
{
  aLine *line, *firstLine, *lastLine, *newLine, *dotLine,
        *firstInsertedLine, *nextLine, *insLine;
  int i, j, x, numCLines, oldDotPos, oldCursorCol;
  boolean repaint, partialLastLine;
  char *temp;

  oldDotPos = currentBuffer->dotPos;
  oldCursorCol = currentWindow->cursorColumn;

  if (buff->firstLine->isEof) {
    setCursor();
    return 1;
  }

  if (copyLines) firstLine = lastLine = NULL;
  else  {
    firstLine = buff->firstLine;
    lastLine  = buff->lastLine;
  }

  /* count lines and clone buffer if necessary.  After this,
     numCLines == number of lines (including EOF)
     firstLine == first line of buffer
     lastLine  == last line of buffer (EOF)
     */
  line = buff->firstLine;
  numCLines = 0;
  do {
    if (numCLines) line = line->next;  /* skip first time through */
    numCLines++;
    if (copyLines) {
      newLine = makeNewLine(line->sz+1, line->sz);
      if (!line->isEof)
        strcpy(newLine->value, line->value);
      newLine->isEof = line->isEof;
      newLine->last = lastLine;
      if (lastLine) lastLine->next = newLine;
      lastLine = newLine;
      if (!firstLine) firstLine = newLine;
    }
  } while (!line->isEof);

  if (lastLine) lastLine->next = NULL;
  else          lastLine = buff->lastLine;  /* i.e. didn't copy buffer lines */

  repaint = currentWindow->cursorLine + numCLines > currentWindow->lastLine;

  /* save and delete the current line to simplify the insertion
     dotLine == cut line
     instLine == line prior to insertion point */

  if (!currentBuffer->dotLine->isEof) {
    dotLine = currentBuffer->dotLine;
    if (dotLine->last) dotLine->last->next = dotLine->next;
    else               currentBuffer->firstLine = dotLine->next;
    dotLine->next->last = dotLine->last;  /* since not eof, next can't be null */
    insLine = dotLine->last;
    currentBuffer->dotLine = dotLine->last;  /* this may be NULL */
  }
  else {
    /* if at eof, simply set the insertion point to just before the eof line */
    dotLine = NULL;
    insLine = currentBuffer->dotLine->last;
  }

  /* insert the text into the buffer */

  /* first the head of the buffer */
  if (insLine == NULL) {
    nextLine = currentBuffer->firstLine;
    currentBuffer->firstLine = firstLine;
  }
  else {
    firstLine->last = insLine;
    nextLine = insLine->next;
    insLine->next = firstLine;
  }

  /* now the tail of the buffer */
  lastLine->last->next = nextLine;
  nextLine->last = lastLine->last;
  free(lastLine->value);
  free(lastLine);	/* discard the buffer's eof record */


  if (currentBuffer->dotLine)  /* check for first line being removed (above) */
    firstInsertedLine = currentBuffer->dotLine->next;
  else
    firstInsertedLine = currentBuffer->firstLine;

  /* now add the text from dotLine */
  if (dotLine) {
    /* put the front text in */
    line = firstInsertedLine;
    temp = myalloc(line->sz+currentBuffer->dotPos+4);
    if (currentBuffer->dotPos)
      strncpy(temp, dotLine->value, currentBuffer->dotPos);
    strcpy(temp+currentBuffer->dotPos, line->value);
    free(line->value);
    line->value = temp;
    line->sz += currentBuffer->dotPos;

    /* put the end text in (nextLine == line after insertion) */
    line = nextLine->last;
    if (line->value[line->sz] == '\n') { /* terminated line? */
      /* line is terminated: add dotLine text in a line after it */
      partialLastLine = FALSE;
      x = strlen(dotLine->value+currentBuffer->dotPos);
      newLine = makeNewLine(x, x-1);
      strcpy(newLine->value, dotLine->value+currentBuffer->dotPos);
      newLine->last = line;
      newLine->next = line->next;
      line->next = newLine;
      newLine->next->last = newLine;
      currentBuffer->dotLine = newLine;
      currentBuffer->dotPos = 0;
    }
    else {
      /* line is not terminated: append dotLine text to it */
      partialLastLine = TRUE;
      temp = myalloc(line->sz+dotLine->sz-currentBuffer->dotPos+4);
      strcpy(temp, line->value);
      strcpy(temp+line->sz, dotLine->value+currentBuffer->dotPos);
      free(line->value);
      line->value = temp;
      x = line->sz; /* save the old size */
      line->sz = strlen(line->value)-1;
      currentBuffer->dotLine = line;
      currentBuffer->dotPos = x;
    }

    computeCursorCol();
    free(dotLine->value);
    free(dotLine);
  }
  else {
    /* else dotLine was not saved, so dotLine & dotPos are correct
       just have to fix up the last line, in case it was a partial line
       (may need a line terminator) */
    if (nextLine->last  &&  nextLine->last->value[0] == 0) {
      nextLine->last->value[0] = '\n';
    }
  }

  if (!repaint) {
    x = numCLines-1;
    if (partialLastLine) x--;
    if (x) {
      /* move lines down and draw new lines */
      if (termType == VT100TERM) {
        setScroll(currentWindow->cursorLine,currentWindow->lastLine);
        positionCursor(currentWindow->cursorLine,0);
        upTermLines(x);
        setScroll(0,terminalLines);
      }
      for (i=0; i<x; i++) {  /* scroll the screen memory */
        temp = screen[currentWindow->lastLine];
        for (j=currentWindow->lastLine-1; j>=currentWindow->cursorLine; j--)
          screen[j+1] = screen[j];
        screen[currentWindow->cursorLine] = temp;
      }
      if (termType != VT100TERM) {
        for (i=currentWindow->cursorLine+x; i<=currentWindow->lastLine; i++) {
          positionCursor(i,0);
          fputs(screen[i], stdout);  eraseToEnd();
        }
      }
      line = firstInsertedLine;
      /* 6/7/88 - added following line to fix drawing lines past window bottom */
      if ((x+currentWindow->cursorLine) > currentWindow->lastLine)
        x = currentWindow->labelLine - currentWindow->cursorLine;
      for (i=0; i<=x; i++) {
        displayLine(currentWindow, line, currentWindow->cursorLine+i);
        if (line) line = line->next;
      }
      currentWindow->cursorLine += x;
    }
    else {
      /* no new lines were added: merely display to end of line from
         the old line/column position */
      i = currentWindow->cursorColumn;
      j = currentBuffer->dotPos;
      currentWindow->cursorColumn = oldCursorCol;
      currentBuffer->dotPos = oldDotPos;
      dispToEol();
      currentWindow->cursorColumn = i;
      currentBuffer->dotPos = j;
    }
  }
  else {
    /* repaint the whole screen */
    if (currentWindow->cursorLine == currentWindow->firstLine) {
      currentWindow->cursorLine = currentWindow->firstLine +
      (currentWindow->numLines / 2);
    }
    displayWindow(currentWindow);
  }

  setModified(TRUE);

  setCursor();
}

