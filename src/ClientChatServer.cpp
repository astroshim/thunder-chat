#include "../include/ClientChatServer.h"
#include "../include/NPUtil.h"
#include "../include/ChatManager.h"

ClientChatServer::ClientChatServer()
{
}

ClientChatServer::ClientChatServer(Socket *const _cSocket)
    : Client(_cSocket, CLIENT_USER)
{
}

ClientChatServer::~ClientChatServer()
{
}

void ClientChatServer::SetUniqId(int _uniqId) { 
    m_uniqId = _uniqId;
}

const int ClientChatServer::GetUniqId() {
    return m_uniqId;
}

void ClientChatServer::WorkChatServerHello(const T_PACKET &_tPacket)
{
    int iMaxUser = 0;
    Tcmd_HELLO_DS_DSM *pClientBody = (Tcmd_HELLO_DS_DSM *)_tPacket.data;

    SetType(CLIENT_CHAT_SERVER);
    ChatManager *pManager = NULL;
    if ((pManager = dynamic_cast<ChatManager *>(m_pMainProcess)))
    {
        if (pManager->SetDS(&iMaxUser) < 0)
        {
            CNPLog::GetInstance().Log("WorkChatServerHello DS OverFlow");
            return;
        }
    }
    else
    {
        CNPLog::GetInstance().Log("Work WorkChatServerHello(%p) pManager is NULL!! ", this);
    }

    T_PACKET tPacket;
    Tcmd_HELLO_DSM_DS *sndbody = (Tcmd_HELLO_DSM_DS *)tPacket.data;
    memset((char *)&tPacket, 0x00, sizeof(T_PACKET));
    tPacket.header.command = cmd_HELLO_DSM_DS;
    tPacket.header.length = sizeof(Tcmd_HELLO_DSM_DS);

    sndbody->iMaxUser = iMaxUser;
    sndbody->uniqId = pClientBody->uniqId;

    m_uniqId = pClientBody->uniqId;
    CNPLog::GetInstance().Log("Hello Ack to uniqId=(%llu),maxUser=(%d)", pClientBody->uniqId, iMaxUser);

    GetSocket()->Write((char *)&tPacket, PDUHEADERSIZE + tPacket.header.length);
}

void ClientChatServer::WorkDSMPing(const T_PACKET &_tPacket)
{
    T_PACKET *pSendPacket = new T_PACKET;
    memset((char *)pSendPacket, 0x00, sizeof(T_PACKET));
    pSendPacket->header.command = cmd_HEARTBEAT_DSM_DC;
    pSendPacket->header.length = 0;

    CNPLog::GetInstance().Log("ClientChatServer::DSM Ping(%p)", this);
    //((Socket *)(GetSocket()))->Write((char *)pSendPacket, PDUHEADERSIZE+pSendPacket->header.length);
    GetSocket()->Write((char *)pSendPacket, PDUHEADERSIZE + pSendPacket->header.length);
    delete pSendPacket;
}

void ClientChatServer::MessageBroadcast(const T_PACKET &_tPacket)
{
    Tcmd_CHAT_BROADCAST *pChatPacket = (Tcmd_CHAT_BROADCAST *)_tPacket.data;
    ChatManager *pManager = NULL;
    if ((pManager = dynamic_cast<ChatManager *>(m_pMainProcess)))
    {
        pManager->MessageBroadcast(_tPacket);
    }
    else
    {
        CNPLog::GetInstance().Log("Work WorkGetDSInfo(%p) pManager is NULL!! ", this);
    }
}

const int ClientChatServer::ExecuteCommand(Thread *_pThread)
{
    // PACKET_HEADER *pPacketHeader = (PACKET_HEADER *)m_cCBuff.GetHeaderPoint();
    char packetHeader[PDUHEADERSIZE+1];
    memset((char *)packetHeader, 0x00, PDUHEADERSIZE+1);
    m_cCBuff.GetHeader(packetHeader);
    PACKET_HEADER *pPacketHeader = (PACKET_HEADER *)packetHeader;

#ifdef _DEBUG
    CNPLog::GetInstance().Log("ClientChatServer::ExecuteCommand packet length=(%d), command=(%d)", 
                            pPacketHeader->length, pPacketHeader->command);
#endif

    T_PACKET tPacket;
    memset((char *)&tPacket, 0x00, sizeof(tPacket));
    if (Client::GetPacket((char *)&tPacket, pPacketHeader->length + PDUHEADERSIZE) < 0)
    {
        CNPLog::GetInstance().Log("In ClientChatServer::ExecuteCommand() GetPacketError!");
        return -1;
    }

#ifdef _DEBUG
    CNPLog::GetInstance().Log("ClientChatServer::ExecuteCommand(%p) command=(%d)", this, tPacket.header.command);
#endif

    switch (tPacket.header.command)
    {
    case cmd_CHAT_BROADCAST:
        MessageBroadcast(tPacket);
        break;

    case cmd_HELLO_DS_DSM:
        WorkChatServerHello(tPacket);
        break;

    case cmd_HEARTBEAT_DC_DSM:
        WorkDSMPing(tPacket);
        break;

    default:
        //WorkDSMPing(tPacket);
        CNPLog::GetInstance().Log("ClientChatServer::ExecuteCommand UNKNOWN PDU TYPE(%p) (%d)", this,
                                  tPacket.header.command);
    }

    return 0;
}
