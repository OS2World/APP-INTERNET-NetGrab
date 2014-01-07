#include <stdio.h>
#include <stdlib.h>    // needed for atoi
#include <conio.h>     // cprintf
#include <os2.h>
#include <ctype.h>     // needed for isdigit
#include <string.h>
#include <io.h>        // needed for access
#include "netgrab.h"   // needed for error_out and make_upper
#include "urlparse.h"

/***********************************************************/
/*               URLPARSE                                  */
/* parses the url into a structure                         */
/*                                                         */
/*                                                         */
/***********************************************************/
int URLParse(char *url, URLData_p pURLData)
{

   /* Initialize URL Data variable */
   memset(pURLData, 0, sizeof(URLData_t));

   // Check for the HTTP://
   if (url_check_begin(&url, pURLData, URLHTTPSIG) == 1)
      return(get_http_url(&url, pURLData));

   // Check for the FTP://
   if (url_check_begin(&url, pURLData, URLFTPSIG) == 1)
      return(get_ftp_url(&url, pURLData));

   // Check for the NNTP://
   if (url_check_begin(&url, pURLData, URLNNTPSIG) == 1)
      return(get_nntp_url(&url, pURLData));

   // Check for the GOPHER://
   if (url_check_begin(&url, pURLData, URLGOPHERSIG) == 1)
      return(get_goph_url(&url, pURLData));

   // Check for the FINGER://
   if (url_check_begin(&url, pURLData, URLFINGERSIG) == 1)
      return(get_fing_url(&url, pURLData));

   // Check for the TIME://
   if (url_check_begin(&url, pURLData, URLTIMESIG) == 1)
      return(get_time_url(&url, pURLData));

   // Check for the POPMAIL://
   if (url_check_begin(&url, pURLData, URLPOPMAILSIG) == 1)
      return(get_mail_url(&url, pURLData));

   // Check for the TELNET://
   if (url_check_begin(&url, pURLData, URLTELNETSIG) == 1)
      return(get_telnet_url(&url, pURLData));

   // Check for the FILE://
   if (url_check_begin(&url, pURLData, URLFILESIG) == 1)
      return(get_file_url(&url, pURLData));

   return(0);
}

/*************************************************************************************************/
/* Function: URL_CHECK_BEGIN                                                                     */
/* Usage:                                                                                        */
/*                                                                                               */
/*                                                                                               */
/* Inputs: a pointer to the url, a pointer to our url data structure, and the signature we're    */
/*         looking for                                                                           */
/* Outputs: 1 if successful, 0 if unsuccessful                                                   */
/* Global Vars: none                                                                             */
/*************************************************************************************************/
int url_check_begin(char **url, URLData_p pURLData,char *begin)
{
   char *Purl;
   int counter=0;

   // Check the beginning of the string to see if the signature matches
   if (strnicmp(*url, begin, strlen(begin)) == 0)
   {
      // Update the string pointer since we are finished with the signature part
      *url += strlen(begin);
      Purl = *url;

      for (counter=0;counter<2;counter++)
      {
         if ((Purl[0] == '/') || (Purl[0] == '\\'))  // remove the /
            Purl++;
         else
            return (0);
      }
      *url=Purl;
      return(1); // success
   }
   else
      return(0); // not this one
}

