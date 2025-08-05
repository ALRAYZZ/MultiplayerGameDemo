#pragma once
#include "Network.h"
#include "GameProtocol.h"
#include "Renderer.h"

class Client
{
public:
	Client(const std::string& serverIp, uint16_t serverPort);
	~Client();
	bool initialize();
	void run();

private:
	void handleInput();
	void handlePacket(const Packet& packet);

	Network network;
	Renderer renderer;
	SOCKET socket;
	sockaddr_in serverAddr;
	uint32_t playerId;
	bool connected;
};