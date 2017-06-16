/* grep - dec's grep functions altered to work with niggle */

/*
 * name - %M%
 * ccsid - %W% - %G% %U%
 * from  - %F%
 * date  - %H% %T%
 */


/********************************
char    *documentation[] = {
"grep searches a file for a given pattern.  Execute by",
"   grep [flags] regular_expression file_list",
"",
"Flags are single characters preceeded by '-':",
"   -c      Only a count of matching lines is printed",
"   -f      Print file name for matching lines switch, see below",
"   -n      Each line is preceeded by its line number",
"   -v      Only print non-matching lines",
"",
"The file_list is a list of files (wildcards are acceptable on RSX modes).",

"",
"The file name is normally printed if there is a file given.",
"The -f flag reverses this action (print name no file, not if more).",
"",
0 };


char    *patdoc[] = {
"The regular_expression defines the pattern to search for.  Upper- and",
"lower-case are always ignored.  Blank lines never match.  The expression",
"should be quoted to prevent file-name translation.",
"x      An ordinary character (not mentioned below) matches that character.",
"'\\'    The backslash quotes any character.  \"\\$\" matches a dollar-sign.",
"'^'    A circumflex at the beginning of an expression matches the",
"       beginning of a line.",
"'$'    A dollar-sign at the end of an expression matches the end of a line.",
"'.'    A period matches any character except \"new-line\".",
"':a'   A colon matches a class of characters described by the following",
"':d'     character.  \":a\" matches any alphabetic, \":d\" matches digits,",
"':n'     \":n\" matches alphanumerics, \": \" matches spaces, tabs, and",
"': '     other control characters, such as new-line.",
"'*'    An expression followed by an asterisk matches zero or more",
"       occurrances of that expression: \"fo*\" matches \"f\", \"fo\"",
"       \"foo\", etc.",
"'+'    An expression followed by a plus sign matches one or more",
"       occurrances of that expression: \"fo+\" matches \"fo\", etc.",
"'-'    An expression followed by a minus sign optionally matches",
"       the expression.",
"'[]'   A string enclosed in square brackets matches any character in",
"       that string, but no others.  If the first character in the",
"       string is a circumflex, the expression matches any character",
"       except \"new-line\" and the characters in the string.  For",
"       example, \"[xyz]\" matches \"xx\" and \"zyx\", while \"[^xyz]\"",
"       matches \"abc\" but not \"axb\".  A range of characters may be",
"       specified by two characters separated by \"-\".  Note that,",
"       [a-z] matches alphabetics, while [z-a] never matches.",
"The concatenation of regular expressions is a regular expression.",
0};
**************************************************************/

#include <setjmp.h>


#define LMAX    512
#define PMAX    256

#define CHAR    1
#define BOL     2
#define EOL     3
#define ANY     4
#define CLASS   5
#define NCLASS  6
#define STAR    7
#define PLUS    8
#define MINUS   9
#define ALPHA   10
#define DIGIT   11
#define NALPHA  12
#define PUNCT   13
#define RANGE   14
#define ENDPAT  15

int     cflag;
int     fflag;
int     nflag;
int     vflag;
int     nfile;

int     debug   =       0;         /* Set for debug code      */

char    *pp;

#ifndef vms
char    file_name[81];
#endif

char    lbuf[LMAX];
char    pbuf[PMAX];

jmp_buf error_jump;               /* bjs - add error handler */
char error_text[128];



/*******************************************************/

