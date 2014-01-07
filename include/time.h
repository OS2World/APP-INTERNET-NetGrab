#if (defined(__IBMC__) || defined(__IBMCPP__))
#pragma info( none )
#ifndef __CHKHDR__
   #pragma info( none )
#endif
#pragma info( restore )
#endif

#ifndef __time_h
   #define __time_h

   #ifdef __cplusplus
      extern "C" {
   #endif

   #ifndef  _LNK_CONV
      #ifdef _M_I386
         #define _LNK_CONV   _Optlink
      #else
         #define _LNK_CONV
      #endif
   #endif

   #ifndef _IMPORT
      #ifdef __IMPORTLIB__
         #define _IMPORT _Import
      #else
         #define _IMPORT
      #endif
   #endif

   /********************************************************************/
   /*  <time.h> header file                                            */
   /*                                                                  */
   /*  IBM VisualAge C++ for OS/2, Version 3.00                        */
   /*  (C) Copyright IBM Corp. 1991, 1995.                             */
   /*  - Licensed Material - Program-Property of IBM                   */
   /*  - All rights reserved                                           */
   /*                                                                  */
   /********************************************************************/

   #ifndef NULL
      #if (defined(__EXTENDED__)  || defined( __cplusplus ))
         #define NULL 0
      #else
         #define NULL ((void *)0)
      #endif
   #endif

   #define CLOCKS_PER_SEC 1000

   #ifndef __size_t
      #define __size_t
      typedef unsigned int size_t;
   #endif

   typedef unsigned long clock_t;

   #ifndef __time_t
      #define __time_t
      typedef long time_t;
   #endif

   #ifndef __tm_t
      #define __tm_t
      struct tm
         {
         int tm_sec;      /* seconds after the minute [0-61]        */
         int tm_min;      /* minutes after the hour [0-59]          */
         int tm_hour;     /* hours since midnight [0-23]            */
         int tm_mday;     /* day of the month [1-31]                */
         int tm_mon;      /* months since January [0-11]            */
         int tm_year;     /* years since 1900                       */
         int tm_wday;     /* days since Sunday [0-6]                */
         int tm_yday;     /* days since January 1 [0-365]           */
         int tm_isdst;    /* Daylight Saving Time flag              */
      };
   #endif

   clock_t     _IMPORT _LNK_CONV clock( void );
   double      _IMPORT _LNK_CONV difftime( time_t, time_t );
   time_t      _IMPORT _LNK_CONV mktime( struct tm * );
   time_t      _IMPORT _LNK_CONV time( time_t * );
   char *      _IMPORT _LNK_CONV asctime( const struct tm * );
   char *      _IMPORT _LNK_CONV ctime( const time_t * );
   struct tm * _IMPORT _LNK_CONV gmtime( const time_t * );
   struct tm * _IMPORT _LNK_CONV localtime( const time_t * );
   size_t      _IMPORT _LNK_CONV strftime( char *, size_t, const char *, const struct tm * );
   void        _IMPORT _LNK_CONV _tzset( void );
   char *      _IMPORT _LNK_CONV _strdate( char * );
   char *      _IMPORT _LNK_CONV _strtime( char * );

   #ifdef __EXTENDED__
     char *    _IMPORT _LNK_CONV strptime(const char *, const char *, struct tm *);
   #endif

   extern int  _IMPORT _daylight; /* non-zero if daylight savings time is used */
   extern long _IMPORT _timezone; /* difference in seconds between UCT and local time */
   extern char * _IMPORT _tzname[2]; /* std/daylight savings time zone names  */

#if (defined(__IBMC__) || defined(__IBMCPP__))
  #pragma info( none )
#endif
   #define difftime( t1, t0 ) ((double)((t1) - (t0)))
   #define ctime( t ) (asctime(localtime(t)))
#if (defined(__IBMC__) || defined(__IBMCPP__))
  #pragma info( restore )
#endif

   #ifndef __ANSI__

      #define CLK_TCK  CLOCKS_PER_SEC

      #ifndef __SAA_L2__

         #if defined(__EXTENDED__)

            #define daylight _daylight
            #define tzname   _tzname
            #define tzset( ) _tzset( )

         #endif

      #endif

   #endif

   #ifdef __cplusplus
      }
   #endif

#endif

#if (defined(__IBMC__) || defined(__IBMCPP__))
#pragma info( none )
#ifndef __CHKHDR__
   #pragma info( restore )
#endif
#pragma info( restore )
#endif
