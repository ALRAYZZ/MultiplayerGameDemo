#pragma once
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>

class Network
{
public:
	Network();
	~Network();

	bool initialize();
	SOCKET createUdpSocket(bool nonBlocking = true);
	bool bindSocket(SOCKET socket, const std::string& ip, uint16_t port);
	bool sendPacket(SOCKET socket, const sockaddr_in& addr, const char* buffer, size_t size);
	bool receivePacket(SOCKET socket, char* buffer, size_t size, sockaddr_in& fromAddr, int& fromAddrLen);

private:
	bool initialized;
};