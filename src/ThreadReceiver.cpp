#include "../include/ThreadReceiver.h"
#include "../include/ChatServer.h"
#ifdef _FREEBSD
#include "../include/IOMP_KQUEUE.h"
#else
#include "../include/IOMP_EPoll.h"
#endif
#include "../include/Client.h"
#include "../include/NPLog.h"

ThreadReceiver::ThreadReceiver()
    : m_pChatServer(NULL)
{
  CNPLog::GetInstance().Log("ThreadReceiver Construct");
}

ThreadReceiver::ThreadReceiver(ChatServer *const _pChatServer)
    : m_pChatServer(_pChatServer)
{
  m_pChatServer = _pChatServer;
  CNPLog::GetInstance().Log("ThreadReceiver Construct");
}

ThreadReceiver::~ThreadReceiver()
{
  this->SetStarted(false);
  CNPLog::GetInstance().Log("ThreadReceiver Destruct");
}

void ThreadReceiver::Run()
{
  this->SetStarted(true);

  while (1)
  {
    Client *pClient = (Client *)m_pChatServer->GetReceiveQueue();
#ifdef _DEBUG
    CNPLog::GetInstance().Log("In ThreadReceiver [%p]thread Client got! (%p) fd=(%d)",
                              this, pClient, pClient->GetSocket()->GetFd());
#endif

    int iRet = 0;
    if ((iRet = pClient->FillFromSocket()) <= 0)
    {
      if (iRet == USER_CLOSE)
      {
        CNPLog::GetInstance().Log("Close In ThreadReceiver.(%p), fd=(%d)", pClient, pClient->GetSocket()->GetFd());
        m_pChatServer->CloseClient(pClient);
        continue;
      }
    }
    else
    {
      int bufferedPacketSize = 0;
#ifdef _USE_LT
      while ((bufferedPacketSize = pClient->IsValidPacket()) > 0)
      {
        if (pClient->ExecuteCommand(this) < 0)
        {
          break;
        }
      }
#else

      // Edge trigger 일때는 EAGAIN 까지 읽어야 한다.

      // if (pClient->IsValidPacket() > 0)
      while ((bufferedPacketSize = pClient->IsValidPacket()) > 0)
      {
#ifdef _DEBUG
        CNPLog::GetInstance().Log("In ThreadReceiver.(%p), fd=(%d), bufferedPacketSize=(%d)", 
                          pClient, pClient->GetSocket()->GetFd(), bufferedPacketSize);
#endif

        if (pClient->ExecuteCommand(this) > 0)
        {
          CNPLog::GetInstance().Log("In ThreadReceiver Go To the Sender (%p) fd=(%d)",  pClient, pClient->GetSocket()->GetFd());
          // m_pChatServer->PutSendQueue(pClient);
          // continue;
        }
      }
#endif

#ifdef _DEBUG
      CNPLog::GetInstance().Log("In ThreadReceiver [%p]thread Client (%p) fd=(%d), bufferedPacketSize=(%d)",
                                this, pClient, pClient->GetSocket()->GetFd(), bufferedPacketSize);
#endif
    }

#ifdef _FREEBSD
    unsigned short flags = 0;
    flags = EV_ADD|EV_ENABLE|EV_ONESHOT|EV_ERROR;
    m_pChatServer->AddEPoll(pClient, EVFILT_READ, flags);
#else
    unsigned int flags = 0;

    #ifdef _USE_LT
    flags = EPOLLIN;
    #endif

    #ifdef _USE_ET
    flags = EPOLLIN|EPOLLET;
    #endif

    #ifdef _USE_ONESHOT
    flags |= EPOLLONESHOT;

    // EPOLLONESHOT 일 경우는 user level 단에서 감시하도록 시켜야 함.
    m_pChatServer->UpdateEPoll(pClient, flags);
    #else 
    m_pChatServer->AddEPoll(pClient, flags);
    #endif
#endif
  }

  delete this;
  pthread_exit(NULL);
}
