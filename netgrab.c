#define ERR(szz,czz) if(opterr){fprintf(stderr,"%s%s%c\n",argv[0],szz,czz);}
#define BSD_SELECT                                                          // used in ftp_con
#define INCL_NOPMAPI
#define INCL_VIO
#define INCL_DOSPROCESS
#define INCL_DOS
#define INCL_DOSDATETIME           /* Date and time values */
#define INCL_DOSERRORS             /* DOS error values     */
#include <stdlib.h>
#include <sys\types.h>
#include <types.h>
#include <netinet\in.h>
#include <sys\socket.h>
#include <sys\select.h>
#include <mt.h>
#include <os2def.h>
#include <netdb.h>
#include <stdio.h>
#include <conio.h>
#include <os2.h>
#include <nerrno.h>
#include <ctype.h>
#include <string.h>
#include <process.h>
#include <time.h>
#include <io.h>
#include <signal.h>

#include "netgrab.h"
#include "urlparse.h"
#include "sockets.h"
#include "spliturl.h"

#ifndef errno
extern int errno;
#endif



int client_socket;                 // The current socket #


/* Global variables */
FILE *out;
FILE *in;
char *filen;                       // used in main and fileout
char *logfile;
TID thread3Tid=1;                  // this thread is used for writing articles
TID ftp_conTid=2;                  // a thread for ftp data connections
FTP_DATA *ftp_get;
URLData_t url;
int y;                             // a buffer integer
unsigned char *buf;                // a buffer variable - used all over
char file_write[1024000];         // a variable to put the article into before writing to disk
char article[1024000];            // large string to put article into
time_t wait_time;                  // this is our variable to check against timing out
int bugcheck=0;                    // for bug() can remove when finished
LIST *list_begin = NULL;           // linked list with 1 element called text
LIST *list_end = NULL;             // linked list with 1 element called text
LIST *list_current = NULL;         // linked list with 1 element called text
VIOCURSORINFO oinfo,ninfo;

// stuff for the options parser
int opterr = 1;                    //
int optind = 1;                    // option index
int optopt;                        //
char *optarg;                      //

//options
int get_news_head = 1;             // default yes
int append_to_file = 0;            // default overwrite file
int overwrite_file = 1;            // default yes, overwrite file
int strip_html = 0;                // default no
int timeout = 120;                 // default seconds to timeout = 120
int ftp_mode_binary = 1;           // default yes
int watch_server = 0;              // default no
int debug = 0;                     // default is on for testing porpoises
int get_head_only = 0;             // default is no
int set_time=0;                    // default no.
int delete_mail=0;                 // default no.
int get_all = 0;                   // default no way!!!
int get_site_only = 1;             // default yes
int beep_when_done = 0;            // default no again.
int keep_a_log = 0;                // default not.
char *user_agent = "OS/2_NetGrab"; //default netgrab
int parse_html = 0;                //default nope
int quiet_mode = 0;                // Default noisy
int caps_sensitive = 0;            // no.

int is_glob=0;                     // this one is set by the program if the filename is a globbing filename



/***********************************************************/
/*                  MANI                                   */
/*                                                         */
/*                                                         */
/*                                                         */
/***********************************************************/

void main(int argc, char **argv)   /* begin da program */
{
   TID thread2Tid=0;                  // a thread to watch for timing out
   char *destsite;
   int temptime;
   char *option_output;
   if ((destsite = (char *)malloc(1024*(sizeof(char)))) == NULL)
      error_out("Error allocating memory");
   if ((buf =  (char *) malloc(8196*(sizeof(char)))) == NULL)
      error_out("Error allocating memory");
   if ((filen = (char *)malloc(128*(sizeof(char)))) == NULL)
      error_out("Error allocating memory");
   if ((logfile = (char *)malloc(128*(sizeof(char)))) == NULL)
      error_out("Error allocating memory");
   if ((option_output = (char *)malloc(4096*(sizeof(char)))) == NULL)
      error_out("Error allocating memory");


   client_socket = 128;            /* dummy value so we don't try to shutdown stdin */
   strcpy(option_output,"");

   VioGetCurType(&oinfo, 0); // turn off our cursor
   ninfo = oinfo;            // turn off our cursor
   ninfo.attr = -1;          // turn off our cursor
   VioSetCurType(&ninfo, 0); // turn off our cursor

   signal(SIGBREAK,sig_handler);
   signal(SIGINT,sig_handler);

   time(&wait_time);

   if ( argc == 1 )
   {
      bannr();
      usage();
   }

   temptime=timeout; // store out timeout time
   timeout=60;       // set our timeout time to 60 seconds for connect

   parseOptions(argc, argv, &option_output);

   bannr();
   if((!(quiet_mode)) && (strlen(option_output) > 1))
      cprintf("%s",option_output);// print everything the options wanted to say

   check_options();

   if(!(quiet_mode))
      cprintf("Looking at URL\r\n");


   if (!URLParse(argv[optind], &url))
   {
      showurl();
      error_out("Error parsing URL");
   }

   log_to_file("\r\n\r\n*********************************************************************************\r\n");
   sprintf(buf,"        Started session at %s",asctime(localtime(&wait_time)));
   log_to_file(buf);
   log_to_file("*********************************************************************************\r\n");
   log_to_file(argv[optind]);
   log_to_file("\r\n\r\n");

   if ((url.type == URLHTTP) && (url.port == 0 ))
      url.port = 80;
   else if ((url.type == URLTIME) && (url.port == 0 ))
      url.port = 13;
   else if (( url.type == URLFTP)  && (url.port == 0 ))
      url.port = 21;
   else if (( url.type == URLNNTP) && (url.port == 0 ))
      url.port = 119;
   else if (( url.type == URLPOPMAIL) && (url.port == 0 ))
      url.port = 110;
   else if (( url.type == URLGOPHER) && (url.port == 0 ))
      url.port = 70;
   else if (( url.type == URLFINGER) && (url.port == 0 ))
      url.port = 79;
   else if (( url.type == URLTELNET) && (url.port == 0 ))
      url.port = 23;

   /* begin a new thread to check the timeout value */
   if (timeout)
   {
      thread2Tid=_beginthread(check_time, NULL, 16384, NULL);
      if (thread2Tid == -1)
         error_out("could not start thread");
   }

   optind++; // have to increment the option counter for next operation
   if (argc <= optind)
   {
      strcpy(filen,"NoFIleNaME!"); // ok, it's a kludge
   }
   else
      sprintf(filen,"%s",argv[optind]);

   showurl();

   // need to handle file before connecting to server
   if ( url.type == URLFILE)
      handle_file();

   strcpy(destsite,url.hostname);
   if(!(quiet_mode))
      cprintf("\r[1;37;40mAttempting to connect to %s[0;37;40m\r",destsite);

   client_socket = getconn(destsite, url.port);

   if ( client_socket == -1 )
   {
      sprintf(buf,"Error: Could not connect to host %s \r\n",destsite);
      error_out(buf);
   }
   else
      if (!(quiet_mode))
         cprintf("[1;37;40mConnected to %s            [0;37;40m\r\n\r\n",destsite);
   timeout = temptime;

   // handle file is above this
   if ( url.type == URLHTTP)    //add feature to pull all files on page
      handle_http();
   else if ( url.type == URLFTP)     //done?
      handle_ftp();
   else if ( url.type == URLGOPHER)  // needs work
      handle_gopher();
   else if ( url.type == URLFINGER)  // done?
      handle_finger();
//   if ( url.type == URLTELNET) // barely even started
//      handle_telnet();
   else if ( url.type == URLTIME)    // add feature to set time on local machine
      handle_time();
   else if ( url.type == URLPOPMAIL) // done?  add souper format?
      handle_popmail();
   else if ( url.type == URLNNTP)    // add uudeview / mime decode features?
   {
      if(!strcmp(url.path, "GeTlIsT"))
         handle_newslist();
      else
         handle_nntp();
   }
   else
      error_out("Unsupported at this time.");


   if ((get_all) && (list_begin != NULL))
      print_list();

   free(option_output); // we're done with this var now.
   close_exit();
}



/***********************************************************/
/*                  TIME                                   */
/*                                                         */
/*                                                         */
/*                                                         */
/***********************************************************/
void handle_time (void)
{
   int counter;
   char *line_in;
   time_t ltime;
   long int sec_diff=55,min_diff=1;
   DATETIME   DateTime = {0};       /* Structure to hold date/time info.   */
   APIRET     rc       = NO_ERROR;  /* Return code                         */

   if ((line_in = (char *) malloc (4096*(sizeof(char)))) == NULL)
      error_out("Error allocating memory");
   strcpy(article,"");


   y = getaline(client_socket,line_in); /* this inputs our line */
   time(&ltime);
   if ((strlen(line_in)) < 10) // a second try.  10 is arbitrary
   {
   y = getaline(client_socket,line_in); /* this inputs our line */
   time(&ltime);
   }

   if ((strlen(line_in)) < 10) // if still not enough data, error out
      error_out("Not enough data received from server");

   for(counter=0;counter<(strlen(line_in))-2;counter++) // -l allows for /n/r at end of line
   {  // check to make sure we're not getting some wierd crap
      if (!((isdigit(line_in[counter]))||(isalpha(line_in[counter]))||(isspace(line_in[counter]))||(ispunct(line_in[counter]))))
         error_out("Incorrect data received - try a different server on port 13");
   }

   if(!(quiet_mode))
      cprintf("[1;33;40m%s [1;37;40m\r Time at %s\n\r\n\r",line_in,url.hostname);
   sprintf(buf,"%s \r Time at %s\n\r\n\r",line_in,url.hostname);
   strcat(article, buf);

   if(!(quiet_mode))
      cprintf("[1;33;40m%s [1;37;40m\r Time on local machine\n\r",ctime(&ltime));
   sprintf(buf,"%s \r Time on local machine\n\r",ctime(&ltime));
   strcat(article, buf);

   sprintf(buf,"\n\r-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n\r\n\r");
   strcat(article, buf);

   if (set_time)
   {
      rc = DosGetDateTime(&DateTime);  /* Retrieve the current date and time  */
      if (rc != NO_ERROR)
      {
         sprintf (buf,"DosGetDateTime error : return code = %u\n", rc);
         error_out(buf);
      }
      DateTime.seconds = (UCHAR) ((BYTE) ((DateTime.seconds + sec_diff)%60));
      DateTime.minutes = (UCHAR) ((BYTE) ((DateTime.minutes + min_diff)%60));
      rc = DosSetDateTime(&DateTime);  /* Update the date and time            */
      if (rc!= NO_ERROR)
      {
         sprintf (buf,"DosSetDateTime error : return code = %u\n", rc);
         error_out(buf);
      }
      else
      {
         printf("Time adjusted to match %s\r\n",url.hostname);
      }
   }
   if ((strcmp(filen, "NoFIleNaME!"))!=0)
   {
      clear_file();
      writef(article);
   }
   free(line_in);
   close_exit();
}

