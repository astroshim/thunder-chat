/**
 * Author:  astrosim
 */
#include "../include/ChatManager.h"
#include "../include/ChatServer.h"
#include "../include/TcpSocket.h"
#include "../include/ServerSocket.h"
#include "../include/Thread.h"
#include "../include/ThreadWorker.h"
#include "../include/ThreadTic.h"
#include "../include/ThreadManager.h"
#include "../include/ServerInfoDNMgr.h"
#include "../include/NPUtil.h"
#include "../include/NPLog.h"
#include "../include/NPDebug.h"
#include "../include/ClientServer.h"
#include "../include/ClientChatServer.h"

#ifdef _FREEBSD
#include "../include/IOMP_KQUEUE.h"
#else
#include "../include/IOMP_EPoll.h"
#endif

#include "../include/CircularQueue.h"

#include "../include/SharedMemory.h"

#ifndef _FREEBSD
#include <curl/curl.h>
#endif

ChatManager::ChatManager()
  :m_pIOMP(NULL)
  ,m_iConnCount(0)
  ,m_pServerInfo(NULL)
  ,m_pWorkQueue(NULL)
{
  this->SetStarted(false);
  pthread_mutex_init(&m_lockClient, NULL);
  m_pchStatistics = new char[MAX_STATISTICS];
}

ChatManager::ChatManager(Properties& _cProperties)
  :m_pIOMP(NULL)
  ,m_iConnCount(0)
  ,m_pWorkQueue(new CircularQueue())
{
  this->SetStarted(false);
  pthread_mutex_init(&m_lockClient, NULL);
  m_pServerInfo = new ServerInfoDNMgr(_cProperties);
  m_pchStatistics = new char[MAX_STATISTICS];
}

ChatManager::~ChatManager()
{
  this->SetStarted(false);
  delete m_pIOMP;
  delete m_pServerInfo;
  delete m_pWorkQueue;
  delete [] m_pchStatistics;
}

const int ChatManager::GetCurrentUserCount()
{
  return m_iConnCount;
}

const int ChatManager::GetMaxUser()
{
  return m_pServerInfo->GetMaxUser() + m_pServerInfo->GetDNCnt();
}

const char* const ChatManager::GetIPAddr()
{
  return m_pServerInfo->GetIPAddr();
}

const int ChatManager::GetDNServerPort()
{
  return m_pServerInfo->GetDNPort();
}

const char* const ChatManager::GetLogFileName()
{
  return m_pServerInfo->GetLogFileName();
}

const int ChatManager::PostData(const char* _pData, const char* _pchURL, const char *_pchAuth)
{
#ifndef _FREEBSD

  /*
     CURL *curl;
     CURLcode res;

     curl = curl_easy_init();
     if(curl)
     {
     curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
     curl_easy_setopt(curl, CURLOPT_URL, _pchURL);
     curl_easy_setopt(curl, CURLOPT_POSTFIELDS, _pData);
     curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(_pData));

     curl_easy_setopt (curl, CURLOPT_TIMEOUT, 150L);
     curl_easy_setopt (curl, CURLOPT_CONNECTTIMEOUT, 15L);
     res = curl_easy_perform(curl);
     if (res != 0)
     {
     CNPLog::GetInstance().Log("cURL PostData perform error(%d)", res);
     }

     curl_easy_cleanup(curl);
     } else {
     CNPLog::GetInstance().Log("cURL PostData init error");
     return -1;
     }
     */

  CURL *curl;
  CURLcode res;

  struct curl_slist *headers=NULL;
  //headers = curl_slist_append(headers, "Authorization: Basic d2ViOnFuc3Rrcw==");
  headers = curl_slist_append(headers, _pchAuth);
  curl = curl_easy_init();
  if(curl)
  {
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
    curl_easy_setopt(curl, CURLOPT_URL, _pchURL);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, _pData);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(_pData));
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers );

    //curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
    //curl_easy_setopt(curl, CURLOPT_USERPWD, "Authorization: Basic d2ViOnFuc3Rrcw==" );
    ////curl_easy_setopt(curl, CURLOPT_USERPWD, "d2ViOnFuc3Rrcw==" );

    curl_easy_setopt (curl, CURLOPT_TIMEOUT, 150L);
    curl_easy_setopt (curl, CURLOPT_CONNECTTIMEOUT, 15L);
    res = curl_easy_perform(curl);
    if (res != 0)
    {
      CNPLog::GetInstance().Log("cURL perform error(%d)", res);
    }

    curl_easy_cleanup(curl);
  } else {
    CNPLog::GetInstance().Log("cURL init error");
    return -1;
  }

  if(headers) curl_slist_free_all(headers);

