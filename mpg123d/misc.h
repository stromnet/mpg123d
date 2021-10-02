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

    $Id: misc.h,v 1.2 2004/02/03 18:03:08 johan Exp $
    $Log: misc.h,v $
    Revision 1.2  2004/02/03 18:03:08  johan
    Added Id and Log entrys in all files.


*/

class Misc
{
public:
	/* Not in use!
	Misc::Misc();	// Class constructor
	Misc::~Misc();	// class deconstructor
	*/
	int Misc::FileExists(char*filename);
	int Misc::DirectoryExists(char*dir);
	int Misc::cleanup_exit(int retval);
	int Misc::trimstring(char*string);
	int Misc::lcasestring(char*string);
	int Misc::Daemonize();
};
