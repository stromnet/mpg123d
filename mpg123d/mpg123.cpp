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

    $Id: mpg123.cpp,v 1.2 2004/02/03 18:03:08 johan Exp $
    $Log: mpg123.cpp,v $
    Revision 1.2  2004/02/03 18:03:08  johan
    Added Id and Log entrys in all files.


*/

#include "common.h"

/***************************************
Constructor.. 
****************************************/

MPG123::MPG123 ()
{
	doexit = 0;
	manual_stopped = 0;
}


/***************************************
Deconstructor.. stop mpg123 if running
****************************************/

MPG123::~MPG123 ()
{
	if(gMPG123.isRunning())
		gMPG123.stop();
}

/***************************************
Start mpg123
****************************************/
int MPG123::start()
{
	// we need to socket-pairs
	int fd_recv[2], fd_send[2];
	// If pid is more than 0, mpg123 should be alive, right?
	if(pid) return 1;
	
	gDebugger.LogMessage(LOG_DEBUG, "Starting mpg123..");
	
	// Create 2 pair of sockets for communication with mpg123.
	// Sending
	if(socketpair(AF_UNIX, SOCK_STREAM, 0, fd_send) == -1)
	{
		gDebugger.LogMessage(LOG_ERR, "Failed to create socketpair for sending data to mpg123! Error %m occured");
		return 0;
	}
	
	// Recieving
	if(socketpair(AF_UNIX, SOCK_STREAM, 0, fd_recv) == -1)
	{
		gDebugger.LogMessage(LOG_ERR, "Failed to create socketpair for recieving data from mpg123! Error %m occured");
		return 0;
	}
	
	signal(SIGPIPE, mpg123__sigpipe_handler_caller);
	signal(SIGCHLD, mpg123__sigchld_handler_caller);
	
	gDebugger.LogMessage(LOG_DEBUG, "Forking..");	// Fork ourself
	pid = fork();
	
	if(pid == 0)
	{
		// We are the child proccess
		
		// Redirect stdin/stdout/stderr to our parent using sockets
		dup2(fd_send[0], STDIN_FILENO);
		close(fd_send[0]);
		close(fd_send[1]);
		
		dup2(fd_recv[0], STDOUT_FILENO);
		dup2(fd_recv[0], STDERR_FILENO);
		close(fd_recv[0]);
		close(fd_recv[1]);
		
		gDebugger.LogMessage(LOG_DEBUG, "Executing %s -R -...", gSettings.mpg123exec);
		if(execlp(gSettings.mpg123exec, gSettings.mpg123exec, "-R", "-", NULL) == -1)
			gDebugger.LogMessage(LOG_ERR, "Failed to execute mpg123. exevp gave error %m");
		
		// We should never get here if execvp executed correctly
		exit(-1);
	}
	
	// Close fd_send[0] and fd_recv[0]
	close(fd_send[0]);
	close(fd_recv[0]);
	
	fd_read  = fd_recv[1];
	fd_write = fd_send[1];
	
	gDebugger.LogMessage(LOG_DEBUG, "mpg123: fd_read=%d, fd_send=%d", fd_read, fd_write);
	
	// We wan't to wait half a second to let mpg123 start...
	usleep(500000);
	
	if(!pid)
	{
		gDebugger.LogMessage(LOG_ERR, "failed to start mpg123!");
		return 0;
	}
	
	gDebugger.LogMessage(LOG_NOTICE, "mpg123 started with pid %d", pid);
	return 1;
}

/***************************************
Stop mpg123
****************************************/
int MPG123::stop()
{
	int retval;
	if(!pid) return 1;	// not started? return 1
	
	retval = writedata("quit");
	
	/*
	if(retval == 0 && errno == EPIPE)	// we got an EPIPE error, that is mpg123 is already dead...
	
	currently we dont give a shit.. we told him to quit, if he isn't dead after half a 1 sec, kill him
	*/
	
	usleep(500000);
	
	gDebugger.LogMessage(LOG_INFO, "Sending SIGTERM to mpg123 (%d)", pid);
	
	kill(pid, SIGTERM);
		
	
	close(fd_read);
	close(fd_write);
	
	pid=0;
		
	return 1;
}

