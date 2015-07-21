
#include "FreeRTOS.h"
#include "tcpServer.h"
#include "SimpleLinkWrapper.h"
#include "simplelink.h"
#include "sl_common.h"
#include "tcpServerTask.h"
#include "logger.h"

#define PORT	5001

extern g_isApConnected;
bool g_tcpServerInitialized = false;
uint16_t g_TcpServSocketId;
SlSockAddrIn_t  g_localAddr;



uint8_t initTcpServer()
{
    SlSockAddrIn_t  Addr;

    _u16          AddrSize = 0;
    _i32          Status = 0;
    _i16          newSockID = 0;
    _i16          recvSize = 0;

	if(!g_isApConnected)
	{
		if(!connectToAP())
			return false;
	}

	g_localAddr.sin_family = SL_AF_INET;
	g_localAddr.sin_port = sl_Htons((_u16)PORT);
	g_localAddr.sin_addr.s_addr = 0;

    g_TcpServSocketId = sl_Socket(SL_AF_INET,SL_SOCK_STREAM, 0);
    if( g_TcpServSocketId < 0 )
    {
        logger(Error, Log_TcpServer, "[initTcpServer] Create socket Error");
        return false;
    }

    AddrSize = sizeof(SlSockAddrIn_t);
    Status = sl_Bind(g_TcpServSocketId, (SlSockAddr_t *)&g_localAddr, AddrSize);
    if( Status < 0 )
    {
        sl_Close(g_TcpServSocketId);
        logger(Error, Log_TcpServer, "[initTcpServer] Socket address assignment Error");
        return false;
    }

    Status = sl_Listen(g_TcpServSocketId, 0);
    if( Status < 0 )
    {
        sl_Close(g_TcpServSocketId);
        logger(Error, Log_TcpServer, "[initTcpServer] Listen Error");
        return false;
    }

    g_tcpServerInitialized = true;

    logger(Info, Log_TcpServer, "[initTcpServer] Server initiated");

    return true;
}

uint8_t runTcpServer()
{
	if(!g_tcpServerInitialized)
		return false;

	uint16_t newSockID;

	SlSocklen_t remoteAddr;// = (SlSockAddrIn_t*) pvPortMalloc(sizeof(SlSockAddrIn_t));

    newSockID = sl_Accept(g_TcpServSocketId, ( struct SlSockAddr_t *) &remoteAddr,
                              (SlSocklen_t*) &remoteAddr);
    if( newSockID < 0 )
    {
        sl_Close(g_TcpServSocketId);
        logger(Error, Log_TcpServer, "[runTcpServer] Accept connection Error");
        return false;
    }

    SlSocklen_t* clientAddr = (SlSockAddrIn_t*) pvPortMalloc(sizeof(SlSockAddrIn_t));
    *clientAddr = remoteAddr;

    if(!tcpServerHandlerTaskInit(newSockID))
    {
    	logger(Error, Log_TcpServer, "[runTcpServer] Couldn't create task for new connection");
    	return false;
    }

    logger(Debug, Log_TcpServer, "[runTcpServer] New connection created");

    return true;
}
