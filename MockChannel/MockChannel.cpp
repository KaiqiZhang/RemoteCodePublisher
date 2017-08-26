///////////////////////////////////////////////////////////////////////////////////////
// MockChannel.cpp - Demo for CSE687 Project #4, Spring 2015                         //
// - build as static library showing how C++\CLI client can use native code channel  //
// - MockChannel reads from sendQ and writes to recvQ                                //
//                                                                                   //
// Jim Fawcett, CSE687 - Object Oriented Design, Spring 2015                         //
///////////////////////////////////////////////////////////////////////////////////////

#define IN_DLL
#include "MockChannel.h"
#include "../Cpp11-BlockingQueue/Cpp11-BlockingQueue.h"
#include <string>
#include <thread>
#include <iostream>
#include "../Client/Client.h"
#include "../Logger/Logger.h"

using BQueue = Async::BlockingQueue< Message >;
using Show = StaticLogger<1>;

/////////////////////////////////////////////////////////////////////////////
// Sendr class
// - accepts messages from client for consumption by MockChannel
//
class Sendr : public ISendr
{
public:
  void postMessage(const Message& msg);
  BQueue& queue();
private:
  BQueue sendQ_;
};

void Sendr::postMessage(const Message& msg)
{
  sendQ_.enQ(msg);
}

BQueue& Sendr::queue() { return sendQ_; }

/////////////////////////////////////////////////////////////////////////////
// Recvr class
// - accepts messages from MockChanel for consumption by client
//
class Recvr : public IRecvr
{
public:
  Message getMessage();
  BQueue& queue();
private:
  BQueue recvQ_;
};

Message Recvr::getMessage()
{
  return recvQ_.deQ();
}

BQueue& Recvr::queue()
{
  return recvQ_;
}
/////////////////////////////////////////////////////////////////////////////
// MockChannel class
// - reads messages from Sendr and writes messages to Recvr
//
class MockChannel : public IMockChannel
{
public:
  MockChannel(ISendr* pSendr, IRecvr* pRecvr);
  void start();
  void stop();
private:
  std::thread connectThread_;
  std::thread sendThread_;
  std::thread recvThread_;
  Client* client_;
  ISendr* pISendr_;
  IRecvr* pIRecvr_;
  bool stop_ = false;

  void createConnectThread();
  void createSendThread();
  void createRecvThread();
};

//----< pass pointers to Sender and Receiver >-------------------------------

MockChannel::MockChannel(ISendr* pSendr, IRecvr* pRecvr) : pISendr_(pSendr), pIRecvr_(pRecvr)
{
  client_ = new Client();
}

//----< creates thread to read from sendQ and echo back to the recvQ >-------

void MockChannel::start()
{
  Show::write("\n  MockChannel starting up");

  // 1st thread: create socket and connect to server
  createConnectThread();

  // 2nd thread: send & parse thread
  createSendThread();

  // 3rd thread: recv & parse thread
  createRecvThread();
}

//----< create a thread to connect to server >-------------------------------

void MockChannel::createConnectThread()
{
  connectThread_ = std::thread(
    [this] {
    Client* pClient = dynamic_cast<Client*>(client_);

    if (pClient == nullptr)
    {
      Show::write("\n  failed to start Mock Channel\n\n");
      return;
    }

    pClient->connectServer();
  });
}
//----< create a thread to dequeue msg and send to server >------------------

void MockChannel::createSendThread()
{
  sendThread_ = std::thread(
    [this] {
    Sendr* pSendr = dynamic_cast<Sendr*>(pISendr_);
    Client* pClient = dynamic_cast<Client*>(client_);

    if (pSendr == nullptr || pClient == nullptr)
    {
      Show::write("\n  failed to start Mock Channel\n\n");
      return;
    }

    BQueue& sendQ = pSendr->queue();

    while (!stop_)
    {
      Show::write("\n  channel deQing message");
      Message msg = sendQ.deQ();  // will block here so send quit message when stopping

      pClient->parseAndSendMsg(msg);
    }
    Show::write("\n  Server stopping\n\n");
  });
}
//----< create a thread to receive msg from server and queue >----------------

void MockChannel::createRecvThread()
{
  recvThread_ = std::thread(
    [this] {
    Recvr* pRecvr = dynamic_cast<Recvr*>(pIRecvr_);
    Client* pClient = dynamic_cast<Client*>(client_);

    if (pRecvr == nullptr || pClient == nullptr)
    {
      Show::write("\n  failed to start Mock Channel\n\n");
      return;
    }

    BQueue& recvQ = pRecvr->queue();
    while (!stop_)
    {
      Message msg = pClient->recvAndParseMsg();
      Show::write("\n  channel enQing message");
      recvQ.enQ(msg);
    }
    Show::write("\n  Server stopping\n\n");
  });
}
//----< signal server thread to stop >---------------------------------------

void MockChannel::stop() { stop_ = true; }

//----< factory functions >--------------------------------------------------

ISendr* ObjectFactory::createSendr() { return new Sendr; }

IRecvr* ObjectFactory::createRecvr() { return new Recvr; }

IMockChannel* ObjectFactory::createMockChannel(ISendr* pISendr, IRecvr* pIRecvr) 
{ 
  return new MockChannel(pISendr, pIRecvr); 
}

#ifdef TEST_MOCKCHANNEL

//----< test stub >----------------------------------------------------------

int main()
{
  ObjectFactory objFact;
  ISendr* pSendr = objFact.createSendr();
  IRecvr* pRecvr = objFact.createRecvr();
  IMockChannel* pMockChannel = objFact.createMockChannel(pSendr, pRecvr);
  pMockChannel->start();
  pSendr->postMessage("Hello World");
  pSendr->postMessage("CSE687 - Object Oriented Design");
  Message msg = pRecvr->getMessage();
  std::cout << "\n  received message = \"" << msg << "\"";
  msg = pRecvr->getMessage();
  std::cout << "\n  received message = \"" << msg << "\"";
  pSendr->postMessage("stopping");
  msg = pRecvr->getMessage();
  std::cout << "\n  received message = \"" << msg << "\"";
  pMockChannel->stop();
  pSendr->postMessage("quit");
  std::cin.get();
}
#endif
