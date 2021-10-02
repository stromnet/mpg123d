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

    $Id: mp3info.h,v 1.2 2004/02/03 18:03:08 johan Exp $
    $Log: mp3info.h,v $
    Revision 1.2  2004/02/03 18:03:08  johan
    Added Id and Log entrys in all files.


*/

class MP3Info
{
public:
	MP3Info::MP3Info();
	MP3Info::~MP3Info();
	
	char *MP3Info::artist;
	char *MP3Info::title;
	char *MP3Info::album;
	char *MP3Info::year;
	char *MP3Info::comment;
	char *MP3Info::genre;
	
	char *MP3Info::s_mpeg_type;
	int   MP3Info::s_layer;
	int   MP3Info::s_sampling_freq;
	char *MP3Info::s_mode;
	int   MP3Info::s_mode_ext;
	int   MP3Info::s_framesize;
	int   MP3Info::s_stereo;
	int   MP3Info::s_copyright;
	int   MP3Info::s_error_prot;
	int   MP3Info::s_emphasis;
	int   MP3Info::s_bitrate;
	int   MP3Info::s_extension;
	
	int   MP3Info::dur_frames;
	int   MP3Info::dur_frames_left;
	float MP3Info::dur_seconds;
	float MP3Info::dur_seconds_left;
	
	int MP3Info::setID3Data(char*infoStr);
	int MP3Info::setStreamData(char*infoStr);
	int MP3Info::setTimeData(char*infoStr);
};
