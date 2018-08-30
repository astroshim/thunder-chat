#ifndef __CHATMANAGER_H
#define __CHATMANAGER_H

#include "./Process.h"
#include "./Packet.h"

class Client;
#ifdef _FREEBSD
class IOMP_KQUEUE;
#else
class IOMP_EPoll;
#endif
class ClientSocket;
class Properties;
class ServerInfoMgr;
class CircularQueue;
class ServerInfoDNMgr;

const unsigned int MAX_DN = 10;
const unsigned int MAX_CLIENT = 2000;
const unsigned int MAX_MAC_LEN = 20;

class ChatManager : public Process
{
  private:
    static const int TIME_ALIVE = 300; //
    //static const int TIME_ALIVE = 600;  //
    static const int MAX_STATISTICS = 20000; //

#ifdef _FREEBSD
    IOMP_KQUEUE *m_pIOMP;
#else
    IOMP_EPoll *m_pIOMP;
#endif

    int m_iConnCount;
    char *m_pchStatistics;

    /**
     * ChatManager Server Infomation
     */
    ServerInfoDNMgr *m_pServerInfo;
    CircularQueue *m_pWorkQueue;
    CircularQueue *m_pBBSQueue;

    // ChatServer
    list<Client *> m_lstChatServer;
    pthread_mutex_t m_lockClient;

    void DeleteAllMember();

    const int PingRoutine(Client *const _pClient, const int _iCommand);

  public:
    ChatManager();
    ChatManager(Properties &_cProperties);
    virtual ~ChatManager();

    void Run();
    const int GetDNServerPort();
    const char *const GetLogFileName();

    void DoFork(Process *_pProcess);

    const int GetMaxUser();
    const char *const GetIPAddr();
    const int GetCurrentUserCount();
    void HealthCheckUsers();

    const int MessageBroadcast(const T_PACKET &_tPacket);

#ifdef _FREEBSD
    void AddEPoll(Client *const _pClient, const short _filter, const unsigned short _usFlags);
#else
    void AddEPoll(Client *const _pClient, const unsigned int _uiEvents);
#endif
    void UpdateEPoll(Client *const _pClient, const unsigned int _uiEvents);
    void CloseClient(Client *const _pClient);

    void PutWorkQueue(const void *const _pVoid);
    const void *const GetWorkQueue();

    const int PostData(const char *_pData, const char *_pchURL);
    const int PostData(const char *_pData, const char *_pchURL, const char *_pchAuth);

    const int SetDS(int *const _piMaxUser);
};

#endif
