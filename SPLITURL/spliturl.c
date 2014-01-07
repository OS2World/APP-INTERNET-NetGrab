#include <stdlib.h>
#include <stdio.h>
#include <conio.h> // cprintf
#include <string.h>
#include "main.h"
#include "spliturl.h"

/*************************************************************************************************/
/* Function: PARSE_OUT_TAGS                                                                      */
/* Usage: gets a char at a time, and returns a full html tag when it has gotten it all           */
/*                                                                                               */
/*                                                                                               */
/* Inputs: 1 char from a html file                                                               */
/* Outputs: NULL if it's inside a html tag but hasn't gotten a full html tag,                    */
/*          returns the character if it's outside a html tag, or the full html tag when done     */
/*          also, does not return the opening or closing html brackets                           */
/* Global Vars: NONE                                                                             */
/*************************************************************************************************/
char *parse_out_tags(char readchar)
{
   static char *tagbuffer;
   static int tagflag = -1; // set for the first time through
   static char charbuf[2];

   if (tagflag == -1) // so we only malloc the first time through.
   {
      if ((tagbuffer = (char *) malloc (2045 * (sizeof(char)))) == NULL)
         error_out("Error allocating memory"); // 2k is a HUGE html tag
      sprintf(tagbuffer,"");
      tagflag = 0;                          // set to actual start value
      charbuf[1] = '\0';
   }

   switch (readchar)
   {
      case '<':
      {
         if (tagflag != 1) // look for the beginning of a tag
         {
            tagflag = 1;                          // we're inside a tag!!!
            sprintf(tagbuffer,"");                // good time to clear our tag buffer
            return("");                         // get the next char
         }
         break;
      }

      case '\r':
      case '\n':
      {
         if(tagflag == 1) // get rid of all the damned carriage returns and linefeeds inside the tags
            return("");
         break;
      }

      case '>':
      {
         if (tagflag == 1) // look for the end of a tag
            tagflag = 2;                          // now we're outside a tag
         break;
      }
   }


   switch (tagflag)
   {
      case 0:  // outside a tag
      {
         charbuf[0] = readchar;
         return(charbuf);
         break;
      }

      case 1: // inside a tag
      {
         if (strlen(tagbuffer) > 2040)        // make sure the tag isn't just WAY too long
         {
            strcpy(tagbuffer,""); // reset our tagbuffer - we're not gonna get this one
            tagflag = 3;
            return("");  // go on to next char
         }
         charbuf[0] = readchar;
         strcat(tagbuffer, charbuf); // add this char to our tag
         return("");  // go on to next char
         break;
      }

      case 2: // done with a tag
      {
         tagflag = 0;
         return(tagbuffer);
         break;
      }
   }
   return("");
}

/*************************************************************************************************/
/* Function: CHECK_LINK                                                                          */
/* Usage: Decides if a link is a image or link or if it's just a comment or crap                 */
/*                                                                                               */
/*                                                                                               */
/* Inputs: a single html tag                                                                     */
/* Outputs:  a url to an image or something on the web                                           */
/* Global Vars: NONE                                                                             */
/*************************************************************************************************/
char *check_link(char *html_tag)
{
   static int flag = 1;
   static char *link;
   static char *lower_tag;

   if (flag == 1)
   {
      if ((lower_tag = (char *) malloc (2048 * (sizeof(char)))) == NULL)
         error_out("Error allocating memory");
   flag = 0;
   }

   strcpy(lower_tag,html_tag);
   strlwr(lower_tag);

   while (lower_tag[0] == ' ') // get rid of any spaces at the beginning of the tag
   {
      if(lower_tag[0] == '\0')
         return("");
      lower_tag++;
   }

   if ((lower_tag[0] == '/') || (lower_tag[0] == '!')  || (strlen(lower_tag) < 7))  // get rid of end tags, comments, and short tags
      return("");

   if (lower_tag[0] == 'a') // hrefs are the most common, check them first
   {
      if(strstr(lower_tag,"href") != NULL)
      {
         link = pull_out_url(html_tag,"href");
         return(link);
      }
   }

   if (strnicmp(lower_tag,"img",3) == 0)  // images are next most common, check them next
   {
      link = pull_out_url(html_tag,"src");
      return(link);
   }

   if (strnicmp(lower_tag,"frame",5) == 0)
   {
      link = pull_out_url(html_tag,"src");
      return(link);
   }

   if (strnicmp(lower_tag,"body",4) == 0)
   {
      link = pull_out_url(html_tag,"background");
      return(link);
   }

   return(""); // no match
}

