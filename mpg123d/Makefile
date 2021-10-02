all:	mpg123d

SOURCEFILES=mpg123d.cpp settings.cpp debugger.cpp misc.cpp mpg123.cpp udpserver.cpp mp3info.cpp playlist.cpp
LIBS=-lpthread

mpg123d:
	gcc -O $(SOURCEFILES) $(LIBS) -o mpg123d

clean:
	rm -f mpg123d


