#include "../include/ThreadAcceptor.h"
#include "../include/ChatServer.h"
#include "../include/TcpSocket.h"
#include "../include/ServerSocket.h"
#include "../include/Client.h"
#include "../include/ChatUser.h"
#include "../include/IOMP_Select.h"
#include "../include/NPLog.h"

ThreadAcceptor::ThreadAcceptor(ChatServer *const _pMainProcess)
    : m_pMainProcess(_pMainProcess)
{
  this->SetStarted(true);
  CNPLog::GetInstance().Log("ThreadAcceptor Construct");
}

ThreadAcceptor::ThreadAcceptor()
{
  this->SetStarted(false);
}

ThreadAcceptor::~ThreadAcceptor()
{
  this->SetStarted(false);
  CNPLog::GetInstance().Log("ThreadAcceptor Destruct");
}

void ThreadAcceptor::Run()
{
  pthread_detach(pthread_self());
  int iServerFd = m_pMainProcess->GetServerSocket()->GetSocket()->GetFd();

  IOMP *pcIomp = new IOMP_Select(0, 1000);
  pcIomp->AddFd(iServerFd);

  while (1)
  {
    int eventCnt = 0;
    if ((eventCnt = pcIomp->Polling()) <= 0)
    {
      continue;
    }

    if (pcIomp->CheckEvent(iServerFd))
    {
      Socket *pClientSocket;
      if ((pClientSocket = static_cast<ServerSocket *>(((Client *)m_pMainProcess->GetServerSocket())->GetSocket())->Accept()) != NULL)
      {
        if (m_pMainProcess->GetCurrentUserCount() >= m_pMainProcess->GetMaxUser())
        {
          CNPLog::GetInstance().Log("Acceptor Max User OverFlow! CurrentUser=(%d), MaxUser=(%d)",
                                    m_pMainProcess->GetCurrentUserCount(),
                                    m_pMainProcess->GetMaxUser());

          delete pClientSocket;
          sleep(5);
          continue;
        }

        struct in_addr laddr;
        laddr.s_addr = static_cast<TcpSocket *>(pClientSocket)->GetClientIp();

        pClientSocket->SetNonBlock();
        CNPLog::GetInstance().Log("Acceptor pClientSocket=(%p), ClientIp=(%s), CurrentUser=(%d), Max=(%d)",
                                  pClientSocket,
                                  inet_ntoa(laddr),
                                  m_pMainProcess->GetCurrentUserCount(),
                                  m_pMainProcess->GetMaxUser());

        m_pMainProcess->AcceptClient(pClientSocket, CLIENT_USER);
      }
    }
    else
    {
      CNPLog::GetInstance().Log("Acceptor EventCheck error!");
    }
  }

  pthread_exit(NULL);
}
