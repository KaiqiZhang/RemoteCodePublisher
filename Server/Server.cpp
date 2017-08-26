/////////////////////////////////////////////////////////////////////
//  Server.cpp - Remote Code Publisher Server                      //
//  ver 1.0                                                        //
//  Language:      Visual C++ 2015                                 //
//  Platform:      Microsoft Surface, Windows 10                   //
//  Application:   Used to perform remote code publisher           //
//  Author:        Kaiqi Zhang, Syracuse University                //
//                 kzhang17@syr.edu                                //
/////////////////////////////////////////////////////////////////////
#pragma warning(disable:4996)

#include "Server.h"
#include "../FileSystem/FileSystem.h"
#include "../Logger/Logger.h"
#include "../Utilities/Utilities.h"
#include <string>
#include <iostream>
#include "../CodePublisher/CodePublisher.h"
#include "../Analyzer/Executive.h"
#include "../Analyzer/TypeAnal.h"

using Show = StaticLogger<0>;
using namespace Utilities;

//----< usage message >----------------------------------------------

void ClientHandler::showUsage()
{
  std::ostringstream out;
  out << "\n  Usage:";
  out << "\n  Command Line Arguments are:";
  out << "\n  - 1st: path to subdirectory to store source files";
  //out << "\n  - 2st: path to subdirectory to store published html files";
  //out << "\n  - 3st: server port number. default: 8080";

  out << "\n\n";
  std::cout << out.str();
}

//----< handle command line arguments >------------------------------
/*
* Arguments are:
* - path: possibly relative path to folder containing all analyzed code,
*   e.g., may be anywhere in the directory tree rooted at that path
* - patterns: one or more file patterns of the form *.h, *.cpp, and *.cs
* - options: /m (show metrics), /s (show file sizes), and /a (show AST)
*/
bool ClientHandler::ProcessCommandLine(int argc, char* argv[])
{
  if (argc < 2)
  {
    showUsage();
    return false;
  }
  try {
    rootPath_ = FileSystem::Path::getFullFileSpec(argv[1]);    
    if (!FileSystem::Directory::exists(rootPath_))
    {
      //std::cout << "\n\n  path \"" << rootPath_ << "\" does not exist\n\n";
      FileSystem::Directory::create(rootPath_);
    }
    std::cout << "\n  Code Repo Path: " << rootPath_ << "\n";
  }
  catch (std::exception& ex)
  {
    std::cout << "\n\n  command line argument parsing error:";
    std::cout << "\n  " << ex.what() << "\n\n";
    return false;
  }
  return true;
}
//----< this defines processing to frame messages >------------------

HttpMessage ClientHandler::readMessage(Socket& socket)
{
  connectionClosed_ = false;
  HttpMessage msg;
  // read message attributes
  while (true)
  {
    std::string attribString = socket.recvString('\n');
    if (attribString.size() <= 1) break;
    HttpMessage::Attribute attrib = HttpMessage::parseAttribute(attribString);
    msg.addAttribute(attrib);
  }
  // If client is done, connection breaks and recvString returns empty string
  if (msg.attributes().size() == 0)
  {
    connectionClosed_ = true;
    return msg;
  }
  // read body if POST - all messages in this demo are POSTs
  if (msg.attributes()[0].first == "POST")
  {
    // is this a file message?
    std::string filename = msg.findValue("file");
    if (filename != "")
    {
      size_t contentSize;
      std::string sizeString = msg.findValue("content-length");
      if (sizeString != "")
        contentSize = Converter<size_t>::toValue(sizeString);
      else
        return msg;

      readFile(msg.findValue("path"), contentSize, socket);
    } else {
      // read message body
      size_t numBytes = 0;
      size_t pos = msg.findAttribute("content-length");
      if (pos < msg.attributes().size())
      {
        numBytes = Converter<size_t>::toValue(msg.attributes()[pos].second);
        Socket::byte* buffer = new Socket::byte[numBytes + 1];
        socket.recv(numBytes, buffer);
        buffer[numBytes] = '\0';
        std::string msgBody(buffer);
        msg.addBody(msgBody);
        delete[] buffer;
      }
    }
  }
  return msg;
}
//----< read a binary file from socket and save >--------------------
/*
* This function expects the sender to have already send a file message,
* and when this function is running, continuosly send bytes until
* fileSize bytes have been sent.
*/
bool ClientHandler::readFile(const std::string& filename, size_t fileSize, Socket& socket)
{
  // create directory if not exist
  std::string filePath = FileSystem::Path::getPath(filename);
  superCreateDir(filePath);

  std::string fqname = filename;
  FileSystem::File file(fqname);
  file.open(FileSystem::File::out, FileSystem::File::binary);
  if (!file.isGood())
  {
    /*
    * This error handling is incomplete.  The client will continue
    * to send bytes, but if the file can't be opened, then the server
    * doesn't gracefully collect and dump them as it should.  That's
    * an exercise left for students.
    */
    Show::write("\n\n  can't open file " + fqname);
    return false;
  }

  const size_t BlockSize = 2048;
  Socket::byte buffer[BlockSize];

  size_t bytesToRead;
  while (true)
  {
    if (fileSize > BlockSize)
      bytesToRead = BlockSize;
    else
      bytesToRead = fileSize;

    socket.recv(bytesToRead, buffer);

    FileSystem::Block blk;
    for (size_t i = 0; i < bytesToRead; ++i)
      blk.push_back(buffer[i]);

    file.putBlock(blk);
    if (fileSize < BlockSize)
      break;
    fileSize -= BlockSize;
  }
  file.close();
  return true;
}