/***************************************
Check if mpg123 is alive
****************************************/
int MPG123::isRunning()
{
	if(pid)
		return 1;
	else
		return 0;
}

/***************************************
Sends a command to mpg123
****************************************/
int MPG123::sendCmd(char*cmd)
{

	if(!cmd || !strlen(cmd))
		return 0;
	
	// First check command
	char *command=new char[50];
	char *p = NULL;
	
	memset(command, 0, 50);
	
	// Extract command.. All begins with a word, then space and param, or just a newline
	p = strchr(cmd, ' ');
	if(p)
	{
		strncpy(command, cmd, (p-cmd));
		p++;
	}
	else
		strncpy(command, cmd, 49);
	
	// Check for valid cmd
	if(strcmp(command, "load") && strcmp(command, "pause") && strcmp(command, "stop") && strcmp(command,"jump") && strcmp(command, "quit"))
	{
		gDebugger.LogMessage(LOG_ERR, "Got invalid command %s to MPG123::sendCmd()", command);
		delete command;
		return 0;
	}
	
	// If jump, check for valid parameter
	if(!strcmp(command, "jump"))
	{
		/*
			Valid paramters:
			
			JUMP [+|-]<a>
                        Skip <a> frames:
                        +32     forward 32 frames
                        -32     rewind 32 frames
                        0       jump to the beginning
                        1024    jump to frame 1024

			That is, we want [+|-]<integer>
		*/
		
		// If param begins with a + or -
		if(*p == '-' || *p == '+')
			p++;
			
		for(char *p2 = p; *p2!=0;p2++)
		{
			if(!isdigit(*p2))
			{
				gDebugger.LogMessage(LOG_ERR, "Invalid jump command recieved: %s", cmd);
				delete command;
				return 0;
			}
		}
	}
	
	// Add a \n
	char *cmdLine =new char[strlen(cmd)+5];
	sprintf(cmdLine	, "%s\n", cmd);
		
	// If we get here, the command-line SHOULD be correct and don't cause any errors
	int retval = writedata(cmdLine);
	
	delete cmdLine;
	delete command;
	return retval;
}

/***************************************
Create a thread that polls fd_read
****************************************/
int MPG123::createPollThread()
{
	int retval;
	
	gDebugger.LogMessage(LOG_DEBUG, "Starting polling thread");
	retval = pthread_create(&this->poll_threadid, NULL, mpg123__poll_thread_caller, NULL);
	
	if(retval == 0)
		return 1;
	else
		return 1;
}

/***************************************
Caller that just calls poll_thread for pthread_create...
****************************************/
void* mpg123__poll_thread_caller(void*param)
{
	return gMPG123.poll_thread(param);
}