/***********************************************************/
/*                  POPMAIL                                */
/*                                                         */
/*                                                         */
/*                                                         */
/***********************************************************/
void handle_popmail (void)
{
   int counter=1, num_of_messages;
   char *line_out;
   char *line_in;
   long total_lines,line;

   if ((line_in = (char *) malloc (4096*(sizeof(char)))) == NULL)
      error_out("Error allocating memory");
   if ((line_out = (char *) malloc (4096*(sizeof(char)))) == NULL)
      error_out("Error allocating memory");

   strcpy(line_in,""); // reset line_in variable

   if ((url.endx != 0) && (url.startx > url.endx))
      error_out("Error: First message must be lower than last message");

   while (line_in[0] != '+') // skip over any initial messages and get to the meat of things....
   {
      y = getaline(client_socket,line_in);
      watch(line_in);
   }

   // send the username and check responses
   sprintf(line_out,"user %s\r\n",url.username);
   send_input(client_socket,line_out);
   y = getaline(client_socket,buf);
   if ( !y  )
      error_out("Error: Problem retrieving header from server");
   watch(buf);
   if (buf[0] != '+')
     {
        sprintf(buf,"Error: No account for %s",url.username);
        error_out(buf);
     }

   // send the password and check responses
   sprintf(line_out,"pass %s\r\n",url.password);
   send_input(client_socket,line_out);
   y = getaline(client_socket,buf);
   if ( !y  )
      error_out("Error: no response from server");
   watch(buf);
   if ( buf[0] != '+' )
      error_out("Error: Login incorrect");

   // get status of mail
   send_input(client_socket,"stat\r\n");
   y = getaline(client_socket,buf);
   if ( !y  )
      error_out("Error: no response from server");
   watch(buf);
   if ( buf[0] != '+' )
     error_out("Error: Could not determine number of messages on server");
   buf=buf+4;
   num_of_messages = atoi(buf);

   if (num_of_messages != 0)
   {
      if (url.startx > 0)
      {
         counter = url.startx;
         if (counter > num_of_messages)
            error_out("First message requested is higher than number of messages on server\r\n");
         else
         {
            if(!(quiet_mode))
               cprintf("Beginning at message #%u\n\r",counter);
         }
      }
      else
         url.startx=1;// set url.startx to the real starting number - we use it later.

      if (url.endx > 0)
      {
         if (url.endx > num_of_messages)
         {
            if(!(quiet_mode))
               cprintf("[1;31;40mLast message requested is higher than last message on server[0;37;40m\r\n");
         }
         else
         {
            num_of_messages = url.endx;
            if(!(quiet_mode))
               cprintf("Last message reset to %u\r\n",num_of_messages);
         }
      }

      if ((num_of_messages+1-url.startx) > 1)
      {
         if(!(quiet_mode))
            cprintf("You have %u messages to get from the server.\r\n",(num_of_messages+1-url.startx));
      }
      else
      {
         if(!(quiet_mode))
            cprintf("You have %u message to get from the server.\r\n",(num_of_messages+1-url.startx));
      }

      // check to make sure we have a filename
      if ((strcmp(filen, "NoFIleNaME!"))==0)
      {
         //if no filename, set it to popmail.txt
         strcpy(filen,"popmail.txt");
      }

      // if we're overwriting the file, reset it now.
      clear_file();

      // loop until we've gotten all the mail
      for(;counter<=num_of_messages;counter++)
      {
         strcpy(article,""); // reset mail variable
         line = 0;

         // tell the server to send the message
         sprintf(line_out,"retr %u\r\n",counter);
         send_input(client_socket,line_out);
         y = getaline(client_socket,buf);
         if ( !y  )
            error_out("Error: no response from server");
         watch(buf);
         if ( buf[0] != '+' )
         {
            if(!(quiet_mode))
               cprintf("Error: Server could not send message #%u",counter);
         }
         // get the message
         do
         {
            y = getaline(client_socket,buf);
            if ( !y  )
               error_out("Error: no response from server");
            watch("                                                                   \r");
            watch(buf);
            line++;
            total_lines++;
            if ((strcmp(buf, ".\r\n"))!=0)
               strcat(article, buf);
            if(!(quiet_mode))
               cprintf("[1;33;40mRemain: %ld  Message: %ld  Total lines: %ld  Line: %ld[1;37;40m\r",(num_of_messages-counter), counter,total_lines,line);
         }
         while ((strcmp(buf, ".\r\n"))!=0);

         sprintf(buf,"-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n\r\n\r");
         strcat(article, buf);

         writef(article);
         if(!(quiet_mode))
            cprintf("Message #%u retrieved successfully                                     \r\n",counter);
         if(!(quiet_mode))
            cprintf("[1;33;40mRemain: %ld  Message: %ld  Total lines: %ld  Line: %ld[1;37;40m\r",(num_of_messages-counter), counter,total_lines,line);

         // now that we've written the mail to a file, we can delete it if they want
         if (delete_mail == 1)
         {
            sprintf(line_out,"dele %u\r\n",counter);
            send_input(client_socket,line_out);
            y = getaline(client_socket,buf);
            if ( !y  )
               error_out("Error: no response from server");
            watch(buf);
            if ( buf[0] != '+' )
            {
               if(!(quiet_mode))
                  cprintf("\r\n[1;31;40mError: Server would not delete mail[0;37;40m                               \r\n");
            }
            else
            {
               if(!(quiet_mode))
               {
                  cprintf("Article #%u deleted from server                                  \r\n",counter);
                  cprintf("[1;33;40mRemain: %ld  Message: %ld  Total lines: %ld  Line: %ld[1;37;40m\r",(num_of_messages-counter), counter,total_lines,line);
               }
            }
         }
      }
      send_input(client_socket,"quit\r\n"); // we're done
      y = getaline(client_socket,buf);// trash - we don't care - we're outta here
      watch(buf);
      if ((num_of_messages-url.startx) > 1)
      {
         if(!(quiet_mode))
            cprintf ("\r\n\r\n%u messages saved to %s",(counter-url.startx),filen);
      }
      else
      {
         if(!(quiet_mode))
            cprintf ("\r\n\r\n%u message saved to %s",(counter-url.startx),filen);
      }
   }
   // if stat returns 0 (there is no mail)
   else
      if(!(quiet_mode))
         cprintf("No mail on server");

   free(line_in);
   free(line_out);
   close_exit();
}



/***********************************************************/
/*               find_GLOB                                 */
/*                                                         */
/*                                                         */
/*                                                         */
/***********************************************************/
int find_glob (char *pattern)
{
   int counter;

   for (counter=0;counter < strlen(pattern);counter++)
   {
      // if we're at the end of the string, it's not globbing
      if (pattern[counter] == 0)
      {
         is_glob =0;
         break;
      }
      // if we find a globbing character...
      if ((pattern[counter] == '*') || (pattern[counter] == '?'))
      {
         is_glob = 1;
         break;
      }
   }
   return is_glob;
}

/***********************************************************/
/*                  GLOB                                   */
/*                                                         */
/*                                                         */
/*                                                         */
/***********************************************************/
int glob(char *pattern, char *checkme)
{
   char pattern_char,current_char,next_char;

   if ((strlen(checkme) == 1) && (checkme[0] = '.'))// . and .. are not included.
      return (0);
   if ((strlen(checkme) == 2) && (checkme[0] = '.') && (checkme[1] = '.'))
      return (0);

   // walk through the pattern matching each charater as we go
   if (!(caps_sensitive))
   {
      makeupper(&pattern);
      makeupper(&checkme);
   }

   if (((strlen (pattern)) < 1) || ((strlen (checkme)) < 1))
      return (0); // no pattern or something to check, no match

   for (;;) // we'll break out when we're done.
   {
      if ((pattern[0] == 0) || (checkme[0] == 0)) // see if we're at the end of the pattern or string to match
         return(1); // if we made it this far, it's a go.

      if (( (pattern[0] != '?') && (pattern[0] != '*')) && (pattern[0]) != checkme[0])
         return(0); // it doesn't match, we're outta here
      if (pattern [0] == '*')
      {
         pattern++; // advance pattern to next char
         if (pattern[0] == 0) // if the * was the last character we have to match, it's a go
            return(1);

         // if the next character in our string to check matches the next character in the pattern, we're good...
         while (checkme[0] != pattern[0])
         {
            checkme++;
            if (checkme[0] == 0)
               return(0); // no match
         }
      }
      // don't even need any code for ? - just advance to the next character
      pattern++;
      checkme++;
   }
}


