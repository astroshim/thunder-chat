#ifndef __CLIENT_H
#define __CLIENT_H

#include "./Include.h"
#include "./Packet.h"
#include "./ClientSocket.h"
#include "./CircularBuff.h"

class Process;
class Thread;

const unsigned int MAX_ID     = 50;
const unsigned int MAX_ENC_KEY  = 32;

class Client
{
  private:
    Socket        *m_cSocket; 

    unsigned char   m_iState;
    ENUM_CLIENT_TYPE  m_eType;
    double        m_tAccessTime;    // Access time

  protected:
    CircularBuff  m_cCBuff;
    Process   *m_pMainProcess;  

  public: 
    Client();
    Client(Socket* const _cSocket, const ENUM_CLIENT_TYPE _eType);
    virtual ~Client();

    const unsigned int GetIp();
    const ENUM_CLIENT_TYPE GetType();
    void SetType(const ENUM_CLIENT_TYPE _eType);

    void InitCircularBuffer();
    Socket* const GetSocket();
    void SetSocketFd(const int _iFd);
    void SetMainProcess(Process* const _pMainProcess);

    //  virtual void ExecuteCommand(int _iPacketLen)=0;
    virtual const int ExecuteCommand(Thread *_pThread)=0;

    // 
    const int   FillFromSocket();

    // This functions are to get the data from CircularBuff
    const int   GetPacket(char* const _pchBuffer, const int _iLength);

    // set client state
    void SetState(const ENUM_CLIENT_STATE _state);

    // bit compare client state
    const int   GetState(const ENUM_CLIENT_STATE _state);
    const int   GetState();

    const bool  IsClosed();

    const double GetAccessTime();
    void  SetAccessTime();

    virtual const int SendToClient(const T_PACKET* const _stSendPacket, const int _iLen);
    const int   IsValidPacket();
};

#endif