//----< recursively remove files and directories >-------------------

void recursiveRemoveDirectory(std::string path)
{
  std::vector<std::string> files = FileSystem::Directory::getFiles(path);
  // remove all the files inside the directory
  for (auto file : files)
    FileSystem::File::remove(path + "\\" + file);

  std::vector<std::string> dirs = FileSystem::Directory::getDirectories(path);
  for (auto dir : dirs)
  {
    // YOU MUST SKIP THESE OR YOU WILL ENTER IN INFINITE LOOP
    if (dir == "." || dir == "..") continue;
    recursiveRemoveDirectory(path + "\\" + dir);
  }
  // at this moment, the directory is empty and all 
  // its sub-directories are deleted, so just delete it now
  FileSystem::Directory::remove(path);
}
//----< receiver functionality is defined by this function >---------

void ClientHandler::operator()(Socket socket)
{
  while (true)
  {
    HttpMessage msg = readMessage(socket);
    if (connectionClosed_ || msg.bodyString() == "quit")
    {
      Show::write("\n\n  clienthandler thread is terminating");
      break;
    }

    std::string fromAddr = msg.findValue("fromAddr");
    HttpMessage sendMsg;

    std::string cmdStr = msg.findValue("Command");
    if (cmdStr == "GetFileDirs")
    {
      std::string path = msg.findValue("Path");
      std::string nopStr = msg.findValue("NoParent");
      bool noParent = false;;
      if (nopStr == "NoParent")
        noParent = true;

      // body msg: files and dirs
      std::string msgBody;

      FileSystem::Directory::setCurrentDirectory(rootPath_ + "\\" + path);
      std::string absPath = FileSystem::Directory::getCurrentDirectory();

      // add files & dirs to list
      std::vector<std::string> files = FileSystem::Directory::getFiles(absPath, "*.htm");
      for (auto file : files)
      {
        FileSystem::FileInfo fileInfo(absPath + "\\" + file);
        std::string fullName = fileInfo.name();
        // remove htm externsion
        std::string displayName = fullName.substr(0, fullName.size() - 4);
        std::string relaFilePath = path + "\\" + file;
        std::string origFilePath = relaFilePath.substr(0, relaFilePath.size() - 4);

        if (!noParent || depTable_.getDepFiles(origFilePath).size() == 0)
          msgBody += (displayName + ";" + fileInfo.date() + ";f,");
      }
      std::vector<std::string> dirs = FileSystem::Directory::getDirectories(absPath);
      for (auto dir : dirs)
      {
        if (dir == "." || dir == "..") continue;

        FileSystem::FileInfo fileInfo(absPath + "\\" + dir);
        msgBody += (fileInfo.name() + ";" + fileInfo.date() + ";d,");
      }

      sendMsg = makeMessage(1, msgBody, fromAddr);  //content-size is important!
      sendMsg.addAttribute(HttpMessage::Attribute("Content", "FileDirs"));
    }
    else if (cmdStr == "OpenFile")
    {
      std::string path = msg.findValue("Path");
      
      if (path == ".\\index")
      {
        sendFile(rootPath_ + "\\" + "index.htm", "index.htm", true, fromAddr, socket);

        for (auto item : depTable_)
        {
          std::string file = item.first;
          std::cout << "\n file: " << file;
          sendFile(rootPath_ + "\\" + file + ".htm", file + ".htm", false, fromAddr, socket);
        }
      }
      else {
        std::unordered_set<std::string> connectedFiles;
        findConnectedFiles(connectedFiles, path);

        for (auto file : connectedFiles)
        {
          if (file == (path))
            sendFile(rootPath_ + "\\" + file + ".htm", file + ".htm", true, fromAddr, socket);
          else
            sendFile(rootPath_ + "\\" + file + ".htm", file + ".htm", false, fromAddr, socket);
        }
      }

      continue;
    }
    else if (cmdStr == "DownloadCssJs")
    {
      std::string path = "template.css";
      sendFile(rootPath_ + "\\" + path, path, false, fromAddr, socket);
      path = "template.js";
      sendFile(rootPath_ + "\\" + path, path, false, fromAddr, socket);
      continue;
    }
    else if (cmdStr == "DelFile")
    {
      std::string path = msg.findValue("Path");
      // remove both source file and htm file
      FileSystem::File::remove(rootPath_ + "\\" + path);
      FileSystem::File::remove(rootPath_ + "\\" + path + ".htm");

      sendMsg = makeMessage(1, "File Delete OK", fromAddr);
      sendMsg.addAttribute(HttpMessage::Attribute("Content", "DelFile"));
    }
    else if (cmdStr == "DelDir")
    {
      std::string path = msg.findValue("Path");

      // remove entire directory
      recursiveRemoveDirectory(rootPath_ + "\\" + path);
      
      sendMsg = makeMessage(1, "Dir Delete OK", fromAddr);
      sendMsg.addAttribute(HttpMessage::Attribute("Content", "DelDir"));
    }
    else if (cmdStr == "Publish")
    {
      int argc = 5;
      char* argv[6];
      char path[256];
      strcpy(path, rootPath_.c_str());

      argv[0] = "";
      argv[1] = path;
      argv[2] = path;
      argv[3] = "*.h";
      argv[4] = "*.cpp";
      //argv[5] = "/r";

      publishCode(argc, argv);
      sendMsg = makeMessage(1, "Publish OK", fromAddr);
      sendMsg.addAttribute(HttpMessage::Attribute("Content", "Published"));
    }
    else {
      continue;
    }

    Show::write("\n\n  server sent\n" + sendMsg.toIndentedString());
    sendMessage(sendMsg, socket);
  }
}
//----< factory for creating messages >------------------------------
/*
* This function only creates one type of message for this demo.
* - To do that the first argument is 1, e.g., index for the type of message to create.
* - The body may be an empty string.
* - EndPoints are strings of the form ip:port, e.g., localhost:8081. This argument
*   expects the receiver EndPoint for the toAddr attribute.
*/
HttpMessage ClientHandler::makeMessage(size_t n, const std::string& body, const EndPoint& ep)
{
  HttpMessage msg;
  HttpMessage::Attribute attrib;
  EndPoint myEndPoint = "localhost:8080";  // ToDo: make this a member of the sender
                                           // given to its constructor.
  switch (n)
  {
  case 1:
    msg.clear();
    msg.addAttribute(HttpMessage::attribute("POST", "Message"));
    msg.addAttribute(HttpMessage::Attribute("mode", "oneway"));
    msg.addAttribute(HttpMessage::parseAttribute("toAddr:" + ep));
    msg.addAttribute(HttpMessage::parseAttribute("fromAddr:" + myEndPoint));

    msg.addBody(body);
    if (body.size() > 0)
    {
      attrib = HttpMessage::attribute("content-length", Converter<size_t>::toString(body.size()));
      msg.addAttribute(attrib);
    }
    break;
  default:
    msg.clear();
    msg.addAttribute(HttpMessage::attribute("Error", "unknown message type"));
  }
  return msg;
}

