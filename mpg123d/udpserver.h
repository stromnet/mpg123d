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

    $Id: udpserver.h,v 1.2 2004/02/03 18:03:08 johan Exp $
    $Log: udpserver.h,v $
    Revision 1.2  2004/02/03 18:03:08  johan
    Added Id and Log entrys in all files.


*/

class UDPServer
{
public:
	UDPServer::UDPServer();	// Constructor
	UDPServer::~UDPServer();// Deconstructor
	int UDPServer::start(int port);
	int UDPServer::checkdata();
	int UDPServer::closeserv();
	int UDPServer::isactive();
private:
	int UDPServer::polldata();
	int UDPServer::readdata(char*buffer);
	int UDPServer::sendreply(char*msg, struct sockaddr_in sinDest);
	int UDPServer::checkcmd(char*cmd);
	
	int UDPServer::handlePlaylistMessage(char*cmd);
	int UDPServer::handlePlayerMessage(char*cmd);
	int UDPServer::handleServerMessage(char*cmd);
	
	int fdsocket;
	int active;
	
	struct sockaddr_in sinLocalBind;
	struct sockaddr_in sinRemote;
	
	char*sendNext;
};
