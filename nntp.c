#define INCL_NOPMAPI
#define INCL_VIO
#define INCL_DOSPROCESS
#define INCL_DOS
#include <sys\types.h>
#include <netinet\in.h>
#include <sys\socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <os2.h>
#include <ctype.h>
#include <string.h>
#include <process.h>
#include "netgrab.h"
#include "urlparse.h"
#include "sockets.h"
#include <time.h>

/* Global variables */
extern int client_socket;                 // The current socket #
extern struct sockaddr_in saddr;          //the address that we're connecting to.
extern unsigned char *filen;              // used in main and fileout
extern FILE *out;                         // used in main and fileout
extern TID thread3Tid;                  // this thread is used for writing articles
extern URLData_t url;
extern int y;                             // a buffer integer
extern char *buf;                // a buffer variable - used all over
extern char file_write[1024000]; // a variable to put the article into before writing to disk
extern char article[1024000];    // large string to put article into
extern time_t wait_time;                  // this is our variable to check against timing out

//options
extern int get_news_head;   // default yes
extern int overwrite_file;  // default yes
extern int watch_server;    // default no
extern int get_head_only;   // default is no
extern int quiet_mode;

/***********************************************************/
/*                   write_file                            */
/* called from _beginthread command, writes a file from    */
/* global variables                                        */
/* Inputs: filename, file_write                            */
/* outputs: none unless fails to open file                 */
/***********************************************************/

void write_file (void *fee) /* actually writes the file */
{
   /* access file to write */
   out = fopen(filen,"a");
   time(&wait_time);
   if (fwrite(file_write,strlen(file_write),1,out)==0) /* writes the subject to the file */
   {
      fclose(out);
      error_out("Error: Could not write to file");
   }
   time(&wait_time);
   fclose(out);
}

/***********************************************************/
/*                      HANDLE_NEWSLIST                    */
/* gets the list of available groups from the server and   */
/* saves them to a specified file                          */
/*                                                         */
/*                                                         */
/*                                                         */
/***********************************************************/

void handle_newslist(void) /* gets the list of available newsgroups */
{
   unsigned char ok=1;
   unsigned long length=0;
   NODE *root = NULL;
   FILE *writeout;

   y = getaline(client_socket,buf);
   if ( !y  ) error_out("Error: retrieving header from server");
   y = atoi(buf);
   if ( y != 200 ) error_out("Error: Didn\'t get a server response");
   sprintf(buf,"list\r\n");
   send_input(client_socket,buf);
   y = getaline(client_socket,buf);
   if ( !y  ) error_out("Error: Getting list ack");
   y = atoi(buf);
   if ( y != 215 ) error_out("Error: Couldn\'t get news list");

   if (strstr(filen,"NoFIleNaME") != NULL )
   {
      strcpy(filen,"Newslist.txt");
   }
  if (overwrite_file)
   {
      out = fopen(filen,"w");
      fclose(out);
   }

   while(ok)
   {
      length++;
      if(!(quiet_mode))
         cprintf("Group: %ld\r",length);
      y = getaline(client_socket,buf);
      if ( (buf[0] == '.') && (strlen(buf) < 4) )
         ok = 0;
      else
         root = add_node(buf);
   }
   if(!(quiet_mode))
      cprintf("Saving list to %s\r\n",filen);
   out = fopen(filen,"a");
   write_sorted_tree(root);
   fclose(out);
   return;
}

