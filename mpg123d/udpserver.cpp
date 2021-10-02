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

    $Id: udpserver.cpp,v 1.2 2004/02/03 18:03:08 johan Exp $
    $Log: udpserver.cpp,v $
    Revision 1.2  2004/02/03 18:03:08  johan
    Added Id and Log entrys in all files.


*/

#include "common.h"


#define DATAGRAMSIZE	300
	
	

/***************************************
Constructor..
****************************************/
UDPServer::UDPServer()
{
	active=0;
	sendNext=NULL;
}


/***************************************
Deconstructor.. close socket if open
****************************************/
UDPServer::~UDPServer()
{
	if(fdsocket)
		gUDPServ.closeserv();
	
	if(sendNext)
		delete sendNext;
}


/***************************************
Start socket
****************************************/
int UDPServer::start(int port)
{
	if(active) return -1;
	
	gDebugger.LogMessage(LOG_DEBUG, "Setting up UDP socket.. ");
	
	int fdtmp;
	int retval;
	
	// Create a socket
	fdtmp = socket(AF_INET, SOCK_DGRAM, 0);
	
	
	// Check it
	if(fdtmp == -1)
	{
		gDebugger.LogMessage(LOG_ERR, "Failed to create a socket! Error %m");
		return 0;
	}
	
	// Setup bind-struct
	memset(&sinLocalBind, 0, sizeof(struct sockaddr_in));
	sinLocalBind.sin_family = AF_INET;
	sinLocalBind.sin_addr.s_addr = INADDR_ANY;
	sinLocalBind.sin_port = htons(gSettings.udpport);
	
	// Bind port
	retval = bind(fdtmp, (struct sockaddr*)&sinLocalBind, sizeof(sinLocalBind));
	
	// Check for error
	if(retval == -1)
	{
		gDebugger.LogMessage(LOG_ERR, "Failed to bind to local UDP port %d.  %m", gSettings.udpport);
		close(fdtmp);
		return 0;
	}
	
	// Well, now we should have a working UDP socket
	
	fdsocket=fdtmp;
	active=1;
	return 1;
}


/***************************************
Poll port/read data/proccess data. Only
proccess one line, cause we are going in
a loop...
****************************************/
int UDPServer::checkdata()
{
	if(!active) return -1;
	
	// First poll for data..
	if(!polldata())
		return 0;
	
	// We got data
	char * buffer = new char[DATAGRAMSIZE+1];
	int retval;
	
	memset(buffer, 0, DATAGRAMSIZE);
	
	retval = readdata(buffer);
	
	if(retval == 0)	// This shouldnt happen
	{
		gDebugger.LogMessage(LOG_WARNING, "Got strange error... polldata returned 1, but readdata returns 0?");
		return 0;
	}
	
	// Remove newlines
	gMisc.trimstring(buffer);
	
	// Remove 0xff-chars at end
	char*p = strchr(buffer, 0xff);
	*p--=0;
	
	int cmdtype;
	// Check command
	if((cmdtype = checkcmd(buffer)) == -1)
	{
		gDebugger.LogMessage(LOG_WARNING, "Got invalid message from %s: %s", inet_ntoa(sinRemote.sin_addr), buffer);
		sendreply("Invalid Command", sinRemote);
		delete buffer;
		return 0;
	}
	
	gDebugger.LogMessage(LOG_INFO, "Got message %s from %s\n", buffer, inet_ntoa(sinRemote.sin_addr));
	
	char*subcmd=buffer;
	switch(cmdtype)
	{
		case 1:
			subcmd+=9;
			retval = handlePlaylistMessage(subcmd);
			break;
		case 2:
			subcmd+=7;
			retval = handlePlayerMessage(subcmd);
			break;
		case 3:
			subcmd+=7;
			retval = handleServerMessage(subcmd);
			break;
	}
	
	
	switch(retval)
	{
		case MPG123_ERR_FILENOTFOUND:
			sendreply("File or directory not found", sinRemote);
			break;
		case MPG123_RETURN_NEXTMESSAGE:
			sendreply(sendNext, sinRemote);
			delete sendNext;
			sendNext=NULL;
	}
	
	delete buffer;
}


