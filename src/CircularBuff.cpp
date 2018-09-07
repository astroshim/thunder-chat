#include "../include/NPLog.h"
#include "../include/NPUtil.h"
#include "../include/CircularBuff.h"
#include "../include/Packet.h"
#include "../include/Socket.h"

CircularBuff::CircularBuff()
    : m_iHead(0), m_iTail(0), m_iBufferSize(0), m_iUseBufferSize(0), m_pchBufferHeader(NULL)
{
  NewBuffer(CIR_BUFSIZE);
}

CircularBuff::~CircularBuff()
{
  m_iHead = 0;
  m_iTail = 0;

  if (m_pchBufferHeader != NULL)
  {
    delete[] m_pchBufferHeader;
    m_pchBufferHeader = NULL;
  }
}

const int CircularBuff::NewBuffer(const int iSize)
{
  if (m_pchBufferHeader != NULL)
  {
    return -1;
  }

  m_pchBufferHeader = new char[iSize + 1];
  m_iBufferSize = iSize;
  m_iUseBufferSize = 0;

  return iSize;
}

void CircularBuff::InitBuffer()
{
  m_iHead = 0;
  m_iTail = 0;
  m_iUseBufferSize = 0;
  memset(m_pchBufferHeader, 0x00, sizeof(char) * m_iBufferSize);
}

const char* CircularBuff::GetBufferHeader()
{
  return m_pchBufferHeader;
}

const int CircularBuff::GetTotalSize()
{
  return m_iBufferSize;
}

const int CircularBuff::GetUsedSize()
{
  return m_iUseBufferSize;
}

const int CircularBuff::GetFreeSize()
{
  return (m_iBufferSize - m_iUseBufferSize);
}

const int CircularBuff::GetHeader(char *const _pchBuffer)
{
  int packetHeaderSize = sizeof(PACKET_HEADER);

  CNPLog::GetInstance().Log("CircularBuff::GetHeader packetHeaderSize=%d, m_iHead=%d, m_iTail=%d",
                            packetHeaderSize, m_iHead, m_iTail);

  if (m_iHead <= (m_iTail + packetHeaderSize))
  {
    memcpy(_pchBuffer, &m_pchBufferHeader[m_iHead], packetHeaderSize);
  }
  else
  {
    int iRightLen = m_iBufferSize - m_iHead; 

    CNPLog::GetInstance().Log("CircularBuff::GetHeader 버퍼가  역전되는 상황 m_iBufferSize=%d, m_iHead=%d, rightBuffLen=%d",
                            m_iBufferSize, m_iHead, iRightLen);

    if (packetHeaderSize <= iRightLen)
    {
      memcpy(_pchBuffer, &m_pchBufferHeader[m_iHead], packetHeaderSize);
    }
    else
    {
      memcpy(_pchBuffer, &m_pchBufferHeader[m_iHead], iRightLen);
      memcpy(((char *)_pchBuffer + iRightLen), m_pchBufferHeader, packetHeaderSize - iRightLen);
    }
  }

  return 0;
}

// const unsigned char *const CircularBuff::GetHeaderPoint()
// {
//   return (unsigned char *)&m_pchBufferHeader[m_iHead];
// }

void CircularBuff::PrintBufferDump()
{
  CNPLog::GetInstance().LogDump("CircularBuff", (char *)&m_pchBufferHeader[m_iHead], m_iUseBufferSize);
}

/**
*/
void CircularBuff::Resize()
{
  char *pchNewBuffer = new char[(m_iBufferSize + CIR_BUFSIZE) + 1];

  if (m_iHead <= m_iTail)
  {
    memcpy(pchNewBuffer, &m_pchBufferHeader[m_iHead], (m_iTail - m_iHead));
  }
  else
  {
    int iRightLen = m_iBufferSize - m_iHead;
    memcpy(pchNewBuffer, &m_pchBufferHeader[m_iHead], iRightLen);
    if (m_iTail > 0)
    {
      memcpy(&pchNewBuffer[iRightLen], m_pchBufferHeader, m_iTail);
    }
  }

  delete m_pchBufferHeader;
  m_pchBufferHeader = pchNewBuffer;

  m_iBufferSize = m_iBufferSize + CIR_BUFSIZE;
  m_iHead = 0;
  m_iTail = m_iUseBufferSize;
}