/***********************************************************/
/*               GET_FTP_URL                               */
/* parses the FTP url into a structure                     */
/*                                                         */
/*                                                         */
/***********************************************************/
int get_ftp_url(char **url, URLData_p pURLData)
{
   char TempStr[1025];
   char TempStr2[2];
   char *Purl;

   /* Initialize URL Data variable */
   memset(pURLData, 0, sizeof(URLData_t));
   /* Set the URL type */
   pURLData->type = URLFTP;

   url_get_pass(url,pURLData);

   if(url_get_host(url,pURLData) == 0)
      return(0);
   if(url_get_netrc(pURLData) == 0)
   {
      strcpy(pURLData->username, "anonymous");
      strcpy(pURLData->password, "os2user@");
   }

   // See if there is anything after the hostname.
   // If not return successful
   Purl = *url;
   if (Purl[0] == 0)
   {
      strcpy(pURLData->path, "/");
      return(1);
   }

   url_get_port(url,pURLData);

   Purl = *url;
   if (Purl[0] == 0)
   {
      strcpy(pURLData->path, "/");
      return(1);
   }

   //                 PATH   FOR FTP
   // See if there is anything else in the URL.   If so it is the path
   if (Purl[0] != 0)
   {
      // take off the trailing / or \ if there is one - it screws up the cwd
      if ((strlen(*url) > 1) && ((Purl[(strlen(*url)-1)] == '/') || (Purl[(strlen(*url)-1)] == '\\')))
         Purl[(strlen(*url)-1)] = 0;
      sprintf(pURLData->path,"%s",*url);
   }
   return(1);
}

/***********************************************************/
/*               GET_GOPH_URL                              */
/* parses the gofur url into a structure                   */
/*                                                         */
/*                                                         */
/***********************************************************/
int get_goph_url(char **url, URLData_p pURLData)
{
   char TempStr[1025];
   char TempStr2[2];
   int counter=0;
   char *Purl;

   /* Initialize URL Data variable */
   memset(pURLData, 0, sizeof(URLData_t));
   /* Set the URL type */
   pURLData->type = URLGOPHER;

   url_get_host(url,pURLData);

   // See if there is anything after the hostname.
   Purl = *url;
   if (Purl[0] == 0)
      return(0);

   url_get_port(url,pURLData);

   Purl = *url;
   // See if there is anything after the port.
   if (Purl[0] == 0)
      return(0);

   //                 PATH
   // anything else is the path
   strcpy(pURLData->path, *url);
   return(1);
}

/***********************************************************/
/*               GET_HTTP_URL                              */
/* parses the HTTP url into a structure                    */
/*                                                         */
/*                                                         */
/***********************************************************/
int get_http_url(char **url, URLData_p pURLData)
{
   char TempStr[1025];
   char TempStr2[2];
   int counter=0;
   char *Purl;

   /* Initialize URL Data variable */
   memset(pURLData, 0, sizeof(URLData_t));
   // Set the URL type
   pURLData->type = URLHTTP;

   url_get_pass(url,pURLData);

   if (url[0] == 0)
      return (0);

   url_get_host(url,pURLData);

   // See if there is anything after the hostname.
   // If not return failed

   url_get_port(url,pURLData);

   //                 PATH   FOR HTTP OR GOPHER
   // See if there is anything else in the URL.   If so it is the path
   Purl = *url;
   if (Purl[0] != 0)
   {
      strcpy(pURLData->path, *url);
   }
      return(1);
}