/***************************************
Close socket..
****************************************/
int UDPServer::closeserv()
{
	if(active)
		close(fdsocket);
}

/***************************************
Tell if there is an active socket
****************************************/
int UDPServer::isactive()
{
	if(active)
		return 1;
	else
		return 0;
}

/**********************************************************************************************************
											PRIVATE FUNCTIONS
**********************************************************************************************************/

/***************************************
Check if there is any data on socket
****************************************/
int UDPServer::polldata()
{
 	fd_set readfdset;
 	struct timeval tv;
 	int retval;
 	
 	
 	FD_ZERO(&readfdset);
 	FD_SET(fdsocket, &readfdset);
 	
 	tv.tv_sec  = 0;
 	tv.tv_usec = 1;
 	
 	// Check for data
 	retval = select(fdsocket+1, &readfdset, NULL, NULL, &tv);
 	
 	if(retval == 0)
 		// Timeout
 		return 0;
 	
 	if(retval == -1 && errno != EINTR) // skip EINTR errors... happens some times now and then :P
 	{	// An error
 		gDebugger.LogMessage(LOG_ERR, "Got error %m when checking for data on UDP socket");
 		return 0;
 	}
 	
 	// Well we got data
 	return 1;
}

/***************************************
Read a line from socket
****************************************/
int UDPServer::readdata(char*buffer)
{
	int retval;
	int remotesize;
	
	memset(&sinRemote, 0, sizeof(struct sockaddr_in));
	remotesize = sizeof(struct sockaddr_in);
	
	retval = recvfrom(fdsocket, buffer, DATAGRAMSIZE, 0, (struct sockaddr*)&sinRemote, (socklen_t*)&remotesize);
	
	if(retval == DATAGRAMSIZE)	// valid data recieved
		return 1;
	else
		return 0;
}
/***************************************
Sends data to specifyed host
****************************************/
int UDPServer::sendreply(char*msg, struct sockaddr_in sinDest)
{
	int retval;
	char*tmpbuff=new char[DATAGRAMSIZE+1];
	char *tpp = tmpbuff;
	
	memset(tmpbuff, 0x0,  DATAGRAMSIZE+1);
	memset(tmpbuff, 0xff, DATAGRAMSIZE);
	
	for(char*p=msg; *p != 0;p++)
		*tpp++ = *p;
	
	retval = sendto(fdsocket, tmpbuff, strlen(tmpbuff), 0, (struct sockaddr*)&sinDest, sizeof(sinDest));
	
	gDebugger.LogMessage(LOG_DEBUG, "Sent %s to %s", msg, inet_ntoa(sinDest.sin_addr));
	
	delete tmpbuff;
	
	if(retval == DATAGRAMSIZE)	// valid data sent
		return 1;
	else
		return 0;
}


/***************************************
Check if it's a valid cmd from socket
****************************************/
int UDPServer::checkcmd(char*cmd)
{
	/*
		Valid commands
		playlist <something>
		player <something>
		server <something>
	*/
	
	if(!strncmp(cmd, "playlist", 8))
		return 1;
	if(!strncmp(cmd, "player"  , 6))
		return 2;
	if(!strncmp(cmd, "server"  , 6))
		return 3;
	
	return -1;
}


