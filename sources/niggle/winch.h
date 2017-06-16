
/*
 * name - %M%
 * ccsid - %W% - %G% %U%
 * from  - %F%
 * date  - %H% %T%
 * published variables and functions */

extern void winchInit();       /* initialize the winch module               */
extern void ttsize();          /* get current terminalColumns, terminalRows */
extern int checkWinch();       /* process window change if necessary        */
extern int winchReceivedFlag;  /* pending resize flag - if set checkWinch() */
                               /* should be called                          */
