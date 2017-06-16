/* s e a r c h i n g --------------------------------------------------- */


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
}
  

/*****/
search(again)
  boolean again;
{
  int x,y,newSize;
  int row,col, lineCount,tmpCount;
  boolean notAbort,notFound,forward;
  boolean match,mdone;
  aLine *base,*tempBase;
  int basePos,tempPos;
  int aChar;


  /* prepare */

  row = promptLine;

  /* get the search string from the user */

  if (again) {
    newSize = -1;
  }
  else {
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

  if (cmdDirection == '>') forward=TRUE; else forward=FALSE;
  base = currentBuffer->dotLine;
  basePos = currentBuffer->dotPos;
  lineCount = 0;

  /* do the search */

/***
  for (x=1, notAbort=TRUE; x<=repCount && notAbort; x++) {
***/  notAbort = TRUE;

    if (!forward) {
      aChar = lastChar(&base,&basePos);
      if (newLine) lineCount++;
    }
    notFound = TRUE;
    while (notFound && notAbort) {
      if ((base->isEof && forward)
         || (basePos == 0  &&  base->last == NULL && !forward) || intAbort)
	notAbort = FALSE;
      else {
	tempBase = base; tempPos = basePos;
	aChar = charAt(tempBase,tempPos);
	y=0;
	tmpCount = 0;
	match = FALSE;
	mdone = FALSE;

	do {	/* the actual matching for the current base is done here */
          if (caseSensitive && aChar != sString[y])
	    mdone = TRUE;
          else if (!caseSensitive && lower(aChar) != lower(sString[y]))
            mdone = TRUE;
	  else if (y < sSize) {
	    y++;
	    aChar = nextChar(&tempBase,&tempPos);
	    if (newLine) tmpCount++;
	  }
	  else {
	    mdone = TRUE;
	    match = TRUE;
	  }
	} while (!mdone);

	newLine = FALSE;
	if (match) notFound = FALSE;

	else if (forward)
	  aChar = nextChar(&base,&basePos);
	else
	  aChar = lastChar(&base,&basePos);
	if (newLine) lineCount++;
	}

      if (!notFound && forward) {  /*if we matched going forward*/
	base = tempBase;	   /*advance to the end of the match*/
	basePos = tempPos;
    	aChar = nextChar(&base,&basePos);
	if (newLine) lineCount++;
	lineCount = lineCount + tmpCount;  /*add those found in matching*/
	}
    }
/****
  }
****/

  /* now we've either found it or not found it.
     if we have, lineCount will hold the number of lines we've moved */

  if (notAbort) {
    if (lineCount > currentWindow->numLines) {
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
  }
  else {
    notify("no match"); bell();
  }
  redrawStatus = TRUE;
  setCursor();
}
