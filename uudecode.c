
/*
 * uudecode [input]
 *
 * create the specified file, decoding as you go.
 * used with uuencode.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>   /* MSDOS or UNIX */
#include <sys/stat.h>
/* single-character decode */
#define DEC(c)   (((c) - ' ') & 077)
#define   NULL   0

void decode(FILE *in, FILE *out);
void outdec(char *p, FILE *f,int n);
char *index(char *sp, char c);

void main(int argc,char **argv)
{
   FILE *in, *out;
   int mode;
   char dest[128];
   char buf[80];

   /* optional input arg */
   if (argc > 1)
   {
      if ((in = fopen(argv[1], "r")) == NULL)
      {
         perror(argv[1]);
         exit(1);
      }
      argv++; argc--;
   }
   else
      in = stdin;

   if (argc != 1)
   {
      printf("Usage: uudecode [infile]\n");
      exit(2);
   }

   /* search for header line */
   for (;;)
   {
      if (fgets(buf, sizeof buf, in) == NULL)
      {
         fprintf(stderr, "No begin line\n");
         exit(3);
      }
      if (strncmp(buf, "begin ", 6) == 0)
         break;
   }
   (void)sscanf(buf, "begin %o %s", &mode, dest);


   out = fopen(dest, "wb");   /* Binary file */

   if (out == NULL)
   {
      perror(dest);
      exit(4);
   }

   decode(in, out);

   if (fgets(buf, sizeof buf, in) == NULL || strcmp(buf, "end\n"))
   {
      fprintf(stderr, "No end line\n");
      exit(5);
   }
   exit(0);
}

/*
 * copy from in to out, decoding as you go along.
 */
void decode(FILE *in, FILE *out)
{
   char buf[80];
   char *bp;
   int n, i, expected;

   for (;;)
   {
      /* for each input line */
      if (fgets(buf, sizeof buf, in) == NULL)
      {
         printf("Short file\n");
         exit(10);
      }
      n = DEC(buf[0]);
      if ((n <= 0) || (buf[0] == '\n'))
         break;

      /* Calculate expected # of chars and pad if necessary */
      expected = ((n+2)/3)<<2;
      for (i = strlen(buf)-1; i <= expected; i++) buf[i] = ' ';

      bp = &buf[1];
      while (n > 0) {
         outdec(bp, out, n);
         bp += 4;
         n -= 3;
      }
   }
}

/*
 * output a group of 3 bytes (4 input characters).
 * the input chars are pointed to by p, they are to
 * be output to file f.  n is used to tell us not to
 * output all of them at the end of the file.
 */
void outdec(char *p, FILE *f,int n)
{
   int c1, c2, c3;

   c1 = DEC(*p) << 2 | DEC(p[1]) >> 4;
   c2 = DEC(p[1]) << 4 | DEC(p[2]) >> 2;
   c3 = DEC(p[2]) << 6 | DEC(p[3]);
   if (n >= 1)
      putc(c1, f);
   if (n >= 2)
      putc(c2, f);
   if (n >= 3)
      putc(c3, f);
}

/*
 * Return the ptr in sp at which the character c appears;
 * NULL if not found
 */
char *index(char *sp, char c)
{
   do
   {
      if (*sp == c)
         return(sp);
   }
   while (*sp++);
   return(NULL);
}
