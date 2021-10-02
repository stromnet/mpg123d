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


#include "mpg123dc-lib.h"

/* Constructor */
MPG123DC::MPG123DC()
{
#ifdef WIN32
    initWinsock();
#endif

    sUDPSocket = INVALID_SOCKET;
    ready = 0;
    
	status = MPG123D_STATE_UNKNOWN;
	
	bitrate    = 0;
	frequency  = 0;
	
    lpszArtist = NULL;
    lpszTitle = NULL;
    lpszAlbum = NULL;
    lpszComment = NULL;
    lpszYear = NULL;
    lpszGenre = NULL;
}

/* Deconstructor */
MPG123DC::~MPG123DC()
{
#ifndef WIN32
    if(sUDPSocket)
        close(sUDPSocket);
#endif
	
    if(lpszArtist) delete lpszArtist;
    if(lpszTitle) delete lpszTitle;
    if(lpszAlbum) delete lpszAlbum;
    if(lpszComment) delete lpszComment;
    if(lpszYear) delete lpszYear;
    if(lpszGenre) delete lpszGenre;

#ifdef WIN32
	WSACleanup();
#endif
}

/* Call that setup's socket */
int MPG123DC::setupSocket(LPSTR lpszHost, long port)
{
    unsigned long inaddr;
    
    // Try to convert to IP
    inaddr = inet_addr(lpszHost);

    // Check result
    if(inaddr == INADDR_NONE)
    {
        // Try with gethostbyaddr
        LPHOSTENT lpHostEnt;
        lpHostEnt = gethostbyname(lpszHost);
        
        // Check result, return 1 on error
        if(lpHostEnt == NULL)
                return 1;
  
        // move to inaddr
        inaddr=*((unsigned long *) (lpHostEnt)->h_addr);
    }
    
    // by now we should have a valid IP... in inaddr
    
    // empty sinRemote
    memset(&sinRemote,0, sizeof(struct sockaddr_in));
    
    // Setup sinRemote
    sinRemote.sin_family = AF_INET;
    sinRemote.sin_addr.s_addr = inaddr;
    sinRemote.sin_port = htons(port);

    // Create socket
    sUDPSocket = socket(AF_INET, SOCK_DGRAM, 0);
    
    // Check result, return 1 on error
    if(sUDPSocket == INVALID_SOCKET)
        return 1;
    
    ready = 1;
    // Now we are ready for sending data... 
    return 0;    
}

/*

Takes a path, and use directives in lpszConfigFile to translate path, and put it in lpszNewName (that should be unallocated)

CALLER IS RESPONSIBLE FOR delete lpszNewName IF ALLOCATED

*/
int MPG123DC::doPathTranslation(LPSTR lpszOldName, LPSTR *lpszNewName, LPSTR lpszConfigFile)
{
    // if lpszNewName is already initialized, die
    if(*lpszNewName) return 1;
    
    int iNumTranslations;
    iNumTranslations = GetSettingInt("PathTranslator", "NumTranslates", 0, lpszConfigFile);
    
    LPSTR lpszNewPath = new char[lstrlen(lpszOldName)+255]; // give generous place...
     
    if(iNumTranslations)
    {
        LPSTR lpszTranslatorLocal = new char[255];
        LPSTR lpszTranslatorRemote = new char[255];
        LPSTR lpszKeyName = new char[10];
        
        // empty and copy current string..
        memset(lpszNewPath, 0, lstrlen(lpszOldName)+255);
        lstrcpy(lpszNewPath, lpszOldName);
        
        for(int iTransl=1; iTransl <= iNumTranslations; iTransl++)
        {
            memset(lpszTranslatorLocal, 0, 255);
            memset(lpszTranslatorRemote, 0, 255);
            
            // Read local and remote from configuraton file
            wsprintf(lpszKeyName, "Local%d", iTransl);
            GetSettingString("PathTranslator", lpszKeyName, "", lpszTranslatorLocal, 255, lpszConfigFile);
            
            wsprintf(lpszKeyName, "Remote%d", iTransl);
            GetSettingString("PathTranslator", lpszKeyName, "", lpszTranslatorRemote, 255, lpszConfigFile);
            
            // Find the path..
            LPSTR p = strstr(lpszNewPath, lpszTranslatorLocal);
            if(p)
            {  
                // found..
                LPSTR lpszTemp = new char[lstrlen(p)];  // alocate for memory for the string that is at pos p+lstrlen(lpszTranslatorLocal)
                LPSTR p2 = p+lstrlen(lpszTranslatorLocal);
                
                // move content in p2 to lpszTemp
                lstrcpy(lpszTemp, p2);
                
                // Set *p to 0 (that is, nullterminate the string away :P)
                *p=0;
                
                // add lpszTranslatorRemote
                lstrcat(lpszNewPath, lpszTranslatorRemote);
                
                // add lpszTemp
                lstrcat(lpszNewPath, lpszTemp);
                
                delete lpszTemp;
            }
        }
        
        delete lpszTranslatorLocal;
        delete lpszTranslatorRemote;
        delete lpszKeyName;
    }else
        // no specific stuff done.. use same
        lstrcpy(lpszNewPath, lpszOldName);
    
    // now, translate \ to /
    for(LPSTR p=lpszNewPath; *p != 0; p++)
    {
        if(*p == '\\')
            *p='/';
    }

    // Set new path..
    *lpszNewName = lpszNewPath;
}