/***********************************************************/
/*               GET_NNTP_URL                              */
/* parses the NNTP url into a structure                    */
/*                                                         */
/*                                                         */
/***********************************************************/
int get_nntp_url(char **url, URLData_p pURLData)
{
   char *tempstr3;
   char TempStr[1025];
   char TempStr2[2];
   int counter= 0;
   char *Purl;

   /* Initialize URL Data variable */
   memset(pURLData, 0, sizeof(URLData_t));

//    Set the URL type
   pURLData->type = URLNNTP;

   url_get_host(url,pURLData);

   // See if there is anything after the hostname.
   // If not return successful
   Purl = *url;
   if (Purl[0] == 0)
   {
      strcpy(pURLData->path, "GeTlIsT");
      return(1);
   }

   url_get_port(url,pURLData);

   //                 PATH   FOR NNTP (GROUP)
   Purl = *url;
   if ((Purl[0] == '\\') || (Purl[0] == '/')) // Increment the pointer to skip the slash
      Purl++;
   *url = Purl;

   // check a second time to see if we need to get a list
   if (Purl[0] == 0)
   {
      strcpy(pURLData->path, "GeTlIsT");
      return(1);
   }

   // PATH
   // Reinitialize the temporaray strings
   strcpy(TempStr, "");
   strcpy(TempStr2, "");
   for (;;)
   {
      // See if we are at the end of the URL
      if (Purl[0] == 0)
         break;
      // See if we are at one of the characters that denotes the starting of the port or path
      if ((Purl[0] == '\\') || (Purl[0] == '/'))
         break;
      // Add the character to the string
      TempStr2[0] = Purl[0];
      strcat(TempStr, TempStr2);
      // Increment the pointer
      Purl++;
   }
   *url = Purl;
   // Store the group name in the path variable
   strcpy(pURLData->path, TempStr);
   // if that's the end of the url, we're done
   if (Purl[0] == 0)
   {
      pURLData->startx = 0;// if no beginning article, set it to 0
      *url = Purl;
      return(1);
   }

   //                  STARTX    (NNTP ONLY)
   // if not, get the article number
   Purl = *url;
   if ((Purl[0] == '\\') || (Purl[0] == '/'))
   {
      // Increment the pointer to skip the slash
      Purl++;
      // Reinitialize the temporaray strings
      strcpy(TempStr, "");
      strcpy(TempStr2, "");
      for (;;)
      {
         // See if we are at the end of the URL
         if (Purl[0] == 0)
            break;
         // See if we are at one of the characters that denotes the starting of the port or path
         if (!isdigit(Purl[0]))
            break;
         // Add the character to the string
         TempStr2[0] = Purl[0];
         strcat(TempStr, TempStr2);
         // Increment the pointer
         Purl++;
      }
      // if there is no beginning article, set it to 0
      if (TempStr == NULL)
         pURLData->startx = 0;
      // Store the starting number
      else
         pURLData->startx = atoi(TempStr);
   }

   // if that's the end of the url, we're done
   if (Purl[0] == 0)
      return(1);

   //            ENDX

   // See if we need to find endx
   if (Purl[0] == ':')
   {
      // Increment the pointer to skip the colon
      Purl++;
      // Reinitialize the temporaray strings
      strcpy(TempStr, "");
      strcpy(TempStr2, "");
      for (;;)
      {
         // See if we are at the end of the URL
         if (Purl[0] == 0)
            break;
         // See if we are at one of the characters that
         // denotes the starting of the port or path
         if (!isdigit(Purl[0]))
            break;
         // Add the character to the string
         TempStr2[0] = Purl[0];
         strcat(TempStr, TempStr2);
         // Increment the pointer
         Purl++;
      }
      // Store the port number
      pURLData->endx = atoi(TempStr);
   }


   //              LOOKFOR  (NNTP ONLY)

   // if not, get the things to look for
   if ((Purl[0] == ' ') || (Purl[0] == ','))
   {
      counter = 0;
      do
      {
         // Increment the pointer to skip the spacer
         if (counter > 20)
            error_out("Error:20 things to look for max");
         Purl++;
         // Reinitialize the temporaray strings
         strcpy(TempStr, "");
         strcpy(TempStr2, "");
         for (;;)
         {
            // See if we are at the end of the URL
            // if we get a quotation mark, keep adding characters til we get another
            if (Purl[0] == '`')
            {
               for (;;)
               {
                  if (Purl[0] == 0)
                     break;
                  if (Purl[0] == '`')
                  {
                     Purl++;
                     break;
                  }
                  // Add the character to the string
                  TempStr2[0] = Purl[0];
                  strcat(TempStr, TempStr2);
                  // Increment the pointer
                  Purl++;
               }
            }
            if (Purl[0] == 0)
               break;
            if (Purl[0] == ',')
               break;
            // Add the character to the string
            TempStr2[0] = Purl[0];
            strcat(TempStr, TempStr2);
            // Increment the pointer
            Purl++;
         }
         // Store the  lookfor variable
         tempstr3 = &TempStr[0];
         makeupper(&tempstr3);
         sprintf(pURLData->lookfor[counter],"%s",tempstr3);
         counter++;
         pURLData->looknum = counter;
      }
      while (Purl[0] != 0);
   }
   return(1);
}

