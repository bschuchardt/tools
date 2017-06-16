/*
 * name - %M%
 * ccsid - %W% - %G% %U%
 * from  - %F%
 * date  - %H% %T%
 */

/* functions for 1) file name and user name completion
                    complete(name_buffer) - buffer is overwritten
                 2) ~ style file name expansion
                    get_full_path(name, target_buffer)

   These functions were taken from the program "touchup", a bitmap
   editor distributed on usenet */



#ifdef UNIX
/* no other implementations so far */

#include <sys/types.h>

#if defined(SOLARIS) || defined(CYGNUS)
#include <dirent.h>
#else
#include <sys/dir.h>
#endif

#include <sys/file.h>
#include <pwd.h>
#include <sys/stat.h>

#if defined(CYGNUS) || defined(LINUX)
#include <errno.h>
#else
extern  int   errno;
#endif

/*--------------------------------------*/

/* This function, written by Marc J Newberger,
 * will do both login name completion and file name completion, DAM fast.
 * That means as fast as the csh does it.
 */
int complete(template)

    char    *template;

{

    char    dirName[255];
    char   *prefix;    
    int     pref_len;
    char   *suffix;     
    char   *p, *q;
    char    first;
    char    nonUnique;
    char    twiddleUserCompletion;

#if defined(SOLARIS) || defined(CYGNUS)
    struct  dirent     *nameEntry;
#else
    struct  direct     *nameEntry;
#endif
    DIR                *dirChan;
    struct  passwd     *pwdEntry;

    /*
     *  First do a little parsing of the input. Separate the
     *  prefix template from the directory if there is one.
     */
    twiddleUserCompletion= 0;
    prefix= template+strlen(template);
    while (*(--prefix) != '/' && prefix >= template);

    /*
     *  See if a directory was specified:
     */
    if (prefix < template) {
        /*
         *  No /'s, could be either a username completion or
         *  a completion in the current directory.
         */
        if (template[0] == '~') {
            prefix++;
            twiddleUserCompletion= 1;
          }
        else {
            strcpy(dirName, ".");
          }
      }
    else if (prefix == template) {
        /*
         *  Special case !! The directory excluding the trailing
         *  '/' is zero length. It's the root:
         */
        strcpy(dirName, "/");
      }
    else {
        /*
         *  We're completing a file in a directory.
         *  The directory may be lead by a ~<username> abbreviation.
         *  If that's the case expand it.
         */
        if (template[0] == '~') {
            /*
             *  We need to do twiddle directory expansion.
             *  See if it's our directory:
             */
            if (template[1] == '/') {
                strcpy(dirName, getHome());
		if ( &template[1] != prefix )
		  {
                    p= dirName+strlen(dirName);
    		    q= &template[1];
                    while (q < prefix) {
                        *p= *q;
                        p++, q++;
                      }
                    *p= 0;
		}
              }
            else {
                /*
                 * It's someone else's. Let our fingers
                 * do the walking.
                 */
                for (p= dirName, q= &template[1];
                        (*p= *q) != '/';
                            p++, q++);
                *p= 0;
                if (!(pwdEntry= getpwnam(dirName))) {
                    return errno;
                  }
                strcpy(dirName, pwdEntry->pw_dir);
                p= dirName+strlen(dirName);
                while (q < prefix) {
                    *p= *q;
                    p++, q++;
                  }
                *p= 0;
              }
          }
        else {
            /*
             *  It's a vanilla directory. Strip it out.
             */
            strncpy(dirName, template, prefix-template);
            dirName[prefix-template]= 0;
          }
      }
    /*
     *  Bump prefix past the '/'.
     */
    prefix++;

    /*
     *  Get the prefix length and a pointer to the end of the
     *  prefix.
     */
    pref_len= strlen(prefix);
    suffix= template + strlen(template);

    /*
     *  See whether we're doing filename or username completion:
     */
    if (!twiddleUserCompletion) {

        /*
         *  It's filename completion. Read through the directory:
         */
        if ((dirChan= opendir(dirName)) == 0) {
            return errno;
          }

        first= 1;
        nonUnique= 0;
        for (;;) {
            if (!(nameEntry= readdir(dirChan))) {
                break;
              }
            if (!strncmp(prefix, nameEntry->d_name, pref_len)) {
                /*
                 *  We have a file that matches the template.
                 *  If it's the first one, we fill the completion
                 *  suffix with it. Otherwise we scan and pare down
                 *  the suffix.
                 */
                if (first) {
                    first=  0 ;
                    strcpy(suffix, nameEntry->d_name+pref_len);
                  }
                else {
                    nonUnique= 1;
                    p= suffix;
                    q= nameEntry->d_name+pref_len;
                    while (*p == *q) {
                        ++p; ++q;
                      }
                    *p= 0;

                    /*
                     *  A little optimization: If p == suffix, we
                     *  were unable to do any extension of the name.
                     *  We might as well quit here.
                     */
                    if (p == suffix) {
                        break;
                      }
                  }
              }
          }

        closedir(dirChan);
      }
    else {
        /*
         *  Do ~Username completion. Start by resetting the passwd file.
         */
        setpwent();

        first= 1;
        nonUnique= 0;
        for (;;) {
            if (!(pwdEntry= getpwent())) {
                break;
              }
            if (!strncmp(prefix, pwdEntry->pw_name, pref_len)) {
                /*
                 *  We have a user that matches the template.
                 *  If it's the first one, we fill the completion
                 *  suffix with it. Otherwise we scan and pare down
                 *  the suffix.
                 */
                if (first) {
                    first=  0 ;
                    strcpy(suffix, pwdEntry->pw_name+pref_len);
                  }
                else {
                    p= suffix;
                    q= pwdEntry->pw_name+pref_len;
                    while (*p == *q) {
                        ++p; ++q;
                      }

                    /*
                     *  Here there is a possibility of seeing the
                     *  same username twice. For this reason, we
                     *  only set nonUnique to 1 if we're shortening
                     *  the suffix. This means that the new name is
                     *  distinct from any name we've seen.
                     */
                    if (*p) {
                        nonUnique= 1;
                        *p= 0;
                      }

                    /*
                     *  A little optimization: If p == suffix, we
                     *  were unable to do any extension of the name.
                     *  We might as well quit here.
                     */
                    if (p == suffix) {
                        break;
                      }
                  }
              }
          }
      }

    /*
     *  If nothing matched, return a -1, if there was non-uniqueness
     *  return -2.
     */ 
    if (first) {
        return -1;
      }
    else if (nonUnique) {
        return -2;
      }
    else {
        return 0;
      }

}




