#if (defined(__IBMC__) || defined(__IBMCPP__))
#pragma info( none )
#ifndef __CHKHDR__
   #pragma info( none )
#endif
#pragma info( restore )
#endif

#ifndef __ctype_h
   #define __ctype_h

   #ifdef __cplusplus
      extern "C" {
   #endif

   #ifndef  _LNK_CONV
     #ifdef _M_I386
         #define  _LNK_CONV   _Optlink
     #else
       #define  _LNK_CONV
     #endif
   #endif

   /********************************************************************/
   /*  <ctype.h> header file                                           */
   /*                                                                  */
   /*  IBM VisualAge C++ for OS/2, Version 3.00                        */
   /*  (C) Copyright IBM Corp. 1991, 1995.                             */
   /*  - Licensed Material - Program-Property of IBM                   */
   /*  - All rights reserved                                           */
   /*                                                                  */
   /********************************************************************/

   int _LNK_CONV isalnum( int );
   int _LNK_CONV isalpha( int );
   #ifdef __EXTENDED__
     int _LNK_CONV isblank( int );
   #endif
   int _LNK_CONV iscntrl( int );
   int _LNK_CONV isdigit( int );
   int _LNK_CONV isgraph( int );
   int _LNK_CONV islower( int );
   int _LNK_CONV isprint( int );
   int _LNK_CONV ispunct( int );
   int _LNK_CONV isspace( int );
   int _LNK_CONV isupper( int );
   int _LNK_CONV isxdigit( int );
   int _LNK_CONV tolower( int );
   int _LNK_CONV toupper( int );

   /* masks for using external character table */
   #ifndef __ISXDIGIT
      #define __ISXDIGIT       0x0001U
   #endif
   #ifndef __ISDIGIT
      #define __ISDIGIT        0x0002U
   #endif
   #ifndef __ISSPACE
      #define __ISSPACE        0x0008U
   #endif
   #ifndef __ISPUNCT
      #define __ISPUNCT        0x0010U
   #endif
   #ifndef __ISCNTRL
      #define __ISCNTRL        0x0020U
   #endif
   #ifndef __ISLOWER
      #define __ISLOWER        0x0040U
   #endif
   #ifndef __ISUPPER
      #define __ISUPPER        0x0080U
   #endif
   #ifndef __ISALPHA
      #define __ISALPHA        0x0100U
   #endif
   #ifndef __ISGRAPH
      #define __ISGRAPH        0x0200U
   #endif
   #ifndef __ISPRINT
      #define __ISPRINT        0x0400U
   #endif
   #ifndef __ISALNUM
      #define __ISALNUM        0x0800U
   #endif
   #ifndef __ISBLANK
      #define __ISBLANK        0x1000U
   #endif

   /* indexes of toupper/tolower tables in _ctype table */
   #define _TOUPPER_INDEX   257
   #define _TOLOWER_INDEX   514

   extern const unsigned short *_ctype;

   #ifdef __cplusplus
      inline int isalnum ( int c )
                          { return _ctype[(c)] & __ISALNUM; }
      inline int isalpha ( int c )
                          { return _ctype[(c)] & __ISALPHA; }
      inline int iscntrl ( int c )
                          { return _ctype[(c)] & __ISCNTRL; }
      inline int isdigit ( int c )
                          { return _ctype[(c)] & __ISDIGIT; }
      inline int isgraph ( int c )
                          { return _ctype[(c)] & __ISGRAPH; }
      inline int islower ( int c )
                          { return _ctype[(c)] & __ISLOWER; }
      inline int isprint ( int c )
                          { return _ctype[(c)] & __ISPRINT; }
      inline int ispunct ( int c )
                          { return _ctype[(c)] & __ISPUNCT; }
      inline int isspace ( int c )
                          { return _ctype[(c)] & __ISSPACE; }
      inline int isupper ( int c )
                          { return _ctype[(c)] & __ISUPPER; }
      inline int isxdigit( int c )
                          { return _ctype[(c)] & __ISXDIGIT; }
      inline int tolower ( int c )
                          { return (signed short)_ctype[(c) + _TOLOWER_INDEX]; }
      inline int toupper ( int c )
                          { return (signed short)_ctype[(c) + _TOUPPER_INDEX]; }
   #else
      #if (defined(__IBMC__) || defined(__IBMCPP__))
      #pragma info( none )
      #endif
      #define isalnum( c )  ( _ctype[(c)] & __ISALNUM )
      #define isalpha( c )  ( _ctype[(c)] & __ISALPHA )
      #define iscntrl( c )  ( _ctype[(c)] & __ISCNTRL )
      #define isdigit( c )  ( _ctype[(c)] & __ISDIGIT )
      #define isgraph( c )  ( _ctype[(c)] & __ISGRAPH )
      #define islower( c )  ( _ctype[(c)] & __ISLOWER )
      #define isprint( c )  ( _ctype[(c)] & __ISPRINT )
      #define ispunct( c )  ( _ctype[(c)] & __ISPUNCT )
      #define isspace( c )  ( _ctype[(c)] & __ISSPACE )
      #define isupper( c )  ( _ctype[(c)] & __ISUPPER )
      #define isxdigit( c ) ( _ctype[(c)] & __ISXDIGIT )
      #define tolower( c )  (signed short)( _ctype[(c) + _TOLOWER_INDEX] )
      #define toupper( c )  (signed short)( _ctype[(c) + _TOUPPER_INDEX] )
      #if (defined(__IBMC__) || defined(__IBMCPP__))
      #pragma info( restore )
      #endif
   #endif

   #if defined(__EXTENDED__)

      int _LNK_CONV _tolower( int );
      int _LNK_CONV _toupper( int );
      int _LNK_CONV _isascii( int );
      int _LNK_CONV _iscsymf( int );
      int _LNK_CONV _iscsym( int );
      int _LNK_CONV _toascii( int );
      int _LNK_CONV isblank( int );

      #if (defined(__IBMC__) || defined(__IBMCPP__))
      #pragma info( none )
      #endif
      #define _tolower( c ) ( (c) + 'a' - 'A' )
      #define _toupper( c ) ( (c) + 'A' - 'a' )
      #define _isascii( c ) ( (unsigned)(c) < 0x80 )
      #define _iscsymf( c ) ( isalpha( c ) || (c) == '_' )
      #define _iscsym( c )  ( isalnum( c ) || (c) == '_' )
      #define _toascii( c ) ( (c) & 0x7f )

      #ifdef __cplusplus
         inline int isascii( int c ) { return _isascii( c ); }
         inline int iscsymf( int c ) { return _iscsymf( c ); }
         inline int iscsym ( int c ) { return _iscsym ( c ); }
         inline int toascii( int c ) { return _toascii( c ); }
         inline int isblank ( int c )
                             { return _ctype[(c)] & __ISBLANK; }
      #else
         #define  isascii( c )        _isascii( c )
         #define  iscsymf( c )        _iscsymf( c )
         #define  iscsym( c )         _iscsym( c )
         #define  toascii( c )        _toascii( c )
         #define  isblank( c )  ( _ctype[(c)] & __ISBLANK )
      #endif
      #if (defined(__IBMC__) || defined(__IBMCPP__))
      #pragma info( restore )
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