/***********************************************************/
/*                   listgroup                             */
/*                                                         */
/*                                                         */
/*                                                         */
/***********************************************************/
int listgroup (int reset_startx)
{
   int current,counter=0,first=1,ok=1;


   sprintf(buf,"listgroup\r\n");  // try listing the groups
   watch(buf);
   send_input(client_socket,buf);
   y = getaline(client_socket,buf);
   watch(buf);
   if ( !y  )
      error_out("Error: Problem getting beginning article");
   y = atoi(buf);
   if ( y == 211 ) // get the next article out of the list
   {
      if(!(quiet_mode))
         cprintf("\r\nGetting list of newsgroups. \r\n");
      while(ok) // run this loop until we get to the end of the list
      {
         y = getaline(client_socket,buf); /* this inputs our line */
         if ( (buf[0] == '.') && (strlen(buf) < 4) ) // a single . indicates the end.
            ok = 0; // so we set our flag.
         else // if we haven't gotten to the end, continue to compare.
         {
            if ((current = atoi(buf) > url.startx) && (first) && (reset_startx))// when we hit our first number greater than startx, this sets startx to it.
            {
               reset_startx=0;
               first=0;
               url.startx = atoi(buf);
            }
            if ((reset_startx == 0) && (current < url.endx)) //  if startx is set, here or in the other code, just look for remaining
               counter++;
         }
      }
      sprintf(buf,"stat %u\r\n",url.startx);  // change to the beginning article
      watch(buf);
      send_input(client_socket,buf);
      y = getaline(client_socket,buf);
      if ( !y  )
         error_out("Error: Problem getting beginning article");
      y = atoi(buf);
      if ( y == 423 )
      {
         error_out("Could not get first article");
      }
      return (counter);
   }
   else // if listx didn't work
      return (-1);
}


/***********************************************************/
/*                   handle_nntp                           */
/*                                                         */
/*                                                         */
/*                                                         */
/***********************************************************/
void handle_nntp(void) /* actually does the work */
{
   unsigned int ky,ok=1;
   long int index, remain,first_article,last_article;
   int good_article,try_listgroup=1;
   unsigned char *divider;
   unsigned char *buf1, *buf2,*buf3;


   buf1   = ( unsigned char *) malloc(2048);
   buf2 = ( unsigned char *) malloc(2048);
   buf3 = ( unsigned char *) malloc(2048);
   divider  = ( unsigned char *) malloc(100);
   sprintf(divider,"\r\n================================================================================\r\n");

   if ((url.endx != 0) && (url.startx > url.endx))
      error_out("Error: First article must be lower than last article number");

   if (strstr(filen,"NoFIleNaME") != NULL )
   {
      strcpy(filen,"News.txt");
      if(!(quiet_mode))
         cprintf("Saving news to News.txt\r\n");
   }
   if (overwrite_file)
   {
      out = fopen(filen,"w");
      fclose(out);
   }

   /* this section begins the connection */
   y = getaline(client_socket,buf);
   if ( !y  )
      error_out("Error: Problem retrieving header from server");
   watch(buf);
   y = atoi(buf);
   if ( y != 200 )
     error_out("Error: Didn't get a server response");
   sprintf(buf,"group %s\r\n",url.path);
   watch(buf);
   send_input(client_socket,buf);
   y = getaline(client_socket,buf);
   if ( !y  )
   {
      sprintf(buf,"Error: Couldn't get ack for %s",url.path);
      error_out(buf);
   }
   watch(buf);
   y = atoi(buf);
   while(isdigit(*buf)) buf++;
   buf++;
   remain = atol(buf);
   if (remain == 0)
      error_out ("No articles in group");
   while(isdigit(*buf)) buf++;
   buf++;
   first_article = atol(buf);
   while(isdigit(*buf)) buf++;
   buf++;
   last_article = atol(buf);
   if (url.endx == 0)
      url.endx = last_article;


   if ( y != 211 )
   {
      sprintf(buf,"Error: Couldn't get %s",url.path);
      error_out(buf);
   }

   if (url.startx != 0)
   {
      // if article is too low
      if (url.startx < first_article)
      {
         if(!(quiet_mode))
         {
            cprintf("\a\r\n[1;33;40mArticle #%u is lower than the first article avalible, #%u\r\n",url.startx,first_article);
            cprintf("Resetting to first article[0;37;40m\r\n\r\n");
         }
         url.startx = first_article;
      }

      // if article is too high
      if (url.startx > last_article)
      {
         sprintf(buf,"\r\nArticle #%u is higher than the last article avalible, #%u\r\n",url.startx,last_article);
         error_out(buf);
      }

      // if article number is *JUST RIGHT*
      if ((url.startx >= first_article) && (url.startx < last_article))
      {
         do
         {
            sprintf(buf,"stat %u\r\n",url.startx);  // change to the beginning article
            watch(buf);
            send_input(client_socket,buf);
            y = getaline(client_socket,buf);
            if ( !y  )
               error_out("Error: Problem getting beginning article");
            watch(buf);
            y = atoi(buf);
            //try to figure this out, i dare you
            remain = ((last_article - url.startx)-((last_article-first_article-remain)*((url.startx-first_article)/(last_article-first_article))));
            if (( y == 423 ) && (remain < 1))
            {
               error_out("Last article in newsgroup");
            }

            // if the beginning article isn't there
            if ( y == 423 )
            {
               if(!(quiet_mode))
                  cprintf("Article %u not on server.  Going to next article \r\n",url.startx);

               if (try_listgroup)
               {
                  remain == listgroup(1);// set listgroup to reset startx
                  if (remain == -1)
                  {
                     try_listgroup=0;
                     if(!(quiet_mode))
                        cprintf("Listgroup not supported, finding next article manually\r\n");
                     url.startx++; //increase the beginning article number by 1
                  }
               }
               else
                  url.startx++; //increase the beginning article number by 1
            }
         }
         while (y==423);
      }
   }
   else // if url.startx == 0
      url.startx = first_article;

   if (try_listgroup)
   {
      printf("First Article = %ld\r\n",url.startx);
      printf("Last Article = %ld\r\n",url.endx);
      printf("Total Articles = %ld\r\n",remain);
      printf("Missing Articles = %ld\r\n\r\n",(url.endx-url.startx-remain+1));
   }
   else
   {
      printf("First Article = %ld\r\n",url.startx);
      printf("Last Article = %ld\r\n",last_article);
      printf("Aproximate Total Articles = %ld\r\n",remain);
      printf("Aproximate Missing Articles = %ld\r\n\r\n",(last_article-url.startx-remain+1));
   }

   //*****************************************************************
   //*****    connected to the group  - start getting groups     *****
   //*****************************************************************
   for (;;)
   {
      strcpy(article,""); // resets article to empty
      good_article=getnews("xhdr subject\r\n", remain); /* get the subject */
      if (good_article==1)
      {
         if (get_news_head)
            getnews("head\r\n",remain); // gets entire header and attaches it to the article
         if (get_head_only == 0)
            getnews("body\r\n", remain); //gets body and attaches it to the article
         if ((get_head_only == 0) || (get_news_head))
            strcat(article,divider); /* writes the divider to the end of the article */

         DosWaitThread(&thread3Tid, DCWW_WAIT);  /* make sure it was done writing */
         strcpy(file_write, article);

         thread3Tid=_beginthread(  /* begin a new thread to write the file */
            write_file,
            NULL,
            8192,
            NULL);
      }

      sprintf(buf,"next\r\n");
      watch(buf);
      send_input(client_socket,buf);
      y = getaline(client_socket,buf);
      watch(buf);
      y = atoi(buf);
      if ((y != 223) || (remain == 0))
      {
         if(!(quiet_mode))
            cprintf("\r\nLast article in newsgroup.\r\n");
         return;
      }
      remain--;
   }
}