/***********************************************************/
/*                      UNHTML                             */
/*                                                         */
/*                                                         */
/*                                                         */
/***********************************************************/
char *unhtml(char *linein)
{
   static char tempstr[2]; // a character buffer with an extra char for a '/0' so we can strcat it
   static char lineout[4096];  // a string buffer
   static char lastchar = 0;
   static char last2char = 0;
   static int flag=0;   // a flag to tell if we're inside a html tag or not.

   strcpy(tempstr, ""); // clear our buffers
   strcpy(lineout, "");

   if (flag) // if the last line ended inside a tag
   {
      while (linein[0] != '>')  // exit when the tag ends
      {
         if (strlen(linein) == 0)  // or exit if we're done with the string
            break;
         linein++;                 // else on to the next char
      }
      if (linein[0] == '>')        // reset our flag if we exited from a tag
         flag=0;
   }

   for (;;)
   {
      // See if we are at the beginning of some http code
      if (strlen(linein) == 0)  // if the line is clear, exit.
         break;
      while (linein[0] == '<') //use while instead of if in case of 2 in a row
      {
         flag=1;
         do
         {
            if (strlen (linein) == 0)
               break;
            linein++; //go over all the stuff in between
         }
         while (linein[0] != '>');

         if (linein[0] != '>') // if the line ended, exit before we reset our flag or go on
         {
            flag = 1;
            break;
         }
         flag=0; // we're outside the tag now
         linein++; //advance over the >
      }

      if (strlen (linein) == 0) // if the line ended, we're done
         break;

      if (linein[0] == '&') // get rid of '&nbsp;'
      {
         if((linein[1] == 'n') && (linein[2] == 'b') && (linein[3] == 's') && (linein[4] == 'p') && (linein[5] == ';'))
         {
            linein = linein + 5;
            linein[0] = ' ';
         }
      }

      if (linein[0] != '>')
      {
         last2char = lastchar;
         lastchar = tempstr[0];
         tempstr[0] = linein[0];
         if (!(((last2char == '\n') && (lastchar == '\n')) && (tempstr[0] == '\n')))  // try to cut down on the number of linefeeds
            strcat(lineout, tempstr);    // Add the character to the string
      }
      linein++;      // Increment the pointer
   }
   if (strlen(lineout) <=3)
      strcpy(lineout,"");
   return(lineout);
}


/***********************************************************/
/*               HANDLE_FINGER                             */
/*                                                         */
/*                                                         */
/*                                                         */
/***********************************************************/
void handle_finger (void)
{
   char *line_in;

   sprintf(buf,"%s\n\r",url.username);
   send_input(client_socket,buf);

   sprintf(buf,"Finger %s@%s\r\n\r\n",url.username,url.hostname);
   strcpy(article,buf);

   y=1;
   do
   {
      sprintf(line_in,"");
      y = getaline(client_socket,line_in); /* this inputs our line */
      if(!(quiet_mode))
         cprintf("%s",line_in);
      strcat(article, line_in);
   }
   while (y);

   sprintf(buf,"\n\r-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n\r\n\r");
   strcat(article, buf);

   if ((strcmp(filen, "NoFIleNaME!"))!=0)
   {
      clear_file();
      writef(article);
   }
   close_exit();
}


/***********************************************************/
/*                       MAKEUPPER                         */
/* converts an entire string to upper case                 */
/* inputs: string that may or may not be upper case        */
/* outputs: a completely uppercase string                  */
/***********************************************************/
char *makeupper (char **maybelower)  // converts an entire string to upper case
{
   int count, lngth;
   char temp_char;
   unsigned char nowupper[1024];

   if (*maybelower == NULL)
      return "";
   lngth=(strlen(*maybelower));
   for (count=0;count<lngth+1;count++)
   {
      nowupper[count] = toupper(**maybelower);
      *(*maybelower)++;
   }
   strcpy(*maybelower,nowupper);
   return(*maybelower);
}

/***********************************************************/
/*                            ERROR_OUT                    */
/* prints out what any errors are then exits               */
/* inputs: any error codes to print                        */
/* outputs: none                                           */
/***********************************************************/

void error_out(char *erro) /* if there was an error, prints what the error was and exits */
{
   if(!(quiet_mode))
   {
      cprintf("\r\n[1;37;40m");
      cprintf("%s",erro);
      cprintf("[0;37;40m\r\n");
   }
   sprintf(buf,"%s\r\n",erro);
   log_to_file(buf);
   close_exit();
}

/***********************************************************/
/*                        WARNING_OUT                      */
/* prints out what any errors are then exits               */
/* inputs: any error codes to print                        */
/* outputs: none                                           */
/***********************************************************/

void warning_out(char *erro) /* if there was an error, prints what the error was and exits */
{
   if(!(quiet_mode))
   {
      cprintf("\r\n[1;37;40m");
      cprintf("%s",erro);
      cprintf("[0;37;40m\r\n");
   }
   sprintf(buf,"%s\r\n",erro);
   log_to_file(buf);
}

/***********************************************************/
/*                        CLOSE_EXIT                       */
/* shuts down everything and leaves                        */
/* inputs: none                                            */
/* outputs: none                                           */
/***********************************************************/
void close_exit() /* shuts everything down and exits */
{
   APIRET   rc = NO_ERROR;     /* Return code                       */

   if (beep_when_done)
   {
      rc = DosBeep(1440L,         /* Beep frequency, in hertz          */
                   1000L);        /* Duration of beep, in milliseconds */
   }

   VioSetCurType(&oinfo,0);     //turns cursor back on
   close_socks(client_socket);  /* who cares- were outta here. */
   cprintf("[1;37;40m\r\n");   //changes the text color back to white
   exit(0);
}

/***********************************************************/
/*                   WRITEF                               */
/* called from _beginthread command, writes a file from    */
/* global variables                                        */
/* Inputs: filename, file_write                            */
/* outputs: none unless fails to open file                 */
/***********************************************************/

void writef (char *writeme) /* actually writes the file */
{
   /* access file to write */
   out = fopen(filen,"a");
   time(&wait_time);
   if (fwrite(writeme,strlen(writeme),1,out)==0) /* writes the subject to the file */
   {
      fclose(out);
      error_out("Error: Could not write to file");
   }
   time(&wait_time);
   fclose(out);
}


/***********************************************************/
/*         BBBBB   U    U   GGGG                           */
/*         B    B  U    U  G                               */
/*         BBBBB   U    U  G  GGG                          */
/*         B    B  U    U  G    G                          */
/*         BBBBB    UUUU    GGGG                           */
/***********************************************************/
void bug (void)
{
cprintf("bugcheck: %u\r\n",++bugcheck);
}

/***********************************************************/
/*                         bannr                           */
/* prints a title and author banner with a bunch of ansi   */
/* inputs: none                                            */
/* outputs: none                                           */
/***********************************************************/

void bannr() /* prints a title and author banner with a bunch of ansi crap */
{
   if (!(quiet_mode))
   {
      cprintf("\r\n[1;33;40mNETGRAB [1;37;40m(c)1995, 1997[1;31;40m");
      cprintf(" Martin L. Roth & Stephen Loomis[0;37;40m v0.98b\r\n");
      cprintf("[1;37;40m        Address all questions and comments to tidbit@ezlink.com[0;37;40m\r\n\r\n");
   }
}

/***********************************************************/
/*                          usage                          */
/* prints the instructions for the program                 */
/* inputs: none                                            */
/* outputs: none                                           */
/***********************************************************/

void usage() // prints the instructions
{
   if(!(quiet_mode))
   {
      cprintf("[1;33;40mWEB:\r\n[1;37;40m");
      cprintf("netgrab -options http://<site>[:port]/<pathname>/<filename> [localfile]\r\n");

      cprintf("[1;33;40mNEWS:\r\n[1;37;40m");
      cprintf("netgrab -options nntp://<site>[:port]/<group>/[1st][:last],\r\n");
      cprintf("        [-don't get],[[+]do get] [localfile]\r\n");
      cprintf("netgrab nntp://site[:port] [localfile] Gets a list of available newsgroups.\r\n");

      cprintf("[1;33;40mFTP:\r\n[1;37;40m");
      cprintf("netgrab -options ftp://[user][:pass@]<site>[:port]/[remotefile] [localfile]\r\n");

      cprintf("[1;33;40mPOPMAIL:\r\n[1;37;40m");
      cprintf("netgrab -options popmail://<user><:pass>@<site>[:port]/[1st][:last][localfile]\r\n");

      cprintf("[1;33;40mTIME:\r\n[1;37;40m");
      cprintf("netgrab -options time://<site>[:port] [localfile]\r\n");

      cprintf("[1;33;40mFINGER:\r\n[1;37;40m");
      cprintf("netgrab -options finger://[user@]<site>[:port] [localfile]\r\n");

      cprintf("[1;33;40mOptions:[1;37;40m");
      cprintf("  -a   - append to localfile instead of overwriting\r\n");
      cprintf("          -A   - FTP in ASCII mode instead of binary\r\n");
      cprintf("          -b   - Beep when finished\r\n");
      cprintf("          -d   - Delete mail after receiving\r\n");
//      cprintf("          -D   - Debug mode\r\n");
      cprintf("          -g   - get all files from html page\r\n");
      cprintf("          -h   - just get news subjects - no articles or headers\r\n");
      cprintf("          -l   - save session to log file (netgrab.log is default)\r\n");
      cprintf("          -n   - don't get news headers\r\n");
      cprintf("          -o   - skip file if file of that name exists already\r\n");
//      cprintf("          -p   - Parse out all urls from html page\r\n");
      cprintf("          -q   - quiet mode - does not print anything\r\n");
      cprintf("          -s   - strips all html codes\r\n");
      cprintf("          -tX  - times out after X seconds.  Default = 120\r\n");
//      cprintf("          -T   - Changes machine's time to match remotehost\r\n");
      cprintf("          -uX  - change user-agent name sent 'X'.  Default = OS/2 NetGrab\r\n");
      cprintf("          -w   - Watch all messages from server.\r\n");
   }

exit(0);
}

