/* c o p y i n g   t e x t ---------------------------------------------*/


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
copyText()
{
  /* copy the text marked by dot 10 and the current position */

  aLine *line, *startLine, *endLine, *lastLine, *newLine;
  int   x,y,z,i,startPos,endPos,aPos;
  char *srcPtr, *dstPtr, aChar;


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
    startPos = currentBuffer->dotPos;
    startLine = currentBuffer->dotLine;
    /* move forward to find line/pos of x */
    endLine = currentBuffer->dotLine;
    endPos  = currentBuffer->dotPos;
    aPos = y;
    while (aPos < x  && !endLine->isEof) {
      nextChar(&endLine,&endPos);
      aPos++;
    }
  }
  else {
    endPos = currentBuffer->dotPos;
    endLine = currentBuffer->dotLine;
    /* move backward to find line/pos of x */
    startLine = currentBuffer->dotLine;
    startPos  = currentBuffer->dotPos;
    aPos = y;
    while (aPos > x) {
      lastChar(&startLine,&startPos);
      aPos--;
    }
  }


  /* now, startLine/startPos and endLine/endPos are known - copy the text */
  
  if (startLine == endLine) { /* special case for one-line copy */
    newLine = makeNewLine(endPos-startPos+2,endPos-startPos);
    for (i=startPos; i<endPos; i++)
      newLine->value[i-startPos] = startLine->value[i];
    cutBuffer->firstLine =
    cutBuffer->lastLine  = newLine;
  }
    
  else {  /* startLine != endLine */

    /* copy contents from the first line */
    strcpy(lineBuff, &startLine->value[startPos]);
    cutBuffer->firstLine = makeNewLine(strlen(lineBuff)+1, strlen(lineBuff)-1);
    strcpy(cutBuffer->firstLine->value, lineBuff);
    lastLine = cutBuffer->firstLine;

    /* copy lines up to end line */
    for (line=startLine->next; line != endLine; line=line->next) {
      newLine = makeNewLine(line->sz+4, line->sz);
      strcpy(newLine->value, line->value);
      newLine->last = lastLine;
      lastLine->next = newLine;
      lastLine = newLine;
    }

    /* copy contents from end line */
    newLine = makeNewLine(line->sz, line->value);
    newLine->sz = endPos;
    strcpy(newLine->value, line->value);
    newLine->value[endPos] = '\0';
    newLine->last = lastLine;
    lastLine->next = newLine;
    cutBuffer->lastLine = newLine;
  }
    
  /* add an EOF line to cutbuffer */
  newLine = makeNewLine(2, 0);
  newLine->value[0] = '\n';
  newLine->value[1] = '\0';
  newLine->isEof    = TRUE;
  newLine->last     = cutBuffer->lastLine;
  newLine->last->next = newLine;
  cutBuffer->lastLine = newLine;

  notify("copied");
}

