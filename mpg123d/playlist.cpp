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


#include "common.h"


/*********************************************************************************************************
										PLAYLIST_ENTRY FUNCTIONS
**********************************************************************************************************/
PlayList_Entry::PlayList_Entry(PlayList_Entry * prev, PlayList_Entry * next, char*filename)
{
	prev_entry = prev;
	next_entry = next;
	
	if(prev_entry)
		prev_entry->next_entry=this;
	
	if(next_entry)
		next_entry->prev_entry=this;
	
	if(!gPlayList.first_entry)
		gPlayList.first_entry = this;
		
	if(!gPlayList.last_entry)
		gPlayList.last_entry = this;
	
	if(!gPlayList.current_entry)
		gPlayList.current_entry=this;
	
	file = new char[strlen(filename)+1];
	strcpy(file, filename);
}

PlayList_Entry::~PlayList_Entry()
{
	/*
		as im goin away, remap next and prev:
		
		if im first, next prev's should be null
		if im last, prev's next should be null
		if im somewhere else, nexts perv should be my prev, and prev's next should be my next
	*/	
	
	if(next_entry && prev_entry) 	// Im in the midle
	{
		next_entry->prev_entry = prev_entry;
		prev_entry->next_entry = next_entry;
	}else if(!next_entry && !prev_entry)	// im self
	{
		gPlayList.first_entry = NULL;
		gPlayList.last_entry = NULL;
		gPlayList.current_entry = NULL;
	}else if(!next_entry)			// im last
	{
		prev_entry->next_entry=NULL;
		gPlayList.last_entry = prev_entry;
	}else if(!prev_entry)			// im first
	{
		next_entry->prev_entry=NULL;
		gPlayList.first_entry = next_entry;
	}
	
	gPlayList.numitems--;
	
	delete file;
}


/*********************************************************************************************************
										PLAYLIST FUNCTIONS
**********************************************************************************************************/

PlayList::PlayList()
{
	first_entry  = NULL;
	last_entry   = NULL;
	current_entry= NULL;
	numitems = 0;
}

PlayList::~PlayList()
{
	// WE are responsible for deleting ALL entrys
	gPlayList.clearPlayList();
}

int PlayList::clearPlayList()
{
	// begin clearing... start in the begining
	PlayList_Entry *pCurr = first_entry;
	PlayList_Entry *pNext;
	
	while(pCurr)
	{
		pNext = pCurr->next_entry;
		delete pCurr;
		
		pCurr=pNext;
	}
	
	first_entry = NULL;
	last_entry = NULL;
	current_entry = NULL;
	
	// now there should be any left..
	return 1;
}

int PlayList::addEntry(char*filename, int pos)
{
	// Check if we have any list
	if(!first_entry)
		first_entry = new PlayList_Entry(NULL,NULL, filename);
	else
	{
		if(pos==0)
		{
			// Add in begining
			PlayList_Entry *pNext;
			pNext = first_entry;
			first_entry = new PlayList_Entry(NULL, pNext, filename);
		}else
		{
			// Add at end
			PlayList_Entry *pPrev;
			pPrev = last_entry;
			last_entry = new PlayList_Entry(pPrev, NULL, filename);
		}
	}
	
	gDebugger.LogMessage(LOG_INFO, "%s added to playlist", filename);
	
	numitems++;
}

PlayList_Entry * PlayList::getEntryNo(int pos)
{
	PlayList_Entry *pCurr;
	int counter = 0;
	
	if(pos==0)
		return first_entry;
	
	if(pos==numitems)
		return last_entry;
	
	pCurr=first_entry;
	while(1)
	{
		if(counter==pos)
			break;
		
		counter++;
		pCurr = pCurr->next_entry;
	}
	
	return pCurr;
}

int PlayList::playCurrent()
{
	if(!current_entry)
		return 0;
	
	char*cmd = new char[strlen(current_entry->file)+10];
	
	sprintf(cmd, "load %s", current_entry->file);
	
	gMPG123.sendCmd(cmd);
	
	delete cmd;
}

int PlayList::playPrev( int num )
{
	if(!current_entry)
		return 0;
	
	for(int i=num; i > 0; i--)
	{
		if(current_entry->prev_entry)
			current_entry = current_entry->prev_entry;
		else
			break;
	}
	
	gPlayList.playCurrent();
}
	
int PlayList::playNext( int num )
{
	if(!current_entry)
		return 0;
	
	for(int i=num; i > 0; i--)
	{
		if(current_entry->next_entry)
			current_entry = current_entry->next_entry;
		else
			break;
	}
	
	gPlayList.playCurrent();
}
	