/***********************************************************/
/*                  parseoptions                           */
/* passes a list of vaild options to getopt & changes vars */
/* based on the return                                     */
/* input: argc, argv                                       */
/* output: none                                            */
/***********************************************************/
void parseOptions (int argc, char **argv,char **option_output)
{
   char *temp_output;
   int c;

   if((temp_output = malloc (1024 * (sizeof(char)))) == NULL)
      error_out("Error allocating memory");

   sprintf(temp_output,"");
   sprintf(*option_output,"");

   // remember, the : comes after the option that takes the argument
   while ((c = getopt(argc, argv, "aAbdDghlnopqst:Tu:wz")) != EOF)
   {
      switch (c)
      {
         case 'a':
            append_to_file = 1;
            break;
         case 'A':
            ftp_mode_binary=0;
            break;
         case 'b':
            beep_when_done=1;
            break;
         case 'd':
            delete_mail=1;
            break;
         case 'D':
            watch_server=1;
            debug=1;
            break;
         case 'g':
            get_all=1;
            break;
         case 'h':
            get_head_only=1;
            break;
         case 'l':
            if (getenv("ETC") != NULL)              // get the ETC variable from the environment
               sprintf(logfile,"%s\\netgrab.log",getenv("ETC"));
            else
               strcpy(logfile,"netgrab.log");
            sprintf(temp_output,"Saving log to %s\r\n",logfile);
            strcat(*option_output,temp_output);
            keep_a_log=1;
            break;
         case 'n':
            get_news_head=0;
            break;
         case 'o':
         overwrite_file=0;
            break;
         case 'p':
            parse_html=1;
            break;
         case 'q':
            quiet_mode=1;
            break;
         case 's':
            strip_html=1;
            break;
         case 'w':
            watch_server=1;
            break;
         case 't':

            if (optarg[0] == '0') // if optarg really is '0'
            {
               sprintf(temp_output,"Timeout disabled\r\n");
               strcat(*option_output,temp_output);
               timeout=0;
               break;
            }
            timeout=atoi(optarg);
            if (timeout == 0)
            {
               sprintf(temp_output,"Error: cannot accept timeout value. (Enter as -t 150)\r\nTimeout value set at default.\r\n");
               timeout=120;
               strcat(*option_output,temp_output);
            }
            sprintf(temp_output,"Times out after %u seconds\r\n",timeout);
            strcat(*option_output,temp_output);
            break;
         case 'T':
            set_time=1;
            break;
         case 'u':
            if (strlen(optarg) > 1)
            {
               strcpy(user_agent,optarg);
               sprintf(temp_output,"User-agent set to %s\r\n",user_agent);
               strcat(*option_output,temp_output);
            }
            else
            {
               sprintf(temp_output,"Error: cannot accept User-agent.  (Enter as -u Netscape)\r\nUser-agent defaulting to 'OS/2 NetGrab'\r\n");
               strcat(*option_output,temp_output);
            }
            break;
         case 'z':
            sprintf(temp_output,"\r\n\r\n[1;31;40mHEY, READ THE DOCS!!!  THERE'S NO OPTION 'z'!!!![1;37;40m\r\n\r\n");
            strcat(*option_output,temp_output);
            break;
         default:
            usage();
      }
   }
   free (temp_output);
}

/***********************************************************/
/*               GETOPT                                    */
/*                                                         */
/*                                                         */
/*                                                         */
/***********************************************************/
int getopt(int argc, char **argv, char *opts)
{
   static int sp = 1;
   int c=0;
   char *cp;

   if((cp=malloc (256 * (sizeof(char)))) == NULL)
      error_out("Error allocating memory");

   if(sp == 1) // if this is the first time through check to see if there are any command line options
   {
      if(optind >= argc || argv[optind][0] != '-' || argv[optind][1] == '\0')
      {
         return(EOF);
      }
      else if(strcmp(argv[optind], "--") == 0)
      {
         optind++;
         return(EOF);
      }
   }

   optopt = c = argv[optind][sp];
   if(c == ':' || (cp=strchr(opts, c)) == NULL)
   {
      ERR(": illegal option -- ", c);
      if(argv[optind][++sp] == '\0')
      {
         optind++;
         sp = 1;
      }
      return('?');
   }
   if(*++cp == ':')
   {
      if(argv[optind][sp+1] != '\0')
      {
         optarg = &argv[optind++][sp+1];
         if(!(quiet_mode))
            cprintf("-%s-\n\r",optarg);
      }
      else if(++optind >= argc)
      {

         ERR(": option requires an argument -- ", c);
         sp = 1;
         return('?');
      }
      else
         optarg = argv[optind++];
      sp = 1;
   }
   else
   {
      if(argv[optind][++sp] == '\0')
      {
         sp = 1;
         optind++;
      }
      optarg = NULL;
   }
   return(c);
}

/***********************************************************/
/*                       checktime                         */
/* waits for a connection to time out                      */
/* inputs: a global variable of the last time we got data  */
/* outputs: exits if error                                 */
/***********************************************************/
void _Optlink check_time (void *foo)
{
   time_t current_time;
   for(;;)
   {
      if (difftime(time(&current_time),wait_time) >timeout)
      {
         cprintf("\r\nError: Connection timed out\r\n");
         VioSetCurType(&oinfo,0);     //turns cursor back on
         close_socks(client_socket);  /* who cares- were outta here. */
         cprintf("[1;37;40m\r\n");   //changes the text color back to white
         exit (1);
      }
      else
         DosSleep(10000);
   }
}

/***********************************************************/
/*                WATCH                                    */
/*                                                         */
/*                                                         */
/*                                                         */
/***********************************************************/
void watch (char *seeme)
{
   if (watch_server)
      if(!(quiet_mode))
         cprintf("%s",seeme);
   log_to_file(seeme);
}

/***********************************************************/
/*               SHOWURL                                   */
/* Shows the Url we just parsed                            */
/*                                                         */
/*                                                         */
/***********************************************************/
void showurl (void)
{
int counter=0;

   if (debug)
   {
      if(!(quiet_mode))
      {
         cprintf("\r\n");
         cprintf("Url Type   = %u\r\n",url.type);
         cprintf("Hostname   = %s\r\n",url.hostname);
         cprintf("Port #     = %u\r\n",url.port);
         cprintf("Path       = %s\r\n",url.path);
         cprintf("Start #    = %u\r\n",url.startx);
         cprintf("End #      = %u\r\n",url.endx);
         cprintf("Username   = %s\r\n",url.username);
         cprintf("Password   = %s\r\n",url.password);
         cprintf("# to parse = %u\r\n",url.looknum);
         cprintf("Filename   = %s\r\n",filen);
      }
      sprintf(buf,"type       = %u\r\n",url.type);
      log_to_file(buf);
      sprintf(buf,"hostname   = %s\r\n",url.hostname);
      log_to_file(buf);
      sprintf(buf,"port       = %u\r\n",url.port);
      log_to_file(buf);
      sprintf(buf,"path       = %s\r\n",url.path);
      log_to_file(buf);
      sprintf(buf,"start num  = %u\r\n",url.startx);
      log_to_file(buf);
      sprintf(buf,"end num    = %u\r\n",url.endx);
      log_to_file(buf);
      sprintf(buf,"username   = %s\r\n",url.username);
      log_to_file(buf);
      sprintf(buf,"password   = %s\r\n",url.password);
      log_to_file(buf);
      sprintf(buf,"# to parse = %u\r\n",url.looknum);
      log_to_file(buf);
      sprintf(buf,"Filename   = %s\r\n",filen);
      log_to_file(buf);
   }
}



/***********************************************************/
/*                  HANDLE_HTTP                            */
/*                                                         */
/*                                                         */
/*                                                         */
/***********************************************************/
void handle_http(void)
{
   struct timeval timeout;
   float length_of_time;
   time_t begin_time, finish_time;
   unsigned long length, readin=0;
   long rval,block,rate;
   int z,x,a,b,c,d,trash,ch;
   char *getstring, *buf1, *buf2;
   char *header, *filename;
   char *leng, *build, *tmp, *buffer, *ext;

   if ((getstring =  (char *) malloc(1024*(sizeof(char)))) == NULL)
      error_out("Error allocating memory");
   if ((buf1   = (char *) malloc(4096*(sizeof(char)))) == NULL)
      error_out("Error allocating memory");
   if ((buf2 =  (char *) malloc(2048*(sizeof(char)))) == NULL)
      error_out("Error allocating memory");
   if ((tmp =  (char *) malloc(2048*(sizeof(char)))) == NULL)
      error_out("Error allocating memory");
   if ((filename =  (char *) malloc(128*(sizeof(char)))) == NULL)
      error_out("Error allocating memory");
   if ((header =  (char *) malloc(2048*(sizeof(char)))) == NULL)
      error_out("Error allocating memory");
   if ((leng =  (char *) malloc(2048*(sizeof(char)))) == NULL)
      error_out("Error allocating memory");
   if ((build = (char *) malloc(20*(sizeof(char)))) == NULL)
      error_out("Error allocating memory");
   if ((ext = (char *) malloc (20 * (sizeof(char)))) == NULL)
      error_out("Error allocating memory");
   if (( buffer = (char *) malloc (2048 * (sizeof(char)))) == NULL )
      error_out("Error allocating data buffer");


   sprintf(build,"úùþ+*°±²Û");

   _splitpath(url.path,NULL,NULL,buf,buf1);
   sprintf(filename,"%s%s",buf,buf1);

   if (strlen(buf1) < 6)
      strcpy(ext,buf1); // get our extension to see if we want to look for html tags in this for get_all
   else
      strcpy(ext,"");

   // if length of filename is 0, we're getting an index.htm
   if ((strlen (filen)) == 0)
      strcpy(filen, "index.htm");

   if ((strlen (filename)) == 0)
   {
      strcpy(filename, "index.htm");
      strcpy(ext,".htm");
   }
   // if filen is null, set it to the remote filename
   if ((strcmp(filen, "NoFIleNaME!"))==0)
      strcpy(filen, filename);


   sprintf(getstring,"GET %s HTTP/1.0\r\nAccept: */*; q=0.300\r\nUser-Agent: %s \r\n\r\n",url.path,user_agent);
   send_input(client_socket,getstring);
   watch(getstring);
   rval = infromsock(client_socket,buf,1512);
   if ( rval == -1 )
      error_out("Error: retrieving header from server");
   strncpy(buf1,buf,rval);
   z=0;

   while (z<1)
   {
      c=0;
      b=0;
      sprintf(buf2,"%c%c\0",0x0a,0x0a);
      z = strstr(buf1,buf2) - buf1;
      if ( z > 0 )
         c =2;
      sprintf(buf2,"%c%c%c\0",0x0a,0x0d,0x0a);
      x = 0;
      x = strstr(buf1,buf2) - buf1;
      if ( x > 0 )
         b=3;
      if ( x<z && x>0)
         { z=x; c=b; }
      if ( z <1 && x == 0)
      {
         for (z=0;z<rval;z++)
            printf("%0X ",buf1[z]);
         close_exit();
      }
      if ( z<1)
      {
         z=x;
         c=b;
      }
   }

   strncpy(header,buf1,z);
   memcpy(tmp,buf+(z+c),rval-(z+c));
   watch(header);
   if ( (leng = strstr(header," 404 ")) != NULL )
       error_out("HTTP: File not found\r\n");
   if ( (leng = strstr(header,"ound")) != NULL )
       error_out("HTTP: File not found\r\n");
   if ( (leng = strstr(header,"ength:")) == NULL )
   {
      length = 538400;
   }
   else
   {
      leng+=7;
      length = atol(leng);
   }
   time(&begin_time); // set up our beginning time here.

   if ( length != 538400)
      if(!(quiet_mode))
         cprintf("%s is %ld bytes long\r\n\r\n",filename,length);

   if (overwrite_file)
      clear_file();
   else
      if (0 == access(filen,00))// if file already exists
         error_out("Error: file exists on drive");

   readin=rval-(z+c);
   if(!(quiet_mode))
      cprintf("[1;32;40m%c",build[0]);
   ch=1;
   block=1;

   memcpy(buffer,tmp,readin);

   // 1st time we fill buf
   // for getting a full site
   if ((get_all) && (strnicmp(ext,".htm",4) == 0))
      get_site(buffer,(int)readin);

   out = fopen(filen,"ab");

   if (strip_html)
   {
      sprintf(buf,"%s",unhtml(buffer));
      fwrite(buf,sizeof(char),strlen(buf),out);
   }
   else
      fwrite(buffer,sizeof(char),readin,out);

   while (rval > 0)
   {
      rval = recv(client_socket, buf, 4096, 0x0);
      readin = readin + rval;
      time(&wait_time);
      if ( rval > 0)
      {

         // for getting a full site
         if ((get_all) && (strnicmp(ext,".htm",4) == 0))
            get_site(buf,(int)rval);
         if (strip_html) //2nd
         {
            strcpy(buf1,(unhtml(buf)));
            fwrite(buf1,sizeof(char),strlen(buf1),out);
         }
         else
            fwrite(buf,sizeof(char),rval,out);

         if(!(quiet_mode))
            cprintf("%c%c",8,build[ch]);
         ch++;
         if ( ch == 10)
         {
            ch =0;
            if(!(quiet_mode))
               cprintf(" ");
         }
         block++;
      }
   }
   fclose(out);
   time(&finish_time);
   length_of_time=difftime(finish_time,begin_time);

   if(!(quiet_mode))
   {
      cprintf("[1;37;40m");
      cprintf( "\r\n\r\nFinished receiving %s\r\n",filename);
      if(debug)
      {
         cprintf("%u bytes received in %1.0f seconds\n\r",readin,length_of_time);
         cprintf("For an average rate of %1.1f cps\n\r",readin/(length_of_time+.001));
         cprintf("Saved to %s",filen);
      }
      cprintf("[0;37;40m");
   }
   close_socks(client_socket);

/*   free(getstring);
   free(buf1);
   free(buf2);
   free(tmp);
   free(filename);
   free(header);
   free(leng);
   free(build);
   free(ext);
   free(buffer);
*/

   return;
}

