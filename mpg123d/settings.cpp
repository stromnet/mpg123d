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

    $Id: settings.cpp,v 1.3 2004/02/03 18:13:29 johan Exp $
    $Log: settings.cpp,v $
    Revision 1.3  2004/02/03 18:13:29  johan
    Enabled -v switch to show version number.

    Revision 1.2  2004/02/03 18:03:08  johan
    Added Id and Log entrys in all files.


*/

#include "common.h"

/************************************************
		PUBLIC FUNTIONS
*************************************************/


/**************************************
Initializing
***************************************/
Settings::Settings()
{
	//gDebugger.LogMessage(LOG_DEBUG, "Entering Settings constructor");
	debuglevel=0;
	cfgfile=NULL;
	mpg123exec = NULL;
	udpport = DEFAULTPORT;
	nodaemon=0;
	
	gDebugger.ChangeLogMask(debuglevel);
	
	//gDebugger.LogMessage(LOG_DEBUG, "Leaving Settings constructor");
	
}


/**************************************
Cleanup
***************************************/
Settings::~Settings()
{
	//gDebugger.LogMessage(LOG_DEBUG, "Entering Settings deconstructor");
	if(cfgfile) delete cfgfile;
	if(mpg123exec) delete mpg123exec;
	//gDebugger.LogMessage(LOG_DEBUG, "Leaving Settings deconstructor");
}


/**************************************
Parse command line options
***************************************/
int Settings::parseArgv(int argc, char**argv, char*version)
{
	#define ACCEPTED_PARAMETERS "d:c:nv"
	
	char c;
	opterr = 0;
	gDebugger.LogMessage(LOG_DEBUG, "Entering Settings::parseArgv with %d arguments", argc);
	
	while((c=getopt(argc, argv, ACCEPTED_PARAMETERS)) != -1)
	{
		switch(c)
		{
		case 'd':
			gSettings.setDebug(atoi(optarg));
			break;
		case 'c':
			if(!gSettings.setConfigFile(optarg))
				return 0;
			
			break;
		case 'n':
			nodaemon = 1;
			break;
		case 'v':
			printf("mpg123dc version %s\n\n", version);
		default:
			/* Print usage information and exit */
			/* This is the ONLY case in this program that we use printf!! ALL other info will be logged to syslog */
			printf("usage: %s [-d level] [-c configuration file] [-n]\n", argv[0]);
			printf("-d level\t\tSet debuglevel to use. 0-3. 0=Just errors, 1+=warnings and notices, 2+=info, 3+=debug\n");
			printf("-c filename\t\tConfiguration file to read\n");
			printf("-n\t\t\tDon't go into background\n");
			return 0;
		}
	}
	
	return 1;	
}

/**************************************
Set debuglevel
***************************************/
int Settings::setDebug(int level)
{
	int old = debuglevel;
	
	if(level < 0) level=0;
	if(level > 3) level=3;
	
	debuglevel = level;
	
	gDebugger.ChangeLogMask(level);
	
	return old;
}

/**************************************
Set configuration file
***************************************/
int Settings::setConfigFile(char*filename)
{
	size_t datalen = strlen(filename);
	gDebugger.LogMessage(LOG_NOTICE, "Changing configuration file to %s", filename);
	
	// Check if file exists
	if(!gMisc.FileExists(filename))
	{
		gDebugger.LogMessage(LOG_ERR, "Configuration file %s doesn't exist!", filename);
		return 0; // return error
	}
	
	// File exists. 
	
	// Remove old buffer if it exists..
	if(cfgfile) delete cfgfile;
	
	// Allocate buffer space
	cfgfile = new char[datalen+1];
	
	// fill with \0
	memset(cfgfile, 0, datalen+1);
	
	// Copy data
	strncpy(cfgfile, filename, datalen);
	
	return 1;
} 

/**************************************
Set mpg123-exectubale
***************************************/
int Settings::setMPG123(char*filename)
{
	size_t datalen = strlen(filename);
	gDebugger.LogMessage(LOG_DEBUG, "Changing mpg123exec to %s", filename);
	
	// Check if file exists
	if(!gMisc.FileExists(filename))
	{
		gDebugger.LogMessage(LOG_ERR, "Configuration file says mpg213 is located at %s, but I can't find it!", filename);
		return 0; // return error
	}
	
	// File exists. 
	
	// Remove old buffer if it exists..
	if(mpg123exec) delete mpg123exec;
	
	// Allocate buffer space
	mpg123exec = new char[datalen+1];
	
	// fill with \0
	memset(mpg123exec, 0, datalen+1);
	
	// Copy data
	strncpy(mpg123exec, filename, datalen);
	
	return 1;
} 