/* like windows GetPrivateProfileInt (will be called if we are on windows..) */
int	MPG123DC::GetSettingInt(LPSTR lpAppName, LPSTR lpKeyName, int nDefault, LPSTR lpFileName)
{
#ifdef WIN32
	// Use windows regular function
	return GetPrivateProfileInt(lpAppName, lpKeyName, nDefault, lpFileName);
#endif
	
	// Just call GetSettingString..
	char *buff = new char[255];
	char *lpDefault = new char[10];
	
	memset(buff, 0, 255);
	wsprintf(lpDefault, "%d", nDefault);
	
	GetSettingString(lpAppName, lpKeyName, lpDefault, buff, 255, lpFileName);
	
	int value = atoi(buff);
	
 	delete lpDefault;
	delete buff;
	
	return value;
}

/* like windows GetPrivateProfileString (will be called if we are on windows..) */
int MPG123DC::GetSettingString(LPSTR lpAppName, LPSTR lpKeyName, LPSTR lpDefault, LPSTR lpReturnedString, int nSize, LPSTR lpFileName)
{
#ifdef WIN32
	// Use windows regular function
	return GetPrivateProfileString(lpAppName, lpKeyName, lpDefault, lpReturnedString, nSize, lpFileName);
#else
	
	/*	Open lpFileName as stream */
	
	FILE* fstream;
	
	fstream = fopen(lpFileName, "r");
	
	if(!fstream)
		return -1;
  	
	/* Setup buffers */
	#define READBUFF_SIZE	512
	#define BUFF_SIZE		512
	
	char * readBuff = new char[READBUFF_SIZE];
	char * directive= new char[BUFF_SIZE];
	char * value 	= new char[BUFF_SIZE];
	
	int cfg_line_no=0;
	int foundAppName = 0;
	
	/* Start loopin throu the file */
	while(!feof(fstream))
	{
		// Fill with \0
		memset(readBuff,0,READBUFF_SIZE);
		
		// Read a line or max READBUFF_SIZE chars
		fgets(readBuff, READBUFF_SIZE, fstream);
		cfg_line_no++;
		
		// Check for any newlines (shouldnt be any?), and replace with \0
		for(char*p=readBuff;*p!=0;p++)		if(*p=='\n' || *p=='\r') *p=0;
		
		// Ignore empty lines
		if(!strlen(readBuff)) continue;
		
		// Remove all empty spaces
		trimstring(readBuff);
		
		// Check for comment
		if(readBuff[0] == '#') continue;
		
		// Make a basic check, that we have an '=' in the string.
		if(!strchr(readBuff, '='))
		{
			// if we DONT, check for [<appname>]
			if(!foundAppName)
			{
				if(readBuff[0] == '[' && readBuff[strlen(readBuff) - 1] == ']')
				{
					// It is an [xx] row..
					char*p=readBuff+1;
					if(!strncmp(p, lpAppName, strlen(p)-1))
						foundAppName = 1;
				}
			}
			
			continue;
   		}
		
		// Zero variables
		memset(directive, 0, BUFF_SIZE);
		memset(value, 0, BUFF_SIZE);
		
		/* Now split the directive, and the value.. Should be delimited with an = (well, if there were no = in the line, it was skipped :P)  */
		for(char *p = readBuff; p != 0; p++)
		{
			// Check if it's the delimiter
			if(*p == '=')
			{
				// Copy data
				strncpy(directive, readBuff, (p-readBuff) );
				strcpy (value, ++p);
				
				// Get rid of whitespaces
				trimstring(directive);
				trimstring(value);
				
				// Lower case name (we should be case insensitive)
				lcasestring(directive);
				
				// Get outa loop
				break;
			}
		}
		
		// Now check if it's the key we are looking for
		if(!strcmp(lpKeyName, directive) && foundAppName)
		{
			memset(lpReturnedString, 0, nSize);
			strncpy(lpReturnedString, value, nSize-1);
			return strlen(value);
		}
	}
	
	memset(lpReturnedString, 0, nSize);
	if(lpDefault)
	{
		strncpy(lpReturnedString, lpDefault, nSize-1);
		return strlen(lpDefault);
	}else
		return 0;
#endif
}