/***********************************************************/
/*                   HANDLE FILE                           */
/*                                                         */
/*                                                         */
/*                                                         */
/***********************************************************/
void handle_file (void)
{
   char readchar;
   char *build,*buf1;
   char buf2[2];
   long counter=0,counter2=0,counter3=0,filelength=0,newlength=0; // counter for file length
   int ch=1,linelength=0;           // counter for build

   if( (build = (char *) malloc(20*(sizeof(char)))) == NULL)
      error_out("ERROR allocating memory");
   if ((buf1 = (char *) malloc(1030*(sizeof(char)))) == NULL)
      error_out("ERROR allocating memory");

   sprintf(build,"úùþ+*°±²Û");
   sprintf(buf2," ");

   strcpy(buf1,"");

   in = fopen(url.path,"rb");   // open the file
   if (in == NULL)
      error_out("Error: Could not open file");

   if (watch_server == 0)     // start doing our hash marks
      if(!(quiet_mode))
         cprintf("%c",build[0]);

   // read in the file
   for (;;)
   {
      readchar = fgetc(in);   // read in a char
      if (feof(in) != 0)      // if we're at the end of file, exit the loop
         break;
      else
      {
         time(&wait_time);
         article[counter] = readchar; //append the char to the article
         counter++;                   // add one to the number of characters read in

         if (watch_server)
            if(!(quiet_mode))
               cprintf("%c",readchar);
         else
         {
            if (!(counter % 128)) // if counter is divisible evenly by 128
            {
               if(!(quiet_mode))
                  cprintf("%c%c",8,build[ch]); // then add a hash mark
               ch++;
               if ( ch == 10) // if we're at the end of our hash string
               {
                  ch=0;      //reset the string to 0
                  if(!(quiet_mode))
                     cprintf(" "); //and advance to the next char
               }
            }
         }
      }
   }
   fclose (in);
   filelength = counter;
   if(!(quiet_mode))
      cprintf("\n\r\n\r%s is %u bytes long\r\n",url.path,filelength); // then add a hash mark
   // see what needs to be done

   // check for unhtml
   if (strip_html)
   {

      if(!(quiet_mode))
         cprintf("\r\nRemoving html code\r\n");
      for (counter=0;counter<filelength;counter++)
      {
         buf2[0] = article[counter];
         strcat(buf1,buf2);
         if ((strlen(buf1) == 1024) || (counter == filelength -1)) // watch for end of line
         {
            sprintf(buf,"%s",unhtml(buf1));// send the line to unhtml
            linelength = strlen(buf);
            for(counter3=0;counter3<=strlen(buf);counter3++) // count up to the length of the unhtmled line
            {
               article[(newlength+counter3)] = buf[counter3]; // put the new line right back into the article variable
            }

            newlength=newlength+linelength;
            sprintf(buf2,"");
            sprintf(buf1,"");
         }
      }
      if(!(quiet_mode))
         cprintf("%u bytes removed from file\r\n",(filelength-newlength));
      filelength = newlength;
   }


   // save the file
   // save to another file?
   if ((strcmp(filen, "NoFIleNaME!"))==0)
      strcpy(filen,url.path);
   clear_file();
   if(!(quiet_mode))
      cprintf("\r\nSaving %u bytes to %s\r\n",filelength,filen);
   out = fopen(filen,"ab");
   for (counter = 0;counter<filelength;counter++)
   {
      fputc(article[counter],out); /* writes the subject to the file */
   }
   fclose(out);
   close_exit();
}


/***********************************************************/
/*                   HANDLE GOPHER                         */
/*                                                         */
/*                                                         */
/*                                                         */
/***********************************************************/
void handle_gopher(void)
{
   int ok=1,counter;
   char *getstring, *buf1, *buf2,*buf3,*path;
   char *filename,*I;
   char *line_in;

   if ((line_in = (char *) malloc (4096*(sizeof(char)))) == NULL)
      error_out("Error allocating memory");
   if ((getstring =  malloc(1024*(sizeof(char)))) == NULL)
      error_out("Error allocating memory");
   if ((buf1 =  malloc(2048*(sizeof(char)))) == NULL)
      error_out("Error allocating memory");
   if ((buf2 =  malloc(2048*(sizeof(char)))) == NULL)
      error_out("Error allocating memory");
   if ((buf3 =  malloc(2048*(sizeof(char)))) == NULL)
      error_out("Error allocating memory");
   if ((filename =  malloc(128*(sizeof(char)))) == NULL)
      error_out("Error allocating memory");
   if ((I = malloc(128*(sizeof(char)))) == NULL)
      error_out("Error allocating memory");
   if ((path =  malloc(1024*(sizeof(char)))) == NULL)
      error_out("Error allocating memory");

   // if length of filename is 0, we're getting an index.htm
   if ((strlen (filen)) == 0)
      strcpy(filen, "gopher.txt");

   // if filen is null, set it to the remote filename
   if ((strcmp(filen, "gopher.txt"))==0)
      strcpy(filen, filename);


   // splits the url.path into the path and the filename
   if ((strlen(url.path)) > 1)
   {
      _splitpath(url.path,buf,buf1,buf2,buf3);
      sprintf(filename,"%s%s",buf2,buf3);
      sprintf(path,"%s%s",buf,buf1);
   }
   else
   {
      sprintf(filename,"");
      sprintf(path,"%s",url.path);
   }
   if(!(quiet_mode))
      cprintf("[1;37;40m");
   sprintf(buf,"\n\r");
   send_input(client_socket,buf);

   while(ok)
   {
      y = getaline(client_socket,line_in); /* this inputs our line */
      if ( (line_in[0] == '.') && (strlen(line_in) < 4) ) // a single . indicates the end.
         ok = 0;
      else
      {
         if(!(quiet_mode))
            cprintf("%s",line_in);
         writef(line_in);
      }
   }

close_exit();
}


