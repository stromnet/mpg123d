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

    $Id: misc.cpp,v 1.2 2004/02/03 18:03:08 johan Exp $
    $Log: misc.cpp,v $
    Revision 1.2  2004/02/03 18:03:08  johan
    Added Id and Log entrys in all files.


*/

#include "common.h"

/**************************************
Check if given file is there
***************************************/

int Misc::FileExists(char*filename)
{
	int fh;
	gDebugger.LogMessage(LOG_INFO, "Checking if file %s exists...", filename);
	
	// Try to open the file
	fh = open(filename, O_RDONLY);
	
	// If fh is -1 and errno is ENOENT, the file or directory doesnt exists.. if fh is -1, but another errno, return errno.
	
	if(fh == -1 &&  errno == ENOENT)
		return 0;
	else if(fh == -1)
		return errno;
	
	// fh succeded, close fh and return 1
	close(fh);	
	return 1;
}

/**************************************
Check if given file is there
***************************************/
int Misc::DirectoryExists(char*dir)
{
	DIR *dirent;
	gDebugger.LogMessage(LOG_INFO, "Checking if directory %s exists...", dir);
	
	// Try to open the dir
	dirent = opendir(dir);
	
	if(!dirent)
	{
		if(errno == EACCES || errno == ENOENT || errno == ENOTDIR)
			return 0;
		else
			return -1;
	}
	
	closedir(dirent);
	return 1;
}

/***************************************
Do some cleanup, and return retval.

SHOULD ONLY BE CALLED TO BEFORE EXITING!
****************************************/
int Misc::cleanup_exit(int retval)
{
	gMPG123.doexit=1;
	
	if(gMPG123.isRunning())
		gMPG123.stop();
	
	if(gUDPServ.isactive())
		gUDPServ.closeserv();

	if(retval == 0)
		gDebugger.LogMessage(LOG_ERR, "Exiting"); // log as ERR, cause we always wana say this
	else
		gDebugger.LogMessage(LOG_ERR, "Exiting due to errors"); // log as ERR, cause we always wana say this
	
	return retval;
}

/***************************************
Trim spaces from beginning/end of string
****************************************/
int Misc::trimstring(char*string)
{
	char*p = string;
	
	while(*p == ' ' || *p == '\t')
		p++;
	
	strcpy(string, p);
	
	p=string+strlen(string)-1;
	while(p >= string && ( *p==' ' || *p=='\t' ))
		*p--=0;
}

/***************************************
Lowercase whole string
****************************************/
int Misc::lcasestring(char*string)
{
	for(char*p=string; *p!=0; p++)
		*p = tolower(*p);
}

/***************************************
Daemonize our proccess
****************************************/
int Misc::Daemonize()
{
	// Get our-self into background
	gDebugger.LogMessage(LOG_NOTICE, "Going into background...");
	
	// First, do a fork.. Exit the parent
	if(fork() > 0 ) exit(0);
	
	// Make us session leader
	setsid();	
	
	// Fork again, kill parent
	if(fork() > 0 ) exit(0);
	
	// Change current directory to /
	chdir("/");
	
	// Change umask
	umask(0);
	
	// Close stdin-stdout-stderr
	/*
		Well don't cus that will make mpg123 unable to talk with us :P
	close(0);
	close(1);
	close(2);
	*/
	gDebugger.LogMessage(LOG_NOTICE, "Running in background...");
}
