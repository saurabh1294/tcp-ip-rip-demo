{\rtf1\ansi\ansicpg1252\deff0\deflang1033{\fonttbl{\f0\fnil\fcharset0 Courier New;}{\f1\fswiss\fcharset0 Arial;}}
{\*\generator Msftedit 5.41.21.2500;}\viewkind4\uc1\pard\f0\fs20 CC= cc\par
\par
INC=-Iinclude\par
LIB=-Llib\par
CFLAGS= \par
\par
\par
OBJS = nlp.o udp.o tlp.o ftp.o \par
FOBJS = start.o $(OBJS)\par
\par
all: start\par
\par
start : $(FOBJS)\par
\tab $(CC) -o start $(FOBJS) $(FLAGS) -pthread\par
\par
start.o : start.c \par
\tab $(CC) -c start.c $(FLAGS) -pthread\par
\par
ftp.o : ftp.c\par
\tab $(CC) -c ftp.c $(FLAGS) -pthread\par
\par
tlp.o : tlp.c\par
\tab $(CC) -c tlp.c $(FLAGS) -pthread\par
\par
nlp.o : nlp.c\par
\tab $(CC) -c nlp.c $(FLAGS) -pthread\par
\par
udp.o : udp.c\par
\tab $(CC) -c udp.c $(FLAGS)\par
\par
clean :\par
\tab rm udp.o \par
\tab rm tlp.o nlp.o ftp.o\par
\tab rm start.o\par
\par
\par
\par
\f1\par
}
 