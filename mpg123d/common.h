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

    $Id: common.h,v 1.2 2004/02/03 18:03:08 johan Exp $
    $Log: common.h,v $
    Revision 1.2  2004/02/03 18:03:08  johan
    Added Id and Log entrys in all files.

*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include <errno.h>

#include <syslog.h>
#include <getopt.h>

#include <ctype.h>	// for tolower

#include <pthread.h>

#include <signal.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <sys/wait.h>

#include "debugger.h"
#include "settings.h"
#include "misc.h"
#include "mp3info.h"
#include "mpg123.h"
#include "udpserver.h"
#include "playlist.h"


extern Settings 	gSettings;
extern Debugger  	gDebugger;
extern Misc			gMisc;
extern MPG123		gMPG123;
extern UDPServer	gUDPServ;
extern PlayList		gPlayList;

#define DEFAULTPORT	3322
