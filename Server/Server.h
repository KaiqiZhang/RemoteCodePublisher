#pragma once
/////////////////////////////////////////////////////////////////////
//  Server.h - Remote Code Publisher Server                        //
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
This package defines a ClientHandle class for remote code publisher
to handle requests from clients.

Public Interface:
=================
SocketSystem ss;                       // create socket instance instance
SocketListener sl(8080, Socket::IP6);  // create socket listener instance
ClientHandler cp(msgQ);                // create client handler instance
cp.ProcessCommandLine(argc, argv);     // process command line arguments
sl.start(cp);                          // start client handler

Build Process:
==============
Required files
- Server.h, Server.cpp
- Socket.h, Socket.cpp
- HttpMessage.h, HttpMessage.cpp
- CodePublisher.h, CodePublisher.cpp
- ScopeTable.h, ScopeTable.cpp
- DepAnal.h, DepAnal.cpp
- AbstrSynTree.h, AbstrSynTree.cpp

Maintenance History:
====================
ver 1.0 : 06 May 2017
- first release

*/
#include <iostream>
#include <sstream>
#include <string>
#include "../Cpp11-BlockingQueue/Cpp11-BlockingQueue.h"
#include "../Sockets/Sockets.h"
#include "../HttpMessage/HttpMessage.h"
#include "../CodePublisher/CodePublisher.h"

using namespace Async;
using namespace CodePublisher;
using namespace CodeAnalysis;
using EndPoint = std::string;

/////////////////////////////////////////////////////////////////////
// ClientHandler class
/////////////////////////////////////////////////////////////////////
// - instances of this class are passed by reference to a SocketListener
// - when the listener returns from Accept with a socket it creates an
//   instance of this class to manage communication with the client.
// - You no longer need to be careful using data members of this class
//   because each client handler thread gets its own copy of this 
//   instance so you won't get unwanted sharing.
// - I changed the SocketListener semantics to pass
//   instances of this class by value for version 5.2.
// - that means that all ClientHandlers need copy semantics.
//
class ClientHandler
{
public:
  ClientHandler(BlockingQueue<HttpMessage>& msgQ) : msgQ_(msgQ) {}
  void operator()(Socket socket);
  bool ProcessCommandLine(int argc, char* argv[]);

private:
  bool connectionClosed_;
  BlockingQueue<HttpMessage>& msgQ_;
  std::string rootPath_;
  DepTable depTable_;

  void publishCode(int argc, char* argv[]);

  HttpMessage readMessage(Socket& socket);
  bool readFile(const std::string& filename, size_t fileSize, Socket& socket);
  HttpMessage makeMessage(size_t n, const std::string& body, const EndPoint& ep);
  void sendMessage(HttpMessage& msg, Socket& socket);
  bool sendFile(const std::string& localPath, const std::string& remotePath, bool openFile, const EndPoint& ep, Socket& socket);

  void findConnectedFiles(std::unordered_set<std::string>& files, const std::string& relativePath);
  void superCreateDir(const std::string& path);

  void showUsage();
};
