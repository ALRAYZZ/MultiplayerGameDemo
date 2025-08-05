#include <Network.h>
#include <iostream>


// Constructor initializes the network state as uninitialized
Network::Network() : initialized(false) {}

// Destructor cleans up the Winsock resources if initialized
Network::~Network()
{
	if (initialized)
	{
		// Cleanup Winsock
		WSACleanup();
	}
}

// Initializes Winsock and sets the initialized flag
bool Network::initialize()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		std::cerr << "WSAStartup failed: " << WSAGetLastError() << std::endl;
		return false;
	}
	initialized = true;
	return true;
}

// Creates a UDP socket and optionally sets it to non-blocking mode
SOCKET Network::createUdpSocket(bool nonBlocking)
{
	SOCKET socketHandle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (socketHandle == INVALID_SOCKET)
	{
		std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
		return INVALID_SOCKET;
	}

	if (nonBlocking)
	{
		u_long mode = 1;
		if (ioctlsocket(socketHandle, FIONBIO, &mode) != 0) // Trying to set socket to non-blocking mode
		{
			std::cerr << "Failed to set non-blocking mode: " << WSAGetLastError() << std::endl;
			closesocket(socketHandle);
			return INVALID_SOCKET;
		}
	}
	return socketHandle;
}

// Binds the socket to the specified IP address and port
bool Network::bindSocket(SOCKET socketHandle, const std::string& ip, uint16_t port)
{
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

	if (bind(socketHandle, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		std::cerr << "Bind failed: " << WSAGetLastError() << std::endl;
		return false;
	}
	return true;
}

// Sends a packet to the specified address using the provided socket
bool Network::sendPacket(SOCKET socketHandle, const sockaddr_in& addr, const char* buffer, size_t size)
{
	if (sendto(socketHandle, buffer, static_cast<int>(size), 0, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
		return false;
	}
	return true;
}

// Receives a packet from the socket and fills the fromAddr structure with the sender's address
bool Network::receivePacket(SOCKET socketHandle, char* buffer, size_t size, sockaddr_in& fromAddr, int& fromAddrLen)
{
	int bytesReceived = recvfrom(socketHandle, buffer, static_cast<int>(size), 0, (sockaddr*)&fromAddr, &fromAddrLen);
	if (bytesReceived <= 0)
	{
		if (bytesReceived == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK)
		{
			std::cerr << "Receive failed: " << WSAGetLastError() << std::endl;
		}
		return false;
	}
	return true;
}