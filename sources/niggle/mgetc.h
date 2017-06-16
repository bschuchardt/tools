
/*
 * name - %M%
 * ccsid - %W% - %G% %U%
 * from  - %F%
 * date  - %H% %T%
 */

extern void mgetcInit();          /* initialize the module          */
extern void massign();            /* set i/o for window work        */
extern void mdeassign();          /* set i/o for normal tty work    */
extern byte mgetc();              /* get byte from input stream     */

extern boolean waitingForKBFlag;  /* TRUE when waiting for KB input */