#endif
  return 0;
}

const int ChatManager::PostData(const char* _pData, const char* _pchURL)
{
#ifndef _FREEBSD
  CURL *curl;
  CURLcode res;

  curl = curl_easy_init();
  if(curl)
  {
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
    curl_easy_setopt(curl, CURLOPT_URL, _pchURL);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, _pData);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(_pData));

    curl_easy_setopt (curl, CURLOPT_TIMEOUT, 150L);
    curl_easy_setopt (curl, CURLOPT_CONNECTTIMEOUT, 15L);
    res = curl_easy_perform(curl);
    if (res != 0)
    {
      CNPLog::GetInstance().Log("cURL perform error(%d)", res);
    }

    curl_easy_cleanup(curl);
  } else {
    CNPLog::GetInstance().Log("cURL init error");
    return -1;
  }
#endif

  return 0;
}

void ChatManager::PutWorkQueue(const void* const _pVoid)
{
  m_pWorkQueue->EnQueue(_pVoid);
}

const void* const ChatManager::GetWorkQueue()
{
  return m_pWorkQueue->DeQueue();
}

void ChatManager::HealthCheckUsers()
{
  pthread_mutex_lock(&m_lockClient);

  std::list<Client*>::iterator iter = m_lstChatServer.begin();
  while( iter != m_lstChatServer.end() )
  {
    Client *pClient = static_cast<Client *>(*iter);
    if(pClient->GetType() == CLIENT_USER)
    {
      if(CNPUtil::GetMicroTime()-pClient->GetAccessTime() > TIME_ALIVE ||
          pClient->IsClosed())
      {
        CNPLog::GetInstance().Log("ChatManager::HealthCheckUsers Kill Client [%p] fd=(%d)=(%d)",
            pClient, pClient->GetSocket()->GetFd(), CNPUtil::GetUnixTime()-pClient->GetAccessTime());
        iter = m_lstChatServer.erase( iter );

#ifdef _FREEBSD
        m_pIOMP->DelClient(pClient, EVFILT_READ);
#else
        m_pIOMP->DelClient(pClient);
#endif
        delete pClient;
        m_iConnCount--;
      }
    }

    iter++;
  }
  pthread_mutex_unlock(&m_lockClient);
  // HealthCheckClosedList();
}

void ChatManager::UpdateEPoll(Client* const _pClient, const unsigned int _uiEvents)
{
#ifndef _FREEBSD
  m_pIOMP->ModifyFd(_pClient, _uiEvents);
#endif
}

#ifdef _FREEBSD
void ChatManager::AddEPoll(Client* const _pClient, const short _filter, const unsigned short _usFlags)
{
  m_pIOMP->AddClient(_pClient, _filter, _usFlags);
}
#else
void ChatManager::AddEPoll(Client* const _pClient, const unsigned int _uiEvents)
{
  m_pIOMP->AddClient(_pClient, _uiEvents);
}
#endif

void ChatManager::CloseClient(Client* const _pClient)
  //void ChatManager::CloseClient(Client* _pClient)
{
  CNPLog::GetInstance().Log("ChatManager::CloseClient ==> (%p)(%d)", _pClient, _pClient->GetSocket()->GetFd());
#ifdef _FREEBSD
  m_pIOMP->DelClient(_pClient, EVFILT_READ);
#else
  m_pIOMP->DelClient(_pClient);
#endif

  pthread_mutex_lock(&m_lockClient);
  m_lstChatServer.remove(_pClient);
  pthread_mutex_unlock(&m_lockClient);

  // if you SetState() before erase the Client in the client map,
  // it could occur the crash program.
  // because of SetState() is on a collision with HealthCheck
  _pClient->SetState(STATE_CLOSED);

  delete _pClient;

  m_iConnCount--;
}

const int ChatManager::SetDS(int* const _piMaxUser)
{
  *_piMaxUser   = m_pServerInfo->GetDSMaxUser();
  return 0;
}

const int ChatManager::MessageBroadcast(const T_PACKET &_tPacket)
{
  Tcmd_CHAT_BROADCAST *pChatPacket = (Tcmd_CHAT_BROADCAST *)_tPacket.data;
#ifdef _DEBUG
  CNPLog::GetInstance().Log("ClientChatServer::MessageBroadcast Broadcast 요청 받음 from uniqId=(%llu), length=(%d), message=(%s)", 
                              pChatPacket->uniqId, _tPacket.header.length, pChatPacket->message);
#endif

  std::list<Client *>::iterator iter = m_lstChatServer.begin();
  while( iter != m_lstChatServer.end() )
  {
    ClientChatServer *chatServer = static_cast<ClientChatServer *>(*iter);
    if( chatServer->GetUniqId() != pChatPacket->uniqId) 
    {
#ifdef _DEBUG
      CNPLog::GetInstance().Log("message broadcasting from manager to (%llu)", chatServer->GetUniqId());
#endif
      chatServer->GetSocket()->Write((char *)&_tPacket, PDUHEADERSIZE+_tPacket.header.length);
    }

    iter++;
  }
  return 0;
}

