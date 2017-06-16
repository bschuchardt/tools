/* c u t t i n g   t e x t ---------------------------------------------*/


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
cutText()
{
  /* cut the text marked by dot 10 and the current position */

  aLine *line, *srcLine,*lastLine, *firstLine;
  boolean done,chScr,partialFirstLine,partialLastLine,
          fixUpSecondLineBackpointer;
  int   x,y,i;
  char *srcPtr, *dstPtr, aChar;

  if (currentBuffer->readOnly) {
    notifyReadOnly();
    return(0);
  }

  if (cutBuffer->firstLine != NULL) {
    while (cutBuffer->firstLine->next) {
      free(cutBuffer->firstLine->value);
      cutBuffer->firstLine = cutBuffer->firstLine->next;
      free(cutBuffer->firstLine->last);
    }
    free(cutBuffer->firstLine->value);
    free(cutBuffer->firstLine);
  }

  cutBuffer->firstLine = cutBuffer->lastLine = cutBuffer->dotLine = NULL;
  cutBuffer->dotPos = 0;
  cutBuffer->modified  = FALSE;




  x = currentBuffer->aDot[9];
  y = curCharNo( &i /*junk*/ );
  if (x == y) return 1;
  if (x > y) {
    i = x;
    x = y;
    y = i;
  }
  else
    gotoPos(x);     /* set to the start of the text to be cut */


  srcLine = currentBuffer->dotLine;
  srcPtr  = currentBuffer->dotLine->value + currentBuffer->dotPos;
  lastLine = NULL;
  dstPtr  = lineBuff;
  aChar = '\0';

  partialFirstLine = FALSE;
  fixUpSecondLineBackpointer = FALSE;
  for (i = x; i < y; i++) {
    *dstPtr++ = aChar = *srcPtr++;
    if (aChar == '\n') {
      /* create the new line or move existing line */
      if (srcLine != currentBuffer->dotLine  /* a whole line after 1st */
         || currentBuffer->dotPos == 0) {    /* or the whole 1st line  */
        if (cutBuffer->firstLine == NULL)
          cutBuffer->firstLine = srcLine;
        lastLine = srcLine;
      }
      else {                                 /* partial first line     */
        *dstPtr = '\0';
        line = makeNewLine(strlen(lineBuff), strlen(lineBuff)-1);
        line->next = srcLine->next;
        /* avoid modifying the lines in currentBuffer for now.  The
           backpointer for the second cut line will be modified later,
           when we actually know that a second line is to be cut */
        fixUpSecondLineBackpointer = TRUE;
        strcpy(line->value, lineBuff);
        partialFirstLine = TRUE;
        lastLine = line;
        cutBuffer->firstLine = line;
      }
      dstPtr = lineBuff;
      srcLine = srcLine->next;
      srcPtr  = srcLine->value;
    }
  }

  /* copy any partial last line */
  partialLastLine = FALSE;
  if (aChar != '\n') {
    partialLastLine = TRUE;
    *dstPtr = '\0';
    line = makeNewLine(strlen(lineBuff), strlen(lineBuff)-1);
    line->last = lastLine;
    if (lastLine) lastLine->next = line;
    line->sz++;
    line->isEof = FALSE;
    strcpy(line->value, lineBuff);
    cutBuffer->lastLine = line;
    if (!cutBuffer->firstLine) {
      cutBuffer->firstLine = line;      /* single line cut */
      if (currentBuffer->dotPos) partialFirstLine = TRUE;
    }
  }
  else {
    cutBuffer->lastLine = lastLine;
  }

  /* All of the information has now been attached or copied to the
     cutBuffer.  Now we'll remove the attached lines (and any
     partial first line).  After that we will fix up the cutBuffer
     pointers and then handle any partial first and last lines
     remaining in currentBuffer.

     srcLine points to the line containing the next character after
     the cut.
   */

  if (currentBuffer->dotLine->last == NULL)    /* we cut the first line */
    currentBuffer->firstLine = srcLine;
  else
    currentBuffer->dotLine->last->next = srcLine;

  srcLine->last = currentBuffer->dotLine->last;
  firstLine = currentBuffer->dotLine;  /* save dotLine */
  currentBuffer->dotLine = srcLine;


  if (fixUpSecondLineBackpointer &&  cutBuffer->firstLine->next)
     cutBuffer->firstLine->next->last = cutBuffer->firstLine;
  cutBuffer->firstLine->last = NULL;
  cutBuffer->lastLine->next = NULL;
  cutBuffer->dotLine = cutBuffer->firstLine;



  /* merge partial lines */

  strcpy(lineBuff, firstLine->value);  /* save firstLine's contents
                                          so they won't change */

  if (partialLastLine) {
    /* this could change firstLine's contents */
    strcpy(srcLine->value, srcPtr);
    srcLine->sz = strlen(srcLine->value) - 1;
  }

  if (partialFirstLine) {
    /* make a new value for the end line that includes the
       prefix from the partially deleted first line */

    dstPtr = lineBuff + currentBuffer->dotPos;
    strcpy(dstPtr, srcLine->value);
    free(srcLine->value);
    i = strlen(lineBuff) + 1;
    srcLine->value = myalloc(i);
    strcpy(srcLine->value, lineBuff);
    srcLine->sz = i - 2;

  }

  setModified(TRUE);

  /* add an EOF line to cutbuffer */
  if (!cutBuffer->lastLine->isEof) {
    line = makeNewLine(2, 0);
    line->value[0] = '\n';
    line->value[1] = 0;
    line->isEof = TRUE;
    line->last = cutBuffer->lastLine;
    line->last->next = line;
    cutBuffer->lastLine = line;
  }

  if (srcLine != firstLine)
    dispFromCurr();
  if (partialFirstLine  ||  partialLastLine) dispToEol();
  setCursor();

  /* now free the old first line, since it was removed from currentBuffer
     but was not moved to cutBuffer (its value was copied) */
  if (partialFirstLine  &&  !partialLastLine) {
    free(firstLine->value);
    free(firstLine);
  }

}

