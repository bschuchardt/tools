/*  p a r a g r a p h   m a n a g e m e n t-----------------------------  */

/*
 * name - %M%
 * ccsid - %W% - %G% %U%
 * from  - %F%
 * date  - %H% %T%
 */


#include "types.mh"
#include "defn.h"

#define IS_PSEP(c) ((c)=='\n' || (c)=='.')
#define IS_BOP(c) ((c)==' ' || (c)==9)



/*****/
wrapCurrentParagraph(count)
  int count;
{
  /* word-wrap the current paragraph in the current buffer */
  unsigned char aChar;

  if (currentBuffer->readOnly) {
    notifyReadOnly();
    return(0);
  }

  while (count-->0  &&  !intAbort) {
    /* if this is a blank line, just move down one */
    if (currentBuffer->dotLine->isEof) return 1;

    while (IS_PSEP(currentBuffer->dotLine->value[0])) {
      moveDown(1);
      setCursor();
      if (currentBuffer->dotLine->isEof) return 1;
    }

    /* find start of paragraph */
    currentBuffer->dotPos = 0;
    for (aChar=currentBuffer->dotLine->value[0];
        !IS_BOP(aChar) &&
          currentBuffer->dotLine->last != NULL &&
          !IS_PSEP(currentBuffer->dotLine->last->value[0]);
        moveUp(1), aChar=currentBuffer->dotLine->value[0])
    {}
  
    /* wrap until a new paragraph or eof is hit */
    do {
      wrapCurrentLine(1);
      aChar = currentBuffer->dotLine->value[0];
    } while (!currentBuffer->dotLine->isEof
                && !IS_PSEP(aChar) && !IS_BOP(aChar) && !intAbort);
  }
  computeCursorCol();
  setCursor();
}





/*****/
wrapCurrentLine(count)
  int count;
{
  /* word-wrap the current line, and possibly subsequent lines
     to fit into the current right-margin */
  int x, y, pos, col;
  unsigned char aChar;
  boolean oldAutoIndent;
  
  /* turn off auto-indent while working here */
  oldAutoIndent = autoIndent;
  autoIndent = FALSE;

  while (count-->0  &&  !intAbort) {
    if (currentBuffer->dotLine->isEof)
      break;
    if (IS_PSEP(currentBuffer->dotLine->value[0])) {
      moveDown(1);
      setCursor();
      return 1;
    }
  
    
    /* make sure we have a long line.  Merge subsequent lines if necessary */
    while (displayedLength(currentBuffer->dotLine->value) < wrapMargin-2  &&
           /* don't merge next line if it is eof or a new paragraph */
           !currentBuffer->dotLine->next->isEof  &&
           (aChar=currentBuffer->dotLine->next->value[0],
           !IS_BOP(aChar) && !IS_PSEP(aChar)) && !intAbort) {
      /* append a space or two first, if needed */
      aChar = currentBuffer->dotLine->value[
                  currentBuffer->dotLine->sz - 1];
      if (aChar != ' '  &&  aChar != 9  && aChar != '-') {
        currentBuffer->dotPos = currentBuffer->dotLine->sz;
        computeCursorCol();
        setCursor();
        insertChar(' ');
        if (aChar == '.') insertChar(' ');
      }
      mergeLines();
      setModified(TRUE);
      dispToEol();
      scrollToCurr();
    }
  
    /* OK, if we really have a line that can be wrapped, do the wrapping
       now */
    if (displayedLength(currentBuffer->dotLine->value) > wrapMargin) {
      /* start at the beginning and sequence forward over words until
         the next word would put us past the wrapMargin */
      x = 0;
      col = 0;
      pos = 0;
      while (col <= wrapMargin  && !intAbort) {
        /* skip separators */
        while ((aChar=currentBuffer->dotLine->value[pos]) != NULL  &&
               aChar != '\n'  &&
               aChar != '('  &&    /* don't wrap at left parens */
               isSeparator(aChar)  && !intAbort) {
          col += charWidth(aChar, col);
          pos++;
        }
        if (aChar) {
          /* sequence over word */
          x = pos;
          do {
            col += charWidth(aChar, col);
            pos++;
          } while ((aChar=currentBuffer->dotLine->value[pos]) != NULL  &&
               aChar != '\n'  &&
              !isSeparator(aChar)  &&  !intAbort);
        }
      }
      currentBuffer->dotPos = x;
      computeCursorCol();
      if (currentBuffer->dotLine->value[x] == '\n') {  /* weird case */
        moveRight(1);
        setCursor();
      }
      else {
        setCursor();
        insertChar('\n');
      }
    }
    else { /* not a long line - go to the next line to show that this
            line has been done. */
      moveDown(1);
      setCursor();
    }
  
  }
  autoIndent = oldAutoIndent;
}




/*****/
displayedLength(text)
  char *text;
{
  /* how long is the text when displayed? */
  int col, nonWhite;
  
  col = 0;
  nonWhite = 0;
  while (*text  && !intAbort) {
    col += charWidth(*text, col);
    if (*text != ' '  &&  !text != 9) nonWhite = col;
    text++;
  }
  return nonWhite;
}

