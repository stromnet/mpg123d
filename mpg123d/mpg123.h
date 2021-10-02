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

    $Id: mpg123.h,v 1.2 2004/02/03 18:03:08 johan Exp $
    $Log: mpg123.h,v $
    Revision 1.2  2004/02/03 18:03:08  johan
    Added Id and Log entrys in all files.


*/

class MPG123
{
public:
	MPG123::MPG123();	// Constructor
	MPG123::~MPG123();	// Deconstructor
	
	int MPG123::start();
	int MPG123::stop();
	int MPG123::isRunning();
	
	int MPG123::sendCmd(char*cmd);
	
	int MPG123::createPollThread();
	
	void* MPG123::poll_thread(void*param);
	
	void MPG123::sigpipe_handler(int signum);
	void MPG123::sigchld_handler(int signum);
	
	int doexit;
	int status;
	int manual_stopped;
	
	MP3Info CurrentPlaying;
private:
	int MPG123::writedata(char*data);
	int MPG123::readline(int fd, char *buff, int buffsize);
	
	
	int fd_read;
	int fd_write;
	int pid;

	
	pthread_t poll_threadid;
};

void* mpg123__poll_thread_caller(void*param);
void mpg123__sigpipe_handler_caller(int signum);
void mpg123__sigchld_handler_caller(int signum);

#define MPG123_STATUS_STOPPED	0
#define MPG123_STATUS_PAUSED	1
#define MPG123_STATUS_PLAYING	2
#define	MPG123_ERR_FILENOTFOUND	-1
#define MPG123_RETURN_NEXTMESSAGE 3