/***********************************************************/
/*                     handle_ftp                          */
/*  this function opens a control connection to an ftp     */
/*  server then hands off to another thread for the data   */
/*  connection                                             */
/*                                                         */
/*                                                         */
/***********************************************************/
void handle_ftp(void)
{
   int A,B,C,D,E,F,flag;
   char *getstring, *buf1, *buf2,*buf3,*path;
   char *filename,*I;

   if ((getstring =  malloc(1024*(sizeof(char)))) == NULL)
      error_out("Error allocating memory");
   if ((buf1 =       malloc(2048*(sizeof(char)))) == NULL)
      error_out("Error allocating memory");
   if ((buf2 =       malloc(2048*(sizeof(char)))) == NULL)
      error_out("Error allocating memory");
   if ((buf3 =       malloc(2048*(sizeof(char)))) == NULL)
      error_out("Error allocating memory");
   if ((filename =   malloc(128*(sizeof(char)))) == NULL)
      error_out("Error allocating memory");
   if ((I = malloc(128*(sizeof(char)))) == NULL)
      error_out("Error allocating memory");
   if ((path = malloc(1024*(sizeof(char)))) == NULL)
      error_out("Error allocating memory");
   if ((ftp_get = malloc(sizeof(FTP_DATA))) == NULL)
      error_out("Error allocating memory");

   // splits the url.path into the path and the filename
   if ((strlen(url.path)) > 1)
   {
      _splitpath(url.path,buf,buf1,buf2,buf3);
      sprintf(filename,"%s%s",buf2,buf3);
      sprintf(path,"%s%s",buf,buf1);
   }
   else
   {
      sprintf(filename,"");
      sprintf(path,"%s",url.path);
   }

   find_glob(filename);
   if (is_glob)
   {
      filen = filename;
   }
   if(!(quiet_mode))
      cprintf("[1;37;40m");
   do
   {
      y = getaline(client_socket,buf);// trash line
      watch(buf);
   }
   while (buf[3] == '-');

   // send the username and check responses
   sprintf(getstring,"user %s\r\n",url.username);
   send_input(client_socket,getstring);
   if(!(quiet_mode))
      cprintf("Sent username: %s\r",url.username);
   y = getaline(client_socket,buf);
   if ( !y  )
      error_out("Error: Problem retrieving header from server");
   watch(buf);
   y = atoi(buf);
   if (y != 230) // checks to see if server accepted username and doesn't need a password
   {
      if ( y != 331 )
        {
           sprintf(buf,"Error: No account for %s",url.username);
           error_out(buf);
        }
      if(!(quiet_mode))
         cprintf("Username Accepted                                      \r");
      // send the password and check responses
      sprintf(getstring,"pass %s\r\n",url.password);
      send_input(client_socket,getstring);
      if(!(quiet_mode))
         cprintf("Sent Password:                                          \r");
      y = getaline(client_socket,buf);
      if ( !y  )
         error_out("Error: no response from server");
      watch(buf);
      y = atoi(buf);
      if ( y == 530 )
        error_out("Error: Login incorrect");
      if ( y != 230 )
        error_out("Error: Didn't get a server response");
      if(!(quiet_mode))
         cprintf("Password Accepted                                       \r");
   }
   while (buf[3] == '-')
   {
      y = getaline(client_socket,buf);// trash line
      watch(buf);
   }
   // change to stream mode and check responses
   sprintf(getstring,"mode s\r\n");
   send_input(client_socket,getstring);
   y = getaline(client_socket,buf);
   if ( !y  )
      error_out("Error: no response from server");
   watch(buf);
   y = atoi(buf);
   if ( y != 200 )
     error_out("Error: stream mode not available");
   // change to passive mode and check responses
   sprintf(getstring,"pasv\r\n");
   send_input(client_socket,getstring);
   y = getaline(client_socket,buf);
   if ( !y  )
      error_out("Error: no response from server");
   watch(buf);
   y = atoi(buf);
   if ( y != 227 )
     error_out("Error: Can\'t open passive connection");
   I = strstr(buf,"(");    // this section parses out the ip and socket of the
   I += 1;                 // data connection
   A = atoi(I);
   I = strstr(I,",");      // a.b.c.d is the ip
   I += 1;
   B = atoi(I);
   I = strstr(I,",");      // and e*256 + f is the socket number
   I += 1;
   C = atoi(I);
   I = strstr(I,",");
   I += 1;
   D = atoi(I);
   I = strstr(I,",");
   I += 1;
   E = atoi(I);
   I = strstr(I,",");
   I += 1;
   F = atoi(I);

   ftp_get->Fflag = 1;
   sprintf(ftp_get->host,"%d.%d.%d.%d",A,B,C,D); // the ip address of the data connection
   sprintf(ftp_get->lhost,url.path);
   ftp_get->hostp=(E*256)+F; // the socket number for the data connection
   ftp_get->ls_list=0; // set this so that the new thread doesn't try to build a dir automatically
   if (watch_server)
      if(!(quiet_mode))
         cprintf("Starting data connection with %s on socket %u\n\r",ftp_get->host,ftp_get->hostp);

   ftp_conTid= _beginthread(ftp_con, 0, STACK_SIZE,  ftp_get); // start a thread to get the data

   // change to the directory the file should be in
//   if ((strlen(path) > 1) && ((path[(strlen(path)-1)] == '/') || (path[(strlen(path)-1)] == '\\')))
//      path[(strlen(path)-1)] = 0;
   sprintf(getstring,"CWD %s\r\n",path);
   watch(getstring);
   send_input(client_socket,getstring);
   do
   {
      y = getaline(client_socket,buf);
      if ( !y  )
         error_out("Error: no response from server on CWD");
      watch(buf);
   }
   while (buf[3] == '-');

   y = atoi(buf);
   if ( y != 250 )
   {
      reverse_slashes(&path);
      strcpy(buf,url.path);
      reverse_slashes(&buf);

   sprintf(ftp_get->lhost,buf);

      sprintf(getstring,"CWD %s\r\n",path);
      watch(getstring);
      send_input(client_socket,getstring);
      do
      {
         y = getaline(client_socket,buf);
         if ( !y  )
            error_out("Error: no response from server on CWD");
         watch(buf);
      }
      while (buf[3] == '-');

      y = atoi(buf);
      if ( y != 250 )
         error_out("Error: Specified directory does not exist.");
   }
   if(!(is_glob))
   {
      // change to the path to see if it's a dir or file
      if (!(strlen(filename)<=1))
      {
         if ((strlen(filename) > 1) && ((filename[(strlen(filename)-1)] == '/') || (filename[(strlen(filename)-1)] == '\\')))
            filename[(strlen(filename)-1)] = 0;
         sprintf(getstring,"CWD %s\r\n",filename);
         watch(getstring);
         send_input(client_socket,getstring);
         do
         {
            y = getaline(client_socket,buf);
            if ( !y  )
               error_out("Error: no response from server on CWD");
            watch(buf);
         }
         while (buf[3] == '-');
         y = atoi(buf);
      }
      else
         y=250;
      if ( y == 250 ) // it's a directory - get a dirlist
      {
         ftp_get->ls_list=1;
         sprintf(getstring,"LIST\r\n");
         watch(getstring);
         send_input(client_socket,getstring);
         y = getaline(client_socket,buf);
         if ( !y  )
            error_out("Error: no response from server on list");
         watch(buf);
         y = atoi(buf);
         if (y != 150)
         {
            if (watch_server)
               if(!(quiet_mode))
                  cprintf("%u\r\n",y);
            error_out("Error: Could not get directory list");
         }
         // if filen is null, set it to 00index.txt
         if ((strcmp(filen, "NoFIleNaME!"))==0)
         {
            strcpy(filen, "00index.txt");
         }
         if (overwrite_file)
         {
            out = fopen(filen,"wb");
            fclose(out);
         }
         if(!(quiet_mode))
            cprintf("Saving directory of %s to %s\r\n",url.path,filen);
      }

      else if ( y == 550 ) // it's a file (or not there) try to get it
      {
         if (ftp_mode_binary)
            sprintf(getstring,"TYPE I\r\n");
         else
            sprintf(getstring,"TYPE A\r\n");
         watch(getstring);
         send_input(client_socket,getstring);
         y = getaline(client_socket,buf);
         if ( !y  )
            error_out("Error: no response from server on tupe");
         watch(buf);
         y = atoi(buf);
         if ( y != 200 )
            error_out("Error: Could not set type");
         sprintf(getstring,"RETR %s\r\n",filename);
         watch(getstring);
         send_input(client_socket,getstring);
         y = getaline(client_socket,buf);
         if ( !y  )
            error_out("Error: no response from server on send");
         watch(buf);
         y = atoi(buf);
         if ( y != 150 )
            error_out("Error: No such file or directory");
         // if filen is null, set it to the remote filename
         if ((strcmp(filen, "NoFIleNaME!"))==0)
         {
            strcpy(filen, filename);
         }
         if (overwrite_file)
         {
            out = fopen(filen,"wb");
            fclose(out);
         }
         if(!(quiet_mode))
            cprintf("Saving %s to %s\r\n",filename,filen);
      }
      ftp_get->Fflag=0;
      DosWaitThread(&ftp_conTid, DCWW_WAIT);  // wait til file is here
   }







   if (is_glob)
   {
      sprintf(ftp_get->lhost,path);
      if(!(quiet_mode))
         cprintf("Getting file list. \r\n");

      ftp_get->ls_list = 1;

      sprintf(getstring,"LIST\r\n");
      watch(getstring);
      send_input(client_socket,getstring);
      y = getaline(client_socket,buf);
      if ( !y  )
         error_out("Error: no response from server on list");
      watch(buf);
      y = atoi(buf);
      if (y != 150)
      {
         if (watch_server)
            if(!(quiet_mode))
               cprintf("%u\r\n",y);
         error_out("Error: Could not get directory list");
      }

      ftp_get->Fflag=0;
      DosWaitThread(&ftp_conTid, DCWW_WAIT);  // wait til file is here

      y = getaline(client_socket,buf);
      watch(buf);

      // setting type for ftp transfer
      if (ftp_mode_binary)
         sprintf(getstring,"TYPE I\r\n");
      else
         sprintf(getstring,"TYPE A\r\n");
      watch(getstring);
      send_input(client_socket,getstring);
      y = getaline(client_socket,buf);
      if ( !y  )
         error_out("Error: no response from server on tupe");
      watch(buf);
      y = atoi(buf);
      if ( y != 200 )
         error_out("Error: Could not set type");
      // done setting type for ftp transfer



      list_current = list_begin;
      if(list_begin->text == NULL)
         error_out("No files to get");

      for(;;)
      {
         filen = list_current->text;
         if (overwrite_file)
         {
            out = fopen(filen,"wb");
            fclose(out);
         }
         else
            while (0 == access(filen,00))// if file already exists
            {
               if(!(quiet_mode))
                  cprintf("\r\n%s already exists on drive\n\r",filen);
               if(list_current == list_end)
               {
                  // we're done and outta here.
                  sprintf(getstring,"QUIT\r\n");
                  send_input(client_socket,getstring);
                  watch(getstring);
                  y = getaline(client_socket,buf);
                  watch(buf);
                  close_exit();                           // then end
               }
               list_current=list_current->next; // move to next node in list
               filen = list_current->text;
            }
         sprintf(getstring,"pasv\r\n");
         send_input(client_socket,getstring);
         y = getaline(client_socket,buf);
         if ( !y  )
            error_out("Error: no response from server");
         watch(buf);
         y = atoi(buf);
         if ( y != 227 )
           error_out("Error: stream mode not available");
         I = strstr(buf,"(");    // this section parses out the ip and socket of the
         I += 1;                 // data connection
         A = atoi(I);
         I = strstr(I,",");      // a.b.c.d is the ip
         I += 1;
         B = atoi(I);
         I = strstr(I,",");      // and e*256 + f is the socket number
         I += 1;
         C = atoi(I);
         I = strstr(I,",");
         I += 1;
         D = atoi(I);
         I = strstr(I,",");
         I += 1;
         E = atoi(I);
         I = strstr(I,",");
         I += 1;
         F = atoi(I);

         ftp_get->Fflag = 1;
         sprintf(ftp_get->host,"%d.%d.%d.%d",A,B,C,D); // the ip address of the data connection
         sprintf(ftp_get->lhost,url.path);
         ftp_get->hostp=(E*256)+F; // the socket number for the data connection
         ftp_get->ls_list=0; // set this so that the new thread doesn't try to build a dir automatically
         sprintf(buf,"Starting data connection with %s on socket %u\n\r",ftp_get->host,ftp_get->hostp);
         log_to_file(buf);
         if (watch_server)
            if(!(quiet_mode))
               cprintf("%s",buf);

         ftp_conTid= _beginthread(ftp_con, 0, STACK_SIZE,  ftp_get); // start a thread to get the data

         // get the file
         sprintf(getstring,"RETR %s\r\n",list_current->text);
         watch(getstring);
         send_input(client_socket,getstring);
         y = getaline(client_socket,buf);
         if ( !y  )
            error_out("Error: no response from server on send");
         watch(buf);
         y = atoi(buf);
         if ( y != 150 )
         {
            ftp_get->Fflag=2;
            if (y != 550) // it's not a dir
               if(!(quiet_mode))
                  cprintf("Error: file %s is missing from server\r\n",list_current->text);
         }
         else
         {
            if(!(quiet_mode))
               cprintf("Getting file %s.\n\r",list_current->text);
            ftp_get->Fflag=0;
            DosWaitThread(&ftp_conTid, DCWW_WAIT);  // wait til file is here
            y = getaline(client_socket,buf);
            watch(buf);
         }
         if(list_current == list_end)
            break;
         list_current=list_current->next; // move to next node in list
      }
   }

   // we're done and outta here.
   sprintf(getstring,"QUIT\r\n");
   send_input(client_socket,getstring);
   watch(getstring);
   y = getaline(client_socket,buf);
   watch(buf);
   if(!(quiet_mode))
      cprintf("\r\nDone\r\n");
   close_exit();                           // then end
}

