#ifndef __CLIENTDN_H
#define __CLIENTDN_H

#include "./Client.h"

class ClientChatServer : public Client
{
private:
  uint64_t m_uniqId;

public:
  ClientChatServer();
  ClientChatServer(Socket *const _cSocket);
  virtual ~ClientChatServer();

  // ChatServer->ChatManager command
  void WorkChatServerHello(const T_PACKET &_tPacket);
  void WorkUserClose(const T_PACKET &_tPacket);

  // DC->DSM command
  void WorkDSMHello(const T_PACKET &_tPacket);
  void WorkGetDSInfo(const T_PACKET &_tPacket);
  void WorkDSMPing(const T_PACKET &_tPacket);
  void WorkGoodBye(const T_PACKET &_tPacket);

  void MessageBroadcast(const T_PACKET &_tPacket);
  const int ExecuteCommand(Thread *_pThread);

  const int GetUniqId();
  void SetUniqId(int _uniqId);
};

#endif