/***********************************************************************
    Begin Controling Functions
***********************************************************************/

/* Tell server to play: num=0: current track, num>0: track <num> in playlist*/
int MPG123DC::player_Play(int num)
{
    if(!ready) return 1;
    
    size_t nBuffLen = lstrlen(SERVCMD_PLAYLIST_PLAY) + 20;
    
    int retval;
    LPSTR lpszBuffer = new char[nBuffLen];
    
    memset(lpszBuffer, 0, nBuffLen);
    
    wsprintf(lpszBuffer, "%s %d", SERVCMD_PLAYLIST_PLAY, num);
    
    retval = sendCmd(lpszBuffer);
    
    delete lpszBuffer;
    return retval;
}

/* Tell server to pause/unpause playback */
int MPG123DC::player_Pause()
{
    if(!ready) return 1;
    return sendCmd(SERVCMD_PLAYER_PAUSE);
}

/* Tell server to play prevous track */
int MPG123DC::player_Prev(int num)
{
    if(!ready) return 1;
    
    size_t nBuffLen = lstrlen(SERVCMD_PLAYLIST_PREV) + 10;
    
    int retval;
    LPSTR lpszBuffer = new char[nBuffLen];
    
    memset(lpszBuffer, 0, nBuffLen);
    
    wsprintf(lpszBuffer, "%s %d", SERVCMD_PLAYLIST_PREV, num);
    
    retval = sendCmd(lpszBuffer);
    
    delete lpszBuffer;
    return retval;
}

/* Tell server to play next track */
int MPG123DC::player_Next(int num)
{
	if(!ready) return 1;
    
    size_t nBuffLen = lstrlen(SERVCMD_PLAYLIST_NEXT) + 10;
    
    int retval;
    LPSTR lpszBuffer = new char[nBuffLen];
    
    memset(lpszBuffer, 0, nBuffLen);
    
    wsprintf(lpszBuffer, "%s %d", SERVCMD_PLAYLIST_NEXT, num);
    
    retval = sendCmd(lpszBuffer);
    
    delete lpszBuffer;
    return retval;
}

/* Tell server to stop playing */
int MPG123DC::player_Stop()
{
    if(!ready) return 1;
    return sendCmd(SERVCMD_PLAYER_STOP);
}

