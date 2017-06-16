/*  t e x t   d e l e t i o n ---------------------------------------- */


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
mergeLines()

{  /* merge this line with the next one */

  aLine *line,*line2;
  int x,s2;
  char *newvalue;

  line = currentBuffer->dotLine;

  /* create the new line */

  s2 = line->sz + line->next->sz + 1; 	/* sizes start at 0 */
  newvalue = myalloc(s2 + (s2 % 2) + 6);

  line->value[line->sz] = 0;			/* make sure the null's there */
  strcpy(newvalue,line->value);

  for (x=0; x<=line->next->sz; x++) 
        newvalue[line->sz + x] = line->next->value[x];
  newvalue[s2] = 0;

  /* make the new string the value of the current line */
  free(line->value);
  line->value = newvalue;
  line->sz = strlen(newvalue) - 1;

  /* remove the next line */
  line2 = line->next;
  line->next = line->next->next;
  line->next->last = line;
  free(line2->value);
  free(line2);

}



/*****/
delToBeg()
{
  aLine *line;
  int x;

  if (currentBuffer->readOnly) {
    notifyReadOnly();
    return(0);
  }

  line = currentBuffer->dotLine;
  if (line->isEof  || currentBuffer->dotPos <= 0) return 1;

  /* save the text to be deleted */
  strcpy(delLin,currentBuffer->dotLine->value);
  delLin[currentBuffer->dotPos] = 0;
  delLinSize = strlen(delLin) - 1;

  for (x=currentBuffer->dotPos; x<=line->sz; x++)
    line->value[x-currentBuffer->dotPos] = line->value[x];

  line->sz = line->sz - currentBuffer->dotPos;
  line->value[line->sz+1] = 0;

  currentBuffer->dotPos = 0;
  setModified(TRUE);

  displayLine(currentWindow, line, currentWindow->cursorLine);
  currentWindow->cursorColumn = 0;
  setCursor();
}


/*****/
delToEnd()
{
  aLine *line;
  int x;

  if (currentBuffer->readOnly) {
    notifyReadOnly();
    return(0);
  }

  line = currentBuffer->dotLine;
  if (line->isEof) return 1;

  /* save the text to be deleted */
  strcpy(delLin,line->value+currentBuffer->dotPos);
  if (delLin[(x=strlen(delLin)-1)] == 10)
    delLin[x] = 0;
  delLinSize = strlen(delLin) - 1;

  line->value[currentBuffer->dotPos] = 10;
  line->value[currentBuffer->dotPos+1] = 0;
  line->sz = currentBuffer->dotPos;

  setModified(TRUE);

  dispToEol();
  setCursor();
}





/*****/
delThruEnd(chScr)
  boolean chScr;
{
  aLine *line;

  if (currentBuffer->readOnly) {
    notifyReadOnly();
    return(0);
  }

  line = currentBuffer->dotLine;
  if (line->isEof) return 1;
  if (currentBuffer->dotPos < 0) currentBuffer->dotPos++;

  /* save the text to be deleted */
  strcpy(delLin,&currentBuffer->dotLine->value[currentBuffer->dotPos]);
  delLinSize = strlen(delLin) - 1;

  line->sz = currentBuffer->dotPos;

  /* take care of the case of this being the last line of valid
     text */
  if (line->next->isEof) {
    line->value[line->sz] = '\n';
    line->value[line->sz+1] = 0;
    setCursor();
    eraseToEnd();
    return 1;
  }

  mergeLines();		/* merge the next line with this one */

  if (chScr) {
    dispToEol();		/* display the merged text */
    scrollToCurr();		/* scroll up the screen to remove the next line */
    setCursor();
  }

  setModified(TRUE);
}
  





/*****/
rubChar()
{
  int x, s1, s2, row, col;
  int aChar;
  aLine *line2,*line;
  char *newvalue;

  if (currentBuffer->readOnly) {
    notifyReadOnly();
    return(0);
  }

  if (currentBuffer->dotPos <= 0  &&  currentBuffer->dotLine->last == NULL) {
    bell();		   /* check for beg of buffer */
    return 1;
  }

  moveLeft();
  aChar = charAt(currentBuffer->dotLine,currentBuffer->dotPos);

  line = currentBuffer->dotLine; /* shorthand */


  if (aChar != '\n') {
    for (x=currentBuffer->dotPos; x<=line->sz; x++)
      line->value[x] = line->value[x+1];
    line->value[line->sz] = 0;
    (line->sz)--;
  }
  else {
    if (line->next->isEof) return 1; /* force an eoln at eof */

    mergeLines();
    scrollToCurr();
  }

  dispToEol();
  setModified(TRUE);
  deletedChar = aChar;
}



  