void ChatManager::DoFork(Process *_pProcess)
{
  /**
   *   IOMP Class Create
   */
#ifdef _FREEBSD
  m_pIOMP = new IOMP_KQUEUE(500000000); // 0.5 sec
#else
  m_pIOMP = new IOMP_EPoll(100);
#endif

  /**
   * Create Server Socket for ChatManager
   */
  Client *pServer = NULL;
  serverInfoMap &tmpMap = (serverInfoMap &)m_pServerInfo->GetPortMap();
  for(serverInfoMapItor itor = tmpMap.begin(); itor != tmpMap.end(); ++itor)
  {
    ServerSocket *pServerSocket = new ServerSocket(itor->first);
    // pServerSocket->SetReUse();
    if(pServerSocket->Bind(itor->second) < 0)
    {
      return;
    }
    if(pServerSocket->Listen(10) < 0)
    {
      return;
    }
    if(pServerSocket->SetNonBlock() < 0)
    {
      return;
    }
    pServerSocket->SetLinger();
    // pServerSocket->SetReUse();
    pServer = new ClientServer(pServerSocket);
    ///
    //   ServerSocket add to IOMP.
    //
#ifdef _FREEBSD
    m_pIOMP->AddClient(pServer, EVFILT_READ, EV_ADD|EV_ENABLE|EV_ERROR);
#else
    m_pIOMP->AddClient(pServer, EPOLLIN);

#endif
  }

  /**
   * Create Server Socket for ChatServer
   */
  Client *pDNServer = NULL;
  ServerSocket *pServerSocket = new ServerSocket(SERVER_PORT);
  if(pServerSocket->Bind(m_pServerInfo->GetDNPort()) < 0)
  {
    return;
  }
  if(pServerSocket->Listen(10000) < 0)
  {
    return;
  }
  if(pServerSocket->SetNonBlock() < 0)
  {
    return;
  }

  pServerSocket->SetLinger();
  // pServerSocket->SetReUse();
  pDNServer = new ClientServer(pServerSocket);

  ((ChatServer *)_pProcess)->SetServerSocket(pDNServer);

  CNPLog::GetInstance().BackupLogFile2((char *)m_pServerInfo->GetLogFileName());
  //SpawnProcessWithPipe(_pProcess, m_pServerInfo->GetDNCnt());
  SpawnProcess(_pProcess, m_pServerInfo->GetDNCnt());

  // Run Manager
  Run();
}