/***********************************************************/
/*               GET_TIME_URL                              */
/* parses the TIME url into a structure                    */
/*                                                         */
/*                                                         */
/***********************************************************/
int get_time_url(char **url, URLData_p pURLData)
{
   char TempStr[1025];
   char TempStr2[2];
   int counter=0;
   char *Purl;

   /* Initialize URL Data variable */
   memset(pURLData, 0, sizeof(URLData_t));
   /* Set the URL type */
   pURLData->type = URLTIME;

   url_get_host(url,pURLData);

   // See if there is anything after the hostname.
   // If not return successful
   Purl = *url;
   if (Purl[0] == 0)
      return(1);

   url_get_port(url,pURLData);

   //                 PATH
   // See if there is anything else in the URL.   If so it is the path
   Purl = *url;
   if (Purl[0] != 0)
   {
      strcpy(pURLData->path, *url);
      // return successful
   }
   return(1);// we're happy either way!
}

/***********************************************************/
/*               GET_FING_URL                              */
/* parses the TIME url into a structure                    */
/*                                                         */
/*                                                         */
/***********************************************************/
int get_fing_url(char **url, URLData_p pURLData)
{
   char TempStr[1025];
   char TempStr2[2];
   int counter=0;
   char *Purl;

   /* Initialize URL Data variable */
   memset(pURLData, 0, sizeof(URLData_t));
   /* Set the URL type */
   pURLData->type = URLFINGER;

   Purl = *url;
   if (Purl[0] == 0)
      return(0);

   url_get_pass(url,pURLData);

   url_get_host(url,pURLData);

   // See if there is anything after the hostname.  If not return successful
   Purl = *url;

   if (Purl[0] == 0)
      return(1);

   // See if we need to find the port
   url_get_port(url,pURLData);


   Purl = *url;
   if ((Purl[0] == '/') || (Purl[0] == '\\'))  // remove the /
      Purl++;
   *url = Purl;

   //                 PATH
   // See if there is anything else in the URL.   If so it is the path
   Purl = *url;
   if (Purl[0] != 0)
      strcpy(pURLData->username, *url);

   return(1);// we're happy either way!
}

/***********************************************************/
/*               GET_MAIL_URL                              */
/* parses the MAIL url into a structure                    */
/*                                                         */
/*                                                         */
/***********************************************************/
int get_mail_url(char **url, URLData_p pURLData)
{
   char TempStr[1025];
   char TempStr2[2];
   int counter=0;
   int flag = 0;
   char *Purl;

   /* Initialize URL Data variable */
   memset(pURLData, 0, sizeof(URLData_t));
   /* Set the URL type */
   pURLData->type = URLPOPMAIL;

   // separate username & password
   Purl = *url;
   if (Purl[0] == 0)  // beginning of section that looks for password & username
      return(0);

   url_get_pass(url,pURLData);
   url_get_host(url,pURLData);


   // See if there is anything after the hostname.
   // If not return successful
   Purl = *url;
   if (Purl[0] == 0)
   {
      pURLData->startx = 0;
      pURLData->endx = 0;
      return(1);
   }

   url_get_port(url,pURLData);

   //                  STARTX

   // if not, get the beginning letter number
   Purl = *url;
   if ((Purl[0] == '\\') || (Purl[0] == '/'))
   {
      // Increment the pointer to skip the slash
      Purl++;
      // Reinitialize the temporaray strings
      strcpy(TempStr, "");
      strcpy(TempStr2, "");
      for (;;)
      {
         // See if we are at the end of the URL
         if (Purl[0] == 0)
            break;
         // See if we are at one of the characters that denotes the starting of the port or path
         if (!isdigit(Purl[0]))
            break;
         // Add the character to the string
         TempStr2[0] = Purl[0];
         strcat(TempStr, TempStr2);
         // Increment the pointer
         Purl++;
      }
      // if there is no beginning letter, set it to 1
      if (TempStr == NULL)
         pURLData->startx = 0;
      // Store the starting number
      else
         pURLData->startx = atoi(TempStr);
   }
   else
      pURLData->startx = 0;

   // if that's the end of the url, we're done
   Purl = *url;
   if (Purl[0] == 0)
   {
      pURLData->endx = 0;
      return(1);
   }
   //            ENDX

   // See if we need to find endx
   Purl = *url;
   if (Purl[0] == ':')
   {
      // Increment the pointer to skip the colon
      Purl++;
      // Reinitialize the temporaray strings
      strcpy(TempStr, "");
      strcpy(TempStr2, "");
      for (;;)
      {
         // See if we are at the end of the URL
         if (Purl[0] == 0)
            break;
         // See if we are at one of the characters that
         // denotes the starting of the port or path
         if (!isdigit(Purl[0]))
            break;
         // Add the character to the string
         TempStr2[0] = Purl[0];
         strcat(TempStr, TempStr2);
         // Increment the pointer
         Purl++;
      }
      // Store the port number
      pURLData->endx = atoi(TempStr);
   }
   else
      pURLData->endx = 0;
   return (1);
}


