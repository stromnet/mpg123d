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

    $Id: settings.h,v 1.3 2004/02/03 18:13:29 johan Exp $
    $Log: settings.h,v $
    Revision 1.3  2004/02/03 18:13:29  johan
    Enabled -v switch to show version number.

    Revision 1.2  2004/02/03 18:03:08  johan
    Added Id and Log entrys in all files.


*/

class Settings
{
public:
	Settings::Settings();	// Init
	Settings::~Settings();	// Deinit
	
	int Settings::parseArgv(int argc, char**argv, char*version);

	int Settings::setDebug(int level);
	
	int Settings::setMPG123(char*filename);

	int Settings::setUDPport(int port);
	
	int Settings::setConfigFile(char*filename);
	int Settings::parseConfig();
	
	
	int debuglevel;
	int udpport;
	int nodaemon;
	char * cfgfile;
	char * mpg123exec;
	
};