//----< send message using socket >----------------------------------

void ClientHandler::sendMessage(HttpMessage& msg, Socket& socket)
{
  std::string msgString = msg.toString();
  socket.send(msgString.size(), (Socket::byte*)msgString.c_str());
}

//----< send file using socket >-------------------------------------
/*
* - Sends a message to tell receiver a file is coming.
* - Then sends a stream of bytes until the entire file
*   has been sent.
* - Sends in binary mode which works for either text or binary.
*/
bool ClientHandler::sendFile(const std::string& localPath, const std::string& remotePath, bool openFile, const EndPoint& ep, Socket& socket)
{
  // assumes that socket is connected
  FileSystem::FileInfo fi(localPath);
  size_t fileSize = fi.size();
  std::string sizeString = Converter<size_t>::toString(fileSize);
  FileSystem::File file(localPath);
  file.open(FileSystem::File::in, FileSystem::File::binary);
  if (!file.isGood())
    return false;

  HttpMessage msg = makeMessage(1, "", ep);
  msg.addAttribute(HttpMessage::Attribute("Content", "File"));
  msg.addAttribute(HttpMessage::Attribute("file", FileSystem::Path::getName(localPath)));
  msg.addAttribute(HttpMessage::Attribute("path", remotePath));
  if (openFile)
    msg.addAttribute(HttpMessage::Attribute("open", "true"));
  else
    msg.addAttribute(HttpMessage::Attribute("open", "false"));
  msg.addAttribute(HttpMessage::Attribute("content-length", sizeString));
  Show::write("\n\n  file sent\n" + msg.toIndentedString());
  sendMessage(msg, socket);
  const size_t BlockSize = 2048;
  Socket::byte buffer[BlockSize];
  while (true)
  {
    FileSystem::Block blk = file.getBlock(BlockSize);
    if (blk.size() == 0)
      break;
    for (size_t i = 0; i < blk.size(); ++i)
      buffer[i] = blk[i];
    socket.send(blk.size(), buffer);
    if (!file.isGood())
      break;
  }
  file.close();
  return true;
}