const int CircularBuff::Put(const char *const _pchBuffer, const int _iLength)
{
  if (_iLength > GetFreeSize())
  {
    return -1;
  }

  int iFree = 0;

  CNPLog::GetInstance().Log("CircularBuff::PUT 1. Head(%d),Tail(%d), lenth=(%d)", m_iHead, m_iTail, _iLength);

  if (m_iHead <= m_iTail + _iLength)
  {
    if (m_iHead == 0)
    {
      iFree = m_iBufferSize - m_iTail;
      memcpy(&m_pchBufferHeader[m_iTail], _pchBuffer, _iLength);
    }
    else
    {
      iFree = m_iBufferSize - m_iTail;

      CNPLog::GetInstance().Log("CircularBuff::PUT 2. Head(%d),Tail(%d), lenth=(%d), free=(%d)", m_iHead, m_iTail, _iLength, iFree);

      if (_iLength <= iFree)
      {
        memcpy(&m_pchBufferHeader[m_iTail], _pchBuffer, _iLength);
      }
      else
      {
        CNPLog::GetInstance().Log("CircularBuff::PUT 3. Head(%d),Tail(%d), lenth=(%d), free=(%d)", m_iHead, m_iTail, _iLength, iFree);

        memcpy(&m_pchBufferHeader[m_iTail], _pchBuffer, iFree);
        memcpy(m_pchBufferHeader, &_pchBuffer[iFree], _iLength - iFree);
      }
    }
  }
  else
  {
    CNPLog::GetInstance().Log("CircularBuff::PUT 4. Head(%d),Tail(%d), lenth=(%d), free=(%d)", m_iHead, m_iTail, _iLength, iFree);
    memcpy(&m_pchBufferHeader[m_iTail], _pchBuffer, _iLength);
  }

  m_iTail += _iLength;

  if (m_iBufferSize <= m_iTail)
  {
    m_iTail -= m_iBufferSize;
  }

  m_iUseBufferSize += _iLength;

  return _iLength;
}

const int CircularBuff::Get(char *const _pchBuffer, const int _iLength)
{
#ifdef _DEBUG
  CNPLog::GetInstance().Log("CircularBuff::Get getPacketSize=(%d) BufferUseBytes=(%d) Head(%d),Tail(%d), buff=(%p)",
                            _iLength, GetUsedSize(), m_iHead, m_iTail, m_pchBufferHeader);
#endif
  if (_iLength > GetUsedSize())
  {
    CNPLog::GetInstance().Log("CircularBuff::Get len=(%d) BufferUseBytes=(%d)", _iLength, GetUsedSize());
    return RECV_NOT_ENOUGH;
  }

  if (m_iHead <= (m_iTail + _iLength))
  {
    memcpy(_pchBuffer, &m_pchBufferHeader[m_iHead], _iLength);
  }
  else
  {
    int iRightLen = m_iBufferSize - m_iHead; 

    CNPLog::GetInstance().Log("CircularBuff::Get 버퍼가  역전되는 상황 (%p), m_iBufferSize=%d, m_iHead=%d, rightBuffLen=%d",
                            m_pchBufferHeader, m_iBufferSize, m_iHead, iRightLen);

    if (_iLength <= iRightLen)
    {
      memcpy(_pchBuffer, &m_pchBufferHeader[m_iHead], _iLength);
    }
    else
    {
      memcpy(_pchBuffer, &m_pchBufferHeader[m_iHead], iRightLen);
      memcpy(((char *)_pchBuffer + iRightLen), m_pchBufferHeader, _iLength - iRightLen);
    }
  }

  m_iHead += _iLength;
  if (m_iBufferSize <= m_iHead)
    m_iHead -= m_iBufferSize;

  m_iUseBufferSize -= _iLength;
  return _iLength;
}

