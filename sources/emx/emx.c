/*
    EMX - Non-Widget Main

*/


#include "emx.h"


main (argc, argv)

int	argc;
char	*argv[];

{
    XEvent  event;

    emxeditinit(argc, argv);

    while (TRUE)
	{
	XNextEvent(g_display, &event);
	/*printf("received event %d type %d\n", event.serial, event.type);*/
	emxeventhandle(&(event.xany));
	}

}