/*************************************************************************************************/
/* Function: PULL_OUT_URL                                                                        */
/*                                                                                               */
/* Usage: inputs a html tag that is known to be an image or an href,                             */
/*        and sends back the url for it                                                          */
/*                                                                                               */
/*                                                                                               */
/* Inputs: a single html tag that is known to contain an image, the beginning of that tag        */
/* Outputs: the url for the image                                                                */
/*************************************************************************************************/
char *pull_out_url(char *tag, char *parse_for)
{
   static char *newurl;
   static char tempchr[2] = "";
   static int flag = 1;

   if (flag == 1)
   {
      if ((newurl = (char *) malloc (2048 * (sizeof(char)))) == NULL)
         error_out("Error allocating memory");
      flag = 0;
   }

   sprintf(tempchr,"");
   strcpy(newurl,"");

   for(;;)  // walks through the string that was passed, and looks for the first instance of the beginning of the tag
   {        // which will usually be either 'src' or 'href'

      if(tag[0] == '\0')  // didn't find it
         return("");

      if(strnicmp(tag,parse_for,(strlen(parse_for))) != 0)
         tag++;
      else
         break;
   }

   tag+=(strlen(parse_for));           // advance over the 'src' or 'href' or whatever


   while (tag[0] == ' ') // skip white space
      tag++;

   if (tag[0] == '=')    // skip the '='
      tag++;

   while (tag[0] == ' ') // skip any more white space
      tag++;

   if(tag[0] =='\"') // advance over the " mark if it's there
      tag++;

   if(tag[0] =='\\') // advance over the \ mark if it's there
      tag++;

   if(tag[0] =='/') // advance over the \ mark if it's there
      tag++;

   while((!(tag[0] == ' ')) && (!(tag[0] == '\"')) && (!(tag[0] == '#')) && (strlen(tag) > 1)) // look for a space, a " mark or the end of the string
   {
      tempchr[1] = '\0';
      tempchr[0] = tag[0];
      strcat(newurl,tempchr);
      tag++;
   }
   return(newurl);
}

/*************************************************************************************************/
/* Function:  ADD_BEGIN_URL                                                                      */
/*                                                                                               */
/*                                                                                               */
/*                                                                                               */
/*                                                                                               */
/*  Inputs:  the beginning url to add if this doesn't already begin with http, ftp, gopher, etc  */
/* outputs: a single url                                                                         */
/*************************************************************************************************/
char *add_begin_url(char *begin_url, char *end_url)
{
   static char *full_url;
   static int flag = 1;

   if (flag == 1)
   {
      if ((full_url = (char *) malloc ( 2148 *  (sizeof(char)))) == NULL)
         error_out("Error allocating memory");
      flag = 0;
   }

   if (strnicmp(end_url,"http",4) == 0)
         return(end_url);
   if (strnicmp(end_url,"nntp",4) == 0)
      return(end_url);
   if (strnicmp(end_url,"ftp",3) == 0)
      return(end_url);
   if (strnicmp(end_url,"gopher",6) == 0)
      return(end_url);
   if (strnicmp(end_url,"mailto",6) == 0) // this is NOT supported
      return("");

   sprintf(full_url,"%s%s",begin_url,end_url);
   return(full_url);
}