/***********************************************************/
/*               GET_FILE_URL                              */
/* parses the file url into a structure                    */
/*                                                         */
/*                                                         */
/***********************************************************/
int get_file_url(char **url, URLData_p pURLData)
{
   char TempStr[1025];
   char TempStr2[2];
   int counter=0;
   char *Purl;

   /* Initialize URL Data variable */
   memset(pURLData, 0, sizeof(URLData_t));
   /* Set the URL type */
   pURLData->type = URLFILE;

   //                 PATH
   // See if there is anything else in the URL.   If so it is the path
   Purl = *url;
   if (Purl[0] != 0)
   {
      strcpy(pURLData->path, *url);
      // return successful
      return(1);
   }
   else
      return(0);
}

/***********************************************************/
/*               GET_TELNET_URL                            */
/* parses the TELNET url into a structure                  */
/*                                                         */
/*                                                         */
/***********************************************************/
int get_telnet_url(char **url, URLData_p pURLData)
{
   char TempStr[1025];
   char TempStr2[2];
   int counter=0;
   int flag = 0;
   char *Purl;

   /* Initialize URL Data variable */
   memset(pURLData, 0, sizeof(URLData_t));
   /* Set the URL type */
   pURLData->type = URLTELNET;

   url_get_pass(url,pURLData);
   if(url_get_host(url,pURLData) == 0)
      return(0);
   if(url_get_netrc(pURLData) == 0)
      return(0);

   // See if there is anything after the hostname.
   // If not return failure - no command to execute
   Purl = *url;
   if (Purl[0] == 0)
      return(0);

   url_get_port(url,pURLData);

   //                 PATH   FOR TELNET
   // See if there is anything else in the URL.   If so it is the path
   Purl = *url;
   if (Purl[0] != 0)
   {
      if ((strlen(*url) > 1) && ((Purl[(strlen(*url)-1)] == '/') || (Purl[(strlen(*url)-1)] == '\\')))
         Purl[(strlen(*url)-1)] = 0;
      strcpy(pURLData->path, *url);
      // return successful
      return(1);
   }
   else
      return(0); //  if not, kill the program
}

