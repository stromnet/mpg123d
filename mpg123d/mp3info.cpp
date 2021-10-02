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

    $Id: mp3info.cpp,v 1.2 2004/02/03 18:03:08 johan Exp $
    $Log: mp3info.cpp,v $
    Revision 1.2  2004/02/03 18:03:08  johan
    Added Id and Log entrys in all files.


*/

#include "common.h"

MP3Info::MP3Info()
{
	// Alloc memory
	artist = new char[31];
	title  = new char[31];
	album  = new char[31];
	year   = new char[5];
	comment= new char[31];
	
	s_mpeg_type = new char[50];
	s_mode = new char[50];
	
	memset(artist,0,31);
	memset(title,0,31);
	memset(album,0,31);
	memset(year,0,5);
	memset(comment,0,31);
	
	memset(s_mpeg_type,0,50);
	memset(s_mode,0,50);
}

MP3Info::~MP3Info()
{
	delete artist;
	delete title;
	delete album;
	delete year;
	delete comment;
	delete genre;
	
	delete s_mpeg_type;
	delete s_mode;
}
	
/***************************************
Parse mpg123-file info
****************************************/
int MP3Info::setID3Data(char*infoStr)
{
	// First, look at the first 4 characters, and check if they contains ID3:
	if(!strncmp(infoStr, "ID3:", 4))
	{
		// We got ourself an ID3-tag
		char *p=infoStr;
		p+=4;	// skipp ID3:
	
		memset(artist,0,31);
		memset(title,0,31);
		memset(album,0,31);
		memset(year,0,5);
		memset(comment,0,31);
			
		// Start copying...
		strncpy(title  , p, 30); p+=30;
		strncpy(artist , p, 30); p+=30;
		strncpy(album  , p, 30); p+=30;
		strncpy(year   , p, 4);  p+=4;
		strncpy(comment, p, 30); p+=30;
		
		gMisc.trimstring(title);
		gMisc.trimstring(artist);
		gMisc.trimstring(album);
		gMisc.trimstring(year);
		gMisc.trimstring(comment);
		
		// The rest in p should be genre now..		
		genre = new char[strlen(p)+1];
		memset(genre,0, strlen(p)+1);
		strcpy(genre, p);
		
		gMisc.trimstring(genre);
		
		gDebugger.LogMessage(LOG_INFO, "Artist=%s Title=%s Album=%s Year=%s Comment=%s Genre=%s", artist,title,album,year,comment,genre);
		
		return 1;
	}else
	{
		// Uhm.. we got a filename "without path and extension" :(
		gDebugger.LogMessage(LOG_NOTICE, "no id3tag for file %s", infoStr);
		
		// well empty em
		memset(artist,0,31);
		memset(title,0,31);
		memset(album,0,31);
		memset(year,0,5);
		memset(comment,0,31);
		
		return 1;
	}	
}

/***************************************
Parse mpg123-file stream-info
****************************************/
int MP3Info::setStreamData(char*infoStr)
{
	// we need a tmp for strtok_r
	char *temp = new char[strlen(infoStr)+1];
	char *tokbuff;
	char *p;
	int  part=0;
	
	
	sscanf(infoStr, "%s %d %d %s %d %d %d %d %d %d %d %d",
					s_mpeg_type, &s_layer, &s_sampling_freq, s_mode,
					&s_mode_ext, &s_framesize, &s_stereo, &s_copyright,
					&s_error_prot, &s_emphasis, &s_bitrate, &s_extension);
	
	
	gDebugger.LogMessage(LOG_INFO, "MPEG Type=%s Layer=%d Sampling Frequency=%d Mode=%s Mode Extension=%d Framesize=%d Stereo=%d Copyright=%d Error Protection=%d Emphasis=%d Bitrate=%d Extension=%d",
									s_mpeg_type, s_layer, s_sampling_freq, s_mode, s_mode_ext, s_framesize, s_stereo, s_copyright, s_error_prot, s_emphasis, s_bitrate, s_extension);
	
	return 1;
}

/***************************************
Parse mpg123-@F-data
****************************************/
int MP3Info::setTimeData(char*infoStr)
{
	sscanf(infoStr, "%d %d %f %f", &dur_frames, &dur_frames_left, &dur_seconds, &dur_seconds_left);
	
	return 1;
}