//----< find all files required by lazy download >------------------

void ClientHandler::findConnectedFiles(std::unordered_set<std::string>& files, const std::string& relativePath)
{
  if (files.find(relativePath) != files.end())
    return;

  files.insert(relativePath);

  // recusively find connected files
  for (auto file : depTable_.getDepFiles(relativePath))
    findConnectedFiles(files, file);
}

//----< progressively create directories >---------------------------

void ClientHandler::superCreateDir(const std::string& path)
{
  size_t prepos = 0;

  while (true)
  {
    size_t pos = path.find("/", prepos);
    if (pos >= path.length())
      pos = path.find("\\", prepos);
    if (pos >= path.length())
      return;

    std::string spath = path.substr(0, pos);
    if (!FileSystem::Directory::exists(spath))
      FileSystem::Directory::create(spath);

    prepos = pos + 1;
  }
}

//----< call code publisher and return file map >--------------------

void ClientHandler::publishCode(int argc, char* argv[])
{
  CodeAnalysisExecutive exec;
  bool succeeded = exec.ProcessCommandLine(argc, argv);
  if (!succeeded) return;
  exec.setDisplayModes();
  exec.startLogger(std::cout);

  std::ostringstream tOut("CodePublisher - Version 1.0");
  Utils::sTitle(tOut.str(), 3, 92, tOut, '=');
  Rslt::write(tOut.str());

  Rslt::write("\n     " + exec.systemTime());
  Rslt::flush();
  exec.showCommandLineArguments(argc, argv);
  Rslt::write("\n");

  exec.getSourceFiles();
  exec.processSourceCode(true);
  exec.complexityAnalysis();
  exec.dispatchOptionalDisplays();
  exec.flushLogger();
  Rslt::write("\n");
  std::ostringstream out;
  out << "\n  " << std::setw(10) << "searched" << std::setw(6) << exec.numDirs() << " dirs";
  out << "\n  " << std::setw(10) << "processed" << std::setw(6) << exec.numFiles() << " files";
  out << "\n    Code Analysis completed";

  // do type analysis
  TypeAnal typeAnal(exec.getAnalysisPath());
  typeAnal.doTypeAnal();

  // do dependency analysis
  DepAnal depAnal(exec.getFileMap(), exec.getAnalysisPath());
  depAnal.initDepTable();
  depAnal.doDepAnal();
  depTable_ = depAnal.depTable();

  // publish code
  Publisher publisher(depAnal.depTable(), exec.getAnalysisPath(), exec.getPublishDir());
  publisher.doPublish();
  out << "\n    Code Publish completed";

  exec.stopLogger();
  Rslt::write(out.str());
  Rslt::write("\n");
}

//----< test stub >--------------------------------------------------

int main(int argc, char* argv[])
{
  ::SetConsoleTitle(L"HttpMessage Server - Runs Forever");

  //Show::attach(&std::cout);
  Show::start();
  Show::title("\n  HttpMessage Server started");

  BlockingQueue<HttpMessage> msgQ;

  try
  {
    SocketSystem ss;
    SocketListener sl(8080, Socket::IP6);
    ClientHandler cp(msgQ);
    cp.ProcessCommandLine(argc, argv);
    sl.start(cp);
    
    Show::write("\n -------------------\n  press key to exit: \n -------------------");
    std::cout.flush();
    std::cin.get();
  }
  catch (std::exception& exc)
  {
    Show::write("\n  Exeception caught: ");
    std::string exMsg = "\n  " + std::string(exc.what()) + "\n\n";
    Show::write(exMsg);
  }
}