/*************************************************************************************************/
/* Function: PARSE_NETRC                                                                         */
/* Usage:  looks for netrc environment variable,  then parses through the netrc file to get      */
/*         password and username for the machine we are going to.                                */
/*                                                                                               */
/* Inputs: pURLData - a structure to store everything relating to a url in                            */
/* Outputs: 1 if it worked, 0 if it failed                                                       */
/* Global Vars: none (url is passed)                                                             */
/*************************************************************************************************/
int parse_netrc(URLData_p pURLData)
{
   char *multivar; // a variable for many uses
   char *environ;  // store the returns from our environment variables
   char *oneline;
   char *onebegin;
   char tempbuf[2];
   FILE *netrc;

   if ((multivar = (char *) malloc ( 128 * (sizeof(char)))) == NULL)
      error_out("Error allocating memory");
   if ((oneline = (char *) malloc ( 258 * (sizeof(char)))) == NULL)
      error_out("Error allocating memory");

   onebegin = oneline;
   sprintf(tempbuf,"");
   sprintf(oneline,"");
   sprintf(multivar,"");
   tempbuf[1] = '\0';
   environ = getenv("NETRC");
   if (environ == NULL)
   {
      environ = getenv("ETC");
      if (environ == NULL) // not set
         return(0);
      else
      {
         strcpy(oneline,environ);
         strcat(oneline,"/netrc");
         environ = oneline;
      }
   }


   if (!(0 == access(environ,00)))// if file doesn't exist
      return(0);

   netrc = fopen(environ,"r");
   sprintf(environ,"");       // clear the buffer for re-use
   while (0 == feof(netrc))
   {
      if(fgets(oneline,256,netrc) == NULL)
         return(0);

      if((strnicmp(oneline,"machine",7)) != 0) // if the line doesn't begin with 'machine'
         continue;
      oneline+=7;

      while(oneline[0] == ' ') // get rid of any spaces
         oneline++;

      if( ( strnicmp(oneline, pURLData->hostname,(strlen(pURLData->hostname)) ) == 0))
         oneline += strlen (pURLData->hostname);

      else if((strnicmp(oneline,"netgrab",7) ) == 0) // look for netgrab - that's the default
         oneline += 7;
      else
         continue;

      while(oneline[0] == ' ') // get rid of any spaces
         oneline++;

      if((strnicmp(oneline,"login",5)) != 0) // if the line doesn't continue with 'login'
         return(0);
      oneline+=5;

      while(oneline[0] == ' ') // get rid of any spaces
         oneline++;

      while(oneline[0] != ' ') // parse for username until we get to another space
      {
         tempbuf[0] = oneline[0];
         strcat(multivar,tempbuf);
         oneline++;
      }
      strcpy(pURLData->username,multivar);

      sprintf(multivar,""); // clear the buffer

      while(oneline[0] == ' ') // get rid of any spaces
         oneline++;

      if((strnicmp(oneline,"password",8)) != 0) // if the line doesn't continue with 'password'
         return(0);
      oneline+=8;

      while(oneline[0] == ' ') // get rid of any spaces
         oneline++;

      while((oneline[0] != ' ') && (oneline[0] != '\r') && (oneline[0] != '\n')&& (oneline[0] != '\0')) // parse for username until we get to a space, a newline, a carriage return, or the end of the string
      {
         tempbuf[0] = oneline[0];
         strcat(multivar,tempbuf);
         oneline++;
      }
      strcpy(pURLData->password,multivar);
      break;
   }
   oneline = environ;
   fclose(netrc);
   free((char *)multivar);
   oneline = onebegin;
   free((char *)oneline);
   return(1);
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
int url_get_pass(char **url, URLData_p pURLData)
{
   char TempStr[1025];
   char TempStr2[2];
   int counter=0;
   int flag = 0;
   char *Purl;


   Purl = *url;
   // separate username & password
   if (Purl[0] != 0)  // beginning of section that looks for password & username
   {
      // so if we have an @ in the url before the first / after the sig we have a password
      // because of url://user:password@host
      if ((((strstr(*url,"@")) != 0)                     // if there is an @ in the url
      && ((strstr(*url,"@")) < (strstr(*url,"/"))      // and the @ is before the first remaining /
      || (strstr(*url,"@")) < (strstr(*url,"\\"))))     // or the @ is before the first remaining \
       /* or if there is an @ and there is no / and no \ */
      || (((strstr(*url,"@")) != 0) && ((strstr(*url,"/")) == 0) && ((strstr(*url,"\\")) == 0)))      {

         // look for user first

         // Reinitialize the temporaray strings
         strcpy(TempStr, "");
         strcpy(TempStr2, "");
         TempStr2[1] = 0;
         for (;;)
         {
            // See if we are at the end of the URL
            if (Purl[0] == 0)
               break;
            // See if we are at one of the characters that
            // denotes the starting of the password
            if (Purl[0] == ':')
            {
            flag = 1; // set a flag to do the password check
            break;
            }
            if ((Purl[0] == '@') || (Purl[0] == '\\') || (Purl[0] == '/'))
               break;
            // Add the character to the string
            TempStr2[0] = Purl[0];
            strcat(TempStr, TempStr2);
            // Increment the pointer
            Purl++;
            *url = Purl;
         }
         // Store the username
         strcpy(pURLData->username, TempStr);
         Purl++;

         if (flag)
         {
            //next look for password
            // Reinitialize the temporaray strings
            strcpy(TempStr, "");
            strcpy(TempStr2, "");

            if (Purl[0] == '\'')
            {
               Purl++;// advance past the first quote
               while (Purl[0] != '\'')
               {
                  if (Purl[0] == 0)
                     break;
                  // Add the character to the string
                  TempStr2[0] = Purl[0];
                  strcat(TempStr, TempStr2);
                  // Increment the pointer
                  Purl++;
               }
               Purl++; // advance past the last quote
            }

            for (;;)
            {
               // See if we are at the end of the URL
               if (Purl[0] == 0)
                  break;
               // See if we are at one of the characters that
               // denotes the starting of the password
               if ((Purl[0] == ':') || (Purl[0] == '@') || (Purl[0] == '\\') || (Purl[0] == '/'))
                  break;
               // Add the character to the string
               TempStr2[0] = Purl[0];
               strcat(TempStr, TempStr2);
               // Increment the pointer
               Purl++;

            }
            // Store the password
            strcpy(pURLData->password, TempStr);
            Purl++;
         }
         *url = Purl;
         return(1);
      }
   } // end of the section that looks for password & username
   return (0);
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
int url_get_host(char **url, URLData_p pURLData)
{
   char TempStr[1025];
   char TempStr2[2];
   char *Purl;
   // Retrieve the host name.
   strcpy(TempStr, "");
   strcpy(TempStr2, "");
   TempStr2[1] = 0;


   Purl = *url;

   for (;;)
   {
      // See if we are at the end of the URL
      if (Purl[0] == 0)
         break;
      // See if we are at one of the characters that denotes the starting of the port or path
      if ((Purl[0] == ':') || (Purl[0] == '/') || (Purl[0] == '\\'))
         break;
      // Add the character to the string
      TempStr2[0] = Purl[0];
      strcat(TempStr, TempStr2);
      // Increment the pointer
      Purl++;
   }
   // Store the hostname retrieved
   memset(pURLData->hostname,0,1024);
   strcpy(pURLData->hostname, TempStr);
   *url = Purl;
   return(1);
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
int url_get_port(char **url, URLData_p pURLData)
{
   char TempStr[1025];
   char TempStr2[2];
   char *Purl;

   Purl = *url;
   // See if we need to find the port
   if (Purl[0] == ':')
   {
      // Increment the pointer to skip the colon
      Purl++;
      // Reinitialize the temporaray strings
      strcpy(TempStr, "");
      strcpy(TempStr2, "");
      for (;;)
      {
         // See if we are at the end of the URL
         if (Purl[0] == 0)
            break;
         // See if we are at one of the characters that
         // denotes the starting of the port or path
         if (!isdigit(Purl[0]))
            break;
         // Add the character to the string
         TempStr2[0] = Purl[0];
         strcat(TempStr, TempStr2);
         // Increment the pointer
         Purl++;
      }
      // Store the port number
      pURLData->port = atoi(TempStr);
      *url=Purl;
      return(1);
   }
   else
      return(0);
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
int url_get_netrc(URLData_p pURLData)
{
   if ((strlen(pURLData->username)<1) && (strlen(pURLData->password) <1 ))
   {
      parse_netrc(pURLData); // try to get the passwords from the netrc if they weren't given in the password section

      if ((strlen(pURLData->username)<1) && (strlen(pURLData->password) <1 ))
      {
         return (0);
      }
   }
   return (1);
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