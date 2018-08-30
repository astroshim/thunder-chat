#ifndef __PACKET_H
#define __PACKET_H

#pragma once

#include "./Include.h"

const int MAXPDUSIZE      = 4096; /* 4KB */
const int PDUHEADERSIZE     = 4;
const int PDUBODYSIZE     = (MAXPDUSIZE-PDUHEADERSIZE);

// command length
const unsigned int COMMAND_SIZE = 2;
const unsigned int CLIENT_DATA_SIZE = (MAXPDUSIZE - PDUHEADERSIZE);

const unsigned short cmd_HELLO_DS_DSM                   = 1802;
const unsigned short cmd_HELLO_DSM_DS                   = 1803;

const unsigned short cmd_USER_CLOSE_DS_DSM              = 1804;

/*----------------------------------------------------------------------------
 *  Chat Client(DC) <------> Chat Server Manager CMD 1600~1699
 *  --------------------------------------------------------------------------*/
const unsigned short cmd_HEARTBEAT_DC_DSM               = 1610;
const unsigned short cmd_HEARTBEAT_DSM_DC               = 1611;

const unsigned short cmd_CHAT_BROADCAST                = 1620;
// const unsigned short cmd_CHAT_DSM_DS                = 1621;

#pragma pack(push, 2)
typedef struct
{
  unsigned short    command;
  unsigned short    length;
}PACKET_HEADER;

typedef struct
{
  PACKET_HEADER   header;
  char            data[PDUBODYSIZE];
}T_PACKET;

/*************************************************************
 * ChatServer ---> ChatManager
 *************************************************************/
typedef struct
{
  uint64_t uniqId;
  char message[1000];
}Tcmd_CHAT_BROADCAST;

// hello ChatServer -> ChatManager
typedef struct
{
  uint64_t uniqId;
}Tcmd_HELLO_DS_DSM;

// hello ChatManager -> ChatServer
typedef struct
{
  uint64_t uniqId;
  uint32_t iMaxUser;
  double   dHelloTime;
}Tcmd_HELLO_DSM_DS;

typedef struct
{
} Tcmd_HEARTBEAT_DC_DS;

typedef struct
{
} Tcmd_HEARTBEAT_DS_DC;

#pragma pack(pop)

#endif

