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

#include <stdio.h>
#include <getopt.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../mpg123dc-lib/mpg123dc-lib.h"

#define MPG123DC_ACTION_NONE				0
#define MPG123DC_ACTION_PLAY				1
#define MPG123DC_ACTION_PAUSE				2
#define MPG123DC_ACTION_NEXT				3
#define MPG123DC_ACTION_PREV				4
#define MPG123DC_ACTION_STOP				5
#define MPG123DC_ACTION_STATUS				6
#define MPG123DC_ACTION_RAWSTATUS			7
#define MPG123DC_ACTION_ADDFILE				8
#define MPG123DC_ACTION_ADDDIR				9
#define MPG123DC_ACTION_CLEAR				10
#define MPG123DC_ACTION_JUMP				11
#define MPG123DC_ACTION_DUMP				12
#define MPG123DC_ACTION_DEFAULT				MPG123DC_ACTION_STATUS

struct actionlist
{
	int action;
	char*param;
	int pos;
	struct actionlist *next;
};

int fileExists(char*filename);
void usage();
void addAction(struct actionlist **alFirst, struct actionlist **alLast, int action, char*param, int pos);

int main(int argc, char**argv)
{
 	int nextPos = MPG123DC_POS_END;	
	
 	opterr=0;
	char *configFile = NULL;
	
	char*serverHost = NULL;
	int serverPort=0;

	int debug=0;

	struct actionlist *alFirst = NULL;
 	struct actionlist *alLast = NULL;
	struct actionlist *alP = NULL;

	MPG123DC myClient;
 	
	/*
		As we have no idea where the config file is, try some defaults.. 
		/etc/mpg123dc.conf
		/usr/local/etc/mpg123dc.conf
		./mpg123dc.conf
	*/

	const char def_cfgfiles[][100] = {"/etc/mpg123dc.conf", "/usr/local/etc/mpg123dc.conf", "./mpg123dc.conf"};

	#define NUM_CFGFILES  3
	for(int fileno=0; fileno < NUM_CFGFILES; fileno++)
	{
		if(fileExists((char*)def_cfgfiles[fileno]))
		{
			configFile = new char[strlen(def_cfgfiles[fileno]) + 1];
			strcpy(configFile, def_cfgfiles[fileno]);
			break;
		}
	}
	
	while(1)
	{
		static char shortOpts[] = "DC:H:P:pY:unrN:R:sSwFLf:d:cj:U:";
		static struct option longOpts[] =
		{{"config"	, 1, 0, 'C' },
		{ "debug"		, 0, 0, 'D' },
		{ "host"		, 1, 0, 'H' },
		{ "port"		, 1, 0, 'P' },
		{ "playx"		, 1, 0, 'Y' },
		{ "play"		, 0, 0, 'p' },
		{ "pause"		, 0, 0, 'u' },
		{ "next"		, 0, 0, 'n' },
		{ "prev"		, 0, 0, 'r' },
		{ "nextx"		, 1, 0, 'N' },
		{ "prevx"		, 1, 0, 'R' },
		{ "stop"		, 0, 0, 's' },
		{ "status"		, 0, 0, 'S' },
		{ "raw-status"	, 0, 0, 'w' },
		{ "first"		, 0, 0, 'F' },
		{ "last"		, 0, 0, 'L' },
		{ "addfile"		, 1, 0, 'f' },
		{ "adddir"		, 1, 0, 'd' },
		{ "clear"		, 0, 0, 'c'	},
		{ "dump"		, 1, 0, 'U'	},
		{ "jump"		, 1, 0, 'j'	},
		{ 0				, 0, 0, 0   } };

		char c = getopt_long(argc, argv, shortOpts, longOpts, NULL);

		if(c==-1) break;

		switch(c)
		{
		case 'D':
			printf("Debug mode activated...\n");
			debug=1;
   			break;
		case 'C':

			if(fileExists(optarg) != 1)
			{
				fprintf(stderr, "Failed to open specified configuration file %s!\n", optarg);
				goto CleanupExit;
			}

			if(configFile) delete configFile;
			configFile = new char[strlen(optarg)+1];
			strcpy(configFile, optarg);

			break;
		case 'H':
			if(serverHost) delete serverHost;
			serverHost = new char[strlen(optarg)+1];
			strcpy(serverHost, optarg);
			break;
		case 'P':
			serverPort = atoi(optarg);
			break;
		case 'p':
			addAction(&alFirst, &alLast, MPG123DC_ACTION_PLAY, "0", 0);
			break;
		case 'Y':
			addAction(&alFirst, &alLast, MPG123DC_ACTION_PLAY, optarg, 0);
			break;
		case 'u':
			addAction(&alFirst, &alLast, MPG123DC_ACTION_PAUSE, NULL, 0);
			break;
		case 'n':
			addAction(&alFirst, &alLast, MPG123DC_ACTION_NEXT, NULL, 0);
			break;
		case 'r':
			addAction(&alFirst, &alLast, MPG123DC_ACTION_PREV, NULL, 0);
			break;
		case 'N':
			addAction(&alFirst, &alLast, MPG123DC_ACTION_NEXT, optarg, 0);
			break;
		case 'R':
			addAction(&alFirst, &alLast, MPG123DC_ACTION_PREV, optarg, 0);
			break;
		case 's':
			addAction(&alFirst, &alLast, MPG123DC_ACTION_STOP, NULL, 0);
			break;
		case 'S':
			addAction(&alFirst, &alLast, MPG123DC_ACTION_STATUS, NULL, 0);
			break;
		case 'w':
			addAction(&alFirst, &alLast, MPG123DC_ACTION_RAWSTATUS, NULL, 0);
			break;
		case 'F':
			nextPos = MPG123DC_POS_START;
			break;
		case 'L':
			nextPos = MPG123DC_POS_END;
			break;
		case 'f':
			addAction(&alFirst, &alLast, MPG123DC_ACTION_ADDFILE, optarg, nextPos);
			break;
		case 'd':
			addAction(&alFirst, &alLast, MPG123DC_ACTION_ADDDIR, optarg, nextPos);
			break;
		case 'c':
			addAction(&alFirst, &alLast, MPG123DC_ACTION_CLEAR, NULL, 0);
			break;
		case 'j':
			addAction(&alFirst, &alLast, MPG123DC_ACTION_JUMP, optarg, 0);
			break;	
		case 'U':
			addAction(&alFirst, &alLast, MPG123DC_ACTION_DUMP, optarg, 0);
			break;
   		default:
			// Print usage
			usage();

			goto CleanupExit;
		}
	}

	if(!configFile)
	{
		if(debug)
		{
  			printf("Warning, no configuration file specified/found. Using defaults for host and port (localhost:%d), unless other is specified on command line.\n", MPG123D_DEFAULT_PORT);
		}
	}
	else
	{
		 // If no host is specified, use one from configuration file
		if(!serverHost)
		{
			serverHost = new char[256];
			myClient.GetSettingString("mpg123d", "host", "", serverHost, 255, configFile);
		}
		
		// If no port is specified, use one from configuration file
		if(!serverPort)
	 		serverPort = myClient.GetSettingInt("mpg123d", "port", serverPort, configFile);
	}
	
	// if still no host specified, use localhost
	if(!serverHost || !strlen(serverHost))
	{	
		
		if(serverHost) delete serverHost;
		serverHost = new char[10];
		strcpy(serverHost, "localhost");
	}
	
	// same her, but the default port..
	if(!serverPort)
		serverPort = MPG123D_DEFAULT_PORT;

	if(debug) printf("Setting up myClient object to use host %s and port %d...\n", serverHost, serverPort);
	
	myClient.setupSocket(serverHost, serverPort);
	
	if(!alFirst)
	{
		// Add a default, display status..
		addAction(&alFirst, &alLast, MPG123DC_ACTION_DEFAULT, NULL, 0);
	}
	
	alP = alFirst;

  	while(alP)
    {
    	switch(alP->action)
    	{
    		case MPG123DC_ACTION_PLAY:
    			myClient.player_Play(atoi(alP->param));
    			break;
			case MPG123DC_ACTION_PAUSE:
    			myClient.player_Pause();
    			break;
			case MPG123DC_ACTION_NEXT:
				if(alP->param)
    				myClient.player_Next(atoi(alP->param));
				else
					myClient.player_Next(1);
    			break;
			case MPG123DC_ACTION_PREV:
				if(alP->param)
    				myClient.player_Prev(atoi(alP->param));
				else
					myClient.player_Prev(1);
    			break;
			case MPG123DC_ACTION_STOP:
    			myClient.player_Stop();
    			break;
			case MPG123DC_ACTION_STATUS:
    			myClient.player_getState();
    			printf("Playing %s - %s..\n", myClient.lpszArtist, myClient.lpszTitle);
    			break;
			case MPG123DC_ACTION_RAWSTATUS:
    			myClient.player_getState();
					if(myClient.status != MPG123D_STATE_STOPPED)
	    			printf("%.2f\n%.2f\n%s\n%s\n%s\n%s\n%s\n%s\n%d\n%d\n", myClient.timePlayed, myClient.timeLeft, myClient.lpszArtist, myClient.lpszTitle, myClient.lpszAlbum,
       												myClient.lpszComment, myClient.lpszYear, myClient.lpszGenre, myClient.frequency, myClient.bitrate);
					else
						printf("Stopped\n");
    			break;
			case MPG123DC_ACTION_ADDFILE:
    			myClient.playlist_addFile(alP->param, alP->pos);
    			break;
			case MPG123DC_ACTION_ADDDIR:
    			myClient.playlist_addDir(alP->param, alP->pos);
    			break;
			case MPG123DC_ACTION_CLEAR:
    			myClient.playlist_Clear();
    			break;
			case MPG123DC_ACTION_JUMP:
			{
				char *p = alP->param;
				char d=0;
				
     			if(*p == '-' || *p == '+')
					d=*p++;
				
				for(char *p2 = p; *p2!=0;p2++)
				{
					if(!isdigit(*p2))
					{
						fprintf(stderr, "Invalid jump-command! Have to have the following format: \n\n"\
      									" %s -j [+|-]<number>\nor\n %s -j [+|-]<number>\n"\
           								"where number is the position in seconds.\n", argv[0], argv[0]);
						goto CleanupExit;
					}
				}

				if(d=='+')
					myClient.player_JumpForward(atoi(p));
				else if(d=='-')
					myClient.player_JumpBackward(atoi(p));
				else
					myClient.player_JumpTo(atoi(p));
				break;
			}	
			case MPG123DC_ACTION_DUMP:
    			myClient.playlist_Dump(alP->param);
    			break;
    	} 
    	
    	alP=alP->next;
	}
	
CleanupExit:
	
	// Delete all actions
	struct actionlist *alNext;
	alP = alFirst;
	
	while(alP)
   	{
   		if(alP->param) delete alP->param;
    		alNext = alP->next;
   		delete alP;
   		alP=alNext;
   	}	
	
	if(configFile) delete configFile;
	return 0;
}

