/*  b u f f e r   m a n a g e m e n t-----------------------------  */

/*
 * name - %M%
 * ccsid - %W% - %G% %U%
 * from  - %F%
 * date  - %H% %T%
 */


#include "types.mh"
#include "defn.h"
#include "edit.h"
#include "mgetc.h"

#ifndef VMS
#include <sys/types.h>          /* for fstat() call */
#include <sys/stat.h>           /* for fstat() call */
#endif
#ifdef UNIX
#include <signal.h>
#include <sys/file.h>
#include <sys/fcntl.h>
extern char *getenv();
#endif
#if defined(MSDOS) | defined(SOLARIS)
#include <fcntl.h>	        /* open() mode constants */
#endif
#if defined(CYGNUS)
#include <sys/fcntl.h>
#endif

#if defined(IBMRT)
#define	O_RDONLY	0
#define	O_WRONLY	1
#define	O_RDWR		2
#define	O_APPEND	FAPPEND	/* append (writes guaranteed at the end) */
#define	O_CREAT		FCREAT	/* open with file create */
#define	O_TRUNC		FTRUNC	/* open with truncation */
#define	O_EXCL		FEXCL	/* error on create if file exists */
#endif

extern void *malloc(size_t amount);
extern void *realloc(void *ptr, size_t size);

/*****/
char *myalloc(amount)
  int amount;
  {
  char *result;
  char yesno;

  result = (char *)malloc(amount);
  if (!result) {
    mdeassign();
    eraseScreen();
    printf("out of memory - requesting 0x%x bytes\n", amount);
    if (anyModifiedBuffers()) {
      fputs("save modified files (default is Yes)? ", stdout);
      yesno = getchar();
      if (yesno == 'y'  ||  yesno == 'Y'  ||  yesno == '\n')
        writeModifiedBuffers(TRUE);
#if defined(UNIX)
      fputs("\ndump core (default is No)? ", stdout);
      yesno = getchar();
      if (yesno == 'y' || yesno == 'Y')
        kill(getpid(), SIGQUIT);
#endif
    }
    exit(2);
  }
  return result;
}
  
  


/*****/
aLine *makeNewLine(allocSize, lineSize)
  {
  aLine *newLine;

  newLine = (aLine *)myalloc(sizeof(aLine));
  newLine->value = myalloc(allocSize + (allocSize % 2) + 4);
  newLine->sz  = lineSize;
  newLine->next  = newLine->last = NULL;
  newLine->isEof = FALSE;
  return newLine;
}



/*****/
createCutBuffer()
{  /* create the buffer to hold cuts */

  aBuffer *buff;
  aLine   *line;
  int      i;

  cutBuffer = (aBuffer *)myalloc(sizeof(aBuffer));
  cutBuffer->modified = FALSE;
  cutBuffer->notSaveable = TRUE;
  cutBuffer->crlf = FALSE;
  strcpy(cutBuffer->fileName,"cutbuffer");
  for (i=0; i<=numDots; i++) cutBuffer->aDot[i] = 0;

  /*  create the eof record for the cut buffer */
  line = cutBuffer->firstLine = cutBuffer->lastLine = cutBuffer->dotLine =
      (aLine *)myalloc(sizeof(aLine));
  line->next = line->last = NULL;
  line->sz = 0;
  line->isEof = TRUE;
  line->value = myalloc(4);
  line->value[0] = '\n';
  line->value[1] = 0;


  cutBuffer->dotPos = 0;


  /* append to the list of buffers */
  buff = buffers;
  while (buff->next != NULL) buff = buff->next;
  buff->next = cutBuffer;
  cutBuffer->next = NULL;
  cutBuffer->last = buff;

}






/*****/
writeCurrentBuffer()
{
  if (writeToFile(currentBuffer)) {
    setModified(FALSE);
    positionCursor(promptLine,0);
    printf("%s %s",currentBuffer->fileName,"written");
    eraseToEnd();
    redrawStatus = TRUE;
    setCursor();
  }
}




/*****/
writeModifiedBuffers(verbose)
  int verbose;          /* print out statistics? */
{
  aBuffer *fly;
  aLine *line;
  int numLines, numChars;

  fly = buffers;
  while (fly != NULL) {
    if (fly->modified) {
      printf("writing %s ", fly->fileName);
      fflush(stdout);
      if (!writeToFile(fly)) return FALSE;
      numLines = numChars = 0;
      for (line=fly->firstLine; line!=NULL && !line->isEof; line=line->next){
        numLines++;
        numChars = numChars + line->sz + 1;
    }
      if (verbose) {
        printf(" %d lines,  %d characters\r\n", numLines, numChars);
        fflush(stdout);
    }
      fly->modified = FALSE;
  }
    fly = fly->next;
}
  return TRUE;
}