char *re_comp(source)
char       *source;   /* Pattern to compile         */
/*
 * Compile the pattern into global pbuf[]
 */
{
   register char  *s;         /* Source string pointer     */
   register char  *lp;        /* Last pattern pointer      */
   register int   c;          /* Current character         */
   int            o;          /* Temp                      */
   char           *spp;       /* Save beginning of pattern */
   char           *cclass();  /* Compile class routine     */


   if (setjmp(error_jump) < 0)
    return error_text;

   s = source;
   if (debug)
      printf("Pattern = \"%s\"\n", s);
   pp = pbuf;
   while ( (c = *s++) != 0 ) {
      /*
       * STAR, PLUS and MINUS are special.
       */
      if (c == '*' || c == '+' || c == '-') {
         if (pp == pbuf ||
              (o=pp[-1]) == BOL ||
              o == EOL ||
              o == STAR ||
              o == PLUS ||
              o == MINUS)
            badpat("Illegal occurrance op.", source, s);
         store(ENDPAT);
         store(ENDPAT);
         spp = pp;               /* Save pattern end     */
         while (--pp > lp)       /* Move pattern down    */
            *pp = pp[-1];        /* one byte             */
         *pp =   (c == '*') ? STAR :
            (c == '-') ? MINUS : PLUS;
         pp = spp;               /* Restore pattern end  */
         continue;
    }
      /*
       * All the rest.
       */
      lp = pp;         /* Remember start       */
      switch(c) {

      case '^':
         store(BOL);
         break;

      case '$':
         store(EOL);
         break;

      case '.':
         store(ANY);
         break;

      case '[':
         s = cclass(source, s);
         break;

      case ':':
         if (*s) {
            c = *s++;
            switch(c) {

            case 'a':
            case 'A':
               store(ALPHA);
               break;

            case 'd':
            case 'D':
               store(DIGIT);
               break;

            case 'n':
            case 'N':
               store(NALPHA);
               break;

            case ' ':
               store(PUNCT);
               break;

            default:
               badpat("Unknown : type", source, s);

          }
            break;
       }
         else    badpat("No : type", source, s);

      case '\\':
         if (*s)
            c = *s++;

      default:
         store(CHAR);
         store(c);
    }
 }
   store(ENDPAT);
   store(0);                /* Terminate string     */
   if (debug) {
      for (lp = pbuf; lp < pp;) {
         if ((c = (*lp++ & 0377)) < ' ')
            printf("\\%o ", c);
         else    printf("%c ", c);
      }
        printf("\n");
 }
  return (char *)0;
}

/*******************************************************/

char *
cclass(source, src)
char       *source;   /* Pattern start -- for error msg.      */
char       *src;      /* Class start           */
/*
 * Compile a class (within [])
 */
{
   register char   *s;        /* Source pointer    */
   register char   *cp;       /* Pattern start     */
   register int    c;         /* Current character */
   int             o;         /* Temp              */

   s = src;
   o = CLASS;
   if (*s == '^') {
      ++s;
      o = NCLASS;
 }
   store(o);
   cp = pp;
   store(0);                          /* Byte count      */
   while ((c = *s++) != 0  &&  c!=']') {
      if (c == '\\') {                /* Store quoted char    */
         if ((c = *s++) == '\0')      /* Gotta get something  */
            badpat("Class terminates badly", source, s);
         else    store(c);
    }
      else if (c == '-' &&
            (pp - cp) > 1 && *s != ']' && *s != '\0') {
         c = pp[-1];             /* Range start     */
         pp[-1] = RANGE;         /* Range signal    */
         store(c);               /* Re-store start  */
         c = *s++;               /* Get end char and*/
         store(c);               /* Store it        */
    }
      else {
         store(c);               /* Store normal char */
    }
 }
   if (c != ']')
      badpat("Unterminated class", source, s);
   if ((c = (pp - cp)) >= 256)
      badpat("Class too large", source, s);
   if (c == 0)
      badpat("Empty class", source, s);
   *cp = c;
   return(s);
}

/*******************************************************/

store(op)
{
   if (pp >= &pbuf[PMAX])
      error("Pattern too complex\n");
   *pp++ = op;
}


/*******************************************************/

badpat(message, source, stop)
char  *message;       /* Error message */
char  *source;        /* Pattern start */
char  *stop;          /* Pattern end   */
{
   register int    c;

   sprintf(error_text, "search error %s, pattern is\"%s\"\n", message, source);
   longjmp(error_jump, -1);
}



