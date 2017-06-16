/*  t e r m i n a l   m a n i p u l a t i o n ------------------------ */



/*
 * name - %M%
 * ccsid - %W% - %G% %U%
 * from  - %F%
 * date  - %H% %T%
 */



#include "types.mh"
#include "defn.h"
#include "mgetc.h"


#ifdef MSDOS
#include <conio.h>   /* Borland's console handling routines */
#include <dos.h>
union REGS inregs, outregs;
#endif


int lastScrollTop = -1;
int lastScrollBottom = -1;

/*****/
#ifdef MSDOS
toVideo(row, str)
  int row;
  char *str;
  {
  char far *lstr;

  lstr = (char far *)((unsigned long)0xb8000000 +
                (unsigned long)(2 * 80 * row));
  while (*str) {
    *lstr++ = *str++;
    *lstr++ = videoAttr;  /* attribute byte */
  }
}
#endif


/*****/
gotoPrimaryScreen() {
  fputs(term_primary, stdout);
}

/*****/
gotoAlternateScreen() {
  fputs(term_alternate, stdout);
}

/*****/
#ifdef MSDOS
clearLine(row)
  int row;
  {
  char far *lstr;
  int i;

  lstr = (char far *)((unsigned long)0xb8000000 +
                (unsigned long)(2 * 80 * row));
  for (i=0; i<=terminalColumns; i++) {
    *lstr++ = ' ';
    *lstr++ = videoAttr;  /* attribute byte */
  }
}
#endif






/*****/
/* here a window means the last set scrolling region */
scrUpWindow(nLines)
  int nLines;
{
  int i;

#ifdef MSDOS
  inregs.h.ah = 6;
  inregs.h.al = nLines;
  inregs.h.ch = lastScrollTop - 1;
  inregs.h.cl = 0;
  inregs.h.dh = lastScrollBottom - 1;
  inregs.h.dl = 79;
  inregs.h.bh = 0;
  int86(16, &inregs, &outregs);
#else
  positionCursor(lastScrollBottom-1,0);
  for (i=0; i<nLines; i++) putchar('\n');
#endif
}


/*****/
scrDnWindow(nLines)
  int nLines;
{
#ifdef MSDOS
  inregs.h.ah = 7;
  inregs.h.al = nLines;
  inregs.h.ch = lastScrollTop - 1;
  inregs.h.cl = 0;
  inregs.h.dh = lastScrollBottom - 1;
  inregs.h.dl = 79;
  inregs.h.bh = 0;
  int86(16, &inregs, &outregs);
#else
  positionCursor(lastScrollTop-1, 0);
  upTermLines(nLines);
#endif
}




/*****/
eraseToEnd()
{
#ifdef MSDOS
  clreol();
#else
  if (isColor) {
    fputs(bg_default, stdout);
    fputs(fg_default, stdout);
}
  fputs(term_cleol, stdout);
  if (isColor) {
    fputs(fg_default, stdout);
    fputs(bg_default, stdout);
}
#endif
}


/*****/
eraseScreen()
{
  int i;

#ifdef MSDOS
#ifdef PC_VIDEO
  for (i=0; i<=terminalLines; i++)
    clearLine(i);
#else
  clrscr();
#endif
#else
  if (1) { /* termType == SUNTERM) { */
    positionCursor(0,0);
    printf("%c[J", Esc);
  }
  else
    fputs(term_clear, stdout);
#endif
}



/*****/
positionCursor(line, col)
  int line, col;
{
#ifdef MSDOS
  gotoxy(col+1, line+1);
#else
  if (col==0)
    printf(term_line_position, ++line);
  else
    printf(term_position,++line,++col);
#endif
}



/*****/
setScroll(first,last)
  int first,last;
{
  first++;  last++;
  if (first!=lastScrollTop || last!=lastScrollBottom) {
#ifndef MSDOS
    printf(term_scrollingRegion,first,last);
#endif
    lastScrollTop = first;
    lastScrollBottom = last;
#if defined(CYGNUS_OLD) /* bug in window scrolling */
    positionCursor(0,0);
    fputs(screen[0], stdout);
#endif
  }
}



/*****/
upTermLines(nLines)
  int nLines;
{
  int x;

#ifdef MSDOS
  gotoxy(wherex(), wherey()-nLines);
#else
  for (x=0; x<nLines; x++)
    fputs(term_up, stdout);
#endif
}


/*****/
setCursor()
{
  positionCursor(currentWindow->cursorLine, currentWindow->cursorColumn);
}


/*****/
setInverse()
{
#ifdef MSDOS
  textattr(BLACK + (WHITE<<4));  /*  in a word, BBBB FFFF */
#else
  fputs(term_inverse, stdout);
#endif
}


/*****/
setBold()
{
#ifdef MSDOS
  textattr(BLACK + (WHITE<<4));  /*  in a word, BBBB FFFF */
#else
  fputs(term_bold, stdout);
#endif
}


/*****/
setNormal()
{
#ifdef MSDOS
  textattr(WHITE + (BLACK<<4));
#else
  fputs(term_normal, stdout);
#endif
}


