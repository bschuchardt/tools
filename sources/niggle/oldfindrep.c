/* s e a r c h   a n d   r e p l a c e---------------------------------- */


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
findrep(again)
  boolean again;
{
  int x,y,newSize,rep;
  int row,col, lineCount,tmpCount;
  boolean notAbort, notFound, forward, didAsk, doit, match, mdone;
  boolean ask = TRUE;
  boolean global = FALSE;
  aLine *base,*tempBase,*saveDot;
  int basePos,tempPos,savePos;
  char *newValue;
  int saveCLine, saveCColumn;
  int aChar;
  int changeCount = 0;

  if (currentBuffer->readOnly) {
    notifyReadOnly();
    return(0);
  }

  /* prepare */

  row = promptLine;
  saveDot = currentBuffer->dotLine;
  savePos = currentBuffer->dotPos;
  saveCLine = currentWindow->cursorLine;
  saveCColumn = currentWindow->cursorColumn;

  /* get the search and replacement strings from the user */

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

  if (getTtyData(row,"replace with: ",&newSize) < 0) return 1;
  for (y=0; y<=newSize; y++) {
    if (lineBuff[y] == 13) lineBuff[y] = '\n';
  }
  rSize = newSize;
  strcpy(rString,lineBuff);

  /* prepare to search */

  if (cmdDirection == '>') forward=TRUE; else forward=FALSE;
  base = currentBuffer->dotLine;
  basePos = currentBuffer->dotPos;
  lineCount = 0;

  /* do the search */

  for (rep=1, notAbort=TRUE;  notAbort; rep++) {
    if (!forward) {
      aChar = lastChar(&base,&basePos);
      if (newLine) lineCount++;
    }
    notFound = TRUE;
    notAbort = !intAbort;
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

	do {		/* the actual matching is done here */
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

      /* do the replacement, prompting the user */

      if (!notFound && !intAbort) {
	 doit = TRUE;
	 didAsk = FALSE;
	 if (ask) {
	    lineCount += tmpCount;
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
	      tmpCount = lineCount = 0;
	    }
            lineCount = 0;
	    aChar = promptc("replace(y/n/g)? ",FALSE);
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
	   x = base->sz + 1 - (sSize + 1) + (rSize + 1);
	   newValue = malloc(x + 4);
	   for (y = 0; y < basePos; y++) {
	     newValue[y] = base->value[y];
	   }
	   for (y = 0; y <= rSize; y++) {
	     newValue[y + basePos] = rString[y];
	   }
	   x = basePos + rSize;
	   for (y = tempPos + 1; y <= base->sz; y++) {
	     newValue[++x] = base->value[y];
	   }
	   newValue[x + 1] = '\0';
	   free(base->value);
	   base->value = newValue;
	   base->sz = x;
	   if (didAsk)
	     displayLine(currentWindow,base,currentWindow->cursorLine);
	 }
	 if (forward) {
	   if (doit) basePos = basePos + rSize;
	   aChar = nextChar(&base,&basePos);
	   if (newLine) lineCount++;
	 }
	 }
    }
  }


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
}