/*******************************************************/

re_exec(line)
char *line;
/*
 * Scan the line for the pattern in pbuf[]
 */
{

  if (setjmp(error_jump) < 0) {
    notify(error_text);
    return -1;
  }

  return match(line);

}


/*******************************************************/

match(line)
  char *line;
/*
 * Match the give text, return 1 if it does, 0 if not
 */
{
   register char   *l;        /* Line pointer       */
   char *pmatch();

   for (l = line; *l; l++) {
      if (pmatch(l, pbuf))
         return(1);
      if (pbuf[0] == BOL) break;
 }
   return(0);
}

/*******************************************************/

char *
pmatch(line, pattern)
char               *line;     /* (partial) line to match      */
char               *pattern;  /* (partial) pattern to match   */
{
   register char   *l;        /* Current line pointer         */
   register char   *p;        /* Current pattern pointer      */
   register char   c;         /* Current character            */
   char            *e;        /* End for STAR and PLUS match  */
   int             op;        /* Pattern operation            */
   int             n;         /* Class counter                */
   char            *are;      /* Start of STAR match          */

   l = line;
   if (debug > 1)
      printf("pmatch(\"%s\")\n", line);
   p = pattern;
   while ((op = *p++) != ENDPAT) {
      if (debug > 1)
         printf("byte[%d] = 0%o, '%c', op = 0%o\n",
               l-line, *l, *l, op);
      switch(op) {

      case CHAR:
         if (*l++ != *p++)
            return(0);
         break;

      case BOL:
         if (l != line)
            return(0);
         break;

      case EOL:
         if (*l != '\0')
            return(0);
         break;

      case ANY:
         if (*l++ == '\0')
            return(0);
         break;

      case DIGIT:
         if ((c = *l++) < '0' || (c > '9'))
            return(0);
         break;

      case ALPHA:
         c = *l++;
         if (c < 'a' || c > 'z')
            return(0);
         break;

      case NALPHA:
         c = *l++;
         if (c >= 'a' && c <= 'z')
            break;
         else if (c < '0' || c > '9')
            return(0);
         break;

      case PUNCT:
         c = *l++;
         if (c == 0 || c > ' ')
            return(0);
         break;

      case CLASS:
      case NCLASS:
         c = *l++;
         n = *p++ & 0377;
         do {
            if (*p == RANGE) {
               p += 3;
               n -= 2;
               if (c >= p[-2] && c <= p[-1])
                  break;
          }
            else if (c == *p++)
               break;
       } while (--n > 1);
         if ((op == CLASS) == (n <= 1))
            return(0);
         if (op == CLASS)
            p += n - 2;
         break;

      case MINUS:
         e = pmatch(l, p);       /* Look for a match    */
         while (*p++ != ENDPAT); /* Skip over pattern   */
         if (e)                  /* Got a match?        */
            l = e;               /* Yes, update string  */
         break;                  /* Always succeeds     */

      case PLUS:                 /* One or more ...     */
         if ((l = pmatch(l, p)) == 0)
            return(0);           /* Gotta have a match  */
      case STAR:                 /* Zero or more ...    */
         are = l;                /* Remember line start */
	 while (*l && (e = pmatch(l, p)) != 0)
            l = e;               /* Get longest match   */
         while (*p++ != ENDPAT); /* Skip over pattern   */
         while (l >= are) {      /* Try to match rest   */
	    if ( (e = pmatch(l, p)) != 0 )
               return(e);
            --l;                 /* Nope, try earlier   */
       }
         return(0);              /* Nothing else worked */

      default:
         printf("Bad op code %d\n", op);
         error("Cannot happen -- match\n");
    }
 }
   return(l);
}

/*******************************************************/

error(s)
char *s;
{
   strcpy(error_text, s);
   longjmp(error_jump, -1);
}

