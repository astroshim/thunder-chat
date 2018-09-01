#include "../include/ClientServer.h"
#include "../include/NPUtil.h"

ClientServer::ClientServer()
    : m_iPort(0)
{
}

ClientServer::ClientServer(Socket *const _cSocket)
    : Client(_cSocket, CLIENT_SERVER),
      m_iPort(0)
{
}

ClientServer::~ClientServer()
{
}

const int ClientServer::ExecuteCommand(Thread *_pThread)
{
  T_PACKET tPacket;
  // PACKET_HEADER *pPacketHeader = (PACKET_HEADER *)m_cCBuff.GetHeaderPoint();
  char packetHeader[sizeof(PACKET_HEADER)+1];
  memset((char *)packetHeader, 0x00, sizeof(packetHeader));
  m_cCBuff.GetHeader(packetHeader);
  PACKET_HEADER *pPacketHeader = (PACKET_HEADER *)packetHeader;


  memset((char *)&tPacket, 0x00, sizeof(tPacket));
  if (Client::GetPacket((char *)&tPacket, pPacketHeader->length + PDUHEADERSIZE) < 0)
  {
    CNPLog::GetInstance().Log("In ClientServer::ExecuteCommand() GetPacketError!");
    return -1;
  }
  CNPLog::GetInstance().Log("ClientServer::ExecuteCommand(%p) command=(%d)", this, tPacket.header.command);

  switch (tPacket.header.command)
  {
    /*
    case MCHS_MS_HELLO_RES:
        WorkHello(tPacket);
        break;
*/
  default:
    CNPLog::GetInstance().Log("ClientServer::ExecuteCommand UNKNOWN PDU TYPE(%p) (%d)", this,
                              tPacket.header.command);
  }

  return 0;
}
