ERASE=ERASE
.SUFFIXES:
.SUFFIXES: .c .cpp

NAME = gendata

ALL: $(NAME).EXE

$(NAME).EXE:   \
  $(NAME).OBJ
   ICC.EXE @<<
  /Tdp  /B"/DE /STACK:32768"
  /Fe"$(NAME).EXE"
  $(NAME).OBJ
<<

{.}.cpp.obj:
   ICC.EXE  /Ti+ /Tdp /Gm /Gd /C  .\$*.cpp

$(NAME).obj: \
    $(NAME).cpp \
    draw.h

clean:
   $(ERASE) $(NAME).obj
   $(ERASE) $(NAME).exe