/* Tell server to jump to position pos */
int MPG123DC::player_JumpTo(long sec_pos)
{
    if(!ready) return 1;
    
    size_t nBuffLen = lstrlen(SERVCMD_PLAYER_JUMP) + 20;
    int retval;
    LPSTR lpszBuffer = new char[nBuffLen];
    
    memset(lpszBuffer, 0, nBuffLen);
    
    wsprintf(lpszBuffer, "%s %d", SERVCMD_PLAYER_JUMP, sec_pos*38);
    
    retval = sendCmd(lpszBuffer);
    
    delete lpszBuffer;
    return retval;
}

/* Tell server to jump sec_pos seconds forward */
int MPG123DC::player_JumpForward(long sec_pos)
{
    if(!ready) return 1;
    
    int nBuffLen = lstrlen(SERVCMD_PLAYER_JUMP) + 20;
    int retval;
    LPSTR lpszBuffer = new char[nBuffLen];
    
    memset(lpszBuffer, 0, nBuffLen);
    
    wsprintf(lpszBuffer, "%s +%d", SERVCMD_PLAYER_JUMP, sec_pos*38);
    
    retval = sendCmd(lpszBuffer);
    
    delete lpszBuffer;
    return retval;
}

/* Tell server to jump sec_pos seconds backward */
int MPG123DC::player_JumpBackward(long sec_pos)
{
    if(!ready) return 1;
    
    int nBuffLen = lstrlen(SERVCMD_PLAYER_JUMP) + 20;
    int retval;
    LPSTR lpszBuffer = new char[nBuffLen];
    
    memset(lpszBuffer, 0, nBuffLen);
    
    wsprintf(lpszBuffer, "%s -%d", SERVCMD_PLAYER_JUMP, sec_pos*38);
    
    retval = sendCmd(lpszBuffer);
    
    delete lpszBuffer;
    return retval;
}

