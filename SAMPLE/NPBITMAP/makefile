# makefile
#
# The actions included in this make file are:
#  Compile::C++ Compiler
#  Link::Linker

COMMON = ..\..\common
NAME = npbitmap
INCLUDEPATH = ..\..\include

.SUFFIXES: .cpp .obj .rc .res

.all: \
    $(NAME).dll

!if "$(CRT)" == "DYNAMIC"
CRT_TYPE = /Gd+
!else
CRT_TYPE = /Gd- /DSTATIC_LINK
!endif

!if "$(OPT)" == "DEBUG"
DEBUG_C_FLAGS = /DDEBUG /Ti+ /O-
DEBUG_L_FLAGS = /de /map
!else
DEBUG_C_FLAGS = /O+
DEBUG_L_FLAGS = /nodebug
!endif

#CPP_FLAGS = /Ti /Gm /Gd+ /Ge- /G5 /D_USRDLL /D_MBCS
CPP_FLAGS = $(DEBUG_C_FLAGS) $(CRT_TYPE) /Gm+ /Ge- /D_USRDLL /D_MBCS

.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe $(CPP_FLAGS) -Fo$*.obj /I$(INCLUDEPATH) /C /Q %s

.rc.res:
    @echo " Compile::Resource Compile "
    rc.exe -i $(INCLUDEPATH) -r $<

$(NAME).dll: \
    $(COMMON)\npos2.obj \
    $(NAME).obj \
    $(NAME).res\
    $(NAME).def
    @echo " Link::Linker "
    @icc.exe @<<
     /B" /nologo /noe $(DEBUG_L_FLAGS)"
     /Fe$(NAME).dll
     $(NAME).def
     $(COMMON)\npos2.obj
     $(NAME).obj
<<
  @echo " Compile::Resource Bind "
  rc $*.res $@

$(NAME).obj: \
    $(NAME).cpp \
    {.\;$(INCLUDEPATH);$(INCLUDE);}npapi.h

$(COMMON)\npos2.obj: \
    $(COMMON)\npos2.cpp \
    {.\;$(INCLUDEPATH);$(INCLUDE);}npapi.h \
    {.\;$(INCLUDEPATH);$(INCLUDE);}npupp.h

$(NAME).res: \
    $(NAME).rc
