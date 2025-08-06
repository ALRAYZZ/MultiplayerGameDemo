#pragma once
#include "Network.h"
#include "GameProtocol.h"
#include "Renderer.h"
#include <queue>

class Client
{
public:
	Client(const std::string& serverIp, uint16_t serverPort);
	~Client();
	bool initialize();
	void run();

private:
	void tick();
	void handleInput();
	void handlePacket(const Packet& packet);
	void resendUnacknowledgedInputs();

	Network network;
	Renderer renderer;
	SOCKET socket;
	sockaddr_in serverAddr;
	uint32_t playerId;
	bool connected;
	uint32_t inputSequence;
	std::queue<InputPacket> unacknowledgedInputs; // Queue for unacknowledged inputs
};