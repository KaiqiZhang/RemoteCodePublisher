#pragma once
/////////////////////////////////////////////////////////////////////
//  Client.h - Remote Code Publisher Client                        //
//  ver 1.0                                                        //
//  Language:      Visual C++ 2015                                 //
//  Platform:      Microsoft Surface, Windows 10                   //
//  Application:   Used to perform remote code publisher           //
//  Author:        Kaiqi Zhang, Syracuse University                //
//                 kzhang17@syr.edu                                //
/////////////////////////////////////////////////////////////////////
/*
Package Operations:
==================
This package defines a Client class to receive requests from GUI and
send requests to the remote server. It as well as handle file tranfer.
This class should be created and called by MockChannel.

Public Interface:
=================
Client cl;                 // create socket instance instance
cl.connectServer();        // connect to server

Build Process:
==============
Required files
- Client.h, Client.cpp
- Socket.h, Socket.cpp
- HttpMessage.h, HttpMessage.cpp
- FileSystem.h, FileSystem.cpp
- Logger.h, Logger.cpp
- Utilities.h, Utilities.cpp

Maintenance History:
====================
ver 1.0 : 06 May 2017
- first release

*/
#include <string>
#include "../HttpMessage/HttpMessage.h"
#include "../Sockets/Sockets.h"
#include "../Logger/Logger.h"

using Show = StaticLogger<1>;
using namespace Utilities;
using Utils = StringHelper;

class Client
{
public:
  using Message = std::string;
  using EndPoint = std::string;

  Client();
  void connectServer();
  void parseAndSendMsg(const Message& msg);
  Message recvAndParseMsg();

private:
  SocketSystem ss_;
  SocketConnecter si_;
  bool connectionClosed_;
  std::string tempDir_;
  
  HttpMessage makeMessage(size_t n, const std::string& msgBody, const EndPoint& ep);
  void sendMessage(HttpMessage& msg, Socket& socket);
  bool sendFile(const std::string& localPath, const std::string& remotePath, Socket& socket);

  HttpMessage readMessage(Socket& socket);
  bool readFile(const std::string& filename, size_t fileSize, Socket& socket);

  void superCreateDir(const std::string& path);
};