/**************************************
Set mpg123d udpport
***************************************/
int Settings::setUDPport(int port)
{
	int old = udpport;
	udpport = port;
	return old;
} 


/**************************************
Parse configuration file
***************************************/

int Settings::parseConfig()
{
	// Check if a configuration file is there...
	if(!cfgfile)
	{
		/*
			Ok, no configuration file specified.. We're goin to trie some defaults:
			/etc/mpg123d.conf
			/usr/local/etc/mpg123d.conf
			./mpg123d.conf
		*/
		
		const char def_cfgfiles[][100] = {"/etc/mpg123d.conf", "/usr/local/etc/mpg123d.conf", "./mpg123d.conf"};
		#define NUM_CFGFILES  3
		
		for(int fileno=0; fileno < NUM_CFGFILES; fileno++)
		{
			if(gMisc.FileExists((char*)def_cfgfiles[fileno]))
			{
				gSettings.setConfigFile((char*)def_cfgfiles[fileno]); // set cfgfile
				break;						// break
			}
		}
		
		// Check for config-file again
		if(!cfgfile)
		{
			gDebugger.LogMessage(LOG_ERR, "No suitable configuration file found!");
			return 0;
		}
		
	}
	
	gDebugger.LogMessage(LOG_INFO, "Parsing configuration file %s", cfgfile);
	/*  If we get here, we should have a valid configuration file
		Open it as stream											*/
	
	FILE* fstream;
	
	fstream = fopen(cfgfile, "r");
	
	if(!fstream)
	{
		gDebugger.LogMessage(LOG_ERR, "Failed to open configuration file %s for reading: %m", cfgfile);
		return 0;
	}
	
	/* At this point, we got a valid file-handle. Now setup valid directives */
	static char VALID_DIRECTIVES[][50] = {"debug", "mpg123", "udpport"};
	static int  NUM_VALID_DIRECT = 3;
	
	/* Setup buffers */
	#define READBUFF_SIZE	512
	#define BUFF_SIZE		512
	
	char * readBuff = new char[READBUFF_SIZE];
	char * directive= new char[BUFF_SIZE];
	char * value 	= new char[BUFF_SIZE];
	
	int cfg_line_no=0;
	
	/* Start loopin throu the file */
	while(!feof(fstream))
	{
		// Fill with \0
		memset(readBuff,0,READBUFF_SIZE);
		
		// Read a line or max READBUFF_SIZE chars
		fgets(readBuff, READBUFF_SIZE, fstream);
		cfg_line_no++;
		
		// Check for any newlines (shouldnt be any?), and replace with \0
		for(char*p=readBuff;*p!=0;p++)		if(*p=='\n') *p=0;
		
		// Ignore empty lines
		if(!strlen(readBuff)) continue;
		
		// Remove all empty spaces
		gMisc.trimstring(readBuff);
		
		// Check for comment
		if(readBuff[0] == '#') continue;
		
		// Make a basic check, that we have an '=' in the string.
		if(!strchr(readBuff, '='))
		{
			gDebugger.LogMessage(LOG_ERR, "Line %d in configuration file is INVALID!", cfg_line_no);
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
				gMisc.trimstring(directive);
				gMisc.trimstring(value);
				
				// Lower case name (we should be case insensitive)
				gMisc.lcasestring(directive);
				
				// Get outa loop
				break;
			}
		}
		
		/* Check if it's an valid directive */
		bool bExists=0;
		for(int iter=0;iter<NUM_VALID_DIRECT; iter++)
			if(!strcmp(VALID_DIRECTIVES[iter], directive))
			{
				bExists=1;
				break;
			}
		
		
		if(!bExists)
		{
			gDebugger.LogMessage(LOG_ERR, "Line %d in configuration file contains an invalid directive %s", cfg_line_no, directive);
			continue;
		}
		
		gDebugger.LogMessage(LOG_INFO, "Configuration file: %s = %s", directive, value);
		
		/* Now we come to the part where we save the setting */
		
		
		if(!strcmp(directive, "debug"))
		{
			if(gSettings.debuglevel == 0)
				gSettings.setDebug(atoi(value));	// Only change if it has default value, cmdline overrides
		}
		else if( !strcmp(directive, "mpg123"))
			gSettings.setMPG123(value);
		else if( !strcmp(directive, "udpport"))
			gSettings.setUDPport(atoi(value));
		else
			gDebugger.LogMessage(LOG_DEBUG, "Eh.. somehow the diretive %s got into the 'set setting' proccess :(.. Full line is %s", directive, readBuff);
		
	}
	
	delete readBuff;
	delete directive;
	delete value;
	
	fclose(fstream);
	
	gDebugger.LogMessage(LOG_INFO, "Configuration file parsing done.");
	
	return 1;
}