/* Retrieve stats */
int MPG123DC::player_getState()
{
	if(!ready) return 0;
	
	sendCmd(SERVCMD_PLAYER_STATUS);

	// Now, we have to wait for data..
	struct timeval tvTimeout;
	fd_set fdRead;

	tvTimeout.tv_sec=3;
	tvTimeout.tv_usec=0;

 	FD_ZERO(&fdRead);
 	FD_SET(sUDPSocket, &fdRead);

	int retval;
	
	retval = select(sUDPSocket+1, &fdRead, NULL, NULL, &tvTimeout);
	
	if(retval == 0)
		// No data recieved.. 
		return 0;
	else if(retval==SOCKET_ERROR)
		return -1;
	else
	{
		LPSTR lpszBuff = new char[301];	// We will recieve a cmd just like we send em
		struct sockaddr_in sinServ;
		memset(&sinServ, 0, sizeof (struct sockaddr_in));
		int servSize = sizeof(sinServ);
		
#ifdef WIN32
		int len = recvfrom(sUDPSocket, lpszBuff, 300, 0, (struct sockaddr*)&sinServ, &servSize);
#else
		int len = recvfrom(sUDPSocket, lpszBuff, 300, 0, (struct sockaddr*)&sinServ, (socklen_t*)&servSize);
#endif
		
		if(len==300)
		{
  			// Change first 0xff to 0x0
			LPSTR x = strchr(lpszBuff, 0xff);
			*x=0;
			
			if(!lstrcmp(lpszBuff, "Stopped"))
			{
				status=MPG123D_STATE_STOPPED;
    			
       			if(lpszArtist) delete lpszArtist;
				if(lpszTitle) delete lpszTitle;
				if(lpszAlbum) delete lpszAlbum;
				if(lpszComment) delete lpszComment;
				if(lpszYear) delete lpszYear;
		  		if(lpszGenre) delete lpszGenre;
		  		timePlayed = 0;
		  		timeLeft   = 0;
		  		bitrate    = 0;
		  		frequency  = 0;
			}
			else
			{
				status=MPG123D_STATE_PLAYING;
		  		LPSTR lpszTemp = new char[255];
		        LPSTR p=lpszBuff;
				
				if(lpszArtist) delete lpszArtist;
				if(lpszTitle) delete lpszTitle;
				if(lpszAlbum) delete lpszAlbum;
				if(lpszComment) delete lpszComment;
				if(lpszYear) delete lpszYear;
		  		if(lpszGenre) delete lpszGenre;
		
				lpszArtist = new char[31];
				lpszTitle = new char[31];
				lpszAlbum = new char[31];
				lpszComment = new char[31];
		  		lpszYear = new char[5];
		  		
      			memset(lpszArtist, 0, 31);
				memset(lpszTitle, 0, 31);
				memset(lpszAlbum, 0, 31);
				memset(lpszComment, 0, 31);
				memset(lpszYear, 0, 5);
				
    			// Get time played, 10 chars
				lstrcpyn(lpszTemp, p, 11);
				timePlayed = atof(lpszTemp);
				p+=10;
				
				// Get time left, 10 chars
				lstrcpyn(lpszTemp, p, 11);
				timeLeft = atof(lpszTemp);
				p+=10;

				// Get frequency, 10 chars
				lstrcpyn(lpszTemp, p, 11);
				frequency = atoi(lpszTemp);
				p+=10;
				
    			// Get bitrate, 10 chars
				lstrcpyn(lpszTemp, p, 11);
				bitrate = atoi(lpszTemp);
				p+=10;
				
		  		lstrcpyn(lpszArtist, 	p, 31);		p+=30;
		  		trimstring(lpszArtist);
		  		
      			lstrcpyn(lpszTitle, 	p, 31);		p+=30;
		  		trimstring(lpszTitle);
		  		
		  		lstrcpyn(lpszAlbum, 	p, 31);		p+=30;
		  		trimstring(lpszAlbum);
		  		
      			lstrcpyn(lpszComment, 	p, 31);		p+=30;
		  		trimstring(lpszComment);
		  		
      			lstrcpyn(lpszYear, 		p, 5);		p+=4;
		  		trimstring(lpszYear);
		  		
		  		lpszGenre = new char[strlen(p)+1];
				memset(lpszGenre, 0, strlen(p)+1);
		    	lstrcpy(lpszGenre, p);
		    	trimstring(lpszGenre);
		    	
		    	delete lpszTemp;
			}
		}
		
  		delete lpszBuff;
		
		return 1;
	}
	
    return 0;
}

/* Tell server to clear playlist */
int MPG123DC::playlist_Clear()
{
    if(!ready) return 1;
    return sendCmd(SERVCMD_PLAYLIST_CLEAR);
}

/* Tell server to add a file */
int MPG123DC::playlist_addFile(LPSTR lpszFilename, int nPos)
{
    if(!ready) return 1;
    
    int nBuffLen = lstrlen(SERVCMD_PLAYLIST_ADDFILE) + lstrlen(lpszFilename) + 20;
    int retval;
    LPSTR lpszBuffer = new char[nBuffLen];
    
    memset(lpszBuffer, 0, nBuffLen);
    
    wsprintf(lpszBuffer, "%s %d %s", SERVCMD_PLAYLIST_ADDFILE, (nPos==MPG123DC_POS_START)?MPG123DC_POS_START:MPG123DC_POS_END, lpszFilename);
    
    retval = sendCmd(lpszBuffer);
    
    delete lpszBuffer;
    return retval;
}

/* Tell server to add a directory */
int MPG123DC::playlist_addDir(LPSTR lpszDirname, int nPos)
{
    if(!ready) return 1;
    
    int nBuffLen = lstrlen(SERVCMD_PLAYLIST_ADDDIR) + lstrlen(lpszDirname) + 20;
    int retval;
    LPSTR lpszBuffer = new char[nBuffLen];
    
    memset(lpszBuffer, 0, nBuffLen);
    
    wsprintf(lpszBuffer, "%s %d %s", SERVCMD_PLAYLIST_ADDDIR, (nPos==MPG123DC_POS_START)?MPG123DC_POS_START:MPG123DC_POS_END, lpszDirname);
    
    retval = sendCmd(lpszBuffer);
    
    delete lpszBuffer;
    return retval;
}