/*-----------------------------------------------------*/

/*
 * Take a filename with a ~ character at the begining and return
 * the full path name to that file
 */
get_full_path(template,full_path)
char template[];
char full_path[];
{
    char   *p, *q;
    struct  passwd     *pwdEntry;
        /*
         *  We're completing a file in a directory.
         *  The directory may be lead by a ~<username> abbreviation.
         *  If that's the case expand it.
         */
        if (template[0] == '~') {
            /*
             *  We need to do twiddle directory expansion.
             *  See if it's our directory:
             */
            if (template[1] == '/') {
                strcpy(full_path, getHome());
		strcat(full_path,&template[1]);
              }
            else {
                /*
                 * It's someone else's. Let our fingers
                 * do the walking. (Why the fxxx do they call it
                 * the "yellow pages" anyway. They're white pages
                 * dammit !  If they were YELLOW pages, we could
                 * say ypmatch "Automobile, Dealers, Retail", and
                 * things like that !).
                 */
                for (p= full_path, q= &template[1];
                        (*p= *q) != '/';
                            p++, q++);
                *p= 0;
                if (!(pwdEntry= getpwnam(full_path))) {
                    return errno;
                  }
                strcpy(full_path, pwdEntry->pw_dir);
		strcat(full_path,q);
              }
	}
	else
	  	strcpy(full_path,template);
}		 


#endif /* if defined(UNIX) */
