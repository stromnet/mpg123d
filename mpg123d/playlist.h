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

    $Id: playlist.h,v 1.2 2004/02/03 18:03:08 johan Exp $
    $Log: playlist.h,v $
    Revision 1.2  2004/02/03 18:03:08  johan
    Added Id and Log entrys in all files.


*/


class PlayList_Entry
{
public:
	PlayList_Entry::PlayList_Entry(PlayList_Entry * prev, PlayList_Entry * next, char*filename);
	PlayList_Entry::~PlayList_Entry();
	
	PlayList_Entry *PlayList_Entry::prev_entry;
	PlayList_Entry *PlayList_Entry::next_entry;
	
	char* PlayList_Entry::file;
};

class PlayList
{
public:
	PlayList::PlayList();
	PlayList::~PlayList();
	
	int PlayList::clearPlayList();
	
	int PlayList::addEntry(char*filename, int pos);	// pos is between 0 and numitems
	PlayList_Entry * PlayList::getEntryNo(int pos);	// pos is integer between 0 and numitems
	
	int PlayList::playCurrent();
	int PlayList::playPrev( int num );
	int PlayList::playNext( int num );
	
	PlayList_Entry *PlayList::first_entry;
	PlayList_Entry *PlayList::current_entry;
	PlayList_Entry *PlayList::last_entry;
	int PlayList::numitems;
};