/***************************
Callback function for scandir
****************************/
int scandir_select (const struct dirent * pDir)
{
	char *p = (char*)pDir->d_name;
	
	p+=strlen(pDir->d_name) - 4;
	
	if(!strcasecmp(p, ".mp3"))
		return 1;
	else
		return 0;
}
		
		
/***************************************
Handle a playlist message
****************************************/
int UDPServer::handlePlaylistMessage(char*cmd)
{
	char*p;
	
	if(!strncmp(cmd, "addfile", 7))	// add <1|0> <file>
	{
		p=cmd+8;
		
		// pos is 1 or 0... 0 for first in list, 1 for end of list.. 
		int pos = *p=='0'?0:1;
		
		p+=2;
		
		if(!gMisc.FileExists(p))
		{
			return MPG123_ERR_FILENOTFOUND;
		}
		
		gPlayList.addEntry(p, pos);
	}else if(!strncmp(cmd, "adddir", 6))// diradd <1|0> <dir>
	{
		p=cmd+7;
		
		// pos is 1 or 0... 0 for first in list, 1 for end of list
		int pos= *p=='0'?0:1;
		
		p+=2;
		
		// alloc dir and copy dirname to it...
		char*dir = new char[strlen(p)+1];
		memset(dir,0, strlen(p)+1);
		
		strcpy(dir, p);
		
		// remove trailing / if there is any
		if(dir[strlen(dir)-1] == '/')
			dir[strlen(dir)-1] = 0;
		
		if(!gMisc.DirectoryExists(dir))
		{
			delete dir;
			return MPG123_ERR_FILENOTFOUND;
		}
		
		struct dirent ** deFiles;
		int numfound;
		
		gDebugger.LogMessage(LOG_INFO, "Scanning dir %s for .mp3-files", dir);
		
		numfound = scandir(dir, &deFiles, scandir_select, alphasort);
		
		if(numfound<0)
			gDebugger.LogMessage(LOG_ERR, "scandir failed with error %m");
		else
		{
			char*fbuff;			
			/*
				If we wan't to add the tracks at the begining of
				the playlist (that is, pos is 0), we have to add them in reversed order...
			
			*/
			
			if(pos)
			{	
				/* Add regulary at end of list.. */
				for(int fileno=0; fileno<numfound; fileno++) 
				{	
					int len = strlen(dir) + strlen(deFiles[fileno]->d_name)+2;
					fbuff = new char[len];
					memset(fbuff, 0, len);
					snprintf(fbuff,len, "%s/%s", dir, deFiles[fileno]->d_name);
					
					free (deFiles[fileno]);
					
					gPlayList.addEntry(fbuff, 1);
					
					delete fbuff;
				}
			}
			else
			{
				/* Do a reverse enumeration.. */
				while(numfound--)
				{	
					int len = strlen(dir) + strlen(deFiles[numfound]->d_name)+2;
					fbuff = new char[len];
					memset(fbuff, 0, len);
					snprintf(fbuff,len, "%s/%s", dir, deFiles[numfound]->d_name);
					
					free (deFiles[numfound]);
					
					gPlayList.addEntry(fbuff, 0);
					
					delete fbuff;
				}
			}
			free (deFiles);
		}
		
		delete dir;
	}else if(!strncmp(cmd, "play", 4))
	{
		int pos = -1;
		p=cmd+=5;
		
		pos=atoi(p);
		
		if(pos!=0)
			gPlayList.current_entry =  gPlayList.getEntryNo(pos);
			
		gPlayList.playCurrent();
	}else if(!strncmp(cmd, "dump", 4))
	{
		p=cmd+=5;
		
		gDebugger.LogMessage(LOG_INFO, "Dumping playlist to file %s", p);
		
		FILE *hFile;
		hFile = fopen(p, "w");
		
		if(!hFile)
		{
			
			gDebugger.LogMessage(LOG_ERR, "Failed to dump playlist to %s. %m", p);
			return 1;
		}
		
		// Go thru list and print to file..
		PlayList_Entry *pEntr = gPlayList.first_entry;
		
		while(pEntr)
		{
			fprintf(hFile, "%s\n", pEntr->file);
			pEntr = pEntr->next_entry;
		}
		
		fclose(hFile);
		gDebugger.LogMessage(LOG_INFO, "Dump done");
	}else if(!strncmp(cmd, "load", 4))
	{
		p=cmd+=5;
		
		if(!gMisc.FileExists(p))
			return MPG123_ERR_FILENOTFOUND;
		
		gDebugger.LogMessage(LOG_INFO, "Loading playlist from %s", p);
		
		FILE *hFile;
		hFile = fopen(p, "r");
		
		if(!hFile)
		{
			gDebugger.LogMessage(LOG_ERR, "Failed to open playlist %s. %m", p);
			return 1;
		}
		
		char*buffer = new char[512];
		
		while(!feof(hFile))
		{
			memset(buffer, 0, 512);
			fgets(buffer, 512, hFile);
			
			// remove any newlines
			gMisc.trimstring(buffer);
			
			if(!gMisc.FileExists(buffer))
			{
				gDebugger.LogMessage(LOG_ERR, "File %s specified in playlist %d doesn't exist", buffer, p);
			}
			
			gPlayList.addEntry(buffer, 1);	// always add to end of list..
		}
		
		delete buffer;
		
		fclose(hFile);
		gDebugger.LogMessage(LOG_INFO, "Load done");
	}else if(!strncmp(cmd, "prev", 4))
	{
		p=cmd+5;
		
		if(p)
			gPlayList.playPrev(atoi(p));
		else
			gPlayList.playPrev(1);
	}else if(!strncmp(cmd, "next", 4))
	{
		p=cmd+5;
		
		if(p)
			gPlayList.playNext(atoi(p));
		else
			gPlayList.playNext(1);
	}else if(!strncmp(cmd, "clear", 5))
	{
		gPlayList.clearPlayList();
	}
}