/***********************************************************/
/*                     GETNEWS                             */
/* handles the news for handle_nntp                        */
/*                                                         */
/*                                                         */
/***********************************************************/
int getnews (char *action, int remain)
{
   int ok=1,counter;
   static unsigned long article_num=0, sum=0, total_lines=0,length=0;
   static int first_time=1;
   float length_of_time;
   int good_article;
   char *lookfor;
   char *line_in;
   static time_t begin_time, finish_time;

   lookfor = (char *) malloc (128*(sizeof(char)));
   line_in = (char *) malloc (4096*(sizeof(char)));

   watch(action);
   send_input(client_socket,action);
   y = getaline(client_socket,line_in); /* trash line */
   watch(line_in);
   good_article = 1;

   if (first_time)
   {
      time(&begin_time);
      first_time=0;
   }

   while(ok)
   {
      y = getaline(client_socket,line_in); /* this inputs our line */
      time(&wait_time);
      if ( (line_in[0] == '.') && (strlen(line_in) < 4) ) // a single . indicates the end.
         ok = 0;
      else
      {
         // handle subject line
         if (strstr(action,"subject\r\n") != NULL)
         {
            // extract the article number
            sum=0;
            length=(length + strlen(line_in));
            article_num = atol(line_in);

            if (article_num == 0) // if we get a bad article
            {
               good_article = 0;
               break;
            }
            // if we are looking for something, lets do it.
            if (url.looknum != 0)
            {
               // lets look for the negative things first
               for (counter=0;counter < url.looknum;counter++)
               {
                  lookfor = url.lookfor[counter];
                  // if the first character's a - remove it
                  if (lookfor[0] == '-')
                  {
                     lookfor++;
                     // parse our variable
                     if (strstr((makeupper(&line_in)),lookfor)!=NULL)
                     {
                        // if we found a matching word, it's a bad article
                        // we can stop looking
                        good_article=-1;
                        break;
                     }
                  }
               }
               // if we didn't find a bad article, look for good articles
               if (good_article == 1)
               {
                  // lets look for the positive things next
                  for (counter=0;counter < url.looknum;counter++)
                  {
                     lookfor = url.lookfor[counter];
                     // if the first character's a + remove it
                     if (lookfor[0] != '-')
                     {
                        if (lookfor[0] == '+')
                        {
                           lookfor++;
                        }
                        // then parse the subject
                        if (strstr((makeupper(&line_in)),lookfor)!=NULL)
                        {
                           // if true, this is a good article, we can stop looking
                           good_article=1;
                           break;
                        }
                        else
                        {
                           // if false, we need to keep looking
                           good_article=0;
                           sum = 0;
                        }
                     }
                  }
               }
            }
            if(!(quiet_mode))
               cprintf("[K");  // go to beginning of line
            if (good_article == 1)
            {
               if(!(quiet_mode))
                  cprintf("[1;37;40m");// if it's a good article, print it in bright white
               strcat(article,line_in); // writes the subject to article
            }
            if (good_article == -1)
               if(!(quiet_mode))
                  cprintf("[1;36;40m");// if it matches a reject code, print it in light cyan
            if (good_article == 0)
              if(!(quiet_mode))
                  cprintf("[0;36;40m");// if it doesn't match a reject or good code, print it in dark cyan
            cprintf("%s",line_in); // print the subject line
         }
         else // if it's not our subject line
         {
            length=(length + strlen(line_in));
            strcat(article,line_in); // writes the body to article
         }
      }

      if ( !(sum % 7) && (!(strcmp(action,"body\r\n"))))
      {
         time(&finish_time);
         length_of_time=difftime(finish_time,begin_time);
         if(!(quiet_mode))
            cprintf("[1;33;40mRemain: %ld  Article: %ld  Total lines: %ld  CPS: %1.1f Line: %ld[1;37;40m\r",remain-1, article_num,total_lines,length/(length_of_time+.001),sum);
      }
      sum++;
      total_lines++;
   }
   time(&finish_time);
   length_of_time=difftime(finish_time,begin_time);
   if(!(quiet_mode))
      cprintf("[1;33;40mRemain: %ld  Article: %ld  Total lines: %ld  CPS: %1.1f Line: %ld[1;37;40m\r",remain-1, article_num,total_lines,length/(length_of_time+.001),sum);
   watch("[0;37;40m\r\n"); // if we're watching the server we need a crlf
   return good_article;
}

NODE *add_node (char *text)
{
   int c;
   static NODE *root = NULL;
   NODE *newnode = NULL;

   newnode = allocate(text);
   root = add_to_tree(root, newnode);

   return(root);
}

NODE *allocate(char *text)
{
   NODE *newnode;

   if ((newnode = (NODE *)malloc(sizeof(NODE))) == NULL)
      error_out("Error allocating memory");
   if ((newnode->text = (char *)malloc(strlen(text)+1)) == NULL)
      error_out("Error allocating memory");

   strcpy(newnode->text,text);
   return(newnode);
}

NODE *add_to_tree(NODE *r, NODE *newnode)
{
   if (!r)
   {
      r=newnode;
      r->left = r->right = NULL;
      return (r);
   }
   else
   {
      if (strcmp(newnode->text, r->text ) <= 0)
         r->left = add_to_tree(r->left,newnode);
      else
         r->right = add_to_tree(r->right,newnode);
   }
   return(r);
}

void write_sorted_tree(NODE *r)
{
   if (r)
   {
      write_sorted_tree(r->left);
      fwrite(r->text,strlen(r->text),1,out);
      watch(r->text);
      write_sorted_tree(r->right);
   }
}