/*****/
anyModifiedBuffers()    /* answer True if any buffers have been changed */
{
  aBuffer *flyBuffer;

  flyBuffer = buffers;
  while (flyBuffer != NULL) {
    if (flyBuffer->modified  &&  !flyBuffer->notSaveable)
      return(TRUE);
    flyBuffer = flyBuffer->next;
  }
  return(FALSE);
}




/*****/
setModified(abool)   /* set modification flag for currentBuffer */
  boolean abool;
  {
  if ((abool && currentBuffer->modified) ||
      (!abool && !currentBuffer->modified)) return 1;
  currentBuffer->modified = abool;
  displayLabel(currentWindow);
  setCursor();
}




/*****/
deleteCurrentBuffer()
  {
  aLine *line;

  if (currentBuffer->last)
    currentBuffer->last->next = currentBuffer->next;
  else
    buffers = currentBuffer->next;
  if (currentBuffer->next)
    currentBuffer->next->last = currentBuffer->last;
  for (line=currentBuffer->firstLine; line; ) {
    free(line->value);
    if (!line->isEof) {
      line=line->next;
      free(line->last);
    }
    else {
      free(line);
      line = NULL;
    }
  }
#ifdef UNIX
  if (currentBuffer->locked) {
    fclose(currentBuffer->lockedFP);
    currentBuffer->locked = FALSE;
}
#endif
  free(currentBuffer);
  /* set up a default replacement buffer */
  currentBuffer = cutBuffer;
  currentWindow->buffer = cutBuffer;
}





