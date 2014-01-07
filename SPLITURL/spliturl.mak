# spliturl.mak
# Created by IBM WorkFrame/2 MakeMake at 17:11:53 on 11 Sept 1997
#
# The actions included in this make file are:
#  Compile::C++ Compiler
#  Link::Linker

.SUFFIXES: .c .obj

.all: \
    .\spliturl.exe

.c.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /DOS2 /Sm /Ss /Q  /Ti /O+ /Op+ /Tx- /W3 /C /Gm+ /N10 /Gd /C /Gh /Fb %s

{E:\code\current\spliturl}.c.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /DOS2 /Sm /Ss /Q  /Ti /O+ /Op+ /Tx- /W3 /C /Gm+ /N10 /Gd /C /Gh /Fb %s

.\spliturl.exe: \
    .\main.obj \
    .\spliturl.obj \
    .\btree.obj \
    spliturl.mak
    @echo " Link::Linker "
    icc.exe @<<
     /B" /de /br /noe"
     /Fespliturl.exe
     CPPOPA3.OBJ
     .\main.obj
     .\spliturl.obj
     .\btree.obj
<<

.\main.obj: \
    c:\code\current\spliturl\main.c \
    {c:\code\current\spliturl;$(INCLUDE);}main.h \
    {c:\code\current\spliturl;$(INCLUDE);}spliturl.h \
    {c:\code\current\spliturl;$(INCLUDE);}btree.h \
    spliturl.mak

.\btree.obj: \
    c:\code\current\spliturl\btree.c \
    {c:\code\current\spliturl;$(INCLUDE);}main.h \
    {c:\code\current\spliturl;$(INCLUDE);}btree.h \
    spliturl.mak

.\spliturl.obj: \
    c:\code\current\spliturl\spliturl.c \
    {c:\code\current\spliturl;$(INCLUDE);}main.h \
    {c:\code\current\spliturl;$(INCLUDE);}spliturl.h \
    spliturl.mak
