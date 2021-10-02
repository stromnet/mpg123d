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

    $Id: mpg123d.cpp,v 1.3 2004/02/03 18:13:29 johan Exp $
    $Log: mpg123d.cpp,v $
    Revision 1.3  2004/02/03 18:13:29  johan
    Enabled -v switch to show version number.

    Revision 1.2  2004/02/03 18:03:08  johan
    Added Id and Log entrys in all files.


*/

#include "common.h"

Debugger 	gDebugger;
Settings 	gSettings;
Misc		gMisc;
MPG123		gMPG123;
UDPServer	gUDPServ;
PlayList	gPlayList;

void handlesigint(int signum)
{
	gDebugger.LogMessage(LOG_NOTICE, "SIGINT recieved!");
	gMPG123.doexit = 1;
}

int main(int argc, char**argv)
{
	char szVersion[] = "$Id: mpg123d.cpp,v 1.3 2004/02/03 18:13:29 johan Exp $";

	gDebugger.LogMessage(LOG_ERR, "-----------"); // Log as ERR, cause we always wana say this :P
	gDebugger.LogMessage(LOG_ERR, "starting");
	
	// Parse arguments passed to us
	if(!gSettings.parseArgv(argc, argv, szVersion))
		return gMisc.cleanup_exit(1);
	
	// Parse configuration file
	if(!gSettings.parseConfig())
		return gMisc.cleanup_exit(1);
	
	// Get our self in background (make us a daemon), unless user asked us not to
	if(!gSettings.nodaemon)
		gMisc.Daemonize();
	
	// Now we should be daemonized... Initialize mpg123
	if(!gMPG123.start())
		return gMisc.cleanup_exit(1);
	
	signal(SIGINT, handlesigint);
	
	// Well let's hope mpg123 is running now..
	// Create thread that reads mpg123
	if(!gMPG123.createPollThread())
		return gMisc.cleanup_exit(1);
	
	if(!gUDPServ.start(gSettings.udpport))
		return gMisc.cleanup_exit(1);
	
	while(!gMPG123.doexit)
	{
		gUDPServ.checkdata();
		usleep(200000);
	}
	
	return gMisc.cleanup_exit(0);
}