int MPG123DC::playlist_Dump(LPSTR lpszFilename)
{
	if(!ready) return 1;
    
    int nBuffLen = lstrlen(SERVCMD_PLAYLIST_DUMP) + lstrlen(lpszFilename) + 20;
    int retval;
    LPSTR lpszBuffer = new char[nBuffLen];
    
    memset(lpszBuffer, 0, nBuffLen);
    
    wsprintf(lpszBuffer, "%s %s", SERVCMD_PLAYLIST_DUMP, lpszFilename);
    
    retval = sendCmd(lpszBuffer);
    
    delete lpszBuffer;
    return retval;
}

int MPG123DC::playlist_Load(LPSTR lpszFilename)
{
	if(!ready) return 1;
    
    int nBuffLen = lstrlen(SERVCMD_PLAYLIST_LOAD) + lstrlen(lpszFilename) + 20;
    int retval;
    LPSTR lpszBuffer = new char[nBuffLen];
    
    memset(lpszBuffer, 0, nBuffLen);
    
    wsprintf(lpszBuffer, "%s %s", SERVCMD_PLAYLIST_LOAD, lpszFilename);
    
    retval = sendCmd(lpszBuffer);
    
    delete lpszBuffer;
    return retval;
}

/****************************************************************************

Begin of private functions

****************************************************************************/

#ifdef WIN32
/* Simple function that just calls WSAStatup, only compiled on Windows */
int MPG123DC::initWinsock()
{
    WSADATA wsaData;
    int retval;
    
    retval = WSAStartup(MAKEWORD(2,2), &wsaData);
    
    if(retval)
        return 1;
    else
        return 0;    
}
#endif

/* Sends an command to the server */
int MPG123DC::sendCmd(LPSTR cmd)
{
    // Check if we have a valid connection..
    if(!ready) return 1;
    
    // Check command length.. our protocol can 
    // handle up to 300 bytes
    if(lstrlen(cmd) > 300) return 1;
    
    // Allocate memory
    LPSTR lpszFormated_Command = new char[301];
    
    // Fill with 0x0
    memset(lpszFormated_Command, 0, 0x0);
        // Copy command
    lstrcpy(lpszFormated_Command, cmd);
    
    // Fill rest with 0xff
    for(int i = lstrlen(cmd); i <= 300; i++)
        lpszFormated_Command[i] = 0xff;
    
    // Just make sure last char is a 0x0
    lpszFormated_Command[300] = 0x0;
    
    // Send command
    int retval = sendto(sUDPSocket, lpszFormated_Command, lstrlen(lpszFormated_Command), 0, (struct sockaddr*) &sinRemote, sizeof(sinRemote));

    // Delete buffer
    delete lpszFormated_Command;
    
    // Do a litle sleep here :P
#ifdef WIN32
    Sleep(500);
#else
	usleep(500);
#endif
    // Check for errors, return 1 for error and 0 for OK
    if(retval == SOCKET_ERROR)
        return 1;
    else
        return 0;
}

/* trimstring */
void MPG123DC::trimstring(LPSTR lpszString)
{
	LPSTR p = lpszString;

	while(*p == ' ' || *p == '\t')
    	p++;

    lstrcpy(lpszString, p);

	p=lpszString+strlen(lpszString)-1;
	
	while(p >= lpszString && ( *p==' ' || *p=='\t' ))
		*p--=0;
}

void MPG123DC::lcasestring(LPSTR lpszString)
{
#ifdef WIN32
	CharLower(lpszString);
#else
	for(LPSTR p = lpszString; *p!=0; p++)
		*p = tolower(*p);
#endif
}
