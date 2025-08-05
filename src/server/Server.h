#pragma once
#include "Network.h"
#include "GameState.h"



class Server
{
public:
	Server(uint16_t port);
	~Server();
	bool initialize();
	void run();

private:
	void handlePacket(const Packet& packet, const sockaddr_in& clientAddr);

	Network network;
	GameState gameState;
	SOCKET socket;
	uint16_t port;
};