#if _USE_LT
const int CircularBuff::Put(Socket *const _pSocket)
{
  int iReadLen = 0, iFree;
  int messageLengthInSocket = _pSocket->Available();

  if (messageLengthInSocket <= 0)
  {
    CNPLog::GetInstance().Log("ClientBuffer USER_CLOSE!(%p) fd=(%d), messageLengthInSocket=(%d), BufferFreeSize=(%d), BufferedPacketSize=(%d)", 
                m_pchBufferHeader, _pSocket->GetFd(), messageLengthInSocket, GetFreeSize(), m_iUseBufferSize);
    return USER_CLOSE;
  }

  if (messageLengthInSocket > GetFreeSize())
  {
    CNPLog::GetInstance().Log("ClientBuffer OverFlow!(%p) fd=(%d), messageLengthInSocket=(%d), BufferFreeSize=(%d), BufferedPacketSize=(%d)", 
                m_pchBufferHeader, _pSocket->GetFd(), messageLengthInSocket, GetFreeSize(), m_iUseBufferSize);
    // buffer overflow!
    return USER_CLOSE;

    /**
     * resize?
     Resize();
     */
  }
#ifdef _DEBUG
  CNPLog::GetInstance().Log("In CircularBuff::Put(%p) fd=(%d), messageLengthInSocket=(%d), BufferedPacketSize=(%d), BufferFreeSize=(%d), m_iHead=(%d), m_iTail=(%d)",
                            m_pchBufferHeader, _pSocket->GetFd(), messageLengthInSocket, m_iUseBufferSize, GetFreeSize(), m_iHead, m_iTail);
#endif

  if (m_iHead <= m_iTail)
  {
    if (m_iHead == 0)
    {
      iFree = m_iBufferSize - m_iTail;
      if ((iReadLen = _pSocket->Read((char *)&m_pchBufferHeader[m_iTail], messageLengthInSocket)) <= 0)
      {
        return USER_CLOSE;
      }
    }
    else
    {
      iFree = m_iBufferSize - m_iTail;

      if (messageLengthInSocket <= iFree)
      {
        if ((iReadLen = _pSocket->Read((char *)&m_pchBufferHeader[m_iTail], messageLengthInSocket)) <= 0)
        {
          return USER_CLOSE;
        }
        /**
         *
        #ifdef _DEBUG
        CNPLog::GetInstance().LogDump("CircularBuff::Put", (char *)&m_pchBufferHeader[m_iTail], messageLengthInSocket);
        #endif
        */
      }
      else
      {
        if ((iReadLen = _pSocket->Read((char *)&m_pchBufferHeader[m_iTail], iFree)) <= 0)
        {
          return USER_CLOSE;
        }
        int iTmp = 0;
        if ((iTmp = _pSocket->Read((char *)m_pchBufferHeader, messageLengthInSocket - iFree)) <= 0)
        {
          return USER_CLOSE;
        }
        iReadLen += iTmp;
      }
    }
  }
  else
  {
    if ((iReadLen = _pSocket->Read((char *)&m_pchBufferHeader[m_iTail], messageLengthInSocket)) <= 0)
    {
      return USER_CLOSE;
    }
  }

  m_iTail += iReadLen;

  if (m_iBufferSize <= m_iTail)
  {
    m_iTail -= m_iBufferSize;
  }
  m_iUseBufferSize += iReadLen;

#ifdef _DEBUG
  CNPLog::GetInstance().Log("In CircularBuff::Put Leave iReadLen=(%d), m_iUseBufferSize=(%d), m_iHead=(%d), m_iTail=(%d)",
                            iReadLen, m_iUseBufferSize, m_iHead, m_iTail);
#endif
  return iReadLen;
}
#else

