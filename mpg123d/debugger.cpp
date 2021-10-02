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

    $Id: debugger.cpp,v 1.2 2004/02/03 18:03:08 johan Exp $
    $Log: debugger.cpp,v $
    Revision 1.2  2004/02/03 18:03:08  johan
    Added Id and Log entrys in all files.


*/

#include "common.h"

/***************************************
Constructor. Call openlog
****************************************/
Debugger::Debugger()
{
	openlog("mpg123d", LOG_PID, LOG_LOCAL0);
	//gDebugger.LogMessage(LOG_DEBUG, "Entering Debug constructor.. openlog called");
}

/***************************************
Deconstructor, call closelog
****************************************/
Debugger::~Debugger()
{
	//gDebugger.LogMessage(LOG_DEBUG, "Entering Debug deconstructor.. just calling closelog()");
	closelog();
}

/***************************************
Log a message to syslog
****************************************/
void Debugger::LogMessage(int level, char*format, ...)
{
	/* Just log the messages to syslog */
	va_list pa;
	
	va_start(pa, format);
	vsyslog(level,format, pa);
	va_end(pa);
}

/***************************************
Change loglevel
****************************************/
void Debugger::ChangeLogMask(int level)
{
	int logmask;
	
	// Always log errors
	logmask = LOG_MASK(LOG_ERR);
	
	if(gSettings.debuglevel >= 1)
		logmask|=LOG_MASK(LOG_NOTICE) | LOG_MASK(LOG_WARNING);
	
	if(gSettings.debuglevel >= 2)
		logmask|=LOG_MASK(LOG_INFO);
	
	if(gSettings.debuglevel >= 3)
		logmask|=LOG_MASK(LOG_DEBUG);
	
	setlogmask(logmask);
	gDebugger.LogMessage(LOG_DEBUG, "Changed loglevel to %d (logmask 0x%x)", level, logmask);	
}
