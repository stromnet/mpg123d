/*
	Copyright 2002 Johan Ström (jstrom@telia.com).
	
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#ifdef WIN32
// Windows just require windows.h and stdlib.h...
#include <windows.h>
#include <stdlib.h>
#else
// Linux require others
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>	// For tolower
#include <sys/time.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>	// gethostbyname

// To avoid alot of #ifdef's in code, we just replace the commands with this:
#define LPSTR  		char*
#define SOCKET		int
#define LPHOSTENT 	struct hostent *

#define lstrlen		strlen
#define lstrcpy		strcpy
#define lstrcpyn(x,y,z)	strncpy(x,y,z-1)
#define lstrcat		strcat
#define lstrcmp		strcmp
#define wsprintf	sprintf

#define INVALID_SOCKET 	-1
#define SOCKET_ERROR 	-1

#endif

class MPG123DC
{
public:
    MPG123DC::MPG123DC();
    MPG123DC::~MPG123DC();
    
    int MPG123DC::setupSocket(LPSTR lpszHost, long port);
    
    int MPG123DC::doPathTranslation(LPSTR lpszOldName, LPSTR *lpszNewName, LPSTR lpszConfigFile);

	/* Functions that simluate WIN32 GetPrivateProfileString/Int (will be called if we are on windows..) */
	int	MPG123DC::GetSettingInt(LPSTR lpAppName, LPSTR lpKeyName, int nDefault, LPSTR lpFileName);
	int MPG123DC::GetSettingString(LPSTR lpAppName, LPSTR lpKeyName, LPSTR lpDefault, LPSTR lpReturnedString, int nSize, LPSTR lpFileName);
    
    int MPG123DC::player_Play(int num);
    int MPG123DC::player_Pause();
    int MPG123DC::player_Prev(int num);
    int MPG123DC::player_Next(int num);
    int MPG123DC::player_Stop();
    int MPG123DC::player_JumpTo(long sec_pos);
    int MPG123DC::player_JumpForward(long sec_pos);
    int MPG123DC::player_JumpBackward(long sec_pos);
    
    int MPG123DC::player_getState();

    int MPG123DC::playlist_Clear();
    int MPG123DC::playlist_addFile(LPSTR lpszFilename, int nPos);
    int MPG123DC::playlist_addDir(LPSTR lpszDirname, int nPos);
    int MPG123DC::playlist_Dump(LPSTR lpszFilename);
    int MPG123DC::playlist_Load(LPSTR lpszFilename);
    
    int status;
    
    float timePlayed;
    float timeLeft;
    
    int bitrate;
    int frequency;
    
    LPSTR lpszArtist;
    LPSTR lpszTitle;
    LPSTR lpszAlbum;
    LPSTR lpszComment;
    LPSTR lpszYear;
    LPSTR lpszGenre;
    
private:
#ifdef WIN32
    int MPG123DC::initWinsock();
#endif
    int MPG123DC::sendCmd(LPSTR cmd);
    void MPG123DC::trimstring(LPSTR lpszString);
    void MPG123DC::lcasestring(LPSTR lpszString);
    
    SOCKET sUDPSocket;
    struct sockaddr_in sinRemote;
    int ready;
};


/* Commands to send to server */
#define SERVCMD_PLAYLIST_PLAY       "playlist play"
#define SERVCMD_PLAYER_PAUSE        "player pause"
#define SERVCMD_PLAYLIST_PREV       "playlist prev"
#define SERVCMD_PLAYLIST_NEXT       "playlist next"
#define SERVCMD_PLAYER_STOP         "player stop"
#define SERVCMD_PLAYER_JUMP         "player jump"
#define SERVCMD_PLAYLIST_CLEAR      "playlist clear"
#define SERVCMD_PLAYLIST_ADDFILE    "playlist addfile"
#define SERVCMD_PLAYLIST_ADDDIR     "playlist adddir"
#define SERVCMD_PLAYLIST_DUMP	    "playlist dump"
#define SERVCMD_PLAYLIST_LOAD	    "playlist load"
#define SERVCMD_PLAYER_STATUS		"player status"

#define MPG123DC_POS_START          0
#define MPG123DC_POS_END            1

#define	MPG123D_STATE_UNKNOWN		-1
#define	MPG123D_STATE_STOPPED		0
#define	MPG123D_STATE_PLAYING		1

#define MPG123D_DEFAULT_PORT		3322
