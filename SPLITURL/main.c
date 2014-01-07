#include <stdlib.h>
#include <stdio.h>
#include <conio.h> // cprintf
#include <string.h>
#include "main.h"
#include "spliturl.h"
#include "btree.h"

/*************************************************************************************************/
/*  Function:  MAIN                                                                              */
/*  Usage: reads in command line arguments for the names, if they exist, and calls the function  */
/*         that splits up the html tags                                                          */
/*                                                                                               */
/*                                                                                               */
/*  inputs: input filename, and output filename if user chooses                                  */
/*  outputs: 1 if successful                                                                     */
/*************************************************************************************************/
int main (int argc, char** argv)
{
   char *in_file;
   char *out_file;
   int number_tags;
   char *begin_url;

   in_file = (char *) malloc (128 * (sizeof(char)));
   out_file = (char *) malloc (128 * (sizeof(char)));
   begin_url = (char *) malloc (1024 * (sizeof(char)));


   if ((argc < 2) || (argc > 4)  )
      usage();

   sprintf(in_file,"%s",argv[1]);

   if (argc < 3)
      strcpy(begin_url,"");
   else
   {
      sprintf(begin_url,"%s",argv[2]);
      *(strrchr(begin_url,'/')+1) = '\0';
   }
   if (argc == 4)
      sprintf(out_file,"%s",argv[3]);
   else
      strcpy(out_file,"geturls.cmd");

   bannr();
   cprintf("\r\nProcessing: %s\r\n",in_file);
   cprintf("Beginning url is: %s\r\n",begin_url);
   cprintf("Saving to: %s\r\n\r\n",out_file);


   number_tags = load_file(in_file,out_file,begin_url);

   if (number_tags < 1)
   {
      cprintf("\r\n\r\nNo URLs found in file[0;37;40m\r\n");
      return(0);
   }
   if (number_tags > 1)
      cprintf("\r\n\r\nSuccess - Found %u URLs[0;37;40m\r\n",number_tags);
   else if (number_tags == 1)
      cprintf("\r\n\r\nSuccess - Found 1 URL[0;37;40m\r\n");
   return(1);
}

/*************************************************************************************************/
/* Function:  USAGE                                                                              */
/* Usage:  yes.  shows the instructions for this program then ends                               */
/*                                                                                               */
/*                                                                                               */
/* Inputs:  NOPE                                                                                 */
/* Outputs:  NONE                                                                                */
/* Global Vars: NOT EVEN.                                                                        */
/*************************************************************************************************/
void usage (void)
{
   error_out("Usage: spliturl.exe [input filename] (reference_url) (output filename)\r\n");
}

/*************************************************************************************************/
/* Function: LOAD_FILE                                                                           */
/* Usage:                                                                                        */
/*                    GONNA GET SCRAPPED                                                         */
/*                                                                                               */
/* Inputs:                                                                                       */
/* Outputs:                                                                                      */
/* Global Vars                                                                                   */
/*************************************************************************************************/
int load_file (char* file_in, char *file_out, char *begin_url)
{
   char readchar;
   char *write_buf;
   char *tag_buf;
   char *url_buf;
   char *path_buf;

   FILE *in,*out;
   int num_tags = 0;
   int path_tags = 0;
   int dupe_tags = 0;
   NODE *root = NULL;

   write_buf = (char *) malloc (4096 * (sizeof(char)));
   tag_buf = (char *) malloc (4096 * (sizeof(char)));
   url_buf = (char *) malloc (4096 * (sizeof(char)));
   path_buf = (char *) malloc (256 * (sizeof(char)));

   strcpy(write_buf,""); // clear our buffers
   strcpy(tag_buf,"");

   in = fopen(file_in,"r");   // open the file
   if (in == NULL)
      error_out("Error: Could not open input file");

   out = fopen(file_out,"w");
   if (out == NULL)
      error_out("Error: Could not open output file");

   // read in the file
   for (;;)
   {
      cprintf("Processing URL # %u\r",num_tags);
      readchar = fgetc(in);   // read in a char
      if (feof(in) != 0)      // if we're at the end of file, exit the loop
         break;

      else
      {
         strcpy(tag_buf,parse_out_tags(readchar));
         if (strlen(tag_buf) < 2 )
            continue;
         else
         {
            strcpy (url_buf, check_link(tag_buf));
            if(strlen (url_buf) > 2)
            {
               sprintf(write_buf,"netgrab %s %s\n",add_begin_url(begin_url,url_buf),url_buf);
               if (strlen(write_buf) > (11 +strlen(begin_url))) // check to make sure something's in the string
               {
                  num_tags++;
                  _splitpath(url_buf,NULL,path_buf,NULL,NULL);
                  sprintf(url_buf,"md %s\n",path_buf);
                  if (strlen(url_buf) > 4)
                  {
                     root = add_node(url_buf);
                     path_tags++;
                  }
                  root = add_node(write_buf);
               }
            }
            sprintf(tag_buf,"");                                   // clear our buffer again
         }
      }
   }

   dupe_tags = write_sorted_tree(root,out);                       //write out all our netgrab commands
   dupe_tags -= path_tags;

   fclose (in);
   fclose(out);

   if (dupe_tags > 1)
      cprintf("\r\nDeleted %u duplicate URLs",dupe_tags);
   else if (dupe_tags == 1)
      cprintf("\r\nDeleted 1 duplicate URL");

   return(num_tags-dupe_tags);
}

/*************************************************************************************************/
/* Function:                                                                                     */
/* Usage:                                                                                        */
/*                                                                                               */
/*                                                                                               */
/* Inputs:                                                                                       */
/* Outputs:                                                                                      */
/* Global Vars:                                                                                   */
/*************************************************************************************************/
void error_out(char *erro) /* if there was an error, prints what the error was and exits */
{
   cprintf("\r\n[1;37;40m");
   cprintf("%s",erro);
   cprintf("[0;37;40m\r\n");
   exit(0);
}

/***********************************************************/
/*                         bannr                           */
/* prints a title and author banner with a bunch of ansi   */
/* inputs: none                                            */
/* outputs: none                                           */
/***********************************************************/

void bannr(void) /* prints a title and author banner with a bunch of ansi crap */
{
   cprintf("\r\n[1;33;40mSplitUrl [1;37;40m(c)1997[1;31;40m");
   cprintf(" Martin L. Roth[0;37;40m v1.0\r\n");
   cprintf("[1;37;40m         Address all questions and comments to mroth@mailcity.com\r\n\r\n");
}