/*****/
#ifndef VMS
makeBackup(buff)
  aBuffer *buff;
  {
  char tmpBuff[256];
  char *homeDir;
  char *backupDir;
  int i;

  if (!backupFiles)
    return 1;

  /* can only backup the file if it exists and is readable.  If the
     file exists, I assume it is readable unless the user shelled and
     did a chmod on it.  */
#ifdef MSDOS
  if (access(buff->fileName,0)==0) {
#else
  if (access(buff->fileName,4)==0) {
#endif

    /* see if user has defined a niggle backup directory */
#ifdef MSDOS
    backupDir = getenv("NGBKUP");
#else
    backupDir = getenv("NIGGLEBACKUP");
#endif
    if (backupDir) {
      strcpy(lineBuff, backupDir);
      goto doBackup;
    }

    /* no backup directory defined - default to HOME/ngbackup */
    homeDir = getHome();

    if (homeDir) {  /* user must have HOME defined */
      strcpy(lineBuff,homeDir);
#ifdef MSDOS
      strcat(lineBuff,"\\ngbackup");
#else
      strcat(lineBuff, "/ngbackup");
#endif

      /* make sure there is an ngbackup directory */
      if (access(lineBuff,0)==-1) { /* no backup directory? */
        if (mkdir(lineBuff,0700)==-1) goto skipBackup;
      }

doBackup:
#ifdef MSDOS
      strcpy(tmpBuff,"copy ");
#else
      strcpy(tmpBuff,"cp ");
#endif
      strcat(tmpBuff,buff->fileName);
      strcat(tmpBuff," ");
      strcat(tmpBuff,lineBuff);
#ifdef MSDOS
      for (i=0; tmpBuff[i]; i++)
        if (tmpBuff[i] == '/')
          tmpBuff[i] = '\\';
#endif
      system(tmpBuff);
      /* leave the original file in place, in case it is linked */
    }
    else
      puts("no home directory for backups");
  }

skipBackup:
  return 1;
}
#endif





/*****/
boolean writeToFile(buff)
  aBuffer *buff;
{
  aLine  *flyLine;
  FILE *fp;
  int filedes;

  if (buff->notSaveable) { bell(); return(FALSE); }

#ifdef VMS
  /* use VMS versions for backups */
  filedes = creat(buff->fileName,0,"rat=cr","rfm=var");
#else
  makeBackup(buff);
  filedes = open(buff->fileName
		  ,O_WRONLY | O_CREAT | O_TRUNC, buff->fileProt);
#endif
  if (filedes < 0) {
    positionCursor(promptLine,0);
    printf("%cunable to write %s. Press return to continue:",7,buff->fileName);
    filedes = mgetc();
    return(FALSE);
  }

  flyLine = buff->firstLine;
  while (!flyLine->isEof) {
    if (buff->crlf) {
      write(filedes, flyLine->value, flyLine->sz);
      write(filedes, "\015\n", 2);
    }
    else {
      write(filedes,flyLine->value,flyLine->sz+1);
    }
    flyLine = flyLine->next;
  }

  return(TRUE);
}





/*****/
long linecount(buff)
  aBuffer *buff;
{
  aLine  *flyLine;
  int count = 0;

  flyLine = buff->firstLine;
  while (!flyLine->isEof) {
    flyLine = flyLine->next;
    count++;
  }

  return count;
}




#ifdef MSDOS
translateFileName(fileName)
  char *fileName;
{ }
#endif





#if defined(UNIX) | defined(CYGNUS)
/*****/
translateFileName(fileName)
  char *fileName;
  {
  /* convert any environment variables found in the given file name.  The
     contents of the argument are replaced with the translated file name. */
  int i, sz;
  char *envName;
  
  if (fileName[0] == '$') {
    sz = strlen(fileName);
    for (i=0; i<sz; i++) {
      if (fileName[i] == '/')
        break;
    }

    if (i < sz) {
      strncpy(tempBuff, &fileName[1], i-1);
      tempBuff[i-1] = '\0';
      envName = getenv(tempBuff);

      if (envName) {
        strcpy(tempBuff, envName);
        strcat(tempBuff, &fileName[i]);
        strcpy(fileName, tempBuff);
    }
  }
}

#if !defined(CYGNUS)
  complete(fileName);  /* do tilda translations and file name completion */
  get_full_path(fileName, tempBuff);
  strcpy(fileName, tempBuff);
#endif
}

#endif


/*****/
aBuffer *readFile(fileName,addToList)
  char fileName[];
  boolean addToList;
{
  aBuffer *newBuffer,*buff;
  aLine *flyLine, *newLine;
  int i;
  FILE *fp;
  
  newBuffer = (aBuffer *)myalloc(sizeof(aBuffer));
  newBuffer->next = NULL;
  newBuffer->last = NULL;
  if (addToList) {
    if (buffers == NULL) {
      /* create the first buffer of the list */
      buffers = newBuffer;
      buffers->next = NULL;
      buffers->last = NULL;
    }
    else {
      /* append a new buffer to the end of the list */
      buff = buffers;
      while (buff->next != NULL) {
	buff = buff->next;
	}
      buff->next = newBuffer;
      newBuffer->last = buff;
    }
  }


  newBuffer->modified = FALSE;
  newBuffer->notSaveable = FALSE;
  newBuffer->crlf = FALSE;
  newBuffer->locked = FALSE;
  strcpy(newBuffer->fileName, fileName);
  for (i=0; i<=numDots; i++)		/* initialize the dots */
    newBuffer->aDot[i] = 0;


  fp = fopen(fileName,"r");
  if (fp != NULL && lockFiles) {
    /* file must be open for writing in order to lock it */
    fclose(fp);
    fp = fopen(fileName, "r+");
    if (fp == NULL) {
      if (!initializing) {
        notify("Unable to lock read-only file");
      }
      else {
        numFilesNotLocked++;
      }
      newBuffer->readOnly = TRUE;
      newBuffer->notSaveable = TRUE;
      fp = fopen(fileName, "r");
    }
  }

  if (fp == NULL) {		/* an error in reading the file? */
    newLine = (aLine *)myalloc(sizeof(aLine));
    newLine->isEof = TRUE;
    newLine->value = myalloc(4);
    newLine->value[0] = '\n';
    newLine->value[1] = 0;
    newLine->sz = 0;
    newLine->next = newLine->last = NULL;
    newBuffer->firstLine = newBuffer->lastLine = newBuffer->dotLine = newLine;
    newBuffer->dotPos = 0;
    if (initializing) {
      numFilesNotFound++;
    }
    else {
      notify("file not found. new buffer created.");
    }
#ifdef UNIX
    newBuffer->fileProt = 0666;
#endif
    goto done;
  }


#ifdef UNIX
  /* get the file's protection bits */
  {
    struct stat buf;
    
    fstat(fileno(fp), &buf);
    newBuffer->fileProt = buf.st_mode & 0777;
  }

#if !defined(CYGNUS) & !defined(MACOS)
  /* if requested, lock the file */
  {
    int flstatus;
    
    if (lockFiles && !newBuffer->readOnly) {
      flstatus = lockf(fileno(fp), F_TLOCK, 0);
      if (flstatus < 0) {
        fclose(fp);
        newLine = (aLine *)myalloc(sizeof(aLine));
        newLine->isEof = TRUE;
        newLine->value = malloc(30);
        strcpy(newLine->value, "unable to lock file\n");
        newLine->sz = strlen(newLine->value);
        newLine->next = newLine->last = NULL;
        newBuffer->firstLine = newBuffer->lastLine = newBuffer->dotLine = newLine;
        newBuffer->dotPos = 0;
        newBuffer->notSaveable = TRUE;
        newBuffer->readOnly = TRUE;
        if (initializing) {
          numFilesNotLocked++;
        }
        else {
          notify("unable to write-lock file");
        }
        goto done;
      }
      newBuffer->locked = TRUE;
      newBuffer->lockedFP = fp;
    }
  }
#endif
#endif

  /* read the file into a linked list of lines */
  newBuffer->firstLine = NULL;
  flyLine = NULL;
  do {
    n_getline(fp,newBuffer,&newLine);
    if (newBuffer->firstLine == NULL) {
      newBuffer->firstLine = newLine;
      newLine->last = NULL;
      flyLine = newLine;
    }
    else {
      flyLine->next = newLine;
      newLine->last = flyLine;
      flyLine = newLine;
    }
  } while (! flyLine->isEof );

  /* book-keep */
  newBuffer->lastLine = flyLine;
  newBuffer->dotLine = newBuffer->firstLine;
  newBuffer->dotPos  = 0;

  if (!newBuffer->locked) {
    fclose(fp);
  }

done:
  return(newBuffer);
}




/*****/
n_getline(fp,buff,line)
  FILE *fp;		/* file pointer for input file */
  aBuffer *buff;
  aLine **line;		/* address of a line pointer */
{
  char *c;
  int x;

  c = fgets(lineBuff, sizeof(lineBuff), fp);
  if (c == NULL)
    x = -1;
  else
    x = strlen(lineBuff);
    
  /* note that x is either -1 or >0 */

  *line = (aLine *)myalloc(sizeof(aLine));
  if ( x < 0 ) {
    (*line)->isEof = TRUE;
    (*line)->value = myalloc(4);
    (*line)->value[0] = '\n';
    (*line)->value[1] = 0;
    (*line)->sz = 0;
  }
  else {
    (*line)->isEof = FALSE;
    (*line)->value = myalloc(x + 4 + (x % 2));
    /* make sure a newLine exists */
    if (lineBuff[x-1] != '\n') {
      lineBuff[x] = '\n';
      x++;
    }
    if (x>1 && lineBuff[x-2] == '\015') {
      lineBuff[x-2] = lineBuff[x-1];
      x--;
      buff->crlf = TRUE;
    }
    lineBuff[x] = 0;
    strcpy((*line)->value, lineBuff);
    (*line)->value[x + 1] = 0;
    (*line)->sz = x - 1;
  }

  (*line)->next = NULL;
}




/*****/
char nextChar(line,inLine)
  aLine **line;
  int *inLine;
{  /* get the next character in the buffer.  on end of buffer, return EOF */
newLine = FALSE;
nCstart:
  if ( (*inLine) >= (*line)->sz ) {
    if ((*line)->isEof) return(EOF);
    *line = (*line)->next;
    *inLine = -1;
    newLine = TRUE;
    goto nCstart;
  }
  return((*line)->value[++(*inLine)]);
}

nextCchar()
{
  return(nextChar(&currentBuffer->dotLine,&currentBuffer->dotPos));
}




/*****/
char lastChar(line,inLine)
  aLine **line;
  int *inLine;
{  /* get the last character in the buffer.  on beg of buffer, return EOF */
newLine = FALSE;
lCstart:
  if ((*inLine) <= 0) {
    if ((*line)->last == NULL) { (*inLine) = 0; return(EOF); }
    *line = (*line)->last;
    *inLine = (*line)->sz + 1;
    newLine = TRUE;
    goto lCstart;
  }
  return((*line)->value[--(*inLine)]);
}

lastCchar()
{
  return(lastChar(&currentBuffer->dotLine,&currentBuffer->dotPos));
}



/*****/
char charAt(line,inLine)
  aLine *line;
  int inLine;
{     /* note that this is called by value, not address */

  if (inLine < 0 || line->isEof) return(EOF);
  return(line->value[inLine]);

}






/*****/
analyseLines()
{
  aLine *line;
  int x,y,aChar;


  line = currentBuffer->dotLine;

  aChar = '*';
  while (aChar != ' ') {
#ifdef UNIX
    mdeassign();
#endif
    eraseScreen();
    positionCursor(0,0);
    printf("l i n e   d u m p :\n\n");

    if (line->last == NULL)
      printf("top-of-buffer\n\n");
    if (line->isEof)
      printf("end-of-buffer\n\n");
    printf("size = %d\n",line->sz);
    printf("contents:\n%s\n\n",line->value);
    fflush(stdout);
    printf("dump:\n");

    for (x=0, y=0; x<=line->sz+1; x++) {
      printf("%3x ",line->value[x]);
      if (++y > 19) {
	 y = 0;
	 printf("\n");
         fflush(stdout);
	 }
    }
    printf("\n\npress - to go back 1 line, + to go forward, Space to resume editing");
    fflush(stdout);
#ifdef UNIX
    massign();
#endif
    aChar = mgetc();
    if (aChar == '-' && line->last != NULL) line = line->last;
    if (aChar == '+' && !line->isEof)       line = line->next;
  }
}







/*****/
bufferStatus()
{
  int numLines, numChars, dLine;
  char *mod;
  aLine *line;
  aBuffer *fly;

#ifdef UNIX
  mdeassign();
#endif
  eraseScreen();
  positionCursor(0,0);
  printf("%s\n\n",
"                b u f f e r   s t a t u s");  fflush(stdout);
  printf("%s\n",
" name                          modified     #lines     #characters"); fflush(stdout);
  printf("%s\n",
" ---------------------------   ----------   --------   ------------"); fflush(stdout);
/*
           11111111112222222222333333333344444444445555555
 012345678901234567890123456789012345678901234567890123456
*/
  dLine = 5;

  for (fly=buffers; fly!=NULL; fly=fly->next) {
    numLines = 0;
    numChars = 0;
    for (line=fly->firstLine; line!=NULL && !line->isEof; line=line->next){
      numLines++;
      numChars = numChars + line->sz + 1;
    }
    mod = (fly->modified ? "yes" : "no");
    positionCursor(dLine,0);
      printf(" %s",fly->fileName);  fflush(stdout);
    positionCursor(dLine,33);
      printf("%s",mod);             fflush(stdout);
    positionCursor(dLine,45);
      printf("%6d",numLines);       fflush(stdout);
    positionCursor(dLine,58);
      printf("%6d",numChars);       fflush(stdout);
    dLine++;
  }

  positionCursor(promptLine,0);
  printf("%s","press any key to continue: ");
#ifdef UNIX
  massign();
#endif
  numChars = mgetc();
}


/*****/
windowStatus()
{
  int dLine;
  aWindow *fly;

#ifdef UNIX
  mdeassign();
#endif
  eraseScreen();
  positionCursor(0,0);
  printf("%s\n\n",
"                   w i n d o w   s t a t u s");                fflush(stdout);
  printf("%s\n",
" name                          first  last  label  #lines");   fflush(stdout);
  printf("%s\n",
" ---------------------------   -----  ----- -----  ------");   fflush(stdout);
/*
           11111111112222222222333333333344444444445555555
 012345678901234567890123456789012345678901234567890123456
*/
  dLine = 5;

  for (fly=windows; fly!=NULL; fly=fly->next) {
    positionCursor(dLine,0);
      printf(" %s",fly->buffer->fileName);                      fflush(stdout);
    positionCursor(dLine,31);
      printf("%5d",fly->firstLine);                             fflush(stdout);
    positionCursor(dLine,38);
      printf("%5d",fly->lastLine);                              fflush(stdout);
    positionCursor(dLine,44);
      printf("%5d",fly->labelLine);                             fflush(stdout);
    positionCursor(dLine,51);
      printf("%5d",fly->numLines);                              fflush(stdout);
    dLine++;
  }

  positionCursor(promptLine,0);
  printf("%s","press any key to continue: ");
#ifdef UNIX
  massign();
#endif
  dLine = mgetc();
}





