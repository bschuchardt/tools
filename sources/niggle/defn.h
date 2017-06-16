/* g l o b a l   v a r i a b l e   d e f i n i t i o n s  */


/*
 * name - %M%
 * ccsid - %W% - %G% %U%
 * from  - %F%
 * date  - %H% %T%
 */


/* backspace and delete character codes */
#define BS      8
#define DELETE 127

  /* side-effect variables */

#ifdef MSDOS
extern unsigned char videoAttr;
#endif

extern int repCount;
extern int lastRepCount;
extern int lastCommand;
extern boolean defaultRep;
extern boolean lastDefRep;
extern int cmdDirection;
extern char sString[131];
extern int sSize;
extern char rString[131];
extern int rSize;
extern boolean newLine;
extern boolean redrawStatus;

extern int numFilesNotFound;
extern int numFilesNotLocked;
extern boolean quitProgram;
extern boolean initializing;
extern boolean intAbort;
extern int tabColumn;
extern int lastColumn;
extern boolean keepColumn;
extern char *mode;
extern char overmode[40];

extern boolean caseSensitive;       /* TRUE if case sensitive searches */
extern boolean backupFiles;
extern boolean regularExpressions;

extern aBuffer *buffers;
extern aBuffer *currentBuffer, *cutBuffer;
extern aWindow *windows;
extern aWindow *currentWindow;

extern int statusColumn;
extern char lineBuff[102400];
extern char tempBuff[102400];        /* for fleeting local use only */

extern char deletedChar;

extern char delLin[102400];
extern int delLinSize;

extern boolean wrapOn;
extern int wrapMargin;

extern boolean autoIndent;
extern boolean lockFiles;
extern boolean useTabs;
extern BS8; /* is the backspace key really a backspace? usually it's not*/

extern boolean recording;
extern char recordBuffer[];
extern int  recordSize;
extern boolean playback;
extern int  playbackIdx;

extern int terminalLines;
extern int saveTerminalLines;
extern int terminalColumns;
extern int promptLine;

extern int leftColumn;      /* left column position of virtual window */

/* define the screen memory */
extern char * screen[200];

/* define terminal type */
extern int termType;

/* define escape sequences for vt100 */
extern char term_cleol[];
extern char term_position[];
extern char term_line_position[];
extern char term_clear[];
extern char term_up[];
extern char term_scrollingRegion[];
extern char term_inverse[];
extern char term_bold[];
extern char term_normal[];

extern char term_primary[];
extern char term_alternate[];

/* color terminal ISO 6429 escape sequences */
extern char fg_black[];
extern char fg_red[];
extern char fg_green[];
extern char fg_yellow[];
extern char fg_blue[];
extern char fg_magenta[];
extern char fg_cyan[];
extern char fg_white[];
extern char fg_default[];

extern char bg_black[];
extern char bg_red[];
extern char bg_green[];
extern char bg_yellow[];
extern char bg_blue[];
extern char bg_magenta[];
extern char bg_cyan[];
extern char bg_white[];
extern char bg_default[];

extern int isColor;
extern char separators[];
extern int  seplen;

extern char *getHome();
