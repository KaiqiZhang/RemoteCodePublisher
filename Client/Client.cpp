/////////////////////////////////////////////////////////////////////
//  Client.cpp - Remote Code Publisher Client                      //
//  ver 1.0                                                        //
//  Language:      Visual C++ 2015                                 //
//  Platform:      Microsoft Surface, Windows 10                   //
//  Application:   Used to perform remote code publisher           //
//  Author:        Kaiqi Zhang, Syracuse University                //
//                 kzhang17@syr.edu                                //
/////////////////////////////////////////////////////////////////////

#include "Client.h"
#include <iostream>
#include "../FileSystem/FileSystem.h"
#include "../Utilities/Utilities.h"

//----< class constructor >----------------------------------------------
Client::Client()
{
  tempDir_ = FileSystem::Directory::getCurrentDirectory() + "\\TempDir";

  Show::attach(&std::cout);
  //Show::start();
}
//----< socket connect to remote server >--------------------------------

void Client::connectServer()
{
  try
  {
    while (!si_.connect("localhost", 8080))
    {
      Show::write("\n client waiting to connect");
      ::Sleep(100);
    }
  }
  catch (std::exception& exc)
  {
    Show::write("\n  Exeception caught: ");
    std::string exMsg = "\n  " + std::string(exc.what()) + "\n\n";
    Show::write(exMsg);
  }
}
//----< receive msg from GUI and translate to server msg >----------------

void Client::parseAndSendMsg(const Message& msg)
{
  HttpMessage httpMsg;

  std::vector<std::string> args = StringHelper::split(msg);
  if (args.size() == 0) return;

  std::string cmdStr = args[0];

  if (cmdStr == "GetFileDirs" && args.size() == 3)
  {
    httpMsg = makeMessage(1, "", "localhost::8080");
    httpMsg.addAttribute(HttpMessage::Attribute("Command", cmdStr));
    httpMsg.addAttribute(HttpMessage::Attribute("Path", args[1]));
    httpMsg.addAttribute(HttpMessage::Attribute("NoParent", args[2]));
  }
  else if (cmdStr == "Upload" && args.size() == 3)
  {
    std::string localPath = args[1];
    std::string remotePath = args[2];
    sendFile(localPath, remotePath, si_);
    return;
  }
  else if (cmdStr == "OpenFile" || cmdStr == "DelFile" || cmdStr == "DelDir" && args.size() == 2)
  {
    httpMsg = makeMessage(1, "", "localhost::8080");
    httpMsg.addAttribute(HttpMessage::Attribute("Command", cmdStr));
    httpMsg.addAttribute(HttpMessage::Attribute("Path", args[1]));
  }
  else if (cmdStr == "Publish" || cmdStr == "DownloadCssJs")
  {
    httpMsg = makeMessage(1, "", "localhost::8080");
    httpMsg.addAttribute(HttpMessage::Attribute("Command", cmdStr));
  }
  else {
    return;
  }

  Show::write("\n\n  client sent\n" + httpMsg.toIndentedString());
  sendMessage(httpMsg, si_);
}
//----< receive msg from remote server and translate to gui msg >-----------

Client::Message Client::recvAndParseMsg()
{
  HttpMessage httpMsg = readMessage(si_);
  while (connectionClosed_)
  {
    //Show::write("\n not connected or connection closed");
    ::Sleep(100);
    httpMsg = readMessage(si_);
  }

  if (httpMsg.bodyString() == "quit")
  {
    Show::write("\n\n  clienthandler thread is terminating");
    return "quit";
  }
  Show::write("\n\n  client received\n" + httpMsg.toIndentedString());

  Message returnMsg;
  std::string content = httpMsg.findValue("Content");
  if (content == "FileDirs")
  {
    returnMsg = "FileDirs," + httpMsg.bodyString();
  }
  else if (content == "DelFile" || content == "DelDir")
  {
    returnMsg = content + "," + httpMsg.bodyString();
  }
  else if (content == "File")
  {
    returnMsg = content + "," + httpMsg.findValue("path");
    std::string openStr = httpMsg.findValue("open");

    std::string downloadedFilePath = tempDir_ + "\\" + httpMsg.findValue("path");
    if (openStr == "true" && FileSystem::Path::getExt(downloadedFilePath) == "htm")
    {
      std::string command("start \"\" \"" + downloadedFilePath + "\"");
      std::system(command.c_str());
    }
  }
  else if (content == "Published")
    returnMsg = content;
  else
    returnMsg = "Invalid";

  return returnMsg;
}