/***************************************
Thread that polls fd_read for data
****************************************/
void* MPG123::poll_thread(void*param)
{
	gDebugger.LogMessage(LOG_DEBUG, "Polling thread started");
	
	#define BUFF_SIZE 512
	
	fd_set readset;
	struct timeval tv;
	int retval;
	char *readBuff = new char[BUFF_SIZE];
	char *p;
	
	while(!doexit)
	{
		FD_SET(fd_read, &readset);
		tv.tv_sec = 0;
		tv.tv_usec = 10;
		
		retval = select(fd_read+1, &readset, NULL, NULL, &tv);
		
		if(retval > 0)
		{
			memset(readBuff, 0, BUFF_SIZE);
			retval=gMPG123.readline(fd_read, readBuff, BUFF_SIZE);
			
			if(retval > 0)
			{
				p=readBuff;
				
				if(*p=='@')
				{
					p++;
					
					// log all but @F's
					if(*p != 'F')	gDebugger.LogMessage(LOG_DEBUG, "Got %s from mpg123", readBuff);
					
					switch(*p)
					{
						case 'R':	// mpg123 start report
							status=MPG123_STATUS_STOPPED;
							break;
						case 'I':	// ID3-info
							gMPG123.CurrentPlaying.setID3Data(p+2);
							break;
						case 'S':	// streaminfo
							gMPG123.CurrentPlaying.setStreamData(p+2);
							status=MPG123_STATUS_PLAYING;
							manual_stopped = 0;
							break;
						case 'F':	// Frameinfo
							gMPG123.CurrentPlaying.setTimeData(p+2);
							break;
						case 'P':	// statusinfo
						{
							int newstatus;
							p+=2;
							
							switch(*p)
							{
								case '0': newstatus=MPG123_STATUS_STOPPED; break;
								case '1': newstatus=MPG123_STATUS_PAUSED;  break;
								case '2': newstatus=MPG123_STATUS_PLAYING; break;
							}
							
							if(	status == MPG123_STATUS_PLAYING &&
								newstatus == MPG123_STATUS_STOPPED &&
								!manual_stopped &&
								gPlayList.current_entry &&
								gPlayList.current_entry->next_entry)
							{
								gPlayList.current_entry = gPlayList.current_entry->next_entry;
								gPlayList.playCurrent();
							}
							
							status=newstatus;
							
							break;
						}
						case 'E':	// error
							gDebugger.LogMessage(LOG_WARNING, "mpg123 said: \"%s\"", p+2);
							break;
						default:
							break;
					}
					
				}
				
				// If we isn't a daemon, print all to stderr..
				if(gSettings.nodaemon) fprintf(stderr, "Got %s from mgp123\n", readBuff);
			}
		}
		
		usleep(10000);
		
	}
	
	return 0;
}

/***************************************
Caller that just calls sigpipe_handler
****************************************/
void mpg123__sigpipe_handler_caller(int signum)
{
	return gMPG123.sigpipe_handler(signum);
}

/***************************************
SIGPIPE Signal handler
****************************************/
void MPG123::sigpipe_handler(int signum)
{
	//gDebugger.LogMessage(LOG_DEBUG, "SIGPIPE signal recieved...");
}

/***************************************
Caller that just calls sigchild_handler
****************************************/
void mpg123__sigchld_handler_caller(int signum)
{
	return gMPG123.sigchld_handler(signum);
}

/***************************************
SIGCHLD Signal handler
****************************************/
void MPG123::sigchld_handler(int signum)
{
	gDebugger.LogMessage(LOG_DEBUG, "SIGCHLD signal recieved...");
	while(wait(NULL)>0);
	
	// Did mpg123 die??
	int retval;
	retval = waitpid(pid, NULL, WNOHANG);
	if(retval==-1)
	{
		if(!doexit)
		{
			gDebugger.LogMessage(LOG_ERR, "mpg123 died.. restarting..");
			gMPG123.stop();
			gMPG123.start();
		}
	}
}

/************************************************************************************************************
										INTERNAL FUNCTIONS
*************************************************************************************************************/


/***************************************
Writes data (no validation is made!)
****************************************/
int MPG123::writedata(char*data)
{
	int retval;
	retval = write(fd_write, data, strlen(data));
	
	gDebugger.LogMessage(LOG_DEBUG, "Wrote %s to mpg123, with result %d", data, retval);
	
	if(retval > 0)
		return 1;
	else
		return 0;
}

/***************************************
Reads a row from fd
****************************************/
int MPG123::readline(int fd, char *buff, int buffsize)
{
	char c=0;
	int retval;
	char *p = buff;
	
	while((p-buff) < buffsize-1)
	{
		retval = read(fd, p, 1);
		
		if(retval <= 0 || *p == '\n')
			break;
		
		p++;
	}
	
	*p=0;
	return (p-buff);
}