const int CircularBuff::Put(Socket *const _pSocket)
{
  int iReadLen = 0, iFree;
  /*
  int messageLengthInSocket = _pSocket->Available();

  if (messageLengthInSocket <= 0)
  {
    CNPLog::GetInstance().Log("ClientBuffer USER_CLOSE!(%p) fd=(%d), messageLengthInSocket=(%d), BufferFreeSize=(%d), BufferedPacketSize=(%d)", 
                m_pchBufferHeader, _pSocket->GetFd(), messageLengthInSocket, GetFreeSize(), m_iUseBufferSize);
    return USER_CLOSE;
  }

  if (messageLengthInSocket > GetFreeSize())
  {
    CNPLog::GetInstance().Log("ClientBuffer OverFlow!(%p) fd=(%d), messageLengthInSocket=(%d), BufferFreeSize=(%d), BufferedPacketSize=(%d)", 
                m_pchBufferHeader, _pSocket->GetFd(), messageLengthInSocket, GetFreeSize(), m_iUseBufferSize);
    // buffer overflow!
    return USER_CLOSE;

    //  need resize?
    //  Resize();
  }

#ifdef _DEBUG
  CNPLog::GetInstance().Log("In CircularBuff::Put(%p) fd=(%d), messageLengthInSocket=(%d), BufferedPacketSize=(%d), BufferFreeSize=(%d), m_iHead=(%d), m_iTail=(%d)",
                            m_pchBufferHeader, _pSocket->GetFd(), messageLengthInSocket, m_iUseBufferSize, GetFreeSize(), m_iHead, m_iTail);
#endif
  */

  int readn;
  int sizen = 0; 
  char buf_in[102400] = {'\0'}; 
  char readbuf[256] = {'\0'}; 

  memset(buf_in, 0x00, sizeof(buf_in));
  while(1) { 
    memset(readbuf, 0x00, sizeof(readbuf));
    readn = read (_pSocket->GetFd(), readbuf, 256); 
    if (readn <= 0 ) { 
      if (EAGAIN == errno ) { 
        CNPLog::GetInstance().Log("EAGAIN!!!!!!!! sizen=(%d)", sizen);
        break; 
      } 

      return USER_CLOSE;
    } 
    sizen += readn; 
    if (sizen >= 102400) { 
      CNPLog::GetInstance().Log("size is big!!!!!!!! sizen=(%d)", sizen);
      return USER_CLOSE;
    } 

    CNPLog::GetInstance().Log("readn=(%d)", readn);
    memcpy(buf_in, readbuf, readn); 
  } 

  return Put(buf_in, sizen);

/*
  if (m_iHead <= m_iTail)
  {
    if (m_iHead == 0)
    {
      iFree = m_iBufferSize - m_iTail;
      memcpy((void *)&m_pchBufferHeader[m_iTail], buf_in, sizen); 
    }
    else
    {
      iFree = m_iBufferSize - m_iTail;
      if (sizen > iFree)
      {
        CNPLog::GetInstance().Log("Buffer Overflow!!!!!!!! iFree=(%d), sizen=(%d), head=(%d), tail=(%d)", 
                                      iFree, sizen, m_iHead, m_iTail);
        return USER_CLOSE;
      }

      if (sizen <= iFree)
      {
        memcpy(&m_pchBufferHeader[m_iTail], buf_in, sizen);
      }
      else
      {
        memcpy((void *)&m_pchBufferHeader[m_iTail], buf_in, iFree); 
        memcpy((void *)&m_pchBufferHeader, buf_in+iFree, sizen - iFree); 
      }
    }
  }
  else
  {
    memcpy((void *)&m_pchBufferHeader[m_iTail], buf_in, sizen); 
  }

  m_iTail += sizen;

  if (m_iBufferSize <= m_iTail)
  {
    m_iTail -= m_iBufferSize;
  }
  m_iUseBufferSize += sizen;

  CNPLog::GetInstance().Log("In CircularBuff::Put Leave sizen=(%d), m_iUseBufferSize=(%d), m_iHead=(%d), m_iTail=(%d)",
                            sizen, m_iUseBufferSize, m_iHead, m_iTail);
#ifdef _DEBUG
#endif
  return sizen;
  */
}
#endif