void ChatManager::Run()
{
  this->SetStarted(true);

  // CNPUtil::GetIPConfig(&m_iMacAddr, &m_iIPAddr);

  if(CNPLog::GetInstance().SetFileName((char *)(m_pServerInfo->GetLogFileName())) )
  {
    Assert(false, "LogFile create error! ");
    return;
  }

  // struct in_addr laddr;
  // laddr.s_addr = m_iIPAddr;

  // struct ether_addr haddr;
  // memcpy((void *)&haddr, (void *)&m_iMacAddr, 6);

  // Worker create.
  for(int i = 0; i < m_pServerInfo->GetThreadCount(THREAD_WORKER); i++)
  {
    Thread *t = new ThreadWorker(this);
    ThreadManager::GetInstance()->Spawn(t);
    CNPLog::GetInstance().Log("In ChatManager WORKER Create (%p,%lu) ", t, t->GetThreadID());
  }

  // ThreadTic *tTic = new ThreadTic(this);
  // ThreadManager::GetInstance()->Spawn(tTic);
  // CNPLog::GetInstance().Log("In ChatManager ThreadTic Create (%p) ", tTic);

  while(1)
  {
    int iEventCount;

    WatchChildProcess();

    if((iEventCount = m_pIOMP->Polling()) <= 0)
    {
  
#ifdef _DEBUG
      // CNPLog::GetInstance().Log("epoll_wait error errno=%d, strerror=(%s)", errno, strerror(errno));
#endif
      if(!GetStarted())
      {
        break;
      }
      continue;
    }

    for(int i = 0; i <iEventCount; ++i)
    {

#ifdef _FREEBSD
      struct kevent *tEvents = m_pIOMP->GetEventStructure();
      Client *pClient = static_cast<Client *>(tEvents[i].udata);
#else
      struct  epoll_event *tEvents= m_pIOMP->GetEventStructure();
      Client *pClient = (Client *)(tEvents[i].data.ptr);
#endif

      // ServerSocket event Check
      //if(pServer == pClient)
      if(pClient->GetType() == CLIENT_SERVER)
      {
        //m_pIOMP->ModifyFd(pServer);
#ifdef _DEBUG
        CNPLog::GetInstance().Log("ServerSocket Event !");
#endif
        Socket *pClientSocket;
        //if((pClientSocket = ((ServerSocket *)pServer->GetSocket())->Accept()) != NULL)
        //if((pClientSocket = ((ServerSocket *)pClient->GetSocket())->Accept()) != NULL)
        if((pClientSocket = static_cast<ServerSocket *>(pClient->GetSocket())->Accept()) != NULL)
        {
          pClientSocket->SetNonBlock();

          struct in_addr laddr;
          laddr.s_addr = static_cast<TcpSocket *>(pClientSocket)->GetClientIp();

          CNPLog::GetInstance().Log("ChatManager Accept ClientIp=(%s), conncnt=(%d), maxuser=(%d)",
              inet_ntoa(laddr), m_iConnCount, GetMaxUser());
          if(m_iConnCount >= GetMaxUser())
          {
            CNPLog::GetInstance().Log("Max User OverFlow! Force Kill CurrentUser=(%d), MaxUser=(%d)",
                m_iConnCount,
                GetMaxUser());
            delete pClientSocket;
            continue;
          }

          Client *pNewClient;
          if(static_cast<ServerSocket *>(pClient->GetSocket())->GetType() == SERVER_PORT)
          {
            pNewClient = new ClientChatServer(pClientSocket);
#ifdef _FREEBSD
            if(m_pIOMP->AddClient(pNewClient, EVFILT_READ, EV_ADD|EV_ENABLE|EV_ONESHOT|EV_ERROR) < 0)
#else
              if(m_pIOMP->AddClient(pNewClient, EPOLLIN|EPOLLET|EPOLLONESHOT) < 0)
#endif
              {
                delete pNewClient;
                continue;
              }

            pNewClient->SetState(STATE_WAIT);
            pNewClient->SetMainProcess(this);
            pthread_mutex_lock(&m_lockClient);
            m_lstChatServer.push_back((Client *)pNewClient);
            pthread_mutex_unlock(&m_lockClient);

            CNPLog::GetInstance().Log("NewClient(%p) fd ==>(%d) ",
                pNewClient,
                pNewClient->GetSocket()->GetFd());

            m_iConnCount++;
          }
          else
          {
            CNPLog::GetInstance().Log("There is no platform!");
            delete pClientSocket;
          }
        }
        continue;
      }

#ifdef _FREEBSD

      if( tEvents[i].flags & EV_ERROR )
      {
        CNPLog::GetInstance().Log("In KQUEUE disconnect (%p) (%d) errno=(%d)(%s)", pClient,
            pClient->GetSocket()->GetFd(), errno, strerror(errno));

        CloseClient(pClient);
        /*
           if( EventList[i].data == EBADF || tEvents[i].data == ENOENT )
           else
           */
      }

      if( tEvents[i].filter == EVFILT_READ )
      {
        CNPLog::GetInstance().Log("In KQUEUE READ (%p) (%d) ", pClient, pClient->GetSocket()->GetFd());
        pClient->SetAccessTime();
        PutWorkQueue(pClient);
      }

#else
      if(tEvents[i].events & (EPOLLERR | EPOLLHUP))
      {
        CNPLog::GetInstance().Log("In EPOLLERR or EPOLLHUP disconnect (%p) (%d) errno=(%d)(%s)", pClient,
            pClient->GetSocket()->GetFd(), errno, strerror(errno));

        CloseClient(pClient);
      }
      else if(tEvents[i].events & EPOLLIN)
      {
        //m_pIOMP->DelClient(pClient);
#ifdef _DEBUG
        CNPLog::GetInstance().Log("EPOLLIN Client %p, events=(%d)", pClient, tEvents[i].events);
#endif
        pClient->SetAccessTime();
        PutWorkQueue(pClient);
      }
      else if(tEvents[i].events & EPOLLOUT)
      {
        CNPLog::GetInstance().Log("EPOLLOUT Client %p, events=(%d)", pClient, tEvents[i].events);
      }
#endif

      continue;
    }
  }

  ThreadManager::GetInstance()->ReleaseInstance();
}