// Simple function to check wheter a file exist or not
int fileExists(char*filename)
{
	int fh;
	
	fh=open(filename, O_RDONLY);
	
	if(fh == -1 && errno == ENOENT)
		return 0;
	else if(fh==-1)
		return -1;
	
	close(fh);
	
	return 1;
}

// print usage
void usage()
{
			fprintf( stderr, "Valid options:\n\n"\
			"Short      Long            Action\n"\
			"-------------------------------------------------------------------------\n"\
			"-D         --debug         Turn on debug mode\n"\
			"-C <f>     --config <f>    Use file <f> as configuration file\n"\
			"-H <host>  --host <host>   Server's host/IP\n"\
			"-P <port>  --port <port>   Server's port number\n"\
			"-p         --play          Begin play\n"\
			"-u         --pause         Pauses/Unpauses\n"\
			"-n         --next          Plays next track\n"\
			"-r         --prev          Plays prevous track\n"\
			"-N <n>     --nextx <n>     Go forward <n> tracks and play it\n"\
			"-R <n>     --prevx <n>     Go backward <n> tracks and play it\n"\
			"-s         --stop          Stops playing\n"\
			"-S         --status        Display status\n"\
			"-w         --raw-status    Display raw status info\n"\
			"-F         --first         Adds next file/dir to beginning of playlist\n"\
			"-L         --last          Adds next file/dir to end of playlist (default)\n"\
			"-f <f>     --addfile <f>   Adds file <f> to playlist\n"\
			"-d <d>     --adddir <d>    Adds directory <d> to playlist\n"\
   			"-c         --clear         Clear playlist\n"\
   			"-U <f>     --dump <f>      Dump playlist to file <f>\n"\
	      		"-j <pos>   --jump <pos>    Jump to <pos>, or jump +<pos> forward, or -<pos> back\n");
			
}

// Add action to list
void addAction(struct actionlist **alFirst, struct actionlist **alLast, int action, char*param, int pos)
{
	struct actionlist *alCurrent = new actionlist;
	alCurrent->action 	= action;
	alCurrent->pos 		= pos;
	
	if(param)
	{
		alCurrent->param = new char[strlen(param)+1];
		strcpy(alCurrent->param, param);
	}
 	else
		alCurrent->param = NULL;
	
	alCurrent->next = NULL;

	if(*alLast)
		(*alLast)->next = alCurrent;
	else if(!*alLast && *alFirst)
		(*alFirst)->next = alCurrent;
	else if(!*alLast && !*alFirst)
	  	*alFirst = alCurrent;

	*alLast = alCurrent;
}
