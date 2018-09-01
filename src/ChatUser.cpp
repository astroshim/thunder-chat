#include "../include/ChatUser.h"
#include "../include/NPUtil.h"
#include "../include/ChatServer.h"
#include "../include/ThreadReceiver.h"
#include "../include/BroadcastMessage.h"

ChatUser::ChatUser()
{
  memset((char *)&m_tSendPacket, 0x00, sizeof(T_PACKET));
  m_eSendMode = SEND_ONLY_FILE;
  pthread_mutex_init(&m_mtLst, NULL);

  // CNPUtil::GetUnixTimeStr()
}

ChatUser::ChatUser(Socket *const _cSocket)
    : Client(_cSocket, CLIENT_USER)
{
  memset((char *)&m_tSendPacket, 0x00, sizeof(T_PACKET));
  m_eSendMode = SEND_ONLY_FILE;
  pthread_mutex_init(&m_mtLst, NULL);
}

ChatUser::~ChatUser()
{
  FreePacket();
}

void ChatUser::FreePacket()
{
  std::list<T_PACKET *>::iterator pos;
  pthread_mutex_lock(&m_mtLst);
  pos = m_lstSendPacket.begin();
  while (pos != m_lstSendPacket.end())
  {
    delete *pos++;
  }
  pthread_mutex_unlock(&m_mtLst);
}

const int ChatUser::GetSendPacketCount()
{
  return m_lstSendPacket.size();
}

list<T_PACKET *> *const ChatUser::GetPacketList()
{
  return &m_lstSendPacket;
}

void ChatUser::AddPacketFront(T_PACKET *_pPacket)
{
  pthread_mutex_lock(&m_mtLst);
  m_lstSendPacket.push_front(_pPacket);
  pthread_mutex_unlock(&m_mtLst);
}

void ChatUser::RemovePacket(T_PACKET *_pPacket)
{
  pthread_mutex_lock(&m_mtLst);
  m_lstSendPacket.remove(_pPacket);
  pthread_mutex_unlock(&m_mtLst);
}

void ChatUser::SendPacket(const T_PACKET *_pPacket)
{
#ifdef _USE_SENDTHREAD
  pthread_mutex_lock(&m_mtLst);
  m_lstSendPacket.push_back(_pPacket);
  pthread_mutex_unlock(&m_mtLst);
#else
  // response directly
  if (GetSocket()->Write((char *)_pPacket, PDUHEADERSIZE + _pPacket->header.length) < 0)
  {
    CNPLog::GetInstance().Log("In ChatUser Write Error (%p)", this);
  }
  delete _pPacket;
#endif
}

void ChatUser::MessageBroadcast(BroadcastMessage *message)
{
  ChatServer *chatServer = NULL;
  if ((chatServer = dynamic_cast<ChatServer *>(m_pMainProcess)))
  {
    chatServer->PutBroadcastQueue(message, this);
  }
  else
  {
    CNPLog::GetInstance().Log("BroadcastMessage(%p) chatServer is NULL!! ", this);
  }
}

const int ChatUser::ExecuteCommand(Thread *_pThread)
{
  /**
   * todo: 여기서 \n 을 검사해서 보내면 된다.
   *       부하가 문제 된다면, 일단 skip 하자
   */

  if (GetType() == CLIENT_CHAT_MANAGER) // if message is comming from ChatManager.
                                        // this message must broadcast but not to be relayed to other managers!
  {
#ifdef _DEBUG
    CNPLog::GetInstance().Log("Broadcast 요청 (manager를 통해 전달해온 메세지) == client 에 broadcasting 만 하면 되는 메세지.");
#endif

    T_PACKET tPacket;
    // PACKET_HEADER *pPacketHeader = (PACKET_HEADER *)m_cCBuff.GetHeaderPoint();
    char packetHeader[sizeof(PACKET_HEADER)+1];
    memset((char *)packetHeader, 0x00, sizeof(packetHeader));
    m_cCBuff.GetHeader(packetHeader);
    PACKET_HEADER *pPacketHeader = (PACKET_HEADER *)packetHeader;


    memset((char *)&tPacket, 0x00, sizeof(tPacket));
    if (Client::GetPacket((char *)&tPacket, pPacketHeader->length + PDUHEADERSIZE) < 0)
    {
      CNPLog::GetInstance().Log("In ClientUser::ExecuteCommand() GetPacketError!");
      return -1;
    }

    if (tPacket.header.command == cmd_CHAT_BROADCAST)
    {
      BroadcastMessage *broadcastMessage = new BroadcastMessage();
      Tcmd_CHAT_BROADCAST *pChatPacket = (Tcmd_CHAT_BROADCAST *)tPacket.data;

      // int messageSize = pPacketHeader->length - sizeof(uint32_t);
      int messageSize = pPacketHeader->length - sizeof(uint64_t);
#ifdef _DEBUG
      CNPLog::GetInstance().Log("MESSAGE, packet_length=(%d), messageSize=(%d)(%s)", pPacketHeader->length, messageSize, pChatPacket->message);
#endif

      broadcastMessage->SetMessageSize(messageSize);
      broadcastMessage->SetMessage(pChatPacket->message, messageSize);
      broadcastMessage->SetSocketFd(GetSocket()->GetFd());
      broadcastMessage->SetMessageType(RELAYED_MESSAGE);

#ifdef _DEBUG
      CNPLog::GetInstance().Log("In ChatUser:: message (%d)(%s), buffedSize: %d",
                                broadcastMessage->GetSocketFd(),
                                broadcastMessage->GetMessage(),
                                broadcastMessage->GetMessageSize());
#endif
      MessageBroadcast(broadcastMessage);
    }
  }
  else  // this message is comming from client == plain text message.
        // so this message must be relayed to managers.
  {
#ifdef _DEBUG
    ChatServer *pServer = dynamic_cast<ChatServer *>(m_pMainProcess);
    CNPLog::GetInstance().Log("ChatUser::ExecuteCommand pServer=(%p)", pServer);
#endif

    BroadcastMessage *broadcastMessage = new BroadcastMessage();

    broadcastMessage->SetMessageSize(m_cCBuff.GetUsedSize());
    if (Client::GetPacket((char *)broadcastMessage->GetMessage(), broadcastMessage->GetMessageSize()) < 0)
    {
      CNPLog::GetInstance().Log("In ChatUser::ExecuteCommand() GetPacketError! ");
      return -1;
    }

    broadcastMessage->SetSocketFd(GetSocket()->GetFd());

#ifdef _DEBUG
    CNPLog::GetInstance().Log("In ChatUser:: message (%d)(%s), buffedSize: %d",
                              broadcastMessage->GetSocketFd(),
                              broadcastMessage->GetMessage(),
                              broadcastMessage->GetMessageSize());
#endif
    MessageBroadcast(broadcastMessage);
  }

  return 0;
}
