#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h> // cprintf
#include "btree.h"
#include "main.h"





/*************************************************************************************************/
/* Function:                                                                                     */
/* Usage:                                                                                        */
/*                                                                                               */
/*                                                                                               */
/* Inputs:                                                                                       */
/* Outputs:                                                                                      */
/* Global Vars:                                                                                  */
/*************************************************************************************************/
NODE *add_node (char *text)
{
   int c;
   static NODE *root = NULL;
   NODE *newnode = NULL;

   newnode = allocate(text);
   root = add_to_tree(root, newnode);

   return(root);
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

/*************************************************************************************************/
/* Function:                                                                                     */
/* Usage:                                                                                        */
/*                                                                                               */
/*                                                                                               */
/* Inputs:                                                                                       */
/* Outputs:                                                                                      */
/* Global Vars:                                                                                  */
/*************************************************************************************************/
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

/*************************************************************************************************/
/* Function:                                                                                     */
/* Usage:                                                                                        */
/*                                                                                               */
/*                                                                                               */
/* Inputs:                                                                                       */
/* Outputs:                                                                                      */
/* Global Vars:                                                                                  */
/*************************************************************************************************/
int write_sorted_tree(NODE *r, FILE *out)
{
   static int dupes;

   if (r)
   {
      write_sorted_tree(r->left, out);
      if(deldupes(r->text))
         fwrite(r->text,strlen(r->text),1,out);
      else
         dupes++;
      write_sorted_tree(r->right, out);
   }
   return dupes;
}

/*************************************************************************************************/
/* Function: DELDUPES                                                                            */
/* Usage: compares the last string to go in with the current string - if they match, return a 0  */
/*        if they don't match, copy the current to the last and return a 1                       */
/*                                                                                               */
/* Inputs: a string of chars                                                                     */
/* Outputs: 0 if current string is dupe of last 1 if not                                         */
/* Global Vars: none                                                                             */
/*************************************************************************************************/
int deldupes(char *current)
{
   static int flag= 1;
   static char *last;

   if (flag == 1)
   {
      if ( (last = (char *)malloc(2048 * (sizeof(char)))) == NULL)
         error_out("Error allocating memory");
      flag = 0;
      strcpy(last,current);
      return(1);
   }

   if (strnicmp(last,current,strlen(current)) == 0)
      return(0);
   else
      strcpy(last,current);
      return(1);
}