/***********************************************************/
/*                      FTP_CON                            */
/*                                                         */
/*                                                         */
/*                                                         */
/***********************************************************/

void _Optlink ftp_con(void *args)
{
   struct timeval timeout;
   int rval,wval,selectr,ch=0,walker=0,strlength=0;
   unsigned long length;
   float length_of_time;
   int msgsock,outsock=0,insock,outsocknum,x,y,f=0,d=0;
   struct list *newlist;
   struct hostent *hp;
   struct sockaddr_in server;
   unsigned char ftphost[1024];
   unsigned char buf[1024],tmp[2048],hold[2048],*I;
   long block;
   time_t begin_time, finish_time;
   char *build,*recvbuf;
   FTP_DATA *player = NULL;
   fd_set fdset;
   LIST *list_new = NULL;  // linked list with 1 element called text

   player = args;

   if ((build = malloc(20*(sizeof(char)))) == NULL)
      error_out("Error allocating memory");
   if ((recvbuf = malloc(8096*(sizeof(char)))) == NULL)
      error_out("Error allocating memory");

   sprintf(build,"úùþ+*°±²Û");
   /* Create output socket to server */
   if ((outsock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
   {
      free (build);
      free (recvbuf);
      fclose(out);
      shutdown(outsock,2);
      soclose(outsock);
      error_out("Error: Couldn't open data socket");
   }

   sprintf(ftphost,"%s",player->host);
   outsocknum=player->hostp;
   timeout.tv_sec = 190;
   timeout.tv_usec = 0;
   sprintf(tmp,"connecting to host %s on socket %u\r\n",ftphost,outsocknum);
   watch(tmp);

   /* Connect socket using machine & port specified  */
   server.sin_family = AF_INET;
   server.sin_addr.s_addr = inet_addr(ftphost);
   server.sin_port = (unsigned short) htons((unsigned short)outsocknum);

   if (connect(outsock, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) < 0)
   {
      shutdown(outsock,2);
      soclose(outsock);
      sprintf(tmp,"Could not connect to remote socket %s",ftphost);
      error_out(tmp);
   }

   /* Open the output files */
   while ( player->Fflag == 1 )
   {
      DosSleep(200);
   }

   if (player->Fflag == 2)
   {
      free (build);
      free (recvbuf);
      shutdown(outsock,2);
      soclose(outsock);
      DosExit(0,0);
   }

   /* find out who's talking */
   FD_ZERO(&fdset);
   FD_SET(outsock, &fdset);
   selectr = select(2048, &fdset, 0,0, &timeout);
   if (selectr<1)
   {
      shutdown(outsock,2);
      soclose(outsock);
      free(player);
      error_out("ERROR");
   }
   memset(buf,0,1024);


   if (player->ls_list) // if we're trying to get a list
   {

      if (!(is_glob))
      {
         clear_file();

         sprintf(buf,"Directory of %s%s\n\r",url.hostname,player->lhost);
         if(!(quiet_mode))
            cprintf("%s",buf);
         writef(buf);
      }
      memset(buf,0,1024);

      while ( getaline(outsock, buf) )
      {
         // looking at the first char to determine file, dir, or link
         if ( (buf[0] == '-') || ( buf[0] == 'd') || (buf[0] == 'l') || (buf[0] == ' '))
         {
            if ( buf[0] == '-' )// this is a file
               f++;
            if ( buf[0] == 'd' )// directory
               d++;
            if ( buf[0] == ' ' )// check to see if dir or file
            {
               strlength = strlen(buf);
               //parse for 'dir' & if filelength = 0  then dir
               for(walker=0;walker < (strlength-2);walker++)
               {
                  if((buf[walker] == 'D') && (buf[walker+1] == 'I') && (buf[walker+2] == 'R'))
                  {
                     f--; // we're decrementing file here and adding it back later if it's a dir
                     d++;
                     break;
                  }
                  if((buf[walker] == 'd') && (buf[walker+1] == 'i') && (buf[walker+2] == 'r'))
                  {
                     f--; // we're decrementing file here and adding it back later if it's a dir
                     d++;
                     break;
                  }
               }
            //if there is no dir and filesize is positive then it's a file
            f++; // if this was a dir, we decremented the file counter so it's ok to add it back now.
            }

            I = strstr(buf, "\n" ); // move to the newline char
            while ( *I != ' ' )  // move back to the last space in the line
            {
               if ( (*I == 10) || (*I == 13 ) ) // kill any carriage returns or newlines
                  *I =0;
               I--;
            }

            *I=0;
            I++;
            sprintf(tmp,"%s",I); // this is the filename


            if(is_glob) // if we're globbing check the filename to see if it matches
            {           // if it doesn't, quit, if it does, add it to the list
               if (glob(filen, tmp)) // checks to see if this matches the pattern - if not, break out.  if it does, add it to the list
               {
                  if ((list_new = (LIST *)malloc (sizeof(LIST))) == NULL)  // if we have problems allocating memory, shut down
                  {
                     free (build);
                     free (recvbuf);
                     fclose(out);
                     shutdown(outsock,2);
                     soclose(outsock);
                     error_out("Error allocating memory for dir list");
                  }
                  if ((list_new->text = (char *)malloc(strlen(tmp)+1)) == NULL) // if we have problems allocating memory, shut down
                  {
                     free (build);
                     free (recvbuf);
                     fclose(out);
                     shutdown(outsock,2);
                     soclose(outsock);
                     error_out("Error allocating memory for filename");
                  }
                  strcpy (list_new->text,tmp);
                  if (list_begin == NULL)
                  {
                     list_begin = list_new;
                     list_current = list_new;
                  }
                  else
                     list_current->next = list_new;
                  list_current = list_new;
                  list_end = list_new;
               }
            }

            if (!(is_glob))
            {
               if ( strlen(player->lhost) >1)
                  sprintf(hold,"%s %s/%s\n",buf,player->lhost,tmp);
               else
                  sprintf(hold,"%s /%s\n",buf,tmp);
               if(!(quiet_mode))
                  cprintf("%s\r",hold);
               writef(hold);
            }
         }
         memset(buf,0,1024);

      }
      if (!(is_glob))
      {
         sprintf(buf,"Total Files : %d\n",f);
         if(!(quiet_mode))
            cprintf("%s\r",buf);
         writef(buf);
         sprintf(buf,"Total Directories : %d\n",d);
         if(!(quiet_mode))
            cprintf("%s\r",buf);
         writef(buf);
      }
   }
   else // get a file
   {
      length = 0;
      if (NULL == (out = fopen(filen,"ab")))
      {
         sprintf(buf,"\r\nNon-fatal error: Could not open file %s\r\n",filen);
         log_to_file(buf);
         if(!(quiet_mode))
            cprintf("[1;37;40m%s\r\n",buf);
         filen = tmpnam(NULL);
         sprintf(buf,"Saving file as %s instead\r\n",filen);
         log_to_file(buf);
         if(!(quiet_mode))
            cprintf("%s\r\n",buf);
         out = fopen(filen,"ab");

      }
      if(!(quiet_mode))
         cprintf("[1;32;40m%c",build[0]);
      time(&begin_time);
      do
      {
         if ((rval = recv(outsock, recvbuf, 4096,0)) < 0)
         {
            shutdown(outsock,2);
            soclose(outsock);
            free(player);
            break;
         }
         if (rval == 0)
         {
            time(&finish_time);
            length_of_time=difftime(finish_time,begin_time);
            if((!(quiet_mode)) && (!(is_glob)))
            {
               cprintf("[1;37;40m\r\nEnding server connection\n\r");
               cprintf("%u bytes received in %1.0f seconds\n\r",length,length_of_time);
               cprintf("For an average rate of %1.1f cps\n\r\n\r",length/(length_of_time+.001));
            }
            if((!(quiet_mode)) && (is_glob))
               cprintf("[1;37;40m\r\n");
            break;
         }
         length = length + rval;
         time(&wait_time);
         if (fwrite(recvbuf,sizeof(char),rval,out)==0) /* writes to the file */
         {
            fclose(out);
            sprintf(buf,"\r\nNon-fatal Error: Could not write to file %s\n\r",filen);
            free (build);
            free (recvbuf);
            fclose(out);
            shutdown(outsock,2);
            soclose(outsock);
            if (!(is_glob))
               error_out(buf);
            else if(!(quiet_mode))
               cprintf("[1;37;40m%s\r\n",buf);
            return;
         }
         time(&wait_time);
         if(!(quiet_mode))
            cprintf("%c%c",8,build[ch]);
         ch++;
         if ( ch == 10)
         {
            ch =0;
            if(!(quiet_mode))
               cprintf(" ");
         }
         block++;
      }
      while (rval > 0);
   }

   /* Close up shop */

   free (build);
   free (recvbuf);
   fclose(out);
   shutdown(outsock,2);
   soclose(outsock);
}


/***********************************************************/
/*                Clear File                               */
/*                                                         */
/*                                                         */
/*                                                         */
/***********************************************************/
void clear_file(void)
{
   if ((!(append_to_file)) || (overwrite_file))
   {
      if (0 == access(filen,00))// if file already exists
      {
         if (!(0 == remove(filen)))// if file already exists
            if(!(quiet_mode))
               cprintf("Could not remove file %s.",filen);
      }
   }
}


/***********************************************************/
/*                Log to file                              */
/*                                                         */
/*                                                         */
/*                                                         */
/***********************************************************/
void log_to_file(char *line_to_log)
{
   FILE *logout;

   if (!(keep_a_log))
      return;

   logout = fopen(logfile,"a");
   if (fwrite(line_to_log,strlen(line_to_log),1,logout)==0) /* writes the subject to the file */
   {
      fclose(logout);
      if(!(quiet_mode))
         cprintf("\r\nNon-Fatal error: Could not write to log file\r\n");
   }
   fclose(logout);

   return;
}

/***********************************************************/
/*                UUdecode                                 */
/*                                                         */
/*                                                         */
/*                                                         */
/***********************************************************/
int uudecode(void)
{


   return(1);
}

/***********************************************************/
/*                HANDLE_TELNET                            */
/*                                                         */
/*                                                         */
/*                                                         */
/***********************************************************/
void handle_telnet(void)
{




}

/***********************************************************/
/*                CHECK_OPTIONS                            */
/*                                                         */
/*                                                         */
/*                                                         */
/***********************************************************/
void check_options(void)
{
   if ((parse_html) && (strip_html))
      error_out("Error: options -p & -s are incompatible");
   if (timeout < 0)
      error_out("Error: Timeout value too low");
   if ((append_to_file) && (overwrite_file))            // default yes
      error_out("Error: options -a & -o are incompatible");



   return;
}

/***********************************************************/
/*                REVERSE_SLASHES                          */
/*                                                         */
/*                                                         */
/*                                                         */
/***********************************************************/
void reverse_slashes(char **pathstring)
{
   char temp_char;
   char *slashbuf;
   char *slashbuf2;

   if ((slashbuf = malloc(((strlen(*pathstring))+1) * (sizeof(char)))) == NULL)
      error_out("Error allocating memory");
   if ((slashbuf2= malloc(3 * (sizeof(char)))) == NULL)
      error_out("Error allocating memory");

   sprintf(slashbuf,"");
   sprintf(slashbuf2,"");

   log_to_file(*pathstring);
   log_to_file("\r\ndid not work, changed path to \r\n");

   if(strlen(*pathstring) == 0)
      return;

   for(;;)
   {
      temp_char = **pathstring;

      if(temp_char == '\\')
         temp_char = '/';
      else if(temp_char == '/')
         temp_char = '\\';
      sprintf(slashbuf2,"%c",temp_char);
      strcat(slashbuf,slashbuf2);
      *(*pathstring)++;
      if (**pathstring == '\0')
         break;
   }
   strcpy(*pathstring,slashbuf);
   log_to_file(*pathstring);
   log_to_file("\r\n");

   free(slashbuf);
   return;
}

/*************************************************************************************************/
/* Function: GET SITE                                                                            */
/* Usage:  Sets up a linked list, parses through a html file, sent a block at a time pulls out   */
/*         any links to images pages, files, etc and saves them in the linked list.              */
/*                                                                                               */
/* Inputs: a block of text from a html file                                                      */
/* Outputs: 0 if successful, 1 if there was a problem and we need to shut down                   */
/*          saves output in global vars                                                          */
/* Global Vars: list_begin, list_end, and list_current                                           */
/*************************************************************************************************/
int get_site(char *html, int len)
{
   static int counter=-1;
   static char *url_buf;
   static char *tag_buf;
   static char *path;


   if (counter == -1)
   {
      if((url_buf = (char *) malloc (2048 * (sizeof(char)))) == NULL)
         return(1);
      if((tag_buf = (char *) malloc (2048 * (sizeof(char)))) == NULL)
         return(1);
      if ((path = (char *) malloc (512 * (sizeof(char)))) == NULL)
         return(1);
   }
   counter = 0;

   _splitpath(url.path,url_buf,tag_buf,NULL,NULL); // just temp uses of the two buffers instead of initing 2 more vars
   sprintf(path,"http://%s%s%s",url.hostname,url_buf,tag_buf);  // get the beginning of the url

   strcpy(url_buf,"");
   strcpy(tag_buf,"");


   for(;;)
   {
      if(counter > len ) // if we hit the end of the string
         return(0);

      strcpy(tag_buf,(parse_out_tags(html[counter])));

      if (strlen(tag_buf) < 1)
      {
         counter++;
         continue;
      }
      else
      {
         strcpy(url_buf, check_link(tag_buf));
         if (strlen(url_buf) > 1)
         {
            strcpy(tag_buf,(add_begin_url(path,url_buf)));
            if (add_to_list(tag_buf) == 0) // adds the url to the list
               error_out("Error: could not allocate memory");
         }
         strcpy(url_buf,"");
         strcpy(tag_buf,"");
         counter++;
      }
   }
   return (0);
}

/*************************************************************************************************/
/* Function:ADD_TO_LIST                                                                          */
/* Usage:  receives a string and adds it to the linked list                                      */
/*                                                                                               */
/*                                                                                               */
/* Inputs: a string to add to the linked list                                                    */
/* Outputs: 1 if successful 0 if failed                                                          */
/* Global Vars: list_begin, list_current, list_end                                               */
/*************************************************************************************************/
int add_to_list(char *addme)
{
   LIST *list_new;  // linked list with 1 element called text

   if ((list_new = (LIST *)malloc (sizeof(LIST))) == NULL)  // if we have problems allocating memory, shut down
      return(0);
   if ((list_new->text = (char *)malloc(strlen(addme)+1)) == NULL) // if we have problems allocating memory, shut down
      return(0);

   strcpy(list_new->text,addme);
   if (list_begin == NULL)
   {
      list_begin = list_new;
      list_current = list_new;
   }
   else
      list_current->next = list_new;
   list_current = list_new;
   list_end = list_new;
   return (1);
}

/*************************************************************************************************/
/* Function: PRINT_LIST                                                                          */
/* Usage: prints out all the members of the linked list                                          */
/*                                                                                               */
/*                                                                                               */
/* Inputs: NONE                                                                                  */
/* Outputs: NONE                                                                                 */
/* Global Vars: list_begin, list_current                                                         */
/*************************************************************************************************/
void print_list(void)
{
   char *text_buf;

   list_current = list_begin;

   for(;;)
   {
      text_buf = get_next_from_list();
      if (text_buf == NULL)
         return;
      else
         cprintf("%s\r\n",text_buf);
   }
}

/*************************************************************************************************/
/* Function: GET_FROM_LIST                                                                       */
/* Usage: returns the current text from out linked list and advances the current pointer to      */
/*        the next node in the list                                                              */
/*                                                                                               */
/* Inputs: NONE                                                                                  */
/* Outputs: a character pointer pointing to the text in out current node of the linked list      */
/*          or NULL if we hit the end pointer                                                    */
/* Global Vars: list_begin,list_current, list_end                                                */
/*************************************************************************************************/
char *get_next_from_list (void)
{
   char *text_buf;

   if (list_current == NULL)          // if we're done, or if there's a problem, return ""
      return(NULL);

   text_buf =  list_current->text;

   if (list_current == list_end)
      list_current = NULL;               // if we're at the end, set list_current to null
   else
      list_current = list_current->next; // move list_current to next node in list

   return(text_buf);

}

/*************************************************************************************************/
/* Function: SIG_HANDLER                                                                         */
/* Usage: pointed to by the signal handlers  -  exits gracefully instead of just breaking        */
/*                                                                                               */
/*                                                                                               */
/* Inputs: int required by the signal handler                                                    */
/* Outputs: none                                                                                 */
/* Global Vars: none                                                                             */
/*************************************************************************************************/
void sig_handler(int sig)
{
   error_out("Break sensed.");
}


/*************************************************************************************************/
/* Function: FIND_LIST_DUPES                                                                     */
/* Usage: looks for duplicates in the linked list and deletes them                               */
/*                                                                                               */
/*                                                                                               */
/* Inputs:                                                                                       */
/* Outputs:                                                                                      */
/* Global Vars:  list_begin,list_current, list_end                                               */
/*************************************************************************************************/
void find_list_dupes (void)
{









}

/*************************************************************************************************/
/* Function: DELETE_NODE_FROM_LIST                                                               */
/* Usage:                                                                                        */
/*                                                                                               */
/*                                                                                               */
/* Inputs:                                                                                       */
/* Outputs:                                                                                      */
/* Global Vars: list_current, list_end                                                           */
/*************************************************************************************************/
void delete_node_from_list (void)
{







}

/*************************************************************************************************/
/* Function:                                                                                     */
/* Usage:                                                                                        */
/*                                                                                               */
/*                                                                                               */
/* Inputs:                                                                                       */
/* Outputs:                                                                                      */
/* Global Vars:                                                                                  */
/*************************************************************************************************/
char *translate_path(char *path)
{
   char *new_path;
   char current[2];

// %3 :
// %20 (space)





}









/*************************************************************************************************/
/* Function:                                                                                     */
/* Usage:                                                                                        */
/*                                                                                               */
/*                                                                                               */
/* Inputs:                                                                                       */
/* Outputs:                                                                                      */
/* Global Vars:                                                                                  */
/*************************************************************************************************/