/*****/
static iGetTtyData(row,prompt,newSize,defaultStr,doFileCompletion)
  /* fn returns <0 if ^g is typed, 0 if all goes well */
  int row;
  char *prompt;	/*null terminated string*/
  int *newSize;	/*pass back the new size*/
    		/*results will be in lineBuff*/
  char *defaultStr;
  boolean doFileCompletion;  /* true if should complete file names on esc */
{
  int col,y,aChar,saveRow;
  int timesThrough = 0;

  setScroll(0,terminalLines);
restart:
  timesThrough++;
  strcpy(screen[row],prompt);
  if (defaultStr && (timesThrough<2)) {
    strcpy(lineBuff, defaultStr);
    *newSize=strlen(lineBuff+1);
    strcat(screen[row], defaultStr);
}
  else
    *newSize = -1;
  positionCursor(row,0);
  if (isColor) {
#ifdef CYGNUS
    setBold();
#endif
    fputs(fg_red, stdout);
    setNormal();
}
  fputs(screen[row], stdout);
  if (isColor) {
#ifdef CYGNUS
    setBold();
#endif
    fputs(fg_blue, stdout);
    setNormal();
}
  eraseToEnd();
  col = strlen(prompt);
  while ((aChar = mgetc()) != 13 && aChar != 10) {
    switch (aChar) {
    case 14:
      aChar = 13; break;
    case 6:     /* ^f - yank in old text */
      *newSize = strlen(lineBuff+1);
      goto repaint;
    case 1:     /* ^a */
      aChar = lineBuff[(*newSize)+1];
      if (aChar == 10 || aChar == 0) goto done;
      break;
    }
    switch (aChar) {
    case BS:
    case DELETE:
      if (*newSize < 0) break;
      col = strlen(prompt);
      saveRow = row;
      (*newSize)--;
      for (y=0; y<=(*newSize); y++)
	toScreen(NULL, &row,&col,lineBuff[y]);
      screen[row][col] = 0;
      row = saveRow;
      positionCursor(row,0);
      fputs(screen[row], stdout); eraseToEnd();
      break;
    case 7:
      positionCursor(row,0);
      printf("%cAborted",7); eraseToEnd();
      setCursor();
      redrawStatus = TRUE;
      lineBuff[(*newSize)+1] = 0;
      if (isColor)
        fputs(fg_black, stdout);
      return(-1);
    case 11:  /* ^k */
      doCommand(11  /* ^k */);
      if (*newSize < 0) {
        positionCursor(row, 0);
        if (isColor) {
#ifdef CYGNUS
          setBold();
#endif
          fputs(fg_red, stdout);
          setNormal();
      }
        fputs(prompt, stdout);
        if (isColor) {
#ifdef CYGNUS
          setBold();
#endif
          fputs(fg_blue, stdout);
          setNormal();
      }
        break;
      }
repaint:
      if (isColor)
        fputs(fg_black, stdout);
      col = strlen(prompt);
      saveRow = row;
      for (y=0; y<=(*newSize); y++)
	toScreen(NULL, &row,&col,lineBuff[y]);
      screen[row][col] = 0;
      row = saveRow;
      positionCursor(row,0);
      fputs(screen[row], stdout);
      continue;
    case 21:
      goto restart;
#ifdef ESCAPE_FOR_FILEC
    case 27:
      aChar = mgetc();
#else
    case 32:
#endif
      if ((*newSize >= 0) && doFileCompletion) {
        lineBuff[(*newSize)+1] = '\0';
#if defined(UNIX) & !defined(CYGNUS_OLD)
        /* translate environment variables 1st, then complete the name */
        translateFileName(lineBuff);
        complete(lineBuff);
#endif
        *newSize = strlen(lineBuff) - 1;
        goto repaint;
      }
      /* fall through to default insertion */
    default:
      lineBuff[++(*newSize)] = aChar;
      y = col;
      toScreen(NULL, &row,&col,aChar);
      for (;y<col; y++)
        putchar(screen[row][y]);
      screen[row][y] = 0;
      break;
    }
  }
done:
  positionCursor(promptLine,0);
  lineBuff[(*newSize)+1] = 0;
  redrawStatus = TRUE;
  if (isColor)
    fputs(fg_black, stdout);
  return(0);
}



/*****/
getTtyData(row,prompt,newSize)
  /* same result returned as iGetTtyData() */
  int row;
  char *prompt;	/*null terminated string*/
  int *newSize;	/*pass back the new size*/
    		/*results will be in lineBuff*/
{
  return iGetTtyData(row,prompt,newSize,0,FALSE);
}




/*****/
getFileName(row,prompt,newSize)
  /* same result returned as iGetTtyData() */
  int row;
  char *prompt;	/*null terminated string*/
  int *newSize;	/*pass back the new size*/
    		/*results will be in lineBuff*/
{
  int result;
  char *dir;
  char *pos;
  int amt;
  char achar;
  pos = (char *)strrchr(currentBuffer->fileName,
    '/');
  if (!pos)
    dir = (char *)0;
  else {
    pos++;
    achar = *pos;  *pos = (char)0;
    dir = (char *)strdup(currentBuffer->fileName);
    *pos = achar;
}
  result = iGetTtyData(row,prompt,newSize,dir,TRUE);
  if (dir)
    free(dir);
  if (result >= 0  &&  lineBuff[0])
    translateFileName(lineBuff);
  return result;
}


