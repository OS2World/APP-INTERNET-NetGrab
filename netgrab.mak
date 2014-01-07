# Netgrab.mak
# Created by IBM WorkFrame/2 MakeMake at 19:57:56 on 5 Sept 1997
#
# The actions included in this make file are:
#  Compile::C++ Compiler
#  Link::Linker

.SUFFIXES: .c .obj

.all: \
    .\Netgrab.exe

.c.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /DOS2 /Sm /Ss /Q  /Ti /O+ /Op+ /Tx- /W3 /C /Gm+ /N10 /Gd /C /De /Gh /Fb %s

{c:\code\current\netgrab}.c.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /DOS2 /Sm /Ss /Q  /Ti /O+ /Op+ /Tx- /W3 /C /Gm+ /N10 /Gd /C /De /Gh /Fb %s

.\Netgrab.exe: \
    .\spliturl.obj \
    .\urlparse.obj \
    .\netgrab.obj \
    .\nntp.obj \
    .\sockets.obj \
    {$(LIB)}Netgrab.def \
    Netgrab.mak
    @echo " Link::Linker "
    icc.exe @<<
     /B" /de /br /pmtype:vio /noe /st:128000"
     /FeNetgrab.exe
     CPPOPA3.OBJ
     Netgrab.def
     .\spliturl.obj
     .\urlparse.obj
     .\netgrab.obj
     .\nntp.obj
     .\sockets.obj
     .\lib\os2386.lib
     .\lib\so32dll.lib
     .\lib\tcp32dll.lib
<<

.\spliturl.obj: \
    c:\code\current\netgrab\spliturl.c \
    {c:\code\current\netgrab;$(INCLUDE);}netgrab.h \
    {c:\code\current\netgrab;$(INCLUDE);}spliturl.h \
    Netgrab.mak

.\sockets.obj: \
    c:\code\current\netgrab\sockets.c \
    {c:\code\current\netgrab;$(INCLUDE);}sockets.h \
    Netgrab.mak

.\nntp.obj: \
    c:\code\current\netgrab\nntp.c \
    {c:\code\current\netgrab;$(INCLUDE);}netgrab.h \
    {c:\code\current\netgrab;$(INCLUDE);}urlparse.h \
    {c:\code\current\netgrab;$(INCLUDE);}sockets.h \
    Netgrab.mak

.\netgrab.obj: \
    c:\code\current\netgrab\netgrab.c \
    {c:\code\current\netgrab;$(INCLUDE);}netgrab.h \
    {c:\code\current\netgrab;$(INCLUDE);}urlparse.h \
    {c:\code\current\netgrab;$(INCLUDE);}sockets.h \
    {c:\code\current\netgrab;$(INCLUDE);}spliturl.h \
    Netgrab.mak

.\urlparse.obj: \
    c:\code\current\netgrab\urlparse.c \
    {c:\code\current\netgrab;$(INCLUDE);}netgrab.h \
    {c:\code\current\netgrab;$(INCLUDE);}urlparse.h \
    Netgrab.mak
