/* s e a r c h   a n d   r e p l a c e---------------------------------- */


/*
 * name - %M%
 * ccsid - %W% - %G% %U%
 * from  - %F%
 * date  - %H% %T%
 */

/* 10/9/90 - modified to deassign the keyboard while searching so that
 *           sigint interrupts can be caught on pee-unix machines */


#include "types.mh"
#include "defn.h"
#include "edit.h"
#include "buffer.h"

extern char *re_comp();
extern int re_exec();

extern char *strupr();      /* from search.c */
extern int removeRegEx();   /* ditto         */




/*****/
findrep(again)
  boolean again;
{
  int x,y, newSize, row,col, lineCount;
  boolean newline,aborted,found,forward,terminate,matchStart;
  aLine *base;
  int basePos;
  int aChar;
  char *compError;

  int savePos, saveCLine, saveCColumn;
  int tSize;
  aLine *saveDot;
  boolean didAsk, doit;
  boolean ask = TRUE;
  boolean global = FALSE;
  char *newValue;
  int changeCount = 0;


  if (currentBuffer->readOnly) {
    notifyReadOnly();
    return 1;
  }

  /* prepare */

  row = promptLine;
  saveDot = currentBuffer->dotLine;
  savePos = currentBuffer->dotPos;
  saveCLine = currentWindow->cursorLine;
  saveCColumn = currentWindow->cursorColumn;

  /* get the search and replacement strings from the user */

  strcpy(lineBuff, sString);
  if (getTtyData(row,"search for: ",&newSize) < 0) return 1;

  /* clean up after the data entry */

  if (newSize >= 0) {		/* copy the new search string */
    for (y=0; y<=newSize; y++) {
      if (lineBuff[y] == 13) lineBuff[y] = '\n';
  }
    sSize = newSize;
    strcpy(sString,lineBuff);
}
  if (sSize < 0) {
    notify("no search string"); bell();
    return 1;
}


  if (intAbort) return 1;

  strcpy(lineBuff, rString);
  if (getTtyData(row,"replace with: ",&newSize) < 0) return 1;
  for (y=0; y<=newSize; y++) {
    if (lineBuff[y] == 13) lineBuff[y] = '\n';
}
  rSize = newSize;
  strcpy(rString,lineBuff);

  /* prepare to search */

  matchStart = sString[0] == '^';
  if (matchStart) strcpy(tempBuff, sString);
  else {
    strcpy(tempBuff, "^");
    strcat(tempBuff, sString);
}
  if (!caseSensitive) strupr(tempBuff);
  if (!regularExpressions) {
    if (matchStart) removeRegEx(tempBuff);
    else removeRegEx(tempBuff+1);
}
  compError = re_comp(tempBuff);
  if (compError != (char *)NULL) {
    notify(compError);
    bell();
    return 1;
}

  if (cmdDirection == '>') forward=TRUE; else forward=FALSE;
  base = currentBuffer->dotLine;
  basePos = currentBuffer->dotPos;
  strcpy(tempBuff, base->value);
  if (!caseSensitive) strupr(tempBuff);
  lineCount = 0;

  /* --  do the search  -- */

  mdeassign();
  aborted = FALSE;
  found   = TRUE;
  while (!aborted && found) {
    positionCursor(row, 0);

    if (forward)
      nextChar(&base,&basePos);
    else
      lastChar(&base,&basePos);
    if (newLine) {
      lineCount++;
  }

    strcpy(tempBuff, base->value);
    if (!caseSensitive) strupr(tempBuff);

    found = FALSE;
    aborted = intAbort;
    while (!found && !aborted) {
      if ((base->isEof && forward)
         || (basePos == 0  &&  base->last == NULL && !forward) || intAbort)
        aborted = TRUE;
      else {
        newLine = FALSE;
        x = re_exec(tempBuff+basePos);
        if (x < 0) {
#ifndef MSDOS
          notify("invalid regular expression");
#endif
          bell();
          massign();
          return 1;
      }
        /* don't match middle of line if user has requested start of line */
        if (x == 1  &&  matchStart  &&  basePos > 0)  x = 0;
        if (x == 1)
          found = TRUE;
        else if (forward)
          nextChar(&base,&basePos);
        else
          lastChar(&base,&basePos);
        if (newLine) {
          strcpy(tempBuff, base->value);
          if (!caseSensitive) strupr(tempBuff);
          lineCount++;
      }
    }
  
      aborted |= intAbort;
      if (!found || aborted) {
        continue;
    }

      /* if we found a match, find out how long the matched string is.
         This code is inefficient, but re_exec() doesn't return the size
         of the matched string.  Sigh... */
      tSize = strlen(tempBuff);
      while (re_exec(tempBuff+basePos) == 1) tempBuff[--tSize] = '\0';
      tSize++;            /* adjust for last matching character */
      tSize -= basePos;   /* and remove base (non-matching) part */
      
      /* do the replacement, prompting the user */
      doit = TRUE;
      didAsk = FALSE;
      if (ask) {
        if (lineCount > currentWindow->numLines) { /* fix the screen */
          currentBuffer->dotLine = base;
          currentBuffer->dotPos = basePos;
          displayWindow(currentWindow);
      }
        else {
          if (forward) moveDown(lineCount);
          else moveUp(lineCount);
          currentBuffer->dotPos = basePos;
          computeCursorCol();
      }
        lineCount = 0;
        massign();
        aChar = promptc("replace(y/n/g)? ",FALSE);
        mdeassign();
        if (aChar == 7  ||  aChar == 3)
          goto frdone;
        else {
          didAsk = TRUE;
          switch (aChar) {
          case 'y':
          case 'Y':
          case ' ':
          case '\n':
          case 13:		/* VMS kludge */
            doit = TRUE;
            break;
          case 'G':
          case 'g':
            global = TRUE;
            doit = TRUE;
            ask = FALSE;
            break;
          default:
            doit = FALSE;
            break;
        }
      }
    }
      doit = doit && !intAbort;
      if (doit) {
        /* replace the old data with the new.  This will involve
           creating a new line-value buffer to hold the text */
        changeCount++;
        x = base->sz + 1 - tSize + rSize;
        newValue = myalloc(x + 4);
        for (y = 0; y < basePos; y++) {
          newValue[y] = base->value[y];
      }
        for (y = 0; y <= rSize; y++) {
          newValue[y + basePos] = rString[y];
      }
        x = basePos + rSize;
        for (y = basePos + tSize; y <= base->sz; y++) {
          newValue[++x] = base->value[y];
      }
        newValue[x + 1] = '\0';
        free(base->value);
        base->value = newValue;
        base->sz = x;
        strcpy(tempBuff, base->value);
        if (!caseSensitive) strupr(tempBuff);
        if (didAsk)
          displayLine(currentWindow,base,currentWindow->cursorLine);
    }
      if (forward) {
        if (doit) basePos = basePos + rSize - 1;
        aChar = nextChar(&base,&basePos);
        if (newLine) {
          lineCount++;
          strcpy(tempBuff, base->value);
          if (!caseSensitive) strupr(tempBuff);
      }
    }
  }
}  /* end major loop */


frdone:
  currentBuffer->dotLine = saveDot;
  currentBuffer->dotPos = savePos;
  currentWindow->cursorLine = saveCLine;
  currentWindow->cursorColumn = saveCColumn;
  displayWindow(currentWindow);
  positionCursor(promptLine,0);
  if (changeCount > 0) {
    printf("%d occurrences changed",changeCount);
    eraseToEnd();
    setModified(TRUE);
}
  else {
    printf("no changes made");
    eraseToEnd();
}
  setCursor();
  redrawStatus = TRUE;
  massign();
}