//----< factory for creating messages >------------------------------
/*
* This function only creates one type of message for this demo.
* - To do that the first argument is 1, e.g., index for the type of message to create.
* - The body may be an empty string.
* - EndPoints are strings of the form ip:port, e.g., localhost:8081. This argument
*   expects the receiver EndPoint for the toAddr attribute.
*/
HttpMessage Client::makeMessage(size_t n, const std::string& body, const EndPoint& ep)
{
  HttpMessage msg;
  HttpMessage::Attribute attrib;
  EndPoint myEndPoint = "localhost:8081";  // ToDo: make this a member of the sender
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

void Client::sendMessage(HttpMessage& msg, Socket& socket)
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
bool Client::sendFile(const std::string& localPath, const std::string& remotePath, Socket& socket)
{
  // assumes that socket is connected
  FileSystem::FileInfo fi(localPath);
  size_t fileSize = fi.size();
  std::string sizeString = Converter<size_t>::toString(fileSize);
  FileSystem::File file(localPath);
  file.open(FileSystem::File::in, FileSystem::File::binary);
  if (!file.isGood())
    return false;

  HttpMessage msg = makeMessage(1, "", "localhost::8080");
  msg.addAttribute(HttpMessage::Attribute("file", FileSystem::Path::getName(localPath)));
  msg.addAttribute(HttpMessage::Attribute("path", remotePath));
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

//----< this defines processing to frame messages >------------------

HttpMessage Client::readMessage(Socket& socket)
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
bool Client::readFile(const std::string& filename, size_t fileSize, Socket& socket)
{
  std::string localFileName = tempDir_ + "\\" + filename;
  // create directory if not exist
  std::string filePath = FileSystem::Path::getPath(localFileName);
  superCreateDir(filePath);

  FileSystem::File file(localFileName);
  file.open(FileSystem::File::out, FileSystem::File::binary);
  if (!file.isGood())
  {
    /*
    * This error handling is incomplete.  The client will continue
    * to send bytes, but if the file can't be opened, then the server
    * doesn't gracefully collect and dump them as it should.  That's
    * an exercise left for students.
    */
    Show::write("\n\n  can't open file " + localFileName);
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
//----< progressively create directories >---------------------------

void Client::superCreateDir(const std::string& path)
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
//----< test stub >--------------------------------------------------

#ifdef TEST_CLIENT

int main(int argc, char* argv[])
{

  Client cl;
  cl.connectServer();

  try
  {
    std::thread sendThread(
      [] {
      while (true)
      {
        //Show::write("\n  channel deQing message");
        //Message msg = sendQ.deQ();  // will block here so send quit message when stopping
        //cl.parseAndSendMsg(msg);
      }
      Show::write("\n  Server stopping\n\n");
    });

    std::thread recvThread(
      [] {
      while (true)
      {
        /*Message msg = cl.recvAndParseMsg();
        Show::write("\n  channel enQing message");
        recvQ.enQ(msg);*/
      }
      Show::write("\n  Server stopping\n\n");
    });
  }
  catch (std::exception& exc)
  {
    Show::write("\n  Exeception caught: ");
    std::string exMsg = "\n  " + std::string(exc.what()) + "\n\n";
    Show::write(exMsg);
  }
}

#endif // TEST_CLIENT
