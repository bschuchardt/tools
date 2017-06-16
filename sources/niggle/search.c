/* s e a r c h i n g --------------------------------------------------- */


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
#include "buffer.h"

extern char *re_comp();
extern int re_exec();


/*****/
char *strupr(s)
  char *s;
  {
  char *orig = s;

  while (*s) {
    *s = (*s >= 'a' && *s <= 'z')? *s-32 : *s;
    s++;
  }
  return orig;
}

removeRegEx(s)  /* remove regular expression functionality from string */
  char *s;
  {
  char *t;
  char tmp[512];
  
  strcpy(tmp, s);
  t = tmp;
  while (*t) {
    switch (*t) {
    case '.':   case '^':
    case '$':   case '*':
    case '[':   case ']':
    case '\\':
      *s++ = '\\';
    default:
      *s++ = *t++;
    }
  }
  *s++ = '\0';
}
  


/*****/
search(again)
  boolean again;
{
  int x,y, newSize, row,col, lineCount;
  boolean newline,aborted,found,forward,terminate,matchStart;
  aLine *base;
  int basePos;
  int aChar;
  char *compError;


  /* prepare */

  row = promptLine;

  /* get the search string from the user */

  if (again) {
    newSize = -1;
  }
  else {
    strcpy(lineBuff, sString);
    if (getTtyData(row,"search for: ",&newSize) < 0) return 1;
  }

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

  /* do the search */

  if (!again) positionCursor(promptLine, 0);

  if (forward)
    nextChar(&base,&basePos);
  else
    lastChar(&base,&basePos);
  if (newLine) {
    strcpy(tempBuff, base->value);
    if (!caseSensitive) strupr(tempBuff);
    lineCount++;
  }

  aborted = FALSE;
  found = FALSE;
  mdeassign();

  while (!found && !aborted) {
    if (  (forward  &&  base->isEof)
       || (!forward  &&  basePos == 0  &&  base->last == NULL)
       || intAbort)
      aborted = TRUE;
    else {
      newLine = FALSE;
      x = re_exec(tempBuff+basePos);
      if (x == -1) {
#ifdef UNIX             /* my grep will automatically notify */
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
  }

  /* now we've either found it or not found it.
     if we have, lineCount will hold the number of lines we've moved */

  if (!aborted) {
    if (lineCount > currentWindow->numLines) {
      currentBuffer->dotLine = base;
      currentBuffer->dotPos = basePos;
      displayWindow(currentWindow);
    }
    else {
      if (forward) moveDown(lineCount);
      else moveUp(lineCount);           /* this changes dotLine */
      currentBuffer->dotPos = basePos;
      computeCursorCol();
    }
  }
  else {
    notify("no match"); bell();
  }
  redrawStatus = TRUE;
  setCursor();
  massign();
}