/***************************************
Handle a player message
****************************************/
int UDPServer::handlePlayerMessage(char*cmd)
{
	if(!strncmp(cmd, "pause", 5))
		gMPG123.sendCmd("pause");
	else if(!strncmp(cmd, "stop", 4))
	{
		gMPG123.manual_stopped = 1;
		gMPG123.sendCmd("stop");
	}
	else if(!strncmp(cmd, "status", 6))
	{
		/*
			Get status of player
			
			return value is either Stopped if its stopped, or:
			
			Timeinfo, exactly 10 bytes
			time left, exactly 10 bytes
			artist, exactly 30 bytes
			title, exactly 30 bytes
			album, exactly 30 bytes
			comment, exactly 30 bytes
			year, exactly 4 bytes
			genre
		*/
		
		if(!sendNext) sendNext = new char[255];
		
		if(gMPG123.status != MPG123_STATUS_STOPPED)
		{
			MP3Info*info = &gMPG123.CurrentPlaying;
			
			char*timePlayed = new char[11];
			char*timeLeft = new char[11];
			char*frequency = new char[11];
			char*bitrate = new char[11];
			sprintf(timePlayed, "%.2f", info->dur_seconds);
			sprintf(timeLeft, "%.2f", info->dur_seconds_left);
			sprintf(frequency, "%d", info->s_sampling_freq);
			sprintf(bitrate, "%d", info->s_bitrate);
			
			snprintf(sendNext, 255, "%-10s%-10s%-10s%-10s%-30s%-30s%-30s%-30s%-4s%s", timePlayed, timeLeft, frequency, bitrate, info->artist, info->title, info->album, info->comment, info->year, info->genre);
			
			delete timePlayed;
			delete timeLeft;
			delete frequency;
			delete bitrate;
		}else
			sprintf(sendNext, "Stopped");
		
		return MPG123_RETURN_NEXTMESSAGE;
	}
	else if(!strncmp(cmd, "jump", 4))
		gMPG123.sendCmd(cmd);
	else
		gDebugger.LogMessage(LOG_ERR, "Got invalid player-message: %s", cmd);
}

/***************************************
Handle a server message
****************************************/
int UDPServer::handleServerMessage(char*cmd)
{
	if(!strncmp(cmd, "quit", 4))
	{
		gDebugger.LogMessage(LOG_ERR, "Client %s told me to quit", inet_ntoa(sinRemote.sin_addr));	// log as ERR, always visible
		gMPG123.doexit=1;
		gMPG123.stop();
	}
	else
		gDebugger.LogMessage(LOG_ERR, "Got invalid server-message: %s", cmd);
}
	
