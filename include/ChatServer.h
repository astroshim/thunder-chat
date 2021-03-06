#ifndef __CHATSERVER_H
#define __CHATSERVER_H

#include "./Process.h"

class Socket;
class Client;
#ifdef _FREEBSD
class IOMP_KQUEUE;
#else
class IOMP_EPoll;
#endif
class ClientSocket;
class Properties;
class ServerInfoDN;
class CircularQueue;
class BroadcastMessage;

using namespace std;
#include <iostream>
#include <sstream>
#include <vector>

class ChatServer : public Process
{
  private:
    static const int TIME_ALIVE = 300; // 5��
    static const int MAX_COMPANY = 100;

    /**
     * ChatServer Server Infomation
     */
    ServerInfoDN *m_pServerInfo;
    int m_iConnCount;
    Client *m_pDNServerSocket; //

#ifdef _FREEBSD
    IOMP_KQUEUE *m_pIOMP;
#else
    IOMP_EPoll *m_pIOMP;
#endif

    CircularQueue *m_pReceiveQueue;
    CircularQueue *m_pSendQueue;
    CircularQueue *m_pBroadcastQueue;

    list<Client *> m_lstClient;
    pthread_mutex_t m_lockClient;

    list<ClientSocket *> m_lstChatManagerSocket;

    int m_iMaxUser;
    // char uniqId[32];
    unsigned long long m_uniqId;

  public:
    ChatServer();
    ChatServer(Properties &_cProperties);
    virtual ~ChatServer();

    void Run();
    Client *const GetServerSocket();
    const char *const GetLogFileName();

    const int ConnectToMgr();
    ClientSocket *const NegotiationWithManager(string server, int port);

    const int GetMaxUser();
    void SetMaxUser(const int _iMaxUser);

    const char *const GetIPAddr();
    const int GetCurrentUserCount();
    void HealthCheckUsers();

#ifdef _FREEBSD
    void AddEPoll(Client *const _pClient, const short _filter, const unsigned short _usFlags);
#else
    void AddEPoll(Client *const _pClient, const unsigned int _uiEvents);
#endif
    void UpdateEPoll(Client *const _pClient, const unsigned int _uiEvents);
    void AcceptClient(Socket *const _pClientSocket, ENUM_CLIENT_TYPE type);
    void CloseClient(Client *const _pClient);
    int RegisterManager();

    Client *const GetClient(const unsigned int _uiSessionKey);

    void PutReceiveQueue(const void *const _pVoid);
    const void *const GetReceiveQueue();

    void PutSendQueue(const void *const _pVoid);
    const void *const GetSendQueue();

    void PutBroadcastQueue(BroadcastMessage *message, Client *const _pClient);
    const void *const GetBroadcastQueue();

    const int GetDNServerPort();
    const int GetServerPort();
    void SetServerSocket(Client *_pClient);

    const unsigned long long GetUniqId();

    const uint64_t GetDownloadSize(const uint32_t _nBillNo);

    const unsigned int GetBandwidth(const char _chID);
    void MessageBroadcast(BroadcastMessage *_message);
    void MessageBroadcastToManagers(BroadcastMessage *_message);
    list<int> GetBroadcastTargets(BroadcastMessage *_message);
};

#endif
